/*
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#include "config.h"

#include "helpers/WKCDeviceOrientationController.h"
#include "helpers/privates/WKCDeviceOrientationControllerPrivate.h"

#include "DeviceOrientationController.h"

#include "helpers/privates/WKCDeviceOrientationPrivate.h"

namespace WKC {

// Private Implementation

DeviceOrientationControllerPrivate::DeviceOrientationControllerPrivate(WebCore::DeviceOrientationController* parent)
     : m_webcore(parent)
     , m_wkc(*this)
     , m_orientation(0)
{
}

DeviceOrientationControllerPrivate::~DeviceOrientationControllerPrivate()
{
    delete m_orientation;
}

void
DeviceOrientationControllerPrivate::didChangeDeviceOrientation(DeviceOrientation* orientation)
{
    if (!orientation)
        return;
    delete m_orientation;
    m_orientation = orientation;
    m_webcore->didChangeDeviceOrientation(m_orientation->priv()->webcore().get());
}


// Implementation

DeviceOrientationController::DeviceOrientationController(DeviceOrientationControllerPrivate& parent)
     : m_private(parent)
{
}

DeviceOrientationController::~DeviceOrientationController()
{
}

void
DeviceOrientationController::didChangeDeviceOrientation(DeviceOrientation* orientation)
{
    m_private.didChangeDeviceOrientation(orientation);
}

} // namespace
