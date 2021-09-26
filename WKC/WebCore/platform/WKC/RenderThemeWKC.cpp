/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
 *
 */
#include "config.h"
#include <stdio.h>

#include "CSSValueKeywords.h"
#include "GraphicsContext.h"
#include "RenderTheme.h"
#include "HTMLInputElement.h"
#include "HTMLMediaElement.h"
#include "HTMLSelectElement.h"
#include "Font.h"
#include "FontSelector.h"
#include "shadow/MediaControlElements.h"
#include "PaintInfo.h"
#include "FileList.h"
#include "StringTruncator.h"
#include "LocalizedStrings.h"
#include "TimeRanges.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>
#if ENABLE(VIDEO)
# include <wkc/wkcmediapeer.h>
#endif
#if ENABLE(PROGRESS_TAG)
# include "RenderProgress.h"
#endif
#if ENABLE(METER_TAG)
# include "RenderMeter.h"
# include "HTMLMeterElement.h"
#endif

namespace WebCore {

class RenderThemeWKC : public RenderTheme
{
public:
    RenderThemeWKC() : RenderTheme() { };
    virtual ~RenderThemeWKC();

public:
   static PassRefPtr<RenderTheme> create();

    // A method asking if the theme's controls actually care about redrawing when hovered.
    virtual bool supportsHover(const RenderStyle*) const { return true; }

    virtual bool supportsFocusRing(const RenderStyle*) const;

    virtual bool paintCheckbox(RenderObject*, const PaintInfo&, const IntRect&);
 
    virtual void setCheckboxSize(RenderStyle*) const;

    virtual bool paintRadio(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void setRadioSize(RenderStyle*) const;

    virtual void adjustRepaintRect(const RenderObject*, IntRect&);

    virtual void adjustButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintButton(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustTextFieldStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintTextField(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustTextAreaStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintTextArea(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchField(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldCancelButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldCancelButton(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldDecorationStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldDecoration(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldResultsDecorationStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldResultsDecoration(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldResultsButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldResultsButton(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSliderTrackStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSliderThumbStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSliderThumbSize(RenderStyle*) const;

    virtual int minimumMenuListSize(RenderStyle*) const;

    virtual void adjustMenuListStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintMenuList(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustMenuListButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintMenuListButton(RenderObject*, const PaintInfo&, const IntRect&);

    virtual bool isControlStyled(const RenderStyle*, const BorderData&,
                                 const FillLayer&, const Color&) const;

    virtual bool controlSupportsTints(const RenderObject*) const;

    virtual Color platformFocusRingColor() const;

    virtual void systemFont(int propId, FontDescription&) const;
    virtual Color systemColor(int cssValueId) const;

    virtual Color platformActiveSelectionBackgroundColor() const;
    virtual Color platformInactiveSelectionBackgroundColor() const;
    
    virtual Color platformActiveSelectionForegroundColor() const;
    virtual Color platformInactiveSelectionForegroundColor() const;

    virtual int popupInternalPaddingLeft(RenderStyle*) const;
    virtual int popupInternalPaddingRight(RenderStyle*) const;
    virtual int popupInternalPaddingTop(RenderStyle*) const;
    virtual int popupInternalPaddingBottom(RenderStyle*) const;

    // CSSs
    virtual String extraDefaultStyleSheet();
    virtual String extraQuirksStyleSheet();

#if ENABLE(VIDEO)
    // Media controls
    virtual bool supportsClosedCaptioning() const;
    virtual bool hasOwnDisabledStateHandlingFor(ControlPart) const;
    virtual bool usesMediaControlStatusDisplay();
    virtual bool usesMediaControlVolumeSlider() const;
    virtual double mediaControlsFadeInDuration();
    virtual double mediaControlsFadeOutDuration();
    virtual String formatMediaControlsTime(float time) const;
    virtual String formatMediaControlsCurrentTime(float currentTime, float duration) const;
    virtual String formatMediaControlsRemainingTime(float currentTime, float duration) const;
    
    // Returns the media volume slider container's offset from the mute button.
    virtual IntPoint volumeSliderOffsetFromMuteButton(RenderBox*, const IntSize&) const;

    virtual bool paintMediaFullscreenButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaPlayButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaOverlayPlayButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaMuteButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSeekBackButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSeekForwardButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderContainer(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaRewindButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaReturnToRealtimeButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaToggleClosedCaptionsButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaControlsBackground(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaCurrentTime(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaTimeRemaining(RenderObject*, const PaintInfo&, const IntRect&);
    virtual String extraMediaControlsStyleSheet();

    bool paintMediaButton(int index, WebCore::RenderObject* o, const PaintInfo& i, const IntRect& r, int state = WKC_IMAGE_MEDIA_STATE_NORMAL);
#endif

#if ENABLE(PROGRESS_TAG)
    virtual double animationRepeatIntervalForProgressBar(RenderProgress*) const;
    virtual double animationDurationForProgressBar(RenderProgress*) const;
    virtual void adjustProgressBarStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintProgressBar(RenderObject*, const PaintInfo&, const IntRect&);
#endif

#if ENABLE(METER_TAG)
    virtual bool supportsMeter(ControlPart) const { return true; }
    virtual void adjustMeterStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintMeter(RenderObject*, const PaintInfo&, const IntRect&);
#endif

    // disable spin button
    virtual bool shouldHaveSpinButton(HTMLInputElement*) const { return false; }

    virtual void adjustInnerSpinButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintInnerSpinButton(RenderObject*, const PaintInfo&, const IntRect&);

#if defined(__ghs__) || defined(__ARMCC_VERSION)
    virtual bool delegatesMenuListRendering() const { return true; }
#else
    virtual bool delegatesMenuListRendering() const { return false; }
#endif
    virtual bool popsMenuByArrowKeys() const { return false; }
    virtual bool popsMenuBySpaceOrReturn() const { return true; }

    virtual String fileListNameForWidth(const FileList*, const Font&, int width, bool multipleFilesAllowed) const;

    static void resetVariables();

private:
    void addIntrinsicMargins(RenderStyle*) const;
    void close();

    bool supportsFocus(ControlPart) const;
};

// parameters
// it should be in skins...

static const int cPopupInternalPaddingLeft   = 4;
static const int cPopupInternalPaddingRight  = 4;
static const int cPopupInternalPaddingTop    = 1;
static const int cPopupInternalPaddingBottom = 1;

// implementations

RenderThemeWKC::~RenderThemeWKC()
{
}

PassRefPtr<RenderTheme> RenderThemeWKC::create()
{
    return adoptRef(new RenderThemeWKC());
}

PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page* page)
{
    WKC_DEFINE_STATIC_PTR(RenderTheme*, gTheme, RenderThemeWKC::create().leakRef());
    return gTheme;
}

bool RenderThemeWKC::isControlStyled(const RenderStyle* style, const BorderData& border,
                                     const FillLayer& background, const Color& backgroundColor) const
{
    if (style->appearance() == TextFieldPart || style->appearance() == TextAreaPart || style->appearance() == ListboxPart)
        return style->border() != border;

    return RenderTheme::isControlStyled(style, border, background, backgroundColor);
}

void RenderThemeWKC::adjustRepaintRect(const RenderObject* o, IntRect& r)
{
    switch (o->style()->appearance()) {
        case MenulistPart: {
            r.setWidth(r.width() + 100);
            break;
        }
        default:
            break;
    }
}

bool RenderThemeWKC::controlSupportsTints(const RenderObject* o) const
{
    if (!isEnabled(o))
        return false;

    // Checkboxes only have tint when checked.
    if (o->style()->appearance() == CheckboxPart)
        return isChecked(o);

    // For now assume other controls have tint if enabled.
    return true;
}

void RenderThemeWKC::systemFont(int propId, FontDescription& fontDescription) const
{
    int type = 0;
    float size = 0.f;

    switch (propId) {
    case CSSValueCaption:
        type = WKC_SYSTEMFONT_TYPE_CAPTION; break;
    case CSSValueIcon:
        type = WKC_SYSTEMFONT_TYPE_ICON; break;
    case CSSValueMenu:
        type = WKC_SYSTEMFONT_TYPE_MENU; break;
    case CSSValueMessageBox:
        type = WKC_SYSTEMFONT_TYPE_MESSAGE_BOX; break;
    case CSSValueSmallCaption:
        type = WKC_SYSTEMFONT_TYPE_SMALL_CAPTION; break;
    case CSSValueWebkitMiniControl:
        type = WKC_SYSTEMFONT_TYPE_WEBKIT_MINI_CONTROL; break;
    case CSSValueWebkitSmallControl:
        type = WKC_SYSTEMFONT_TYPE_WEBKIT_SMALL_CONTROL; break;
    case CSSValueWebkitControl:
        type = WKC_SYSTEMFONT_TYPE_WEBKIT_CONTROL; break;
    case CSSValueStatusBar:
        type = WKC_SYSTEMFONT_TYPE_STATUS_BAR; break;
    default:
        return;
    }
    size = wkcStockImageGetSystemFontSizePeer(type);
    const char* familyName = wkcStockImageGetSystemFontFamilyNamePeer(type);
    if (size && familyName) {
        fontDescription.setSpecifiedSize(size);
        fontDescription.setIsAbsoluteSize(true);
        fontDescription.setGenericFamily(FontDescription::NoFamily);
        fontDescription.firstFamily().setFamily(familyName);
        fontDescription.setWeight(FontWeightNormal);
        fontDescription.setItalic(false);
    }
}

bool RenderThemeWKC::supportsFocusRing(const RenderStyle* style) const
{
    // no themes can support drawing focus-rings...
    return false;
}

Color RenderThemeWKC::systemColor(int cssValueId) const
{
    int id = 0;

    switch (cssValueId) {
    case CSSValueActiveborder:
        id = WKC_SKINCOLOR_ACTIVEBORDER; break;
    case CSSValueActivecaption:
        id = WKC_SKINCOLOR_ACTIVECAPTION; break;
    case CSSValueAppworkspace:
        id = WKC_SKINCOLOR_APPWORKSPACE; break;
    case CSSValueBackground:
        id = WKC_SKINCOLOR_BACKGROUND; break;
    case CSSValueButtonface:
        id = WKC_SKINCOLOR_BUTTONFACE; break;
    case CSSValueButtonhighlight:
        id = WKC_SKINCOLOR_BUTTONHIGHLIGHT; break;
    case CSSValueButtonshadow:
        id = WKC_SKINCOLOR_BUTTONSHADOW; break;
    case CSSValueButtontext:
        id = WKC_SKINCOLOR_BUTTONTEXT; break;
    case CSSValueCaptiontext:
        id = WKC_SKINCOLOR_CAPTIONTEXT; break;
    case CSSValueGraytext:
        id = WKC_SKINCOLOR_GRAYTEXT; break;
    case CSSValueHighlight:
        id = WKC_SKINCOLOR_HIGHLIGHT; break;
    case CSSValueHighlighttext:
        id = WKC_SKINCOLOR_HIGHLIGHTTEXT; break;
    case CSSValueInactiveborder:
        id = WKC_SKINCOLOR_INACTIVEBORDER; break;
    case CSSValueInactivecaption:
        id = WKC_SKINCOLOR_INACTIVECAPTION; break;
    case CSSValueInactivecaptiontext:
        id = WKC_SKINCOLOR_INACTIVECAPTIONTEXT; break;
    case CSSValueInfobackground:
        id = WKC_SKINCOLOR_INFOBACKGROUND; break;
    case CSSValueInfotext:
        id = WKC_SKINCOLOR_INFOTEXT; break;
    case CSSValueMenu:
        id = WKC_SKINCOLOR_MENU; break;
    case CSSValueMenutext:
        id = WKC_SKINCOLOR_MENUTEXT; break;
    case CSSValueScrollbar:
        id = WKC_SKINCOLOR_SCROLLBAR; break;
    case CSSValueText:
        id = WKC_SKINCOLOR_TEXT; break;
    case CSSValueThreeddarkshadow:
        id = WKC_SKINCOLOR_THREEDDARKSHADOW; break;
    case CSSValueThreedface:
        id = WKC_SKINCOLOR_THREEDFACE; break;
    case CSSValueThreedhighlight:
        id = WKC_SKINCOLOR_THREEDHIGHLIGHTA; break;
    case CSSValueThreedlightshadow:
        id = WKC_SKINCOLOR_THREEDLIGHTSHADOW; break;
    case CSSValueThreedshadow:
        id = WKC_SKINCOLOR_THREEDSHADOW; break;
    case CSSValueWindow:
        id = WKC_SKINCOLOR_WINDOW; break;
    case CSSValueWindowframe:
        id = WKC_SKINCOLOR_WINDOWFRAME; break;
    case CSSValueWindowtext:
        id = WKC_SKINCOLOR_WINDOWTEXT; break;
    default:
        return RenderTheme::systemColor(cssValueId);

    }
    return wkcStockImageGetSkinColorPeer(id);
}

Color RenderThemeWKC::platformFocusRingColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_FOCUSRING);
}

void RenderThemeWKC::addIntrinsicMargins(RenderStyle* style) const
{
    // Cut out the intrinsic margins completely if we end up using a small font size
    if (style->fontSize() < 11)
        return;

    // Intrinsic margin value.
    const int m = 2;

    // FIXME: Using width/height alone and not also dealing with min-width/max-width is flawed.
    if (style->width().isIntrinsicOrAuto()) {
        if (style->marginLeft().quirk())
            style->setMarginLeft(Length(m, Fixed));

        if (style->marginRight().quirk())
            style->setMarginRight(Length(m, Fixed));
    }

    if (style->height().isAuto()) {
        if (style->marginTop().quirk())
            style->setMarginTop(Length(m, Fixed));

        if (style->marginBottom().quirk())
            style->setMarginBottom(Length(m, Fixed));
    }
}

void RenderThemeWKC::setCheckboxSize(RenderStyle* style) const
{
    unsigned int w,dummy=0;
    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    wkcStockImageGetSizePeer (WKC_IMAGE_CHECKBOX_UNCHECKED, &w, &dummy);
    if (style->width().isIntrinsicOrAuto())
        style->setWidth(Length((int)w, Fixed));

    if (style->height().isAuto())
        style->setHeight(Length((int)w, Fixed));
}

static void
_bitblt(void* ctx, int type, void* bitmap, int rowbytes, void* mask, int maskrowbytes, const WKCFloatRect* srcrect, const WKCFloatRect* destrect, int op)
{
    WKCPeerImage img = {0};

    img.fType = type;
    img.fBitmap = bitmap;
    img.fRowBytes = rowbytes;
    img.fMask = mask;
    img.fMaskRowBytes = maskrowbytes;
    WKCFloatRect_SetRect(&img.fSrcRect, srcrect);
    WKCFloatSize_Set(&img.fScale, 1, 1);
    WKCFloatSize_Set(&img.fiScale, 1, 1);
    WKCFloatPoint_Set(&img.fPhase, 0, 0);
    WKCFloatSize_Set(&img.fiTransform, 1, 1);

    wkcDrawContextBitBltPeer(ctx, &img, destrect, op);
}
// If skin size is larger than rect, need to reduce skin image for fitting the area.
static void
calcSkinRect(unsigned int in_skinWidth, unsigned int in_skinHeight, const IntRect& in_r, WKCFloatRect& out_dest)
{
    if (in_r.width() < in_skinWidth || in_r.height() < in_skinHeight) {
        if (in_r.width() * in_skinHeight < in_r.height() * in_skinWidth) { // in_r.width() / in_skinWidth < in_r.height() / in_skinHeight
            out_dest.fWidth = in_r.width();
            out_dest.fHeight = (float)in_skinHeight * in_r.width() / in_skinWidth;
        } else {
            out_dest.fWidth = (float)in_skinWidth * in_r.height() / in_skinHeight;
            out_dest.fHeight = in_r.height();
        }
    } else {
        out_dest.fWidth = in_skinWidth;
        out_dest.fHeight = in_skinHeight;
    }
    // centering skin
    out_dest.fX = in_r.x() + (in_r.width() - out_dest.fWidth) / 2; 
    out_dest.fY = in_r.y() + (in_r.height() - out_dest.fHeight) / 2; 
}

bool RenderThemeWKC::paintCheckbox(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const unsigned char* image_buf;
    unsigned int rowbytes;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        if (this->isPressed(o))
            index = WKC_IMAGE_CHECKBOX_PRESSED;
        else if (this->isFocused(o) || this->isHovered(o))
              index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_FOCUSED : WKC_IMAGE_CHECKBOX_UNCHECKED_FOCUSED;
        else
            index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED : WKC_IMAGE_CHECKBOX_UNCHECKED;
    }
    else {
        index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_DISABLED : WKC_IMAGE_CHECKBOX_UNCHECKED_DISABLED;
    }
    
    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;

    rowbytes = width * 4;

    src.fX = 0; src.fY = 0;src.fWidth = (int) width; src.fHeight = (int) height;
    calcSkinRect(width, height, r, dest);
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::setRadioSize(RenderStyle* style) const
{
    // This is the same as checkboxes.
    setCheckboxSize(style);
}

bool RenderThemeWKC::paintRadio(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const unsigned char* image_buf;
    unsigned int rowbytes;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        if (this->isPressed(o))
              index = WKC_IMAGE_RADIO_PRESSED;
        else if (this->isFocused(o) || this->isHovered(o))
              index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_FOCUSED : WKC_IMAGE_RADIO_UNCHECKED_FOCUSED;
        else 
            index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED : WKC_IMAGE_RADIO_UNCHECKED;
    }
    else {
        index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_DISABLED : WKC_IMAGE_RADIO_UNCHECKED_DISABLED;
    }
    
    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;

    rowbytes = width * 4;

    src.fX = 0; src.fY = 0;src.fWidth = (int) width; src.fHeight = (int) height;
    calcSkinRect(width, height, r, dest);
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

bool RenderThemeWKC::supportsFocus(ControlPart part) const
{
    switch (part) {
    case PushButtonPart:
    case ButtonPart:
    case DefaultButtonPart:
    case RadioPart:
    case CheckboxPart:
    case TextFieldPart:
    case SearchFieldPart:
    case TextAreaPart:
        return true;
    default: // No for all others...
        return false;
    }
}

void RenderThemeWKC::adjustButtonStyle(StyleResolver* /*selector*/, RenderStyle* /*style*/, Element* /*e*/) const
{
}

static void drawScalingBitmapPeer(RenderObject* in_o, void* in_context, void* in_bitmap, int rowbytes, WKCSize *in_size, const WKCPoint *in_points, const WKCRect *in_destrect, int op)
{
    WKCFloatRect src, dest;

      // upper
    src.fX = in_points[0].fX; src.fY = 0; 
    src.fWidth = in_points[1].fX - in_points[0].fX;
    src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX + in_points[0].fX;
    dest.fY = in_destrect->fY;
    dest.fWidth = in_destrect->fWidth - in_points[0].fX - (in_size->fWidth - in_points[1].fX);
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // lower
    src.fX = in_points[2].fX; src.fY = in_points[2].fY;
    src.fWidth = in_points[3].fX - in_points[2].fX;
    src.fHeight = in_size->fHeight - in_points[2].fY;
    dest.fX = in_destrect->fX + in_points[2].fX;
    dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = in_destrect->fWidth - in_points[2].fX - (in_size->fWidth - in_points[3].fX);
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // left
    src.fX = 0; src.fY = in_points[0].fY;
    src.fWidth = in_points[0].fX;
    src.fHeight = in_points[2].fY - in_points[0].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY + in_points[0].fY;
    dest.fWidth = src.fWidth;
    dest.fHeight = in_destrect->fHeight - in_points[0].fY - (in_size->fHeight - in_points[2].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    //right
    src.fX = in_points[1].fX; src.fY = in_points[1].fY;
    src.fWidth = in_size->fWidth - in_points[1].fX;
    src.fHeight = in_points[3].fY - in_points[1].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth;
    dest.fY = in_destrect->fY + in_points[1].fY;
    dest.fWidth = src.fWidth;
    dest.fHeight = in_destrect->fHeight - in_points[1].fY - (in_size->fHeight - in_points[3].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // center
    src.fX = in_points[0].fX; src.fY = in_points[0].fY;
    src.fWidth = in_points[3].fX - in_points[0].fX;
    src.fHeight = in_points[3].fY - in_points[0].fY;
    dest.fX = in_destrect->fX + in_points[0].fX;
    dest.fY = in_destrect->fY + in_points[0].fY;
    dest.fWidth = in_destrect->fWidth - in_points[0].fX - (in_size->fWidth - in_points[3].fX);
    dest.fHeight = in_destrect->fHeight - in_points[0].fY - (in_size->fHeight - in_points[3].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0)) {
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);
    }

    // top left corner
    src.fX = 0; src.fY = 0; src.fWidth = in_points[0].fX; src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY; dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // top right
    src.fX = in_points[1].fX; src.fY = 0; src.fWidth = in_size->fWidth - in_points[1].fX; src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth; dest.fY = in_destrect->fY;
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // bottom left
    src.fX = 0; src.fY = in_points[2].fY; src.fWidth = in_points[2].fX; src.fHeight = in_size->fHeight - in_points[2].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // bottom right corner
    src.fX = in_points[3].fX; src.fY = in_points[3].fY;
    src.fWidth = in_size->fWidth - in_points[3].fX; src.fHeight = in_size->fHeight - in_points[3].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth;
    dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = src.fWidth;
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);
}

bool RenderThemeWKC::paintButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCSize img_size;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;
    
    if (!o)
          return false;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        index = WKC_IMAGE_BUTTON;
        if (this->isHovered(o) || this->isFocused(o))
              index = WKC_IMAGE_BUTTON_HOVERED;
        if (this->isPressed(o))
              index = WKC_IMAGE_BUTTON_PRESSED;
    }
    else {
        index = WKC_IMAGE_BUTTON_DISABLED;
    }

    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;
    
    img_size.fWidth = width;
    img_size.fHeight = height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (o, drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::adjustTextFieldStyle(StyleResolver* css, RenderStyle* style, Element* e) const
{
    if (style->hasBackgroundImage()) {
        style->resetBorder();
        const unsigned int defaultBorderColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER);
        const short defaultBorderWidth = 1;
        const EBorderStyle defaultBorderStyle = SOLID;
        style->setBorderLeftWidth(defaultBorderWidth);
        style->setBorderLeftStyle(defaultBorderStyle);
        style->setBorderLeftColor(defaultBorderColor);
        style->setBorderRightWidth(defaultBorderWidth);
        style->setBorderRightStyle(defaultBorderStyle);
        style->setBorderRightColor(defaultBorderColor);
        style->setBorderTopWidth(defaultBorderWidth);
        style->setBorderTopStyle(defaultBorderStyle);    
        style->setBorderTopColor(defaultBorderColor);
        style->setBorderBottomWidth(defaultBorderWidth);
        style->setBorderBottomStyle(defaultBorderStyle);
        style->setBorderBottomColor(defaultBorderColor);
   }
 
}

static void
_setBorder(GraphicsContext* context, const Color& color, float thickness)
{
    if (!context)
        return;

    context->setStrokeColor(color, context->strokeColorSpace());
    context->setStrokeThickness(thickness);
    context->setStrokeStyle(SolidStroke);
}

bool RenderThemeWKC::paintTextField(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    Color backgroundColor;
    const Color defaultBorderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER));
    bool ret = false;

    if (!o)
        return false;

    i.context->save();

    _setBorder(i.context, defaultBorderColor, 1.0);
    if (o->style()->hasBackground()) {
        backgroundColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    } else {
        backgroundColor = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BACKGROUND));
    }

    if (!o->style()->hasBackgroundImage()) {
        i.context->setFillColor(backgroundColor, i.context->fillColorSpace());
        i.context->drawRect(r);
    } else {
        ret = true;
    }

    i.context->restore();

    return ret;
}

void RenderThemeWKC::adjustTextAreaStyle(StyleResolver* css, RenderStyle* style, Element* e) const
{
    adjustTextFieldStyle(css, style, e);
}

bool RenderThemeWKC::paintTextArea(RenderObject* o, const PaintInfo& info, const IntRect& r)
{
    return paintTextField(o, info, r);
}

void RenderThemeWKC::adjustSearchFieldStyle(StyleResolver* css, RenderStyle* style, Element* e) const
{
    adjustTextFieldStyle(css, style, e);
}

bool RenderThemeWKC::paintSearchField(RenderObject* o, const PaintInfo& info, const IntRect& r)
{
    return paintTextField(o, info, r);
}

void RenderThemeWKC::adjustSearchFieldCancelButtonStyle(StyleResolver*, RenderStyle* style, Element*) const
{
    unsigned int width=0, height=0;
    wkcStockImageGetSizePeer(WKC_IMAGE_SEARCHFIELD_CANCELBUTTON, &width, &height);
    const IntSize size(width, height);
    style->setWidth(Length(size.width(), Fixed));
    style->setHeight(Length(size.height(), Fixed));
}

bool RenderThemeWKC::paintSearchFieldCancelButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCSize img_size;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;
    
    if (!o)
          return false;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = WKC_IMAGE_SEARCHFIELD_CANCELBUTTON;
    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;
    
    img_size.fWidth = width;
    img_size.fHeight = height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (o, drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::adjustSearchFieldResultsButtonStyle(StyleResolver* css, RenderStyle* style, Element* e) const
{
    unsigned int width=0, height=0;
    wkcStockImageGetSizePeer(WKC_IMAGE_SEARCHFIELD_RESULTBUTTON, &width, &height);
    const IntSize size(width, height);
    style->setWidth(Length(size.width(), Fixed));
    style->setHeight(Length(size.height(), Fixed));
}

bool RenderThemeWKC::paintSearchFieldResultsButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCSize img_size;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;
    
    if (!o)
          return false;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = WKC_IMAGE_SEARCHFIELD_RESULTBUTTON;
    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;
    
    img_size.fWidth = width;
    img_size.fHeight = height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (o, drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::adjustSearchFieldDecorationStyle(StyleResolver* css, RenderStyle* style, Element*) const
{
}

bool RenderThemeWKC::paintSearchFieldDecoration(RenderObject*, const PaintInfo&, const IntRect&)
{
    return false;
}

void RenderThemeWKC::adjustSearchFieldResultsDecorationStyle(StyleResolver* css, RenderStyle* style, Element*) const
{
}

bool RenderThemeWKC::paintSearchFieldResultsDecoration(RenderObject*, const PaintInfo&, const IntRect&)
{
    return true;
}

int RenderThemeWKC::minimumMenuListSize(RenderStyle* style) const 
{
    // same as safari.
    int fontsize = style->fontSize();
    if (fontsize >= 13) {
        return 9;
    } else if (fontsize >= 11) {
        return 5;
    }
    return 0;
}

void RenderThemeWKC::adjustMenuListStyle(StyleResolver* selector, RenderStyle* style, Element* e) const
{
    style->resetBorder();
    style->resetPadding();
    style->setHeight(Length(Auto));
    style->setWhiteSpace(PRE);
}

bool RenderThemeWKC::paintMenuList(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    paintTextField(o, i, r);
    return paintMenuListButton (o, i, r);
}

void RenderThemeWKC::adjustMenuListButtonStyle(StyleResolver* selector, RenderStyle* style, Element* e) const
{
    style->resetPadding();
    style->setLineHeight(RenderStyle::initialLineHeight());
}

bool RenderThemeWKC::paintMenuListButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    const int menulistPadding = 0;
    const int minimumHeight = 8;
    unsigned int rowbytes;
    int tmp_height;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        if (this->isPressed(o))
              index = WKC_IMAGE_MENU_LIST_BUTTON_PRESSED;
        else if (this->isFocused(o) || this->isHovered(o))
              index = WKC_IMAGE_MENU_LIST_BUTTON_FOCUSED;
        else
              index = WKC_IMAGE_MENU_LIST_BUTTON;
    }
    else {
        index = WKC_IMAGE_MENU_LIST_BUTTON_DISABLED;
    }

    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;

    rowbytes = width * 4;

    // center
    if (r.height() <= minimumHeight) { 
        src.fX = 0; src.fY = points[1].fY; 
        src.fWidth = (int) width; 
        src.fHeight = points[2].fY - points[1].fY;
        dest.fX = r.x() + r.width() - (int) width;
        dest.fY = r.y() + (r.height() - src.fHeight)/2;
        dest.fWidth = (int) width;
        dest.fHeight = src.fHeight;
        _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
        return false; // only paint the cneter
    }

    // upper + lower + center
    if (r.height() <= (points[3].fY - points[0].fY + 2 * menulistPadding)) {
          // upper
          src.fX = 0;
          src.fY = points[0].fY;
        src.fWidth = (int)width;
        src.fHeight = points[1].fY - points[0].fY;
        dest.fX = r.x() + r.width() - (int)width;
        dest.fY = r.y() + menulistPadding;
        dest.fWidth = (int)width;
        tmp_height = r.height() - (points[2].fY - points[1].fY);
        dest.fHeight = tmp_height / 2;
        if (dest.fHeight > 0)
            _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

        // center
        src.fY += src.fHeight;
        src.fHeight = points[2].fY - points[1].fY;
        dest.fY += dest.fHeight;
        dest.fHeight = src.fHeight;
        _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

        // lower
        src.fY += src.fHeight;
        src.fHeight = points[3].fY - points[2].fY;
        dest.fY += dest.fHeight;
        dest.fHeight = tmp_height / 2 + tmp_height % 2;
        if (dest.fHeight > 0)
            _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
        
        return false;
    }
    // paint all
    // top
    src.fX = 0; src.fY = 0;
    src.fWidth = (int) width;
    src.fHeight = points[0].fY;
    dest.fX = r.x() + r.width() - (int)width;
    dest.fY = r.y() + menulistPadding; 
    dest.fWidth = (int)width;
    dest.fHeight = src.fHeight; 
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    // upper
    src.fY += src.fHeight;
    src.fHeight = points[1].fY - points[0].fY;
    dest.fY += dest.fHeight;
    tmp_height = r.height() - ((int)height - points[3].fY + points[0].fY + points[2].fY - points[1].fY);
    dest.fHeight = tmp_height / 2;
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    // center
    src.fY += src.fHeight;
    src.fHeight = points[2].fY - points[1].fY;
    dest.fY += dest.fHeight;
    dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    // lower
    src.fY += src.fHeight;
    src.fHeight = points[3].fY - points[2].fY;
    dest.fY += dest.fHeight;
    dest.fHeight = tmp_height / 2 + tmp_height % 2;
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
    
    // bottom
    src.fY += src.fHeight;
    src.fHeight = (int) height - points[3].fY;
    dest.fY += dest.fHeight;
    dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}


void RenderThemeWKC::adjustSliderTrackStyle(StyleResolver* selector, RenderStyle* style, Element* element) const
{
    RenderTheme::adjustSliderTrackStyle(selector, style, element);
}

bool RenderThemeWKC::paintSliderTrack(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int width = r.width();
    int height = r.height();
    IntRect rect(r);
    Color backgroundColor;
    const Color defaultBorderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BORDER));

    if (!o)
        return false;

    if (o->style()->appearance() == SliderHorizontalPart) {
        rect.setHeight(4);
        rect.move(0, (height - rect.height())/2);
    } else if (o->style()->appearance() == SliderVerticalPart) {
        rect.setWidth(4);
        rect.move((width - rect.width())/2, 0);
    } else {
        return false;
    }

    i.context->save();

    _setBorder(i.context, defaultBorderColor, 1.0);
    if (!this->isEnabled(o)) {
        backgroundColor = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BACKGROUND_DISABLED));
    } else {
        backgroundColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    }
    i.context->setFillColor(backgroundColor, i.context->fillColorSpace());
    i.context->drawRect(rect);

    i.context->restore();

    return false;
}

void RenderThemeWKC::adjustSliderThumbStyle(StyleResolver* selector, RenderStyle* style, Element* e) const
{
    RenderTheme::adjustSliderThumbStyle(selector, style, e);
    style->setBoxShadow(nullptr);
}

bool RenderThemeWKC::paintSliderThumb(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    
    if (!o)
          return false;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (o->style()->appearance() == SliderThumbHorizontalPart) {
        if (this->isEnabled(o)) {
            index = WKC_IMAGE_H_RANGE;
            if (this->isHovered(o) || this->isFocused(o))
                index = WKC_IMAGE_H_RANGE_HOVERED;
            if (this->isPressed(o))
                index = WKC_IMAGE_H_RANGE_PRESSED;
        } else {
            index = WKC_IMAGE_H_RANGE_DISABLED;
        }
    } else if (o->style()->appearance() == SliderThumbVerticalPart) {
        if (this->isEnabled(o)) {
            index = WKC_IMAGE_V_RANGE;
            if (this->isHovered(o) || this->isFocused(o))
                index = WKC_IMAGE_V_RANGE_HOVERED;
            if (this->isPressed(o))
                index = WKC_IMAGE_V_RANGE_PRESSED;
        } else {
            index = WKC_IMAGE_V_RANGE_DISABLED;
        }
    } else {
        return false;
    }

    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    
    rowbytes = width * 4;

    src.fX = 0; src.fY = 0;src.fWidth = (int) width; src.fHeight = (int) height;
    dest.fX = r.x(); dest.fY = r.y() + (r.height() - src.fHeight) / 2; 
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::adjustSliderThumbSize(RenderStyle* style) const
{
    unsigned int width=0, height=0;
    if (style->appearance() == SliderThumbHorizontalPart) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_RANGE, &width, &height);
    } else if (style->appearance() == SliderThumbVerticalPart) {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_RANGE, &width, &height);
    } else if (style->appearance() == MediaSliderThumbPart) {
        wkcMediaPlayerSkinGetSizePeer(WKC_IMAGE_MEDIA_SLIDER_THUMB, &width, &height);
    } else {
        return;
    }

    style->setWidth(Length((int)width, Fixed));
    style->setHeight(Length((int)height, Fixed));
}

Color RenderThemeWKC::platformActiveSelectionBackgroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONBACKGROUND);
}

Color RenderThemeWKC::platformInactiveSelectionBackgroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONBACKGROUND);
}

Color RenderThemeWKC::platformActiveSelectionForegroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONFOREGROUND);
}

Color RenderThemeWKC::platformInactiveSelectionForegroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONFOREGROUND);
}

int RenderThemeWKC::popupInternalPaddingLeft(RenderStyle* style) const 
{ 
    return cPopupInternalPaddingLeft;
}

int RenderThemeWKC::popupInternalPaddingRight(RenderStyle* style) const 
{
    unsigned int w=0, h=0;
    wkcStockImageGetSizePeer(WKC_IMAGE_MENU_LIST_BUTTON, &w, &h);
    return w + cPopupInternalPaddingRight;
}

int RenderThemeWKC::popupInternalPaddingTop(RenderStyle* style) const 
{
    return cPopupInternalPaddingTop;
}

int RenderThemeWKC::popupInternalPaddingBottom(RenderStyle* style) const
{
    return cPopupInternalPaddingBottom;
}

typedef void (*ResolveFilenameForDisplayProc)(const unsigned short* path, const int path_len, unsigned short* out_path, int* out_path_len, const int path_maxlen);
WKC_DEFINE_GLOBAL_PTR(ResolveFilenameForDisplayProc, gResolveFilenameForDisplayProc, 0);

String RenderThemeWKC::fileListNameForWidth(const FileList* fileList, const Font& font, int width, bool multipleFilesAllowed) const
{
    if (width <= 0)
        return String();

    String str;
    unsigned short filenameBuffer[MAX_PATH] = {0};

    if (1 < fileList->length()) {
        str = multipleFileUploadText(fileList->length());
        return StringTruncator::rightTruncate(str, width, font);
    } else {
        if (gResolveFilenameForDisplayProc != 0) {
            String file;
            if (fileList->isEmpty()) {
                file = String();
            } else {
                file = fileList->item(0)->path();
            }
            int len = 0;
            int cur_len = file.length();
            (*gResolveFilenameForDisplayProc)(file.charactersWithNullTermination(), cur_len, filenameBuffer, &len, MAX_PATH);
            str = String(filenameBuffer, len);
        } else {
            if (fileList->isEmpty()) {
                str = fileButtonNoFileSelectedLabel();
            } else {
                str = fileList->item(0)->path();
            }
        }
        return StringTruncator::centerTruncate(str, width, font);
    }
}

void
RenderTheme_SetResolveFilenameForDisplayProc(WebCore::ResolveFilenameForDisplayProc proc)
{
    gResolveFilenameForDisplayProc = proc;
}

// CSSs
String RenderThemeWKC::extraDefaultStyleSheet()
{
    return String(wkcStockImageGetDefaultStyleSheetPeer());
}

String RenderThemeWKC::extraQuirksStyleSheet()
{
    return String(wkcStockImageGetQuirksStyleSheetPeer());
}

#if ENABLE(VIDEO)
    // Media controls
bool RenderThemeWKC::supportsClosedCaptioning() const
{
    return false;
}
bool RenderThemeWKC::hasOwnDisabledStateHandlingFor(ControlPart) const
{
    return false;
}
bool RenderThemeWKC::usesMediaControlStatusDisplay()
{
    return false;
}
bool RenderThemeWKC::usesMediaControlVolumeSlider() const
{
    return true;
}
double RenderThemeWKC::mediaControlsFadeInDuration() { return 0.1; }
double RenderThemeWKC::mediaControlsFadeOutDuration() { return 0.3; }

String RenderThemeWKC::formatMediaControlsTime(float in_time) const
{
    if (!isfinite(in_time))
        in_time = 0;
    int seconds = (int)fabsf(in_time);
    int hours = seconds / (60 * 60);
    int minutes = (seconds / 60) % 60;
    seconds %= 60;
    if (hours)
        return String::format("%s%01d:%02d:%02d", (in_time < 0 ? "-" : ""), hours, minutes, seconds);

    return String::format("%s%01d:%02d", (in_time < 0 ? "-" : ""), minutes, seconds);
}

String RenderThemeWKC::formatMediaControlsCurrentTime(float cur, float duration) const
{
    return RenderTheme::formatMediaControlsCurrentTime(cur, duration);
}

String RenderThemeWKC::formatMediaControlsRemainingTime(float cur, float duration) const
{
    return RenderTheme::formatMediaControlsRemainingTime(cur, duration);
}
    
// Returns the media volume slider container's offset from the mute button.
IntPoint RenderThemeWKC::volumeSliderOffsetFromMuteButton(RenderBox*, const IntSize&) const
{
    return IntPoint(0,0);
}

static void _fillRect(RenderObject* o, const PaintInfo& i, const IntRect& r, unsigned int color)
{
    Color backgroundColor;
    Color borderColor;

    if (!o)
        return;

    i.context->save();

    if (o->style()->hasBorder()) {
        borderColor = o->style()->visitedDependentColor(CSSPropertyBorderLeftColor);
        _setBorder(i.context, borderColor, 1.0);
    } else {
        _setBorder(i.context, 0x00000000, 0);
    }

    if (!o->style()->hasBackgroundImage()) {
        if (o->style()->hasBackground())
            backgroundColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor);
        else
            backgroundColor = color;
        i.context->setFillColor(backgroundColor, i.context->fillColorSpace());
        i.context->drawRect(r);
    }

    i.context->restore();
}

static bool hasSource(const HTMLMediaElement* m)
{
    return m->networkState() != HTMLMediaElement::NETWORK_EMPTY
        && m->networkState() != HTMLMediaElement::NETWORK_NO_SOURCE;
}

bool
RenderThemeWKC::paintMediaButton(int index, WebCore::RenderObject* o, const PaintInfo& i, const IntRect& r, int state)
{
    void *drawContext = 0;
    WKCSize img_size = {0};
    unsigned int width=0, height=0;
    const WKCPoint* points = 0;
    const unsigned char* image_buf = 0;
    unsigned int rowbytes = 0;
    WKCRect rect = {0};
    
    if (!o)
          return false;

    HTMLMediaElement* m = toParentMediaElement(o);

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    if (m && !hasSource(m)) {
        state = WKC_IMAGE_MEDIA_STATE_DISABLED;
        index = (index==WKC_IMAGE_MEDIA_PAUSE_BUTTON) ? WKC_IMAGE_MEDIA_PLAY_BUTTON : index;
    } else{
        if (isHovered(o) || isFocused(o)) {
            state = WKC_IMAGE_MEDIA_STATE_HOVERED;
        }
        if (isPressed(o)) {
            // force update even if hovered / focused is selected.
            state = WKC_IMAGE_MEDIA_STATE_PRESSED;
        }
    }

    image_buf = wkcMediaPlayerSkinGetBitmapPeer(index, state);
    if (!image_buf)
          return false;

    wkcMediaPlayerSkinGetSizePeer(index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcMediaPlayerSkinGetLayoutPointsPeer(index, state);
    if (!points)
          return false;
    
    img_size.fWidth = width;
    img_size.fHeight = height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer(o, drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return true;
}

bool RenderThemeWKC::paintMediaFullscreenButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int type = WKC_IMAGE_MEDIA_FULLSCREEN_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaPlayButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m)
        return false;

    int type = WKC_IMAGE_MEDIA_PAUSE_BUTTON;

    if (m->paused() || m->ended()) {
        type = WKC_IMAGE_MEDIA_PLAY_BUTTON;
    }
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaOverlayPlayButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m)
        return false;

    if (!m->paused() && !m->ended() && hasSource(m)) {
        // Overlay-play-button disappears if playback being started, not paused and not ended.
        // If video source is not found, disabled overlay-play-button will be painted in paintMediaButton.
        return false;
    }
    return paintMediaButton(WKC_IMAGE_MEDIA_OVERLAY_PLAY_BUTTON, o, i, r);
}

bool RenderThemeWKC::paintMediaMuteButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m)
        return false;

    int state = WKC_IMAGE_MEDIA_STATE_NORMAL;
    if (!m->hasAudio()) {
        state = WKC_IMAGE_MEDIA_STATE_DISABLED;
    }

    int type = WKC_IMAGE_MEDIA_MUTE_BUTTON;
    if (m->muted()) {
        type = WKC_IMAGE_MEDIA_VOLUME_BUTTON;
    }
    return paintMediaButton(type, o, i, r, state);
}

bool RenderThemeWKC::paintMediaSeekBackButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int type = WKC_IMAGE_MEDIA_SEEKBACK_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaSeekForwardButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int type = WKC_IMAGE_MEDIA_SEEKFORWARD_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaRewindButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{ 
    int type = WKC_IMAGE_MEDIA_REWIND_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaReturnToRealtimeButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int type = WKC_IMAGE_MEDIA_RETURNTOREALTIME_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaToggleClosedCaptionsButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int type = WKC_IMAGE_MEDIA_TOGGLECLOSEDCAPTION_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaSliderTrack(RenderObject* o, const PaintInfo& info, const IntRect& r)
{
    // This is the implementation that doesn't use the image of WKC_IMAGE_MEDIA_SLIDER_TRACK or any other resources.
    HTMLMediaElement* mediaElement = toParentMediaElement(o);
    if (!mediaElement)
        return false;

    float duration = mediaElement->duration();
    if (!duration)
        return false;
    float currentTime = mediaElement->currentTime();

    unsigned int thumbWidth=0, thumbHeight=0;
    wkcMediaPlayerSkinGetSizePeer(WKC_IMAGE_MEDIA_SLIDER_THUMB, &thumbWidth, &thumbHeight);

    // Adjust the current position to the center of the thumb.
    // The adjustment offset changes gradually from thumbWidth/2 to -thumbWidth/2 according to the playback progress.
    int offsetForSnappingToThumbCenter = (thumbWidth / 2) * (1.0 - 2.0 * currentTime / duration);
    int currentPosition = (currentTime / duration * r.width()) + offsetForSnappingToThumbCenter;

    IntRect backgroundRect(r);
    IntRect playedRect(r);
    IntRect bufferedRect(r);

    Color backgroundColor(0, 0, 0);
    Color playedColor(0, 196, 222);
    Color bufferedColor(0, 78, 89);

    int borderRadius = r.height() / 2;
    IntSize radii(borderRadius, borderRadius);

    info.context->save();

    // paint background rect
    if (borderRadius <= backgroundRect.width())
        info.context->fillRoundedRect(backgroundRect, radii, radii, radii, radii, backgroundColor, ColorSpaceDeviceRGB);

    // paint played rect
    playedRect.setWidth(currentPosition);
    if (borderRadius <= playedRect.width())
        info.context->fillRoundedRect(playedRect, radii, IntSize(0, 0), radii, IntSize(0, 0), playedColor, ColorSpaceDeviceRGB);

    // paint buffered rect
    RefPtr<TimeRanges> bufferedTimeRanges = mediaElement->buffered();
    for (unsigned i = 0; i < bufferedTimeRanges->length(); ++i) {
        float start = bufferedTimeRanges->start(i, ASSERT_NO_EXCEPTION);
        float end = bufferedTimeRanges->end(i, ASSERT_NO_EXCEPTION);
        if (start <= currentTime && currentTime <= end) {
            bufferedRect.move(currentPosition, 0);
            bufferedRect.setWidth((end / duration * r.width()) - currentPosition);
            if (borderRadius <= bufferedRect.width())
                info.context->fillRoundedRect(bufferedRect, IntSize(0, 0), radii, IntSize(0, 0), radii, bufferedColor, ColorSpaceDeviceRGB);
            break;
        }
    }

    info.context->restore();

    return true;
}

bool RenderThemeWKC::paintMediaSliderThumb(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    int type = WKC_IMAGE_MEDIA_SLIDER_THUMB;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaVolumeSliderContainer(RenderObject* o, const PaintInfo& i, const IntRect& r)
{ 
    _fillRect(o, i, r, 0x80888888);
    return false;
}

bool RenderThemeWKC::paintMediaVolumeSliderTrack(RenderObject* o, const PaintInfo& i, const IntRect& r)
{ 
    _fillRect(o, i, r, 0xff880088);
    return true;
}

bool RenderThemeWKC::paintMediaVolumeSliderThumb(RenderObject* o, const PaintInfo& i, const IntRect& r)
{ 
    _fillRect(o, i, r, 0xff888800);
    return true;
}

bool RenderThemeWKC::paintMediaControlsBackground(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    _fillRect(o, i, r, 0x00000000);
    return true;
}

bool RenderThemeWKC::paintMediaCurrentTime(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    _fillRect(o, i, r, 0x00000000);
    return true;
}

bool RenderThemeWKC::paintMediaTimeRemaining(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    _fillRect(o, i, r, 0x00000000);
    return true;
}

String RenderThemeWKC::extraMediaControlsStyleSheet()
{
    const char* css = wkcMediaPlayerSkinGetStyleSheetPeer();
    if (css) {
        return String(css);
    } else {
        return String();
    }
}
#endif

#if ENABLE(PROGRESS_TAG)
// Ugh!: it should be in skin parameter!
// 111005 ACCESS Co.,Ltd.
static const double gFrameRateForProgressBarAnimation = 0.033;
static const double gDurationForProgressBarAnimation = 2.0;

double RenderThemeWKC::animationRepeatIntervalForProgressBar(RenderProgress* p) const
{
    return gFrameRateForProgressBarAnimation;
}

double RenderThemeWKC::animationDurationForProgressBar(RenderProgress* p) const
{
    if (p->isDeterminate())
        return gFrameRateForProgressBarAnimation;
    return gDurationForProgressBarAnimation;
}

void RenderThemeWKC::adjustProgressBarStyle(StyleResolver*, RenderStyle*, Element*) const
{
}

bool RenderThemeWKC::paintProgressBar(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    if (!o)
        return true;
    if (!o->isProgress())
        return true;

    Color backgroundColor;
    const Color defaultBorderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BORDER));
    const Color bodyColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BODY));

    i.context->save();

    _setBorder(i.context, defaultBorderColor, 1.0);
    if (!this->isEnabled(o)) {
        backgroundColor = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BACKGROUND_DISABLED));
    } else if (o->style()->hasBackground()) {
        backgroundColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    } else {
        backgroundColor = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BACKGROUND));
    }
    if (!o->style()->hasBackgroundImage()) {
        i.context->setFillColor(backgroundColor, i.context->fillColorSpace());
        i.context->drawRect(r);
    }

    RenderProgress* renderProgress = toRenderProgress(o);
    IntRect vr = IntRect(0,0,0,0);
    IntRect vrr = IntRect(0,0,0,0);
    if (renderProgress->isDeterminate()) {
        vr = r;
        vr.expand(-2,-2);
        vr.move(1,1);
        int w = vr.width() * renderProgress->position();
        vr.setWidth(w);
    } else {
        vr = r;
        vr.expand(-2,-2);
        int mx = vr.maxX() - 1;
        int x = vr.width() * renderProgress->animationProgress();
        vr.move(x, 1);
        int w = vr.width() * 0.25;
        vr.setWidth(w);
        if (vr.maxX() > mx) {
            vrr = r;
            vrr.expand(-2,-2);
            vrr.move(1,1);
            vrr.setWidth(vr.maxX() - mx);
            vr.expand(-vrr.width(), 0);
        }
    }

    _setBorder(i.context, Color(), 0.0);
    i.context->setFillColor(bodyColor, i.context->fillColorSpace());
    i.context->fillRect(vr);
    if (!vrr.isEmpty()) {
        i.context->fillRect(vrr);
    }

    i.context->restore();

    return true;
}
#endif

#if ENABLE(METER_TAG)
void RenderThemeWKC::adjustMeterStyle(StyleResolver*, RenderStyle*, Element*) const
{
}

bool RenderThemeWKC::paintMeter(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    if (!o)
        return true;
    if (!o->isMeter())
        return true;

    Color backgroundColor;
    const Color defaultBorderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BORDER));
    const Color bodyColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BODY));

    i.context->save();

    _setBorder(i.context, defaultBorderColor, 1.0);
    if (!this->isEnabled(o)) {
        backgroundColor = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BACKGROUND_DISABLED));
    } else if (o->style()->hasBackground()) {
        backgroundColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    } else {
        backgroundColor = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BACKGROUND));
    }
    if (!o->style()->hasBackgroundImage()) {
        i.context->setFillColor(backgroundColor, i.context->fillColorSpace());
        i.context->drawRect(r);
    }

    double value = 0;
    RenderMeter* renderMeter = toRenderMeter(o);
    if (renderMeter) {
        HTMLMeterElement* e = static_cast<HTMLMeterElement*>(renderMeter->node());
        if (e)
            value = e->valueRatio();
    }
    IntRect vr = r;
    vr.expand(-2,-2);
    vr.move(1,1);
    int w = vr.width() * value;
    vr.setWidth(w);

    _setBorder(i.context, Color(), 0.0);
    i.context->setFillColor(bodyColor, i.context->fillColorSpace());
    i.context->fillRect(vr);

    i.context->restore();

    return true;
}
#endif

void RenderThemeWKC::adjustInnerSpinButtonStyle(StyleResolver*, RenderStyle* style, Element*) const
{
    int width = style->fontSize();
    if (width <= 0)
        width = 18;
    style->setWidth(Length(width, Fixed));
    style->setMinWidth(Length(width, Fixed));
}

static bool _paintSpinButton(RenderObject* o, const PaintInfo& i, const IntRect& r, int index, bool up)
{
    void *drawContext;
    WKCSize img_size;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;
    
    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;
    
    img_size.fWidth = width;
    img_size.fHeight = height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (o, drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    int cw = rect.fWidth - points[0].fX - (img_size.fWidth - points[3].fX);
    int ch = rect.fHeight - points[0].fY - (img_size.fHeight - points[3].fY);
    int cx0 = rect.fX + points[0].fX;
    int cy0 = rect.fY + points[0].fY;
    int cx1 = cx0 + cw;
    int cy1 = cy0 + ch;
    int cx = cx0 + cw / 2;

    Path path;
    if (up) {
        path.moveTo(FloatPoint(cx0, cy1));
        path.addLineTo(FloatPoint(cx, cy0));
        path.addLineTo(FloatPoint(cx1, cy1));
        path.addLineTo(FloatPoint(cx0, cy1));
    } else {
        path.moveTo(FloatPoint(cx0, cy0));
        path.addLineTo(FloatPoint(cx, cy1));
        path.addLineTo(FloatPoint(cx1, cy0));
        path.addLineTo(FloatPoint(cx0, cy0));
    }
    Color c(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_BUTTONTEXT));
    i.context->save();
    i.context->setFillColor(c, i.context->fillColorSpace());
    i.context->fillPath(path);
    i.context->restore();

    return false;
}

bool RenderThemeWKC::paintInnerSpinButton(RenderObject* o, const PaintInfo& i, const IntRect& r)
{
    IntRect r1(r), r2(r);
    r1.setHeight(r1.height()/2);
    r2.setHeight(r2.height()/2);
    r2.move(0, r1.height());

    if (!o)
        return false;

    int index = 0;
    if (isEnabled(o)) {
        index = WKC_IMAGE_BUTTON;
        if (isHovered(o) && isSpinUpButtonPartHovered(o))
            index = WKC_IMAGE_BUTTON_HOVERED;
        if (isPressed(o) && isSpinUpButtonPartPressed(o))
            index = WKC_IMAGE_BUTTON_PRESSED;
    } else {
        index = WKC_IMAGE_BUTTON_DISABLED;
    }

    _paintSpinButton(o, i, r1, index, true);

    index = 0;
    if (isEnabled(o)) {
        index = WKC_IMAGE_BUTTON;
        if (isHovered(o) && !isSpinUpButtonPartHovered(o))
            index = WKC_IMAGE_BUTTON_HOVERED;
        if (isPressed(o) && !isSpinUpButtonPartPressed(o))
            index = WKC_IMAGE_BUTTON_PRESSED;
    } else {
        index = WKC_IMAGE_BUTTON_DISABLED;
    }

    _paintSpinButton(o, i, r2, index, false);

    return true;
}

}
