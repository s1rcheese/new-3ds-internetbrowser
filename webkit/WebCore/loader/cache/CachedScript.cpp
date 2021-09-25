/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
    Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
    Copyright (c) 2012, 2013 ACCESS CO., LTD. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#include "config.h"
#include "CachedScript.h"

#include "MemoryCache.h"
#include "CachedResourceClient.h"
#include "CachedResourceClientWalker.h"
#include "SharedBuffer.h"
#include "TextResourceDecoder.h"
#include <wtf/Vector.h>

#if USE(JSC)  
#include <parser/SourceProvider.h>
#endif

#if PLATFORM(WKC)
#include "TextCodecWKC.h"
#include "TextEncodingDetector.h"
#include "TextEncodingRegistry.h"
#include <wkc/wkcmpeer.h>
#endif

namespace WebCore {

CachedScript::CachedScript(const ResourceRequest& resourceRequest, const String& charset)
    : CachedResource(resourceRequest, Script)
    , m_decoder(TextResourceDecoder::create("application/javascript", charset))
    , m_decodedDataDeletionTimer(this, &CachedScript::decodedDataDeletionTimerFired)
{
    // It's javascript we want.
    // But some websites think their scripts are <some wrong mimetype here>
    // and refuse to serve them if we only accept application/x-javascript.
    setAccept("*/*");
}

CachedScript::~CachedScript()
{
}

void CachedScript::didAddClient(CachedResourceClient* c)
{
    if (m_decodedDataDeletionTimer.isActive())
        m_decodedDataDeletionTimer.stop();

    CachedResource::didAddClient(c);
}

void CachedScript::allClientsRemoved()
{
    if (double interval = memoryCache()->deadDecodedDataDeletionInterval())
        m_decodedDataDeletionTimer.startOneShot(interval);
}

void CachedScript::setEncoding(const String& chs)
{
    m_decoder->setEncoding(chs, TextResourceDecoder::EncodingFromHTTPHeader);
}

String CachedScript::encoding() const
{
    return m_decoder->encoding().name();
}

const String& CachedScript::script()
{
    ASSERT(!isPurgeable());

    if (!m_script && m_data) {
#if PLATFORM(WKC)
        unsigned int bytesToBeAllocated;
        const char* data;
        size_t encodedLength;
        int decodedLength;
        OwnPtr<TextCodec> codecPassOwnPtr;
        if (m_data->needsReallocationToMergeBuffers(&bytesToBeAllocated)
            && !wkcMemoryCheckMemoryAllocatablePeer(bytesToBeAllocated, WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JAVASCRIPT)) {
            // Failed to allocate continuous memory area for encoded script.
            // The area is used for a text decoder to detect encoding of the script.
            wkcMemoryNotifyMemoryAllocationErrorPeer(bytesToBeAllocated, WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JAVASCRIPT);
            goto setEmptyScript;
        }
        data = m_data->data();
        encodedLength = encodedSize();
        if (!data || !encodedLength) {
            goto setEmptyScript;
        }

        decodedLength = 0;
        codecPassOwnPtr = newTextCodec(m_decoder->encoding());
        if (codecPassOwnPtr->isTextCodecWKC()) {
            TextCodecWKC* codec = static_cast<TextCodecWKC*>(codecPassOwnPtr.get());
            decodedLength = codec->getDecodedTextLength(data, encodedLength);
        } else {
            // encoding is Latin1 or UserDefined
            decodedLength = encodedLength;
            //How about the case where encoding is UTF-16???
            //decodedLength = encodedLength / 2;
        }
        if (decodedLength <= 0) {
            goto setEmptyScript;
        }

        bytesToBeAllocated = sizeof(StringImpl) + decodedLength * sizeof(UChar);
        if (!wkcMemoryCheckMemoryAllocatablePeer(bytesToBeAllocated, WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JAVASCRIPT)) {
            // Failed to allocate continuous memory area for decoded script.
            wkcMemoryNotifyMemoryAllocationErrorPeer(bytesToBeAllocated, WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JAVASCRIPT);
            goto setEmptyScript;
        } else {
#endif
        m_script = m_decoder->decode(m_data->data(), encodedSize());
        m_script += m_decoder->flush();
        setDecodedSize(m_script.length() * sizeof(UChar));
#if PLATFORM(WKC)
        }
        goto junction;

setEmptyScript:
        m_script = emptyString();
        setDecodedSize(0);
        setStatus(DecodeError);

junction:
        ;
#endif
    }
    m_decodedDataDeletionTimer.startOneShot(0);
    
    return m_script;
}

void CachedScript::data(PassRefPtr<SharedBuffer> data, bool allDataReceived)
{
    if (!allDataReceived)
        return;

    m_data = data;
    setEncodedSize(m_data.get() ? m_data->size() : 0);
    setLoading(false);
    checkNotify();
}

void CachedScript::error(CachedResource::Status status)
{
    setStatus(status);
    ASSERT(errorOccurred());
    setLoading(false);
    checkNotify();
}

void CachedScript::destroyDecodedData()
{
    m_script = String();
    unsigned extraSize = 0;
#if USE(JSC)
#if PLATFORM(WKC)
    if (m_sourceProviderCache)
#else
    if (m_sourceProviderCache && m_clients.isEmpty())
#endif
        m_sourceProviderCache->clear();

    extraSize = m_sourceProviderCache ? m_sourceProviderCache->byteSize() : 0;
#endif
    setDecodedSize(extraSize);
    if (!MemoryCache::shouldMakeResourcePurgeableOnEviction() && isSafeToMakePurgeable())
        makePurgeable(true);
}

void CachedScript::decodedDataDeletionTimerFired(Timer<CachedScript>*)
{
    destroyDecodedData();
}

#if USE(JSC)
JSC::SourceProviderCache* CachedScript::sourceProviderCache() const
{   
    if (!m_sourceProviderCache) 
        m_sourceProviderCache = adoptPtr(new JSC::SourceProviderCache); 
    return m_sourceProviderCache.get(); 
}

void CachedScript::sourceProviderCacheSizeChanged(int delta)
{
    setDecodedSize(decodedSize() + delta);
}
#endif

} // namespace WebCore
