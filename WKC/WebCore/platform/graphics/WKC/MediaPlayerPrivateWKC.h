/*
 *  Copyright (c) 2011-2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef _MEDIAPLAYERPRIVATEWKC_H_
#define _MEDIAPLAYERPRIVATEWKC_H_

#include "config.h"
#include "MediaPlayerPrivate.h"
#include "ResourceHandleClient.h"

#if ENABLE(VIDEO)

namespace WebCore {

class ResourceHandle;

class MediaPlayerPrivate :  public MediaPlayerPrivateInterface, public ResourceHandleClient {
public:
    static PassOwnPtr<MediaPlayerPrivateInterface> create(MediaPlayer* player);
    static void registerMediaEngine(MediaEngineRegistrar);
    static void getSupportedTypes(HashSet<String>& types);
    static MediaPlayer::SupportsType supportsType(const String& type, const String& codecs);
    static void getSitesInMediaCache(Vector<String>&);
    static void clearMediaCache();
    static void clearMediaCacheForSite(const String&);

    virtual ~MediaPlayerPrivate();

    // interfaces
    virtual void load(const String& url);
    virtual void cancelLoad();
    
    virtual void prepareToPlay();
    virtual PlatformMedia platformMedia() const;

    virtual void play();
    virtual void pause();    

    virtual bool supportsFullscreen() const;
    virtual bool supportsSave() const;

    virtual IntSize naturalSize() const;

    virtual bool hasVideo() const;
    virtual bool hasAudio() const;

    virtual void setVisible(bool);

    virtual float duration() const;

    virtual float currentTime() const;
    virtual void seek(float time);
    virtual bool seeking() const;

    virtual float startTime() const;

    virtual void setEndTime(float);

    virtual void setRate(float);
    virtual void setPreservesPitch(bool);

    virtual bool paused() const;

    virtual void setVolume(float);

    virtual bool hasClosedCaptions() const;
    virtual void setClosedCaptionsVisible(bool);

    virtual MediaPlayer::NetworkState networkState() const;
    virtual MediaPlayer::ReadyState readyState() const;

    virtual float maxTimeSeekable() const;
    virtual PassRefPtr<TimeRanges> buffered() const;
    virtual double maximumDurationToCacheMediaTime() const;

    virtual int dataRate() const;

    virtual bool totalBytesKnown() const;
    virtual unsigned totalBytes() const;
    virtual unsigned bytesLoaded() const;

    virtual void setSize(const IntSize&);

    virtual void paint(GraphicsContext*, const IntRect&);

    virtual void paintCurrentFrameInContext(GraphicsContext* c, const IntRect& r);

    virtual void setAutobuffer(bool);

    virtual bool canLoadPoster() const;
    virtual void setPoster(const String&);

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    virtual void deliverNotification(MediaPlayerProxyNotificationType);
    virtual void setMediaPlayerProxy(WebMediaPlayerProxy*);
#endif

    virtual void enterFullscreen() const;
    virtual void exitFullscreen();
    virtual bool canEnterFullscreen() const;

#if USE(ACCELERATED_COMPOSITING)
    virtual bool supportsAcceleratedRendering() const;
    virtual void acceleratedRenderingStateChanged();
    virtual PlatformLayer* platformLayer() const;
#endif

    virtual bool hasSingleSecurityOrigin() const;

    virtual MediaPlayer::MovieLoadType movieLoadType() const;

    virtual void removedFromDocument();
    virtual void willBecomeInactive();
    virtual void* platformPlayer() const;

    // downloader
    virtual void didReceiveResponse(ResourceHandle*, const ResourceResponse&);
    virtual void didReceiveData(ResourceHandle*, const char*, int, int /*encodedDataLength*/);
    virtual void didFinishLoading(ResourceHandle*, double /*finishTime*/);
    virtual void didFail(ResourceHandle*, const ResourceError&);

private:
    MediaPlayerPrivate(MediaPlayer* player);
    bool construct();

    // callbacks
    // player
    static void notifyPlayerStateProc(void* s, int i) { ((MediaPlayerPrivate *)s)->notifyPlayerState(i); }
    static void notifyRequestInvalidateProc(void* s, int x, int y, int w, int h) { ((MediaPlayerPrivate *)s)->notifyRequestInvalidate(x,y,w,h); }
    static void notifyAudioDataAvailableProc(void* s, float t, unsigned int l) { ((MediaPlayerPrivate *)s)->notifyAudioDataAvailable(t, l); }
    void notifyPlayerState(int);
    void notifyRequestInvalidate(int, int, int, int);
    void notifyAudioDataAvailable(float, int);

    // network
    static int notifyPlayerStreamGetProxyProc(void* s, void* o) { return ((MediaPlayerPrivate *)s)->notifyPlayerStreamGetProxy(o); }
    static int notifyPlayerStreamGetCookieProc(void* s, const char* u, char* o, int l) { return ((MediaPlayerPrivate *)s)->notifyPlayerStreamGetCookie(u, o,l); }

    static int notifyPlayerStreamOpenURIProc(void* s, const char* u, void* p) { return ((MediaPlayerPrivate *)s)->notifyPlayerStreamOpenURI(u,p); }
    static int notifyPlayerStreamCloseProc(void* s, int i) { return ((MediaPlayerPrivate *)s)->notifyPlayerStreamClose(i); }
    static int notifyPlayerStreamCancelProc(void* s, int i) { return ((MediaPlayerPrivate *)s)->notifyPlayerStreamCancel(i); }

    int notifyPlayerStreamGetProxy(void* out_info);
    int notifyPlayerStreamGetCookie(const char* uri, char* out_cookie, int buflen);
    int notifyPlayerStreamOpenURI(const char* uri, void* procs);
    int notifyPlayerStreamClose(int id);
    int notifyPlayerStreamCancel(int id);

    typedef struct httpConnection_ {
        int m_id;
        void* m_connection;
        void* m_instance;
        void* m_infoProc;
        void* m_receivedProc;
        void* m_errorProc;
    } httpConnection;
    httpConnection* findConnection(int);
    httpConnection* findConnection(ResourceHandle*);
    void removeConnection(ResourceHandle*);

    static bool chromeVisibleProc(void* s) { return ((MediaPlayerPrivate *)s)->chromeVisible(); }
    static void* createHTMLMediaElementProc(void* s) { return ((MediaPlayerPrivate *)s)->createHTMLMediaElement(); }
    static void destroyHTMLMediaElementProc(void* s, void* p) { return ((MediaPlayerPrivate *)s)->destroyHTMLMediaElement(p); }
    bool chromeVisible();
    void* createHTMLMediaElement();
    void destroyHTMLMediaElement(void* p);
    bool canManipulatePlayer();
    void notifyVideoPlayerEnd();

private:
    MediaPlayer* m_player;
    void* m_peer;
    void* m_audiopeer;
    PlatformMedia m_platformMedia;

    bool m_resizeAtDrawing;
    IntSize m_size;

    Vector<httpConnection> m_httpConnections;
};

} // namespace WebCore

#endif// ENABLE(VIDEO)

#endif // _MEDIAPLAYERPRIVATEWKC_H_
