/*
 * Copyright (c) 2013 ACCESS CO., LTD. All rights reserved.
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

#ifndef MPOImageDecoder_h
#define MPOImageDecoder_h

#include "ImageDecoder.h"
#include <wtf/OwnPtr.h>

namespace WebCore {

class JPEGImageDecoder;

    class MPOImageDecoder : public ImageDecoder {
    public:
        MPOImageDecoder(ImageSource::AlphaOption, ImageSource::GammaAndColorProfileOption);
        virtual ~MPOImageDecoder();

        virtual String filenameExtension() const { return "mpo"; }
        virtual void setData(SharedBuffer* data, bool allDataReceived);
        virtual bool isSizeAvailable();
        virtual IntSize size() const;
        virtual bool setSize(unsigned width, unsigned height);
        virtual ImageFrame* frameBufferAtIndex(size_t index);
        virtual bool setFailed();

        virtual size_t frameCount() { return 2; }

#if PLATFORM(WKC)
        virtual bool isThreeDImage() const;
#endif

    private:
        JPEGImageDecoder* m_decoderL;
        JPEGImageDecoder* m_decoderR;
        int m_rightImageOffset;
        int m_rightImageLen;
        bool m_finishedLDecode;
        bool m_decodingL;
        bool m_failedL;
        bool m_failedR;
    };
} // namespace WebCore

#endif
