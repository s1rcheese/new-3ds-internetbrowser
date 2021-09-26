/*
 * Copyright (c) 2011-2015 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_WKC_RESOURCEREQUEST_H_
#define _WKC_HELPERS_WKC_RESOURCEREQUEST_H_

#include <wkc/wkcbase.h>

namespace WKC {
class KURL;
class ResourceRequestPrivate;
class String;
class FormData;

class WKC_API ResourceRequest {
public:
    ResourceRequest(ResourceRequestPrivate&);
    ~ResourceRequest();

    const KURL& url() const;
    void setURL(const KURL&);

    void clearHTTPReferrer();
    void setHTTPHeaderField(const char*, const char*);
    void setHTTPBody(FormData*);
    void setHTTPMethod(const String&);
    String httpHeaderField(const char*) const;
    const String& httpMethod() const;

    bool isNull() const;

    enum TargetType {
        TargetIsMainFrame,
        TargetIsSubframe,
        TargetIsSubresource,
        TargetIsStyleSheet,
        TargetIsScript,
        TargetIsFontResource,
        TargetIsImage,
        TargetIsObject,
        TargetIsMedia,
        TargetIsWorker,
        TargetIsSharedWorker,
        TargetIsPrefetch,
        TargetIsPrerender,
        TargetIsFavicon,
        TargetIsXHR,
        TargetIsTextTrack,
        TargetIsUnspecified,
    };
    TargetType targetType() const;
    bool compare(const ResourceRequest& other) const;

    ResourceRequestPrivate& priv() const { return m_private; }

    bool isMainResource() const;

private:
    ResourceRequestPrivate& m_private;
};
} // namespace

#endif // _WKC_HELPERS_WKC_RESOURCEREQUEST_H_

