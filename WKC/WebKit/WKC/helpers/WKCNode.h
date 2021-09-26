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

#ifndef _WKC_HELPERS_WKC_NODE_H_
#define _WKC_HELPERS_WKC_NODE_H_

#include <wkc/wkcbase.h>

namespace WKC {
class Document;
class Element;
class NodeList;
class RenderObject;
class String;

class NodePrivate;
class NamedNodeMap;

class HTMLElement;

class GraphicsLayer;

enum {
    HTMLNames_inputTag,
    HTMLNames_textareaTag,
    HTMLNames_selectTag,
    HTMLNames_formTag,
    HTMLNames_frameTag,
    HTMLNames_iframeTag,
    HTMLNames_videoTag,
    HTMLNames_areaTag,
};

enum {
    eventNames_clickEvent,
    eventNames_mousedownEvent,
    eventNames_mousemoveEvent,
    eventNames_dragEvent,
    eventNames_dragstartEvent,
    eventNames_dragendEvent,
    eventNames_touchstartEvent,
    eventNames_touchmoveEvent,
    eventNames_touchendEvent,
    eventNames_touchcancelEvent,
};

class WKC_API Node {
public:
    Node(NodePrivate&);
    Node(Node*, bool needsRef = false);
    virtual ~Node();

    bool compare(const Node*) const;

    bool hasTagName(int) const;
    bool isFocusable() const;
    bool isHTMLElement() const;
    bool isElementNode() const;
    bool isFrameOwnerElement() const;
    bool inDocument() const;
    String nodeName() const;

    bool hasEventListeners(int);

    Document* document() const;
    RenderObject* renderer() const;
    Element* parentElement() const;

    Node* parent() const;
    Node* parentNode() const;
    Node* firstChild() const;
    Node* traverseNextNode() const;
    Node* traverseNextSibling() const;
    Node* shadowAncestorNode() const;

    bool isInShadowTree() const;

    NodePrivate& priv() const { return m_private; }

    NamedNodeMap* attributes() const;

    bool isContentEditable() const;
    String textContent(bool conv) const;
    void setTextContent(const String& text, int& ec);
    HTMLElement* toHTMLElement() const;
    void dispatchChangeEvent();

    const GraphicsLayer* enclosingGraphicsLayer() const;

    bool isScrollableOverFlowBlockNode() const;
    void getNodeCompositeRect(WKCRect* rect, int tx = 0, int ty = 0);

    NodeList* getElementsByTagName(const String&);

protected:
    NodePrivate* m_ownedPrivate;
    NodePrivate& m_private;
    bool m_needsRef;
};
}

#endif // _WKC_HELPERS_WKC_NODE_H_
