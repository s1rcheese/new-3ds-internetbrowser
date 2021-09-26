/*
 * Copyright (C) 2011 ACCESS Co.,Ltd.
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

#ifndef FONTCUSTOMPLATFORMDATA_H
#define FONTCUSTOMPLATFORMDATA_H

#include "FontRenderingMode.h"
#include "FontPlatformData.h"

namespace WebCore {

class SharedBuffer;

class FontCustomPlatformData
{
public:
    static FontCustomPlatformData* create(SharedBuffer* buf);
    ~FontCustomPlatformData();

    FontPlatformData fontPlatformData(int size, bool bold, bool italic, FontOrientation orientation, TextOrientation textOrientation, FontWidthVariant widthVariant, FontRenderingMode mode);

    static bool supportsFormat(const WTF::String&);

private:
    FontCustomPlatformData(SharedBuffer* buf);
    bool construct();

private:
    SharedBuffer* m_buffer;
    char m_familyName[128];
    int m_registeredId;
};

FontCustomPlatformData* createFontCustomPlatformData(SharedBuffer* buffer);

} // namespace

#endif // FONTCUSTOMPLATFORMDATA_H
