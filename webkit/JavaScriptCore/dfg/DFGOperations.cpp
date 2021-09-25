/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 * Copyright (c) 2013 NINTENDO.
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
#include "DFGOperations.h"

#include "CodeBlock.h"
#include "DFGOSRExit.h"
#include "DFGRepatch.h"
#include "HostCallReturnValue.h"
#include "GetterSetter.h"
#include <wtf/InlineASM.h>
#include "Interpreter.h"
#include "JITExceptions.h"
#include "JSActivation.h"
#include "JSGlobalData.h"
#include "JSStaticScopeObject.h"
#include "Operations.h"

#if ENABLE(DFG_JIT)

#if CPU(X86_64)

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, register) \
    asm( \
    ".globl " SYMBOL_STRING(function) "\n" \
    HIDE_SYMBOL(function) "\n" \
    SYMBOL_STRING(function) ":" "\n" \
        "mov (%rsp), %" STRINGIZE(register) "\n" \
        "jmp " SYMBOL_STRING_RELOCATION(function##_WithReturnAddress) "\n" \
    );
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(function)    FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, rsi)
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_ECI(function)  FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, rcx)
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(function)  FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, rcx)
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(function) FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, r8)

#elif CPU(X86)

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, offset) \
    asm( \
    ".text" "\n" \
    ".globl " SYMBOL_STRING(function) "\n" \
    HIDE_SYMBOL(function) "\n" \
    SYMBOL_STRING(function) ":" "\n" \
        "mov (%esp), %eax\n" \
        "mov %eax, " STRINGIZE(offset) "(%esp)\n" \
        "jmp " SYMBOL_STRING_RELOCATION(function##_WithReturnAddress) "\n" \
    );
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(function)    FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, 8)
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_ECI(function)  FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, 16)
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(function)  FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, 20)
#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(function) FUNCTION_WRAPPER_WITH_RETURN_ADDRESS(function, 24)

#elif COMPILER(GCC) && CPU(ARM_THUMB2)

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(function) \
    asm ( \
    ".text" "\n" \
    ".align 2" "\n" \
    ".globl " SYMBOL_STRING(function) "\n" \
    HIDE_SYMBOL(function) "\n" \
    ".thumb" "\n" \
    ".thumb_func " THUMB_FUNC_PARAM(function) "\n" \
    SYMBOL_STRING(function) ":" "\n" \
        "mov a2, lr" "\n" \
        "b " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
    );

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_ECI(function) \
    asm ( \
    ".text" "\n" \
    ".align 2" "\n" \
    ".globl " SYMBOL_STRING(function) "\n" \
    HIDE_SYMBOL(function) "\n" \
    ".thumb" "\n" \
    ".thumb_func " THUMB_FUNC_PARAM(function) "\n" \
    SYMBOL_STRING(function) ":" "\n" \
        "mov a4, lr" "\n" \
        "b " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
    );

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(function) \
    asm ( \
    ".text" "\n" \
    ".align 2" "\n" \
    ".globl " SYMBOL_STRING(function) "\n" \
    HIDE_SYMBOL(function) "\n" \
    ".thumb" "\n" \
    ".thumb_func " THUMB_FUNC_PARAM(function) "\n" \
    SYMBOL_STRING(function) ":" "\n" \
        "str lr, [sp, #0]" "\n" \
        "b " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
    );

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(function) \
    asm ( \
    ".text" "\n" \
    ".align 2" "\n" \
    ".globl " SYMBOL_STRING(function) "\n" \
    HIDE_SYMBOL(function) "\n" \
    ".thumb" "\n" \
    ".thumb_func " THUMB_FUNC_PARAM(function) "\n" \
    SYMBOL_STRING(function) ":" "\n" \
        "str lr, [sp, #4]" "\n" \
        "b " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
    );


#elif CPU(PPC)

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(function) \
extern "C" { void* function(ExecState*); } \
void __dfgDummy##function () \
{ \
    asm ( \
	".text\n" \
	".align	2\n" \
	".global " SYMBOL_STRING(function) "\n" \
SYMBOL_STRING(function) " :\n" \
	"mflr  r4\n" \
	"stwu  r1, -8(r1)\n" \
	"stw   r4,  4(r1)\n" \
	"addi  r4, r1,  4\n" \
	"addi  r1, r1, -8\n" \
    "bl    " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
	"addi  r1, r1,  8\n" \
	"lwz   r0,  4(r1)\n" \
	"addi  r1, r1,  8\n" \
	"mtlr  r0\n" \
    ); \
}

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_ECI(function) \
extern "C" { EncodedJSValue function(ExecState*, JSCell*, Identifier*); } \
void __dfgDummy##function () \
{ \
    asm ( \
	".text\n" \
	".align	2\n" \
	".global " SYMBOL_STRING(function) "\n" \
SYMBOL_STRING(function) " :\n" \
	"mflr  r6\n" \
	"stwu  r1, -8(r1)\n" \
	"stw   r6,  4(r1)\n" \
	"addi  r6, r1,  4\n" \
	"addi  r1, r1, -8\n" \
    "bl    " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"addi  r1, r1,  8\n" \
	"lwz   r0,  4(r1)\n" \
	"addi  r1, r1,  8\n" \
	"mtlr  r0\n" \
    ); \
}

#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(function) \
extern "C" { EncodedJSValue function(ExecState*, EncodedJSValue, Identifier*); } \
void __dfgDummy##function () \
{ \
    asm ( \
	".text\n" \
	".align	2\n" \
	".global " SYMBOL_STRING(function) "\n" \
SYMBOL_STRING(function) " :\n" \
	"mflr  r8\n" \
	"stwu  r1, -8(r1)\n" \
	"stw   r8,  4(r1)\n" \
	"addi  r8, r1,  4\n" \
	"addi  r1, r1, -8\n" \
	"mr    r7, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"addi  r1, r1,  8\n" \
	"lwz   r0,  4(r1)\n" \
	"addi  r1, r1,  8\n" \
	"mtlr  r0\n" \
    ); \
}


#define FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(function) \
extern "C" { void DFG_OPERATION function(ExecState*, EncodedJSValue, JSCell*, Identifier*); } \
void __dfgDummy##function () \
{ \
    asm ( \
	".text\n" \
	".align	2\n" \
	".global " SYMBOL_STRING(function) "\n" \
SYMBOL_STRING(function) " :\n" \
	"mflr  r9\n" \
	"stwu  r1, -8(r1)\n" \
	"stw   r9,  4(r1)\n" \
	"addi  r9, r1,  4\n" \
	"addi  r1, r1, -8\n" \
	"mr    r8, r7\n" \
	"mr    r7, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(function) "_WithReturnAddress" "\n" \
	"addi  r1, r1,  8\n" \
	"lwz   r0,  4(r1)\n" \
	"addi  r1, r1,  8\n" \
	"mtlr  r0\n" \
    ); \
}

#endif

#define P_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(function) \
void* DFG_OPERATION function##_WithReturnAddress(ExecState*, ReturnAddressPtr); \
FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(function)

#define J_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_ECI(function) \
EncodedJSValue DFG_OPERATION function##_WithReturnAddress(ExecState*, JSCell*, Identifier*, ReturnAddressPtr); \
FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_ECI(function)

#define J_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(function) \
EncodedJSValue DFG_OPERATION function##_WithReturnAddress(ExecState*, EncodedJSValue, Identifier*, ReturnAddressPtr); \
FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(function)

#define V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(function) \
void DFG_OPERATION function##_WithReturnAddress(ExecState*, EncodedJSValue, JSCell*, Identifier*, ReturnAddressPtr); \
FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(function)


//
// PPC arranges register-arguments differently depending on whether they are 32-bit or 64-bit
// Therefore, we need "stub" functions that re-order the arguments (and also handle BIG-ENDIAN return values, as above)
//

#if CPU(X86) || CPU(X86_64) || CPU(ARM_THUMB2)

// Other CPUs may not require the stub-functions, but we take advantage of being able to append the '_WithReturnAddress' tag

#define DFG_OPERATION_IMPL(fname)   DFG_OPERATION fname
#define DFG_OPERATION_IMPL_R(fname) DFG_OPERATION fname##_WithReturnAddress


#define J_DFG_OPERATION_STUB(fname)
#define X_DFG_OPERATION_STUB(fname)

#define X_DFG_OPERATION_STUB_J(rtype, fname)
#define J_DFG_OPERATION_STUB_EJ(rtype, fname)
#define X_DFG_OPERATION_STUB_EJ(rtype, fname)
#define J_DFG_OPERATION_STUB_EPJ(rtype, fname)
#define J_DFG_OPERATION_STUB_EJA(rtype, fname)
#define J_DFG_OPERATION_STUB_EJP(rtype, fname)
#define J_DFG_OPERATION_STUB_EJJ(rtype, fname)
#define X_DFG_OPERATION_STUB_EJJ(rtype, fname)
#define V_DFG_OPERATION_STUB_EPJJ(rtype, fname)
#define V_DFG_OPERATION_STUB_EPIJ(rtype, fname)
#define V_DFG_OPERATION_STUB_EJPP(rtype, fname)
#define V_DFG_OPERATION_STUB_EJJJ(rtype, fname)


#elif CPU(PPC)

#define DFG_OPERATION_IMPL(fname)   DFG_OPERATION fname##_Wrapped
#define DFG_OPERATION_IMPL_R(fname) DFG_OPERATION fname##_WithReturnAddress


#define J_DFG_OPERATION_STUB(fname) \
void fname##_Stub() \
{ \
    asm ( \
    ".text" "\n" \
    ".align 2" "\n" \
    ".global " SYMBOL_STRING(fname) "\n" \
SYMBOL_STRING(fname) ":" "\n" \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define X_DFG_OPERATION_STUB(fname) \
void fname##_Stub() \
{ \
    asm ( \
    ".text" "\n" \
    ".align 2" "\n" \
    ".global " SYMBOL_STRING(fname) "\n" \
SYMBOL_STRING(fname) ":" "\n" \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}


#define X_DFG_OPERATION_STUB_J(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(EncodedJSValue); \
rtype DFG_OPERATION fname(EncodedJSValue jsval) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define J_DFG_OPERATION_STUB_EJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define X_DFG_OPERATION_STUB_EJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define J_DFG_OPERATION_STUB_EPJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, JSCell*, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, JSCell* pCell, EncodedJSValue jsval) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r0, r5\n" \
	"mr    r5, r6\n" \
	"mr    r6, r0\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define J_DFG_OPERATION_STUB_EJA(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue, JSArray*); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval, JSArray* ptr) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r7, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define J_DFG_OPERATION_STUB_EJP(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue, Identifier*); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval, Identifier* ptr) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r7, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define J_DFG_OPERATION_STUB_EJJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval1, EncodedJSValue jsval2) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r8, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"mr    r0, r3\n" \
	"mr    r3, r4\n" \
	"mr    r4, r0\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define X_DFG_OPERATION_STUB_EJJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval1, EncodedJSValue jsval2) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r8, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define V_DFG_OPERATION_STUB_EPJJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, JSCell*, EncodedJSValue, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, JSCell* pCell, EncodedJSValue jsval1, EncodedJSValue jsval2) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r0, r8\n" \
	"mr    r8, r7\n" \
	"mr    r7, r0\n" \
	"mr    r0, r6\n" \
	"mr    r6, r5\n" \
	"mr    r5, r0\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define V_DFG_OPERATION_STUB_EPIJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, JSArray*, int32_t, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, JSArray* ptr, int32_t ival, EncodedJSValue jsval) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r8, r6\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define V_DFG_OPERATION_STUB_EJPP(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue, JSCell*, Identifier*); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval, JSCell* ptr1, Identifier* ptr2) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r8, r7\n" \
	"mr    r7, r6\n" \
	"mr    r6, r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#define V_DFG_OPERATION_STUB_EJJJ(rtype, fname) \
rtype DFG_OPERATION fname##_Wrapped(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue); \
rtype DFG_OPERATION fname(ExecState* esptr, EncodedJSValue jsval1, EncodedJSValue jsval2, EncodedJSValue jsval3) \
{ \
    asm ( \
	"mflr  r0\n" \
	"stwu  r0, -8(r1)\n" \
	"addi  r1, r1, -8\n" \
	"mr    r10, r8\n" \
	"mr    r8,  r6\n" \
	"mr    r6,  r4\n" \
    "bl    " SYMBOL_STRING_RELOCATION(fname) "_Wrapped" "\n" \
	"lwz   r0,  8(r1)\n" \
	"addi  r1, r1, 16\n" \
	"mtlr  r0\n" \
    ); \
}

#endif


namespace JSC { namespace DFG {

template<bool strict>
static inline void putByVal(ExecState* exec, JSValue baseValue, uint32_t index, JSValue value)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    if (isJSArray(baseValue)) {
        JSArray* array = asArray(baseValue);
        if (array->canSetIndex(index)) {
            array->setIndex(globalData, index, value);
            return;
        }

        JSArray::putByIndex(array, exec, index, value, strict);
        return;
    }

    baseValue.putByIndex(exec, index, value, strict);
}

template<bool strict>
ALWAYS_INLINE static void DFG_OPERATION operationPutByValInternal(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue property = JSValue::decode(encodedProperty);
    JSValue value = JSValue::decode(encodedValue);

    if (LIKELY(property.isUInt32())) {
        putByVal<strict>(exec, baseValue, property.asUInt32(), value);
        return;
    }

    if (property.isDouble()) {
        double propertyAsDouble = property.asDouble();
        uint32_t propertyAsUInt32 = static_cast<uint32_t>(propertyAsDouble);
        if (propertyAsDouble == propertyAsUInt32) {
            putByVal<strict>(exec, baseValue, propertyAsUInt32, value);
            return;
        }
    }


    // Don't put to an object if toString throws an exception.
    Identifier ident(exec, property.toString(exec)->value(exec));
    if (!globalData->exception) {
        PutPropertySlot slot(strict);
        baseValue.put(exec, ident, value, slot);
    }
}

extern "C" {

J_DFG_OPERATION_STUB_EJ(EncodedJSValue, operationConvertThis);
EncodedJSValue DFG_OPERATION_IMPL(operationConvertThis)(ExecState* exec, EncodedJSValue encodedOp)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    return JSValue::encode(JSValue::decode(encodedOp).toThisObject(exec));
}

inline JSCell* createThis(ExecState* exec, JSCell* prototype, JSFunction* constructor)
{
#if !ASSERT_DISABLED
    ConstructData constructData;
    ASSERT(constructor->methodTable()->getConstructData(constructor, constructData) == ConstructTypeJS);
#endif
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);

    Structure* structure;
    if (prototype->isObject())
        structure = asObject(prototype)->inheritorID(globalData);
    else
        structure = constructor->scope()->globalObject->emptyObjectStructure();
    
    return constructEmptyObject(exec, structure);
}

X_DFG_OPERATION_STUB(operationCreateThis);
JSCell* DFG_OPERATION_IMPL(operationCreateThis)(ExecState* exec, JSCell* prototype)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    return createThis(exec, prototype, jsCast<JSFunction*>(exec->callee()));
}

X_DFG_OPERATION_STUB(operationCreateThisInlined);
JSCell* DFG_OPERATION_IMPL(operationCreateThisInlined)(ExecState* exec, JSCell* prototype, JSCell* constructor)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    return createThis(exec, prototype, jsCast<JSFunction*>(constructor));
}

X_DFG_OPERATION_STUB(operationNewObject);
JSCell* DFG_OPERATION_IMPL(operationNewObject)(ExecState* exec)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    return constructEmptyObject(exec);
}

J_DFG_OPERATION_STUB_EJJ(EncodedJSValue, operationValueAdd);
EncodedJSValue DFG_OPERATION_IMPL(operationValueAdd)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    return JSValue::encode(jsAdd(exec, op1, op2));
}

J_DFG_OPERATION_STUB_EJJ(EncodedJSValue, operationValueAddNotNumber);
EncodedJSValue DFG_OPERATION_IMPL(operationValueAddNotNumber)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    ASSERT(!op1.isNumber() || !op2.isNumber());
    
    if (op1.isString() && !op2.isObject())
        return JSValue::encode(jsString(exec, asString(op1), op2.toString(exec)));

    return JSValue::encode(jsAddSlowCase(exec, op1, op2));
}

static inline EncodedJSValue getByVal(ExecState* exec, JSCell* base, uint32_t index)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    // FIXME: the JIT used to handle these in compiled code!
    if (isJSArray(base) && asArray(base)->canGetIndex(index))
        return JSValue::encode(asArray(base)->getIndex(index));

    // FIXME: the JITstub used to relink this to an optimized form!
    if (isJSString(base) && asString(base)->canGetIndex(index))
        return JSValue::encode(asString(base)->getIndex(exec, index));

    return JSValue::encode(JSValue(base).get(exec, index));
}

J_DFG_OPERATION_STUB_EJJ(EncodedJSValue, operationGetByVal);
EncodedJSValue DFG_OPERATION_IMPL(operationGetByVal)(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue property = JSValue::decode(encodedProperty);

    if (LIKELY(baseValue.isCell())) {
        JSCell* base = baseValue.asCell();

        if (property.isUInt32()) {
            return getByVal(exec, base, property.asUInt32());
        } else if (property.isDouble()) {
            double propertyAsDouble = property.asDouble();
            uint32_t propertyAsUInt32 = static_cast<uint32_t>(propertyAsDouble);
            if (propertyAsUInt32 == propertyAsDouble)
                return getByVal(exec, base, propertyAsUInt32);
        } else if (property.isString()) {
            if (JSValue result = base->fastGetOwnProperty(exec, asString(property)->value(exec)))
                return JSValue::encode(result);
        }
    }

    Identifier ident(exec, property.toString(exec)->value(exec));
    return JSValue::encode(baseValue.get(exec, ident));
}

J_DFG_OPERATION_STUB_EPJ(EncodedJSValue, operationGetByValCell);
EncodedJSValue DFG_OPERATION_IMPL(operationGetByValCell)(ExecState* exec, JSCell* base, EncodedJSValue encodedProperty)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue property = JSValue::decode(encodedProperty);

    if (property.isUInt32())
        return getByVal(exec, base, property.asUInt32());
    if (property.isDouble()) {
        double propertyAsDouble = property.asDouble();
        uint32_t propertyAsUInt32 = static_cast<uint32_t>(propertyAsDouble);
        if (propertyAsUInt32 == propertyAsDouble)
            return getByVal(exec, base, propertyAsUInt32);
    } else if (property.isString()) {
        if (JSValue result = base->fastGetOwnProperty(exec, asString(property)->value(exec)))
            return JSValue::encode(result);
    }

    Identifier ident(exec, property.toString(exec)->value(exec));
    return JSValue::encode(JSValue(base).get(exec, ident));
}

J_DFG_OPERATION_STUB_EJP(EncodedJSValue, operationGetById);
EncodedJSValue DFG_OPERATION_IMPL(operationGetById)(ExecState* exec, EncodedJSValue base, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue);
    return JSValue::encode(baseValue.get(exec, *propertyName, slot));
}

J_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(operationGetByIdBuildList);
EncodedJSValue DFG_OPERATION_IMPL_R(operationGetByIdBuildList)(ExecState* exec, EncodedJSValue base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue);
    JSValue result = baseValue.get(exec, *propertyName, slot);

    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    dfgBuildGetByIDList(exec, baseValue, *propertyName, slot, stubInfo);

    return JSValue::encode(result);
}

J_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(operationGetByIdProtoBuildList);
EncodedJSValue DFG_OPERATION_IMPL_R(operationGetByIdProtoBuildList)(ExecState* exec, EncodedJSValue base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue);
    JSValue result = baseValue.get(exec, *propertyName, slot);

    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    dfgBuildGetByIDProtoList(exec, baseValue, *propertyName, slot, stubInfo);

    return JSValue::encode(result);
}

J_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJI(operationGetByIdOptimize);
EncodedJSValue DFG_OPERATION_IMPL_R(operationGetByIdOptimize)(ExecState* exec, EncodedJSValue base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue);
    JSValue result = baseValue.get(exec, *propertyName, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    if (stubInfo.seen)
        dfgRepatchGetByID(exec, baseValue, *propertyName, slot, stubInfo);
    else
        stubInfo.seen = true;

    return JSValue::encode(result);
}

J_DFG_OPERATION_STUB(operationCallCustomGetter);
EncodedJSValue DFG_OPERATION_IMPL(operationCallCustomGetter)(ExecState* exec, JSCell* base, PropertySlot::GetValueFunc function, Identifier* ident)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    return JSValue::encode(function(exec, asObject(base), *ident));
}

J_DFG_OPERATION_STUB(operationCallGetter);
EncodedJSValue DFG_OPERATION_IMPL(operationCallGetter)(ExecState* exec, JSCell* base, JSCell* value)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    GetterSetter* getterSetter = asGetterSetter(value);
    JSObject* getter = getterSetter->getter();
    if (!getter)
        return JSValue::encode(jsUndefined());
    CallData callData;
    CallType callType = getter->methodTable()->getCallData(getter, callData);
    return JSValue::encode(call(exec, getter, callType, callData, asObject(base), ArgList()));
}

V_DFG_OPERATION_STUB_EJJJ(void, operationPutByValStrict);
void DFG_OPERATION_IMPL(operationPutByValStrict)(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    operationPutByValInternal<true>(exec, encodedBase, encodedProperty, encodedValue);
}

V_DFG_OPERATION_STUB_EJJJ(void, operationPutByValNonStrict);
void DFG_OPERATION_IMPL(operationPutByValNonStrict)(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    operationPutByValInternal<false>(exec, encodedBase, encodedProperty, encodedValue);
}

V_DFG_OPERATION_STUB_EPJJ(void, operationPutByValCellStrict);
void DFG_OPERATION_IMPL(operationPutByValCellStrict)(ExecState* exec, JSCell* cell, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    operationPutByValInternal<true>(exec, JSValue::encode(cell), encodedProperty, encodedValue);
}

V_DFG_OPERATION_STUB_EPJJ(void, operationPutByValCellNonStrict);
void DFG_OPERATION_IMPL(operationPutByValCellNonStrict)(ExecState* exec, JSCell* cell, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    operationPutByValInternal<false>(exec, JSValue::encode(cell), encodedProperty, encodedValue);
}

V_DFG_OPERATION_STUB_EPIJ(void, operationPutByValBeyondArrayBoundsStrict);
void DFG_OPERATION_IMPL(operationPutByValBeyondArrayBoundsStrict)(ExecState* exec, JSArray* array, int32_t index, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    if (index >= 0) {
        // We should only get here if index is outside the existing vector.
        ASSERT(!array->canSetIndex(index));
        JSArray::putByIndex(array, exec, index, JSValue::decode(encodedValue), true);
        return;
    }
    
    PutPropertySlot slot(true);
    array->methodTable()->put(
        array, exec, Identifier::from(exec, index), JSValue::decode(encodedValue), slot);
}

V_DFG_OPERATION_STUB_EPIJ(void, operationPutByValBeyondArrayBoundsNonStrict);
void DFG_OPERATION_IMPL(operationPutByValBeyondArrayBoundsNonStrict)(ExecState* exec, JSArray* array, int32_t index, EncodedJSValue encodedValue)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    if (index >= 0) {
        // We should only get here if index is outside the existing vector.
        ASSERT(!array->canSetIndex(index));
        JSArray::putByIndex(array, exec, index, JSValue::decode(encodedValue), false);
        return;
    }
    
    PutPropertySlot slot(false);
    array->methodTable()->put(
        array, exec, Identifier::from(exec, index), JSValue::decode(encodedValue), slot);
}

J_DFG_OPERATION_STUB_EJA(EncodedJSValue, operationArrayPush);
EncodedJSValue DFG_OPERATION_IMPL(operationArrayPush)(ExecState* exec, EncodedJSValue encodedValue, JSArray* array)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    array->push(exec, JSValue::decode(encodedValue));
    return JSValue::encode(jsNumber(array->length()));
}

J_DFG_OPERATION_STUB(operationRegExpExec);
EncodedJSValue DFG_OPERATION_IMPL(operationRegExpExec)(ExecState* exec, JSCell* base, JSCell* argument)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    
    if (!base->inherits(&RegExpObject::s_info))
        return throwVMTypeError(exec);

    ASSERT(argument->isString() || argument->isObject());
    JSString* input = argument->isString() ? asString(argument) : asObject(argument)->toString(exec);
    return JSValue::encode(asRegExpObject(base)->exec(exec, input));
}

X_DFG_OPERATION_STUB(operationRegExpTest);
size_t DFG_OPERATION_IMPL(operationRegExpTest)(ExecState* exec, JSCell* base, JSCell* argument)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);

    if (!base->inherits(&RegExpObject::s_info)) {
        throwTypeError(exec);
        return false;
    }

    ASSERT(argument->isString() || argument->isObject());
    JSString* input = argument->isString() ? asString(argument) : asObject(argument)->toString(exec);
    return asRegExpObject(base)->test(exec, input);
}
        
J_DFG_OPERATION_STUB(operationArrayPop);
EncodedJSValue DFG_OPERATION_IMPL(operationArrayPop)(ExecState* exec, JSArray* array)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return JSValue::encode(array->pop(exec));
}
        
V_DFG_OPERATION_STUB_EJPP(void, operationPutByIdStrict);
void DFG_OPERATION_IMPL(operationPutByIdStrict)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    PutPropertySlot slot(true);
    base->methodTable()->put(base, exec, *propertyName, JSValue::decode(encodedValue), slot);
}

V_DFG_OPERATION_STUB_EJPP(void, operationPutByIdNonStrict);
void DFG_OPERATION_IMPL(operationPutByIdNonStrict)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    PutPropertySlot slot(false);
    base->methodTable()->put(base, exec, *propertyName, JSValue::decode(encodedValue), slot);
}

V_DFG_OPERATION_STUB_EJPP(void, operationPutByIdDirectStrict);
void DFG_OPERATION_IMPL(operationPutByIdDirectStrict)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    PutPropertySlot slot(true);
    ASSERT(base->isObject());
    asObject(base)->putDirect(exec->globalData(), *propertyName, JSValue::decode(encodedValue), slot);
}

V_DFG_OPERATION_STUB_EJPP(void, operationPutByIdDirectNonStrict);
void DFG_OPERATION_IMPL(operationPutByIdDirectNonStrict)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    PutPropertySlot slot(false);
    ASSERT(base->isObject());
    asObject(base)->putDirect(exec->globalData(), *propertyName, JSValue::decode(encodedValue), slot);
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdStrictOptimize);
void DFG_OPERATION_IMPL_R(operationPutByIdStrictOptimize)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    JSValue baseValue(base);
    PutPropertySlot slot(true);
    
    baseValue.put(exec, *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    if (stubInfo.seen)
        dfgRepatchPutByID(exec, baseValue, *propertyName, slot, stubInfo, NotDirect);
    else
        stubInfo.seen = true;
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdNonStrictOptimize);
void DFG_OPERATION_IMPL_R(operationPutByIdNonStrictOptimize)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    JSValue baseValue(base);
    PutPropertySlot slot(false);
    
    baseValue.put(exec, *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    if (stubInfo.seen)
        dfgRepatchPutByID(exec, baseValue, *propertyName, slot, stubInfo, NotDirect);
    else
        stubInfo.seen = true;
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdDirectStrictOptimize);
void DFG_OPERATION_IMPL_R(operationPutByIdDirectStrictOptimize)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    PutPropertySlot slot(true);
    
    ASSERT(base->isObject());
    asObject(base)->putDirect(exec->globalData(), *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    if (stubInfo.seen)
        dfgRepatchPutByID(exec, base, *propertyName, slot, stubInfo, Direct);
    else
        stubInfo.seen = true;
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdDirectNonStrictOptimize);
void DFG_OPERATION_IMPL_R(operationPutByIdDirectNonStrictOptimize)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    PutPropertySlot slot(false);
    
    ASSERT(base->isObject());
    asObject(base)->putDirect(exec->globalData(), *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    if (stubInfo.seen)
        dfgRepatchPutByID(exec, base, *propertyName, slot, stubInfo, Direct);
    else
        stubInfo.seen = true;
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdStrictBuildList);
void DFG_OPERATION_IMPL_R(operationPutByIdStrictBuildList)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    JSValue baseValue(base);
    PutPropertySlot slot(true);
    
    baseValue.put(exec, *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    dfgBuildPutByIdList(exec, baseValue, *propertyName, slot, stubInfo, NotDirect);
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdNonStrictBuildList);
void DFG_OPERATION_IMPL_R(operationPutByIdNonStrictBuildList)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    JSValue baseValue(base);
    PutPropertySlot slot(false);
    
    baseValue.put(exec, *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    dfgBuildPutByIdList(exec, baseValue, *propertyName, slot, stubInfo, NotDirect);
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdDirectStrictBuildList);
void DFG_OPERATION_IMPL_R(operationPutByIdDirectStrictBuildList)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    PutPropertySlot slot(true);
    
    ASSERT(base->isObject());
    asObject(base)->putDirect(exec->globalData(), *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    dfgBuildPutByIdList(exec, base, *propertyName, slot, stubInfo, Direct);
}

V_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_EJCI(operationPutByIdDirectNonStrictBuildList);
void DFG_OPERATION_IMPL_R(operationPutByIdDirectNonStrictBuildList)(ExecState* exec, EncodedJSValue encodedValue, JSCell* base, Identifier* propertyName, ReturnAddressPtr returnAddress)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    JSValue value = JSValue::decode(encodedValue);
    PutPropertySlot slot(false);
    
    ASSERT(base->isObject());
    asObject(base)->putDirect(exec->globalData(), *propertyName, value, slot);
    
    StructureStubInfo& stubInfo = exec->codeBlock()->getStubInfo(returnAddress);
    dfgBuildPutByIdList(exec, base, *propertyName, slot, stubInfo, Direct);
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareLess);
size_t DFG_OPERATION_IMPL(operationCompareLess)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return jsLess<true>(exec, JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareLessEq);
size_t DFG_OPERATION_IMPL(operationCompareLessEq)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return jsLessEq<true>(exec, JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareGreater);
size_t DFG_OPERATION_IMPL(operationCompareGreater)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return jsLess<false>(exec, JSValue::decode(encodedOp2), JSValue::decode(encodedOp1));
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareGreaterEq);
size_t DFG_OPERATION_IMPL(operationCompareGreaterEq)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return jsLessEq<false>(exec, JSValue::decode(encodedOp2), JSValue::decode(encodedOp1));
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareEq);
size_t DFG_OPERATION_IMPL(operationCompareEq)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return JSValue::equalSlowCaseInline(exec, JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareStrictEqCell);
size_t DFG_OPERATION_IMPL(operationCompareStrictEqCell)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    ASSERT(op1.isCell());
    ASSERT(op2.isCell());
    
    return JSValue::strictEqualSlowCaseInline(exec, op1, op2);
}

X_DFG_OPERATION_STUB_EJJ(size_t, operationCompareStrictEq);
size_t DFG_OPERATION_IMPL(operationCompareStrictEq)(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    JSValue src1 = JSValue::decode(encodedOp1);
    JSValue src2 = JSValue::decode(encodedOp2);
    
    return JSValue::strictEqual(exec, src1, src2);
}

static void* handleHostCall(ExecState* execCallee, JSValue callee, CodeSpecializationKind kind)
{
    ExecState* exec = execCallee->callerFrame();
    JSGlobalData* globalData = &exec->globalData();

    execCallee->setScopeChain(exec->scopeChain());
    execCallee->setCodeBlock(0);
    execCallee->clearReturnPC();

    if (kind == CodeForCall) {
        CallData callData;
        CallType callType = getCallData(callee, callData);
    
        ASSERT(callType != CallTypeJS);
    
        if (callType == CallTypeHost) {
            NativeCallFrameTracer tracer(globalData, execCallee);
            execCallee->setCallee(asObject(callee));
            globalData->hostCallReturnValue = JSValue::decode(callData.native.function(execCallee));
            if (globalData->exception)
                return 0;

            return reinterpret_cast<void*>(getHostCallReturnValue);
        }
    
        ASSERT(callType == CallTypeNone);
        exec->globalData().exception = createNotAFunctionError(exec, callee);
        return 0;
    }

    ASSERT(kind == CodeForConstruct);
    
    ConstructData constructData;
    ConstructType constructType = getConstructData(callee, constructData);
    
    ASSERT(constructType != ConstructTypeJS);
    
    if (constructType == ConstructTypeHost) {
        NativeCallFrameTracer tracer(globalData, execCallee);
        execCallee->setCallee(asObject(callee));
        globalData->hostCallReturnValue = JSValue::decode(constructData.native.function(execCallee));
        if (globalData->exception)
            return 0;

        return reinterpret_cast<void*>(getHostCallReturnValue);
    }
    
    ASSERT(constructType == ConstructTypeNone);
    exec->globalData().exception = createNotAConstructorError(exec, callee);
    return 0;
}

inline void* linkFor(ExecState* execCallee, ReturnAddressPtr returnAddress, CodeSpecializationKind kind)
{
    ExecState* exec = execCallee->callerFrame();
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    JSValue calleeAsValue = execCallee->calleeAsValue();
    JSCell* calleeAsFunctionCell = getJSFunction(calleeAsValue);
    if (!calleeAsFunctionCell)
        return handleHostCall(execCallee, calleeAsValue, kind);

    JSFunction* callee = jsCast<JSFunction*>(calleeAsFunctionCell);
    execCallee->setScopeChain(callee->scopeUnchecked());
    ExecutableBase* executable = callee->executable();

    MacroAssemblerCodePtr codePtr;
    CodeBlock* codeBlock = 0;
    if (executable->isHostFunction())
        codePtr = executable->generatedJITCodeFor(kind).addressForCall();
    else {
        FunctionExecutable* functionExecutable = static_cast<FunctionExecutable*>(executable);
        JSObject* error = functionExecutable->compileFor(execCallee, callee->scope(), kind);
        if (error) {
            globalData->exception = createStackOverflowError(exec);
            return 0;
        }
        codeBlock = &functionExecutable->generatedBytecodeFor(kind);
        if (execCallee->argumentCountIncludingThis() < static_cast<size_t>(codeBlock->numParameters()))
            codePtr = functionExecutable->generatedJITCodeWithArityCheckFor(kind);
        else
            codePtr = functionExecutable->generatedJITCodeFor(kind).addressForCall();
    }
    CallLinkInfo& callLinkInfo = exec->codeBlock()->getCallLinkInfo(returnAddress);
    if (!callLinkInfo.seenOnce())
        callLinkInfo.setSeen();
    else
        dfgLinkFor(execCallee, callLinkInfo, codeBlock, callee, codePtr, kind);
    return codePtr.executableAddress();
}

P_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(operationLinkCall);
void* DFG_OPERATION_IMPL_R(operationLinkCall)(ExecState* execCallee, ReturnAddressPtr returnAddress)
{
    return linkFor(execCallee, returnAddress, CodeForCall);
}

P_FUNCTION_WRAPPER_WITH_RETURN_ADDRESS_E(operationLinkConstruct);
void* DFG_OPERATION_IMPL_R(operationLinkConstruct)(ExecState* execCallee, ReturnAddressPtr returnAddress)
{
    return linkFor(execCallee, returnAddress, CodeForConstruct);
}

inline void* virtualFor(ExecState* execCallee, CodeSpecializationKind kind)
{
    ExecState* exec = execCallee->callerFrame();
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    JSValue calleeAsValue = execCallee->calleeAsValue();
    JSCell* calleeAsFunctionCell = getJSFunction(calleeAsValue);
    if (UNLIKELY(!calleeAsFunctionCell))
        return handleHostCall(execCallee, calleeAsValue, kind);
    
    JSFunction* function = jsCast<JSFunction*>(calleeAsFunctionCell);
    execCallee->setScopeChain(function->scopeUnchecked());
    ExecutableBase* executable = function->executable();
    if (UNLIKELY(!executable->hasJITCodeFor(kind))) {
        FunctionExecutable* functionExecutable = static_cast<FunctionExecutable*>(executable);
        JSObject* error = functionExecutable->compileFor(execCallee, function->scope(), kind);
        if (error) {
            exec->globalData().exception = error;
            return 0;
        }
    }
    return executable->generatedJITCodeWithArityCheckFor(kind).executableAddress();
}

X_DFG_OPERATION_STUB(operationVirtualCall);
void* DFG_OPERATION_IMPL(operationVirtualCall)(ExecState* execCallee)
{    
    return virtualFor(execCallee, CodeForCall);
}

X_DFG_OPERATION_STUB(operationVirtualConstruct);
void* DFG_OPERATION_IMPL(operationVirtualConstruct)(ExecState* execCallee)
{
    return virtualFor(execCallee, CodeForConstruct);
}

J_DFG_OPERATION_STUB(operationResolve);
EncodedJSValue DFG_OPERATION_IMPL(operationResolve)(ExecState* exec, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    ScopeChainNode* scopeChain = exec->scopeChain();
    ScopeChainIterator iter = scopeChain->begin();
    ScopeChainIterator end = scopeChain->end();
    ASSERT(iter != end);

    do {
        JSObject* record = iter->get();
        PropertySlot slot(record);
        if (record->getPropertySlot(exec, *propertyName, slot))
            return JSValue::encode(slot.getValue(exec, *propertyName));
    } while (++iter != end);

    return throwVMError(exec, createUndefinedVariableError(exec, *propertyName));
}

J_DFG_OPERATION_STUB(operationResolveBase);
EncodedJSValue DFG_OPERATION_IMPL(operationResolveBase)(ExecState* exec, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return JSValue::encode(resolveBase(exec, *propertyName, exec->scopeChain(), false));
}

J_DFG_OPERATION_STUB(operationResolveBaseStrictPut);
EncodedJSValue DFG_OPERATION_IMPL(operationResolveBaseStrictPut)(ExecState* exec, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    JSValue base = resolveBase(exec, *propertyName, exec->scopeChain(), true);
    if (!base)
        throwError(exec, createErrorForInvalidGlobalAssignment(exec, propertyName->ustring()));
    return JSValue::encode(base);
}

J_DFG_OPERATION_STUB(operationResolveGlobal);
EncodedJSValue DFG_OPERATION_IMPL(operationResolveGlobal)(ExecState* exec, GlobalResolveInfo* resolveInfo, Identifier* propertyName)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();

    PropertySlot slot(globalObject);
    if (globalObject->getPropertySlot(exec, *propertyName, slot)) {
        JSValue result = slot.getValue(exec, *propertyName);

        if (slot.isCacheableValue() && !globalObject->structure()->isUncacheableDictionary() && slot.slotBase() == globalObject) {
            resolveInfo->structure.set(exec->globalData(), exec->codeBlock()->ownerExecutable(), globalObject->structure());
            resolveInfo->offset = slot.cachedOffset();
        }

        return JSValue::encode(result);
    }

    return throwVMError(exec, createUndefinedVariableError(exec, *propertyName));
}

J_DFG_OPERATION_STUB_EJ(EncodedJSValue, operationToPrimitive);
EncodedJSValue DFG_OPERATION_IMPL(operationToPrimitive)(ExecState* exec, EncodedJSValue value)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return JSValue::encode(JSValue::decode(value).toPrimitive(exec));
}

J_DFG_OPERATION_STUB(operationStrCat);
EncodedJSValue DFG_OPERATION_IMPL(operationStrCat)(ExecState* exec, void* buffer, size_t size)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    return JSValue::encode(jsString(exec, static_cast<Register*>(buffer), size));
}

J_DFG_OPERATION_STUB(operationNewArray);
EncodedJSValue DFG_OPERATION_IMPL(operationNewArray)(ExecState* exec, void* buffer, size_t size)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    return JSValue::encode(constructArray(exec, static_cast<JSValue*>(buffer), size));
}

J_DFG_OPERATION_STUB(operationNewArrayBuffer);
EncodedJSValue DFG_OPERATION_IMPL(operationNewArrayBuffer)(ExecState* exec, size_t start, size_t size)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    return JSValue::encode(constructArray(exec, exec->codeBlock()->constantBuffer(start), size));
}

J_DFG_OPERATION_STUB(operationNewRegexp);
EncodedJSValue DFG_OPERATION_IMPL(operationNewRegexp)(ExecState* exec, void* regexpPtr)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    RegExp* regexp = static_cast<RegExp*>(regexpPtr);
    if (!regexp->isValid()) {
        throwError(exec, createSyntaxError(exec, "Invalid flags supplied to RegExp constructor."));
        return JSValue::encode(jsUndefined());
    }
    
    return JSValue::encode(RegExpObject::create(exec->globalData(), exec->lexicalGlobalObject(), exec->lexicalGlobalObject()->regExpStructure(), regexp));
}

X_DFG_OPERATION_STUB(operationCreateActivation);
JSCell* DFG_OPERATION_IMPL(operationCreateActivation)(ExecState* exec)
{
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    JSActivation* activation = JSActivation::create(
        globalData, exec, static_cast<FunctionExecutable*>(exec->codeBlock()->ownerExecutable()));
    exec->setScopeChain(exec->scopeChain()->push(activation));
    return activation;
}

X_DFG_OPERATION_STUB(operationTearOffActivation);
void DFG_OPERATION_IMPL(operationTearOffActivation)(ExecState* exec, JSCell* activation)
{
    ASSERT(activation);
    ASSERT(activation->inherits(&JSActivation::s_info));
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    jsCast<JSActivation*>(activation)->tearOff(exec->globalData());
}

X_DFG_OPERATION_STUB(operationNewFunction);
JSCell* DFG_OPERATION_IMPL(operationNewFunction)(ExecState* exec, JSCell* functionExecutable)
{
    ASSERT(functionExecutable->inherits(&FunctionExecutable::s_info));
    JSGlobalData& globalData = exec->globalData();
    NativeCallFrameTracer tracer(&globalData, exec);
    return static_cast<FunctionExecutable*>(functionExecutable)->make(exec, exec->scopeChain());
}

X_DFG_OPERATION_STUB(operationNewFunctionExpression);
JSCell* DFG_OPERATION_IMPL(operationNewFunctionExpression)(ExecState* exec, JSCell* functionExecutableAsCell)
{
    ASSERT(functionExecutableAsCell->inherits(&FunctionExecutable::s_info));
    FunctionExecutable* functionExecutable =
        static_cast<FunctionExecutable*>(functionExecutableAsCell);
    JSFunction *function = functionExecutable->make(exec, exec->scopeChain());
    if (!functionExecutable->name().isNull()) {
        JSStaticScopeObject* functionScopeObject =
            JSStaticScopeObject::create(
                exec, functionExecutable->name(), function, ReadOnly | DontDelete);
        function->setScope(exec->globalData(), function->scope()->push(functionScopeObject));
    }
    return function;
}

X_DFG_OPERATION_STUB_J(size_t, operationIsObject);
size_t DFG_OPERATION_IMPL(operationIsObject)(EncodedJSValue value)
{
    return jsIsObjectType(JSValue::decode(value));
}

X_DFG_OPERATION_STUB_J(size_t, operationIsFunction);
size_t DFG_OPERATION_IMPL(operationIsFunction)(EncodedJSValue value)
{
    return jsIsFunctionType(JSValue::decode(value));
}

X_DFG_OPERATION_STUB(operationFModOnInts);
double DFG_OPERATION_IMPL(operationFModOnInts)(int32_t a, int32_t b)
{
    return fmod(a, b);
}

X_DFG_OPERATION_STUB(lookupExceptionHandler);
DFGHandlerEncoded DFG_OPERATION_IMPL(lookupExceptionHandler)(ExecState* exec, uint32_t callIndex)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    JSValue exceptionValue = exec->exception();
    ASSERT(exceptionValue);
    
    unsigned vPCIndex = exec->codeBlock()->bytecodeOffsetForCallAtIndex(callIndex);
    ExceptionHandler handler = genericThrow(globalData, exec, exceptionValue, vPCIndex);
    ASSERT(handler.catchRoutine);
    return dfgHandlerEncoded(handler.callFrame, handler.catchRoutine);
}

X_DFG_OPERATION_STUB(lookupExceptionHandlerInStub);
DFGHandlerEncoded DFG_OPERATION_IMPL(lookupExceptionHandlerInStub)(ExecState* exec, StructureStubInfo* stubInfo)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    JSValue exceptionValue = exec->exception();
    ASSERT(exceptionValue);
    
    CodeOrigin codeOrigin = stubInfo->codeOrigin;
    while (codeOrigin.inlineCallFrame)
        codeOrigin = codeOrigin.inlineCallFrame->caller;
    
    ExceptionHandler handler = genericThrow(globalData, exec, exceptionValue, codeOrigin.bytecodeIndex);
    ASSERT(handler.catchRoutine);
    return dfgHandlerEncoded(handler.callFrame, handler.catchRoutine);
}

X_DFG_OPERATION_STUB_EJ(double, dfgConvertJSValueToNumber);
double DFG_OPERATION_IMPL(dfgConvertJSValueToNumber)(ExecState* exec, EncodedJSValue value)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);

    return JSValue::decode(value).toNumber(exec);
}

X_DFG_OPERATION_STUB_EJ(size_t, dfgConvertJSValueToInt32);
size_t DFG_OPERATION_IMPL(dfgConvertJSValueToInt32)(ExecState* exec, EncodedJSValue value)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    // toInt32/toUInt32 return the same value; we want the value zero extended to fill the register.
    return JSValue::decode(value).toUInt32(exec);
}

X_DFG_OPERATION_STUB_EJ(size_t, dfgConvertJSValueToBoolean);
size_t DFG_OPERATION_IMPL(dfgConvertJSValueToBoolean)(ExecState* exec, EncodedJSValue encodedOp)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    return JSValue::decode(encodedOp).toBoolean(exec);
}

#if DFG_ENABLE(VERBOSE_SPECULATION_FAILURE)
X_DFG_OPERATION_STUB(debugOperationPrintSpeculationFailure);
void DFG_OPERATION_IMPL(debugOperationPrintSpeculationFailure)(ExecState* exec, void* debugInfoRaw)
{
    JSGlobalData* globalData = &exec->globalData();
    NativeCallFrameTracer tracer(globalData, exec);
    
    SpeculationFailureDebugInfo* debugInfo = static_cast<SpeculationFailureDebugInfo*>(debugInfoRaw);
    CodeBlock* codeBlock = debugInfo->codeBlock;
    CodeBlock* alternative = codeBlock->alternative();
    dataLog("Speculation failure in %p at @%u with executeCounter = %d, "
            "reoptimizationRetryCounter = %u, optimizationDelayCounter = %u, "
            "success/fail %u/(%u+%u)\n",
            codeBlock,
            debugInfo->nodeIndex,
            alternative ? alternative->jitExecuteCounter() : 0,
            alternative ? alternative->reoptimizationRetryCounter() : 0,
            alternative ? alternative->optimizationDelayCounter() : 0,
            codeBlock->speculativeSuccessCounter(),
            codeBlock->speculativeFailCounter(),
            codeBlock->forcedOSRExitCounter());
}
#endif

} // extern "C"
} } // namespace JSC::DFG

#endif // ENABLE(DFG)

#if COMPILER(GCC) || COMPILER(CCPPC)

namespace JSC {

#if CPU(X86_64)
asm (
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "mov -40(%r13), %r13\n"
    "mov %r13, %rdi\n"
    "jmp " SYMBOL_STRING_RELOCATION(getHostCallReturnValueWithExecState) "\n"
);
#elif CPU(X86)
asm (
".text" "\n" \
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "mov -40(%edi), %edi\n"
    "mov %edi, 4(%esp)\n"
    "jmp " SYMBOL_STRING_RELOCATION(getHostCallReturnValueWithExecState) "\n"
);
#elif CPU(ARM_THUMB2)
asm (
".text" "\n"
".align 2" "\n"
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
".thumb" "\n"
".thumb_func " THUMB_FUNC_PARAM(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "ldr r5, [r5, #-40]" "\n"
    "mov r0, r5" "\n"
    "b " SYMBOL_STRING_RELOCATION(getHostCallReturnValueWithExecState) "\n"
);
#endif

#if CPU(PPC)
void __getHostCallReturnValue ()    // dummy name (actual function is getHostCallReturnValue() in asm() block)
{
asm (
	".text\n"
	".align	 2\n"
	".global getHostCallReturnValue\n"
"getHostCallReturnValue:\n"
	"mflr  r0\n"
	"stwu  r0, -8(r1)\n"
	"lwz   r29, -36(r29)\n" // not '-40' as above since we are accessing the "payload" portion of the ExecState
	"addi  r1, r1, -8\n"
	"mr    r3,  r29\n"
    "bl    getHostCallReturnValueWithExecState\n"
	"addi  r1, r1,  8\n"
	"mr    r0, r3\n"
	"mr    r3, r4\n"
	"mr    r4, r0\n"
	"lwz   r0,  0(r1)\n"
	"addi  r1, r1,  8\n"
    "mtlr  r0\n"
);
}
#endif

extern "C" EncodedJSValue HOST_CALL_RETURN_VALUE_OPTION getHostCallReturnValueWithExecState(ExecState* exec)
{
    if (!exec)
        return JSValue::encode(JSValue());
    return JSValue::encode(exec->globalData().hostCallReturnValue);
}

} // namespace JSC

#endif // COMPILER(GCC) || COMPILER(CCPPC)

