/*
 *  WKCLocalStorageUtil.cpp
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

#include "config.h"

#include "WKCLocalStorageUtil.h"

#include "StringImpl.h"

#include "StorageNamespace.h"
#include "StorageTracker.h"
#include "StorageTrackerClient.h"
#include "SecurityOrigin.h"

namespace WKC {

// LocalStorage tracker
WKC_DEFINE_GLOBAL_BOOL(gStorageTrackerInitialized, false);
WKC_DEFINE_GLOBAL_PTR(const LocalStorageTrackerCallbacks*, gStorageTrackerCallbacks, 0);

class StorageTrackerClientProc : public WebCore::StorageTrackerClient {
public:
    virtual void dispatchDidModifyOrigin(const WTF::String& originIdentifier);
    virtual void didFinishLoadingOrigins();
};

void
StorageTrackerClientProc::dispatchDidModifyOrigin(const WTF::String& originIdentifier)
{
    if(gStorageTrackerCallbacks){
        gStorageTrackerCallbacks->fDispatchDidModifyOriginProc(originIdentifier.utf8().data());
    }
}

void
StorageTrackerClientProc::didFinishLoadingOrigins()
{
    if(gStorageTrackerCallbacks){
        gStorageTrackerCallbacks->fDidFinishLoadingOriginsProc();
    }
}

static StorageTrackerClientProc gStorageTrackerClient;

void
WKCWebKitInitializeLocalStorageTracker(const char* path, const LocalStorageTrackerCallbacks* callbacks)
{
    if (!path || !callbacks) {
        return;
    }
    if (!gStorageTrackerInitialized) {
        gStorageTrackerCallbacks = callbacks;
        WebCore::StorageTracker::initializeTracker(WTF::String::fromUTF8(path), &gStorageTrackerClient);
        WebCore::StorageTracker::tracker(); // execute internal initialize on StorageTracker.
        gStorageTrackerInitialized = true;
    }
}

unsigned int
WKCWebKitGetLocalStorageOriginsNum()
{
    if (!gStorageTrackerInitialized) {
        return 0;
    }

    Vector<RefPtr<WebCore::SecurityOrigin> > coreOrigins;

    WebCore::StorageTracker::tracker().origins(coreOrigins);
    return coreOrigins.size();
}

int
WKCWebKitGetLocalStorageOrigin(int index, char* buf, int length)
{
    if (!gStorageTrackerInitialized) {
        return 0;
    }

    Vector<RefPtr<WebCore::SecurityOrigin> > coreOrigins;

    WebCore::StorageTracker::tracker().origins(coreOrigins);
    
    if (index >= coreOrigins.size() ){
        return 0;
    }

    WebCore::SecurityOrigin* origin = coreOrigins[index].get();

    if (!buf){
        return strlen(origin->databaseIdentifier().utf8().data());
    }
    
    strncpy(buf, origin->databaseIdentifier().utf8().data(), length-1);
    buf[length-1] = '\0';
    return strlen(buf);
}

long long
WKCWebKitGetLocalStorageDiskUsageForOrigin(const char* originIdentifier)
{
    if (!gStorageTrackerInitialized) {
        return 0;
    }

    RefPtr<WebCore::SecurityOrigin> origin = WebCore::SecurityOrigin::createFromDatabaseIdentifier(WTF::String::fromUTF8(originIdentifier));
    if (!origin)
        return 0;

    return WebCore::StorageTracker::tracker().diskUsageForOrigin(origin.get());
}

void
WKCWebKitDeleteLocalStorageOrigin(const char* originIdentifier)
{
    if (!gStorageTrackerInitialized) {
        return;
    }

    WebCore::StorageTracker::tracker().deleteOrigin(WTF::String::fromUTF8(originIdentifier));
}

void
WKCWebKitDeleteAllLocalStorageOrigins()
{
    if (!gStorageTrackerInitialized) {
        return;
    }

    WebCore::StorageTracker::tracker().deleteAllOrigins();
}

} // namespace
