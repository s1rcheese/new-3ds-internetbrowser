/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (c) 2012, 2013 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
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
 *
 */

#include "config.h"
#include "SuspendableTimer.h"

#include "ScriptExecutionContext.h"

namespace WebCore {

SuspendableTimer::SuspendableTimer(ScriptExecutionContext* context)
    : ActiveDOMObject(context, this)
    , m_nextFireInterval(0)
    , m_repeatInterval(0)
    , m_active(false)
#if PLATFORM(WKC)
    , m_suspendReason_general(ActiveDOMObject::None)
    , m_suspendReason_chromeVisible(ActiveDOMObject::None)
#elif !ASSERT_DISABLED
    , m_suspended(false)
#endif
{
}

SuspendableTimer::~SuspendableTimer()
{
}

bool SuspendableTimer::hasPendingActivity() const
{
    return isActive();
}

void SuspendableTimer::stop()
{
    TimerBase::stop();
}

#if PLATFORM(WKC)
void SuspendableTimer::suspend(ReasonForSuspension reason)
{
    bool suspended = (m_suspendReason_chromeVisible != ActiveDOMObject::None || m_suspendReason_general != ActiveDOMObject::None);
    setSuspensionReason(reason);
    if (suspended)
        return;
#else
void SuspendableTimer::suspend(ReasonForSuspension)
{
#if !ASSERT_DISABLED
    ASSERT(!m_suspended);
    m_suspended = true;
#endif
#endif
    m_active = isActive();
    if (m_active) {
        m_nextFireInterval = nextFireInterval();
        m_repeatInterval = repeatInterval();
        TimerBase::stop();
    }
}

void SuspendableTimer::resume()
{
#if PLATFORM(WKC)
    bool isResumable = clearSuspensionReason();
    if (!isResumable)
        return; // Keep suspended because another reason remains.

    if (m_active)
        start(m_nextFireInterval, m_repeatInterval);
    m_active = false; // start() only once even if resume() is called many times
#else
#if !ASSERT_DISABLED
    ASSERT(m_suspended);
    m_suspended = false;
#endif
    if (m_active)
        start(m_nextFireInterval, m_repeatInterval);
#endif
}

#if PLATFORM(WKC)
void SuspendableTimer::resumeOnInvisibleChrome()
{
    if (m_suspendReason_chromeVisible == ActiveDOMObject::ChromeIsInvisible) {
        setSuspensionReason(ActiveDOMObject::ChromeWillBecomeVisible); // Set ready to clear the reason ChromeIsInvisible.
        resume();
    }
}

void SuspendableTimer::setSuspensionReason(ReasonForSuspension reason)
{
    ASSERT(reason != ActiveDOMObject::None);
    if (reason == ActiveDOMObject::None)
        return; // Must use clearSuspensionReason() to set any suspension reasons to None.

    if (reason == ActiveDOMObject::ChromeWillBecomeVisible) {
        // Come here only when this method is called from SuspendableTimer::resumeOnInvisibleChrome().
        // Make m_suspendReason_chromeVisible ready to be cleared.
        ASSERT(m_suspendReason_chromeVisible == ActiveDOMObject::ChromeIsInvisible);
        m_suspendReason_chromeVisible = reason;
    } else {
        // Normal case, called from SuspendableTimer::suspend().
        if (reason == ActiveDOMObject::ChromeIsInvisible) {
            ASSERT(m_suspendReason_chromeVisible == ActiveDOMObject::None);
            m_suspendReason_chromeVisible = reason;
        } else {
            ASSERT(m_suspendReason_general == ActiveDOMObject::None);
            m_suspendReason_general = reason;
        }
    }
}

bool SuspendableTimer::clearSuspensionReason()
{
    ASSERT(m_suspendReason_general != ActiveDOMObject::None || m_suspendReason_chromeVisible != ActiveDOMObject::None);

    bool isResumable = false;
    if (m_suspendReason_chromeVisible == ActiveDOMObject::ChromeWillBecomeVisible) {
        // resume() is called from resumeOnInvisibleChrome().
        m_suspendReason_chromeVisible = ActiveDOMObject::None;
        if (m_suspendReason_general == ActiveDOMObject::None)
            isResumable = true; // All suspension reasons are cleared, so this timer is ready to resume.
    } else {
        // resume() is called from other functions than resumeOnInvisibleChrome(): original flow.
        m_suspendReason_general = ActiveDOMObject::None;
        if (m_suspendReason_chromeVisible == ActiveDOMObject::None)
            isResumable = true; // All suspension reasons are cleared, so this timer is ready to resume.
    }

    return isResumable;
}
#endif

bool SuspendableTimer::canSuspend() const
{
    return true;
}

} // namespace WebCore
