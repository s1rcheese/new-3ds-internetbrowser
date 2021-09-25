/*
 *  Copyright (C) 2010 Igalia S.L.
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "RefPtrCairo.h"

#include <cairo.h>

#if USE(FREETYPE)
#include <cairo-ft.h>
#include <fontconfig/fcfreetype.h>
#endif

#if USE(WKC_CAIRO)
#include <wkc/wkcgpeer.h>
#endif

namespace WTF {
#if USE(WKC_CAIRO)
template<> void refIfNotNull(cairo_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        cairo_reference(ptr);
        WKC_CAIRO_ADD_OBJECT(ptr, cairo);
    }
}

template<> void derefIfNotNull(cairo_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        WKC_CAIRO_REMOVE_OBJECT(ptr);
        cairo_destroy(ptr);
    }
}

template<> void refIfNotNull(cairo_surface_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        cairo_surface_reference(ptr);
        WKC_CAIRO_ADD_OBJECT(ptr, cairo_surface);
    }
}

template<> void derefIfNotNull(cairo_surface_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        WKC_CAIRO_REMOVE_OBJECT(ptr);
        cairo_surface_destroy(ptr);
    }
}

template<> void refIfNotNull(cairo_font_face_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        cairo_font_face_reference(ptr);
        WKC_CAIRO_ADD_OBJECT(ptr, cairo_font_face);
    }
}

template<> void derefIfNotNull(cairo_font_face_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        WKC_CAIRO_REMOVE_OBJECT(ptr);
        cairo_font_face_destroy(ptr);
    }
}

template<> void refIfNotNull(cairo_scaled_font_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        cairo_scaled_font_reference(ptr);
        WKC_CAIRO_ADD_OBJECT(ptr, cairo_scaled_font);
    }
}

template<> void derefIfNotNull(cairo_scaled_font_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        WKC_CAIRO_REMOVE_OBJECT(ptr);
        cairo_scaled_font_destroy(ptr);
    }
}

template<> void refIfNotNull(cairo_pattern_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        cairo_pattern_reference(ptr);
        WKC_CAIRO_ADD_OBJECT(ptr, cairo_pattern);
    }
}

template<> void derefIfNotNull(cairo_pattern_t* ptr)
{
    if (LIKELY(ptr != 0)) {
        WKC_CAIRO_REMOVE_OBJECT(ptr);
        cairo_pattern_destroy(ptr);
    }
}
#else /* USE(WKC_CAIRO) */
template<> void refIfNotNull(cairo_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_reference(ptr);
}

template<> void derefIfNotNull(cairo_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_destroy(ptr);
}

template<> void refIfNotNull(cairo_surface_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_surface_reference(ptr);
}

template<> void derefIfNotNull(cairo_surface_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_surface_destroy(ptr);
}

template<> void refIfNotNull(cairo_font_face_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_font_face_reference(ptr);
}

template<> void derefIfNotNull(cairo_font_face_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_font_face_destroy(ptr);
}

template<> void refIfNotNull(cairo_scaled_font_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_scaled_font_reference(ptr);
}

template<> void derefIfNotNull(cairo_scaled_font_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_scaled_font_destroy(ptr);
}

template<> void refIfNotNull(cairo_pattern_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_pattern_reference(ptr);
}

template<> void derefIfNotNull(cairo_pattern_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_pattern_destroy(ptr);
}

template<> void refIfNotNull(cairo_region_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_region_reference(ptr);
}

template<> void derefIfNotNull(cairo_region_t* ptr)
{
    if (LIKELY(ptr != 0))
        cairo_region_destroy(ptr);
}
#endif /* USE(WKC_CAIRO) */

#if USE(FREETYPE)
template<> void refIfNotNull(FcPattern* ptr)
{
    if (LIKELY(ptr != 0))
        FcPatternReference(ptr);
}

template<> void derefIfNotNull(FcPattern* ptr)
{
    if (LIKELY(ptr != 0))
        FcPatternDestroy(ptr);
}

#endif

}
