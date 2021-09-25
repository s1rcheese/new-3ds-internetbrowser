/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_Complex_h
#define WTF_Complex_h

#if PLATFORM(WKC)
# if defined(_WKCPTHREAD_H_)
#  undef pthread_t
#  undef pthread_mutex_t
#  undef pthread_cond_t
#  undef pthread_key_t
#  undef pthread_once_t
#  undef pthread_create
#  undef pthread_self
#  undef pthread_equal
#  undef pthread_key_create
#  undef pthread_key_delete
#  undef pthread_setspecific
#  undef pthread_getspecific
#  undef pthread_mutex_init
#  undef pthread_mutex_destroy
#  undef pthread_mutex_lock
#  undef pthread_mutex_unlock
#  undef pthread_mutex_trylock
#  undef pthread_cond_init
#  undef pthread_cond_destroy
#  undef pthread_cond_signal
#  undef pthread_cond_wait
#  undef PTHREAD_MUTEX_INITIALIZER
#  undef PTHREAD_ONCE_INIT
#  undef pthread_once
# endif
# ifdef _PTHREAD_H
#  undef _PTHREAD_H
# endif
#endif
#include <complex>
#include <wtf/MathExtras.h>

namespace WTF {

typedef std::complex<double> Complex;

inline Complex complexFromMagnitudePhase(double magnitude, double phase)
{
    return Complex(magnitude * cos(phase), magnitude * sin(phase));
}

} // namespace WTF

using WTF::Complex;
using WTF::complexFromMagnitudePhase;

#endif // WTF_Complex_h
