/*
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCAtomicString.h"
#include "helpers/privates/WKCAtomicStringPrivate.h"

#include "helpers/WKCString.h"

#include "AtomicString.h"
/*
namespace WKC {
    //const AtomicString nullAtom;
    //DEFINE_STATIC_LOCAL(AtomicString, nullAtom, 0);
    //WKC_DEFINE_GLOBAL_PTR(const WKC::AtomicString, nullAtom, 0);
    WKC_DEFINE_GLOBAL_CLASS_OBJ(AtomicString*, AtomicString, m_nullAtom, 0);
} // namespace
*/
namespace WKC {

AtomicStringPrivate::AtomicStringPrivate()
    : m_webcoreowner(1)
{
    m_wkc = 0;
    m_webcore = new WTF::AtomicString();
}

AtomicStringPrivate::AtomicStringPrivate(WTF::AtomicString* value)
    : m_webcore(value)
    , m_webcoreowner(0)
{
    m_wkc = new AtomicString(this);
}

AtomicStringPrivate::~AtomicStringPrivate()
{
    delete m_wkc;
    if (m_webcoreowner)
        delete m_webcore;
}

AtomicStringPrivate::AtomicStringPrivate(const AtomicStringPrivate& other)
{
    if (this!=&other)
        ::memcpy(this, &other, sizeof(AtomicStringPrivate));
}

AtomicStringPrivate&
AtomicStringPrivate::operator =(const AtomicStringPrivate& other)
{
    if (this!=&other) {
        ::memcpy(this, &other, sizeof(AtomicStringPrivate));
    }

    return *this;
}

String&
AtomicStringPrivate::string()
{
    m_wkc_string = m_webcore->string();
    return m_wkc_string;
}

AtomicString::AtomicString()
    : m_privateowner(true)
{
    m_private = new AtomicStringPrivate();
}

AtomicString::AtomicString(AtomicStringPrivate* priv)
    : m_privateowner(false)
{
    m_private = priv;
}

AtomicString::AtomicString(const AtomicString& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_privateowner = false;
    }
}

AtomicString&
AtomicString::operator =(const AtomicString& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_privateowner = false;
    }
    return *this;
}

AtomicString::~AtomicString()
{
    if (m_privateowner)
        delete m_private;
}

const String&
AtomicString::string() const
{
    return m_private->string();
}

} // namespace