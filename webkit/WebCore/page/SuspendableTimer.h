/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
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

#ifndef SuspendableTimer_h
#define SuspendableTimer_h

#include "ActiveDOMObject.h"
#include "Timer.h"

namespace WebCore {

class SuspendableTimer : public TimerBase, public ActiveDOMObject {
public:
    explicit SuspendableTimer(ScriptExecutionContext*);
    virtual ~SuspendableTimer();

    // ActiveDOMObject
    virtual bool hasPendingActivity() const;
    virtual void stop();
    virtual bool canSuspend() const;
    virtual void suspend(ReasonForSuspension);
    virtual void resume();
#if PLATFORM(WKC)
    virtual void resumeOnInvisibleChrome();
#endif

private:
    virtual void fired() = 0;

#if PLATFORM(WKC)
    void setSuspensionReason(ReasonForSuspension reason);
    bool clearSuspensionReason();
#endif

    double m_nextFireInterval;
    double m_repeatInterval;
    bool m_active;
#if PLATFORM(WKC)
    // Assumed that m_suspendReason_XXX are changed only in {set,clear}SuspensionReason().
    ReasonForSuspension m_suspendReason_general; // used for not ChromeIsInvisible nor ChromeWillBecomeVisible
    ReasonForSuspension m_suspendReason_chromeVisible; // used only for ChromeIsInvisible or ChromeWillBecomeVisible
#elif !ASSERT_DISABLED
    bool m_suspended;
#endif
};

} // namespace WebCore

#endif // SuspendableTimer_h

