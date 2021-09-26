/*
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
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
 */

#include "config.h"

#if ENABLE(GEOLOCATION)

#include "GeolocationClientWKC.h"
#include "Modules/geolocation/GeolocationPosition.h"
#include "WKCWebView.h"
#include "WKCWebViewPrivate.h"
#include "helpers/GeolocationClientIf.h"

#include <wkc/wkcpeer.h>

#include "helpers/privates/WKCGeolocationPrivate.h"

namespace WKC {

GeolocationClientWKC::GeolocationClientWKC(WKCWebViewPrivate* view)
     : m_view(view)
{
    m_appClient = 0;
}

GeolocationClientWKC::~GeolocationClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteGeolocationClient(m_appClient);
        m_appClient = 0;
    }
}

GeolocationClientWKC*
GeolocationClientWKC::create(WKCWebViewPrivate* view)
{
    GeolocationClientWKC* self = 0;
    self = new GeolocationClientWKC(view);
    if (!self)
      return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;

}

bool
GeolocationClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createGeolocationClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
GeolocationClientWKC::geolocationDestroyed()
{
    delete this;
}

void
GeolocationClientWKC::startUpdating()
{
    m_appClient->startUpdating();
}

void
GeolocationClientWKC::stopUpdating()
{
    m_appClient->stopUpdating();
}

void
GeolocationClientWKC::setEnableHighAccuracy(bool flag)
{
    m_appClient->setEnableHighAccuracy(flag);
}

WebCore::GeolocationPosition*
GeolocationClientWKC::lastPosition()
{
    GeolocationPositionWKC pos = {0};
    m_appClient->lastPosition(pos);
    WTF::RefPtr<WebCore::GeolocationPosition> wpos = WebCore::GeolocationPosition::create(
        pos.fTimestamp,
        pos.fLatitude,
        pos.fLongitude,
        pos.fAccuracy,
        pos.fHaveAltitude, pos.fAltitude,
        pos.fHaveAltitudeAccuracy, pos.fAltitudeAccuracy,
        pos.fHaveHeading, pos.fHeading,
        pos.fHaveSpeed, pos.fSpeed);
    return wpos.release().leakRef();
}

void
GeolocationClientWKC::requestPermission(WebCore::Geolocation* webcore)
{
    GeolocationPrivate g(webcore);
    m_appClient->requestPermission(&g.wkc());
}

void
GeolocationClientWKC::cancelPermissionRequest(WebCore::Geolocation* webcore)
{
    GeolocationPrivate g(webcore);
    m_appClient->cancelPermissionRequest(&g.wkc());
}

} // namespace

#else

#include "helpers/GeolocationClientIf.h"

#endif // ENABLE(GEOLOCATION)
