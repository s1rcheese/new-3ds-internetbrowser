/*
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2004, 2005, 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2011-2014 ACCESS CO., LTD. All rights reserved.
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
#include "BitmapImage.h"

#include "FloatRect.h"
#include "ImageObserver.h"
#include "IntRect.h"
#include "MIMETypeRegistry.h"
#include "PlatformString.h"
#include "Timer.h"
#include <wtf/CurrentTime.h>
#include <wtf/Vector.h>

#if PLATFORM(WKC)
#include "ImageWKC.h"
#include "ImageDecoder.h"
#endif

namespace WebCore {

#if PLATFORM(WKC)
void BitmapImage::syncFrameData(size_t index)
{
    ImageWKC* iw = (ImageWKC*)m_frames[index].m_frame;

    if (!iw) {
        return;
    }
    if (iw->bitmap()) {
        return;
    }

    ImageFrame* buffer = m_source.frameAtIndex(index);
    if (buffer) {
        delete iw;
        m_frames[index].m_frame = buffer->asNewNativeImage();
    }
}

static int frameBytes(const IntSize& frameSize, NativeImagePtr img)
{
    ImageWKC* iw = (ImageWKC *)img;
    int bpp = 0;

    if (iw && iw->bitmap()) {
        bpp = iw->bpp();
    }

    return frameSize.width() * frameSize.height() * bpp;
}
#else
static int frameBytes(const IntSize& frameSize)
{
    return frameSize.width() * frameSize.height() * 4;
}
#endif

BitmapImage::BitmapImage(ImageObserver* observer)
    : Image(observer)
    , m_currentFrame(0)
    , m_frames(0)
    , m_frameTimer(0)
    , m_repetitionCount(cAnimationNone)
    , m_repetitionCountStatus(Unknown)
    , m_repetitionsComplete(0)
    , m_desiredFrameStartTime(0)
    , m_decodedSize(0)
    , m_decodedPropertiesSize(0)
    , m_frameCount(0)
    , m_isSolidColor(false)
    , m_checkedForSolidColor(false)
    , m_animationFinished(false)
    , m_allDataReceived(false)
    , m_haveSize(false)
    , m_sizeAvailable(false)
    , m_hasUniformFrameSize(true)
    , m_haveFrameCount(false)
{
    initPlatformData();
}

BitmapImage::~BitmapImage()
{
    invalidatePlatformData();
    stopAnimation();
}

bool BitmapImage::isBitmapImage() const
{
    return true;
}

bool BitmapImage::hasSingleSecurityOrigin() const
{
    return true;
}


void BitmapImage::destroyDecodedData(bool destroyAll)
{
    int framesCleared = 0;
    const size_t clearBeforeFrame = destroyAll ? m_frames.size() : m_currentFrame;
    for (size_t i = 0; i < clearBeforeFrame; ++i) {
        // The underlying frame isn't actually changing (we're just trying to
        // save the memory for the framebuffer data), so we don't need to clear
        // the metadata.
#if PLATFORM(WKC)
        int framesize = frameBytes(m_size, m_frames[i].m_frame);
        if (m_frames[i].clear(false)) {
            framesCleared += framesize;
        }
#else
        if (m_frames[i].clear(false))
          ++framesCleared;
#endif
    }

    destroyMetadataAndNotify(framesCleared);

    m_source.clear(destroyAll, clearBeforeFrame, data(), m_allDataReceived);
    return;
}

void BitmapImage::destroyDecodedDataIfNecessary(bool destroyAll)
{
    // Animated images >5MB are considered large enough that we'll only hang on
    // to one frame at a time.
#if PLATFORM(WKC)
    static const unsigned cLargeAnimationCutoff = 0;
    int total = 0;
    for (int i=0; i<m_frames.size(); i++) {
        total += frameBytes(m_size, m_frames[i].m_frame);
    }
    if (total > cLargeAnimationCutoff || (m_frames.size() > 0 && destroyAll && !m_frames[0].m_frame)) 
        destroyDecodedData(destroyAll);
#else
    static const unsigned cLargeAnimationCutoff = 5242880;
    if (m_frames.size() * frameBytes(m_size) > cLargeAnimationCutoff)
        destroyDecodedData(destroyAll);
#endif
}

void BitmapImage::destroyMetadataAndNotify(int framesCleared)
{
    m_isSolidColor = false;
    m_checkedForSolidColor = false;
    invalidatePlatformData();

#if PLATFORM(WKC)
    int deltaBytes = -framesCleared;
#else
    int deltaBytes = framesCleared * -frameBytes(m_size);
#endif
    m_decodedSize += deltaBytes;
    if (framesCleared > 0) {
        deltaBytes -= m_decodedPropertiesSize;
        m_decodedPropertiesSize = 0;
    }
    if (deltaBytes && imageObserver())
        imageObserver()->decodedSizeChanged(this, deltaBytes);
}

void BitmapImage::cacheFrame(size_t index)
{
    size_t numFrames = frameCount();
    ASSERT(m_decodedSize == 0 || numFrames > 1);

#if PLATFORM(WKC)
    if (numFrames >1 ) {
        wkcMemorySetAllocationForAnimeGifPeer(true);
    }
#endif

    if (m_frames.size() < numFrames)
        m_frames.grow(numFrames);

    m_frames[index].m_frame = m_source.createFrameAtIndex(index);
#if PLATFORM(WKC)
    if (!m_frames[index].m_frame) {
        destroyDecodedData(true);
        wkcMemorySetAllocationForAnimeGifPeer(false);
        return;
    }
#endif
    if (numFrames == 1 && m_frames[index].m_frame)
        checkForSolidColor();

    m_frames[index].m_orientation = m_source.orientationAtIndex(index);
    m_frames[index].m_haveMetadata = true;
    m_frames[index].m_isComplete = m_source.frameIsCompleteAtIndex(index);
    if (repetitionCount(false) != cAnimationNone)
        m_frames[index].m_duration = m_source.frameDurationAtIndex(index);
    m_frames[index].m_hasAlpha = m_source.frameHasAlphaAtIndex(index);

    const IntSize frameSize(index ? m_source.frameSizeAtIndex(index) : m_size);
    if (frameSize != m_size)
        m_hasUniformFrameSize = false;
    if (m_frames[index].m_frame) {
#if PLATFORM(WKC)
        int deltaBytes = frameBytes(frameSize, m_frames[index].m_frame);
#else
        int deltaBytes = frameBytes(frameSize);
#endif
        m_decodedSize += deltaBytes;
        // The fully-decoded frame will subsume the partially decoded data used
        // to determine image properties.
        deltaBytes -= m_decodedPropertiesSize;
        m_decodedPropertiesSize = 0;
        if (imageObserver())
            imageObserver()->decodedSizeChanged(this, deltaBytes);
    }

#if PLATFORM(WKC)
    wkcMemorySetAllocationForAnimeGifPeer(false);
#endif
}

void BitmapImage::didDecodeProperties() const
{
    if (m_decodedSize)
        return;
    size_t updatedSize = m_source.bytesDecodedToDetermineProperties();
    if (m_decodedPropertiesSize == updatedSize)
        return;
    int deltaBytes = updatedSize - m_decodedPropertiesSize;
#if !ASSERT_DISABLED
    bool overflow = updatedSize > m_decodedPropertiesSize && deltaBytes < 0;
    bool underflow = updatedSize < m_decodedPropertiesSize && deltaBytes > 0;
    ASSERT(!overflow && !underflow);
#endif
    m_decodedPropertiesSize = updatedSize;
    if (imageObserver())
        imageObserver()->decodedSizeChanged(this, deltaBytes);
}

IntSize BitmapImage::size() const
{
    if (m_sizeAvailable && !m_haveSize) {
        m_size = m_source.size();
        m_sizeRespectingOrientation = m_source.size(RespectImageOrientation);
        m_haveSize = true;
        didDecodeProperties();
    }
    return m_size;
}

IntSize BitmapImage::sizeRespectingOrientation() const
{
    if (m_sizeAvailable && !m_haveSize) {
        m_size = m_source.size();
        m_sizeRespectingOrientation = m_source.size(RespectImageOrientation);
        m_haveSize = true;
        didDecodeProperties();
    }
    return m_sizeRespectingOrientation;
}

IntSize BitmapImage::currentFrameSize() const
{
    if (!m_currentFrame || m_hasUniformFrameSize)
        return size();
    IntSize frameSize = m_source.frameSizeAtIndex(m_currentFrame);
    didDecodeProperties();
    return frameSize;
}

bool BitmapImage::getHotSpot(IntPoint& hotSpot) const
{
    bool result = m_source.getHotSpot(hotSpot);
    didDecodeProperties();
    return result;
}

bool BitmapImage::dataChanged(bool allDataReceived)
{
    // Clear all partially-decoded frames. For most image formats, there is only
    // one frame, but at least GIF and ICO can have more. With GIFs, the frames
    // come in order and we ask to decode them in order, waiting to request a
    // subsequent frame until the prior one is complete. Given that we clear
    // incomplete frames here, this means there is at most one incomplete frame
    // (even if we use destroyDecodedData() -- since it doesn't reset the
    // metadata), and it is after all the complete frames.
    //
    // With ICOs, on the other hand, we may ask for arbitrary frames at
    // different times (e.g. because we're displaying a higher-resolution image
    // in the content area and using a lower-resolution one for the favicon),
    // and the frames aren't even guaranteed to appear in the file in the same
    // order as in the directory, so an arbitrary number of the frames might be
    // incomplete (if we ask for frames for which we've not yet reached the
    // start of the frame data), and any or none of them might be the particular
    // frame affected by appending new data here. Thus we have to clear all the
    // incomplete frames to be safe.
    int framesCleared = 0;
    for (size_t i = 0; i < m_frames.size(); ++i) {
        // NOTE: Don't call frameIsCompleteAtIndex() here, that will try to
        // decode any uncached (i.e. never-decoded or
        // cleared-on-a-previous-pass) frames!
#if PLATFORM(WKC)
        if (m_frames[i].m_haveMetadata && !m_frames[i].m_isComplete) {
            int bytes = frameBytes(m_size, m_frames[i].m_frame);
            framesCleared += (m_frames[i].clear(true) ? bytes : 0);
        }
#else
        if (m_frames[i].m_haveMetadata && !m_frames[i].m_isComplete)
            framesCleared += (m_frames[i].clear(true) ? 1 : 0);
#endif
    }
    destroyMetadataAndNotify(framesCleared);
    
    // Feed all the data we've seen so far to the image decoder.
    m_allDataReceived = allDataReceived;
    m_source.setData(data(), allDataReceived);
    
    m_haveFrameCount = false;
    m_hasUniformFrameSize = true;
    return isSizeAvailable();
}

String BitmapImage::filenameExtension() const
{
    return m_source.filenameExtension();
}

size_t BitmapImage::frameCount()
{
    if (!m_haveFrameCount) {
        m_haveFrameCount = true;
#if PLATFORM(WKC)
        if (m_frames.size() > 1) {
            wkcMemorySetAllocationForAnimeGifPeer(true);
        }
        size_t old = m_frameCount;
        m_frameCount = m_source.frameCount();
        wkcMemorySetAllocationForAnimeGifPeer(false);

        if (old != m_frameCount) {
            for (int index = 0; index < m_frames.size(); index++) {
                syncFrameData(index);
            }
        }
#else
        m_frameCount = m_source.frameCount();
#endif
        didDecodeProperties();
    }
    return m_frameCount;
}

bool BitmapImage::isSizeAvailable()
{
    if (m_sizeAvailable)
        return true;

    m_sizeAvailable = m_source.isSizeAvailable();
    didDecodeProperties();

    return m_sizeAvailable;
}

bool BitmapImage::ensureFrameIsCached(size_t index)
{
    if (index >= frameCount())
        return false;

    if (index >= m_frames.size() || !m_frames[index].m_frame)
        cacheFrame(index);
    return true;
}

NativeImagePtr BitmapImage::frameAtIndex(size_t index)
{
    if (!ensureFrameIsCached(index))
        return 0;
    return m_frames[index].m_frame;
}

bool BitmapImage::frameIsCompleteAtIndex(size_t index)
{
    if (!ensureFrameIsCached(index))
        return true; // Why would an invalid index return true here?
    return m_frames[index].m_isComplete;
}

float BitmapImage::frameDurationAtIndex(size_t index)
{
    if (!ensureFrameIsCached(index))
        return 0;
    return m_frames[index].m_duration;
}

NativeImagePtr BitmapImage::nativeImageForCurrentFrame()
{
    return frameAtIndex(currentFrame());
}

#if PLATFORM(WKC)
bool BitmapImage::isThreeDImage() const
{
    return m_source.isThreeDImage();
}
#endif

bool BitmapImage::frameHasAlphaAtIndex(size_t index)
{
    if (!ensureFrameIsCached(index))
        return true; // Why does an invalid index mean alpha?
    return m_frames[index].m_hasAlpha;
}

bool BitmapImage::currentFrameHasAlpha()
{
    return frameHasAlphaAtIndex(currentFrame());
}

ImageOrientation BitmapImage::currentFrameOrientation()
{
    return frameOrientationAtIndex(currentFrame());
}

ImageOrientation BitmapImage::frameOrientationAtIndex(size_t index)
{
    if (!ensureFrameIsCached(index))
        return DefaultImageOrientation;
    return m_frames[index].m_orientation;
}

#if PLATFORM(WKC)
void BitmapImage::selectFrameBySize(const IntSize& size)
{
    IntSize maxSize(INT_MAX, INT_MAX);
    size_t candidateIndex = 0;

    // Search for the frame of the desired size.
    // If no frame of the size is found, use the smallest one.
    for (size_t i = 0; i < frameCount(); i++) {
        ImageWKC* image = reinterpret_cast<ImageWKC*>(frameAtIndex(i));
        if (!image) {
            continue;
        }
        if (image->size().width() == size.width() && image->size().height() == size.height()) {
            candidateIndex = i;
            break;
        }
        if (image->size().width() < maxSize.width() && image->size().height() < maxSize.height()) {
            maxSize = image->size();
            candidateIndex = i;
        }
    }
    m_currentFrame = candidateIndex;
}
#endif

#if !ASSERT_DISABLED
bool BitmapImage::notSolidColor()
{
    return size().width() != 1 || size().height() != 1 || frameCount() > 1;
}
#endif



int BitmapImage::repetitionCount(bool imageKnownToBeComplete)
{
    if ((m_repetitionCountStatus == Unknown) || ((m_repetitionCountStatus == Uncertain) && imageKnownToBeComplete)) {
        // Snag the repetition count.  If |imageKnownToBeComplete| is false, the
        // repetition count may not be accurate yet for GIFs; in this case the
        // decoder will default to cAnimationLoopOnce, and we'll try and read
        // the count again once the whole image is decoded.
#if PLATFORM(WKC)
        if (imageKnownToBeComplete)
            m_source.isSizeAvailable();
#endif
        m_repetitionCount = m_source.repetitionCount();
        didDecodeProperties();
        m_repetitionCountStatus = (imageKnownToBeComplete || m_repetitionCount == cAnimationNone) ? Certain : Uncertain;
    }
    return m_repetitionCount;
}

bool BitmapImage::shouldAnimate()
{
    return (repetitionCount(false) != cAnimationNone && !m_animationFinished && imageObserver());
}

void BitmapImage::startAnimation(bool catchUpIfNecessary)
{
    if (m_frameTimer || !shouldAnimate() || frameCount() <= 1)
        return;

    // If we aren't already animating, set now as the animation start time.
    const double time = monotonicallyIncreasingTime();
    if (!m_desiredFrameStartTime)
        m_desiredFrameStartTime = time;

    // Don't advance the animation to an incomplete frame.
    size_t nextFrame = (m_currentFrame + 1) % frameCount();
    if (!m_allDataReceived && !frameIsCompleteAtIndex(nextFrame))
        return;

    // Don't advance past the last frame if we haven't decoded the whole image
    // yet and our repetition count is potentially unset.  The repetition count
    // in a GIF can potentially come after all the rest of the image data, so
    // wait on it.
    if (!m_allDataReceived && repetitionCount(false) == cAnimationLoopOnce && m_currentFrame >= (frameCount() - 1))
        return;

    // Determine time for next frame to start.  By ignoring paint and timer lag
    // in this calculation, we make the animation appear to run at its desired
    // rate regardless of how fast it's being repainted.
    const double currentDuration = frameDurationAtIndex(m_currentFrame);
    m_desiredFrameStartTime += currentDuration;

    // When an animated image is more than five minutes out of date, the
    // user probably doesn't care about resyncing and we could burn a lot of
    // time looping through frames below.  Just reset the timings.
#if PLATFORM(WKC)
    const double cAnimationResyncCutoff = 0;
#else
    const double cAnimationResyncCutoff = 5 * 60;
#endif
    if ((time - m_desiredFrameStartTime) > cAnimationResyncCutoff)
        m_desiredFrameStartTime = time + currentDuration;

    // The image may load more slowly than it's supposed to animate, so that by
    // the time we reach the end of the first repetition, we're well behind.
    // Clamp the desired frame start time in this case, so that we don't skip
    // frames (or whole iterations) trying to "catch up".  This is a tradeoff:
    // It guarantees users see the whole animation the second time through and
    // don't miss any repetitions, and is closer to what other browsers do; on
    // the other hand, it makes animations "less accurate" for pages that try to
    // sync an image and some other resource (e.g. audio), especially if users
    // switch tabs (and thus stop drawing the animation, which will pause it)
    // during that initial loop, then switch back later.
    if (nextFrame == 0 && m_repetitionsComplete == 0 && m_desiredFrameStartTime < time)
        m_desiredFrameStartTime = time;

    if (!catchUpIfNecessary || time < m_desiredFrameStartTime) {
        // Haven't yet reached time for next frame to start; delay until then.
        m_frameTimer = new Timer<BitmapImage>(this, &BitmapImage::advanceAnimation);
        m_frameTimer->startOneShot(std::max(m_desiredFrameStartTime - time, 0.));
    } else {
        // We've already reached or passed the time for the next frame to start.
        // See if we've also passed the time for frames after that to start, in
        // case we need to skip some frames entirely.  Remember not to advance
        // to an incomplete frame.
        for (size_t frameAfterNext = (nextFrame + 1) % frameCount(); frameIsCompleteAtIndex(frameAfterNext); frameAfterNext = (nextFrame + 1) % frameCount()) {
            // Should we skip the next frame?
            double frameAfterNextStartTime = m_desiredFrameStartTime + frameDurationAtIndex(nextFrame);
            if (time < frameAfterNextStartTime)
                break;

            // Yes; skip over it without notifying our observers.
            if (!internalAdvanceAnimation(true))
                return;
            m_desiredFrameStartTime = frameAfterNextStartTime;
            nextFrame = frameAfterNext;
        }

        // Draw the next frame immediately.  Note that m_desiredFrameStartTime
        // may be in the past, meaning the next time through this function we'll
        // kick off the next advancement sooner than this frame's duration would
        // suggest.
        if (internalAdvanceAnimation(false)) {
            // The image region has been marked dirty, but once we return to our
            // caller, draw() will clear it, and nothing will cause the
            // animation to advance again.  We need to start the timer for the
            // next frame running, or the animation can hang.  (Compare this
            // with when advanceAnimation() is called, and the region is dirtied
            // while draw() is not in the callstack, meaning draw() gets called
            // to update the region and thus startAnimation() is reached again.)
            // NOTE: For large images with slow or heavily-loaded systems,
            // throwing away data as we go (see destroyDecodedData()) means we
            // can spend so much time re-decoding data above that by the time we
            // reach here we're behind again.  If we let startAnimation() run
            // the catch-up code again, we can get long delays without painting
            // as we race the timer, or even infinite recursion.  In this
            // situation the best we can do is to simply change frames as fast
            // as possible, so force startAnimation() to set a zero-delay timer
            // and bail out if we're not caught up.
            startAnimation(false);
        }
    }
}

void BitmapImage::stopAnimation()
{
    // This timer is used to animate all occurrences of this image.  Don't invalidate
    // the timer unless all renderers have stopped drawing.
    delete m_frameTimer;
    m_frameTimer = 0;
}

void BitmapImage::resetAnimation()
{
    stopAnimation();
    m_currentFrame = 0;
    m_repetitionsComplete = 0;
    m_desiredFrameStartTime = 0;
    m_animationFinished = false;
    
    // For extremely large animations, when the animation is reset, we just throw everything away.
    destroyDecodedDataIfNecessary(true);
}

unsigned BitmapImage::decodedSize() const
{
    return m_decodedSize;
}



void BitmapImage::advanceAnimation(Timer<BitmapImage>*)
{
    internalAdvanceAnimation(false);
    // At this point the image region has been marked dirty, and if it's
    // onscreen, we'll soon make a call to draw(), which will call
    // startAnimation() again to keep the animation moving.
}

bool BitmapImage::internalAdvanceAnimation(bool skippingFrames)
{
    // Stop the animation.
    stopAnimation();
    
    // See if anyone is still paying attention to this animation.  If not, we don't
    // advance and will remain suspended at the current frame until the animation is resumed.
    if (!skippingFrames && imageObserver()->shouldPauseAnimation(this))
        return false;

    ++m_currentFrame;
    bool advancedAnimation = true;
    bool destroyAll = false;
    if (m_currentFrame >= frameCount()) {
        ++m_repetitionsComplete;

        // Get the repetition count again.  If we weren't able to get a
        // repetition count before, we should have decoded the whole image by
        // now, so it should now be available.
        // Note that we don't need to special-case cAnimationLoopOnce here
        // because it is 0 (see comments on its declaration in ImageSource.h).
        if (repetitionCount(true) != cAnimationLoopInfinite && m_repetitionsComplete > m_repetitionCount) {
            m_animationFinished = true;
            m_desiredFrameStartTime = 0;
            --m_currentFrame;
            advancedAnimation = false;
        } else {
            m_currentFrame = 0;
            destroyAll = true;
        }
    }
    destroyDecodedDataIfNecessary(destroyAll);

    // We need to draw this frame if we advanced to it while not skipping, or if
    // while trying to skip frames we hit the last frame and thus had to stop.
    if (skippingFrames != advancedAnimation)
        imageObserver()->animationAdvanced(this);
    return advancedAnimation;
}

bool BitmapImage::mayFillWithSolidColor()
{
    if (!m_checkedForSolidColor && frameCount() > 0) {
        checkForSolidColor();
        // WINCE PORT: checkForSolidColor() doesn't set m_checkedForSolidColor until
        // it gets enough information to make final decision.
#if !OS(WINCE)
        ASSERT(m_checkedForSolidColor);
#endif
    }
    return m_isSolidColor && !m_currentFrame;
}

Color BitmapImage::solidColor() const
{
    return m_solidColor;
}

#if !USE(CG)
void BitmapImage::draw(GraphicsContext* ctx, const FloatRect& dstRect, const FloatRect& srcRect, ColorSpace styleColorSpace, CompositeOperator op, RespectImageOrientationEnum)
{
    draw(ctx, dstRect, srcRect, styleColorSpace, op);
}
#endif

}
