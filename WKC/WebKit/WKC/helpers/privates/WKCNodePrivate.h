/*
 * Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_PRIVATE_NODE_H_
#define _WKC_HELPERS_PRIVATE_NODE_H_

#include "helpers/WKCNode.h"

namespace WebCore {
class Node;
} // namespace

namespace WKC {

class RenderObjectPrivate;
class ElementPrivate;
class DocumentPrivate;
class RenderObject;
class Element;
class Document;
class NamedNodeMapPrivate;
class HTMLElement;
class HTMLElementPrivate;
class NodeListPrivate;

class NodePrivate {
public:
    static NodePrivate* create(WebCore::Node*);
    static NodePrivate* create(const WebCore::Node*);
    static NodePrivate* create(const NodePrivate&);
    virtual ~NodePrivate();

#ifdef WKC_CUSTOMER_PATCH_0304674
    void destroy();
#endif

    WebCore::Node* webcore() const { return m_webcore; }
    Node& wkc() { return m_wkc; }

    bool hasTagName(int) const;
    bool isFocusable() const;
    bool isHTMLElement() const;
    bool isElementNode() const;
    bool isFrameOwnerElement() const;
    bool inDocument() const;
    String nodeName() const;
    bool hasEventListeners(int);

    Document* document();
    RenderObject* renderer();
    Element* parentElement();

    Node* parent();
    Node* parentNode();
    Node* firstChild();
    Node* traverseNextNode();
    Node* traverseNextSibling();
    Node* shadowAncestorNode();

    bool isInShadowTree() const;

    NamedNodeMap* attributes();

    bool isContentEditable() const;
    String textContent(bool conv) const;
    void setTextContent(const String& text, int& ec);
    HTMLElement* toHTMLElement();
    void dispatchChangeEvent();

    const GraphicsLayer* enclosingGraphicsLayer() const;

    bool isScrollableOverFlowBlockNode() const;
    void getNodeCompositeRect(WKCRect* rect, int tx = 0, int ty = 0);

    NodeList* getElementsByTagName(const String&);

private:
    friend class ElementPrivate;
    friend class DocumentPrivate;
    NodePrivate(WebCore::Node*);
    NodePrivate(const WebCore::Node*);

private:
    WebCore::Node* m_webcore;
    Node m_wkc;

    DocumentPrivate* m_document;
    ElementPrivate* m_parentElement;
    RenderObjectPrivate* m_renderer;
    NamedNodeMapPrivate* m_parentNamedNodeMap;

    NodePrivate* m_parent;
    NodePrivate* m_parentNode;
    NodePrivate* m_firstChild;
    NodePrivate* m_traverseNextNode;
    NodePrivate* m_traverseNextSibling;
    NodePrivate* m_shadowAncestorNode;

    HTMLElementPrivate* m_HTMLElement;
    NodeListPrivate* m_nodeList;
};
} // namespace

#endif // _WKC_HELPERS_PRIVATE_NODE_H_
