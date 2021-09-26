/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007, 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * All rights reserved.
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "SimpleFontData.h"

#include "FloatRect.h"
#include "Font.h"
#include "FontCache.h"
#include "FontDescription.h"
#include "FontPlatformDataWKC.h"
#include "GlyphBuffer.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

static int averageGlyph(int glyph=0, bool set=false)
{
    WKC_DEFINE_STATIC_INT(gGlyph, 0);
    if (set) {
        gGlyph = glyph;
    }
    return gGlyph;
}

void SimpleFontData::platformInit()
{
    WKC::WKCFontInfo* info = m_platformData.font();
    if (!info)
        return;

    void* font = info->font();
    float scale = info->scale();
    if (!font) return;

    m_fontMetrics.setAscent(info->ascent() * scale);
    m_fontMetrics.setDescent(info->descent() * scale);
    m_fontMetrics.setLineSpacing(info->lineSpacing() * scale);
    m_fontMetrics.setXHeight(wkcFontGetXHeightPeer(font) * scale);
    m_fontMetrics.setUnitsPerEm(1);
    m_fontMetrics.setLineGap(wkcFontGetLineGapPeer(font) * scale);
}

void SimpleFontData::platformCharWidthInit()
{
    WKC::WKCFontInfo* info = m_platformData.font();
    if (!info)
        return;

    void* font = info->font();
    float scale = info->scale();

    if (!font) return;
    if (!averageGlyph()) {
        m_avgCharWidth = wkcFontGetAverageCharWidthPeer(font) * scale;
    } else {
        unsigned short buf[2] = {0};
        buf[0] = (unsigned short)averageGlyph();
        m_avgCharWidth = Font::getTextWidth(font, WKC_FONT_FLAG_NONE, buf, 1, 0) * scale;
    }
    m_maxCharWidth = (float)wkcFontGetMaxCharWidthPeer(font) * scale;
}

void SimpleFontData::platformDestroy()
{
}

SimpleFontData* SimpleFontData::smallCapsFontData(const FontDescription& fontDescription) const
{
    if (!m_derivedFontData) {
        m_derivedFontData = DerivedFontData::create(isCustomFont());
    }
    if (!m_derivedFontData->smallCaps){
        FontDescription desc = FontDescription(fontDescription);
        desc.setComputedSize(0.70f*fontDescription.computedSize());
        const String fn = String::fromUTF8(m_platformData.font()->familyName().data());
        m_derivedFontData->smallCaps = adoptPtr(new SimpleFontData(FontPlatformData(desc, fn), isCustomFont(), false));
    }
    return m_derivedFontData->smallCaps.get();
}

SimpleFontData* SimpleFontData::emphasisMarkFontData(const FontDescription& fontDescription) const
{
    if (!m_derivedFontData) {
        m_derivedFontData = DerivedFontData::create(isCustomFont());
    }
    if (!m_derivedFontData->emphasisMark){
        FontDescription desc = FontDescription(fontDescription);
        desc.setComputedSize(0.50f*fontDescription.computedSize());
        const String fn = String::fromUTF8(m_platformData.font()->familyName().data());
        m_derivedFontData->emphasisMark = adoptPtr(new SimpleFontData(FontPlatformData(desc, fn), isCustomFont(), false));
    }
    return m_derivedFontData->emphasisMark.get();
}


FloatRect SimpleFontData::platformBoundsForGlyph(Glyph glyph) const
{
    WKC::WKCFontInfo* info = m_platformData.font();
    if (!info || !info->font()) return FloatRect();

    const float scale = info->scale();
    UChar32 c = glyph;
    int u = info->specificUnicodeChar();

    if (u==0) {
        if (Font::treatAsSpace(c)) {
            c = 0x20;
        } else if ((Font::treatAsZeroWidthSpace(c) || c==0xfeff)) {
            return FloatRect(0,0,0,0);
        }
    } else {
        c = u;
    }

    unsigned short buf[2] = {0};
    int len = 1;
    if (c<0x10000) {
        buf[0] = c;
        len = 1;
    } else {
        buf[0] = U16_LEAD(c);
        buf[1] = U16_TRAIL(c);
        len = 2;
    }
    int cw = 0;
    /*const int tw = */Font::getTextWidth(info->font(), WKC_FONT_FLAG_NONE, buf, len, &cw);

    float w = (float)cw;
    if (scale!=1.f) {
        w = (float)cw * scale;
    }

    const float h = wkcFontGetSizePeer(info->font());
    return FloatRect(0, 0, w, h);
}


bool SimpleFontData::containsCharacters(const UChar* characters, int length) const
{
    ASSERT(characters && (length==1||length==2));
    UChar32 c = 0;
    if (length==1) {
        c = characters[0];
    } else{ 
        c = U16_GET_SUPPLEMENTARY(characters[0], characters[1]);
    }
    return (m_platformData.font() && c==m_platformData.font()->specificUnicodeChar());
}

void SimpleFontData::determinePitch()
{
    WKC::WKCFontInfo* info = m_platformData.font();
    if (!info || !info->font()) return;

    m_treatAsFixedPitch = wkcFontIsFixedFontPeer(info->font());
}

float SimpleFontData::platformWidthForGlyph(Glyph glyph) const
{
    WKC::WKCFontInfo* info = m_platformData.font();
    if (!info || !info->font()) return 0;

    const float scale = info->scale();
    UChar32 c = glyph;
    int u = info->specificUnicodeChar();

    if (u==0) {
        if (Font::treatAsSpace(c)) {
            c = 0x20;
        } else if ((Font::treatAsZeroWidthSpace(c) || c==0xfeff)) {
            return 0.f;
        }
    } else {
        c = u;
    }

    unsigned short buf[2] = {0};
    int len = 1;
    if (c<0x10000) {
        buf[0] = c;
        len = 1;
    } else {
        buf[0] = U16_LEAD(c);
        buf[1] = U16_TRAIL(c);
        len = 2;
    }
    const int w = Font::getTextWidth(info->font(), WKC_FONT_FLAG_NONE, buf, len, 0);

    if (scale==1.f) {
        return (float)w;
    } else {
        return (float)w * scale;
    }
}

void
SimpleFontData_SetAverageFontGlyph(const unsigned int in_glyph)
{
    averageGlyph(in_glyph, true);
}

}
