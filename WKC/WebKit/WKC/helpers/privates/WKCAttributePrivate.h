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

#ifndef _WKC_HELPERS_PRIVATE_ATTRIBUTE_H_
#define _WKC_HELPERS_PRIVATE_ATTRIBUTE_H_

#include "helpers/WKCAttribute.h"
#include "Attribute.h"

namespace WebCore {
class QualifiedName;
}
namespace WTF {
class AtomicString;
} // namespace

namespace WKC {
class AtomicString;
class AtomicStringPrivate;
} // namespace

namespace WKC {

class AttributePrivate {
public:
    AttributePrivate(WebCore::Attribute *webcore);
    ~AttributePrivate();
    WebCore::Attribute* webcore() const { return m_webcore; }
    Attribute& wkc() { return m_wkc; }
    const AtomicString& value();

private:
    WebCore::Attribute* m_webcore;
    WTF::AtomicString m_value;
    Attribute m_wkc;
    AtomicStringPrivate *m_atomicstring_priv;
};

} // namespace

#endif //_WKC_HELPERS_PRIVATE_ATTRIBUTE_H_
