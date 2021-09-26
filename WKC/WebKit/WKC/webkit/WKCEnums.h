/*
 *  WKCEnums.h
 *
 *  Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCEnums_h
#define WKCEnums_h

#include "WKCEnumKeys.h"

namespace WKC {

/*@{*/

/** @brief Offscreen format type. */
enum OffscreenFormat {
    /** @brief Expresses one pixel in 5-bit red (R), 6-bit green (G), 5-bit blue (B), and 1-bit alpha from the top. */
    EOffscreenFormatRGBA5650 = 0,
    /** @brief Expresses one pixel in 8-bit alpha, 8-bit red (R), 8-bit green (G), and 8-bit blue (B) from the top. */
    EOffscreenFormatARGB8888,
    /** @brief reserved */
    EOffscreenFormatPolygon,  // OpenGL
    /** @brief Expresses one pixel in 0-bit alpha, 5-bit red (R), 6-bit green (G), and 5-bit blue (B) from the top.@n
        A cairo image surface in CAIRO_FORMAT_RGB16_565 is used.@n */
    EOffscreenFormatCairo16,
    /** @brief Expresses one pixel in 8-bit alpha, 8-bit red (R), 8-bit green (G), and 8-bit blue (B) from the top.@n
        A cairo image surface in CAIRO_FORMAT_ARGB32 is used.@n */
    EOffscreenFormatCairo32,
    /** @brief reserved */
    EOffscreenFormatCairoSurface,

    EOffscreenFormats,
};

/** @brief Key event type. */
enum KeyEventType {
    /** @brief Key press event. */
    EKeyEventPressed = 0,
    /** @brief Key release event. */
    EKeyEventReleased,
    /** @brief Character key press event. */
    EKeyEventChar,
    /** @brief Access key press event. */
    EKeyEventAccessKey,
    EKeyEvents
};

/** @brief Mouse event type. */
enum MouseEventType {
    /** @brief Mouse down event. */
    EMouseEventDown = 0,
    /** @brief Mouse up event. */
    EMouseEventUp,
    /** @brief Mouse move event. */
    EMouseEventMove,
    /** @brief Mouse drag event. */
    EMouseEventDrag,
    /** @brief Mouse double-click event. */
    EMouseEventDoubleClick,
    /** @brief Mouse long press event. */
    EMouseEventLongPressed,
    EMouseEvents
};

/** @brief Mouse button type. */
enum MouseButton {
    /** @brief No mouse button. */
    EMouseButtonNone = 0,
    /** @brief Left mouse button. */
    EMouseButtonLeft,
    /** @brief Middle mouse button. */
    EMouseButtonMiddle,
    /** @brief Right mouse button. */
    EMouseButtonRight,
    EMouseButtons
};

/** @brief Modifier key type. */
enum Modifier {
    /** @brief No modifier key. */
    EModifierNone  = 0x0000,
    /** @brief CTRL key. */
    EModifierCtrl  = 0x0001,
    /** @brief SHIFT key. */
    EModifierShift = 0x0002,
    /** @brief ALT key. */
    EModifierAlt   = 0x0004,
    /** @brief Modifier 1 key */
    EModifierMod1  = 0x0008,
    /** @brief Modifier 2 key */
    EModifierMod2  = 0x0010,
    /** @brief Modifier 3 key */
    EModifierMod3  = 0x0020,
    /** @brief Modifier 4 key */
    EModifierMod4  = 0x0040,
};

/** @brief Scroll type. */
enum ScrollType {
    /** @brief Scroll up. */
    EScrollUp = 0,
    /** @brief Scroll down. */
    EScrollDown,
    /** @brief Scroll left. */
    EScrollLeft,
    /** @brief Scroll right. */
    EScrollRight,
    /** @brief Scroll down. */
    EScrollPageUp,
    /** @brief Scroll up one screen. */
    EScrollPageDown,
    /** @brief Scroll down one screen. */
    EScrollPageLeft,
    /** @brief Scroll left one screen. */
    EScrollPageRight,
    /** @brief Scroll right one screen. */
    EScrollTop,
    /** @brief Scroll to top of page. */
    EScrollBottom,
    /** @brief Scroll to bottom of page. */
    EScrolls
};

/** @brief Page load status type. */
enum LoadStatus {
    /** @brief Page not loaded. */
    ELoadStatusNone = 0,
    /** @brief Start page loading. */
    ELoadStatusProvisional,
    /** @brief Get first data. */
    ELoadStatusCommitted,
    /** @brief Finished page loading. */
    ELoadStatusFinished,
    /** @brief Start first display of page. */
    ELoadStatusFirstVisual,
    /** @brief Page load error. */
    ELoadStatusFailed,
};

/** @brief Scroll bar setting type. */
enum ScrollbarMode {
    /** @brief Scroll bar set to On. */
    EScrollbarAlwaysOn = 0,
    /** @brief Scroll bar set to Off. */
    EScrollbarAlwaysOff,
    /** @brief Scroll bar set to Auto. */
    EScrollbarAuto
};

/** @brief Focus direction type. */
enum FocusDirection {
    /** @brief Upward. */
    EFocusDirectionUp = 0,
    /** @brief Downward. */
    EFocusDirectionDown,
    /** @brief Left direction. */
    EFocusDirectionLeft,
    /** @brief Right direction. */
    EFocusDirectionRight,
    EFocusDirections
};

/** @brief Cursor type. */
enum CursorType {
    /** @brief Pointer cursor. */
    ECursorTypePointer = 0,
    /** @brief Cross cursor. */
    ECursorTypeCross,
    /** @brief Hand cursor. */
    ECursorTypeHand,
    /** @brief IBeam cursor. */
    ECursorTypeIBeam,
    /** @brief Wait cursor. */
    ECursorTypeWait,
    /** @brief Help cursor. */
    ECursorTypeHelp,
    /** @brief East resize cursor. */
    ECursorTypeEastResize,
    /** @brief North resize cursor. */
    ECursorTypeNorthResize,
    /** @brief North-east resize cursor. */
    ECursorTypeNorthEastResize,
    /** @brief North-west resize cursor. */
    ECursorTypeNorthWestResize,
    /** @brief South resize cursor. */
    ECursorTypeSouthResize,
    /** @brief South-east resize cursor. */
    ECursorTypeSouthEastResize,
    /** @brief South-west resize cursor. */
    ECursorTypeSouthWestResize,
    /** @brief West resize cursor. */
    ECursorTypeWestResize,
    /** @brief North-south resize cursor. */
    ECursorTypeNorthSouthResize,
    /** @brief East-west resize cursor. */
    ECursorTypeEastWestResize,
    /** @brief North-east/South-west resize cursor. */
    ECursorTypeNorthEastSouthWestResize,
    /** @brief North-west/South-east resize cursor. */
    ECursorTypeNorthWestSouthEastResize,
    /** @brief Column-resize cursor. */
    ECursorTypeColumnResize,
    /** @brief Row-resize cursor. */
    ECursorTypeRowResize,
    /** @brief Middle panning cursor. */
    ECursorTypeMiddlePanning,
    /** @brief East panning cursor. */
    ECursorTypeEastPanning,
    /** @brief North panning cursor. */
    ECursorTypeNorthPanning,
    /** @brief North-east panning cursor. */
    ECursorTypeNorthEastPanning,
    /** @brief North-west panning cursor. */
    ECursorTypeNorthWestPanning,
    /** @brief South panning cursor. */
    ECursorTypeSouthPanning,
    /** @brief South-east panning cursor. */
    ECursorTypeSouthEastPanning,
    /** @brief South-west panning cursor. */
    ECursorTypeSouthWestPanning,
    /** @brief West panning cursor. */
    ECursorTypeWestPanning,
    /** @brief All direction move cursor. */
    ECursorTypeMove,
    /** @brief Cursor for vertical direction text. */
    ECursorTypeVerticalText,
    /** @brief Cell cursor. */
    ECursorTypeCell,
    /** @brief Context menu cursor. */
    ECursorTypeContextMenu,
    /** @brief Alias cursor. */
    ECursorTypeAlias,
    /** @brief Progress cursor. */
    ECursorTypeProgress, 
    /** @brief No-drop cursor. */
    ECursorTypeNoDrop,
    /** @brief Copy cursor. */
    ECursorTypeCopy,
    /** @brief Hides cursor. */
    ECursorTypeNone,
    /** @brief Do not allow cursor. */
    ECursorTypeNotAllowed,
    /** @brief Zoom in cursor. */
    ECursorTypeZoomIn,
    /** @brief Zoom out cursor. */
    ECursorTypeZoomOut,
    /** @brief Grab cursor. */
    ECursorTypeGrab,
    /** @brief Grabbing cursor. */
    ECursorTypeGrabbing,
    /** @brief Custom cursor. */
    ECursorTypeCustom,
    ECursorTypes
};

/** @brief Touch event type. */
enum TouchType {
    /** @brief Touch start event. */
    ETouchTypeStart = 0,
    /** @brief Move event. */
    ETouchTypeMove,
    /** @brief Touch end event. */
    ETouchTypeEnd,
    /** @brief Cancel event. */
    ETouchTypeCancel,
    ETouchTypes
};

/** @brief Touch states. */
enum TouchPointStates {
    /** @brief Pressed state. */
    ETouchPointStatePressed = 0,
    /** @brief Released state. */
    ETouchPointStateReleased,
    /** @brief Moved state. */
    ETouchPointStateMoved,
    /** @brief Stationary state. */
    ETouchPointStateStationary,
    /** @brief Canceled state. */
    ETouchPointStateCancelled,
    ETouchPointStates
};

/**
 * @enum ContentComposition
 * @brief Content composition.
 */
enum ContentComposition {
    /** @brief Error content composition. */
    EErrorContentComposition = -1,
    /** @brief Inclusion content composition. */
    EInclusionContentComposition = 0,
    /** @brief Sub-frame content composition. */
    ESubFrameRootContentComposition = 1,
    /** @brief Root Frame content composition. */
    ERootFrameRootContentComposition = 2,
    /** @brief Other content composition. */
    EOtherContentComposition = 3,
    EContentCompositionNum = 4 // no use
};

/**
 * @enum SSLHandshakeStatus
 * @brief SSL Handshake status
 */
enum SSLHandshakeStatus {
    /** @brief Handshake failed. */
    EHandshakeFail = -1,
    /** @brief Handshake initialized. */
    EHandshakeInitialize = 0,
    /** @brief Handshake established. */
    EHandshakeEstablished = 1,
    /** @brief Handshake succeeded. */
    EHandshakeSuccess = 2,
};

/**
 * @enum SSLSecureState
 * @brief SSL/TLS Seucre State
 */
enum SSLSecureState {
    ESecureStateRed,   /**< DANGER */
    ESecureStateWhite, /**< non-SSL */
    ESecureStateBlue,  /**< Normal SSL */
    ESecureStateGreen, /**< EV SSL */
    ESecureStates      /**< no use */
};

/**
 @enum SSLSecureLevel
 @brief SSL/TLS Seucrity Level
*/
enum SSLSecureLevel {
    ESecureLevelUnahthorized,  /**< Not Autherized */
    ESecureLevelInsecure,      /**< Insecure */
    ESecureLevelNonSSL,        /**< NO SSL/TLS */
    ESecureLevelSecure,        /**< SSL */
    ESecureLevels              /**< no use */
};

enum JSExtensionEvent {
    EImageViewStart = 0,
    EImageViewEnd,
    EImageViewChangeViewMode,
    EImageViewChangeContent,
    EImageViewError,
    EJSExtensionEvents
};

/*@}*/

} // namespace

#endif // WKCEnums_h
