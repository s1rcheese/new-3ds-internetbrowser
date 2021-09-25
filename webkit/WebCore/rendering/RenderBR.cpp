/**
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2006 Apple Computer, Inc.
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
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
 *
 */

#include "config.h"
#include "RenderBR.h"

#include "Document.h"
#include "InlineTextBox.h"
#include "VisiblePosition.h"

namespace WebCore {

RenderBR::RenderBR(Node* node)
    : RenderText(node, StringImpl::create("\n"))
    , m_lineHeight(-1)
{
}

RenderBR::~RenderBR()
{
}

int RenderBR::lineHeight(bool firstLine) const
{
    if (firstLine && document()->usesFirstLineRules()) {
        RenderStyle* s = style(firstLine);
        if (s != style())
            return s->computedLineHeight(view());
    }
    
    if (m_lineHeight == -1)
        m_lineHeight = style()->computedLineHeight(view());
    
    return m_lineHeight;
}

void RenderBR::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    RenderText::styleDidChange(diff, oldStyle);
    m_lineHeight = -1;
}

int RenderBR::caretMinOffset() const 
{ 
    return 0;
}

int RenderBR::caretMaxOffset() const 
{ 
    return 1;
}

VisiblePosition RenderBR::positionForPoint(const LayoutPoint&)
{
    return createVisiblePosition(0, DOWNSTREAM);
}

#if PLATFORM(WKC)
using namespace std;

// copied from RenderText::absoluteRectsForRange()
void RenderBR::absoluteBRRectsForRange(Vector<IntRect>& rects, unsigned start, unsigned end, bool useSelectionHeight)
{
    // Work around signed/unsigned issues. This function takes unsigneds, and is often passed UINT_MAX
    // to mean "all the way to the end". InlineTextBox coordinates are unsigneds, so changing this 
    // function to take ints causes various internal mismatches. But selectionRect takes ints, and 
    // passing UINT_MAX to it causes trouble. Ideally we'd change selectionRect to take unsigneds, but 
    // that would cause many ripple effects, so for now we'll just clamp our unsigned parameters to INT_MAX.
    ASSERT(end == UINT_MAX || end <= INT_MAX);
    ASSERT(start <= INT_MAX);
    start = min(start, static_cast<unsigned>(INT_MAX));
    end = min(end, static_cast<unsigned>(INT_MAX));
    
    for (InlineTextBox* box = firstTextBox(); box; box = box->nextTextBox()) {
        // Note: box->end() returns the index of the last character, not the index past it
        if (start <= box->start() && box->end() < end) {
            FloatRect r = box->calculateBoundaries();
            if (useSelectionHeight) {
                FloatRect selectionRect = box->localSelectionRect(start, end);
                if (box->isHorizontal()) {
                    r.setHeight(selectionRect.height());
                    r.setY(selectionRect.y());
                } else {
                    r.setWidth(selectionRect.width());
                    r.setX(selectionRect.x());
                }
            }
            rects.append(localToAbsoluteQuad(FloatQuad(r)).enclosingBoundingBox());
        } else {
            unsigned realEnd = min(box->end() + 1, end);
            FloatRect r = box->localSelectionRect(start, realEnd);
			r.setWidth(1);
            if (!r.isEmpty()) {
                if (!useSelectionHeight) {
                    // change the height and y position because selectionRect uses selection-specific values
                    if (box->isHorizontal()) {
                        r.setHeight(box->logicalHeight());
                        r.setY(box->y());
                    } else {
                        r.setWidth(box->logicalWidth());
                        r.setX(box->x());
                    }
                }
                rects.append(localToAbsoluteQuad(FloatQuad(r)).enclosingBoundingBox());
            }
        }
    }
}
#endif

} // namespace WebCore
