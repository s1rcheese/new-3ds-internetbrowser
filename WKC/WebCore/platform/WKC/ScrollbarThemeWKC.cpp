/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include <stdio.h>
#include "GraphicsContext.h"
#include "Scrollbar.h"
#include "ScrollbarThemeComposite.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

class RenderThemeWKC;

class ScrollbarThemeWKC : public ScrollbarThemeComposite
{
public:
    virtual ~ScrollbarThemeWKC();
    virtual int scrollbarThickness(ScrollbarControlSize = RegularScrollbar);

    void splitTrack(ScrollbarThemeClient* scrollbar, const IntRect& track, IntRect& startTrack, IntRect& thumb, IntRect& endTrack) {return WebCore::ScrollbarThemeComposite::splitTrack(scrollbar, track, startTrack, thumb, endTrack);}

    virtual IntRect backButtonRect(ScrollbarThemeClient*, ScrollbarPart, bool painting = false);
    virtual IntRect forwardButtonRect(ScrollbarThemeClient*, ScrollbarPart, bool painting = false);
    virtual IntRect trackRect(ScrollbarThemeClient*, bool painting = false);

    virtual void paintScrollCorner(ScrollView*, GraphicsContext*, const IntRect& cornerRect);

protected:
    virtual bool hasButtons(ScrollbarThemeClient*);
    virtual bool hasThumb(ScrollbarThemeClient*);

    virtual int minimumThumbLength(ScrollbarThemeClient*);

    virtual void invalidatePart(ScrollbarThemeClient*, ScrollbarPart);
    
    virtual void paintScrollbarBackground(GraphicsContext*, ScrollbarThemeClient*) {}
    virtual void paintTrackBackground(GraphicsContext*, ScrollbarThemeClient*, const IntRect&);
    virtual void paintTrackPiece(GraphicsContext*, ScrollbarThemeClient*, const IntRect&, ScrollbarPart) {}
    virtual void paintButton(GraphicsContext*, ScrollbarThemeClient*, const IntRect&, ScrollbarPart);
    virtual void paintThumb(GraphicsContext*, ScrollbarThemeClient*, const IntRect&);
    virtual void paintTickmarks(GraphicsContext*, ScrollbarThemeClient*, const IntRect&) {}

    virtual bool paint(ScrollbarThemeClient*, GraphicsContext*, const IntRect& damageRect);
};

ScrollbarTheme* ScrollbarTheme::nativeTheme()
{
    WKC_DEFINE_STATIC_PTR(ScrollbarThemeWKC*, gTheme, new ScrollbarThemeWKC)
    return gTheme;
}

ScrollbarThemeWKC::~ScrollbarThemeWKC()
{
}

int
ScrollbarThemeWKC::scrollbarThickness(ScrollbarControlSize controlSize)
{    
    if (controlSize == RegularScrollbar) {
        unsigned int w,h;
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w, &h);
        return h;
    } else {
        return 0;
    }
}

bool ScrollbarThemeWKC::hasButtons(ScrollbarThemeClient* scrollbar)
{
    unsigned int dummy, ss;
    int ds;

    //if (!scrollbar->enabled()) return false;

    if (scrollbar->orientation() == HorizontalScrollbar) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &ss, &dummy);
        ds = scrollbar->width();
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &dummy, &ss);
        ds = scrollbar->height();
    }
    return ds >= 2*ss;
}

bool ScrollbarThemeWKC::hasThumb(ScrollbarThemeClient* scrollbar)
{
    unsigned int dummy, ss;
    unsigned int ts;
    int ds;

    if (!scrollbar->enabled()) return false;

    if (scrollbar->orientation() == HorizontalScrollbar) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &ss, &dummy);
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_THUMB, &ts, &dummy);
        ds = scrollbar->width();
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &dummy, &ss);
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_THUMB, &dummy, &ts);
        ds = scrollbar->height();
    }
    if (hasButtons(scrollbar)) {
        return ds >= 2*ss + ts;
    } else {
        return ds >= ts;
    }
}

IntRect ScrollbarThemeWKC::backButtonRect(ScrollbarThemeClient* scrollbar, ScrollbarPart part, bool)
{
    unsigned int w,h;
    if (part == BackButtonEndPart)
        return IntRect(0,0,0,0);
    
    if (scrollbar->orientation() == HorizontalScrollbar) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w, &h);
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &w, &h);
    }
    return IntRect(scrollbar->x(), scrollbar->y(), w, h);
}

IntRect ScrollbarThemeWKC::forwardButtonRect(ScrollbarThemeClient* scrollbar, ScrollbarPart part, bool)
{
    unsigned int w,h;
    if (part == ForwardButtonStartPart)
        return IntRect(0,0,0,0);

    if (scrollbar->orientation() == HorizontalScrollbar) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_RIGHT, &w, &h);
        return IntRect(scrollbar->x()+scrollbar->width() - w, scrollbar->y(), w, h);
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_DOWN, &w, &h);
        return IntRect(scrollbar->x(), scrollbar->y()+scrollbar->height() - h, w, h);
    }
}

IntRect ScrollbarThemeWKC::trackRect(ScrollbarThemeClient* scrollbar, bool)
{
    unsigned int w0,h0;
    unsigned int w1,h1;

    if (!hasButtons(scrollbar))
        return scrollbar->frameRect();

    if (scrollbar->orientation() == HorizontalScrollbar) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w0, &h0);
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_RIGHT, &w1, &h1);

        return IntRect(scrollbar->x() + w0, scrollbar->y(), 
                       scrollbar->width() - w0 - w1, h0);
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &w0, &h0);
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_DOWN, &w1, &h1);

        return IntRect(scrollbar->x(), scrollbar->y() + h0,
                       w0, scrollbar->height() - h0 - h1);
    }
}

int ScrollbarThemeWKC::minimumThumbLength(ScrollbarThemeClient* scrollbar)
{
    unsigned int ss, dummy;

    if (scrollbar->orientation() == HorizontalScrollbar) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_THUMB, &ss, &dummy);
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_THUMB, &dummy, &ss);
    }
    return ss;
}

void ScrollbarThemeWKC::invalidatePart(ScrollbarThemeClient* scrollbar, ScrollbarPart part)
{
    ScrollbarThemeComposite::invalidatePart(scrollbar, part);
}

static void
_bitblt(void* ctx, int type, void* bitmap, int rowbytes, void* mask, int maskrowbytes, const WKCFloatRect* srcrect, const WKCFloatRect* destrect, int op)
{
    WKCPeerImage img = {0};

    img.fType = type;
    img.fBitmap = bitmap;
    img.fRowBytes = rowbytes;
    img.fMask = mask;
    img.fMaskRowBytes = maskrowbytes;
    WKCFloatRect_SetRect(&img.fSrcRect, srcrect);
    WKCFloatSize_Set(&img.fScale, 1, 1);
    WKCFloatSize_Set(&img.fiScale, 1, 1);
    WKCFloatPoint_Set(&img.fPhase, 0, 0);
    WKCFloatSize_Set(&img.fiTransform, 1, 1);

    wkcDrawContextBitBltPeer(ctx, &img, destrect, op);
}

static void drawScalingBitmapPeer(void* in_context, void* in_bitmap, int rowbytes, WKCSize *in_size, const WKCPoint *in_points, const WKCRect *in_destrect, int op)
{
    WKCFloatRect src, dest;

      // upper
    src.fX = in_points[0].fX; src.fY = 0; 
    src.fWidth = in_points[1].fX - in_points[0].fX;
    src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX + in_points[0].fX;
    dest.fY = in_destrect->fY;
    dest.fWidth = in_destrect->fWidth - in_points[0].fX - (in_size->fWidth - in_points[1].fX);
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // lower
    src.fX = in_points[2].fX; src.fY = in_points[2].fY;
    src.fWidth = in_points[3].fX - in_points[2].fX;
    src.fHeight = in_size->fHeight - in_points[2].fY;
    dest.fX = in_destrect->fX + in_points[2].fX;
    dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = in_destrect->fWidth - in_points[2].fX - (in_size->fWidth - in_points[3].fX);
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // left
    src.fX = 0; src.fY = in_points[0].fY;
    src.fWidth = in_points[0].fX;
    src.fHeight = in_points[2].fY - in_points[0].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY + in_points[0].fY;
    dest.fWidth = src.fWidth;
    dest.fHeight = in_destrect->fHeight - in_points[0].fY - (in_size->fHeight - in_points[2].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    //right
    src.fX = in_points[1].fX; src.fY = in_points[1].fY;
    src.fWidth = in_size->fWidth - in_points[1].fX;
    src.fHeight = in_points[3].fY - in_points[1].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth;
    dest.fY = in_destrect->fY + in_points[1].fY;
    dest.fWidth = src.fWidth;
    dest.fHeight = in_destrect->fHeight - in_points[1].fY - (in_size->fHeight - in_points[3].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // center
    src.fX = in_points[0].fX; src.fY = in_points[0].fY;
    src.fWidth = in_points[3].fX - in_points[0].fX;
    src.fHeight = in_points[3].fY - in_points[0].fY;
    dest.fX = in_destrect->fX + in_points[0].fX;
    dest.fY = in_destrect->fY + in_points[0].fY;
    dest.fWidth = in_destrect->fWidth - in_points[0].fX - (in_size->fWidth - in_points[3].fX);
    dest.fHeight = in_destrect->fHeight - in_points[0].fY - (in_size->fHeight - in_points[3].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // top left corner
    src.fX = 0; src.fY = 0; src.fWidth = in_points[0].fX; src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY; dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // top right
    src.fX = in_points[1].fX; src.fY = 0; src.fWidth = in_size->fWidth - in_points[1].fX; src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth; dest.fY = in_destrect->fY;
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // bottom left
    src.fX = 0; src.fY = in_points[2].fY; src.fWidth = in_points[2].fX; src.fHeight = in_size->fHeight - in_points[2].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // bottom right corner
    src.fX = in_points[3].fX; src.fY = in_points[3].fY;
    src.fWidth = in_size->fWidth - in_points[3].fX; src.fHeight = in_size->fHeight - in_points[3].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth;
    dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = src.fWidth;
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);
}

void ScrollbarThemeWKC::paintTrackBackground(GraphicsContext* context, ScrollbarThemeClient* scrollbar, const IntRect& r)
{
      void *drawContext;
    WKCSize img_size;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;

    drawContext = context->platformContext();
    if (!drawContext)
          return;

    if (scrollbar->enabled())
        index = scrollbar->orientation() == HorizontalScrollbar ?
                  WKC_IMAGE_H_SCROLLBAR_BACKGROUND : WKC_IMAGE_V_SCROLLBAR_BACKGROUND;
    else
        index = scrollbar->orientation() == HorizontalScrollbar ?
                  WKC_IMAGE_H_SCROLLBAR_BACKGROUND_DISABLED : WKC_IMAGE_V_SCROLLBAR_BACKGROUND_DISABLED;
          
    image_buf = wkcStockImageGetBitmapPeer(index);
    if (!image_buf)
          return;
    wkcStockImageGetSizePeer(index, &width, &height);
    if (width == 0 || height == 0)
          return;
    points = wkcStockImageGetLayoutPointsPeer(index);
    if (!points)
          return;

    img_size.fWidth = (int)width;
    img_size.fHeight = (int)height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintButton(GraphicsContext* context, ScrollbarThemeClient* scrollbar, const IntRect& r, ScrollbarPart part)
{
    void *drawContext;
//    WKCSize img_size;
    int index = 0;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCFloatRect src, dest;

    drawContext = context->platformContext();
    if (!drawContext)
          return;

    if (!hasButtons(scrollbar))
          return;

    if (scrollbar->enabled()) {
          if (part == scrollbar->pressedPart() || part == scrollbar->hoveredPart()) {
              if (part == BackButtonStartPart)
                index = scrollbar->orientation() == HorizontalScrollbar ?
                          WKC_IMAGE_H_SCROLLBAR_LEFT_HOVERED : WKC_IMAGE_V_SCROLLBAR_UP_HOVERED;
            else if (part == ForwardButtonEndPart)
                index = scrollbar->orientation() == HorizontalScrollbar ?
                          WKC_IMAGE_H_SCROLLBAR_RIGHT_HOVERED : WKC_IMAGE_V_SCROLLBAR_DOWN_HOVERED;
        }
        else {
            if (part == BackButtonStartPart)
                index = scrollbar->orientation() == HorizontalScrollbar ?
                          WKC_IMAGE_H_SCROLLBAR_LEFT : WKC_IMAGE_V_SCROLLBAR_UP;
            else if (part == ForwardButtonEndPart)
                index = scrollbar->orientation() == HorizontalScrollbar ?
                          WKC_IMAGE_H_SCROLLBAR_RIGHT : WKC_IMAGE_V_SCROLLBAR_DOWN;
        }
    }
    else {
        if (part == BackButtonStartPart)
            index = scrollbar->orientation() == HorizontalScrollbar ?
                      WKC_IMAGE_H_SCROLLBAR_LEFT_DISABLED : WKC_IMAGE_V_SCROLLBAR_UP_DISABLED;
        else if (part == ForwardButtonEndPart)
            index = scrollbar->orientation() == HorizontalScrollbar ?
                      WKC_IMAGE_H_SCROLLBAR_RIGHT_DISABLED : WKC_IMAGE_V_SCROLLBAR_DOWN_DISABLED;
    }

    image_buf = wkcStockImageGetBitmapPeer(index);
    if (!image_buf)
          return;
    wkcStockImageGetSizePeer(index, &width, &height);
    if (width == 0 || height == 0)
          return;
    points = wkcStockImageGetLayoutPointsPeer(index);
    if (!points)
          return;

//    img_size.fWidth = (int)width;
//    img_size.fHeight = (int)height;
    rowbytes = width * 4;

    src.fX = 0; src.fY = 0; src.fWidth = (int)width; src.fHeight = (int)height;
    dest.fX = r.x(); dest.fY = r.y(); dest.fWidth = r.width(); dest.fHeight = r.height();

    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintThumb(GraphicsContext* context, ScrollbarThemeClient* scrollbar, const IntRect& r)
{
    void *drawContext;
    WKCSize img_size;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;

    drawContext = context->platformContext();
    if (!drawContext)
          return;

    if (!scrollbar->enabled() || !hasThumb(scrollbar))
          return; // do not draw the thumb when disabled or when ther is no thumb

    if (ThumbPart == scrollbar->pressedPart() || ThumbPart == scrollbar->hoveredPart())
        index = scrollbar->orientation() == HorizontalScrollbar ?
                  WKC_IMAGE_H_SCROLLBAR_THUMB_HOVERED : WKC_IMAGE_V_SCROLLBAR_THUMB_HOVERED;
    else
        index = scrollbar->orientation() == HorizontalScrollbar ?
                  WKC_IMAGE_H_SCROLLBAR_THUMB : WKC_IMAGE_V_SCROLLBAR_THUMB;
          

    image_buf = wkcStockImageGetBitmapPeer(index);
    if (!image_buf)
          return;
    wkcStockImageGetSizePeer(index, &width, &height);
    if (width == 0 || height == 0)
          return;
    points = wkcStockImageGetLayoutPointsPeer(index);
    if (!points)
          return;

    img_size.fWidth = (int)width;
    img_size.fHeight = (int)height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

bool ScrollbarThemeWKC::paint(ScrollbarThemeClient* scrollbar, GraphicsContext* graphicsContext, const IntRect& damageRect)
{
    return ScrollbarThemeComposite::paint(scrollbar, graphicsContext, damageRect);
}

IntRect scrollbarTrackRect(Scrollbar* scrollbar)
{
    return ((WebCore::ScrollbarThemeWKC*)scrollbar->theme())->trackRect(scrollbar);
}

void scrollbarSplitTrack(Scrollbar* scrollbar, const IntRect& track, IntRect& startTrack, IntRect& thumb, IntRect& endTrack)
{
    return ((WebCore::ScrollbarThemeWKC*)scrollbar->theme())->splitTrack(scrollbar, track, startTrack, thumb, endTrack);
}

IntRect scrollbarBackButtonRect(Scrollbar* scrollbar, ScrollbarPart part)
{
    return ((WebCore::ScrollbarThemeWKC*)scrollbar->theme())->backButtonRect(scrollbar, part);
}

IntRect scrollbarForwardButtonRect(Scrollbar* scrollbar, ScrollbarPart part)
{
    return ((WebCore::ScrollbarThemeWKC*)scrollbar->theme())->forwardButtonRect(scrollbar, part);
}

void ScrollbarThemeWKC::paintScrollCorner(ScrollView* view, GraphicsContext* context, const IntRect& r)
{
    void *drawContext;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCFloatRect src, dest;

    drawContext = context->platformContext();
    if (!drawContext)
          return;

    // no way to obtain scroll-bar state...
    index = WKC_IMAGE_SCROLLBAR_CROSS_CORNER;

    image_buf = wkcStockImageGetBitmapPeer(index);
    if (!image_buf)
          return;
    wkcStockImageGetSizePeer(index, &width, &height);
    if (width == 0 || height == 0)
          return;
    points = wkcStockImageGetLayoutPointsPeer(index);
    if (!points)
          return;

    rowbytes = width * 4;

    src.fX = 0; src.fY = 0; src.fWidth = (int)width; src.fHeight = (int)height;
    dest.fX = r.x(); dest.fY = r.y(); dest.fWidth = r.width(); dest.fHeight = r.height();

    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

}

