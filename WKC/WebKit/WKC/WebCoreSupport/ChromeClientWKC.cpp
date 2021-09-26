/*
 * Copyright (C) 2007, 2008 Holger Hans Peter Freyther
 * Copyright (C) 2007, 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Gustavo Noronha Silva <gns@gnome.org>
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "Page.h"
#include "Frame.h"
#include "FrameView.h"
#include "WKCWebView.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"
#include "ChromeClientWKC.h"
#include "PopupMenuWKC.h"
#include "SearchPopupMenuWKC.h"
#include "FileChooser.h"
#include "Modules/geolocation/Geolocation.h"
#include "FrameLoadRequest.h"
#include "WindowFeatures.h"
#include "PlatformString.h"

#if ENABLE(INPUT_TYPE_COLOR)
# include "ColorChooser.h"
#endif

#if USE(ACCELERATED_COMPOSITING)
# include "GraphicsLayer.h"
#endif

#include <wkc/wkcgpeer.h>

#include "WKCWebViewPrivate.h"

#include "NotImplemented.h"

#include "helpers/ChromeClientIf.h"
#include "helpers/WKCPage.h"
#include "helpers/WKCString.h"
#include "helpers/WKCHitTestResult.h"
#include "helpers/WKCFrameLoadRequest.h"

#include "helpers/privates/WKCHitTestResultPrivate.h"
#include "helpers/privates/WKCFrameLoadRequestPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCElementPrivate.h"
#include "helpers/privates/WKCFileChooserPrivate.h"
#include "helpers/privates/WKCGeolocationPrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCPagePrivate.h"
#include "helpers/privates/WKCSecurityOriginPrivate.h"
#include "helpers/WKCViewportArguments.h"

#if USE(ACCELERATED_COMPOSITING)
# include "helpers/privates/WKCGraphicsLayerPrivate.h"
namespace WebCore {
WKC::GraphicsLayerPrivate* GraphicsLayerWKC_wkc(const WebCore::GraphicsLayer* layer);
}
#endif


namespace WKC {

ChromeClientWKC::ChromeClientWKC(WKCWebViewPrivate* view)
     : m_view(view)
{
    m_appClient = 0;
}

ChromeClientWKC::~ChromeClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteChromeClient(m_appClient);
        m_appClient = 0;
    }
}

ChromeClientWKC*
ChromeClientWKC::create(WKCWebViewPrivate* view)
{
    ChromeClientWKC* self = 0;
    self = new ChromeClientWKC(view);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
ChromeClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createChromeClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void*
ChromeClientWKC::webView() const
{
    return m_view->parent();
}

void
ChromeClientWKC::chromeDestroyed()
{
    delete this;
}

void
ChromeClientWKC::setWindowRect(const WebCore::FloatRect& rect)
{
    WKCFloatRect r = { rect.x(), rect.y(), rect.width(), rect.height() };
    m_appClient->setWindowRect(r);
}
WebCore::FloatRect
ChromeClientWKC::windowRect()
{
    WKCFloatRect rr = m_appClient->windowRect();
    return WebCore::FloatRect(rr.fX, rr.fY, rr.fWidth, rr.fHeight);
}

WebCore::FloatRect
ChromeClientWKC::pageRect()
{
    WKCFloatRect rr = m_appClient->pageRect();
    return WebCore::FloatRect(rr.fX, rr.fY, rr.fWidth, rr.fHeight);
}

void
ChromeClientWKC::focus()
{
    m_appClient->focus();
}
void
ChromeClientWKC::unfocus()
{
    m_appClient->unfocus();
}

bool
ChromeClientWKC::canTakeFocus(WebCore::FocusDirection dir)
{
    return m_appClient->canTakeFocus((WKC::WKCFocusDirection)dir);
}
void
ChromeClientWKC::takeFocus(WebCore::FocusDirection dir)
{
    m_appClient->takeFocus((WKC::WKCFocusDirection)dir);
}

void
ChromeClientWKC::focusedNodeChanged(WebCore::Node* node)
{
    if (!node) {
        m_appClient->focusedNodeChanged(0);
    } else {
        NodePrivate* n = NodePrivate::create(node);
        m_appClient->focusedNodeChanged(&n->wkc());
        delete n;
    }
}

WebCore::Page*
ChromeClientWKC::createWindow(WebCore::Frame* frame, const WebCore::FrameLoadRequest& request, const WebCore::WindowFeatures& features, const WebCore::NavigationAction& navact)
{
    FramePrivate fp(frame);
    FrameLoadRequestPrivate req(request);
    WKC::Page* ret = m_appClient->createWindow(&fp.wkc(), req.wkc(), (const WKC::WindowFeatures &)features);
    if (!ret)
        return 0;
    return (WebCore::Page *)ret->priv().webcore();
}
void
ChromeClientWKC::show()
{
    m_appClient->show();
}

bool
ChromeClientWKC::canRunModal()
{
    return m_appClient->canRunModal();
}
void
ChromeClientWKC::runModal()
{
    m_appClient->runModal();
}

void
ChromeClientWKC::setToolbarsVisible(bool visible)
{
    m_appClient->setToolbarsVisible(visible);
}
bool
ChromeClientWKC::toolbarsVisible()
{
    return m_appClient->toolbarsVisible();
}

void
ChromeClientWKC::setStatusbarVisible(bool visible)
{
    m_appClient->setStatusbarVisible(visible);
}
bool
ChromeClientWKC::statusbarVisible()
{
    return m_appClient->statusbarVisible();
}

void
ChromeClientWKC::setScrollbarsVisible(bool visible)
{
    m_appClient->setScrollbarsVisible(visible);
}
bool
ChromeClientWKC::scrollbarsVisible()
{
    return m_appClient->scrollbarsVisible();
}

void
ChromeClientWKC::setMenubarVisible(bool visible)
{
    m_appClient->setMenubarVisible(visible);
}
bool
ChromeClientWKC::menubarVisible()
{
    return m_appClient->menubarVisible();
}

void
ChromeClientWKC::setResizable(bool flag)
{
    m_appClient->setResizable(flag);
}

void
ChromeClientWKC::addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type,
                                  WebCore::MessageLevel level, const WTF::String& message,
                                  unsigned int lineNumber, const WTF::String& sourceID)
{
    m_appClient->addMessageToConsole((WKC::MessageSource)source, (WKC::MessageType)type, (WKC::MessageLevel)level, message, lineNumber, sourceID);
}

bool
ChromeClientWKC::canRunBeforeUnloadConfirmPanel()
{
    return m_appClient->canRunBeforeUnloadConfirmPanel();
}
bool
ChromeClientWKC::runBeforeUnloadConfirmPanel(const WTF::String& message, WebCore::Frame* frame)
{
    FramePrivate fp(frame);
    return m_appClient->runBeforeUnloadConfirmPanel(message, &fp.wkc());
}

void
ChromeClientWKC::closeWindowSoon()
{
	wkcWebView()->stopLoading();
	wkcWebView()->mainFrame()->privateFrame()->core()->page()->setGroupName(WTF::String());
	m_appClient->closeWindowSoon();
}

void
ChromeClientWKC::runJavaScriptAlert(WebCore::Frame* frame, const WTF::String& string)
{
    FramePrivate fp(frame);
    m_appClient->runJavaScriptAlert(&fp.wkc(), string);
}
bool
ChromeClientWKC::runJavaScriptConfirm(WebCore::Frame* frame, const WTF::String& string)
{
    FramePrivate fp(frame);
    return m_appClient->runJavaScriptConfirm(&fp.wkc(), string);
}
bool
ChromeClientWKC::runJavaScriptPrompt(WebCore::Frame* frame, const WTF::String& message, const WTF::String& defaultvalue, WTF::String& out_result)
{
    WKC::String result("");
    FramePrivate fp(frame);
    bool ret = m_appClient->runJavaScriptPrompt(&fp.wkc(), message, defaultvalue, result);
    out_result = result;
    return ret;
}
void
ChromeClientWKC::setStatusbarText(const WTF::String& string)
{
    m_appClient->setStatusbarText(string);
}
bool
ChromeClientWKC::shouldInterruptJavaScript()
{
    return m_appClient->shouldInterruptJavaScript();
}

#if ENABLE(REGISTER_PROTOCOL_HANDLER)
void
ChromeClientWKC::registerProtocolHandler(const WTF::String& scheme, const WTF::String& baseURL, const WTF::String& url, const WTF::String& title)
{
    m_appClient->registerProtocolHandler(scheme, baseURL, url, title);
}
#endif

WebCore::IntRect
ChromeClientWKC::windowResizerRect() const
{
    return m_appClient->windowResizerRect();
}

void
ChromeClientWKC::invalidateContentsAndRootView(const WebCore::IntRect& rect, bool immediate)
{
    m_view->updateOverlay(rect, immediate);
    m_appClient->repaint(rect, true /*contentChanged*/, immediate, true /*repaintContentOnly*/);
}

void
ChromeClientWKC::invalidateRootView(const WebCore::IntRect& rect, bool immediate)
{
    m_view->updateOverlay(rect, immediate);
    m_appClient->repaint(rect, false /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void
ChromeClientWKC::invalidateContentsForSlowScroll(const WebCore::IntRect& rect, bool immediate)
{
    m_appClient->invalidateContentsForSlowScroll(rect, immediate);
}

void
ChromeClientWKC::scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect)
{
    m_appClient->scroll(scrollDelta, rectToScroll, clipRect);
}
WebCore::IntPoint
ChromeClientWKC::screenToRootView(const WebCore::IntPoint& pos) const
{
    return pos;
}
WebCore::IntRect
ChromeClientWKC::rootViewToScreen(const WebCore::IntRect& rect) const
{
    return rect;
}
PlatformPageClient
ChromeClientWKC::platformPageClient() const
{
    // This code relate to the PopupMenuWKC.
    // If you modified this code, check to the PopupMenuWKC also.
    return (PlatformPageClient)m_view;
}
void
ChromeClientWKC::contentsSizeChanged(WebCore::Frame* frame, const WebCore::IntSize& size) const
{
    WKCSize s = { size.width(), size.height() };
    FramePrivate fp(frame);
    m_appClient->contentsSizeChanged(&fp.wkc(), s);
}

#if ENABLE(REQUEST_ANIMATION_FRAME) && !USE(REQUEST_ANIMATION_FRAME_TIMER)
void
ChromeClientWKC::scheduleAnimation()
{
    m_appClient->scheduleAnimation();
}
#endif

void
ChromeClientWKC::scrollbarsModeDidChange() const
{
    m_appClient->scrollbarsModeDidChange();
}
void
ChromeClientWKC::mouseDidMoveOverElement(const WebCore::HitTestResult& result, unsigned modifierFlags)
{
    HitTestResultPrivate r(result);
    m_appClient->mouseDidMoveOverElement(r.wkc(), modifierFlags);
}

void
ChromeClientWKC::setToolTip(const WTF::String& string, WebCore::TextDirection dir)
{
    m_appClient->setToolTip(string, (WKC::TextDirection)dir);
}

void
ChromeClientWKC::print(WebCore::Frame* frame)
{
    FramePrivate f(frame);
    m_appClient->print(&f.wkc());
}
#if ENABLE(SQL_DATABASE)
void
ChromeClientWKC::exceededDatabaseQuota(WebCore::Frame* frame, const WTF::String& string)
{
    FramePrivate f(frame);
    m_appClient->exceededDatabaseQuota(&f.wkc(), string);
}
#endif
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
void
ChromeClientWKC::reachedMaxAppCacheSize(int64_t spaceNeeded)
{
    m_appClient->reachedMaxAppCacheSize(spaceNeeded);
}

void 
ChromeClientWKC::reachedApplicationCacheOriginQuota(WebCore::SecurityOrigin* origin, int64_t totalSpaceNeeded)
{
    SecurityOriginPrivate o(origin);
    m_appClient->reachedApplicationCacheOriginQuota(&o.wkc(), totalSpaceNeeded);
}
#endif

void
ChromeClientWKC::runOpenPanel(WebCore::Frame* frame, PassRefPtr<WebCore::FileChooser> chooser)
{
    FramePrivate fp(frame);
    FileChooserPrivate fc(chooser.get());
    m_appClient->runOpenPanel(&fp.wkc(), &fc.wkc());
}

void
ChromeClientWKC::formStateDidChange(const WebCore::Node* node)
{
    if (!node) {
        m_appClient->formStateDidChange(0);
    } else {
        NodePrivate* n = NodePrivate::create(node);
        m_appClient->formStateDidChange(&n->wkc());
        delete n;
    }
}

void
ChromeClientWKC::elementDidFocus(const WebCore::Node* node)
{
    if (!node) {
        m_appClient->elementDidFocus(0);
    } else {
        NodePrivate* n = NodePrivate::create(node);
        m_appClient->elementDidFocus(&n->wkc());
        delete n;
    }
}

void
ChromeClientWKC::elementDidBlur(const WebCore::Node* node)
{
    if (!node) {
        m_appClient->elementDidBlur(0);
    } else {
        NodePrivate* n = NodePrivate::create(node);
        m_appClient->elementDidBlur(&n->wkc());
        delete n;
    }
}

void
ChromeClientWKC::setCursor(const WebCore::Cursor& handle)
{
    WKCPlatformCursor* p = reinterpret_cast<WKCPlatformCursor*>(handle.impl());
    m_appClient->setCursor(p);
}

void
ChromeClientWKC::setCursorHiddenUntilMouseMoves(bool)
{
    notImplemented();
}

void
ChromeClientWKC::scrollRectIntoView(const WebCore::IntRect& rect) const
{
    m_appClient->scrollRectIntoView(rect);
}

void
ChromeClientWKC::focusedFrameChanged(WebCore::Frame* frame)
{
    FramePrivate fp(frame);
    m_appClient->focusedFrameChanged(&fp.wkc());
}

WebCore::KeyboardUIMode
ChromeClientWKC::keyboardUIMode()
{
    WKC::KeyboardUIMode wmode = m_appClient->keyboardUIMode();
    unsigned mode = WebCore::KeyboardAccessDefault;

    if (wmode&WKC::KeyboardAccessFull) {
        mode |= WebCore::KeyboardAccessFull;
    }
    if (wmode&WKC::KeyboardAccessTabsToLinks) {
        mode |= WebCore::KeyboardAccessTabsToLinks;
    }
    return (WebCore::KeyboardUIMode)mode;
}

bool
ChromeClientWKC::shouldRubberBandInDirection(WebCore::ScrollDirection dir) const
{
    WKC::ScrollDirection wdir = WKC::ScrollUp;
    switch (dir) {
    case WebCore::ScrollUp:
        wdir = WKC::ScrollUp;
        break;
    case WebCore::ScrollDown:
        wdir = WKC::ScrollDown;
        break;
    case WebCore::ScrollLeft:
        wdir = WKC::ScrollLeft;
        break;
    case WebCore::ScrollRight:
        wdir = WKC::ScrollRight;
        break;
    default:
        return false;
    }
    return m_appClient->shouldRubberBandInDirection(wdir);
}

bool
ChromeClientWKC::selectItemWritingDirectionIsNatural()
{
    return m_appClient->selectItemWritingDirectionIsNatural();
}

bool
ChromeClientWKC::selectItemAlignmentFollowsMenuWritingDirection()
{
    return m_appClient->selectItemAlignmentFollowsMenuWritingDirection();
}

bool
ChromeClientWKC::hasOpenedPopup() const
{
    return m_appClient->hasOpenedPopup();
}

PassRefPtr<WebCore::PopupMenu>
ChromeClientWKC::createPopupMenu(WebCore::PopupMenuClient* client) const
{
	return adoptRef(new WebCore::PopupMenuWKC(client));
}

PassRefPtr<WebCore::SearchPopupMenu>
ChromeClientWKC::createSearchPopupMenu(WebCore::PopupMenuClient* client) const
{
	return adoptRef(new WebCore::SearchPopupMenuWKC(client));
}

#if 0
bool
ChromeClientWKC::willAddTextFieldDecorationsTo(WebCore::HTMLInputElement*)
{
    // This function is called whenever a text field <input> is
    // created. The implementation should return true if it wants
    // to do something in addTextFieldDecorationsTo().
    return true;
}

void
ChromeClientWKC::addTextFieldDecorationsTo(WebCore::HTMLInputElement*)
{
}
#endif

void
ChromeClientWKC::numWheelEventHandlersChanged(unsigned num)
{
    m_appClient->numWheelEventHandlersChanged(num);
}

void
ChromeClientWKC::numTouchEventHandlersChanged(unsigned num)
{
    m_appClient->numTouchEventHandlersChanged(num);
}

void
ChromeClientWKC::dispatchViewportPropertiesDidChange(const WebCore::ViewportArguments& arg) const
{
    WKC::ViewportArguments warg;

    warg.initialScale = arg.initialScale;
    warg.minimumScale = arg.minimumScale;
    warg.maximumScale = arg.maximumScale;
    warg.width = arg.width;
    warg.height = arg.height;
    warg.targetDensityDpi = arg.targetDensityDpi;
    warg.userScalable = arg.userScalable;

    m_appClient->dispatchViewportDataDidChange(warg);
}

void
ChromeClientWKC::layoutUpdated(WebCore::Frame*) const
{
}

bool
ChromeClientWKC::shouldUnavailablePluginMessageBeButton(WebCore::RenderEmbeddedObject::PluginUnavailabilityReason reason) const
{
    return m_appClient->shouldUnavailablePluginMessageBeButton((int)reason);
}

void
ChromeClientWKC::unavailablePluginButtonClicked(WebCore::Element* el, WebCore::RenderEmbeddedObject::PluginUnavailabilityReason reason) const
{
    WKC::ElementPrivate we(el);
    m_appClient->unavailablePluginButtonClicked(&we.wkc(), (int)reason);
}

void
ChromeClientWKC::populateVisitedLinks()
{
    m_appClient->populateVisitedLinks();
}


WebCore::FloatRect
ChromeClientWKC::customHighlightRect(WebCore::Node* node, const WTF::AtomicString& type, const WebCore::FloatRect& lineRect)
{
    if (!node)
        return WebCore::FloatRect();
    WKC::NodePrivate* wn = WKC::NodePrivate::create(node);
    const WTF::String wt = type;
    WebCore::FloatRect ret = m_appClient->customHighlightRect(&wn->wkc(), wt, lineRect);
    delete wn;
    return ret;
}

void
ChromeClientWKC::paintCustomHighlight(WebCore::Node* node, const WTF::AtomicString& type, const WebCore::FloatRect& boxRect, const WebCore::FloatRect& lineRect, bool behindText, bool entireLine)
{
    if (!node)
        return;
    WKC::NodePrivate* wn = WKC::NodePrivate::create(node);
    const WTF::String wt = type;
    m_appClient->paintCustomHighlight(&wn->wkc(), wt, boxRect, lineRect, behindText, entireLine);
    delete wn;
}

bool
ChromeClientWKC::shouldReplaceWithGeneratedFileForUpload(const WTF::String& path, WTF::String& generatedFilename)
{
    WKC::String gf = generatedFilename;
    bool ret = m_appClient->shouldReplaceWithGeneratedFileForUpload(path, gf);
    generatedFilename = gf;
    return ret;

}

WTF::String
ChromeClientWKC::generateReplacementFile(const WTF::String& path)
{
    return m_appClient->generateReplacementFile(path);
}

bool
ChromeClientWKC::supportsFullscreenForNode(const WebCore::Node* node)
{
    if (!node)
        return m_appClient->supportsFullscreenForNode(0);

    WKC::NodePrivate* wn = WKC::NodePrivate::create(node);
    bool ret = m_appClient->supportsFullscreenForNode(&wn->wkc());
    delete wn;
    return ret;
}

void
ChromeClientWKC::enterFullscreenForNode(WebCore::Node* node)
{
    if (!node) {
        m_appClient->enterFullscreenForNode(0);
        return;
    }

    WKC::NodePrivate* wn = WKC::NodePrivate::create(node);
    m_appClient->enterFullscreenForNode(&wn->wkc());
    delete wn;
}

void
ChromeClientWKC::exitFullscreenForNode(WebCore::Node* node)
{
    if (!node) {
        m_appClient->exitFullscreenForNode(0);
        return;
    }
    WKC::NodePrivate* wn = WKC::NodePrivate::create(node);
    m_appClient->exitFullscreenForNode(&wn->wkc());
    delete wn;
}

bool
ChromeClientWKC::requiresFullscreenForVideoPlayback()
{
    return m_appClient->requiresFullscreenForVideoPlayback();
}

void
ChromeClientWKC::notifyScrollerThumbIsVisibleInRect(const WebCore::IntRect& rect)
{
    m_appClient->notifyScrollerThumbIsVisibleInRect(rect);
}

void
ChromeClientWKC::recommendedScrollbarStyleDidChange(int newStyle)
{
    m_appClient->recommendedScrollbarStyleDidChange(newStyle);
}

bool
ChromeClientWKC::shouldRunModalDialogDuringPageDismissal(const ChromeClient::DialogType& type, const WTF::String& dialogMessage, WebCore::FrameLoader::PageDismissalType dismissalType) const
{
    ChromeClientIf::DialogType wtype = WKC::ChromeClientIf::AlertDialog;
    switch (type) {
    case WebCore::ChromeClient::AlertDialog:
        wtype = WKC::ChromeClientIf::AlertDialog; break;
    case WebCore::ChromeClient::ConfirmDialog:
        wtype = WKC::ChromeClientIf::ConfirmDialog; break;
    case WebCore::ChromeClient::PromptDialog:
        wtype = WKC::ChromeClientIf::PromptDialog; break;
    case WebCore::ChromeClient::HTMLDialog:
        wtype = WKC::ChromeClientIf::HTMLDialog; break;
    default:
        return false;
    }

    return m_appClient->shouldRunModalDialogDuringPageDismissal(wtype, dialogMessage, (int)dismissalType);
}

#if ENABLE(TOUCH_EVENTS)
void
ChromeClientWKC::needTouchEvents(bool flag)
{
    m_appClient->needTouchEvents(flag);
}
#endif

#if ENABLE(DIRECTORY_UPLOAD)
void
ChromeClientWKC::enumerateChosenDirectory(WebCore::FileChooser* chooser)
{
    FileChooserPrivate fc(chooser);
    m_appClient->enumerateChosenDirectory(&fc.wkc());
}
#endif

bool
ChromeClientWKC::isSVGImageChromeClient() const
{
    return false;
}

// not implemented....

void
ChromeClientWKC::loadIconForFiles(const WTF::Vector<WTF::String>& icons, WebCore::FileIconLoader* loader)
{
    notImplemented();
}

bool
ChromeClientWKC::paintCustomOverhangArea(WebCore::GraphicsContext*, const WebCore::IntRect&, const WebCore::IntRect&, const WebCore::IntRect&)
{
    notImplemented();
    return false;
}

#if ENABLE(INPUT_TYPE_COLOR)
WTF::PassOwnPtr<WebCore::ColorChooser>
ChromeClientWKC::createColorChooser(WebCore::ColorChooserClient*, const WebCore::Color&)
{
    notImplemented();
    return nullptr;
}
#endif

#if USE(ACCELERATED_COMPOSITING)
void
ChromeClientWKC::attachRootGraphicsLayer(WebCore::Frame* frame, WebCore::GraphicsLayer* layer)
{
    WKC::FramePrivate wf(frame);
    if (layer) {
        // attach
        WKC::GraphicsLayerPrivate* wg = GraphicsLayerWKC_wkc(layer);
        m_appClient->attachRootGraphicsLayer(&wf.wkc(), &wg->wkc());
    } else {
        // detach
        m_appClient->attachRootGraphicsLayer(&wf.wkc(), 0);
    }
    m_view->setRootGraphicsLayer(layer);
}

void
ChromeClientWKC::setNeedsOneShotDrawingSynchronization()
{
    m_appClient->setNeedsOneShotDrawingSynchronization();
}

void
ChromeClientWKC::scheduleCompositingLayerSync()
{
    m_appClient->scheduleCompositingLayerSync();
}

bool
ChromeClientWKC::allowsAcceleratedCompositing() const
{
    return m_appClient->allowsAcceleratedCompositing();
}

WebCore::ChromeClient::CompositingTriggerFlags
ChromeClientWKC::allowedCompositingTriggers() const
{
    return static_cast<WebCore::ChromeClient::CompositingTriggerFlags>(m_appClient->allowedCompositingTriggers());
}
#endif

#if ENABLE(FULLSCREEN_API)

bool
ChromeClientWKC::supportsFullScreenForElement(const WebCore::Element* element, bool flag)
{
    WKC::ElementPrivate we(const_cast<WebCore::Element*>(element));
    return m_appClient->supportsFullScreenForElement(&we.wkc(), flag);
}

void
ChromeClientWKC::enterFullScreenForElement(WebCore::Element* element)
{
    if (!element)
        return;

    WKC::ElementPrivate we(element);
    m_appClient->enterFullScreenForElement(&we.wkc());
}

void
ChromeClientWKC::exitFullScreenForElement(WebCore::Element* element)
{
    if (!element)
        return;

    WKC::ElementPrivate we(element);
    m_appClient->exitFullScreenForElement(&we.wkc());
}

void
ChromeClientWKC::fullScreenRendererChanged(WebCore::RenderBox* box)
{
    notImplemented();
}

void
ChromeClientWKC::setRootFullScreenLayer(WebCore::GraphicsLayer* layer)
{
    notImplemented();
}
#endif

void
ChromeClientWKC::postAccessibilityNotification(WebCore::AccessibilityObject*, WebCore::AXObjectCache::AXNotification)
{
    notImplemented();
}

WKCWebView*
ChromeClientWKC::wkcWebView() const
{
    return m_view->parent();
}

} // namespace

