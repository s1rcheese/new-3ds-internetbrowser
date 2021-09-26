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

#ifndef _WKC_HELPERS_WKC_FOCUSCONTROLLER_H_
#define _WKC_HELPERS_WKC_FOCUSCONTROLLER_H_

#include <wkc/wkcbase.h>

#include "WKCEnums.h"

namespace WKC {
class Frame;
class FocusControllerPrivate;
class Node;

class WKC_API FocusController {
public:
    FocusController(FocusControllerPrivate&);
    ~FocusController();

    Frame* focusedOrMainFrame();
    FocusControllerPrivate& priv() const { return m_private; }

    Node* findNextFocusableNode(const FocusDirection direction, const WKCRect* specificRect);
    Node* findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const WKCRect* rect, bool enableContainer);
    Node* findNearestFocusableNodeFromPoint(const WKCPoint point, const WKCRect* rect = 0);

private:
    FocusControllerPrivate& m_private;
};

bool isScrollableContainerNode(Node*);
bool hasOffscreenRect(Node*);

}

#endif // _WKC_HELPERS_WKC_FOCUSCONTROLLER_H_
