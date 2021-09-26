/*
 * Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCHTMLInputElement.h"
#include "helpers/WKCString.h"

#include "HTMLInputElement.h"

#include "helpers/WKCString.h"
#include "helpers/privates/WKCHTMLInputElementPrivate.h"

namespace WKC {

HTMLInputElementPrivate::HTMLInputElementPrivate(WebCore::HTMLInputElement* parent)
    : HTMLFormControlElementPrivate(parent)
{
}

HTMLInputElementPrivate::~HTMLInputElementPrivate()
{
}

String
HTMLInputElementPrivate::value() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->value();
}

bool
HTMLInputElementPrivate::readOnly() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->readOnly();
}

bool
HTMLInputElementPrivate::disabled() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->disabled();
}

int
HTMLInputElementPrivate::maxLength() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->maxLength();
}

void
HTMLInputElementPrivate::setValue(const String& str, bool sendChangeEvent)
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->setValue(str, sendChangeEvent ? WebCore::DispatchInputAndChangeEvent : WebCore::DispatchNoEvent);
}
 
bool
HTMLInputElementPrivate::isTextButton() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isTextButton();
}
 
bool
HTMLInputElementPrivate::isRadioButton() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isRadioButton();
}

bool
HTMLInputElementPrivate::shouldAutocomplete() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->shouldAutocomplete();
}

bool
HTMLInputElementPrivate::isTextField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isTextField();
}

bool
HTMLInputElementPrivate::isSearchField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isSearchField();
}

bool
HTMLInputElementPrivate::isInputTypeHidden() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isInputTypeHidden();
}

bool
HTMLInputElementPrivate::isPasswordField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isPasswordField();
}

bool
HTMLInputElementPrivate::isCheckbox() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isCheckbox();
}

bool
HTMLInputElementPrivate::isRangeControl() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isRangeControl();
}

bool
HTMLInputElementPrivate::isSteppable() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isSteppable();
}

bool
HTMLInputElementPrivate::isText() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isText();
}

bool
HTMLInputElementPrivate::isEmailField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isEmailField();
}

bool
HTMLInputElementPrivate::isFileUpload() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isFileUpload();
}

bool
HTMLInputElementPrivate::isImageButton() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isImageButton();
}

bool
HTMLInputElementPrivate::isNumberField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isNumberField();
}

bool
HTMLInputElementPrivate::isSubmitButton() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isSubmitButton();
}

bool
HTMLInputElementPrivate::isTelephoneField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isTelephoneField();
}

bool
HTMLInputElementPrivate::isURLField() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isURLField();
}

bool
HTMLInputElementPrivate::isSpeechEnabled() const
{
#if ENABLE(INPUT_SPEECH)
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->isSpeechEnabled();
#else
    return false;
#endif
}

const String
HTMLInputElement::value() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).value();
}

bool
HTMLInputElement::readOnly() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).readOnly();
}

bool
HTMLInputElement::disabled() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).disabled();
}

int
HTMLInputElement::maxLength() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).maxLength();
}

void
HTMLInputElement::setValue(const String& str, bool sendChangeEvent)
{
    reinterpret_cast<HTMLInputElementPrivate&>(priv()).setValue(str, sendChangeEvent);
}

bool
HTMLInputElement::shouldAutocomplete() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).shouldAutocomplete();
}

bool
HTMLInputElement::isTextButton() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isTextButton();
}

bool
HTMLInputElement::isRadioButton() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isRadioButton();
}

bool
HTMLInputElement::isTextField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isTextField();
}

bool
HTMLInputElement::isSearchField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isSearchField();
}

bool
HTMLInputElement::isInputTypeHidden() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isInputTypeHidden();
}

bool
HTMLInputElement::isPasswordField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isPasswordField();
}

bool
HTMLInputElement::isCheckbox() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isCheckbox();
}

bool
HTMLInputElement::isRangeControl() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isRangeControl();
}

bool
HTMLInputElement::isSteppable() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isSteppable();
}

bool
HTMLInputElement::isText() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isText();
}

bool
HTMLInputElement::isEmailField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isEmailField();
}

bool
HTMLInputElement::isFileUpload() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isFileUpload();
}

bool
HTMLInputElement::isImageButton() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isImageButton();
}

bool
HTMLInputElement::isNumberField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isNumberField();
}

bool
HTMLInputElement::isSubmitButton() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isSubmitButton();
}

bool
HTMLInputElement::isTelephoneField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isTelephoneField();
}

bool
HTMLInputElement::isURLField() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isURLField();
}

bool
HTMLInputElement::isSpeechEnabled() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).isSpeechEnabled();
}

} // namespace
