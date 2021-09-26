/*
 *  config.h for libcairo
 *
 *  Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
 */

#define HAVE_INTTYPES_H 1
#define HAVE_SETJMP_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_UINT64_T 1

#define PACKAGE_BUGREPORT "http://bugs.freedesktop.org/enter_bug.cgi?product=cairo"
#define PACKAGE_NAME USE_cairo_INSTEAD
#define PACKAGE_STRING USE_cairo_version_OR_cairo_version_string_INSTEAD
#define PACKAGE_TARNAME USE_cairo_INSTEAD
#define PACKAGE_URL ""
#define PACKAGE_VERSION USE_cairo_version_OR_cairo_version_string_INSTEAD

#define STDC_HEADERS 1

#ifdef __ghs__
#define WORDS_BIGENDIAN 1
#define FLOAT_WORDS_BIGENDIAN 1
#endif


#define WKC_CAIRO_CUSTOMIZE 1
#include "wkccairorename.h"

#include <wkc/wkcclib.h>
#define malloc  wkc_malloc
#define realloc wkc_realloc
#define calloc  wkc_calloc
#define free    wkc_free
#define strdup  wkc_strdup
