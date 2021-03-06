/*
 * Copyright (C) 2008 Apple Computer, Inc.  All rights reserved.
 * Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "DNS.h"
#include "DNSResolveQueue.h"
#include "ResourceHandleManagerWKC.h"

#include <wkc/wkcsocket.h>

namespace WebCore {

void prefetchDNS(const String& hostname)
{
    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();
    String proxy = mgr->proxy();
    if (proxy.length() != 0)
        return;
    if (hostname.length() == 0)
        return;

    DNSResolveQueue::shared().add(hostname);
}

bool DNSResolveQueue::platformProxyIsEnabledInSystemPreferences()
{
    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();
    String proxy = mgr->proxy();
    if (proxy.length() != 0)
        return true;
    return false;
}

void DNSResolveQueue::platformResolve(const String& hostname)
{
    wkcNetPrefetchDNSPeer(hostname.utf8().data(), hostname.length());
    DNSResolveQueue::shared().decrementRequestCount();
}

}
