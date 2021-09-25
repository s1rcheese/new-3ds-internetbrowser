/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (c) 2012-2014 ACCESS CO., LTD. All rights reserved.
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
#include "FocusController.h"

#include "AXObjectCache.h"
#include "Chrome.h"
#include "ComposedShadowTreeWalker.h"
#include "Document.h"
#include "Editor.h"
#include "EditorClient.h"
#include "Element.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "Frame.h"
#include "FrameSelection.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "HTMLAreaElement.h"
#include "HTMLImageElement.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "KeyboardEvent.h"
#include "Page.h"
#include "Range.h"
#include "RenderObject.h"
#include "RenderWidget.h"
#include "ScrollAnimator.h"
#include "Settings.h"
#include "ShadowRoot.h"
#include "ShadowTree.h"
#include "SpatialNavigation.h"
#include "Widget.h"
#include "htmlediting.h" // For firstPositionInOrBeforeNode
#include <limits>

#if PLATFORM(WKC)
#include "RenderLayer.h"
#endif

namespace WebCore {

using namespace HTMLNames;
using namespace std;

static inline bool isShadowHost(const Node* node)
{
    return node && node->isElementNode() && toElement(node)->hasShadowRoot();
}

static inline ComposedShadowTreeWalker walkerFrom(const Node* node)
{
    return ComposedShadowTreeWalker(node, ComposedShadowTreeWalker::DoNotCrossUpperBoundary);
}

static inline ComposedShadowTreeWalker walkerFromNext(const Node* node)
{
    ComposedShadowTreeWalker walker = ComposedShadowTreeWalker(node, ComposedShadowTreeWalker::DoNotCrossUpperBoundary);
    walker.next();
    return walker;
}

static inline ComposedShadowTreeWalker walkerFromPrevious(const Node* node)
{
    ComposedShadowTreeWalker walker = ComposedShadowTreeWalker(node, ComposedShadowTreeWalker::DoNotCrossUpperBoundary);
    walker.previous();
    return walker;
}

static inline Node* nextNode(const Node* node)
{
    return walkerFromNext(node).get();
}

static inline Node* previousNode(const Node* node)
{
    return walkerFromPrevious(node).get();
}

FocusScope::FocusScope(TreeScope* treeScope)
    : m_rootTreeScope(treeScope)
{
    ASSERT(treeScope);
    ASSERT(!treeScope->rootNode()->isShadowRoot() || toShadowRoot(treeScope->rootNode())->isYoungest());
}

Node* FocusScope::rootNode() const
{
    return m_rootTreeScope->rootNode();
}

Element* FocusScope::owner() const
{
    Node* root = rootNode();
    if (root->isShadowRoot())
        return root->shadowHost();
    if (Frame* frame = root->document()->frame())
        return frame->ownerElement();
    return 0;
}

FocusScope FocusScope::focusScopeOf(Node* node)
{
    ASSERT(node);
    TreeScope* scope = node->treeScope();
    if (scope->rootNode()->isShadowRoot())
        return FocusScope(toShadowRoot(scope->rootNode())->tree()->youngestShadowRoot());
    return FocusScope(scope);
}

FocusScope FocusScope::focusScopeOwnedByShadowHost(Node* node)
{
    ASSERT(isShadowHost(node));
    return FocusScope(toElement(node)->shadowTree()->youngestShadowRoot());
}

FocusScope FocusScope::focusScopeOwnedByIFrame(HTMLFrameOwnerElement* frame)
{
    ASSERT(frame && frame->contentFrame());
    return FocusScope(frame->contentFrame()->document());
}

static inline void dispatchEventsOnWindowAndFocusedNode(Document* document, bool focused)
{
    // If we have a focused node we should dispatch blur on it before we blur the window.
    // If we have a focused node we should dispatch focus on it after we focus the window.
    // https://bugs.webkit.org/show_bug.cgi?id=27105

    // Do not fire events while modal dialogs are up.  See https://bugs.webkit.org/show_bug.cgi?id=33962
    if (Page* page = document->page()) {
        if (page->defersLoading())
            return;
    }

    if (!focused && document->focusedNode())
        document->focusedNode()->dispatchBlurEvent(0);
    document->dispatchWindowEvent(Event::create(focused ? eventNames().focusEvent : eventNames().blurEvent, false, false));
    if (focused && document->focusedNode())
        document->focusedNode()->dispatchFocusEvent(0);
}

static inline bool hasCustomFocusLogic(Node* node)
{
    return node->hasTagName(inputTag) || node->hasTagName(textareaTag) || node->hasTagName(videoTag) || node->hasTagName(audioTag);
}

static inline bool isNonFocusableShadowHost(Node* node, KeyboardEvent* event)
{
    ASSERT(node);
    return !node->isKeyboardFocusable(event) && isShadowHost(node) && !hasCustomFocusLogic(node);
}

static inline bool isFocusableShadowHost(Node* node, KeyboardEvent* event)
{
    ASSERT(node);
    return node->isKeyboardFocusable(event) && isShadowHost(node) && !hasCustomFocusLogic(node);
}

static inline int adjustedTabIndex(Node* node, KeyboardEvent* event)
{
    ASSERT(node);
    return isNonFocusableShadowHost(node, event) ? 0 : node->tabIndex();
}

static inline bool shouldVisit(Node* node, KeyboardEvent* event)
{
    ASSERT(node);
    return node->isKeyboardFocusable(event) || isNonFocusableShadowHost(node, event);
}

FocusController::FocusController(Page* page)
    : m_page(page)
    , m_isActive(false)
    , m_isFocused(false)
    , m_isChangingFocusedFrame(false)
    , m_containingWindowIsVisible(false)
#if PLATFORM(WKC)
    , m_lastExitRect(0,0,0,0)
    , m_lastEntryRect(0,0,0,0)
    , m_lastDirection(FocusDirectionNone)
#endif
{
}

PassOwnPtr<FocusController> FocusController::create(Page* page)
{
    return adoptPtr(new FocusController(page));
}

void FocusController::setFocusedFrame(PassRefPtr<Frame> frame)
{
    ASSERT(!frame || frame->page() == m_page);
    if (m_focusedFrame == frame || m_isChangingFocusedFrame)
        return;

    m_isChangingFocusedFrame = true;

    RefPtr<Frame> oldFrame = m_focusedFrame;
    RefPtr<Frame> newFrame = frame;

    m_focusedFrame = newFrame;

    // Now that the frame is updated, fire events and update the selection focused states of both frames.
    if (oldFrame && oldFrame->view()) {
        oldFrame->selection()->setFocused(false);
        oldFrame->document()->dispatchWindowEvent(Event::create(eventNames().blurEvent, false, false));
    }

    if (newFrame && newFrame->view() && isFocused()) {
        newFrame->selection()->setFocused(true);
        newFrame->document()->dispatchWindowEvent(Event::create(eventNames().focusEvent, false, false));
    }

    m_page->chrome()->focusedFrameChanged(newFrame.get());

    m_isChangingFocusedFrame = false;
}

Frame* FocusController::focusedOrMainFrame() const
{
    if (Frame* frame = focusedFrame())
        return frame;
    return m_page->mainFrame();
}

#if PLATFORM(WKC)
void FocusController::setFocused(bool focused, bool dispatchEvents)
#else
void FocusController::setFocused(bool focused)
#endif
{
    if (isFocused() == focused)
        return;
    
    m_isFocused = focused;

    if (!m_isFocused)
        focusedOrMainFrame()->eventHandler()->stopAutoscrollTimer();

    if (!m_focusedFrame)
        setFocusedFrame(m_page->mainFrame());

    if (m_focusedFrame->view()) {
        m_focusedFrame->selection()->setFocused(focused);

#if PLATFORM(WKC)
        if (dispatchEvents){
#endif
        dispatchEventsOnWindowAndFocusedNode(m_focusedFrame->document(), focused);
#if PLATFORM(WKC)
        }
#endif
    }
}

Node* FocusController::findFocusableNodeDecendingDownIntoFrameDocument(FocusDirection direction, Node* node, KeyboardEvent* event)
{
    // The node we found might be a HTMLFrameOwnerElement, so descend down the tree until we find either:
    // 1) a focusable node, or
    // 2) the deepest-nested HTMLFrameOwnerElement.
    while (node && node->isFrameOwnerElement()) {
        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
        if (!owner->contentFrame())
            break;
        Node* foundNode = findFocusableNode(direction, FocusScope::focusScopeOwnedByIFrame(owner), 0, event);
        if (!foundNode)
            break;
        ASSERT(node != foundNode);
        node = foundNode;
    }
    return node;
}

bool FocusController::setInitialFocus(FocusDirection direction, KeyboardEvent* event)
{
    bool didAdvanceFocus = advanceFocus(direction, event, true);
    
    // If focus is being set initially, accessibility needs to be informed that system focus has moved 
    // into the web area again, even if focus did not change within WebCore. PostNotification is called instead
    // of handleFocusedUIElementChanged, because this will send the notification even if the element is the same.
    if (AXObjectCache::accessibilityEnabled())
        focusedOrMainFrame()->document()->axObjectCache()->postNotification(focusedOrMainFrame()->document()->renderer(), AXObjectCache::AXFocusedUIElementChanged, true);

    return didAdvanceFocus;
}

bool FocusController::advanceFocus(FocusDirection direction, KeyboardEvent* event, bool initialFocus)
{
#if !PLATFORM(WKC) || defined(WKC_ENABLE_WEBKIT_SPATIAL_NAVIGATION)
    switch (direction) {
    case FocusDirectionForward:
    case FocusDirectionBackward:
        return advanceFocusInDocumentOrder(direction, event, initialFocus);
    case FocusDirectionLeft:
    case FocusDirectionRight:
    case FocusDirectionUp:
    case FocusDirectionDown:
        return advanceFocusDirectionally(direction, event);
    default:
        ASSERT_NOT_REACHED();
    }

    return false;
#else
    return false;
#endif
}

bool FocusController::advanceFocusInDocumentOrder(FocusDirection direction, KeyboardEvent* event, bool initialFocus)
{
    Frame* frame = focusedOrMainFrame();
    ASSERT(frame);
    Document* document = frame->document();

    Node* currentNode = document->focusedNode();
    // FIXME: Not quite correct when it comes to focus transitions leaving/entering the WebView itself
    bool caretBrowsing = frame->settings() && frame->settings()->caretBrowsingEnabled();

    if (caretBrowsing && !currentNode)
        currentNode = frame->selection()->start().deprecatedNode();

    document->updateLayoutIgnorePendingStylesheets();

    RefPtr<Node> node = findFocusableNodeAcrossFocusScope(direction, FocusScope::focusScopeOf(currentNode ? currentNode : document), currentNode, event);

    if (!node) {
        // We didn't find a node to focus, so we should try to pass focus to Chrome.
        if (!initialFocus && m_page->chrome()->canTakeFocus(direction)) {
            document->setFocusedNode(0);
            setFocusedFrame(0);
            m_page->chrome()->takeFocus(direction);
            return true;
        }

        // Chrome doesn't want focus, so we should wrap focus.
        node = findFocusableNodeRecursively(direction, FocusScope::focusScopeOf(m_page->mainFrame()->document()), 0, event);
        node = findFocusableNodeDecendingDownIntoFrameDocument(direction, node.get(), event);

        if (!node)
            return false;
    }

    ASSERT(node);

    if (node == document->focusedNode())
        // Focus wrapped around to the same node.
        return true;

    if (!node->isElementNode())
        // FIXME: May need a way to focus a document here.
        return false;

    if (node->isFrameOwnerElement()) {
        // We focus frames rather than frame owners.
        // FIXME: We should not focus frames that have no scrollbars, as focusing them isn't useful to the user.
        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node.get());
        if (!owner->contentFrame())
            return false;

        document->setFocusedNode(0);
        setFocusedFrame(owner->contentFrame());
        return true;
    }
    
    // FIXME: It would be nice to just be able to call setFocusedNode(node) here, but we can't do
    // that because some elements (e.g. HTMLInputElement and HTMLTextAreaElement) do extra work in
    // their focus() methods.

    Document* newDocument = node->document();

    if (newDocument != document)
        // Focus is going away from this document, so clear the focused node.
        document->setFocusedNode(0);

    if (newDocument)
        setFocusedFrame(newDocument->frame());

    if (caretBrowsing) {
        Position position = firstPositionInOrBeforeNode(node.get());
        VisibleSelection newSelection(position, position, DOWNSTREAM);
        if (frame->selection()->shouldChangeSelection(newSelection))
            frame->selection()->setSelection(newSelection);
    }

    static_cast<Element*>(node.get())->focus(false);
    return true;
}

Node* FocusController::findFocusableNodeAcrossFocusScope(FocusDirection direction, FocusScope scope, Node* currentNode, KeyboardEvent* event)
{
    ASSERT(!currentNode || !isNonFocusableShadowHost(currentNode, event));
    Node* found;
    if (currentNode && direction == FocusDirectionForward && isFocusableShadowHost(currentNode, event)) {
        Node* foundInInnerFocusScope = findFocusableNodeRecursively(direction, FocusScope::focusScopeOwnedByShadowHost(currentNode), 0, event);
        found = foundInInnerFocusScope ? foundInInnerFocusScope : findFocusableNodeRecursively(direction, scope, currentNode, event);
    } else
        found = findFocusableNodeRecursively(direction, scope, currentNode, event);

    // If there's no focusable node to advance to, move up the focus scopes until we find one.
    while (!found) {
        Node* owner = scope.owner();
        if (!owner)
            break;
        scope = FocusScope::focusScopeOf(owner);
        if (direction == FocusDirectionBackward && isFocusableShadowHost(owner, event)) {
            found = owner;
            break;
        }
        found = findFocusableNodeRecursively(direction, scope, owner, event);
    }
    found = findFocusableNodeDecendingDownIntoFrameDocument(direction, found, event);
    return found;
}

Node* FocusController::findFocusableNodeRecursively(FocusDirection direction, FocusScope scope, Node* start, KeyboardEvent* event)
{
    // Starting node is exclusive.
    Node* found = findFocusableNode(direction, scope, start, event);
    if (!found)
        return 0;
    if (direction == FocusDirectionForward) {
        if (!isNonFocusableShadowHost(found, event))
            return found;
        Node* foundInInnerFocusScope = findFocusableNodeRecursively(direction, FocusScope::focusScopeOwnedByShadowHost(found), 0, event);
        return foundInInnerFocusScope ? foundInInnerFocusScope : findFocusableNodeRecursively(direction, scope, found, event);
    }
    ASSERT(direction == FocusDirectionBackward);
    if (isFocusableShadowHost(found, event)) {
        Node* foundInInnerFocusScope = findFocusableNodeRecursively(direction, FocusScope::focusScopeOwnedByShadowHost(found), 0, event);
        return foundInInnerFocusScope ? foundInInnerFocusScope : found;
    }
    if (isNonFocusableShadowHost(found, event)) {
        Node* foundInInnerFocusScope = findFocusableNodeRecursively(direction, FocusScope::focusScopeOwnedByShadowHost(found), 0, event);
        return foundInInnerFocusScope ? foundInInnerFocusScope :findFocusableNodeRecursively(direction, scope, found, event);
    }
    return found;
}

Node* FocusController::findFocusableNode(FocusDirection direction, FocusScope scope, Node* node, KeyboardEvent* event)
{
    return (direction == FocusDirectionForward)
        ? nextFocusableNode(scope, node, event)
        : previousFocusableNode(scope, node, event);
}

Node* FocusController::findNodeWithExactTabIndex(Node* start, int tabIndex, KeyboardEvent* event, FocusDirection direction)
{
    // Search is inclusive of start
    for (ComposedShadowTreeWalker walker = walkerFrom(start); walker.get(); direction == FocusDirectionForward ? walker.next() : walker.previous()) {
        if (shouldVisit(walker.get(), event) && adjustedTabIndex(walker.get(), event) == tabIndex)
            return walker.get();
    }
    return 0;
}

static Node* nextNodeWithGreaterTabIndex(Node* start, int tabIndex, KeyboardEvent* event)
{
    // Search is inclusive of start
    int winningTabIndex = std::numeric_limits<short>::max() + 1;
    Node* winner = 0;
    for (ComposedShadowTreeWalker walker = walkerFrom(start); walker.get(); walker.next()) {
        Node* node = walker.get();
        if (shouldVisit(node, event) && node->tabIndex() > tabIndex && node->tabIndex() < winningTabIndex) {
            winner = node;
            winningTabIndex = node->tabIndex();
        }
    }

    return winner;
}

static Node* previousNodeWithLowerTabIndex(Node* start, int tabIndex, KeyboardEvent* event)
{
    // Search is inclusive of start
    int winningTabIndex = 0;
    Node* winner = 0;
    for (ComposedShadowTreeWalker walker = walkerFrom(start); walker.get(); walker.previous()) {
        Node* node = walker.get();
        int currentTabIndex = adjustedTabIndex(node, event);
        if ((shouldVisit(node, event) || isNonFocusableShadowHost(node, event)) && currentTabIndex < tabIndex && currentTabIndex > winningTabIndex) {
            winner = node;
            winningTabIndex = currentTabIndex;
        }
    }
    return winner;
}

Node* FocusController::nextFocusableNode(FocusScope scope, Node* start, KeyboardEvent* event)
{
    if (start) {
        int tabIndex = adjustedTabIndex(start, event);
        // If a node is excluded from the normal tabbing cycle, the next focusable node is determined by tree order
        if (tabIndex < 0) {
            for (ComposedShadowTreeWalker walker = walkerFromNext(start); walker.get(); walker.next()) {
                if (shouldVisit(walker.get(), event) && adjustedTabIndex(walker.get(), event) >= 0)
                    return walker.get();
            }
        }

        // First try to find a node with the same tabindex as start that comes after start in the scope.
        if (Node* winner = findNodeWithExactTabIndex(nextNode(start), tabIndex, event, FocusDirectionForward))
            return winner;

        if (!tabIndex)
            // We've reached the last node in the document with a tabindex of 0. This is the end of the tabbing order.
            return 0;
    }

    // Look for the first node in the scope that:
    // 1) has the lowest tabindex that is higher than start's tabindex (or 0, if start is null), and
    // 2) comes first in the scope, if there's a tie.
    if (Node* winner = nextNodeWithGreaterTabIndex(scope.rootNode(), start ? adjustedTabIndex(start, event) : 0, event))
        return winner;

    // There are no nodes with a tabindex greater than start's tabindex,
    // so find the first node with a tabindex of 0.
    return findNodeWithExactTabIndex(scope.rootNode(), 0, event, FocusDirectionForward);
}

Node* FocusController::previousFocusableNode(FocusScope scope, Node* start, KeyboardEvent* event)
{
    Node* last = 0;
    for (ComposedShadowTreeWalker walker = walkerFrom(scope.rootNode()); walker.get(); walker.lastChild())
        last = walker.get();
    ASSERT(last);

    // First try to find the last node in the scope that comes before start and has the same tabindex as start.
    // If start is null, find the last node in the scope with a tabindex of 0.
    Node* startingNode;
    int startingTabIndex;
    if (start) {
        startingNode = previousNode(start);
        startingTabIndex = adjustedTabIndex(start, event);
    } else {
        startingNode = last;
        startingTabIndex = 0;
    }

    // However, if a node is excluded from the normal tabbing cycle, the previous focusable node is determined by tree order
    if (startingTabIndex < 0) {
        for (ComposedShadowTreeWalker walker = walkerFrom(startingNode); walker.get(); walker.previous()) {
            if (shouldVisit(walker.get(), event) && adjustedTabIndex(walker.get(), event) >= 0)
                return walker.get();
        }
    }

    if (Node* winner = findNodeWithExactTabIndex(startingNode, startingTabIndex, event, FocusDirectionBackward))
        return winner;

    // There are no nodes before start with the same tabindex as start, so look for a node that:
    // 1) has the highest non-zero tabindex (that is less than start's tabindex), and
    // 2) comes last in the scope, if there's a tie.
    startingTabIndex = (start && startingTabIndex) ? startingTabIndex : std::numeric_limits<short>::max();
    return previousNodeWithLowerTabIndex(last, startingTabIndex, event);
}

static bool relinquishesEditingFocus(Node *node)
{
    ASSERT(node);
    ASSERT(node->rendererIsEditable());

    Node* root = node->rootEditableElement();
    Frame* frame = node->document()->frame();
    if (!frame || !root)
        return false;

    return frame->editor()->shouldEndEditing(rangeOfContents(root).get());
}

static void clearSelectionIfNeeded(Frame* oldFocusedFrame, Frame* newFocusedFrame, Node* newFocusedNode)
{
    if (!oldFocusedFrame || !newFocusedFrame)
        return;
        
    if (oldFocusedFrame->document() != newFocusedFrame->document())
        return;
    
    FrameSelection* s = oldFocusedFrame->selection();
    if (s->isNone())
        return;

    bool caretBrowsing = oldFocusedFrame->settings()->caretBrowsingEnabled();
    if (caretBrowsing)
        return;

    Node* selectionStartNode = s->selection().start().deprecatedNode();
    if (selectionStartNode == newFocusedNode || selectionStartNode->isDescendantOf(newFocusedNode) || selectionStartNode->shadowAncestorNode() == newFocusedNode)
        return;
        
    if (Node* mousePressNode = newFocusedFrame->eventHandler()->mousePressNode()) {
        if (mousePressNode->renderer() && !mousePressNode->canStartSelection()) {
            // Don't clear the selection for contentEditable elements, but do clear it for input and textarea. See bug 38696.
            Node * root = s->rootEditableElement();
            if (!root)
                return;

            if (Node* shadowAncestorNode = root->shadowAncestorNode()) {
                if (!shadowAncestorNode->hasTagName(inputTag) && !shadowAncestorNode->hasTagName(textareaTag))
                    return;
            }
        }
    }
    
    s->clear();
}

bool FocusController::setFocusedNode(Node* node, PassRefPtr<Frame> newFocusedFrame)
{
    RefPtr<Frame> oldFocusedFrame = focusedFrame();
    RefPtr<Document> oldDocument = oldFocusedFrame ? oldFocusedFrame->document() : 0;
    
    Node* oldFocusedNode = oldDocument ? oldDocument->focusedNode() : 0;
    if (oldFocusedNode == node)
        return true;

    // FIXME: Might want to disable this check for caretBrowsing
    if (oldFocusedNode && oldFocusedNode->rootEditableElement() == oldFocusedNode && !relinquishesEditingFocus(oldFocusedNode))
        return false;

    m_page->editorClient()->willSetInputMethodState();

    clearSelectionIfNeeded(oldFocusedFrame.get(), newFocusedFrame.get(), node);

    if (!node) {
        if (oldDocument)
            oldDocument->setFocusedNode(0);
        m_page->editorClient()->setInputMethodState(false);
        return true;
    }

    RefPtr<Document> newDocument = node->document();

    if (newDocument && newDocument->focusedNode() == node) {
        m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());
        return true;
    }
    
    if (oldDocument && oldDocument != newDocument)
        oldDocument->setFocusedNode(0);
    
    if (newFocusedFrame && !newFocusedFrame->page()) {
        setFocusedFrame(0);
        return false;
    }
    setFocusedFrame(newFocusedFrame);

    // Setting the focused node can result in losing our last reft to node when JS event handlers fire.
    RefPtr<Node> protect = node;
    if (newDocument) {
        bool successfullyFocused = newDocument->setFocusedNode(node);
        if (!successfullyFocused)
            return false;
    }

    if (newDocument->focusedNode() == node)
        m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());

    return true;
}

#if PLATFORM(WKC)
void FocusController::setActive(bool active, bool dispatchEvents)
#else
void FocusController::setActive(bool active)
#endif
{
    if (m_isActive == active)
        return;

    m_isActive = active;

    if (FrameView* view = m_page->mainFrame()->view()) {
        if (!view->platformWidget()) {
            view->updateLayoutAndStyleIfNeededRecursive();
            view->updateControlTints();
        }
    }

    focusedOrMainFrame()->selection()->pageActivationChanged();
    
#if PLATFORM(WKC)
    if (dispatchEvents && m_focusedFrame && isFocused())
#else
    if (m_focusedFrame && isFocused())
#endif
        dispatchEventsOnWindowAndFocusedNode(m_focusedFrame->document(), active);
}

static void contentAreaDidShowOrHide(ScrollableArea* scrollableArea, bool didShow)
{
    if (didShow)
        scrollableArea->contentAreaDidShow();
    else
        scrollableArea->contentAreaDidHide();
}

void FocusController::setContainingWindowIsVisible(bool containingWindowIsVisible)
{
    if (m_containingWindowIsVisible == containingWindowIsVisible)
        return;

    m_containingWindowIsVisible = containingWindowIsVisible;

    FrameView* view = m_page->mainFrame()->view();
    if (!view)
        return;

    contentAreaDidShowOrHide(view, containingWindowIsVisible);

    for (Frame* frame = m_page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        FrameView* frameView = frame->view();
        if (!frameView)
            continue;

        const HashSet<ScrollableArea*>* scrollableAreas = frameView->scrollableAreas();
        if (!scrollableAreas)
            continue;

        for (HashSet<ScrollableArea*>::const_iterator it = scrollableAreas->begin(), end = scrollableAreas->end(); it != end; ++it) {
            ScrollableArea* scrollableArea = *it;
            ASSERT(scrollableArea->isOnActivePage());

            contentAreaDidShowOrHide(scrollableArea, containingWindowIsVisible);
        }
    }
}

static void updateFocusCandidateIfNeeded(FocusDirection direction, const FocusCandidate& current, FocusCandidate& candidate, FocusCandidate& closest)
{
    ASSERT(candidate.visibleNode->isElementNode());
    ASSERT(candidate.visibleNode->renderer());

#if PLATFORM(WKC)
    if (candidate.visibleNode->hasTagName(aTag) && candidate.rect.isEmpty())
        return;

    // Ugh! Ignore any frames... May be inappropriate to scroll frames by focus.
    if (frameOwnerElement(candidate))
        return;

    if (adjustedTabIndex(candidate.focusableNode, 0) < 0)
        return;
#endif

    // Ignore iframes that don't have a src attribute
    if (frameOwnerElement(candidate) && (!frameOwnerElement(candidate)->contentFrame() || candidate.rect.isEmpty()))
        return;

#if PLATFORM(WKC)
    // Ignore off screen child nodes.
    if (candidate.isOffscreen)
        return;
#else
    // Ignore off screen child nodes of containers that do not scroll (overflow:hidden)
    if (candidate.isOffscreen && !canBeScrolledIntoView(direction, candidate))
        return;
#endif

    distanceDataForNode(direction, current, candidate);
    if (candidate.distance == maxDistance())
        return;

    if (candidate.isOffscreenAfterScrolling && candidate.alignment < Full)
        return;

#if PLATFORM(WKC)
    // Heuristic adjustment
    // The rectangles of the same size placed next to one another like a drop down list are more preferable to move focus to.
    if ((candidate.alignment == Full)
     && (candidate.rect.size() == current.rect.size())
     && (candidate.rect.x() == current.rect.x() || candidate.rect.y() == current.rect.y())
     && (candidate.distance < 10)) {
        candidate.alignment = SpecialAlignment;
    }

    // Heuristic adjustment
    if (direction == FocusDirectionUp || direction == FocusDirectionDown) {
        if (candidate.alignment == Partial)
            candidate.alignment = Full;
        if (candidate.alignment == OverlapPartial)
            candidate.alignment = OverlapFull;
    }

    if (closest.isNull()) {
        closest = candidate;
        return;
    }

    if (candidate.alignment < closest.alignment)
        return;

    if (candidate.alignment > closest.alignment) {
        closest = candidate;
        return;
    }

    ASSERT(candidate.alignment == closest.alignment);

    if (candidate.distance > closest.distance)
        return;

    if (candidate.distance < closest.distance) {
        closest = candidate;
        return;
    }

    ASSERT(candidate.distance == closest.distance);

    if (candidate.distanceFromCenter > closest.distanceFromCenter)
        return;

    if (candidate.distanceFromCenter < closest.distanceFromCenter) {
        closest = candidate;
        return;
    }

    ASSERT(candidate.distanceFromCenter == closest.distanceFromCenter);

    LayoutRect intersectionRect = intersection(candidate.rect, closest.rect);
    if (!intersectionRect.isEmpty()) {
        // If 2 nodes are intersecting, do hit test to find which node in on top.
        LayoutUnit x = intersectionRect.x() + intersectionRect.width() / 2;
        LayoutUnit y = intersectionRect.y() + intersectionRect.height() / 2;
        HitTestResult result = candidate.visibleNode->document()->page()->mainFrame()->eventHandler()->hitTestResultAtPoint(IntPoint(x, y), false, true);
        if (candidate.visibleNode->contains(result.innerNode()))
            closest = candidate;
    }
#else
    if (closest.isNull()) {
        closest = candidate;
        return;
    }

    LayoutRect intersectionRect = intersection(candidate.rect, closest.rect);
    if (!intersectionRect.isEmpty() && !areElementsOnSameLine(closest, candidate)) {
        // If 2 nodes are intersecting, do hit test to find which node in on top.
        LayoutUnit x = intersectionRect.x() + intersectionRect.width() / 2;
        LayoutUnit y = intersectionRect.y() + intersectionRect.height() / 2;
        HitTestResult result = candidate.visibleNode->document()->page()->mainFrame()->eventHandler()->hitTestResultAtPoint(IntPoint(x, y), false, true);
        if (candidate.visibleNode->contains(result.innerNode())) {
            closest = candidate;
            return;
        }
        if (closest.visibleNode->contains(result.innerNode()))
            return;
    }

    if (candidate.alignment == closest.alignment) {
        if (candidate.distance < closest.distance)
            closest = candidate;
        return;
    }

    if (candidate.alignment > closest.alignment)
        closest = candidate;
#endif
}

void FocusController::findFocusCandidateInContainer(Node* container, const LayoutRect& startingRect, FocusDirection direction, KeyboardEvent* event, FocusCandidate& closest)
{
    ASSERT(container);
    Node* focusedNode = (focusedFrame() && focusedFrame()->document()) ? focusedFrame()->document()->focusedNode() : 0;

#if PLATFORM(WKC)
    if (!focusedNode)
        focusedNode = container->document()->focusedNode();
#endif

    Node* node = container->firstChild();
    FocusCandidate current;
    current.rect = startingRect;
    current.focusableNode = focusedNode;
    current.visibleNode = focusedNode;

    for (; node; node = (node->isFrameOwnerElement() || canScrollInDirection(node, direction)) ? node->traverseNextSibling(container) : node->traverseNextNode(container)) {
        if (node == focusedNode)
            continue;

        if (!node->isElementNode())
            continue;

        if (!node->isKeyboardFocusable(event) && !node->isFrameOwnerElement() && !canScrollInDirection(node, direction))
            continue;

        FocusCandidate candidate = FocusCandidate(node, direction);
        if (candidate.isNull())
            continue;

        candidate.enclosingScrollableBox = container;
        updateFocusCandidateIfNeeded(direction, current, candidate, closest);
    }
}

bool FocusController::advanceFocusDirectionallyInContainer(Node* container, const LayoutRect& startingRect, FocusDirection direction, KeyboardEvent* event)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    if (!container || !container->document())
        return false;

    LayoutRect newStartingRect = startingRect;

    if (startingRect.isEmpty())
        newStartingRect = virtualRectForDirection(direction, nodeRectInAbsoluteCoordinates(container));

    // Find the closest node within current container in the direction of the navigation.
    FocusCandidate focusCandidate;
    findFocusCandidateInContainer(container, newStartingRect, direction, event, focusCandidate);

    if (focusCandidate.isNull()) {
        // Nothing to focus, scroll if possible.
        // NOTE: If no scrolling is performed (i.e. scrollInDirection returns false), the
        // spatial navigation algorithm will skip this container.
        return scrollInDirection(container, direction);
    }

#if PLATFORM(WKC)
    if (!focusCandidate.visibleNode->supportsFocus())
#endif
    if (HTMLFrameOwnerElement* frameElement = frameOwnerElement(focusCandidate)) {
        // If we have an iframe without the src attribute, it will not have a contentFrame().
        // We ASSERT here to make sure that
        // updateFocusCandidateIfNeeded() will never consider such an iframe as a candidate.
        ASSERT(frameElement->contentFrame());

        if (focusCandidate.isOffscreenAfterScrolling) {
            scrollInDirection(focusCandidate.visibleNode->document(), direction);
            return true;
        }
        // Navigate into a new frame.
        LayoutRect rect;
        Node* focusedNode = focusedOrMainFrame()->document()->focusedNode();
        if (focusedNode && !hasOffscreenRect(focusedNode))
            rect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
        frameElement->contentFrame()->document()->updateLayoutIgnorePendingStylesheets();
        if (!advanceFocusDirectionallyInContainer(frameElement->contentFrame()->document(), rect, direction, event)) {
            // The new frame had nothing interesting, need to find another candidate.
            return advanceFocusDirectionallyInContainer(container, nodeRectInAbsoluteCoordinates(focusCandidate.visibleNode, true), direction, event);
        }
        return true;
    }
#if PLATFORM(WKC)
    // If focusCandidate is block element and has OverFlow settings(except for VISIBLE setting) or Select element, this processing is skipped. 
    if (!focusCandidate.visibleNode->isScrollableOverFlowBlockNode() && !focusCandidate.visibleNode->hasTagName(selectTag) && canScrollInDirection(focusCandidate.visibleNode, direction)) {
#else
    if (canScrollInDirection(focusCandidate.visibleNode, direction)) {
#endif
        if (focusCandidate.isOffscreenAfterScrolling) {
            scrollInDirection(focusCandidate.visibleNode, direction);
            return true;
        }
        // Navigate into a new scrollable container.
        LayoutRect startingRect;
        Node* focusedNode = focusedOrMainFrame()->document()->focusedNode();
        if (focusedNode && !hasOffscreenRect(focusedNode))
            startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true);
        return advanceFocusDirectionallyInContainer(focusCandidate.visibleNode, startingRect, direction, event);
    }
#if PLATFORM(WKC)
    if (focusCandidate.isOffscreenAfterScrolling || (focusCandidate.isOrgRectEmpty && focusCandidate.isOffscreen)) {
#else
    if (focusCandidate.isOffscreenAfterScrolling) {
#endif
        Node* container = focusCandidate.enclosingScrollableBox;
        scrollInDirection(container, direction);
        return true;
    }

    // We found a new focus node, navigate to it.
    Element* element = toElement(focusCandidate.focusableNode);
    ASSERT(element);

    element->focus(false);
    return true;
}

bool FocusController::advanceFocusDirectionally(FocusDirection direction, KeyboardEvent* event)
{
    Frame* curFrame = focusedOrMainFrame();
    ASSERT(curFrame);

    Document* focusedDocument = curFrame->document();
    if (!focusedDocument)
        return false;

    Node* focusedNode = focusedDocument->focusedNode();
    Node* container = focusedDocument;

    if (container->isDocumentNode())
        static_cast<Document*>(container)->updateLayoutIgnorePendingStylesheets();
        
    // Figure out the starting rect.
    LayoutRect startingRect;
    if (focusedNode) {
#if PLATFORM(WKC)
    //If focsedNode or ParentFrame is block element and has OverFlow settings(except for VISIBLE setting), container is them.
        Node* scrollableNode = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, focusedNode);
        if (focusedNode->isScrollableOverFlowBlockNode()) {
            container = focusedNode;
            startingRect = nodeRectInAbsoluteCoordinates(container, true /* ignore border */);
            startingRect = virtualRectForDirection(direction, startingRect, 1);
        } else if (scrollableNode && scrollableNode->isScrollableOverFlowBlockNode()) {
            container = scrollableNode;
            startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
        } else if (!hasOffscreenRect(focusedNode)) {
            container = scrollableNode;
            startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
        } else if (focusedNode->hasTagName(areaTag)) {
            HTMLAreaElement* area = static_cast<HTMLAreaElement*>(focusedNode);
            container = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, area->imageElement());
            startingRect = virtualRectForAreaElementAndDirection(area, direction);
        }
        // Pieris:0295616
        // scrollableNode may be 0 if focusedNode is a node under the shadow root.
        // In this case we use focusedDocument as container.
        if (!container) {
            container = focusedDocument;
        }
#else
        if (!hasOffscreenRect(focusedNode)) {
            container = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, focusedNode);
            startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
        } else if (focusedNode->hasTagName(areaTag)) {
            HTMLAreaElement* area = static_cast<HTMLAreaElement*>(focusedNode);
            container = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, area->imageElement());
            startingRect = virtualRectForAreaElementAndDirection(area, direction);
        }
#endif
    }

    bool consumed = false;
    do {
        consumed = advanceFocusDirectionallyInContainer(container, startingRect, direction, event);
        startingRect = nodeRectInAbsoluteCoordinates(container, true /* ignore border */);
        container = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, container);
        if (container && container->isDocumentNode())
            static_cast<Document*>(container)->updateLayoutIgnorePendingStylesheets();
    } while (!consumed && container);

    return consumed;
}

#if PLATFORM(WKC)
bool
FocusController::setFocusedNode(Node *node)
{
    Frame* frame = focusedOrMainFrame();
    if ( !frame ){
        return false;
    }
    Document* newDocument = 0;
    if (node) {
        Document* focusedDocument = frame->document();
        newDocument = node->document();
        if (newDocument != focusedDocument) {
            focusedDocument->setFocusedNode(0);
        }
        if (newDocument) {
            setFocused(true);
            setFocusedFrame(newDocument->frame());
        }
        if (node->isElementNode()) {
            static_cast<Element *>(node)->focus(false);
        }
        if (newDocument) {
            return newDocument->setFocusedNode(node);
        }
    }
    return focusedOrMainFrame()->document()->setFocusedNode(node);
}

bool isNodeInSpecificRect(Node* node, const IntRect* specificRect)
{
    if (!specificRect || specificRect->isEmpty())
        return !hasOffscreenRect(node);

    FrameView* frameView = node->document()->view();
    if (!frameView)
        return false;

    IntRect rect;
    if (node->hasTagName(HTMLNames::areaTag)) {
        HTMLAreaElement* area = static_cast<HTMLAreaElement*>(node);
        HTMLImageElement* image = area->imageElement();
        if (!image || !image->renderer())
            return false;
        rect = rectToAbsoluteCoordinates(area->document()->frame(), area->computeRect(image->renderer()));
    } else {
        RenderObject* render = node->renderer();
        if (!render)
            return false;
        rect = node->getRect();
    }
    if (rect.isEmpty())
        return false;
    rect = frameView->contentsToWindow(rect);

    return specificRect->intersects(rect);
}

bool isNodeInContentsRect(Node* node, const IntRect* contentsRect)
{
    if (!contentsRect || contentsRect->isEmpty())
        return !hasOffscreenRect(node);

    FrameView* frameView = node->document()->view();
    if (!frameView)
        return false;

    IntRect rect;
    if (node->hasTagName(HTMLNames::areaTag)) {
        HTMLAreaElement* area = static_cast<HTMLAreaElement*>(node);
        HTMLImageElement* image = area->imageElement();
        if (!image || !image->renderer())
            return false;
        rect = rectToAbsoluteCoordinates(area->document()->frame(), area->computeRect(image->renderer()));
    } else {
        RenderObject* render = node->renderer();
        if (!render)
            return false;
        rect = nodeRectInAbsoluteCoordinates(node, true /* ignore border */);
    }
    if (rect.isEmpty())
        return false;

    return contentsRect->intersects(rect);
}

bool isScrollableContainerNode(Node* node)
{
    if (!node)
        return false;

    if (RenderObject* renderer = node->renderer()) {
        return (renderer->isBox() && toRenderBox(renderer)->canBeScrolledAndHasScrollableArea()
             && node->hasChildNodes() && !node->isDocumentNode());
    }

    return false;
}

void FocusController::findFocusableNodeInDirection(Node* container, const IntRect& startingRect, FocusDirection direction, KeyboardEvent* event, FocusCandidate& closest, const IntRect* contentsRect)
{
    ASSERT(container);
    Node* focusedNode = (focusedFrame() && focusedFrame()->document()) ? focusedFrame()->document()->focusedNode() : 0;

    Node* node = container->firstChild();
    FocusCandidate current;
    current.rect = startingRect;
    current.focusableNode = focusedNode;
    current.visibleNode = focusedNode;
    current.direction = m_lastDirection;
    current.exitRect = m_lastExitRect;
    if (!m_lastEntryRect.isEmpty() && current.rect.contains(m_lastEntryRect))
        current.entryRect = m_lastEntryRect;
    else 
        current.entryRect = startingRect;

    for (; node; node = (node->isFrameOwnerElement() || canScrollInDirection(node, direction) || isScrollableContainerNode(node)) ? node->traverseNextSibling(container) : node->traverseNextNode(container)) {

        if (contentsRect && !isNodeInContentsRect(node, contentsRect))
            continue;

        if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            findFocusableNodeInDirection(node, startingRect, direction, event, closest, contentsRect);
            continue;
        }

        if (node == focusedNode)
            continue;

        if (!node->isElementNode())
            continue;

        if (!node->isKeyboardFocusable(event) && !node->isFrameOwnerElement() && !canScrollInDirection(node, direction))
            continue;

        FocusCandidate candidate = FocusCandidate(node, direction);
        if (candidate.isNull())
            continue;

        candidate.enclosingScrollableBox = container;
        updateFocusCandidateIfNeeded(direction, current, candidate, closest);

        if (HTMLFrameOwnerElement* frameElement = frameOwnerElement(candidate)) {
            // Navigate into a new frame.
            if (!frameElement->contentFrame())
                continue;
            frameElement->contentFrame()->document()->updateLayoutIgnorePendingStylesheets();
            findFocusableNodeInDirection(frameElement->contentFrame()->document(), startingRect, direction, event, closest, contentsRect);
            continue;
        }
    }
    if (closest.focusableNode && closest.focusableNode->isElementNode() && closest.distance != maxDistance()) {
        m_lastExitRect = current.entryRect;
        m_lastEntryRect = closest.entryRect;
        m_lastDirection = direction;
    }
}

static Node*
findFirstFocusableNode(Frame* frame, const IntRect* specificRect)
{
    Node* node = frame->document()->firstChild();
    while (node) {
        if (!isNodeInSpecificRect(node, specificRect) || adjustedTabIndex(node, 0) < 0) {
            node = node->traverseNextNode();
            continue;
        }
        if (node->isFrameOwnerElement()) {
            HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
            if (owner->contentFrame()) {
                node = findFirstFocusableNode(owner->contentFrame(), specificRect);
                if (node)
                    break;
                node = owner;
            }
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            node = node->firstChild();
        }
        if (node->isFocusable() && !node->isFrameOwnerElement()) {
            break;
        }
        node = node->traverseNextNode();
    }

    if (node) {
        return node->isFocusable() ? node : 0;
    }
    return 0;
}

static Node*
findLastFocusableNode(Frame* frame, const IntRect* specificRect)
{
    Node* node = frame->document()->lastChild();
    while (node) {
        if (!isNodeInSpecificRect(node, specificRect) || adjustedTabIndex(node, 0) < 0) {
            node = node->traversePreviousNodePostOrder();
            continue;
        }
        if (node->isFrameOwnerElement()) {
            HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
            if (owner->contentFrame()) {
                node = findLastFocusableNode(owner->contentFrame(), specificRect);
                if (node)
                    break;
                node = owner;
            }
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            node = node->lastChild();
        }
        if (node->isFocusable() && !node->isFrameOwnerElement()) {
            break;
        }
        node = node->traversePreviousNodePostOrder();
    }

    if (node) {
        return node->isFocusable() ? node : 0;
    }
    return 0;
}

Node* FocusController::findNextFocusableNode(const FocusDirection direction, const IntRect* specificRect)
{
    Frame* frame = focusedOrMainFrame();
    ASSERT(frame);
    Document* focusedDocument = frame->document();
    if (!focusedDocument)
        return 0;

    focusedDocument->updateLayoutIgnorePendingStylesheets();

    Node* focusedNode = focusedDocument->focusedNode();
    if (!focusedNode) {
        if (direction == FocusDirectionUp) {
            return findLastFocusableNode(m_page->mainFrame(), specificRect);
        } else {
            return findFirstFocusableNode(m_page->mainFrame(), specificRect);
        }
    }

    IntRect startingRect;
    Node* container = focusedDocument;
    if (!hasOffscreenRect(focusedNode)) {
        startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
    } else if (focusedNode->hasTagName(areaTag)) {
        HTMLAreaElement* area = static_cast<HTMLAreaElement*>(focusedNode);
        startingRect = virtualRectForAreaElementAndDirection(area, direction);
    }

    FocusCandidate focusCandidate;
    frame = frame->tree()->top();
    IntRect rect = frame->view()->windowToContents(*specificRect);    
    findFocusableNodeInDirection(frame->document(), startingRect, direction, 0, focusCandidate, &rect);
    ASSERT(!frameOwnerElement(focusCandidate));

    Node* node = focusCandidate.focusableNode;
    if (!node || !node->isElementNode()) {
        return 0;
    }

    if (focusCandidate.isOffscreen) {
        return 0;
    }

    return node;
}

static Node*
getClosestNode(Node* node, FocusDirection direction, bool sibling = false)
{
    ASSERT(node);
    ASSERT(direction == FocusDirectionUp    ||
           direction == FocusDirectionDown  ||
           direction == FocusDirectionLeft  ||
           direction == FocusDirectionRight);

    Node* closestNode;
    switch (direction) {
    case FocusDirectionUp:
        if (sibling)
            closestNode = node->traversePreviousSiblingPostOrder();
        else
            closestNode = node->traversePreviousNodePostOrder();
        break;
    case FocusDirectionDown:
    case FocusDirectionLeft:
    case FocusDirectionRight:
        if (sibling)
            closestNode = node->traverseNextSibling();
        else
            closestNode = node->traverseNextNode();
        break;
    default:
        ASSERT(false);
        closestNode = 0; // avoid warning
        break;
    }
    return closestNode;
}

Node* FocusController::findVerticallyFocusableNodeInRect(FocusDirection direction, Node* start, KeyboardEvent* event, const IntRect* rect, bool enableContainer)
{
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);

    ASSERT(direction == FocusDirectionUp || direction == FocusDirectionDown);

    if (!start)
        return 0;

    Node* node = 0;
    Document* document;

    for (node = start; node; node = getClosestNode(node, direction)) {

        if (!node->isFocusable()) {
            continue;
        }

        if (adjustedTabIndex(node, 0) < 0) {
            continue;
        }

        if (!isNodeInSpecificRect(node, rect)) {
            continue;
        }

        if (node->isFrameOwnerElement() || (isScrollableContainerNode(node) && !node->renderer()->isTextArea())) {
            Node* firstChildNode = 0;
            Node* ChildNode = 0;
            IntRect nodeRect;
            if (node->isFrameOwnerElement()) {
                HTMLFrameOwnerElement* owner;
                owner = static_cast<HTMLFrameOwnerElement*>(node);
                if (!owner->contentFrame()) {
                    node = 0;
                    break;
                }
                document = owner->contentFrame()->document();
                if (direction == FocusDirectionUp) {
                    firstChildNode = document->lastChild();
                } else {
                    firstChildNode = document->firstChild();
                }
            } else {
                if (direction == FocusDirectionUp) {
                    firstChildNode = node->lastChild();
                } else {
                    firstChildNode = node->firstChild();
                }
            }

            nodeRect = node->renderer()->absoluteBoundingBoxRect();
            FrameView* frameView = node->document()->view();
            if (!frameView) {
                return 0;
            }                
            nodeRect = frameView->contentsToWindow(nodeRect);
            nodeRect.intersect(*rect);

            ChildNode = findVerticallyFocusableNodeInRect(direction, firstChildNode, event, &nodeRect, enableContainer);

            if (ChildNode) {
                node = ChildNode;
                break;
            } else if (node->isFocusable() && enableContainer){
                break;
            }
            continue;
        }
        break;
    }
    return node;
}

Node* FocusController::findHorizontallyFocusableNodeInRect(FocusDirection direction, Node* start, KeyboardEvent* event, const IntRect* rect, bool enableContainer)
{
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);

    ASSERT(direction == FocusDirectionLeft || direction == FocusDirectionRight);

    if (!start)
        return 0;

    Node* node;
    HTMLFrameOwnerElement* owner;
    Document* document;
    IntRect nodeRect;
    Node* candidateNode = 0;
    IntRect candidateNodeRect;

    for (node = start; node; node = getClosestNode(node, direction)) {

        if (!node->isFocusable()) {
            continue;
        }

        if (adjustedTabIndex(node, 0) < 0) {
            continue;
        }

        if (!isNodeInSpecificRect(node, rect)) {
            continue;
        }
        owner = 0;
        if (node->isFrameOwnerElement() || (isScrollableContainerNode(node) && !node->renderer()->isTextArea())) {
            Node* firstChildNode = 0;
            Node* childNode = 0;
            if (node->isFrameOwnerElement()) {
                owner = static_cast<HTMLFrameOwnerElement*>(node);
                if (!owner->contentFrame()) {
                    return 0;
                }
                document = owner->contentFrame()->document();
                firstChildNode = document->firstChild();
            } else {
                firstChildNode = node->firstChild();
            }
            nodeRect = node->renderer()->absoluteBoundingBoxRect();
            FrameView* frameView = node->document()->view();
            if (!frameView) {
                return 0;
            }                
            nodeRect = frameView->contentsToWindow(nodeRect);
            nodeRect.intersect(*rect);
            childNode = findHorizontallyFocusableNodeInRect(direction, firstChildNode, 0, &nodeRect, enableContainer);
            if (childNode)
                node = childNode;
            else if (!enableContainer) {
                continue;
            }
        }

        if (node->isFocusable()) {
            nodeRect = node->renderer()->absoluteBoundingBoxRect();
            FrameView* frameView = node->document()->view();
            if (!frameView) {
                return 0;
            }                
            nodeRect = frameView->contentsToWindow(nodeRect);
            nodeRect.intersect(*rect);
            if (!nodeRect.isEmpty()) {
                if (!candidateNode) {
                    candidateNode = node;
                    candidateNodeRect = nodeRect;
                }
                if (direction == FocusDirectionRight && candidateNodeRect.x() > nodeRect.x()) {
                    candidateNode = node;
                    candidateNodeRect = nodeRect;
                } else if (direction == FocusDirectionLeft && candidateNodeRect.x() < nodeRect.x()) {
                    candidateNode = node;
                    candidateNodeRect = nodeRect;
                }
            }
        }
    }

    return candidateNode;
}

Node* FocusController::findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const IntRect* rect, bool enableContainer)
{
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);

    ASSERT(frame);
    ASSERT(rect);

    Node* start;
    if (direction == FocusDirectionUp) {
        Node* last = 0;
        for (last = frame->document()->lastChild(); last && last->lastChild(); last = last->lastChild())
            ; // Empty loop.
        start = last;
    } else {
        ASSERT(direction == FocusDirectionDown || direction == FocusDirectionLeft || direction == FocusDirectionRight);
        start = frame->document()->firstChild();
    }

    frame->document()->updateLayoutIgnorePendingStylesheets();

    Node* node = 0;
    do {
        IntRect candidateNodeRect;
        switch (direction) {
        case FocusDirectionUp:
        case FocusDirectionDown:
            node = findVerticallyFocusableNodeInRect(direction, start, 0, rect, enableContainer);
            break;
        case FocusDirectionRight:
        case FocusDirectionLeft:
            node = findHorizontallyFocusableNodeInRect(direction, start, 0, rect, enableContainer);
            break;
        default:
            node = 0;
            break;
        }
        if (!node || node->isKeyboardFocusable(0))
            break;
        start = node->traverseNextNode();
    } while (start);

    return node;
}

static IntRect
getNodeRect(Node* node)
{
    IntRect nodeRect(0,0,0,0);
    if( !node )
        return nodeRect;

    if (node->hasTagName(HTMLNames::areaTag)) {
        HTMLAreaElement* area = static_cast<HTMLAreaElement*>(node);
        HTMLImageElement* image = area->imageElement();
        if (!image || !image->renderer())
            nodeRect = IntRect();
        else
            nodeRect = rectToAbsoluteCoordinates(area->document()->frame(), area->computeRect(image->renderer()));
    } else {
        nodeRect = node->getRect();
    }
    return nodeRect;
}

Node*
FocusController::findNearestFocusableNodeFromPoint(const IntPoint point, const IntRect* rect)
{
    Node* node = 0;
    Node* nearest = 0;
    int nearDist = INT_MAX;
    IntRect cr;
    IntRect* contentsRect = 0;

    Frame* fr = m_page->mainFrame();
    Document* doc = m_page->mainFrame()->document();
    FrameView* frameView = m_page->mainFrame()->view();
    if( !frameView )
        return 0;

    RefPtr<FrameView> protector(frameView);

    IntPoint contentsPoint = frameView->windowToContents( point );
    if( rect && !(rect->isEmpty()) ){
        cr = frameView->windowToContents( *rect );
        contentsRect = &cr;
    }

    node = findFirstFocusableNode(fr, rect);
    while( node ){
        IntRect nodeRect = getNodeRect(node);
        bool keyboardFocusable = node->isKeyboardFocusable(0);
        if( !keyboardFocusable ){
            node = findVerticallyFocusableNodeInRect(FocusDirectionDown, node->traverseNextNode(), 0, rect, false);
            continue;
        }
        if( nodeRect.contains( contentsPoint ) ){
            nearest = node;
            break;
        }

        IntPoint center = nodeRect.center();
        IntSize sz = contentsPoint - center;
        int dist = abs(sz.width()) + abs(sz.height());
        ASSERT( dist >= 0 );
        if ( dist < nearDist ){
            nearest = node;
            nearDist = dist;
        }
        node = findVerticallyFocusableNodeInRect(FocusDirectionDown, node->traverseNextNode(), 0, rect, false);
    }

    return nearest;
}

#endif // PLATFORM(WKC)

} // namespace WebCore
