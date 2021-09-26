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

#ifndef _WKC_HELPERS_WKC_GRAPHICSLAYER_H_
#define _WKC_HELPERS_WKC_GRAPHICSLAYER_H_

#include <wkc/wkcbase.h>

namespace WKC {

class GraphicsLayerPrivate;

class WKC_API GraphicsLayer {
public:
    GraphicsLayer(GraphicsLayerPrivate&);
    ~GraphicsLayer();

    bool preserves3D() const;
    bool masksToBounds() const;
    bool drawsContent() const;
    bool isRootClippingLayer() const;
    unsigned int backgroundColor() const;
    bool backgroundColorSet() const;
    bool contentsOpaque() const;
    bool backfaceVisibility() const;
    float opacity() const;
    bool isThreeDImageLayer() const;
    int side() const;

    size_t dirtyRectsSize() const;  // reset by paint()
    void dirtyRects(int, WKCFloatRect&) const;
    void resetDirtyRects();

    void clear(const WKCRect&, void*);
    void paint(const WKCRect&);
    void paint(const WKCRect&, const WKCPoint&, float, GraphicsLayer*, void*);
    WKCRect lastPaintedRect() const;

    void layerDidDisplay(void*);
    enum {
        EBothSide=0,
        ELeftSide,
        ERightSide,
        ESides
    };
    void* platformLayer(int side=EBothSide) const;
    bool ensureOffscreen(int, int);
    void disposeOffscreen();

    GraphicsLayer* parent() const;
    size_t childrenSize() const;
    GraphicsLayer* children(int);

    const WKCFloatSize size() const;
    const WKCFloatPoint position() const;
    const WKCFloatPoint anchorPoint(float&) const;
    void transform(double*) const;
    void childrenTransform(double*) const;
    void setNeedsDisplay() const;
    GraphicsLayer* maskLayer();

    void setOpticalZoom(float zoom);

    bool isFixedPosition() const;

    bool isLeftDefined() const;
    bool isRightDefined() const;
    bool isTopDefined() const;
    bool isBottomDefined() const;

    static void disposeAllButDescendantsOf(GraphicsLayer*);

#ifdef WKC_CUSTOMER_PATCH_0304674
    void setOffscreenBitmap( void* bitmap );
#endif

private:
    GraphicsLayerPrivate& m_private;
};
} // namespace

#endif // _WKC_HELPERS_WKC_GRAPHICSLAYER_H_
