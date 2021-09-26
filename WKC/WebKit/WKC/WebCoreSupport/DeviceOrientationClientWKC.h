/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#ifndef DeviceOrientationClientWKC_h
#define DeviceOrientationClientWKC_h

#include "DeviceOrientationClient.h"

namespace WebCore {
class DeviceOrientationController;
class DeviceOrientation;
}

namespace WKC {
class DeviceOrientationClientIf;
class WKCWebViewPrivate;
class DeviceOrientationControllerPrivate;
class DeviceOrientation;

class DeviceOrientationClientWKC : public WebCore::DeviceOrientationClient {
public:
    static DeviceOrientationClientWKC* create(WKCWebViewPrivate*);
    ~DeviceOrientationClientWKC();

    virtual void setController(WebCore::DeviceOrientationController*);
    virtual void startUpdating();
    virtual void stopUpdating();
    virtual WebCore::DeviceOrientation* lastOrientation() const;
    virtual void deviceOrientationControllerDestroyed();

private:
    DeviceOrientationClientWKC(WKCWebViewPrivate* webView);
    bool construct();

private:
    WKCWebViewPrivate* m_view;
    DeviceOrientationClientIf* m_appClient;
    DeviceOrientationControllerPrivate* m_controller;
    mutable DeviceOrientation* m_orientation;
};

} // namespece

#endif // DeviceOrientationClientWKC_h
