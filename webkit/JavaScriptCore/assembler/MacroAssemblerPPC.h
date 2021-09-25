/*
 * Copyright (c) 2012-2013 NINTENDO.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions, and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NINTENDO AND ITS CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL NINTENDO OR CONTRIBUTORS BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MacroAssemblerPPC_h
#define MacroAssemblerPPC_h

#if ENABLE(ASSEMBLER) && CPU(PPC)

#include "PPCAssembler.h"
#include "AbstractMacroAssembler.h"

namespace JSC {

class MacroAssemblerPPC : public AbstractMacroAssembler<PPCAssembler> {
public:
    typedef PPCRegisters::FPRegisterID FPRegisterID;

    MacroAssemblerPPC()
        : m_fixedWidth(false)
    {
    }

    static const Scale ScalePtr = TimesFour;

    // For storing immediate number
    static const RegisterID immTempRegister = PPCRegisters::r9;
    // For storing data loaded from the memory
    static const RegisterID dataTempRegister = PPCRegisters::r10;
    // For storing address base
    static const RegisterID addrTempRegister = PPCRegisters::r11;
    // For storing compare result
    static const RegisterID cmpTempRegister = PPCRegisters::r12;

    // link register
    static const RegisterID linkRegister = PPCRegisters::lr;
	
    // FP temp register
    static const FPRegisterID fpTempRegister = PPCRegisters::f9;
	
	// FP constant registers [NOTE: these values are initialized in the JITStubs.cpp ctiTrampoline() function]
    static const FPRegisterID fpMagicRegister = PPCRegisters::f30;		// int -> double magic
    static const FPRegisterID fpZeroRegister = PPCRegisters::f31;		// 0.0

    static const int MaximumCompactPtrAlignedAddressOffset = 0x7FFFFFFF;

    enum RelationalCondition {
        Equal,
        NotEqual,
        Above,
        AboveOrEqual,
        Below,
        BelowOrEqual,
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual
    };

    enum ResultCondition {
        Overflow,
        Signed,
        Zero,
        NonZero
    };

    enum DoubleCondition {
        DoubleEqual,
        DoubleNotEqual,
        DoubleGreaterThan,
        DoubleGreaterThanOrEqual,
        DoubleLessThan,
        DoubleLessThanOrEqual,
        DoubleEqualOrUnordered,
        DoubleNotEqualOrUnordered,
        DoubleGreaterThanOrUnordered,
        DoubleGreaterThanOrEqualOrUnordered,
        DoubleLessThanOrUnordered,
        DoubleLessThanOrEqualOrUnordered
    };

    static const RegisterID stackPointerRegister = PPCRegisters::sp;
    static const RegisterID returnAddressRegister = PPCRegisters::lr;

    // Invert a relational condition, e.g. == becomes !=, < becomes >=, etc.
    static RelationalCondition invert(RelationalCondition cond)
    {
        switch (cond)
        {
            case Equal:
                return NotEqual;
            case NotEqual:
                return Equal;
            case Above:
                return BelowOrEqual;
            case AboveOrEqual:
                return Below;
            case Below:
                return AboveOrEqual;
            case BelowOrEqual:
                return Above;
            case GreaterThan:
                return LessThanOrEqual;
            case GreaterThanOrEqual:
                return LessThan;
            case LessThan:
                return GreaterThanOrEqual;
            case LessThanOrEqual:
                return GreaterThan;
                
            default:
                ASSERT_NOT_REACHED();
                return Equal;   // make compiler happy
        }
    }



    // Integer arithmetic operations:
    //
    // Operations are typically two operand - operation(source, srcDst)
    // For many operations the source may be an TrustedImm32, the srcDst operand
    // may often be a memory location (explictly described using an Address
    // object).

    void add32(RegisterID src, RegisterID dest)
    {
        m_assembler.add(dest, dest, src);
    }

    void add32(TrustedImm32 imm, RegisterID dest)
    {
        add32(imm, dest, dest);
    }

    void add32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        if (!imm.m_isPointer && imm.m_value >= -32768 && imm.m_value <= 32767
            && !m_fixedWidth) {
            /*
              addiu     dest, src, imm
            */
            m_assembler.addi(dest, src, imm.m_value);
        } else {
            /*
              li        immTemp, imm
              addu      dest, src, immTemp
            */
            move(imm, immTempRegister);
            m_assembler.add(dest, src, immTempRegister);
        }
    }

    void add32(TrustedImm32 imm, Address address)
    {
        if (address.offset >= -32768 && address.offset <= 32767
            && !m_fixedWidth) {
            /*
              lw        dataTemp, offset(base)
              li        immTemp, imm
              addu      dataTemp, dataTemp, immTemp
              sw        dataTemp, offset(base)
            */
            m_assembler.lwz(dataTempRegister, address.base, address.offset);
            if (!imm.m_isPointer
                && imm.m_value >= -32768 && imm.m_value <= 32767
                && !m_fixedWidth)
                m_assembler.addi(dataTempRegister, dataTempRegister, imm.m_value);
            else {
                move(imm, immTempRegister);
                m_assembler.add(dataTempRegister, dataTempRegister, immTempRegister);
            }
            m_assembler.stw(dataTempRegister, address.base, address.offset);
        } else {
            /*
              addis     addrTemp, base, (offset + 0x8000) >> 16
              lwz       dataTemp, (offset & 0xffff)(addrTemp)
              li        immtemp, imm
              add       dataTemp, dataTemp, immTemp
              stw       dataTemp, (offset & 0xffff)(addrTemp)
            */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lwz(dataTempRegister, addrTempRegister, address.offset);

            if (imm.m_value >= -32768 && imm.m_value <= 32767 && !m_fixedWidth)
                m_assembler.addi(dataTempRegister, dataTempRegister, imm.m_value);
            else {
                move(imm, immTempRegister);
                m_assembler.add(dataTempRegister, dataTempRegister, immTempRegister);
            }
            m_assembler.stw(dataTempRegister, addrTempRegister, address.offset);
        }
    }

    void add32(Address src, RegisterID dest)
    {
        load32(src, dataTempRegister);
        add32(dataTempRegister, dest);
    }

    void add32(RegisterID src, Address dest)
    {
        if (dest.offset >= -32768 && dest.offset <= 32767 && !m_fixedWidth) {
            /*
              lwz       dataTemp, offset(base)
              add       dataTemp, dataTemp, src
              stw       dataTemp, offset(base)
            */
            m_assembler.lwz(dataTempRegister, dest.base, dest.offset);
            m_assembler.add(dataTempRegister, dataTempRegister, src);
            m_assembler.stw(dataTempRegister, dest.base, dest.offset);
        } else {
            /*
              addis     addrTemp, base, (offset + 0x8000) >> 16
              lwz       dataTemp, (offset & 0xffff)(addrTemp)
              add       dataTemp, dataTemp, src
              stw       dataTemp, (offset & 0xffff)(addrTemp)
            */
            m_assembler.addis(addrTempRegister, dest.base, (dest.offset + 0x8000) >> 16);
            m_assembler.lwz(dataTempRegister, addrTempRegister, dest.offset);
            m_assembler.add(dataTempRegister, dataTempRegister, src);
            m_assembler.stw(dataTempRegister, addrTempRegister, dest.offset);
        }
    }

    void add32(TrustedImm32 imm, AbsoluteAddress address)
    {
        /*
           li   addrTemp, address
           li   immTemp, imm
           lwz  dataTemp, 0(addrTemp)
           add  dataTemp, dataTemp, immTemp
           stw  dataTemp, 0(addrTemp)
        */
        move(TrustedImmPtr(address.m_ptr), addrTempRegister);
        m_assembler.lwz(dataTempRegister, addrTempRegister, 0);
        if (!imm.m_isPointer && imm.m_value >= -32768 && imm.m_value <= 32767
            && !m_fixedWidth)
            m_assembler.addi(dataTempRegister, dataTempRegister, imm.m_value);
        else {
            move(imm, immTempRegister);
            m_assembler.add(dataTempRegister, dataTempRegister, immTempRegister);
        }
        m_assembler.stw(dataTempRegister, addrTempRegister, 0);
    }

    void and32(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        m_assembler.andInsn(dest, op1, op2);
    }
    
    void and32(RegisterID src, RegisterID dest)
    {
        m_assembler.andInsn(dest, dest, src);
    }

    void and32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer && imm.m_value >= 0 && imm.m_value <= 65535 && !m_fixedWidth)
            m_assembler.andi(dest, dest, imm.m_value);
        else {
            /*
              li        immTemp, imm
              and       dest, dest, immTemp
            */
            move(imm, immTempRegister);
            m_assembler.andInsn(dest, dest, immTempRegister);
        }
    }

    void and32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        if (!imm.m_isPointer && imm.m_value >= 0 && imm.m_value <= 65535 && !m_fixedWidth)
            m_assembler.andi(dest, src, imm.m_value);
        else {
            /*
              li        immTemp, imm
              and       dest, src, immTemp
            */
            move(imm, immTempRegister);
            m_assembler.andInsn(dest, src, immTempRegister);
        }
    }

    void lshift32(RegisterID src, RegisterID shiftAmount, RegisterID dest)
    {
        ASSERT(src != PPCRegisters::r0);
		m_assembler.andi(PPCRegisters::r0, shiftAmount, 0x1f);
        m_assembler.slw(dest, src, PPCRegisters::r0);
    }
    
    void lshift32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer)
            m_assembler.slwi(dest, dest, imm.m_value);
        else {
            move(imm, immTempRegister);
            m_assembler.slwi(dest, dest, immTempRegister);
        }
    }

    void lshift32(RegisterID shiftAmount, RegisterID dest)
    {
        lshift32 (dest,shiftAmount,dest);
    }

    void lshift32(RegisterID src, TrustedImm32 shiftAmount, RegisterID dest)
    {
        if (!shiftAmount.m_isPointer)
            m_assembler.slwi(dest, src, shiftAmount.m_value);
        else {
            move(shiftAmount, immTempRegister);
            m_assembler.slwi(dest, src, immTempRegister);
        }
    }

    void mul32(RegisterID src, RegisterID dest)
    {
        m_assembler.mullw(dest, dest, src);
    }

    void mul32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
	// opt: could add code to allow other "simple muls" like 48, 24 etc.
		if (!imm.m_isPointer && !m_fixedWidth)
		{
			if (imm.m_value <= 16)
			{
				if (imm.m_value == 1)
				{
					if (src != dest)
						m_assembler.move(dest, src);
					return;
				}
				if (imm.m_value == 2)
				{
					m_assembler.slwi(dest, src, 1);
					return;
				}
				if (imm.m_value == 4)
				{
					m_assembler.slwi(dest, src, 2);
					return;
				}
				if (imm.m_value == 8)
				{
					m_assembler.slwi(dest, src, 3);
					return;
				}
				if (imm.m_value == 16)
				{
					m_assembler.slwi(dest, src, 4);
					return;
				}
			}
			if (imm.m_value >= -32768 && imm.m_value <= 32767)
			{
				m_assembler.mulli(dest, src, imm.m_value);
				return;
			}
		}
		/*
			li      dataTemp, imm
			mul     dest, src, dataTemp
		*/
		move(imm, dataTempRegister);
		m_assembler.mullw(dest, src, dataTempRegister);
    }

    void neg32(RegisterID srcDest)
    {
        m_assembler.neg(srcDest, srcDest);
    }

    void not32(RegisterID srcDest)
    {
        m_assembler.nor(srcDest, srcDest, srcDest);
    }

    void or32(RegisterID src, RegisterID dest)
    {
        m_assembler.orInsn(dest, dest, src);
    }

    void or32(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        m_assembler.orInsn(dest, op1, op2);
    }

    void or32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer && !imm.m_value && !m_fixedWidth)
            return;

        if (!imm.m_isPointer && imm.m_value >= 0 && imm.m_value <= 65535 && !m_fixedWidth)
		{
            m_assembler.ori(dest, dest, imm.m_value);
            return;
        }

        /*
            li      dataTemp, imm
            or      dest, dest, dataTemp
        */
        move(imm, dataTempRegister);
        m_assembler.orInsn(dest, dest, dataTempRegister);
    }

    void or32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        if (!imm.m_isPointer && !imm.m_value && !m_fixedWidth)
        {
            move(src, dest);
            return;
        }

        if (!imm.m_isPointer && imm.m_value >= 0 && imm.m_value <= 65535 && !m_fixedWidth)
		{
            m_assembler.ori(dest, src, imm.m_value);
            return;
        }

        /*
            li      dataTemp, imm
            or      dest, dest, dataTemp
        */
        move(imm, dataTempRegister);
        m_assembler.orInsn(dest, src, dataTempRegister);
    }

    void rshift32(RegisterID src, RegisterID shiftAmount, RegisterID dest)
    {
        ASSERT(src != PPCRegisters::r0);
		m_assembler.andi(PPCRegisters::r0, shiftAmount, 0x1f);
        m_assembler.sraw(dest, src, PPCRegisters::r0);
    }
    
    void rshift32(RegisterID shiftAmount, RegisterID dest)
    {
        rshift32 (dest,shiftAmount,dest);
    }

    void rshift32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer && !imm.m_value && !m_fixedWidth)
            return;

        if (!imm.m_isPointer && imm.m_value >= 0 && imm.m_value <= 65535 && !m_fixedWidth)
		{
            m_assembler.srawi(dest, dest, imm.m_value);
            return;
        }

        /*
            li      dataTemp, imm
            srawi   dest, dest, dataTemp
        */
        move(imm, dataTempRegister);
        m_assembler.srawi(dest, dest, dataTempRegister);
    }

    void rshift32(RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer)
            m_assembler.srawi(dest, src, imm.m_value);
        else {
            move(imm, immTempRegister);
            m_assembler.srawi(dest, src, immTempRegister);
        }
    }

    void urshift32(RegisterID src, RegisterID shiftAmount, RegisterID dest)
    {
        ASSERT(src != PPCRegisters::r0);
		m_assembler.andi(PPCRegisters::r0, shiftAmount, 0x1f);
        m_assembler.srw(dest, src, PPCRegisters::r0);
    }
    
    void urshift32(RegisterID shiftAmount, RegisterID dest)
    {
        urshift32(dest,shiftAmount,dest);
    }

    void urshift32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer)
            m_assembler.srwi(dest, dest, imm.m_value);
        else {
            move(imm, immTempRegister);
            m_assembler.srwi(dest, dest, immTempRegister);
        }
    }

    void urshift32(RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer)
            m_assembler.srwi(dest, src, imm.m_value);
        else {
            move(imm, immTempRegister);
            m_assembler.srwi(dest, src, immTempRegister);
        }
    }

    void sub32(RegisterID src, RegisterID dest)
    {
        m_assembler.sub(dest, dest, src);
    }

    void sub32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer && imm.m_value >= -32767 && imm.m_value <= 32768 && !m_fixedWidth)
		{
            /*
              addi      dest, src, imm
            */
			if (imm.m_value)
				m_assembler.addi(dest, dest, -imm.m_value);
        }
		else
		{
            /*
              li        immTemp, imm
              sub       dest, src, immTemp
            */
            move(imm, immTempRegister);
            m_assembler.sub(dest, dest, immTempRegister);
        }
    }

    void sub32(TrustedImm32 imm, Address address)
    {
        if (address.offset >= -32768 && address.offset <= 32767
            && !m_fixedWidth) {
            /*
              lwz       dataTemp, offset(base)
              li        immTemp, imm
              sub       dataTemp, dataTemp, immTemp
              stw       dataTemp, offset(base)
            */
            m_assembler.lwz(dataTempRegister, address.base, address.offset);
            if (!imm.m_isPointer && imm.m_value >= -32767 && imm.m_value <= 32768 && !m_fixedWidth)
			{
				if (imm.m_value)
					m_assembler.addi(dataTempRegister, dataTempRegister, -imm.m_value);
            }
			else
			{
                move(imm, immTempRegister);
                m_assembler.sub(dataTempRegister, dataTempRegister, immTempRegister);
            }
            m_assembler.stw(dataTempRegister, address.base, address.offset);
        } else {
            /*
              addis     addrTemp, base, (offset + 0x8000) >> 16
              lwz       dataTemp, (offset & 0xffff)(addrTemp)
              li        immtemp, imm
              sub       dataTemp, dataTemp, immTemp
              stw       dataTemp, (offset & 0xffff)(addrTemp)
            */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lwz(dataTempRegister, addrTempRegister, address.offset);

            if (!imm.m_isPointer && imm.m_value >= -32767 && imm.m_value <= 32768 && !m_fixedWidth)
			{
				if (imm.m_value)
					m_assembler.addi(dataTempRegister, dataTempRegister, -imm.m_value);
			}
            else
			{
                move(imm, immTempRegister);
                m_assembler.sub(dataTempRegister, dataTempRegister, immTempRegister);
            }
            m_assembler.stw(dataTempRegister, addrTempRegister, address.offset);
        }
    }

    void sub32(Address src, RegisterID dest)
    {
        load32(src, dataTempRegister);
        sub32(dataTempRegister, dest);
    }

    void sub32(TrustedImm32 imm, AbsoluteAddress address)
    {
        /*
           li   addrTemp, address
           li   immTemp, imm
           lwz  dataTemp, 0(addrTemp)
           sub  dataTemp, dataTemp, immTemp
           stw  dataTemp, 0(addrTemp)
        */
        move(TrustedImmPtr(address.m_ptr), addrTempRegister);
        m_assembler.lwz(dataTempRegister, addrTempRegister, 0);

        if (!imm.m_isPointer && imm.m_value >= -32767 && imm.m_value <= 32768 && !m_fixedWidth)
		{
			if (imm.m_value)
				m_assembler.addi(dataTempRegister, dataTempRegister, -imm.m_value);
        }
		else
		{
            move(imm, immTempRegister);
            m_assembler.sub(dataTempRegister, dataTempRegister, immTempRegister);
        }
        m_assembler.stw(dataTempRegister, addrTempRegister, 0);
    }

    void xor32(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        m_assembler.xorInsn(dest, op1, op2);
    }

    void xor32(RegisterID src, RegisterID dest)
    {
        m_assembler.xorInsn(dest, dest, src);
    }

    void xor32(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_isPointer && imm.m_value > 0 && imm.m_value < 65535 && !m_fixedWidth)
            m_assembler.xori(dest, dest, imm.m_value);
        else {
            /*
              li        immTemp, imm
              xor       dest, dest, immTemp
            */
            move(imm, immTempRegister);
            m_assembler.xorInsn(dest, dest, immTempRegister);
        }
    }

    void xor32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        if (!imm.m_isPointer && imm.m_value > 0 && imm.m_value < 65535 && !m_fixedWidth)
            m_assembler.xori(dest, src, imm.m_value);
        else {
            /*
              li        immTemp, imm
              xor       dest, src, immTemp
            */
            move(imm, immTempRegister);
            m_assembler.xorInsn(dest, src, immTempRegister);
        }
    }

    void sqrtDouble(FPRegisterID src, FPRegisterID dst)
    {
#if 0
// we might want to do a custom sub routine. No need for a real trampoline or anything. we can make something fitting into the jit world, but being precompiled...
	register u32 p = (u32)sqrtConsts;

	register f32 _x = x;
	register f32 xp;
	register f32 est;
	register f32 est05;
	register f32 est2;
	register f32 cnst3;
	register f32 cnst05;

	asm volatile {
		frsqrte		est,_x
	
		lfs			cnst05,4(p)
		lfs			cnst3,0(p)
	
		fmul		est05,est,cnst05			// get to 24 significant bits
		fmul		est2,est,est
	// 1
		fcmpo		cr0,est05,est
	// 2
		beq			@zeroOut
	
		fnmsub		est2,_x,est2,cnst3
	// 2
		fmul		est,est05,est2
	// 2
		fmul		xp,_x,est
	// 2
		frsp		_x,xp
@zeroOut:
		}

	return(_x);
#endif
//#TF - possibly only a temp solution until we put in our own code (without the need to rescue the regs!)
// fpRegTx  			10-13
// fpTempRegister		9
	/*
			stwu	sp, -48(sp)
			lis		r0, sqrt>>16
			stfd	f9,  8(sp)
			ori		r0, r0, sqrt & 0xffff
			stfd	f10, 16(sp)
			mtctr
			stfd	f11, 24(sp)
			stfd	f12, 32(sp)
			stfd	f13, 40(sp)
			
			fmr		f3, src
			bctr

			lfd		f9,  8(sp)
			lfd		f10, 16(sp)
			lfd		f11, 24(sp)
			lfd		f12, 32(sp)
			lfd		f13, 40(sp)
			addi	sp, sp, 48
			fmr		dst, f3
	*/
	double (*sqrtPtr)(double) = sqrt;

	m_assembler.stwu(PPCRegisters::sp, PPCRegisters::sp, -48);
	
	m_assembler.lis(PPCRegisters::r0, (intptr_t)(*sqrtPtr) >> 16);
	m_assembler.stfd(PPCRegisters::f9, PPCRegisters::sp, 8);
	m_assembler.ori(PPCRegisters::r0, PPCRegisters::r0, (intptr_t)(*sqrtPtr) & 0xffff);
	m_assembler.stfd(PPCRegisters::f10, PPCRegisters::sp, 16);
	m_assembler.mtctr(PPCRegisters::r0);
	m_assembler.stfd(PPCRegisters::f11, PPCRegisters::sp, 24);
	m_assembler.stfd(PPCRegisters::f12, PPCRegisters::sp, 32);
	m_assembler.stfd(PPCRegisters::f13, PPCRegisters::sp, 40);
	
	m_assembler.fmr(PPCRegisters::f3, src);
	m_assembler.bctr();
	
	m_assembler.lfd(PPCRegisters::f9, PPCRegisters::sp, 8);
	m_assembler.lfd(PPCRegisters::f10, PPCRegisters::sp, 16);
	m_assembler.lfd(PPCRegisters::f11, PPCRegisters::sp, 24);
	m_assembler.lfd(PPCRegisters::f12, PPCRegisters::sp, 32);
	m_assembler.lfd(PPCRegisters::f13, PPCRegisters::sp, 40);
	m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 48);
	m_assembler.fmr(dst, PPCRegisters::f3);
    }
    
    void negateDouble(FPRegisterID src, FPRegisterID dest)
    {
        m_assembler.fneg(dest,src);
    }
    
    void absDouble(FPRegisterID src, FPRegisterID dest)
    {
        m_assembler.fabs(dest,src);
    }

    void andnotDouble(FPRegisterID, FPRegisterID)
    {
        ASSERT_NOT_REACHED();
    }

    // Memory access operations:
    //
    // Loads are of the form load(address, destination) and stores of the form
    // store(source, address).  The source for a store may be an TrustedImm32.  Address
    // operand objects to loads and store will be implicitly constructed if a
    // register is passed.

    /* Need to use zero-extened load byte for load8.  */
    void load8(ImplicitAddress address, RegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
            m_assembler.lbz(dest, address.base, address.offset);
        else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                lbz     dest, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lbz(dest, addrTempRegister, address.offset);
        }
    }

    void load8Signed(ImplicitAddress address, RegisterID dest)
    {
        load8 (address,dest);
        /* extsb dest,dest */
        m_assembler.extsb(dest,dest);
    }

    /* Need to use zero-extened load byte for load8.  */
    void load8(BaseIndex address, RegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
			{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                lbz     dest, address.offset(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.lbz(dest, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                lbz     dest, (address.offset & 0xffff)(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.lbz(dest, addrTempRegister, address.offset);
        }
    }

    void load8Signed(BaseIndex address, RegisterID dest)
    {
        load8 (address,dest);
        /* extsb dest,dest */
        m_assembler.extsb(dest,dest);
    }

    void store8(RegisterID src, BaseIndex address)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
			{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                stb     src, address.offset(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.stb(src, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                stb     src, (address.offset & 0xffff)(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.stb(src, addrTempRegister, address.offset);
        }
    }
    

    void load32(ImplicitAddress address, RegisterID dest)
    {
		bool bMoveLR;
		if (dest == PPCRegisters::lr)
		{
			dest = PPCRegisters::r0;
			bMoveLR = true;
		}
		else
			bMoveLR = false;
			
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
            m_assembler.lwz(dest, address.base, address.offset);
        else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                lwz     dest, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lwz(dest, addrTempRegister, address.offset);
        }
		
		if (bMoveLR || m_fixedWidth)
		{
			if (bMoveLR)
				m_assembler.mtlr(dest);
			else
				m_assembler.nop();
		}
    }

    void load32(BaseIndex address, RegisterID dest)
    {
		bool bMoveLR;
		if (dest == PPCRegisters::lr)
		{
			dest = PPCRegisters::r0;
			bMoveLR = true;
		}
		else
			bMoveLR = false;
			
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
		{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                lwz     dest, address.offset(addrTemp)
            */
			m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.lwz(dest, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                lwz     dest, (address.offset & 0xffff)(at)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.lwz(dest, addrTempRegister, address.offset);
        }

		if (bMoveLR || m_fixedWidth)
		{
			if (bMoveLR)
				m_assembler.mtlr(dest);
			else
				m_assembler.nop();
		}
    }

    void load32ByteReversed(BaseIndex address, RegisterID dest)
    {
        m_assembler.slwi(addrTempRegister, address.index, address.scale);
        m_assembler.add (addrTempRegister, addrTempRegister, address.base);
        m_assembler.addi(addrTempRegister, addrTempRegister, address.offset);
        m_assembler.lwbrx(dest, (RegisterID)0, addrTempRegister);
    }
    
    void load32WithUnalignedHalfWords(BaseIndex address, RegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32764 && !m_fixedWidth)
		{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                lwz     dest, address.offset(addrTemp)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
			}
			else
				m_assembler.add(addrTempRegister, address.index, address.base);
            m_assembler.lwz(dest, addrTempRegister, address.offset);
        } else {
			if (address.scale != TimesOne)
			{
				/*
					slw     addrTemp, address.index, address.scale
					add     addrTemp, addrTemp, address.base
					addis   addrTemp, addrTemp, address.offset >> 16
					lwz     dest, (address.offset & 0xffff)(addrTemp)
				*/
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				m_assembler.lwz(dest, addrTempRegister, address.offset & 0xffff);
			}
			else
			{
				/*
					add     addrTemp, address.index, address.base
					addis   addrTemp, addrTemp, address.offset >> 16
					lwz     dest, (address.offset & 0xffff)(addrTemp)
				*/
				m_assembler.add(addrTempRegister, address.index, address.base);
				if (m_fixedWidth)
					m_assembler.nop();
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				m_assembler.lwz(dest, addrTempRegister, address.offset & 0xffff);
			}
        }
    }

    void load32(const void* address, RegisterID dest)
    {
		m_assembler.lis(addrTempRegister, ((intptr_t)address + 0x8000) >> 16);
        m_assembler.lwz(dest, addrTempRegister, (intptr_t)address & 0xffff);
    }

    DataLabel32 load32WithAddressOffsetPatch(Address address, RegisterID dest)
    {
        m_fixedWidth = true;
        /*
            lis  addrTemp, address.offset >> 16
            ori  addrTemp, addrTemp, address.offset & 0xffff
            add  addrTemp, addrTemp, address.base
            lwz  dest, 0(addrTemp)
        */
        DataLabel32 dataLabel(this);
        move(TrustedImm32(address.offset), addrTempRegister);
        m_assembler.add(addrTempRegister, addrTempRegister, address.base);
        m_assembler.lwz(dest, addrTempRegister, 0);
        m_fixedWidth = false;
        return dataLabel;
    }
    
    DataLabelCompact load32WithCompactAddressOffsetPatch(Address address, RegisterID dest)
    {
        DataLabelCompact dataLabel(this);
        load32WithAddressOffsetPatch(address, dest);
        return dataLabel;
    }

    /* Need to use zero-extened load half-word for load16.  */
    void load16(ImplicitAddress address, RegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
            m_assembler.lhz(dest, address.base, address.offset);
        else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                lhz     dest, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lhz(dest, addrTempRegister, address.offset);
        }
    }

    void load16Signed(ImplicitAddress address, RegisterID dest)
    {
        load16 (address,dest);
        /* extsh dest,dest */
        m_assembler.extsh(dest,dest);
    }
    
    /* Need to use zero-extened load half-word for load16.  */
    void load16(BaseIndex address, RegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
			{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                lhz     dest, address.offset(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.lhz(dest, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                lhz     dest, (address.offset & 0xffff)(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.lhz(dest, addrTempRegister, address.offset);
        }
    }

    void load16Signed(BaseIndex address, RegisterID dest)
    {
        load16 (address,dest);
        /* extsh dest,dest */
        m_assembler.extsh(dest,dest);
    }
    
    void load16ByteReversed(BaseIndex address, RegisterID dest)
    {
        m_assembler.slwi(addrTempRegister, address.index, address.scale);
        m_assembler.add(addrTempRegister, addrTempRegister, address.base);
        m_assembler.addi(addrTempRegister, addrTempRegister, address.offset);
        m_assembler.lhbrx(dest, (RegisterID)0, addrTempRegister);
    }
    
    void store16(RegisterID src, BaseIndex address)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
			{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                sth     src, address.offset(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.sth(src, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                sth     src, (address.offset & 0xffff)(addrTemp)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.sth(src, addrTempRegister, address.offset);
        }
    }
    
    void load16Unaligned(BaseIndex address, RegisterID dest)
    {
        /* PPC is ok with unaligned data access */
        load16(address,dest);
    }
    
    DataLabel32 store32WithAddressOffsetPatch(RegisterID src, Address address)
    {
        m_fixedWidth = true;
        /*
            lis   addrTemp, address.offset >> 16
            ori   addrTemp, addrTemp, address.offset & 0xffff
            add   addrTemp, addrTemp, address.base
            stw   src, 0(addrTemp)
        */
        DataLabel32 dataLabel(this);
        move(TrustedImm32(address.offset), addrTempRegister);
        m_assembler.add(addrTempRegister, addrTempRegister, address.base);
        m_assembler.stw(src, addrTempRegister, 0);
        m_fixedWidth = false;
        return dataLabel;
    }

    void store32(RegisterID src, ImplicitAddress address)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
            m_assembler.stw(src, address.base, address.offset);
        else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                stw     src, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.stw(src, addrTempRegister, address.offset);
        }
    }

    void store32(RegisterID src, BaseIndex address)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth) {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                stw     src, address.offset(addrTemp)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.stw(src, addrTempRegister, address.offset);
			}
			else
			{
				m_assembler.add(addrTempRegister, address.index, address.base);
				m_assembler.stw(src, addrTempRegister, address.offset);
			}
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                stw     src, (address.offset & 0xffff)(at)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				m_assembler.stw(src, addrTempRegister, address.offset);
			}
			else
			{
				m_assembler.add(addrTempRegister, address.index, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				if (m_fixedWidth)
					m_assembler.nop();
				m_assembler.stw(src, addrTempRegister, address.offset);
			}
        }
    }

    void store32(TrustedImm32 imm, BaseIndex address)
    {
        move(imm, immTempRegister);
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth) {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                stw     immTempRegister, address.offset(addrTemp)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.stw(immTempRegister, addrTempRegister, address.offset);
			}
			else
			{
				m_assembler.add(addrTempRegister, address.index, address.base);
				m_assembler.stw(immTempRegister, addrTempRegister, address.offset);
			}
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                stw     immTempRegister, (address.offset & 0xffff)(at)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				m_assembler.stw(immTempRegister, addrTempRegister, address.offset);
			}
			else
			{
				m_assembler.add(addrTempRegister, address.index, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				if (m_fixedWidth)
					m_assembler.nop();
				m_assembler.stw(immTempRegister, addrTempRegister, address.offset);
			}
        }
    }
    
    void store32(TrustedImm32 imm, ImplicitAddress address)
    {
        move(imm, immTempRegister);
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
		{
			m_assembler.stw(immTempRegister, address.base, address.offset);
        } else {
            /*
				li	    immTemp, imm
                addis   addrTemp, base, (offset + 0x8000) >> 16
                stw     immTemp, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
			m_assembler.stw(immTempRegister, addrTempRegister, address.offset);
        }
    }

    void store32(RegisterID src, const void* address)
    {
        /*
            li  addrTemp, address
            stw src, 0(addrTemp)
        */
        move(TrustedImmPtr(address), addrTempRegister);
        m_assembler.stw(src, addrTempRegister, 0);
    }

    void store32(TrustedImm32 imm, const void* address)
    {
        /*
            li   immTemp, imm
            li   addrTemp, address
            stw  src, 0(addrTemp)
        */
		move(imm, immTempRegister);
		move(TrustedImmPtr(address), addrTempRegister);
		m_assembler.stw(immTempRegister, addrTempRegister, 0);
    }

    // Floating-point operations:

    static bool supportsFloatingPoint() { return true; }
    static bool supportsFloatingPointTruncate() { return true; }
    static bool supportsFloatingPointSqrt() { return false; }       // REVISIT
    static bool supportsFloatingPointAbs() { return false; }        // REVISIT

    bool supportsDoubleBitops() const { return false; }

    // Stack manipulation operations:
    //
    // The ABI is assumed to provide a stack abstraction to memory,
    // containing machine word sized units of data.  Push and pop
    // operations add and remove a single register sized unit of data
    // to or from the stack.  Peek and poke operations read or write
    // values on the stack, without moving the current stack position.

    void pop(RegisterID dest)
    {
        m_assembler.lwz(dest, PPCRegisters::sp, 0);
        m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 4);
    }

    void push(RegisterID src)
    {
        m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, -4);
        m_assembler.stw(src, PPCRegisters::sp, 0);
    }

    void push(Address address)
    {
        load32(address, dataTempRegister);
        push(dataTempRegister);
    }

    void push(TrustedImm32 imm)
    {
        move(imm, immTempRegister);
        push(immTempRegister);
    }

    // Register move operations:
    //
    // Move values in registers.

    void move(TrustedImm32 imm, RegisterID dest)
    {
		bool bToLR;
		if (dest == PPCRegisters::lr)
			{
			dest = PPCRegisters::r0;
			bToLR = true;
			}
		else
			bToLR = false;
		
		if (imm.m_isPointer || m_fixedWidth)
		{
			m_assembler.lis(dest, imm.m_value >> 16);
			m_assembler.ori(dest, dest, imm.m_value);
		}
		else
			m_assembler.li(dest, imm.m_value);
			
		if (bToLR)
			m_assembler.mtlr(dest);
    }

    void move(RegisterID src, RegisterID dest)
    {
        if (src != dest || m_fixedWidth)
		{
			if (src == PPCRegisters::lr)
				m_assembler.mflr(dest);
			else if (dest == PPCRegisters::lr)
				m_assembler.mtlr(src);
			else
				m_assembler.move(dest, src);
		}
    }

    void move(TrustedImmPtr imm, RegisterID dest)
    {
        move(TrustedImm32(imm), dest);
    }

    void swap(RegisterID reg1, RegisterID reg2)
    {
        move(reg1, immTempRegister);
        move(reg2, reg1);
        move(immTempRegister, reg2);
    }

    void signExtend32ToPtr(RegisterID src, RegisterID dest)
    {
		// assumes 32-bit pointers
        if (src != dest || m_fixedWidth)
            move(src, dest);
    }

    void zeroExtend32ToPtr(RegisterID src, RegisterID dest)
    {
		// assumes 32-bit pointers
        if (src != dest || m_fixedWidth)
            move(src, dest);
    }

    // Forwards / external control flow operations:
    //
    // This set of jump and conditional branch operations return a Jump
    // object which may linked at a later point, allow forwards jump,
    // or jumps that will require external linkage (after the code has been
    // relocated).
    //
    // For branches, signed <, >, <= and >= are denoted as l, g, le, and ge
    // respecitvely, for unsigned comparisons the names b, a, be, and ae are
    // used (representing the names 'below' and 'above').
    //
    // Operands to the comparision are provided in the expected order, e.g.
    // jle32(reg1, TrustedImm32(5)) will branch if the value held in reg1, when
    // treated as a signed 32bit value, is less than or equal to 5.
    //
    // jz and jnz test whether the first operand is equal to zero, and take
    // an optional second operand of a mask under which to perform the test.

    Jump branch8(RelationalCondition cond, Address left, TrustedImm32 right)
    {
        // Make sure the immediate value is unsigned 8 bits.
        ASSERT(!(right.m_value & 0xFFFFFF00));
        load8(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    void compare8(RelationalCondition cond, Address left, TrustedImm32 right, RegisterID dest)
    {
        // Make sure the immediate value is unsigned 8 bits.
        ASSERT(!(right.m_value & 0xFFFFFF00));
        load8(left, dataTempRegister);
        compare32(cond, dataTempRegister, right, dest);
    }

    Jump branch32(RelationalCondition cond, RegisterID left, RegisterID right)
    {
        if (cond == Equal) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
            return branchEqual(PPCAssembler::CR0);
		}
        if (cond == NotEqual) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
            return branchNotEqual(PPCAssembler::CR0);
		}
        if (cond == Above) {
			m_assembler.cmpl(PPCAssembler::CR0, left, right);
            return branchGreater(PPCAssembler::CR0);
		}
        if (cond == AboveOrEqual) {
			m_assembler.cmpl(PPCAssembler::CR0, left, right);
            return branchGreaterEqual(PPCAssembler::CR0);
		}
        if (cond == Below) {
			m_assembler.cmpl(PPCAssembler::CR0, left, right);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == BelowOrEqual) {
			m_assembler.cmpl(PPCAssembler::CR0, left, right);
            return branchLessEqual(PPCAssembler::CR0);
        }
        if (cond == GreaterThan) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
            return branchGreater(PPCAssembler::CR0);
        }
        if (cond == GreaterThanOrEqual) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
            return branchGreaterEqual(PPCAssembler::CR0);
        }
        if (cond == LessThan) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == LessThanOrEqual) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
            return branchLessEqual(PPCAssembler::CR0);
        }
        ASSERT(0);

        return Jump();
    }

    Jump branch32(RelationalCondition cond, RegisterID left, TrustedImm32 right)
    {
		// make sure the order of the constants actually allows for our quick checks below
		ASSERT(GreaterThanOrEqual > GreaterThan);
		ASSERT(LessThan > GreaterThan);
		ASSERT(LessThanOrEqual > GreaterThan);
		ASSERT(Above > Equal);
		ASSERT(Above > NotEqual);
		ASSERT(AboveOrEqual > Above);
		ASSERT(Below > Above);
		ASSERT(BelowOrEqual > Above);
		ASSERT(Above < GreaterThan);
		ASSERT(Equal < Above);

		bool bEqual = (cond < Above);
		bool bSigned = (cond >= GreaterThan);
		if ((right.m_isPointer || m_fixedWidth ||
			(!bEqual &&  bSigned && (right.m_value < -32768		|| right.m_value > 32767)) ||		// signed comparison sign extends
			(!bEqual && !bSigned && (right.m_value < 0      	|| right.m_value > 65535)) ||		// unsigned comparison zero extends
			( bEqual &&			    ((right.m_value >> 16) != 0 && (right.m_value >> 16) != -1))))	// for equal we can choose as long as we only have variations in the lower 16 bits
		{
			// Either the immediate value didn't fit or we are working with pointers or we need to keep the code size constant
			move(right, immTempRegister);
			return branch32(cond, left, immTempRegister);
		}
		
        if (cond == Equal) {
			// choose signed or unsigned compare depending on the constant
			if (right.m_value >> 16)
				m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			else
				m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
            return branchEqual(PPCAssembler::CR0);
		}
        if (cond == NotEqual) {
			// choose signed or unsigned compare depending on the constant
			if (right.m_value >> 16)
				m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			else
				m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
            return branchNotEqual(PPCAssembler::CR0);
		}
        if (cond == Above) {
			m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
            return branchGreater(PPCAssembler::CR0);
		}
        if (cond == AboveOrEqual) {
			m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
            return branchGreaterEqual(PPCAssembler::CR0);
		}
        if (cond == Below) {
			m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == BelowOrEqual) {
			m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
            return branchLessEqual(PPCAssembler::CR0);
        }
        if (cond == GreaterThan) {
			m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
            return branchGreater(PPCAssembler::CR0);
        }
        if (cond == GreaterThanOrEqual) {
			m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
            return branchGreaterEqual(PPCAssembler::CR0);
        }
        if (cond == LessThan) {
			m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == LessThanOrEqual) {
			m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
            return branchLessEqual(PPCAssembler::CR0);
        }
        ASSERT(0);

        return Jump();
    }

    Jump branch32(RelationalCondition cond, RegisterID left, Address right)
    {
        load32(right, dataTempRegister);
        return branch32(cond, left, dataTempRegister);
    }

    Jump branch32(RelationalCondition cond, Address left, RegisterID right)
    {
        load32(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch32(RelationalCondition cond, Address left, TrustedImm32 right)
    {
        load32(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch32(RelationalCondition cond, BaseIndex left, TrustedImm32 right)
    {
        load32(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch32WithUnalignedHalfWords(RelationalCondition cond, BaseIndex left, TrustedImm32 right)
    {
        load32WithUnalignedHalfWords(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch32(RelationalCondition cond, AbsoluteAddress left, RegisterID right)
    {
        load32(left.m_ptr, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch32(RelationalCondition cond, AbsoluteAddress left, TrustedImm32 right)
    {
        load32(left.m_ptr, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch16(RelationalCondition cond, BaseIndex left, RegisterID right)
    {
        load16(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branch16(RelationalCondition cond, BaseIndex left, TrustedImm32 right)
    {
        ASSERT(!(right.m_value & 0xFFFF0000));
        load16(left, dataTempRegister);
        return branch32(cond, dataTempRegister, right);
    }

    Jump branchTest32(ResultCondition cond, RegisterID reg, RegisterID mask)
    {
        ASSERT((cond == Zero) || (cond == NonZero));
        m_assembler.andInsn(cmpTempRegister, reg, mask, true);
        if (cond == Zero)
            return branchEqual(PPCAssembler::CR0);
        return branchNotEqual(PPCAssembler::CR0);
    }

    Jump branchTest32(ResultCondition cond, RegisterID reg, TrustedImm32 mask = TrustedImm32(-1))
    {
        ASSERT((cond == Zero) || (cond == NonZero));
		if (!m_fixedWidth && !mask.m_isPointer)
		{
			if (mask.m_value == -1)
			{
				m_assembler.cmpi(PPCAssembler::CR0, reg, 0);
				return (cond == Zero) ? branchEqual(PPCAssembler::CR0) : branchNotEqual(PPCAssembler::CR0);
			}
			if (mask.m_value >= 0 && mask.m_value <= 65535)
			{
				m_assembler.andi(cmpTempRegister, reg, mask.m_value);	// always sets CR0!
				return (cond == Zero) ? branchEqual(PPCAssembler::CR0) : branchNotEqual(PPCAssembler::CR0);
			}
		}

        move(mask, immTempRegister);
        return branchTest32(cond, reg, immTempRegister);
    }

    Jump branchTest32(ResultCondition cond, Address address, TrustedImm32 mask = TrustedImm32(-1))
    {
        load32(address, dataTempRegister);
        return branchTest32(cond, dataTempRegister, mask);
    }

    Jump branchTest32(ResultCondition cond, BaseIndex address, TrustedImm32 mask = TrustedImm32(-1))
    {
        load32(address, dataTempRegister);
        return branchTest32(cond, dataTempRegister, mask);
    }

    Jump branchTest8(ResultCondition cond, Address address, TrustedImm32 mask = TrustedImm32(-1))
    {
        load8(address, dataTempRegister);
        return branchTest32(cond, dataTempRegister, mask);
    }

    Jump jump()
    {
        m_assembler.appendJump();
        m_assembler.bt(0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    void jump(RegisterID target)
    {
        m_assembler.mtctr(target);
        m_assembler.bctr();
    }

    void jump(Address address)
    {
        m_fixedWidth = true;
        load32(address, PPCRegisters::r0);
        m_assembler.mtctr(PPCRegisters::r0);
        m_assembler.bctr();
        m_fixedWidth = false;
    }

    void jump(AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), addrTempRegister);
        m_assembler.lwz(PPCRegisters::r0, addrTempRegister, 0);
        m_assembler.mtctr(PPCRegisters::r0);
        m_assembler.bctr();
    }

    // Arithmetic control flow operations:
    //
    // This set of conditional branch operations branch based
    // on the result of an arithmetic operation.  The operation
    // is performed as normal, storing the result.
    //
    // * jz operations branch if the result is zero.
    // * jo operations branch if the (signed) arithmetic
    //   operation caused an overflow to occur.

    Jump branchAdd32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        ASSERT((cond == Overflow) || (cond == Signed) || (cond == Zero) || (cond == NonZero));
        if (cond == Overflow) {
			m_assembler.addo(dest, dest, src);			// calculate the result; XER[OV] is set if overflow
			m_assembler.mcrxr(PPCAssembler::CR0);
            return branchGreater(PPCAssembler::CR0);	// branch if bit 1 (Motorola/IBM-style count) in CR0 is set (when copied from XER this is the OV bit)
        }
        if (cond == Signed) {
			m_assembler.add(dest, dest, src, true);
            // Check if dest is negative.
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == Zero) {
			m_assembler.add(dest, dest, src, true);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == NonZero) {
			m_assembler.add(dest, dest, src, true);
            return branchNotEqual(PPCAssembler::CR0);
        }
        ASSERT(0);
        return Jump();
    }

    Jump branchAdd32(ResultCondition cond, TrustedImm32 imm, AbsoluteAddress dest)
    {
        move(imm, immTempRegister);

        move(TrustedImmPtr(dest.m_ptr), addrTempRegister);
        m_assembler.lwz(dataTempRegister, addrTempRegister, 0);
        
        ASSERT((cond == Overflow) || (cond == Signed) || (cond == Zero) || (cond == NonZero));
        if (cond == Overflow) {
			m_assembler.addo(dataTempRegister, dataTempRegister, immTempRegister);	// calculate the result; XER[OV] is set if overflow
			m_assembler.mcrxr(PPCAssembler::CR0);
            m_assembler.stw(dataTempRegister, addrTempRegister, 0);
            return branchGreater(PPCAssembler::CR0);	// branch if bit 1 (Motorola/IBM-style count) in CR0 is set (when copied from XER this is the OV bit)
        }
        if (cond == Signed) {
			m_assembler.add(dataTempRegister, dataTempRegister, immTempRegister, true);
            m_assembler.stw(dataTempRegister, addrTempRegister, 0);
            // Check if dest is negative.
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == Zero) {
			m_assembler.add(dataTempRegister, dataTempRegister, immTempRegister, true);
            m_assembler.stw(dataTempRegister, addrTempRegister, 0);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == NonZero) {
			m_assembler.add(dataTempRegister, dataTempRegister, immTempRegister, true);
            m_assembler.stw(dataTempRegister, addrTempRegister, 0);
            return branchNotEqual(PPCAssembler::CR0);
        }
        ASSERT(0);
        return Jump();
    }
    
    Jump branchAdd32(ResultCondition cond, TrustedImm32 imm, RegisterID dest)
    {
        move(imm, immTempRegister);
        return branchAdd32(cond, immTempRegister, dest);
    }

    Jump branchAdd32(ResultCondition cond, RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        move(imm, immTempRegister);
        move(src, dest);
        return branchAdd32(cond, immTempRegister, dest);
    }

    Jump branchAdd32(ResultCondition cond, RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == dest)
            return branchAdd32(cond, op2, dest);
        else
        {
            move(op2, dest);
            return branchAdd32(cond, op1, dest);
        }
    }

    Jump branchMul32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        ASSERT((cond == Overflow) || (cond == Signed) || (cond == Zero) || (cond == NonZero));
        if (cond == Overflow) {
			m_assembler.mullwo(dest, src, dest);				// calculate the result; XER[OV] is set if overflow
			m_assembler.mcrxr(PPCAssembler::CR0);
            return branchGreater(PPCAssembler::CR0);			// branch if bit 1 (Motorola/IBM-style count) in CR0 is set (when copied from XER this is the OV bit)
		}
        if (cond == Signed) {
			m_assembler.mullw(dest, dest, src, true);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == Zero) {
			m_assembler.mullw(dest, dest, src, true);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == NonZero) {
			m_assembler.mullw(dest, dest, src, true);
            return branchNotEqual(PPCAssembler::CR0);
		}
        ASSERT(0);
        return Jump();
    }

    Jump branchMul32(ResultCondition cond, RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src1 == dest)
            return branchMul32(cond, src2, dest);
        else
        {
            move(src2, dest);
            return branchMul32(cond, src1, dest);
        }
    }

    Jump branchMul32(ResultCondition cond, TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        move(imm, immTempRegister);
        move(src, dest);
        return branchMul32(cond, immTempRegister, dest);
    }
    
    Jump branchNeg32(ResultCondition cond, RegisterID srcDest)
    {
        ASSERT((cond == Overflow) || (cond == Signed) || (cond == Zero) || (cond == NonZero));
        if (cond == Overflow) {
			m_assembler.nego(srcDest, srcDest);			// calculate the result; XER[OV] is set if overflow
			m_assembler.mcrxr(PPCAssembler::CR0);
            return branchGreater(PPCAssembler::CR0);	// branch if bit 1 (Motorola/IBM-style count) in CR0 is set (when copied from XER this is the OV bit)
        }
        if (cond == Signed) {
			m_assembler.neg(srcDest, srcDest, true);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == Zero) {
			m_assembler.neg(srcDest, srcDest, true);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == NonZero) {
			m_assembler.neg(srcDest, srcDest, true);
            return branchNotEqual(PPCAssembler::CR0);
        }
        ASSERT(0);
        return Jump();
    }

    Jump branchSub32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        ASSERT((cond == Overflow) || (cond == Signed) || (cond == Zero) || (cond == NonZero));
        if (cond == Overflow) {
			m_assembler.subo(dest, dest, src);			// calculate the result; XER[OV] is set if overflow
			m_assembler.mcrxr(PPCAssembler::CR0);
            return branchGreater(PPCAssembler::CR0);	// branch if bit 1 (Motorola/IBM-style count) in CR0 is set (when copied from XER this is the OV bit)
        }
        if (cond == Signed) {
			m_assembler.sub(dest, dest, src, true);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == Zero) {
			m_assembler.sub(dest, dest, src, true);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == NonZero) {
			m_assembler.sub(dest, dest, src, true);
            return branchNotEqual(PPCAssembler::CR0);
        }
        ASSERT(0);
        return Jump();
    }

    Jump branchSub32(ResultCondition cond, TrustedImm32 imm, RegisterID dest)
    {
        move(imm, immTempRegister);
        return branchSub32(cond, immTempRegister, dest);
    }

    Jump branchSub32(ResultCondition cond, RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        move(imm, immTempRegister);
        move(src, dest);
        return branchSub32(cond, immTempRegister, dest);
    }

    Jump branchSub32(ResultCondition cond, RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 != dest)
            move(op1, dest);
        return branchSub32(cond, op2, dest);
    }

    Jump branchOr32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        ASSERT((cond == Signed) || (cond == Zero) || (cond == NonZero));
        if (cond == Signed) {
			m_assembler.orInsn(dest, dest, src, true);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == Zero) {
			m_assembler.orInsn(dest, dest, src, true);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == NonZero) {
			m_assembler.orInsn(dest, dest, src, true);
            return branchNotEqual(PPCAssembler::CR0);
        }
        ASSERT(0);
        return Jump();
    }

    // Miscellaneous operations:

    void breakpoint()
    {
        m_assembler.bkpt();
    }

    Call nearCall()
    {
        /* We need three words for relaxation */
        m_assembler.nop();
        m_assembler.nop();
        m_assembler.nop();
        m_assembler.bl(0);
        return Call(m_assembler.label(), Call::LinkableNear);
    }

    Call call()
    {
        m_assembler.lis(PPCRegisters::r0, 0);
        m_assembler.ori(PPCRegisters::r0, PPCRegisters::r0, 0);
		m_assembler.mtlr(PPCRegisters::r0);
        m_assembler.blrl();
        return Call(m_assembler.label(), Call::Linkable);
    }

    Call call(RegisterID target)
    {
		m_assembler.mtlr(target);
        m_assembler.blrl();
        return Call(m_assembler.label(), Call::None);
    }

    Call call(Address address)
    {
        m_fixedWidth = true;
        load32(address, PPCRegisters::r0);
        m_assembler.mtlr(PPCRegisters::r0);
        m_assembler.blrl();
        m_fixedWidth = false;
        return Call(m_assembler.label(), Call::None);
    }

    void ret()
    {
        m_assembler.blr();
    }

    void compare32(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID dest)
    {
		/*
			CR0:    LT|GT|EQ|SO
			Shifts:  1  2  3  4
		*/
        if (cond == Equal) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
        } else if (cond == NotEqual) {
			m_assembler.cmp(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
        } else if (cond == Above) {
            m_assembler.cmpl(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else if (cond == AboveOrEqual) {
            m_assembler.cmpl(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == Below) {
            m_assembler.cmpl(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == BelowOrEqual) {
            m_assembler.cmpl(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else if (cond == GreaterThan) {
            m_assembler.cmp(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else if (cond == GreaterThanOrEqual) {
            m_assembler.cmp(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == LessThan) {
            m_assembler.cmp(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == LessThanOrEqual) {
            m_assembler.cmp(PPCAssembler::CR0, left, right);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else {
			ASSERT(0);
		}
    }

    void compare32(RelationalCondition cond, RegisterID left, TrustedImm32 right, RegisterID dest)
    {
		// make sure the order of the constants actually allows for our quick checks below
		ASSERT(GreaterThanOrEqual > GreaterThan);
		ASSERT(LessThan > GreaterThan);
		ASSERT(LessThanOrEqual > GreaterThan);
		ASSERT(Above > Equal);
		ASSERT(Above > NotEqual);
		ASSERT(AboveOrEqual > Above);
		ASSERT(Below > Above);
		ASSERT(BelowOrEqual > Above);
		ASSERT(Above < GreaterThan);
		ASSERT(Equal < Above);

		bool bEqual = (cond < Above);
		bool bSigned = (cond >= GreaterThan);
		if ((right.m_isPointer || m_fixedWidth ||
			(!bEqual &&  bSigned && (right.m_value < -32768		|| right.m_value > 32767)) ||		// signed comparison sign extends
			(!bEqual && !bSigned && (right.m_value < 0      	|| right.m_value > 65535)) ||		// unsigned comparison zero extends
			( bEqual &&			    ((right.m_value >> 16) != 0 && (right.m_value >> 16) != -1))))	// for equal we can choose as long as we only have variations in the lower 16 bits
		{
			// Either the immediate value didn't fit or we are working with pointers or we need to keep the code size constant
			move(right, immTempRegister);
			compare32(cond, left, immTempRegister, dest);
			return;
		}
		
		/*
			CR0:    LT|GT|EQ|SO
			Shifts:  1  2  3  4
		*/
        if (cond == Equal) {
			if (right.m_value >> 16)
				m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			else
				m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
        } else if (cond == NotEqual) {
			if (right.m_value >> 16)
				m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			else
				m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
        } else if (cond == Above) {
            m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else if (cond == AboveOrEqual) {
            m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == Below) {
            m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == BelowOrEqual) {
            m_assembler.cmpli(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else if (cond == GreaterThan) {
            m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else if (cond == GreaterThanOrEqual) {
            m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == LessThan) {
            m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 1, 31, 31);
        } else if (cond == LessThanOrEqual) {
            m_assembler.cmpi(PPCAssembler::CR0, left, right.m_value);
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 2, 31, 31);
        } else {
			ASSERT(0);
		}
    }

    void test8(ResultCondition cond, Address address, TrustedImm32 mask, RegisterID dest)
    {
        ASSERT((cond == Zero) || (cond == NonZero));
        load8(address, dataTempRegister);
		if (mask.m_value == -1)
		{
			m_assembler.cmpi(PPCAssembler::CR0, dataTempRegister, 0);
		}
		else if ((mask.m_value >> 16) == 0)
		{
			m_assembler.andi(cmpTempRegister, dataTempRegister, mask.m_value);		// always sets CR0
		}
		else
		{
			move(mask, immTempRegister);
			m_assembler.andInsn(cmpTempRegister, dataTempRegister, immTempRegister, true);
		}
		/*
			CR0:    LT|GT|EQ|SO
			Shifts:  1  2  3  4
		*/
		if (cond == Zero) {
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
		} else {
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
		}
    }

    void test32(ResultCondition cond, Address address, TrustedImm32 mask, RegisterID dest)
    {
	//opt: could further lower cost by using the bit-mask & roll opcodes PPC has if such cases are seen (often)
        ASSERT((cond == Zero) || (cond == NonZero));
        load32(address, dataTempRegister);
		if (mask.m_value == -1)
		{
			m_assembler.cmpi(PPCAssembler::CR0, dataTempRegister, 0);
		}
		else if ((mask.m_value >> 16) == 0)
		{
			m_assembler.andi(cmpTempRegister, dataTempRegister, mask.m_value);		// always sets CR0
		}
		else
		{
			move(mask, immTempRegister);
			m_assembler.andInsn(cmpTempRegister, dataTempRegister, immTempRegister, true);
		}
		/*
			CR0:    LT|GT|EQ|SO
			Shifts:  1  2  3  4
		*/
		if (cond == Zero) {
			m_assembler.mfcr(dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
		} else {
			m_assembler.mfcr(dest);
			m_assembler.nor(dest, dest, dest);
			m_assembler.rlwinm(dest, dest, 3, 31, 31);
		}
    }

    DataLabel32 moveWithPatch(TrustedImm32 imm, RegisterID dest)
    {
        m_fixedWidth = true;
        DataLabel32 label(this);
        move(imm, dest);
        m_fixedWidth = false;
        return label;
    }

    DataLabelPtr moveWithPatch(TrustedImmPtr initialValue, RegisterID dest)
    {
        m_fixedWidth = true;
        DataLabelPtr label(this);
        move(initialValue, dest);
        m_fixedWidth = false;
        return label;
    }

    Jump branchPtrWithPatch(RelationalCondition cond, RegisterID left, DataLabelPtr& dataLabel, TrustedImmPtr initialRightValue = TrustedImmPtr(0))
    {
        m_fixedWidth = true;
        dataLabel = moveWithPatch(initialRightValue, immTempRegister);
        Jump temp = branch32(cond, left, immTempRegister);
        m_fixedWidth = false;
        return temp;
    }

    Jump branchPtrWithPatch(RelationalCondition cond, Address left, DataLabelPtr& dataLabel, TrustedImmPtr initialRightValue = TrustedImmPtr(0))
    {
        m_fixedWidth = true;
        load32(left, dataTempRegister);
        dataLabel = moveWithPatch(initialRightValue, immTempRegister);
        Jump temp = branch32(cond, dataTempRegister, immTempRegister);
        m_fixedWidth = false;
        return temp;
    }

    DataLabelPtr storePtrWithPatch(TrustedImmPtr initialValue, ImplicitAddress address)
    {
        m_fixedWidth = true;
        DataLabelPtr dataLabel = moveWithPatch(initialValue, dataTempRegister);
        store32(dataTempRegister, address);
        m_fixedWidth = false;
        return dataLabel;
    }

    DataLabelPtr storePtrWithPatch(ImplicitAddress address)
    {
        return storePtrWithPatch(TrustedImmPtr(0), address);
    }

    Call tailRecursiveCall()
    {
        // Like a normal call, but don't update the returned address register
        m_fixedWidth = true;
        move(TrustedImm32(0), PPCRegisters::r0);		// note: this will be patched to some other value (lui, ori combo is needed!)
        m_assembler.mtctr(PPCRegisters::r0);
		m_assembler.bctr();
        m_fixedWidth = false;
        return Call(m_assembler.label(), Call::Linkable);
    }

    Call makeTailRecursiveCall(Jump oldJump)
    {
        oldJump.link(this);
        return tailRecursiveCall();
    }

    void moveDouble(FPRegisterID src, FPRegisterID dest)
    {
        if (src != dest)
            m_assembler.fmr(dest, src);
    }

    void loadDouble(ImplicitAddress address, FPRegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth) {
            m_assembler.lfd(dest, address.base, address.offset);
        } else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                lfd     dest, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lfd(dest, addrTempRegister, address.offset);
        }
    }

    void storeFloat(FPRegisterID src, ImplicitAddress address)
    {
        // floating-point registers always contain double-precision values
		m_assembler.frsp(fpTempRegister, src);
        
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth) {
            m_assembler.stfs(fpTempRegister, address.base, address.offset);
        } else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                stfs    src, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.stfs(fpTempRegister, addrTempRegister, address.offset);
        }
    }
    
    void loadFloat(ImplicitAddress address, FPRegisterID dest)
    {
        // NOTE-- 'lfs' converts single-precision float to double-precision value immediately!
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth) {
            m_assembler.lfs(dest, address.base, address.offset);
        } else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                lfs     dest, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.lfs(dest, addrTempRegister, address.offset);
        }
    }
    
    void loadDouble(const void* address, FPRegisterID dest)
    {
		/*
			lis     addrTemp, base, (offset + 0x8000) >> 16
			lfd     dest, (offset & 0xffff)(addrTemp)
		  */
		m_assembler.lis(addrTempRegister, ((intptr_t)address + 0x8000) >> 16);
		m_assembler.lfd(dest, addrTempRegister, (intptr_t)address & 0xffff);
    }

    void loadDouble(BaseIndex address, FPRegisterID dest)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
		{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                lfd     dest, address.offset(addrTemp)
            */
			m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.lfd(dest, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                lfd     dest, (address.offset & 0xffff)(at)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.lfd(dest, addrTempRegister, address.offset);
        }
    }

    void loadFloat(BaseIndex address, FPRegisterID dest)
    {
        // NOTE-- 'lfs' converts single-precision float to double-precision value immediately!
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
		{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                lfs     dest, address.offset(addrTemp)
            */
			m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.lfs(dest, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                lfs     dest, (address.offset & 0xffff)(at)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.lfs(dest, addrTempRegister, address.offset);
        }
    }
    
    void storeFloat(FPRegisterID src, BaseIndex address)
    {
        // floating-point registers always contain double-precision values
		m_assembler.frsp(fpTempRegister, src);
        
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
		{
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                stfs    dest, address.offset(addrTemp)
            */
			m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.stfs(fpTempRegister, addrTempRegister, address.offset);
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                stfs    dest, (address.offset & 0xffff)(at)
            */
            m_assembler.slwi(addrTempRegister, address.index, address.scale);
            m_assembler.add(addrTempRegister, addrTempRegister, address.base);
            m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
            m_assembler.stfs(fpTempRegister, addrTempRegister, address.offset);
        }
    }

    void storeDouble(FPRegisterID src, const void* address)
    {
        /*
            addis   addrTemp, base, (offset + 0x8000) >> 16
            stfd    src, (offset & 0xffff)(addrTemp)
        */
		m_assembler.lis(addrTempRegister, ((intptr_t)address + 0x8000) >> 16);
		m_assembler.stfd(src, addrTempRegister, (intptr_t)address & 0xffff);
    }

    void storeDouble(FPRegisterID src, ImplicitAddress address)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth)
            m_assembler.stfd(src, address.base, address.offset);
        else {
            /*
                addis   addrTemp, base, (offset + 0x8000) >> 16
                stfd    src, (offset & 0xffff)(addrTemp)
              */
            m_assembler.addis(addrTempRegister, address.base, (address.offset + 0x8000) >> 16);
            m_assembler.stfd(src, addrTempRegister, address.offset);
        }
    }

    void storeDouble(FPRegisterID src, BaseIndex address)
    {
        if (address.offset >= -32768 && address.offset <= 32767 && !m_fixedWidth) {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                stfd    src, address.offset(addrTemp)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.stfd(src, addrTempRegister, address.offset);
			}
			else
			{
				m_assembler.add(addrTempRegister, address.index, address.base);
				m_assembler.stfd(src, addrTempRegister, address.offset);
			}
        } else {
            /*
                slw     addrTemp, address.index, address.scale
                add     addrTemp, addrTemp, address.base
                addis   addrTemp, addrTemp, (address.offset + 0x8000) >> 16
                stfd    src, (address.offset & 0xffff)(at)
            */
			if (address.scale != TimesOne)
			{
				m_assembler.slwi(addrTempRegister, address.index, address.scale);
				m_assembler.add(addrTempRegister, addrTempRegister, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				m_assembler.stfd(src, addrTempRegister, address.offset);
			}
			else
			{
				m_assembler.add(addrTempRegister, address.index, address.base);
				m_assembler.addis(addrTempRegister, addrTempRegister, (address.offset + 0x8000) >> 16);
				if (m_fixedWidth)
					m_assembler.nop();
				m_assembler.stfd(src, addrTempRegister, address.offset);
			}
        }
    }

    void addDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        m_assembler.fadd(dest, op1, op2);
    }

    void addDouble(FPRegisterID src, FPRegisterID dest)
    {
        addDouble (src,dest,dest);
    }

    void addDouble(AbsoluteAddress address, FPRegisterID dest)
    {
        move(TrustedImmPtr(address.m_ptr), addrTempRegister);
		m_assembler.lfd(fpTempRegister, addrTempRegister, 0);
        m_assembler.fadd(dest, dest, fpTempRegister);
    }

    void addDouble(Address src, FPRegisterID dest)
    {
        loadDouble(src, fpTempRegister);
        m_assembler.fadd(dest, dest, fpTempRegister);
    }

    void subDouble(FPRegisterID src, FPRegisterID dest)
    {
        m_assembler.fsub(dest, dest, src);
    }

    void subDouble(Address src, FPRegisterID dest)
    {
        loadDouble(src, fpTempRegister);
        m_assembler.fsub(dest, dest, fpTempRegister);
    }

    void subDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        m_assembler.fsub(dest, op1, op2);
    }
    
    void mulDouble(FPRegisterID src, FPRegisterID dest)
    {
        m_assembler.fmul(dest, dest, src);
    }

    void mulDouble(Address src, FPRegisterID dest)
    {
        loadDouble(src, fpTempRegister);
        m_assembler.fmul(dest, dest, fpTempRegister);
    }

    void mulDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        m_assembler.fmul(dest, op1, op2);
    }
    
    void divDouble(FPRegisterID src, FPRegisterID dest)
    {
        m_assembler.fdiv(dest, dest, src);
    }

    void divDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        m_assembler.fdiv(dest, op1, op2);
    }
    
    void convertInt32ToDouble(RegisterID src, FPRegisterID dest)
    {
		/*
			stwu	sp, -16(sp)
			lis		r0, 0x4330
			xoris	dataTempRegister, src, 0x8000
			stw		r0, 8(sp)
			stw		dataTempRegister, 12(sp)
			lfd		dest, 8(sp)
			addi	sp, sp, 16
			fsub	dest, dest, fpMagicRegister
		*/
		m_assembler.stwu(PPCRegisters::sp, PPCRegisters::sp, -16);
		m_assembler.lis(PPCRegisters::r0, 0x4330);
		m_assembler.xoris(dataTempRegister, src, 0x8000);
		m_assembler.stw(PPCRegisters::r0, PPCRegisters::sp, 8);
		m_assembler.stw(dataTempRegister, PPCRegisters::sp, 12);
		m_assembler.lfd(dest, PPCRegisters::sp, 8);
		m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 16);
		m_assembler.fsub(dest, dest, fpMagicRegister);
    }

    void convertInt32ToDouble(Address src, FPRegisterID dest)
    {
        load32(src, dataTempRegister);
		convertInt32ToDouble(dataTempRegister, dest);
    }

    void convertInt32ToDouble(AbsoluteAddress src, FPRegisterID dest)
    {
        load32(src.m_ptr, dataTempRegister);
		convertInt32ToDouble(dataTempRegister, dest);
    }

    void convertFloatToDouble(FPRegisterID src, FPRegisterID dest)
    {
        // conversion is not necessary-- any floating-point register will already contain a double-precision value!
        moveDouble(src, dest);
	}
    
    void convertDoubleToFloat(FPRegisterID src, FPRegisterID dest)
    {
        // conversion is not necessary-- we will convert double-precision (64-bits) to single-precision (32 bits) when stored as float!
        moveDouble(src, dest);
    }
    
    void insertRelaxationWords()
    {
        /* We need four words for relaxation. */
        m_assembler.nop();
        m_assembler.nop();
        m_assembler.nop();
        m_assembler.nop();
    }
	
    Jump branchEqual(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.beq(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchNotEqual(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.bne(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchGreater(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.bgt(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchGreaterEqual(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.bge(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchLess(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.blt(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchLessEqual(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.ble(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchOverflow(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.bso(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchNoOverflow(PPCAssembler::CrID cr)
    {
        m_assembler.appendJump();
        m_assembler.bno(cr, 0);
        insertRelaxationWords();
        return Jump(m_assembler.label());
    }

    Jump branchDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right)
    {
        if (cond == DoubleEqual) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			// If unordered: false anyways
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleNotEqual) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.cror(2, 2, 3);				// If unordered: make it false
            return branchNotEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleGreaterThan) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			// If unordered: false anyways
            return branchGreater(PPCAssembler::CR0);
        }
        if (cond == DoubleGreaterThanOrEqual) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.cror(0, 0, 3);				// If unordered: make it false
            return branchGreaterEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleLessThan) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			// If unordered: false anyways
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == DoubleLessThanOrEqual) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.cror(1, 1, 3);				// If unordered: make it false
            return branchLessEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleEqualOrUnordered) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.cror(2, 2, 3);
            return branchEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleNotEqualOrUnordered) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.crandc(2, 2, 3);
            return branchNotEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleGreaterThanOrUnordered) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.cror(1, 1, 3);
            return branchGreater(PPCAssembler::CR0);
        }
        if (cond == DoubleGreaterThanOrEqualOrUnordered) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.crandc(0, 0, 3);
            return branchGreaterEqual(PPCAssembler::CR0);
        }
        if (cond == DoubleLessThanOrUnordered) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.cror(0, 0, 3);
            return branchLess(PPCAssembler::CR0);
        }
        if (cond == DoubleLessThanOrEqualOrUnordered) {
			m_assembler.fcmpu(PPCAssembler::CR0, left, right);
			m_assembler.crandc(1, 1, 3);
            return branchLessEqual(PPCAssembler::CR0);
        }
        ASSERT(0);

        return Jump();
    }

    void truncateDoubleToInt32(FPRegisterID src, RegisterID dest)
    {
		m_assembler.stwu(PPCRegisters::sp, PPCRegisters::sp, -8);
		m_assembler.fctiwz(fpTempRegister, src);
		m_assembler.addi(addrTempRegister, PPCRegisters::sp, 4);
		m_assembler.stfiwx(fpTempRegister, (RegisterID)0, addrTempRegister);
		m_assembler.lwz(dest, PPCRegisters::sp, 4);
		m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 8);
    }

    void truncateDoubleToUint32(FPRegisterID src, RegisterID dest)
    {
        truncateDoubleToUint32 (src,dest);
    }
    
    enum BranchTruncateType { BranchIfTruncateFailed, BranchIfTruncateSuccessful };
    
    // Truncates 'src' to an integer, and places the resulting 'dest'.
    // If the result is not representable as a 32 bit value, branch.
    Jump branchTruncateDoubleToInt32(FPRegisterID src, RegisterID dest, BranchTruncateType branchType = BranchIfTruncateFailed)
    {
		// note: this will reject 0x80000000 as well as 0x7fffffff, although they are very much valid
		m_assembler.stwu(PPCRegisters::sp, PPCRegisters::sp, -16);

        m_assembler.xorInsn(cmpTempRegister, cmpTempRegister, cmpTempRegister);    // assume that conversion will be a failure
		m_assembler.lis(PPCRegisters::r0, 0x8000);  // 0x80000000
        
        // check for negative-zero (which cannot be represented in twos-complement)
		m_assembler.stfd(src, PPCRegisters::sp, 8);
		m_assembler.lwz(dataTempRegister, PPCRegisters::sp, 8);
		m_assembler.cmp(PPCAssembler::CR0, dataTempRegister, PPCRegisters::r0);
        Jump notNegativeZero = branchNotEqual(PPCAssembler::CR0);

		m_assembler.lwz(dataTempRegister, PPCRegisters::sp, 12);
		m_assembler.lis(immTempRegister, 0x0000);
		m_assembler.cmp(PPCAssembler::CR0, dataTempRegister, immTempRegister);
        Jump isNegativeZero = branchEqual(PPCAssembler::CR0);

        notNegativeZero.link (this);

        // convert 64-bit double to 32-bit integer        
		m_assembler.fctiwz(fpTempRegister, src);
		m_assembler.nor(immTempRegister, PPCRegisters::r0, PPCRegisters::r0);	// 0x7fffffff
		m_assembler.stfd(fpTempRegister, PPCRegisters::sp, 8);
		m_assembler.lwz(dest, PPCRegisters::sp, 12);
		m_assembler.cmp(PPCAssembler::CR0, dest, PPCRegisters::r0);
		m_assembler.cmp(PPCAssembler::CR1, dest, immTempRegister);
		m_assembler.cror(2, 2, 6);
		Jump tooBigFor32Bit = branchEqual(PPCAssembler::CR0);

		m_assembler.addi(cmpTempRegister, cmpTempRegister, 0x0001);     // the conversion was a success!

        tooBigFor32Bit.link (this);
        isNegativeZero.link (this);

		m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 16);
        
		m_assembler.cmpi(PPCAssembler::CR0, cmpTempRegister, 0);
		return (branchType == BranchIfTruncateSuccessful) ? branchNotEqual(PPCAssembler::CR0) : branchEqual(PPCAssembler::CR0);
    }

    Jump branchTruncateDoubleToUint32(FPRegisterID src, RegisterID dest, BranchTruncateType branchType = BranchIfTruncateFailed)
    {
		// note: this will reject 0x80000000 as well as 0x7fffffff, although they are very much valid
		m_assembler.stwu(PPCRegisters::sp, PPCRegisters::sp, -16);

        m_assembler.xorInsn(cmpTempRegister, cmpTempRegister, cmpTempRegister);    // assume that conversion will be a failure
		m_assembler.lis(PPCRegisters::r0, 0x8000);								// 0x80000000
        
        // check for negative-zero (which cannot be represented in twos-complement)
		m_assembler.stfd(src, PPCRegisters::sp, 8);
		m_assembler.lwz(dataTempRegister, PPCRegisters::sp, 8);
		m_assembler.cmp(PPCAssembler::CR0, dataTempRegister, PPCRegisters::r0);
        Jump notNegativeZero = branchNotEqual(PPCAssembler::CR0);

		m_assembler.lwz(dataTempRegister, PPCRegisters::sp, 12);
		m_assembler.lis(immTempRegister, 0x0000);
		m_assembler.cmp(PPCAssembler::CR0, dataTempRegister, immTempRegister);
        Jump isNegativeZero = branchEqual(PPCAssembler::CR0);

        notNegativeZero.link (this);

        // convert 64-bit double to 32-bit integer        
		m_assembler.fctiwz(fpTempRegister, src);
		m_assembler.nor(immTempRegister, PPCRegisters::r0, PPCRegisters::r0);	// 0x7fffffff
		m_assembler.stfd(fpTempRegister, PPCRegisters::sp, 8);
		m_assembler.lwz(dest, PPCRegisters::sp, 12);
        
        // check for an unsigned result
		m_assembler.cmpi(PPCAssembler::CR0, dest, 0);
        Jump isNegative = branchLess(PPCAssembler::CR0);
        
		m_assembler.cmp(PPCAssembler::CR0, dest, immTempRegister);
		Jump tooBigFor32Bit = branchEqual(PPCAssembler::CR0);

		m_assembler.addi(cmpTempRegister, cmpTempRegister, 0x0001);     // the conversion was a success!

        isNegative.link (this);
        tooBigFor32Bit.link (this);
        isNegativeZero.link (this);

		m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 16);
        
		m_assembler.cmpi(PPCAssembler::CR0, cmpTempRegister, 0);
		return (branchType == BranchIfTruncateSuccessful) ? branchNotEqual(PPCAssembler::CR0) : branchEqual(PPCAssembler::CR0);
    }


    // Convert 'src' to an integer, and places the resulting 'dest'.
    // If the result is not representable as a 32 bit value, branch.
    void branchConvertDoubleToInt32(FPRegisterID src, RegisterID dest, JumpList& failureCases, FPRegisterID fpTemp)
    {
		// note: this will reject 0x80000000 as well as 0x7fffffff, although they are very much valid
		m_assembler.stwu(PPCRegisters::sp, PPCRegisters::sp, -16);

        m_assembler.xorInsn(cmpTempRegister, cmpTempRegister, cmpTempRegister);    // assume that conversion will be a failure
		m_assembler.lis(PPCRegisters::r0, 0x8000);  // 0x80000000
        
        // check for negative-zero (which cannot be represented in twos-complement)
		m_assembler.stfd(src, PPCRegisters::sp, 8);
		m_assembler.lwz(dataTempRegister, PPCRegisters::sp, 8);
		m_assembler.cmp(PPCAssembler::CR0, dataTempRegister, PPCRegisters::r0);
        Jump notNegativeZero = branchNotEqual(PPCAssembler::CR0);

		m_assembler.lwz(dataTempRegister, PPCRegisters::sp, 12);
		m_assembler.lis(immTempRegister, 0x0000);
		m_assembler.cmp(PPCAssembler::CR0, dataTempRegister, immTempRegister);
        Jump isNegativeZero = branchEqual(PPCAssembler::CR0);

        notNegativeZero.link (this);

        // convert 64-bit double to 32-bit integer        
		m_assembler.fctiwz(fpTempRegister, src);
		m_assembler.nor(immTempRegister, PPCRegisters::r0, PPCRegisters::r0);	// 0x7fffffff
		m_assembler.stfd(fpTempRegister, PPCRegisters::sp, 8);
		m_assembler.lwz(dest, PPCRegisters::sp, 12);
		m_assembler.cmp(PPCAssembler::CR0, dest, PPCRegisters::r0);
		m_assembler.cmp(PPCAssembler::CR1, dest, immTempRegister);
		m_assembler.cror(2, 2, 6);
		Jump tooBigFor32Bit = branchEqual(PPCAssembler::CR0);

		m_assembler.addi(cmpTempRegister, cmpTempRegister, 0x0001);     // the conversion was a success!

        tooBigFor32Bit.link (this);
        isNegativeZero.link (this);

		m_assembler.addi(PPCRegisters::sp, PPCRegisters::sp, 16);
        
		m_assembler.cmpi(PPCAssembler::CR0, cmpTempRegister, 0);
		failureCases.append (branchEqual(PPCAssembler::CR0));

        // Convert the integer result back to float & compare to the original value - if not equal or unordered (NaN) then jump.
        convertInt32ToDouble (dest, fpTemp);
        failureCases.append(branchDouble (DoubleNotEqual, src, fpTemp));
    }

    Jump branchDoubleNonZero(FPRegisterID reg, FPRegisterID scratch)
    {
        return branchDouble(DoubleNotEqual, reg, fpZeroRegister);
    }

    Jump branchDoubleZeroOrNaN(FPRegisterID reg, FPRegisterID scratch)
    {
        return branchDouble(DoubleEqualOrUnordered, reg, fpZeroRegister);
    }

    void nop()
    {
        m_assembler.nop();
    }

    static FunctionPtr readCallTarget(CodeLocationCall call)
    {
        return FunctionPtr(reinterpret_cast<void(*)()>(PPCAssembler::readCallTarget(call.dataLocation())));
    }

    void* code() const
    {
        return m_assembler.code();
    }

    size_t codeSize() const
    {
        return m_assembler.codeSize();
    }

    
private:
    // If m_fixedWidth is true, we will generate a fixed number of instructions.
    // Otherwise, we can emit any number of instructions.
    bool m_fixedWidth;

    friend class LinkBuffer;
    friend class RepatchBuffer;

    static void linkCall(void* code, Call call, FunctionPtr function)
    {
        PPCAssembler::linkCall(code, call.m_label, function.value());
    }

    static void repatchCall(CodeLocationCall call, CodeLocationLabel destination)
    {
        PPCAssembler::relinkCall(call.dataLocation(), destination.executableAddress());
    }

    static void repatchCall(CodeLocationCall call, FunctionPtr destination)
    {
        PPCAssembler::relinkCall(call.dataLocation(), destination.executableAddress());
    }

};

}

#endif // ENABLE(ASSEMBLER) && CPU(PPC)

#endif // MacroAssemblerPPC_h
