/*
 * Copyright (C) 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2011, 2012 Google Inc. All rights reserved.
 * Copyright (C) 2013 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
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
 */

#ifndef MediaControlRootElement_h
#define MediaControlRootElement_h

#if ENABLE(VIDEO)

#include "MediaControlElements.h"
#include "MediaControls.h"
#include <wtf/RefPtr.h>

namespace WebCore {

class HTMLInputElement;
class HTMLMediaElement;
class Event;
class MediaControlPanelMuteButtonElement;
class MediaControlPlayButtonElement;
class MediaControlSeekButtonElement;
class MediaControlRewindButtonElement;
class MediaControlReturnToRealtimeButtonElement;
class MediaControlToggleClosedCaptionsButtonElement;
class MediaControlCurrentTimeDisplayElement;
class MediaControlTimelineElement;
class MediaControlTimeRemainingDisplayElement;
class MediaControlVolumeSliderElement;
class MediaControlFullscreenButtonElement;
class MediaControlTimeDisplayElement;
class MediaControlStatusDisplayElement;
class MediaControlTimelineContainerElement;
class MediaControlSeekBackButtonElement;
class MediaControlSeekForwardButtonElement;
class MediaControlMuteButtonElement;
class MediaControlVolumeSliderElement;
class MediaControlVolumeSliderMuteButtonElement;
class MediaControlVolumeSliderContainerElement;
class MediaControlFullscreenVolumeMinButtonElement;
class MediaControlFullscreenVolumeSliderElement;
class MediaControlFullscreenVolumeMaxButtonElement;
class MediaControlPanelElement;
class MediaPlayer;

class RenderBox;
class RenderMedia;

#if ENABLE(VIDEO_TRACK)
class MediaControlTextTrackContainerElement;
class MediaControlTextTrackDisplayElement;
#endif

class MediaControlOverlayPlayButtonElement;


class MediaControlEnclosureElement : public MediaControlElement {
protected:
    explicit MediaControlEnclosureElement(Document*);

private:
    virtual MediaControlElementType displayType() const;
};


class MediaControlPanelEnclosureElement : public MediaControlEnclosureElement {
public:
    static PassRefPtr<MediaControlPanelEnclosureElement> create(Document*);

protected:
    explicit MediaControlPanelEnclosureElement(Document*);
    virtual const AtomicString& shadowPseudoId() const;
};


class MediaControlOverlayEnclosureElement : public MediaControlEnclosureElement {
public:
    static PassRefPtr<MediaControlOverlayEnclosureElement> create(Document*);

protected:
    explicit MediaControlOverlayEnclosureElement(Document*);
    virtual const AtomicString& shadowPseudoId() const;
};


class MediaControlRootElement : public MediaControls {
public:
    static PassRefPtr<MediaControlRootElement> create(Document*);

    // MediaControls implementation.
    void setMediaController(MediaControllerInterface*);

    void show();
    void hide();
    void makeOpaque();
    void makeTransparent();

    void reset();

    void playbackProgressed();
    void playbackStarted();
    void playbackStopped();

    void changedMute();
    void changedVolume();

    void enteredFullscreen();
    void exitedFullscreen();

    void reportedError();
    void loadedMetadata();
    void changedClosedCaptionsVisibility();

    void showVolumeSlider();
    void updateTimeDisplay();
    void updateStatusDisplay();

#if ENABLE(VIDEO_TRACK)
    void createTextTrackDisplay();
    void showTextTrackDisplay();
    void hideTextTrackDisplay();
    void updateTextTrackDisplay();
#endif

    virtual bool shouldHideControls();

    void bufferingProgressed();

private:
    MediaControlRootElement(Document*);

    virtual void defaultEventHandler(Event*);
    void hideControlsTimerFired(Timer<MediaControlRootElement>*);
    void startHideControlsTimer(bool overwriteFireTime);
    void stopHideControlsTimer();

    virtual const AtomicString& shadowPseudoId() const;

    bool containsRelatedTarget(Event*);

    MediaControllerInterface* m_mediaController;

    MediaControlRewindButtonElement* m_rewindButton;
    MediaControlPlayButtonElement* m_playButton;
    MediaControlReturnToRealtimeButtonElement* m_returnToRealTimeButton;
    MediaControlStatusDisplayElement* m_statusDisplay;
    MediaControlCurrentTimeDisplayElement* m_currentTimeDisplay;
    MediaControlTimelineElement* m_timeline;
    MediaControlTimeRemainingDisplayElement* m_timeRemainingDisplay;
    MediaControlTimelineContainerElement* m_timelineContainer;
    MediaControlSeekBackButtonElement* m_seekBackButton;
    MediaControlSeekForwardButtonElement* m_seekForwardButton;
    MediaControlToggleClosedCaptionsButtonElement* m_toggleClosedCaptionsButton;
    MediaControlPanelMuteButtonElement* m_panelMuteButton;
    MediaControlVolumeSliderElement* m_volumeSlider;
    MediaControlVolumeSliderMuteButtonElement* m_volumeSliderMuteButton;
    MediaControlVolumeSliderContainerElement* m_volumeSliderContainer;
    MediaControlFullscreenButtonElement* m_fullScreenButton;
    MediaControlFullscreenVolumeMinButtonElement* m_fullScreenMinVolumeButton;
    MediaControlFullscreenVolumeSliderElement* m_fullScreenVolumeSlider;
    MediaControlFullscreenVolumeMaxButtonElement* m_fullScreenMaxVolumeButton;
    MediaControlPanelElement* m_panel;
    MediaControlPanelEnclosureElement* m_enclosure;
#if ENABLE(VIDEO_TRACK)
    MediaControlTextTrackContainerElement* m_textDisplayContainer;
    MediaControlTextTrackDisplayElement* m_textTrackDisplay;
#endif
    Timer<MediaControlRootElement> m_hideControlsTimer;
    bool m_isMouseOverControls;
    bool m_isFullscreen;

    // Returns true if successful, otherwise return false.
    bool initializeControls(Document*);

    MediaControlOverlayPlayButtonElement* m_overlayPlayButton;
    MediaControlOverlayEnclosureElement* m_overlayEnclosure;

    bool m_playbackStartedOnce;
    bool m_opaque;
};

}

#endif

#endif
