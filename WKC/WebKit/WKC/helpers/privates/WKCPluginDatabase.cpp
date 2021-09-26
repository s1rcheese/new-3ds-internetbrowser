/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
#include "helpers/WKCPluginDatabase.h"
#include "helpers/privates/WKCPluginDatabasePrivate.h"

#include "helpers/WKCString.h"

#include "PluginDatabase.h"

namespace WKC {

PluginDatabasePrivate::PluginDatabasePrivate(WebCore::PluginDatabase* parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

PluginDatabasePrivate::~PluginDatabasePrivate()
{
}
    
PluginDatabasePrivate*
PluginDatabasePrivate::installedPlugins(bool populate)
{
    WebCore::PluginDatabase* parent = WebCore::PluginDatabase::installedPlugins(populate);
    DEFINE_STATIC_LOCAL(PluginDatabasePrivate, gInstalledPlugins, (parent));

    return &gInstalledPlugins;
}

bool
PluginDatabasePrivate::isMIMETypeRegistered(const String& mime)
{
    return m_webcore->isMIMETypeRegistered(mime);
}

PluginDatabase::PluginDatabase(PluginDatabasePrivate& priv)
    : m_private(priv)
{
}

PluginDatabase::~PluginDatabase()
{
}

PluginDatabase*
PluginDatabase::installedPlugins(bool populate)
{
    return &PluginDatabasePrivate::installedPlugins(populate)->wkc();
}

bool
PluginDatabase::isMIMETypeRegistered(const String& mime)
{
    return priv().isMIMETypeRegistered(mime);
}

} // namespace WKC
