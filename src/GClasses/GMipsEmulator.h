#ifndef __GMIPSEMULATOR_H__
#define __GMIPSEMULATOR_H__

class GHashTable;

// This is used by GMipsEmulator
struct sMipsHeader
{
	unsigned long DeadBeef;
	unsigned long nEntryPoint;
	unsigned long nInitRegs[32];
	unsigned long nCodeSize;
	unsigned long nDataSize;
	unsigned long nStEntries;
};

// This class emulates a MIPS microprocessor (This could be
// the backbone for a Nintendo or Sony Playstation emulator,
// or any other system that uses MIPS.)  The floating-point
// instructions haven't been implemented yet, but you can add
// it if you want to.
class GMipsEmulator
{
public:
	long m_nCodeSeg;
	long m_nDataSeg;
	long m_nStackSeg;
	unsigned long nRegisters[37];	// The registers
	bool bKeepGoin;

	GMipsEmulator(long nCodeSeg, long nDataSeg, long nStackSeg);
	virtual ~GMipsEmulator();

	void ProcessInstruction(unsigned long ins);
	void Execute(struct sMipsHeader* s);

	virtual void SysCall(int nNum);
//protected:
	// You're supposed to override each of these methods
	virtual unsigned long GetWord(long addr);
	virtual void WriteWord(long addr, unsigned long nWord);
	virtual unsigned char GetByte(long addr);
	virtual void WriteByte(long addr, unsigned char c);
};

// This simulates memory (in memory).  It supports addresses up to
// 1 Gb.  (This would be useful with GMipsEmulator to simulate a
// MIPS machine.)
class GVirtualMem
{
protected:
	GHashTable* pHash;

public:
	GVirtualMem();
	virtual ~GVirtualMem();

	void WriteByte(long addr, unsigned char b);
	unsigned char ReadByte(long addr);
	void WriteWord(long addr, unsigned long l);
	unsigned long ReadWord(long addr);
};


#endif // __GMIPSEMULATOR_H__
