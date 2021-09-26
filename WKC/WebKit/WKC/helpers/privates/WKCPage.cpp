/*
 * Copyright (c) 2011-2014 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCPage.h"
#include "helpers/privates/WKCPagePrivate.h"

#include "Page.h"

#include "helpers/privates/WKCBackForwardListPrivate.h"
#include "helpers/privates/WKCFocusControllerPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCHistoryItemPrivate.h"

namespace WKC {

PagePrivate::PagePrivate(WebCore::Page* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_mainFrame(0)
    , m_focusController(0)
    , m_backForwardList(0)
{
}

PagePrivate::~PagePrivate()
{
    delete m_mainFrame;
    delete m_focusController;
    delete m_backForwardList;
}
FocusController*
PagePrivate::focusController()
{
    if (!m_webcore)
        return 0;

    WebCore::FocusController* controller = m_webcore->focusController();
    if (!controller)
        return 0;

    if (!m_focusController || m_focusController->webcore()!=controller) {
        delete m_focusController;
        m_focusController = new FocusControllerPrivate(controller);
    }

    return &m_focusController->wkc();
}

BackForwardList*
PagePrivate::backForwardList()
{
    if (!m_webcore)
        return 0;

    WebCore::BackForwardList* bfl = m_webcore->backForwardList();
    if (!bfl)
        return 0;

    if (!m_backForwardList || m_backForwardList->webcore()!=bfl) {
        delete m_backForwardList;
        m_backForwardList = new BackForwardListPrivate(bfl);
    }

    return &m_backForwardList->wkc();
}


Frame*
PagePrivate::mainFrame()
{
    if (!m_webcore)
        return 0;

    WebCore::Frame* frame = m_webcore->mainFrame();
    if (!frame)
        return 0;

    if (!m_mainFrame || m_mainFrame->webcore()!=frame) {
        delete m_mainFrame;
        m_mainFrame = new FramePrivate(frame);
    }

    return &m_mainFrame->wkc();
}

void
PagePrivate::goToItem(HistoryItem* item, FrameLoadType type)
{
    if (!m_webcore)
        return;
    if (!item)
        return;

    m_webcore->goToItem(item->priv().webcore(), (WebCore::FrameLoadType)type);
}

void
PagePrivate::setGroupName(const String& name)
{
    if (!m_webcore)
        return;

    m_webcore->setGroupName(name);
}

const String&
PagePrivate::groupName()
{
    if (!m_webcore)
        return m_groupName;

    m_groupName = m_webcore->groupName();

    return m_groupName;
}

void
PagePrivate::setDeviceScaleFactor(float scaleFactor)
{
    if (!m_webcore)
        return;

    m_webcore->setDeviceScaleFactor(scaleFactor);
}

void
PagePrivate::clear()
{
    delete m_mainFrame;
    m_mainFrame = 0;
    delete m_focusController;
    m_focusController = 0;
    delete m_backForwardList;
    m_backForwardList = 0;
}

Page::Page(PagePrivate& parent)
    : m_private(parent)
{
}

Page::~Page()
{
}

FocusController*
Page::focusController() const
{
    return m_private.focusController();
}

BackForwardList*
Page::backForwardList() const
{
    return m_private.backForwardList();
}


Frame*
Page::mainFrame() const
{
    return m_private.mainFrame();
}

void
Page::goToItem(HistoryItem* item, FrameLoadType type)
{
    m_private.goToItem(item, type);
}

void
Page::setGroupName(const String& name)
{
    m_private.setGroupName(name);
}

const String&
Page::groupName() const
{
    return m_private.groupName();
}

void
Page::setDeviceScaleFactor(float scaleFactor)
{
    m_private.setDeviceScaleFactor(scaleFactor);
}

void
Page::clear()
{
    m_private.clear();
}


} // namespace

