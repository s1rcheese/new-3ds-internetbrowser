/*
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEB_AUDIO)

#include "AudioDestination.h"
#include "AudioSourceProvider.h"
#include "AudioSourceProviderClient.h"
#include "AudioBus.h"

#include "CurrentTime.h"
#include "Threading.h"

#include <wkc/wkcmediapeer.h>

namespace WebCore {

class AudioDestinationWKC : public AudioDestination, AudioSourceProviderClient
{
public:
    static AudioDestinationWKC* create(AudioSourceProvider&, float sampleRate);

    // AudioDestination
    virtual ~AudioDestinationWKC();

    virtual void start();
    virtual void stop();
    virtual bool isPlaying();

    virtual float sampleRate() const;

    // AudioSourceProviderClient
    virtual void setFormat(size_t numberOfChannels, float sampleRate);

private:
    AudioDestinationWKC(AudioSourceProvider&, float sampleRate);
    bool construct();
    static void threadProc(void* obj) { ((AudioDestinationWKC *)obj)->drain(); }
    void drain();

private:
    AudioSourceProvider& m_audioSourceProvider;
    AudioBus m_bus;
    static const unsigned int c_busFrameSize = 128;
    float m_sampleRate;
    size_t m_channels;

    void* m_peer;

    static const unsigned int c_deviceBufSize = 4096;
    unsigned char m_buf[c_deviceBufSize];
    int m_bufPtr;
    double m_lastwritetime;

    ThreadIdentifier m_thread;
    bool m_quit;
};

AudioDestinationWKC::AudioDestinationWKC(AudioSourceProvider& audioSourceProvider, float sampleRate)
    : m_audioSourceProvider(audioSourceProvider)
    , m_bus(2, c_busFrameSize, true)
    , m_sampleRate(sampleRate)
    , m_channels(2)
    , m_peer(0)
    , m_bufPtr(0)
    , m_lastwritetime(0)
    , m_thread(0)
    , m_quit(false)
{
    m_audioSourceProvider.setClient(this);
    m_buf[0] = 0;
}

AudioDestinationWKC::~AudioDestinationWKC()
{
    m_audioSourceProvider.setClient(0);
    if (m_thread) {
        m_quit = true;
        detachThread(m_thread);
    }
}

AudioDestinationWKC*
AudioDestinationWKC::create(AudioSourceProvider& audioSourceProvider, float sampleRate)
{
    AudioDestinationWKC* self = 0;
    self = new AudioDestinationWKC(audioSourceProvider, sampleRate);
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
AudioDestinationWKC::construct()
{
    return true;
}

void
AudioDestinationWKC::setFormat(size_t numberOfChannels, float sampleRate)
{
    m_channels = numberOfChannels;
    m_sampleRate = sampleRate;
}

void
AudioDestinationWKC::start()
{
    m_quit = false;
    m_thread = createThread(threadProc, this, "WKC: AudioDestination");
}

void
AudioDestinationWKC::stop()
{
    if (m_thread) {
        m_quit = true;
        detachThread(m_thread);
        m_thread = 0;        
    }
}

bool
AudioDestinationWKC::isPlaying()
{
    return m_peer ? true : false;
}

float
AudioDestinationWKC::sampleRate() const
{
    return m_sampleRate;
}

void AudioDestinationWKC::drain()
{
    m_peer = wkcAudioOpenPeer((int)m_sampleRate, 16, m_channels, 0);
    if (!m_peer)
        return;
    m_bufPtr = 0;
    m_lastwritetime = 0;
    double frametime = sizeof(m_buf) / (m_channels==2 ? 4 : 2) / m_sampleRate;

    while (!m_quit) {
        m_audioSourceProvider.provideInput(&m_bus, c_busFrameSize);

        AudioChannel *cl=0, *cr=0;
        const float *sl=0, *sr=0;
        cl = m_bus.channel(AudioBus::ChannelLeft);
        cr = m_bus.channel(AudioBus::ChannelRight);
        sl = cl->data();
        sr = cr->data();

        int v = 0;
        if (m_channels==1) {
            for (int i=0; i<c_busFrameSize; i++) {
                v = (int)((sl[i]+sr[i]) * 32768);
                if (v<-32768) v=-32768;
                else if (v>32767) v=32767;
                m_buf[m_bufPtr++] = v&0xff;
                m_buf[m_bufPtr++] = (v>>8)&0xff;
            }
        } else {
            for (int i=0; i<c_busFrameSize; i++) {
                v = (int)(sl[i] * 32768);
                if (v<-32768) v=-32768;
                else if (v>32767) v=32767;
                m_buf[m_bufPtr++] = v&0xff;
                m_buf[m_bufPtr++] = (v>>8)&0xff;
                v = (int)(sr[i] * 32768);
                if (v<-32768) v=-32768;
                else if (v>32767) v=32767;
                m_buf[m_bufPtr++] = v&0xff;
                m_buf[m_bufPtr++] = (v>>8)&0xff;
            }
        }

        if (m_bufPtr >= sizeof(m_buf)) {
            double cur = WTF::currentTime();
            if (cur - m_lastwritetime < frametime) {
                wkc_usleep(1000*1000*(frametime - (cur-m_lastwritetime)));
            }
            int remains = m_bufPtr;
            int ptr = 0;
            do {
                int ret = wkcAudioWritePeer(m_peer, m_buf + ptr, remains);
                if (ret<0)
                    break;
                if (ret==remains)
                    break;
                ptr += ret;
                remains-=ret;
                if (ret==0) {
                    wkc_usleep(1000*1000*frametime/m_channels/2);
                } else {
                    wkc_usleep(1000*1000*ret/m_sampleRate/(m_channels*2));
                }
            } while (remains>0);
            m_bufPtr = 0;
            m_lastwritetime = WTF::currentTime();
        }
    }

    wkcAudioClosePeer(m_peer);
    m_peer = 0;
}

PassOwnPtr<AudioDestination> AudioDestination::create(AudioSourceProvider& provider, float sampleRate)
{
    return adoptPtr(AudioDestinationWKC::create(provider, sampleRate));
}

float AudioDestination::hardwareSampleRate()
{
    return wkcAudioPreferredSampleRatePeer();
}

} // namespace WebCore

#endif // ENABLE(WEB_AUDIO)
