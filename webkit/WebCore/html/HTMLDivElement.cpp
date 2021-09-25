/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2013 ACCESS CO., LTD. All rights reserved.
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
#include "HTMLDivElement.h"

#include "Attribute.h"
#include "CSSPropertyNames.h"
#include "CSSValueKeywords.h"
#include "HTMLNames.h"
#if PLATFORM(WKC)
#include "Event.h"
#endif

namespace WebCore {

using namespace HTMLNames;

HTMLDivElement::HTMLDivElement(const QualifiedName& tagName, Document* document)
    : HTMLElement(tagName, document)
{
    ASSERT(hasTagName(divTag));
}

PassRefPtr<HTMLDivElement> HTMLDivElement::create(Document* document)
{
    return adoptRef(new HTMLDivElement(divTag, document));
}

PassRefPtr<HTMLDivElement> HTMLDivElement::create(const QualifiedName& tagName, Document* document)
{
    return adoptRef(new HTMLDivElement(tagName, document));
}

bool HTMLDivElement::isPresentationAttribute(const QualifiedName& name) const
{
    if (name == alignAttr)
        return true;
    return HTMLElement::isPresentationAttribute(name);
}

void HTMLDivElement::collectStyleForAttribute(Attribute* attr, StylePropertySet* style)
{
    if (attr->name() == alignAttr) {
        if (equalIgnoringCase(attr->value(), "middle") || equalIgnoringCase(attr->value(), "center"))
            addPropertyToAttributeStyle(style, CSSPropertyTextAlign, CSSValueWebkitCenter);
        else if (equalIgnoringCase(attr->value(), "left"))
            addPropertyToAttributeStyle(style, CSSPropertyTextAlign, CSSValueWebkitLeft);
        else if (equalIgnoringCase(attr->value(), "right"))
            addPropertyToAttributeStyle(style, CSSPropertyTextAlign, CSSValueWebkitRight);
        else
            addPropertyToAttributeStyle(style, CSSPropertyTextAlign, attr->value());
    } else
        HTMLElement::collectStyleForAttribute(attr, style);
}

#if PLATFORM(WKC)
// Prevent capturing/bubbling event handlers caused by operations on the control bar of media player.
void* HTMLDivElement::preDispatchEventHandler(Event* event)
{
    if (event->isMouseEvent()) {
        if (shadowPseudoId() == AtomicString("-webkit-slider-runnable-track")) {
            Node* n = shadowAncestorNode();
            ASSERT(n);
            if (n->isElementNode() && static_cast<Element*>(n)->shadowPseudoId() == AtomicString("-webkit-media-controls-timeline"))
                event->stopPropagation();
        }
    }
    return HTMLElement::preDispatchEventHandler(event);
}
#endif

}
