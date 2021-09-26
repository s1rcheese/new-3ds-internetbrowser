/*
    WKCWebView.cpp
    Copyright (C) 2007, 2008 Holger Hans Peter Freyther
    Copyright (C) 2007, 2008, 2009 Christian Dywan <christian@imendio.com>
    Copyright (C) 2007 Xan Lopez <xan@gnome.org>
    Copyright (C) 2007, 2008 Alp Toker <alp@atoker.com>
    Copyright (C) 2008 Jan Alonzo <jmalonzo@unpluggable.com>
    Copyright (C) 2008 Gustavo Noronha Silva <gns@gnome.org>
    Copyright (C) 2008 Nuanti Ltd.
    Copyright (C) 2008, 2009 Collabora Ltd.
    Copyright (C) 2009 Igalia S.L.
    Copyright (C) 2009 Movial Creative Technologies Inc.
    Copyright (C) 2009 Bobby Powers
    Copyright (c) 2010-2015 ACCESS CO., LTD. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"

#include "WKCWebView.h"
#include "WKCWebViewPrivate.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"
#include "WKCPlatformEvents.h"
#include "WKCClientBuilders.h"
#include "WKCWebViewPrefs.h"
#include "WKCPrefs.h"
#include "WKCMemoryInfo.h"
#include "WKCOverlayPrivate.h"

#include "BackForwardClientWKC.h"
#include "ChromeClientWKC.h"
#if ENABLE(CONTEXT_MENUS)
#include "ContextMenuClientWKC.h"
#endif
#include "EditorClientWKC.h"
#include "DragClientWKC.h"
#include "FrameLoaderClientWKC.h"
#include "InspectorClientWKC.h"
#include "DropDownListClientWKC.h"
#if ENABLE(GEOLOCATION)
#include "GeolocationClientWKC.h"
#endif
#if ENABLE(INPUT_SPEECH)
#include "SpeechInputClientWKC.h"
#endif
#if ENABLE(INSPECTOR_SERVER)
#include "WebInspectorServer.h"
#include "InspectorController.h"
#include "InspectorServerClientWKC.h"
#endif
#if ENABLE(INSPECTOR)
#include "InspectorController.h"
#endif
#if ENABLE(DEVICE_ORIENTATION)
#include "DeviceMotionClientWKC.h"
#include "DeviceOrientationClientWKC.h"
#endif
#if ENABLE(MEDIA_STREAM)
#include "UserMediaClientImplWKC.h"
#endif
#if ENABLE(BATTERY_STATUS)
#include "BatteryClientWKC.h"
#endif

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>
#include <wkc/wkcmpeer.h>
#include <wkc/wkcmediapeer.h>
#include <wkc/wkcpluginpeer.h>
#include <wkc/wkcglpeer.h>
#include <wkc/wkcsocket.h>

#include "helpers/WKCHistoryItem.h"
#include "helpers/WKCNode.h"
#include "helpers/WKCSettings.h"
#include "helpers/WKCEditor.h"
#include "helpers/privates/WKCHistoryItemPrivate.h"
#include "helpers/privates/WKCResourceHandlePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCPagePrivate.h"
#include "helpers/privates/WKCEventHandlerPrivate.h"
#include "helpers/privates/WKCFocusControllerPrivate.h"
#include "helpers/privates/WKCPluginDatabasePrivate.h"
#include "helpers/privates/WKCHitTestResultPrivate.h"
#include "helpers/privates/WKCSecurityOriginPrivate.h"

#include "Chrome.h"
#include "Cookie.h"
#include "Page.h"
#include "PageGroup.h"
#include "Frame.h"
#include "FrameView.h"
#include "NodeList.h"
#include "HTMLElement.h"
#include "HTMLLinkElement.h"
#include "HistoryItem.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "ImageBufferData.h"
#include "ImageWKC.h"
#include "RenderView.h"
#include "RenderText.h"
#include "FocusController.h"
#include "BackForwardList.h"
#include "BackForwardListImpl.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformMouseEvent.h"
#include "PlatformTouchEvent.h"
#include "PlatformWheelEvent.h"
#include "StringImpl.h"
#include "DocumentLoader.h"
#include "ProgressTracker.h"
#include "HTMLAreaElement.h"
#include "HTMLInputElement.h"
#include "IconDatabase.h"
#include "PageCache.h"
#include "TextEncodingRegistry.h"
#include "Settings.h"
#include "FloatRect.h"
#include "MemoryCache.h"
#include "ScriptValue.h"
#include "HTTPRequest.h"
#include "Text.h"
#include "UserGestureIndicator.h"
#include "RenderTextControl.h"
#include "RenderBR.h"
#include "SpatialNavigation.h"

#include "InitializeThreading.h"
#include "PageGroup.h"
#include "Pasteboard.h"

#include "ImageSource.h"

#include "CookieJar.h"

#include "RenderLayer.h"
#include "FontCache.h"
#include "CrossOriginPreflightResultCache.h"
#include "ResourceHandleManagerWKC.h"
#include "SharedTimer.h"
#include "GCController.h"
#include "SQLiteFileSystem.h"

#include "StorageNamespace.h"

#include "RenderFrameSet.h"
#include "BitmapImage.h"

#include "Modules/websockets/WebSocket.h"

#include "bindings/generic/RuntimeEnabledFeatures.h"

#include <wkc/wkcheappeer.h>
#include "FastMalloc.h"

#include "NotImplemented.h"

#if ENABLE(SVG)
#include "SVGDocumentExtensions.h"
#endif

#ifdef __MINGW32__
# ifdef LoadString
#  undef LoadString
# endif
#endif // __MINGW32__

#if USE(MUTEX_DEBUG_LOG)
CRITICAL_SECTION gCriticalSection;
static bool gCriticalSectionFlag = false;
#endif

namespace WebCore {
extern WebCore::IntRect scrollbarTrackRect(Scrollbar* scrollbar);
extern void scrollbarSplitTrack(Scrollbar* scrollbar, const WebCore::IntRect& track, WebCore::IntRect& startTrack, WebCore::IntRect& thumb, WebCore::IntRect& endTrack);
extern WebCore::IntRect scrollbarBackButtonRect(Scrollbar* scrollbar, ScrollbarPart part);
extern WebCore::IntRect scrollbarForwardButtonRect(Scrollbar* scrollbar, ScrollbarPart part);
extern void InitializeLoggingChannelsIfNecessary();
extern void EventLoop_setCycleProc(bool(*)(void*), void*);
extern void initializeGamepads(int pads);
extern bool notifyGamepadEvent(int index, const WTF::String& id, long long timestamp, int naxes, const float* axes, int nbuttons, const float* buttons);
}

namespace WTF {
extern void finalizeMainThreadPlatform();
}

namespace JSC {
class JSGlobalData;
}

extern "C" {
void wkc_libxml2_resetVariables(void);
void wkc_libxslt_resetVariables(void);
void wkc_sqlite3_force_terminate(void);
#ifdef WKC_USE_WKC_OWN_CAIRO
void wkc_pixman_resetVariables(void);
cairo_public void wkc_cairo_resetVariables(void);
#endif
}

using namespace WTF;

#if 0
extern "C" void GUI_Printf(const char*, ...);
# define NF4_DP(a) GUI_Printf a
#else
# define NF4_DP(a) (void(0))
#endif

#if COMPILER(MSVC) && defined(_DEBUG)
#if 0
extern "C" void GUI_Printf(const char*, ...);
# define NF4_MEM_DP LOG_ERROR
#else
# define NF4_MEM_DP(...) ((void)0)
#endif
#else
# define NF4_MEM_DP(...) ((void)0)
#endif /* COMPILER(MSVC) && defined(_DEBUG) */

namespace WKC {

WKC_WEB_VIEW_COMPILE_ASSERT(sizeof(WKCSocketStatistics) == sizeof(SocketStatistics), sizeof__WKCSocketStatistics);

// private container
WKCWebViewPrivate::WKCWebViewPrivate(WKCWebView* parent, WKCClientBuilders& builders)
     : m_parent(parent),
       m_clientBuilders(builders)
{
    m_corePage = 0;
    m_wkcCorePage = 0;
    m_mainFrame = 0;
    m_inspector = 0;
    m_dropdownlist = 0;
#if ENABLE(INPUT_SPEECH)
    m_speechinput = 0;
#endif
    m_settings = 0;

    m_offscreen = 0;
    m_drawContext = 0;
#ifdef USE_WKC_CAIRO
    m_offscreenFormat = 0;
    m_offscreenBitmap = 0;
    m_offscreenRowBytes = 0;
    m_offscreenSize.fWidth = 0;
    m_offscreenSize.fHeight = 0;
#endif

    m_isZoomFullContent = true;
    m_isTransparent = false;
    m_loadStatus = ELoadStatusNone;

    m_opticalZoomLevel = 1.f;
    WKCFloatPoint_Set(&m_opticalZoomOffset, 0, 0);

    m_encoding = 0;
    m_customEncoding = 0;
    m_selectionText = 0;

    m_focusedNode = 0;
    m_nodeFromPoint = 0;
    m_editableNode = 0;

    m_lastNodeUnderMouse = 0;

    m_rootGraphicsLayer = 0;

#if ENABLE(INSPECTOR_SERVER)
    m_inspectorServerClient = 0;
    m_inspectorIsEnabled = false;
#endif

    m_editable = true;

    m_forceTerminated = false;
}

WKCWebViewPrivate::~WKCWebViewPrivate()
{
    if (m_forceTerminated) {
        return;
    }

    delete m_focusedNode;
    delete m_nodeFromPoint;
    delete m_editableNode;

#if ENABLE(INSPECTOR_SERVER)
    if (m_inspectorServerClient) {
        if (m_inspector)
            m_inspector->setInspectorServerClient(0);
        delete m_inspectorServerClient;
        m_inspectorServerClient = 0;
    }
#endif

    if (m_offscreen) {
        wkcOffscreenDeletePeer(m_offscreen);
    }
    m_offscreen = 0;
    if (m_drawContext) {
        wkcDrawContextDeletePeer(m_drawContext);
    }
    m_drawContext = 0;

    if (m_encoding) {
        WTF::fastFree(m_encoding);
        m_encoding = 0;
    }
    if (m_customEncoding) {
        WTF::fastFree(m_customEncoding);
        m_customEncoding = 0;
    }
    if (m_selectionText) {
        WTF::fastFree(m_selectionText);
        m_selectionText = 0;
    }

    if (m_wkcCorePage) {
        delete m_wkcCorePage;
    }
    if (m_corePage) {
        if (m_corePage->settings())
            m_corePage->settings()->setUsesPageCache(false);
        m_mainFrame->privateFrame()->core()->loader()->detachFromParent();
        WebCore::PageGroup* pg = m_corePage->groupPtr();
        if (pg && pg->name().isEmpty()) {
            pg->removeVisitedLinks();
            pg = 0;
        }
        delete m_corePage;
        m_corePage = 0;
        if(pg) {
            if (pg->pages().isEmpty()) {
                pg->removeVisitedLinks();
            }
        }
    }
    if (m_dropdownlist) {
        delete m_dropdownlist;
        m_dropdownlist = 0;
    }

#if ENABLE(INPUT_SPEECH)
    delete m_speechinput;
    m_speechinput = 0;
#endif
    if (m_inspector) {
#if !ENABLE(INSPECTOR)
        delete m_inspector;
#endif
        m_inspector = 0;
    }
    // m_mainFrame will be deleted automatically...

    if (m_settings) {
        delete m_settings;
        m_settings = 0;
    }
}

WKCWebViewPrivate*
WKCWebViewPrivate::create(WKCWebView* parent, WKCClientBuilders& builders)
{
    WKCWebViewPrivate* self = 0;
    self = new WKCWebViewPrivate(parent, builders);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
WKCWebViewPrivate::construct()
{
#ifdef WKC_USE_CUSTOM_BACKFORWARD_LIST
    RefPtr<WKC::BackForwardClientWKC> backforward = 0;
#endif
    WKC::ChromeClientWKC* chrome = 0;
#if ENABLE(CONTEXT_MENUS)
    WKC::ContextMenuClientWKC* contextmenu = 0;
#endif
    WKC::EditorClientWKC* editor = 0;
#if ENABLE(DRAG_SUPPORT)
    WKC::DragClientWKC* drag = 0;
#endif
#if ENABLE(GEOLOCATION)
    WKC::GeolocationClientWKC* geolocation = 0;
#endif
#if ENABLE(DEVICE_ORIENTATION)
    WKC::DeviceMotionClientWKC* devicemotion = 0;
    WKC::DeviceOrientationClientWKC* deviceorientation = 0;
#endif
#if ENABLE(MEDIA_STREAM)
    WKC::UserMediaClientWKC* usermedia = 0;
#endif
#if ENABLE(BATTERY_STATUS)
    WKC::BatteryClientWKC* battery = 0;
#endif
    WebCore::Settings* settings = 0;
    WebCore::Page::PageClients cli;

#ifdef WKC_USE_CUSTOM_BACKFORWARD_LIST
    backforward = adoptRef(WKC::BackForwardClientWKC::create(this));
    if (!backforward) goto error_end;
#endif
    chrome = WKC::ChromeClientWKC::create(this);
    if (!chrome) goto error_end;
#if ENABLE(CONTEXT_MENUS)
    contextmenu = WKC::ContextMenuClientWKC::create(this);
    if (!contextmenu) goto error_end;
#endif
    editor = WKC::EditorClientWKC::create(this);
    if (!editor) goto error_end;
#if ENABLE(DRAG_SUPPORT)
    drag = WKC::DragClientWKC::create(this);
    if (!drag) goto error_end;
#endif
#if ENABLE(GEOLOCATION)
    geolocation = WKC::GeolocationClientWKC::create(this);
    if (!geolocation) goto error_end;
#endif
#if ENABLE(INPUT_SPEECH)
    m_speechinput = WKC::SpeechInputClientWKC::create(this);
    if (!m_speechinput) goto error_end;
#endif
#if ENABLE(DEVICE_ORIENTATION)
    devicemotion = WKC::DeviceMotionClientWKC::create(this);
    if (!devicemotion) goto error_end;
    deviceorientation = WKC::DeviceOrientationClientWKC::create(this);
    if (!deviceorientation) goto error_end;
#endif
#if ENABLE(MEDIA_STREAM)
    usermedia = WKC::UserMediaClientWKC::create(this);
    if (!usermedia) goto error_end;
#endif
#if ENABLE(BATTERY_STATUS)
    battery = WKC::BatteryClientWKC::create(this);
    if (!battery) goto error_end;
#endif

    m_inspector = WKC::InspectorClientWKC::create(this);
    if (!m_inspector) goto error_end;
#ifdef WKC_USE_CUSTOM_BACKFORWARD_LIST
    cli.backForwardClient = backforward;
#endif
    cli.chromeClient = chrome;
#if ENABLE(CONTEXT_MENUS)
    cli.contextMenuClient = contextmenu;
#endif
    cli.editorClient = editor;
#if ENABLE(DRAG_SUPPORT)
    cli.dragClient = drag;
#endif
    cli.inspectorClient = m_inspector;
    cli.alternativeTextClient = 0;
    m_corePage = new WebCore::Page(cli);
    if (!m_corePage) goto error_end;
    m_wkcCorePage = new PagePrivate(m_corePage);
#if ENABLE(GEOLOCATION)
    provideGeolocationTo(m_corePage, geolocation);
#endif
#if ENABLE(INPUT_SPEECH)
    provideSpeechInputTo(m_corePage, m_speechinput);
#endif
#if ENABLE(DEVICE_ORIENTATION)
    provideDeviceMotionTo(m_corePage, devicemotion);
    provideDeviceOrientationTo(m_corePage, deviceorientation);
#endif
#if ENABLE(MEDIA_STREAM)
    provideUserMediaTo(m_corePage, usermedia);
#endif
#if ENABLE(BATTERY_STATUS)
    provideBatteryTo(m_corePage, battery);
#endif

    m_mainFrame = WKC::WKCWebFrame::create(this, m_clientBuilders);
    if (!m_mainFrame) goto error_end;
    m_mainFrame->privateFrame()->core()->init();

    m_dropdownlist = WKC::DropDownListClientWKC::create(this);
    if (!m_dropdownlist) goto error_end;

    settings = m_corePage->settings();

    m_settings = new WKC::WKCSettings(this);
    if (!m_settings) goto error_end;

    settings->setDefaultTextEncodingName("UTF-8");
    settings->setSerifFontFamily("Times New Roman");
    settings->setFixedFontFamily("Courier New");
    settings->setSansSerifFontFamily("Arial");
    settings->setStandardFontFamily("Times New Roman");
    settings->setLoadsImagesAutomatically(true);
    settings->setShrinksStandaloneImagesToFit(true);
    settings->setShouldPrintBackgrounds(true);
    settings->setScriptEnabled(true);
    settings->setSpatialNavigationEnabled(true);
    settings->setImagesEnabled(true);
    settings->setMediaEnabled(true);
    settings->setPluginsEnabled(false);
    settings->setLocalStorageEnabled(false);
    settings->setTextAreasAreResizable(true);
    settings->setUserStyleSheetLocation(WebCore::KURL());
    settings->setDeveloperExtrasEnabled(false);
    settings->setPrivateBrowsingEnabled(false);
    settings->setCaretBrowsingEnabled(false);
    settings->setLocalStorageEnabled(false);
    settings->setXSSAuditorEnabled(false);
    settings->setJavaScriptCanOpenWindowsAutomatically(false);
    settings->setJavaScriptCanAccessClipboard(false);
    settings->setOfflineWebApplicationCacheEnabled(false);
    settings->setAllowUniversalAccessFromFileURLs(false);
    settings->setDOMPasteAllowed(false);
    settings->setNeedsSiteSpecificQuirks(false);
    settings->setUsesPageCache(false);
    settings->setDefaultFixedFontSize(14);
    settings->setDefaultFontSize(14);
    settings->setDownloadableBinaryFontsEnabled(true);
    settings->setShowDebugBorders(false);
    settings->setAuthorAndUserStylesEnabled(true);
//  settings->setAllowScriptsToCloseWindows(true);
    settings->setDNSPrefetchingEnabled(true);

    settings->setCanvasUsesAcceleratedDrawing(false);
    settings->setAcceleratedDrawingEnabled(false);
    settings->setAcceleratedFiltersEnabled(false);
    settings->setCSSCustomFilterEnabled(true);
    settings->setCSSRegionsEnabled(true);
    settings->setRegionBasedColumnsEnabled(false);

    settings->setAcceleratedCompositingEnabled(false);
    settings->setAcceleratedCompositingFor3DTransformsEnabled(false);
    settings->setAcceleratedCompositingForVideoEnabled(false);
    settings->setAcceleratedCompositingForPluginsEnabled(false);
    settings->setAcceleratedCompositingForCanvasEnabled(false);
    settings->setAcceleratedCompositingForAnimationEnabled(false);
    settings->setAcceleratedCompositingForFixedPositionEnabled(false);
    settings->setAcceleratedCompositingForScrollableFramesEnabled(false);

    settings->setExperimentalNotificationsEnabled(false);

    settings->setWebGLEnabled(true);
    settings->setWebGLErrorsToConsoleEnabled(true);
    settings->setOpenGLMultisamplingEnabled(true);
    settings->setPrivilegedWebGLExtensionsEnabled(true);

    settings->setWebAudioEnabled(false);

    settings->setAccelerated2dCanvasEnabled(true);
    settings->setDeferred2dCanvasEnabled(false);
    settings->setMinimumAccelerated2dCanvasSize(64);
    
    settings->setLoadDeferringEnabled(true);
    settings->setTiledBackingStoreEnabled(true);
    settings->setPaginateDuringLayoutEnabled(true);

#if ENABLE(FULLSCREEN_API)
    settings->setFullScreenEnabled(true);
#endif

    settings->setAsynchronousSpellCheckingEnabled(false);
    settings->setMemoryInfoEnabled(false);

    settings->setUsePreHTML5ParserQuirks(false);

    settings->setForceCompositingMode(false);

    settings->setAllowDisplayOfInsecureContent(true);
    settings->setAllowRunningOfInsecureContent(true);

    settings->setMaximumHTMLParserDOMTreeDepth(WebCore::Settings::defaultMaximumHTMLParserDOMTreeDepth);

    settings->setHyperlinkAuditingEnabled(true);
    settings->setCrossOriginCheckInGetMatchedCSSRulesDisabled(true);

    settings->setLayoutFallbackWidth(1024);

#if ENABLE(WEB_SOCKETS)
    settings->setUseHixie76WebSocketProtocol(false);
    WebCore::WebSocket::setIsAvailable(true);
#endif

#if ENABLE(VIDEO_TRACK)
     WebCore::RuntimeEnabledFeatures::setWebkitVideoTrackEnabled(true);
     settings->setShouldDisplaySubtitles(true);
     settings->setShouldDisplayCaptions(true);
     settings->setShouldDisplayTextDescriptions(true);
#endif

#if ENABLE(INDEXED_DATABASE)
     WebCore::RuntimeEnabledFeatures::setWebkitIndexedDBEnabled(true);
#endif

#if ENABLE(MEDIA_STREAM)
     WebCore::RuntimeEnabledFeatures::setMediaStreamEnabled(true);
#endif

    m_corePage->setJavaScriptURLsAreAllowed(true);
    m_corePage->mainFrame()->view()->setClipsRepaints(WKCWebView::clipsRepaints());

#if ENABLE(INSPECTOR_SERVER)
    if (WebInspectorServer::sharedInstance()) {
        m_inspectorServerClient = WKC::InspectorServerClientWKC::create(this);
        m_inspectorServerClient->disableRemoteInspection();
        m_inspector->setInspectorServerClient(m_inspectorServerClient);
    }
#endif

    return true;

error_end:
    if (m_settings) {
        delete m_settings;
        m_settings = 0;
    }
    if (m_dropdownlist) {
        delete m_dropdownlist;
        m_dropdownlist = 0;
    }
    if (m_corePage) {
        delete m_corePage;
        m_corePage = 0;
        // m_mainFrame will be deleted automatically...
    } else {
#if ENABLE(BATTERY_STATUS)
        delete battery;
#endif
#if ENABLE(MEDIA_STREAM)
        delete usermedia;
#endif
#if ENABLE(DEVICE_ORIENTATION)
        delete deviceorientation;
        delete devicemotion;
#endif
#if ENABLE(INPUT_SPEECH)
        delete m_speechinput;
        m_speechinput = 0;
#endif
#if ENABLE(GEOLOCATION)
        delete geolocation;
#endif
#if ENABLE(DRAG_SUPPORT)
        delete drag;
#endif
        delete editor;
#if ENABLE(CONTEXT_MENUS)
        delete contextmenu;
#endif
        delete chrome;
    }
    if (m_inspector) {
        delete m_inspector;
        m_inspector = 0;
    }
    return false;
}

#ifdef WKC_CUSTOMER_PATCH_0304674
void
WKCWebViewPrivate::setOffscreenPointer(void* bitmap)
{
    wkcOffscreenSetBitmapPeer(m_offscreen, bitmap);
}
#endif

void
WKCWebViewPrivate::notifyForceTerminate()
{
    NF4_DP(("WKCWebViewPrivate::notifyForceTerminate\n"));
    m_forceTerminated = true;
    if (m_mainFrame) {
        m_mainFrame->notifyForceTerminate();
    }
}

WKC::WKCSettings*
WKCWebViewPrivate::settings()
{
    return m_settings;
}

WKC::Page*
WKCWebViewPrivate::wkcCore() const
{
    return &m_wkcCorePage->wkc();
}

// drawings
bool
WKCWebViewPrivate::setOffscreen(OffscreenFormat format, void* bitmap, int rowbytes, const WebCore::IntSize& offscreensize, const WebCore::IntSize& viewsize, bool fixedlayout, const WebCore::IntSize* desktopsize, bool needsLayout)
{
    int pformat = 0;
    WKCSize size;

    if (!desktopsize)
        desktopsize = &offscreensize;

    if (m_drawContext) {
        wkcDrawContextDeletePeer(m_drawContext);
        m_drawContext = 0;
    }
    if (m_offscreen) {
        wkcOffscreenDeletePeer(m_offscreen);
        m_offscreen = 0;
    }

    switch (format) {
    case EOffscreenFormatRGBA5650:
        pformat = WKC_OFFSCREEN_TYPE_RGBA5650;
        break;
    case EOffscreenFormatARGB8888:
        pformat = WKC_OFFSCREEN_TYPE_ARGB8888;
        break;
    case EOffscreenFormatPolygon:
        pformat = WKC_OFFSCREEN_TYPE_POLYGON;
        break;
    case EOffscreenFormatCairo16:
        pformat = WKC_OFFSCREEN_TYPE_CAIRO16;
        break;
    case EOffscreenFormatCairo32:
        pformat = WKC_OFFSCREEN_TYPE_CAIRO32;
        break;
    case EOffscreenFormatCairoSurface:
        pformat = WKC_OFFSCREEN_TYPE_CAIROSURFACE;
        break;
    default:
        return false;
    }
    size.fWidth = offscreensize.width();
    size.fHeight = offscreensize.height();

#ifdef USE_WKC_CAIRO
    m_offscreenFormat = pformat;
    m_offscreenBitmap = bitmap;
    m_offscreenRowBytes = rowbytes;
    m_offscreenSize = size;
#endif

    m_offscreen = wkcOffscreenNewPeer(pformat, bitmap, rowbytes, &size);
    if (!m_offscreen) return false;

    m_drawContext = wkcDrawContextNewPeer(m_offscreen);
    if (!m_drawContext) return false;

    if (needsLayout) {
        m_desktopSize = *desktopsize;
        m_defaultDesktopSize = *desktopsize;
        m_defaultViewSize = viewsize;
        m_viewSize = viewsize;

        WebCore::Frame* frame = core()->mainFrame();
        if (!frame || !frame->view()) {
            return false;
        }

        if (fixedlayout) {
            frame->view()->setUseFixedLayout(true);
            frame->view()->setFixedLayoutSize(viewsize);
        } else {
            frame->view()->setUseFixedLayout(false);
        }

        frame->view()->resize(desktopsize->width(), desktopsize->height());
        frame->view()->forceLayout();
        frame->view()->adjustViewSize();
    }

    return true;
}

void
WKCWebViewPrivate::notifyResizeDesktopSize(const WebCore::IntSize& size, bool sendresizeevent)
{
    m_desktopSize = size;

    WebCore::Frame* frame = core()->mainFrame();
    if (!frame || !frame->view()) {
        return;
    }

    frame->view()->resize(size.width(), size.height());
    if (sendresizeevent)
        frame->eventHandler()->sendResizeEvent();
    frame->view()->forceLayout();
    frame->view()->adjustViewSize();

    updateOverlay(WebCore::IntRect(), true);
}

void
WKCWebViewPrivate::notifyResizeViewSize(const WebCore::IntSize& size)
{
    WebCore::Frame* frame = core()->mainFrame();
    if (!frame) return;
    WebCore::FrameView* view = frame->view();
    if (!view) return;

    m_viewSize = size;
    view->setFixedLayoutSize(size);
}

const WebCore::IntSize&
WKCWebViewPrivate::desktopSize() const
{
    return m_desktopSize;
}

const WebCore::IntSize&
WKCWebViewPrivate::viewSize() const
{
    return m_viewSize;
}

const WebCore::IntSize&
WKCWebViewPrivate::defaultDesktopSize() const
{
    return m_defaultDesktopSize;
}

const WebCore::IntSize&
WKCWebViewPrivate::defaultViewSize() const
{
    return m_defaultViewSize;
}

void
WKCWebViewPrivate::notifyRelayout()
{
    WebCore::Frame* frame = core()->mainFrame();

    if (!frame || !frame->contentRenderer() || !frame->view()) {
        return;
    }

	frame->view()->updateLayoutAndStyleIfNeededRecursive();
}

void
WKCWebViewPrivate::notifyPaintOffscreenFrom(const WebCore::IntRect& rect, const WKCPoint& p)
{
#ifdef USE_WKC_CAIRO
    if (!recoverFromCairoError())
        return;
#else
    if (!m_offscreen)
        return;
#endif

    WebCore::Frame* frame = core()->mainFrame();

    if (!frame || !frame->contentRenderer() || !frame->view()) {
        return;
    }

    WebCore::GraphicsContext ctx((PlatformGraphicsContext *)m_drawContext);

    wkcOffscreenBeginPaintPeer(m_offscreen);
    float transX, transY;

    WebCore::IntRect scrolledRect = rect;
    scrolledRect.move(p.fX, p.fY);

    transX = p.fX;
    transY = p.fY;
    WebCore::FloatRect cr;
    cr = WebCore::FloatRect(scrolledRect.x() - transX, scrolledRect.y() - transY, scrolledRect.width(), scrolledRect.height());

    ctx.save();
#ifdef USE_WKC_CAIRO
    wkcDrawContextSetOpticalZoomPeer(m_drawContext, m_opticalZoomLevel, &m_opticalZoomOffset);
#endif
    ctx.clip(cr);
    ctx.translate(-transX, -transY);
    NF4_DP(("Paint(%d, %d, %d, %d)", scrolledRect.x(), scrolledRect.y(), scrolledRect.width(), scrolledRect.height()));
    frame->view()->paintContents(&ctx, scrolledRect);
    if (m_overlayList && !m_rootGraphicsLayer) {
        // WKCOverlayList::paintOffscreen will translate (-visibleRect.x(), -visibleRect.y()), so cancel out the offset here.
        WebCore::FloatRect visibleRect = frame->view()->visibleContentRect();
        ctx.translate(visibleRect.x(), visibleRect.y());
        m_overlayList->paintOffscreen(ctx);
    }
    ctx.restore();
    wkcOffscreenEndPaintPeer(m_offscreen);
}

void
WKCWebViewPrivate::notifyPaintOffscreen(const WebCore::IntRect& rect)
{
#ifdef USE_WKC_CAIRO
    if (!recoverFromCairoError())
        return;
#else
    if (!m_offscreen)
        return;
#endif

    WebCore::Frame* frame = core()->mainFrame();

    if (!frame || !frame->contentRenderer() || !frame->view()) {
        return;
    }

    WebCore::GraphicsContext ctx((PlatformGraphicsContext *)m_drawContext);

    wkcOffscreenBeginPaintPeer(m_offscreen);
    WebCore::FloatRect cr(rect.x(), rect.y(), rect.width(), rect.height());
    ctx.save();
#if USE(WKC_CAIRO)
    wkcDrawContextSetOpticalZoomPeer(m_drawContext, m_opticalZoomLevel, &m_opticalZoomOffset);
#endif
    ctx.clip(cr);
    frame->view()->paint(&ctx, rect);
    if (m_overlayList && !m_rootGraphicsLayer)
        m_overlayList->paintOffscreen(ctx);
    ctx.restore();
    wkcOffscreenEndPaintPeer(m_offscreen);
}

#ifdef USE_WKC_CAIRO
#include <cairo.h>
void
WKCWebViewPrivate::notifyPaintToContext(const WebCore::IntRect& rect, void* context)
{
    WebCore::Frame* frame = core()->mainFrame();

    if (!frame || !frame->contentRenderer() || !frame->view()) {
        return;
    }

    WebCore::GraphicsContext ctx((cairo_t *)context);

    WebCore::FloatRect cr(rect.x(), rect.y(), rect.width(), rect.height());
    ctx.save();
    ctx.clip(cr);
    frame->view()->paint(&ctx, rect);

#if ENABLE(INSPECTOR_SERVER)
    if (m_inspectorIsEnabled) {
        WebCore::InspectorController* controller = m_corePage->inspectorController();
        if (controller && controller->enabled()) {
            controller->drawHighlight(ctx);
        }
    }
#endif

    ctx.restore();
}
#endif

void
WKCWebViewPrivate::notifyScrollOffscreen(const WebCore::IntRect& rect, const WebCore::IntSize& diff)
{
    if (!m_offscreen) return;

    WKCRect r;
    WKCSize d;
    r.fX = rect.x();
    r.fY = rect.y();
    r.fWidth = rect.width();
    r.fHeight = rect.height();
    d.fWidth = diff.width();
    d.fHeight = diff.height();
    wkcOffscreenBeginPaintPeer(m_offscreen);
    wkcOffscreenScrollPeer(m_offscreen, &r, &d);
    wkcOffscreenEndPaintPeer(m_offscreen);
}

void
WKCWebViewPrivate::notifyServiceScriptedAnimations()
{
#if ENABLE(REQUEST_ANIMATION_FRAME) && !USE(REQUEST_ANIMATION_FRAME_TIMER)
    WebCore::Frame* frame = core()->mainFrame();
    if (!frame || !frame->view()) return;
    double cur = WTF::currentTime();
    frame->view()->serviceScriptedAnimations(WebCore::convertSecondsToDOMTimeStamp(cur));
#endif
}

void
WKCWebViewPrivate::setTransparent(bool flag)
{
    m_isTransparent = flag;
    WebCore::Frame* frame = core()->mainFrame();
    if (!frame) return;
    frame->view()->setTransparent(flag);
}

void
WKCWebViewPrivate::setOpticalZoom(float zoom_level, const WKCFloatPoint& offset)
{
    if (!m_offscreen) return;

    m_opticalZoomLevel = zoom_level;
    m_opticalZoomOffset = offset;
    wkcOffscreenSetOpticalZoomPeer(m_offscreen, zoom_level, &offset);
}

void
WKCWebViewPrivate::enterCompositingMode()
{
    WebCore::Frame* frame = core()->mainFrame();
    if (!frame || !frame->view()) {
        return;
    }
    frame->view()->enterCompositingMode();
}

void
WKCWebViewPrivate::setUseAntiAliasForDrawings(bool flag)
{
    if (!m_offscreen) return;
    wkcOffscreenSetUseAntiAliasForPolygonPeer(m_offscreen, flag);
}

void
WKCWebViewPrivate::setUseAntiAliasForCanvas(bool flag)
{
#ifndef USE_WKC_CAIRO
    WebCore::ImageBufferData::setUseAA(flag);
#endif
}

void
WKCWebViewPrivate::setUseBilinearForScaledImages(bool flag)
{
    if (!m_offscreen) return;
    wkcOffscreenSetUseInterpolationForImagePeer(m_offscreen, flag);
}

void
WKCWebViewPrivate::setUseBilinearForCanvasImages(bool flag)
{
#ifndef USE_WKC_CAIRO
    WebCore::ImageBufferData::setUseBilinear(flag);
#endif
}


void
WKCWebViewPrivate::setScrollPositionForOffscreen(const WebCore::IntPoint& scrollPosition)
{
    if (!m_offscreen) return;
    const WKCPoint pos = { scrollPosition.x(), scrollPosition.y() };
    wkcOffscreenSetScrollPositionPeer(m_offscreen, &pos);
}

void
WKCWebViewPrivate::notifyScrollPositionChanged()
{
    WebCore::FrameView* view = m_mainFrame->privateFrame()->core()->view();
    view->scrollPositionChanged();
}

WKC::Node*
WKCWebViewPrivate::findNeighboringEditableNode(WKC::WKCFocusDirection direction)
{
    if (direction != FocusDirectionForward &&
        direction != FocusDirectionBackward)
        return 0;

    WebCore::FocusController* focusController = core()->focusController();
    WebCore::Document* document = focusController->focusedOrMainFrame()->document();
    if (!document)
        return 0;
    WebCore::Node* node = document->focusedNode();

    while (1) {
        node = focusController->findFocusableNode(static_cast<WebCore::FocusDirection>(direction), WebCore::FocusScope::focusScopeOf(document), node, 0);

        if (!node)
            return 0;

        if (!node->isElementNode())
            continue;

        WebCore::Element* element = static_cast<WebCore::Element*>(node);
        if (element->isTextFormControl() || node->isContentEditable()) {
            if (!m_editableNode || m_editableNode->webcore()!=node) {
                delete m_editableNode;
                m_editableNode = NodePrivate::create(node);
            }
            return &m_editableNode->wkc();
        }
    }
}

WKC::Node*
WKCWebViewPrivate::getFocusedNode()
{
    WebCore::Document* doc = core()->focusController()->focusedOrMainFrame()->document();
    WebCore::Node* node = doc->focusedNode();

    if (!node)
        return 0;

    if (!m_focusedNode || m_focusedNode->webcore()!=node) {
        delete m_focusedNode;
        m_focusedNode = NodePrivate::create(node);
    }
    return &m_focusedNode->wkc();
}

WKC::Node*
WKCWebViewPrivate::getNodeFromPoint(int x, int y)
{
    WebCore::Node* node = 0;
    WebCore::Frame* frame = core()->mainFrame();

    while (frame) {
        WebCore::Document* doc = frame->document();
        WebCore::FrameView* view = frame->view();
        WebCore::IntPoint documentPoint = view ? view->windowToContents(WebCore::IntPoint(x, y)) : WebCore::IntPoint(x, y);
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult result(documentPoint);

        renderView->layer()->hitTest(request, result);
        node = result.innerNode();
        while (node && !node->isElementNode())
            node = node->parentNode();
        if (node)
            node = node->shadowAncestorNode();

        frame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    if (!node)
        return 0;

    if (!m_nodeFromPoint || m_nodeFromPoint->webcore()!=node) {
        delete m_nodeFromPoint;
        m_nodeFromPoint = NodePrivate::create(node);
    }
    return &m_nodeFromPoint->wkc();
}

#ifdef USE_WKC_CAIRO
bool
WKCWebViewPrivate::recoverFromCairoError()
{
    if (!m_offscreenBitmap)
        return false;

    if (!m_offscreen || wkcOffscreenIsErrorPeer(m_offscreen) ||
        !m_drawContext || wkcDrawContextIsErrorPeer(m_drawContext)) {
        if (m_drawContext) {
            wkcDrawContextDeletePeer(m_drawContext);
            m_drawContext = 0;
        }
        if (m_offscreen) {
            wkcOffscreenDeletePeer(m_offscreen);
            m_offscreen = 0;
        }

        m_offscreen = wkcOffscreenNewPeer(m_offscreenFormat, m_offscreenBitmap, m_offscreenRowBytes, &m_offscreenSize);
        if (!m_offscreen)
            return false;
        m_drawContext = wkcDrawContextNewPeer(m_offscreen);
        if (!m_drawContext) {
            wkcOffscreenDeletePeer(m_offscreen);
            m_offscreen = 0;
            return false;
        }
    }

    return true;
}
#endif


// implementations
bool WKCWebView::m_clipsRepaints = true;

WKCWebView::WKCWebView()
     : m_private(0)
{
}

WKCWebView::~WKCWebView()
{
    if (m_private) {
        if (!m_private->m_forceTerminated) {
            stopLoading();
        }
        delete m_private;
        m_private = 0;
    }
}

WKCWebView*
WKCWebView::create(WKCClientBuilders& builders)
{
    WKCWebView* self = 0;

    self = new WKCWebView();
    if (!self) return 0;
    if (!self->construct(builders)) {
        delete self;
        return 0;
    }

    return self;
}

bool
WKCWebView::construct(WKCClientBuilders& builders)
{
    m_private = WKCWebViewPrivate::create(this, builders);
    if (!m_private) return false;
    return true;
}

void
WKCWebView::deleteWKCWebView(WKCWebView *self)
{
    delete self;
}

void
WKCWebView::notifyForceTerminate()
{
    if (m_private) {
        m_private->notifyForceTerminate();
    }
}

// off-screen draw

bool
WKCWebView::setOffscreen(OffscreenFormat format, void* bitmap, int rowbytes, const WKCSize& offscreensize, const WKCSize& viewsize, bool fixedlayout, const WKCSize* const desktopsize, bool needsLayout)
{
    WebCore::IntSize os(offscreensize.fWidth, offscreensize.fHeight);
    WebCore::IntSize vs(viewsize.fWidth, viewsize.fHeight);
    WebCore::IntSize ds;
    if (desktopsize) {
        ds.setWidth(desktopsize->fWidth);
        ds.setHeight(desktopsize->fHeight);
    }
    return m_private->setOffscreen(format, bitmap, rowbytes, os, vs, fixedlayout, desktopsize ? &ds : 0, needsLayout);
}

#ifdef WKC_CUSTOMER_PATCH_0304674
void
WKCWebView::setOffscreenPointer( void* bitmap )
{
    m_private->setOffscreenPointer( bitmap );
}
#endif

void
WKCWebView::notifyResizeViewSize(const WKCSize& size)
{
    WebCore::IntSize s(size.fWidth, size.fHeight);
    m_private->notifyResizeViewSize(s);
}

void
WKCWebView::notifyResizeDesktopSize(const WKCSize& size, bool sendresizeevent)
{
    WebCore::IntSize s(size.fWidth, size.fHeight);
    m_private->notifyResizeDesktopSize(s, sendresizeevent);
}

void
WKCWebView::notifyRelayout(bool force)
{
    if (force) {
        WebCore::Frame* frame = m_private->core()->mainFrame();
        if (!frame) return;
        WebCore::Document* document = frame->document();
        if (!document) return;
        WebCore::RenderView* renderView = document->renderView();
        if (!renderView) return;
        renderView->setNeedsLayout(true);
    }
    m_private->notifyRelayout();
}

void
WKCWebView::notifyPaintOffscreenFrom(const WKCRect& rect, const WKCPoint& p)
{
    WebCore::IntRect r(rect.fX, rect.fY, rect.fWidth, rect.fHeight);
    m_private->notifyPaintOffscreenFrom(r, p);
}

void
WKCWebView::notifyPaintOffscreen(const WKCRect& rect)
{
    WebCore::IntRect r(rect.fX, rect.fY, rect.fWidth, rect.fHeight);
    m_private->notifyPaintOffscreen(r);
}

#ifdef USE_WKC_CAIRO
void
WKCWebView::notifyPaintToContext(const WKCRect& rect, void* context)
{
    WebCore::IntRect r(rect.fX, rect.fY, rect.fWidth, rect.fHeight);
    m_private->notifyPaintToContext(r, context);
}
#endif

void
WKCWebView::notifyScrollOffscreen(const WKCRect& rect, const WKCSize& diff)
{
    WebCore::IntRect r(rect.fX, rect.fY, rect.fWidth, rect.fHeight);
    WebCore::IntSize d(diff.fWidth, diff.fHeight);
    m_private->notifyScrollOffscreen(r, d);
}

void
WKCWebView::notifyServiceScriptedAnimations()
{
    m_private->notifyServiceScriptedAnimations();
}

// events

bool
WKCWebView::notifyKeyPress(WKC::Key key, WKC::Modifier modifiers, bool in_autorepeat)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    if (!frame->view()) return false;

    WKC::WKCKeyEvent ev;
    ev.m_type = WKC::EKeyEventPressed;
    ev.m_key = key;
    ev.m_modifiers = modifiers;
    ev.m_char = 0;
    ev.m_autoRepeat = in_autorepeat;
    WebCore::PlatformKeyboardEvent keyboardEvent((void *)&ev);

    if (frame->eventHandler()->keyEvent(keyboardEvent)) {
        return true;
    }
    return false;
}
bool
WKCWebView::notifyKeyRelease(WKC::Key key, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();

    if (!frame->view()) return false;

    WKC::WKCKeyEvent ev;
    ev.m_type = WKC::EKeyEventReleased;
    ev.m_key = key;
    ev.m_modifiers = modifiers;
    ev.m_char = 0;
    ev.m_autoRepeat = false;
    WebCore::PlatformKeyboardEvent keyboardEvent((void *)&ev);

    if (frame->eventHandler()->keyEvent(keyboardEvent)) {
        return true;
    }
    return false;
}
bool
WKCWebView::notifyKeyChar(unsigned int in_char)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();

    if (!frame->view()) return false;

    WKC::WKCKeyEvent ev;
    ev.m_type = WKC::EKeyEventChar;
    ev.m_char = in_char;
    ev.m_key = (WKC::Key)0;
    ev.m_autoRepeat = false;
    WebCore::PlatformKeyboardEvent keyboardEvent((void *)&ev);

    if (frame->eventHandler()->keyEvent(keyboardEvent)) {
        return true;
    }
    return false;
}

bool
WKCWebView::notifyIMEComposition(const unsigned short* in_string, WKC::CompositionUnderline* in_underlines, unsigned int in_underlineNum, unsigned int in_cursorPosition, unsigned int in_selectionEnd, bool in_confirm)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();

    if (!frame->editor()) return false;

    if (in_confirm) {
        frame->editor()->confirmComposition(in_string);
    } else {
        WTF::Vector<WebCore::CompositionUnderline> underlines;
        if (in_underlineNum > 0) {
            underlines.resize(in_underlineNum);
            for (unsigned i = 0; i < in_underlineNum; i++) {
                underlines[i].startOffset = in_underlines[i].startOffset;
                underlines[i].endOffset = in_underlines[i].endOffset;
                underlines[i].thick = in_underlines[i].thick;
                underlines[i].color = in_underlines[i].color;
            }
        }
        frame->editor()->setComposition(in_string, underlines, in_cursorPosition, in_selectionEnd);
    }
    return true;
}

bool
WKCWebView::notifyAccessKey(unsigned int in_char)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    if (!frame->view()) return false;

    WKC::WKCKeyEvent ev;
    ev.m_type = WKC::EKeyEventAccessKey;
    ev.m_key = WKC::EKeyUnknown;
    switch (WebCore::EventHandler::accessKeyModifiers()) {
    case WebCore::PlatformKeyboardEvent::AltKey:
        ev.m_modifiers = WKC::EModifierAlt; break;
    case WebCore::PlatformKeyboardEvent::CtrlKey:
        ev.m_modifiers = WKC::EModifierCtrl; break;
    case WebCore::PlatformKeyboardEvent::MetaKey:
        ev.m_modifiers = WKC::EModifierMod1; break;
    case WebCore::PlatformKeyboardEvent::ShiftKey:
        ev.m_modifiers = WKC::EModifierShift; break;
    default:
        ev.m_modifiers = WKC::EModifierNone; break;
    }
    ev.m_char = in_char;
    ev.m_autoRepeat = false;
    WebCore::PlatformKeyboardEvent keyboardEvent((void *)&ev);

    if (frame->eventHandler()->handleAccessKey(keyboardEvent)) {
        return true;
    }
    return false;
}

bool
WKCWebView::notifyMouseDown(const WKCPoint& pos, WKC::MouseButton button, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventDown;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->handleMousePressEvent(mouseEvent);
}
bool
WKCWebView::notifyMouseUp(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventUp;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->handleMouseReleaseEvent(mouseEvent);
}
bool
WKCWebView::notifyMouseMove(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (!frame->view()) return false;

    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventMove;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->mouseMoved(mouseEvent);
}
static int
getRendererCount(WebCore::RenderObject* renderer, WebCore::Node* linkNode)
{
    int count = 0;
    for (WebCore::RenderObject* r = renderer; r;) {
        if (r->node() == linkNode) {
            r = r->nextInPreOrderAfterChildren(renderer);
            continue;
        }
        count++;
        r = r->nextInPreOrder(renderer);
    }
    return count;
}
bool
WKCWebView::notifyMouseMoveTest(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers, bool& contentChanged)
{
    WKC::Node* node;
    WebCore::Node* coreNode;
    WebCore::Node* linkNode;
    WebCore::Node* targetNode;
    WebCore::RenderObject* renderer;
    WebCore::RenderStyle* style;
    int* visibilityInfoList;
    int rendererCount = 0;
    int visibility = 0;
    int index = 0;
    bool result = false;

    contentChanged = false;

    node = getNodeFromPoint(pos.fX, pos.fY);
    coreNode = node ? node->priv().webcore() : 0;
    linkNode = coreNode ? coreNode->enclosingLinkEventParentOrSelf() : 0;

    if (!coreNode || coreNode == m_private->m_lastNodeUnderMouse || !linkNode)
        return notifyMouseMove(pos, button, modifiers);

    m_private->m_lastNodeUnderMouse = coreNode;

    renderer = coreNode->renderer();
    renderer = renderer ? renderer->hoverAncestor() : 0;
    renderer = renderer ? renderer->containingBlock() : 0;
    while (renderer && renderer->isAnonymousBlock())
        renderer = renderer->parent();
    targetNode = renderer ? renderer->node() : 0;

    if (!renderer || !targetNode)
        return notifyMouseMove(pos, button, modifiers);

    for (WebCore::Node* n = coreNode; n && n != targetNode; n = n->parentNode()) {
        if (n->hasEventListeners(WebCore::eventNames().mouseoverEvent) &&
            n->hasEventListeners(WebCore::eventNames().mousemoveEvent)) {
            contentChanged = true;
            return notifyMouseMove(pos, button, modifiers);
        }
    }
    for (WebCore::Node* n = targetNode; n; n = n->parentNode()) {
        if (n == linkNode)
            return notifyMouseMove(pos, button, modifiers);
    }

    rendererCount = getRendererCount(renderer, linkNode);
    if (rendererCount == 0)
        return notifyMouseMove(pos, button, modifiers);

    visibilityInfoList = (int*)WTF::fastMalloc(sizeof(int) * rendererCount);
    if (!visibilityInfoList)
        return notifyMouseMove(pos, button, modifiers);

    index = 0;
    for (WebCore::RenderObject* r = renderer; r;) {
        if (r->node() == linkNode) {
            r = r->nextInPreOrderAfterChildren(renderer);
            continue;
        }
        visibility = 0;
        style = r->style();
        if (style) {
            if (WebCore::VISIBLE == style->visibility()) {
                visibility = 1;
            }
        }
        visibilityInfoList[index] = visibility;
        index++;
        r = r->nextInPreOrder(renderer);
    }

    result = notifyMouseMove(pos, button, modifiers);

    renderer->frame()->document()->updateStyleIfNeeded();

    renderer = targetNode->renderer();

    for (WebCore::Node* n = coreNode; n && n != targetNode; n = n->parentNode()) {
        if (n->hasEventListeners(WebCore::eventNames().mouseoverEvent) &&
            n->hasEventListeners(WebCore::eventNames().mousemoveEvent)) {
            contentChanged = true;
            goto exit;
        }
    }

    if (getRendererCount(renderer, linkNode) != rendererCount) {
        contentChanged = true;
        goto exit;
    }

    index = 0;

    for (WebCore::RenderObject* r = renderer; r;) {
        if (r->node() == linkNode) {
            r = r->nextInPreOrderAfterChildren(renderer);
            continue;
        }
        style = r->style();
        visibility = 0;
        if (style) {
            if (WebCore::VISIBLE == style->visibility()) {
                visibility = 1;
            }
        }
        if (visibility != visibilityInfoList[index] && r->firstChild()) {
            contentChanged = true;
            goto exit;
        }
        index++;
        r = r->nextInPreOrder(renderer);
    }

exit:
    WTF::fastFree(visibilityInfoList);

    return result;
}
bool
WKCWebView::notifyMouseDoubleClick(const WKCPoint& pos, WKC::MouseButton button, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventDoubleClick;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->handleMousePressEvent(mouseEvent);
}
bool
WKCWebView::notifyMouseWheel(const WKCPoint& pos, const WKCSize& diff, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCWheelEvent ev;

    ev.m_dx = diff.fWidth;
    ev.m_dy = diff.fHeight;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    WebCore::PlatformWheelEvent wheelEvent((void *)&ev);
    return frame->eventHandler()->handleWheelEvent(wheelEvent);
}

void
WKCWebView::notifySetMousePressed(bool pressed)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    frame->eventHandler()->setMousePressed(pressed);
}

void
WKCWebView::notifyLostMouseCapture()
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    frame->eventHandler()->lostMouseCapture();
}

bool
WKCWebView::notifyTouchEvent(int type, const TouchPoint* points, int npoints, WKC::Modifier in_modifiers)
{
#if ENABLE(TOUCH_EVENTS)
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCTouchEvent ev = {0};
    WTF::Vector<WKCTouchPoint> tp(npoints);

    for (int i=0; i<npoints; i++) {
        tp[i].m_id = points[i].fId;
        tp[i].m_state = points[i].fState;
        WKCPoint_SetPoint(&tp[i].m_pos, &points[i].fPoint);
    }

    ev.m_type = type;
    ev.m_points = tp.data();
    ev.m_npoints = npoints;
    ev.m_modifiers = in_modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformTouchEvent touchEvent((void *)&ev);

    return frame->eventHandler()->handleTouchEvent(touchEvent);
#else
    return false;
#endif
}

bool
WKCWebView::notifyScroll(WKC::ScrollType type)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WebCore::ScrollDirection dir = WebCore::ScrollUp;
    WebCore::ScrollGranularity gra = WebCore::ScrollByLine;

    switch (type) {
    case EScrollUp:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollUp;
        break;
    case EScrollDown:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollDown;
        break;
    case EScrollLeft:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollLeft;
        break;
    case EScrollRight:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollRight;
        break;
    case EScrollPageUp:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollUp;
        break;
    case EScrollPageDown:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollDown;
        break;
    case EScrollPageLeft:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollLeft;
        break;
    case EScrollPageRight:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollRight;
        break;
    case EScrollTop:
        gra = WebCore::ScrollByDocument;
        dir = WebCore::ScrollUp;
        break;
    case EScrollBottom:
        gra = WebCore::ScrollByDocument;
        dir = WebCore::ScrollDown;
        break;
    default:
        return false;
    }

    if (!frame->eventHandler()->scrollOverflow(dir, gra)) {
        frame->view()->scroll(dir, gra);
        return true;
    }
    return false;
}

bool
WKCWebView::notifyScroll(int dx, int dy)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    frame->view()->scrollBy(WebCore::IntSize(dx, dy));

    m_private->updateOverlay(WebCore::IntRect(), true);

    return true;
}

bool
WKCWebView::notifyScrollTo(int x, int y)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    frame->view()->setScrollPosition(WebCore::IntPoint(x, y));

    m_private->updateOverlay(WebCore::IntRect(), true);

    return true;
}

void
WKCWebView::scrollPosition(WKCPoint& pos)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    WebCore::IntPoint p = frame->view()->scrollPosition();
    pos.fX = p.x();
    pos.fY = p.y();
}

void
WKCWebView::minimumScrollPosition(WKCPoint& pos) const
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    WebCore::IntPoint p = frame->view()->minimumScrollPosition();
    pos.fX = p.x();
    pos.fY = p.y();
}

void
WKCWebView::maximumScrollPosition(WKCPoint& pos) const
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    WebCore::IntPoint p = frame->view()->maximumScrollPosition();
    pos.fX = p.x();
    pos.fY = p.y();
}

void
WKCWebView::contentsSize(WKCSize& size)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    WebCore::IntSize s = frame->view()->contentsSize();
    size.fWidth = s.width();
    size.fHeight = s.height();
}

void
WKCWebView::notifyFocusIn()
{
    WebCore::FocusController* focusController = m_private->core()->focusController();

    focusController->setActive(true);

    if (focusController->focusedFrame()) {
        focusController->setFocused(true);
    } else {
        focusController->setFocused(true);
        focusController->setFocusedFrame(m_private->core()->mainFrame());
    }
}

void
WKCWebView::notifyFocusOut()
{
    m_private->core()->focusController()->setActive(false);
    m_private->core()->focusController()->setFocused(false);
}

void
WKCWebView::notifyScrollPositionChanged()
{
    m_private->notifyScrollPositionChanged();
}

WKC::Node*
WKCWebView::findFocusableNode(const WKC::FocusDirection direction, const WKCRect* specificRect)
{
    if (!m_private->wkcCore()->focusController())
        return 0;
    return m_private->wkcCore()->focusController()->findNextFocusableNode(direction, specificRect);
}

WKC::Node*
WKCWebView::findFocusableNodeInRect(const WKC::FocusDirection direction, const WKCRect* rect, bool enableContainer)
{
    if (!m_private->wkcCore()->focusController())
        return 0;
    return m_private->wkcCore()->focusController()->findNextFocusableNodeInRect(direction, m_private->m_mainFrame->core(), rect, enableContainer);
}

WKC::Node*
WKCWebView::findNearestFocusableNodeFromPoint(const WKCPoint point, const WKCRect* rect)
{
    if (!m_private->wkcCore()->focusController())
        return 0;
    return m_private->wkcCore()->focusController()->findNearestFocusableNodeFromPoint(point, rect);
}

WKC::Node*
WKCWebView::findNeighboringEditableNode(WKC::WKCFocusDirection direction)
{
    return m_private->findNeighboringEditableNode(direction);
}

bool
WKCWebView::setFocusedNode(WKC::Node* inode)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    WebCore::Document* newDocument = 0;
    WebCore::Node* node = inode ? inode->priv().webcore() : 0;
    if (node && frame) {
        WebCore::Document* focusedDocument = frame->document();
        newDocument = node->document();
        if (newDocument != focusedDocument) {
            focusedDocument->setFocusedNode(0);
        }
        if (newDocument)
            m_private->core()->focusController()->setFocusedFrame(newDocument->frame());
    }
    if (newDocument) {
        return newDocument->setFocusedNode(node);
    }
    return m_private->core()->focusController()->focusedOrMainFrame()->document()->setFocusedNode(node);
}

void
WKCWebView::notifySuspend()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return;
}

void
WKCWebView::notifyResume()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return;
}

void WKCWebView::notifyChromeVisible(bool in_visible)
{
    WebCore::Page* page = m_private->core();
    if (!page)
        return;

    if (page->chrome())
        page->chrome()->setChromeVisible(in_visible);

    // Tasks to be done in sync with chrome visibility change.
    if (in_visible) {
        for (WebCore::Frame* frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
            if (frame->document() && frame->document()->scriptExecutionContext())
                frame->document()->scriptExecutionContext()->resumeTimersOnInvisibleChrome();
        }

        // Start CSS/SVG animations.
        WebCore::Frame* mainFrame = page->mainFrame();
        if (mainFrame && mainFrame->animation())
            mainFrame->animation()->resumeAnimations();
#if ENABLE(SVG)
        for (WebCore::Frame* frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
            if (frame->document() && frame->document()->svgExtensions())
                frame->document()->accessSVGExtensions()->unpauseAnimations();
        }
#endif
    } else {
        // Stop CSS/SVG animations.
        WebCore::Frame* mainFrame = page->mainFrame();
        if (mainFrame && mainFrame->animation())
            mainFrame->animation()->suspendAnimations();
#if ENABLE(SVG)
        for (WebCore::Frame* frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
            if (frame->document() && frame->document()->svgExtensions())
                frame->document()->accessSVGExtensions()->pauseAnimations();
        }
#endif
    }
}

bool WKCWebView::chromeVisible()
{
    bool ret = false;

    if (m_private->core() && m_private->core()->chrome())
        ret = m_private->core()->chrome()->chromeVisible();

    return ret;
}

// APIs
const unsigned short*
WKCWebView::title()
{
    return m_private->m_mainFrame->title();
}
const char*
WKCWebView::uri()
{
    return m_private->m_mainFrame->uri();
}

#ifndef WKC_USE_CUSTOM_BACKFORWARD_LIST
void
WKCWebView::setMaintainsBackForwardList(bool flag)
{
    reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList())->setEnabled(flag);
}

void
WKCWebView::addHistoryItem(const char* uri, const unsigned short* title, const WKCPoint* scrollPoint)
{
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return;

    RefPtr<WebCore::HistoryItem> item = WebCore::HistoryItem::create(WebCore::KURL(WebCore::KURL(), WTF::String::fromUTF8(uri)), WTF::String(title), 0);
    if (scrollPoint)
        item.get()->setScrollPoint(WebCore::IntPoint(*scrollPoint));
    list->addItem(item.release());
}

unsigned int
WKCWebView::getHistoryLength()
{
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return 0;

    return list->entries().size();
}

bool
WKCWebView::getHistoryCurrentIndex(unsigned int& index)
{
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return false;

    if (getHistoryLength() == 0)
        return false;

    index = list->backListCount();
    return true;
}

bool
WKCWebView::getHistoryIndexByItem(WKC::HistoryItem* item, unsigned int& index)
{
    unsigned int i;
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return false;

    WebCore::HistoryItemVector& historyItems = list->entries();
    if (!historyItems.size() || !item)
        return false;
        

    for (i=0; i < historyItems.size(); ++i) {
        if (historyItems[i] == item->priv().webcore()) {
            index = i;
            return true;
        }
    }

    return false;
}

void
WKCWebView::removeHistoryItemByIndex(unsigned int index)
{
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return;

    unsigned int current;
    if (!getHistoryCurrentIndex(current))
        return;

    WebCore::HistoryItem* item = list->itemAtIndex(index - current);
    if (!item)
        return;
    list->removeItem(item);
}
 
bool
WKCWebView::getHistoryItemByIndex(unsigned int index, char* const uri, unsigned int& uriLen, unsigned short* const title, unsigned int& titleLen)
{
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return false;

    unsigned int current;
    if (!getHistoryCurrentIndex(current))
        return false;

    WebCore::HistoryItem* item = list->itemAtIndex(index - current);
    if (!item)
        return false;

    if (uri) {
        strncpy(uri, item->urlString().utf8().data(), item->urlString().utf8().length());
        *(uri + item->urlString().utf8().length()) = 0;
    } else
        uriLen = item->urlString().utf8().length();

    if (title)
        wkc_wstrncpy(title, 0, item->title().characters(), item->title().length());
    else
        titleLen = item->title().length();

    return true;
}

void
WKCWebView::gotoHistoryItemByIndex(unsigned int index)
{
    WebCore::BackForwardListImpl* list;

    list = reinterpret_cast<WebCore::BackForwardListImpl*>(m_private->core()->backForwardList());
    if (!list || !list->enabled())
        return;

    unsigned int current;
    if (!getHistoryCurrentIndex(current))
        return;

    // Following is a part of implementation of Page::goBackOrForward(int distance)
    // except 0 check of distance.
    // Currently we need to do goToItem even if distance is 0.
    int distance = index - current;
    WebCore::HistoryItem* item = list->itemAtIndex(distance);
    if (!item) {
        if (distance > 0) {
            int forwardListCount = list->forwardListCount();
            if (forwardListCount > 0) 
                item = list->itemAtIndex(forwardListCount);
        } else {
            int backListCount = list->backListCount();
            if (backListCount > 0)
                item = list->itemAtIndex(-backListCount);
        }
    }
    if (!item)
        return;

    m_private->core()->goToItem(item, WebCore::FrameLoadTypeIndexedBackForward);
}
#endif // WKC_USE_CUSTOM_BACKFORWARD_LIST

bool
WKCWebView::canGoBack()
{
    NF4_DP(("WKCWebView::canGoBack Enter\n"));

    if (!m_private->core()) {
        NF4_DP(("WKCWebView::canGoBack Exit 1\n"));
        return false;
    }
    if (!m_private->core()->backForwardList()->backItem()) {
        NF4_DP(("WKCWebView::canGoBack Exit 2\n"));
        return false;
    }

    NF4_DP(("WKCWebView::canGoBack Exit 3\n"));
    return true;
}
bool
WKCWebView::canGoBackOrForward(int steps)
{
    return m_private->core()->canGoBackOrForward(steps);
}
bool
WKCWebView::canGoForward()
{
    if (!m_private->core()) {
        return false;
    }
    if (!m_private->core()->backForwardList()->forwardItem()) {
        return false;
    }
    return true;
}
bool
WKCWebView::goBack()
{
    return m_private->core()->goBack();
}
void
WKCWebView::goBackOrForward(int steps)
{
    m_private->core()->goBackOrForward(steps);
}
bool
WKCWebView::goForward()
{
    return m_private->core()->goForward();
}

void
WKCWebView::stopLoading()
{
    WebCore::Frame* frame = 0;
    WebCore::FrameLoader* loader = 0;
    frame = m_private->core()->mainFrame();
    if (!frame) return;
    loader = frame->loader();
    if (!loader) return;
    loader->stopForUserCancel();
}
void
WKCWebView::reload()
{
    m_private->core()->mainFrame()->loader()->reload();
}
void
WKCWebView::reloadBypassCache()
{
    m_private->core()->mainFrame()->loader()->reload(true);
}
void
WKCWebView::loadURI(const char* uri, const char* referer)
{
    if (!uri) return;
    if (!uri[0]) return;
    if (!m_private) return;

    WKCWebFrame* frame = m_private->m_mainFrame;
    frame->loadURI(uri, referer);
}
void
WKCWebView::loadString(const char* content, const unsigned short* mimetype, const unsigned short* encoding, const char* base_uri)
{
    if (!content) return;
    if (!content[0]) return;
    if (!m_private) return;

    WKCWebFrame* frame = m_private->m_mainFrame;
    frame->loadString(content, mimetype, encoding, base_uri);
}
void
WKCWebView::loadHTMLString(const char* content, const char* base_uri)
{
    static const unsigned short cTextHtml[] = {'t','e','x','t','/','h','t','m','l',0};
    loadString(content, cTextHtml, 0, base_uri);
}

bool
WKCWebView::searchText(const unsigned short* text, bool case_sensitive, bool forward, bool wrap)
{
    WTF::TextCaseSensitivity ts = WTF::TextCaseSensitive;
    WebCore::FindDirection dir = WebCore::FindDirectionForward;

    if (case_sensitive) {
        ts = WTF::TextCaseSensitive;
    } else {
        ts = WTF::TextCaseInsensitive;
    }
    if (forward) {
        dir = WebCore::FindDirectionForward;
    } else {
        dir = WebCore::FindDirectionBackward;
    }

    return m_private->core()->findString(WTF::String(text), ts, dir, wrap);
}
unsigned int
WKCWebView::markTextMatches(const unsigned short* string, bool case_sensitive, unsigned int limit)
{
    WTF::TextCaseSensitivity ts = WTF::TextCaseSensitive;

    if (case_sensitive) {
        ts = WTF::TextCaseSensitive;
    } else {
        ts = WTF::TextCaseInsensitive;
    }

    return m_private->core()->markAllMatchesForText(WTF::String(string), ts, false, limit);
}
void
WKCWebView::setHighlightTextMatches(bool highlight)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    do {
        if (frame->editor()) {
            frame->editor()->setMarkedTextMatchesAreHighlighted(highlight);
        }
        frame = frame->tree()->traverseNextWithWrap(false);
    } while (frame);
}
void
WKCWebView::unmarkTextMatches()
{
    m_private->core()->unmarkAllTextMatches();
}

static WKCWebFrame*
kit(WebCore::Frame* coreFrame)
{
    if (!coreFrame)
      return 0;

    ASSERT(coreFrame->loader());
    FrameLoaderClientWKC* client = static_cast<FrameLoaderClientWKC*>(coreFrame->loader()->client());
    return client ? client->webFrame() : 0;
}

WKCWebFrame*
WKCWebView::mainFrame()
{
    return m_private->m_mainFrame;
}
WKCWebFrame*
WKCWebView::focusedFrame()
{
    WebCore::Frame* focusedFrame = m_private->core()->focusController()->focusedFrame();
    return kit(focusedFrame);
}

void
WKCWebView::executeScript(const char* script)
{
    m_private->core()->mainFrame()->script()->executeScript(WTF::String::fromUTF8(script), true);
}

bool
WKCWebView::hasSelection()
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    return (frame->selection()->start() != frame->selection()->end());
}

void
WKCWebView::clearSelection()
{
    for (WebCore::Frame* frame = m_private->core()->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        frame->selection()->clear();
    }
}

static bool
_selectionRects(WebCore::Page* page, WTF::Vector<WebCore::IntRect>& rects, bool textonly, bool useSelectionHeight)
{
//    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    WebCore::Frame* frame = page->focusController()->focusedOrMainFrame();
    RefPtr<WebCore::Range> range = frame->selection()->toNormalizedRange();
    if (!range) {
        return false; 
    }
    WebCore::Node* startContainer = range->startContainer();
    WebCore::Node* endContainer = range->endContainer();

    if (!startContainer || !endContainer)
        return false;

    WebCore::Node* stopNode = range->pastLastNode();
    for (WebCore::Node* node = range->firstNode(); node && node != stopNode; node = node->traverseNextNode()) {
        if (!node)
            continue;
        WebCore::RenderObject* r = node->renderer();
        if (!r)
            continue;
        bool istext = r->isText();
        if (textonly && !istext)
            continue;
        if (istext) {
            WebCore::RenderText* renderText = WebCore::toRenderText(r);
            int startOffset = node == startContainer ? range->startOffset() : 0;
            int endOffset = node == endContainer ? range->endOffset() : std::numeric_limits<int>::max();
            renderText->absoluteRectsForRange(rects, startOffset, endOffset, useSelectionHeight);
        } else {
            const WebCore::FloatPoint absPos = r->localToAbsolute();
            const WebCore::LayoutPoint lp(absPos.x(), absPos.y());
            r->absoluteRects(rects, lp);
        }
    }

    /* adjust the rect with visible content rect */
    WebCore::IntRect visibleContentRect = frame->view()->visibleContentRect();
    for (size_t i = 0; i < rects.size(); ++i) {
        rects[i] = WebCore::intersection(rects[i], visibleContentRect);
        /* remove empty rect */
        if (rects[i].isEmpty()) {
            rects.remove(i);
            i--;
        }
    }

    return true;
}

WKCRect
WKCWebView::selectionBoundingBox(bool textonly, bool useSelectionHeight)
{
    WebCore::IntRect result;
    Vector<WebCore::IntRect> rects;

    _selectionRects(m_private->core(), rects, textonly, useSelectionHeight);
    const size_t n = rects.size();
    for (size_t i = 0; i < n; ++i)
        result.unite(rects[i]);

    WKCRect r = { result.x(), result.y(), result.width(), result.height() };
    return r;
}

static void
getPointsFromIntRect(WebCore::IntRect &r, WKCPoint &top, WKCPoint &bottom, bool fromRight)
{
    top.fX = r.x();
    top.fY = r.y();
    bottom.fX = r.x();
    bottom.fY = r.y() + r.height();
    if (fromRight) {
        top.fX += r.width();
        bottom.fX += r.width();
    }
}

static void
getRenderTextSelectionRectsForRange(WebCore::RenderText *renderText, Vector<WebCore::IntRect> &rects, const int startOffset, const int endOffset, const bool useSelectionHeight)
{
    if (renderText->isBR()) {
        WebCore::RenderBR *renderBR = static_cast<WebCore::RenderBR *>(renderText);
        renderBR->absoluteBRRectsForRange(rects, startOffset, endOffset, useSelectionHeight);
    } else {
        renderText->absoluteRectsForRange(rects, startOffset, endOffset, useSelectionHeight);
    }
}

static bool
getSelectionTopBottomFromNode(WebCore::Node *node, const int offset, WKCPoint &top, WKCPoint &bottom, bool fromRight = false)
{
    Vector<WebCore::IntRect> rects;
    WebCore::RenderObject* r = node->renderer();

    if (!r)
        return false;

    if (r->isText()) {
        WebCore::Text *textNode = 0;
        WebCore::RenderText* renderText = WebCore::toRenderText(r);
        const bool isBR = r->isBR();
        int startOffset = 0;
        int endOffset = 0;
        if (!isBR) {
            textNode = reinterpret_cast<WebCore::Text *>(node);
            startOffset = offset;
            if (startOffset == textNode->length()) {
                fromRight = true;
                startOffset--;
            }
            ASSERT(startOffset < textNode->length());
            endOffset = startOffset + 1;
        }
        getRenderTextSelectionRectsForRange(renderText, rects, startOffset, endOffset, true);
        int savedStartOffset = startOffset;
        // Collapsed white space is not contained in InlineTextBox. Skip to the "first" contained text.
        while (!isBR && rects.size() == 0 && endOffset != textNode->length()) {
            startOffset++;
            endOffset++;
            getRenderTextSelectionRectsForRange(renderText, rects, startOffset, endOffset, true);
        }
        startOffset = savedStartOffset;
        endOffset = startOffset + 1;
        // Collapsed white space is not contained in InlineTextBox. Skip to the "last" contained text.
        while (!isBR && rects.size() == 0 && startOffset != 0) {
            fromRight = true;
            startOffset--;
            endOffset--;
            getRenderTextSelectionRectsForRange(renderText, rects, startOffset, endOffset, true);
        }
    } else {
        WebCore::FloatPoint absPos = r->localToAbsolute();
        const WebCore::LayoutPoint pos(absPos.x(), absPos.y());
        r->absoluteRects(rects, pos);
    }

    if (rects.size() == 0)
        return false;

    getPointsFromIntRect(rects[0], top, bottom, fromRight);

    return true;
}

void
WKCWebView::selectionStartEnd(WKCPoint &startTop, WKCPoint &startBottom, WKCPoint &endTop, WKCPoint &endBottom)
{
    WebCore::Page* page = m_private->core();
    WebCore::Frame* frame = page->focusController()->focusedOrMainFrame();

    WebCore::VisibleSelection selection = frame->selection()->selection();

    if (selection.isNone())
        return;

    WebCore::Position start = selection.start();
    WebCore::Position end = selection.end();

    bool success;
    WebCore::Node* node = start.anchorNode();
    switch (start.anchorType()) {
    case WebCore::Position::PositionIsOffsetInAnchor:
        while (node) {
            success = getSelectionTopBottomFromNode(node, start.offsetInContainerNode(), startTop, startBottom);
            if (success)
                break;
            node = node->traverseNextNode();
        }
        ASSERT(success);
        break;
    case WebCore::Position::PositionIsBeforeAnchor:
        success = getSelectionTopBottomFromNode(node, 0, startTop, startBottom);
        ASSERT(success);
        break;
    case WebCore::Position::PositionIsAfterAnchor:
        // e.g. <table><tr><td><img>|<td>aa|aa</table>
        success = getSelectionTopBottomFromNode(node, 0, startTop, startBottom, true);
        ASSERT(success);
        break;
    default:
        ASSERT(false);
        break;
    }

    node = end.anchorNode();
    switch (end.anchorType()) {
    case WebCore::Position::PositionIsOffsetInAnchor:
        success = getSelectionTopBottomFromNode(node, end.offsetInContainerNode(), endTop, endBottom);
        ASSERT(success);
        break;
    case WebCore::Position::PositionIsAfterAnchor:
        node = node->traverseNextNode();
        while (node) {
            success = getSelectionTopBottomFromNode(node, 0, endTop, endBottom);
            if (success)
                break;
            node = node->traverseNextNode();
        }
        ASSERT(success);
        break;
    case WebCore::Position::PositionIsBeforeAnchor:
        // e.g. <table><td>aa|</table>|<hr>
        success = getSelectionTopBottomFromNode(node, 0, endTop, endBottom);
        ASSERT(success);
        break;
    default:
        ASSERT(false);
        break;
    }
}

void
WKCWebView::selectWordAt(const WKCPoint &target)
{
    WebCore::Node* node;
    WebCore::HitTestResult result;
    WebCore::Frame* nextFrame = m_private->core()->mainFrame();
    WebCore::Frame* frame;
    int x = target.fX;
    int y = target.fY;

    while (nextFrame) {
        frame = nextFrame;
        WebCore::Document* doc = frame->document();
        WebCore::FrameView* view = frame->view();
        WebCore::IntPoint documentPoint = view ? view->windowToContents(WebCore::IntPoint(x, y)) : WebCore::IntPoint(x, y);
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult r(documentPoint);
        result = r;

        renderView->layer()->hitTest(request, result);
        node = result.innerNode();
        while (node && !node->isElementNode())
            node = node->parentNode();
        if (node)
            node = node->shadowAncestorNode();

        nextFrame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    frame->eventHandler()->selectClosestWordFromHitTestResult(result);
}

void
WKCWebView::changeSelectionAt(bool base, const WKCPoint &target)
{
    WebCore::Node* node;
    WebCore::HitTestResult result;
    WebCore::Frame* nextFrame = m_private->core()->mainFrame();
    WebCore::Frame* frame;
    int x = target.fX;
    int y = target.fY;

    while (nextFrame) {
        frame = nextFrame;
        WebCore::Document* doc = frame->document();
        WebCore::FrameView* view = frame->view();
        WebCore::IntPoint documentPoint = view ? view->windowToContents(WebCore::IntPoint(x, y)) : WebCore::IntPoint(x, y);
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult r(documentPoint);
        result = r;

        renderView->layer()->hitTest(request, result);
        node = result.innerNode();
        while (node && !node->isElementNode())
            node = node->parentNode();
        if (node)
            node = node->shadowAncestorNode();

        nextFrame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    bool realBase = base;
    if (!frame->selection()->selection().isBaseFirst())
        realBase = !realBase;
    frame->eventHandler()->updateSelectionForRangeDrag(result, realBase);
}

void
WKCWebView::changeSelectionByOne(bool base, bool left)
{
    WebCore::Page* page = m_private->core();
    WebCore::Frame* frame = page->focusController()->focusedOrMainFrame();
    WebCore::VisibleSelection selection = frame->selection()->selection();

    bool realBase = base;
    if (!selection.isBaseFirst())
        realBase = !realBase;

    WebCore::VisiblePosition p;
    if (realBase)
        p = WebCore::VisiblePosition(selection.base(), selection.affinity());
    else
        p = WebCore::VisiblePosition(selection.extent(), selection.affinity());

    if (left)
        p = p.previous(WebCore::CannotCrossEditingBoundary);
    else
        p = p.next(WebCore::CannotCrossEditingBoundary);

    // Ignore clearing selection case. e.g. Change extent at last text of document to right.
    if (p.isNull())
        return;

    // Need to check new base == new extent?
    // It changes selection type to CaretSelection.

    if (realBase)
        frame->selection()->setBase(p);
    else
        frame->selection()->setExtent(p);
}

const unsigned short*
WKCWebView::selectionText()
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    RefPtr<WebCore::Range> range = frame->selection()->toNormalizedRange();
    if (!range) {
        return 0; 
    }

    if (m_private->m_selectionText) {
        WTF::fastFree(m_private->m_selectionText);
        m_private->m_selectionText = 0;
    }
    m_private->m_selectionText = wkc_wstrdup(range->text().charactersWithNullTermination());
    return m_private->m_selectionText;
}

void
WKCWebView::selectAll()
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    frame->selection()->selectAll();
}

WKC::Page*
WKCWebView::core()
{
    return m_private->wkcCore();
}

WKC::WKCSettings*
WKCWebView::settings()
{
    return m_private->settings();
}

bool
WKCWebView::canShowMimeType(const unsigned short* mime_type)
{
    WebCore::Frame* frame = 0;
    WebCore::FrameLoader* loader = 0;
    frame = m_private->core()->mainFrame();
    loader = frame->loader();
    if (loader) {
        return loader->client()->canShowMIMEType(WTF::String(mime_type));
    }
    return false;
}

float
WKCWebView::zoomLevel()
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (!frame) return 1.0f;
    return frame->pageZoomFactor();
}
float
WKCWebView::setZoomLevel(float zoom_level)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (frame) {
        wkcOffscreenClearGlyphCachePeer();
        frame->setPageZoomFactor(zoom_level);
    }
    return zoom_level;
}
void
WKCWebView::zoomIn(float ratio)
{
    float cur = 0;
    cur = zoomLevel();
    setZoomLevel(cur + ratio);
}
void
WKCWebView::zoomOut(float ratio)
{
    float cur = 0;
    cur = zoomLevel();
    setZoomLevel(cur - ratio);
}

float
WKCWebView::textOnlyZoomLevel()
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (!frame) return 1.0f;
    return frame->textZoomFactor();
}
float
WKCWebView::setTextOnlyZoomLevel(float zoom_level)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (frame) {
        wkcOffscreenClearGlyphCachePeer();
        frame->setTextZoomFactor(zoom_level);
    }
    return zoom_level;
}
void
WKCWebView::textOnlyZoomIn(float ratio)
{
    float cur = 0;
    cur = textOnlyZoomLevel();
    setTextOnlyZoomLevel(cur + ratio);
}
void
WKCWebView::textOnlyZoomOut(float ratio)
{
    float cur = 0;
    cur = textOnlyZoomLevel();
    setTextOnlyZoomLevel(cur - ratio);
}

bool
WKCWebView::fullContentZoom()
{
    return m_private->m_isZoomFullContent;
}
void
WKCWebView::setFullContentZoom(bool full_content_zoom)
{
    float cur = 0;
    cur = zoomLevel();
    if (m_private->m_isZoomFullContent == full_content_zoom) {
        return;
    }
    m_private->m_isZoomFullContent = full_content_zoom;
    setZoomLevel(cur);
}

float
WKCWebView::opticalZoomLevel() const
{
    return m_private->opticalZoomLevel();
}
const WKCFloatPoint&
WKCWebView::opticalZoomOffset() const
{
    return m_private->opticalZoomOffset();
}

float
WKCWebView::setOpticalZoom(float zoom_level, const WKCFloatPoint& offset)
{
    m_private->setOpticalZoom(zoom_level, offset);
    return zoom_level;
}

void
WKCWebView::viewSize(WKCSize& size) const
{
    const WebCore::IntSize& s = m_private->viewSize();
    size.fWidth = s.width();
    size.fHeight = s.height();
}

const unsigned short*
WKCWebView::encoding()
{
    WTF::String encoding = m_private->core()->mainFrame()->document()->inputEncoding();

    if (encoding.isEmpty()) {
        return 0;
    }
    if (m_private->m_encoding) {
        WTF::fastFree(m_private->m_encoding);
        m_private->m_encoding = 0;
    }
    m_private->m_encoding = wkc_wstrdup(encoding.charactersWithNullTermination());
    return m_private->m_encoding;
}
void
WKCWebView::setCustomEncoding(const unsigned short* encoding)
{
    m_private->core()->mainFrame()->loader()->reloadWithOverrideEncoding(WTF::String(encoding));
}
const unsigned short*
WKCWebView::customEncoding()
{
    WTF::String overrideEncoding = m_private->core()->mainFrame()->loader()->documentLoader()->overrideEncoding();

    if (overrideEncoding.isEmpty()) {
        return 0;
    }
    if (m_private->m_customEncoding) {
        WTF::fastFree(m_private->m_customEncoding);
        m_private->m_customEncoding = 0;
    }
    m_private->m_customEncoding = wkc_wstrdup(overrideEncoding.charactersWithNullTermination());
    return m_private->m_customEncoding;
}

WKC::LoadStatus
WKCWebView::loadStatus()
{
    return m_private->m_loadStatus;
}
double
WKCWebView::progress()
{
    return m_private->core()->progress()->estimatedProgress();
}


bool
WKCWebView::hitTestResultForNode(const WKC::Node* node, WKC::HitTestResult& result)
{
    if (!node)
        return false;

    WebCore::Node* n = node->priv().webcore();
    WebCore::HitTestResult& hitTest = const_cast<WebCore::HitTestResult&>(result.priv()->webcore());

    hitTest.setPoint(WebCore::IntPoint(0, 0));
    hitTest.setInnerNode(n);
    hitTest.setInnerNonSharedNode(n);

    if (n->hasTagName(WebCore::HTMLNames::areaTag)) {
        WebCore::HTMLImageElement* img = reinterpret_cast<WebCore::HTMLAreaElement*>(n)->imageElement();
        if (img)
            hitTest.setInnerNonSharedNode(reinterpret_cast<WebCore::Node*>(img));
    }

    n = n->enclosingLinkEventParentOrSelf();
    if (n && n->isElementNode())
        hitTest.setURLElement(static_cast<WebCore::Element*>(n));

    return true;
}

void
WKCWebView::enterCompositingMode()
{
    m_private->enterCompositingMode();
}

void
WKCWebView::cachedSize(unsigned int& dead_resource, unsigned int& live_resource)
{
    live_resource = WebCore::memoryCache()->liveSize();
    dead_resource = WebCore::memoryCache()->deadSize();
}

void
WKCWebView::setPageCacheCapacity(int capacity)
{
    WebCore::pageCache()->setCapacity(capacity);
}

void
WKCWebView::clearCaches(bool clearhttpcache)
{
    if (wkcMemoryGetAllocationStatePeer() == WKC_MEMORYALLOC_TYPE_IMAGE_GIFANIME) {
        /* The GIF-decoder is deleted in use. It cause the access violation. */
        return;
    }

    /* clear conetnts cahces */
    WebCore::memoryCache()->setCapacities(0, 0, 0);

#if ENABLE(WKC_HTTPCACHE)
    if (WebCore::ResourceHandleManager::sharedInstance() && clearhttpcache)
        WebCore::ResourceHandleManager::sharedInstance()->clearHTTPCache();
#endif

    /* other caches */
    clearFontCache(true);
    clearCrossOriginPreflightResultCache();
}

size_t
WKCWebView::fontDataCount()
{
    return WebCore::fontCache()->fontDataCount();
}

size_t
WKCWebView::inactiveFontDataCount()
{
    return WebCore::fontCache()->inactiveFontDataCount();
}

void
WKCWebView::clearFontCache(bool in_clearsAll)
{
    if (in_clearsAll) {
        // Clear cache completely, including even the one used on a displayed page.
        WebCore::fontCache()->invalidate();
    } else {
        // Clear cache without the one used on a displayed page.
        WebCore::fontCache()->purgeInactiveFontData();
    }
}

void
WKCWebView::clearCrossOriginPreflightResultCache()
{
    WebCore::CrossOriginPreflightResultCache::shared().empty();
}

void 
WKCWebView::releaseAutoreleasedPagesNow()
{
    WebCore::pageCache()->releaseAutoreleasedPagesNow();
}

unsigned int 
WKCWebView::getCachedPageCount()
{
    return WebCore::pageCache()->pageCount();
}

void
WKCWebView::setPluginsFolder(const char* in_folder)
{
    wkcPluginSetPluginPathPeer(in_folder);
}

void
WKCWebView::setIconDatabaseFolder(const char* in_folder)
{
#if ENABLE(ICONDATABASE)
    if (in_folder && in_folder[0]) {
        WebCore::iconDatabase().setEnabled(true);
        WebCore::iconDatabase().open(in_folder, "favicons.dat");
    } else {
        WebCore::iconDatabase().setEnabled(false);
    }
#else
    (void)in_folder;
#endif
}

void
WKCWebView::setIconDatabaseOnMemory()
{
#if ENABLE(ICONDATABASE)
    WebCore::iconDatabase().setEnabled(true);
    WebCore::iconDatabase().open("", ":memory:");
#endif
}

void
WKCWebView::clearIconDatabase()
{
#if ENABLE(ICONDATABASE)
    WebCore::iconDatabase().removeAllIcons();
#endif
}

void
WKCWebView::setVisibilityState(int state, bool isInitialState)
{
#if ENABLE(PAGE_VISIBILITY_API)
    WebCore::PageVisibilityState st = WebCore::PageVisibilityStateVisible;
    switch (state) {
    case PageVisibilityStateVisible:
        st = WebCore::PageVisibilityStateVisible;
        break;
    case PageVisibilityStateHidden:
        st = WebCore::PageVisibilityStateHidden;
        break;
    case PageVisibilityStatePrerender:
        st = WebCore::PageVisibilityStatePrerender;
        break;
    case PageVisibilityStatePreview:
        st = WebCore::PageVisibilityStatePreview;
        break;
    default:
        return;
    }
    m_private->core()->setVisibilityState(st, isInitialState);
#else
    (void)state;
    (void)isInitialState;
#endif
}

void
WKCWebKitSetPluginInstances(void* instance1, void* instance2)
{
    wkcPluginWindowSetInstancesPeer(instance1, instance2);
}

unsigned int
WKCWebView::getRSSLinkNum()
{
    WebCore::Document* doc = m_private->core()->mainFrame()->document();
    RefPtr<WebCore::NodeList> list = doc->getElementsByTagName("link");
    unsigned len = list.get()->length();
    unsigned int ret = 0;
    
    if (len > 0) {
        for (unsigned i = 0; i < len; i++) {
            WebCore::HTMLLinkElement* link = static_cast<WebCore::HTMLLinkElement*>(list.get()->item(i));
            if (!link->getAttribute("href").isEmpty() && !link->getAttribute("href").isNull()) {
                if (equalIgnoringCase(link->getAttribute("rel"), "alternate")) {
                    if (equalIgnoringCase(link->getAttribute("type"), "application/rss+xml")
                        || equalIgnoringCase(link->getAttribute("type"), "application/atom+xml")) {
                        ret ++;
                    }
                }
            }
        }
    }

    return ret;
}

unsigned int
WKCWebView::getRSSLinkInfo(WKCRSSLinkInfo* info, unsigned int info_len)
{
    WebCore::Document* doc = m_private->core()->mainFrame()->document();
    RefPtr<WebCore::NodeList> list = doc->getElementsByTagName("link");
    unsigned int len = list.get()->length();
    unsigned int count = 0;

    if (info_len == 0)
        return 0;

    for (unsigned i = 0; i < len; i++) {
        WebCore::HTMLLinkElement* link = static_cast<WebCore::HTMLLinkElement*>(list.get()->item(i));
        if (!link->getAttribute("href").isEmpty() && !link->getAttribute("href").isNull()) {
            if (equalIgnoringCase(link->getAttribute("rel"), "alternate")) {
                if (equalIgnoringCase(link->getAttribute("type"), "application/rss+xml")
                    || equalIgnoringCase(link->getAttribute("type"), "application/atom+xml")) {
                    /* get title and href attributes from DOM */   
                    (info + count)->m_flag = ERSSLinkFlagNone;
                    int title_len = link->getAttribute("title").length();
                    if (title_len > ERSSTitleLenMax) {
                        title_len = ERSSTitleLenMax;
                        (info + count)->m_flag |= ERSSLinkFlagTitleTruncate;
                    }
                    wkc_wstrncpy((info + count)->m_title, ERSSTitleLenMax, link->getAttribute("title").characters(), title_len);

                    WebCore::KURL url = doc->completeURL(link->getAttribute("href").string());
                    int url_len = url.string().utf8().length();
                    if (url_len > ERSSUrlLenMax) {
                        url_len = ERSSUrlLenMax;
                        (info + count)->m_flag |= ERSSLinkFlagUrlTruncate;
                    }
                    strncpy((info + count)->m_url, url.string().utf8().data(), url_len);
                    (info + count)->m_url[url_len] = 0;
                    
                    count ++;
                    if (count == info_len)
                        break;
                }
            }
        }
    }

    return count;
}

WKC::Node*
WKCWebView::getFocusedNode()
{
    return m_private->getFocusedNode();
}

WKC::Node*
WKCWebView::getNodeFromPoint(int x, int y)
{
    return m_private->getNodeFromPoint(x,y);
}

bool
WKCWebView::clickableFromPoint(int x, int y)
{
    WKC::Node* pnode = getNodeFromPoint(x, y);

    if (pnode) {
        WebCore::Node* node = (WebCore::Node *)pnode->priv().webcore();
        if (node->hasEventListeners(WebCore::eventNames().clickEvent)) {
            return true;
        }
    }
    return false;
}

bool
WKCWebView::draggableFromPoint(int x, int y)
{
    WKC::Node* pnode = getNodeFromPoint(x, y);
    if (!pnode)
        return false;
    WebCore::Node* node = (WebCore::Node *)pnode->priv().webcore();
    bool hasmousedown = false;
    while (node) {
        if (!hasmousedown)
            hasmousedown = node->hasEventListeners(WebCore::eventNames().mousedownEvent);
        if (node->hasEventListeners(WebCore::eventNames().dragEvent)
         || node->hasEventListeners(WebCore::eventNames().dragstartEvent)
         || node->hasEventListeners(WebCore::eventNames().dragendEvent)
         // In some contents there is mousemove event handler in parent node 
         // and mousedown event handler in child node. So we treat this node
         // as draggable.
         || (hasmousedown && node->hasEventListeners(WebCore::eventNames().mousemoveEvent))) {
            return true;
        }
        WebCore::RenderObject* renderer = node->renderer();
        if (renderer && renderer->isFrameSet()) {
            WebCore::RenderFrameSet* frameSetRenderer = toRenderFrameSet(renderer);
            if (frameSetRenderer && (frameSetRenderer->isResizingRow() || frameSetRenderer->isResizingColumn()))
                return true;
        }
        if (node->isHTMLElement() && node->hasTagName(WebCore::HTMLNames::inputTag)) {
            WebCore::HTMLInputElement* ie = node->toInputElement();
            if (ie && ie->isRangeControl())
                return true;
        }
        node = node->parentNode();
    }
    return false;
}

static bool
getScrollbarPartFromPoint(int x, int y, WebCore::Scrollbar* scrollbar, WebCore::FrameView* view, bool isFrameScrollbar, WKCWebView::ScrollbarPart& part, WebCore::IntRect& rect)
{
    WebCore::IntPoint mousePosition = scrollbar->convertFromContainingWindow(WebCore::IntPoint(x, y));
    mousePosition.move(scrollbar->x(), scrollbar->y());

    if (!scrollbar->frameRect().contains(mousePosition))
        return false;

    WebCore::IntRect track = WebCore::scrollbarTrackRect(scrollbar);
    if (track.contains(mousePosition)) {
        WebCore::IntRect beforeThumbRect;
        WebCore::IntRect thumbRect;
        WebCore::IntRect afterThumbRect;
        WebCore::scrollbarSplitTrack(scrollbar, track, beforeThumbRect, thumbRect, afterThumbRect);
        if (thumbRect.contains(mousePosition)) {
            part = WKCWebView::ThumbPart;
            rect = thumbRect;
        } else if (beforeThumbRect.contains(mousePosition)) {
            part = WKCWebView::BackTrackPart;
            rect = beforeThumbRect;
        } else if (afterThumbRect.contains(mousePosition)) {
            part = WKCWebView::ForwardTrackPart;
            rect = afterThumbRect;
        } else {
            part = WKCWebView::TrackBGPart;
            rect = track;
        }
    } else if (WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonStartPart).contains(mousePosition)) {
        part = WKCWebView::BackButtonPart;
        rect = WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonStartPart);
    } else if (WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonEndPart).contains(mousePosition)) {
        part = WKCWebView::BackButtonPart;
        rect = WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonEndPart);
    } else if (WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonStartPart).contains(mousePosition)) {
        part = WKCWebView::ForwardButtonPart;
        rect = WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonStartPart);
    } else if (WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonEndPart).contains(mousePosition)) {
        part = WKCWebView::ForwardButtonPart;
        rect = WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonEndPart);
    } else {
        part = WKCWebView::ScrollbarBGPart;
        rect = scrollbar->frameRect();
    }

    if (isFrameScrollbar)
        rect = view->convertToContainingWindow(rect);
    else
        rect = view->contentsToWindow(rect);

    return true;
}

static bool
isScrollbarOfFrameView(int x, int y, WebCore::FrameView* view, WKCWebView::ScrollbarPart& part, WKCRect& ir)
{
    WebCore::Scrollbar* scrollbar = view->verticalScrollbar();
    WebCore::IntRect rect(ir.fX, ir.fY, ir.fWidth, ir.fHeight);

    if (scrollbar) {
        if (getScrollbarPartFromPoint(x, y, scrollbar, view, true, part, rect)) {
            ir = rect;
            return true;
        }
    }
    scrollbar = view->horizontalScrollbar();
    if (scrollbar) {
        if (getScrollbarPartFromPoint(x, y, scrollbar, view, true, part, rect)) {
            ir = rect;
            return true;
        }
    }

    return false;
}

bool
WKCWebView::isScrollbarFromPoint(int x, int y, ScrollbarPart& part, WKCRect& ir)
{
    WebCore::Scrollbar* scrollbar = 0;
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WebCore::FrameView* view = 0;
    WebCore::IntRect rect;

    part = NoPart;

    while (frame) {
        WebCore::Document* doc = frame->document();
        view = frame->view();
        if (!view)
            return false;
        WebCore::IntPoint documentPoint = view->windowToContents(WebCore::IntPoint(x, y));
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult result(documentPoint);

        renderView->layer()->hitTest(request, result);
        scrollbar = result.scrollbar();

        // Following is a part of MouseEventWithHitTestResults::targetNode() in order to get targetNode for subframe
        WebCore::Node* node = result.innerNode();
        if (!node)
            return false;
        if (!node->inDocument()) {
            WebCore::Element* element = node->parentElement();
            if (element && element->inDocument())
                node = element;
        }

        // Check if there is subframe
        frame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    bool isScroll;
    if (scrollbar) {
        isScroll = getScrollbarPartFromPoint(x, y, scrollbar, view, false, part, rect);
        ir = rect;
    } else {
        isScroll = isScrollbarOfFrameView(x, y , view, part, ir);
    }

    return isScroll;
}

// History
bool WKCWebView::addVisitedLink(const char* uri, const unsigned short* title, const struct tm* date)
{
    WebCore::Page* page = m_private->core();

    if (!page)
        return false;

    WebCore::PageGroup* pagegrp = page->groupPtr();

    if (!pagegrp)
        return false;

    WebCore::KURL kurl(WebCore::KURL(), WTF::String::fromUTF8(uri));

    pagegrp->addVisitedLink(kurl);

    return true;
}

bool WKCWebView::addVisitedLinkHash(LinkHash hash)
{
    WebCore::Page* page = m_private->core();

    if (!page)
        return false;

    WebCore::PageGroup* pagegrp = page->groupPtr();

    if (!pagegrp)
        return false;

    pagegrp->addVisitedLinkHash(hash);

    return true;
}

void WKCWebView::setInternalColorFormat(int fmt)
{
#ifdef USE_WKC_CAIRO
    switch (fmt) {
    case EInternalColorFormat8888:
        WebCore::ImageWKC::setInternalColorFormatARGB8888(false);
        break;
    case EInternalColorFormat8888or565:
    case EInternalColorFormat5515withMask:
        WebCore::ImageWKC::setInternalColorFormatARGB8888(true);
        break;
    }
#else
    switch (fmt) {
    case EInternalColorFormat8888:
        WebCore::ImageWKC::setInternalColorFormatRGBA8888();
        break;
    case EInternalColorFormat5515withMask:
    default:
        WebCore::ImageWKC::setInternalColorFormatRGAB5515();
        break;
    }
#endif
}

void WKCWebView::setUseAntiAliasForDrawings(bool flag)
{
    m_private->setUseAntiAliasForDrawings(flag);
}

void WKCWebView::setUseAntiAliasForCanvas(bool flag)
{
    WKCWebViewPrivate::setUseAntiAliasForCanvas(flag);
}

void WKCWebView::setUseBilinearForScaledImages(bool flag)
{
    m_private->setUseBilinearForScaledImages(flag);
}

void WKCWebView::setUseBilinearForCanvasImages(bool flag)
{
    WKCWebViewPrivate::setUseBilinearForCanvasImages(flag);
}

void WKCWebView::setCookieEnabled(bool flag)
{
    m_private->core()->setCookieEnabled(flag);
}

bool WKCWebView::cookieEnabled()
{
    return m_private->core()->cookieEnabled();
}

void WKCWebKitClearCookies(void)
{
    WebCore::ResourceHandleManager::sharedInstance()->clearCookies();
}

int WKCWebKitCookieSerializeNum(void)
{
    return WebCore::ResourceHandleManager::sharedInstance()->CookieSerializeNum();
}

int WKCWebKitCookieSerialize(char* buff, int bufflen)
{
    return WebCore::ResourceHandleManager::sharedInstance()->CookieSerialize(buff, bufflen);
}

void WKCWebKitCookieDeserialize(const char* buff, bool restart)
{
    WebCore::ResourceHandleManager::sharedInstance()->CookieDeserialize(buff, restart);
}

int WKCWebKitCookieGet(const char* uri, char* buf, unsigned int len)
{
    WebCore::KURL ki(WebCore::KURL(), WTF::String::fromUTF8(uri));
    WTF::String domain = ki.host();
    WTF::String path = ki.path();
    bool secure = ki.protocolIs("https");

    WTF::Vector<WebCore::Cookie> rawCookiesList;
    bool rawCookiesImplemented = WebCore::ResourceHandleManager::sharedInstance()->getRawCookies(domain, path, secure, rawCookiesList);
    if (!rawCookiesImplemented) {
        return 0;
    }

    char* pbuf = buf;
    unsigned int remaining_buf_len = len;
    unsigned int cookies_len = 0;
    unsigned int cookies_size = rawCookiesList.size();

    for (int i = 0; i < cookies_size; i++) {
        unsigned int name_len = rawCookiesList[i].name.utf8().length();
        unsigned int value_len = rawCookiesList[i].value.utf8().length();
        unsigned int cookie_len = name_len + value_len + 2; // 2: length of '=' and ';'.

        if (name_len == 0)
            continue;

        cookies_len += cookie_len;

        if (!pbuf)
            continue;

        if (cookie_len > remaining_buf_len)
            break;

        strncpy(pbuf, rawCookiesList[i].name.utf8().data(), name_len);
        pbuf[name_len] = '=';
        pbuf += name_len + 1;
        remaining_buf_len -= name_len + 1;

        if (value_len > 0) {
            strncpy(pbuf, rawCookiesList[i].value.utf8().data(), value_len);
            pbuf += value_len;
            remaining_buf_len -= value_len;
        }

        *pbuf = ';';
        pbuf += 1;
        remaining_buf_len -= 1;
    }

    if (!pbuf)
        return cookies_len;

    if (pbuf - buf > 0) {
        pbuf -= 1;
        *pbuf = '\0'; // replace the last ';' with null terminator.
    }

    return pbuf - buf;
}

WKC_API void
WKCWebKitCookieSet(const char* uri, const char* cookie)
{
    WebCore::KURL ki(WebCore::KURL(), WTF::String::fromUTF8(uri));
    WTF::String domain = ki.host();
    WTF::String path = ki.path();

    WebCore::ResourceHandleManager::sharedInstance()->setCookie(domain, path, WTF::String::fromUTF8(cookie));
}

int WKCWebKitCurrentWebSocketConnectionsNum(void)
{
    WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
    if (mgr)
        return mgr->getCurrentWebSocketConnectionsNum();
    else 
        return -1;
}

void WKCWebKitSetDNSPrefetchProc(void(*requestprefetchproc)(const char*), void* resolverlocker)
{
    wkcNetSetPrefetchDNSCallbackPeer(requestprefetchproc, resolverlocker);
}

void WKCWebKitCachePrefetchedDNSEntry(const char* name, const unsigned char* ipaddr)
{
    wkcNetCachePrefetchedDNSEntryPeer(name, ipaddr);
}

int WKCWebKitGetNumberOfSockets(void)
{
    return wkcNetGetNumberOfSocketsPeer();
}

int WKCWebKitGetSocketStatistics(int in_numberOfArray, SocketStatistics* out_statistics)
{
    return wkcNetGetSocketStatisticsPeer(in_numberOfArray, (WKCSocketStatistics*)out_statistics);
}

void WKCWebView::permitSendRequest(void *handle, bool permit)
{
    WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
    if (mgr)
        mgr->permitRequest(handle, permit);
}

#if ENABLE(WKC_ANDROID_LAYOUT)
void WKCWebView::setScreenWidth(int width)
{
    WebCore::Frame* frame;
    for (frame = m_private->core()->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        if (frame && frame->view()) {
            frame->view()->setScreenWidth(width);
            frame->view()->setNeedsLayout();
        }
    }
}
#endif

// global initialize / finalize

class WKCWebKitMemoryEventHandler {
public:
    WKCWebKitMemoryEventHandler(WKCMemoryEventHandler& handler)
         : m_memoryEventHandler(handler) {};
    ~WKCWebKitMemoryEventHandler() {};

    bool checkMemoryAvailability(unsigned int request_size, bool forimage) {
        return m_memoryEventHandler.checkMemoryAvailability(request_size, forimage);
    }
    bool checkMemoryAllocatable(unsigned int request_size, WKCMemoryEventHandler::AllocationReason reason) {
        return m_memoryEventHandler.checkMemoryAllocatable(request_size, reason);
    }
    void* notifyMemoryExhaust(unsigned int request_size, unsigned int& allocated_size) {
        return m_memoryEventHandler.notifyMemoryExhaust(request_size, allocated_size);
    }
    void notifyMemoryAllocationError(unsigned int request_size, WKCMemoryEventHandler::AllocationReason reason) {
        m_memoryEventHandler.notifyMemoryAllocationError(request_size, reason);
    }
    void notifyCrash(const char* file, int line, const char* function, const char* assertion) {
        m_memoryEventHandler.notifyCrash(file, line, function, assertion);
    }
    void notifyStackOverflow(bool need_restart, unsigned int stack_size, unsigned int consumption, unsigned int margin, void* stack_top, void* stack_base, void* current_stack_top, const char* file, int line, const char* function) {
        m_memoryEventHandler.notifyStackOverflow(need_restart, stack_size, consumption, margin, stack_top, stack_base, current_stack_top, file, line, function);
    }

private:
    WKCMemoryEventHandler& m_memoryEventHandler;
};

static unsigned char gMemoryEventHandlerInstance[sizeof(WKCWebKitMemoryEventHandler)];
static WKCWebKitMemoryEventHandler* gMemoryEventHandler = (WKCWebKitMemoryEventHandler *)&gMemoryEventHandlerInstance;
static void*
WKCWebKitNotifyNoMemory(unsigned int request_size)
{
    unsigned int dummy = 0;

    // Kill Timer
    WebCore::stopSharedTimer();

    // Kill Network Thread
    WebCore::ResourceHandleManager::forceTerminateInstance();

    return gMemoryEventHandler->notifyMemoryExhaust(request_size, dummy);
}

static WKCMemoryEventHandler::AllocationReason
WKCWebKitConvertAllocationReason(int in_reason)
{
    WKCMemoryEventHandler::AllocationReason result;

    switch (in_reason) {
    case WKC_MEMORYALLOC_TYPE_IMAGE_UNSPECIFIED:
        result = WKCMemoryEventHandler::ImageUnspecified;
        break;
    case WKC_MEMORYALLOC_TYPE_IMAGE_GIFANIME:
        result = WKCMemoryEventHandler::ImageGifAnime;
        break;
    case WKC_MEMORYALLOC_TYPE_IMAGE_JPEG:
        result = WKCMemoryEventHandler::ImageJpeg;
        break;
    case WKC_MEMORYALLOC_TYPE_IMAGE_PNG:
        result = WKCMemoryEventHandler::ImagePng;
        break;
    case WKC_MEMORYALLOC_TYPE_LAYER:
        result = WKCMemoryEventHandler::Layer;
        break;
    case WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_GIF:
        result = WKCMemoryEventHandler::SharedBufferGif;
        break;
    case WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JPEG:
        result = WKCMemoryEventHandler::SharedBufferJpeg;
        break;
    case WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_PNG:
        result = WKCMemoryEventHandler::SharedBufferPng;
        break;
    case WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JAVASCRIPT:
        result = WKCMemoryEventHandler::SharedBufferJavaScript;
        break;
    case WKC_MEMORYALLOC_TYPE_JAVASCRIPT_HEAP:
        result = WKCMemoryEventHandler::JavaScriptHeap;
        break;
#ifdef USE_WKC_CAIRO
    case WKC_MEMORYALLOC_TYPE_PIXMAN:
        result = WKCMemoryEventHandler::Pixman;
        break;
#endif
    case WKC_MEMORYALLOC_TYPE_ASSEMBLERBUFFER:
        result = WKCMemoryEventHandler::AssemblerBuffer;
        break;
    default:
        result = WKCMemoryEventHandler::Normal;
        break;
    }
    return result;
}

static void
WKCWebKitNotifyMemoryAllocationError(unsigned int request_size, int in_reason)
{
    gMemoryEventHandler->notifyMemoryAllocationError(request_size, WKCWebKitConvertAllocationReason(in_reason));
}

static void
WKCWebKitNotifyCrash(const char* file, int line, const char* function, const char* assertion)
{
    gMemoryEventHandler->notifyCrash(file, line, function, assertion);
}

static void
WKCWebKitNotifyStackOverflow(bool need_restart, unsigned int stack_size, unsigned int consumption, unsigned int margin, void* stack_top, void* stack_base, void* current_stack_top, const char* file, int line, const char* function)
{
    gMemoryEventHandler->notifyStackOverflow(need_restart, stack_size, consumption, margin, stack_top, stack_base, current_stack_top, file, line, function);
}

static bool
WKCWebKitCheckMemoryAvailability(unsigned int size, bool forimage)
{
    if (!gMemoryEventHandler)
        return false;
    return gMemoryEventHandler->checkMemoryAvailability(size, forimage);
}

static bool
WKCWebKitCheckMemoryAllocatable(unsigned int request_size, int in_reason)
{
    return gMemoryEventHandler->checkMemoryAllocatable(request_size, WKCWebKitConvertAllocationReason(in_reason));
}

static void*
peer_malloc_proc(unsigned int in_size, int in_crashonfailure)
{
    void* ptr = 0;

    if (in_crashonfailure) {
        ptr = WTF::fastMalloc(in_size);
    } else {
        WTF::TryMallocReturnValue rv = tryFastMalloc(in_size);
        if (!rv.getValue(ptr)) {
            return 0;
        }
    }
    return ptr;
}

static void
peer_free_proc(void* in_ptr)
{
    WTF::fastFree(in_ptr);
}

static void*
peer_realloc_proc(void* in_ptr, unsigned int in_size, int in_crashonfailure)
{
    void* ptr = 0;

    if (in_crashonfailure) {
        ptr = WTF::fastRealloc(in_ptr, in_size);
    } else {
        WTF::TryMallocReturnValue rv = WTF::tryFastRealloc(in_ptr, in_size);
        if (!rv.getValue(ptr)) {
            return 0;
        }
    }
    return ptr;
}

class WKCWebKitTimerEventHandler {
public:
    WKCWebKitTimerEventHandler(WKCTimerEventHandler& handler)
        : m_timerEventHandler(handler) {};
    ~WKCWebKitTimerEventHandler() {};

    bool requestWakeUp(unsigned int in_ms, void* in_data) {
        return m_timerEventHandler.requestWakeUp(in_ms, in_data);
    };
private:
    WKCTimerEventHandler& m_timerEventHandler;
};

static unsigned char gTimerEventHandlerInstance[sizeof(WKCWebKitTimerEventHandler)];
static WKCWebKitTimerEventHandler* gTimerEventHandler = (WKCWebKitTimerEventHandler *)&gTimerEventHandlerInstance;

static bool
WKCWebKitRequestWakeUp(unsigned int in_ms, void* in_data)
{
    return gTimerEventHandler->requestWakeUp(in_ms, in_data);
}

WKC_API void
WKCWebKitWakeUp(void* in_opaque)
{
    wkcTimerWakeUpPeer(in_opaque);
}

unsigned int
WKCWebKitGetTickCount()
{
    return wkcGetTickCountPeer();
}

bool
WKCWebKitIsMemoryCrashing()
{
    return wkcMemoryIsCrashingPeer();
}

static void WKCResetVariables();

bool
WKCWebKitInitialize(void* memory, unsigned int memory_size, void* font_memory, unsigned int font_memory_size, WKCMemoryEventHandler& memory_event_handler, WKCTimerEventHandler& timer_event_handler)
{
    NF4_DP(("WKCWebKitInitialize Enter\n"));

    WKCResetVariables();

#if USE(MUTEX_DEBUG_LOG)
    InitializeCriticalSection(&gCriticalSection);
    gCriticalSectionFlag = true;
#endif

    if (!wkcSystemInitializePeer())
        return false;

    if (!wkcDebugPrintInitializePeer())
        return false;

    new (gMemoryEventHandler) WKCWebKitMemoryEventHandler(memory_event_handler);
    new (gTimerEventHandler) WKCWebKitTimerEventHandler(timer_event_handler);

    if (!wkcMemoryInitializePeer(peer_malloc_proc, peer_free_proc, peer_realloc_proc))
        return false;

    wkcMemorySetNotifyNoMemoryProcPeer(WKCWebKitNotifyNoMemory);
    wkcMemorySetNotifyMemoryAllocationErrorProcPeer(WKCWebKitNotifyMemoryAllocationError);
    wkcMemorySetNotifyCrashProcPeer(WKCWebKitNotifyCrash);
    wkcMemorySetNotifyStackOverflowProcPeer(WKCWebKitNotifyStackOverflow);
    wkcMemorySetCheckAvailabilityProcPeer(WKCWebKitCheckMemoryAvailability);
    wkcMemorySetCheckMemoryAllocatableProcPeer(WKCWebKitCheckMemoryAllocatable);

    if (!wkcThreadInitializePeer())
        return false;

#if ENABLE(WEBGL)
    if (!wkcGLInitializePeer())
        return false;
#endif
#if USE(ACCELERATED_COMPOSITING)
    wkcLayerInitializePeer();
#endif

    if (!wkcHWOffscreenInitializePeer())
        return false;

    if (!wkcAudioInitializePeer())
        return false;
    if (!wkcMediaPlayerInitializePeer())
        return false;
    if (!wkcPluginInitializePeer())
        return false;

    if (!wkcTimerInitializePeer(WKCWebKitRequestWakeUp))
        return false;

    if (!wkcFontEngineInitializePeer(font_memory, font_memory_size, (fontPeerMalloc)peer_malloc_proc, (fontPeerFree)peer_free_proc, true))
        return false;

    wkcHeapInitializePeer(memory, memory_size);

    JSC::initializeThreading();
    WebCore::InitializeLoggingChannelsIfNecessary();

    if (!wkcFileInitializePeer())
        return false;
    if (!wkcNetInitializePeer())
        return false;
    if (!wkcSSLInitializePeer())
        return false;

    WebCore::atomicCanonicalTextEncodingName("UTF-8");

    WebCore::PageGroup::setShouldTrackVisitedLinks(true);

#if ENABLE(SQL_DATABASE)
    WebCore::SQLiteFileSystem::registerSQLiteVFS();
#endif

    WKC::WKCPrefs::initialize();

    if (!WKCGlobalSettings::isExistSharedInstance())
        if (!WKCGlobalSettings::createSharedInstance(true))
            return false;

    if (!WebCore::ResourceHandleManager::isExistSharedInstance())
        if (!WebCore::ResourceHandleManager::createSharedInstance())
            return false;

    // cache
    WebCore::memoryCache()->setCapacities(0, 0, 1*1024*1024);
    WebCore::memoryCache()->setDeadDecodedDataDeletionInterval(0);
    WKC::WKCPrefs::setMinDelayBeforeLiveDecodedPruneCaches(1);
    WebCore::pageCache()->setCapacity(0); /* dont use page cache now*/

#if ENABLE(REPAINT_THROTTLING)
    WKCWebView::setRepaintThrottling(0.016, 0, 2.5, 0.5);
#endif

    NF4_DP(("WKCWebKitInitialize Exit\n"));
    return true;
}

void
WKCWebKitFinalize()
{
#if ENABLE(INSPECTOR_SERVER)
    if (WebInspectorServer::sharedInstance())
        WebInspectorServer::deleteSharedInstance();
#endif

    if (WebCore::ResourceHandleManager::isExistSharedInstance())
        WebCore::ResourceHandleManager::deleteSharedInstance();

#if ENABLE(ICONDATABASE)
    if (WebCore::iconDatabase().isEnabled()) {
        WebCore::iconDatabase().close();
    }
#endif

    WebCore::PageGroup::closeLocalStorage();

    WTF::finalizeMainThreadPlatform();

    WKC::WKCPrefs::finalize();

    wkcHWOffscreenFinalizePeer();
    wkcPluginFinalizePeer();
    wkcMediaPlayerFinalizePeer();
    wkcAudioFinalizePeer();
#if USE(ACCELERATED_COMPOSITING)
    wkcLayerFinalizePeer();
#endif
#if ENABLE(WEBGL)
    wkcGLFinalizePeer();
#endif
    wkcSSLFinalizePeer();
    wkcFontEngineFinalizePeer();
    wkcSystemFinalizePeer();
    wkcNetFinalizePeer();
    wkcFileFinalizePeer();
    wkcTimerFinalizePeer();

    wkcHeapFinalizePeer();

    gTimerEventHandler->~WKCWebKitTimerEventHandler();
    memset(gTimerEventHandler, 0, sizeof(WKCWebKitTimerEventHandler));
    gMemoryEventHandler->~WKCWebKitMemoryEventHandler();
    memset(gMemoryEventHandler, 0, sizeof(WKCWebKitMemoryEventHandler));

    wkcThreadFinalizePeer();
    wkcMemoryFinalizePeer();

    wkcDebugPrintFinalizePeer();

#if USE(MUTEX_DEBUG_LOG)
    DeleteCriticalSection(&gCriticalSection);
    gCriticalSectionFlag = false;
#endif
}

bool
WKCWebKitSuspendFont()
{
    if (wkcFontEngineCanSuspendPeer()) {
        wkcFontEngineFinalizePeer();
        return true;
    }
    return false;
}

void
WKCWebKitResumeFont(void* font_memory, unsigned int font_memory_size)
{
    if (wkcFontEngineCanSuspendPeer()) {
        wkcFontEngineInitializePeer(font_memory, font_memory_size, (fontPeerMalloc)peer_malloc_proc, (fontPeerFree)peer_free_proc, true);
    }
}

unsigned int WKCWebKitFontHeapSize()
{
    return wkcFontEngineHeapSizePeer();
}

int WKCWebKitRegisterFontOnMemory(const unsigned char* memPtr, unsigned int len)
{
    return wkcFontEngineRegisterSystemFontPeer(WKC_FONT_ENGINE_REGISTER_TYPE_MEMORY, memPtr, len);
}

int WKCWebKitRegisterFontInFile(const char* filePath)
{
    if (!filePath)
        return -1;
    return wkcFontEngineRegisterSystemFontPeer(WKC_FONT_ENGINE_REGISTER_TYPE_FILE, (const unsigned char *)filePath, ::strlen(filePath));
}

void WKCWebKitUnregisterFonts()
{
    wkcFontEngineUnregisterFontsPeer();
}

bool WKCWebKitSetFontScale(int id, float scale)
{
    return wkcFontEngineSetFontScalePeer(id, scale);
}

void
WKCWebKitSetHWOffscreenDeviceParams(const HWOffscreenDeviceParams* params, void* opaque)
{
    WKCHWOffscreenParams procs = {
        params->fLockProc,
        params->fUnlockProc,
        params->fEnable,
        params->fEnableForImagebuffer,
        params->fScreenWidth,
        params->fScreenHeight
    };
    wkcHWOffscreenSetParamsPeer(&procs, opaque);
#if ENABLE(WEBGL)
    wkcGLRegisterDeviceLockProcsPeer(params->fLockProc, params->fUnlockProc, opaque);
#endif
}

void
WKCWebKitSetLayerCallbacks(const LayerCallbacks* callbacks)
{
#if USE(ACCELERATED_COMPOSITING)
    wkcLayerInitializeCallbacksPeer(callbacks->fTextureMakeProc,
                                    callbacks->fTextureDeleteProc,
                                    callbacks->fTextureUpdateProc,
                                    callbacks->fTextureChangeProc,
                                    callbacks->fDidChangeParentProc,
                                    callbacks->fCanAllocateProc);
#endif
}

void
WKCWebKitGetLayerProperties(void* layer, void** opaque_texture, int* width, int* height, bool* need_yflip, void** out_offscreen, int* offscreenwidth, int* offscreenheight)
{
#if USE(ACCELERATED_COMPOSITING)
    if (opaque_texture)
        *opaque_texture = wkcLayerGetOpaqueTexturePeer(layer);
    if (width && height)
        wkcLayerGetAllocatedSizePeer(layer, width, height);
    if (need_yflip)
        *need_yflip = wkcLayerIsNeededYFlipPeer(layer);
    if (out_offscreen)
        *out_offscreen = wkcLayerGetOffscreenPeer(layer);
    if (offscreenwidth && offscreenheight)
        wkcLayerGetOriginalSizePeer(layer, offscreenwidth, offscreenheight);
#else
    if (opaque_texture)
        *opaque_texture = 0;
    if (width && height)
        *width = *height = 0;
    if (need_yflip)
        *need_yflip = false;
    if (out_offscreen)
        *out_offscreen = 0;
    if (offscreenwidth && offscreenheight)
        *offscreenwidth = *offscreenheight = 0;
#endif
}

void*
WKCWebKitOffscreenNew(OffscreenFormat format, void* bitmap, int rowbytes, const WKCSize* size)
{
    int pformat = 0;

    switch (format) {
    case EOffscreenFormatRGBA5650:
        pformat = WKC_OFFSCREEN_TYPE_RGBA5650;
        break;
    case EOffscreenFormatARGB8888:
        pformat = WKC_OFFSCREEN_TYPE_ARGB8888;
        break;
    case EOffscreenFormatPolygon:
        pformat = WKC_OFFSCREEN_TYPE_POLYGON;
        break;
    case EOffscreenFormatCairo16:
        pformat = WKC_OFFSCREEN_TYPE_CAIRO16;
        break;
    case EOffscreenFormatCairo32:
        pformat = WKC_OFFSCREEN_TYPE_CAIRO32;
        break;
    case EOffscreenFormatCairoSurface:
        pformat = WKC_OFFSCREEN_TYPE_CAIROSURFACE;
        break;
    default:
        return 0;
    }
    return wkcOffscreenNewPeer(pformat, bitmap, rowbytes, size);
}

void
WKCWebKitOffscreenDelete(void* offscreen)
{
    wkcOffscreenDeletePeer(offscreen);
}

bool
WKCWebKitOffscreenIsError(void* offscreen)
{
#ifdef USE_WKC_CAIRO
    return wkcOffscreenIsErrorPeer(offscreen);
#else
    return false;
#endif
}

void*
WKCWebKitDrawContextNew(void* offscreen)
{
    return wkcDrawContextNewPeer(offscreen);
}

void
WKCWebKitDrawContextDelete(void* context)
{
    wkcDrawContextDeletePeer(context);
}

bool
WKCWebKitDrawContextIsError(void* context)
{
#ifdef USE_WKC_CAIRO
    return wkcDrawContextIsErrorPeer(context);
#else
    return false;
#endif
}

// SSL
void* WKCWebKitSSLRegisterRootCA(const char* cert, int cert_len)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterRootCA(cert, cert_len);
    }
    return NULL;
}

void* WKCWebKitSSLRegisterRootCAByDER(const char* cert, int cert_len)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterRootCAByDER(cert, cert_len);
    }
    return NULL;
}

int WKCWebKitSSLUnregisterRootCA(void* certid)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLUnregisterRootCA(certid);
    }
    return -1;
}

void WKCWebKitSSLRootCADeleteAll(void)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->SSLRootCADeleteAll();
    }
}

void* WKCWebKitSSLRegisterCRL(const char* crl, int crl_len)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterCRL(crl, crl_len);
    }
    return NULL;
}

int WKCWebKitSSLUnregisterCRL(void* crlid)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLUnregisterCRL(crlid);
    }
    return -1;
}

void WKCWebKitSSLCRLDeleteAll(void)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->SSLCRLDeleteAll();
    }
}

void* WKCWebKitSSLRegisterClientCert(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterClientCert(pkcs12, pkcs12_len, pass, pass_len);
    }
    return NULL;
}

void* WKCWebKitSSLRegisterClientCertByDER(const unsigned char* cert, int cert_len, const unsigned char* key, int key_len)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterClientCertByDER(cert, cert_len, key, key_len);
    }
    return NULL;
}

int WKCWebKitSSLUnregisterClientCert(void* certid)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLUnregisterClientCert(certid);
    }
    return -1;
}

void WKCWebKitSSLClientCertDeleteAll(void)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->SSLClientCertDeleteAll();
    }
}

bool WKCWebKitSSLRegisterBlackCert(const char* issuerCommonName, const char* SerialNumber)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterBlackCert(issuerCommonName, SerialNumber);
    }
    return false;
}

void WKCWebKitSSLBlackCertDeleteAll(void)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->SSLBlackCertDeleteAll();
    }
}

bool WKCWebKitSSLRegisterEVSSLOID(const char *issuerCommonName, const char *OID, const char *sha1FingerPrint, const char *SerialNumber)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->SSLRegisterEVSSLOID(issuerCommonName, OID, sha1FingerPrint, SerialNumber);
    }
    return false;
}

void WKCWebKitSSLEVSSLOIDDeleteAll(void)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->SSLEVSSLOIDDeleteAll();
    }
}

void WKCWebKitSSLSetAllowServerHost(const char *host_w_port)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->setAllowServerHost(host_w_port);
    }
}

const char** WKCWebKitSSLGetServerCertChain(const char* in_url, int& out_num)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        return mgr->getServerCertChain(in_url, out_num);
    }
    return (const char**)0;
}

void WKCWebKitSSLFreeServerCertChain(const char** chain, int num)
{
    if (WebCore::ResourceHandleManager::sharedInstance()) {
        WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance();
        mgr->freeServerCertChain(chain, num);
    }
}

// File System
void WKCWebKitSetFileSystemProcs(const WKC::FileSystemProcs* procs)
{
    wkcFileCallbackSetPeer(static_cast<const WKCFileProcs *>(procs));
}

// Media Player
void WKCWebKitSetMediaPlayerProcs(const WKC::MediaPlayerProcs* procs)
{
    wkcMediaPlayerCallbackSetPeer(static_cast<const WKCMediaPlayerProcs *>(procs));
}

// Pasteboard
void WKCWebKitSetPasteboardProcs(const WKC::PasteboardProcs* procs)
{
    wkcPasteboardCallbackSetPeer(static_cast<const WKCPasteboardProcs *>(procs));
}

// glyph / image cache
bool WKCWebKitSetGlyphCache(int format, void* cache, const WKCSize* size)
{
    bool ret = false;
    if (cache) {
        ret = wkcOffscreenCreateGlyphCachePeer(format, cache, size);
        if (!ret) return false;
        ret = wkcHWOffscreenCreateGlyphCachePeer(format, cache, size);
        if (!ret) {
            wkcOffscreenDeleteGlyphCachePeer();
        }
        return ret;
    } else {
       wkcHWOffscreenDeleteGlyphCachePeer();
       wkcOffscreenDeleteGlyphCachePeer();
       return true;
    }
}
 
bool WKCWebKitSetImageCache(int format, void* cache, const WKCSize* size)
{
    bool ret = false;
    if (cache) {
        ret = wkcOffscreenCreateImageCachePeer(format, cache, size);
        if (!ret) return false;
        ret = wkcHWOffscreenCreateImageCachePeer(format, cache, size);
        if (!ret) {
            wkcOffscreenDeleteImageCachePeer();
        }
        return ret;
    } else {
       wkcHWOffscreenDeleteImageCachePeer();
       wkcOffscreenDeleteImageCachePeer();
       return true;
    }
}

void
WKCWebKitForceTerminate()
{
    NF4_DP(("WKCWebKitForceTerminate Enter\n"));

    if (WebCore::ResourceHandleManager::isExistSharedInstance())
        WebCore::ResourceHandleManager::forceTerminateInstance();

    if (WKCGlobalSettings::isAutomatic())
        WKCGlobalSettings::deleteSharedInstance();

#if ENABLE(ICONDATABASE)
    if (WebCore::iconDatabase().isEnabled())
        WebCore::iconDatabase().forceTerminate();
#endif

#if ENABLE(INSPECTOR_SERVER)
    if (WebInspectorServer::sharedInstance())
        WebInspectorServer::forceTerminate();
#endif

    WKC::WKCPrefs::forceTerminate();
    WKC::WKCWebViewPrefs::forceTerminate();

#ifdef USE_WKC_CAIRO
    wkcDrawContextForceTerminateAllocatedObjectsPeer();
#endif

    wkcHeapForceTerminatePeer();

    wkcFontEngineForceTerminatePeer();
    wkcPluginForceTerminatePeer();
    wkcMediaPlayerForceTerminatePeer();
    wkcAudioForceTerminatePeer();
    wkcHWOffscreenForceTerminatePeer();
#if USE(ACCELERATED_COMPOSITING)
    wkcLayerForceTerminatePeer();
#endif
#if ENABLE(WEBGL)
    wkcGLForceTerminatePeer();
#endif
    wkcTextBreakIteratorForceTerminatePeer();
    wkcOffscreenForceTerminatePeer();
    wkcDrawContextForceTerminatePeer();
    wkcSSLForceTerminatePeer();
    wkcNetForceTerminatePeer();
    wkcThreadForceTerminateThreadsPeer();
    wkcFileForceTerminatePeer();
    wkcTimerForceTerminatePeer();
    wkcThreadForceTerminateExceptThreadsPeer();
    wkcMemoryForceTerminatePeer();
    wkcDebugPrintForceTerminatePeer();
#if ENABLE(SQL_DATABASE)
    wkc_sqlite3_force_terminate();
#endif
    memset(gTimerEventHandler, 0, sizeof(WKCWebKitTimerEventHandler));
    memset(gMemoryEventHandler, 0, sizeof(WKCWebKitMemoryEventHandler));

    NF4_DP(("WKCWebKitForceTerminate Exit\n"));
}

void
WKCWebKitForceFinalize()
{
    WKCWebKitForceTerminate();
}

void
WKCWebKitResetMaxHeapUsage()
{
    wkcHeapResetMaxHeapUsagePeer();
}

static void
WKCResetVariables()
{
    wkc_libxml2_resetVariables();
#if ENABLE(XSLT)
    wkc_libxslt_resetVariables();
#endif
#ifdef WKC_USE_WKC_OWN_CAIRO
    wkc_pixman_resetVariables();
    wkc_cairo_resetVariables();
#endif

    WebCore::UserGestureIndicator::initializeUserGestureIndicator();
}

unsigned int
WKCWebKitAvailableMemory()
{
    NF4_DP(("WKC::WKCWebKitAvailableMemory() is deprecated. Use WKC::Heap::GetAvailableSize() instead.\n"));
    return Heap::GetAvailableSize();
}

unsigned int
WKCWebKitMaxAvailableBlock()
{
    NF4_DP(("WKC::WKCWebKitMaxAvailableBlock() is deprecated. Use WKC::Heap::GetMaxAvailableBlockSize() instead.\n"));
    return Heap::GetMaxAvailableBlockSize();
}
void
WKCWebKitRequestGarbageCollect(bool is_now, int gctype)
{
    if (is_now) {
        if (gctype == EJSGCTypeDoSweep) {
            WebCore::gcController().garbageCollectNow();
            WebCore::gcController().releaseFreeBlocksInHeap();
        } else {
            ASSERT(gctype == EJSGCTypeDoNotSweep);
            WebCore::gcController().garbageCollectNowDoNotSweep();
        }
    } else { 
        WebCore::gcController().garbageCollectSoon();
    }
}

extern "C" void wkcMediaPlayerSetAudioResourcesPathPeer(const char* in_path);

void
WKCWebKitSetWebAudioResourcePath(const char* path)
{
    wkcMediaPlayerSetAudioResourcesPathPeer(path);
}

void
WKCWebViewPrivate::addOverlay(WKCOverlayIf* overlay, int zOrder, int fixedDirectionFlag)
{
    if (!m_overlayList)
        m_overlayList = WKCOverlayList::create(this);

    m_overlayList->add(overlay, zOrder, fixedDirectionFlag);
}

void
WKCWebViewPrivate::removeOverlay(WKCOverlayIf* overlay)
{
    if (m_overlayList && m_overlayList->remove(overlay) && m_overlayList->empty())
        m_overlayList = nullptr;
}

void
WKCWebViewPrivate::updateOverlay(const WebCore::IntRect& rect, bool immediate)
{
    if (m_overlayList)
        m_overlayList->update(rect, immediate);
}

#if ENABLE(INSPECTOR_SERVER)
void
WKCWebKitSetWebInspectorResourcePath(const char* path)
{
    WebInspectorServer::setResourcePath(path);
}

bool
WKCWebKitStartWebInspector(const char* addr, int port, bool(*modalcycle)(void*), void* opaque)
{
    if (!addr || !modalcycle)
        return false;
    WebInspectorServer::createSharedInstance();

    bool success = WebInspectorServer::sharedInstance()->listen(addr, port);
    if (success) {
        WebCore::EventLoop_setCycleProc(modalcycle, opaque);
        NF4_DP(("Inspector server started successfully. Listening at: http://%s:%d/", bindAddress, port));
    } else {
        NF4_DP(("Couldn't start the inspector server."));
        WebInspectorServer::deleteSharedInstance();
    }
    return success;
}

void
WKCWebKitStopWebInspector()
{
    if (WebInspectorServer::sharedInstance()) {
        WebInspectorServer::deleteSharedInstance();
        WebCore::EventLoop_setCycleProc(0, 0);
    }
}

void
WKCWebViewPrivate::enableWebInspector(bool enable)
{
    if (!WebInspectorServer::sharedInstance())
        return;
    if (!m_inspectorServerClient)
        return;

    if (enable)
        m_inspectorServerClient->enableRemoteInspection();
    else
        m_inspectorServerClient->disableRemoteInspection();
    m_inspectorIsEnabled = enable;
    m_settings->setDeveloperExtrasEnabled(enable);
}

bool
WKCWebViewPrivate::isWebInspectorEnabled()
{
    return m_inspectorIsEnabled;
}
#else
void
WKCWebKitSetWebInspectorResourcePath(const char*)
{
}

bool
WKCWebKitStartWebInspector(const char*, int, bool(*)(void*), void*)
{
    return true;
}

void
WKCWebKitStopWebInspector()
{
}

void
WKCWebViewPrivate::enableWebInspector(bool)
{
}

bool
WKCWebViewPrivate::isWebInspectorEnabled()
{
    return false;
}
#endif

void
WKCWebView::enableWebInspector(bool enable)
{
    m_private->enableWebInspector(enable);
}

bool
WKCWebView::isWebInspectorEnabled()
{
    return m_private->isWebInspectorEnabled();
}

void
WKCWebView::setScrollPositionForOffscreen(const WKCPoint& scrollPosition)
{
    WebCore::IntPoint p(scrollPosition.fX, scrollPosition.fY);
    m_private->setScrollPositionForOffscreen(scrollPosition);
}

void
WKCWebView::jsJITCodePageAllocatedBytes(size_t& allocated_bytes, size_t& total_bytes, size_t& max_allocatable_bytes)
{
#if ENABLE(JIT)
    notImplemented();
    allocated_bytes = 0;
    total_bytes = 0;
    max_allocatable_bytes = 0;
#else
    allocated_bytes = 0;
    total_bytes = 0;
    max_allocatable_bytes = 0;
#endif
}

void
WKCWebView::scrollNodeByRecursively(WKC::Node* node, int dx, int dy)
{
    if (!node) {
        return;
    } 
    
    WebCore::Node* coreNode = node->priv().webcore();
    if (!coreNode->renderer() || !coreNode->renderer()->enclosingLayer()) {
        return;
    }
    coreNode->renderer()->enclosingLayer()->scrollByRecursively(dx, dy);
}

void
WKCWebView::scrollNodeBy(WKC::Node* node, int dx, int dy)
{
    if (!node) {
        return;
    } 
    
    WebCore::Node* coreNode = node->priv().webcore();
    WebCore::RenderObject* renderer = coreNode->renderer();
    WebCore::RenderLayer* layer = renderer ? renderer->enclosingLayer() : 0;

    if (!layer) {
        return;
    }

    bool isLineClampNoneByParent = true;
    if (renderer->parent()) {
        isLineClampNoneByParent = renderer->parent()->style()->lineClamp().isNone();
    }
    if (renderer->hasOverflowClip() && isLineClampNoneByParent) {
        int offsetX = layer->scrollXOffset() + dx;
        int offsetY = layer->scrollYOffset() + dy;
        layer->scrollToOffset(offsetX, offsetY);

        WebCore::Frame* frame = renderer->frame();
        if (frame) {
            WebCore::EventHandler* eventHandler = frame->eventHandler();
            eventHandler->cancelFakeMouseMoveEvent(); // to avoid hover event when scrolling
            eventHandler->updateAutoscrollRenderer();
        }

    } else if (renderer->view()->frameView()) {
        renderer->view()->frameView()->scrollBy(WebCore::IntSize(dx, dy));
    }
}

// The logic of WKCWebView::isScrollableNode() is from SpatialNavigation.cpp's isScrollableNode().
bool
WKCWebView::isScrollableNode(const WKC::Node* node)
{
    if (!node) {
        return false;
    }
    WebCore::Node* coreNode = node->priv().webcore();

    if (WebCore::RenderObject* renderer = coreNode->renderer()) {
        if (!renderer->isBox() || !toRenderBox(renderer)->canBeScrolledAndHasScrollableArea())
            return false;
        if (renderer->isTextArea()) {
            // Check shadow tree for the text control.
            WebCore::HTMLTextFormControlElement* form = toRenderTextControl(renderer)->textFormControlElement();
            if (form) {
                WebCore::HTMLElement* innerTextElement = form->innerTextElement();
                return innerTextElement ? innerTextElement->hasChildNodes() : false;
            }
            return false;
        } else {
            return coreNode->hasChildNodes();
        }
    }
    return false;
}

bool
WKCWebView::canScrollNodeInDirection(const WKC::Node* node, WKC::WKCFocusDirection direction)
{
    if (!node) {
        return false;
    }

    WebCore::Node *coreNode = node->priv().webcore();

    return WebCore::canScrollInDirection(coreNode, static_cast<WebCore::FocusDirection>(direction));
}

WKCRect
WKCWebView::transformedRect(const WKC::Node* node, const WKCRect& rect)
{
    if (!node) {
        return rect;
    }
    WebCore::Node* coreNode = node->priv().webcore();
    WebCore::RenderObject* renderer = coreNode->renderer();
    if (!renderer) {
        return rect;
    }

    WebCore::FloatPoint absPos = renderer->localToAbsolute();
    WebCore::IntRect r(rect.fX - absPos.x(), rect.fY - absPos.y(), rect.fWidth, rect.fHeight);
    WebCore::FloatQuad quad = renderer->localToAbsoluteQuad(WebCore::FloatQuad(r));
    WebCore::IntRect br = quad.enclosingBoundingBox();

    return br;
}

bool
WKCWebView::containsPoint(const WKC::Node* node, const WKCPoint& point)
{
    if (!node) {
        return false;
    }
    WebCore::Node* coreNode = node->priv().webcore();
    WebCore::RenderObject* renderer = coreNode->renderer();
    if (!renderer) {
        return false;
    }
    WebCore::FrameView* view = coreNode->document()->view();
    if (!view) {
        return false;
    }

    WebCore::IntPoint pos = view->windowToContents(WebCore::IntPoint(point.fX, point.fY));
    WTF::Vector<WebCore::FloatQuad> quads;

    renderer->absoluteQuads(quads);

    for (size_t i = 0; i < quads.size(); ++i) {
        if (quads[i].containsPoint(pos)) {
            return true;
        }
    }
    return false;
}

bool
WKCWebView::editable()
{
    return m_private->editable();
}

void
WKCWebView::setEditable(bool enable)
{
    m_private->setEditable(enable);
}

void
WKCWebView::notifyJSExtensionEvent(JSExtensionEvent eventId) const
{
    char* eventName;

    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (!frame) return;
    WebCore::Document* document = frame->document();
    if (!document) return;
    WebCore::DOMWindow* domWindow = document->domWindow();
    if (!domWindow) return;

    switch (eventId) {
#if defined(__ghs__) // ENABLE(WKC_WIIU_JS_EXTENSION)
    case EImageViewStart:
        eventName = "wiiu_imageview_start";
        break;

    case EImageViewEnd:
        eventName = "wiiu_imageview_end";
        break;

    case EImageViewChangeViewMode:
        eventName = "wiiu_imageview_change_viewmode";
        break;

    case EImageViewChangeContent:
        eventName = "wiiu_imageview_change_content";
        break;

    case EImageViewError:
        eventName = "wiiu_imageview_error";
        break;
#endif
    default:
        return;
    }

    WebCore::ExceptionCode ec = 0;
    RefPtr<WebCore::Event> event = document->createEvent("CustomEvent", ec);
    if (!event) return;

    event->initEvent(eventName, true, true);
    domWindow->dispatchEvent(event.release());
}

void
WKCWebView::setRepaintThrottling(double deferredRepaintDelay, double initialDeferredRepaintDelayDuringLoading, double maxDeferredRepaintDelayDuringLoading, double deferredRepaintDelayIncrementDuringLoading)
{
#if ENABLE(REPAINT_THROTTLING)
    WebCore::FrameView::setRepaintThrottlingDeferredRepaintDelay(deferredRepaintDelay);
    WebCore::FrameView::setRepaintThrottlingnInitialDeferredRepaintDelayDuringLoading(initialDeferredRepaintDelayDuringLoading);
    WebCore::FrameView::setRepaintThrottlingMaxDeferredRepaintDelayDuringLoading(maxDeferredRepaintDelayDuringLoading);
    WebCore::FrameView::setRepaintThrottlingDeferredRepaintDelayIncrementDuringLoading(deferredRepaintDelayIncrementDuringLoading);
#endif
}

void
WKCWebView::cancelFullScreen()
{
    // Unlike the name we cannot use webkitCancelFullScreen() here.
    // Because the method does not consider the elements inside frames...

    WebCore::Document* childmost = 0;
    for (WebCore::Frame* frame = m_private->core()->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        if (frame->document()->webkitFullscreenElement())
            childmost = frame->document();
    }
    if (childmost)
        childmost->webkitExitFullscreen();
}

bool
WKCWebView::isFullScreen() const
{
    const WebCore::Document* doc = m_private->core()->mainFrame()->document();
    return doc->webkitIsFullScreen();
}

// session storage and local storage
unsigned
WKCWebView::sessionStorageMemoryConsumptionBytes()
{
    WebCore::Page* page = m_private->core();

    if (!page) {
        return 0;
    }
    return page->sessionStorageMemoryConsumptionBytes();
}

unsigned
WKCWebView::localStorageMemoryConsumptionBytes()
{
    return WebCore::PageGroup::localStorageMemoryConsumptionBytes();
}

void
WKCWebView::clearSessionStorage()
{
    WebCore::Page* page = m_private->core();

    if(!page) {
        return;
    }
    page->clearSessionStorageForAllOrigins();
}

void
WKCWebView::clearLocalStorage()
{
    WebCore::PageGroup::clearLocalStorageForAllOrigins();
}

void
WKCWebView::initializeGamepads(int num)
{
#if ENABLE(GAMEPAD)
    WebCore::initializeGamepads(num);
#endif
}

bool
WKCWebView::notifyGamepadEvent(int index, const WKC::String& id, long long timestamp, int naxes, const float* axes, int nbuttons, const float* buttons)
{
#if ENABLE(GAMEPAD)
    return WebCore::notifyGamepadEvent(index, id, timestamp, naxes, axes, nbuttons, buttons);
#else
    return false;
#endif
}

// idn

namespace IDN {

int
toUnicode(const char* host, unsigned short* idn, int maxidn)
{
    return wkcI18NIDNtoUnicodePeer((const unsigned char *)host, -1, idn, maxidn);
}

int
fromUnicode(const unsigned short* idn, char* host, int maxhost)
{
    return wkcI18NIDNfromUnicodePeer(idn, -1, (unsigned char *)host, maxhost);
}

} // namespace

// network utility

namespace NetUtil {

/*
 * correctIPAddress()
 *
 * description:
 *   Check format of IP address.
 *   Whether the IP Address is not considered valid.
 *
 * argument:
 *   in_ipaddress: String of IP Address
 *
 * return value:
 *   0: in_ipaddress is not IP Address.
 *   4: in_ipaddress is IPv4 Address.
 *   6: in_ipaddress is IPv6 Address.
 */
int
correctIPAddress(const char *in_ipaddress)
{
    return wkcNetCheckCorrectIPAddressPeer(in_ipaddress);
}

} // namespace

} // namespace

#ifdef _MSC_VER
void _jsapi_dummy_func() {
    JSEvaluateScript(0,0,0,0,0,0);
    JSContextGetGlobalObject(0);
}
#endif
