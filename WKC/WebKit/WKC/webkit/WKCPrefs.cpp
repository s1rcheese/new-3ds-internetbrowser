/*
 * WKCPrefs.cpp
 *
 * Copyright (c) 2011-2014 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "WKCPrefs.h"

#include "WKCWebFrame.h"
#include "FrameLoaderClientWKC.h"

#include "Settings.h"
#include "ResourceHandleManagerWKC.h"
#include "FontPlatformData.h"
#include "FrameView.h"
#include "MemoryCache.h"
#include "ApplicationCacheStorage.h"
#include "HTTPCacheWKC.h"
#include "ImageSource.h"
#include "SchemeRegistry.h"

#include "platform/ScrollView.h"

#include <wkc/wkcgpeer.h>
#include <wkc/wkcmediapeer.h>

#include "NotImplemented.h"

namespace WebCore {
extern void setScreenDPI(int horizontaldpi, int verticaldpi);
extern void setScreenSizeWKC(const WebCore::IntSize& size);
extern void setAvailableScreenSize(const WebCore::IntSize& size);
extern void setScreenDepth(int depth, int depth_per_component);
extern void setIsMonochrome(bool monochrome);
typedef void (*ResolveFilenameForDisplayProc)(const unsigned short* path, const int path_len, unsigned short* out_path, int* out_path_len, const int path_maxlen);
extern void RenderTheme_SetResolveFilenameForDisplayProc(WebCore::ResolveFilenameForDisplayProc proc);
extern void SimpleFontData_SetAverageFontGlyph(const unsigned int in_glyph);
extern void FontPlatformData_enableScalingMonosizeFont(bool flag);
}

namespace WKC {
namespace WKCPrefs {

static wkcSkin*
gSkin()
{
    DEFINE_STATIC_LOCAL(wkcSkin, skin, ());
    return &skin;
}

static wkcMediaSkins*
gMediaSkin()
{
    DEFINE_STATIC_LOCAL(wkcMediaSkins, skin, ());
    return &skin;
}

void
setCacheCapacities(unsigned int min_dead_resource, unsigned int max_dead_resource, unsigned int total)
{
    unsigned int min_dead = min_dead_resource;
    unsigned int max_dead = max_dead_resource;

    if (min_dead > total) {
        min_dead = total;
    }
    if (max_dead > total) {
        max_dead = total;
    }

    WebCore::memoryCache()->setCapacities(min_dead, max_dead, total);
}

void
setDeadDecodedDataDeletionInterval(double interval)
{
    WebCore::memoryCache()->setDeadDecodedDataDeletionInterval(interval);
}

void 
setMinDelayBeforeLiveDecodedPruneCaches(double delay)
{
    WebCore::MemoryCache::setMinDelayBeforeLiveDecodedPrune(delay);
}

void
setApplicationCacheDirectory(const char* directory)
{
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    WTF::String dir(directory);

    WebCore::cacheStorage().setCacheDirectory(dir);
#endif
}

void
setApplicationCacheMaximumSize(wkc_int64 size)
{
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    WebCore::cacheStorage().setMaximumSize(size);
#endif
}

void
setProxy(bool enable, const char* host, int port, bool isHTTP10, const char* proxyuser, const char* proxypass, const char* filters)
{
    WebCore::ResourceHandleManager::ProxyType proxy_type = WebCore::ResourceHandleManager::HTTP;

    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        if (enable) {
            const char* user = "";
            const char* pass = "";
            if (proxyuser) user = proxyuser;
            if (proxypass) pass = proxypass;
            if (isHTTP10) proxy_type = WebCore::ResourceHandleManager::HTTP10;
            mgr->setProxyInfo(host, port, proxy_type, user, pass, WTF::String::fromUTF8(filters));
        } else {
            mgr->setProxyInfo("", 0, proxy_type, "", "", "");
        }
    }
}

void setSSLProtocols(bool isEnableSSL2, bool isEnableSSL3, bool isEnableTLS10, bool isEnableTLS11, bool isEnableTLS12)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        unsigned int ver = 0;
        if (isEnableSSL2)   ver |= 1;
        if (isEnableSSL3)   ver |= (1 << 1);
        if (isEnableTLS10)  ver |= (1 << 2);
        if (isEnableTLS11)  ver |= (1 << 3);
        if (isEnableTLS12)  ver |= (1 << 4);
        mgr->SSLEnableProtocols(ver);
    }
}

void setSSLEnableOnlineCertChecks(bool isEnableOCSP, bool isEnableCRLDP)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->SSLEnableOnlineCertChecks(isEnableOCSP, isEnableCRLDP);
    }
}

void
setMaxHTTPConnections(long num)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setMaxHTTPConnections(num);
    }
}

void
setMaxWebSocketConnections(long num)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setMaxWebSocketConnections(num);
    }
}

void
setMaxCookieEntries(long number)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setMaxCookieEntries(number);
    }
}

void
setDNSCacheTimeout(int sec)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setDNSCacheTimeout(sec);
    }
}

void
setServerResponseTimeout(int sec)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setServerResponseTimeout(sec);
    }
}

void
setConnectTimeout(int sec)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setConnectTimeout(sec);
    }
}

void
setAcceptEncoding(const char* encodings)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setAcceptEncoding(encodings);
    }
}

void
setHarmfulSiteFilter(bool enable)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setHarmfulSiteFilter(enable);
    }
}

void
setDoNotTrack(bool enable)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setDoNotTrack(enable);
    }
}

void
setRedirectInWebKit(bool enable)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setRedirectInWKC(enable);
    }
}

void
setStackSize(unsigned int in_stack_size)
{
    void* thread;

    thread = wkcThreadCurrentThreadPeer();
    wkcThreadSetStackSizePeer(thread, in_stack_size);
}

void
setScreenDeviceParams(const ScreenDeviceParams& params)
{
    WebCore::setScreenSizeWKC(WebCore::IntSize(params.fScreenWidth, params.fScreenHeight));
    WebCore::setAvailableScreenSize(WebCore::IntSize(params.fAvailableScreenWidth, params.fAvailableScreenHeight));
    WebCore::setScreenDepth(params.fScreenDepth, params.fScreenDepthPerComponent);
    WebCore::setIsMonochrome(params.fIsMonochrome);
}

void
setSystemStrings(const WKC::SystemStrings* strings)
{
    wkcSystemSetNavigatorPlatformPeer(strings->fNavigatorPlatform);
    wkcSystemSetNavigatorProductPeer(strings->fNavigatorProduct);
    wkcSystemSetNavigatorProductSubPeer(strings->fNavigatorProductSub);
    wkcSystemSetNavigatorVendorPeer(strings->fNavigatorVendor);
    wkcSystemSetNavigatorVendorSubPeer(strings->fNavigatorVendorSub);
    wkcSystemSetLanguagePeer(strings->fLanguage);
    wkcSystemSetButtonLabelSubmitPeer(strings->fButtonLabelSubmit);
    wkcSystemSetButtonLabelResetPeer(strings->fButtonLabelReset);
    wkcSystemSetButtonLabelFilePeer(strings->fButtonLabelFile);
}

bool
setSystemString(const unsigned char* key, const unsigned char* text)
{
    if (!key || !text)
        return false;
    return wkcSystemSetSystemStringPeer(key, text);
}

void 
setThreadInfo(void* threadID, void* stackBase)
{
    wkcThreadSetMainThreadInfoPeer(threadID, stackBase);
}

void
setEncodingDetectorLanguageSet(int languageSetFlag)
{
    int flags = WKC_I18N_DETECT_ENCODING_NONE;

    if (languageSetFlag & WKC::EEncodingDetectorUniversal) {
        flags |= WKC_I18N_DETECT_ENCODING_UNIVERSAL;
    }
    if (languageSetFlag & WKC::EEncodingDetectorJapanese) {
        flags |= WKC_I18N_DETECT_ENCODING_JAPANESE;
    }
    if (languageSetFlag & WKC::EEncodingDetectorKorean) {
        flags |= WKC_I18N_DETECT_ENCODING_KOREAN;
    }
    if (languageSetFlag & WKC::EEncodingDetectorTraditionalChinese) {
        flags |= WKC_I18N_DETECT_ENCODING_TRADITIONAL_CHINESE;
    }
    if (languageSetFlag & WKC::EEncodingDetectorSimplifiedChinese) {
        flags |= WKC_I18N_DETECT_ENCODING_SIMPLIFIED_CHINESE;
    }
    if (languageSetFlag & WKC::EEncodingDetectorSBCS) {
        flags |= WKC_I18N_DETECT_ENCODING_SBCS;
    }
    wkcI18NSetDetectEncodingLanguageSetPeer(flags);
}

bool
setPrimaryFont(int fontID)
{
    return wkcFontEngineSetPrimaryFontPeer(fontID);
}

void
setEnableScalingMonosizeFont(bool flag)
{
    WebCore::FontPlatformData_enableScalingMonosizeFont(flag);
}

void 
setResolveFilenameForDisplayProc(WKC::ResolveFilenameForDisplayProc proc)
{
    WebCore::ResolveFilenameForDisplayProc iproc = (WebCore::ResolveFilenameForDisplayProc)proc;
    WebCore::RenderTheme_SetResolveFilenameForDisplayProc(iproc);
}

void
setGlyphForAverageWidth(const unsigned int glyph)
{
    WebCore::SimpleFontData_SetAverageFontGlyph(glyph);
}

void
registerSkin(const WKC::WKCSkin* skin)
{
    int i=0, j=0;

    if (gSkin()->fDefaultStyleSheet) {
        fastFree((void *)gSkin()->fDefaultStyleSheet);
        gSkin()->fDefaultStyleSheet = 0;
    }
    if (gSkin()->fQuirksStyleSheet) {
        fastFree((void *)gSkin()->fQuirksStyleSheet);
        gSkin()->fQuirksStyleSheet = 0;
    }
    for (i=0; i<WKC::ESystemFontTypes; i++) {
        if (gSkin()->fSystemFontFamilyName[i]) {
            fastFree((void *)gSkin()->fSystemFontFamilyName[i]);
            gSkin()->fSystemFontFamilyName[i] = 0;
        }
    }
    if (gSkin()->fResourceImages) {
        i=0;
        while (gSkin()->fResourceImages[i]) {
            fastFree((void *)gSkin()->fResourceImages[i]);
            i++;
        }
        fastFree((void *)gSkin()->fResourceImages);
        gSkin()->fResourceImages = 0;
    }

    if (!skin) {
        wkcStockImageRegisterSkinPeer((const wkcSkin *)0);
        return;
    }

    // for safety: WKC::WKCSkin and wkcSkin would be same structure,
    // but it should be safe transfer each members one by one...
    for (i=0; i<WKC::ESkinImages; i++) {
        gSkin()->fImages[i].fBitmap = skin->fImages[i].fBitmap;
        gSkin()->fImages[i].fSize.fWidth = skin->fImages[i].fSize.fWidth;
        gSkin()->fImages[i].fSize.fHeight = skin->fImages[i].fSize.fHeight;
        for (j=0; j<4; j++) {
            gSkin()->fImages[i].fPoints[j] = skin->fImages[i].fPoints[j];
        }
    }
    for (i=0; i<WKC::ESkinColors; i++) {
        gSkin()->fColors[i] = skin->fColors[i];
    }
    for (i=0; i<WKC::ESystemFontTypes; i++) {
        gSkin()->fSystemFontSize[i] = skin->fSystemFontSize[i];
        if (skin->fSystemFontFamilyName[i] && skin->fSystemFontFamilyName[i][0]) {
            gSkin()->fSystemFontFamilyName[i] = fastStrDup(skin->fSystemFontFamilyName[i]);
        }
    }
    if (skin->fDefaultStyleSheet && skin->fDefaultStyleSheet[0]) {
        gSkin()->fDefaultStyleSheet = fastStrDup(skin->fDefaultStyleSheet);
    }
    if (skin->fQuirksStyleSheet && skin->fQuirksStyleSheet[0]) {
        gSkin()->fQuirksStyleSheet = fastStrDup(skin->fQuirksStyleSheet);
    }
    if (skin->fResourceImages) {
        i=0;
        while (skin->fResourceImages[i++]);
        gSkin()->fResourceImages = (wkcSkinResourceImage **)fastMalloc(sizeof(wkcSkinResourceImage*)*(i+1));
        i=0;
        while (skin->fResourceImages[i]) {
            gSkin()->fResourceImages[i] = (wkcSkinResourceImage *)fastMalloc(sizeof(wkcSkinResourceImage));
            gSkin()->fResourceImages[i]->fName = skin->fResourceImages[i]->fName;
            gSkin()->fResourceImages[i]->fSize.fWidth = skin->fResourceImages[i]->fSize.fWidth;
            gSkin()->fResourceImages[i]->fSize.fHeight = skin->fResourceImages[i]->fSize.fHeight;
            gSkin()->fResourceImages[i]->fBitmap = skin->fResourceImages[i]->fBitmap;
            i++;
        }
        gSkin()->fResourceImages[i] = 0;
    }
    wkcStockImageRegisterSkinPeer(gSkin());
}

void
registerMediaSkin(const WKC::WKCMediaSkin* skin)
{
    int i=0;

    if (gMediaSkin()->fStyleSheet) {
        fastFree((void *)gMediaSkin()->fStyleSheet);
        gMediaSkin()->fStyleSheet = 0;
    }

    if (!skin) {
        wkcMediaPlayerSkinRegisterSkinsPeer((const wkcMediaSkins *)0);
        return;
    }

    for (i=0; i<WKC::EMediaSkinImages; i++) {
        gMediaSkin()->fSkins[i].fSize.fWidth = skin->fImages[i].fSize.fWidth;
        gMediaSkin()->fSkins[i].fSize.fHeight = skin->fImages[i].fSize.fHeight;
        for (int j=0; j<4; j++) {
            gMediaSkin()->fSkins[i].fImages[j].fBitmap = skin->fImages[i].fImages[j].fBitmap;
            for (int k=0; k<4; k++) {
                gMediaSkin()->fSkins[i].fImages[j].fPoints[k] = skin->fImages[i].fImages[j].fPoints[k];
            }
        }
    }
    if (skin->fStyleSheet && skin->fStyleSheet[0]) {
        gMediaSkin()->fStyleSheet = wkc_strdup(skin->fStyleSheet);
    }
    wkcMediaPlayerSkinRegisterSkinsPeer(gMediaSkin());
}

void
setHTTPCache(bool enable, long long limitTotalSize, long limitContentSize, int limitEntries, const char *filePath)
{
#if ENABLE(WKC_HTTPCACHE)
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->httpCache()->setDisabled(!enable);
        mgr->httpCache()->setFilePath(filePath);
        mgr->httpCache()->setMaxTotalCacheSize(limitTotalSize);
        mgr->httpCache()->setMaxContentSize(limitContentSize);
        mgr->httpCache()->setMaxContentEntries(limitEntries);
    }
#else
    (void)enable;
    (void)limitTotalSize;
    (void)limitContentSize;
    (void)limitEntries;
    (void)filePath;
#endif
}

void
setDecodeAfterDownloading(bool decodeAfterLoading)
{
    WebCore::ImageSource::setDecodeAfterDownloading(decodeAfterLoading);
}

void
registerURLSchemeAsNoAccess(const char* scheme)
{
    if (!scheme)
        return;
    WebCore::SchemeRegistry::registerURLSchemeAsNoAccess(WTF::String(scheme));
}

void
forceTerminate()
{
    wkcStockImageRegisterSkinPeer(0);
    wkcMediaPlayerSkinRegisterSkinsPeer(0);
}

void
initialize()
{
    gMediaSkin()->fStyleSheet = 0;
    gSkin()->fDefaultStyleSheet = 0;
    gSkin()->fQuirksStyleSheet = 0;
    for (int i=0; i<WKC::ESystemFontTypes; i++) {
        gSkin()->fSystemFontFamilyName[i] = 0;
    }
}

void
finalize()
{
    if (gMediaSkin()->fStyleSheet) {
        fastFree((void *)gMediaSkin()->fStyleSheet);
    }
    if (gSkin()->fDefaultStyleSheet) {
        fastFree((void *)gSkin()->fDefaultStyleSheet);
    }
    if (gSkin()->fQuirksStyleSheet) {
        fastFree((void *)gSkin()->fQuirksStyleSheet);
    }
    for (int i=0; i<WKC::ESystemFontTypes; i++) {
        if (gSkin()->fSystemFontFamilyName[i]) {
            fastFree((void *)gSkin()->fSystemFontFamilyName[i]);
        }
    }

    if (gSkin()->fResourceImages) {
        int i=0;
        while (gSkin()->fResourceImages[i]) {
            fastFree((void *)gSkin()->fResourceImages[i]);
            i++;
        }
        fastFree((void *)gSkin()->fResourceImages);
        gSkin()->fResourceImages = 0;
    }
}

} // namespace
} // namespace
