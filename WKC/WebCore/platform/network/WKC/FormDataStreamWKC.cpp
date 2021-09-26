/*
 * Copyright (C) 2008 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Alp Toker <alp.toker@collabora.co.uk>
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora Ltd.
 * All rights reserved.
 * Copyright (c) 2010, 2015 ACCESS CO., LTD. All rights reserved.
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
#include "FormDataStreamWKC.h"

#include "CString.h"
#include "FormData.h"
#include "ResourceRequest.h"
#include "ResourceHandle.h"
#include "ResourceHandleInternalWKC.h"
#include "ResourceHandleClient.h"
#if ENABLE(BLOB)
#include "BlobRegistryImpl.h"
#endif

namespace WebCore {

extern "C" void peerDebugPrintf(const char* in_format, ...);

FormDataStream::~FormDataStream()
{
    if (m_file)
        wkcFileFClosePeer(m_file);
}

size_t FormDataStream::read(void* ptr, size_t blockSize, size_t numberOfBlocks)
{
    ResourceHandleInternal* d = m_resourceHandle->getInternal();
    if (!d->client())
        return 0;

    // Check for overflow.
    if (!numberOfBlocks || blockSize > std::numeric_limits<size_t>::max() / numberOfBlocks)
        return 0;

    Vector<FormDataElement> elements;
    if (m_resourceHandle->firstRequest().httpBody())
        elements = m_resourceHandle->firstRequest().httpBody()->elements();

    if (m_formDataElementIndex >= elements.size())
        return 0;

    FormDataElement element = elements[m_formDataElementIndex];

    size_t toSend = blockSize * numberOfBlocks;
    size_t sent;

    if (element.m_type == FormDataElement::encodedFile) {
        if (!m_file) {
            m_file = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_FORMDATA, element.m_filename.utf8().data(), "rb");
            if (!m_file) {
#ifndef NDEBUG
                peerDebugPrintf("Failed while trying to open %s for upload\n", element.m_filename.utf8().data());
#endif
                m_file = 0;
                m_formDataElementIndex++;
                return 0;
            }
        }

        sent = wkcFileFReadPeer(ptr, blockSize, numberOfBlocks, m_file);
        if ((int)sent < 0) {
#ifndef NDEBUG
            peerDebugPrintf("Failed while trying to read %s for upload\n", element.m_filename.utf8().data());
#endif
            return 0;
        }
        if (wkcFileFeofPeer(m_file)) {
            wkcFileFClosePeer(m_file);
            m_file = 0;
            m_formDataElementIndex++;
        }
#if ENABLE(BLOB)
    } else if (element.m_type == FormDataElement::encodedBlob) {
        RefPtr<BlobStorageData> blobData = static_cast<BlobRegistryImpl&>(blobRegistry()).getBlobDataFromURL(KURL(ParsedURLString, element.m_blobURL));
        if (blobData) {
            for (size_t j = 0; j < blobData->items().size(); ++j) {
                const BlobDataItem& blobItem = blobData->items()[j];
                if (blobItem.type == BlobDataItem::Data) {
                    size_t elementSize = static_cast<int>(blobItem.length) - m_formDataElementDataOffset;
                    sent = elementSize > toSend ? toSend : elementSize;
                    const char* p = blobItem.data->data() + static_cast<int>(blobItem.offset);
                    memcpy(ptr, p + m_formDataElementDataOffset, sent);
                    if (elementSize > sent)
                        m_formDataElementDataOffset += sent;
                    else {
                        m_formDataElementDataOffset = 0;
                        m_formDataElementIndex++;
                    }
                }
            }
        }
#endif
    } else {
        size_t elementSize = element.m_data.size() - m_formDataElementDataOffset;
        sent = elementSize > toSend ? toSend : elementSize;
        memcpy(ptr, element.m_data.data() + m_formDataElementDataOffset, sent);
        if (elementSize > sent)
            m_formDataElementDataOffset += sent;
        else {
            m_formDataElementDataOffset = 0;
            m_formDataElementIndex++;
        }
    }

    return sent;
}

bool FormDataStream::hasMoreElements() const
{
    Vector<FormDataElement> elements;
    if (m_resourceHandle->firstRequest().httpBody())
        elements = m_resourceHandle->firstRequest().httpBody()->elements();

    return m_formDataElementIndex < elements.size();
}

void FormDataStream::refresh()
{
    m_formDataElementIndex = 0;
    m_formDataElementDataOffset = 0;
}

} // namespace WebCore
