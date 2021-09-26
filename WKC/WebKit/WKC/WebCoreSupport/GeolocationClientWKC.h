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

#ifndef GeolocationClientWKC_h
#define GeolocationClientWKC_h

#include "GeolocationClient.h"

namespace WKC {
class GeolocationClientIf;
class WKCWebViewPrivate;

class GeolocationClientWKC : public WebCore::GeolocationClient
{
public:
    static GeolocationClientWKC* create(WKCWebViewPrivate* view);
    ~GeolocationClientWKC();

    virtual void geolocationDestroyed();

    virtual void startUpdating();
    virtual void stopUpdating();
    virtual void setEnableHighAccuracy(bool);
    virtual WebCore::GeolocationPosition* lastPosition();

    virtual void requestPermission(WebCore::Geolocation*);
    virtual void cancelPermissionRequest(WebCore::Geolocation*);

private:
    GeolocationClientWKC(WKCWebViewPrivate* view);
    bool construct();

private:
    WKCWebViewPrivate* m_view;
    GeolocationClientIf* m_appClient;
};

}

#endif // GeolocationClientWKC_h
