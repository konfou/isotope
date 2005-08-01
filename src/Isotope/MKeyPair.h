#ifndef __MKEYPAIR_H__
#define __MKEYPAIR_H__

#include "Model.h"

#define ENTROPY_BYTES 16384
#define KEY_BIT_SIZE 128

class GKeyPair;


class MKeyPair : public Model
{
protected:
	unsigned char* m_pEntropy;
	int m_nPos;
	const char* m_szFilename;

public:
	MKeyPair(Controller* pController, const char* szFilename);
	virtual ~MKeyPair();

	virtual void Update(double time)
	{
	}

	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew)
	{
		return true;
	}

	virtual ModelType GetType() { return KeyGenerator; }

	void AddEntropy(int n);
	double GetPercent();
	void SaveKeyPair();
};

#endif // __MKEYPAIR_H__
