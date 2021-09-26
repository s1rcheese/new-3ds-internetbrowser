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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#if USE(ACCELERATED_COMPOSITING)

#include "GraphicsLayer.h"

#include "GraphicsContext.h"
#include "FloatRect.h"
#include "FloatSize.h"

#include "helpers/privates/WKCGraphicsLayerPrivate.h"

#include "NotImplemented.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

WKC::GraphicsLayerPrivate* GraphicsLayerWKC_wkc(const WebCore::GraphicsLayer* layer);

class GraphicsLayerWKC : public GraphicsLayer
{
public:
    GraphicsLayerWKC(GraphicsLayerClient* client);

    virtual ~GraphicsLayerWKC();

    virtual void addChild(GraphicsLayer*);
    virtual void addChildAtIndex(GraphicsLayer*, int index);
    virtual void addChildAbove(GraphicsLayer* layer, GraphicsLayer* sibling);
    virtual void addChildBelow(GraphicsLayer* layer, GraphicsLayer* sibling);
    virtual bool replaceChild(GraphicsLayer* oldChild, GraphicsLayer* newChild);
    virtual void removeFromParent();

    virtual void setNeedsDisplay();
    virtual void setNeedsDisplayInRect(const FloatRect&);
    virtual void setContentsNeedsDisplay();

    virtual void setMaskLayer(GraphicsLayer* layer);
    virtual void setReplicatedByLayer(GraphicsLayer*);
    virtual void setPosition(const FloatPoint& p);
    virtual void setBoundsOrigin(const FloatPoint& origin);
    virtual void setMasksToBounds(bool b);
    virtual void setPreserves3D(bool b);

    virtual void setContentsOpaque(bool b);
    virtual void setBackfaceVisibility(bool b);
    virtual void setOpacity(float opacity);
    virtual void setDrawsContent(bool drawsContent);

    virtual void setSize(const FloatSize& size);
    virtual void setTransform(const TransformationMatrix& t);
    virtual void setChildrenTransform(const TransformationMatrix& t);
    virtual void setAnchorPoint(const FloatPoint3D& p);

    virtual void setContentsVisible(bool b);
    virtual void setAcceleratesDrawing(bool b);
    virtual void setBackgroundColor(const Color&);
    virtual void setContentsRect(const IntRect& r);
    virtual void setContentsOrientation(CompositingCoordinatesOrientation orientation);
    virtual void setZPosition(float);
    virtual void setMaintainsPixelAlignment(bool maintainsAlignment);

    virtual void setContentsToImage(Image*);
    virtual void setContentsToMedia(PlatformLayer*);
    virtual void setContentsToBackgroundColor(const Color&);
    virtual void setContentsToCanvas(PlatformLayer*);

#if ENABLE(CSS_FILTERS)
    virtual bool setFilters(const FilterOperations&) { return false; }
#endif

    virtual PlatformLayer* platformLayer() const;

    virtual void syncCompositingState(const FloatRect& clipRect);

    inline WKC::GraphicsLayerPrivate* wkc() const { return m_wkc; }

private:
    WKC::GraphicsLayerPrivate* m_wkc;
    PlatformLayer* m_activeLayer;
};

GraphicsLayerWKC::GraphicsLayerWKC(GraphicsLayerClient* client)
    : GraphicsLayer(client)
    , m_wkc(new WKC::GraphicsLayerPrivate(this))
{
    m_activeLayer = 0;
}

GraphicsLayerWKC::~GraphicsLayerWKC()
{
    GraphicsLayer::willBeDestroyed();
    delete m_wkc;
}

void
GraphicsLayerWKC::addChild(GraphicsLayer* childlayer)
{
    bool changed = (childlayer->parent() != this);
    GraphicsLayer::addChild(childlayer);
    if (changed)
        GraphicsLayerWKC_wkc(childlayer)->layerDidAttachToTree(m_wkc);
}

void 
GraphicsLayerWKC::addChildAtIndex(GraphicsLayer* childlayer, int index)
{
    bool changed = (childlayer->parent() != this);
    GraphicsLayer::addChildAtIndex(childlayer, index);
    if (changed)
        GraphicsLayerWKC_wkc(childlayer)->layerDidAttachToTree(m_wkc);
}

void 
GraphicsLayerWKC::addChildAbove(GraphicsLayer* childlayer, GraphicsLayer* sibling)
{
    bool changed = (childlayer->parent() != this);
    GraphicsLayer::addChildAbove(childlayer, sibling);
    if (changed)
        GraphicsLayerWKC_wkc(childlayer)->layerDidAttachToTree(m_wkc);
}

void 
GraphicsLayerWKC::addChildBelow(GraphicsLayer* childlayer, GraphicsLayer* sibling)
{
    bool changed = (childlayer->parent() != this);
    GraphicsLayer::addChildBelow(childlayer, sibling);
    if (changed)
        GraphicsLayerWKC_wkc(childlayer)->layerDidAttachToTree(m_wkc);
}

bool 
GraphicsLayerWKC::replaceChild(GraphicsLayer* oldChild, GraphicsLayer* newChild)
{
    bool changed = (newChild->parent() != this);
    bool replaced = GraphicsLayer::replaceChild(oldChild, newChild);
    if (replaced) {
        GraphicsLayerWKC_wkc(oldChild)->layerDidDetachFromTree();
        if (changed)
            GraphicsLayerWKC_wkc(newChild)->layerDidAttachToTree(m_wkc);
    }
    return replaced;
}

void 
GraphicsLayerWKC::removeFromParent()
{
    bool changed = (parent() != 0);
    GraphicsLayer::removeFromParent();
    if (changed)
        m_wkc->layerDidDetachFromTree();
}

void
GraphicsLayerWKC::setNeedsDisplay()
{
    m_wkc->setNeedsDisplay();
    bool added = m_wkc->addDirtyRect();
    if (added && client()) {
        client()->notifySyncRequired(this);
    }
}

void
GraphicsLayerWKC::setNeedsDisplayInRect(const FloatRect& fr)
{
    m_wkc->setNeedsDisplay();
    WKCFloatRect r = { fr.x(), fr.y(), fr.width(), fr.height() };
    bool added = m_wkc->addDirtyRect(r);
    if (added && client()) {
        client()->notifySyncRequired(this);
    }
}

void
GraphicsLayerWKC::setContentsNeedsDisplay()
{
    bool added = m_wkc->addDirtyRect();
    if (added && client()) {
        client()->notifySyncRequired(this);
    }
}

#define DEFINE_SYNCING_SETTER(setter, getter, arg_type) \
  void GraphicsLayerWKC::setter(arg_type arg) \
  { \
      if (getter() == arg) \
          return; \
      GraphicsLayer::setter(arg); \
      if (client()) \
          client()->notifySyncRequired(this); \
  }

DEFINE_SYNCING_SETTER(setMaskLayer, maskLayer, GraphicsLayer*)
DEFINE_SYNCING_SETTER(setReplicatedByLayer, replicaLayer, GraphicsLayer*)
DEFINE_SYNCING_SETTER(setPosition, position, const FloatPoint&)
DEFINE_SYNCING_SETTER(setBoundsOrigin, boundsOrigin, const FloatPoint&)
DEFINE_SYNCING_SETTER(setMasksToBounds, masksToBounds, bool)
DEFINE_SYNCING_SETTER(setPreserves3D, preserves3D, bool)
DEFINE_SYNCING_SETTER(setContentsOpaque, contentsOpaque, bool)
DEFINE_SYNCING_SETTER(setBackfaceVisibility, backfaceVisibility, bool)
DEFINE_SYNCING_SETTER(setOpacity, opacity, float)
DEFINE_SYNCING_SETTER(setDrawsContent, drawsContent, bool)
DEFINE_SYNCING_SETTER(setSize, size, const FloatSize&)
DEFINE_SYNCING_SETTER(setTransform, transform, const TransformationMatrix&)
DEFINE_SYNCING_SETTER(setChildrenTransform, childrenTransform, const TransformationMatrix&)
DEFINE_SYNCING_SETTER(setAnchorPoint, anchorPoint, const FloatPoint3D&)
DEFINE_SYNCING_SETTER(setContentsVisible, contentsAreVisible, bool)
DEFINE_SYNCING_SETTER(setAcceleratesDrawing, acceleratesDrawing, bool)
DEFINE_SYNCING_SETTER(setBackgroundColor, backgroundColor, const Color&)
DEFINE_SYNCING_SETTER(setContentsRect, contentsRect, const IntRect&)
DEFINE_SYNCING_SETTER(setContentsOrientation, contentsOrientation, CompositingCoordinatesOrientation)
DEFINE_SYNCING_SETTER(setZPosition, zPosition, float)
DEFINE_SYNCING_SETTER(setMaintainsPixelAlignment, maintainsPixelAlignment, bool)

void
GraphicsLayerWKC::setContentsToImage(Image* img)
{
    m_wkc->setContentsToImage(img);
}

void
GraphicsLayerWKC::setContentsToMedia(PlatformLayer* layer)
{
    m_activeLayer = layer;
}

void
GraphicsLayerWKC::setContentsToBackgroundColor(const Color&)
{
    notImplemented();
}

void
GraphicsLayerWKC::setContentsToCanvas(PlatformLayer* layer)
{
    m_activeLayer = layer;
}

PlatformLayer*
GraphicsLayerWKC::platformLayer() const
{
    if (m_activeLayer)
        return m_activeLayer;
    else
        return reinterpret_cast<PlatformLayer*>(m_wkc->offscreenLayer(0));
}

void
GraphicsLayerWKC::syncCompositingState(const FloatRect& clipRect)
{
    m_wkc->syncCompositingState(clipRect);
}

PassOwnPtr<GraphicsLayer>
GraphicsLayer::create(GraphicsLayerClient* client)
{
    return adoptPtr(new GraphicsLayerWKC(client));
}

WKC::GraphicsLayerPrivate*
GraphicsLayerWKC_wkc(const GraphicsLayer* layer)
{
    const GraphicsLayerWKC* wl = static_cast<const GraphicsLayerWKC*>(layer);
    return wl->wkc();
}

} // namespace

#endif // USE(ACCELERATED_COMPOSITING)
