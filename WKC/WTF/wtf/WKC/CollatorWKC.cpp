/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#include "config.h"
#include "Collator.h"

namespace WTF {

Collator::Collator(const char*)
    : m_locale(0)
    , m_lowerFirst(false)
{
}

Collator::~Collator()
{
}

void Collator::setOrderLowerFirst(bool flag)
{
    m_lowerFirst = flag;
}

PassOwnPtr<Collator> Collator::userDefault()
{
    return adoptPtr(new Collator(0));
}

Collator::Result Collator::collate(const UChar* lhs, size_t lhsLength, const UChar* rhs, size_t rhsLength) const
{
    // only act as us-ascii collator...

    int lmin = lhsLength < rhsLength ? lhsLength : rhsLength;
    int l = 0;
    while (l < lmin && *lhs == *rhs) {
        lhs++;
        rhs++;
        l++;
    }

    if (l < lmin) {
        int lv = *lhs;
        int rv = *rhs;
        int lf = wkcUnicodeFoldCasePeer(lv);
        int rf = wkcUnicodeFoldCasePeer(rv);
        if (lf==rf) {
            if (m_lowerFirst)
                return (rv > lv) ? Greater : Less;
            else
                return (lv > rv) ? Greater : Less;
        }
        return (lf > rf) ? Greater : Less;
    }

    if (lhsLength == rhsLength)
        return Equal;

    return (lhsLength > rhsLength) ? Greater : Less;
}

}
