/*
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"

#if ENABLE(WKC_HTTPCACHE)

#include "HTTPCacheWKC.h"
#include "SharedBuffer.h"
#include "KURL.h"
#include "CString.h"
#include "FileSystem.h"
#include "ResourceResponse.h"
#include "ResourceHandleClient.h"
#include <wtf/MathExtras.h>
#include <wtf/MD5.h>
#include <wkc/wkcclib.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcmpeer.h>

#ifdef __WKC_IMPLICIT_INCLUDE_SYSSTAT
#include <sys/stat.h>
#endif


#if 1
# define W_DP(a) ((void)0)
#else
#include <wkc/wkcpeer.h>
# define W_DP(a) wkcDebugPrintfPeer a
#endif

namespace WebCore {

// HTTPCacheResource

HTTPCachedResource::HTTPCachedResource()
{
    m_used = false;
    m_httpequivflags = 0;
    m_resourceData = 0;

    m_expectedContentLength = 0;
    m_httpStatusCode = 0;
    m_noCache = false;
    m_mustRevalidate = false;
    m_expires = 0;
    m_maxAge = 0;
    m_date = 0;
    m_lastModified = 0;

    m_contentLength = 0;
    m_resourceSize = 0;
}

HTTPCachedResource::HTTPCachedResource(const KURL &url, const ResourceResponse &response)
{
    m_used = false;
    m_httpequivflags = 0;

    m_contentLength = 0;
    m_resourceSize = 0;

    m_url = url.string();
    setResourceResponse(response);
}


HTTPCachedResource::~HTTPCachedResource()
{
    if (m_resourceData)
        m_resourceData = 0;
}

// copy from CachedResource::freshnessLifetime()
double HTTPCachedResource::freshnessLifetime()
{
    // RFC2616 13.2.4
    if (isfinite(m_maxAge))
        return m_maxAge;
    if (isfinite(m_expires))
        return m_expires - m_date;
    if (isfinite(m_lastModified))
        return (m_date - m_lastModified) * 0.1;
    // If no cache headers are present, the specification leaves the decision to the UA. Other browsers seem to opt for 0.
    return 0;
}

bool HTTPCachedResource::isExpired()
{
    return (currentTime() - m_date > freshnessLifetime());
}

bool HTTPCachedResource::needRevalidate()
{
    return m_mustRevalidate || m_noCache || isExpired();
}

void HTTPCachedResource::update(bool noCache, bool mustRevalidate, double expires, double maxAge, int httpequivflags)
{
    if ((!(m_httpequivflags&EHTTPEquivNoCache)) || (httpequivflags&EHTTPEquivNoCache))
        m_noCache = noCache;
    if ((!(m_httpequivflags&EHTTPEquivMustRevalidate)) || (httpequivflags&EHTTPEquivMustRevalidate))
        m_mustRevalidate = mustRevalidate;

    if ((!(m_httpequivflags&EHTTPEquivMaxAge)) || (httpequivflags&EHTTPEquivMaxAge)) {
        if (isfinite(maxAge)) {
            if (!isfinite(m_maxAge) || (m_maxAge > maxAge))
                m_maxAge = maxAge;
        }
    }
    if (isfinite(expires)) {
        if (!isfinite(m_expires) || (m_expires > expires))
            m_expires = expires;
    }

    m_httpequivflags |= httpequivflags;
}

bool HTTPCachedResource::setResourceResponse(const ResourceResponse &response)
{
    m_mimeType = response.mimeType();
    m_expectedContentLength = response.expectedContentLength();
    m_textEncodingName = response.textEncodingName();
    m_suggestedFilename = response.suggestedFilename();
    m_httpStatusCode = response.httpStatusCode();

    m_expires = response.expires();
    m_date = isfinite(response.date()) ? response.date() : currentTime();
    m_lastModified = response.lastModified();
    m_lastModifiedHeader = response.httpHeaderField("Last-Modified");
    m_eTagHeader = response.httpHeaderField("ETag");

    if (!(m_httpequivflags&EHTTPEquivNoCache))
        m_noCache = response.cacheControlContainsNoCache();
    if (!(m_httpequivflags&EHTTPEquivMustRevalidate))
        m_mustRevalidate |= response.cacheControlContainsMustRevalidate();
    if (!(m_httpequivflags&EHTTPEquivMaxAge))
        m_maxAge = response.cacheControlMaxAge();

    return true;
}

bool HTTPCachedResource::writeFile(const String& filename, const String& filepath)
{
    void *fd = 0;
    bool result = false;

    if (!m_resourceData)
        return false;

    String fileFullPath = pathByAppendingComponent(filepath, filename);

    fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_CACHE, fileFullPath.utf8().data(), "wb");
    if (!fd)
        return result;

    const char *buf;
    int len, length, written = 0, position = 0;

    for (len = m_resourceData->size(); len > 0;  len -= length) {
        length = m_resourceData->getSomeData(buf, position);
        if (!length)
            goto error_end;

        for (int writelength = length; writelength > 0; writelength -= written) {
            written = wkcFileFWritePeer(buf, sizeof(char), writelength, fd);
            if (written < 0)
                goto error_end;
        }
        position += length;
    }
    W_DP(("cache write %s", m_url.utf8().data()));

    m_contentLength = m_resourceData->size();
    m_fileName = filename;
    calcResourceSize();

    m_resourceData = 0;

    result = true;
error_end:
    if (fd)
        wkcFileFClosePeer(fd);

    return result;
}

bool HTTPCachedResource::readFile(char *out_buf, const String& filepath)
{
    void *fd = 0;
    bool result = false;
    char *buf = out_buf;

    String fileFullPath = pathByAppendingComponent(filepath, m_fileName);

    fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_CACHE, fileFullPath.utf8().data(), "rb");
    if (!fd)
        goto error_end;

    int len, read;
    for (len = m_contentLength; len > 0; len -= read) {
        read = wkcFileFReadPeer(buf, sizeof(char), len, fd);
        if (read < 0)
            break;
        buf += read; 
    }
    W_DP(("cache read %s", m_url.utf8().data()));

    m_used = true;
    result = true;
error_end:
    if (fd)
        wkcFileFClosePeer(fd);

    return result;
}

#define ROUNDUP(x, y)   (((x)+((y)-1))/(y)*(y))
#define ROUNDUP_UNIT    4

void HTTPCachedResource::calcResourceSize()
{
    int size = sizeof(long long) * 2
        + sizeof(int) * 3
        + sizeof(double) * 4
        + sizeof(int) + ROUNDUP(m_url.utf8().length(), ROUNDUP_UNIT)
        + sizeof(int) + ROUNDUP(m_mimeType.utf8().length(), ROUNDUP_UNIT)
        + sizeof(int) + ROUNDUP(m_textEncodingName.utf8().length(), ROUNDUP_UNIT)
        + sizeof(int) + ROUNDUP(m_suggestedFilename.utf8().length(), ROUNDUP_UNIT)
        + sizeof(int) + ROUNDUP(m_fileName.utf8().length(), ROUNDUP_UNIT)
        + sizeof(int) + ROUNDUP(m_lastModifiedHeader.utf8().length(), ROUNDUP_UNIT)
        + sizeof(int) + ROUNDUP(m_eTagHeader.utf8().length(), ROUNDUP_UNIT);

    m_resourceSize = ROUNDUP(size, ROUNDUP_UNIT);
}

int writeString(char *buffer, const String &str)
{
    CString cstr = str.utf8();
    int length = cstr.length();

    (*(int*)buffer) = length; buffer += sizeof(int);

    if (length>0)
        memcpy(buffer, cstr.data(), length);
    return ROUNDUP(length + sizeof(int), ROUNDUP_UNIT);
}

int HTTPCachedResource::serialize(char *buffer)
{
    char *buf = buffer;

    // number field
    (*(long long*)buffer) = m_expectedContentLength; buffer += sizeof(long long);
    (*(long long*)buffer) = m_contentLength; buffer += sizeof(long long);
    (*(int*)buffer) = m_httpStatusCode; buffer += sizeof(int);
    (*(int*)buffer) = m_noCache ? 1 : 0; buffer += sizeof(int);
    (*(int*)buffer) = m_mustRevalidate ? 1 : 0; buffer += sizeof(int);
    (*(double*)buffer) = m_expires; buffer += sizeof(double);
    (*(double*)buffer) = m_maxAge; buffer += sizeof(double);
    (*(double*)buffer) = m_date; buffer += sizeof(double);
    (*(double*)buffer) = m_lastModified; buffer += sizeof(double);
    // string field
    buffer += writeString(buffer, m_url);
    buffer += writeString(buffer, m_mimeType);
    buffer += writeString(buffer, m_textEncodingName);
    buffer += writeString(buffer, m_suggestedFilename);
    buffer += writeString(buffer, m_fileName);
    buffer += writeString(buffer, m_lastModifiedHeader);
    buffer += writeString(buffer, m_eTagHeader);

    return ROUNDUP(buffer - buf, ROUNDUP_UNIT);
}

int readString(char *buffer, String &str)
{
    int length;
    
    length= (*(int*)buffer); buffer += sizeof(int);
    str = String::fromUTF8(buffer, length);

    return ROUNDUP(length + sizeof(int), ROUNDUP_UNIT);
}

int HTTPCachedResource::deserialize(char *buffer)
{
    char *buf = buffer;

    // number field
    m_expectedContentLength = (*(long long*)buffer); buffer += sizeof(long long);
    m_contentLength = (*(long long*)buffer); buffer += sizeof(long long);
    m_httpStatusCode = (*(int*)buffer); buffer += sizeof(int);
    m_noCache = (*(int*)buffer); buffer += sizeof(int);
    m_mustRevalidate = (*(int*)buffer); buffer += sizeof(int);
    m_expires = (*(double*)buffer); buffer += sizeof(double);
    m_maxAge = (*(double*)buffer); buffer += sizeof(double);
    m_date = (*(double*)buffer); buffer += sizeof(double);
    m_lastModified = (*(double*)buffer); buffer += sizeof(double);
    // string field
    buffer += readString(buffer, m_url);
    buffer += readString(buffer, m_mimeType);
    buffer += readString(buffer, m_textEncodingName);
    buffer += readString(buffer, m_suggestedFilename);
    buffer += readString(buffer, m_fileName);
    buffer += readString(buffer, m_lastModifiedHeader);
    buffer += readString(buffer, m_eTagHeader);

    calcResourceSize();

    return ROUNDUP(buffer - buf, ROUNDUP_UNIT);
}


// HTTPCache

#define DEFAULT_FAT_FILENAME        "cache.fat"
#define DEFAULT_FILEPATH            "cache/"
#define DEFAULT_CONTENTENTRIES_LIMIT     (1024)
#define DEFAULT_CONTENTSIZE_LIMIT        (10 * 1024 * 1024)
#define DEFAULT_TOTALCONTENTSSIZE_LIMIT  (10 * 1024 * 1024)

HTTPCache::HTTPCache()
    : m_fatFileName(DEFAULT_FAT_FILENAME)
    , m_filePath(DEFAULT_FILEPATH)
{
    m_disabled = false;
    m_fileNumber = 0;
    m_totalResourceSize = 0;

    m_totalContentsSize = 0;
    m_limitContentEntries = DEFAULT_CONTENTENTRIES_LIMIT;
    m_limitContentSize = DEFAULT_CONTENTSIZE_LIMIT;
    m_limitTotalContentsSize = DEFAULT_TOTALCONTENTSSIZE_LIMIT;
}

HTTPCache::~HTTPCache()
{
    reset();
}

void HTTPCache::reset()
{
    HTTPCachedResource *resource;

    for (int num = 0; num < m_resourceList.size(); num++) {
        resource = m_resourceList[num];
        delete resource;
    }
    m_resourceList.clear();
    m_resources.clear();

    m_totalResourceSize = 0;
    m_totalContentsSize = 0;
}

HTTPCachedResource* HTTPCache::createHTTPCachedResource(KURL &url, RefPtr<SharedBuffer> resourceData, ResourceResponse &response, bool noCache, bool mustRevalidate, double expires, double maxAge)
{
    if (disabled())
        return 0;

    int contentLength = resourceData->size();
    if (m_limitContentSize < contentLength)
        return 0;   // size over
    if (m_limitTotalContentsSize < contentLength)
        return 0;   // size over
    if (m_limitTotalContentsSize < m_totalContentsSize + contentLength)
        if (purgeBySize(contentLength)==0)
            return 0; // total size over
    if (m_resourceList.size() >= m_limitContentEntries)
        if (purgeOldest()==0)
            return 0; // entry limit over

    const KURL kurl = removeFragmentIdentifierIfNeeded(url);
    HTTPCachedResource *resource = new HTTPCachedResource(kurl, response);
    if (!resource)
        return 0;
    
    resource->update(noCache, mustRevalidate, expires, maxAge);
    resource->setResourceData(resourceData);
    return resource;
}

bool HTTPCache::addCachedResource(HTTPCachedResource *resource)
{
    if (disabled())
        return false;

    m_resources.set(resource->url(), resource);
    m_resourceList.append(resource);
    m_totalResourceSize += resource->resourceSize();
    m_totalContentsSize += resource->contentLength();

    return true;
}

void HTTPCache::updateCachedResource(HTTPCachedResource *resource, RefPtr<SharedBuffer> resourceData, ResourceResponse &response, bool noCache, bool mustRevalidate, double expires, double maxAge)
{
    resource->setResourceResponse(response);
    resource->setResourceData(resourceData);
    resource->update(noCache, mustRevalidate, expires, maxAge);
}

void HTTPCache::removeResource(HTTPCachedResource *resource)
{
    m_resources.remove(resource->url());
    for (int num = 0; num < m_resourceList.size(); num++) {
        if (m_resourceList[num] == resource) {
            m_resourceList.remove(num);
            break;
        }
    }
    m_totalResourceSize -= resource->resourceSize();
    m_totalContentsSize -= resource->contentLength();
}

void HTTPCache::removeResourceByNumber(int listNumber)
{
    HTTPCachedResource *resource = m_resourceList[listNumber];

    W_DP(("remove cache file %s", resource->fileName().utf8().data()));

    m_resources.remove(resource->url());
    m_resourceList.remove(listNumber);
    m_totalResourceSize -= resource->resourceSize();
    m_totalContentsSize -= resource->contentLength();

    const String fileFullPath = pathByAppendingComponent(m_filePath, resource->fileName());
    wkcFileUnlinkPeer(fileFullPath.utf8().data());

    delete resource;
}

void HTTPCache::remove(HTTPCachedResource *resource)
{
    removeResource(resource);

    W_DP(("remove cache file %s", resource->fileName().utf8().data()));

    const String fileFullPath = pathByAppendingComponent(m_filePath, resource->fileName());
    wkcFileUnlinkPeer(fileFullPath.utf8().data());

    delete resource;
}

void HTTPCache::detach(HTTPCachedResource *resource)
{
    removeResource(resource);

    if (resource->fileName().isEmpty())
        return;
    const String fileFullPath = pathByAppendingComponent(m_filePath, resource->fileName());
    wkcFileUnlinkPeer(fileFullPath.utf8().data());
}

void HTTPCache::removeAll()
{
    reset();
    m_fileNumber = 0;

    Vector<String> items(listDirectory(m_filePath.utf8().data(), "*.dcf"));

    char fullpath[MAX_PATH] = {0};
    int count = items.size();
    for (int i=0; i<count; i++) {
        int ret = wkcFilePathByAppendingComponentPeer(m_filePath.utf8().data(), items[i].utf8().data(), fullpath, MAX_PATH);
        if (!ret)
            break;
        wkcFileUnlinkPeer(fullpath);
    }

    int ret = wkcFilePathByAppendingComponentPeer(m_filePath.utf8().data(), m_fatFileName.utf8().data(), fullpath, MAX_PATH);
    if (ret)
        wkcFileUnlinkPeer(fullpath);
}

void HTTPCache::setDisabled(bool disabled)
{
    m_disabled = disabled;
    if (!disabled)
        return;
}

void HTTPCache::setMaxContentEntries(int limit)
{
    if (limit<=0)
        limit = 0x7fffffff; // INT_MAX
    m_limitContentEntries = limit;
}

void HTTPCache::setMaxContentSize(long long limit)
{
    if (limit<=0)
        limit = 0x7fffffffffffffffLL; // LONG_LONG_MAX
    m_limitContentSize = limit;
}

void HTTPCache::setMaxTotalCacheSize(long long limit)
{
    if (limit<=0)
        limit = 0x7fffffffffffffffLL; // LONG_LONG_MAX
    m_limitTotalContentsSize = limit;

    if (limit < m_totalContentsSize) {
        purgeBySize(m_totalContentsSize - limit);
        writeFATFile();
    }
}

void HTTPCache::setFilePath(const char* path)
{
    if (!path || !strlen(path))
        return;

    // TODO: If path is same as the default one, readFATFile will never be called even if HTTPCache is enabled...
    if (m_filePath == path)
        return;

    m_filePath = String::fromUTF8(path);

    // ensure the destination path is exist
    (void)makeAllDirectories(m_filePath);

    readFATFile();
}

KURL HTTPCache::removeFragmentIdentifierIfNeeded(const KURL& originalURL)
{
    if (!originalURL.hasFragmentIdentifier())
        return originalURL;
    // Strip away fragment identifier from HTTP URLs.
    // Data URLs must be unmodified. For file and custom URLs clients may expect resources 
    // to be unique even when they differ by the fragment identifier only.
    if (!originalURL.protocolIsInHTTPFamily())
        return originalURL;
    KURL url = originalURL;
    url.removeFragmentIdentifier();
    return url;
}

HTTPCachedResource* HTTPCache::resourceForURL(const KURL& resourceURL)
{
    KURL url = removeFragmentIdentifierIfNeeded(resourceURL);
    HTTPCachedResource* resource = m_resources.get(url);

    return resource;
}

bool HTTPCache::equalHTTPCachedResourceURL(HTTPCachedResource *resource, KURL& resourceURL)
{
    KURL url = removeFragmentIdentifierIfNeeded(resourceURL);
    if (equalIgnoringCase(resource->url(), url.string()))
        return true;

    return false;
}

String HTTPCache::makeFileName()
{
    String fileName;

    fileName = String::format("%08d.dcf", m_fileNumber);
    if (m_fileNumber==WKC_INT_MAX)
        m_fileNumber = 0;
    else
        m_fileNumber++;

    return fileName;
}

long long HTTPCache::purgeBySizeInternal(long long size)
{
    HTTPCachedResource *resource;
    long long purgedSize = 0;

    for (int num = 0; num < m_resourceList.size(); ) {
        resource = m_resourceList[num];
        if (resource->isUsed())
            num++;
        else {
            purgedSize += resource->contentLength();
            removeResourceByNumber(num);
            if (purgedSize > size)
                break;
        }
    }
    return purgedSize;
}

long long HTTPCache::purgeBySize(long long size)
{
    long long purgedSize = purgeBySizeInternal(size);
    if (purgedSize < size) {
        for (int num = 0; num < m_resourceList.size(); num++)
            m_resourceList[num]->setUsed(false);
        purgedSize +=purgeBySizeInternal(size - purgedSize);
    }
    return purgedSize;
}

long long HTTPCache::purgeOldest()
{
    HTTPCachedResource *resource;
    long long purgedSize = 0;

    for (int num = 0; num < m_resourceList.size(); ) {
        resource = m_resourceList[num];
        if (resource->isUsed())
            num++;
        else {
            purgedSize += resource->contentLength();
            removeResourceByNumber(num);
            break;
        }
    }
    return purgedSize;
}

bool HTTPCache::write(HTTPCachedResource *resource)
{
    long long contentLength = resource->contentLength();
    if (m_totalContentsSize + contentLength > m_limitTotalContentsSize)
        purgeBySize(contentLength);

    if (!resource->writeFile(makeFileName(), m_filePath))
        return false;
    
    m_resources.add(resource->url(), resource);
    m_resourceList.append(resource);
    m_totalResourceSize += resource->resourceSize();
    m_totalContentsSize += resource->contentLength();

    return true;
}

bool HTTPCache::read(HTTPCachedResource *resource, char *buf)
{
    return resource->readFile(buf, m_filePath);
}

void HTTPCache::serializeFATData(char *buffer)
{
    for (int num = 0; num < m_resourceList.size(); num++)
        buffer += m_resourceList[num]->serialize(buffer);
}

bool HTTPCache::deserializeFATData(char *buffer, int length)
{
    int read = 0;
    HTTPCachedResource *resource;

    for (; length > 0; length -= read) {
        resource = new HTTPCachedResource();
        if (!resource)
            return false;

        read = resource->deserialize(buffer);
        buffer += read;
        addCachedResource(resource);
    }    
    return true;
}

#define DEFAULT_CACHEFAT_FILENAME   "cache.fat"
#define CACHEFAT_FORMAT_VERSION     3  // Number of int. Increment this if you changed the content format in the fat file.

#define MD5_DIGESTSIZE 16

void HTTPCache::writeDigest(unsigned char *data, int length)
{
    MD5 md5;
    md5.addBytes(data + MD5_DIGESTSIZE, length - MD5_DIGESTSIZE);
    Vector<uint8_t, 16> digest;
    md5.checksum(digest);
    memcpy(data, digest.data(), MD5_DIGESTSIZE);
}

bool HTTPCache::writeFATFile()
{
    char *buf = 0, *buffer = 0;
    bool result = false;

    int totalSize = 0;
    int headerSize = 0;

    headerSize = sizeof(int) * 2 + MD5_DIGESTSIZE;
    totalSize = headerSize + m_totalResourceSize;

    WTF::TryMallocReturnValue rv = tryFastZeroedMalloc(totalSize);
    if (!rv.getValue(buf))
        return false;

    buffer = buf + MD5_DIGESTSIZE;
    (*(int*)buffer) = CACHEFAT_FORMAT_VERSION; buffer += sizeof(int);
    (*(int*)buffer) = m_fileNumber; buffer += sizeof(int);

    serializeFATData(buffer);

    writeDigest((unsigned char*)buf, totalSize);

    String fileFullPath = pathByAppendingComponent(m_filePath, m_fatFileName);

    void *fd = 0;
    int len, written = 0;
    fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_CACHE, fileFullPath.utf8().data(), "wb");
    if (!fd)
        goto error_end;

    buffer = buf;
    for (len = totalSize; len > 0;  len -= written) {
        written = wkcFileFWritePeer(buffer, sizeof(char), len, fd);
        if (written <= 0)
            break;
        buffer += written;
    }
    if (len > 0)
        goto error_end;

    result = true;
error_end:
    if (fd)
        wkcFileFClosePeer(fd);

    fastFree(buf);
    return result;
}

bool HTTPCache::verifyDigest(unsigned char *data, int length)
{
    if (length <= MD5_DIGESTSIZE)
        return false;

    bool result = false;
    MD5 md5;
    md5.addBytes(data + MD5_DIGESTSIZE, length - MD5_DIGESTSIZE);
    Vector<uint8_t, 16> digest;
    md5.checksum(digest);

    if (memcmp(data, digest.data(), MD5_DIGESTSIZE) == 0)
        result = true;

    return result;
}

bool HTTPCache::readFATFile()
{
    void *fd = 0;
    bool result = false, remove = false;
    char *buf = 0, *buffer = 0;
    int format_version = 0;
    int len = 0;

    String fileFullPath = pathByAppendingComponent(m_filePath, m_fatFileName);

    fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_CACHE, fileFullPath.utf8().data(), "rb");
    if (!fd)
        return false;

    struct stat st;
    if (wkcFileFStatPeer(fd, &st)) {
        W_DP(("FAT file read error: failed to stat"));
        wkcFileFClosePeer(fd);
        return false;
    }
    if (st.st_size <= MD5_DIGESTSIZE + sizeof(int)*2) {
        W_DP(("FAT file read error: size is too small"));
        wkcFileFClosePeer(fd);
        return false;
    }

    WTF::TryMallocReturnValue rv = tryFastZeroedMalloc(st.st_size);
    if (!rv.getValue(buf)) {
        W_DP(("FAT file read error: no memory"));
        goto error_end;
    }

    len = wkcFileFReadPeer(buf, sizeof(char), st.st_size, fd);
    wkcFileFClosePeer(fd);
    fd = 0;
    if (len<0 || len!=st.st_size) {
        W_DP(("FAT file read error: failed to read"));
        goto error_end;
    }

    buffer = buf;

    if (!verifyDigest((unsigned char*)buffer, len)) {
        W_DP(("FAT file read error: MD5 check"));
        remove = true;
        goto error_end;
    }

    buffer += MD5_DIGESTSIZE;

    format_version = (*(int*)buffer); buffer += sizeof(int);
    if (format_version != CACHEFAT_FORMAT_VERSION) {
        W_DP(("FAT file format is old : %d", format_version));
        remove = true;
        goto error_end;
    }

    m_fileNumber = (*(int*)buffer); buffer += sizeof(int);
    len -= sizeof(int) + sizeof(int) + MD5_DIGESTSIZE;

    if (len<=0) {
        W_DP(("FAT file read error: size is too small"));
        remove = true;
        goto error_end;
    }

    if (deserializeFATData(buffer, len))
        result = true;
    else
        remove = true;

error_end:
    if (fd)
        wkcFileFClosePeer(fd);

    if (remove)
        removeAll();

    fastFree(buf);
    return result;
}

} // namespace

#endif // ENABLE(WKC_HTTPCACHE)
