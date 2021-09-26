/*
 * Copyright (C) 2004, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2009 Appcelerator Inc.
 * Copyright (C) 2009 Brent Fulgham <bfulgham@webkit.org>
 * All rights reserved.
 * Copyright (c) 2010-2015 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "AuthenticationJarWKC.h"
#include "ResourceHandleManagerWKC.h"
#include "ResourceHandleManagerWKCSSL.h"

#include "Base64.h"
#include "Cookie.h"
#include "CookieJar.h"
#include "CString.h"
#include "DataURL.h"
#include "HTTPParsers.h"
#include "MIMETypeRegistry.h"
#include "NotImplemented.h"
#include "ResourceError.h"
#include "ResourceHandle.h"
#include "ResourceHandleInternalWKC.h"
#include "TextEncoding.h"
#include "FrameLoaderClientWKC.h"
#include "DocumentLoader.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameView.h"
#include "Page.h"
#include "Settings.h"
#include "HTTPCacheWKC.h"
#if ENABLE(BLOB)
#include "BlobRegistryImpl.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <wtf/Threading.h>
#include <wtf/Vector.h>

#include <wkc/wkcpeer.h>

#if !PLATFORM(WIN_OS)
#include <sys/param.h>
#undef MAX_PATH
#define MAX_PATH MAXPATHLEN
#endif

#if COMPILER(MSVC)
#define strncasecmp _strnicmp
#endif

#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

namespace WebCore {

const double pollTimeSeconds = 0.001;

#define HTTP_DEFAULT_MAX_CONNECTIONS       3L
#define HTTP_RESERVED_CONNECTIONS          2L /* Synchronous XHR + OCSP */
#define WEBSOCKET_DEFAULT_MAX_CONNECTIONS  2L
#define WEBSOCKET_RESERVED_CONNECTIONS     1L /* WebInspector */
#define DEFAULT_TCP_MAX_CONNECTIONS   (HTTP_DEFAULT_MAX_CONNECTIONS + HTTP_RESERVED_CONNECTIONS + WEBSOCKET_DEFAULT_MAX_CONNECTIONS + WEBSOCKET_RESERVED_CONNECTIONS)

// for debug
#undef DEBUG_LOADING
#undef DEBUG_LOADING_MORE
#undef DEBUG_ERROR
#undef DEBUG_FUNCTION
#undef DEBUG_FUNCTION_MORE

#undef CURL_DEBUG_CALLBACK
#ifdef CURL_DEBUG_CALLBACK
#  undef CURL_DEBUG_INFO
#  define CURL_DEBUG_HTTPHEADER_IN
#  define CURL_DEBUG_HTTPHEADER_OUT
#  undef CURL_DEBUG_HTTPDATA_IN
#  undef CURL_DEBUG_HTTPDATA_OUT
#  undef CURL_DEBUG_SSLDATA_IN
#  undef CURL_DEBUG_SSLDATA_OUT
#endif

#ifdef DEBUG_LOADING
  #define LOADINGPRINTF(a)  wkcDebugPrintfPeer a
#else
  #define LOADINGPRINTF(a)
#endif

#ifdef DEBUG_LOADING_MORE
  #define LOADINGMOREPRINTF(a)  wkcDebugPrintfPeer a
#else
  #define LOADINGMOREPRINTF(a)
#endif

#ifdef DEBUG_ERROR
  #define ERRORPRINTF(a)  wkcDebugPrintfPeer a
#else
  #define ERRORPRINTF(a)
#endif

#ifdef DEBUG_FUNCTION
  #define FUNCTIONPRINTF(a)  wkcDebugPrintfPeer a
#else
  #define FUNCTIONPRINTF(a)
#endif

#if defined(DEBUG_FUNCTION) && defined(DEBUG_FUNCTION_MORE)
  #define FUNCTIONMOREPRINTF(a)  wkcDebugPrintfPeer a
#else
  #define FUNCTIONMOREPRINTF(a)
#endif

//
// Mutex
//
WKC_DEFINE_GLOBAL_PTR(void*, gShareMutex, 0);
WKC_DEFINE_GLOBAL_PTR(void*, gCookieMutex, 0);
WKC_DEFINE_GLOBAL_PTR(void*, gDnsMutex, 0);
WKC_DEFINE_GLOBAL_PTR(void*, gSSLSessionMutex, 0);
WKC_DEFINE_GLOBAL_PTR(void*, gConnectMutex, 0);

static void sharedResourceMutexFinalize(void)
{
    if (gShareMutex) {
        wkcMutexDeletePeer(gShareMutex);
        gShareMutex = 0;
    }
    if (gCookieMutex) {
        wkcMutexDeletePeer(gCookieMutex);
        gCookieMutex = 0;
    }
    if (gDnsMutex) {
        wkcMutexDeletePeer(gDnsMutex);
        gDnsMutex = 0;
    }
    if (gSSLSessionMutex) {
        wkcMutexDeletePeer(gSSLSessionMutex);
        gSSLSessionMutex = 0;
    }
    if (gConnectMutex) {
        wkcMutexDeletePeer(gConnectMutex);
        gConnectMutex = 0;
    }
}

static void sharedResourceMutexInitialize(void)
{
    sharedResourceMutexFinalize();
    gShareMutex       = wkcMutexNewPeer();
    gCookieMutex      = wkcMutexNewPeer();
    gDnsMutex         = wkcMutexNewPeer();
    gSSLSessionMutex  = wkcMutexNewPeer();
    gConnectMutex     = wkcMutexNewPeer();
}

static void* sharedResourceMutex(curl_lock_data data)
{
    switch (data) {
    case CURL_LOCK_DATA_SHARE:
        return gShareMutex;
    case CURL_LOCK_DATA_COOKIE:
        return gCookieMutex;
    case CURL_LOCK_DATA_DNS:
        return gDnsMutex;
    case CURL_LOCK_DATA_SSL_SESSION:
        return gSSLSessionMutex;
    case CURL_LOCK_DATA_CONNECT:
        return gConnectMutex;
    default:
        ASSERT_NOT_REACHED();
        return NULL;
    }
}

// libcurl does not implement its own thread synchronization primitives.
// these two functions provide mutexes for cookies, and for the global DNS
// cache.
static void curl_lock_callback(CURL* handle, curl_lock_data data, curl_lock_access access, void* userPtr)
{
    if (void* mutex = sharedResourceMutex(data))
        wkcMutexLockPeer(mutex);
}

static void curl_unlock_callback(CURL* handle, curl_lock_data data, void* userPtr)
{
    if (void* mutex = sharedResourceMutex(data))
        wkcMutexUnlockPeer(mutex);
}

//
// Utilities
//
static char *fastStrdup(const char *str)
{
    char *new_str;
    size_t size;

    if (!str) {
        return NULL;
    }
    size = strlen(str) + 1;

    new_str = (char *)fastMalloc(size);
    if (!new_str) {
        return NULL;
    }
    memcpy(new_str, str, size);

    return new_str;
}

static String hostAndPort(const KURL& kurl)
{
    String url;

    if (kurl.hasPort()) {
        char port_buf[8];
        snprintf(port_buf, 7, "%d", kurl.port());
        url = kurl.host() + ":" + port_buf;
    }
    else
        url = kurl.host();

    return url;
}

WKC::FrameLoaderClientWKC* frameloaderclientwkc(ResourceHandle* job)
{
    if (!job || !job->frameloaderclient())
        return 0;

    if (!job || !job->getInternal() || job->getInternal()->m_cancelled)
        return 0;

    if (!job->mainFrame() || !job->mainFrame()->view() || !job->mainFrame()->page())
        return 0;

    WebCore::FrameLoaderClient* flcwkc = (WebCore::FrameLoaderClient *)job->frameloaderclient();
    if (!flcwkc || !flcwkc->byWKC())
        return 0;

    return (WKC::FrameLoaderClientWKC*)(flcwkc);
}


static long ProtectionSpaceAuthSchemeTocURLType(ProtectionSpaceAuthenticationScheme scheme)
{
    switch (scheme) {
    case ProtectionSpaceAuthenticationSchemeDefault:
        return CURLAUTH_ANY;
    case ProtectionSpaceAuthenticationSchemeHTTPBasic:
        return CURLAUTH_BASIC;
    case ProtectionSpaceAuthenticationSchemeHTTPDigest:
        return CURLAUTH_DIGEST;
    case ProtectionSpaceAuthenticationSchemeHTMLForm:
        return CURLAUTH_NONE;
    case ProtectionSpaceAuthenticationSchemeNTLM:
        return CURLAUTH_NTLM;
    case ProtectionSpaceAuthenticationSchemeNegotiate:
        return CURLAUTH_GSSNEGOTIATE;
    case ProtectionSpaceAuthenticationSchemeClientCertificateRequested:
    case ProtectionSpaceAuthenticationSchemeServerTrustEvaluationRequested:
    case ProtectionSpaceAuthenticationSchemeUnknown:
    default:
        return CURLAUTH_NONE;
    }
}

//
//  Constructor and Destructor
//
ResourceHandleManager::ResourceHandleManager()
    : m_downloadTimer(this, &ResourceHandleManager::downloadTimerCallback)
    , m_rhmssl(0)
    , m_proxyPort(0)
    , m_proxyType(HTTP)
    , m_cookiesDeleting(false)
    , m_cookieMaxEntries(20) /* default */
    , m_cookieJarFileName(0)
    , m_tryProxySingleConnect(false)
    , m_httpConnections(HTTP_DEFAULT_MAX_CONNECTIONS)
    , m_maxWebSocketConnections(WEBSOCKET_DEFAULT_MAX_CONNECTIONS)
    , m_harmfulSiteFilter(false)
    , m_redirectInWKC(false)
    , m_DNSCacheTimeout(60 * 5) // 5 minutes
    , m_serverResponseTimeout(60)
    , m_connectTimeout(60)
    , m_acceptEncoding()
    , m_DoNotTrack(false)
#if ENABLE(WKC_HTTPCACHE)
    , m_readCacheTimer(this, &ResourceHandleManager::readCacheTimerCallback)
    , m_writeCacheTimer(this, &ResourceHandleManager::writeCacheTimerCallback)
#endif
{
    FUNCTIONPRINTF(("<rhm>ResourceHandleManager()"));

    curl_global_init(CURL_GLOBAL_ALL);
    m_curlMultiHandle = curl_multi_init();
    curl_multi_setopt(m_curlMultiHandle, CURLMOPT_MAXCONNECTS, m_httpConnections);

    sharedResourceMutexInitialize();

    m_curlShareHandle = curl_share_init();
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_LOCKFUNC, curl_lock_callback);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_UNLOCKFUNC, curl_unlock_callback);

    /* customized option */
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_COOKIE_MAX_ENTRIES, m_cookieMaxEntries);

    /* for dispatchSynchronousJob() */
    m_curlMultiSyncHandle = curl_multi_init();
    curl_multi_setopt(m_curlMultiSyncHandle, CURLMOPT_MAXCONNECTS, 1L);
    m_mainThreadId = wkcThreadCurrentThreadPeer();
    FUNCTIONPRINTF(("m_mainThreadId:%p", m_mainThreadId));
}

ResourceHandleManager::~ResourceHandleManager()
{
    FUNCTIONPRINTF(("<rhm>~ResourceHandleManager()"));

    removeAllRunningJobs();
#if ENABLE(WKC_HTTPCACHE)
    resetHTTPCache();
#endif

    delete m_rhmssl;
    m_rhmssl = 0;

    curl_multi_cleanup(m_curlMultiHandle);
    curl_share_cleanup(m_curlShareHandle);
    curl_multi_cleanup(m_curlMultiSyncHandle);
    sharedResourceMutexFinalize();
    curl_global_cleanup();
}

ResourceHandleManager* ResourceHandleManager::create()
{
    ResourceHandleManager* self;

    self = new ResourceHandleManager();
    if (!self)
        goto exit_func;

    if (!self->construct()) {
        delete self;
        self = 0;
        goto exit_func;
    }

exit_func:
    return self;
}

bool ResourceHandleManager::construct()
{
    m_rhmssl = ResourceHandleManagerSSL::create(this, m_curlMultiHandle, m_curlShareHandle);
    if (!m_rhmssl)
        return false;

    return true;
}

// Singleton
WKC_DEFINE_GLOBAL_CLASS_OBJ(ResourceHandleManager*, ResourceHandleManager, m_sharedInstance, 0);

bool ResourceHandleManager::createSharedInstance()
{
    if (m_sharedInstance)
        return true;

    m_sharedInstance = create();
    if (!m_sharedInstance)
        return false;

    return true;
}

ResourceHandleManager* ResourceHandleManager::sharedInstance()
{
    return m_sharedInstance;
}

void ResourceHandleManager::deleteSharedInstance()
{
    if (m_sharedInstance)
        delete m_sharedInstance;

    m_sharedInstance = 0;
}

bool ResourceHandleManager::isExistSharedInstance()
{
    return m_sharedInstance ? true : false;
}

//
// Force Terminate
//
void ResourceHandleManager::forceTerminateInstance()
{
    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();
    if (mgr)
        mgr->forceTerminate();
}

void ResourceHandleManager::forceTerminate()
{
    ResourceHandleManagerSSL::resetVariables();

#if ENABLE(WKC_HTTPCACHE)
    // should not call the function because they call delete / deref and memory related codes...
//    resetHTTPCache();
#endif

    m_curlMultiHandle = 0;
    m_curlShareHandle = 0;
    m_curlMultiSyncHandle = 0;
    m_mainThreadId = 0;
    sharedResourceMutexFinalize();
    curl_global_reset_variables();

    m_sharedInstance = 0;
}

//
// settings
//
void ResourceHandleManager::setMaxHTTPConnections(long number)
{
/* dispatchSynchronousJob() reserves one connection  */
/* OCSP reserves one connection  */

    if (number < 0)
        return;

    if (0 == number) {
        /* infinity */
        m_httpConnections = number;
    }
    else if (HTTP_RESERVED_CONNECTIONS < number) {
        m_httpConnections = number - HTTP_RESERVED_CONNECTIONS;
    }
    else
        return;

    curl_multi_setopt(m_curlMultiHandle, CURLMOPT_MAXCONNECTS, m_httpConnections);
}

void ResourceHandleManager::setMaxWebSocketConnections(long number)
{
/* WebInspector reserves one connection  */

    if (number < 0)
        return;

    if (0 == number) {
        /* infinity */
        m_maxWebSocketConnections = INT_MAX;
    }
    else if (WEBSOCKET_RESERVED_CONNECTIONS < number) {
        m_maxWebSocketConnections = number - WEBSOCKET_RESERVED_CONNECTIONS;
    }

}
void ResourceHandleManager::setMaxCookieEntries(long number)
{
    if (0 < number) {
        m_cookieMaxEntries = number;
        curl_share_setopt(m_curlShareHandle, CURLSHOPT_COOKIE_MAX_ENTRIES, number);
    }
}

void ResourceHandleManager::setHarmfulSiteFilter(bool enable)
{
    m_harmfulSiteFilter = enable;
}

void ResourceHandleManager::setRedirectInWKC(bool enable)
{
    m_redirectInWKC = enable;
}

void ResourceHandleManager::setDNSCacheTimeout(int sec)
{
    m_DNSCacheTimeout = sec;
}

void ResourceHandleManager::setServerResponseTimeout(int sec)
{
    m_serverResponseTimeout = sec;
}

void ResourceHandleManager::setConnectTimeout(int sec)
{
    m_connectTimeout = sec;
}

void ResourceHandleManager::setAcceptEncoding(const char* encodings)
{
    m_acceptEncoding = String::fromUTF8(encodings);
}

//
// handling HTTP
//
static String guessMIMETypeByURL(const KURL& kurl, long httpCode, bool isxhr = false)
{
    if (httpCode == 200) {
        String byPath = MIMETypeRegistry::getMIMETypeForPath(kurl.lastPathComponent());
        if (byPath == "application/octet-stream") {
            if (!kurl.hasPath())
                return "text/html";
            if (kurl.lastPathComponent().isEmpty())
                return "text/html";
            return isxhr ? byPath : "";
        }
        else
            return byPath;
    }
    else if (httpCode < 200 || httpCode == 204 || httpCode == 205 ||httpCode == 304) {
        // These response code MUST NOT has response body.
        return "";
    }
    else {
        // Other status code like 301,401,407 may return with text html body.
        return "text/html";
    }
}

static String guessMIMETypeByContent(unsigned char* data, int len)
{
    return wkcGuessMIMETypeByContentPeer((const unsigned char*)data, len);
}

static void handleLocalReceiveResponse(CURL* handle, ResourceHandle* job, ResourceHandleInternal* d)
{
    FUNCTIONPRINTF(("<rhm>handleLocalReceiveResponse()"));
    // since the code in headerCallback will not have run for local files
    // the code to set the URL and fire didReceiveResponse is never run,
    // which means the ResourceLoader's response does not contain the URL.
    // Run the code here for local files to resolve the issue.
    // TODO: See if there is a better approach for handling this.
    const char* hdr;
    CURLcode err = curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &hdr);
    ASSERT(CURLE_OK == err);
    (void)err;
    d->m_response.setURL(KURL(ParsedURLString, hdr));

    long httpCode = 0;
    err = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpCode);
    ASSERT(CURLE_OK == err);

    if (d->m_response.mimeType().isEmpty()) {
        d->m_response.setMimeType(extractMIMETypeFromMediaType(d->m_response.httpHeaderField("Content-Type")).lower());
        if (d->m_response.mimeType().isEmpty()) {
            d->m_response.setMimeType(guessMIMETypeByURL(d->m_response.url(), httpCode));
        }
    }

    if (d->client())
        d->client()->didReceiveResponse(job, d->m_response);
    if (d->m_cancelled)
        return;

    d->m_response.setResponseFired(true);
}

/*
 * This is being called for HTTP body in the response. 
 *
 * called with data after all headers have been processed via headerCallback
 */
static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
    FUNCTIONMOREPRINTF(("<rhm>writeCallback(%p)", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();
    char* ptrData = static_cast<char*>(ptr);

    size_t totalSize = size*nmemb;
    size_t dataSize = totalSize;

    if (d->m_cancelled)
        return 0;

#if LIBCURL_VERSION_NUM > 0x071200
    // We should never be called when deferred loading is activated.
    ASSERT(!d->m_defersLoading);
#endif


    // this shouldn't be necessary but apparently is. CURL writes the data
    // of html page even if it is a redirect that was handled internally
    // can be observed e.g. on gmail.com
    CURL* h = d->m_handle;
    long httpCode = 0;
    CURLcode err = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &httpCode);
    ASSERT(CURLE_OK == err);
    (void)err;
    if (100 <= httpCode && httpCode < 200) {
        return 0;
    }
    else if (300 <= httpCode && httpCode < 400) {
#if 0
        String location = d->m_response.httpHeaderField("location");
        if (location.isEmpty()) {
            return 0;
        }
        return totalSize;
#else
        if (d->m_doRedirectChallenge)
            return totalSize;
        if (!ptrData) {
            return 0;
        }
        // would be HTTP/0.9...
        if (!d->m_response.responseFired()) {
            d->m_response.setHTTPStatusCode(200);
            d->m_response.setURL(d->m_firstRequest.url());
            d->m_response.setExpectedContentLength(0);
            d->m_response.setTextEncodingName(String());
            d->m_response.setSuggestedFilename(String());
            d->m_response.setMimeType(guessMIMETypeByContent((unsigned char *)ptrData, totalSize));

            if (d->client())
                d->client()->didReceiveResponse(job, d->m_response);
            if (d->m_cancelled)
                return 0;

            d->m_response.setResponseFired(true);
        }
#endif
    }
    else if (401 == httpCode || 407 == httpCode) {
        d->m_receivedData->append(ptrData, totalSize);
        return totalSize;
    }

    if (!d->m_response.responseFired()) {
        if (d->m_response.mimeType().isEmpty()) {
            int len = (dataSize < (DATA_KEEP_LENGTH - d->m_recvDataLength)) ? dataSize : (DATA_KEEP_LENGTH - d->m_recvDataLength);
            memcpy(d->m_recvData + d->m_recvDataLength, ptrData, len);
            d->m_recvDataLength += len;
            String MIMEType = guessMIMETypeByContent(d->m_recvData, d->m_recvDataLength);
            if (MIMEType.isEmpty()) {
                if (d->m_recvDataLength < DATA_KEEP_LENGTH && d->m_recvDataLength < d->m_response.expectedContentLength()) {
                    return totalSize;
                }
                else
                    MIMEType = "application/octet-stream";
            }
            d->m_response.setMimeType(MIMEType);
            ptrData += len;
            dataSize -= len;
        }
        handleLocalReceiveResponse(h, job, d);
        if (d->m_cancelled)
            return 0;

        if (d->m_recvDataLength) {
            if (frameloaderclientwkc(job) &&!frameloaderclientwkc(job)->dispatchWillReceiveData(job, d->m_recvDataLength)) {
                return 0;
            }
            if (d->client() && !d->client()->willReceiveData(job, d->m_recvDataLength)) {
                return 0;
            }
            if (d->client())
                d->client()->didReceiveData(job, (const char *)d->m_recvData, d->m_recvDataLength, 0);
            if (d->m_cancelled)
                return 0;
            d->m_recvDataLength = 0;
        }
    }

    if (dataSize && frameloaderclientwkc(job) && !frameloaderclientwkc(job)->dispatchWillReceiveData(job, dataSize)) {
        return 0;
    }
    if (d->client() && !d->client()->willReceiveData(job, dataSize)) {
        return 0;
    }

    if (dataSize && d->client())
        d->client()->didReceiveData(job, ptrData, dataSize, 0);
    if (d->m_cancelled)
        return 0;

    LOADINGMOREPRINTF(("Received Data  [%p] len=%d", job, totalSize));
    return totalSize;
}

/*
 * This is being called for each HTTP header in the response. This includes '\r\n'
 * for the last line of the header.
 *
 * We will add each HTTP Header to the ResourceResponse and on the termination
 * of the header (\r\n) we will parse Content-Type and Content-Disposition and
 * update the ResourceResponse and then send it away.
 *
 */
static bool headerRealm(ResourceHandleInternal* d, int httpCode)
{
    Vector<String> list;

    String header;
    if (401 == httpCode)
        header = d->m_response.httpHeaderField("WWW-Authenticate");
    else
        header = d->m_response.httpHeaderField("Proxy-Authenticate");
    if (header.isNull())
        return false; // do nothing

    header = header.stripWhiteSpace();
    if (header.lower().startsWith("digest")) {
        header = header.right(header.length() - 6);
        header.split(",", list);
    }
    else if (header.lower().startsWith("basic")) {
        header = header.right(header.length() - 5);
        header = header.stripWhiteSpace();
        list.append(header);
    }

    for (int i = 0; i < list.size(); i++) {
        Vector<String> items;
        list[i].split("=", items);
        if (items.size() < 2) continue;
        if (items.size() > 2) {
            for (int j=2; j<items.size(); j++) {
                items[1] += "="+items[j];
            }
        }

        if (items[0].stripWhiteSpace().lower() != "realm") {
            continue;
        }
        String value = items[1].stripWhiteSpace();
        if (value[0]=='"') {
            value = value.right(value.length()-1);
        }
        if (value[value.length()-1]=='"') {
            value = value.left(value.length()-1);
        }
        if (401 == httpCode)
            d->m_webAuthRealm = value;
        else
            d->m_proxyAuthRealm = value;
        return true;
    }

    return false;
}

static ProtectionSpaceAuthenticationScheme valueToScheme(String value)
{
    ProtectionSpaceAuthenticationScheme scheme = ProtectionSpaceAuthenticationSchemeDefault;

    // support ntlm/digest/basic
    if (value.lower().startsWith("ntlm")) {
        scheme = ProtectionSpaceAuthenticationSchemeNTLM;
    }
    else if (value.lower().startsWith("digest")) {
        scheme = ProtectionSpaceAuthenticationSchemeHTTPDigest;
    }
    else if (value.lower().startsWith("basic")) {
        scheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;
    }
    return scheme;
}

static bool AuthenticateHeaderDuplicate(ResourceHandleInternal* d, String name, String value)
{
    // if Authenticate Header is wrong , do not set header field.
    // if there are Authenticate Headers more than 2, set just first header.
    String old_value;

    if (equalIgnoringCase(name, "WWW-Authenticate")) {
        old_value = d->m_response.httpHeaderField("WWW-Authenticate");
        name = "WWW-Authenticate";
    }
    else if (equalIgnoringCase(name, "Proxy-Authenticate")) {
        old_value = d->m_response.httpHeaderField("Proxy-Authenticate");
        name = "Proxy-Authenticate";
    }
    else {
        return true;
    }

    ProtectionSpaceAuthenticationScheme curAuthScheme = valueToScheme(value);
    ProtectionSpaceAuthenticationScheme oldAuthScheme = valueToScheme(old_value);

    switch (oldAuthScheme) {
    case ProtectionSpaceAuthenticationSchemeDefault:
        if (curAuthScheme != ProtectionSpaceAuthenticationSchemeDefault)
            return true;
        break;
    case ProtectionSpaceAuthenticationSchemeHTTPBasic:
        if (ProtectionSpaceAuthenticationSchemeHTTPBasic < curAuthScheme)
            return true;
        break;
    case ProtectionSpaceAuthenticationSchemeHTTPDigest:
        if (ProtectionSpaceAuthenticationSchemeHTTPDigest < curAuthScheme)
            return true;
        break;
    case ProtectionSpaceAuthenticationSchemeNTLM:
        if (ProtectionSpaceAuthenticationSchemeNTLM == curAuthScheme)
            return true;
        break;
    default:
        break;
    }
    return false;
}

static bool isAppendableHeader(const String &key)
{
    static const char* appendableHeaders[] = {
        "access-control-allow-headers",
        "access-control-allow-methods",
        "access-control-allow-origin",
        "access-control-expose-headers",
        "allow",
        "cache-control",
        "connection",
        "content-encoding",
        "content-language",
        "if-match",
        "if-none-match",
        "keep-alive",
        "pragma",
        "proxy-authenticate",
        "public",
        "server",
        "te",
        "trailer",
        "transfer-encoding",
        "upgrade",
        "user-agent",
        "vary",
        "via",
        "warning",
        "www-authenticate",
        0
    };

    // Custom headers start with 'X-', and need no further checking.
    if (key.startsWith("x-", /* caseSensitive */ false)) {
        return true;
    }

    for (unsigned i = 0; appendableHeaders[i]; ++i) {
        if (equalIgnoringCase(key, appendableHeaders[i])) {
            return true;
        }
    }

    return false;
}


static size_t headerCallback(char* ptr, size_t size, size_t nmemb, void* data)
{
    FUNCTIONMOREPRINTF(("<rhm>headerCallback(%p)", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();
    if (d->m_cancelled)
        return 0;

    ResourceHandleManager *rhm_self = ResourceHandleManager::sharedInstance();

#if LIBCURL_VERSION_NUM > 0x071200
    // We should never be called when deferred loading is activated.
    ASSERT(!d->m_defersLoading);
#endif

    size_t totalSize = size * nmemb;

    if (frameloaderclientwkc(job) && !frameloaderclientwkc(job)->dispatchWillReceiveData(job, totalSize)) {
        return 0;
    }
    if (d->client() && !d->client()->willReceiveData(job, totalSize)) {
        return 0;
    }

    String header;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(ptr);
    for (int i=0; i<totalSize; i++) {
        if (isASCII(p[i]))
            header.append(p[i]);
        else
            header.append(String::format("%%%02x", p[i]));
    }

    /*
     * a) We can finish and send the ResourceResponse
     * b) We will add the current header to the HTTPHeaderMap of the ResourceResponse
     *
     * The HTTP standard requires to use \r\n but for compatibility it recommends to
     * accept also \n.
     */
    if (header == String("\r\n") || header == String("\n")) {
        LOADINGMOREPRINTF(("Header Received[%p]", job));

        CURL* h = d->m_handle;

        const char* hdr;
        (void)curl_easy_getinfo(h, CURLINFO_EFFECTIVE_URL, &hdr);
        d->m_response.setURL(KURL(ParsedURLString, hdr));

        long httpCode = 0;
        (void)curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &httpCode);

        // SSL
        if (0 == httpCode && d->m_isSSL) {
            long connectCode = 0;
            (void)curl_easy_getinfo(h, CURLINFO_HTTP_CONNECTCODE, &connectCode);
            if (connectCode == 200) {
                return totalSize;
            }
            else if (connectCode == 407) {
                /* should callback like didReceiveAuthenticationChallenge() for Proxy Authentication */;
                /* If httpCode is 407 then it will calls didReceiveAuthenticationChallenge(). */;
                httpCode = 407;
            }
            else if (connectCode >= 300 && connectCode < 400) {
                /* Proxy's redirecting.... Is it avaiable??? */;
                /* should do something else */;
            }
            else {
                /* should do something else */;
                httpCode = connectCode;
            }
        }
        d->m_response.setHTTPStatusCode(httpCode);

        // 100 continue
        if (100 == httpCode) {
            return totalSize;
        }

        double contentLength = 0;
        (void)curl_easy_getinfo(h, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
        d->m_response.setExpectedContentLength(static_cast<long long int>(contentLength));

        d->m_response.setMimeType(extractMIMETypeFromMediaType(d->m_response.httpHeaderField("Content-Type")).lower());
        d->m_response.setTextEncodingName(extractCharsetFromMediaType(d->m_response.httpHeaderField("Content-Type")));
        d->m_response.setSuggestedFilename(filenameFromHTTPContentDisposition(d->m_response.httpHeaderField("Content-Disposition")));

        if (d->m_response.mimeType().isEmpty()) {
            d->m_response.setMimeType(guessMIMETypeByURL(d->m_response.url(), httpCode, d->m_firstRequest.targetType()==ResourceRequest::TargetIsXHR));
        }

        if ((401 == httpCode || 407 == httpCode) && d->m_response.isHTTP()) {
            headerRealm(d, httpCode);
            return totalSize;
        }

        if (httpCode < 300) {
            if (!d->m_webAuthURL.isEmpty()) {
                rhm_self->authJar()->setWebUserPassword(d->m_webAuthURL, ProtectionSpaceServerHTTP, d->m_webAuthScheme, d->m_webAuthRealm, d->m_webAuthUser, d->m_webAuthPass, "", true);
            }
        }

        // HTTP redirection
        if (300 <= httpCode && httpCode < 400) {
            String location = d->m_response.httpHeaderField("location");
            if (!location.isEmpty()) {
                KURL newURL = KURL(job->firstRequest().url(), location);

                if (httpCode < 304 && !d->m_webAuthURL.isEmpty()) {
                    rhm_self->authJar()->setWebUserPassword(d->m_webAuthURL, ProtectionSpaceServerHTTP, d->m_webAuthScheme, d->m_webAuthRealm, d->m_webAuthUser, d->m_webAuthPass, location, true);
                }

                if (WKC::EInclusionContentComposition < d->m_composition) {
                    d->m_firstRequest.setFirstPartyForCookies(newURL);
                }
                ResourceRequest redirectedRequest = job->firstRequest();
                redirectedRequest.setURL(newURL);

                // To match a http method when the curl performs redirect
                // (referenced lib/transfer.c,Curl_follow())
                switch(httpCode){
                    case 301:
                    case 302:
                        if(equalIgnoringCase(redirectedRequest.httpMethod(), "POST") ||
                           equalIgnoringCase(redirectedRequest.httpMethod(), "PUT")) {
                            redirectedRequest.setHTTPMethod("GET");
                        }
                        break;
                    case 303:
                        if(!equalIgnoringCase(redirectedRequest.httpMethod(), "GET")) {
                            redirectedRequest.setHTTPMethod("GET");
                        }
                        break;
                }

                if (!protocolHostAndPortAreEqual(job->firstRequest().url(), redirectedRequest.url())) {
                    redirectedRequest.clearHTTPOrigin();
                }

                if (d->client() && !d->m_isSynchronous)
                    d->client()->willSendRequest(job, redirectedRequest, d->m_response);
                if (d->m_cancelled)
                    return 0;

                d->m_response.setHTTPHeaderField("Location", String());

                String x_frame_options = d->m_response.httpHeaderField("X-Frame-Options");
                if (!x_frame_options.isEmpty()) {
                    d->m_response.setHTTPHeaderField("X-Frame-Options", String());
                }

                d->m_firstRequest.setURL(newURL);

                if (rhm_self->didReceiveRedirect(job, newURL))
                    return totalSize;
            }
        }

        if (!d->m_response.mimeType().isEmpty()) {
            if (d->client())
                d->client()->didReceiveResponse(job, d->m_response);
            if (d->m_cancelled)
                return 0;

            d->m_response.setResponseFired(true);
        }
    }
    else {
        size_t splitPos = header.find(":");
        if (splitPos != notFound) {
            String name = header.left(splitPos);
            String value = header.substring(splitPos+1).stripWhiteSpace();

            if (!AuthenticateHeaderDuplicate(d, name, value)) {
                return totalSize;
            }

            if (isAppendableHeader(name)) {
                d->m_response.addHTTPHeaderField(name, value);
            }
            else {
                d->m_response.setHTTPHeaderField(name, value);
            }

        }
        /* find status text and set it */
        else if (header.find("HTTP/", 0, false) != notFound) {
            long httpcode = 0;
            CURL* h = d->m_handle;
            int ret = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &httpcode);

            if (d->m_isSSL && frameloaderclientwkc(job)) {
                long connectCode = 0;
                (void)curl_easy_getinfo(h, CURLINFO_HTTP_CONNECTCODE, &connectCode);
                if (httpcode
                    || (connectCode != 0 && connectCode != 407 && header.find("Connection established", 0, false) == notFound)) {
                    rhm_self->rhmssl()->SSLHandshakeInfo(job);
                    frameloaderclientwkc(job)->notifySSLHandshakeStatus(job, WKC::EHandshakeSuccess);
                    d->m_SSLHandshaked = true;
                }
            }

            if (ret == CURLE_OK) {
                char buf[4] = {0};
                size_t pos;
                int len;
                len = snprintf(buf, 3, "%ld", httpcode);
                if (len > 0) {
                    buf[len] = '\0';
                    pos = header.find(buf);
                    if (pos != notFound) {
                        pos += len;
                        d->m_response.setHTTPStatusText(header.substring(pos).stripWhiteSpace());
                    }
                }
            }
        }
    }

    return totalSize;
}

/* This is called to obtain HTTP POST, PUT or OPTIONS data.
   Iterate through FormData elements and upload files.
   Carefully respect the given buffer size and fill the rest of the data at the next calls.
*/
static size_t readCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
    FUNCTIONMOREPRINTF(("<rhm>readCallback(%p)", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();

    if (d->m_cancelled)
        return 0;

#if LIBCURL_VERSION_NUM > 0x071200
    // We should never be called when deferred loading is activated.
    ASSERT(!d->m_defersLoading);
#endif

    if (!size || !nmemb)
        return 0;

    if (!d->m_formDataStream.hasMoreElements())
        return 0;

    size_t sent = d->m_formDataStream.read(ptr, size, nmemb);
    if (sent == 0)
        sent = CURL_READFUNC_NODATA;

    return sent;
}

/* This is called to obtain HTTP POST, PUT or OPTIONS data and to receive redirect. */
static curlioerr ioctlCallback(CURL *handle, int cmd, void *clientp)
{
    FUNCTIONMOREPRINTF(("<rhm>ioctlCallback(%p)", clientp));

    ResourceHandle* job = static_cast<ResourceHandle*>(clientp);
    ResourceHandleInternal* d = job->getInternal();
    if (!d || !d->m_handle)
        return CURLIOE_OK;

    // erase "Content-Type" header
    struct curl_slist *cur_headers = d->m_customHeaders;
    struct curl_slist *new_headers = NULL;
    while (cur_headers) {
        if (cur_headers->data) {
            if (strncasecmp(cur_headers->data, "Content-Type:", 13)) {
                new_headers = curl_slist_append(new_headers, cur_headers->data);
            }
        }
        cur_headers = cur_headers->next;
    }
    if (new_headers) {
        curl_slist_free_all(d->m_customHeaders);
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPHEADER, new_headers);
        d->m_customHeaders = new_headers;
    }

    /* no operation */
    return CURLIOE_OK;
}

#ifdef CURL_DEBUG_CALLBACK
int debugCallback(CURL *handle, curl_infotype type,  char *data, size_t size, void *userptr)
{
    ResourceHandle* job = static_cast<ResourceHandle*>(userptr);
    ResourceHandleInternal* d = job->getInternal();
    if (!d || !d->m_handle || d->m_handle != handle)
        return 0;

    int len = size;
    char* buff = NULL;

    if (type == CURLINFO_TEXT || type == CURLINFO_HEADER_IN || type == CURLINFO_HEADER_OUT) {
        buff = (char*)fastMalloc(size+1);
        if (!buff)
            return 0;
        memcpy(buff, data, size);
        buff[size] = 0x00;
    }

    switch (type) {
    case CURLINFO_TEXT:
#ifdef CURL_DEBUG_INFO
        if (2 < len) {
            if (buff[len-2] == '\r' || buff[len-2] == '\n') {
                buff[len-2] = 0x00;
            }
            else {
                buff[len-1] = 0x00;
            }
            wkcDebugPrintfPeer("[curl log] %s", buff);
        }
#endif
        break;

    case CURLINFO_HEADER_IN:
#ifdef CURL_DEBUG_HTTPHEADER_IN
        if (2 < len) {
            if (buff[len-2] == '\r' || buff[len-2] == '\n') {
                buff[len-2] = 0x00;
            }
            else if (buff[len-1] == '\r' || buff[len-1] == '\n') {
                buff[len-1] = 0x00;
            }
            wkcDebugPrintfPeer("[curl log] Receive Header for (job:%p): %s", job, buff);
        }
#endif
        break;

    case CURLINFO_HEADER_OUT:
#ifdef CURL_DEBUG_HTTPHEADER_OUT
        if (4 < len) {
            if (buff[len-4] == '\r' || buff[len-4] == '\n') {
                buff[len-4] = 0x00;
            }
            else if (buff[len-3] == '\r' || buff[len-3] == '\n') {
                buff[len-3] = 0x00;
            }
            else if (buff[len-2] == '\r' || buff[len-2] == '\n') {
                buff[len-2] = 0x00;
            }
            else if (buff[len-1] == '\r' || buff[len-1] == '\n') {
                buff[len-1] = 0x00;
            }
            wkcDebugPrintfPeer("[curl log] Send Header for (job:%p) %s:\n=====\n%s\n=====", job, d->m_url, buff);
        }
#endif
        break;

    case CURLINFO_DATA_IN:
#ifdef CURL_DEBUG_HTTPDATA_IN
        // data may be binary
        wkcDebugPrintfPeer("[curl log] Send Data for (job:%p): CURLINFO_DATA_IN", job);
#endif
        break;

    case CURLINFO_DATA_OUT:
#ifdef CURL_DEBUG_HTTPDATA_OUT
        // data may be binary
        wkcDebugPrintfPeer("[curl log] Receive Data for (job:%p): CURLINFO_DATA_OUT", job);
#endif
        break;

    case CURLINFO_SSL_DATA_IN:
#ifdef CURL_DEBUG_SSLDATA_IN
        // data is binary
        wkcDebugPrintfPeer("[curl log] SSL   Send  Data for (job:%p): CURLINFO_SSL_DATA_IN", job);
#endif
        break;

    case CURLINFO_SSL_DATA_OUT:
#ifdef CURL_DEBUG_SSLDATA_OUT
        // data is binary
        wkcDebugPrintfPeer("[curl log] SSL Receive Data for (job:%p): CURLINFO_SSL_DATA_OUT", job);
#endif
        break;

    default:
        wkcDebugPrintfPeer("<rhm>curl log: unknown");
        break;
    }

    if (buff)
        fastFree(buff);
    return 0;
}
#endif

void ResourceHandleManager::updateProxyAuthenticate(ResourceHandleInternal* d)
{
    if (!m_tryProxySingleConnect)
        return;
    m_tryProxySingleConnect = false;

    long avail = CURLAUTH_ANY;
    ProtectionSpaceAuthenticationScheme authScheme = ProtectionSpaceAuthenticationSchemeDefault;

    (void)curl_easy_getinfo(d->m_handle, CURLINFO_PROXYAUTH_AVAIL, &avail);
    if (CURLAUTH_NTLM & avail)
        authScheme = ProtectionSpaceAuthenticationSchemeNTLM;
    else if (CURLAUTH_DIGEST & avail)
        authScheme = ProtectionSpaceAuthenticationSchemeHTTPDigest;
    else if (CURLAUTH_BASIC & avail)
        authScheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;

    if (ProtectionSpaceAuthenticationSchemeDefault != authScheme)
        m_authJar.setProxyUserPassword(m_proxy, m_proxyPort, ProtectionSpaceProxyHTTP, authScheme, d->m_proxyAuthUser, d->m_proxyAuthPass);
}

int ResourceHandleManager::_downloadTimerCallback()
{
    FUNCTIONMOREPRINTF(("<rhm>_downloadTimerCallback()"));

    int runningHandles = 0;
    while (curl_multi_perform(m_curlMultiHandle, &runningHandles) == CURLM_CALL_MULTI_PERFORM) { }

    // check the curl messages indicating completed transfers
    // and free their resources
    while (true) {
        int messagesInQueue;
        CURLMsg* msg = curl_multi_info_read(m_curlMultiHandle, &messagesInQueue);
        if (!msg)
            break;

        // find the node which has same d->m_handle as completed transfer
        CURL* handle = msg->easy_handle;
        ASSERT(handle);
        ResourceHandle* job = 0;
        CURLcode err = curl_easy_getinfo(handle, CURLINFO_PRIVATE, &job);
        ASSERT(CURLE_OK == err);
        if (CURLE_OK != err)
            continue;
        ResourceHandleInternal* d = job->getInternal();
        ASSERT(d->m_handle == handle);

        if (d->m_cancelled || d->m_doRedirectChallenge) {
            removeFromCurl(job);
            continue;
        }

        if (CURLMSG_DONE != msg->msg)
            continue;

        int httpCode = d->m_response.httpStatusCode();
        if (httpCode == 401 || 407 == httpCode) {
            didReceiveAuthenticationChallenge(job, d->m_currentWebChallenge);
            if (d->m_cancelled || d->m_didAuthChallenge) {
                removeFromCurl(job);
                d->m_receivedData->clear();
                continue;
            }
            else {
                if (d->client()) {
                    d->client()->didReceiveResponse(job, d->m_response);
                    d->m_response.setResponseFired(true);
                    const Frame* frame = job->frame();
                    if (!frame || !frame->loader()->documentLoader() || !frame->page() || (!job->mainFrame() && !job->frameloaderclient())) {
                        // may be deleted
                        removeFromCurl(job);
                        d->m_receivedData->clear();
                        continue;
                    } else {
                        if (d->client()) // may be removed at above codes...
                            d->client()->didReceiveData(job, d->m_receivedData->data(), d->m_receivedData->size(), 0);
                    }
                }
                d->m_receivedData->clear();
            }
        }

        if (m_redirectInWKC && d->m_redirectCount > d->m_redirectMaxCount) {
            char* url = 0;
            curl_easy_getinfo(d->m_handle, CURLINFO_EFFECTIVE_URL, &url);
            if (d->client())
                d->client()->didFail(job, ResourceError(String(), CURLE_TOO_MANY_REDIRECTS, String(url), String(curl_easy_strerror(CURLE_TOO_MANY_REDIRECTS)), job));
        }
        else {
            char* url = 0;
            switch (msg->data.result) {
            case CURLE_OK:
                /* Additionally premature termination of HTTP response
                   data is treated like a normal termination, and the
                   data will be shown to the user. */
            case CURLE_PARTIAL_FILE:
                if (!d->m_response.responseFired()) {
                    handleLocalReceiveResponse(d->m_handle, job, d);
                    if (d->m_cancelled || d->m_didAuthChallenge || d->m_doRedirectChallenge) {
                        removeFromCurl(job);
                        continue;
                    }
                }
                if (!d->m_fileLoading && d->m_proxyAuthWellKnownWhenInitializeHandle) {
                    updateProxyAuthenticate(d);
                }
                if (d->client()) {
#if ENABLE(WKC_HTTPCACHE)
                    if (!m_httpCache.disabled()) {
                        KURL url = job->firstRequest().url();
                        switch (d->m_response.httpStatusCode()) {
                        case 200:
                        case 201:
                        case 300:
                        case 301:
                        case 410:
                            addHTTPCache(job, url, d->client()->resourceData(), d->m_response);
                            break;
                        case 304:
                            {
                                if (!d->m_utilizedHTTPCache)
                                    break;  // this case, do nothing since HTTP Cache does not set "If-Modified-Since".
                                if (job->firstRequest().cachePolicy() == ReloadIgnoringCacheData)
                                    break;  // read from the memory cached resource

                                // read from the disk cached resource
                                HTTPCachedResource *resource = m_httpCache.resourceForURL(url);
                                if (resource)
                                    scheduleLoadResourceFromHTTPCache(job);
                                else
                                    d->client()->didFail(job, ResourceError(String(), CURLE_READ_ERROR, String(d->m_url), String("cache read error"), 0));
                                removeFromCurl(job);
                                continue;
                            }
                        default:
                            break;
                        }
                    }
#endif // ENABLE(WKC_HTTPCACHE)
                    d->client()->didFinishLoading(job, currentTime());
                }
                break;
            case CURLE_OPERATION_TIMEDOUT:
                if (d->m_response.responseFired()) {
                    if (!d->m_fileLoading && d->m_proxyAuthWellKnownWhenInitializeHandle) {
                        updateProxyAuthenticate(d);
                    }
                    if (d->client())
                        d->client()->didFinishLoading(job, currentTime());
                    break;
                }
                // !d->m_response.responseFired() means no receive header
                // so, should didFail(), through down!
            default:
                if (d->m_isSSL && !d->m_SSLHandshaked && frameloaderclientwkc(job)) {
                    m_rhmssl->SSLHandshakeInfo(job);
                    if (frameloaderclientwkc(job)->notifySSLHandshakeStatus(job, WKC::EHandshakeFail)) {
                        // true means try to re-connect even though current SSL is failed
                        d->m_sslReconnect = true;
                        break;
                    }
                    d->m_SSLHandshaked = true;
                }
                curl_easy_getinfo(d->m_handle, CURLINFO_EFFECTIVE_URL, &url);
                ERRORPRINTF(("Curl ERROR for url='%s', error: '[%d]%s'", url, msg->data.result, curl_easy_strerror(msg->data.result)));
                if (d->client())
                    d->client()->didFail(job, ResourceError(String(), msg->data.result, String(url), String(curl_easy_strerror(msg->data.result)), job));
            }
        }

        if (d->m_cancelled || !d->m_handle)
            continue;
        {
            int size = m_runningJobList.size();
            bool found = false;
            for (int i = 0; i < size; i++) {
                if (job == m_runningJobList[i]) {
                    found = true;
                    break;
                }
            }
            if (!found)
                continue;
        }

        removeFromCurl(job);
    }

    return runningHandles;
}

void ResourceHandleManager::downloadTimerCallback(Timer<ResourceHandleManager>* timer)
{
    FUNCTIONMOREPRINTF(("<rhm>downloadTimerCallback()"));

    (void)startScheduledJobs();

    int runningHandles = _downloadTimerCallback();

    cancelScheduledJobs();

    bool started = startScheduledJobs(); // new jobs might have been added in the meantime

    if (!m_downloadTimer.isActive() && (started || (runningHandles > 0))) {
        FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by dtc", pollTimeSeconds));
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }

    FUNCTIONMOREPRINTF(("<dtc> EXIT"));
}

static CURLcode cookie_callback(curlcookiedirect direct, CURL *curl, const char *domain, bool tailmatch, void *data)
{
    FUNCTIONPRINTF(("<rhm>cookie_callback(%s, %p, %s, %s, %p)", (direct == CURLCOOKIEDIRECT_RECEIVE)?"recv":"send", curl, domain, (tailmatch)?"True":"False", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();

    bool isIn = (direct == CURLCOOKIEDIRECT_RECEIVE)? true : false;

    if (d->m_cancelled) {
        return CURLE_COOKIE_DENY;
    }

    if (!job->firstRequest().allowCookies()) {
        return CURLE_COOKIE_DENY;
    }

    String firstparty_host = d->m_firstRequest.firstPartyForCookies().host();
    String cookie_domain;
    if (wkcNetCheckCorrectIPAddressPeer(firstparty_host.utf8().data())) {
#if !defined(WKC_DISABLE_CHECK_COOKIE_BY_IP)
        if (firstparty_host != domain) {
            return CURLE_COOKIE_DENY;
        }
#endif
        cookie_domain = domain;
    } else {
        if (domain && strlen(domain) > 0) {
            cookie_domain = String((tailmatch && domain[0] != '.')?".":"") + domain;
        }
    }

    return (CURLcode)ResourceHandleManager::sharedInstance()->cookieCallback(job, isIn, (const char*)d->m_url, firstparty_host, cookie_domain);
}

int ResourceHandleManager::cookieCallback(ResourceHandle* job, bool income, const char* url, String firstparty_host, String cookie_domain)
{
    FUNCTIONPRINTF(("<rhm>cookieCallback(%p, %s, %s, %s, %s)", job, (income)?"recv":"send", url, firstparty_host.utf8().data(), cookie_domain.utf8().data()));

    if (!job) {
    } else if (frameloaderclientwkc(job)) {
        if (frameloaderclientwkc(job)->dispatchWillAcceptCookie(income, job, url, (const char*)firstparty_host.utf8().data(), (const char*)cookie_domain.utf8().data()))
            return (int)CURLE_COOKIE_ACCEPT;
    }

    return (int)CURLE_COOKIE_DENY;
}

int ResourceHandleManager::cookieCallback(const Document* document, bool income, const char* url, String firstparty_host, String cookie_domain)
{
    if (!document || !document->frame() || !document->frame()->page()) {
        return (int)CURLE_COOKIE_DENY;
    }

    Frame* mainFrame = document->frame()->page()->mainFrame();
    if (!mainFrame || !mainFrame->loader()) {
        return (int)CURLE_COOKIE_DENY;
    }

    WebCore::FrameLoaderClient* flc = mainFrame->loader()->client();
    if (flc && flc->byWKC()) {
        WKC::FrameLoaderClientWKC* flcwkc = static_cast<WKC::FrameLoaderClientWKC*>(flc);
        if (flcwkc->dispatchWillAcceptCookie(income, 0, url, (const char*)firstparty_host.utf8().data(), (const char*)cookie_domain.utf8().data()))
            return (int)CURLE_COOKIE_ACCEPT;
    }

    return (int)CURLE_COOKIE_DENY;
}

void ResourceHandleManager::clearCookies()
{
    FUNCTIONPRINTF(("<rhm>clearCookies()"));

    m_cookiesDeleting = true;
    doClearCookies();
}

void ResourceHandleManager::doClearCookies()
{
    FUNCTIONPRINTF(("<rhm>doClearCookies()"));

    if (!m_cookiesDeleting) return;

    if (0 == m_runningJobList.size()) {
        wkcMutexLockPeer(gCookieMutex);

        CURLSHcode shcode = curl_share_setopt(m_curlShareHandle, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_COOKIE);
        if (CURLSHE_OK == shcode) {
            curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
            curl_share_setopt(m_curlShareHandle, CURLSHOPT_COOKIE_MAX_ENTRIES, m_cookieMaxEntries);
            m_cookiesDeleting = false;
        }

        wkcMutexUnlockPeer(gCookieMutex);
    }
}

void ResourceHandleManager::setProxyInfo(const String& host, unsigned long port, ProxyType type, const String& username, const String& password, const String& filters)
{
    m_proxyHost = host;
    m_proxyPort = (port)?port:80;
    m_proxyType = type;

    m_proxyFilters.setProxyFilters(filters);

    /* remove all jobs */
    removeAllScheduledJobs();
    removeAllRunningJobs();

    /* renewal cURL Multi */
    CURLM* curl_multi = curl_multi_init();  // Ugh! Allocate before cleanup to make sure the adress will change.
                                            //      Needed to detect this renewal... (cf. WKC's SocketStreamHandle::platformClose())
    curl_multi_cleanup(m_curlMultiHandle);
    m_curlMultiHandle = curl_multi;
    curl_multi_setopt(m_curlMultiHandle, CURLMOPT_MAXCONNECTS, m_httpConnections);

    if (0 == host.length()) {
        m_proxy = String("");
        m_tryProxySingleConnect = false;
    }
    else {
        m_proxy = String("http://") + host + ":" + String::number(m_proxyPort);
        m_authJar.setProxyUserPassword(m_proxy, m_proxyPort, ProtectionSpaceProxyHTTP, ProtectionSpaceAuthenticationSchemeDefault, username, password);
        m_tryProxySingleConnect = true;
    }
}

void ResourceHandleManager::removeFromCurl(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>removeFromCurl(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    ASSERT(d->m_handle);
    if (!d->m_handle)
        return;

    if (!d->m_cancelled) {
        if (setAuthChallenge(job, true)) {
            curl_multi_add_handle(m_curlMultiHandle, d->m_handle);

            if (!m_downloadTimer.isActive()) {
                FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by removeFromCurl(%p)", pollTimeSeconds, job));
                m_downloadTimer.startOneShot(pollTimeSeconds);
            }
            return;
        }

        if (d->m_sslReconnect) {
            curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
            d->m_permitSend = ResourceHandleInternal::PERMIT;
            d->m_sslReconnect = false;
            curl_easy_setopt(d->m_handle, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(d->m_handle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_multi_add_handle(m_curlMultiHandle, d->m_handle);
            if (!m_downloadTimer.isActive()) {
                FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by removeFromCurl(%p)", pollTimeSeconds, job));
                m_downloadTimer.startOneShot(pollTimeSeconds);
            }
            return;
        }

        if (m_redirectInWKC && doRedirect(job, true)) {
            if (!m_downloadTimer.isActive()) {
                FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by removeFromCurl(%p)", pollTimeSeconds, job));
                m_downloadTimer.startOneShot(pollTimeSeconds);
            }
            return;
        }
    }

    removeRunningJob(job);
    curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
    curl_easy_cleanup(d->m_handle);
    d->m_handle = 0;
    doClearCookies();
    job->deref();
}

bool ResourceHandleManager::setupMethod(ResourceHandle* job, struct curl_slist** headers, int method)
{
    ResourceHandleInternal* d = job->getInternal();

    if (!job->firstRequest().httpBody()) {
        *headers = curl_slist_append(*headers, "Content-Length: 0");
        return false;
    }

    Vector<FormDataElement> elements = job->firstRequest().httpBody()->elements();
    size_t numElements = elements.size();
    if (!numElements) {
        *headers = curl_slist_append(*headers, "Content-Length: 0");
        return false;
    }

    // Do not stream for simple POST/OPTIONS data
    if (HTTPMethod_POST == method || HTTPMethod_OPTIONS == method) {
        if (numElements == 1 && elements[0].m_type != FormDataElement::encodedFile
#if ENABLE(BLOB)
            && elements[0].m_type != FormDataElement::encodedBlob
#endif
        ) {
            job->firstRequest().httpBody()->flatten(d->m_postBytes);
            if (d->m_postBytes.size() != 0) {
                curl_easy_setopt(d->m_handle, CURLOPT_POST, TRUE);
                curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE, d->m_postBytes.size());
                curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDS, d->m_postBytes.data());
            } else {
                *headers = curl_slist_append(*headers, "Content-Length: 0");
            }
            return false;
        }
    }

    // Obtain the total size
    // The size of a curl_off_t could be different in WebKit and in cURL depending on
    // compilation flags of both. For CURLOPT_POSTFIELDSIZE_LARGE/CURLOPT_INFILESIZE_LARGE
    // we have to pass the right size or random data will be used as the size.
    WKC_DEFINE_STATIC_INT(expectedSizeOfCurlOffT, 0);
    if (!expectedSizeOfCurlOffT) {
        curl_version_info_data *infoData = curl_version_info(CURLVERSION_NOW);
        if (infoData->features & CURL_VERSION_LARGEFILE)
            expectedSizeOfCurlOffT = sizeof(long long);
        else
            expectedSizeOfCurlOffT = sizeof(int);
    }

#if COMPILER(MSVC)
    // work around compiler error in Visual Studio 2005.  It can't properly
    // handle math with 64-bit constant declarations.
#pragma warning(disable: 4307)
#endif
    static const long long maxCurlOffT = (1LL << (expectedSizeOfCurlOffT * 8 - 1)) - 1;
    curl_off_t size = 0;
    bool chunkedTransfer = false;
    for (size_t i = 0; i < numElements; i++) {
        FormDataElement element = elements[i];
        if (element.m_type == FormDataElement::encodedFile) {
            long long fileSizeResult;
            if (getFileSize(element.m_filename, fileSizeResult)) {
                if (fileSizeResult > maxCurlOffT) {
                    // File size is too big for specifying it to cURL
                    chunkedTransfer = true;
                    break;
                } else if (fileSizeResult < 0) {
                    chunkedTransfer = true;
                    break;
                }
                size += fileSizeResult;
            }
#if ENABLE(BLOB)
        } else if (element.m_type == FormDataElement::encodedBlob) {
            long long blobSizeResult = 0;
            RefPtr<BlobStorageData> blobData = static_cast<BlobRegistryImpl&>(blobRegistry()).getBlobDataFromURL(KURL(ParsedURLString, element.m_blobURL));
            if (blobData) {
                for (size_t j = 0; j < blobData->items().size(); ++j) {
                    const BlobDataItem& blobItem = blobData->items()[j];
                    if (blobItem.type == BlobDataItem::Data) {
                        blobSizeResult += blobItem.length;
                    }
                }
            }
            size += blobSizeResult;
#endif
        } else
            size += elements[i].m_data.size();
    }

    // cURL guesses that we want chunked encoding as long as we specify the header
    if (chunkedTransfer) {
        *headers = curl_slist_append(*headers, "Transfer-Encoding: chunked");
    }
    else {
        CURLoption option = CURLOPT_LASTENTRY;
        if (HTTPMethod_POST == method || HTTPMethod_OPTIONS == method) {
            if (sizeof(long long) == expectedSizeOfCurlOffT)
                option = CURLOPT_POSTFIELDSIZE_LARGE;
            else
                option = CURLOPT_POSTFIELDSIZE;
        }
        else if (HTTPMethod_PUT == method) {
            if (sizeof(long long) == expectedSizeOfCurlOffT)
                option = CURLOPT_INFILESIZE_LARGE;
            else
                option = CURLOPT_INFILESIZE;
        }
        if (sizeof(long long) == expectedSizeOfCurlOffT)
          curl_easy_setopt(d->m_handle, option, (long long)size);
        else
          curl_easy_setopt(d->m_handle, option, (int)size);
    }

    curl_easy_setopt(d->m_handle, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_READDATA, job);

    return true;
}

void ResourceHandleManager::setupPUT(ResourceHandle* job, struct curl_slist** headers)
{
    FUNCTIONPRINTF(("<rhm>setupPUT()"));

    ResourceHandleInternal* d = job->getInternal();
    curl_easy_setopt(d->m_handle, CURLOPT_UPLOAD, TRUE);
    curl_easy_setopt(d->m_handle, CURLOPT_INFILESIZE, 0);

    (void)setupMethod(job, headers, HTTPMethod_PUT);
}

/* Calculate the length of the POST.
   Force chunked data transfer if size of files can't be obtained.
 */
void ResourceHandleManager::setupPOST(ResourceHandle* job, struct curl_slist** headers)
{
    FUNCTIONPRINTF(("<rhm>setupPOST()"));

    ResourceHandleInternal* d = job->getInternal();
    curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE, 0);

    if (!setupMethod(job, headers, HTTPMethod_POST)) {
        if (d->m_postBytes.size() == 0)
            curl_easy_setopt(d->m_handle, CURLOPT_CUSTOMREQUEST, "POST");
        return;
    }

    curl_easy_setopt(d->m_handle, CURLOPT_POST, TRUE);
    curl_easy_setopt(d->m_handle, CURLOPT_IOCTLFUNCTION, ioctlCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_IOCTLDATA, job);
}

/* Calculate the length of the OPTIONS.
   Force chunked data transfer if size of files can't be obtained.
 */
void ResourceHandleManager::setupOPTIONS(ResourceHandle* job, struct curl_slist** headers)
{
    FUNCTIONPRINTF(("<rhm>setupOPTIONS()"));

    ResourceHandleInternal* d = job->getInternal();

    /* Specify OPTIONS method */
    curl_easy_setopt(d->m_handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    /* The code below is the same as setupPOST(), except comments */
    curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE, 0);

    if (!setupMethod(job, headers, HTTPMethod_OPTIONS))
        return;

    curl_easy_setopt(d->m_handle, CURLOPT_POST, TRUE);
    curl_easy_setopt(d->m_handle, CURLOPT_IOCTLFUNCTION, ioctlCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_IOCTLDATA, job);
}

void ResourceHandleManager::add(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>add(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    KURL kurl = job->firstRequest().url();

    d->m_matchProxyFilter = m_proxyFilters.isMatchProxyFilter(kurl.host());

#if ENABLE(WKC_HTTPCACHE)
    if (!m_httpCache.disabled() && job->firstRequest().cachePolicy() != ReloadIgnoringCacheData) {
        HTTPCachedResource *resource = m_httpCache.resourceForURL(kurl);
        if (resource) {
            const String& lastModified = resource->lastModifiedHeader();
            const String& etag = resource->eTagHeader();
            if (lastModified.isEmpty() && etag.isEmpty()) {
                if (!resource->needRevalidate()) {
                    // read from the cached resource
                    scheduleLoadResourceFromHTTPCache(job);
                    return;
                }
            } else {
                // get from a network unless the cached resource is up-to-date.
                if (!lastModified.isEmpty())
                    job->firstRequest().setHTTPHeaderField("If-Modified-Since", lastModified);
                if (!etag.isEmpty())
                    job->firstRequest().setHTTPHeaderField("If-None-Match", etag);
                job->getInternal()->m_utilizedHTTPCache = true;
            }
        }
    }
#endif // ENABLE(WKC_HTTPCACHE)

    // we can be called from within curl, so to avoid re-entrancy issues
    // schedule this job to be added the next time we enter curl download loop
    job->ref();
    appendScheduledJob(job);

    if (!m_downloadTimer.isActive()) {
        FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by add(%p)", pollTimeSeconds, job));
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }
}

void ResourceHandleManager::didReceiveAuthenticationChallenge(ResourceHandle* job, const AuthenticationChallenge& challenge)
{
    FUNCTIONPRINTF(("<rhm>didReceiveAuthenticationChallenge(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    Vector<String> list;
    String realm = "";
    bool isProxyAuth = (401 == d->m_response.httpStatusCode()) ? false : true;
    bool doUserRequest = false;
    long avail = CURLAUTH_BASIC;
    String host;
    int port;
    ProtectionSpaceAuthenticationScheme requestedAuthScheme;

    if (!isProxyAuth && !job->firstRequest().allowCookies()) {
        d->m_didAuthChallenge = false;
        return;
    }

    (void)curl_easy_getinfo(d->m_handle, (isProxyAuth ? CURLINFO_PROXYAUTH_AVAIL : CURLINFO_HTTPAUTH_AVAIL), &avail);
    if (CURLAUTH_NTLM & avail) {
        requestedAuthScheme = ProtectionSpaceAuthenticationSchemeNTLM;

        String value = d->m_response.httpHeaderField((isProxyAuth) ? "Proxy-Authenticate" : "WWW-Authenticate");
        if (value.lower().startsWith("ntlm")) {
            if (4 < value.length()) {
                // this case means receiving NTLM Step 2,
                // do not call didReceiveAuthenticationChallenge and do nothing if cURL is over 7.27.0.
                return;
            }
            else {
                // this case means receiving NTLM step 0.  delete setHTTPHeaderField.
                d->m_response.setHTTPHeaderField((isProxyAuth)?"Proxy-Authenticate":"WWW-Authenticate", "");
            }
        }

        // IIS/6.0 special case
        value = d->m_response.httpHeaderField("Server");
        if ((value.find("IIS/6.0") != notFound))
            if (d->m_firstRequest.url().path().endsWith(".ico"))
                return;
    }
    else if (CURLAUTH_DIGEST & avail) {
        requestedAuthScheme = ProtectionSpaceAuthenticationSchemeHTTPDigest;
    }
    else if (CURLAUTH_BASIC & avail) {
        requestedAuthScheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;
    }
    else {
        // not support;
        return;
    }

    ProtectionSpaceServerType serverType = ProtectionSpaceServerHTTP;
    ProtectionSpaceAuthenticationScheme authScheme = ProtectionSpaceAuthenticationSchemeDefault;
    String user;
    String passwd;
    if (isProxyAuth) {
        host = m_proxy;
        port = m_proxyPort;
        realm = d->m_proxyAuthRealm;
        serverType = ProtectionSpaceProxyHTTP;

        if (!d->m_proxyAuthURL.isEmpty()) {
            // user entered but fail
            m_authJar.deleteProxyUserPassword(host, port);
            d->m_proxyAuthURL = "";
            doUserRequest = true;
        }
        else if (d->m_proxyAuthCallengeTimes) {
            doUserRequest = true;
        }
        else {
            if (m_authJar.getProxyUserPassword(host, port, serverType, authScheme, user, passwd)) {
                if (ProtectionSpaceAuthenticationSchemeDefault == authScheme || user.isEmpty() || passwd.isEmpty())
                    doUserRequest = true;
                else
                    doUserRequest = false;
            }
            else {
                doUserRequest = true;
            }
        }
    }
    else {
        serverType = ProtectionSpaceServerHTTP;
        KURL kurl = d->m_firstRequest.url();
        kurl.removeFragmentIdentifier();
        kurl.setQuery("");
        port = kurl.port();
        port = (0 < port) ? port : 80;
        kurl.removePort();
        host = kurl.string();
        realm = d->m_webAuthRealm;

        if (!d->m_webAuthURL.isEmpty()) {
            // user entered but fail
            m_authJar.deleteWebUserPassword(host, realm);
            doUserRequest = true;
        }
        else if (d->m_webAuthCallengeTimes) {
            doUserRequest = true;
        }
        else {
            if (m_authJar.getWebUserPassword(host, serverType, authScheme, realm, user, passwd)) {
                doUserRequest = false;
            }
            else {
                doUserRequest = true;
            }
        }
    }

    d->m_currentWebChallenge = AuthenticationChallenge(
        ProtectionSpace(host, port, serverType, realm, requestedAuthScheme),
        challenge.proposedCredential(),
        0,
        d->m_response,
        challenge.error(),
        job);

    if (doUserRequest) {
        job->didReceiveAuthenticationChallenge(d->m_currentWebChallenge);
    }
    else {
        if (ProtectionSpaceAuthenticationSchemeHTTPDigest == requestedAuthScheme) {
            if ((isProxyAuth && d->m_proxyAuthWellKnownWhenInitializeHandle) || (!isProxyAuth && d->m_webAuthWellKnownWhenInitializeHandle)) {
                d->m_didAuthChallenge = false;
            }
            else {
                d->m_didAuthChallenge = true;
            }
        }
        else {
            d->m_didAuthChallenge = true;
        }
    }

    if (isProxyAuth)
        d->m_proxyAuthCallengeTimes++;
    else
        d->m_webAuthCallengeTimes++;
}

void ResourceHandleManager::didAuthChallenge(ResourceHandle* job, bool isProxy)
{
    FUNCTIONPRINTF(("<rhm>didAuthChallenge(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    if (!d) return;

    if (d->m_currentWebChallenge.isNull()) {
        // not web authentication
        return;
    }

    if (isProxy) {
        m_authJar.setProxyUserPassword(
            d->m_currentWebChallenge.protectionSpace().host(),
            d->m_currentWebChallenge.protectionSpace().port(),
            ProtectionSpaceProxyHTTP,
            d->m_currentWebChallenge.protectionSpace().authenticationScheme(),
            d->m_currentWebChallenge.proposedCredential().user(),
            d->m_currentWebChallenge.proposedCredential().password());
        d->m_proxyAuthURL    = d->m_currentWebChallenge.protectionSpace().host();
        d->m_proxyAuthScheme = d->m_currentWebChallenge.protectionSpace().authenticationScheme();
        d->m_proxyAuthUser   = d->m_currentWebChallenge.proposedCredential().user();
        d->m_proxyAuthPass   = d->m_currentWebChallenge.proposedCredential().password();
    }
    else {
        d->m_webAuthURL    = d->m_currentWebChallenge.protectionSpace().host();
        d->m_webAuthScheme = d->m_currentWebChallenge.protectionSpace().authenticationScheme();
        d->m_webAuthRealm  = d->m_currentWebChallenge.protectionSpace().realm();
        d->m_webAuthUser   = d->m_currentWebChallenge.proposedCredential().user();
        d->m_webAuthPass   = d->m_currentWebChallenge.proposedCredential().password();

        m_authJar.setWebUserPassword(d->m_webAuthURL, ProtectionSpaceServerHTTP, d->m_webAuthScheme,d->m_webAuthRealm, d->m_webAuthUser, d->m_webAuthPass);
    }

    d->m_didAuthChallenge = true;
}

void ResourceHandleManager::cancelAuthChallenge(ResourceHandle* job, bool isProxy)
{
    FUNCTIONPRINTF(("<rhm>cancelAuthChallenge(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    if (!d) return;

    if (isProxy) {
        d->m_proxyAuthURL    = "";
        d->m_proxyAuthScheme = ProtectionSpaceAuthenticationSchemeUnknown;
        d->m_proxyAuthUser   = "";
        d->m_proxyAuthPass   = "";
    }
    else {
        d->m_webAuthURL    = "";
        d->m_webAuthScheme = ProtectionSpaceAuthenticationSchemeUnknown;
        d->m_webAuthRealm  = "";
        d->m_webAuthUser   = "";
        d->m_webAuthPass   = "";
    }

    d->m_didAuthChallenge = false;
}

bool ResourceHandleManager::startScheduledJobs()
{
    FUNCTIONPRINTF(("<rhm>startScheduledJobs()"));

    ResourceHandle* job;

    int size = m_scheduledJobList.size();
    if (0 == size)
        return false;

    bool started = false;
    while (0 == m_httpConnections || m_runningJobList.size() < m_httpConnections*2) {
        if (m_tryProxySingleConnect && m_runningJobList.size() > 0) {
            break;
        }
        job = shiftScheduledJob();
        if (!job)
            return true;

        ResourceHandleInternal* d = job->getInternal();
        if (!d)
            continue;
        if (d && d->m_cancelled){
            if (d->m_permitSend == ResourceHandleInternal::CANCEL)
                if (d->client())
                    d->client()->didFail(job, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("prohibit"), 0));
            d->m_handle = 0;
            job->deref();
            continue;
        }

        startJob(job);

        started = true;
    }
    return started;
}

void ResourceHandleManager::cancelScheduledJobs()
{
    FUNCTIONPRINTF(("<rhm>cancelScheduledJobs()"));

    ResourceHandle* job;
    ResourceHandleInternal* d;
    bool loop = true;
    int size;

    while (loop) {
        loop = false;
        size = m_runningJobList.size();
        for (int i = 0; i < size; i++) {
            job = m_runningJobList[i];
            d = job->getInternal();
            if (d && d->m_cancelled) {
                FUNCTIONMOREPRINTF(("<rhm>cancelScheduledJobs(): %s", d->m_url));
                m_runningJobList.remove(i);

                curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
                curl_easy_cleanup(d->m_handle);
                d->m_handle = 0;
                job->deref();
                loop = true;
                break;
            }
        }
    }
    doClearCookies();
}

void ResourceHandleManager::dispatchSynchronousJob(ResourceHandle* rjob)
{
    FUNCTIONPRINTF(("<rhm>dispatchSynchronousJob(%p)", rjob));

    KURL kurl = rjob->firstRequest().url();

    if (kurl.protocolIsData()) {
        handleDataURL(rjob);
        return;
    }

    ResourceHandleInternal* d = rjob->getInternal();
    bool failed = false;

#if LIBCURL_VERSION_NUM > 0x071200
    // If defersLoading is true and we call curl_easy_perform
    // on a paused handle, libcURL would do the transfert anyway
    // and we would assert so force defersLoading to be false.
    d->m_defersLoading = false;
#endif

    d->m_url = fastStrdup(kurl.string().latin1().data());
    d->m_composition = contentComposition(rjob);

    if (m_harmfulSiteFilter) {
        if (!d->m_cancelled && frameloaderclientwkc(rjob)) {
            int ret = frameloaderclientwkc(rjob)->dispatchWillPermitSendRequest(rjob, d->m_url, d->m_composition, true, d->m_response);
            if (1 != ret) {
                d->m_permitSend = ResourceHandleInternal::CANCEL;
                d->m_cancelled = true;
                if (d->client())
                    d->client()->didFail(rjob, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("prohibit"), 0));
                return;
            }
        }
        else {
            d->m_permitSend = ResourceHandleInternal::CANCEL;
            d->m_cancelled = true;
            if (d->client())
                d->client()->didFail(rjob, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("prohibit"), 0));
            return;
        }
    }

    initializeHandle(rjob);
    if (!d->m_handle) {
        ERRORPRINTF(("<rhm>dispatchSynchronousJob(): No Memory or something error"));
        if (d->client())
            d->client()->didFail(rjob, ResourceError(String(), CURLE_OUT_OF_MEMORY, String(d->m_url), String("no memory"), 0));
        return;
    }
    d->m_isSynchronous = true;

    LOADINGPRINTF(("SyncStart Load [%p] %s", rjob, d->m_url));

    curl_multi_add_handle(m_curlMultiSyncHandle, d->m_handle);

    bool isMainThread = (wkcThreadCurrentThreadPeer() == m_mainThreadId) ? true : false;
    bool retry;
    do {
        retry = false;

        int runningHandles = 0;
        while (curl_multi_perform(m_curlMultiSyncHandle, &runningHandles) == CURLM_CALL_MULTI_PERFORM) { }

        int messagesInQueue;
        CURLMsg* msg = curl_multi_info_read(m_curlMultiSyncHandle, &messagesInQueue);
        if (!msg) {
            retry = true;
            continue;
        }

        // find the node which has same d->m_handle as completed transfer
        CURL* handle = msg->easy_handle;
        ASSERT(handle);
        ResourceHandle* job = 0;
        (void)curl_easy_getinfo(handle, CURLINFO_PRIVATE, &job);
        ASSERT(job);
        ResourceHandleInternal* d = job->getInternal();

        CURLcode ret = msg->data.result;

        curl_multi_remove_handle(m_curlMultiSyncHandle, d->m_handle);

        long httpCode = 0;
        (void)curl_easy_getinfo(d->m_handle, CURLINFO_RESPONSE_CODE, &httpCode);

        // HTTP redirection
        if (300 <= httpCode && httpCode < 400) {
            if (m_redirectInWKC && d->m_doRedirectChallenge) {
                d->m_doRedirectChallenge = false;

                if (m_harmfulSiteFilter) {
                    if (!d->m_cancelled && frameloaderclientwkc(job)) {
                        kurl = job->firstRequest().url();
                        if (d->m_url) {
                            fastFree(d->m_url);
                            d->m_url = 0;
                        }
                        d->m_url = fastStrdup(kurl.string().latin1().data());
                        initializeHandle(job);
                        int ret = frameloaderclientwkc(job)->dispatchWillPermitSendRequest(job, d->m_url, d->m_composition, true, d->m_response);
                        if (1 != ret) {
                            d->m_permitSend = ResourceHandleInternal::CANCEL;
                            d->m_cancelled = true;
                            if (d->client())
                                d->client()->didFail(job, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("prohibit"), 0));
                            failed = true;
                            break;
                        }
                    }
                    else {
                        d->m_permitSend = ResourceHandleInternal::CANCEL;
                        d->m_cancelled = true;
                        if (d->client())
                            d->client()->didFail(job, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("prohibit"), 0));
                        failed = true;
                        break;
                    }
                }
                curl_multi_add_handle(m_curlMultiSyncHandle, d->m_handle);
                retry = true;
                continue;
            }
        }

        if ((401 == httpCode || 407 == httpCode) && d->m_response.isHTTP()) {
            headerRealm(d, httpCode);
            didReceiveAuthenticationChallenge(job, d->m_currentWebChallenge);
            if (!d->m_cancelled) {
                retry = setAuthChallenge(job, false);
                if (retry) {
                    curl_multi_add_handle(m_curlMultiSyncHandle, d->m_handle);
                }
            }
            if (401 == httpCode) {
                if (d->client()) {
                    d->client()->didReceiveResponse(job, d->m_response);
                    d->m_response.setResponseFired(true);
                    d->client()->didReceiveData(job, d->m_receivedData->data(), d->m_receivedData->size(), 0);
                }
            }
        }

        if (300 > httpCode && d->m_response.isHTTP()) {
            if (d->client()) {
                d->client()->didReceiveResponse(job, d->m_response);
                d->m_response.setResponseFired(true);
                d->client()->didReceiveData(job, d->m_receivedData->data(), d->m_receivedData->size(), 0);
            }
        }

        if (!retry && ret != 0) {
            char* url = 0;
            if (d->m_isSSL && !d->m_SSLHandshaked && frameloaderclientwkc(job)) {
                m_rhmssl->SSLHandshakeInfo(job);
                frameloaderclientwkc(job)->notifySSLHandshakeStatus(job, WKC::EHandshakeFail);
                d->m_SSLHandshaked = true;
            }
            curl_easy_getinfo(d->m_handle, CURLINFO_EFFECTIVE_URL, &url);
            ERRORPRINTF(("Curl sync ERROR for url='%s', error: '[%d]%s'", url, ret, curl_easy_strerror(ret)));
            if (d->client())
                d->client()->didFail(job, ResourceError(String(), ret, String(url), String(curl_easy_strerror(ret)), 0));
            failed = true;
        }
    } while (retry);

    LOADINGPRINTF(("SyncFinish Load[%p]", rjob));

    if (!failed && d->client())
        d->client()->didReceiveResponse(rjob, d->m_response);

    curl_easy_cleanup(d->m_handle);

    if (isMainThread) {
        curl_multi_refresh_timer(m_curlMultiHandle);
    }
}

void ResourceHandleManager::startJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>startJob(%p)", job));

    KURL kurl = job->firstRequest().url();

    if (kurl.protocolIsData()) {
        handleDataURL(job);
        job->cancel();
        job->deref();
        return;
    }

    ResourceHandleInternal* d = job->getInternal();

    initializeHandle(job);
    if (!d->m_handle) {
        ERRORPRINTF(("<rhm>startJob(): No Memory or Something error"));
        if (d->client())
            d->client()->didFail(job, ResourceError(String(), CURLE_OUT_OF_MEMORY, String(d->m_url), String("no memory"), 0));
        return;
    }

    CURLMcode ret = curl_multi_add_handle(m_curlMultiHandle, d->m_handle);
    // don't call perform, because events must be async
    // timeout will occur and do curl_multi_perform
    if (ret && ret != CURLM_CALL_MULTI_PERFORM) {
        ERRORPRINTF(("Error %d starting job %s\n", ret, encodeWithURLEscapeSequences(job->firstRequest().url().string()).latin1().data()));
        job->cancel();
        if (d->client()) {
            d->client()->didFail(job, ResourceError(String(), CURLE_OUT_OF_MEMORY, String(d->m_url), String("no memory"), 0));
        }
        curl_easy_cleanup(d->m_handle);
        d->m_handle = 0;
        job->deref();
        return;
    }

    if (d->m_isSSL && frameloaderclientwkc(job)) {
        frameloaderclientwkc(job)->notifySSLHandshakeStatus(job, WKC::EHandshakeInitialize);
    }

    appendRunningJob(job);
}

void ResourceHandleManager::initializeHandle(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>initializeHandle(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    // at first, malloc memory
    if (0 == d->m_redirectCount) {
        d->m_handle = curl_easy_init();
        if (!d->m_handle) {
            return;
        }
    }
    d->m_cancelled = false;

    KURL kurl = job->firstRequest().url();
    String url = kurl.string();

    // Remove any fragment part, otherwise curl will send it as part of the request.
    kurl.removeFragmentIdentifier();

    if (kurl.isLocalFile()) {
        String query = kurl.query();
        // Remove any query part sent to a local file.
        if (!query.isEmpty()) {
            size_t queryIndex = url.find(query);
            if (queryIndex != notFound)
                url = url.left(queryIndex - 1);
        }
        // Determine the MIME type based on the path.
        d->m_response.setMimeType(MIMETypeRegistry::getMIMETypeForPath(kurl.path()));
        d->m_fileLoading = true;
    }
    if (kurl.protocolIsData()) {
        d->m_dataLoading = true;
    }

#if LIBCURL_VERSION_NUM > 0x071200
    if (d->m_defersLoading) {
        CURLcode error = curl_easy_pause(d->m_handle, CURLPAUSE_ALL);
        // If we did not pause the handle, we would ASSERT in the
        // header callback. So just assert here.
        ASSERT(error == CURLE_OK);
        if (error != CURLE_OK) {
            curl_easy_cleanup(d->m_handle);
            d->m_handle = NULL;
            return;
        }
    }
#endif

#ifdef CURL_DEBUG_CALLBACK
    curl_easy_setopt(d->m_handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(d->m_handle, CURLOPT_DEBUGFUNCTION, debugCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_DEBUGDATA, job);
#endif

    curl_easy_setopt(d->m_handle, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

    curl_easy_setopt(d->m_handle, CURLOPT_PRIVATE, job);
    curl_easy_setopt(d->m_handle, CURLOPT_ERRORBUFFER, m_curlErrorBuffer);
    curl_easy_setopt(d->m_handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_WRITEDATA, job);
    curl_easy_setopt(d->m_handle, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_WRITEHEADER, job);
    if (d->m_firstRequest.sendAutoHTTPReferfer())
        curl_easy_setopt(d->m_handle, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(d->m_handle, CURLOPT_SHARE, m_curlShareHandle);
    curl_easy_setopt(d->m_handle, CURLOPT_DNS_CACHE_TIMEOUT, m_DNSCacheTimeout);
    // cookie
    curl_easy_setopt(d->m_handle, CURLOPT_COOKIEFUNCTIONDATA, job);
    curl_easy_setopt(d->m_handle, CURLOPT_COOKIEFUNCTION, cookie_callback);
    // redirect
    if (!m_redirectInWKC) {
        curl_easy_setopt(d->m_handle, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(d->m_handle, CURLOPT_MAXREDIRS, 10);
    }
    // only one connection for each handle
    curl_easy_setopt(d->m_handle, CURLOPT_MAXCONNECTS, 1);

    // Set connect timeout.
    curl_easy_setopt(d->m_handle, CURLOPT_SERVER_RESPONSE_TIMEOUT, m_serverResponseTimeout);
    curl_easy_setopt(d->m_handle, CURLOPT_CONNECTTIMEOUT, m_connectTimeout);

    if (equalIgnoringCase(kurl.protocol(), "https")) {
        d->m_isSSL = true;
        m_rhmssl->initializeHandleSSL(job);
    }

    // set accept-encoding:
    if (m_acceptEncoding.length())
        curl_easy_setopt(d->m_handle, CURLOPT_ENCODING, m_acceptEncoding.utf8().data());
    else
        curl_easy_setopt(d->m_handle, CURLOPT_ENCODING, NULL); // don't send accept-encoding

    // url must remain valid through the request
    //ASSERT(!d->m_url);

    // url is in ASCII so latin1() will only convert it to char* without character translation.
    // ~ResourceHandleInternal() will fastFree d->m_url.
    d->m_urlhost = fastStrdup(hostAndPort(kurl).latin1().data());
    curl_easy_setopt(d->m_handle, CURLOPT_URL, d->m_url);

    struct curl_slist* headers = 0;

    if (job->firstRequest().httpHeaderFields().size() > 0) {
        HTTPHeaderMap customHeaders = job->firstRequest().httpHeaderFields();
        HTTPHeaderMap::const_iterator end = customHeaders.end();
        for (HTTPHeaderMap::const_iterator it = customHeaders.begin(); it != end; ++it) {
            String key = it->first;
            String value = it->second;
            if (key == "Accept-Encoding") {
                curl_easy_setopt(d->m_handle, CURLOPT_ACCEPT_ENCODING, value.latin1().data());
            }
            else {
                String headerString(key);
                if (value.length()) {
                    headerString.append(": ");
                    headerString.append(value);
                } else {
                    headerString.append(";");
                }
                if (equalIgnoringCase(key, "Last-Event-ID")) {
                    CString headerUtf8 = headerString.utf8();
                    headers = curl_slist_append(headers, headerUtf8.data());
                } else {
                    CString headerLatin1 = headerString.latin1();
                    headers = curl_slist_append(headers, headerLatin1.data());
                }
            }
        }
    }

    if ("GET" == job->firstRequest().httpMethod())
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPGET, TRUE);
    else if ("POST" == job->firstRequest().httpMethod())
        setupPOST(job, &headers);
    else if ("PUT" == job->firstRequest().httpMethod())
        setupPUT(job, &headers);
    else if ("HEAD" == job->firstRequest().httpMethod())
        curl_easy_setopt(d->m_handle, CURLOPT_NOBODY, TRUE);
    else {
        setupPUT(job, &headers);
        curl_easy_setopt(d->m_handle, CURLOPT_CUSTOMREQUEST, job->firstRequest().httpMethod().utf8().data());
    }

    // Do Not Track
    if (m_DoNotTrack) {
        headers = curl_slist_append(headers, "DNT: 1");
    }

    if (headers) {
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPHEADER, headers);
        d->m_customHeaders = headers;
    }

    curl_easy_setopt(d->m_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    // if file/data scheme loading, do not set proxy/authenticate
    if (d->m_fileLoading || d->m_dataLoading) {
        d->m_response.setResourceHandle(job);
        return;
    }

    // Proxy
    if (m_proxy.length() && !d->m_matchProxyFilter) {
        if (m_proxy.length()) {
            curl_easy_setopt(d->m_handle, CURLOPT_PROXY, m_proxy.utf8().data());

            ProtectionSpaceServerType servertype;
            ProtectionSpaceAuthenticationScheme authscheme;
            String user;
            String pass;
            if (m_authJar.getProxyUserPassword(m_proxy, m_proxyPort, servertype, authscheme, user, pass)) {
                if (!user.isEmpty()) {
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYUSERNAME, user.utf8().data());
                    d->m_proxyAuthUser = user;
                } else {
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYUSERNAME, NULL);
                    d->m_proxyAuthUser = emptyString();
                }
                if(!pass.isEmpty()) {
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYPASSWORD, pass.utf8().data());
                    d->m_proxyAuthPass = pass;
                } else {
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYPASSWORD, NULL);
                    d->m_proxyAuthPass = emptyString();
                }
                curl_easy_setopt(d->m_handle, CURLOPT_PROXYAUTH, ProtectionSpaceAuthSchemeTocURLType(authscheme));
                d->m_proxyAuthURL  = m_proxy;
                d->m_proxyAuthWellKnownWhenInitializeHandle = true;
            }
            if ((int)CURL_HTTP_VERSION_1_0 == m_proxyType) {
                curl_easy_setopt(d->m_handle, CURLOPT_PROXYTYPE, CURL_HTTP_VERSION_1_0);  // cURL uses only for proxy CONNECT.
                curl_easy_setopt(d->m_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            }
        }
    }

    // HTTP Authenticate
    curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    if (d->m_webAuthUser.length() || d->m_webAuthPass.length()) {
        curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, d->m_webAuthUser.utf8().data());
        curl_easy_setopt(d->m_handle, CURLOPT_PASSWORD, d->m_webAuthPass.utf8().data());
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST);
        d->m_webAuthURL    = d->m_url;
        d->m_webAuthScheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;
        d->m_webAuthRealm  = "";
    }
    else if (job->firstRequest().allowCookies()) {
        ProtectionSpaceServerType servertype;
        ProtectionSpaceAuthenticationScheme authscheme;
        String user;
        String pass;
        if (m_authJar.getWebUserPassword(url, servertype, authscheme, "", user, pass)) {
            curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, user.utf8().data());
            curl_easy_setopt(d->m_handle, CURLOPT_PASSWORD, pass.utf8().data());
            curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, ProtectionSpaceAuthSchemeTocURLType(authscheme));
            d->m_webAuthWellKnownWhenInitializeHandle = true;
        }
    }

    d->m_response.setResourceHandle(job);
}

void ResourceHandleManager::cancel(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>cancel(%p)", job));

    if (removeScheduledJob(job))
        return;

    ResourceHandleInternal* d = job->getInternal();
    if (!d)
        return;

    d->m_cancelled = true;
    if (!d->m_handle)
        return;

    if (!m_downloadTimer.isActive()) {
        FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by cancel(%p)", pollTimeSeconds, job));
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }
}

/*
 *  Cookie
 */
void ResourceHandleManager::setCookie(const String &domain, const String &path, const String &cookie)
{
    wkcMutexLockPeer(gCookieMutex);

    curl_share_set_cookie(m_curlShareHandle, domain.utf8().data(), path.utf8().data(), cookie.utf8().data(), COOKIETYPE_HEADERLINE);

    wkcMutexUnlockPeer(gCookieMutex);
}

String ResourceHandleManager::getCookie(const String &domain, const String &path, bool secure, bool includehttponly)
{
    wkcMutexLockPeer(gCookieMutex);
    int len = 0;
    char* buf = 0;

    len = curl_share_get_cookie(m_curlShareHandle, domain.utf8().data(), path.utf8().data(), secure ? TRUE : FALSE, includehttponly ? TRUE : FALSE, NULL, 0);
    if (len>0) {
        buf = (char *)fastMalloc(len+1);
        if (buf) {
            buf[0] = 0x0;
            len = curl_share_get_cookie(m_curlShareHandle, domain.utf8().data(), path.utf8().data(), secure ? TRUE : FALSE, includehttponly ? TRUE : FALSE, buf, len+1);
        }
    }

    wkcMutexUnlockPeer(gCookieMutex);

    if (buf) {
        String result = String::fromUTF8(buf);
        fastFree(buf);
        return result;
    } else {
        return String();
    }
}

int ResourceHandleManager::CookieSerializeNum()
{
    int ret;

    wkcMutexLockPeer(gCookieMutex);
    ret =  curl_share_cookie_num(m_curlShareHandle);
    wkcMutexUnlockPeer(gCookieMutex);

    return ret;
}

int ResourceHandleManager::CookieSerialize(char* buff, int bufflen)
{
    FUNCTIONPRINTF(("<rhm>CookieSerialize(%p, %d)", buff, bufflen));

    wkcMutexLockPeer(gCookieMutex);

    CURLSHcode ret;
    struct curl_slist** cookie_array = 0;
    struct curl_slist* cookies;
    struct curl_slist* current;
    int total_len = 0;
    int remaining_bufflen = bufflen;
    int pos = 0;
    int num = 0;

    ret = curl_share_cookie_list(m_curlShareHandle, &cookies);
    if (ret != CURLSHE_OK) {
        wkcMutexUnlockPeer(gCookieMutex);
        return -1;
    }

    if (!buff) {
        current = cookies;
        while (current) {
            total_len += strlen(current->data) + 2;
            if (!current->next) {
                total_len++;
            }
            current = current->next;
        }
        goto CookieSerializeEnd;
    }

    ASSERT(0 <= remaining_bufflen);

    num = curl_share_cookie_num(m_curlShareHandle);
    if (num <= 0)
        goto CookieSerializeEnd;

    cookie_array = (struct curl_slist**)fastMalloc((num + 1) * sizeof(struct curl_slist*));
    if (!cookie_array)
        goto CookieSerializeEnd;

    current = cookies;
    for (int i = 0; i < num; i++) {
        cookie_array[i] = current;
        current = current->next;
    }
    cookie_array[num] = 0;

    current = cookies;
    for (int i = num - 1; i >= 0; i--) {
        total_len += strlen(cookie_array[i]->data);
        total_len += 2; // 2: length of '\r' and '\n'.
        if (bufflen < total_len + 1) { // 1: length of a null termination character.
            // Discard older cookies if the size of cookies exceeds the buffer size.
            current = cookie_array[i+1];
            break;
        }
    }

    while (current) {
        int datalen = strlen(current->data);
        memcpy(buff + pos, current->data, strlen(current->data));
        pos += strlen(current->data);
        buff[pos] = '\r'; pos++;
        buff[pos] = '\n'; pos++;
        remaining_bufflen -= (datalen + 2);
        if (!current->next) {
            buff[pos] = 0x0;
            remaining_bufflen--;
        }
        current = current->next;
    }
	ASSERT(0 <= remaining_bufflen);
    total_len = bufflen - remaining_bufflen;

CookieSerializeEnd:
    if (cookie_array)
        fastFree(cookie_array);
    curl_slist_free_all(cookies);
    wkcMutexUnlockPeer(gCookieMutex);
    return total_len;
}

void ResourceHandleManager::CookieDeserialize(const char* buff, bool restart)
{
    FUNCTIONPRINTF(("<rhm>CookieDeserialize(%p, %s)", buff, (restart)?"restart":"initial"));

    wkcMutexLockPeer(gCookieMutex);

    while (*buff == '\r' || *buff == '\n')
        buff++;

    size_t buffLen;
    while (0 < (buffLen = strlen(buff))) {
        size_t span = strcspn(buff, "\x0D\x0A");
        if (0 == span)
            span = buffLen;
        char* cookie = (char*)fastMalloc(span + 1);
        if (!cookie)
            goto CookieDeserializeEnd;
        memcpy(cookie, buff, span);
        cookie[span] = 0x0;

        curl_share_set_cookie(m_curlShareHandle, NULL, NULL, cookie, (restart)?COOKIETYPE_COOKIELIST_ALL:COOKIETYPE_COOKIELIST);

        fastFree(cookie);

        buffLen -= span;
        buff += span;

        while (*buff == '\r' || *buff == '\n')
            buff++;
    }

CookieDeserializeEnd:
    wkcMutexUnlockPeer(gCookieMutex);
}

bool ResourceHandleManager::getRawCookies(const String &domain, const String &path, bool secure, Vector<Cookie>& rawCookies)
{
    rawCookies.clear();

    wkcMutexLockPeer(gCookieMutex);

    struct CookieJarIterator* it = curl_share_create_cookies_iterator(m_curlShareHandle, domain.utf8().data(), path.utf8().data(), secure);
    if (!it) {
        wkcMutexUnlockPeer(gCookieMutex);
        return false;
    }

    rawCookies.reserveCapacity(it->length);

    while (CURLSHE_OK == curl_share_advance_cookies_iterator(m_curlShareHandle, it)) {
        String name = String::fromUTF8(it->cookie.name);
        String value = String::fromUTF8(it->cookie.value);
        // All domains are prefixed with a dot if they allow tailmatching. See also get_netscape_format() in cURL/lib/cookie.c.
        String ndomain;
        if (it->cookie.tailmatch && it->cookie.domain && it->cookie.domain[0] != '.')
            ndomain = "." + String::fromUTF8(it->cookie.domain);
        else
            ndomain = String::fromUTF8(it->cookie.domain);
        String npath = String::fromUTF8(it->cookie.path);
        double expires = static_cast<double>(it->cookie.expires) * 1000;
        bool isHttpOnly = it->cookie.httponly;
        bool isSecure = it->cookie.secure;
        bool isSession = !expires;

        rawCookies.uncheckedAppend(Cookie(name, value, ndomain, npath, expires, isHttpOnly, isSecure, isSession));
    }

    curl_share_delete_cookies_iterator(m_curlShareHandle, it);

    wkcMutexUnlockPeer(gCookieMutex);

    return true;
}

void ResourceHandleManager::deleteCookie(const String &domain, const String &path, bool secure, const String &name)
{
    wkcMutexLockPeer(gCookieMutex);

    struct CookieJarIterator* it = curl_share_create_cookies_iterator(m_curlShareHandle, domain.utf8().data(), path.utf8().data(), secure);
    if (!it) {
        wkcMutexUnlockPeer(gCookieMutex);
        return;
    }

    while (CURLSHE_OK == curl_share_advance_cookies_iterator(m_curlShareHandle, it)) {
        if (!equalIgnoringCase(name, it->cookie.name))
            continue;
        // Set the cookie expired to delete.
        String expiredCookie = "Set-Cookie: " + name + "=; expires=Thu, 01-Jan-1970 00:00:00 GMT; path=" + path + "; domain=" + domain + ";";
        curl_share_set_cookie(m_curlShareHandle, domain.utf8().data(), path.utf8().data(), expiredCookie.utf8().data(), COOKIETYPE_HEADERLINE);
        break;
    }

    curl_share_delete_cookies_iterator(m_curlShareHandle, it);

    wkcMutexUnlockPeer(gCookieMutex);
}

//
// Reset Variables
//
void ResourceHandleManager::resetVariables()
{
    ResourceHandleManagerSSL::resetVariables();

    m_sharedInstance = 0;
}

void ResourceHandleManager::cancelByFrame(Frame* frame)
{
    int size = m_scheduledJobList.size();
    for (int i = size - 1; i >= 0; i--) {
        ResourceHandle* job = m_scheduledJobList[i];
        if (job->frame() == frame)
            job->cancel();
    }
    size = m_runningJobList.size();
    for (int i = size - 1; i >= 0; i--) {
        ResourceHandle* job = m_runningJobList[i];
        if (job->frame() == frame)
            job->cancel();
    }
}

void ResourceHandleManager::removeAllScheduledJobs()
{
    FUNCTIONPRINTF(("<rhm>removeAllScheduledJobs()"));

    ResourceHandle* job;
    int num = m_scheduledJobList.size();

    while (num--) {
        job = m_scheduledJobList[num];
        m_scheduledJobList.remove(num);
        ResourceHandleInternal* d = job->getInternal();
        if (d && !d->m_cancelled && d->client()) {
            d->client()->didFail(job, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("aborted"), 0));
            d->m_cancelled = true;
        }
        job->deref();
    }
}

//
// Running job control
//
void ResourceHandleManager::appendScheduledJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>appendScheduledJob(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    KURL kurl = job->firstRequest().url();
    if (kurl.isLocalFile() || kurl.protocolIsData()) {
        d->m_permitSend = ResourceHandleInternal::PERMIT;
    }
    d->m_url = fastStrdup(kurl.string().latin1().data());
    d->m_composition = contentComposition(job);

    m_scheduledJobList.append(job);

    canPermitRequest(job);
}

bool ResourceHandleManager::removeScheduledJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>removeScheduledJob(%p)", job));

    int size = m_scheduledJobList.size();
    for (int i = 0; i < size; i++) {
        if (job == m_scheduledJobList[i]) {
            m_scheduledJobList.remove(i);
            job->deref();
            return true;
        }
    }

#if ENABLE(WKC_HTTPCACHE)
    size = m_readCacheJobList.size();
    for (int i = 0; i < size; i++) {
        if (job == m_readCacheJobList[i]) {
            m_readCacheJobList.remove(i);
            job->deref();
            return true;
        }
    }
#endif
    return false;
}

ResourceHandle* ResourceHandleManager::shiftScheduledJob()
{
    FUNCTIONPRINTF(("<rhm>shiftScheduledJob()"));

    int size = m_scheduledJobList.size();
    if (0 == size)
        return (ResourceHandle*)0;

    ResourceHandle* job = 0;
    int i;
    for (i = 0; i < size; i++) {
        job = m_scheduledJobList[i];
        ResourceHandleInternal* d = job->getInternal();
        if (!d)
            continue;
        if (d->m_cancelled) {
            m_scheduledJobList.remove(i);
            return job;
        }

        switch (d->m_permitSend) {
        case ResourceHandleInternal::PERMIT:
        case ResourceHandleInternal::CANCEL:
            m_scheduledJobList.remove(i);
            return job;
        case ResourceHandleInternal::SUSPEND:
        case ResourceHandleInternal::ASKING:
            break;
        default:
            break;
        }

        if (d->m_fileLoading || d->m_dataLoading || d->m_matchProxyFilter) {
            m_scheduledJobList.remove(i);
            return job;
        }
        else {
            if (m_tryProxySingleConnect && m_runningJobList.size() > 0) { 
                continue;
            }
            m_scheduledJobList.remove(i);
            return job;
        }
    }
    return (ResourceHandle*)0;
}

//
// Running job control
//
void ResourceHandleManager::appendRunningJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>appendRunningJob(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    m_runningJobList.append(job);
    LOADINGPRINTF(("Start  Loading [%p] %s", job, job->getInternal()->m_url));
}

bool ResourceHandleManager::removeRunningJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>removeRunningJob(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    bool ret = false;
    int size = m_runningJobList.size();
    for (int i = 0; i < size; i++) {
        if (job == m_runningJobList[i]) {
            LOADINGPRINTF(("Finish Loading [%p]", job));
            m_runningJobList.remove(i);
            ret = true;
            break;
        }
    }

    if (ret && d) {
        if (d->m_isSSL)
            m_rhmssl->removeRunningJobSSL(job);
    }

    return ret;
}

ResourceHandle* ResourceHandleManager::shiftRunningJob()
{
    /* only it is called removeAllRunningJobs() */
    FUNCTIONPRINTF(("<rhm>shiftRunningJob()"));

    int size = m_runningJobList.size();
    ResourceHandle* job = 0;
    if (0 < size) {
        job = m_runningJobList[0];
        m_runningJobList.remove(0);
    }
    return job;
}

//
// After Auth Challenge Process
//
bool ResourceHandleManager::setAuthChallenge(ResourceHandle* job, bool isMulti)
{
    FUNCTIONPRINTF(("<rhm>setAuthChallenge(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    if (!d->m_currentWebChallenge.isNull() && d->m_didAuthChallenge) {
        // re-use d->m_handle

        long httpcode = d->m_response.httpStatusCode();
        d->m_didAuthChallenge = false;

        if (httpcode == 407 && !m_proxy.length()) {
            return false;
        }

        if (httpcode == 401 || httpcode == 407) {
            if (isMulti)
                curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);

            ProtectionSpaceServerType servertype;
            ProtectionSpaceAuthenticationScheme authscheme;
            String user;
            String pass;
            if (401 == httpcode) {
                if (m_authJar.getWebUserPassword(job->firstRequest().url().string().utf8().data(), servertype, authscheme, d->m_webAuthRealm, user, pass)) {
                    curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, ProtectionSpaceAuthSchemeTocURLType(authscheme));
                    curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, user.utf8().data());
                    curl_easy_setopt(d->m_handle, CURLOPT_PASSWORD, pass.utf8().data());
                    FUNCTIONMOREPRINTF(("<rhm> WWW Authentication: [%p]%s", job, d->m_url));
                }
                else {
                    return false;
                }
            }
            else {
                if (m_authJar.getProxyUserPassword(m_proxy, m_proxyPort, servertype, authscheme, user, pass)) {
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYAUTH,     ProtectionSpaceAuthSchemeTocURLType(authscheme));
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYUSERNAME, user.utf8().data());
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYPASSWORD, pass.utf8().data());
                    FUNCTIONMOREPRINTF(("<rhm> Proxy Authentication: [%p]%s", job, m_proxy.utf8().data()));
                }
                else {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void ResourceHandleManager::removeAllRunningJobs()
{
    FUNCTIONPRINTF(("<rhm>removeAllRunningJobs()"));

    ResourceHandle* job;

    while ((job = shiftRunningJob())) {
        ResourceHandleInternal* d = job->getInternal();
        if (d && d->m_handle) {
            if (d && !d->m_cancelled && d->client()) {
                d->client()->didFail(job, ResourceError(String(), CURLE_GOT_NOTHING, String(d->m_url), String("aborted"), 0));
                d->m_cancelled = true;
            }
            curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
            curl_easy_cleanup(d->m_handle);
            d->m_handle = 0;
        }
        job->deref();
    }
}

//
// can permit send callback
//
void ResourceHandleManager::canPermitRequest(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>canPermitRequest(%p)", job));

    if (!job)
        return;

    ResourceHandleInternal* d = job->getInternal();
    if (d->m_cancelled)
        return;

    if (!m_harmfulSiteFilter) {
        d->m_permitSend = ResourceHandleInternal::PERMIT;
        return;
    }

    if (ResourceHandleInternal::SUSPEND == d->m_permitSend) {
        d->m_permitSend = ResourceHandleInternal::ASKING;  // set asking

        if (d->m_composition != WKC::EErrorContentComposition && frameloaderclientwkc(job)) {
            int ret = frameloaderclientwkc(job)->dispatchWillPermitSendRequest(job, d->m_url, d->m_composition, false, d->m_response);
            if (1 == ret) {
                d->m_permitSend = ResourceHandleInternal::PERMIT;
            }
            else if (-1 == ret) {
                d->m_permitSend = ResourceHandleInternal::CANCEL;
                d->m_cancelled = true;
            }
        }
        else {
            d->m_permitSend = ResourceHandleInternal::CANCEL;
            d->m_cancelled = true;
        }
    }
}

void ResourceHandleManager::permitRequest(void* handle, bool permit)
{
    FUNCTIONPRINTF(("<rhm>permitRequest(%p, %s)", handle, (permit)?"permit":"prohibit"));

    ResourceHandle* requset = (ResourceHandle*)handle;

    ResourceHandle* job;
    ResourceHandleInternal* d;
    bool timer = false;

    int size = m_scheduledJobList.size();
    if (0 == size)
        return;

    int i;
    for (i = 0; i < size; i++) {
        job = m_scheduledJobList[i];
        if (!job || job != requset)
            continue;

        d = job->getInternal();
        if (!d)
            continue;
        if (d->m_cancelled)
            continue;

        if (d->m_permitSend != ResourceHandleInternal::ASKING)
            continue;

        if (permit) {
            d->m_permitSend = ResourceHandleInternal::PERMIT;
        }
        else {
            d->m_permitSend = ResourceHandleInternal::CANCEL;
            d->m_cancelled = true;
        }
        timer = true;
    }

    if (timer && !m_downloadTimer.isActive()) {
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }
}

//
// Do Redirect myself
//
bool ResourceHandleManager::didReceiveRedirect(ResourceHandle* job, KURL newURL)
{
    FUNCTIONPRINTF(("<rhm>didReceiveRedirect(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    if (m_redirectInWKC) {
        job->firstRequest().setURL(newURL);

        d->m_redirectCount++;
        if (d->m_redirectCount > d->m_redirectMaxCount) {
            return false;
        }

        if (d->m_url) {
            fastFree(d->m_url);
            d->m_url = 0;
        }
        if (d->m_urlhost) {
            fastFree(d->m_urlhost);
            d->m_urlhost = 0;
        }
        if (equalIgnoringCase(newURL.protocol(), "https")) {
            d->m_isSSL = true;
            d->m_SSLHandshaked = false;
        }

        removeRunningJob(job);

        switch(d->m_response.httpStatusCode()) {
            case 301: /* Moved Permanently */
            case 302: /* Found */
                if(equalIgnoringCase(job->firstRequest().httpMethod(), "POST") ||
                    equalIgnoringCase(job->firstRequest().httpMethod(), "PUT")) {
                        job->firstRequest().setHTTPMethod("GET");
                }
                break;
            case 303: /* See Other */
                if(!equalIgnoringCase(job->firstRequest().httpMethod(), "GET")) {
                        job->firstRequest().setHTTPMethod("GET");
                }
                break;
            default:
                /* 300 - Multiple Choices */
                /* 306 - Not used */
                /* 307 - Temporary Redirect */
                break;
        }


        d->m_doRedirectChallenge = true;
        return true;
    }
    else {
        /* redirect in cURL */
        if (d->m_url) {
            fastFree(d->m_url);
        }
        d->m_url  = fastStrdup(newURL.string().latin1().data());
        if (d->m_urlhost) {
            fastFree(d->m_urlhost);
        }
        d->m_urlhost = fastStrdup(hostAndPort(newURL).latin1().data());

        if (equalIgnoringCase(newURL.protocol(), "https")) {
            d->m_isSSL = true;
            d->m_SSLHandshaked = false;
            m_rhmssl->initializeHandleSSL(job);
        }

#if ENABLE(WKC_HTTPCACHE)
        curl_easy_setopt(d->m_handle, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
        if (!m_httpCache.disabled() && job->firstRequest().cachePolicy() != ReloadIgnoringCacheData) {
            HTTPCachedResource *resource = m_httpCache.resourceForURL(newURL);
            if (resource) {
                const String& lastModified = resource->lastModifiedHeader();
                const String& etag = resource->eTagHeader();
                if (!lastModified.isEmpty() || !etag.isEmpty()) {
                    job->getInternal()->m_utilizedHTTPCache = true;
                }
                if (!lastModified.isEmpty()) {
                    double ms = parseDate(lastModified);
                    if (isfinite(ms)) {
                        curl_easy_setopt(d->m_handle, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
                        curl_easy_setopt(d->m_handle, CURLOPT_TIMEVALUE, (time_t)(ms/1000));
                    }
                }
            }
        }
#endif // ENABLE(WKC_HTTPCACHE)

        return true;
    }
}

bool ResourceHandleManager::doRedirect(ResourceHandle* job, bool isMulti)
{
    FUNCTIONPRINTF(("<rhm>doRedirect(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    if (!d->m_doRedirectChallenge)
        return false;

    // re-use d->m_handle
    if (isMulti)
        curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);

    curl_easy_setopt(d->m_handle, CURLOPT_SHARE, 0);

    d->m_permitSend = ResourceHandleInternal::SUSPEND;
    d->m_doRedirectChallenge = false;

    appendScheduledJob(job);

    return true;
}

unsigned ResourceHandleManager::maximumTCPConnectionCountPerHostWindow()
{
    unsigned num;
    int tcpconnections = m_httpConnections + m_maxWebSocketConnections;

    if (0 == tcpconnections)
        num = 2048;
    else if (tcpconnections < 16)
        num = 32;
    else
        num = tcpconnections * 2;

    return num;
}

//
// Check this content is inclusion or root
//
int ResourceHandleManager::contentComposition(ResourceHandle* job)
{
    const Frame* frame = job->frame();
    if (!frame || !frame->loader() || !frame->loader()->documentLoader() || !frame->page())
        return WKC::EErrorContentComposition;
    if (!job->mainFrame() && !job->frameloaderclient())
        return WKC::EErrorContentComposition;

    FrameLoader* frameloader = frame->loader();
    DocumentLoader* documentloader = frameloader->documentLoader();

    if (documentloader->isLoadingPlugIns()) {
        return WKC::EOtherContentComposition;
    }
    if (frameloader->isLoadingMainFrame()) {
        if (documentloader->isLoadingSubresources())
            return WKC::EInclusionContentComposition;
        else
            return WKC::ERootFrameRootContentComposition;
    }
    else {
        if (documentloader->isLoadingSubresources())
            return WKC::EInclusionContentComposition;
        else
            return WKC::ESubFrameRootContentComposition;
    }

// for debug
// DP(("  => isLoadingMainResource(): %s", ((loader->isLoadingMainResource())?"true":"false")));
// DP(("  => isLoadingSubresources(): %s", ((loader->isLoadingSubresources())?"true":"false")));
// DP(("  => isLoadingMainFrame(): %s", ((loader->frameLoader()->isLoadingMainFrame())?"true":"false")));
// DP(("  => subframeIsLoading() : %s", ((loader->frameLoader()->subframeIsLoading())?"true":"false")));
    return WKC::EInclusionContentComposition;
}

bool ResourceHandleManager::oneShotDownloadTimer(void)
{
    if (!m_downloadTimer.isActive()) {
        m_downloadTimer.startOneShot(pollTimeSeconds);
        return true;
    }
    return false;
}

//
// SSL rapper
//
void ResourceHandleManager::SSLEnableProtocols(unsigned int versions)
{
    m_rhmssl->SSLEnableProtocols(versions);
}

void ResourceHandleManager::SSLEnableOnlineCertChecks(bool enableOCSP, bool enableCRLDP)
{
    m_rhmssl->SSLEnableOnlineCertChecks(enableOCSP, enableCRLDP);
}

void* ResourceHandleManager::SSLRegisterRootCA(const char* cert, int cert_len)
{
    return m_rhmssl->SSLRegisterRootCA(cert, cert_len);
}

int ResourceHandleManager::SSLUnregisterRootCA(void* certid)
{
    return m_rhmssl->SSLUnregisterRootCA(certid);
}

void* ResourceHandleManager::SSLRegisterRootCAByDER(const char* cert, int cert_len)
{
    return m_rhmssl->SSLRegisterRootCAByDER(cert, cert_len);
}

void ResourceHandleManager::SSLRootCADeleteAll(void)
{
    m_rhmssl->SSLRootCADeleteAll();
}

void* ResourceHandleManager::SSLRegisterCRL(const char* crl, int crl_len)
{
    return m_rhmssl->SSLRegisterCRL(crl, crl_len);
}

int ResourceHandleManager::SSLUnregisterCRL(void* crlid)
{
    return m_rhmssl->SSLUnregisterCRL(crlid);
}

void ResourceHandleManager::SSLCRLDeleteAll(void)
{
    m_rhmssl->SSLCRLDeleteAll();
}

void* ResourceHandleManager::SSLRegisterClientCert(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len)
{
    return m_rhmssl->SSLRegisterClientCert(pkcs12, pkcs12_len, pass, pass_len);
}
void* ResourceHandleManager::SSLRegisterClientCertByDER(const unsigned char* cert, int cert_len, const unsigned char* key, int key_len)
{
    return m_rhmssl->SSLRegisterClientCertByDER(cert, cert_len, key, key_len);
}

int ResourceHandleManager::SSLUnregisterClientCert(void* certid)
{
    return m_rhmssl->SSLUnregisterClientCert(certid);
}

void ResourceHandleManager::SSLClientCertDeleteAll(void)
{
    m_rhmssl->SSLClientCertDeleteAll();
}

bool ResourceHandleManager::SSLRegisterBlackCert(const char* in_issuerCommonName, const char* serialNumber)
{
    return m_rhmssl->SSLRegisterBlackCert(in_issuerCommonName, serialNumber);
}

void ResourceHandleManager::SSLBlackCertDeleteAll(void)
{
    m_rhmssl->SSLBlackCertDeleteAll();
}

bool ResourceHandleManager::isCertificateBlack(void* data)
{
    return m_rhmssl->isCertificateBlack(data);
}

bool ResourceHandleManager::SSLRegisterEVSSLOID(const char *issuerCommonName, const char *OID, const char *sha1FingerPrint, const char *SerialNumber)
{
    return m_rhmssl->SSLRegisterEVSSLOID(issuerCommonName, OID, sha1FingerPrint, SerialNumber);
}

void ResourceHandleManager::SSLEVSSLOIDDeleteAll(void)
{
    m_rhmssl->SSLEVSSLOIDDeleteAll();
}

const char** ResourceHandleManager::getServerCertChain(const char* url, int& outCertNum)
{
    return m_rhmssl->getServerCertChain(url, outCertNum);
}

void ResourceHandleManager::freeServerCertChain(const char** chain, int num)
{
    m_rhmssl->freeServerCertChain(chain, num);
}

void ResourceHandleManager::setAllowServerHost(const char *host_w_port)
{
    m_rhmssl->setAllowServerHost(host_w_port);
}

bool ResourceHandleManager::isMatchProxyFilter(const String& host)
{
    return m_proxyFilters.isMatchProxyFilter(host);
}

//
//  Proxy Filter
//
ResourceHandleManager::ProxyFilter::ProxyFilter()
    : m_proxyFilters(0)
{
}

ResourceHandleManager::ProxyFilter::~ProxyFilter()
{
    allClean();
    Filter* filter;
    int num = m_proxyFilters.size();
    while (num--) {
        filter = m_proxyFilters[num];
        m_proxyFilters.remove(num);
        delete filter;
    }
    if (!m_proxyFilters.isEmpty())
        m_proxyFilters.clear();
}

// public
void ResourceHandleManager::ProxyFilter::setProxyFilters(const String& filters)
{
    allClean();

    if (filters.isNull()) {
        return;
    }

    Vector<String> proxyFilters;
    filters.lower().split(',', proxyFilters);

    int size = proxyFilters.size();
    for (int i = 0; i < size; i++) {
        String filter = proxyFilters[i].stripWhiteSpace();
        if (filter.contains(' '))
            continue;

        if (setDomain(filter)) {
            continue;
        }
        else if (setIPAddress(filter)) {
            continue;
        }
        else if (setIPNetwork(filter)) {
            continue;
        }
        else if (setFQDN(filter)) {
            continue;
        }
    }
}

bool ResourceHandleManager::ProxyFilter::isMatchProxyFilter(const String& host)
{
    String lowerHost = host.lower();
    int hostLen = host.length();

    if (host == "127.0.0.1")
        return true;
    if (host == "localhost")
        return true;

    // remove IPv6 []
    if (host.startsWith("[") && host.endsWith("]")) {
        lowerHost = host.substring(1, host.length() - 2);
        hostLen = lowerHost.length();
    }

    int size = m_proxyFilters.size();
    for (int i = 0; i < size; i++) {
        Filter* filter = m_proxyFilters[i];
        if (FQDN == filter->m_type || IPAddress == filter->m_type) {
            if (filter->m_filter == lowerHost) {
                return true;
            }
        }
        else if (Domain == filter->m_type) {
            int filterLen = filter->m_filter.length();
            if (hostLen <= filterLen) {
                continue;
            }
            if (filter->m_filter == lowerHost.right(filterLen)) {
                return true;
            }
        }
        else {
            //  filter.m_type is IPMask
            unsigned char ipaddr[16];
            if (0 == convertIPStringToChar(host, ipaddr)) {
                continue;
            }
            bool ret = true;
            for (int j = 0; j < 16; j++) {
                if ((ipaddr[j] & filter->m_maskd[j]) != filter->m_maskd[j]) {
                    ret = false;
                    break;
                }
            }
            if (ret) {
                return true;
            }
        }
    }

    return false;
}

// private
void ResourceHandleManager::ProxyFilter::allClean()
{
    Filter* filter;
    int num = m_proxyFilters.size();
    while (num--) {
        filter = m_proxyFilters[num];
        m_proxyFilters.remove(num);
        delete filter;
    }
    if (!m_proxyFilters.isEmpty())
        m_proxyFilters.clear();
}

bool ResourceHandleManager::ProxyFilter::append(Filter* newfilter)
{
    int size = m_proxyFilters.size();
    for (int i = 0; i < size; i++) {
        Filter* filter = m_proxyFilters[i];
        if ((filter->m_type == newfilter->m_type)
            && (filter->m_filter == newfilter->m_filter)
            && (!memcmp((const void*)filter->m_maskd, (const void*)newfilter->m_maskd, 16))) {
            // has the same filter already
            return false;
        }
    }

    m_proxyFilters.append(newfilter);
    return true;
}

bool ResourceHandleManager::ProxyFilter::setFQDN(String filter)
{
    if (!filter.contains('.'))
        return false;

    if (hasProhibitCharactor(filter))
        return false;

    if (filter.endsWith("."))
        filter.left(filter.length() - 1);

    char buf[512];
    buf[0] = '.';
#ifdef WKC_ENABLE_I18N_IDN_SUPPORT
    const UChar* c = filter.charactersWithNullTermination();
    int ret = wkcI18NIDNfromUnicodePeer(c, filter.length(), (unsigned char*)(buf+1), sizeof(buf)-2);
#else
    CString cs = filter.utf8();
    const char* c = cs.data();
    int ret = 0;
    if (c) {
        strncpy(buf+1, c, 510);
        ret = 1;
    }
#endif
    if (c && 0 < ret) {
        Filter *newfilter = new Filter(FQDN, buf+1, NULL);
        if (newfilter && !append(newfilter)) {
            delete newfilter;
        }
    }

    Filter *newfilter = new Filter(Domain, buf, NULL);
    if (newfilter && !append(newfilter)) {
        delete newfilter;
    }

    return true;
}

bool ResourceHandleManager::ProxyFilter::setDomain(String filter)
{
    if (!filter.startsWith("."))
        return false;

    if (hasProhibitCharactor(filter))
        return false;

    if (filter.endsWith("."))
        filter.left(filter.length() - 1);

    char buf[512];
    buf[0] = '.';
#ifdef WKC_ENABLE_I18N_IDN_SUPPORT
    const UChar* r = filter.right(filter.length() - 1).charactersWithNullTermination();
    int ret = wkcI18NIDNfromUnicodePeer(r, filter.length() - 1, (unsigned char*)(buf + 1), sizeof(buf) - 2);
#else
    CString c = filter.right(filter.length() - 1).utf8();
    const char* r = c.data();
    int ret = 0;
    if (r) {
        strncpy(buf+1, r, 510);
        ret = 1;
    }
#endif
    if (r && 0 < ret) {
        Filter *newfilter = new Filter(Domain, buf, NULL);
        if (newfilter && !append(newfilter)) {
            delete newfilter;
        }
    }

    Filter *newfilter = new Filter(FQDN, buf+1, NULL);
    if (newfilter && !append(newfilter)) {
        delete newfilter;
    }

    return true;
}

bool ResourceHandleManager::ProxyFilter::setIPAddress(String filter)
{
    if (filter.contains('/'))
        return false;

    unsigned char ipaddr[16];
    int ipver = convertIPStringToChar(filter, ipaddr);
    if (4 != ipver && 6 != ipver)
        return 0;

    // remove IPv6 []
    if (filter.startsWith("[") && filter.endsWith("]")) {
        filter = filter.substring(1, filter.length() - 2);
    }

    Filter *newfilter = new Filter(IPAddress, filter, NULL);
    if (newfilter && !append(newfilter)) {
        delete newfilter;
    }

    return true;
}

bool ResourceHandleManager::ProxyFilter::setIPNetwork(String filter)
{
    Vector<String> IPs;
    unsigned char ipaddr[16];

    if (!filter.contains('/'))
        return false;

    filter.split('/', IPs);
    if (2 != IPs.size())
        return false;

    int ipver = convertIPStringToChar(IPs[0], ipaddr);
    if (4 != ipver && 6 != ipver)
        return false;

    bool isOK = false;
    int mask = IPs[1].toInt(&isOK);
    if (!isOK)
        return false;
    if (4 == ipver) {
        if (mask < 8 || 32 < mask)
            return false;
    }
    else {
        // 6 == ipver
        if (mask != 48 && mask != 64)
            return false;
    }

    for (int i = 0; i < 16; i++) {
        if (mask < 0)  mask = 0;
        if (mask < 8)  ipaddr[i] = (unsigned char)(ipaddr[i] & (0xff << (8 - mask)));
        mask -= 8;
    }

    Filter *newfilter = new Filter(IPMask, "", ipaddr);
    if (newfilter && !append(newfilter)) {
        delete newfilter;
    }

    return true;
}

#define CONV(c)  (((c) & 0x10 ? (c) : (c) + 9) & 0x0f)

int ResourceHandleManager::ProxyFilter::convertIPStringToChar(String host, unsigned char *ip)
{
    // host must be lower

    int hostLen = host.length();
    Vector<String> IPs;

    memset(ip, 0x0, 16);

    if (host.contains(':')) {
        // Maybe IPv6
        // remove IPv6 []
        if (host.startsWith("[") && host.endsWith("]")) {
            host = host.substring(1, host.length() - 2);
        }
        host.replace("::", ":#:");
        host.split(':', IPs);
        int size = IPs.size();
        if (8 < size)
            return 0;
        int pos = 0;
        for (int i = 0; i < size; i++) {
            String part = IPs[i];
            int len = part.length();
            if (4 < len)
                return 0;
            if ("#" == part) {
                pos = 8 - (size - i) + 1;
                continue;
            }
            if (!IPs[i].containsOnlyASCII())
                return 0;
            ip[pos*2 + 0] = ((3 < len) ? (CONV(*(part.substring(len - 4, 1).utf8().data())) << 4) : 0)
                            | ((2 < len) ?  CONV(*(part.substring(len - 3, 1).utf8().data())) : 0);
            ip[pos*2 + 1] = ((1 < len) ? (CONV(*(part.substring(len - 2, 1).utf8().data())) << 4) : 0)
                            | (CONV(*(part.substring(len - 1, 1).utf8().data())));
            pos++;
        }
        return 6;
    }
    else if (host.contains('.')) {
        // Maybe IPv4
        if (hostLen < 7)
            return 0;
        host.split('.', IPs);
        int size = IPs.size();
        if (4 != size)
            return 0;
        for (int i = 0; i < size; i++) {
            bool isOK = false;
            int num = IPs[i].toInt(&isOK);
            if (!isOK)
                return false;
            if (num < 0 || 255 < num)
                return false;
            ip[i] = (unsigned char)(num);
        }
        return 4;
    }

    return 0;
}

bool ResourceHandleManager::ProxyFilter::hasProhibitCharactor(String filter)
{
    // Prohibit Charactor Rules
    Vector<String> Parts;
    filter.split('.', Parts);
    int size = Parts.size();
    if (size < 2)
        return true;
    for (int i = 0; i < size; i++) {
        if (Parts[i].startsWith("-"))
            return true;
        if (Parts[i].endsWith("-"))
            return true;
        int len = Parts[i].length();
        for (int j = 0; j < len; j++) {
            String C = Parts[i].substring(j, 1);
            // following charactor should not use FQDN
            if (("!" == C) || ("\"" == C) || ("!" == C) || ("$" == C) || ("%" == C)
                || ("&" == C) || ("'" == C) || ("(" == C) || (")" == C) || ("*" == C)
                || ("+" == C) || ("," == C) || ("." == C) || ("/" == C) || (":" == C)
                || (";" == C) || ("<" == C) || ("=" == C) || (">" == C) || ("?" == C)
                || ("@" == C) || ("[" == C) || ("\\" == C) || ("]" == C) || ("^" == C)
                || ("`" == C) || ("{" == C) || ("|" == C) || ("}" == C) || ("~" == C)) {
                return true;
            }
        }
    }
    return false;
}

#if ENABLE(WKC_HTTPCACHE)
void ResourceHandleManager::clearHTTPCache()
{
    if (m_httpCache.disabled())
        return;

    ResourceHandle* job;
    int num = m_readCacheJobList.size();

    m_readCacheTimer.stop();
    while (num--) {
        job = m_readCacheJobList[num];
        m_readCacheJobList.remove(num);
        job->deref();
    }
    
    HTTPCachedResource *resource;
    num = m_writeCacheList.size();

    m_writeCacheTimer.stop();
    while (num--) {
        resource = m_writeCacheList[num];
        m_writeCacheList.remove(num);
        delete resource;
    }

    m_httpCache.removeAll();
    m_httpCache.purgeBySize(0);
    m_httpCache.writeFATFile();
    m_httpCache.readFATFile();
}

HTTPCachedResource* ResourceHandleManager::updateCacheResource(KURL &url, RefPtr<SharedBuffer> resourceData, ResourceResponse &response, bool noCache, bool noStore, bool mustRevalidate, double expires, double maxAge)
{
    HTTPCachedResource *resource = m_httpCache.resourceForURL(url);
    if (resource) {
        if (noStore) {
            m_httpCache.remove(resource);
            return NULL;
        } else {
            m_httpCache.updateCachedResource(resource, resourceData, response, noCache, mustRevalidate, expires, maxAge);
            m_httpCache.detach(resource);
            return resource;
        }
    }
    
    for(int num = 0; num < m_writeCacheList.size(); num++) {
        if (m_httpCache.equalHTTPCachedResourceURL(m_writeCacheList[num], url)) {
            if (noStore) {
                m_writeCacheList.remove(num);
                return NULL;
            }
            resource = m_writeCacheList[num];
            m_httpCache.updateCachedResource(resource, resourceData, response, noCache, mustRevalidate, expires, maxAge);
            m_writeCacheList.remove(num);
            break;
        }
    }
    return resource;
}


bool ResourceHandleManager::addHTTPCache(ResourceHandle *handle, KURL &url, RefPtr<SharedBuffer> resourceData, ResourceResponse &response)
{
    if (!resourceData)
        return false;

    if (!url.protocolIsInHTTPFamily())
        return false;

    if (handle->firstRequest().cachePolicy() == UseProtocolCachePolicy) {
        /*
         * RFC2616 Hypertext Transfer Protocol -- HTTP/1.1
         * 9.2 OPTIONS Responses to this method are not cacheable.
         */
        if (handle->firstRequest().httpMethod() == "OPTIONS") {
            return false;
        }
    }

    ResourceHandleInternal* d = handle->getInternal();

    bool noCache = response.cacheControlContainsNoCache();
    bool noStore = response.cacheControlContainsNoStore();
    bool mustRevalidate = response.cacheControlContainsMustRevalidate();
    double expires = std::numeric_limits<double>::quiet_NaN();
    double maxAge = std::numeric_limits<double>::quiet_NaN();

    if (d->m_httpequivFlags&HTTPCachedResource::EHTTPEquivNoCache)
        noCache = true;
    if (d->m_httpequivFlags&HTTPCachedResource::EHTTPEquivMustRevalidate)
        mustRevalidate = true;
    if (d->m_httpequivFlags&HTTPCachedResource::EHTTPEquivMaxAge)
        maxAge = d->m_httpequivMaxAge;

    HTTPCachedResource *resource = updateCacheResource(url, resourceData, response, noCache, noStore, mustRevalidate, expires, maxAge);
    if (!resource && !noStore) {
        resource = m_httpCache.createHTTPCachedResource(url, resourceData, response, noCache, mustRevalidate, expires, maxAge);
        if (!resource)
            return false;
    }
    if (noStore)
        return false;
    m_writeCacheList.append(resource);

    if (!m_writeCacheTimer.isActive())
        m_writeCacheTimer.startOneShot(pollTimeSeconds);

    return true;
}

void ResourceHandleManager::scheduleLoadResourceFromHTTPCache(ResourceHandle *job)
{
    job->ref();
    m_readCacheJobList.append(job);
    if (!m_readCacheTimer.isActive()) {
        m_readCacheTimer.startOneShot(pollTimeSeconds);
    }
}

void ResourceHandleManager::readCacheTimerCallback(Timer<ResourceHandleManager>* timer)
{
    ResourceHandle* job;

    if (m_readCacheJobList.size() == 0)
        return;

    job = m_readCacheJobList.first();
    KURL kurl = job->firstRequest().url();

    HTTPCachedResource *resource = m_httpCache.resourceForURL(kurl);
    ResourceHandleInternal* d = job->getInternal();
    const Frame* frame = job->frame();
    if (frameloaderclientwkc(job) && resource) {
        if (!frameloaderclientwkc(job)->dispatchWillReceiveData(job, resource->resourceSize()))
            goto cancel;
        if (d->client() && !d->client()->willReceiveData(job, resource->resourceSize()))
            goto cancel;
        d->m_response.setResourceHandle(job);
        d->m_response.setURL(kurl);
        d->m_response.setMimeType(resource->mimeType());
        d->m_response.setExpectedContentLength(resource->expectedContentLength());
        d->m_response.setTextEncodingName(resource->textEncodingName());
        d->m_response.setSuggestedFilename(resource->suggestedFilename());
        d->m_response.setHTTPStatusCode(resource->httpStatusCode());
        d->m_response.setHTTPHeaderField("Last-Modified", resource->lastModifiedHeader());
        d->m_response.setWasCached(true);
        if (!d->client())
            goto cancel;
        d->client()->didReceiveResponse(job, d->m_response);
        if (d->m_cancelled)
            goto cancel;
        if (frame!=job->frame()) {
            goto cancel;
        }
        if (!frameloaderclientwkc(job) || !frameloaderclientwkc(job)->dispatchWillReceiveData(job, resource->contentLength()))
            goto cancel;
        if (!d->client()->willReceiveData(job, resource->contentLength()))
            goto cancel;

        char *buf;
        buf = (char*)fastMalloc(resource->contentLength());
        if (buf) {
            if (m_httpCache.read(resource, buf)) {
                d->client()->didReceiveData(job, buf, resource->contentLength(), 0);
                d->client()->didFinishLoading(job, currentTime());
            } else {
                d->client()->didFail(job, ResourceError(String(), CURLE_READ_ERROR, String(d->m_url), String("cache read error"), 0));
            }
            fastFree(buf);
        }
    }

    if (m_readCacheJobList.size()) {
        m_readCacheJobList.remove(0);
        job->deref();
    }

    if (m_readCacheJobList.size() == 0)
        return;

    if (!m_readCacheTimer.isActive())
        m_readCacheTimer.startOneShot(pollTimeSeconds);

    return;

cancel:
    if (!d->m_cancelled) {
        if (d->m_handle)
            cancel(job);
        if (d->client())
            d->client()->didFail(job, ResourceError(String(), CURLE_READ_ERROR, String(d->m_url), String("cache read error"), 0));
    }

    if (m_readCacheJobList.size() == 0)
        return;

    if (!m_readCacheTimer.isActive())
        m_readCacheTimer.startOneShot(pollTimeSeconds);
}

void ResourceHandleManager::writeCacheTimerCallback(Timer<ResourceHandleManager>* timer)
{
    if (m_writeCacheList.size() == 0)
        return;

    if (m_scheduledJobList.size() > 0 || m_runningJobList.size() > 0) {
        m_writeCacheTimer.startOneShot(pollTimeSeconds);
        return;
    }

    HTTPCachedResource *resource;
    resource = m_writeCacheList.first();

    bool write = m_httpCache.write(resource);
    m_writeCacheList.remove(0);
    if (!write) {
        delete resource;
    }

    if (m_writeCacheList.size() == 0) {
        m_httpCache.writeFATFile();
        return;
    }

    if (!m_writeCacheTimer.isActive())
        m_writeCacheTimer.startOneShot(pollTimeSeconds);
}

void ResourceHandleManager::resetHTTPCache()
{
    ResourceHandle* job;
    int num = m_readCacheJobList.size();

    m_readCacheTimer.stop();
    while (num--) {
        job = m_readCacheJobList[num];
        m_readCacheJobList.remove(num);
        job->deref();
    }
    
    HTTPCachedResource *resource;
    num = m_writeCacheList.size();

    m_writeCacheTimer.stop();
    while (num--) {
        resource = m_writeCacheList[num];
        m_writeCacheList.remove(num);
        delete resource;
    }
    
    m_httpCache.reset();
}

void ResourceHandleManager::processHttpEquiv(const String& content, const KURL& url, bool cachecontrol)
{
    if (!httpCache())
        return;

    int sec = -1;
    bool ismaxage = false;
    if (content.is8Bit()) {
        ismaxage = equalIgnoringCase(content.characters8(), (const LChar *)"max-age", 7);
    } else {
        ismaxage = equalIgnoringCase(content.characters16(), "max-age", 7);
    }
    if (cachecontrol && ismaxage && content.length()>7) {
        int pos = content.find('=');
        if (pos!=WTF::notFound && content.length()>pos+1) {
            const String num = content.substring(pos+1).stripWhiteSpace();
            if (num.length()>0) {
                bool ok = false;
                sec = num.toInt(&ok);
                if (!ok)
                    sec = -1;
            }
        }
    }

    HTTPCachedResource* rsp = httpCache()->resourceForURL(url);
    if (!rsp) {
        int len = m_writeCacheList.size();
        for (int i=0; i<len; i++) {
            if (m_writeCacheList[i] && m_writeCacheList[i]->url()==url) {
                rsp = m_writeCacheList[i];
                break;
            }
        }
        if (!rsp) {
            int num = m_scheduledJobList.size();
            while (num--) {
                ResourceHandle* job = m_scheduledJobList[num];
                if (!job) continue;
                ResourceHandleInternal* d = job->getInternal();
                if (!d) continue;
                const KURL rurl(ParsedURLString, d->m_url);
                if (rurl != url) continue;
                if (equalIgnoringCase(content, "no-cache")) {
                    d->m_httpequivFlags |= HTTPCachedResource::EHTTPEquivNoCache;
                    break;
                } else if (cachecontrol && equalIgnoringCase(content, "must-revalidate")) {
                    d->m_httpequivFlags |= HTTPCachedResource::EHTTPEquivMustRevalidate;
                    break;
                } else if (sec>=0) {
                    d->m_httpequivMaxAge = sec;
                    d->m_httpequivFlags |= HTTPCachedResource::EHTTPEquivMaxAge;
                    break;
                }
            }
            return;
        }
    }

    bool nocache = rsp->noCache();
    bool mustrevalidate = rsp->mustRevalidate();
    double expires = rsp->expires();
    double maxage = rsp->maxAge();

    int flags = 0;
    if (equalIgnoringCase(content, "no-cache")) {
        nocache = true;
        flags |= HTTPCachedResource::EHTTPEquivNoCache;
    } else if (cachecontrol && equalIgnoringCase(content, "must-revalidate")) {
        mustrevalidate = true;
        flags |= HTTPCachedResource::EHTTPEquivMustRevalidate;
    } else if (sec>=0) {
        maxage = sec;
        flags |= HTTPCachedResource::EHTTPEquivMaxAge;
    }

    rsp->update(nocache, mustrevalidate, expires, maxage, flags);
}

#endif // ENABLE(WKC_HTTPCACHE)

//
// WebSocket's socket reserver
//
bool
ResourceHandleManager::reserveWebSocketConnection(SocketStreamHandle* handle)
{
    // If the WebSocket connection was full, try to close the connection waiting to Close message.
    if (m_socketStreamHandleList.size() >= m_maxWebSocketConnections) {
        Vector<SocketStreamHandle*> underClosing;
        for (int num = 0; num < m_socketStreamHandleList.size(); num++) {
            if (m_socketStreamHandleList[num] && m_socketStreamHandleList[num]->isClosingSocketStreamChannel())
                underClosing.append(m_socketStreamHandleList[num]);
        }
        for (int num = 0; num < underClosing.size(); num++) {
            if (underClosing[num])
                underClosing[num]->disconnect();
            // ResourceHandleManager::releaseWebSocketConnection() is called when a socket is actually closed by SocketStreamHandle::platformClose(). 
            // Therefore, there is no need to call releaseWebSocketConnection() here.
        }
    }

    if (m_socketStreamHandleList.size() >= m_maxWebSocketConnections)
        return false;

    m_socketStreamHandleList.append(handle);
    return true;
}

void
ResourceHandleManager::releaseWebSocketConnection(SocketStreamHandle* handle)
{
    for (int num = 0; num < m_socketStreamHandleList.size(); num++) {
        if (m_socketStreamHandleList[num] == handle) {
            m_socketStreamHandleList.remove(num);
            break;
        }
    }
}

bool
ResourceHandleManager::canStartWebSocketConnecting(SocketStreamHandle* handle)
{
    if (!handle)
        return false;
    if (!m_socketStreamHandleList.contains(handle))
        return false;

    const String& target_host = handle->url().host();
    int target_port = handle->url().port();

    for (size_t num = 0; num < m_socketStreamHandleList.size(); num++) {
        SocketStreamHandle* item = m_socketStreamHandleList[num];
        if (item == handle)
            continue;
        const KURL& url = item->url();
        if (target_host!=url.host() || target_port!=url.port())
            continue;
        switch (item->socketState()) {
        case SocketStreamHandle::Initialized:
            continue;
        case SocketStreamHandle::Active:
            continue;
        default:
            return false;
        }
    }
    return true;
}

int
ResourceHandleManager::getCurrentWebSocketConnectionsNum()
{
    return m_socketStreamHandleList.size();
}
} // namespace WebCore
