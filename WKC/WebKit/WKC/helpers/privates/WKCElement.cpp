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

#include "helpers/WKCElement.h"
#include "helpers/privates/WKCElementPrivate.h"


#include "Element.h"


#include "helpers/WKCNode.h"
#include "helpers/WKCQualifiedName.h"
#include "helpers/privates/WKCAttributePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCQualifiedNamePrivate.h"

namespace WKC {

ElementPrivate::ElementPrivate(WebCore::Element* parent)
    : NodePrivate(parent)
    , m_webcore(parent)
    , m_wkc(*this)
    , m_attr(0)
{
}

ElementPrivate::~ElementPrivate()
{
    delete m_attr;
}


Element::Element(ElementPrivate& parent)
    : Node(parent)
{
}

Element::~Element()
{
}

bool
ElementPrivate::isFormControlElement() const
{
    return ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->isFormControlElement();
}

void
ElementPrivate::setChangedSinceLastFormControlChangeEvent(bool flag)
{
    ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->setChangedSinceLastFormControlChangeEvent(flag);
}

bool
ElementPrivate::hasAttributes() const
{
    return ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->hasAttributes();
}

bool
ElementPrivate::hasAttribute(const String& name) const
{
    return ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->hasAttribute(name);
}

Attribute*
ElementPrivate::getAttributeItem(const QualifiedName* name)
{
    WebCore::Attribute *attr = m_webcore->getAttributeItem(*(name->priv()->webcore()));
    if (!attr) {
        return 0;
    }
    if (!m_attr || m_attr->webcore() != attr) {
        delete m_attr;
        m_attr = new AttributePrivate(attr);
    }
    return &m_attr->wkc();
}

void
ElementPrivate::focus(bool restorePreviousSelection)
{
    ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->focus(restorePreviousSelection);
}

void
ElementPrivate::updateFocusAppearance(bool restorePreviousSelection)
{
    ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->updateFocusAppearance(restorePreviousSelection);
}

void
ElementPrivate::blur()
{
    ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->blur();
}


bool
Element::isFormControlElement() const
{
    return ((ElementPrivate&)priv()).isFormControlElement();
}

void
Element::setChangedSinceLastFormControlChangeEvent(bool flag)
{
    ((ElementPrivate&)priv()).setChangedSinceLastFormControlChangeEvent(flag);
}

bool
Element::hasAttributes() const
{
    return ((ElementPrivate&)priv()).hasAttributes();
}

bool
Element::hasAttribute(const String& name) const
{
    return ((ElementPrivate&)priv()).hasAttribute(name);
}

Attribute*
Element::getAttributeItem(const QualifiedName& name) const
{
    return ((ElementPrivate&)priv()).getAttributeItem(&name);
}

void
Element::focus(bool restorePreviousSelection)
{
    ((ElementPrivate&)priv()).focus(restorePreviousSelection);
}

void
Element::updateFocusAppearance(bool restorePreviousSelection)
{
    ((ElementPrivate&)priv()).updateFocusAppearance(restorePreviousSelection);
}

void
Element::blur()
{
    ((ElementPrivate&)priv()).blur();
}


} // namespace
