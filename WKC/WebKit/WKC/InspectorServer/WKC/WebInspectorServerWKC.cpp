/*
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (c) 2012-2014 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */


#include "config.h"

#if ENABLE(INSPECTOR_SERVER)

#include "WebInspectorServer.h"

#include "WKCWebView.h"
#include "WebInspectorServerClient.h"
#include "MIMETypeRegistry.h"
#include "HTTPRequest.h"
#include <wtf/text/CString.h>
#include <wtf/text/StringBuilder.h>

//#ifdef __WKC_IMPLICIT_INCLUDE_SYSSTAT
#include <sys/stat.h>
//#endif

static char gResourcePathPrefix[1024] = {0};

namespace WKC {

void WebInspectorServer::setResourcePath(const char* in_path)
{
    if (!in_path)
        return;
    ::strncpy(gResourcePathPrefix, in_path, sizeof(gResourcePathPrefix)-2);
    int len = strlen(gResourcePathPrefix);
    if (gResourcePathPrefix[len-1]=='/' || gResourcePathPrefix[len-1]=='\\')
        gResourcePathPrefix[len-1] = 0;
}

bool WebInspectorServer::platformResourceForPath(const WTF::String& path, Vector<char>& data, WTF::String& contentType)
{
    // The page list contains an unformated list of pages that can be inspected with a link to open a session.
    if (path == "/pagelist.json") {
        buildPageList(data, contentType, false);
        return true;
    } else if (path == "/pagelist.js") {
        buildPageList(data, contentType, true);
        return true;
    }

    // Point the default path to a formatted page that queries the page list and display them.
    WTF::String localPath(gResourcePathPrefix);
    localPath += (path == "/") ? "/inspectorPageIndex.html" : path;
    // All other paths are mapped directly to a resource, if possible.
    void* fd;
    struct stat st;
    fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_APP/*TODO*/, localPath.utf8().data(), "rb");
    if (!fd)
        return false;
    if (wkcFileFStatPeer(fd, &st) < 0) {
        wkcFileFClosePeer(fd);
        return false;
    }
    data.grow(st.st_size);
    wkc_uint64 ofs = 0;
    while (ofs < st.st_size) {
        wkc_uint64 size;
        size = wkcFileFReadPeer(data.data() + ofs, 1, st.st_size - ofs, fd);
        if (size == 0) {
            wkcFileFClosePeer(fd);
            return false;
        }
        ofs += size;
    }
    wkcFileFClosePeer(fd);

    size_t extStart = localPath.reverseFind('.');
    WTF::String ext = localPath.substring(extStart != notFound ? extStart + 1 : 0);
    contentType = WebCore::MIMETypeRegistry::getMIMETypeForExtension(ext);
    return true;
}

void WebInspectorServer::buildPageList(Vector<char>& data, WTF::String& contentType, bool jsonp)
{
    StringBuilder builder;
    if (jsonp)
        builder.append("showPageList(");
    builder.append("[ ");
    ClientMap::iterator end = m_clientMap.end();
    for (ClientMap::iterator it = m_clientMap.begin(); it != end; ++it) {
        WKCWebView* webView = it->second->webView();
        if (it != m_clientMap.begin())
            builder.append(", ");
        builder.append("{ \"id\": " + WTF::String::number(it->first));
        builder.append(", \"title\": \"");
        builder.append(webView->title());
        builder.append("\", \"url\": \"");
        builder.append(webView->uri());
        builder.append("\", \"inspectorUrl\": \"");
        builder.append("inspector.html?page=" + WTF::String::number(it->first));
        builder.append("\" }");
    }
    builder.append(" ]");
    if (jsonp)
        builder.append(");");
    CString cstr = builder.toString().utf8();
    data.append(cstr.data(), cstr.length());
    contentType = "application/json; charset=utf-8";
}

}

#endif // ENABLE(INSPECTOR_SERVER)
