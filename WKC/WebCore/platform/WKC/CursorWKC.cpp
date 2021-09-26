/*
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Christian Dywan <christian@twotoasts.de>
 * All rights reserved.
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "Cursor.h"

#include "Image.h"
#include "IntPoint.h"
#include "ImageWKC.h"

#include "NotImplemented.h"
#include <wtf/Assertions.h>

#include "WKCEnums.h"
#include "helpers/ChromeClientIf.h"

namespace WebCore {

Cursor::Cursor(const Cursor& other)
{
    if (this==&other)
        return;
    if (other.m_platformCursor) {
        WKC::WKCPlatformCursor* o = reinterpret_cast<WKC::WKCPlatformCursor*>(other.m_platformCursor);
        WKC::WKCPlatformCursor* c = new WKC::WKCPlatformCursor(o->fType);
        if (o->fType==WKC::ECursorTypeCustom) {
            ImageWKC* wi = reinterpret_cast<ImageWKC*>(o->fData);
            wi->ref();
            c->fBitmap = wi->bitmap();
            c->fRowBytes = wi->rowbytes();
            c->fBPP = wi->bpp();
            c->fSize.fWidth = o->fSize.fWidth;
            c->fSize.fHeight = o->fSize.fHeight;
            c->fHotSpot.fX = o->fHotSpot.fX;
            c->fHotSpot.fY = o->fHotSpot.fY;
            c->fData = reinterpret_cast<void*>(wi);
        }
        m_platformCursor = reinterpret_cast<PlatformCursor>(c);
    } else {
        m_platformCursor = 0;
    }
}

Cursor::Cursor(Image* image, const IntPoint& hotSpot)
{
    WKC::WKCPlatformCursor* c = new WKC::WKCPlatformCursor(WKC::ECursorTypeCustom);
    ImageWKC* wi = reinterpret_cast<ImageWKC*>(image->nativeImageForCurrentFrame());
    if (!wi) {
        delete c;
        m_platformCursor = reinterpret_cast<PlatformCursor>(new WKC::WKCPlatformCursor(WKC::ECursorTypePointer));
        return;
    }
    wi->ref();
    c->fBitmap = wi->bitmap();
    c->fRowBytes = wi->rowbytes();
    c->fBPP = wi->bpp();
    c->fSize.fWidth = image->size().width();
    c->fSize.fHeight = image->size().height();
    c->fHotSpot.fX = hotSpot.x();
    c->fHotSpot.fY = hotSpot.y();
    c->fData = reinterpret_cast<void*>(wi);

    m_platformCursor = reinterpret_cast<PlatformCursor>(c);
}

Cursor::Cursor(WebCore::PlatformCursor cursor)
{
    WKC::WKCPlatformCursor* c = new WKC::WKCPlatformCursor((int)reinterpret_cast<intptr_t>(cursor));
    c->fSize.fWidth = 0;
    c->fSize.fHeight = 0;
    c->fHotSpot.fX = 0;
    c->fHotSpot.fY = 0;
    m_platformCursor = reinterpret_cast<PlatformCursor>(c);
}

Cursor::~Cursor()
{
    if (!m_platformCursor)
        return;

    WKC::WKCPlatformCursor* c = reinterpret_cast<WKC::WKCPlatformCursor*>(m_platformCursor);
    if (c->fData) {
        ImageWKC* wi = reinterpret_cast<ImageWKC*>(c->fData);
        wi->unref();
    }
    delete c;
}

Cursor& Cursor::operator=(const Cursor& other)
{
    if (this==&other)
        return *this;

    WKC::WKCPlatformCursor* cur = reinterpret_cast<WKC::WKCPlatformCursor *>(m_platformCursor);
    if (cur) {
        if (cur->fData) {
            ImageWKC* wi = reinterpret_cast<ImageWKC*>(cur->fData);
            wi->unref();
        }
        delete cur;
    }
    m_platformCursor = 0;

    if (other.m_platformCursor) {
        WKC::WKCPlatformCursor* o = reinterpret_cast<WKC::WKCPlatformCursor*>(other.m_platformCursor);
        WKC::WKCPlatformCursor* c = new WKC::WKCPlatformCursor(o->fType);
        if (o->fType==WKC::ECursorTypeCustom) {
            ImageWKC* wi = reinterpret_cast<ImageWKC*>(o->fData);
            wi->ref();
            c->fBitmap = wi->bitmap();
            c->fRowBytes = wi->rowbytes();
            c->fBPP = wi->bpp();
            c->fSize.fWidth = o->fSize.fWidth;
            c->fSize.fHeight = o->fSize.fHeight;
            c->fHotSpot.fX = o->fHotSpot.fX;
            c->fHotSpot.fY = o->fHotSpot.fY;
            c->fData = reinterpret_cast<void*>(wi);
        }
        m_platformCursor = reinterpret_cast<PlatformCursor>(c);
    }
    return *this;
}

const Cursor& pointerCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypePointer));
    return cCursor;
}

const Cursor& crossCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeCross));
    return cCursor;
}

const Cursor& handCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeHand));
    return cCursor;
}

const Cursor& moveCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeMove));
    return cCursor;
}

const Cursor& iBeamCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeIBeam));
    return cCursor;
}

const Cursor& waitCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeWait));
    return cCursor;
}

const Cursor& helpCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeHelp));
    return cCursor;
}

const Cursor& eastResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeEastResize));
    return cCursor;
}

const Cursor& northResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthResize));
    return cCursor;
}

const Cursor& northEastResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthEastResize));
    return cCursor;
}

const Cursor& northWestResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthWestResize));
    return cCursor;
}

const Cursor& southResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeSouthResize));
    return cCursor;
}

const Cursor& southEastResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeSouthEastResize));
    return cCursor;
}

const Cursor& southWestResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeSouthWestResize));
    return cCursor;
}

const Cursor& westResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeWestResize));
    return cCursor;
}

const Cursor& northSouthResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthSouthResize));
    return cCursor;
}

const Cursor& eastWestResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeEastWestResize));
    return cCursor;
}

const Cursor& northEastSouthWestResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthEastSouthWestResize));
    return cCursor;
}

const Cursor& northWestSouthEastResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthWestSouthEastResize));
    return cCursor;
}

const Cursor& columnResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeColumnResize));
    return cCursor;
}

const Cursor& rowResizeCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeRowResize));
    return cCursor;
}
    
const Cursor& middlePanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeMiddlePanning));
    return cCursor;
}

const Cursor& eastPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeEastPanning));
    return cCursor;
}

const Cursor& northPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthPanning));
    return cCursor;
}

const Cursor& northEastPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthEastPanning));
    return cCursor;
}

const Cursor& northWestPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNorthWestPanning));
    return cCursor;
}

const Cursor& southPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeSouthPanning));
    return cCursor;
}

const Cursor& southEastPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeSouthEastPanning));
    return cCursor;
}

const Cursor& southWestPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeSouthWestPanning));
    return cCursor;
}

const Cursor& westPanningCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeWestPanning));
    return cCursor;
}
    

const Cursor& verticalTextCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeVerticalText));
    return cCursor;
}

const Cursor& cellCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeCell));
    return cCursor;
}

const Cursor& contextMenuCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeContextMenu));
    return cCursor;
}

const Cursor& noDropCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNoDrop));
    return cCursor;
}

const Cursor& copyCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeCopy));
    return cCursor;
}

const Cursor& progressCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeProgress));
    return cCursor;
}

const Cursor& aliasCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeAlias));
    return cCursor;
}

const Cursor& noneCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNone));
    return cCursor;
}

const Cursor& notAllowedCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeNotAllowed));
    return cCursor;
}

const Cursor& zoomInCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeZoomIn));
    return cCursor;
}

const Cursor& zoomOutCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeZoomOut));
    return cCursor;
}

const Cursor& grabCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeGrab));
    return cCursor;
}

const Cursor& grabbingCursor()
{
    DEFINE_STATIC_LOCAL(Cursor, cCursor, ((PlatformCursor)WKC::ECursorTypeGrabbing));
    return cCursor;
}

}
