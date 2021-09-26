/*
 * Copyright (C) 2007 Kevin Ollivier.  All rights reserved.
 * Copyright (c) 2010-2015 ACCESS CO., LTD. All rights reserved.
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
#include "Font.h"

#include "FontFallbackList.h"
#include "FontPlatformDataWKC.h"
#include "GlyphBuffer.h"
#include "GraphicsContext.h"
#if USE(WKC_CAIRO)
#include "CairoUtilities.h"
#include "PlatformContextCairo.h"
#else /* USE(WKC_CAIRO) */
#include "ShadowBlur.h"
#include "ImageBuffer.h"
#endif /* USE(WKC_CAIRO) */
#include "IntRect.h"
#include "SimpleFontData.h"
#include "TextRun.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

static const int cGlyphBufMaxLen = 1024;

static const UChar cU3099[] = {
0x304b,0x304c, 0x304d,0x304e, 0x304f,0x3050, 0x3051,0x3052, 0x3053,0x3054, // ka-ko
0x3055,0x3056, 0x3057,0x3058, 0x3059,0x305a, 0x305b,0x305c, 0x305d,0x305e, // sa-so
0x305f,0x3060, 0x3061,0x3062, 0x3064,0x3065, 0x3066,0x3067, 0x3068,0x3069, // ta-to
0x306f,0x3070, 0x3072,0x3073, 0x3075,0x3076, 0x3078,0x3079, 0x307b,0x307c, // ha-ho
0x30ab,0x30ac, 0x30ad,0x30ae, 0x30af,0x30b0, 0x30b1,0x30b2, 0x30b3,0x30b4, // ka-ko
0x30b5,0x30b6, 0x30b7,0x30b8, 0x30b9,0x30ba, 0x30bb,0x30bc, 0x30bd,0x30be, // sa-so
0x30bf,0x30c0, 0x30c1,0x30c2, 0x30c4,0x30c5, 0x30c6,0x30c7, 0x30c8,0x30c9, // ta-to
0x30cf,0x30d0, 0x30d2,0x30d3, 0x30d5,0x30d6, 0x30d8,0x30d9, 0x30db,0x30dc, // ha-ho
0x3046,0x3094, 0x309d,0x309e, 0x30a6,0x30f4, 0x30ef,0x30f7, 0x30f0,0x30f8, 0x30f1,0x30f9, 0x30f2,0x30fa, 0x30fd, 0x30fe, // specials
0
};

static const UChar cU309a[] = {
0x306f,0x3071, 0x3072,0x3074, 0x3075,0x3077, 0x3078,0x307a, 0x307b,0x307d, // ha-ho
0x30cf,0x30d1, 0x30d2,0x30d4, 0x30d5,0x30d7, 0x30d8,0x30da, 0x30db,0x30dd, // ha-ho
0
};

static bool
combineCharsInText(const UChar*& inout_str, int& inout_len)
{
    if (inout_len==0)
        return false;

    static const int cSupportsMn = wkcFontEngineSupportsNonSpacingMarksPeer();
    if (cSupportsMn)
        return false;

    int i=0;
    for (i=1; i<inout_len; i++) {
        if (wkcUnicodeCategoryPeer(inout_str[i])==WKC_UNICODE_CATEGORY_MARKNONSPACING)
            break;
    }
    if (i==inout_len)
        return false;

    WKC_DEFINE_STATIC_PTR(UChar*, gGlyphBuf, new UChar[cGlyphBufMaxLen]);
    UChar* buf = gGlyphBuf;
    bool needdelete = false;
    if (inout_len>=cGlyphBufMaxLen-1) {
        buf = new UChar[inout_len+1];
        needdelete = true;
    }

    int len = 0;
    for (i=0; i<inout_len; i++) {
        const UChar c = inout_str[i];
        buf[len++] = c;
        if (wkcUnicodeCategoryPeer(c)!=WKC_UNICODE_CATEGORY_MARKNONSPACING || len==1)
            continue;

        const UChar* tbl = 0;
        if (c==0x3099)
            tbl = cU3099;
        else if (c==0x309a)
            tbl = cU309a;
        else
            continue;
        const UChar b = buf[len-2];
        for (int k=0; tbl[k]; k++) {
            if (tbl[k]!=b) continue;
            len--;
            buf[len-1] = tbl[k+1];
            break;
        }
    }

    inout_str = buf;
    inout_len = len;
    return needdelete;
}

int
Font::getTextWidth(void* in_font, int in_flags, const unsigned short* in_str, int in_len, int* out_clip_width)
{
    bool needdelete = combineCharsInText(in_str, in_len);
#if defined(__ARMCC_VERSION)
    float clip_width;
    int ret = wkcFontGetTextWidthPeer(in_font, in_flags, in_str, in_len, &clip_width);
    if (out_clip_width) {
        *out_clip_width = (int)clip_width;
    }
#else
    int ret = wkcFontGetTextWidthPeer(in_font, in_flags, in_str, in_len, out_clip_width);
#endif
    if (needdelete)
        delete [] in_str;
    return ret;
}

static void
drawPlatformText(void* in_context, const unsigned short* in_str, int in_len, const WKCFloatRect* in_textbox, const WKCFloatRect* in_clip, void* in_font, int in_flag)
{
    bool needdelete = combineCharsInText(in_str, in_len);
    wkcDrawContextDrawTextPeer(in_context, in_str, in_len, in_textbox, in_clip, (WKCPeerFont *)in_font, in_flag);
    if (needdelete)
        delete [] in_str;
}

#if USE(WKC_CAIRO)
static void
drawTextShadow(GraphicsContext* graphicscontext, void* in_platformcontext, const unsigned short* in_str, int in_len, const WKCFloatRect* in_textbox, const WKCFloatRect* in_clip, WKCPeerFont* in_font, int in_flag)
{
    ShadowBlur& shadow = graphicscontext->platformContext()->shadowBlur();
    if (!(graphicscontext->textDrawingMode() & TextModeFill) || shadow.type() == ShadowBlur::NoShadow)
        return;

    WKCFloatRect textbox(*in_textbox);
    WKCFloatRect clip(*in_clip);
    FloatSize shadowOffset(graphicscontext->state().shadowOffset);
    textbox.fX += shadowOffset.width();
    textbox.fY += shadowOffset.height();
    clip.fX += shadowOffset.width();
    clip.fY += shadowOffset.height();

    if (!shadow.mustUseShadowBlur(graphicscontext)) {
        // Optimize non-blurry shadows, by just drawing text without the ContextShadow.
        graphicscontext->save();
        graphicscontext->setFillColor(graphicscontext->state().shadowColor, ColorSpaceDeviceRGB);
        drawPlatformText(in_platformcontext, in_str, in_len, &textbox, &clip, in_font, in_flag);
        graphicscontext->restore();
        return;
    }

    FloatRect fontRect(*in_textbox);
//    fontRect.inflate(graphicscontext->state().blurDistance);
    GraphicsContext* shadowContext = shadow.beginShadowLayer(graphicscontext, fontRect);
    if (shadowContext) {
        PlatformContextCairo* pf = new PlatformContextCairo(shadowContext->platformContext()->cr());
        wkcDrawContextSaveStatePeer(pf);
        wkcDrawContextSetFillColorPeer(pf, 0xff000000);
        drawPlatformText(pf, in_str, in_len, in_textbox, in_clip, in_font, in_flag);
        wkcDrawContextRestoreStatePeer(pf);
        shadow.endShadowLayer(graphicscontext);
        delete pf;
    }
}
#else /* USE(WKC_CAIRO) */
static void
drawTextShadow(GraphicsContext* graphicscontext, void* in_platformcontext, const unsigned short* in_str, int in_len, const WKCFloatRect* in_textbox, const WKCFloatRect* in_clip, WKCPeerFont* in_font, int in_flag)
{
    FloatSize offset;
    float blur;
    Color color;
    ColorSpace colorspace;
    if (!graphicscontext->getShadow(offset, blur, color, colorspace))
        return;

    int type = wkcDrawContextGetOffscreenTypePeer(in_platformcontext);

    if (!blur || type==WKC_OFFSCREEN_TYPE_POLYGON) {
        // Optimize non-blurry shadows, by just drawing text without the ContextShadow.
        WKCFloatRect textbox(*in_textbox);
        WKCFloatRect clip(*in_clip);
        textbox.fX += offset.width();
        textbox.fY += offset.height();
        clip.fX += offset.width();
        clip.fY += offset.height();

        graphicscontext->save();
        graphicscontext->setFillColor(color, colorspace);
        drawPlatformText(in_platformcontext, in_str, in_len, &textbox, &clip, in_font, in_flag);
        graphicscontext->restore();
        return;
    }

    ShadowBlur shadow(FloatSize(blur, blur), offset, color, colorspace);

    const IntSize bufferSize(in_textbox->fWidth+blur*2, in_textbox->fHeight+blur*2);
    const FloatRect bufferRect(FloatPoint(), bufferSize);
    OwnPtr<ImageBuffer> shadowBuffer = ImageBuffer::create(bufferSize);
    if (!shadowBuffer) return;
    GraphicsContext *shadowContext = shadowBuffer->context();

    shadowContext->clearRect(bufferRect);
    shadowContext->setFillColor(Color::black, ColorSpaceDeviceRGB);
    const WKCFloatRect textbox = {blur, blur, in_textbox->fWidth, in_textbox->fHeight};
    const WKCFloatRect tcr = { in_clip->fX-in_textbox->fX, in_clip->fY-in_textbox->fY, in_clip->fWidth, in_clip->fHeight };
    void* buf_dc = (void *)shadowContext->platformContext();
    drawPlatformText(buf_dc, in_str, in_len, &textbox, &tcr, in_font, in_flag);

    IntRect br(bufferRect);
    RefPtr<Uint8ClampedArray> layerData = shadowBuffer->getUnmultipliedImageData(br);
    if (!layerData) return;
    shadow.blurLayerImage(layerData->data(), bufferSize, bufferSize.width() * 4);
    shadowBuffer->putByteArray(Unmultiplied, layerData.get(), bufferSize, br, IntPoint());

    shadowContext->setCompositeOperation(CompositeSourceIn);
    shadowContext->setFillColor(color, ColorSpaceDeviceRGB);
    shadowContext->fillRect(bufferRect);

    graphicscontext->drawImageBuffer(shadowBuffer.get(), ColorSpaceDeviceRGB, IntPoint(in_textbox->fX+offset.width()-blur, in_textbox->fY+offset.height()-blur));
}
#endif /* USE(WKC_CAIRO) */

inline static void
drawTextWithShadow(GraphicsContext* graphicscontext, void* in_platformcontext, const unsigned short* in_str, int in_len, const WKCFloatRect* in_textbox, const WKCFloatRect* in_clip, WKCPeerFont* in_font, int in_flag)
{
    if (!graphicscontext || !in_platformcontext) return;
    TextDrawingModeFlags mode = graphicscontext->textDrawingMode();
    if (mode==TextModeInvisible)
        return;
    graphicscontext->setTextDrawingMode(TextModeFill);
    drawTextShadow(graphicscontext, in_platformcontext, in_str, in_len, in_textbox, in_clip, in_font, in_flag);
    graphicscontext->setTextDrawingMode(mode);
    drawPlatformText(in_platformcontext, in_str, in_len, in_textbox, in_clip, in_font, in_flag);
}

static UChar*
fixedGlyphs(int sch, const UChar* str, int& len, bool& needdelete)
{
    WKC_DEFINE_STATIC_PTR(UChar*, gGlyphBuf, new UChar[cGlyphBufMaxLen]);

    UChar* buf = gGlyphBuf;
    needdelete = false;
    int j=0;

    if (sch==0) {
        // BMP
        if (len>=cGlyphBufMaxLen-1) {
            buf = new UChar[len+1];
            needdelete = true;
        }
        for (int i=0; i<len; i++) {
            const UChar c = str[i];
            if (Font::treatAsSpace(c)) {
                buf[j++] = 0x20;
            } else if ((c==0x200b || Font::treatAsZeroWidthSpace(c) || c==0xfeff)) {
                buf[j++] = 0x200b;
            } else {
                buf[j++] = c;
            }
        }
    } else {
        if (U_IS_BMP(sch)) {
            if (U_IS_SURROGATE(sch))
                sch = 0xfffd;
            for (int i=0; i<len; i++) {
                buf[j++] = sch;
            }
        } else {
            for (int i=0; i<len; i++) {
                buf[j++] = U16_LEAD(sch);
                buf[j++] = U16_TRAIL(sch);
            }
        }
    }


    buf[j] = 0;
    len = j;
    return buf;
}

bool Font::canReturnFallbackFontsForComplexText()
{
    return false;
}

void Font::drawGlyphs(GraphicsContext* graphicsContext, const SimpleFontData* sfont, const GlyphBuffer& glyphBuffer, 
                      int from, int numGlyphs, const FloatPoint& point) const
{
    FloatPoint pos(point);
    void* dc = (void *)graphicsContext->platformContext();
    WKCPeerFont pf = {0};

    if (!dc) return;

    const FontPlatformData& pd(sfont->platformData());
    WKC::WKCFontInfo* info = pd.font();
    if (!info)
        return;
    pf.fFont = info->font();
    if (!pf.fFont) return;

    const float scale = info->scale();

    pf.fRequestedSize = info->requestSize();
    pf.fCreatedSize = info->createdSize();
    pf.fWeight = info->weight();
    pf.fItalic = info->isItalic();
    pf.fScale = info->scale();
    pf.fiScale = info->iscale();
    pf.fCanScale = info->canScale();
    pf.fFontId = (void *)static_cast<uintptr_t>(pd.hash());

    pos.setY(pos.y() - (float)info->ascent()*scale);
    float h = (info->lineSpacing()) * scale;

    const GlyphBufferGlyph* str = glyphBuffer.glyphs(from);
    bool needdelete = false;
    int len = numGlyphs;
    UChar* buf = fixedGlyphs(info->specificUnicodeChar(), str, len, needdelete);

    /*
     * the following code is a copy from GraphicsContextWKC.cpp::GraphicsContext::fillPath().
     */
    WKCPeerPattern* pt = 0;
    Pattern* pattern = graphicsContext->fillPattern();
    Gradient* gradient = graphicsContext->fillGradient();
    AffineTransform affine;
    if (pattern) {
        pt = (WKCPeerPattern *)pattern->createPlatformPattern(affine);
        if (NULL == pt) {
            if (needdelete) {
                delete [] buf;
            }
            return;
        }
    } else if (gradient) {
        pt = (WKCPeerPattern *)gradient->platformGradient();
        if (NULL == pt) {
            if (needdelete) {
                delete [] buf;
            }
            return;
        }
    } else {
        /*
         * ? why ? even if the alpha value is zero, we have to draw (e.g. SRCCOPY).
         */
        Color c = graphicsContext->fillColor();
        Color sc = graphicsContext->strokeColor();
        if (0 == c.alpha() && 0==sc.alpha()) {
            if (needdelete) {
                delete [] buf;
            }
            return;
        }
    }
    wkcDrawContextSetPatternPeer(dc, pt);

    const unsigned short* tstart = buf;
    int tlen = 0;
    float tofs = 0;
    float aw = 0;
    float w = 0;
    int cw = 0;
    for (int i=0; i<len; i++) {
        tlen++;
        if (U16_IS_LEAD(tstart[tlen-1]))
            continue;

        w += sfont->widthForGlyph(tstart[tlen-1]);
        const FloatRect br = sfont->boundsForGlyph(tstart[tlen-1]);
        cw += br.width();

        if (i<len-1 && from+i<glyphBuffer.size())
            aw += glyphBuffer.advanceAt(from+i);
        else
            aw = -1;
        if (w==aw)
            continue;

        const WKCFloatRect textbox = { pos.x()+tofs, pos.y(), (float)w, h };
        const WKCFloatRect cr = { pos.x()+tofs, pos.y(), (float)cw, h };
        drawTextWithShadow(graphicsContext, dc, (const unsigned short *)tstart, tlen, &textbox, &cr, &pf, WKC_DRAWTEXT_OVERRIDE_BIDI);
        tstart += tlen;
        tofs += aw;
        tlen = 0;
        aw = 0;
        w = 0;
        cw = 0;
    }

    wkcDrawContextSetPatternPeer(dc, 0);
#if USE(WKC_CAIRO)
    if (pt) {
        if (gradient) {
            gradient->destroyPlatformGradient();
        } else {
            cairo_pattern_destroy((cairo_pattern_t *)pt);
        }
    }
#else
    if (pattern) {
        graphicsContext->tidyPattern_i(pattern);
    }
#endif

    if (needdelete)
        delete [] buf;
}

FloatRect Font::selectionRectForComplexText(const TextRun& run, const FloatPoint& point, int h, int from, int to) const
{
    void* font = 0;
    const UChar* str = 0;
    const SimpleFontData* sfont = 0;
    int len;
    float x0,x1,x2;
    float scale = 0;

    len = run.length();
    if (!len) return FloatRect();
    if (from>=len || to>len) return FloatRect();
    if (from>=to) return FloatRect();
    if (from<0) from = 0;
    if (to>len) to = len;

    str = run.characters();
    sfont = primaryFont();
    if (!sfont) return FloatRect();
    if (!sfont->platformData().font()) return FloatRect();
    font = sfont->platformData().font()->font();
    if (!font) return FloatRect();
    scale = sfont->platformData().font()->scale();

    int fflags = WKC_FONT_FLAG_NONE;
    if (run.directionalOverride()) {
        fflags |= WKC_FONT_FLAG_OVERRIDE_BIDI;
    }

    UChar* glyphs = 0;
    bool needdelete = false;
    int plen = 0;

    x0=x1=x2=0;

    plen = from;
    if (plen) {
        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), str, plen, needdelete);
        if (plen) {
            x0 = (float)getTextWidth(font, fflags, glyphs, plen, 0) * scale;
        }
        if (needdelete)
            delete [] glyphs;
    }
    plen = to-from;
    if (plen) {
        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), &str[from], plen, needdelete);
        if (plen) {
            x1 = (float)getTextWidth(font, fflags, glyphs, plen, 0) * scale;
        }
        if (needdelete)
            delete [] glyphs;
    }
    plen = len-to;
    if (plen) {
        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), &str[to], plen, needdelete);
        if (plen) {
            x2 = (float)getTextWidth(font, fflags, glyphs, plen, 0) * scale;
        }
        if (needdelete)
            delete [] glyphs;
    }

    if (run.rtl() && !run.directionalOverride()) {
        return FloatRect(point.x() + x2, point.y(), x1, h);
    } else {
        return FloatRect(point.x() + x0, point.y(), x1, h);
    }
}

void Font::drawComplexText(GraphicsContext* graphicsContext, const TextRun& run, const FloatPoint& point, int from, int to) const
{
    const UChar* str = 0;
    void* dc = (void *)graphicsContext->platformContext();
    const SimpleFontData* sfont = primaryFont();
    float scale = 0;
    int len;
    WKCPeerFont pf;
    WKCFloatRect clip;
    WKCFloatRect rect;
    int a, d;

    if (!dc) return;
    if (!sfont) return;

    len = run.length();
    if (!len) return;
    if (from>=len || to>len) return;
    if (from>=to) return;
    if (from<0) from = 0;
    if (to>len) to = len;

    WKC::WKCFontInfo* info = sfont->platformData().font();
    if (!info) return;
    pf.fFont = info->font();
    if (!pf.fFont) return;
    const FontPlatformData& pd(sfont->platformData());
    pf.fRequestedSize = info->requestSize();
    pf.fCreatedSize = info->createdSize();
    pf.fWeight = info->weight();
    pf.fItalic = info->isItalic();
    pf.fScale = info->scale();
    pf.fiScale = info->iscale();
    pf.fCanScale = info->canScale();
    pf.fFontId = (void *)static_cast<uintptr_t>(pd.hash());
    scale = info->scale();

    str = run.characters();

    a = (float)info->ascent() * scale;
    d = (float)info->descent() * scale;
    rect.fY = point.y() - a;
    rect.fHeight = pd.size();

    clip.fHeight = a+d;
    clip.fY = rect.fY;

    bool needdelete = false;
    UChar* glyphs = 0;
    int cw=0, tw=0;
    int glen = to-from;

    if (!glen) return;

    int fflags = WKC_FONT_FLAG_NONE;
    int dcflags = WKC_DRAWTEXT_COMPLEX;
    if (run.directionalOverride()) {
        fflags |= WKC_FONT_FLAG_OVERRIDE_BIDI;
        dcflags |= WKC_DRAWTEXT_OVERRIDE_BIDI;
    }

    if (run.rtl()) {
        const IntPoint ipoint(point.x(), point.y());
        const FloatRect dr = selectionRectForComplexText(run, ipoint, 1, from, to);
        const float px = dr.x();

        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), &str[from], glen, needdelete);
        if (glen>0) {
            tw = getTextWidth(pf.fFont, fflags, glyphs, glen, &cw);
            clip.fX = px;
            clip.fWidth = (float)cw * scale;
            if (wkcFontCanSupportDrawComplexPeer(pf.fFont)) {
                rect.fX = px;
                rect.fWidth = (float)tw * scale;
                drawTextWithShadow(graphicsContext, dc, glyphs, glen, &rect, &clip, &pf, dcflags|WKC_DRAWTEXT_COMPLEX_RTL);
            } else {
                rect.fX = px + tw*scale;
                for (int i=0; i<glen; i++) {
                    const float w = (float)getTextWidth(pf.fFont, fflags, glyphs+i, 1, 0)*scale;
                    rect.fX-=w;
                    rect.fWidth = w;
                    drawTextWithShadow(graphicsContext, dc, glyphs+i, 1, &rect, &clip, &pf, dcflags|WKC_DRAWTEXT_COMPLEX_TINY_RTL);
                }
            }
        }
        if (needdelete)
            delete [] glyphs;
    } else {
        rect.fX = point.x();
        if (from) {
            int plen = from;
            glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), str, plen, needdelete);
            if (plen) {
                tw = getTextWidth(pf.fFont, fflags, glyphs, plen, 0);
                rect.fX += (float)tw * scale;
            }
            if (needdelete)
                delete [] glyphs;
            glyphs = 0;
            needdelete = false;
        }

        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), &str[from], glen, needdelete);
        if (glen>0) {
            tw = getTextWidth(pf.fFont, fflags, glyphs, glen, &cw);
            clip.fX = rect.fX;
            clip.fWidth = (float)cw * scale;
            rect.fWidth = (float)tw * scale;
            drawTextWithShadow(graphicsContext, dc, glyphs, glen, &rect, &clip, &pf, dcflags);
        }
        if (needdelete)
            delete [] glyphs;
    }
}

float Font::floatWidthForComplexText(const TextRun& run, HashSet<const SimpleFontData*>* /*fallbackFonts*/, GlyphOverflow* /*overflow*/) const
{
    void* font = 0;
    int w = 0;
    const UChar* str = 0;
    const SimpleFontData* sfont = primaryFont();
    float scale = 0;
    int len;

    if (!sfont) return 0.f;
    if (!sfont->platformData().font()) return 0.f;

    len = run.length();
    if (!len) return 0.f;

    str = run.characters();

    font = sfont->platformData().font()->font();
    if (!font) return 0.f;
    scale = sfont->platformData().font()->scale();

    bool needdelete = false;
    UChar* glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), str, len, needdelete);

    int fflags = WKC_FONT_FLAG_NONE;
    if (run.directionalOverride()) {
        fflags |= WKC_FONT_FLAG_OVERRIDE_BIDI;
    }

    if (len) {
        w = getTextWidth(font, fflags, glyphs, len, 0);
    }
    if (needdelete)
        delete [] glyphs;

    return  (float)w * scale;
}

int Font::offsetForPositionForComplexText(const TextRun& run, float x, bool includePartialGlyphs) const
{
    void* font = 0;
    float w = 0.f;
    const UChar* str = 0;
    const SimpleFontData* sfont = 0;
    int i, len;
    float delta = 0;
    float scale = 0;

    len = run.length();
    if (!len) return 0;

    str = run.characters();
    sfont = primaryFont();
    if (!sfont || !sfont->platformData().font()) return 0;
    font = sfont->platformData().font()->font();
    if (!font) return 0;
    scale = sfont->platformData().font()->scale();

    bool needdelete = 0;
    int ulen = len;
    UChar* glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), str, ulen, needdelete);
    if (!ulen) {
        if (needdelete)
            delete [] glyphs;
        return 0;
    }

    int fflags = WKC_FONT_FLAG_NONE;
    if (run.directionalOverride()) {
        fflags |= WKC_FONT_FLAG_OVERRIDE_BIDI;
    }

    i = 0;
    float lw = 0;
    float dw = 0;

    if (run.rtl()) {
        float ofs = (float)x - floatWidthForComplexText(run, 0);
        while (i<len) {
            w = (float)getTextWidth(font, fflags, glyphs, i+1, 0) * scale;
            dw = w - lw;
            lw = w;
            delta = ofs + w;
            if (includePartialGlyphs) {
                if (delta - dw / 2 >= 0)
                    break;
            } else {
                if (delta >= 0)
                    break;
            }
            i++;
        }
    } else {
        float ofs = (float)x;
        while (i<len) {
            w = (float)getTextWidth(font, fflags, glyphs, i+1, 0) * scale;
            dw = w - lw;
            lw = w;
            delta = ofs - w;
            if (includePartialGlyphs) {
                if (delta + dw / 2 <= 0)
                    break;
            } else {
                if (delta <= 0)
                    break;
            }
            i++;
        }
    }

    if (needdelete)
        delete [] glyphs;

    return i;
}

float
Font::getGlyphsAndAdvancesForComplexText(const TextRun& run, int from, int to, GlyphBuffer& glyphBuffer, ForTextEmphasisOrNot forTextEmphasis) const
{
    if (forTextEmphasis) {
        return 0.f;
    }

    int len = run.length();
    if (!len) return 0.f;
    if (from>=len || to>len) return 0.f;
    if (from>=to) return 0.f;
    if (from<0) from = 0;
    if (to>len) to = len;

    const UChar* str = run.characters();
    const SimpleFontData* sfont = primaryFont();
    if (!sfont || !sfont->platformData().font()) return 0.f;
    void* font = sfont->platformData().font()->font();
    if (!font) return 0.f;
    const float scale = 0.f;

    UChar* glyphs = 0;
    bool needdelete = false;
    int plen = 0;

    float x0 = 0.f;
    float x1 = 0.f;
    float x2 = 0.f;

    int fflags = WKC_FONT_FLAG_NONE;
    if (run.directionalOverride()) {
        fflags |= WKC_FONT_FLAG_OVERRIDE_BIDI;
    }

    plen = from;
    if (plen) {
        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), str, plen, needdelete);
        if (plen) {
            x0 = (float)getTextWidth(font, fflags, glyphs, plen, 0) * scale;
        }
        if (needdelete)
            delete [] glyphs;
    }
    plen = to-from;
    if (plen) {
        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), &str[from], plen, needdelete);
        if (plen) {
            x1 = (float)getTextWidth(font, fflags, glyphs, plen, 0) * scale;
        }
        for (int i=0; i<plen; i++) {
            const float fw = getTextWidth(font, fflags, &glyphs[i], 1, 0) * scale;
            glyphBuffer.add(glyphs[i], sfont, fw, 0);
        }
        if (needdelete)
            delete [] glyphs;
    }
    plen = len-to;
    if (plen) {
        glyphs = fixedGlyphs(sfont->platformData().font()->specificUnicodeChar(), &str[to], plen, needdelete);
        if (plen) {
            x2 = (float)getTextWidth(font, fflags, glyphs, plen, 0) * scale;
        }
        if (needdelete)
            delete [] glyphs;
    }

    (void)x1;

    if (run.rtl() && !run.directionalOverride()) {
        return x2;
    } else {
        return x0;
    }
}
 
void
Font::drawEmphasisMarksForComplexText(GraphicsContext* context, const TextRun& run, const AtomicString& mark, const FloatPoint& point, int from, int to) const
{
    GlyphBuffer glyphBuffer;
    float initialAdvance = getGlyphsAndAdvancesForComplexText(run, from, to, glyphBuffer, ForTextEmphasis);

    if (glyphBuffer.isEmpty())
        return;

    drawEmphasisMarks(context, run, mark, FloatPoint(point.x() + initialAdvance, point.y()));

}


bool
Font::canExpandAroundIdeographsInComplexText()
{
    return false;
}

}
