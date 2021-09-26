/*
 * Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCHitTestResult.h"
#include "helpers/privates/WKCHitTestResultPrivate.h"

#include "HitTestResult.h"
#include "KURL.h"
#include "WTFString.h"

#include "helpers/privates/WKCElementPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCImagePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCScrollbarPrivate.h"

namespace WKC {

HitTestResultPrivateBase::HitTestResultPrivateBase()
    : m_innerNode(0)
    , m_innerNonSharedNode(0)
    , m_URLElement(0)
    , m_targetFrame(0)
    , m_image(0)
    , m_scrollbar(0)
{
}

HitTestResultPrivateBase::~HitTestResultPrivateBase()
{
    delete m_innerNode;
    delete m_innerNonSharedNode;
    delete m_URLElement;
    delete m_targetFrame;
    delete m_image;
    delete m_scrollbar;
}

HitTestResultPrivate::HitTestResultPrivate(const WebCore::HitTestResult& parent)
    : HitTestResultPrivateBase()
    , m_webcore(parent)
    , m_wkc(this)
{
}

HitTestResultPrivate::~HitTestResultPrivate()
{
}

HitTestResultPrivate::HitTestResultPrivate(const HitTestResultPrivate& other)
    : HitTestResultPrivateBase()
    , m_webcore(other.m_webcore)
    , m_wkc(this)
{
    if (this!=&other)
        ::memcpy(this, &other, sizeof(HitTestResultPrivate));
}

HitTestResultPrivate&
HitTestResultPrivate::operator =(const HitTestResultPrivate& other)
{
    if (this!=&other)
        ::memcpy(this, &other, sizeof(HitTestResultPrivate));
    return *this;
}

HitTestResultPrivateToCore::HitTestResultPrivateToCore(const HitTestResult& parent, const WKCPoint& pos)
    : HitTestResultPrivateBase()
    , m_wkc(parent)
    , m_webcore(new WebCore::HitTestResult(pos))
{
}

HitTestResultPrivateToCore::~HitTestResultPrivateToCore()
{
    delete m_webcore;
}

Node*
HitTestResultPrivateBase::innerNode()
{
    WebCore::Node* inode = webcore().innerNode();
    if (!inode)
        return 0;

    delete m_innerNode;
    m_innerNode = NodePrivate::create(inode);
    return &m_innerNode->wkc();
}

Node*
HitTestResultPrivateBase::innerNonSharedNode()
{
    WebCore::Node* inode = webcore().innerNonSharedNode();
    if (!inode)
        return 0;

    delete m_innerNonSharedNode;
    m_innerNonSharedNode = NodePrivate::create(inode);
    return &m_innerNonSharedNode->wkc();
}

WKCPoint
HitTestResultPrivateBase::point() const
{
    return webcore().point();
}

WKCPoint
HitTestResultPrivateBase::localPoint() const
{
    return webcore().localPoint();
}

Element*
HitTestResultPrivateBase::URLElement()
{
    WebCore::Element* element = webcore().URLElement();
    if (!element)
        return 0;

    delete m_URLElement;
    m_URLElement = new ElementPrivate(element);
    return &m_URLElement->wkc();
}

Scrollbar*
HitTestResultPrivateBase::scrollbar()
{
    WebCore::Scrollbar* bar = webcore().scrollbar();
    if (!bar)
        return 0;
    if (!m_scrollbar || m_scrollbar->webcore()!=bar) {
        delete m_scrollbar;
        m_scrollbar = new ScrollbarPrivate(bar);
    }
    return &m_scrollbar->wkc();
}

bool
HitTestResultPrivateBase::isOverWidget() const
{
    return webcore().isOverWidget();
}


Frame*
HitTestResultPrivateBase::targetFrame()
{
    WebCore::Frame* frame = webcore().targetFrame();
    if (!frame)
        return 0;

    delete m_targetFrame;
    m_targetFrame = new FramePrivate(frame);
    return &m_targetFrame->wkc();
}

bool
HitTestResultPrivateBase::isSelected() const
{
    return webcore().isSelected();
}

String
HitTestResultPrivateBase::spellingToolTip(TextDirection& dir) const
{
    WebCore::TextDirection wd;
    WTF::String s = webcore().spellingToolTip(wd);
    dir = (wd==WebCore::RTL) ? RTL : LTR;
    return s;
}

String
HitTestResultPrivateBase::replacedString() const
{
    return webcore().replacedString();
}

String
HitTestResultPrivateBase::title(TextDirection& dir) const
{
    WebCore::TextDirection wd;
    String s = webcore().title(wd);
    dir = (wd==WebCore::RTL) ? RTL : LTR;
    return s;
}

String
HitTestResultPrivateBase::altDisplayString() const
{
    return webcore().altDisplayString();
}

String
HitTestResultPrivateBase::titleDisplayString() const
{
    return webcore().titleDisplayString();
}

Image*
HitTestResultPrivateBase::image()
{
    WebCore::Image* img = webcore().image();
    if (!img)
        return 0;

    delete m_image;
    m_image = new ImagePrivate(img);
    return &m_image->wkc();
}

WKCRect
HitTestResultPrivateBase::imageRect() const
{
    return webcore().imageRect();
}

KURL
HitTestResultPrivateBase::absoluteImageURL() const
{
    return webcore().absoluteImageURL();
}

KURL
HitTestResultPrivateBase::absoluteMediaURL() const
{
    return webcore().absoluteMediaURL();
}

KURL
HitTestResultPrivateBase::absoluteLinkURL() const
{
    return webcore().absoluteLinkURL();
}

String
HitTestResultPrivateBase::textContent() const
{
    return webcore().textContent();
}

bool
HitTestResultPrivateBase::isLiveLink() const
{
    return webcore().isLiveLink();
}

bool
HitTestResultPrivateBase::isContentEditable() const
{
    return webcore().isContentEditable();
}

void
HitTestResultPrivateBase::toggleMediaControlsDisplay() const
{
    webcore().toggleMediaControlsDisplay();
}

void
HitTestResultPrivateBase::toggleMediaLoopPlayback() const
{
    webcore().toggleMediaLoopPlayback();
}

void
HitTestResultPrivateBase::enterFullscreenForVideo() const
{
    webcore().enterFullscreenForVideo();
}

bool
HitTestResultPrivateBase::mediaControlsEnabled() const
{
    return webcore().mediaControlsEnabled();
}

bool
HitTestResultPrivateBase::mediaLoopEnabled() const
{
    return webcore().mediaLoopEnabled();
}

bool
HitTestResultPrivateBase::mediaPlaying() const
{
    return webcore().mediaPlaying();
}

bool
HitTestResultPrivateBase::mediaSupportsFullscreen() const
{
    return webcore().mediaSupportsFullscreen();
}

void
HitTestResultPrivateBase::toggleMediaPlayState() const
{
    webcore().toggleMediaPlayState();
}

bool
HitTestResultPrivateBase::mediaHasAudio() const
{
    return webcore().mediaHasAudio();
}

bool
HitTestResultPrivateBase::mediaIsVideo() const
{
    return webcore().mediaIsVideo();
}

bool
HitTestResultPrivateBase::mediaMuted() const
{
    return webcore().mediaMuted();
}

void
HitTestResultPrivateBase::toggleMediaMuteState() const
{
    webcore().toggleMediaMuteState();
}

HitTestResult::HitTestResult(HitTestResultPrivate* parent)
    : m_private(parent)
    , m_owned(false)
{
}

HitTestResult::HitTestResult(const WKCPoint& pos)
    : m_private(reinterpret_cast<HitTestResultPrivate*>(new HitTestResultPrivateToCore(*this, pos)))
    , m_owned(true)
{
}

HitTestResult::~HitTestResult()
{
    if (m_owned)
        delete reinterpret_cast<HitTestResultPrivateToCore*>(m_private);
}

HitTestResult::HitTestResult(const HitTestResult& other)
    : m_private(other.m_private)
    , m_owned(false)
{
}

HitTestResult&
HitTestResult::operator=(const HitTestResult& other)
{
    if (this!=&other) {
        delete m_private;
        m_private = reinterpret_cast<HitTestResultPrivate*>(new HitTestResultPrivateToCore(*this, other.m_private->webcore().point()));
    }
    return *this;
}

Node*
HitTestResult::innerNode() const
{
    return m_private->innerNode();
}

Node*
HitTestResult::innerNonSharedNode() const
{
    return m_private->innerNonSharedNode();
}

WKCPoint
HitTestResult::point() const
{
    return m_private->point();
}

WKCPoint
HitTestResult::localPoint() const
{
    return m_private->localPoint();
}

Element*
HitTestResult::URLElement() const
{
    return m_private->URLElement();
}

Scrollbar*
HitTestResult::scrollbar() const
{
    return m_private->scrollbar();
}

bool
HitTestResult::isOverWidget() const
{
    return m_private->isOverWidget();
}

Frame*
HitTestResult::targetFrame() const
{
    return m_private->targetFrame();
}

bool
HitTestResult::isSelected() const
{
    return m_private->isSelected();
}

String
HitTestResult::spellingToolTip(TextDirection& dir) const
{
    return m_private->spellingToolTip(dir);
}

String
HitTestResult::replacedString() const
{
    return m_private->replacedString();
}

String
HitTestResult::title(TextDirection& dir) const
{
    return m_private->title(dir);
}

String
HitTestResult::altDisplayString() const
{
    return m_private->altDisplayString();
}

String
HitTestResult::titleDisplayString() const
{
    return m_private->titleDisplayString();
}

Image*
HitTestResult::image() const
{
    return m_private->image();
}

WKCRect
HitTestResult::imageRect() const
{
    return m_private->imageRect();
}

KURL
HitTestResult::absoluteImageURL() const
{
    return m_private->absoluteImageURL();
}

KURL
HitTestResult::absoluteMediaURL() const
{
    return m_private->absoluteMediaURL();
}

KURL
HitTestResult::absoluteLinkURL() const
{
    return m_private->absoluteLinkURL();
}

String
HitTestResult::textContent() const
{
    return m_private->textContent();
}

bool
HitTestResult::isLiveLink() const
{
    return m_private->isLiveLink();
}

bool
HitTestResult::isContentEditable() const
{
    return m_private->isContentEditable();
}

void
HitTestResult::toggleMediaControlsDisplay() const
{
    m_private->toggleMediaControlsDisplay();
}

void
HitTestResult::toggleMediaLoopPlayback() const
{
    m_private->toggleMediaLoopPlayback();
}

void
HitTestResult::enterFullscreenForVideo() const
{
    m_private->enterFullscreenForVideo();
}

bool
HitTestResult::mediaControlsEnabled() const
{
    return m_private->mediaControlsEnabled();
}

bool
HitTestResult::mediaLoopEnabled() const
{
    return m_private->mediaLoopEnabled();
}

bool
HitTestResult::mediaPlaying() const
{
    return m_private->mediaPlaying();
}

bool
HitTestResult::mediaSupportsFullscreen() const
{
    return m_private->mediaSupportsFullscreen();
}

void
HitTestResult::toggleMediaPlayState() const
{
    m_private->toggleMediaPlayState();
}

bool
HitTestResult::mediaHasAudio() const
{
    return m_private->mediaHasAudio();
}

bool
HitTestResult::mediaIsVideo() const
{
    return m_private->mediaIsVideo();
}

bool
HitTestResult::mediaMuted() const
{
    return m_private->mediaMuted();
}

void 
HitTestResult::toggleMediaMuteState() const
{
    m_private->toggleMediaMuteState();
}

} // namespace
