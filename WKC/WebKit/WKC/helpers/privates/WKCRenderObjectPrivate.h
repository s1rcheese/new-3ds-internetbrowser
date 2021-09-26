/*
 * Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_PRIVATE_RENDEROBJECT_H_
#define _WKC_HELPERS_PRIVATE_RENDEROBJECT_H_

#include <wkc/wkcbase.h>
#include "helpers/WKCRenderObject.h"
#include "Vector.h"

namespace WebCore {
class RenderObject;
} // namespace

namespace WKC {
class WKCRingRectsPrivate {
public:
    WKCRingRectsPrivate(WTF::Vector<WKCRect>&);
    ~WKCRingRectsPrivate();
    int length() const;
    WKCRect getAt(int) const;
private:
    WKCRingRectsPrivate(const WKCRingRectsPrivate&);
    WKCRingRectsPrivate& operator=(const WKCRingRectsPrivate&);
private:
    WTF::Vector<WKCRect>* m_rects;
};

class RenderObjectPrivate {
public:
    RenderObjectPrivate(WebCore::RenderObject*);
    ~RenderObjectPrivate();

    const WebCore::RenderObject* webcore() const { return m_webcore; }
    RenderObject& wkc() { return m_wkc; }

    bool isTextControl() const;
    bool isTextArea() const;
    WKCRect absoluteBoundingBoxRect(bool usetransform);
    void focusRingRects(WTF::Vector<WKCRect>&);
    WKCRect absoluteClippedOverflowRect();
    bool hasOutline() const;

private:
    WebCore::RenderObject* m_webcore;
    RenderObject m_wkc;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_RENDEROBJECT_H_
