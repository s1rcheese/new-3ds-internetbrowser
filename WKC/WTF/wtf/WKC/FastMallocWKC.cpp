/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 *
 */

#include "config.h"
#include "FastMalloc.h"

#include <wkc/wkcheappeer.h>
#include <string.h>

#define MCRASH(n) wkcMemoryNotifyNoMemoryPeer((n))

namespace WTF {
void* fastCalloc(size_t p,size_t n)
{
    void* result;

    result = wkcHeapCallocPeer(p, n);
    if(!result)
        MCRASH(p);

    return result;
}

void fastFree(void* p)
{
    return wkcHeapFreePeer(p);
}

void* fastMalloc(size_t size)
{
    void* result;

    result = wkcHeapMallocPeer(size);
    if(!result)
        MCRASH(size);

    return result;
}

void* fastRealloc(void* old_ptr, size_t new_size)
{
    void *result;

    result = wkcHeapReallocPeer(old_ptr, new_size);
    if(!result)
        MCRASH(new_size);

    return result;
}

void* fastZeroedMalloc(size_t n) 
{
    void* result;

    result = wkcHeapZeroedMallocPeer(n);
    if (!result)
        MCRASH(n);

    return result;
}

TryMallocReturnValue tryFastCalloc(size_t n, size_t elem_size)
{
    return wkcHeapCallocPeer(n, elem_size);
}

TryMallocReturnValue tryFastMalloc(size_t size)
{
    return wkcHeapMallocPeer(size);
}

TryMallocReturnValue tryFastRealloc(void* old_ptr, size_t new_size)
{
    return wkcHeapReallocPeer(old_ptr, new_size);
}

TryMallocReturnValue tryFastZeroedMalloc(size_t n)
{
    void* result;
    if (!tryFastMalloc(n).getValue(result))
        return 0;
    memset(result, 0, n);
    return result;
}

void fastMallocAllow()
{
    wkcHeapAllowPeer();
}

void fastMallocForbid()
{
    wkcHeapForbidPeer();
}

size_t fastMallocSize(const void* ptr)
{
    return wkcHeapMallocSizePeer(ptr);
}

char* fastStrDup(const char* src)
{
    return wkcHeapStrDupPeer(src);
}

}

