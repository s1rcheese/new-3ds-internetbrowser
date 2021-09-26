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

#ifndef SPEECHINPUTCLIENTWKC_H
#define SPEECHINPUTCLIENTWKC_H

#include "SpeechInputClient.h"
#include "helpers/WKCSpeechInputListener.h"

#if ENABLE(INPUT_SPEECH)

namespace WKC {

class WKCWebViewPrivate;
class SpeechInputClientIf;

class SpeechInputClientWKC : public WebCore::SpeechInputClient, public SpeechInputListener
{
public:
    static SpeechInputClientWKC* create(WKCWebViewPrivate*);
    virtual ~SpeechInputClientWKC();

    virtual void setListener(WebCore::SpeechInputListener*);
    virtual bool startRecognition(int requestId, const WebCore::IntRect& elementRect, const WTF::AtomicString& language, const WTF::String& grammar, WebCore::SecurityOrigin*);
    virtual void stopRecording(int requestId);
    virtual void cancelRecognition(int requestId);

public:
    virtual void didCompleteRecording(int requestId);
    virtual void didCompleteRecognition(int requestId);
    virtual void setRecognitionResult(int requestId, int results, const String*, double*);

private:
    SpeechInputClientWKC(WKCWebViewPrivate*);
    bool construct();

private:
    WKCWebViewPrivate* m_view;
    WebCore::SpeechInputListener* m_listener;
    SpeechInputClientIf* m_appClient;
};

} // namespace

#endif // ENABLE(INPUT_SPEECH)

#endif // SPEECHINPUTCLIENTWKC_H
