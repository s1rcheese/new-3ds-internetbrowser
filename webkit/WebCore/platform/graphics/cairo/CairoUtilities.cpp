/*
 * Copyright (C) 2010 Igalia S.L.
 * Copyright (C) 2011 ProFUSION embedded systems
 * Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
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

#if USE(CAIRO) || USE(WKC_CAIRO)

#include "CairoUtilities.h"

#include "AffineTransform.h"
#include "Color.h"
#include "FloatPoint.h"
#include "FloatRect.h"
#include "IntRect.h"
#include "OwnPtrCairo.h"
#include "Path.h"
#include "PlatformPathCairo.h"
#include "RefPtrCairo.h"
#include <wtf/Vector.h>

#if USE(WKC_CAIRO)
#include <wkc/wkcgpeer.h>
#endif

namespace WebCore {

void copyContextProperties(cairo_t* srcCr, cairo_t* dstCr)
{
    cairo_set_antialias(dstCr, cairo_get_antialias(srcCr));

    size_t dashCount = cairo_get_dash_count(srcCr);
    Vector<double> dashes(dashCount);

    double offset;
    cairo_get_dash(srcCr, dashes.data(), &offset);
    cairo_set_dash(dstCr, dashes.data(), dashCount, offset);
    cairo_set_line_cap(dstCr, cairo_get_line_cap(srcCr));
    cairo_set_line_join(dstCr, cairo_get_line_join(srcCr));
    cairo_set_line_width(dstCr, cairo_get_line_width(srcCr));
    cairo_set_miter_limit(dstCr, cairo_get_miter_limit(srcCr));
    cairo_set_fill_rule(dstCr, cairo_get_fill_rule(srcCr));
}

void setSourceRGBAFromColor(cairo_t* context, const Color& color)
{
    float red, green, blue, alpha;
    color.getRGBA(red, green, blue, alpha);
    cairo_set_source_rgba(context, red, green, blue, alpha);
}

void appendPathToCairoContext(cairo_t* to, cairo_t* from)
{
#if PLATFORM(WKC)
    cairo_path_t* frompath = cairo_copy_path(from);
    if (frompath->status==CAIRO_STATUS_NO_MEMORY) {
        return;
    }
    cairo_append_path(to, frompath);
    cairo_path_destroy(frompath);
#else
    OwnPtr<cairo_path_t> cairoPath = adoptPtr(cairo_copy_path(from));
    cairo_append_path(to, cairoPath.get());
#endif
}

void setPathOnCairoContext(cairo_t* to, cairo_t* from)
{
    cairo_new_path(to);
    appendPathToCairoContext(to, from);
}

void appendWebCorePathToCairoContext(cairo_t* context, const Path& path)
{
    appendPathToCairoContext(context, path.platformPath()->context());
}

#if (CAIRO_VERSION >= 11000)
void appendRegionToCairoContext(cairo_t* to, const cairo_region_t* region)
{
    if (!region)
        return;

    const int rectCount = cairo_region_num_rectangles(region);
    for (int i = 0; i < rectCount; ++i) {
        cairo_rectangle_int_t rect;
        cairo_region_get_rectangle(region, i, &rect);
        cairo_rectangle(to, rect.x, rect.y, rect.width, rect.height);
    }
}
#endif

cairo_operator_t toCairoOperator(CompositeOperator op)
{
    switch (op) {
    case CompositeClear:
        return CAIRO_OPERATOR_CLEAR;
    case CompositeCopy:
        return CAIRO_OPERATOR_SOURCE;
    case CompositeSourceOver:
        return CAIRO_OPERATOR_OVER;
    case CompositeSourceIn:
        return CAIRO_OPERATOR_IN;
    case CompositeSourceOut:
        return CAIRO_OPERATOR_OUT;
    case CompositeSourceAtop:
        return CAIRO_OPERATOR_ATOP;
    case CompositeDestinationOver:
        return CAIRO_OPERATOR_DEST_OVER;
    case CompositeDestinationIn:
        return CAIRO_OPERATOR_DEST_IN;
    case CompositeDestinationOut:
        return CAIRO_OPERATOR_DEST_OUT;
    case CompositeDestinationAtop:
        return CAIRO_OPERATOR_DEST_ATOP;
    case CompositeXOR:
        return CAIRO_OPERATOR_XOR;
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1,10,0)
    case CompositePlusDarker:
        return CAIRO_OPERATOR_DARKEN;
#endif
    case CompositePlusLighter:
        return CAIRO_OPERATOR_ADD;
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1,10,0)
    case CompositeDifference:
        return CAIRO_OPERATOR_DIFFERENCE;
#endif
    default:
        return CAIRO_OPERATOR_SOURCE;
    }
}

void drawPatternToCairoContext(cairo_t* cr, cairo_surface_t* image, const IntSize& imageSize, const FloatRect& tileRect,
                               const AffineTransform& patternTransform, const FloatPoint& phase, cairo_operator_t op, const FloatRect& destRect)
{
    // Avoid NaN
    if (!isfinite(phase.x()) || !isfinite(phase.y()))
       return;

    cairo_save(cr);

    RefPtr<cairo_surface_t> clippedImageSurface = 0;
    if (tileRect.size() != imageSize) {
        IntRect imageRect = enclosingIntRect(tileRect);
#if USE(WKC_CAIRO)
        cairo_surface_t* nsurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageRect.width(), imageRect.height());
        if (cairo_surface_status(nsurface)!=CAIRO_STATUS_SUCCESS) {
            cairo_restore(cr);
            return;
        }
        clippedImageSurface = adoptRef(nsurface);
        cairo_t* ncr = cairo_create(clippedImageSurface.get());
        if (cairo_status(ncr)!=CAIRO_STATUS_SUCCESS) {
            cairo_restore(cr);
            return;
        }
        RefPtr<cairo_t> clippedImageContext = adoptRef(ncr);
        WKC_CAIRO_ADD_OBJECT(clippedImageSurface.get(), cairo_surface);
        WKC_CAIRO_ADD_OBJECT(clippedImageContext.get(), cairo);
#else
        clippedImageSurface = adoptRef(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageRect.width(), imageRect.height()));
        RefPtr<cairo_t> clippedImageContext = adoptRef(cairo_create(clippedImageSurface.get()));
#endif
        cairo_set_source_surface(clippedImageContext.get(), image, -tileRect.x(), -tileRect.y());
        cairo_paint(clippedImageContext.get());
        image = clippedImageSurface.get();
    }

    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(image);
#if USE(WKC_CAIRO)
    if (cairo_pattern_status(pattern)!=CAIRO_STATUS_SUCCESS) {
        cairo_restore(cr);
        return;
    }
    WKC_CAIRO_ADD_OBJECT(pattern, cairo_pattern);
# if USE(WKC_CAIRO_FILTER_NEAREST)
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);
# endif
#endif
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    cairo_matrix_t patternMatrix = cairo_matrix_t(patternTransform);
    cairo_matrix_t phaseMatrix = {1, 0, 0, 1, phase.x() + tileRect.x() * patternTransform.a(), phase.y() + tileRect.y() * patternTransform.d()};
    cairo_matrix_t combined;
    cairo_matrix_multiply(&combined, &patternMatrix, &phaseMatrix);
    cairo_matrix_invert(&combined);
    cairo_pattern_set_matrix(pattern, &combined);

    cairo_set_operator(cr, op);
    cairo_set_source(cr, pattern);
#if USE(WKC_CAIRO)
    WKC_CAIRO_REMOVE_OBJECT(pattern);
#endif
    cairo_pattern_destroy(pattern);
    cairo_rectangle(cr, destRect.x(), destRect.y(), destRect.width(), destRect.height());
    cairo_fill(cr);

    cairo_restore(cr);
}

PassRefPtr<cairo_surface_t> copyCairoImageSurface(cairo_surface_t* originalSurface)
{
    // Cairo doesn't provide a way to copy a cairo_surface_t.
    // See http://lists.cairographics.org/archives/cairo/2007-June/010877.html
    // Once cairo provides the way, use the function instead of this.
#if USE(WKC_CAIRO)
    cairo_surface_t* nsurface = cairo_image_surface_create(cairo_image_surface_get_format(originalSurface), 
                                                           cairo_image_surface_get_width(originalSurface),
                                                           cairo_image_surface_get_height(originalSurface));
    if (cairo_surface_status(nsurface)!=CAIRO_STATUS_SUCCESS)
        return 0;
    cairo_t* ncr = cairo_create(nsurface);
    if (cairo_status(ncr)!=CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(nsurface);
        return 0;
    }
    WKC_CAIRO_ADD_OBJECT(nsurface, cairo_surface);
    WKC_CAIRO_ADD_OBJECT(ncr, cairo);
    cairo_set_source_surface(ncr, originalSurface, 0, 0);
    cairo_set_operator(ncr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(ncr);

    WKC_CAIRO_REMOVE_OBJECT(ncr);
    cairo_destroy(ncr);

    RefPtr<cairo_surface_t> newSurface = adoptRef(nsurface);
    return newSurface.release();
#else
    RefPtr<cairo_surface_t> newSurface = adoptRef(cairo_image_surface_create(cairo_image_surface_get_format(originalSurface), 
                                                                             cairo_image_surface_get_width(originalSurface),
                                                                             cairo_image_surface_get_height(originalSurface)));

    RefPtr<cairo_t> cr = adoptRef(cairo_create(newSurface.get()));
    cairo_set_source_surface(cr.get(), originalSurface, 0, 0);
    cairo_set_operator(cr.get(), CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr.get());
    return newSurface.release();
#endif
}

void copyRectFromCairoSurfaceToContext(cairo_surface_t* from, cairo_t* to, const IntSize& offset, const IntRect& rect)
{
    cairo_set_source_surface(to, from, offset.width(), offset.height());
    cairo_rectangle(to, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_fill(to);
}

void copyRectFromOneSurfaceToAnother(cairo_surface_t* from, cairo_surface_t* to, const IntSize& offset, const IntRect& rect)
{
    RefPtr<cairo_t> context = adoptRef(cairo_create(to));
#if USE(WKC_CAIRO)
    WKC_CAIRO_ADD_OBJECT(to, cairo_surface);
    WKC_CAIRO_ADD_OBJECT(context.get(), cairo);
#endif
    copyRectFromCairoSurfaceToContext(from, context.get(), offset, rect);
}

} // namespace WebCore

#endif // USE(CAIRO) || USE(WKC_CAIRO)
