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

#include "helpers/WKCResourceResponse.h"
#include "helpers/privates/WKCResourceResponsePrivate.h"

#include "ResourceResponse.h"

#include "KURL.h"
#include "PlatformString.h"
#include "ResourceRequest.h"

#include "helpers/WKCKURL.h"
#include "helpers/WKCString.h"
#include "helpers/privates/WKCResourceHandlePrivate.h"

namespace WKC {

ResourceResponsePrivateBase::ResourceResponsePrivateBase()
    : m_resourceHandle(0)
{
}

ResourceResponsePrivateBase::~ResourceResponsePrivateBase()
{
    delete m_resourceHandle;
}

ResourceResponsePrivate::ResourceResponsePrivate(const WebCore::ResourceResponse& parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

ResourceResponsePrivate::~ResourceResponsePrivate()
{
}

ResourceResponsePrivateToCore::ResourceResponsePrivateToCore(const ResourceResponse& wkc)
    : m_instance(new WebCore::ResourceResponse())
    , m_webcore(*m_instance)
    , m_wkc(wkc)
{
}

ResourceResponsePrivateToCore::~ResourceResponsePrivateToCore()
{
    delete m_instance;
}

const KURL
ResourceResponsePrivateBase::url() const
{
    return webcore().url();
}

const String
ResourceResponsePrivateBase::mimeType() const
{
    return webcore().mimeType();
}

bool
ResourceResponsePrivateBase::isAttachment() const
{
    return webcore().isAttachment();
}

bool
ResourceResponsePrivateBase::isNull() const
{
    return webcore().isNull();
}

int
ResourceResponsePrivateBase::httpStatusCode() const
{
    return webcore().httpStatusCode();
}

long long
ResourceResponsePrivateBase::expectedContentLength() const
{
    return webcore().expectedContentLength();
}

const String&
ResourceResponsePrivateBase::httpStatusText()
{
    m_httpStatusText = webcore().httpStatusText();
    return m_httpStatusText;
}

const String
ResourceResponsePrivateBase::httpHeaderField(const char* name) const
{
    if (!name)
        return String();
    return webcore().httpHeaderField(name);
}

bool
ResourceResponsePrivateBase::wasCached() const
{
    return webcore().wasCached();
}

ResourceHandle*
ResourceResponsePrivateBase::resourceHandle()
{
    if (m_resourceHandle) {
        delete m_resourceHandle;
    }
    m_resourceHandle = new ResourceHandlePrivate(webcore().resourceHandle());
    return &m_resourceHandle->wkc();

}

ResourceResponse::ResourceResponse()
    : m_private(0)
    , m_owned(true)
{
    ResourceResponsePrivateToCore* i = new ResourceResponsePrivateToCore(*this);
    m_private = reinterpret_cast<ResourceResponsePrivate*>(i);
}

ResourceResponse::ResourceResponse(ResourceResponsePrivate& parent)
    : m_private(&parent)
    , m_owned(false)
{
}

ResourceResponse::~ResourceResponse()
{
    if (m_owned) {
//        delete m_private;
        ResourceResponsePrivateToCore* i = (ResourceResponsePrivateToCore *)m_private;
        delete i;
    }
}

ResourceResponse::ResourceResponse(const ResourceResponse& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_owned = false;
    }
}

ResourceResponse&
ResourceResponse::operator=(const ResourceResponse& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_owned = false;
    }
    return *this;
}

const KURL
ResourceResponse::url() const
{
    return m_private->url();
}

const String
ResourceResponse::mimeType() const
{
    return m_private->mimeType();
}

bool
ResourceResponse::isAttachment() const
{
    return m_private->isAttachment();
}

bool
ResourceResponse::isNull() const
{
    return m_private->isNull();
}

int
ResourceResponse::httpStatusCode() const
{
    return m_private->httpStatusCode();
}

ResourceHandle*
ResourceResponse::resourceHandle() const
{
    return m_private->resourceHandle();
}

long long
ResourceResponse::expectedContentLength() const
{
    return m_private->expectedContentLength();
}

const String&
ResourceResponse::httpStatusText() const
{
    return m_private->httpStatusText();
}

const String
ResourceResponse::httpHeaderField(const char* name) const
{
    return m_private->httpHeaderField(name);
}

bool
ResourceResponse::wasCached() const
{
    return m_private->wasCached();
}

} // namespace

