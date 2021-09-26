/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * All rights reserved.
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef FrameLoaderClientWKC_h
#define FrameLoaderClientWKC_h

#include "FrameLoaderClient.h"
#include "FrameNetworkingContext.h"
#include "MainResourceLoader.h"
#include "ResourceResponse.h"
#include "PluginView.h"
#include "WKCEnums.h"
#include "WTFString.h"

#ifdef WKC_ENABLE_CUSTOMJS
#include <wkc/wkccustomjs.h>
#endif // WKC_ENABLE_CUSTOMJS

namespace WKC {

class FrameLoaderClientIf;
class WKCWebFramePrivate;
class WKCWebFrame;
class WKCPolicyDecision;

class FrameLoaderClientWKC : public WebCore::FrameLoaderClient {
public:
    static FrameLoaderClientWKC* create(WKCWebFramePrivate*);
    virtual ~FrameLoaderClientWKC();

    WKCWebFrame* webFrame() const;

    //
    // Inheritance FrameLoaderClient
    //
    /* hasHTMLView() */

    virtual void frameLoaderDestroyed();

    virtual bool hasWebView() const;
    virtual void makeRepresentation(WebCore::DocumentLoader*);
    virtual void forceLayout();
    virtual void forceLayoutForNonHTML();
    virtual void setCopiesOnScroll();
    virtual void detachedFromParent2();
    virtual void detachedFromParent3();
    virtual void assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&);
    virtual void dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long  identifier, WebCore::ResourceRequest&, const WebCore::ResourceResponse& redirectResponse);
    virtual bool shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier);
    virtual void dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&);
    virtual void dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long  identifier, const WebCore::AuthenticationChallenge&);
    virtual void dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long  identifier, const WebCore::ResourceResponse&);
    virtual void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long identifier, int lengthReceived);
    virtual void dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long  identifier);
    virtual void dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long  identifier, const WebCore::ResourceError&);
    virtual bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length);

    virtual void dispatchDidHandleOnloadEvents();
    virtual void dispatchDidReceiveServerRedirectForProvisionalLoad();
    virtual void dispatchDidCancelClientRedirect();
    virtual void dispatchWillPerformClientRedirect(const WebCore::KURL&, double, double);
    virtual void dispatchDidNavigateWithinPage();
    virtual void dispatchDidChangeLocationWithinPage();
    virtual void dispatchDidPushStateWithinPage();
    virtual void dispatchDidReplaceStateWithinPage();
    virtual void dispatchDidPopStateWithinPage();
    virtual void dispatchWillClose();
    virtual void dispatchDidReceiveIcon();
    virtual void dispatchDidStartProvisionalLoad();
    virtual void dispatchDidReceiveTitle(const WebCore::StringWithDirection&);
    virtual void dispatchDidChangeIcons(WebCore::IconType);
    virtual void dispatchDidCommitLoad();
    virtual void dispatchDidFailProvisionalLoad(const WebCore::ResourceError&);
    virtual void dispatchDidFailLoad(const WebCore::ResourceError&);
    virtual void dispatchDidFinishDocumentLoad();
    virtual void dispatchDidFinishLoad();
    virtual void dispatchDidFirstLayout();
    virtual void dispatchDidFirstVisuallyNonEmptyLayout();
    virtual void dispatchDidNewFirstVisuallyNonEmptyLayout();
    virtual void dispatchDidLayout();

    virtual WebCore::Frame* dispatchCreatePage(const WebCore::NavigationAction&);
    virtual void dispatchShow();

    virtual void dispatchDecidePolicyForResponse(WebCore::FramePolicyFunction, const WebCore::ResourceResponse&, const WebCore::ResourceRequest&);
    virtual void dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WTF::PassRefPtr<WebCore::FormState>, const WTF::String& frameName);
    virtual void dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WTF::PassRefPtr<WebCore::FormState>);
    virtual void cancelPolicyCheck();
    virtual void dispatchUnableToImplementPolicy(const WebCore::ResourceError&);

    virtual void dispatchWillSendSubmitEvent(WTF::PassRefPtr<WebCore::FormState>);
    virtual void dispatchWillSubmitForm(WebCore::FramePolicyFunction, WTF::PassRefPtr<WebCore::FormState>);

    virtual void revertToProvisionalState(WebCore::DocumentLoader*);
    virtual void setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&);

    // Maybe these should go into a ProgressTrackerClient some day
    virtual void willChangeEstimatedProgress();
    virtual void didChangeEstimatedProgress();
    virtual void postProgressStartedNotification();
    virtual void postProgressEstimateChangedNotification();
    virtual void postProgressFinishedNotification();

    virtual void setMainFrameDocumentReady(bool);

    virtual void startDownload(const WebCore::ResourceRequest&, const WTF::String& suggestedName);

    virtual void willChangeTitle(WebCore::DocumentLoader*);
    virtual void didChangeTitle(WebCore::DocumentLoader*);

    virtual void committedLoad(WebCore::DocumentLoader*, const char*, int);
    virtual void finishedLoading(WebCore::DocumentLoader*);

    virtual void updateGlobalHistory();
    virtual void updateGlobalHistoryRedirectLinks();
    virtual bool shouldGoToHistoryItem(WebCore::HistoryItem*) const;
    virtual bool shouldStopLoadingForHistoryItem(WebCore::HistoryItem*) const;
    virtual void updateGlobalHistoryItemForPage();

    // This frame has displayed inactive content (such as an image) from an
    // insecure source.  Inactive content cannot spread to other frames.
    virtual void didDisplayInsecureContent();
    // The indicated security origin has run active content (such as a
    // script) from an insecure source.  Note that the insecure content can
    // spread to other frames in the same origin.
    virtual void didRunInsecureContent(WebCore::SecurityOrigin*, const WebCore::KURL&);
    virtual void didDetectXSS(const WebCore::KURL&, bool didBlockEntirePage);

    virtual WebCore::ResourceError cancelledError(const WebCore::ResourceRequest&);
    virtual WebCore::ResourceError blockedError(const WebCore::ResourceRequest&);
    virtual WebCore::ResourceError cannotShowURLError(const WebCore::ResourceRequest&);
    virtual WebCore::ResourceError interruptedForPolicyChangeError(const WebCore::ResourceRequest&);
    virtual WebCore::ResourceError cannotShowMIMETypeError(const WebCore::ResourceResponse&);
    virtual WebCore::ResourceError fileDoesNotExistError(const WebCore::ResourceResponse&);
    virtual WebCore::ResourceError pluginWillHandleLoadError(const WebCore::ResourceResponse&);

    virtual bool shouldFallBack(const WebCore::ResourceError&);

    virtual bool canHandleRequest(const WebCore::ResourceRequest&) const;
    virtual bool canShowMIMEType(const WTF::String&) const;
    virtual bool canShowMIMETypeAsHTML(const WTF::String& MIMEType) const;
    virtual bool representationExistsForURLScheme(const WTF::String&) const;
    virtual WTF::String generatedMIMETypeForURLScheme(const WTF::String&) const;

    virtual void frameLoadCompleted();
    virtual void saveViewStateToItem(WebCore::HistoryItem*);
    virtual void restoreViewState();
    virtual void provisionalLoadStarted();
    virtual void didFinishLoad();
    virtual void prepareForDataSourceReplacement();

    virtual WTF::PassRefPtr<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest&, const WebCore::SubstituteData&);

    virtual void setTitle(const WebCore::StringWithDirection& title, const WebCore::KURL&);

    virtual WTF::String userAgent(const WebCore::KURL&);

    virtual void savePlatformDataToCachedFrame(WebCore::CachedFrame*);
    virtual void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*);
    virtual void transitionToCommittedForNewPage();

    virtual void didSaveToPageCache();
    virtual void didRestoreFromPageCache();

    virtual void dispatchDidBecomeFrameset(bool);

    virtual bool canCachePage() const;

    virtual void download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&);

    virtual PassRefPtr<WebCore::Frame> createFrame(const WebCore::KURL& url, const WTF::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
                                                           const WTF::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight);
    virtual PassRefPtr<WebCore::Widget> createPlugin(const WebCore::IntSize&, WebCore::HTMLPlugInElement*, const WebCore::KURL&, const WTF::Vector<WTF::String>&, const WTF::Vector<WTF::String>&, const WTF::String&, bool);
    virtual void redirectDataToPlugin(WebCore::Widget* pluginWidget);
    virtual PassRefPtr<WebCore::Widget> createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const WTF::Vector<WTF::String>& paramNames, const WTF::Vector<WTF::String>& paramValues);

    virtual WebCore::ObjectContentType objectContentType(const WebCore::KURL&, const WTF::String& mimeType, bool shouldPreferPlugInsForImages);
    virtual WTF::String overrideMediaType() const;

    virtual void dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld*);
    virtual void documentElementAvailable();
    virtual void didPerformFirstNavigation() const;

    virtual void registerForIconNotification(bool);

    virtual void didChangeScrollOffset();

    virtual bool allowScript(bool enabledPerSettings);
    virtual bool allowScriptFromSource(bool enabledPerSettings, const WebCore::KURL&);
    virtual bool allowPlugins(bool enabledPerSettings);
    virtual bool allowImage(bool enabledPerSettings, const WebCore::KURL&);
    virtual bool allowDisplayingInsecureContent(bool enabledPerSettings, WebCore::SecurityOrigin*, const WebCore::KURL&);
    virtual bool allowRunningInsecureContent(bool enabledPerSettings, WebCore::SecurityOrigin*, const WebCore::KURL&);
    // This callback notifies the client that the frame was about to run
    // JavaScript but did not because allowJavaScript returned false. We
    // have a separate callback here because there are a number of places
    // that need to know if JavaScript is enabled but are not necessarily
    // preparing to execute script.
    virtual void didNotAllowScript();
    // This callback is similar, but for plugins.
    virtual void didNotAllowPlugins();

    virtual bool shouldForceUniversalAccessFromLocalURL(const WebCore::KURL&);

    virtual WTF::PassRefPtr<WebCore::FrameNetworkingContext> createNetworkingContext();

#if ENABLE(WEB_INTENTS)
    virtual void dispatchIntent(WTF::PassRefPtr<WebCore::IntentRequest>);
#endif

    virtual void dispatchWillOpenSocketStream(WebCore::SocketStreamHandle*);
    virtual void dispatchGlobalObjectAvailable(WebCore::DOMWrapperWorld*);
    virtual void dispatchWillDisconnectDOMWindowExtensionFromGlobalObject(WebCore::DOMWindowExtension*);
    virtual void dispatchDidReconnectDOMWindowExtensionToGlobalObject(WebCore::DOMWindowExtension*);
    virtual void dispatchWillDestroyGlobalObjectForDOMWindowExtension(WebCore::DOMWindowExtension*);

    //
    // WKC extension
    //
public:
    bool byWKC(void) { return true; }
    virtual bool notifySSLHandshakeStatus(WebCore::ResourceHandle* handle, int status);
    virtual bool dispatchWillAcceptCookie(bool income, WebCore::ResourceHandle* handle, const WTF::String& url, const WTF::String& firstparty_host, const WTF::String& cookie_domain);
    virtual bool dispatchWillReceiveData(WebCore::ResourceHandle*, int length);
    virtual int  requestSSLClientCertSelect(WebCore::ResourceHandle* handle, const char* requester, void* certs, int num);
    virtual int  dispatchWillPermitSendRequest(WebCore::ResourceHandle* handle, const char* url, int composition, bool isSync, const WebCore::ResourceResponse& redirectResponse);

#ifdef WKC_ENABLE_CUSTOMJS
    virtual bool dispatchWillCallCustomJS(WKCCustomJSAPIList* api, void** context);
#endif // WKC_ENABLE_CUSTOMJS


private:
    FrameLoaderClientWKC(WKCWebFramePrivate*);
    bool construct();
    void notifyStatus(WKC::LoadStatus loadStatus);

private:
    WKCWebFramePrivate* m_frame;
    WKC::FrameLoaderClientIf* m_appClient;
    WebCore::ResourceResponse m_response;
    WKCPolicyDecision* m_policyDecision;

    // Plugin view to redirect data to
    WebCore::PluginView* m_pluginView;
    bool m_hasSentResponseToPlugin;
};

class FrameNetworkingContextWKC : public WebCore::FrameNetworkingContext
{
public:
    static FrameNetworkingContextWKC* create(WebCore::Frame*);
public:
    virtual WebCore::MainResourceLoader* mainResourceLoader() const;
    virtual WebCore::FrameLoaderClient* frameLoaderClient() const;

public:
    WebCore::Frame* coreFrame(){ return frame(); }

private:
    FrameNetworkingContextWKC(WebCore::Frame*);
};

} // namespace

#endif // FrameLoaderClientWKC_h
