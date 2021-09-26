/*
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include "helpers/WKCNode.h"
#include "helpers/privates/WKCNodePrivate.h"

#include "Node.h"
#include "HTMLNames.h"
#include "Element.h"
#include "AtomicString.h"
#include "NodeList.h"

#if USE(ACCELERATED_COMPOSITING)
#include "RenderLayer.h"
#include "RenderLayerBacking.h"
#endif

#include "helpers/WKCString.h"

#include "helpers/privates/WKCDocumentPrivate.h"
#include "helpers/privates/WKCElementPrivate.h"
#include "helpers/privates/WKCHTMLInputElementPrivate.h"
#include "helpers/privates/WKCHTMLTextAreaElementPrivate.h"
#include "helpers/privates/WKCHTMLAreaElementPrivate.h"
#include "helpers/privates/WKCHTMLIFrameElementPrivate.h"
#include "helpers/privates/WKCHTMLFormElementPrivate.h"
#include "helpers/privates/WKCHTMLFrameElementPrivate.h"
#include "helpers/privates/WKCHTMLMediaElementPrivate.h"
#include "helpers/privates/WKCHTMLSelectElementPrivate.h"
#include "helpers/privates/WKCRenderObjectPrivate.h"
#include "helpers/privates/WKCNamedAttrMapPrivate.h"
#include "helpers/privates/WKCAtomicStringPrivate.h"
#include "helpers/privates/WKCNodeListPrivate.h"

#if USE(ACCELERATED_COMPOSITING)
#include "helpers/privates/WKCGraphicsLayerPrivate.h"
namespace WebCore {
WKC::GraphicsLayerPrivate* GraphicsLayerWKC_wkc(const WebCore::GraphicsLayer* layer);
}
#endif

namespace WKC {

NodePrivate*
NodePrivate::create(WebCore::Node* parent)
{
    NodePrivate* node = 0;
    if (!parent)
        return 0;

    if (parent->hasTagName(WebCore::HTMLNames::inputTag)) {
        node = new HTMLInputElementPrivate(reinterpret_cast<WebCore::HTMLInputElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::textareaTag)) {
        node = new HTMLTextAreaElementPrivate(reinterpret_cast<WebCore::HTMLTextAreaElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::areaTag)) {
        node = new HTMLAreaElementPrivate(reinterpret_cast<WebCore::HTMLAreaElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::iframeTag)) {
        node = new HTMLIFrameElementPrivate(reinterpret_cast<WebCore::HTMLIFrameElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::formTag)) {
        node = new HTMLFormElementPrivate(reinterpret_cast<WebCore::HTMLFormElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLFrameElementPrivate(reinterpret_cast<WebCore::HTMLFrameElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::selectTag)) {
        node = new HTMLSelectElementPrivate(reinterpret_cast<WebCore::HTMLSelectElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::videoTag)) {
        // TODO: If you implement HTMLVideoElement, you need to create HTMLVideoElement instead of HTMLMediaElement.
        node = new HTMLMediaElementPrivate(reinterpret_cast<WebCore::HTMLMediaElement*>(parent));
    } else if (parent->isHTMLElement()) {
        node = new HTMLElementPrivate(reinterpret_cast<WebCore::HTMLElement*>(parent));
    } else if (parent->isElementNode()) {
        node = new ElementPrivate(reinterpret_cast<WebCore::Element*>(parent));
    } else {
        node = new NodePrivate(parent);
    }
    return node;
}

NodePrivate*
NodePrivate::create(const WebCore::Node* parent)
{
    NodePrivate* node = 0;
    if (!parent)
        return 0;

    if (parent->hasTagName(WebCore::HTMLNames::inputTag)) {
        node = new HTMLInputElementPrivate((WebCore::HTMLInputElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::textareaTag)) {
        node = new HTMLTextAreaElementPrivate((WebCore::HTMLTextAreaElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::areaTag)) {
        node = new HTMLAreaElementPrivate((WebCore::HTMLAreaElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::iframeTag)) {
        node = new HTMLIFrameElementPrivate((WebCore::HTMLIFrameElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::formTag)) {
        node = new HTMLFormElementPrivate((WebCore::HTMLFormElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLFrameElementPrivate((WebCore::HTMLFrameElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::selectTag)) {
        node = new HTMLSelectElementPrivate((WebCore::HTMLSelectElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::videoTag)) {
        // TODO: If you implement HTMLVideoElement, you need to create HTMLVideoElement instead of HTMLMediaElement.
        node = new HTMLMediaElementPrivate((WebCore::HTMLMediaElement*)(parent));
    } else if (parent->isHTMLElement()) {
        node = new HTMLElementPrivate((WebCore::HTMLElement*)(parent));
    } else if (parent->isElementNode()) {
        node = new ElementPrivate((WebCore::Element*)(parent));
    } else {
        node = new NodePrivate(parent);
    }
    return node;
}

NodePrivate*
NodePrivate::create(const NodePrivate& wparent)
{
    NodePrivate* node = 0;
    WebCore::Node* parent = wparent.webcore();

    if (parent->hasTagName(WebCore::HTMLNames::inputTag)) {
        node = new HTMLInputElementPrivate((WebCore::HTMLInputElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::textareaTag)) {
        node = new HTMLTextAreaElementPrivate((WebCore::HTMLTextAreaElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::areaTag)) {
        node = new HTMLAreaElementPrivate((WebCore::HTMLAreaElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::iframeTag)) {
        node = new HTMLIFrameElementPrivate((WebCore::HTMLIFrameElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::formTag)) {
        node = new HTMLFormElementPrivate((WebCore::HTMLFormElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLFrameElementPrivate((WebCore::HTMLFrameElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLSelectElementPrivate((WebCore::HTMLSelectElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::videoTag)) {
        // TODO: If you implement HTMLVideoElement, you need to create HTMLVideoElement instead of HTMLMediaElement.
        node = new HTMLMediaElementPrivate((WebCore::HTMLMediaElement*)(parent));
    } else if (parent->isHTMLElement()) {
        node = new HTMLElementPrivate((WebCore::HTMLElement*)(parent));
    } else if (parent->isElementNode()) {
        node = new ElementPrivate((WebCore::Element*)(parent));
    } else {
        node = new NodePrivate(parent);
    }
    return node;
}

#ifdef WKC_CUSTOMER_PATCH_0304674
void 
NodePrivate::destroy()
{
    delete this;
}
#endif

NodePrivate::NodePrivate(WebCore::Node* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_document(0)
    , m_parentElement(0)
    , m_renderer(0)
    , m_parentNamedNodeMap(0)
    , m_parent(0)
    , m_parentNode(0)
    , m_firstChild(0)
    , m_traverseNextNode(0)
    , m_traverseNextSibling(0)
    , m_shadowAncestorNode(0)
    , m_HTMLElement(0)
    , m_nodeList(0)
{
}

NodePrivate::NodePrivate(const WebCore::Node* parent)
    : m_webcore(const_cast<WebCore::Node*>(parent))
    , m_wkc(*this)
    , m_document(0)
    , m_parentElement(0)
    , m_renderer(0)
    , m_parentNamedNodeMap(0)
    , m_parent(0)
    , m_parentNode(0)
    , m_firstChild(0)
    , m_traverseNextNode(0)
    , m_traverseNextSibling(0)
    , m_shadowAncestorNode(0)
    , m_HTMLElement(0)
    , m_nodeList(0)
{
}

NodePrivate::~NodePrivate()
{
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
    delete m_renderer;

    if (m_document)
        delete m_document;
    if (m_parentElement)
        delete m_parentElement;
    if (m_parent)
        delete m_parent;
    if (m_parentNode)
        delete m_parentNode;
    if (m_firstChild)
        delete m_firstChild;
    if (m_traverseNextNode)
        delete m_traverseNextNode;
    if (m_traverseNextSibling)
        delete m_traverseNextSibling;
    if (m_shadowAncestorNode)
        delete m_shadowAncestorNode;
    if (m_parentNamedNodeMap)
        delete m_parentNamedNodeMap;
    if (m_HTMLElement)
        delete m_HTMLElement;
    if (m_nodeList)
        delete m_nodeList;
}

String
NodePrivate::nodeName() const
{
    return m_webcore->nodeName();
}

bool
NodePrivate::hasTagName(int id) const
{
    switch (id) {
    case HTMLNames_inputTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::inputTag);
    case HTMLNames_textareaTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::textareaTag);
    case HTMLNames_selectTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::selectTag);
    case HTMLNames_formTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::formTag);
    case HTMLNames_frameTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::frameTag);
    case HTMLNames_iframeTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::iframeTag);
    case HTMLNames_videoTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::videoTag);
    case HTMLNames_areaTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::areaTag);
    default:
        return false;
    }
}

bool
NodePrivate::isFocusable() const
{
    return m_webcore->isFocusable();
}

bool
NodePrivate::isHTMLElement() const
{
    return m_webcore->isHTMLElement();
}

bool
NodePrivate::isElementNode() const
{
    return m_webcore->isElementNode();
}

bool
NodePrivate::isFrameOwnerElement() const
{
    return m_webcore->isFrameOwnerElement();
}

bool
NodePrivate::inDocument() const
{
    return m_webcore->inDocument();
}

bool
NodePrivate::hasEventListeners(int id)
{
    switch (id ){
    case eventNames_clickEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().clickEvent);
    case eventNames_mousedownEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().mousedownEvent);
    case eventNames_mousemoveEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().mousemoveEvent);
    case eventNames_dragEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().dragEvent);
    case eventNames_dragstartEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().dragstartEvent);
    case eventNames_dragendEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().dragendEvent);
#if ENABLE(TOUCH_EVENTS)
    case eventNames_touchstartEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().touchstartEvent);
    case eventNames_touchmoveEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().touchmoveEvent);
    case eventNames_touchendEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().touchendEvent);
    case eventNames_touchcancelEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().touchcancelEvent);
#endif
    }
    return false;
}


Document*
NodePrivate::document()
{
    WebCore::Document* doc = m_webcore->document();
    if (!doc)
        return 0;
    if (!m_document || m_document->webcore()!=doc) {
        delete m_document;
        m_document = new DocumentPrivate(doc);
    }
    return &m_document->wkc();
}

RenderObject*
NodePrivate::renderer()
{
    WebCore::RenderObject* render = m_webcore->renderer();
    if (!render)
        return 0;
    if (!m_renderer || m_renderer->webcore()!=render) {
        delete m_renderer;
        m_renderer = new RenderObjectPrivate(render);

    }
    return &m_renderer->wkc();
}

Element*
NodePrivate::parentElement()
{
    WebCore::Element* elem = m_webcore->parentElement();
    if (!elem)
        return 0;
    if (!m_parentElement || m_parentElement->webcore()!=elem) {
        delete m_parentElement;
        m_parentElement = new ElementPrivate(elem);
    }
    return &m_parentElement->wkc();
}


Node*
NodePrivate::parent()
{
    if (!m_webcore)
        return 0;

    WebCore::ContainerNode* n = m_webcore->parentNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_parent || m_parent->webcore()!=n) {
        delete m_parent;
        m_parent = create(n);
    }
    return &m_parent->wkc();
}

Node*
NodePrivate::parentNode()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->parentNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_parentNode || m_parentNode->webcore()!=n) {
        delete m_parentNode;
        m_parentNode = create(n);
    }
    return &m_parentNode->wkc();
}

Node*
NodePrivate::firstChild()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->firstChild();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_firstChild || m_firstChild->webcore()!=n) {
        delete m_firstChild;
        m_firstChild = create(n);
    }
    return &m_firstChild->wkc();
}

Node*
NodePrivate::traverseNextNode()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->traverseNextNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_traverseNextNode || m_traverseNextNode->webcore()!=n) {
        delete m_traverseNextNode;
        m_traverseNextNode = create(n);
    }
    return &m_traverseNextNode->wkc();
}

Node*
NodePrivate::traverseNextSibling()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->traverseNextSibling();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_traverseNextSibling || m_traverseNextSibling->webcore()!=n) {
        delete m_traverseNextSibling;
        m_traverseNextSibling = create(n);
    }
    return &m_traverseNextSibling->wkc();
}

Node*
NodePrivate::shadowAncestorNode()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->shadowAncestorNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_shadowAncestorNode || m_shadowAncestorNode->webcore()!=n) {
        delete m_shadowAncestorNode;
        m_shadowAncestorNode = create(n);
    }
    return &m_shadowAncestorNode->wkc();
}

bool
NodePrivate::isInShadowTree() const
{
    if (!m_webcore)
        return false;

    return m_webcore->isInShadowTree();
}

NodeList*
NodePrivate::getElementsByTagName(const String& localName)
{
    if (!m_webcore)
        return 0;

    PassRefPtr<WebCore::NodeList> list = m_webcore->getElementsByTagName(WTF::AtomicString(localName));
    if (!list)
        return 0;
    delete m_nodeList;
    m_nodeList = new NodeListPrivate(list);
    return &m_nodeList->wkc();
}

NamedNodeMap*
NodePrivate::attributes()
{
    WebCore::NamedNodeMap* n;
    n = m_webcore->attributes();

    if (!n)
        return 0;

    if (!m_parentNamedNodeMap){
        m_parentNamedNodeMap = new NamedNodeMapPrivate(n);
    } else if (m_parentNamedNodeMap->webcore() != n) {
        delete m_parentNamedNodeMap;
        m_parentNamedNodeMap = new NamedNodeMapPrivate(n);
    }

    return m_parentNamedNodeMap->wkc();
}

bool
NodePrivate::isContentEditable() const
{
    return m_webcore->isContentEditable();
}

String
NodePrivate::textContent(bool conv) const
{
    return m_webcore->textContent(conv);
}

void
NodePrivate::setTextContent(const String& text, int& ec)
{
    m_webcore->setTextContent(text, ec);
}

HTMLElement*
NodePrivate::toHTMLElement()
{
    if( !(this->isHTMLElement()) )
        return 0;

    WebCore::HTMLElement* elem = (WebCore::HTMLElement *)(m_webcore);
    if (!elem)
        return 0;
    if (!m_HTMLElement || m_HTMLElement->webcore()!=elem) {
        delete m_HTMLElement;
        m_HTMLElement = new HTMLElementPrivate(elem);
    }
    return &m_HTMLElement->wkc();
}

void
NodePrivate::dispatchChangeEvent()
{
    m_webcore->dispatchChangeEvent();
}

const GraphicsLayer*
NodePrivate::enclosingGraphicsLayer() const
{
#if USE(ACCELERATED_COMPOSITING)
    WebCore::RenderObject* renderer = m_webcore->renderer();
    if (!renderer)
        return 0;
    WebCore::RenderLayer* renderlayer = renderer->enclosingLayer();
    if (!renderlayer)
        return 0;
    WebCore::RenderLayer* compositedlayer = renderlayer->enclosingCompositingLayer();
    if (!compositedlayer)
        return 0;
    WebCore::GraphicsLayer* graphicslayer = compositedlayer->backing()->graphicsLayer();
    if (!graphicslayer)
        return 0;
    GraphicsLayerPrivate* wkc = GraphicsLayerWKC_wkc(graphicslayer);
    return &wkc->wkc();
#else
    return 0;
#endif
}

bool
NodePrivate::isScrollableOverFlowBlockNode() const
{
    return m_webcore->isScrollableOverFlowBlockNode();
}

void
NodePrivate::getNodeCompositeRect(WKCRect* rect, int tx, int ty)
{
    WebCore::IntRect core_rect = WebCore::IntRect(rect->fX, rect->fY, rect->fWidth, rect->fHeight);
    m_webcore->getNodeCompositeRect(&core_rect, tx, ty);
    rect->fX = core_rect.x();
    rect->fY = core_rect.y();
    rect->fWidth = core_rect.width();
    rect->fHeight = core_rect.height();
}

Node::Node(NodePrivate& parent)
    : m_ownedPrivate(0)
    , m_private(parent)
    , m_needsRef(false)
{
}

Node::Node(Node* parent, bool needsRef)
    : m_ownedPrivate(NodePrivate::create(parent->priv().webcore()))
    , m_private(*m_ownedPrivate)
    , m_needsRef(needsRef)
{
    if (needsRef)
        m_private.webcore()->ref();
}

Node::~Node()
{
    if (m_needsRef)
        m_private.webcore()->deref();
    if (m_ownedPrivate)
        delete m_ownedPrivate;
}

bool
Node::compare(const Node* other) const
{
    if (this==other)
        return true;
    if (!this || !other)
        return false;
    if (m_private.webcore() == other->m_private.webcore())
        return true;
    return false;
}

bool
Node::hasTagName(int id) const
{
    return m_private.hasTagName(id);
}

bool
Node::isFocusable() const
{
    return m_private.isFocusable();
}

bool
Node::isHTMLElement() const
{
    return m_private.isHTMLElement();
}

bool
Node::isElementNode() const
{
    return m_private.isElementNode();
}

bool
Node::isFrameOwnerElement() const
{
    return m_private.isFrameOwnerElement();
}

bool
Node::inDocument() const
{
    return m_private.inDocument();
}

String
Node::nodeName() const
{
    return m_private.nodeName();
}

bool
Node::hasEventListeners(int id)
{
    return m_private.hasEventListeners(id);
}


Document*
Node::document() const
{
    return m_private.document();
}

RenderObject*
Node::renderer() const
{
    return m_private.renderer();
}

Element*
Node::parentElement() const
{
    return m_private.parentElement();
}


Node*
Node::parent() const
{
    return m_private.parent();
}

Node*
Node::parentNode() const
{
    return m_private.parentNode();
}

Node*
Node::firstChild() const
{
    return m_private.firstChild();
}

Node*
Node::shadowAncestorNode() const
{
    return m_private.shadowAncestorNode();
}

Node*
Node::traverseNextNode() const
{
    return m_private.traverseNextNode();
}

Node*
Node::traverseNextSibling() const
{
    return m_private.traverseNextSibling();
}

NamedNodeMap*
Node::attributes() const
{
    return m_private.attributes();
}

bool
Node::isContentEditable() const
{
    return m_private.isContentEditable();
}

String
Node::textContent(bool conv) const
{
    return m_private.textContent(conv);
}

void
Node::setTextContent(const String& text, int& ec)
{
    m_private.setTextContent(text, ec);
}

HTMLElement*
Node::toHTMLElement() const
{
    return m_private.toHTMLElement();
}

void
Node::dispatchChangeEvent()
{
    return m_private.dispatchChangeEvent();
}

const GraphicsLayer*
Node::enclosingGraphicsLayer() const
{
    return m_private.enclosingGraphicsLayer();
}

bool
Node::isScrollableOverFlowBlockNode() const
{
    return m_private.isScrollableOverFlowBlockNode();
}

void
Node::getNodeCompositeRect(WKCRect* rect, int tx, int ty)
{
    return m_private.getNodeCompositeRect(rect, tx, ty);
}

bool
Node::isInShadowTree() const
{
    return m_private.isInShadowTree();
}

NodeList*
Node::getElementsByTagName(const String& localName)
{
    return m_private.getElementsByTagName(localName);
}


} // namespace
