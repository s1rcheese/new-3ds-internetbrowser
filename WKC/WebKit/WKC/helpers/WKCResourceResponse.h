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

#ifndef _WKC_HELPERS_WKC_RESOURCERESPONSE_H_
#define _WKC_HELPERS_WKC_RESOURCERESPONSE_H_

#include <wkc/wkcbase.h>

namespace WKC {
class String;
class KURL;
class ResourceHandle;
class ResourceResponsePrivate;

class WKC_API ResourceResponse {
public:
    ResourceResponse();
    ~ResourceResponse();

    ResourceResponse(const ResourceResponse&);
    ResourceResponse& operator=(const ResourceResponse&);

    const KURL url() const;

    bool isAttachment() const;
    int httpStatusCode() const;
    bool isNull() const;
    long long expectedContentLength() const;
    const String mimeType() const;
    const String& httpStatusText() const;
    const String httpHeaderField(const char* name) const;
    bool wasCached() const;

    ResourceHandle* resourceHandle() const;

    ResourceResponse(ResourceResponsePrivate&);

    ResourceResponsePrivate* priv() const { return m_private; }

private:
    ResourceResponsePrivate* m_private;
    bool m_owned;
};
} // namespace

#endif // _WKC_HELPERS_WKC_RESOURCERESPONSE_H_

