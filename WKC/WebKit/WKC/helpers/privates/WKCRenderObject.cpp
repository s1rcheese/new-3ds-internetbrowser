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

#include "config.h"

#include "helpers/WKCRenderObject.h"
#include "helpers/privates/WKCRenderObjectPrivate.h"

#include "RenderObject.h"

namespace WKC {
RenderObjectPrivate::RenderObjectPrivate(WebCore::RenderObject* parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

RenderObjectPrivate::~RenderObjectPrivate()
{
}
 
bool
RenderObjectPrivate::isTextControl() const
{
    return m_webcore->isTextControl();
}

bool
RenderObjectPrivate::isTextArea() const
{
    return m_webcore->isTextArea();
}

WKCRect
RenderObjectPrivate::absoluteBoundingBoxRect(bool usetransform)
{
    return m_webcore->absoluteBoundingBoxRect(usetransform);
}

void
RenderObjectPrivate::focusRingRects(WTF::Vector<WKCRect>& rects)
{
    WTF::Vector<WebCore::IntRect> int_rects;
    m_webcore->focusRingRects(int_rects);
    const size_t size = int_rects.size();
    rects.grow(size);
    for (int i = 0; i < size; i++) {
        rects[i] = int_rects[i];
    }
}

WKCRect
RenderObjectPrivate::absoluteClippedOverflowRect()
{
    return m_webcore->absoluteClippedOverflowRect();
}

bool
RenderObjectPrivate::hasOutline() const
{
    return m_webcore->hasOutline();
}

RenderObject::RenderObject(RenderObjectPrivate& parent)
    : m_private(parent)
{
}

RenderObject::~RenderObject()
{
}


bool
RenderObject::isTextControl() const
{
    return m_private.isTextControl();
}

bool
RenderObject::isTextArea() const
{
    return m_private.isTextArea();
}

WKCRect
RenderObject::absoluteBoundingBoxRect(bool usetransform) const
{
    return m_private.absoluteBoundingBoxRect(usetransform);
}

WKCRingRects *
RenderObject::focusRingRects() const
{
    WTF::Vector<WKCRect> core_rects;
    m_private.focusRingRects(core_rects);
    WKCRingRectsPrivate *p = new WKCRingRectsPrivate(core_rects);
    return new WKCRingRects(p);
}

WKCRect
RenderObject::absoluteClippedOverflowRect()
{
    return m_private.absoluteClippedOverflowRect();
}

bool
RenderObject::hasOutline() const
{
    return m_private.hasOutline();
}


WKCRingRectsPrivate::WKCRingRectsPrivate(WTF::Vector<WKCRect>& rects)
    : m_rects(new WTF::Vector<WKCRect>(rects))
{
}
WKCRingRectsPrivate::~WKCRingRectsPrivate() {
    delete m_rects;
}
int
WKCRingRectsPrivate::length() const
{
    return m_rects->size();
}
WKCRect
WKCRingRectsPrivate::getAt(int i) const
{
    return m_rects->at(i);
}

void
WKCRingRects::destroy(WKCRingRects *self)
{
    delete self;
}

WKCRingRects::WKCRingRects(WKCRingRectsPrivate* p)
    : m_private(p)
{
}
WKCRingRects::~WKCRingRects()
{
    delete m_private;
}
int
WKCRingRects::length() const
{
    return m_private->length();
}
WKCRect
WKCRingRects::getAt(int i) const
{
    return m_private->getAt(i);
}

} // namespace
