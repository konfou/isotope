/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMacros.h"
#include "GFlipTable.h"
#include "GMipsEmulator.h"

// Some Macro Definitions
#define REG_ZERO nRegisters[0]	// Zero Register
								// Reg 1 ($at) is reserved for the assembler
#define REG_V0 nRegisters[2]	// Used for Return Value
#define REG_V1 nRegisters[3]
#define REG_A0 nRegisters[4]
#define REG_A1 nRegisters[5]
#define REG_A2 nRegisters[6]
#define REG_A3 nRegisters[7]
#define REG_T0 nRegisters[8]
#define REG_T1 nRegisters[9]
#define REG_T2 nRegisters[10]
#define REG_T3 nRegisters[11]
#define REG_T4 nRegisters[12]
#define REG_T5 nRegisters[13]
#define REG_T6 nRegisters[14]
#define REG_T7 nRegisters[15]
#define REG_S0 nRegisters[16]
#define REG_S1 nRegisters[17]
#define REG_S2 nRegisters[18]
#define REG_S3 nRegisters[19]
#define REG_S4 nRegisters[20]
#define REG_S5 nRegisters[21]
#define REG_S6 nRegisters[22]
#define REG_S7 nRegisters[23]
#define REG_T8 nRegisters[24]
#define REG_T9 nRegisters[25]
								// Reg 26 ($k0) Reserved for OS
								// Reg 27 ($k1) Reserved for OS
#define REG_GP nRegisters[28]
#define REG_SP nRegisters[29]	// Stack Pointer
#define REG_FP nRegisters[30]
#define REG_RA nRegisters[31]	// Return Address
#define REG_PC nRegisters[32]	// Program Counter
#define REG_NPC nRegisters[33]	// Next Program Counter
#define REG_HI nRegisters[34]	// Hi register for div
#define REG_LO nRegisters[35]	// Lo register for div
#define REG_RD nRegisters[36]	// *** I made this one up ***

// ** Global pointer to the emulator object
GMipsEmulator* gpThis;

// Macro Definitions
#define REG gpThis->nRegisters
#define ADVANCE_PC()  gpThis->REG_PC = gpThis->REG_NPC; gpThis->REG_NPC += 4
#define JUMP_PC(ofs)  gpThis->REG_PC = gpThis->REG_NPC; gpThis->REG_NPC += ofs

// *******************
// R-Type Instructions Where Op = 0
// *******************

// Funct = 0x0
void mips_sll(char rs, char rt, char rd, char shamt)
{
	if(rs || rt || rd || shamt)
	{
		REG[rd] = REG[rt] << shamt;
	}
	ADVANCE_PC();
}

// Funct = 0x2
void mips_srl(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rt] >> shamt;
	ADVANCE_PC();
}

// Funct = 0x3
void mips_sra(char rs, char rt, char rd, char shamt)
{
	REG[rd] = (signed)REG[rt] >> shamt;
	ADVANCE_PC();
}

// Funct = 0x4
void mips_sllv(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rt] << REG[rs];
	ADVANCE_PC();
}

// Funct = 0x5
void mips_abs(char rs, char rt, char rd, char shamt)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Funct = 0x6
void mips_srlv(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rt] >> REG[rs];
	ADVANCE_PC();
}

// Funct = 0x7
void mips_srav(char rs, char rt, char rd, char shamt)
{

	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Funct = 0x8
void mips_jr(char rs, char rt, char rd, char shamt)
{
	gpThis->REG_PC = gpThis->REG_NPC;
	gpThis->REG_NPC = gpThis->m_nCodeSeg + (REG[rs] & 0xffff);
}

// Funct = 0x9
void mips_jalr(char rs, char rt, char rd, char shamt)
{
	gpThis->REG_RA = gpThis->REG_NPC + 4;
	gpThis->REG_PC = gpThis->REG_NPC;
	gpThis->REG_NPC = gpThis->m_nCodeSeg + REG[rs];
}

// Funct = 0xc
void mips_syscall(char rs, char rt, char rd, char shamt)
{
	ADVANCE_PC();
	gpThis->SysCall(gpThis->REG_V0);
}

// Funct = 0x10
void mips_mfhi(char rs, char rt, char rd, char shamt)
{
	REG[rd] = gpThis->REG_HI;
	ADVANCE_PC();
}

// Funct = 0x11
void mips_mthi(char rs, char rt, char rd, char shamt)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Funct = 0x12
void mips_mflo(char rs, char rt, char rd, char shamt)
{
	REG[rd] = gpThis->REG_LO;
	ADVANCE_PC();
}

// Funct = 0x13
void mips_mtlo(char rs, char rt, char rd, char shamt)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Funct = 0x18
void mips_mult(char rs, char rt, char rd, char shamt)
{ // **** This is a rig--it should put sumpin in REG_HI too
	gpThis->REG_LO = REG[rs] * REG[rt];
	ADVANCE_PC();
}

// Funct = 0x19
void mips_multu(char rs, char rt, char rd, char shamt)
{ // **** This is a rig--it should put sumpin in REG_HI too
	gpThis->REG_LO = REG[rs] * REG[rt];
	ADVANCE_PC();
}

// Funct = 0x1a
void mips_div(char rs, char rt, char rd, char shamt)
{
	gpThis->REG_LO = REG[rs] / REG[rt];
	gpThis->REG_HI = REG[rs] % REG[rt];
	ADVANCE_PC();
}

// Funct = 0x1b
void mips_divu(char rs, char rt, char rd, char shamt)
{
	gpThis->REG_LO = REG[rs] / REG[rt];
	gpThis->REG_HI = REG[rs] % REG[rt];
	ADVANCE_PC();
}

// Funct = 0x20
void mips_add(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] + REG[rt];
	ADVANCE_PC();
}

// Funct = 0x21
void mips_addu(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] + REG[rt];
	ADVANCE_PC();

}

// Funct = 0x22
void mips_sub(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] - REG[rt];
	ADVANCE_PC();
}

// Funct = 0x23
void mips_subu(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] - REG[rt];
	ADVANCE_PC();
}

// Funct = 0x24
void mips_and(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] & REG[rt];
	ADVANCE_PC();
}

// Funct = 0x25
void mips_or(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] | REG[rt];
	ADVANCE_PC();
}

// Funct = 0x26
void mips_xor(char rs, char rt, char rd, char shamt)
{
	REG[rd] = REG[rs] ^ REG[rt];
	ADVANCE_PC();
}

// Funct = 0x27
void mips_nor(char rs, char rt, char rd, char shamt)
{
	REG[rd] = !(REG[rs] | REG[rt]);
	ADVANCE_PC();
}

// Funct = 0x2a
void mips_slt(char rs, char rt, char rd, char shamt)
{
	if(REG[rs] < REG[rt])
		REG[rd] = 1;
	else
		REG[rd] = 0;
	ADVANCE_PC();
}

// Funct = 0x2b
void mips_sltu(char rs, char rt, char rd, char shamt)
{
	if(REG[rs] < REG[rt])
		REG[rd] = 1;
	else
		REG[rd] = 0;
	ADVANCE_PC();
}

// *******************
// I-Type Instructions
// *******************

// Op = 0x4
void mips_beq(char rs, char rt, short imm)
{
	if(REG[rs] == REG[rt])
	{
		JUMP_PC(imm << 2);
	}
	else
	{
		ADVANCE_PC();
	}
}

// Op = 0x5
void mips_bne(char rs, char rt, short imm)
{
	if(REG[rs] != REG[rt])
	{
		JUMP_PC(imm << 2);
	}
	else
	{
		ADVANCE_PC();
	}
}

// Op = 0x6
void mips_blez(char rs, char rt, short imm)
{
	if((signed)REG[rs] <= 0)
	{
		JUMP_PC(imm << 2);
	}
	else
	{
		ADVANCE_PC();
	}
}

// Op = 0x7
void mips_bgtz(char rs, char rt, short imm)
{
	if((signed)REG[rs] > 0)
	{
		JUMP_PC(imm << 2);
	}
	else
	{
		ADVANCE_PC();
	}
}

// Op = 0x8
void mips_addi(char rs, char rt, short imm)
{
	REG[rt] = REG[rs] + imm;
	ADVANCE_PC();
}

// Op = 0x9
void mips_addiu(char rs, char rt, short imm)
{
	REG[rt] = REG[rs] + imm;
	ADVANCE_PC();
}

// Op = 0xa
void mips_slti(char rs, char rt, short imm)
{

	if((signed)REG[rs] < imm)
		REG[rt] = 1;
	else
		REG[rt] = 0;
	ADVANCE_PC();
}

// Op = 0xb
void mips_sltiu(char rs, char rt, short imm)
{
	if((signed)REG[rs] < imm)
		REG[rt] = 1;
	else
		REG[rt] = 0;
	ADVANCE_PC();
}

// Op = 0xc
void mips_andi(char rs, char rt, short imm)
{
	REG[rt] = (REG[rs] & (imm & 0xffff));
	ADVANCE_PC();
}

// Op = 0xd
void mips_ori(char rs, char rt, short imm)
{
	REG[rt] = (REG[rs] | (imm & 0xffff));
	ADVANCE_PC();
}

// Op = 0xe
void mips_xori(char rs, char rt, short imm)
{
	REG[rt] = (REG[rs] ^ (imm & 0xffff));
	ADVANCE_PC();
}

// Op = 0xf
void mips_lui(char rs, char rt, short imm)
{
	REG[rt] = imm << 16;
	ADVANCE_PC();
}

// Op = 0x20
void mips_lb(char rs, char rt, short imm)
{
	REG[rt] = gpThis->GetByte(REG[rs] + imm);
	ADVANCE_PC();
}

// Op = 0x21
void mips_lh(char rs, char rt, short imm)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Op = 0x22
void mips_lwl(char rs, char rt, short imm)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

unsigned short FlipShort(unsigned short s)
{
	short s2;
	char* pChar = (char*)&s2;
	pChar[0] = FLIP_8_BITS_TABLE[pChar[1]];
	pChar[1] = FLIP_8_BITS_TABLE[pChar[0]];
	return s2;
}

unsigned long FlipLong(unsigned long l)
{
	long l2;
	char* pChar = (char*)&l2;
	pChar[0] = FLIP_8_BITS_TABLE[pChar[3]];
	pChar[1] = FLIP_8_BITS_TABLE[pChar[2]];
	pChar[2] = FLIP_8_BITS_TABLE[pChar[1]];
	pChar[3] = FLIP_8_BITS_TABLE[pChar[0]];
	return l2;
}

// Op = 0x23
void mips_lw(char rs, char rt, short imm)
{
#ifdef LITTLE_ENDIAN
	REG[rt] = FlipLong(gpThis->GetWord(REG[rs] + imm));
#else
	REG[rt] = gpThis->GetWord(REG[rs] + imm);
#endif
	ADVANCE_PC();
}

// Op = 0x24
void mips_lbu(char rs, char rt, short imm)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Op = 0x25
void mips_lhu(char rs, char rt, short imm)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Op = 0x28
void mips_sb(char rs, char rt, short imm)
{
	gpThis->WriteByte(REG[rs] + imm, REG[rt] && 0xff);
	ADVANCE_PC();
}

// Op = 0x29
void mips_sh(char rs, char rt, short imm)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// Op = 0x2a
void mips_swl(char rs, char rt, short imm)
{ // **** This is a rig--it should only store half of the word
#ifdef LITTLE_ENDIAN
	gpThis->WriteWord(REG[rs] + imm, FlipShort((unsigned short)(REG[rt] & 0xffff)));
#else
	gpThis->WriteWord(REG[rs] + imm, (unsigned short)(REG[rt] & 0xffff));
#endif
	ADVANCE_PC();
}

// Op = 0x2b
void mips_sw(char rs, char rt, short imm)
{
#ifdef LITTLE_ENDIAN
	gpThis->WriteWord(REG[rs] + imm, FlipLong(REG[rt]));
#else
	gpThis->WriteWord(REG[rs] + imm, REG[rt]);
#endif
	ADVANCE_PC();
}

// Op = 0x2e
void mips_swr(char rs, char rt, short imm)
{
	GAssert(false, "Not Implemented Yet\n");
	ADVANCE_PC();
}

// *******************
// Branching Instructions
// *******************

void mips_branch(char rs, char rt, short imm)
{
	switch(rt)
	{
		case 0:		// bltz
			if((signed)REG[rs] < 0)
			{
				JUMP_PC(imm << 2);
			}
			else
			{
				ADVANCE_PC();
			}
			break;
		case 1:		// bgez
			if((signed)REG[rs] >= 0)
			{
				JUMP_PC(imm << 2);
			}
			else
			{
				ADVANCE_PC();
			}
			break;
		case 16:	// bltzal
			if((signed)REG[rs] < 0)
			{
				gpThis->REG_RA = gpThis->REG_NPC + 4;
				JUMP_PC(imm << 2);
			}
			else
			{
				ADVANCE_PC();
			}
			break;
		case 17:	// bgezal
			if((signed)REG[rs] >= 0)
			{
				gpThis->REG_RA = gpThis->REG_NPC + 4;
				JUMP_PC(imm << 2);
			}
			else
			{
				ADVANCE_PC();
			}
			break;
		default:
			GAssert(false, "Not Implemented Yet\n");
	}
}

// *******************
// J-Type Instructions
// *******************

// Op = 0x2
void mips_j(long addr)
{
	gpThis->REG_PC = gpThis->REG_NPC;
	gpThis->REG_NPC = gpThis->m_nCodeSeg + ((gpThis->REG_PC & 0xf0000000) | (addr << 2));
}

// Op = 0x3
void mips_jal(long addr)
{
	gpThis->REG_RA = gpThis->REG_NPC + 4;
	gpThis->REG_PC = gpThis->REG_NPC;
	gpThis->REG_NPC = gpThis->m_nCodeSeg + ((gpThis->REG_PC & 0xf0000000) | (addr << 2));
}

// *******************
// Tables
// *******************

typedef void (*R_TYPE)(char, char, char, char);

static R_TYPE R_TYPE_TABLE[] = 
{
	mips_sll,	// 0x0
	NULL,		// 0x1
	mips_srl,	// 0x2
	mips_sra,	// 0x3
	mips_sllv,
	mips_abs,
	mips_srlv,	// 0x6
	mips_srav,
	mips_jr,
	mips_jalr,	// 0x9
	NULL,		// 0xa
	NULL,
	mips_syscall, // 0xc
	NULL,
	NULL,		// 0xe
	NULL,
	mips_mfhi,	// 0x10
	mips_mthi,
	mips_mflo,
	mips_mtlo,	// 0x13
	NULL,
	NULL,
	NULL,
	NULL,
	mips_mult,	// 0x18
	mips_multu,	// 0x19
	mips_div,	// 0x1a
	mips_divu,
	NULL,
	NULL,
	NULL,
	NULL,		// 0x1f
	mips_add,	// 0x20
	mips_addu,	// 0x21
	mips_sub,
	mips_subu,
	mips_and,
	mips_or,
	mips_xor,
	mips_nor,	// 0x27
	NULL,
	NULL,
	mips_slt,	// 0x2a
	mips_sltu,	// 0x2b
	NULL,
	NULL,
	NULL,
	NULL,		// 0x2f
};

typedef void (*I_TYPE)(char, char, short);

static I_TYPE I_TYPE_TABLE[] = 
{
	NULL,			// 0x0
	NULL,			// 0x1
	NULL,
	NULL,
	mips_beq,		// 0x4
	mips_bne,		// 0x5
	mips_blez,
	mips_bgtz,
	mips_addi,
	mips_addiu,		// 0x9
	mips_slti,		// 0xa
	mips_sltiu,
	mips_andi,
	mips_ori,
	mips_xori,
	mips_lui,		// 0xf
	NULL,			// 0x10
	NULL,			// 0x11
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,			// 0x18
	NULL,			// 0x19
	NULL,			// 0x1a
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,			// 0x1f
	mips_lb,		// 0x20
	mips_lh,
	mips_lwl,		// 0x22
	mips_lw,
	mips_lbu,
	mips_lhu,		// 0x25
	NULL,
	NULL,
	mips_sb,		// 0x28
	mips_sh,
	mips_swl,
	mips_sw,		// 0x2b
	NULL,
	NULL,
	mips_swr,		// 0x2e
	NULL,			// 0x2f
};

// *******************
// GMipsEmulator Class
// *******************

GMipsEmulator::GMipsEmulator(long nCodeSeg, long nDataSeg, long nStackSeg)
{
	m_nCodeSeg = nCodeSeg;
	m_nDataSeg = nDataSeg;
	m_nStackSeg = nStackSeg;
	gpThis = this;
	int n;
	for(n = 0; n < 37; n++)
		REG[n] = 0;
}

GMipsEmulator::~GMipsEmulator()
{

}

unsigned long GMipsEmulator::GetWord(long addr)
{
	GAssert(false, "You're supposed to override GetWord to return a word from your simulated memory\n");
	return 0; // The NoOp instruction
}

void GMipsEmulator::WriteWord(long addr, unsigned long nWord)
{
	GAssert(false, "You're supposed to override WriteWord\n");
}

unsigned char GMipsEmulator::GetByte(long addr)
{
	GAssert(false, "You're supposed to override GetByte to return a byte from your simulated memory\n");
	return 0;
}

void GMipsEmulator::WriteByte(long addr, unsigned char c)
{
	GAssert(false, "You're supposed to override WriteByte\n");
}

void GMipsEmulator::SysCall(int nNum)
{
	GAssert(false, "You're supposed to override SysCall to perform OS function calls\n");
}

void GMipsEmulator::ProcessInstruction(unsigned long ins)
{
	unsigned char bytes[4];
#ifdef LITTLE_ENDIAN
	bytes[0] = FLIP_8_BITS_TABLE[((unsigned char*)&ins)[0]];
	bytes[1] = FLIP_8_BITS_TABLE[((unsigned char*)&ins)[1]];
	bytes[2] = FLIP_8_BITS_TABLE[((unsigned char*)&ins)[2]];
	bytes[3] = FLIP_8_BITS_TABLE[((unsigned char*)&ins)[3]];
#else
	*((unsigned long*)bytes) = ins;
#endif

	// Calculate op
#ifdef LITTLE_ENDIAN
	unsigned char op = FLIP_6_BITS_TABLE[bytes[0] & 63]; // op is the first 6 bits
#else
	unsigned char op = bytes[0] & 63; // op is the first 6 bits
#endif
		
	// Do one of the 5 types of instructions (based on op)
	switch(op)
	{
		case 0:		// R-Type Instruction
		{
#ifdef LITTLE_ENDIAN
			unsigned char rs = FLIP_5_BITS_TABLE[((*((unsigned short*)bytes)) >> 6) & 31];
			unsigned char rt = FLIP_5_BITS_TABLE[(bytes[1] >> 3) & 31];
			unsigned char rd = FLIP_5_BITS_TABLE[bytes[2] & 31];
			unsigned char shamt = FLIP_5_BITS_TABLE[((*((unsigned short*)(&bytes[2]))) >> 5) & 31];
			unsigned char func = FLIP_6_BITS_TABLE[(bytes[3] >> 2) & 63];
#else
			unsigned char rs = (*((unsigned short*)bytes) >> 6) & 31;
			unsigned char rt = (bytes[1] >> 3) & 31;
			unsigned char rd = bytes[2] & 31;
			unsigned char shamt = ((*((unsigned short*)(&bytes[2]))) >> 5) & 31;
			unsigned char func = (bytes[3] >> 2) & 63;
#endif
#ifdef WIN32
			void (*pFunc)(char rs, char rt, char rd, char shamt) = (void (__cdecl *)(char,char,char,char))R_TYPE_TABLE[func];
#else
			void (*pFunc)(char rs, char rt, char rd, char shamt) = R_TYPE_TABLE[func];
#endif
			pFunc(rs, rt, rd, shamt);
			break;
		}
		case 1:		// Branch Instruction (I-Type)
		{
#ifdef LITTLE_ENDIAN
			unsigned char rs = FLIP_5_BITS_TABLE[((*((unsigned short*)bytes)) >> 6) & 31];
			unsigned char rt = FLIP_5_BITS_TABLE[(bytes[1] >> 3) & 31];
			short imm;
			((unsigned char*)&imm)[0] = ((unsigned char*)&ins)[3];
			((unsigned char*)&imm)[1] = ((unsigned char*)&ins)[2];
#else
			unsigned char rs = (*((unsigned short*)bytes) >> 6) & 31;
			unsigned char rt = (bytes[1] >> 3) & 31;
			short imm = *((short*)&bytes[2]);
#endif
			mips_branch(rs, rt, imm);
			break;
		}
		case 2:		// j-Instruction (J-Type)
		{
#ifdef LITTLE_ENDIAN
			long addr;
			((unsigned char*)&addr)[0] = ((unsigned char*)&ins)[3];
			((unsigned char*)&addr)[1] = ((unsigned char*)&ins)[2];
			((unsigned char*)&addr)[2] = ((unsigned char*)&ins)[1];
			unsigned char cTmp = FLIP_2_BITS_TABLE[bytes[0] >> 6];
			((unsigned char*)&addr)[3] = cTmp;
#else
			long addr = ins >> 6;
#endif
			mips_j(addr & 0xffff);
			break;
		}
		case 3:		// jal-Instruction (J-Type)
		{
#ifdef LITTLE_ENDIAN
			long addr;
			((unsigned char*)&addr)[0] = ((unsigned char*)&ins)[3];
			((unsigned char*)&addr)[1] = ((unsigned char*)&ins)[2];
			((unsigned char*)&addr)[2] = ((unsigned char*)&ins)[1];
			unsigned char cTmp = FLIP_2_BITS_TABLE[bytes[0] >> 6];
			((unsigned char*)&addr)[3] = cTmp;
#else
			long addr = ins >> 6;
#endif
			mips_jal(addr & 0xffff);
			break;
		}
		default:	// I-Type Instruction
		{
#ifdef LITTLE_ENDIAN
			unsigned char rs = FLIP_5_BITS_TABLE[((*((unsigned short*)bytes)) >> 6) & 31];
			unsigned char rt = FLIP_5_BITS_TABLE[(bytes[1] >> 3) & 31];
			short imm;
			((unsigned char*)&imm)[0] = ((unsigned char*)&ins)[3];
			((unsigned char*)&imm)[1] = ((unsigned char*)&ins)[2];
#else
			unsigned char rs = (*((unsigned short*)bytes) >> 6) & 31;
			unsigned char rt = (bytes[1] >> 3) & 31;
			short imm = *((short*)&bytes[2]);
#endif
#ifdef WIN32
			void (*pFunc)(char rs, char rt, short imm) = (void (__cdecl *)(char,char,short))I_TYPE_TABLE[op];
#else
			void (*pFunc)(char rs, char rt, short imm) = I_TYPE_TABLE[op];
#endif
			pFunc(rs, rt, imm);
		}
	}
}

void GMipsEmulator::Execute(struct sMipsHeader* s)
{
	if(s->DeadBeef != 0xDEADBEEF)
	{
		GAssert(false, "Error, invalid mips header\n");
		return;
	}
	int n;
	for(n = 1; n < 32; n++)
		nRegisters[n] = s->nInitRegs[n];
	REG_NPC = m_nCodeSeg + (s->nEntryPoint & 0xffff);
	bKeepGoin = true;

	REG_NPC = s->nEntryPoint;
	ADVANCE_PC();

	int nCounter = 0;
	while(bKeepGoin)
	{
		nCounter++;
			
		if(REG_PC >= (unsigned long)m_nDataSeg)
		{
			GAssert(false, "Error: PC entered Data Seg!\n");
			break;
		}
		REG_ZERO = 0; // Set the $zero register to 0
		ProcessInstruction(GetWord(REG_PC));
	}
}
