/*
 *  wkcmpeer.h
 *
 *  Copyright(c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 */

#ifndef _WKC_MEMORY_PEER_H_
#define _WKC_MEMORY_PEER_H_

#include <stdio.h>

#include <wkc/wkcbase.h>

/**
@file
@brief Do not change this file since it operates with the current implementation.
*/

/*@{*/

WKC_BEGIN_C_LINKAGE

#ifndef WKC_MPEER_COMPILE_ASSERT
#define WKC_MPEER_COMPILE_ASSERT(exp, name) typedef int dummy##name [(exp) ? 1 : -1]
#endif

// memory peer
typedef void* (*wkcPeerMallocProc) (unsigned int, int);
typedef void  (*wkcPeerFreeProc) (void*);
typedef void* (*wkcPeerReallocProc) (void*, unsigned int, int);

WKC_PEER_API bool wkcMemoryInitializePeer(wkcPeerMallocProc in_malloc, wkcPeerFreeProc in_free, wkcPeerReallocProc in_realloc);
WKC_PEER_API bool wkcMemoryIsInitializedPeer(void);
WKC_PEER_API void wkcMemoryFinalizePeer(void);
WKC_PEER_API void wkcMemoryForceTerminatePeer(void);

WKC_PEER_API void wkcMemorySetNotifyNoMemoryProcPeer(void*(*in_proc)(unsigned int));
WKC_PEER_API void wkcMemorySetNotifyMemoryAllocationErrorProcPeer(void(*in_proc)(unsigned int, int));
WKC_PEER_API void wkcMemorySetNotifyCrashProcPeer(void(*in_proc)(const char*, int, const char*, const char*));
WKC_PEER_API void wkcMemorySetNotifyStackOverflowProcPeer(void(*in_proc)(bool, unsigned int, unsigned int, unsigned int, void*, void*, void*, const char*, int, const char*));
WKC_PEER_API void wkcMemorySetCheckAvailabilityProcPeer(bool(*in_proc)(unsigned int, bool));
WKC_PEER_API void wkcMemorySetCheckMemoryAllocatableProcPeer(bool(*in_proc)(unsigned int, int));

WKC_PEER_API void* wkcMemoryNotifyNoMemoryPeer(unsigned int in_size);
WKC_PEER_API void wkcMemorySetCrashReasonPeer(const char* in_file, int in_line, const char* in_function, const char* in_assertion);
WKC_PEER_API void wkcMemoryNotifyCrashPeer(void);
WKC_PEER_API void wkcMemoryNotifyStackOverflowPeer(bool in_need_restart, unsigned int in_margin, unsigned int in_consumption, void* in_current_stack_top, const char* in_file, int in_line, const char* in_function);
WKC_PEER_API bool wkcMemoryIsStackOverflowPeer(unsigned int in_margin, unsigned int *out_consumption, void** out_current_stack_top);

WKC_PEER_API bool wkcMemoryCheckAvailabilityPeer(unsigned int in_size);
WKC_PEER_API void wkcMemorySetAllocatingForSVGPeer(bool in_flag);
WKC_PEER_API void wkcMemorySetAllocatingForImagesPeer(bool in_flag);
WKC_PEER_API void wkcMemorySetAllocationForAnimeGifPeer(bool in_flag);
WKC_PEER_API bool wkcMemoryIsAllocatingForSVGPeer(void);
WKC_PEER_API int wkcMemoryGetAllocationStatePeer(void);

WKC_PEER_API bool wkcMemoryIsCrashingPeer(void);

enum {
    WKC_MEMORYALLOC_TYPE_IMAGE_UNSPECIFIED,
    WKC_MEMORYALLOC_TYPE_IMAGE_GIFANIME,
    WKC_MEMORYALLOC_TYPE_IMAGE_JPEG,
    WKC_MEMORYALLOC_TYPE_IMAGE_PNG,
    WKC_MEMORYALLOC_TYPE_LAYER,
    WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_GIF,
    WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JPEG,
    WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_PNG,
    WKC_MEMORYALLOC_TYPE_SHAREDBUFFER_JAVASCRIPT,
    WKC_MEMORYALLOC_TYPE_JAVASCRIPT_HEAP,
    WKC_MEMORYALLOC_TYPE_PIXMAN,
    WKC_MEMORYALLOC_TYPE_ASSEMBLERBUFFER,
    WKC_MEMORYALLOC_TYPES
};

WKC_PEER_API bool wkcMemoryCheckMemoryAllocatablePeer(unsigned int in_size, int in_reason);
WKC_PEER_API void wkcMemoryNotifyMemoryAllocationErrorPeer(unsigned int in_size, int in_reason);

WKC_PEER_API void wkcMemoryRegisterGlobalObjPeer(volatile void* in_ptr, int in_size);

/* for peer_fastmalloc */

WKC_PEER_API size_t wkcMemoryGetPageSizePeer(void);
WKC_PEER_API void wkcMemorySetExecutablePeer(void* in_ptr, size_t in_size, bool in_executable);
WKC_PEER_API void wkcMemoryCacheFlushPeer(void* in_ptr, size_t in_size);


WKC_PEER_API void* wkcMemoryFillMem16Peer( void* dst, unsigned short data, unsigned int len );
WKC_PEER_API unsigned short* wkcMemoryFillRange16Peer( unsigned short* dst, unsigned short stride, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int data );
WKC_PEER_API void* wkcMemoryFillMem32Peer( void* dst, unsigned int data, unsigned int len );
WKC_PEER_API unsigned int* wkcMemoryFillRange32Peer( unsigned int* dst, unsigned int stride, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int data );
WKC_PEER_API void* wkcMemoryMoveMemPeer( void* dst, void* src, unsigned int len );
WKC_PEER_API void* wkcMemoryCopyAlignedMemPeer( void* dst, void* src, unsigned int len );

WKC_END_C_LINKAGE

/*@}*/

#endif /* _WKC_MEMORY_PEER_H_ */
