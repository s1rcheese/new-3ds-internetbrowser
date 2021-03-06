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

#ifndef _WKC_HELPERS_WKC_DOCUMENT_H_
#define _WKC_HELPERS_WKC_DOCUMENT_H_

#include <wkc/wkcbase.h>

#include "WKCNode.h"

namespace WKC {

class CachedResourceLoader;
class Element;
class Frame;
class HTMLCollection;
class Node;
class RenderView;
class String;
class KURL;

class DocumentPrivate;

class WKC_API Document : public WKC::Node {
public:
    Document(DocumentPrivate&);
    virtual ~Document();

    bool isImageDocument() const;
    bool isSVGDocument() const;

    RenderView* renderView() const;
    CachedResourceLoader* cachedResourceLoader() const;
    Frame* frame() const;

    Node* focusedNode() const;
    Node* firstChild() const;
    HTMLCollection* forms() const;

    void updateLayoutIgnorePendingStylesheets();

    KURL completeURL(const String&) const;

    bool loadEventFinished() const;

    void webkitWillEnterFullScreenForElement(Element*);
    void webkitDidEnterFullScreenForElement(Element*);
    void webkitWillExitFullScreenForElement(Element*);
    void webkitDidExitFullScreenForElement(Element*);

    DocumentPrivate& priv() const { return m_private; }

private:
    DocumentPrivate& m_private;
};
}

#endif // _WKC_HELPERS_WKC_DOCUMENT_H_
