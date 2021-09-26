/*
 *  WKCLocalStorageUtil.h
 *
 *  Copyright (c) 2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCLocalStorageUtil_h
#define WKCLocalStorageUtil_h

#include <wkc/wkcbase.h>

namespace WKC {

/*@{*/

typedef struct LocalStorageTrackerCallbacks_ {
    void (*fDispatchDidModifyOriginProc)(const char* originIdentifier);
    void (*fDidFinishLoadingOriginsProc)();
} LocalStorageTrackerCallbacks;

/**
@brief Initialize Localstorage Tracker.
@param path Path to save tracker database.
@param callbacks callback functions to notify from StorageTracker.
@retval None
@details
Initialize LocalStorage tracker.
*/
WKC_API void WKCWebKitInitializeLocalStorageTracker(const char* path, const LocalStorageTrackerCallbacks* callbacks);
/**
@brief Get number of LocalStorage origins.
@retval number of LocalStorage origins.
@details
Get number of LocalStorage origins.
*/
WKC_API unsigned int WKCWebKitGetLocalStorageOriginsNum();
/**
@brief Get LocalStorage Origin.
@param index Index of LocalStorage origin data.
@param buf Buffer of saving LocalStorage origin identifier string.
@param length Max length of buf.
@retval Length of origin identifier. (without "\0" terminate)
@details
Get LocalStorage origin identifier string of specified index.
If buf is sets NULL, it returns full length of origin identifier string (without "\0" terminator).
@attention
- size of buf is expected to length parameter.
- If buf size is short than origin identifier string, returned string is imperfectly.
*/
WKC_API int WKCWebKitGetLocalStorageOrigin(int index, char* buf, int length);
/**
@brief Get size of specified LocalStorage database file.
@param originIdentifier OriginIdentifier string.
@retval Size of database file.
@details
Get size of specified LocalStorage database file.
@attention
- originIdentifier must be termited by "\0".
*/
WKC_API long long WKCWebKitGetLocalStorageDiskUsageForOrigin(const char* originIdentifier);
/**
@brief Delete of specified LocalStorage database file.
@param originIdentifier OriginIdentifier string.
@retval None
@details
Delete specified LocalStorage database file.
@attention
- originIdentifier must be termited by "\0".
*/
WKC_API void WKCWebKitDeleteLocalStorageOrigin(const char* originIdentifier);
/**
@brief Delete all LocalStorage Database files.
@retval None
@details
Delete all LocalStorage Database files.
*/
WKC_API void WKCWebKitDeleteAllLocalStorageOrigins();

/*@}*/

} // namespace

#endif // WKCLocalStorageUtil_h
