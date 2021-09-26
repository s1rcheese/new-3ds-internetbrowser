/*
 *  WKCMediaSkin.h
 *
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCMediaSkin_h
#define WKCMediaSkin_h

#include <wkc/wkcbase.h>

namespace WKC {

/*@{*/

/** @brief Media player skin type */
enum MediaSkinImage {
    /** @brief Background image */
    EMediaSkinImageBackground = 0,
    /** @brief Play button */
    EMediaSkinImagePlayButton,
    /** @brief Overlay play button */
    EMediaSkinImageOverlayPlayButton,
    /** @brief Pause button */
    EMediaSkinImagePauseButton,
    /** @brief Rewind button */
    EMediaSkinImageSeekBackButton,
    /** @brief Fast forward button */
    EMediaSkinImageSeekForwardButton,
    /** @brief Return to real time button */
    EMediaSkinImageReturnToRealTimeButton,
    /** @brief Closed-caption toggle button */
    EMediaSkinImageToggleClosedCaptionButton,
    /** @brief Reverse play button */
    EMediaSkinImageRewindButton,
    /** @brief Mute button */
    EMediaSkinImageMuteButton,
    /** @brief Volume button */
    EMediaSkinImageVolumeButton,
    /** @brief Full screen button */
    EMediaSkinImageFullScreenButton,
    /** @brief Background image for current time */
    EMediaSkinImageCurrentTimeBackground,
    /** @brief Background image for remaining time */
    EMediaSkinImageTimeRemainingBackground,
    /** @brief Play slider track */
    EMediaSkinImageSliderTrack,
    /** @brief Play slider */
    EMediaSkinImageSliderThumb,
    /** @brief Volume slider container */
    EMediaSkinImageVolumeSliderContainer,
    /** @brief Volume slider track */
    EMediaSkinImageVolumeSliderTrack,
    /** @brief Volume slider */
    EMediaSkinImageVolumeSliderThumb,
    /** @brief Number of media player skin types */
    EMediaSkinImages
};

/** @brief Media player state */
enum MediaSkinImageState {
    /** @brief Normal state */
    EMediaSkinImageStateNormal = 0,
    /** @brief Hover state */
    EMediaSkinImageStateHovered,
    /** @brief Button pressed state */
    EMediaSkinImageStatePressed,
    /** @brief Button disabled state */
    EMediaSkinImageStateDisabled,
    /** @brief Number of media player states */
    EMediaSkinImageStates
};

/** @brief Structure for storing each state of media player skin image data */
struct WKCMediaSkinImageItem_ {
    /** @brief points */
    WKCPoint fPoints[4];
    /**
       @brief bitmap image
       @details
       bitmap format: ARGB8888 @n
       (lsb) AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB (msb) @n
       rowbytes must be fSize.fWidth * 4 @n
    */
    void* fBitmap;
};
/** @brief Type definition of WKC::WKCMediaSkinImageItem */
typedef struct WKCMediaSkinImageItem_  WKCMediaSkinImageItem;

/** @brief Structure for storing collection of media player skin image data */
struct WKCMediaSkinImage_ {
    /** @brief Image size */
    WKCSize fSize;
    /**
       @brief WKC::WKCMediaSkinImageItem array that represents each state of a skin image
       @details
       0:normal, 1:hovered, 2:pressed, 3:disabled
    */
    WKCMediaSkinImageItem fImages[EMediaSkinImageStates];
};
/** @brief Type definition of WKC::WKCMediaSkinImage */
typedef struct WKCMediaSkinImage_ WKCMediaSkinImage;

/** @brief Structure for storing media player skin data */
struct WKCMediaSkin_ {
    /** @brief Array of structures for storing media player skin image data */
    WKCMediaSkinImage fImages[EMediaSkinImages];
    /** @brief Style sheet for media player */
    const char* fStyleSheet;
};
/** @brief Type definition of WKC::WKCMediaSkin */
typedef struct WKCMediaSkin_ WKCMediaSkin;
/*@}*/

} // namespace

#endif // WKCMediaSkin_h
