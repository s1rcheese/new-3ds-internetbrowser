/*
 *  Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include "helpers/WKCHTMLFormElement.h"
#include "helpers/privates/WKCHTMLFormElementPrivate.h"

#include "HTMLFormElement.h"
#include "HTMLCollection.h"
#include "HTMLNames.h"

namespace WKC {

HTMLFormElementPrivate::HTMLFormElementPrivate(WebCore::HTMLFormElement* parent)
    : HTMLElementPrivate(parent)
    , m_item(0)
{
}

HTMLFormElementPrivate::~HTMLFormElementPrivate()
{
    delete m_item;
}

void
HTMLFormElementPrivate::reset()
{
    reinterpret_cast<WebCore::HTMLFormElement *>(webcore())->reset();
}

unsigned
HTMLFormElementPrivate::length() const
{
    return reinterpret_cast<WebCore::HTMLFormElement *>(webcore())->length();
}

Node*
HTMLFormElementPrivate::item(unsigned index)
{
    WebCore::Node* node = reinterpret_cast<WebCore::HTMLFormElement *>(webcore())->item(index);
    if (!node)
        return 0;

    if (!m_item || m_item->webcore() != node) {
        delete m_item;
        m_item = NodePrivate::create(node);
    }
    return &m_item->wkc();
}

bool
HTMLFormElementPrivate::shouldAutocomplete() const
{
    return reinterpret_cast<WebCore::HTMLFormElement *>(webcore())->shouldAutocomplete();
}

void
HTMLFormElement::reset()
{
    reinterpret_cast<HTMLFormElementPrivate&>(priv()).reset();
}

unsigned
HTMLFormElement::length() const
{
    return reinterpret_cast<HTMLFormElementPrivate&>(priv()).length();
}

Node*
HTMLFormElement::item(unsigned index)
{
    return reinterpret_cast<HTMLFormElementPrivate&>(priv()).item(index);
}

bool
HTMLFormElement::shouldAutocomplete() const
{
    return reinterpret_cast<HTMLFormElementPrivate&>(priv()).shouldAutocomplete();
}

} // namespace
