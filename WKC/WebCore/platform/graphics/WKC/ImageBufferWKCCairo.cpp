/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Holger Hans Peter Freyther <zecke@selfish.org>
 * Copyright (C) 2008, 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010 Torch Mobile (Beijing) Co. Ltd. All rights reserved.
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

#if USE(WKC_CAIRO)

#include "ImageBuffer.h"

#include "Base64.h"
#include "BitmapImage.h"
#include "CairoUtilities.h"
#include "Color.h"
#include "GraphicsContext.h"
#include "ImageData.h"
#include "ImageWKC.h"
#include "image-encoders/JPEGImageEncoder.h"
#include "MIMETypeRegistry.h"
#include "NotImplemented.h"
#include "Pattern.h"
#include "PlatformContextCairo.h"
#include "PlatformString.h"
#include "RefPtrCairo.h"
#include <cairo.h>
#include <wtf/Vector.h>

#include <wkc/wkcgpeer.h>

// followings are originally from WebCore/platform/graphics/cairo/* .

using namespace std;

namespace WebCore {

ImageBufferData::ImageBufferData(const IntSize& size)
    : m_surface(0)
    , m_platformContext(0)
{
}

ImageBuffer::ImageBuffer(const IntSize& size, float resolutionScale, ColorSpace colorSpace, RenderingMode renderingMode, DeferralMode deferralMode, bool& success)
    : m_data(size)
    , m_size(size)
    , m_logicalSize(size)
    , m_resolutionScale(1.f)
{
    success = false;  // Make early return mean error.

    m_data.m_surface = (cairo_surface_t *)wkcDrawContextCairoSurfaceNewPeer(size.width(), size.height());
    if (!m_data.m_surface || (cairo_surface_status(m_data.m_surface) != CAIRO_STATUS_SUCCESS)) {
        // Commented out the call to wkcMemoryNotifyNoMemoryPeer because we want to continue (skipping painting the error surface) without restart.
        // wkcMemoryNotifyNoMemoryPeer(size.width() * size.height() * 4);
        return ;  // create will notice we didn't set m_initialized and fail.
    }
    WKC_CAIRO_ADD_OBJECT(m_data.m_surface, cairo_surface);

    RefPtr<cairo_t> cr = adoptRef(cairo_create(m_data.m_surface));
    WKC_CAIRO_ADD_OBJECT(cr.get(), cairo);
    m_data.m_platformContext.setCr(cr.get());
    m_context = adoptPtr(new GraphicsContext(&m_data.m_platformContext));
    success = true;
}

ImageBuffer::~ImageBuffer()
{
    wkcDrawContextCairoSurfaceDeletePeer(m_data.m_surface);
}

GraphicsContext* ImageBuffer::context() const
{
    return m_context.get();
}

PassRefPtr<Image> ImageBuffer::copyImage(BackingStoreCopy, ScaleBehavior) const
{
    cairo_surface_t* surface = m_data.m_surface;
    cairo_surface_type_t type = cairo_surface_get_type(surface);
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
        surface = cairo_surface_map_to_image(surface, NULL);
#else
        return Image::nullImage();
#endif
    }

    ImageWKC* img = 0;
    void* newsurface = 0;
    cairo_surface_t* src = copyCairoImageSurface(surface).leakRef();
    if (!src)
        return Image::nullImage();
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
        cairo_surface_unmap_image(m_data.m_surface, surface);
    }
#endif
    int format = cairo_image_surface_get_format(src)==CAIRO_FORMAT_RGB16_565 ? ImageWKC::EColorRGB565 : ImageWKC::EColorARGB8888;
    int src_rowbytes = cairo_image_surface_get_stride(src);
    int src_width = cairo_image_surface_get_width(src);
    int src_height = cairo_image_surface_get_height(src);


    bool allocSucceeded = false;
    if (wkcMemoryCheckMemoryAllocatablePeer(src_rowbytes * src_height, WKC_MEMORYALLOC_TYPE_IMAGE_UNSPECIFIED)) {
        wkcMemorySetAllocatingForImagesPeer(true);
        WTF::TryMallocReturnValue rv = tryFastMalloc(src_rowbytes * src_height);
        wkcMemorySetAllocatingForImagesPeer(false);
        allocSucceeded = rv.getValue(newsurface);
    }
    if (!allocSucceeded) {
        wkcMemoryNotifyMemoryAllocationErrorPeer(src_rowbytes * src_height, WKC_MEMORYALLOC_TYPE_IMAGE_UNSPECIFIED);
        newsurface = 0;
    } else {
        memcpy(newsurface, cairo_image_surface_get_data(src), src_rowbytes * src_height);
    }

    WKC_CAIRO_REMOVE_OBJECT(src);
    cairo_surface_destroy(src);

    const IntSize size(src_width, src_height);
    img = ImageWKC::create(format, newsurface, src_rowbytes, size, true);
    if (!img) {
        if (newsurface) {
            fastFree(newsurface);
        }
        return Image::nullImage();
    }
    // BitmapImage will release the passed in surface on destruction
    return BitmapImage::create(img);
}

void ImageBuffer::clip(GraphicsContext* context, const FloatRect& maskRect) const
{
    context->platformContext()->pushImageMask(m_data.m_surface, maskRect);
}

static ImageWKC*
createImageForSurface(cairo_surface_t* surface)
{
    ImageWKC* img = 0;
    int width = 0, height = 0;

    wkcDrawContextCairoSurfaceGetSizePeer((void *)surface, &width, &height);
    if (!width || !height)
        return 0;
    const IntSize size(width, height);

    bool isimage = wkcDrawContextCairoSurfaceIsImagePeer((void *)surface);

    if (!isimage) {
        return 0;
    } else {
        int rowbytes = cairo_image_surface_get_stride(surface);
        void *data = cairo_image_surface_get_data(surface);

        img = ImageWKC::create(ImageWKC::EColorARGB8888, data, rowbytes, size, false);
        if (!img) {
            return 0;
        }
    }

    return img;
}

void ImageBuffer::draw(GraphicsContext* context, ColorSpace styleColorSpace, const FloatRect& destRect, const FloatRect& srcRect,
                       CompositeOperator op , bool useLowQualityScale)
{
    ImageWKC* img = createImageForSurface(m_data.m_surface);
    if (!img)
        return;

    // BitmapImage will release the passed in surface on destruction
    RefPtr<Image> image;
    if (m_context != context) {
        image = BitmapImage::create(img);
    } else {
        image = copyImage();
    }
    context->drawImage(image.get(), styleColorSpace, destRect, srcRect, op, DoNotRespectImageOrientation, useLowQualityScale);
}

void ImageBuffer::drawPattern(GraphicsContext* context, const FloatRect& srcRect, const AffineTransform& patternTransform,
                              const FloatPoint& phase, ColorSpace styleColorSpace, CompositeOperator op, const FloatRect& destRect)
{
    ImageWKC* img = createImageForSurface(m_data.m_surface);
    if (!img)
        return;

    // BitmapImage will release the passed in surface on destruction
    RefPtr<Image> image;
    if (m_context != context) {
        image = BitmapImage::create(img);
    } else {
        image = copyImage();
    }
    image->drawPattern(context, srcRect, patternTransform, phase, styleColorSpace, op, destRect);
}

void ImageBuffer::platformTransformColorSpace(const Vector<int>& lookUpTable)
{
    cairo_surface_type_t type = cairo_surface_get_type(m_data.m_surface);
    cairo_surface_t* surface = m_data.m_surface;
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
        surface = cairo_surface_map_to_image(surface, NULL);
#else
        return;
#endif
    }

    unsigned char* dataSrc = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    for (int y = 0; y < m_size.height(); ++y) {
        unsigned* row = reinterpret_cast<unsigned*>(dataSrc + stride * y);
        for (int x = 0; x < m_size.width(); x++) {
            unsigned* pixel = row + x;
            Color pixelColor = colorFromPremultipliedARGB(*pixel);
            pixelColor = Color(lookUpTable[pixelColor.red()],
                               lookUpTable[pixelColor.green()],
                               lookUpTable[pixelColor.blue()],
                               pixelColor.alpha());
            *pixel = premultipliedARGBFromColor(pixelColor);
        }
    }
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
        cairo_surface_unmap_image(m_data.m_surface, surface);
    }
#endif
    cairo_surface_mark_dirty_rectangle (m_data.m_surface, 0, 0, m_size.width(), m_size.height());
}

template <Multiply multiplied>
PassRefPtr<Uint8ClampedArray> getImageData(const IntRect& rect, const ImageBufferData& data, const IntSize& size)
{
    cairo_surface_type_t type = cairo_surface_get_type(data.m_surface);
    cairo_surface_t* surface = data.m_surface;
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
        surface = cairo_surface_map_to_image(surface, NULL);
#else
        return Image::nullImage();
#endif
    }

    RefPtr<Uint8ClampedArray> result = Uint8ClampedArray::create(rect.width() * rect.height() * 4);
    if (!result || !result->data()) {
        wkcMemoryNotifyNoMemoryPeer(rect.width() * rect.height() * 4);
    }
    unsigned char* dataSrc = cairo_image_surface_get_data(surface);
    unsigned char* dataDst = result->data();

    if (rect.x() < 0 || rect.y() < 0 || (rect.x() + rect.width()) > size.width() || (rect.y() + rect.height()) > size.height())
        memset(dataDst, 0, result->length());

    int originx = rect.x();
    int destx = 0;
    if (originx < 0) {
        destx = -originx;
        originx = 0;
    }
    int endx = rect.maxX();
    if (endx > size.width())
        endx = size.width();
    int numColumns = endx - originx;

    int originy = rect.y();
    int desty = 0;
    if (originy < 0) {
        desty = -originy;
        originy = 0;
    }
    int endy = rect.maxY();
    if (endy > size.height())
        endy = size.height();
    int numRows = endy - originy;

    int stride = cairo_image_surface_get_stride(surface);
    unsigned destBytesPerRow = 4 * rect.width();

    unsigned char* destRows = dataDst + desty * destBytesPerRow + destx * 4;
    for (int y = 0; y < numRows; ++y) {
        unsigned* row = reinterpret_cast<unsigned*>(dataSrc + stride * (y + originy));
        for (int x = 0; x < numColumns; x++) {
            int basex = x * 4;
            unsigned* pixel = row + x + originx;
            Color pixelColor;
            if (multiplied == Unmultiplied)
                pixelColor = colorFromPremultipliedARGB(*pixel);
            else
                pixelColor = Color(*pixel);
            destRows[basex]     = pixelColor.red();
            destRows[basex + 1] = pixelColor.green();
            destRows[basex + 2] = pixelColor.blue();
            destRows[basex + 3] = pixelColor.alpha();
        }
        destRows += destBytesPerRow;
    }

#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
        cairo_surface_unmap_image(data.m_surface, surface);
    }
#endif
    return result.release();
}

PassRefPtr<Uint8ClampedArray> ImageBuffer::getUnmultipliedImageData(const IntRect& rect, CoordinateSystem) const
{
    return getImageData<Unmultiplied>(rect, m_data, m_size);
}

PassRefPtr<Uint8ClampedArray> ImageBuffer::getPremultipliedImageData(const IntRect& rect, CoordinateSystem) const
{
    return getImageData<Premultiplied>(rect, m_data, m_size);
}

template <Multiply multiplied>
void putImageData(Uint8ClampedArray*& source, const IntSize& sourceSize, const IntRect& sourceRect, const IntPoint& destPoint, ImageBufferData& data, const IntSize& size)
{
    cairo_surface_t* surface = data.m_surface;
    cairo_surface_type_t type = cairo_surface_get_type(surface);
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
        surface = cairo_surface_map_to_image(surface, NULL);
#else
        return;
#endif
    }

    unsigned char* dataDst = cairo_image_surface_get_data(surface);

    ASSERT(sourceRect.width() > 0);
    ASSERT(sourceRect.height() > 0);

    int originx = sourceRect.x();
    int destx = destPoint.x() + sourceRect.x();
    ASSERT(destx >= 0);
    ASSERT(destx < size.width());
    ASSERT(originx >= 0);
    ASSERT(originx <= sourceRect.maxX());

    int endx = destPoint.x() + sourceRect.maxX();
    ASSERT(endx <= size.width());

    int numColumns = endx - destx;

    int originy = sourceRect.y();
    int desty = destPoint.y() + sourceRect.y();
    ASSERT(desty >= 0);
    ASSERT(desty < size.height());
    ASSERT(originy >= 0);
    ASSERT(originy <= sourceRect.maxY());

    int endy = destPoint.y() + sourceRect.maxY();
    ASSERT(endy <= size.height());
    int numRows = endy - desty;

    unsigned srcBytesPerRow = 4 * sourceSize.width();
    int stride = cairo_image_surface_get_stride(surface);

    unsigned char* srcRows = source->data() + originy * srcBytesPerRow + originx * 4;
    for (int y = 0; y < numRows; ++y) {
        unsigned* row = reinterpret_cast<unsigned*>(dataDst + stride * (y + desty));
        for (int x = 0; x < numColumns; x++) {
            int basex = x * 4;
            unsigned* pixel = row + x + destx;
            Color pixelColor(srcRows[basex],
                    srcRows[basex + 1],
                    srcRows[basex + 2],
                    srcRows[basex + 3]);
            if (multiplied == Unmultiplied)
                *pixel = premultipliedARGBFromColor(pixelColor);
            else
                *pixel = pixelColor.rgb();
        }
        srcRows += srcBytesPerRow;
    }
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0))
    if (type!=CAIRO_SURFACE_TYPE_IMAGE) {
        cairo_surface_unmap_image(data.m_surface, surface);
    }
#endif
    cairo_surface_mark_dirty_rectangle (data.m_surface,
                                        destx, desty,
                                        numColumns, numRows);
}

void ImageBuffer::putByteArray(Multiply mult, Uint8ClampedArray* source, const IntSize& sourceSize, const IntRect& sourceRect, const IntPoint& destPoint, CoordinateSystem coord)
{
    wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_READPIXELS);
    if (mult==Premultiplied)
        putImageData<Premultiplied>(source, sourceSize, sourceRect, destPoint, m_data, m_size);
    else
        putImageData<Unmultiplied>(source, sourceSize, sourceRect, destPoint, m_data, m_size);
}

static cairo_status_t writeFunction(void* closure, const unsigned char* data, unsigned int length)
{
    Vector<char>* in = reinterpret_cast<Vector<char>*>(closure);
    in->append(data, length);
    return CAIRO_STATUS_SUCCESS;
}

String ImageBuffer::toDataURL(const String& mimeType, const double* quality, CoordinateSystem) const
{
#if CAIRO_HAS_PNG_FUNCTIONS
    cairo_surface_t* image = cairo_get_target(context()->platformContext()->cr());
    if (!image)
        return "data:,";

    String actualMimeType("image/png");
    if (MIMETypeRegistry::isSupportedImageMIMETypeForEncoding(mimeType))
        actualMimeType = mimeType;

    Vector<char> in;
    Vector<char> out;
    if (actualMimeType=="image/png") {
        if (cairo_surface_write_to_png_stream(image, writeFunction, &in)!=CAIRO_STATUS_SUCCESS)
            goto error_end;
    } else if (actualMimeType=="image/jpeg") {
        const IntSize size(cairo_image_surface_get_width(image), cairo_image_surface_get_height(image));
        unsigned char* data = cairo_image_surface_get_data(image);
        if (!data)
            goto error_end;
        WTF::TryMallocReturnValue ret = WTF::tryFastMalloc(size.width() * size.height() * 4);
        unsigned char* buf = 0;
        if (!ret.getValue(buf))
            goto error_end;
        const unsigned char* s = data;
        unsigned char* d = buf;
        for (int i=0; i<size.width()*size.height(); i++, s+=4, d+=4) {
            d[0] = s[2];
            d[1] = s[1];
            d[2] = s[0];
            d[3] = s[3];
        }
        int q = 65;
        if (quality && *quality>=0.0 && *quality<=1.0)
            q = 100 * (*quality);
        bool result = compressRGBABigEndianToJPEG(buf, size, in, q);
        fastFree(buf);
        if (!result)
            goto error_end;
    } else {
        goto error_end;
    }

    base64Encode(in, out);
    return "data:" + actualMimeType + ";base64," + String(out.data(), out.size());

error_end:
#endif
    return "data:,";
}

} // namespace WebCore

#endif // USE(WKC_CAIRO)
