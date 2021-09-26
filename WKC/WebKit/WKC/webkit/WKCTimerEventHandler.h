/*
 *  WKCTimerEventHandler.h
 *
 *  Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCTimerEventHandler_h
#define WKCTimerEventHandler_h

// class definition

namespace WKC {

/*@{*/

/** @brief Class that notifies of timer-related events */
class WKCTimerEventHandler
{
public:
    // requests to call WKCWebKitWakeUp from browser thread.
    /**
       @brief Requests timer firing
       @param in_ms Time until timer wakes it up (ms)
       @param in_data Data to be used in in_proc argument
       @retval "!= false" Succeeded in setting timer
       @retval "== false" Failed to set timer
       @details
       Notifies of timer firing requests in order to call WKCWebKitWakeUp() after the time specified by in_ms.
       For in_ms==0xffffffff, it is treated as timer cancellation.
    */
    virtual bool requestWakeUp(unsigned int in_ms, void* in_data) = 0;
};

/*@}*/

} // namespace

#endif // WKCTimerEventHandler_h
