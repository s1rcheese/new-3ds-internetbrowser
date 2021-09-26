/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#if ENABLE(MEDIA_STREAM)

#include "UserMediaClientImplWKC.h"

#include "WKCWebView.h"
#include <wtf/RefPtr.h>

using namespace WebCore;

namespace WKC {

UserMediaClientWKC::UserMediaClientWKC(WKCWebViewPrivate* webView)
    : m_view(webView)
{
}

UserMediaClientWKC::~UserMediaClientWKC()
{
}

UserMediaClientWKC*
UserMediaClientWKC::create(WKCWebViewPrivate* webView)
{
    UserMediaClientWKC* self = 0;
    self = new UserMediaClientWKC(webView);
    if (!self)
        return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
UserMediaClientWKC::construct()
{
    return true;
}

void UserMediaClientWKC::pageDestroyed()
{
}

void UserMediaClientWKC::requestUserMedia(PassRefPtr<UserMediaRequest> prpRequest, const MediaStreamSourceVector& audioSources, const MediaStreamSourceVector& videoSources)
{
    UserMediaRequest * request = prpRequest.get();
    // Todo:security confirmation?
    request->succeed(audioSources, videoSources);
}

void UserMediaClientWKC::cancelUserMediaRequest(UserMediaRequest* request)
{
}

} // namespace WKC

#endif // ENABLE(MEDIA_STREAM)
