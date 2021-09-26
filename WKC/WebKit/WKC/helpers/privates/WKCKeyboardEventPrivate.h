/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_PRIVATE_KEYBOARDEVENT_H_
#define _WKC_HELPERS_PRIVATE_KEYBOARDEVENT_H_

#include <wkc/wkcbase.h>

#include "helpers/WKCKeyboardEvent.h"

#include "helpers/privates/WKCEventPrivate.h"

#include "WKCPlatformEvents.h"

namespace WebCore {
class KeyboardEvent;
} // namespace

namespace WKC {

class KeyboardEvent;

class KeyboardEventPrivate : public EventPrivate {
public:
    KeyboardEventPrivate(WebCore::KeyboardEvent*);
    ~KeyboardEventPrivate();

    KeyboardEvent& wkc() { return reinterpret_cast<KeyboardEvent&>(EventPrivate::wkc()); }

    WKCKeyEvent keyEvent() const { return m_keyEvent; }

private:
    WKCKeyEvent m_keyEvent;
};


} // namespace

#endif // _WKC_HELPERS_PRIVATE_KEYBOARDEVENT_H_
