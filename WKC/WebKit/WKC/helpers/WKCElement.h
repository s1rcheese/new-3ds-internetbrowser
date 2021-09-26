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

#ifndef _WKC_HELPERS_WKC_ELEMENT_H_
#define _WKC_HELPERS_WKC_ELEMENT_H_

#include <wkc/wkcbase.h>

#include "WKCNode.h"

namespace WKC {
class Attribute;
class QualifiedName;
class ElementPrivate;
class WKC_API Element : public WKC::Node {
public:
    Element(ElementPrivate&);
    virtual ~Element();

    bool isFormControlElement() const;
    void setChangedSinceLastFormControlChangeEvent(bool);
    bool hasAttributes() const;
    bool hasAttribute(const String& name) const;
    Attribute* getAttributeItem(const QualifiedName& name) const;
    void focus(bool restorePreviousSelection = true);
    void updateFocusAppearance(bool restorePreviousSelection);
    void blur();
};
}

#endif // _WKC_HELPERS_WKC_ELEMENT_H_
