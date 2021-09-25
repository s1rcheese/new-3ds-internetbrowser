/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#include "ExecutableAllocator.h"

#if ENABLE(EXECUTABLE_ALLOCATOR_FIXED)

#include "CodeProfiling.h"
#if !PLATFORM(WKC)
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
#include <wtf/MetaAllocator.h>
#include <wtf/PageReservation.h>
#include <wtf/VMTags.h>

#if OS(LINUX)
#include <stdio.h>
#endif

#if PLATFORM(WKC)
#include <wkc/wkcheappeer.h>
#endif

using namespace WTF;

namespace JSC {
    
#if !(PLATFORM(WKC) && COMPILER(CCPPC))
#if CPU(ARM)
static const size_t fixedPoolSize = 16 * 1024 * 1024;
#elif CPU(X86_64)
static const size_t fixedPoolSize = 1024 * 1024 * 1024;
#else
static const size_t fixedPoolSize = 32 * 1024 * 1024;
#endif
#endif // !(PLATFORM(WKC) && COMPILER(CCPPC))

class FixedVMPoolExecutableAllocator : public MetaAllocator {
public:
    FixedVMPoolExecutableAllocator()
        : MetaAllocator(32) // round up all allocations to 32 bytes
    {
#if PLATFORM(WKC) && COMPILER(CCPPC)
        const size_t fixedPoolSize = wkcHeapGetHeapTotalSizeForExecutableRegionPeer();
        ASSERT(fixedPoolSize > 0);
        m_reservation = PageReservation::reserve(fixedPoolSize, OSAllocator::JSJITCodePages, EXECUTABLE_POOL_WRITABLE, true);
#else
        m_reservation = PageReservation::reserveWithGuardPages(fixedPoolSize, OSAllocator::JSJITCodePages, EXECUTABLE_POOL_WRITABLE, true);
#endif
#if !(ENABLE(CLASSIC_INTERPRETER) || ENABLE(LLINT))
        if (!m_reservation)
            CRASH();
#endif
        if (m_reservation) {
            ASSERT(m_reservation.size() == fixedPoolSize);
            addFreshFreeSpace(m_reservation.base(), m_reservation.size());
        }
    }
    
protected:
    virtual void* allocateNewSpace(size_t&)
    {
        // We're operating in a fixed pool, so new allocation is always prohibited.
        return 0;
    }
    
    virtual void notifyNeedPage(void* page)
    {
#if OS(DARWIN)
        UNUSED_PARAM(page);
#else
        m_reservation.commit(page, pageSize());
#endif
    }
    
    virtual void notifyPageIsFree(void* page)
    {
#if OS(DARWIN)
        for (;;) {
            int result = madvise(page, pageSize(), MADV_FREE);
            if (!result)
                return;
            ASSERT(result == -1);
            if (errno != EAGAIN) {
                ASSERT_NOT_REACHED(); // In debug mode, this should be a hard failure.
                break; // In release mode, we should just ignore the error - not returning memory to the OS is better than crashing, especially since we _will_ be able to reuse the memory internally anyway.
            }
        }
#else
        m_reservation.decommit(page, pageSize());
#endif
    }

private:
    PageReservation m_reservation;
};

#if PLATFORM(WKC)
WKC_DEFINE_GLOBAL_PTR(FixedVMPoolExecutableAllocator*, allocator, 0);
#else
static FixedVMPoolExecutableAllocator* allocator;
#endif

void ExecutableAllocator::initializeAllocator()
{
    ASSERT(!allocator);
    allocator = new FixedVMPoolExecutableAllocator();
    CodeProfiling::notifyAllocator(allocator);
}

ExecutableAllocator::ExecutableAllocator(JSGlobalData&)
{
    ASSERT(allocator);
}

ExecutableAllocator::~ExecutableAllocator()
{
}

bool ExecutableAllocator::isValid() const
{
    return !!allocator->bytesReserved();
}

bool ExecutableAllocator::underMemoryPressure()
{
    MetaAllocator::Statistics statistics = allocator->currentStatistics();
    return statistics.bytesAllocated > statistics.bytesReserved / 2;
}

double ExecutableAllocator::memoryPressureMultiplier(size_t addedMemoryUsage)
{
    MetaAllocator::Statistics statistics = allocator->currentStatistics();
    ASSERT(statistics.bytesAllocated <= statistics.bytesReserved);
    size_t bytesAllocated = statistics.bytesAllocated + addedMemoryUsage;
    if (bytesAllocated >= statistics.bytesReserved)
        bytesAllocated = statistics.bytesReserved;
    double result = 1.0;
    size_t divisor = statistics.bytesReserved - bytesAllocated;
    if (divisor)
        result = static_cast<double>(statistics.bytesReserved) / divisor;
    if (result < 1.0)
        result = 1.0;
    return result;
}

PassRefPtr<ExecutableMemoryHandle> ExecutableAllocator::allocate(JSGlobalData& globalData, size_t sizeInBytes, void* ownerUID, JITCompilationEffort effort)
{
    RefPtr<ExecutableMemoryHandle> result = allocator->allocate(sizeInBytes, ownerUID);
    if (!result) {
        if (effort == JITCompilationCanFail)
            return result;
        releaseExecutableMemory(globalData);
        result = allocator->allocate(sizeInBytes, ownerUID);
        if (!result)
            CRASH();
    }
    return result.release();
}

size_t ExecutableAllocator::committedByteCount()
{
    return allocator->bytesCommitted();
}

#if PLATFORM(WKC)
size_t ExecutableAllocator::reservedByteCount()
{
    return allocator->bytesReserved();
}

size_t ExecutableAllocator::allocatedByteCount()
{
    return allocator->bytesAllocated();
}

size_t ExecutableAllocator::allocatableMaxSize()
{
    return allocator->maxContinuousFreeSpaceSize();
}
#endif

#if ENABLE(META_ALLOCATOR_PROFILE)
void ExecutableAllocator::dumpProfile()
{
    allocator->dumpProfile();
}
#endif

}


#endif // ENABLE(EXECUTABLE_ALLOCATOR_FIXED)
