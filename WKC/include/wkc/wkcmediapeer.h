/*
 *  wkcmediapeer.h
 *
 *  Copyright(c) 2011-2014 ACCESS CO., LTD. All rights reserved.
 */

#ifndef _WKC_MEDIA_PEER_H_
#define _WKC_MEDIA_PEER_H_

#include <wkc/wkcbase.h>
#include <wkc/wkcmedia.h>

// Ugh!: for testing. move to wkcconfig.h!
// 110525 ACCESS Co.,Ltd.

#define HAVE_SUPPORT_MEDIASTREAM_NULL 1
#define HAVE_SUPPORT_CONTAINER_NULL 1
#define HAVE_SUPPORT_AUDIO_NULL 1
#define HAVE_SUPPORT_VIDEO_NULL 1

//#define HAVE_SUPPORT_MEDIASTREAM_FILE 1
//#define HAVE_SUPPORT_MEDIASTREAM_HTTP 1
//#define HAVE_SUPPORT_CONTAINER_RIFFWAV 1
//#define HAVE_SUPPORT_AUDIO_WAV 1

//#define HAVE_SUPPORT_CONTAINER_MP3 1
//#define HAVE_SUPPORT_AUDIO_MP3 1

#ifdef WKC_USE_WEBM
# define HAVE_SUPPORT_CONTAINER_WEBM 1
# define HAVE_SUPPORT_CONTAINER_OGG 1
# define HAVE_SUPPORT_AUDIO_VORBIS 1
# define HAVE_SUPPORT_VIDEO_VP8 1
#endif

#ifdef WKC_USE_GSTREAMER
# define HAVE_SUPPORT_MEDIASTREAM_GSTREAMER 1
# define HAVE_SUPPORT_CONTAINER_GSTREAMER 1
# define HAVE_SUPPORT_AUDIO_GSTREAMER 1
# define HAVE_SUPPORT_VIDEO_GSTREAMER 1
#endif

#ifdef WKC_USE_NFMP
# define HAVE_SUPPORT_MEDIASTREAM_NFMP 1
# define HAVE_SUPPORT_CONTAINER_NFMP 1
# define HAVE_SUPPORT_AUDIO_NFMP 1
# define HAVE_SUPPORT_VIDEO_NFMP 1
#endif

/**
   @file
   @brief media (audio / video) playback and FFT related peers.
*/

/*@{*/

WKC_BEGIN_C_LINKAGE

/** @brief Structure that stores proxy information */
struct WKCMediaPlayerStreamProxyInfo_ {
    /** @brief Member that holds whether to use proxy */
    int fUse;
    /** @brief Member that holds host name */
    char fHost[256];
    /** @brief Member that holds port number */
    int fPort;
};
/** @brief Type definition of WKCMediaPlayerStreamProxyInfo */
typedef struct WKCMediaPlayerStreamProxyInfo_ WKCMediaPlayerStreamProxyInfo;

/** @brief Structure that stores stream information */
struct WKCMediaPlayerStreamStreamInfo_ {
    /** @brief Content length */
    long long fContentLength;
    /** @brief MIME-Type */
    const char* fMIMEType;
};
/** @brief Type definition of WKCMediaPlayerStreamStreamInfo */
typedef struct WKCMediaPlayerStreamStreamInfo_ WKCMediaPlayerStreamStreamInfo;

/**
@typedef void(*WKCMPSNotifyStreamInfoProc)(void* in_instance, const WKCMediaPlayerStreamStreamInfo* in_info)
@brief Type of function that notifies of stream info
@details
It must be called at receiving stream info.
*/
typedef void(*WKCMPSNotifyStreamInfoProc)(void* in_instance, const WKCMediaPlayerStreamStreamInfo* in_info);
/**
@typedef void(*WKCMPSNotifyStreamDataReceivedProc)(void* in_instance, const unsigned char* in_data, int in_len)
@brief Type of function that notifies of stream data reception state
@details
It must be called while receiving stream data.
*/
typedef void(*WKCMPSNotifyStreamDataReceivedProc)(void* in_instance, const unsigned char* in_data, int in_len);
/**
@typedef void(*WKCMPSNotifyStreamErrorProc)(void* in_instance, int in_error)
@brief Type of function that gives notification when an error occurs in stream
@details
Call it when an error occurs in stream.
*/
typedef void(*WKCMPSNotifyStreamErrorProc)(void* in_instance, int in_error);
/** @brief Structure that stores callback function for notifying media player of stream state */
struct WKCMediaPlayerStreamStateProcs_ {
    /** @brief Variable that stores instance */
    void* fInstance;
    /** @brief Member that holds WKCMPSNotifyStreamInfoProc */
    WKCMPSNotifyStreamInfoProc fInfoProc;
    /** @brief Member that holds WKCMPSNotifyStreamDataReceivedProc */
    WKCMPSNotifyStreamDataReceivedProc fDataReceivedProc;
    /** @brief Member that holds WKCMPSNotifyStreamErrorProc */
    WKCMPSNotifyStreamErrorProc fErrorProc;
};
/** @brief Type definition of WKCMediaPlayerStreamStateProcs */
typedef struct WKCMediaPlayerStreamStateProcs_ WKCMediaPlayerStreamStateProcs;

/**
@typedef int(*WKCMPSGetProxyInfoProc)(void* in_instance, WKCMediaPlayerStreamProxyInfo* out_info)
@brief Type of callback function that gets proxy information
@details
Call this function when getting proxy information.
*/
typedef int(*WKCMPSGetProxyInfoProc)(void* in_instance, WKCMediaPlayerStreamProxyInfo* out_info);
/**
@typedef int(*WKCMPSGetCookieProc)(void* in_instance, const char* in_uri, char* out_buf, int in_buflen)
@brief Type of callback function that gets cookies
@details
Call this function when getting cookies.
*/
typedef int(*WKCMPSGetCookieProc)(void* in_instance, const char* in_uri, char* out_buf, int in_buflen);

/**
@typedef int(*WKCMPSOpenURIProc)(void* in_instance, const char* in_uri, const WKCMediaPlayerStreamStateProcs* in_procs)
@brief Type of callback function that opens URI stream
@details
Call this function when opening the URI stream.
*/
typedef int(*WKCMPSOpenURIProc)(void* in_instance, const char* in_uri, const WKCMediaPlayerStreamStateProcs* in_procs);
/**
@typedef int(*WKCMPSCloseProc)(void* in_instance, int in_id)
@brief Type of callback function that closes stream
@details
Call this function when closing stream.
*/
typedef int(*WKCMPSCloseProc)(void* in_instance, int in_id);
/**
@typedef int(*WKCMPSCancelProc)(void* in_instance, int in_id)
@brief Type of callback function that cancels stream
@details
Call this function when canceling stream.
*/
typedef int(*WKCMPSCancelProc)(void* in_instance, int in_id);

/** @brief Structure that stores callback function that notifies media player of stream event */
struct WKCMediaPlayerStreamCallbacks_ {
    // for native protocol stacks
    /** @brief Member that holds WKCMPSGetProxyInfoProc */
    WKCMPSGetProxyInfoProc fGetProxyProc;
    /** @brief Member that holds WKCMPSGetCookieProc */
    WKCMPSGetCookieProc fGetCookieProc;

    // for using webkit stack
    /** @brief Member that holds WKCMPSOpenURIProc */
    WKCMPSOpenURIProc fOpenURIProc;
    /** @brief Member that holds WKCMPSCloseProc */
    WKCMPSCloseProc fCloseProc;
    /** @brief Member that holds WKCMPSCancelProc */
    WKCMPSCancelProc fCancelProc;
};
/** @brief Type definition of WKCMediaPlayerStreamCallbacks */
typedef struct WKCMediaPlayerStreamCallbacks_ WKCMediaPlayerStreamCallbacks;

WKC_PEER_API void wkcMediaPlayerCallbackSetPeer(const WKCMediaPlayerProcs* in_procs);

/**
@brief Initializes media player
@retval !=false Succeeded
@retval ==false Failed
@details
Write the necessary processes for initializing the media player.
*/
WKC_PEER_API bool wkcMediaPlayerInitializePeer(void);
/**
@brief Finalizes media player
@details
Write the necessary processes for finalizing the media player.
*/
WKC_PEER_API void wkcMediaPlayerFinalizePeer(void);
/**
@brief Forcibly terminates media player
@details
Write the necessary processes for forcibly terminating the media player.
*/
WKC_PEER_API void wkcMediaPlayerForceTerminatePeer(void);

/**
@brief Determines whether MIME type is supported
@param in_mimetype MIME type string
@param in_codec Codec string
@details
For MIME type represented by in_mimetype and in_codec, either of the following must be returned as a return value.
@retval WKC_MEDIA_SUPPORT_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_SUPPORT_SUPPORTED Supported
@retval WKC_MEDIA_SUPPORT_MAYBESUPPORTED Supported (not guaranteed)
*/
WKC_PEER_API int wkcMediaPlayerIsSupportedMIMETypePeer(const char* in_mimetype, const char* in_codec);
/**
@brief Gets number of supported MIME types
@return Number of supported MIME types
@details
The number of supported MIME types must be returned as a return value.
*/
WKC_PEER_API int wkcMediaPlayerSupportedMIMETypesPeer(void);
/**
@brief Gets string of supported MIME type
@param in_index MIME type index
@return MIME string
@details
A smaller amount of value than the value returned by wkcMediaPlayerSupportedMIMETypesPeer() is passed to in_index.@n
The MIME type string that supports individual in_index must be returned as a return value.
*/
WKC_PEER_API const char* wkcMediaPlayerSupportedMIMETypePeer(int in_index);

/**
@brief Gets strings used for displaying UI
@param in_type String type
@return strings used for displaying UI
@details
The UI string represented by in_type must be returned as a return value. The following value is passed to in_type:
@li WKC_MEDIA_UISTRING_LOADING Loading
@li WKC_MEDIA_UISTRING_BROADCAST Broadcast
@attention The Peer implementer must manage the memory that stores strings.
*/
WKC_PEER_API const char* wkcMediaPlayerGetUIStringPeer(int in_type);
/**
@brief Gets string used for displaying time
@param in_time Time (sec)
@return string used for displaying time
@details
String that represents time passed by in_time must be returned as a return value.
@attention The Peer implementer must manage the memory that stores strings.
*/
WKC_PEER_API const char* wkcMediaPlayerGetUIStringTimePeer(float in_time);

/**
@brief Generates media player
@param in_callbacks Callback for notification
@param in_scallbacks Callback for notifying of stream event
@param in_opaque Callback argument
@return Media player instance
@details
Write the generating process of the media player instance.@n
The media player instance specified for return value is passed as in_self of individual wkcMediaPlayerXXXXPeer().@n
For notifying the media player, in_opaque must be specified when calling the function stored in in_callbacks.
@attention The Peer implementer must not return NULL even if media player instance creation failed. The Peer should return some instances as dummy media player instances in these case.
*/
WKC_PEER_API void* wkcMediaPlayerCreatePeer(const WKCMediaPlayerCallbacks* in_callbacks, const WKCMediaPlayerStreamCallbacks* in_scallbacks, void* in_opaque);
/**
@brief Discards media player
@param in_self Media player instance
@details
Write the discard process of the media player instance.
*/
WKC_PEER_API void wkcMediaPlayerDeletePeer(void* in_self);

/**
@brief Loads content
@param in_self Media player instance
@param in_uri Content URI
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the load process of content.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerLoadPeer(void* in_self, const char* in_uri);
/**
@brief Cancels loading content
@param in_self Media player instance
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process for canceling the loading of content.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerCancelLoadPeer(void* in_self);
/**
@brief Prepares content playback
@param in_self Media player instance
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process before content playback.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerPrepareToPlayPeer(void* in_self);
/**
@brief Plays content
@param in_self Media player instance
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the playback process of content.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerPlayPeer(void* in_self);
/**
@brief Pauses content
@param in_self Media player instance
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the pause process of content.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerPausePeer(void* in_self);
/**
@brief Determines whether content is paused
@param in_self Media player instance
@retval !=false Paused
@retval ==false Not paused
@details
Whether content is paused must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerIsPausedPeer(void* in_self);

/**
@brief Determines whether video player is usable
@param in_self Media player instance
@retval !=false Video player is usable
@retval ==false No video player is usable
@details
Whether video player is usable must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerHasVideoPeer(void* in_self);
/**
@brief Determines whether audio player is usable
@param in_self Media player instance
@retval !=false Audio player is usable
@retval ==false No audio player is usable
@details
Whether audio player is usable must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerHasAudioPeer(void* in_self);
/**
@brief Determines whether media player is displayable in full screen
@param in_self Media player instance
@retval !=false Full screen display available
@retval ==false Full screen display not available
@details
Whether the media player is displayable in full screen must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerSupportsFullScreenPeer(void* in_self);
/**
@brief Determines whether file can be saved
@param in_self Media player instance
@retval !=false File can be saved
@retval ==false File cannot be saved
@details
Whether a file can be saved must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerSupportsSavePeer(void* in_self);
/**
@brief Determines whether closed caption is supported
@param in_self Media player instance
@retval !=false Closed caption is supported
@retval ==false Closed caption is not supported
@details
Whether close caption is supported must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerHasClosedCaptionsPeer(void* in_self);
/**
@brief Toggles closed caption display
@param in_self Media player instance
@param in_flag Display setting value
@li !=false Display
@li ==false Do not display
@retval WKC_MEDIA_ERROR_OK Success
@retval Other Fail
@details
Write the toggle process of closed caption display in accordance with in_flag.
*/
WKC_PEER_API int wkcMediaPlayerSetClosedCaptionsVisiblePeer(void* in_self, bool in_flag);
/**
@brief Determines whether poster attribute resource can be loaded
@param in_self Media player instance
@retval !=false OK
@retval ==false NG
@details
Whether resource written in the poster attribute of the video element is loaded must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerCanLoadPosterPeer(void* in_self);
/**
@brief Sets poster attribute resource
@param in_self Media player instance
@param in_poster Resource string specified by poster attribute
@retval WKC_MEDIA_ERROR_OK Success
@retval Other Fail
@details
Write the process where strings passed by in_poster are treated as the poster attribute.
*/
WKC_PEER_API int wkcMediaPlayerSetPosterPeer(void* in_self, const char* in_poster);
/**
@brief Sets autobuffer attribute
@param in_self Media player instance
@param in_flag Whether autobuffer attribute exists
@li !=false autobuffer attribute exists
@li ==false No autobuffer attribute exists
@retval WKC_MEDIA_ERROR_OK Success
@retval Other Fail
@details
Write the process where the autobuffer attribute is handled in accordance with the value of in_flag.
*/
WKC_PEER_API int wkcMediaPlayerSetAutoBufferPeer(void* in_self, bool in_flag);
/**
@brief Determines whether content is generated from single source
@param in_self Media player instance
@retval !=false Content resource is generated from single source
@retval ==false Content resource is not generated from single source
@details
The determination of whether resource included in content is generated from single source must be returned as a return value.
*/
WKC_PEER_API bool wkcMediaPlayerHasSingleSecurityOriginPeer(void* in_self);

/**
@brief Gets data rate
@param in_self Media player instance
@retval Data rate value
@details
* Currently not used so it is okay to implement an empty function.
*/
WKC_PEER_API int wkcMediaPlayerDataRatePeer(void* in_self);
/**
@brief Determines whether total size of content is known
@param in_self Media player instance
@retval !=false Known
@retval ==false Unknown
@details
Write the process for determining whether the size of content is obtained.
*/
WKC_PEER_API bool wkcMediaPlayerTotalBytesKnownPeer(void* in_self);
/**
@brief Gets total size of content
@param in_self Media player instance
@return Size of content (bytes)
@details
The total size of content must be returned as a return value.
*/
WKC_PEER_API unsigned int wkcMediaPlayerTotalBytesPeer(void* in_self);
/**
@brief Gets size of loaded content
@param in_self Media player instance
@return Size of loaded content (bytes)
@details
The size of loaded content must be returned as a return value.
*/
WKC_PEER_API unsigned int wkcMediaPlayerBytesLoadedPeer(void* in_self);

/**
@brief Gets natural size
@param in_self Media player instance
@param out_size Size
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
The natural size of the player must be set for out_size.
*/
WKC_PEER_API int wkcMediaPlayerNaturalSizePeer(void* in_self, WKCSize* out_size);
/**
@brief Sets size of media player
@param in_self Media player instance
@param in_size Size
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
The player must be set with the size specified by in_size.
*/
WKC_PEER_API int wkcMediaPlayerSetSizePeer(void* in_self, const WKCSize* in_size);
/**
@brief Sets media player visibility
@param in_self Media player instance
@param in_flag Visibility setting
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Set the player to visible if it is in_flag != false, and to invisible if it is in_flag == false.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerSetVisiblePeer(void* in_self, bool in_flag);

/**
@brief Gets content duration
@param in_self Media player instance
@return Content duration (seconds)
@details
The content duration (the duration attribute value) must be returned as a return value.
*/
WKC_PEER_API float wkcMediaPlayerDurationPeer(void* in_self);
/**
@brief Gets content playback start position
@param in_self Media player instance
@return Content playback start position (elapsed seconds from beginning)
@details
The time of the content playback start position from the beginning must be returned as a return value.
*/
WKC_PEER_API float wkcMediaPlayerStartTimePeer(void* in_self);
/**
@brief Gets current content position
@param in_self Media player instance
@return Current position of content (elapsed seconds from beginning)
@details
The time of the current content playback position from the beginning must be returned as a return value.
*/
WKC_PEER_API float wkcMediaPlayerCurrentTimePeer(void* in_self);
/**
@brief Gets the maximum seekable position of content
@param in_self Media player instance
@return Maximum seekable position
@details
The maximum seekable position of content must be returned as a return value.
*/
WKC_PEER_API float wkcMediaPlayerMaxTimeSeekablePeer(void* in_self);
/**
@brief Sets playback end position
@param in_self Media player instance
@param in_time End position
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process where the content playback end position is set to the position specified by in_time.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerSetEndTimePeer(void* in_self, float in_time);
/**
@brief Seeks content
@param in_self Media player instance
@param in_time Seek time
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the seeking operation process for the position represented by in_time from the beginning.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerSeekPeer(void* in_self, float in_time);
/**
@brief Determines whether content is being seeked
@param in_self Media player instance
@retval !=false Seeking
@retval ==false Not seeking
@details
Write the process for determining whether content is being seeked.
*/
WKC_PEER_API bool wkcMediaPlayerIsSeekingPeer(void* in_self);

/**
@brief Sets content playback speed factor
@param in_self Media player instance
@param in_rate Playback rate
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process for setting the playback speed factor specified by in_rate.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerSetRatePeer(void* in_self, float in_rate);
/**
@brief Sets audio playback pitch
@param in_self Media player instance
@param in_flag Sets audio playback pitch
@li in_flag!=false Saves normal pitch when playback speed factor changes
@li in_flag==false Does not save normal pitch when playback speed factor changes
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Set the audio playback pitch represented by in_flag.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerSetPreservesPitchPeer(void* in_self, bool in_flag);

/**
@brief Sets volume
@param in_self Media player instance
@param in_volume Volume (0 <= in_volume <= 1.0)
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process for setting the volume specified by in_volume.
@attention
Currently, there are no differences in operation by return value.
*/
WKC_PEER_API int wkcMediaPlayerSetVolumePeer(void* in_self, float in_volume);

/**
@brief Gets media player network state
@param in_self Media player instance
@retval WKC_MEDIA_NETWORKSTATE_IDLE Idle state
@retval WKC_MEDIA_NETWORKSTATE_LOADING Loading content
@retval WKC_MEDIA_NETWORKSTATE_LOADED Loaded content
@retval WKC_MEDIA_NETWORKSTATE_FORMATERROR Invalid content format
@retval WKC_MEDIA_NETWORKSTATE_NETWORKERROR Network state error
@retval WKC_MEDIA_NETWORKSTATE_DECODEERROR Failed to decode
@details
The appropriate network state of the media player must be returned as a return value.
*/
WKC_PEER_API int wkcMediaPlayerNetworkStatePeer(void* in_self);
/**
@brief Gets content hold state of media player
@param in_self Media player instance
@retval WKC_MEDIA_READYSTATE_HAVE_NOTHING Not held
@retval WKC_MEDIA_READYSTATE_HAVE_METADATA Meta data held
@retval WKC_MEDIA_READYSTATE_HAVE_CURRENTDATA Playback content until current position held
@retval WKC_MEDIA_READYSTATE_HAVE_FUTUREDATA Playback content from current position until some future position held
@retval WKC_MEDIA_READYSTATE_HAVE_ENOUGHDATA Assumed playback content from current position until last held
@details
The appropriate content hold state of the media player must be returned as a return value.@n
*/
WKC_PEER_API int wkcMediaPlayerReadyStatePeer(void* in_self);
/**
@brief Gets playback type of video content
@param in_self Media player instance
@retval WKC_MEDIA_MOVIELOADTYPE_UNKNOWN Unknown
@retval WKC_MEDIA_MOVIELOADTYPE_DOWNLOAD Download
@retval WKC_MEDIA_MOVIELOADTYPE_STOREDSTREAM Pseudo streaming
@retval WKC_MEDIA_MOVIELOADTYPE_LIVESTREAM Streaming
@details
The playback type of content must be returned as a return value.
*/
WKC_PEER_API int wkcMediaPlayerMovieLoadTypePeer(void* in_self);

/**
@brief Gives notification of forced redrawing
@param in_self Media player instance
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process for redrawing the whole media player regardless of the state.@n
* Currently not used so it is okay to implement an empty function.
*/
WKC_PEER_API int wkcMediaPlayerForceRepaintPeer(void* in_self);
/**
@brief Sets media player window position
@param in_self Media player instance
@param in_rect Window position
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process for setting the media player to the position specified by in_rect.@n
* Currently not used so it is okay to implement an empty function.
*/
WKC_PEER_API int wkcMediaPlayerSetWindowPositionPeer(void* in_self, const WKCRect* in_rect);
/**
@brief Gets video drawing area type
@param in_self Media player instance
@retval WKC_MEDIA_VIDEOSINKTYPE_BITMAP Bitmap buffer stream
@retval WKC_MEDIA_VIDEOSINKTYPE_WINDOW Inside separate window
@retval WKC_MEDIA_VIDEOSINKTYPE_HOLEDWINDOW Inside holed parent window
@details
The drawing area type of video content must be returned as a return value.@n
Drawing video is performed using wkcMediaPlayerLockImagePeer() on the browser engine side only when WKC_MEDIA_VIDEOSINKTYPE_BITMAP is returned. In other cases, the media player must draw video appropriately.
*/
WKC_PEER_API int wkcMediaPlayerVideoSinkTypePeer(void* in_self);
/**
@brief Gets video layer for accelerated compositing if supported
@param in_self Media player instance
@retval layer
@details
The layer should be the same type of instance created by wkcLayerNewPeer().
*/
WKC_PEER_API void* wkcMediaPlayerVideoLayerPeer(void* in_self);
/**
@brief Notifies video rendering state for accelerated-composited layer changed
@param in_self Media player instance
@details
Notifies video rendering state for accelerated-composited layer changed.
*/
WKC_PEER_API void wkcMediaPlayerNotifyVideoLayerRenderingStateChangedPeer(void* in_self);
/**
@brief Sets image format
@param in_self Media player instance
@param in_fmt Format
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
* Currently not used so it is okay to implement an empty function.
*/
WKC_PEER_API int wkcMediaPlayerSetPreferredImageFormatPeer(void* in_self, int in_fmt);

/**
@brief Gets number of buffered contents
@param in_self Media player instance
@return Number of content items
@details
The number of buffered content items must be returned as a return value.
*/
WKC_PEER_API int wkcMediaPlayerBufferedRangesPeer(void* in_self);
/**
@brief Gets buffered position of content
@param in_self Media player instance
@param in_index Index
@param out_start Start position
@param out_end End position
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@retval WKC_MEDIA_ERROR_NOTSUPPORTED Unsupported
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Set the buffered content positions in out_start and out_end.@n
The value corresponding to the value returned by wkcMediaPlayerBufferedRangesPeer() is passed to in_index.
*/
WKC_PEER_API int wkcMediaPlayerBufferedRangePeer(void* in_self, int in_index, float* out_start, float* out_end);

/**
@brief Locks drawing buffer
@param in_self Media player instance
@param out_fmt Format
@param out_rowbytes Number of bytes in horizontal direction of drawing buffer
@param out_mask Mask image
@param out_maskrowbytes Number of bytes in horizontal direction of mask image buffer
@param out_size Drawing buffer size
@return Pointer to drawing buffer
@details
Write the processes for locking the drawing buffer for reading data and returning a pointer to the buffer as a return value.@n
The out_fmt format must be specified using the following symbols:
@li WKC_IMAGETYPE_RGBA8888	0x00000000
@li WKC_IMAGETYPE_RGAB5515	0x00000001
@li WKC_IMAGETYPE_RGAB5515MASK	0x00000002
@li WKC_IMAGETYPE_ARGB1232	0x00000003
@li WKC_IMAGETYPE_TEXTURE	0x00000004
@li WKC_IMAGETYPE_TYPEMASK	0x0000ffff
@li WKC_IMAGETYPE_FLAG_HASALPHA	0x00010000
@li WKC_IMAGETYPE_FLAG_HASTRUEALPHA	0x00020000
@li WKC_IMAGETYPE_FLAG_FORSKIN	0x00100000
@li WKC_IMAGETYPE_FLAG_STEREO_M	0x00000000
@li WKC_IMAGETYPE_FLAG_STEREO_L	0x01000000
@li WKC_IMAGETYPE_FLAG_STEREO_R	0x02000000
@li WKC_IMAGETYPE_FLAG_STEREOMASK	0x0f000000
*/
WKC_PEER_API void* wkcMediaPlayerLockImagePeer(void* in_self, int* out_fmt, int* out_rowbytes, void** out_mask, int* out_maskrowbytes, WKCSize* out_size);
/**
@brief Unlocks drawing buffer
@param in_self Media player instance
@param image Pointer to drawing buffer
@details
Write the process for unlocking the drawing buffer specified by image.
*/
WKC_PEER_API void wkcMediaPlayerUnlockImagePeer(void* in_self, void* image);

/**
@brief Do something for when the media element is removed from a document.
@param in_self Media player instance
@details
Write a process for when the media element is removed from a document.
@attention
This call doesn't mean the media element was deleted.
The media element can be attached again to a document later by javascript.
*/
WKC_PEER_API void wkcMediaPlayerRemovedFromDocumentPeer(void* in_self);

WKC_PEER_API void wkcMediaPlayerWillBecomeInactivePeer(void* in_self);

WKC_PEER_API void wkcMediaPlayerEnterFullscreenPeer(void* in_self);

WKC_PEER_API void wkcMediaPlayerExitFullscreenPeer(void* in_self);

WKC_PEER_API bool wkcMediaPlayerCanEnterFullscreenPeer(void* in_self);

/**
@brief Gets output format of audio data
@param in_self Device descriptor
@retval WKC_MEDIA_AUDIOSINKTYPE_DIRECT Direct output
@retval WKC_MEDIA_AUDIOSINKTYPE_BINARYSTREAM Binary data
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@details
Write the process for returning output format.
*/
WKC_PEER_API int wkcMediaPlayerAudioSinkTypePeer(void* in_self);
/**
@brief Gets audio channel
@param in_self Device descriptor
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@retval Audio channel
@details
Write the process for returning audio channel.
*/
WKC_PEER_API int wkcMediaPlayerAudioChannelsPeer(void* in_self);
/**
@brief Gets audio rate
@param in_self Device descriptor
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@retval Audio rate
@details
Write the process for returning audio rate.
*/
WKC_PEER_API int wkcMediaPlayerAudioRatePeer(void* in_self);
/**
@brief Gets sampling rate
@param in_self Device descriptor
@param out_endian Endian
@retval WKC_MEDIA_ERROR_NOTREADY Preparation not complete
@retval Sampling rate
@details
Write the process for returning sampling rate.
*/
WKC_PEER_API int wkcMediaPlayerAudioBitsPerSamplePeer(void* in_self, int* out_endian);
/**
@brief Locks audio buffer
@param in_self Device descriptor
@param out_len Length of buffer
@retval Pointer to buffer
@details
Write the process for returning a pointer to the locked buffer.
*/
WKC_PEER_API void* wkcMediaPlayerLockAudioPeer(void* in_self, unsigned int* out_len);
/**
@brief Unlocks audio buffer
@param in_self Device descriptor
@param in_consumed Usage
@retval None
@details
Write the process for releasing the buffer locked by wkcMediaPlayerLockAudioPeer.
*/
WKC_PEER_API void wkcMediaPlayerUnlockAudioPeer(void* in_self, unsigned int in_consumed);

// for webAudio
/**
@brief Sets audio resource path
@param in_path Path of the storage of audio resources
@retval None
@details
Sets audio resource path.
*/
WKC_PEER_API void wkcMediaPlayerSetAudioResourcesPathPeer(const char* in_path);
/**
@brief Reads an audio resource
@param in_name Resource name
@param out_buf Buffer for read resource. If NULL, do nothing.
@param in_buflen Max length of out_buf
@retval Size of the resource
@details
For webAudio. Reads the audio resource specified in in_name.@n
At first, this peer will be called with out_buf==NULL, and this peer should return size of the specified resource.@n
Then, core will allocate buffer and call this function again. This peer should store the resource to out_buf.
*/
WKC_PEER_API int wkcMediaPlayerLoadPlatformAudioResourcePeer(const char* in_name, void* out_buf, int in_buflen);
/**
@brief Decodes encoded audio data.
@param in_data Encoded audio data
@param in_len Length of in_data
@param out_buf buffer for decoded data. If NULL, do nothing.
@param in_buflen Max length of out_buf
@param out_samplerate Sample rate of the decoded data
@param out_bitspersample Sample bits of the decoded data
@param out_channels channels of the decoded data
@param out_endian Endianness of the decoded data
@retval Size of decoded data
@details
For webAudio. Decodes encoded audio data on memory.@n
The calling sequences is same as wkcMediaPlayerLoadPlatformAudioResourcePeer.
*/
WKC_PEER_API int wkcMediaPlayerDecodeAudioDataPeer(const void* in_data, int in_len, void* out_buf, int in_buflen, int* out_samplerate, int* out_bitspersample, int* out_channels, int* out_endian);


// media skins

enum {
    WKC_IMAGE_MEDIA_BACKGROUND = 0,

    WKC_IMAGE_MEDIA_PLAY_BUTTON,
    WKC_IMAGE_MEDIA_OVERLAY_PLAY_BUTTON,
    WKC_IMAGE_MEDIA_PAUSE_BUTTON,
    WKC_IMAGE_MEDIA_SEEKBACK_BUTTON,
    WKC_IMAGE_MEDIA_SEEKFORWARD_BUTTON,
    WKC_IMAGE_MEDIA_RETURNTOREALTIME_BUTTON,
    WKC_IMAGE_MEDIA_TOGGLECLOSEDCAPTION_BUTTON,
    WKC_IMAGE_MEDIA_REWIND_BUTTON,
    WKC_IMAGE_MEDIA_MUTE_BUTTON,
    WKC_IMAGE_MEDIA_VOLUME_BUTTON,
    WKC_IMAGE_MEDIA_FULLSCREEN_BUTTON,

    WKC_IMAGE_MEDIA_CURRENTTIME_BACKGROUND,
    WKC_IMAGE_MEDIA_TIMEREMAINING_BACKGROUND,

    WKC_IMAGE_MEDIA_SLIDER_TRACK,
    WKC_IMAGE_MEDIA_SLIDER_THUMB,

    WKC_IMAGE_MEDIA_VOLUMESLIDER_CONTAINER,
    WKC_IMAGE_MEDIA_VOLUMESLIDER_TRACK,
    WKC_IMAGE_MEDIA_VOLUMESLIDER_THUMB,

    WKC_IMAGE_MEDIAS
};

enum {
    WKC_IMAGE_MEDIA_STATE_NORMAL = 0,
    WKC_IMAGE_MEDIA_STATE_HOVERED,
    WKC_IMAGE_MEDIA_STATE_PRESSED,
    WKC_IMAGE_MEDIA_STATE_DISABLED,
    WKC_IMAGE_MEDIA_STATES
};

/** @brief Media player skin image */
struct WKCMediaSkinImageItem_ {
    /** @brief Array of coordinate data for specifying four points within skin image */
    WKCPoint fPoints[4];
    /**
       @brief Pointer to image data of skin image
       @details
       The number of bytes in one line of image data must be (width x color depth) bytes.
    */
    const void* fBitmap;
};
/** @brief Type definition of WKCMediaSkinImageItem */
typedef struct WKCMediaSkinImageItem_ WKCMediaSkinImageItem;

/** @brief Structure that stores group of media player skin image data */
struct wkcMediaSkinImage_ {
    /** Size of media player skin image */
    WKCSize fSize;
    /** @brief Media player skin image */
    WKCMediaSkinImageItem fImages[WKC_IMAGE_MEDIA_STATES]; // 0:normal, 1:hovered, 2:pressed, 3:disabled
};
/** @brief Type definition of wkcMediaSkinImage */
typedef struct wkcMediaSkinImage_ wkcMediaSkinImage;

/** @brief Structure that stores media player skin data */
struct wkcMediaSkins_ {
    /**
       @brief Array of structure that stores media player skin image data
       @details
       fSkin must be accessed using the following symbols as indices:
       @li WKC_IMAGE_MEDIA_BACKGROUND Player background
       @li WKC_IMAGE_MEDIA_PLAY_BUTTON Play button 
       @li WKC_IMAGE_MEDIA_OVERLAY_PLAY_BUTTON Overlay play button 
       @li WKC_IMAGE_MEDIA_PAUSE_BUTTON Pause button
       @li WKC_IMAGE_MEDIA_SEEKBACK_BUTTON Rewind button
       @li WKC_IMAGE_MEDIA_SEEKFORWARD_BUTTON Fast-Forward button
       @li WKC_IMAGE_MEDIA_RETURNTOREALTIME_BUTTON 
       @li WKC_IMAGE_MEDIA_TOGGLECLOSEDCAPTION_BUTTON Subtitle toggle button
       @li WKC_IMAGE_MEDIA_REWIND_BUTTON Reverse Play button
       @li WKC_IMAGE_MEDIA_MUTE_BUTTON Mute button
       @li WKC_IMAGE_MEDIA_VOLUME_BUTTON Volume slider container open/close button
       @li WKC_IMAGE_MEDIA_FULLSCREEN_BUTTON Full screen toggle button
       @li WKC_IMAGE_MEDIA_CURRENTTIME_BACKGROUND Playback position background
       @li WKC_IMAGE_MEDIA_TIMEREMAINING_BACKGROUND  Remaining time display background
       @li WKC_IMAGE_MEDIA_SLIDER_TRACK Playback position slider
       @li WKC_IMAGE_MEDIA_SLIDER_THUMB Playback position slider marker
       @li WKC_IMAGE_MEDIA_VOLUMESLIDER_CONTAINER Volume slider container
       @li WKC_IMAGE_MEDIA_VOLUMESLIDER_TRACK Volume slider
       @li WKC_IMAGE_MEDIA_VOLUMESLIDER_THUMB Volume slider marker
    */
    wkcMediaSkinImage fSkins[WKC_IMAGE_MEDIAS];
    /** @brief Style sheet for media player */
    const char* fStyleSheet;
};
/** @brief Type definition of wkcMediaSkins */
typedef struct wkcMediaSkins_ wkcMediaSkins;

/**
@brief Sets skin image for media player
@param in_skin Structure for skin image
@retval !=false Succeeded
@retval ==false Failed
@details
The skin image structure specified by the application is passed to in_skin.@n
Write the process for using the structure for the skin image passed by the Peer (wkcMediaPlayerSkinXXXX) for getting skin images.
@attention
0(NULL) may be passed to in_skin. In this case, implement this function so that the appropriate image is returned as a return value by the Peer for getting skin images.
*/
WKC_PEER_API bool wkcMediaPlayerSkinRegisterSkinsPeer(const wkcMediaSkins* in_skin);
/**
@brief Gets size of skin image for media player
@param in_image_id Image type
@param out_width Image width
@param out_height Image height
@details
Set the width and height (pixels) of the skin image specified by in_image_id to out_width and out_height, respectively.@n
For information about the type of in_image_id, see the wkcMediaSkins::fSkins[WKC_IMAGE_MEDIAS] section.
*/
WKC_PEER_API void wkcMediaPlayerSkinGetSizePeer(int in_image_id, unsigned int *out_width, unsigned int *out_height);
/**
@brief Gets coordinate for resizing skin image for media player
@param in_image_id Image type
@param in_state State type
@retval "== NULL" Failed, There there is no resizing skin image
@retval "!= NULL" WKCPoint

@details
The coordinate data for resizing the skin image specified by in_image_id and in_state must be returned as a return value.@n
For information about the coordinate data, see the Skin section in "NetFront Browser NX API Reference".@n
For information about the type of in_image_id, see the wkcMediaSkins::fSkins[WKC_IMAGE_MEDIAS] section.
*/
WKC_PEER_API const WKCPoint* wkcMediaPlayerSkinGetLayoutPointsPeer(int in_image_id, int in_state);
/**
@brief Gets skin image for media player
@param in_image_id Image type
@param in_state State type
@retval "== NULL" Failed, or there is no image file
@retval "!= NULL" Scaned Image data
@details
The pointer to the skin image specified by in_image_id and in_state must be returned as a return value.@n
See  wkcMediaSkins::fSkins[WKC_IMAGE_MEDIAS] as a kind of in_image_id.
*/
WKC_PEER_API const unsigned char* wkcMediaPlayerSkinGetBitmapPeer(int in_image_id, int in_state);
/**
@brief Gets style sheet for media player
@retval "== NULL" Failed, or there is no the Style Sheet
@retval "!= NULL" Style Sheet String
@details
The style sheet string for the media player must be returned as a return value.
*/
WKC_PEER_API const char* wkcMediaPlayerSkinGetStyleSheetPeer(void);

/**
@brief Initializes audio device
@retval !=false Succeeded
@retval ==false Failed
@details
Write the necessary processes for initializing the audio device.
*/
WKC_PEER_API bool wkcAudioInitializePeer(void);
/**
@brief Finalizes audio device
@details
Write the necessary processes for finalizing the audio device.
*/
WKC_PEER_API void wkcAudioFinalizePeer(void);
/**
@brief Forcibly terminates audio device
@details
Write the necessary processes for forcibly terminating the audio device.
*/
WKC_PEER_API void wkcAudioForceTerminatePeer(void);

/**
@brief Opens audio device
@param in_samplerate Sampling rate
@param in_bitspersample Number of bits per sample
@param in_channels Number of channels
@param in_endian Byte endian
@li WKC_MEDIA_ENDIAN_LITTLEENDIAN Little endian
@li WKC_MEDIA_ENDIAN_BIGENDIAN Big endian
@li WKC_MEDIA_ENDIAN_MIDDLEENDIAN Middle endian
@return Device descriptor
@details
Write the process for opening the audio device with the conditions specified by the argument.@n
The return value is passed to in_self using wkcAudioXXXXPeer().
*/
WKC_PEER_API void* wkcAudioOpenPeer(int in_samplerate, int in_bitspersample, int in_channels, int in_endian);
/**
@brief Closes audio device
@param in_self Device descriptor
@details
Write the process for closing the audio device.
*/
WKC_PEER_API void wkcAudioClosePeer(void* in_self);
/**
@brief Sets audio device volume
@param in_self Device descriptor
@param in_volume Volume (0.0<=in_volume<=1.0)
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@details
Set the audio device volume to the value specified by in_volume.
*/
WKC_PEER_API int wkcAudioSetVolumePeer(void* in_self, float in_volume);
/**
@brief Gets audio device volume
@param in_self Device descriptor
@return Audio device volume
@details
The audio device volume must be returned as a return value within the range from 0.0 to 1.0.@n
Return a negative value if the process fails.
*/
WKC_PEER_API float wkcAudioGetVolumePeer(void* in_self);
/**
@brief Outputs to audio device
@param in_self Device descriptor
@param in_data Stream
@param in_len Stream length
@retval WKC_MEDIA_ERROR_OK Succeeded
@retval WKC_MEDIA_ERROR_GENERIC Failed
@details
Write the process for outputting the stream specified by in_data and in_len to the device.
*/
WKC_PEER_API int wkcAudioWritePeer(void* in_self, void* in_data, unsigned int in_len);
/**
@brief Returns preferred sample rate of audio device
@retval Preferred sample rate of audio device
@details
Returns preferred sample rate of audio device.
*/
WKC_PEER_API int wkcAudioPreferredSampleRatePeer(void);

/**
@brief Initializes FFT functionality
@details
Initializes FFT functionality.
*/
WKC_PEER_API void wkcAudioFFTInitializePeer(void);
/**
@brief Cleanups FFT functionality
@details
Cleanups FFT functionality.
*/
WKC_PEER_API void wkcAudioFFTCleanupPeer(void);
/**
@brief Creates FFT conversion functionality instance
@param in_size Size of FFT conversion
@retval Instance
@details
Creates FFT conversion functionality instance.
*/
WKC_PEER_API void* wkcAudioFFTCreatePeer(int in_size);
/**
@brief Deletes FFT conversion functionality instance
@param in_self Instance
@details
Deletes FFT conversion functionality instance.
*/
WKC_PEER_API void wkcAudioFFTDeletePeer(void* in_self);
/**
@brief Do FFT conversion
@param in_self Instance
@param in_data Input data
@param out_real Real part of FFT converted data
@param out_imag Imaginary part of FFT converted data
@details
Do FFT conversion.
*/
WKC_PEER_API void wkcAudioFFTDoFFTPeer(void* in_self, const float* in_data, float* out_real, float* out_imag);
/**
@brief Do inverse FFT conversion
@param in_self Instance
@param out_data Output data
@param in_real Real part of input data
@param in_imag Imaginary part of input data
@details
Do inverse FFT conversion.
*/
WKC_PEER_API void wkcAudioFFTDoInverseFFTPeer(void* in_self, float* out_data, const float* in_real, const float* in_imag);

WKC_END_C_LINKAGE

/*@}*/

#endif /* _WKC_MEDIA_PEER_H_ */
