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

#ifndef _WKC_HELPERS_PRIVATE_DOCUMENT_H_
#define _WKC_HELPERS_PRIVATE_DOCUMENT_H_

#include "helpers/WKCDocument.h"
#include "WKCNodePrivate.h"

namespace WebCore {
class Document;
class Element;
} // namespace

namespace WKC {

class CachedResourceLoader;
class Frame;
class Node;
class RenderView;
class CachedResourceLoaderPrivate;
class FramePrivate;
class HTMLCollectionPrivate;
class NodePrivate;
class RenderViewPrivate;

class DocumentPrivate : public NodePrivate {
public:
    DocumentPrivate(WebCore::Document*);
    ~DocumentPrivate();

    WebCore::Document* webcore() const { return m_webcore; }
    Document& wkc() { return m_wkc; }

    bool isImageDocument() const;
    bool isSVGDocument() const;
    void updateLayoutIgnorePendingStylesheets();
    KURL completeURL(const String&) const;

    Node* focusedNode();
    RenderView* renderView();
    CachedResourceLoader* cachedResourceLoader();

    Frame* frame();
    Node* firstChild();

    HTMLCollection* forms();

    bool loadEventFinished() const;

    void webkitWillEnterFullScreenForElement(Element*);
    void webkitDidEnterFullScreenForElement(Element*);
    void webkitWillExitFullScreenForElement(Element*);
    void webkitDidExitFullScreenForElement(Element*);

private:
    WebCore::Document* m_webcore;
    Document m_wkc;

    CachedResourceLoaderPrivate* m_cachedResourceLoader;
    NodePrivate* m_focusedNode;
    RenderViewPrivate* m_renderView;
    FramePrivate* m_frame;
    NodePrivate* m_firstChild;
    HTMLCollectionPrivate* m_collection;
};
} // namespace

#endif // _WKC_HELPERS_PRIVATE_DOCUMENT_H_

