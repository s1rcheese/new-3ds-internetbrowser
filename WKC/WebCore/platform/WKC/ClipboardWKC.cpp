/*
 * Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
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
#include "ClipboardWKC.h"

#include "CachedImage.h"
#include "CString.h"
#include "DragData.h"
#include "Editor.h"
#include "Element.h"
#include "Frame.h"
#include "FileList.h"
#include "markup.h"
#include "NotImplemented.h"
#include "RenderImage.h"
#include "StringHash.h"
#include "DataTransferItemList.h"

namespace WebCore {

PassRefPtr<Clipboard> Clipboard::create(ClipboardAccessPolicy policy, DragData* dragData, Frame* frame)
{
    return ClipboardWKC::create(Clipboard::DragAndDrop, policy, dragData, frame);
}

PassRefPtr<Clipboard> Editor::newGeneralClipboard(ClipboardAccessPolicy policy, Frame* frame)
{
    return ClipboardWKC::create(Clipboard::DragAndDrop, policy, 0, frame);
}

ClipboardWKC::ClipboardWKC(ClipboardType type, ClipboardAccessPolicy policy, DragData* data, Frame* frame)
    : Clipboard(policy, type)
    , m_data(data)
    , m_frame(frame)
{
}

ClipboardWKC::~ClipboardWKC()
{
}

void ClipboardWKC::clearData(const String&)
{
    notImplemented();
}

void ClipboardWKC::clearAllData()
{
    notImplemented();
}

String ClipboardWKC::getData(const String&) const
{
    notImplemented();
    return String();
}

bool ClipboardWKC::setData(const String&, const String&)
{
    notImplemented();
    return false;
}

HashSet<String> ClipboardWKC::types() const
{
    notImplemented();
    return HashSet<String>();
}

PassRefPtr<FileList> ClipboardWKC::files() const
{
    notImplemented();
    return 0;
}

IntPoint ClipboardWKC::dragLocation() const
{
    notImplemented();
    return IntPoint(0, 0);
}

CachedImage* ClipboardWKC::dragImage() const
{
    notImplemented();
    return 0;
}

void ClipboardWKC::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}

Node* ClipboardWKC::dragImageElement()
{
    notImplemented();
    return 0;
}

void ClipboardWKC::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

DragImageRef ClipboardWKC::createDragImage(IntPoint&) const
{
    notImplemented();
    return 0;
}

void ClipboardWKC::declareAndWriteDragImage(Element* element, const KURL& url, const String& label, Frame*)
{
    // Ugh!: implement something!
    // 091228 ACCESS Co.,Ltd.
    notImplemented();
}

void ClipboardWKC::writeURL(const KURL& url, const String& label, Frame*)
{
    // Ugh!: implement something!
    // 091228 ACCESS Co.,Ltd.
    notImplemented();
}

void ClipboardWKC::writeRange(Range* range, Frame* frame)
{
    // Ugh!: implement something!
    // 091228 ACCESS Co.,Ltd.
    notImplemented();
}

void ClipboardWKC::writePlainText(const String&)
{
    // Ugh!: implement it!
    // 110621 ACCESS Co.,Ltd.
    notImplemented();
}

bool ClipboardWKC::hasData()
{
    notImplemented();
    return false;
}

#if ENABLE(DATA_TRANSFER_ITEMS)
PassRefPtr<DataTransferItemList> ClipboardWKC::items()
{
    // Ugh!: support it!
    // 110621 ACCESS Co.,Ltd.
    notImplemented();
    return 0;
}
#endif


}
