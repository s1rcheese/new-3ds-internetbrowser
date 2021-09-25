/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "RuntimeApplicationChecks.h"

#if USE(CF)
#include <CoreFoundation/CoreFoundation.h>
#include <wtf/RetainPtr.h>
#endif

#include <wtf/text/WTFString.h>

namespace WebCore {
    
static bool mainBundleIsEqualTo(const String& bundleIdentifierString)
{
#if USE(CF)
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (!mainBundle)
        return false;

    CFStringRef bundleIdentifier = CFBundleGetIdentifier(mainBundle);
    if (!bundleIdentifier)
        return false;

    RetainPtr<CFStringRef> bundleIdentifierToCompare(AdoptCF, bundleIdentifierString.createCFString());
    return CFStringCompare(bundleIdentifier, bundleIdentifierToCompare.get(), 0) == kCFCompareEqualTo;
#else
    return false;
#endif
}

bool applicationIsSafari()
{
    // FIXME: For the WebProcess case, ensure that this is Safari's WebProcess.
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isSafari, mainBundleIsEqualTo("com.apple.Safari") || mainBundleIsEqualTo("com.apple.WebProcess"));
#else
    static bool isSafari = mainBundleIsEqualTo("com.apple.Safari") || mainBundleIsEqualTo("com.apple.WebProcess");
#endif
    return isSafari;
}

bool applicationIsAppleMail()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isAppleMail, mainBundleIsEqualTo("com.apple.mail"));
#else
    static bool isAppleMail = mainBundleIsEqualTo("com.apple.mail");
#endif
    return isAppleMail;
}

bool applicationIsMicrosoftMessenger()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isMicrosoftMessenger, mainBundleIsEqualTo("com.microsoft.Messenger"));
#else
    static bool isMicrosoftMessenger = mainBundleIsEqualTo("com.microsoft.Messenger");
#endif
    return isMicrosoftMessenger;
}

bool applicationIsAdobeInstaller()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isAdobeInstaller, mainBundleIsEqualTo("com.adobe.Installers.Setup"));
#else
    static bool isAdobeInstaller = mainBundleIsEqualTo("com.adobe.Installers.Setup");
#endif
    return isAdobeInstaller;
}

bool applicationIsAOLInstantMessenger()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isAOLInstantMessenger, mainBundleIsEqualTo("com.aol.aim.desktop"));
#else
    static bool isAOLInstantMessenger = mainBundleIsEqualTo("com.aol.aim.desktop");
#endif
    return isAOLInstantMessenger;
}

bool applicationIsMicrosoftMyDay()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isMicrosoftMyDay, mainBundleIsEqualTo("com.microsoft.myday"));
#else
    static bool isMicrosoftMyDay = mainBundleIsEqualTo("com.microsoft.myday");
#endif
    return isMicrosoftMyDay;
}

bool applicationIsMicrosoftOutlook()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isMicrosoftOutlook, mainBundleIsEqualTo("com.microsoft.Outlook"));
#else
    static bool isMicrosoftOutlook = mainBundleIsEqualTo("com.microsoft.Outlook");
#endif
    return isMicrosoftOutlook;
}

bool applicationIsAperture()
{
#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_BOOL(isAperture, mainBundleIsEqualTo("com.apple.Aperture"));
#else
    static bool isAperture = mainBundleIsEqualTo("com.apple.Aperture");
#endif
    return isAperture;
}

} // namespace WebCore
