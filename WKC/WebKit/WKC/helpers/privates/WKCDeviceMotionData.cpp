/*
 * Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCDeviceMotionData.h"
#include "helpers/privates/WKCDeviceMotionDataPrivate.h"

#include "DeviceMotionData.h"

namespace WKC {

// Private Implementation

// DeviceMotionDataPrivate

DeviceMotionDataPrivate::DeviceMotionDataPrivate(DeviceMotionData* parent,
                                                 DeviceMotionData::Acceleration* acceleration, DeviceMotionData::Acceleration* accelerationIncludingGravity, DeviceMotionData::RotationRate* rotationRate,
                                                 bool canProvideInterval, double interval)
     : m_webcore(WebCore::DeviceMotionData::create(acceleration->priv()->webcore(), accelerationIncludingGravity->priv()->webcore(), rotationRate->priv()->webcore(), canProvideInterval, interval))
     , m_wkc(parent)
     , m_acceleration(acceleration)
     , m_accelerationIncludingGravity(accelerationIncludingGravity)
     , m_rotation(rotationRate)
{
}

DeviceMotionDataPrivate::~DeviceMotionDataPrivate()
{
    if (m_acceleration) {
        delete m_acceleration;
    }
    if (m_accelerationIncludingGravity) {
        delete m_accelerationIncludingGravity;
    }
    if (m_rotation) {
        delete m_rotation;
    }
}

// AccelerationPrivate

AccelerationPrivate::AccelerationPrivate(DeviceMotionData::Acceleration* parent,
                                         bool canProvideX, double x, bool canProvideY, double y, bool canProvideZ, double z)
     : m_webcore(WebCore::DeviceMotionData::Acceleration::create(canProvideX, x, canProvideY, y, canProvideZ, z))
     , m_wkc(parent)
{
}

AccelerationPrivate::~AccelerationPrivate()
{
}

// RotationRatePrivate

RotationRatePrivate::RotationRatePrivate(DeviceMotionData::RotationRate* parent,
                                         bool canProvideAlpha, double alpha, bool canProvideBeta,  double beta, bool canProvideGamma, double gamma)
     : m_webcore(WebCore::DeviceMotionData::RotationRate::create(canProvideAlpha, alpha, canProvideBeta, beta, canProvideGamma, gamma))
     , m_wkc(parent)
{
}

RotationRatePrivate::~RotationRatePrivate()
{
}


// Implementation

// DeviceMotionData

DeviceMotionData::DeviceMotionData(Acceleration* acceleration, Acceleration* accelerationIncludingGravity, RotationRate* rotationRate,
                                   bool canProvideInterval, double interval)
     : m_private(new DeviceMotionDataPrivate(this, acceleration, accelerationIncludingGravity, rotationRate, canProvideInterval, interval))
{
}

DeviceMotionData::~DeviceMotionData()
{
    delete m_private;
}

DeviceMotionData*
DeviceMotionData::create(Acceleration* acceleration, Acceleration* accelerationIncludingGravity, RotationRate* rotationRate,
                         bool canProvideInterval, double interval)
{
    return new DeviceMotionData(acceleration, accelerationIncludingGravity, rotationRate, canProvideInterval, interval);
}

void
DeviceMotionData::destroy(DeviceMotionData* self)
{
    delete self;
}

// DeviceMotionData::Acceleration

DeviceMotionData::Acceleration::Acceleration(bool canProvideX, double x, bool canProvideY, double y, bool canProvideZ, double z)
     : m_private(new AccelerationPrivate(this, canProvideX, x, canProvideY, y, canProvideZ, z))
{
}

DeviceMotionData::Acceleration::~Acceleration()
{
    delete m_private;
}

DeviceMotionData::Acceleration*
DeviceMotionData::Acceleration::create(bool canProvideX, double x, bool canProvideY, double y, bool canProvideZ, double z)
{
    return new Acceleration(canProvideX, x, canProvideY, y, canProvideZ, z);
}

void
DeviceMotionData::Acceleration::destroy(DeviceMotionData::Acceleration* self)
{
    delete self;
}

// DeviceMotionData::RotationRate

DeviceMotionData::RotationRate::RotationRate(bool canProvideAlpha, double alpha, bool canProvideBeta, double beta, bool canProvideGamma, double gamma)
     : m_private(new RotationRatePrivate(this, canProvideAlpha, alpha, canProvideBeta, beta, canProvideGamma, gamma))
{
}

DeviceMotionData::RotationRate::~RotationRate()
{
    delete m_private;
}

DeviceMotionData::RotationRate*
DeviceMotionData::RotationRate::create(bool canProvideAlpha, double alpha, bool canProvideBeta, double beta, bool canProvideGamma, double gamma)
{
    return new DeviceMotionData::RotationRate(canProvideAlpha, alpha, canProvideBeta, beta, canProvideGamma, gamma);
}

void
DeviceMotionData::RotationRate::destroy(DeviceMotionData::RotationRate* self)
{
    delete self;
}

} // namespace
