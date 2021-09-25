/*
 * Copyright (C) 2006 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "EmptyClients.h"

#if ENABLE(SVG) || ENABLE(PAGE_POPUP)
#include "FileChooser.h"
#include "FormState.h"
#include "HTMLFormElement.h"

namespace WebCore {

void fillWithEmptyClients(Page::PageClients& pageClients)
{
#if ENABLE(CONTEXT_MENUS)
# if PLATFORM(WKC)
    WKC_DEFINE_STATIC_PTR(ContextMenuClient*, dummyContextMenuClient, adoptPtr(new EmptyContextMenuClient).leakPtr());
# else
    static ContextMenuClient* dummyContextMenuClient = adoptPtr(new EmptyContextMenuClient).leakPtr();
# endif
    pageClients.contextMenuClient = dummyContextMenuClient;
#endif

#if ENABLE(DRAG_SUPPORT)
# if PLATFORM(WKC)
    WKC_DEFINE_STATIC_PTR(DragClient*, dummyDragClient, adoptPtr(new EmptyDragClient).leakPtr());
# else
    static DragClient* dummyDragClient = adoptPtr(new EmptyDragClient).leakPtr();
# endif
    pageClients.dragClient = dummyDragClient;
#endif

#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_PTR(EditorClient*, dummyEditorClient, adoptPtr(new EmptyEditorClient).leakPtr());
#else
    static EditorClient* dummyEditorClient = adoptPtr(new EmptyEditorClient).leakPtr();
#endif
    pageClients.editorClient = dummyEditorClient;

#if PLATFORM(WKC)
    WKC_DEFINE_STATIC_PTR(InspectorClient*, dummyInspectorClient, adoptPtr(new EmptyInspectorClient).leakPtr());
#else
    static InspectorClient* dummyInspectorClient = adoptPtr(new EmptyInspectorClient).leakPtr();
#endif
    pageClients.inspectorClient = dummyInspectorClient;
}

}
#endif
