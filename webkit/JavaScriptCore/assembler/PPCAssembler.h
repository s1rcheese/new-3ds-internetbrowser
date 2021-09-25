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

#ifndef PPCAssembler_h
#define PPCAssembler_h

#if ENABLE(ASSEMBLER) && CPU(PPC)

#include "AssemblerBuffer.h"
#include <wtf/Assertions.h>
#include <wtf/SegmentedVector.h>

namespace JSC {

typedef uint32_t PPCWord;

namespace PPCRegisters {
typedef enum {
    r0 = 0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    r16,
    r17,
    r18,
    r19,
    r20,
    r21,
    r22,
    r23,
    r24,
    r25,
    r26,
    r27,
    r28,
    r29,
    r30,
    r31,
    zero = r0,
    sp = r1,
	lr = 0xFF
} RegisterID;

typedef enum {
    f0,
    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    f11,
    f12,
    f13,
    f14,
    f15,
    f16,
    f17,
    f18,
    f19,
    f20,
    f21,
    f22,
    f23,
    f24,
    f25,
    f26,
    f27,
    f28,
    f29,
    f30,
    f31
} FPRegisterID;

} // namespace PPCRegisters

class PPCAssembler {
public:
    typedef PPCRegisters::RegisterID RegisterID;
    typedef PPCRegisters::FPRegisterID FPRegisterID;
    typedef SegmentedVector<AssemblerLabel, 64> Jumps;

    PPCAssembler()
    {
    }

    // MIPS instruction opcode field position
    enum {
        OP_SH_RD = 11,
        OP_SH_RT = 16,
        OP_SH_RS = 21,
        OP_SH_SHAMT = 6,
        OP_SH_CODE = 16,
        OP_SH_FD = 6,
        OP_SH_FS = 11,
        OP_SH_FT = 16
    };
	
	// PPC logical instructions
	enum {
		LOGIC_OP_SH_OP1 = 26,
		LOGIC_OP_SH_RS = 21,
		LOGIC_OP_SH_RA = 16,
		LOGIC_OP_SH_RB = 11,
		LOGIC_OP_SH_SH = 11,
		LOGIC_OP_SH_OP2 = 1
	};

	// PPC arithmetic instructions
	enum {
		ARITH_OP_SH_OP1 = 26,
		ARITH_OP_SH_RD = 21,
		ARITH_OP_SH_RA = 16,
		ARITH_OP_SH_RB = 11,
		ARITH_OP_SH_OP2 = 1
	};

	// PPC float instructions
	enum {
		FLOAT_OP_SH_OP1 = 26,
		FLOAT_OP_SH_RD = 21,
		FLOAT_OP_SH_RA = 16,
		FLOAT_OP_SH_RB = 11,
		FLOAT_OP_SH_RC = 6,
		FLOAT_OP_SH_OP2 = 1
	};

	// PPC roll instructions
	enum {
		ROLL_OP_SH_OP1 = 26,
		ROLL_OP_SH_RS = 21,
		ROLL_OP_SH_RA = 16,
		ROLL_OP_SH_RB = 11,
		ROLL_OP_SH_SH = 11,
		ROLL_OP_SH_MB = 6,
		ROLL_OP_SH_ME = 1
	};

	// PPC branch instructions
	enum {
		BRANCH_OP_SH_OP1 = 26,
		BRANCH_OP_SH_BO = 21,
		BRANCH_OP_SH_BI = 16,
		BRANCH_OP_SH_BD = 2,
		BRANCH_OP_SH_LK = 0,
		BRANCH_OP_SH_OP2 = 1
	};

	// PPC move special instructions
	enum {
		MOVSPR_OP_SH_OP1 = 26,
		MOVSPR_OP_SH_RD = 21,
		MOVSPR_OP_SH_SPR = 11,
		MOVSPR_OP_SH_OP2 = 1
	};

	// PPC compare instructions
	enum {
		CMP_OP_SH_OP1 = 26,
		CMP_OP_SH_CR = 21,
		CMP_OP_SH_RA = 16,
		CMP_OP_SH_RB = 11,
		CMP_OP_SH_OP2 = 1
	};
	
	enum CrID {
		CR0 = 0,
		CR1 = 4,
		CR2 = 8,
		CR3 = 12,
		CR4 = 16,
		CR5 = 20,
		CR6 = 24,
		CR7 = 28
	};
	
	enum {
		PPC_NOP = 24 << LOGIC_OP_SH_OP1,	// ori 0, 0, 0
		SPR_LR	= 0x100,
		SPR_CTR	= 0x120,
	};

    void emitInst(PPCWord op)
    {
        void* oldBase = m_buffer.data();

        m_buffer.putInt(op);

        void* newBase = m_buffer.data();
        if (oldBase != newBase)
            relocateJumps(oldBase, newBase);
    }

    void nop()
    {
        emitInst(PPC_NOP);
    }

    void move(RegisterID rd, RegisterID rs)
    {
        /* or rd, rs, rs */
        emitInst((31 << LOGIC_OP_SH_OP1) | (444 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (rs << LOGIC_OP_SH_RB));
    }

    /* Set an immediate value to a register.  This may generate 1 or 2
       instructions.  */
    void li(RegisterID dest, int imm)
    {
        if (imm >= -32768 && imm <= 32767)
            addi(dest, PPCRegisters::zero, imm);
        else {
            lis(dest, imm >> 16);
            if (imm & 0xffff)
                ori(dest, dest, imm);
        }
    }

    void lis(RegisterID rt, int imm)
    {
		/* addis rd, 0, imm */
        emitInst((15 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (PPCRegisters::zero << ARITH_OP_SH_RA) | (imm & 0xFFFF));
    }

    void addi(RegisterID rt, RegisterID rs, int imm)
    {
        emitInst((14 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (imm & 0xFFFF));
    }

    void addis(RegisterID rt, RegisterID rs, int imm)
    {
		/* addis rd, rs, imm */
        emitInst((15 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (imm & 0xFFFF));
    }

    void add(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        emitInst((31 << ARITH_OP_SH_OP1) | (266 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (rt << ARITH_OP_SH_RB) | flags);
    }

    void addo(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        emitInst((31 << ARITH_OP_SH_OP1) | (266 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (rt << ARITH_OP_SH_RB) | (1 << 10) | flags);
    }

    void sub(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
		/* subf rd, rt, rs */
        emitInst((31 << ARITH_OP_SH_OP1) | (40 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rt << ARITH_OP_SH_RA) | (rs << ARITH_OP_SH_RB) | flags);
    }

    void subo(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
		/* subf rd, rt, rs */
        emitInst((31 << ARITH_OP_SH_OP1) | (40 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rt << ARITH_OP_SH_RA) | (rs << ARITH_OP_SH_RB) | (1 << 10) | flags);
    }

    void neg(RegisterID rd, RegisterID rs, bool flags = false)
    {
		/* neg rd, rs */
        emitInst((31 << ARITH_OP_SH_OP1) | (104 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | flags);
    }

    void nego(RegisterID rd, RegisterID rs, bool flags = false)
    {
		/* neg rd, rs */
        emitInst((31 << ARITH_OP_SH_OP1) | (104 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (1 << 10) | flags);
    }

    void mullw(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        emitInst((31 << ARITH_OP_SH_OP1) | (235 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (rt << ARITH_OP_SH_RB) | flags);
    }

    void mulli(RegisterID rd, RegisterID rs, int imm)
    {
        emitInst((7 << ARITH_OP_SH_OP1) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (imm & 0xffff));
    }

    void mullwo(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        emitInst((31 << ARITH_OP_SH_OP1) | (235 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (rt << ARITH_OP_SH_RB) | (1 << 10) | flags);
    }

    void mulhw(RegisterID rd, RegisterID rs, RegisterID rt)
    {
        emitInst((31 << ARITH_OP_SH_OP1) | (75 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (rt << ARITH_OP_SH_RB));
    }

    void divw(RegisterID rd, RegisterID rs, RegisterID rt)
    {
        emitInst((31 << ARITH_OP_SH_OP1) | (491 << ARITH_OP_SH_OP2) | (rd << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (rt << ARITH_OP_SH_RB));
    }

    void andInsn(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        /* and rd, rs, rt */
        emitInst((31 << LOGIC_OP_SH_OP1) | (28 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (rt << LOGIC_OP_SH_RB) | flags);
    }

    void andi(RegisterID rt, RegisterID rs, int imm)
    {
		emitInst((28 << LOGIC_OP_SH_OP1) | (rt << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (imm & 0xffff));
    }

    void nor(RegisterID rd, RegisterID rs, RegisterID rt)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (124 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (rt << LOGIC_OP_SH_RB));
    }

    void orInsn(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        /* or rd, rs, rt */
        emitInst((31 << LOGIC_OP_SH_OP1) | (444 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (rt << LOGIC_OP_SH_RB) | flags);
    }

    void ori(RegisterID rt, RegisterID rs, int imm)
    {
        emitInst((24 << LOGIC_OP_SH_OP1) | (rt << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (imm & 0xffff));
    }

    void xorInsn(RegisterID rd, RegisterID rs, RegisterID rt, bool flags = false)
    {
        /* xor rd, rs, rt */
        emitInst((31 << LOGIC_OP_SH_OP1) | (316 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (rt << LOGIC_OP_SH_RB) | flags);
    }

    void xori(RegisterID rt, RegisterID rs, int imm)
    {
        emitInst((26 << LOGIC_OP_SH_OP1) | (rt << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (imm & 0xffff));
    }
	
    void xoris(RegisterID rt, RegisterID rs, int imm)
    {
        emitInst((27 << LOGIC_OP_SH_OP1) | (rt << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RS) | (imm & 0xffff));
    }
	
	void cmp(CrID cr, RegisterID rt, RegisterID rs)
	{
        emitInst((31 << CMP_OP_SH_OP1) | (cr << CMP_OP_SH_CR) | (rt << CMP_OP_SH_RA) | (rs << CMP_OP_SH_RB));
	}

	void cmpl(CrID cr, RegisterID rt, RegisterID rs)
	{
        emitInst((31 << CMP_OP_SH_OP1) | (32 << CMP_OP_SH_OP2) | (cr << CMP_OP_SH_CR) | (rt << CMP_OP_SH_RA) | (rs << CMP_OP_SH_RB));
	}
	
	void cmpi(CrID cr, RegisterID rt, int imm)
	{
        emitInst((11 << CMP_OP_SH_OP1) | (cr << CMP_OP_SH_CR) | (rt << CMP_OP_SH_RA) | (imm & 0xffff));
	}

	void cmpli(CrID cr, RegisterID rt, int imm)
	{
        emitInst((10 << CMP_OP_SH_OP1) | (cr << CMP_OP_SH_CR) | (rt << CMP_OP_SH_RA) | (imm & 0xffff));
	}

    void cror(int d, int a, int b)
    {
        emitInst((19 << LOGIC_OP_SH_OP1) | (449 << LOGIC_OP_SH_OP2) | (d << LOGIC_OP_SH_RS) | (a << LOGIC_OP_SH_RA) | (b << LOGIC_OP_SH_RB));
    }
	
    void crandc(int d, int a, int b)
    {
        emitInst((19 << LOGIC_OP_SH_OP1) | (129 << LOGIC_OP_SH_OP2) | (d << LOGIC_OP_SH_RS) | (a << LOGIC_OP_SH_RA) | (b << LOGIC_OP_SH_RB));
    }
	
    void extsb(RegisterID rt, RegisterID rs)
    {
		emitInst((31 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (954 << ARITH_OP_SH_OP2));
    }
    
    void extsh(RegisterID rt, RegisterID rs)
    {
		emitInst((31 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (922 << ARITH_OP_SH_OP2));
    }
    
	void mfcr(RegisterID rt)
	{
		emitInst((31 << ARITH_OP_SH_OP1) | (19 << ARITH_OP_SH_OP2) | (rt << ARITH_OP_SH_RD));
	}

    void slwi(RegisterID rd, RegisterID rt, int shamt)
    {
		/* rlwinm rd, rt, shamt, 0, 31 - shamt */
        emitInst((21 << ROLL_OP_SH_OP1) | (rd << ROLL_OP_SH_RA) | (rt << ROLL_OP_SH_RS) | ((shamt & 0x1f) << ROLL_OP_SH_SH) | ((31 - (shamt & 0x1f)) << ROLL_OP_SH_ME));
    }

    void slw(RegisterID rd, RegisterID rt, RegisterID rs)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (24 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rt << LOGIC_OP_SH_RS) | (rs << LOGIC_OP_SH_RB));
    }

    void srawi(RegisterID rd, RegisterID rt, int shamt)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (824 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rt << LOGIC_OP_SH_RS) | ((shamt & 0x1f) << LOGIC_OP_SH_SH));
    }

    void sraw(RegisterID rd, RegisterID rt, RegisterID rs)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (792 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rt << LOGIC_OP_SH_RS) | (rs << LOGIC_OP_SH_RB));
    }

    void srwi(RegisterID rd, RegisterID rt, int shamt)
    {
		/* rlwinm rd, rt, 32 - shamt, shamt, 31 */
        emitInst((21 << ROLL_OP_SH_OP1) | (rd << ROLL_OP_SH_RA) | (rt << ROLL_OP_SH_RS) | ((32 - (shamt & 0x1f)) << ROLL_OP_SH_SH) | ((shamt & 0x1f) << ROLL_OP_SH_MB) | (31 << ROLL_OP_SH_ME));
    }

    void srw(RegisterID rd, RegisterID rt, RegisterID rs)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (536 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RA) | (rt << LOGIC_OP_SH_RS) | (rs << LOGIC_OP_SH_RB));
    }
	
    void rlwinm(RegisterID rd, RegisterID rt, int shamt, int mb, int me)
    {
        emitInst((21 << ROLL_OP_SH_OP1) | (rd << ROLL_OP_SH_RA) | (rt << ROLL_OP_SH_RS) | ((shamt & 0x1f) << ROLL_OP_SH_SH) | (mb << ROLL_OP_SH_MB) | (me << ROLL_OP_SH_ME));
    }

    void lbz(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((34 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void lwz(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((32 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void lhz(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((40 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void stb(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((38 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void sth(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((44 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void stw(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((36 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }
	
    void stwu(RegisterID rt, RegisterID rs, int offset)
    {
        emitInst((37 << ARITH_OP_SH_OP1) | (rt << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }
	
    void lhbrx(RegisterID rd, RegisterID rt, RegisterID rs)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (790 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RS) | (rt << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RB));
    }
	
    void lwbrx(RegisterID rd, RegisterID rt, RegisterID rs)
    {
        emitInst((31 << LOGIC_OP_SH_OP1) | (534 << LOGIC_OP_SH_OP2) | (rd << LOGIC_OP_SH_RS) | (rt << LOGIC_OP_SH_RA) | (rs << LOGIC_OP_SH_RB));
    }
	
	void mtlr(RegisterID rs)
	{
		/* mtspr 8, rs */
        emitInst((31 << MOVSPR_OP_SH_OP1) | (rs << MOVSPR_OP_SH_RD) | (SPR_LR << MOVSPR_OP_SH_SPR) | (467 << MOVSPR_OP_SH_OP2));
	}

	void mflr(RegisterID rt)
	{
		/* mfspr 8, rt */
        emitInst((31 << MOVSPR_OP_SH_OP1) | (rt << MOVSPR_OP_SH_RD) | (SPR_LR << MOVSPR_OP_SH_SPR) | (339 << MOVSPR_OP_SH_OP2));
	}

	void mtctr(RegisterID rs)
	{
		/* mtspr 9, rs */
        emitInst((31 << MOVSPR_OP_SH_OP1) | (rs << MOVSPR_OP_SH_RD) | (SPR_CTR << MOVSPR_OP_SH_SPR) | (467 << MOVSPR_OP_SH_OP2));
	}

	void mfctr(RegisterID rt)
	{
		/* mfspr 9, rt */
        emitInst((31 << MOVSPR_OP_SH_OP1) | (rt << MOVSPR_OP_SH_RD) | (SPR_CTR << MOVSPR_OP_SH_SPR) | (339 << MOVSPR_OP_SH_OP2));
	}

	void mtfsb0(int bit)
	{
        emitInst((63 << MOVSPR_OP_SH_OP1) | (bit << MOVSPR_OP_SH_RD) | (70 << MOVSPR_OP_SH_OP2));
	}

	void mffs(RegisterID rt)
	{
        emitInst((63 << MOVSPR_OP_SH_OP1) | (rt << MOVSPR_OP_SH_RD) | (583 << MOVSPR_OP_SH_OP2));
	}
	
	void mcrxr(CrID cr)
	{
        emitInst((31 << MOVSPR_OP_SH_OP1) | ((cr << 2) << MOVSPR_OP_SH_RD) | (512 << MOVSPR_OP_SH_OP2));
	}
	
    void bctr(void)
    {
        emitInst((19 << BRANCH_OP_SH_OP1) | (528 << BRANCH_OP_SH_OP2) | (20 << BRANCH_OP_SH_BO));
    }

    void blr(void)
    {
        emitInst((19 << BRANCH_OP_SH_OP1) | (16 << BRANCH_OP_SH_OP2) | (20 << BRANCH_OP_SH_BO));
    }

    void bctrl(void)
    {
        emitInst((19 << BRANCH_OP_SH_OP1) | (528 << BRANCH_OP_SH_OP2) | (20 << BRANCH_OP_SH_BO) | (1 << BRANCH_OP_SH_LK));
    }

    void blrl(void)
    {
        emitInst((19 << BRANCH_OP_SH_OP1) | (16 << BRANCH_OP_SH_OP2) | (20 << BRANCH_OP_SH_BO) | (1 << BRANCH_OP_SH_LK));
    }

    void bkpt()
    {
		/* illegal */
        emitInst(0x00000000);
    }

    void b(int imm)
    {
		/* b imm */
        emitInst((18 << BRANCH_OP_SH_OP1) | (((imm & 0x03fffffc) >> 2) << BRANCH_OP_SH_BD));
    }

    void bl(int imm)
    {
		/* bl imm */
        emitInst((18 << BRANCH_OP_SH_OP1) | (((imm & 0x03fffffc) >> 2) << BRANCH_OP_SH_BD) | (1 << BRANCH_OP_SH_LK));
    }

    void bt(int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (20 << BRANCH_OP_SH_BO) | (0 << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void bge(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (4 << BRANCH_OP_SH_BO) | ((cr + 0) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void blt(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (12 << BRANCH_OP_SH_BO) | ((cr + 0) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void ble(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (4 << BRANCH_OP_SH_BO) | ((cr + 1) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void bgt(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (12 << BRANCH_OP_SH_BO) | ((cr + 1) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void bne(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (4 << BRANCH_OP_SH_BO) | ((cr + 2) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void beq(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (12 << BRANCH_OP_SH_BO) | ((cr + 2) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void bso(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (12 << BRANCH_OP_SH_BO) | ((cr + 3) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void bno(CrID cr, int imm)
    {
		/* bc BO, BI, imm */
        emitInst((16 << BRANCH_OP_SH_OP1) | (4 << BRANCH_OP_SH_BO) | ((cr + 3) << BRANCH_OP_SH_BI) | ((imm >> 2) << BRANCH_OP_SH_BD));
    }

    void appendJump()
    {
        m_jumps.append(m_buffer.label());
    }

    void fabs(FPRegisterID ft, FPRegisterID fs)
    {
        emitInst((63 << FLOAT_OP_SH_OP1) | (ft << FLOAT_OP_SH_RD) | (fs << FLOAT_OP_SH_RB) | (264 << FLOAT_OP_SH_OP2));
    }
    
    void fneg(FPRegisterID ft, FPRegisterID fs)
    {
        emitInst((63 << FLOAT_OP_SH_OP1) | (ft << FLOAT_OP_SH_RD) | (fs << FLOAT_OP_SH_RB) | ( 40 << FLOAT_OP_SH_OP2));
    }
    
    void fadd(FPRegisterID fd, FPRegisterID fs, FPRegisterID ft)
    {
        emitInst((63 << FLOAT_OP_SH_OP1) | (21 << FLOAT_OP_SH_OP2) | (fd << FLOAT_OP_SH_RD) | (fs << FLOAT_OP_SH_RA) | (ft << FLOAT_OP_SH_RB));
    }

    void fsub(FPRegisterID fd, FPRegisterID fs, FPRegisterID ft)
    {
        emitInst((63 << FLOAT_OP_SH_OP1) | (20 << FLOAT_OP_SH_OP2) | (fd << FLOAT_OP_SH_RD) | (fs << FLOAT_OP_SH_RA) | (ft << FLOAT_OP_SH_RB));
    }

    void fmul(FPRegisterID fd, FPRegisterID fs, FPRegisterID ft)
    {
        emitInst((63 << FLOAT_OP_SH_OP1) | (25 << FLOAT_OP_SH_OP2) | (fd << FLOAT_OP_SH_RD) | (fs << FLOAT_OP_SH_RA) | (ft << FLOAT_OP_SH_RC));
    }

    void fdiv(FPRegisterID fd, FPRegisterID fs, FPRegisterID ft)
    {
        emitInst((63 << FLOAT_OP_SH_OP1) | (18 << FLOAT_OP_SH_OP2) | (fd << FLOAT_OP_SH_RD) | (fs << FLOAT_OP_SH_RA) | (ft << FLOAT_OP_SH_RB));
    }

	void fctiw(FPRegisterID fd, FPRegisterID ft, bool flags = false)
	{
        emitInst((63 << FLOAT_OP_SH_OP1) | (14 << FLOAT_OP_SH_OP2) | (fd << FLOAT_OP_SH_RD) | (ft << FLOAT_OP_SH_RB) | flags);
	}

	void fctiwz(FPRegisterID fd, FPRegisterID ft, bool flags = false)
	{
        emitInst((63 << FLOAT_OP_SH_OP1) | (15 << FLOAT_OP_SH_OP2) | (fd << FLOAT_OP_SH_RD) | (ft << FLOAT_OP_SH_RB) | flags);
	}

    void lfs(FPRegisterID ft, RegisterID rs, int offset)
    {
        emitInst((48 << ARITH_OP_SH_OP1) | (ft << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void lfd(FPRegisterID ft, RegisterID rs, int offset)
    {
        emitInst((50 << ARITH_OP_SH_OP1) | (ft << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void stfs(FPRegisterID ft, RegisterID rs, int offset)
    {
        emitInst((52 << ARITH_OP_SH_OP1) | (ft << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }

    void stfd(FPRegisterID ft, RegisterID rs, int offset)
    {
        emitInst((54 << ARITH_OP_SH_OP1) | (ft << ARITH_OP_SH_RD) | (rs << ARITH_OP_SH_RA) | (offset & 0xFFFF));
    }
	
    void stfiwx(FPRegisterID ft, RegisterID ra, RegisterID rb)
    {
        emitInst((31 << FLOAT_OP_SH_OP1) | (ft << FLOAT_OP_SH_RD) | (ra << FLOAT_OP_SH_RA) | (rb << FLOAT_OP_SH_RB) | (983 << ARITH_OP_SH_OP2));
    }
	
    void fmr(FPRegisterID ft, FPRegisterID fs)
    {
        emitInst((63 << ARITH_OP_SH_OP1) | (ft << ARITH_OP_SH_RD) | (fs << ARITH_OP_SH_RB) | (72 << ARITH_OP_SH_OP2));
    }
	
    void frsp(FPRegisterID ft, FPRegisterID fs)
    {
        // NOTE-- HID2[PSE] should be zero!
        emitInst((63 << ARITH_OP_SH_OP1) | (ft << ARITH_OP_SH_RD) | (fs << ARITH_OP_SH_RB) | (12 << ARITH_OP_SH_OP2));
    }
	
	void fcmpo(CrID cr, FPRegisterID fs, FPRegisterID ft)
	{
		emitInst((63 << CMP_OP_SH_OP1) | (32 << CMP_OP_SH_OP2) | (cr << CMP_OP_SH_CR) | (fs << CMP_OP_SH_RA) | (ft << CMP_OP_SH_RB));
	}

	void fcmpu(CrID cr, FPRegisterID fs, FPRegisterID ft)
	{
		emitInst((63 << CMP_OP_SH_OP1) | (cr << CMP_OP_SH_CR) | (fs << CMP_OP_SH_RA) | (ft << CMP_OP_SH_RB));
	}

    // General helpers

    AssemblerLabel label()
    {
        return m_buffer.label();
    }

    AssemblerLabel align(int alignment)
    {
        while (!m_buffer.isAligned(alignment))
            bkpt();

        return label();
    }

    static void* getRelocatedAddress(void* code, AssemblerLabel label)
    {
        return reinterpret_cast<void*>(reinterpret_cast<char*>(code) + label.m_offset);
    }

    static int getDifferenceBetweenLabels(AssemblerLabel a, AssemblerLabel b)
    {
        return b.m_offset - a.m_offset;
    }

    // Assembler admin methods:

    void* code() const
    {
        return m_buffer.data();
    }

    size_t codeSize() const
    {
        return m_buffer.codeSize();
    }

    PassRefPtr<ExecutableMemoryHandle> executableCopy(JSGlobalData& globalData, void* ownerUID, JITCompilationEffort effort)
    {
        RefPtr<ExecutableMemoryHandle> result = m_buffer.executableCopy(globalData, ownerUID, effort);
        if (!result)
            return 0;

        relocateJumps(m_buffer.data(), result->start());
        return result.release();
    }

#ifndef NDEBUG
    unsigned debugOffset() { return m_buffer.debugOffset(); }
#endif

    static unsigned getCallReturnOffset(AssemblerLabel call)
    {
        // The return address is after a call and a delay slot instruction
        return call.m_offset;
    }

    // Linking & patching:
    //
    // 'link' and 'patch' methods are for use on unprotected code - such as the code
    // within the AssemblerBuffer, and code being patched by the patch buffer.  Once
    // code has been finalized it is (platform support permitting) within a non-
    // writable region of memory; to modify the code in an execute-only execuable
    // pool the 'repatch' and 'relink' methods should be used.

    void linkJump(AssemblerLabel from, AssemblerLabel to)
    {
        ASSERT(to.isSet());
        ASSERT(from.isSet());

        PPCWord* insn = reinterpret_cast<PPCWord*>(reinterpret_cast<intptr_t>(m_buffer.data()) + from.m_offset);
        PPCWord* toPos = reinterpret_cast<PPCWord*>(reinterpret_cast<intptr_t>(m_buffer.data()) + to.m_offset);

		// Check if we are pointing to just BEHIND a branch location. We look for the NOPs in this block as a pattern...
		ASSERT((*(insn - 1) == PPC_NOP) && (*(insn - 2) == PPC_NOP) && (*(insn - 3) == PPC_NOP) && (*(insn - 4) == PPC_NOP));
		insn = insn - 5;
        linkWithOffset(insn, toPos);
    }

    static void linkJump(void* code, AssemblerLabel from, void* to)
    {
        ASSERT(from.isSet());
        PPCWord* insn = reinterpret_cast<PPCWord*>(reinterpret_cast<intptr_t>(code) + from.m_offset);

		// Check if we are pointing to just BEHIND a branch location. We look for the NOPs in this block as a pattern...
		ASSERT((*(insn - 1) == PPC_NOP) && (*(insn - 2) == PPC_NOP) && (*(insn - 3) == PPC_NOP) && (*(insn - 4) == PPC_NOP));
		insn = insn - 5;
		linkWithOffset(insn, to);
    }

    static void linkCall(void* code, AssemblerLabel from, void* to)
    {
        PPCWord* insn = reinterpret_cast<PPCWord*>(reinterpret_cast<intptr_t>(code) + from.m_offset);
        linkCallInternal(insn, to);
    }

    static void linkPointer(void* code, AssemblerLabel from, void* to)
    {
        PPCWord* insn = reinterpret_cast<PPCWord*>(reinterpret_cast<intptr_t>(code) + from.m_offset);
		ASSERT((*insn & 0xfc000000) == 0x3c000000); // lis
        *insn = (*insn & 0xffff0000) | ((reinterpret_cast<intptr_t>(to) >> 16) & 0xffff);
        insn++;
		ASSERT((*insn & 0xfc000000) == 0x60000000); // ori
        *insn = (*insn & 0xffff0000) | (reinterpret_cast<intptr_t>(to) & 0xffff);
    }

    static void relinkJump(void* from, void* to)
    {
        PPCWord* insn = reinterpret_cast<PPCWord*>(from);

		insn = insn - 5;
        int flushSize = linkWithOffset(insn, to);

        ExecutableAllocator::cacheFlush(insn, flushSize);
    }

    static void relinkCall(void* from, void* to)
    {
        void* start;
        int size = linkCallInternal(from, to);
        if (size == sizeof(PPCWord))
            start = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(from) - 1 * sizeof(PPCWord));
        else
            start = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(from) - 4 * sizeof(PPCWord));

        ExecutableAllocator::cacheFlush(start, size);
    }

    static void repatchInt32(void* from, int32_t to)
    {
        PPCWord* insn = reinterpret_cast<PPCWord*>(from);
		ASSERT((*insn & 0xfc000000) == 0x3c000000); // lis
        *insn = (*insn & 0xffff0000) | ((to >> 16) & 0xffff);
        insn++;
		ASSERT((*insn & 0xfc000000) == 0x60000000); // ori
        *insn = (*insn & 0xffff0000) | (to & 0xffff);
        insn--;
        ExecutableAllocator::cacheFlush(insn, 2 * sizeof(PPCWord));
    }

    static int32_t readInt32(void* from)
    {
        PPCWord* insn = reinterpret_cast<PPCWord*>(from);
		ASSERT((*insn & 0xfc000000) == 0x3c000000); // lis
        int32_t result = (*insn & 0x0000ffff) << 16;
        insn++;
		ASSERT((*insn & 0xfc000000) == 0x60000000); // ori
        result |= *insn & 0x0000ffff;
        return result;
    }
    
    static void repatchCompact(void* where, int32_t value)
    {
        repatchInt32(where, value);
    }

    static void repatchPointer(void* from, void* to)
    {
        repatchInt32(from, reinterpret_cast<int32_t>(to));
    }

    static void* readPointer(void* from)
    {
        return reinterpret_cast<void*>(readInt32(from));
    }

    static void* readCallTarget(void* from)
    {
        PPCWord* insn = reinterpret_cast<PPCWord*>(from);
        insn -= 4;
		ASSERT((*insn & 0xfc000000) == 0x3c000000); // lis
        int32_t result = (*insn & 0x0000ffff) << 16;
        insn++;
		ASSERT((*insn & 0xfc000000) == 0x60000000); // ori
        result |= *insn & 0x0000ffff;
        return reinterpret_cast<void*>(result);
    }

private:
    /* Update each jump in the buffer of newBase.  */
    void relocateJumps(void* oldBase, void* newBase)
    {
        // Check each jump
        for (Jumps::Iterator iter = m_jumps.begin(); iter != m_jumps.end(); ++iter) {
            int pos = iter->m_offset;
            PPCWord* insn = reinterpret_cast<PPCWord*>(reinterpret_cast<intptr_t>(newBase) + pos);
			// Move over the initial conditional branch. It'll stay untouched...
            insn = insn + 1;
			
            // Need to make sure we have 5 valid instructions after pos
//#TF: was this correct for MIPS? For PPC I now think it is... (should have been 6 for MIPS?)
            if ((unsigned int)pos >= m_buffer.codeSize() - 5 * sizeof(PPCWord))
                continue;
				
			// Long jump sequence? (starting with 'lis')
			else if ((*insn & 0xfc000000) == 0x3c000000)
			{
				// Ok, this is the only form we need to relocate for. Everything else is PC-relative...
				intptr_t target	 = *(insn + 0) << 16;
				target			|= *(insn + 1) & 0xffff;
				
				target += (intptr_t)newBase - (intptr_t)oldBase;
				
				*(insn + 0) = (*(insn + 0) & 0xffff0000) | (target >> 16);
				*(insn + 1) = (*(insn + 1) & 0xffff0000) | (target & 0xffff);
			}
        }
    }

	/*
		This replaces the originally generated code for branches with alternative code if the distance to branch is too far.
		(Note that a relocation needs to be done in any case as the original distance is set to zero - and hence invalid/useless)
	*/
    static int linkWithOffset(PPCWord* insn, void* to)
    {
		/*
			Convert the sequence:
			  beq $2, $3, target		// could be 'b' target, too!
			  nop
			  nop
			  nop
			  nop
			1:

			to the new sequence if possible:
			  bne $2, $3, 1f
			  b    target
			  nop
			  nop
			  nop
			1:

			OR to the new sequence:
			  bne $2, $3, 1f
			  lis $0, target >> 16
			  ori $0, $0, target & 0xffff
			  mtctr $0
			  bctr
			1:

			Note: the case of a short "bt" instruction will result in it being "nopped" away, rather then inverted as there is no inverted form...

		*/
        ASSERT((*insn & 0xfc000000) == 0x40000000 || // bc - we expect this type of instruction at the begin of all branch-blocks passing through here! (or NOP - see special case below)
				*insn == PPC_NOP);

        intptr_t diff = reinterpret_cast<intptr_t>(to) - reinterpret_cast<intptr_t>(insn);

		// Outside the limits for a simple branch OR already extended?
		if (diff < -32768 || diff > 32767 || (*(insn + 1) != PPC_NOP))
		{
			// If this was still a single branch, we need to invert the logic & adjust the target...
			if (*(insn + 1) == PPC_NOP)
			{
				// Invert the logic & replace the branch target with the first instruction behind the 5-word block we're in...
				if ((*insn & 0xffff0000) == ((16 << BRANCH_OP_SH_OP1) | (20 << BRANCH_OP_SH_BO) | (0 << BRANCH_OP_SH_BI)))
					*insn = PPC_NOP;												// special case if we have to "negate" a "bt" instruction. There is no "bf". Well, a "nop" will do just fine...
				else
					*insn = ((*insn & 0xffff0003) ^ (8 << BRANCH_OP_SH_BO)) | 20;	// normal case: we just invert the state we branch on
			}
			
			// Need to adjust the jump difference. The branch now would sit one instruction further up in memory
			diff -= sizeof(PPCWord);

			// Outside the limits for an unconditional branch OR already extended to one?
			if (diff < -0x2000000 || diff > 0x1ffffff || (*(insn + 2) != PPC_NOP))
			{
				// We must use jump-over-ctr
				intptr_t newTargetAddress = reinterpret_cast<intptr_t>(to);
				*(insn + 1) = (15 << ARITH_OP_SH_OP1) | (0 << ARITH_OP_SH_RD) | (0 << ARITH_OP_SH_RA) | (newTargetAddress >> 16);					// lis r0, target>>16
				*(insn + 2) = (24 << LOGIC_OP_SH_OP1) | (0 << LOGIC_OP_SH_RA) | (0 << LOGIC_OP_SH_RS) | (newTargetAddress & 0xffff);				// ori r0, r0, target & 0xFFFF
				*(insn + 3) = (31 << MOVSPR_OP_SH_OP1) | (0 << MOVSPR_OP_SH_RD) | (SPR_CTR << MOVSPR_OP_SH_SPR) | (467 << MOVSPR_OP_SH_OP2);		// mtctr r0
				*(insn + 4) = (19 << BRANCH_OP_SH_OP1) | (528 << BRANCH_OP_SH_OP2) | (20 << BRANCH_OP_SH_BO);										// bctr
				return 5 * sizeof(PPCWord);
			}
				
			// We can use an unconditional branch...
			*(insn + 1) = 0x48000000 | (diff & 0x03fffffc);
			return 2 * sizeof(PPCWord);
		}

		// A simple branch is enough. Update the delta...
        *insn = (*insn & 0xffff0003) | (diff & 0xfffc);
        return sizeof(PPCWord);
    }

    static int linkCallInternal(void* from, void* to)
    {
		/*
			This setup:

			nop
			nop
			nop
			bl		target
			
			Will be relocated. If the offset is too large, this code is substituted:
			
			lis		r0, target >> 16
			ori		r0, r0, target & 0xffff
			mtctr	r0
			bctrl
		*/
        PPCWord* insn = reinterpret_cast<PPCWord*>(from);
        insn = insn - 4;
		
		// Still the simple form?
		if ((*(insn + 3) & 0xfc000000) == 0x48000000)
		{
			// Yes. Do things still fit?
			intptr_t diff = reinterpret_cast<intptr_t>(to) - reinterpret_cast<intptr_t>(insn + 3);
			if (diff >= -0x2000000 && diff <= 0x1ffffff)
			{
				// Yes. Replace offset...
				*(insn + 3) = (*(insn + 3) & 0xfc000003) | (diff & 0x03fffffc);
				return sizeof(PPCWord);
			}

			// We need the long form
			intptr_t newTargetAddress = reinterpret_cast<intptr_t>(to);
			*(insn + 0) = (15 << ARITH_OP_SH_OP1) | (0 << ARITH_OP_SH_RD) | (PPCRegisters::zero << ARITH_OP_SH_RA) | (newTargetAddress >> 16);	// lis r0, target>>16
			*(insn + 1) = (24 << LOGIC_OP_SH_OP1) | (0 << LOGIC_OP_SH_RA) | (0 << LOGIC_OP_SH_RS) | (newTargetAddress & 0xffff);				// ori r0, r0, target & 0xFFFF
			*(insn + 2) = (31 << MOVSPR_OP_SH_OP1) | (0 << MOVSPR_OP_SH_RD) | (SPR_LR << MOVSPR_OP_SH_SPR) | (467 << MOVSPR_OP_SH_OP2);			// mtlr r0
			*(insn + 3) = (19 << BRANCH_OP_SH_OP1) | (16 << BRANCH_OP_SH_OP2) | (20 << BRANCH_OP_SH_BO) | (1 << BRANCH_OP_SH_LK);				// blrl
			return 4*sizeof(PPCWord);
		}
		
		// Long form already exists. Update it...
		intptr_t newTargetAddress = reinterpret_cast<intptr_t>(to);
		*(insn + 0) = (15 << ARITH_OP_SH_OP1) | (0 << ARITH_OP_SH_RD) | (PPCRegisters::zero << ARITH_OP_SH_RA) | (newTargetAddress >> 16);	// lis r0, target>>16
		*(insn + 1) = (24 << LOGIC_OP_SH_OP1) | (0 << LOGIC_OP_SH_RA) | (0 << LOGIC_OP_SH_RS) | (newTargetAddress & 0xffff);				// ori r0, r0, target & 0xFFFF
		return 4*sizeof(PPCWord);
    }

    AssemblerBuffer m_buffer;
    Jumps m_jumps;
};

} // namespace JSC

#endif // ENABLE(ASSEMBLER) && CPU(PPC)

#endif // PPCAssembler_h
