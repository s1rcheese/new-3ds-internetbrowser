/*
 * Copyright (c) 2013, 2014 ACCESS CO., LTD. All rights reserved.
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

#if USE(MPO)

#include "MPOImageDecoder.h"
#include "JPEGImageDecoder.h"

namespace WebCore {

MPOImageDecoder::MPOImageDecoder(ImageSource::AlphaOption alphaOption, ImageSource::GammaAndColorProfileOption gammaAndColorProfileOption)
     : ImageDecoder(alphaOption, gammaAndColorProfileOption)
     , m_decoderL(new JPEGImageDecoder(alphaOption, gammaAndColorProfileOption))
     , m_decoderR(new JPEGImageDecoder(alphaOption, gammaAndColorProfileOption))
     , m_rightImageOffset(0)
     , m_rightImageLen(0)
     , m_finishedLDecode(false)
     , m_decodingL(true)
     , m_failedL(false)
     , m_failedR(false)
{
}

MPOImageDecoder::~MPOImageDecoder()
{
    delete m_decoderL;
    delete m_decoderR;
}

void
MPOImageDecoder::setData(SharedBuffer* data, bool allDataReceived)
{
    const char* buffer = 0;
    const unsigned char* p = 0;
    bool littleendian = false;

    if (m_rightImageOffset==0) {
        int len = data->getSomeData(buffer, 0);
        p = (unsigned char *)buffer;
        if (len>8) {
            if (!::memcmp(p, "\xFF\xD8\xFF", 3)) {
                int pos = 2;
                bool mpo = false;
                while (p[pos]==0xff && pos+4 < len) {
                    if (p[pos+1]==0xe2 && pos+8<len) {
                        if (!::memcmp(p+pos+4, "MPF\0", 4)) {
                            mpo = true;
                            break;
                        }
                    }
                    int next = p[pos+3] + (p[pos+2]<<8);
                    pos+=next+2;
                }
                if (mpo) {
                    int app2len = p[pos+3] + (p[pos+2]<<8);
                    p = p+pos;
                    if (len>=app2len) {
                        if (!::memcmp(p+8, "\x49\x49\x2a\x00", 4))
                            littleendian = true;
                        int ifd = 0;
                        if (littleendian)
                            ifd = (p[12] + (p[13]<<8) + (p[14]<<16) + (p[15]<<24));
                        else
                            ifd = (p[15] + (p[14]<<8) + (p[13]<<16) + (p[12]<<24));

                        int mpentries = 0;
                        if (littleendian)
                            mpentries = p[ifd+38] + (p[ifd+39]<<8) + (p[ifd+40]<<16) + (p[ifd+41]<<24);
                        else
                            mpentries = p[ifd+41] + (p[ifd+40]<<8) + (p[ifd+39]<<16) + (p[ifd+38]<<24);

                        if (mpentries<=16) {
                            delete m_decoderR;
                            m_decoderR = 0;
                            m_rightImageOffset = 0;
                        } else {
                            int mpe = ifd + 66;
                            if (littleendian)
                                m_rightImageOffset = p[mpe+8] + (p[mpe+9]<<8) + (p[mpe+10]<<16) + (p[mpe+11]<<24);
                            else
                                m_rightImageOffset = p[mpe+11] + (p[mpe+10]<<8) + (p[mpe+9]<<16) + (p[mpe+8]<<24);
                            m_rightImageOffset += (p + 8 - (unsigned char *)buffer);
                        }
                    }
                }
            }
        }
    }

    if (!m_decoderR || data->size() < m_rightImageOffset) {
        m_decodingL = true;
        m_decoderL->setData(data, allDataReceived);
        m_failedL = m_decoderL->failed();
        if (allDataReceived)
            m_finishedLDecode = true;
    } else if (m_decoderR) {
        if (!m_finishedLDecode) {
            m_decodingL = true;
            m_decoderL->setData(data, true);
            m_finishedLDecode = true;
            m_failedL = m_decoderL->failed();
        }
        const char* rdata = data->data() + m_rightImageOffset;
        int rlen = data->size() - m_rightImageOffset;
        RefPtr<SharedBuffer> rbuf = SharedBuffer::create(rdata, rlen);
        m_decodingL = false;
        m_decoderR->setData(rbuf.get(), allDataReceived);
        m_failedR = m_decoderR->failed();
    }
    if (m_failedL && m_failedR)
        ImageDecoder::setFailed();
}

bool
MPOImageDecoder::isSizeAvailable()
{
    return m_decoderL->isSizeAvailable();
}

IntSize
MPOImageDecoder::size() const
{
    return m_decoderL->size();
}

bool
MPOImageDecoder::setSize(unsigned width, unsigned height)
{
    return m_decoderL->setSize(width, height);
}

ImageFrame*
MPOImageDecoder::frameBufferAtIndex(size_t index)
{
    ImageFrame* frame = 0;
    if (index==0)
        frame = m_decoderL->frameBufferAtIndex(0);
    else
        frame = m_decoderR->frameBufferAtIndex(0);

    return frame;
}

bool
MPOImageDecoder::setFailed()
{
    if (m_decodingL)
        return m_decoderL->setFailed();
    else
        return m_decoderR->setFailed();
}

#if PLATFORM(WKC)
bool
MPOImageDecoder::isThreeDImage() const
{
    if (!m_decoderL || !m_decoderR)
        return false;
    if (!m_decoderL->failed() && !m_decoderR->failed())
        return true;
    return false;
}
#endif

} // namespace

#endif // USE(MPO)
