/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include "SpeechInputClientWKC.h"

#include "SpeechInputListener.h"
#include "PlatformString.h"
#include "WKCWebViewPrivate.h"

#include "helpers/SpeechInputClientIf.h"

#include "helpers/privates/WKCAtomicStringPrivate.h"
#include "helpers/privates/WKCSecurityOriginPrivate.h"

#if ENABLE(INPUT_SPEECH)

namespace WKC {

SpeechInputClientWKC::SpeechInputClientWKC(WKCWebViewPrivate* view)
    : m_view(view)
    , m_listener(0)
    , m_appClient(0)
{
}

SpeechInputClientWKC::~SpeechInputClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteSpeechInputClient(m_appClient);
        m_appClient = 0;
    }
}

SpeechInputClientWKC*
SpeechInputClientWKC::create(WKCWebViewPrivate* view)
{
    SpeechInputClientWKC* self = 0;
    self = new SpeechInputClientWKC(view);
    if (!self)
        return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
SpeechInputClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createSpeechInputClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
SpeechInputClientWKC::setListener(WebCore::SpeechInputListener* listener)
{
    m_listener = listener;
    if (!m_appClient)
        return;
    if (m_listener) {
        m_appClient->setListener(this);
    } else {
        m_appClient->setListener(0);
    }
}

bool
SpeechInputClientWKC::startRecognition(int requestId, const WebCore::IntRect& elementRect, const WTF::AtomicString& language, const WTF::String& grammar, WebCore::SecurityOrigin* origin)
{
    const AtomicStringPrivate wlanguage(const_cast<WTF::AtomicString*>(&language));
    SecurityOriginPrivate worigin(origin);

    if (m_appClient)
        return m_appClient->startRecognition(requestId, elementRect, wlanguage.wkc(), grammar, &worigin.wkc());
    else
        return false;
}

void
SpeechInputClientWKC::stopRecording(int requestId)
{
    if (m_appClient)
        m_appClient->stopRecording(requestId);
}

void
SpeechInputClientWKC::cancelRecognition(int requestId)
{
    if (m_appClient)
        m_appClient->cancelRecognition(requestId);
}

void
SpeechInputClientWKC::didCompleteRecording(int requestId)
{
    if (!m_listener)
        return;
    m_listener->didCompleteRecording(requestId);
}

void
SpeechInputClientWKC::didCompleteRecognition(int requestId)
{
    if (!m_listener)
        return;
    m_listener->didCompleteRecognition(requestId);
}

void
SpeechInputClientWKC::setRecognitionResult(int requestId, int results, const String* string, double* confidence)
{
    if (!m_listener)
        return;
    WebCore::SpeechInputResultArray ret;
    for (int i=0; i<results; i++) {
        ret.append(WebCore::SpeechInputResult::create(string[i], confidence[i]));
    }
    m_listener->setRecognitionResult(requestId, ret);
}

} // namespace

#endif // ENABLE(INPUT_SPEECH)
