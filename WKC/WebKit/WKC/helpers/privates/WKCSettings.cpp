/*
 *  WKCSettings.cpp
 *
 *  Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
#include "helpers/WKCSettings.h"
#include "WKCWebViewPrivate.h"

#include "CString.h"
#include "Page.h"
#include "Settings.h"

#define PARENT() (((WebCore::Settings *)m_private))

namespace WKC {

WKCSettings::WKCSettings(WKC::WKCWebViewPrivate* parent)
    : m_private((WKCSettingsPrivate *)parent->core()->settings())
{
}

WKCSettings::~WKCSettings()
{
}

#if ENABLE(WKC_ANDROID_LAYOUT)

WKCSettings::LayoutAlgorithm WKCSettings::layoutAlgorithm() const
{
    return (WKCSettings::LayoutAlgorithm)PARENT()->layoutAlgorithm();
}

void WKCSettings::setLayoutAlgorithm(WKCSettings::LayoutAlgorithm algorithm)
{
    PARENT()->setLayoutAlgorithm((WebCore::Settings::LayoutAlgorithm)algorithm);
}
#endif //ENABLE(WKC_ANDROID_LAYOUT)

void WKCSettings::setStandardFontFamily(const char* arg)
{
    PARENT()->setStandardFontFamily(WTF::String::fromUTF8(arg));
}

const char* WKCSettings::standardFontFamily()
{
    return PARENT()->standardFontFamily().string().utf8().data();
}


void WKCSettings::setFixedFontFamily(const char* arg)
{
    PARENT()->setFixedFontFamily(WTF::String::fromUTF8(arg));
}

const char* WKCSettings::fixedFontFamily() const
{
    return PARENT()->fixedFontFamily().string().utf8().data();
}


void WKCSettings::setSerifFontFamily(const char* arg)
{
    PARENT()->setSerifFontFamily(WTF::String::fromUTF8(arg));
}

const char* WKCSettings::serifFontFamily()
{
    return PARENT()->serifFontFamily().string().utf8().data();
}


void WKCSettings::setSansSerifFontFamily(const char* arg)
{
    PARENT()->setSansSerifFontFamily(WTF::String::fromUTF8(arg));
}

const char* WKCSettings::sansSerifFontFamily()
{
    return PARENT()->sansSerifFontFamily().string().utf8().data();
}


void WKCSettings::setCursiveFontFamily(const char* arg)
{
    PARENT()->setCursiveFontFamily(WTF::String::fromUTF8(arg));
}

const char* WKCSettings::cursiveFontFamily() const
{
    return PARENT()->cursiveFontFamily().string().utf8().data();
}


void WKCSettings::setFantasyFontFamily(const char* arg)
{
    PARENT()->setFantasyFontFamily(WTF::String::fromUTF8(arg));
}

const char* WKCSettings::fantasyFontFamily() const
{
    return PARENT()->fantasyFontFamily().string().utf8().data();
}


void WKCSettings::setMinimumFontSize(int arg)
{
    PARENT()->setMinimumFontSize(arg);
}

int WKCSettings::minimumFontSize() const
{
    return PARENT()->minimumFontSize();
}


void WKCSettings::setMinimumLogicalFontSize(int arg)
{
    PARENT()->setMinimumLogicalFontSize(arg);
}

int WKCSettings::minimumLogicalFontSize() const
{
    return PARENT()->minimumLogicalFontSize();
}


void WKCSettings::setDefaultFontSize(int arg)
{
    PARENT()->setDefaultFontSize(arg);
}

int WKCSettings::defaultFontSize() const
{
    return PARENT()->defaultFontSize();
}


void WKCSettings::setDefaultFixedFontSize(int arg)
{
    PARENT()->setDefaultFixedFontSize(arg);
}

int WKCSettings::defaultFixedFontSize() const
{
    return PARENT()->defaultFixedFontSize();
}


void WKCSettings::setLoadsImagesAutomatically(bool arg)
{
    PARENT()->setLoadsImagesAutomatically(arg);
}

bool WKCSettings::loadsImagesAutomatically() const
{
    return PARENT()->loadsImagesAutomatically();
}

void WKCSettings::setScriptEnabled(bool arg)
{
    PARENT()->setScriptEnabled(arg);
}

bool WKCSettings::isScriptEnabled() const
{
    return PARENT()->isScriptEnabled();
}

void WKCSettings::setWebSecurityEnabled(bool arg)
{
    PARENT()->setWebSecurityEnabled(arg);
}

bool WKCSettings::isWebSecurityEnabled() const
{
    return PARENT()->isWebSecurityEnabled();
}


void WKCSettings::setAllowUniversalAccessFromFileURLs(bool arg)
{
    PARENT()->setAllowUniversalAccessFromFileURLs(arg);
}

bool WKCSettings::allowUniversalAccessFromFileURLs() const
{
    return PARENT()->allowUniversalAccessFromFileURLs();
}


void WKCSettings::setJavaScriptCanOpenWindowsAutomatically(bool arg)
{
    PARENT()->setJavaScriptCanOpenWindowsAutomatically(arg);
}

bool WKCSettings::javaScriptCanOpenWindowsAutomatically() const
{
    return PARENT()->javaScriptCanOpenWindowsAutomatically();
}


#if 1
// added at webkit.org trunk r58703 (modification is changed for this platform)
void WKCSettings::setJavaScriptCanAccessClipboard(bool arg)
{
    PARENT()->setJavaScriptCanAccessClipboard(arg);
}

bool WKCSettings::javaScriptCanAccessClipboard() const
{
    return PARENT()->javaScriptCanAccessClipboard();
}
#endif


void WKCSettings::setSpatialNavigationEnabled(bool arg)
{
    PARENT()->setSpatialNavigationEnabled(arg);
}

bool WKCSettings::isSpatialNavigationEnabled() const
{
    return PARENT()->isSpatialNavigationEnabled();
}


void WKCSettings::setJavaEnabled(bool arg)
{
    PARENT()->setJavaEnabled(arg);
}

bool WKCSettings::isJavaEnabled() const
{
    return PARENT()->isJavaEnabled();
}


void WKCSettings::setPluginsEnabled(bool arg)
{
    PARENT()->setPluginsEnabled(arg);
}

bool WKCSettings::arePluginsEnabled() const
{
    return PARENT()->arePluginsEnabled();
}

void WKCSettings::setLocalStorageEnabled(bool arg)
{
    PARENT()->setLocalStorageEnabled(arg);
}

bool WKCSettings::localStorageEnabled() const
{
    return PARENT()->localStorageEnabled();
}

void WKCSettings::setSessionStorageQuota(unsigned arg)
{
    PARENT()->setSessionStorageQuota(arg);
}

unsigned WKCSettings::sessionStorageQuota() const
{
    return PARENT()->sessionStorageQuota();
}

void WKCSettings::setPrivateBrowsingEnabled(bool arg)
{
    PARENT()->setPrivateBrowsingEnabled(arg);
}

bool WKCSettings::privateBrowsingEnabled() const
{
    return PARENT()->privateBrowsingEnabled();
}


void WKCSettings::setCaretBrowsingEnabled(bool arg)
{
    PARENT()->setCaretBrowsingEnabled(arg);
}

bool WKCSettings::caretBrowsingEnabled() const
{
    return PARENT()->caretBrowsingEnabled();
}


void WKCSettings::setDefaultTextEncodingName(const char* arg)
{
    PARENT()->setDefaultTextEncodingName(arg);
}

const char* WKCSettings::defaultTextEncodingName()
{
    return PARENT()->defaultTextEncodingName().utf8().data();
}


void WKCSettings::setUsesEncodingDetector(bool arg)
{
    PARENT()->setUsesEncodingDetector(arg);
}

bool WKCSettings::usesEncodingDetector() const
{
    return PARENT()->usesEncodingDetector();
}


void WKCSettings::setUserStyleSheetLocation(const char* arg)
{
    WebCore::KURL location(WebCore::KURL(),arg);
    PARENT()->setUserStyleSheetLocation(location);
}

const char* WKCSettings::userStyleSheetLocation() const
{
    return PARENT()->userStyleSheetLocation().string().utf8().data();
}


void WKCSettings::setShouldPrintBackgrounds(bool arg)
{
    PARENT()->setShouldPrintBackgrounds(arg);
}

bool WKCSettings::shouldPrintBackgrounds() const
{
    return PARENT()->shouldPrintBackgrounds();
}


void WKCSettings::setTextAreasAreResizable(bool arg)
{
    PARENT()->setTextAreasAreResizable(arg);
}

bool WKCSettings::textAreasAreResizable() const
{
    return PARENT()->textAreasAreResizable();
}


void WKCSettings::setEditableLinkBehavior(WKCSettings::EditableLinkBehavior arg)
{
    PARENT()->setEditableLinkBehavior((WebCore::EditableLinkBehavior)arg);
}

WKCSettings::EditableLinkBehavior WKCSettings::editableLinkBehavior()
{
    return (WKCSettings::EditableLinkBehavior)PARENT()->editableLinkBehavior();
}


void WKCSettings::setTextDirectionSubmenuInclusionBehavior(WKCSettings::TextDirectionSubmenuInclusionBehavior arg)
{
    PARENT()->setTextDirectionSubmenuInclusionBehavior((WebCore::TextDirectionSubmenuInclusionBehavior)arg);
}

WKCSettings::TextDirectionSubmenuInclusionBehavior WKCSettings::textDirectionSubmenuInclusionBehavior() const
{
    return (WKCSettings::TextDirectionSubmenuInclusionBehavior)PARENT()->textDirectionSubmenuInclusionBehavior();
}


void WKCSettings::setNeedsAdobeFrameReloadingQuirk(bool arg)
{
    PARENT()->setNeedsAdobeFrameReloadingQuirk(arg);
}

bool WKCSettings::needsAcrobatFrameReloadingQuirk() const
{
    return PARENT()->needsAcrobatFrameReloadingQuirk();
}


void WKCSettings::setNeedsKeyboardEventDisambiguationQuirks(bool arg)
{
    PARENT()->setNeedsKeyboardEventDisambiguationQuirks(arg);
}

bool WKCSettings::needsKeyboardEventDisambiguationQuirks() const
{
    return PARENT()->needsKeyboardEventDisambiguationQuirks();
}


void WKCSettings::setTreatsAnyTextCSSLinkAsStylesheet(bool arg)
{
    PARENT()->setTreatsAnyTextCSSLinkAsStylesheet(arg);
}

bool WKCSettings::treatsAnyTextCSSLinkAsStylesheet() const
{
    return PARENT()->treatsAnyTextCSSLinkAsStylesheet();
}


void WKCSettings::setNeedsLeopardMailQuirks(bool arg)
{
    PARENT()->setNeedsLeopardMailQuirks(arg);
}

bool WKCSettings::needsLeopardMailQuirks() const
{
    return PARENT()->needsLeopardMailQuirks();
}


void WKCSettings::setDOMPasteAllowed(bool arg)
{
    PARENT()->setDOMPasteAllowed(arg);
}

bool WKCSettings::isDOMPasteAllowed() const
{
    return PARENT()->isDOMPasteAllowed();
}


void WKCSettings::setUsesPageCache(bool arg)
{
    PARENT()->setUsesPageCache(arg);
}

bool WKCSettings::usesPageCache() const
{
    return PARENT()->usesPageCache();
}


void WKCSettings::setShrinksStandaloneImagesToFit(bool arg)
{
    PARENT()->setShrinksStandaloneImagesToFit(arg);
}

bool WKCSettings::shrinksStandaloneImagesToFit() const
{
    return PARENT()->shrinksStandaloneImagesToFit();
}


void WKCSettings::setShowsURLsInToolTips(bool arg)
{
    PARENT()->setShowsURLsInToolTips(arg);
}

bool WKCSettings::showsURLsInToolTips() const
{
    return PARENT()->showsURLsInToolTips();
}


void WKCSettings::setFTPDirectoryTemplatePath(const char* arg)
{
    PARENT()->setFTPDirectoryTemplatePath(arg);
}

const char* WKCSettings::ftpDirectoryTemplatePath()
{
    return PARENT()->ftpDirectoryTemplatePath().utf8().data();
}


void WKCSettings::setForceFTPDirectoryListings(bool arg)
{
    PARENT()->setForceFTPDirectoryListings(arg);
}

bool WKCSettings::forceFTPDirectoryListings() const
{
    return PARENT()->forceFTPDirectoryListings();
}


void WKCSettings::setDeveloperExtrasEnabled(bool arg)
{
    PARENT()->setDeveloperExtrasEnabled(arg);
}

bool WKCSettings::developerExtrasEnabled() const
{
    return PARENT()->developerExtrasEnabled();
}

void WKCSettings::setFrameFlatteningEnabled(bool frameFlatteningEnabled)
{
    PARENT()->setFrameFlatteningEnabled(frameFlatteningEnabled);
}

bool WKCSettings::frameFlatteningEnabled() const
{
    return PARENT()->frameFlatteningEnabled();
}

void WKCSettings::setAuthorAndUserStylesEnabled(bool arg)
{
    PARENT()->setAuthorAndUserStylesEnabled(arg);
}

bool WKCSettings::authorAndUserStylesEnabled() const
{
    return PARENT()->authorAndUserStylesEnabled();
}


void WKCSettings::setFontRenderingMode(FontRenderingMode mode)
{
    PARENT()->setFontRenderingMode((WebCore::FontRenderingMode)mode);
}

WKCSettings::FontRenderingMode WKCSettings::fontRenderingMode() const
{
    return (WKCSettings::FontRenderingMode)PARENT()->fontRenderingMode();
}


void WKCSettings::setNeedsSiteSpecificQuirks(bool arg)
{
    PARENT()->setNeedsSiteSpecificQuirks(arg);
}

bool WKCSettings::needsSiteSpecificQuirks() const
{
    return PARENT()->needsSiteSpecificQuirks();
}


void WKCSettings::setWebArchiveDebugModeEnabled(bool arg)
{
#if ENABLE(WEB_ARCHIVE)
    PARENT()->setWebArchiveDebugModeEnabled(arg);
#endif
}

bool WKCSettings::webArchiveDebugModeEnabled() const
{
#if ENABLE(WEB_ARCHIVE)
    return PARENT()->webArchiveDebugModeEnabled();
#else
    return false;
#endif
}


void WKCSettings::setLocalFileContentSniffingEnabled(bool arg)
{
    PARENT()->setLocalFileContentSniffingEnabled(arg);
}

bool WKCSettings::localFileContentSniffingEnabled() const
{
    return PARENT()->localFileContentSniffingEnabled();
}


void WKCSettings::setLocalStorageDatabasePath(const char* arg)
{
    PARENT()->setLocalStorageDatabasePath(arg);
}

const char* WKCSettings::localStorageDatabasePath() const
{
    return PARENT()->localStorageDatabasePath().utf8().data();
}


void WKCSettings::setApplicationChromeMode(bool arg)
{
    PARENT()->setApplicationChromeMode(arg);
}

bool WKCSettings::inApplicationChromeMode() const
{
    return PARENT()->inApplicationChromeMode();
}


void WKCSettings::setOfflineWebApplicationCacheEnabled(bool arg)
{
    PARENT()->setOfflineWebApplicationCacheEnabled(arg);
}

bool WKCSettings::offlineWebApplicationCacheEnabled() const
{
    return PARENT()->offlineWebApplicationCacheEnabled();
}

void WKCSettings::setMockScrollbarsEnabled(bool arg)
{
    PARENT()->setMockScrollbarsEnabled(arg);
}

bool WKCSettings::mockScrollbarsEnabled() const
{
    return PARENT()->mockScrollbarsEnabled();
}

void WKCSettings::setEnforceCSSMIMETypeInNoQuirksMode(bool arg)
{
    PARENT()->setEnforceCSSMIMETypeInNoQuirksMode(arg);
}

bool WKCSettings::enforceCSSMIMETypeInNoQuirksMode()
{
    return PARENT()->enforceCSSMIMETypeInNoQuirksMode();
}


void WKCSettings::setMaximumDecodedImageSize(size_t size)
{
    PARENT()->setMaximumDecodedImageSize(size);
}

size_t WKCSettings::maximumDecodedImageSize() const
{
    return PARENT()->maximumDecodedImageSize();
}


void WKCSettings::setAllowScriptsToCloseWindows(bool arg)
{
    PARENT()->setAllowScriptsToCloseWindows(arg);
}

bool WKCSettings::allowScriptsToCloseWindows() const
{
    return PARENT()->allowScriptsToCloseWindows();
}


void WKCSettings::setEditingBehaviorType(EditingBehaviorType behavior)
{
    PARENT()->setEditingBehaviorType((WebCore::EditingBehaviorType)behavior);
}

WKCSettings::EditingBehaviorType WKCSettings::editingBehaviorType() const
{
    return (WKCSettings::EditingBehaviorType)PARENT()->editingBehaviorType();
}


void WKCSettings::setDownloadableBinaryFontsEnabled(bool arg)
{
    PARENT()->setDownloadableBinaryFontsEnabled(arg);
}

bool WKCSettings::downloadableBinaryFontsEnabled() const
{
    return PARENT()->downloadableBinaryFontsEnabled();
}


void WKCSettings::setXSSAuditorEnabled(bool arg)
{
    PARENT()->setXSSAuditorEnabled(arg);
}

bool WKCSettings::xssAuditorEnabled() const
{
    return PARENT()->xssAuditorEnabled();
}

void WKCSettings::setCanvasUsesAcceleratedDrawing(bool arg)
{
    PARENT()->setCanvasUsesAcceleratedDrawing(arg);
}

bool WKCSettings::canvasUsesAcceleratedDrawing() const
{
    return PARENT()->canvasUsesAcceleratedDrawing();
}

void WKCSettings::setAcceleratedDrawingEnabled(bool arg)
{
    PARENT()->setAcceleratedDrawingEnabled(arg);
}

bool WKCSettings::acceleratedDrawingEnabled() const
{
    return PARENT()->acceleratedDrawingEnabled();
}

void WKCSettings::setAcceleratedCompositingEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingEnabled(arg);
}

bool WKCSettings::acceleratedCompositingEnabled() const
{
    return PARENT()->acceleratedCompositingEnabled();
}

void WKCSettings::setAcceleratedCompositingFor3DTransformsEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingFor3DTransformsEnabled(arg);
}

bool WKCSettings::acceleratedCompositingFor3DTransformsEnabled() const
{
    return PARENT()->acceleratedCompositingFor3DTransformsEnabled();
}

void WKCSettings::setAcceleratedCompositingForVideoEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingForVideoEnabled(arg);
}

bool WKCSettings::acceleratedCompositingForVideoEnabled() const
{
    return PARENT()->acceleratedCompositingForVideoEnabled();
}

void WKCSettings::setAcceleratedCompositingForPluginsEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingForPluginsEnabled(arg);
}

bool WKCSettings::acceleratedCompositingForPluginsEnabled() const
{
    return PARENT()->acceleratedCompositingForPluginsEnabled();
}

void WKCSettings::setAcceleratedCompositingForCanvasEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingForCanvasEnabled(arg);
}

bool WKCSettings::acceleratedCompositingForCanvasEnabled() const
{
    return PARENT()->acceleratedCompositingForCanvasEnabled();
}

void WKCSettings::setAcceleratedCompositingForAnimationEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingForAnimationEnabled(arg);
}

bool WKCSettings::acceleratedCompositingForAnimationEnabled() const
{
    return PARENT()->acceleratedCompositingForAnimationEnabled();
}

void WKCSettings::setAcceleratedCompositingForFixedPositionEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingForFixedPositionEnabled(arg);
}

bool WKCSettings::acceleratedCompositingForFixedPositionEnabled() const
{
    return PARENT()->acceleratedCompositingForFixedPositionEnabled();
}

void WKCSettings::setAcceleratedCompositingForScrollableFramesEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingForScrollableFramesEnabled(arg);
}

bool WKCSettings::acceleratedCompositingForScrollableFramesEnabled() const
{
    return PARENT()->acceleratedCompositingForScrollableFramesEnabled();
}

void WKCSettings::setShowDebugBorders(bool arg)
{
    PARENT()->setShowDebugBorders(arg);
}

bool WKCSettings::showDebugBorders() const
{
    return PARENT()->showDebugBorders();
}


void WKCSettings::setShowRepaintCounter(bool arg)
{
    PARENT()->setShowRepaintCounter(arg);
}

bool WKCSettings::showRepaintCounter() const
{
    return PARENT()->showRepaintCounter();
}


void WKCSettings::setExperimentalNotificationsEnabled(bool arg)
{
    PARENT()->setExperimentalNotificationsEnabled(arg);
}

bool WKCSettings::experimentalNotificationsEnabled() const
{
    return PARENT()->experimentalNotificationsEnabled();
}

void WKCSettings::setWebAudioEnabled(bool arg)
{
    PARENT()->setWebAudioEnabled(arg);
}

bool WKCSettings::webAudioEnabled() const
{
    return PARENT()->webAudioEnabled();
}

void WKCSettings::setWebGLEnabled(bool arg)
{
    PARENT()->setWebGLEnabled(arg);
}

bool WKCSettings::webGLEnabled() const
{
    return PARENT()->webGLEnabled();
}

void WKCSettings::setOpenGLMultisamplingEnabled(bool arg)
{
    PARENT()->setOpenGLMultisamplingEnabled(arg);
}
bool WKCSettings::openGLMultisamplingEnabled() const
{
    return PARENT()->openGLMultisamplingEnabled();
}

void WKCSettings::setAccelerated2dCanvasEnabled(bool arg)
{
    PARENT()->setAccelerated2dCanvasEnabled(arg);
}
bool WKCSettings::accelerated2dCanvasEnabled() const
{
    return PARENT()->accelerated2dCanvasEnabled();
}

void WKCSettings::setDeferred2dCanvasEnabled(bool arg)
{
    PARENT()->setDeferred2dCanvasEnabled(arg);
}
bool WKCSettings::deferred2dCanvasEnabled() const
{
    return PARENT()->deferred2dCanvasEnabled();
}

void WKCSettings::setFullScreenEnabled(bool arg)
{
#if ENABLE(FULLSCREEN_API)
    PARENT()->setFullScreenEnabled(arg);
#endif
}
bool WKCSettings::fullScreenEnabled() const
{
#if ENABLE(FULLSCREEN_API)
    return PARENT()->fullScreenEnabled();
#else
    return false;
#endif
}

void WKCSettings::setForceCompositingMode(bool arg)
{
    PARENT()->setForceCompositingMode(arg);
}
bool WKCSettings::forceCompositingMode()
{
    return PARENT()->forceCompositingMode();
}

void WKCSettings::setUseHixie76WebSocketProtocol(bool arg)
{
#if ENABLE(WEB_SOCKETS)
    PARENT()->setUseHixie76WebSocketProtocol(arg);
#endif
}

bool WKCSettings::useHixie76WebSocketProtocol()
{
#if ENABLE(WEB_SOCKETS)
    return PARENT()->useHixie76WebSocketProtocol();
#else
    return false;
#endif
}

#if ENABLE(VIDEO_TRACK)
void WKCSettings::setShouldDisplaySubtitles(bool flag)
{
    PARENT()->setShouldDisplaySubtitles(flag);
}

bool WKCSettings::shouldDisplaySubtitles() const
{
    return PARENT()->shouldDisplaySubtitles();
}

void WKCSettings::setShouldDisplayCaptions(bool flag)
{
    PARENT()->setShouldDisplayCaptions(flag);
}

bool WKCSettings::shouldDisplayCaptions() const
{
    return PARENT()->shouldDisplayCaptions();
}

void WKCSettings::setShouldDisplayTextDescriptions(bool flag)
{
    PARENT()->setShouldDisplayTextDescriptions(flag);
}

bool WKCSettings::shouldDisplayTextDescriptions() const
{
    return PARENT()->shouldDisplayTextDescriptions();
}

#else
void WKCSettings::setShouldDisplaySubtitles(bool) {}
bool WKCSettings::shouldDisplaySubtitles() const { return false; }
void WKCSettings::setShouldDisplayCaptions(bool) {}
bool WKCSettings::shouldDisplayCaptions() const { return false; }
void WKCSettings::setShouldDisplayTextDescriptions(bool) {}
bool WKCSettings::shouldDisplayTextDescriptions() const { return false; }
#endif

void WKCSettings::setInteractiveFormValidationEnabled(bool flag)
{
    PARENT()->setInteractiveFormValidationEnabled(flag);
}

bool WKCSettings::interactiveFormValidationEnabled() const
{
    return PARENT()->interactiveFormValidationEnabled();
}

}

namespace WKC {

WKCGlobalSettings::WKCGlobalSettings()
{
}

WKCGlobalSettings::~WKCGlobalSettings()
{
}

WKCGlobalSettings* WKCGlobalSettings::create()
{
    WKCGlobalSettings *self;

    self = new WKCGlobalSettings();
    if (!self)
        return 0;

    if (!self->construct()) {
        delete self;
        return 0;
    }

    return self;
}

bool WKCGlobalSettings::construct()
{
    return true;
}

WKC_DEFINE_GLOBAL_PTR(WKCGlobalSettings*, gSharedInstance, 0);
WKC_DEFINE_GLOBAL_BOOL(gAutoFlag, false);

bool WKCGlobalSettings::createSharedInstance(bool autoflag)
{
    if (gSharedInstance)
        return true;

    gSharedInstance = create();
    if (!gSharedInstance)
        return false;

    gAutoFlag = autoflag;

    return true;
}

void WKCGlobalSettings::deleteSharedInstance()
{
    if (gSharedInstance)
        delete gSharedInstance;

    gSharedInstance = 0;
}

bool WKCGlobalSettings::isExistSharedInstance()
{
    return gSharedInstance ? true : false;
}

bool WKCGlobalSettings::isAutomatic()
{
    return gAutoFlag;
}

}   // namespace
