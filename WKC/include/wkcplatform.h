/*
 * wkcplatform.h
 *
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc.  All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#ifndef _WKCPLATFORM_H_
#define _WKCPLATFORM_H_

#include <wkc/wkcpeer.h>
#include <wkc/wkcmpeer.h>

#define WTF_PLATFORM_WKC 1

#include <wtf/Compiler.h>

// This definition is here, not in wtf/Compiler.h, for a maintenance reason.
/* COMPILER(CCPPC) */
#if defined(__ghs__)
#define WTF_COMPILER_CCPPC 1
#endif


// defines in makefile
#define ENABLE_CHANNEL_MESSAGING 0
#define ENABLE_DETAILS 1
#define ENABLE_METER_TAG 1
#define ENABLE_PROGRESS_TAG 1
#define ENABLE_OFFLINE_WEB_APPLICATIONS 1
#define ENABLE_SQL_DATABASE 1
#define ENABLE_DATA_TRANSFER_ITEMS 0
#define ENABLE_STYLE_SCOPED 1
//#define ENABLE_INDEXED_DATABASE 1
//#define WTF_USE_LEVELDB 1
#define ENABLE_DIRECTORY_UPLOAD 0
#define ENABLE_MUTATION_OBSERVERS 1
#define ENABLE_FILE_SYSTEM 0
#define ENABLE_QUOTA 0
#define ENABLE_TOUCH_ICON_LOADING 0
#define ENABLE_ICONDATABASE 1
#define ENABLE_ORIENTATION_EVENTS 1
#define ENABLE_DEVICE_ORIENTATION 1
#define ENABLE_GAMEPAD 1
#define ENABLE_BATTERY_STATUS 1
#define ENABLE_TOUCH_EVENTS 1
#define ENABLE_VIDEO 1
#define ENABLE_VIDEO_TRACK 1
#define ENABLE_MEDIA_STATISTICS 1
#define ENABLE_FULLSCREEN_API 1
#define ENABLE_XSLT 0
#define ENABLE_WORKERS 0
#define ENABLE_SHADOW_DOM 1
#define ENABLE_SHARED_WORKERS 0
#define ENABLE_FILTERS 1
#define ENABLE_GEOLOCATION 0
// ref.http://code.google.com/p/chromium/issues/detail?id=152430
//#define ENABLE_MATHML 1
#define ENABLE_NOTIFICATIONS 0
#define ENABLE_LEGACY_NOTIFICATIONS 0
#define ENABLE_SVG 1
#define ENABLE_SVG_FONTS 1
#define ENABLE_BLOB 1
#define ENABLE_MHTML 1
#define ENABLE_MICRODATA 0
#define ENABLE_REGISTER_PROTOCOL_HANDLER 0
#define ENABLE_REQUEST_ANIMATION_FRAME 1
#define ENABLE_WEB_ARCHIVE 1
#define ENABLE_WEB_SOCKETS 1
#ifdef WKC_ENABLE_WEBGL
# define ENABLE_WEBGL 0
#endif
#define ENABLE_WEB_AUDIO 0
#ifdef WKC_ENABLE_ACCELERATED_COMPOSITING
# define ENABLE_ACCELERATED_2D_CANVAS 0
# define WTF_USE_ACCELERATED_COMPOSITING 1
# ifdef WKC_ENABLE_COMPOSITED_FIXED_ELEMENTS
#  define ENABLE_COMPOSITED_FIXED_ELEMENTS 1
# endif
#endif
#define ENABLE_3D_RENDERING 1
#define ENABLE_WEB_INTENTS 0
#define ENABLE_DATALIST 0
#define ENABLE_PAGE_VISIBILITY_API 1
#define ENABLE_MEDIA_STREAM 0
#define ENABLE_XHR_RESPONSE_BLOB 1

#define ENABLE_INPUT_SPEECH 0
#define ENABLE_INPUT_TYPE_COLOR 0
#define ENABLE_INPUT_TYPE_MONTH 1
#define ENABLE_INPUT_TYPE_WEEK 1
#define ENABLE_INPUT_TYPE_DATE 1
// see http://lists.webkit.org/pipermail/webkit-dev/2013-January/023404.html
//#define ENABLE_INPUT_TYPE_DATETIME 1
#define ENABLE_INPUT_TYPE_DATETIMELOCAL 1
#define ENABLE_INPUT_TYPE_TIME 1

#define WTF_USE_REQUEST_ANIMATION_FRAME_TIMER 0

#define ENABLE_CSS3_FLEXBOX 1
#define ENABLE_CSS_EXCLUSIONS 1
#define ENABLE_CSS_FILTERS 1
#define ENABLE_CSS_REGIONS 1
#define ENABLE_CSS_GRID_LAYOUT 1
#define ENABLE_CSS_SHADERS 0

#define ENABLE_WEB_TIMING 1

#ifndef _MSC_VER
#define WKC_USE_REMOTEWEBINSPECTOR
#endif

// wkc configurations

#define HAVE_ERRNO_H 0
#define HAVE_LANGINFO_H 0
#define HAVE_MMAP 0
#define HAVE_SBRK 0
#define HAVE_STRINGS_H 0
#define HAVE_SYS_PARAM_H 0
#define HAVE_SYS_TIME_H 0

#define HAVE_ACCESSIBILITY 0

#define ENABLE_CONTEXT_MENUS 0
#define ENABLE_DRAG_SUPPORT 1
#define ENABLE_DASHBOARD_SUPPORT 0
#define ENABLE_MAC_JAVA_BRIDGE 0
#define ENABLE_JAVA_BRIDGE 0
#define ENABLE_NETSCAPE_PLUGIN_API 1
#define ENABLE_NETSCAPE_PLUGIN_METADATA_CACHE 0
#define WTF_USE_PLUGIN_HOST_PROCESS 0
#define ENABLE_PURGEABLE_MEMORY 0
#define ENABLE_OPCODE_STATS 0
#define ENABLE_LEGACY_VIEWPORT_ADAPTION 1

#define ENABLE_FAST_MALLOC_MATCH_VALIDATION 0
#define ENABLE_GLOBAL_FASTMALLOC_NEW 1
#define ENABLE_TEXT_CARET 1
#define ENABLE_REPAINT_THROTTLING 1
#define ENABLE_PARSED_STYLE_SHEET_CACHING 1
#define ENABLE_SUBPIXEL_LAYOUT 0
#define ENABLE_POINTER_LOCK 0

#define WTF_USE_OS_RANDOMNESS 1

#ifdef WKC_ENABLE_JIT
# define ENABLE_JIT 1
//# define ENABLE_DFG_JIT 1
//# define ENABLE_LLINT 1
# define ENABLE_YARR_JIT 1
# define ENABLE_CLASSIC_INTERPRETER 0
#else
# define ENABLE_JIT 0
# define ENABLE_DFG_JIT 0
# define ENABLE_LLINT 0
# define ENABLE_YARR_JIT 0
# define ENABLE_CLASSIC_INTERPRETER 1
#endif

//#define ENABLE_PAN_SCROLLING 1
//#define ENABLE_SMOOTH_SCROLLING 0

//#define WTF_USE_PROTECTION_SPACE_AUTH_CALLBACK 1

#define WTF_USE_WKC_UNICODE 1

// support MPO file
#define WTF_USE_MPO 1

#ifdef WKC_USE_REMOTEWEBINSPECTOR
# define ENABLE_JAVASCRIPT_DEBUGGER 1
# define ENABLE_INSPECTOR 1
# define ENABLE_INSPECTOR_SERVER 1
#endif

// prevent to include original wtf/Platform.h
#ifndef WTF_Platform_h
# define WTF_Platform_h
#endif // WTF_Platform_h

/* ==== PLATFORM handles OS, operating environment, graphics API, and
   CPU. This macro will be phased out in favor of platform adaptation
   macros, policy decision macros, and top-level port definitions. ==== */
#define PLATFORM(WTF_FEATURE) (defined WTF_PLATFORM_##WTF_FEATURE  && WTF_PLATFORM_##WTF_FEATURE)


/* ==== Platform adaptation macros: these describe properties of the target environment. ==== */

/* CPU() - the target CPU architecture */
#define CPU(WTF_FEATURE) (defined WTF_CPU_##WTF_FEATURE  && WTF_CPU_##WTF_FEATURE)
/* HAVE() - specific system features (headers, functions or similar) that are present or not */
#define HAVE(WTF_FEATURE) (defined HAVE_##WTF_FEATURE  && HAVE_##WTF_FEATURE)
/* OS() - underlying operating system; only to be used for mandated low-level services like 
   virtual memory, not to choose a GUI toolkit */
#define OS(WTF_FEATURE) (defined WTF_OS_##WTF_FEATURE  && WTF_OS_##WTF_FEATURE)


/* ==== Policy decision macros: these define policy choices for a particular port. ==== */

/* USE() - use a particular third-party library or optional OS service */
#define USE(WTF_FEATURE) (defined WTF_USE_##WTF_FEATURE  && WTF_USE_##WTF_FEATURE)
/* ENABLE() - turn on a specific feature of WebKit */
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)


/* ==== CPU() - the target CPU architecture ==== */

/* This also defines CPU(BIG_ENDIAN) or CPU(MIDDLE_ENDIAN) or neither, as appropriate. */

/* CPU(ALPHA) - DEC Alpha */
#if defined(__alpha__)
#define WTF_CPU_ALPHA 1
#endif

/* CPU(IA64) - Itanium / IA-64 */
#if defined(__ia64__)
#define WTF_CPU_IA64 1
/* 32-bit mode on Itanium */
#if !defined(__LP64__)
#define WTF_CPU_IA64_32 1
#endif
#endif

/* CPU(MIPS) - MIPS 32-bit */
/* Note: Only O32 ABI is tested, so we enable it for O32 ABI for now.  */
#if (defined(mips) || defined(__mips__) || defined(MIPS) || defined(_MIPS_)) \
    && defined(_ABIO32)
#define WTF_CPU_MIPS 1
#if defined(__MIPSEB__)
#define WTF_CPU_BIG_ENDIAN 1
#endif
#define WTF_MIPS_PIC (defined __PIC__)
#define WTF_MIPS_ARCH __mips
#define WTF_MIPS_ISA(v) (defined WTF_MIPS_ARCH && WTF_MIPS_ARCH == v)
#define WTF_MIPS_ISA_AT_LEAST(v) (defined WTF_MIPS_ARCH && WTF_MIPS_ARCH >= v)
#define WTF_MIPS_ARCH_REV __mips_isa_rev
#define WTF_MIPS_ISA_REV(v) (defined WTF_MIPS_ARCH_REV && WTF_MIPS_ARCH_REV == v)
#define WTF_MIPS_DOUBLE_FLOAT (defined __mips_hard_float && !defined __mips_single_float)
#define WTF_MIPS_FP64 (defined __mips_fpr && __mips_fpr == 64)
/* MIPS requires allocators to use aligned memory */
#define WTF_USE_ARENA_ALLOC_ALIGNMENT_INTEGER 1
#endif /* MIPS */

/* CPU(PPC) - PowerPC 32-bit */
#if   defined(__ppc__)     \
    || defined(__PPC__)     \
    || defined(__powerpc__) \
    || defined(__powerpc)   \
    || defined(__POWERPC__) \
    || defined(_M_PPC)      \
    || defined(__PPC)
#define WTF_CPU_PPC 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(PPC64) - PowerPC 64-bit */
#if   defined(__ppc64__) \
    || defined(__PPC64__)
#define WTF_CPU_PPC64 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SH4) - SuperH SH-4 */
#if defined(__SH4__)
#define WTF_CPU_SH4 1
#endif

/* CPU(SPARC32) - SPARC 32-bit */
#if defined(__sparc) && !defined(__arch64__) || defined(__sparcv8)
#define WTF_CPU_SPARC32 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SPARC64) - SPARC 64-bit */
#if defined(__sparc__) && defined(__arch64__) || defined (__sparcv9)
#define WTF_CPU_SPARC64 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SPARC) - any SPARC, true for CPU(SPARC32) and CPU(SPARC64) */
#if CPU(SPARC32) || CPU(SPARC64)
#define WTF_CPU_SPARC 1
#endif

/* CPU(S390X) - S390 64-bit */
#if defined(__s390x__)
#define WTF_CPU_S390X 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(S390) - S390 32-bit */
#if defined(__s390__)
#define WTF_CPU_S390 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(X86) - i386 / x86 32-bit */
#if   defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL)
#define WTF_CPU_X86 1
#endif

/* CPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit */
#if   defined(__x86_64__) \
    || defined(_M_X64)
#define WTF_CPU_X86_64 1
#endif

/* CPU(ARM) - ARM, any version*/
#if   defined(arm) \
    || defined(__arm__) \
    || defined(ARM) \
    || defined(_ARM_)
#define WTF_CPU_ARM 1

#if defined(__ARMEB__) || (COMPILER(RVCT) && defined(__BIG_ENDIAN))
#define WTF_CPU_BIG_ENDIAN 1

#elif !defined(__ARM_EABI__) \
    && !defined(__EABI__) \
    && !defined(__VFP_FP__) \
    && !defined(_WIN32_WCE) \
    && !defined(ANDROID)
#define WTF_CPU_MIDDLE_ENDIAN 1

#endif

#define WTF_ARM_ARCH_AT_LEAST(N) (CPU(ARM) && WTF_ARM_ARCH_VERSION >= N)

/* Set WTF_ARM_ARCH_VERSION */
#if   defined(__ARM_ARCH_4__) \
    || defined(__ARM_ARCH_4T__) \
    || defined(__MARM_ARMV4__) \
    || defined(_ARMV4I_)
#define WTF_ARM_ARCH_VERSION 4

#elif defined(__ARM_ARCH_5__) \
    || defined(__ARM_ARCH_5T__) \
    || defined(__MARM_ARMV5__)
#define WTF_ARM_ARCH_VERSION 5

#elif defined(__ARM_ARCH_5E__) \
    || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__)
#define WTF_ARM_ARCH_VERSION 5
/*ARMv5TE requires allocators to use aligned memory*/
#define WTF_USE_ARENA_ALLOC_ALIGNMENT_INTEGER 1

#elif defined(__ARM_ARCH_6__) \
    || defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6T2__) \
    || defined(__ARMV6__)
#define WTF_ARM_ARCH_VERSION 6

#elif defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7R__)
#define WTF_ARM_ARCH_VERSION 7

/* RVCT sets _TARGET_ARCH_ARM */
#elif defined(__TARGET_ARCH_ARM)
#define WTF_ARM_ARCH_VERSION __TARGET_ARCH_ARM

#if defined(__TARGET_ARCH_5E) \
    || defined(__TARGET_ARCH_5TE) \
    || defined(__TARGET_ARCH_5TEJ)
/*ARMv5TE requires allocators to use aligned memory*/
#define WTF_USE_ARENA_ALLOC_ALIGNMENT_INTEGER 1
#endif

#else
#define WTF_ARM_ARCH_VERSION 0

#endif

/* Set WTF_THUMB_ARCH_VERSION */
#if   defined(__ARM_ARCH_4T__)
#define WTF_THUMB_ARCH_VERSION 1

#elif defined(__ARM_ARCH_5T__) \
    || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__)
#define WTF_THUMB_ARCH_VERSION 2

#elif defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6M__)
#define WTF_THUMB_ARCH_VERSION 3

#elif defined(__ARM_ARCH_6T2__) \
    || defined(__ARM_ARCH_7__) \
    || defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7R__) \
    || defined(__ARM_ARCH_7M__)
#define WTF_THUMB_ARCH_VERSION 4

/* RVCT sets __TARGET_ARCH_THUMB */
#elif defined(__TARGET_ARCH_THUMB)
#define WTF_THUMB_ARCH_VERSION __TARGET_ARCH_THUMB

#else
#define WTF_THUMB_ARCH_VERSION 0
#endif


/* CPU(ARMV5_OR_LOWER) - ARM instruction set v5 or earlier */
/* On ARMv5 and below the natural alignment is required. 
   And there are some other differences for v5 or earlier. */
#if !defined(ARMV5_OR_LOWER) && !WTF_ARM_ARCH_AT_LEAST(6)
#define WTF_CPU_ARMV5_OR_LOWER 1
#endif


/* CPU(ARM_TRADITIONAL) - Thumb2 is not available, only traditional ARM (v4 or greater) */
/* CPU(ARM_THUMB2) - Thumb2 instruction set is available */
/* Only one of these will be defined. */
#if !defined(WTF_CPU_ARM_TRADITIONAL) && !defined(WTF_CPU_ARM_THUMB2)
#  if defined(thumb2) || defined(__thumb2__) \
    || ((defined(__thumb) || defined(__thumb__)) && WTF_THUMB_ARCH_VERSION == 4)
#    define WTF_CPU_ARM_TRADITIONAL 0
#    define WTF_CPU_ARM_THUMB2 1
#  elif WTF_ARM_ARCH_AT_LEAST(4)
#    define WTF_CPU_ARM_TRADITIONAL 1
#    define WTF_CPU_ARM_THUMB2 0
#  else
#    error "Not supported ARM architecture"
#  endif
#elif CPU(ARM_TRADITIONAL) && CPU(ARM_THUMB2) /* Sanity Check */
#  error "Cannot use both of WTF_CPU_ARM_TRADITIONAL and WTF_CPU_ARM_THUMB2 platforms"
#endif /* !defined(WTF_CPU_ARM_TRADITIONAL) && !defined(WTF_CPU_ARM_THUMB2) */

#if defined(__ARM_NEON__) && !defined(WTF_CPU_ARM_NEON)
#define WTF_CPU_ARM_NEON 1
#endif

#endif /* ARM */

#if CPU(ARM) || CPU(MIPS) || CPU(SH4) || CPU(SPARC)
#define WTF_CPU_NEEDS_ALIGNED_ACCESS 1
#endif


/* ENABLE macro defaults */

#if PLATFORM(WKC) && COMPILER(MSVC)
# define WTF_USE_QUERY_PERFORMANCE_COUNTER 1
#endif


#define ENABLE_DEBUG_WITH_BREAKPOINT 0
#define ENABLE_SAMPLING_COUNTERS 0
#define ENABLE_SAMPLING_FLAGS 0
#define ENABLE_OPCODE_SAMPLING 0
#define ENABLE_CODEBLOCK_SAMPLING 0
#if ENABLE(CODEBLOCK_SAMPLING) && !ENABLE(OPCODE_SAMPLING)
#error "CODEBLOCK_SAMPLING requires OPCODE_SAMPLING"
#endif
#if ENABLE(OPCODE_SAMPLING) || ENABLE(SAMPLING_FLAGS)
#define ENABLE_SAMPLING_THREAD 1
#endif


/* Configure the JIT */
#if CPU(ARM)
#if !defined(ENABLE_JIT_USE_SOFT_MODULO) && WTF_ARM_ARCH_AT_LEAST(5)
#define ENABLE_JIT_USE_SOFT_MODULO 1
#endif
#endif

#if COMPILER(GCC) && ENABLE(JIT) && (CPU(X86) || CPU(X86_64))
#define ENABLE_DFG_JIT 1
#endif

/* Profiling of types and values used by JIT code. DFG_JIT depends on it, but you
   can enable it manually with DFG turned off if you want to use it as a standalone
   profiler. In that case, you probably want to also enable VERBOSE_VALUE_PROFILE
   below. */
#if !defined(ENABLE_VALUE_PROFILER) && ENABLE(DFG_JIT)
#define ENABLE_VALUE_PROFILER 1
#endif

#if !defined(ENABLE_VERBOSE_VALUE_PROFILE) && ENABLE(VALUE_PROFILER)
#define ENABLE_VERBOSE_VALUE_PROFILE 0
#endif

#if !defined(ENABLE_SIMPLE_HEAP_PROFILING)
#define ENABLE_SIMPLE_HEAP_PROFILING 0
#endif

/* Configure the interpreter */
#if COMPILER(GCC) || (RVCT_VERSION_AT_LEAST(4, 0, 0, 0) && defined(__GNUC__))
#define HAVE_COMPUTED_GOTO 1
#endif
#if HAVE(COMPUTED_GOTO) && ENABLE(INTERPRETER)
#define ENABLE_COMPUTED_GOTO_INTERPRETER 1
#endif

/* Regular Expression Tracing - Set to 1 to trace RegExp's in jsc.  Results dumped at exit */
#define ENABLE_REGEXP_TRACING 0

/* Yet Another Regex Runtime - turned on by default for JIT enabled ports. */
#if PLATFORM(CHROMIUM)
#define ENABLE_YARR_JIT 0

#elif ENABLE(JIT) && !defined(ENABLE_YARR_JIT)
#define ENABLE_YARR_JIT 1

/* Setting this flag compares JIT results with interpreter results. */
#define ENABLE_YARR_JIT_DEBUG 0
#endif

#if ENABLE(JIT) || ENABLE(YARR_JIT)
#define ENABLE_ASSEMBLER 1
#endif
/* Setting this flag prevents the assembler from using RWX memory; this may improve
   security but currectly comes at a significant performance cost. */
#if PLATFORM(IOS) || (PLATFORM(WKC) && ENABLE(ASSEMBLER) && COMPILER(CCPPC)) // NOTE: Currently conflicts with Web Workers on our environment.
#define ENABLE_ASSEMBLER_WX_EXCLUSIVE 1
#endif

/* Pick which allocator to use; we only need an executable allocator if the assembler is compiled in.
   On x86-64 we use a single fixed mmap, on other platforms we mmap on demand. */
#if ENABLE(ASSEMBLER)
# if PLATFORM(WKC)
#  ifdef WKC_USE_FIXED_POOL_EXECUTABLE_ALLOCATOR
#   define ENABLE_EXECUTABLE_ALLOCATOR_FIXED 1
#  else
#   define ENABLE_EXECUTABLE_ALLOCATOR_DEMAND 1
#  endif
# else // PLATFORM(WKC)
#if CPU(X86_64)
#define ENABLE_EXECUTABLE_ALLOCATOR_FIXED 1
#else
#define ENABLE_EXECUTABLE_ALLOCATOR_DEMAND 1
#endif
# endif // PLATFORM(WKC)
#endif

#if CPU(X86_64)
#define WTF_USE_JSVALUE64 1
#else
#define WTF_USE_JSVALUE32_64 1
#endif

#ifndef ENABLE_LARGE_HEAP
#if CPU(X86) || CPU(X86_64)
#define ENABLE_LARGE_HEAP 1
#else
#define ENABLE_LARGE_HEAP 0
#endif
#endif

#ifndef JSC_HOST_CALL
# if CPU(X86) && COMPILER(MSVC)
#  define JSC_HOST_CALL __fastcall
# elif CPU(X86) && COMPILER(GCC)
#  define JSC_HOST_CALL __attribute__ ((fastcall))
# else
#  define JSC_HOST_CALL
# endif
#endif

#if COMPILER(GCC)
#define WARN_UNUSED_RETURN __attribute__ ((warn_unused_result))
#else
#define WARN_UNUSED_RETURN
#endif

#if !ENABLE(NETSCAPE_PLUGIN_API) || (ENABLE(NETSCAPE_PLUGIN_API) && ((OS(UNIX) && (PLATFORM(QT) || PLATFORM(WX))) || PLATFORM(GTK)))
#define ENABLE_PLUGIN_PACKAGE_SIMPLE_HASH 1
#endif

/* Set up a define for a common error that is intended to cause a build error -- thus the space after Error. */
#define WTF_PLATFORM_CFNETWORK Error USE_macro_should_be_used_with_CFNETWORK

/* FIXME: Eventually we should enable this for all platforms and get rid of the define. */
#if PLATFORM(MAC) || PLATFORM(WIN) || PLATFORM(QT)
#define WTF_USE_PLATFORM_STRATEGIES 1
#endif

#if PLATFORM(WIN)
#define WTF_USE_CROSS_PLATFORM_CONTEXT_MENUS 1
#endif

/* Geolocation request policy. pre-emptive policy is to acquire user permission before acquiring location.
   Client based implementations will have option to choose between pre-emptive and nonpre-emptive permission policy.
   pre-emptive permission policy is enabled by default for all client-based implementations. */
#if ENABLE(CLIENT_BASED_GEOLOCATION)
#define WTF_USE_PREEMPT_GEOLOCATION_PERMISSION 1
#endif

#if CPU(ARM_THUMB2)
#define ENABLE_BRANCH_COMPACTION 1
#endif

#if !defined(ENABLE_THREADING_LIBDISPATCH) && HAVE(DISPATCH_H)
#define ENABLE_THREADING_LIBDISPATCH 1
#elif !defined(ENABLE_THREADING_OPENMP) && defined(_OPENMP)
#define ENABLE_THREADING_OPENMP 1
#elif !defined(THREADING_GENERIC)
#define ENABLE_THREADING_GENERIC 1
#endif

/* FIXME: This define won't be needed once #27551 is fully landed. However, 
   since most ports try to support sub-project independence, adding new headers
   to WTF causes many ports to break, and so this way we can address the build
   breakages one port at a time. */
#define WTF_USE_EXPORT_MACROS 0

#ifndef NDEBUG
#ifndef ENABLE_GC_VALIDATION
#define ENABLE_GC_VALIDATION 1
#endif
#endif

#define ENABLE_LAZY_BLOCK_FREEING 0

// static configurations for WKC

#define WTF_USE_TLSF 1
#define WTF_USE_PTHREADS 0
#define WTF_USE_JSC 1
#define WTF_USE_V8 0

#define WTF_USE_WKC_UNICODE 1

#define HAVE_PTHREAD_NP_H 0
#undef WTF_FEATURE_WIN_OS
#undef WTF_PLATFORM_WIN_OS
#undef WTF_PLATFORM_WIN
#undef WTF_USE_WININET
#undef HAVE_STRINGS_H
#undef HAVE_SYS_TIME_H
#undef HAVE_MMAP
#undef HAVE_VIRTUALALLOC
#undef HAVE_LANGINFO_H
#undef HAVE_SYS_PARAM_H
#undef ENABLE_WREC
#undef WTF_PLATFORM_CAIRO
#undef USE_SYSTEM_MALLOC

//#define ENABLE_IMAGE_DECODER_DOWN_SAMPLING 1

#define WTF_USE_CURL 1

#ifdef USE_EXCEPTION
# define ENABLE_WKC_HIDE_SETJMP 1
#endif

// pagesave
#define ENABLE_WKC_PAGESAVE_MHTML 1
// enable to change column-fit layout and small-screen-rendering layout.
#define ENABLE_WKC_ANDROID_LAYOUT 1
// Ensure that the fixed elements are always relative to the top document.
#define ENABLE_WKC_ANDROID_FIXED_ELEMENTS 0
// enable to notify scroll position even if scroll position is not changed
#define ENABLE_WKC_FORCE_NOTIFY_SCROLL 1
// enable shrink decode
#define ENABLE_WKC_IMAGE_DECODER_DOWN_SAMPLING 1

// enable optimization to ignore fixed background images when scrolling a page.
#define ENABLE_FAST_MOBILE_SCROLLING 1

#undef ENABLE_REPLACEMENT_SYSTEMMEMORY

#ifndef DISABLE_WKC_REPLACE_NEWDELETE_WITH_FASTMALLOC
# define ENABLE_WKC_REPLACE_NEWDELETE_WITH_FASTMALLOC 1
#endif

// enable HTTPCache
//#define ENABLE_WKC_HTTPCACHE 1

#define JS_EXPORTDATA
#define __N p__N

// typedef of size_t
#ifndef __BUILDING_IN_VS__
# if CPU(X86_64)
#  include <stddef.h>
# elif COMPILER(CCPPC)
// TODO: Move this typedef to sys/types.h.
typedef unsigned int size_t;
# else
#  include <sys/types.h>
# endif
#endif

typedef void* LPVOID;
#define _CRT_TERMINATE_DEFINED
#define MAX_PATH            260
#if !defined(_TRUNCATE)
#define _TRUNCATE ((size_t)-1)
#endif

#define WEBCORE_NAVIGATOR_PLATFORM wkcSystemGetNavigatorPlatformPeer()
#define WEBCORE_NAVIGATOR_PRODUCT wkcSystemGetNavigatorProductPeer()
#define WEBCORE_NAVIGATOR_PRODUCT_SUB wkcSystemGetNavigatorProductSubPeer()
#define WEBCORE_NAVIGATOR_VENDOR wkcSystemGetNavigatorVendorPeer()
#define WEBCORE_NAVIGATOR_VENDOR_SUB wkcSystemGetNavigatorVendorSubPeer()


#define CRASH() do { \
    wkcMemoryNotifyCrashPeer(); \
} while(false);

#endif // _WKCPLATFORM_H_
