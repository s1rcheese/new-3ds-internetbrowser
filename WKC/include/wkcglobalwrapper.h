/*
 * wkcglobalwrapper.h
 *
 * Copyright(c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _WKCGLOBALWRAPPER_H_
#define _WKCGLOBALWRAPPER_H_

class WKCStaticLocalPtr {
public:
    WKCStaticLocalPtr() {
        fPtr = 0;
        wkcMemoryRegisterGlobalObjPeer(&fPtr, sizeof(void*));
    }
    ~WKCStaticLocalPtr(){};
    volatile void* fPtr;
};

class WKCStaticLocalBool {
public:
    WKCStaticLocalBool() {
        fFlag = false;
        wkcMemoryRegisterGlobalObjPeer(&fFlag, sizeof(bool));
    }
    ~WKCStaticLocalBool(){};
    volatile bool fFlag;
};

class WKCGlobalObjResetter {
public:
    WKCGlobalObjResetter(void* in_obj, int in_size) {
        wkcMemoryRegisterGlobalObjPeer(in_obj, in_size);
    }
    ~WKCGlobalObjResetter(){};
};

#define DEFINE_STATIC_LOCAL(type, name, arg) \
    static WKCStaticLocalPtr name##Ptr; \
    if (!name##Ptr.fPtr) { \
        name##Ptr.fPtr = (void*)(new type arg); \
    } \
    type& name = *((type*)(name##Ptr.fPtr));

#define WKC_DEFINE_STATIC_PTR(type, name, initval) \
    static type name = 0; \
    { \
        static WKCStaticLocalBool name##Inited; \
        if (!name##Inited.fFlag) { \
            name = (initval); \
            name##Inited.fFlag = true; \
        } \
    }

#define WKC_DEFINE_STATIC_HASHMAPPTR(type1, type2, name, initval) \
    static HashMap<type1, type2>* name = 0; \
    { \
        static WKCStaticLocalBool name##Inited; \
        if (!name##Inited.fFlag) { \
            name = (initval); \
            name##Inited.fFlag = true; \
        } \
    }

#define WKC_DEFINE_STATIC_INT(name, initval) \
    static int name = 0; \
    { \
        static WKCStaticLocalBool name##Inited; \
        if (!name##Inited.fFlag) { \
            name = (initval); \
            name##Inited.fFlag = true; \
        } \
    }

#define WKC_DEFINE_STATIC_UINT(name, initval) \
    static unsigned int name = 0; \
    { \
        static WKCStaticLocalBool name##Inited; \
        if (!name##Inited.fFlag) { \
            name = (initval); \
            name##Inited.fFlag = true; \
        } \
    }

#define WKC_DEFINE_STATIC_BOOL(name, initval) \
    static bool name = false; \
    { \
        static WKCStaticLocalBool name##Inited; \
        if (!name##Inited.fFlag) { \
            name = (initval); \
            name##Inited.fFlag = true; \
        } \
    }

#define WKC_DEFINE_STATIC_DOUBLE(name, initval) \
    static double name = 0.0; \
    { \
        static WKCStaticLocalBool name##Inited; \
        if (!name##Inited.fFlag) { \
            name = (initval); \
            name##Inited.fFlag = true; \
        } \
    }

#define WKC_DEFINE_GLOBAL_PTR(type, name, initval) \
    static type name = (initval); \
    static const WKCGlobalObjResetter cReset##name((void*)&name, sizeof(void*));

#define WKC_DEFINE_GLOBAL_CLASS_OBJ_ENTRY(type, name) \
    static type name; \
    static const WKCGlobalObjResetter cReset##name;

#define WKC_DEFINE_GLOBAL_CLASS_OBJ(type, classname, name, initval) \
    type classname::name = (initval); \
    const WKCGlobalObjResetter classname::cReset##name((void*)&name, sizeof(type));

#define WKC_DEFINE_GLOBAL_DOUBLE(name, initval) \
    static double name = (initval); \
    static const WKCGlobalObjResetter cReset##name((void*)&name, sizeof(double));

#define WKC_DEFINE_GLOBAL_FLOAT(name, initval) \
    static float name = (initval); \
    static const WKCGlobalObjResetter cReset##name((void*)&name, sizeof(float));

#define WKC_DEFINE_GLOBAL_INT(name, initval) \
    static int name = (initval); \
    static const WKCGlobalObjResetter cReset##name((void*)&name, sizeof(int));

#define WKC_DEFINE_GLOBAL_UINT(name, initval) \
    static unsigned int name = (initval); \
    static const WKCGlobalObjResetter cReset##name((void*)&name, sizeof(unsigned int));

#define WKC_DEFINE_GLOBAL_BOOL(name, initval) \
    static bool name = (initval); \
    static const WKCGlobalObjResetter cReset##name(&name, sizeof(bool));

#define WKC_DEFINE_GLOBAL_HASHSETPTR(type1, name, initval) \
    static HashSet<type1>* name = (initval); \
    static const WKCGlobalObjResetter cReset##name(&name, sizeof(HashSet<type1>*));

#define WKC_DEFINE_GLOBAL_HASHMAPPTR(type1, type2, name, initval) \
    static HashMap<type1,type2>* name = (initval); \
    static const WKCGlobalObjResetter cReset##name(&name, sizeof(HashMap<type1,type2>*));

#define WKC_DEFINE_GLOBAL_HASHTABLE(name, size, mask, values, tableentry) \
    static const HashTable name = { size, mask, values, tableentry }; \
    static const WKCGlobalObjResetter cReset##name(&(name.table), sizeof(const HashEntry*));

#endif // _WKCGLOBALWRAPPER_H_
