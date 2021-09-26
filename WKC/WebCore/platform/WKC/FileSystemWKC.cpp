/*
 * Copyright (C) 2007, 2009 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora, Ltd.
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "FileSystem.h"
#include "AsyncFileSystem.h"

#include "PlatformString.h"
#include "CString.h"

#include "NotImplemented.h"

#ifdef __WKC_IMPLICIT_INCLUDE_SYSSTAT
#include <sys/stat.h>
#endif

namespace WebCore {

String filenameToString(const char* filename)
{
    notImplemented();
    return String();
}

char* filenameFromString(const String& string)
{
    notImplemented();
    return NULL;
}

// Converts a string to something suitable to be displayed to the user.
String filenameForDisplay(const String& string)
{
    notImplemented();
    return string;
}

bool fileExists(const String& path)
{
    void* fp = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "rb");
    if (!fp)
        return false;
    wkcFileFClosePeer(fp);
    return true;
}

bool deleteFile(const String& path)
{
    wkcFileUnlinkPeer(path.utf8().data());
    return true;
}

bool deleteEmptyDirectory(const String& path)
{
    notImplemented();
    return false;
}

bool getFileSize(const String& path, long long& resultSize)
{
    if (path.isNull() || path.isEmpty()) {
        return false;
    }

    struct stat st;
    int err;

    void* fp = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "rb");
    if (!fp)
        return false;

    err = wkcFileFStatPeer(fp, &st);
    wkcFileFClosePeer(fp);
    if (err == -1) {
        return false;
    }
    resultSize = st.st_size;

    return true;
}

bool getFileModificationTime(const String& path, time_t& modifiedTime)
{
    if (path.isNull() || path.isEmpty()) {
        return false;
    }

    struct stat st;
    int err;

    void* fp = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "rb");
    if (!fp)
        return false;

    err = wkcFileFStatPeer(fp, &st);
    wkcFileFClosePeer(fp);
    if (err == -1) {
        return false;
    }

    modifiedTime = st.st_mtime;

    return true;
}

String pathByAppendingComponent(const String& path, const String& component)
{
    char buf[MAX_PATH] = {0};

    int ret = wkcFilePathByAppendingComponentPeer(path.utf8().data(), component.utf8().data(), buf, MAX_PATH);
    if (!ret) {
        return String();
    }

    return String::fromUTF8(buf);
}

bool makeAllDirectories(const String& path)
{
    return wkcFileMakeAllDirectoriesPeer(path.utf8().data());
}

String homeDirectoryPath()
{
    char buf[MAX_PATH] = {0};

    int ret = wkcFileHomeDirectoryPathPeer(buf, MAX_PATH);
    if (!ret) {
        return String();
    }

    return String::fromUTF8(buf);
}

String pathGetFileName(const String& pathName)
{
    char buf[MAX_PATH] = {0};

    if (pathName.isEmpty() || pathName.isNull())
        return String();

    int ret = wkcFilePathGetFileNamePeer(pathName.utf8().data(), buf, MAX_PATH);

    if (!ret) {
        return String();
    }

    return String::fromUTF8(buf);
}

String directoryName(const String& path)
{
    char buf[MAX_PATH] = {0};

    int ret = wkcFileDirectoryNamePeer(path.utf8().data(), buf, MAX_PATH);
    if (!ret) {
        return String();
    }

    return String::fromUTF8(buf);
}

Vector<String> listDirectory(const String& path, const String& filter)
{
    Vector<String> entries;
    char name[MAX_PATH];
    char fullpath[MAX_PATH];
    void *dir;
    int ret;

    dir = wkcFileOpenDirectoryPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), filter.utf8().data());
    if (!dir)
        return entries;

    while (0 == wkcFileReadDirectoryPeer(dir, name, MAX_PATH)) {
        ret = wkcFilePathByAppendingComponentPeer(path.utf8().data(), name, fullpath, MAX_PATH);
        if (!ret)
            break;
        entries.append(String::fromUTF8(fullpath));
    }

    wkcFileCloseDirectoryPeer(dir);

    return entries;
}

String openTemporaryFile(const String& prefix, PlatformFileHandle& handle)
{
    char name[1024] = {0};
    void* fd = wkcFileOpenTemporaryFilePeer(prefix.utf8().data(), name, sizeof(name));
    if (!fd)
        goto error_end;
    handle = (PlatformFileHandle)reinterpret_cast<uintptr_t>(fd);
    return String::fromUTF8(name);

error_end:
    handle = 0;
    return String();
}

PlatformFileHandle openFile(const String& path, FileOpenMode mode)
{
    void* fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), mode==OpenForRead ? "r" : "w");
    if (!fd) {
        return invalidPlatformFileHandle;
    }
    return (PlatformFileHandle)reinterpret_cast<uintptr_t>(fd);
}

void closeFile(PlatformFileHandle& handle)
{
    if (handle==invalidPlatformFileHandle)
        return;
    void* fd = reinterpret_cast<void *>(handle);
    wkcFileFClosePeer(fd);
}

int writeToFile(PlatformFileHandle handle, const char* data, int length)
{
    if (handle==invalidPlatformFileHandle)
        return 0;
    void* fd = reinterpret_cast<void *>(handle);
    size_t ret = wkcFileFWritePeer(data, 1, length, fd);
    return ret;
}

int readFromFile(PlatformFileHandle handle, char* data, int length)
{
    if (handle==invalidPlatformFileHandle)
        return 0;
    void* fd = reinterpret_cast<void *>(handle);
    size_t ret = wkcFileFReadPeer(data, 1, length, fd);
    return ret;
}

long long seekFile(PlatformFileHandle handle, long long offset, FileSeekOrigin origin)
{
    if (handle==invalidPlatformFileHandle)
        return 0;
    void* fd = reinterpret_cast<void *>(handle);
    size_t pos = wkcFileFSeekPeer(fd, offset, origin);
    return pos;
}

#if 0
#if ENABLE(FILE_SYSTEM)
bool AsyncFileSystem::crackFileSystemURL(const KURL&, Type&, String& filePath)
{
    notImplemented();
    return false;
}
#endif // ENABLE(FILE_SYSTEM)
#endif

}
