/*
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
 * Copyright (c) 2013 ACCESS CO., LTD. All rights reserved.
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
#include "SVGImageCache.h"

#if ENABLE(SVG)
#include "FrameView.h"
#include "GraphicsContext.h"
#include "ImageBuffer.h"
#include "RenderSVGRoot.h"
#include "SVGImage.h"

namespace WebCore {

SVGImageCache::SVGImageCache(SVGImage* svgImage)
    : m_svgImage(svgImage)
    , m_redrawTimer(this, &SVGImageCache::redrawTimerFired)
{
    ASSERT(m_svgImage);
}

SVGImageCache::~SVGImageCache()
{
    m_sizeAndScalesMap.clear();

    ImageDataMap::iterator end = m_imageDataMap.end();
    for (ImageDataMap::iterator it = m_imageDataMap.begin(); it != end; ++it)
        delete it->second.buffer;

    m_imageDataMap.clear();
}

void SVGImageCache::removeRendererFromCache(const RenderObject* renderer)
{
    ASSERT(renderer);
    m_sizeAndScalesMap.remove(renderer);

    ImageDataMap::iterator it = m_imageDataMap.find(renderer);
    if (it == m_imageDataMap.end())
        return;

    delete it->second.buffer;
    m_imageDataMap.remove(it);
}

void SVGImageCache::setRequestedSizeAndScales(const RenderObject* renderer, const SizeAndScales& sizeAndScales)
{
    if (!renderer)
        return;
    ASSERT(renderer);
    ASSERT(!sizeAndScales.size.isEmpty());
    m_sizeAndScalesMap.set(renderer, sizeAndScales);
}

SVGImageCache::SizeAndScales SVGImageCache::requestedSizeAndScales(const RenderObject* renderer) const
{
    if (!renderer)
        return SizeAndScales();
    SizeAndScalesMap::const_iterator it = m_sizeAndScalesMap.find(renderer);
    if (it == m_sizeAndScalesMap.end())
        return SizeAndScales();
    return it->second;
}

void SVGImageCache::imageContentChanged()
{
    ImageDataMap::iterator end = m_imageDataMap.end();
    for (ImageDataMap::iterator it = m_imageDataMap.begin(); it != end; ++it)
        it->second.imageNeedsUpdate = true;

    // Always redraw on a timer because this method may be invoked from destructors of things we are intending to draw.
    if (!m_redrawTimer.isActive())
        m_redrawTimer.startOneShot(0);
}

void SVGImageCache::redraw()
{
    ImageDataMap::iterator end = m_imageDataMap.end();
    for (ImageDataMap::iterator it = m_imageDataMap.begin(); it != end; ++it) {
        ImageData& data = it->second;
        if (!data.imageNeedsUpdate)
            continue;
        // If the content changed we redraw using our existing ImageBuffer.
        ASSERT(data.buffer);
        ASSERT(data.image);
#if PLATFORM(WKC)
        wkcMemorySetAllocatingForSVGPeer(true);
#endif
        m_svgImage->drawSVGToImageBuffer(data.buffer, data.sizeAndScales.size, data.sizeAndScales.zoom, data.sizeAndScales.scale, SVGImage::ClearImageBuffer);
        data.image = data.buffer->copyImage(CopyBackingStore);
#if PLATFORM(WKC)
        wkcMemorySetAllocatingForSVGPeer(false);
#endif
        data.imageNeedsUpdate = false;
    }
    ASSERT(m_svgImage->imageObserver());
    m_svgImage->imageObserver()->animationAdvanced(m_svgImage);
}

void SVGImageCache::redrawTimerFired(Timer<SVGImageCache>*)
{
    // We have no guarantee that the frame does not require layout when the timer fired.
    // So be sure to check again in case it is still not safe to run redraw.
    FrameView* frameView = m_svgImage->frameView();
    if (frameView && (frameView->needsLayout() || frameView->isInLayout())) {
        if (!m_redrawTimer.isActive())
            m_redrawTimer.startOneShot(0);
    } else
       redraw();
}

Image* SVGImageCache::lookupOrCreateBitmapImageForRenderer(const RenderObject* renderer)
{
    if (!renderer)
        return Image::nullImage();

    ASSERT(renderer);

    // The cache needs to know the size of the renderer before querying an image for it.
    SizeAndScalesMap::iterator sizeIt = m_sizeAndScalesMap.find(renderer);
    if (sizeIt == m_sizeAndScalesMap.end())
        return Image::nullImage();

    IntSize size = sizeIt->second.size;
    float zoom = sizeIt->second.zoom;
    float scale = sizeIt->second.scale;
    ASSERT(!size.isEmpty());

    // Lookup image for renderer in cache and eventually update it.
    ImageDataMap::iterator it = m_imageDataMap.find(renderer);
    if (it != m_imageDataMap.end()) {
        ImageData& data = it->second;

        // Common case: image size & zoom remained the same.
        if (data.sizeAndScales.size == size && data.sizeAndScales.zoom == zoom && data.sizeAndScales.scale == scale)
            return data.image.get();

        // If the image size for the renderer changed, we have to delete the buffer, remove the item from the cache and recreate it.
        delete data.buffer;
        m_imageDataMap.remove(it);
    }

    FloatSize scaledSize(size);
    scaledSize.scale(scale);

    // Create and cache new image and image buffer at requested size.
#if PLATFORM(WKC)
    wkcMemorySetAllocatingForSVGPeer(true);
#endif
    OwnPtr<ImageBuffer> newBuffer = ImageBuffer::create(expandedIntSize(scaledSize), 1);
#if PLATFORM(WKC)
    wkcMemorySetAllocatingForSVGPeer(false);
#endif
    if (!newBuffer)
        return Image::nullImage();

#if PLATFORM(WKC)
    wkcMemorySetAllocatingForSVGPeer(true);
#endif
    m_svgImage->drawSVGToImageBuffer(newBuffer.get(), size, zoom, scale, SVGImage::DontClearImageBuffer);

    RefPtr<Image> newImage = newBuffer->copyImage(CopyBackingStore);
#if PLATFORM(WKC)
    wkcMemorySetAllocatingForSVGPeer(false);
#endif
    Image* newImagePtr = newImage.get();
#if PLATFORM(WKC)
    if (!newImagePtr)
        return Image::nullImage();
#else
    ASSERT(newImagePtr);
#endif

    m_imageDataMap.add(renderer, ImageData(newBuffer.leakPtr(), newImage.release(), sizeIt->second));
    return newImagePtr;
}

} // namespace WebCore

#endif // ENABLE(SVG)
