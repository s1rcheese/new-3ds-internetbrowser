/*
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef ChromeClientWKC_h
#define ChromeClientWKC_h

#include "ChromeClient.h"
#include "KURL.h"

namespace WKC {
class ChromeClientIf;
class WKCWebViewPrivate;
class WKCWebView;

class ChromeClientWKC : public WebCore::ChromeClient
{
public:
    static ChromeClientWKC* create(WKCWebViewPrivate* view);
    ~ChromeClientWKC();

    virtual void* webView() const;

    // callbacks
    virtual void chromeDestroyed();

    virtual void setWindowRect(const WebCore::FloatRect&);
    virtual WebCore::FloatRect windowRect();
    virtual WebCore::FloatRect pageRect();

    virtual void focus();
    virtual void unfocus();
    virtual bool canTakeFocus(WebCore::FocusDirection);
    virtual void takeFocus(WebCore::FocusDirection);
    virtual void focusedNodeChanged(WebCore::Node*);
    virtual void focusedFrameChanged(WebCore::Frame*);

    virtual WebCore::Page* createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&, const WebCore::NavigationAction&);
    virtual void show();

    virtual bool canRunModal();
    virtual void runModal();

    virtual void setToolbarsVisible(bool);
    virtual bool toolbarsVisible();

    virtual void setStatusbarVisible(bool);
    virtual bool statusbarVisible();

    virtual void setScrollbarsVisible(bool);
    virtual bool scrollbarsVisible();

    virtual void setMenubarVisible(bool);
    virtual bool menubarVisible();

    virtual void setResizable(bool);

    virtual void addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type,
                                     WebCore::MessageLevel level, const WTF::String& message,
                                     unsigned int lineNumber, const WTF::String& sourceID);

    virtual bool canRunBeforeUnloadConfirmPanel();
    virtual bool runBeforeUnloadConfirmPanel(const WTF::String& message, WebCore::Frame* frame);

    virtual void closeWindowSoon();

    virtual void runJavaScriptAlert(WebCore::Frame*, const WTF::String&);
    virtual bool runJavaScriptConfirm(WebCore::Frame*, const WTF::String&);
    virtual bool runJavaScriptPrompt(WebCore::Frame*, const WTF::String& message, const WTF::String& defaultValue, WTF::String& result);

    virtual void setStatusbarText(const WTF::String&);

    virtual bool shouldInterruptJavaScript();

    virtual WebCore::KeyboardUIMode keyboardUIMode();

#if ENABLE(REGISTER_PROTOCOL_HANDLER)
    virtual void registerProtocolHandler(const WTF::String& scheme, const WTF::String& baseURL, const WTF::String& url, const WTF::String& title);
#endif

    virtual WebCore::IntRect windowResizerRect() const;

    virtual void invalidateRootView(const WebCore::IntRect&, bool);
    virtual void invalidateContentsAndRootView(const WebCore::IntRect&, bool);
    virtual void invalidateContentsForSlowScroll(const WebCore::IntRect&, bool);

    virtual void scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect);

    virtual WebCore::IntPoint screenToRootView(const WebCore::IntPoint&) const;
    virtual WebCore::IntRect rootViewToScreen(const WebCore::IntRect&) const;

    virtual PlatformPageClient platformPageClient() const;

    virtual void scrollbarsModeDidChange() const;
    virtual void setCursor(const WebCore::Cursor&);
    virtual void setCursorHiddenUntilMouseMoves(bool);
#if ENABLE(REQUEST_ANIMATION_FRAME) && !USE(REQUEST_ANIMATION_FRAME_TIMER)
    virtual void scheduleAnimation();
#endif
    virtual void dispatchViewportPropertiesDidChange(const WebCore::ViewportArguments&) const;
    virtual void contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const;
    virtual void layoutUpdated(WebCore::Frame*) const;
    virtual void scrollRectIntoView(const WebCore::IntRect&) const;

    virtual bool shouldUnavailablePluginMessageBeButton(WebCore::RenderEmbeddedObject::PluginUnavailabilityReason) const;
    virtual void unavailablePluginButtonClicked(WebCore::Element*, WebCore::RenderEmbeddedObject::PluginUnavailabilityReason) const;

    virtual void mouseDidMoveOverElement(const WebCore::HitTestResult&, unsigned modifierFlags);

    virtual void setToolTip(const WTF::String&, WebCore::TextDirection);

    virtual void print(WebCore::Frame*);

    virtual bool shouldRubberBandInDirection(WebCore::ScrollDirection) const;

#if ENABLE(SQL_DATABASE)
    virtual void exceededDatabaseQuota(WebCore::Frame*, const WTF::String&);
#endif // ENABLE(SQL_DATABASE)
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded);
    virtual void reachedApplicationCacheOriginQuota(WebCore::SecurityOrigin* origin, int64_t totalSpaceNeeded);
#else
    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded) {}
    virtual void reachedApplicationCacheOriginQuota(WebCore::SecurityOrigin* origin, int64_t totalSpaceNeeded) {}
#endif // ENABLE(OFFLINE_WEB_APPLICATIONS)

    virtual void populateVisitedLinks();

    virtual WebCore::FloatRect customHighlightRect(WebCore::Node*, const WTF::AtomicString& type, const WebCore::FloatRect& lineRect);
    virtual void paintCustomHighlight(WebCore::Node*, const WTF::AtomicString& type, const WebCore::FloatRect& boxRect, const WebCore::FloatRect& lineRect,
            bool behindText, bool entireLine);

    virtual bool shouldReplaceWithGeneratedFileForUpload(const WTF::String& path, WTF::String& generatedFilename);
    virtual WTF::String generateReplacementFile(const WTF::String& path);

    virtual bool paintCustomOverhangArea(WebCore::GraphicsContext*, const WebCore::IntRect&, const WebCore::IntRect&, const WebCore::IntRect&);

#if ENABLE(INPUT_TYPE_COLOR)
    virtual WTF::PassOwnPtr<WebCore::ColorChooser> createColorChooser(WebCore::ColorChooserClient*, const WebCore::Color&);
#endif

    virtual void runOpenPanel(WebCore::Frame*, PassRefPtr<WebCore::FileChooser>);

    virtual void loadIconForFiles(const WTF::Vector<WTF::String>&, WebCore::FileIconLoader*);

#if ENABLE(DIRECTORY_UPLOAD)
    virtual void enumerateChosenDirectory(WebCore::FileChooser*);
#endif

    virtual void formStateDidChange(const WebCore::Node*);
    virtual void elementDidFocus(const WebCore::Node*);
    virtual void elementDidBlur(const WebCore::Node*);

#if USE(ACCELERATED_COMPOSITING)
    virtual void attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer*);
    virtual void setNeedsOneShotDrawingSynchronization();
    virtual void scheduleCompositingLayerSync();
    virtual bool allowsAcceleratedCompositing() const;
    virtual WebCore::ChromeClient::CompositingTriggerFlags allowedCompositingTriggers() const;
#endif

    virtual bool supportsFullscreenForNode(const WebCore::Node*);
    virtual void enterFullscreenForNode(WebCore::Node*);
    virtual void exitFullscreenForNode(WebCore::Node*);
    virtual bool requiresFullscreenForVideoPlayback();
#if ENABLE(FULLSCREEN_API)
    virtual bool supportsFullScreenForElement(const WebCore::Element*, bool);
    virtual void enterFullScreenForElement(WebCore::Element*);
    virtual void exitFullScreenForElement(WebCore::Element*);
    virtual void fullScreenRendererChanged(WebCore::RenderBox*);
    virtual void setRootFullScreenLayer(WebCore::GraphicsLayer*);
#endif

#if ENABLE(TOUCH_EVENTS)
    virtual void needTouchEvents(bool);
#endif

    virtual bool selectItemWritingDirectionIsNatural();
    virtual bool selectItemAlignmentFollowsMenuWritingDirection();

    virtual bool hasOpenedPopup() const;
    virtual PassRefPtr<WebCore::PopupMenu> createPopupMenu(WebCore::PopupMenuClient*) const;
    virtual PassRefPtr<WebCore::SearchPopupMenu> createSearchPopupMenu(WebCore::PopupMenuClient*) const;

#if 0
    virtual bool willAddTextFieldDecorationsTo(WebCore::HTMLInputElement*);
    virtual void addTextFieldDecorationsTo(WebCore::HTMLInputElement*);
#endif

    virtual void postAccessibilityNotification(WebCore::AccessibilityObject*, WebCore::AXObjectCache::AXNotification);

    virtual void notifyScrollerThumbIsVisibleInRect(const WebCore::IntRect&);
    virtual void recommendedScrollbarStyleDidChange(int /*newStyle*/);

    virtual bool shouldRunModalDialogDuringPageDismissal(const ChromeClient::DialogType&, const WTF::String& dialogMessage, WebCore::FrameLoader::PageDismissalType) const;

    virtual void numWheelEventHandlersChanged(unsigned);
    virtual void numTouchEventHandlersChanged(unsigned);

    virtual bool isSVGImageChromeClient() const;

private:
    ChromeClientWKC(WKCWebViewPrivate* view);
    bool construct();
    WKCWebView* wkcWebView() const;

private:
    WKCWebViewPrivate* m_view;
    WKC::ChromeClientIf* m_appClient;
    WebCore::KURL m_hHoveredLinkURL;
};

} // namespace

#endif // ChromeClientWKC_h
