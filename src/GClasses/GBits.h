#ifndef __GBITS_H__
#define __GBITS_H__

#include <stdlib.h>

#define BITS_PER_BYTE 8
#define BITS_PER_UINT (BITS_PER_BYTE * sizeof(unsigned int))

class GBits
{
public:
	// Convert a number to its Gray code encoding
	static inline unsigned int BinaryToGrayCode(unsigned int nBinary)
	{
		return nBinary ^ (nBinary >> 1);
	}

	// Convert a number in Gray code encoding to a value
	static inline unsigned int GrayCodeToBinary(unsigned int nGrayCode)
	{
		unsigned int nMask = nGrayCode >> 1;
		while(nMask > 0)
		{
			nGrayCode ^= nMask;
			nMask >>= 1;
		}
		return nGrayCode;
	}

	// Switch the endian of an unsigned integer
	static inline unsigned int ReverseEndian(unsigned int in)
	{
		unsigned int out;
		((unsigned char*)&out)[0] = ((unsigned char*)&in)[3];
		((unsigned char*)&out)[1] = ((unsigned char*)&in)[2];
		((unsigned char*)&out)[2] = ((unsigned char*)&in)[1];
		((unsigned char*)&out)[3] = ((unsigned char*)&in)[0];
		return out;
	}

	// Returns true if a number is a power of two
	static inline bool IsPowerOfTwo(unsigned int n)
	{
		return ((n & (n - 1)) == 0);
	}

	// Returns a random unsigned integer between 0 and 0xffffffff inclusively
	static inline unsigned int GetRandomUint()
	{
		return
			(((unsigned int)rand() % 0xfff) << 20) |
			(((unsigned int)rand() % 0xfff) << 8) |
			((unsigned int)rand() % 0xff);
	}

	// Returns a random double between 0 and 1
	static inline double GetRandomDouble()
	{
		return (double)rand() / RAND_MAX;
	}
};

#endif // __GBITS_H__
