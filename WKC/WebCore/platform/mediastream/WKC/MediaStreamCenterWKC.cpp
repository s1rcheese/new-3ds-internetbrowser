/*
 *  Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
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

#if ENABLE(MEDIA_STREAM)

#include "MediaStreamCenter.h"
#include "MediaStreamSourcesQueryClient.h"

namespace WebCore {

class MediaStreamCenterWKC : public MediaStreamCenter
{
public:
    MediaStreamCenterWKC();
    virtual ~MediaStreamCenterWKC();

    virtual void queryMediaStreamSources(PassRefPtr<MediaStreamSourcesQueryClient>);

    virtual void didSetMediaStreamTrackEnabled(MediaStreamDescriptor*, MediaStreamComponent*);
    virtual void didStopLocalMediaStream(MediaStreamDescriptor*);
    virtual void didConstructMediaStream(MediaStreamDescriptor*);

    virtual String constructSDP(IceCandidateDescriptor*);
    virtual String constructSDP(SessionDescriptionDescriptor*);
};

MediaStreamCenter&
MediaStreamCenter::instance()
{
    WKC_DEFINE_STATIC_PTR(MediaStreamCenter*, gInstance, new MediaStreamCenterWKC());
    return *gInstance;
}

MediaStreamCenterWKC::MediaStreamCenterWKC()
{
}

MediaStreamCenterWKC::~MediaStreamCenterWKC()
{
}

void
MediaStreamCenterWKC::queryMediaStreamSources(PassRefPtr<MediaStreamSourcesQueryClient> client)
{
    MediaStreamSourceVector audioSources, videoSources;
    client->didCompleteQuery(audioSources, videoSources);
}

void
MediaStreamCenterWKC::didSetMediaStreamTrackEnabled(MediaStreamDescriptor*, MediaStreamComponent*)
{
}

void
MediaStreamCenterWKC::didStopLocalMediaStream(MediaStreamDescriptor*)
{
}

void
MediaStreamCenterWKC::didConstructMediaStream(MediaStreamDescriptor*)
{
}

String
MediaStreamCenterWKC::constructSDP(IceCandidateDescriptor*)
{
    return "";
}

String
MediaStreamCenterWKC::constructSDP(SessionDescriptionDescriptor*)
{
    return "";
}

} // namespace

#endif
