/*
 *  Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "ImageDecoder.h"
#include "FastMalloc.h"

#include "ImageWKC.h"

#if PLATFORM(WKC)
#include <wkc/wkcgpeer.h>
#include <wkc/wkcmpeer.h>
#endif

namespace WebCore {

ImageFrame::ImageFrame()
    : m_originalFrameRect(IntRect())
    , m_status(FrameEmpty)
    , m_duration(0)
    , m_disposalMethod(DisposeNotSpecified)
    , m_premultiplyAlpha(true)
{
    m_image = ImageWKC::create();
}

ImageFrame::~ImageFrame()
{
    if (m_image) {
        m_image->unref();
    }
}

ImageFrame&
ImageFrame::operator=(const ImageFrame& other)
{
    if (this == &other)
        return *this;

    copyReferenceToBitmapData(other);
    setOriginalFrameRect(other.originalFrameRect());
    setStatus(other.status());
    setDuration(other.duration());
    setDisposalMethod(other.disposalMethod());
    setPremultiplyAlpha(other.premultiplyAlpha());
    return *this;
}

void
ImageFrame::clearPixelData()
{
    m_image->clear();
    m_status = FrameEmpty;
}

void
ImageFrame::zeroFillPixelData()
{
    m_image->zeroFill();
    m_image->setHasAlpha(true);
}

bool
ImageFrame::copyBitmapData(const ImageFrame& other)
{
    if (!other.width() || !other.height()) {
        return false;
    }

    ImageWKC* img = ImageWKC::create();
    if (!img)
        return false;

    if (!img->copyImage(other.m_image)) {
        delete img;
        return false;
    }

    m_image->unref();
    m_image = img;

    return true;
}

void
ImageFrame::copyReferenceToBitmapData(const ImageFrame& other)
{
    ASSERT(this != &other);
    m_image = other.m_image;
    m_image->ref();
}

bool
ImageFrame::setSize(int newWidth, int newHeight)
{
    bool ret = m_image->resize(IntSize(newWidth, newHeight));
    if (!ret)
        return false;

    m_image->setHasAlpha(true);
    return true;
}

bool
ImageFrame::hasAlpha() const
{
    return m_image->hasAlpha();
}

void
ImageFrame::setHasAlpha(bool alpha)
{
    m_image->setHasAlpha(alpha);
}

void
ImageFrame::setColorProfile(const ColorProfile&)
{
    // color-correction is not supported.
}

void
ImageFrame::setStatus(FrameStatus status)
{
    m_status = status;
    m_image->notifyStatus(status);
}

int
ImageFrame::width() const
{
    return m_image->size().width();
}

int
ImageFrame::height() const
{
    return m_image->size().height();
}

void
ImageFrame::setAllowReduceColor(bool flag)
{
    if (m_status != FrameEmpty)
        return;

    m_image->setAllowReduceColor(flag);

    if (m_image->allowReduceColor()) {
        m_image->unref();
        m_image = ImageWKC::create(ImageWKC::EColorRGB565);
    }
}

} // namespace

