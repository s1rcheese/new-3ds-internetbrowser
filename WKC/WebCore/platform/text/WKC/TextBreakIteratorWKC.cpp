/*
 * Copyright (C) 2006 Lars Knoll <lars@trolltech.com>
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Jurg Billeter <j@bitron.ch>
 * Copyright (C) 2008 Dominik Rottsches <dominik.roettsches@access-company.com>
 * Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "TextBreakIterator.h"

#include <wkc/wkcpeer.h>

#include "NotImplemented.h"

namespace WebCore {

TextBreakIterator*
characterBreakIterator(const UChar* str, int length)
{
    WKC_DEFINE_STATIC_PTR(void*, gIteratorCharacter, wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_CHARACTER));

    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorCharacter, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorCharacter;
}

TextBreakIterator*
cursorMovementIterator(const UChar* str, int length)
{
    WKC_DEFINE_STATIC_PTR(void*, gIteratorCursorMovement, wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_CURSORMOVEMENT));

    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorCursorMovement, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorCursorMovement;
}

TextBreakIterator*
wordBreakIterator(const UChar* str, int length)
{
    WKC_DEFINE_STATIC_PTR(void*, gIteratorWord, wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_WORD));

    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorWord, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorWord;
}

TextBreakIterator*
acquireLineBreakIterator(const UChar* str, int length, const AtomicString& /*locale*/)
{
    void* iterator = 0;
    iterator = wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_LINE);
    if (!iterator)
        return 0;
    if (!wkcTextBreakIteratorSetStringPeer(iterator, str, length))
        return 0;
    return (TextBreakIterator *)iterator;
}

void
releaseLineBreakIterator(TextBreakIterator* self)
{
    if (self)
        wkcTextBreakIteratorReleasePeer((void *)self);
}


TextBreakIterator*
sentenceBreakIterator(const UChar*str, int length)
{
    WKC_DEFINE_STATIC_PTR(void*, gIteratorSentence, wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_SENTENCE));

    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorSentence, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorSentence;
}

int
textBreakFirst(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorFirstPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakLast(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorLastPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakNext(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorNextPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakPrevious(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorPreviousPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakCurrent(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorCurrentPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakPreceding(TextBreakIterator* self, int pos)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorPrecedingPeer((void *)self, pos);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakFollowing(TextBreakIterator* self, int pos)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorFollowingPeer((void *)self, pos);
    if (ret>=0) return ret;
    return TextBreakDone;
}

bool
isTextBreak(TextBreakIterator* self, int pos)
{
    if (!self) return true;

    return wkcTextBreakIteratorIsTextBreakPeer((void *)self, pos);
}

bool
isWordTextBreak(TextBreakIterator* self)
{
    if (!self) return true;

    notImplemented();
    return true;
}

}
