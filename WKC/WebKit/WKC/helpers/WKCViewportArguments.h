/*
 * Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef _WKC_HELPERS_WKC_VIEWPORTARGUMENTS_H_
#define _WKC_HELPERS_WKC_VIEWPORTARGUMENTS_H_

#include <wkc/wkcbase.h>

namespace WKC {

struct ViewportArguments {
    enum {
        ValueAuto = -1,
        ValueDesktopWidth = -2,
        ValueDeviceWidth = -3,
        ValueDeviceHeight = -4,
        ValueDeviceDPI = -5,
        ValueLowDPI = -6,
        ValueMediumDPI = -7,
        ValueHighDPI = -8
    };

    ViewportArguments()
        : initialScale(ValueAuto)
        , minimumScale(ValueAuto)
        , maximumScale(ValueAuto)
        , width(ValueAuto)
        , height(ValueAuto)
        , targetDensityDpi(ValueAuto)
        , userScalable(ValueAuto)
    {
    }

    float initialScale;
    float minimumScale;
    float maximumScale;
    float width;
    float height;
    float targetDensityDpi;
    float userScalable;

    bool operator==(const ViewportArguments& other) const
    {
        return initialScale == other.initialScale
            && minimumScale == other.minimumScale
            && maximumScale == other.maximumScale
            && width == other.width
            && height == other.height
            && targetDensityDpi == other.targetDensityDpi
            && userScalable == other.userScalable;
    }
};

} // namespace

#endif // _WKC_HELPERS_WKC_VIEWPORTARGUMENTS_H_

