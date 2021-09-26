/*
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "FontCache.h"

#include "Font.h"
#include "FontPlatformDataWKC.h"
#include "SimpleFontData.h"
#include <wtf/Assertions.h>

namespace WebCore {

void FontCache::platformInit()
{
}

const SimpleFontData* FontCache::getFontDataForCharacters(const Font& font, const UChar* characters, int length)
{
    ASSERT(characters && (length==1||length==2));

    UChar32 c = 0;
    if (length==1) {
        c = characters[0];
    } else {
        c = U16_GET_SUPPLEMENTARY(characters[0], characters[1]);
    }
    FontPlatformData alt(font.fontDescription(), font.fontDescription().family().family());
    if (alt.font() && alt.font()->font()) {
        alt.font()->setSpecificUnicodeChar(c);
        return getCachedFontData(&alt, DoNotRetain);
    } else {
        return getLastResortFallbackFont(font.fontDescription());
    }
}

SimpleFontData* FontCache::getSimilarFontPlatformData(const Font& font)
{
    return 0;
}

SimpleFontData* FontCache::getLastResortFallbackFont(const FontDescription& fontDescription, ShouldRetain retain)
{
    AtomicString name("systemfont");
    SimpleFontData* fontData = getCachedFontData(fontDescription, name, false, retain);
    if (!fontData) {
        // The systemfont can not be found if the font engine is being suspended.
        // The nullfont we use here has no font data, which means nothing will be drawn.
        fontData = getCachedFontData(fontDescription, AtomicString("nullfont"), false, retain);
    }
    return fontData;
}

void FontCache::getTraitsInFamily(const AtomicString& familyName, Vector<unsigned>& traitsMasks)
{
}

FontPlatformData* FontCache::createFontPlatformData(const FontDescription& fontDescription, const AtomicString& family)
{
    FontPlatformData* ret = new FontPlatformData(fontDescription, family);

    if (!ret || !ret->font() || (!ret->font()->font() && family != AtomicString("nullfont"))) {
        delete ret;
        return 0;
    }

    return ret;
}

}
