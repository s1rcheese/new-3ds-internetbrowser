/*
 * Copyright (c) 2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_PRIVATE_POLICYCHECKER_H_
#define _WKC_HELPERS_PRIVATE_POLICYCHECKER_H_

#include "helpers/WKCPolicyChecker.h"

namespace WebCore { 
class PolicyChecker;
} // namespace

namespace WKC {

class PolicyCheckerWrap : public PolicyChecker {
public:
    PolicyCheckerWrap(PolicyCheckerPrivate& parent) : PolicyChecker(parent) {}
    ~PolicyCheckerWrap() {}
};

class PolicyCheckerPrivate {
public:
    PolicyCheckerPrivate(WebCore::PolicyChecker*);
    ~PolicyCheckerPrivate();

    WebCore::PolicyChecker* webcore() const { return m_webcore; }
    PolicyChecker& wkc() { return m_wkc; }

    FrameLoadType loadType() const;

private:
    WebCore::PolicyChecker* m_webcore;
    PolicyCheckerWrap m_wkc;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_POLICYCHECKER_H_

