/*
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include "helpers/WKCCredential.h"
#include "helpers/privates/WKCCredentialPrivate.h"

#include "Credential.h"
#include "PlatformString.h"

#include "helpers/WKCString.h"

namespace WKC {
CredentialPrivate::CredentialPrivate(const Credential& cred)
    : m_wkc(cred)
    , m_webcore()
{
}

CredentialPrivate::CredentialPrivate(const Credential& cred, const String& user, const String& password, CredentialPersistence pers)
    : m_wkc(cred)
    , m_webcore(user, password, (WebCore::CredentialPersistence)pers)
{
}

CredentialPrivate::~CredentialPrivate()
{
}

Credential::Credential()
    : m_private(new CredentialPrivate(*this))
{
}

Credential::Credential(const String& user, const String& password, CredentialPersistence pers)
    : m_private(new CredentialPrivate(*this, user, password, pers))
{
}

Credential::~Credential()
{
    delete m_private;
}

Credential::Credential(const Credential& other)
    : m_private(new CredentialPrivate(*this, other.m_private->webcore().user(), other.m_private->webcore().password(), (CredentialPersistence)other.m_private->webcore().persistence()))
{
}

Credential&
Credential::operator=(const Credential& other)
{
    if (this==&other)
        return *this;

    delete m_private;
    m_private = new CredentialPrivate(*this, other.m_private->webcore().user(), other.m_private->webcore().password(), (CredentialPersistence)other.m_private->webcore().persistence());
    return *this;
}

} // namespace
