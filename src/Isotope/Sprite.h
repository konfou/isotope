#ifndef __SPRITE_H__
#define __SPRITE_H__

class GPreRendered3DSprite;
class GPreRendered3DScreen;
class GXMLTag;
class GImage;
class G3DObject;

#include "../Source/GClasses/GRayTrace.h"

class Sprite
{
protected:
	Sprite* m_pPrev;
	Sprite* m_pNext;
	int m_nFrame;

public:
	Sprite();
	~Sprite();

	void Unlink();
	void LinkAfter(Sprite* pPrev);
	Sprite* GetPrev() { return m_pPrev; }
	Sprite* GetNext() { return m_pNext; }
	virtual GImage* GetFrame(const GRect* pScreen, GRect* pOutSourceRect, GRect* pOutDestRect) = 0;
};


class Sprite2 : public Sprite
{
public:
	float m_x, m_y;
	int m_nBaseFrame;
	int m_nStep;

protected:
	int m_nImages;
	GImage* m_pImages;

public:
	Sprite2();
	virtual ~Sprite2();

	virtual GImage* GetFrame(const GRect* pScreen, GRect* pOutSourceRect, GRect* pOutDestRect);
};







struct TransformImagePair
{
	Transform trans;
	GImage image;
	double* pDepthMap;

	TransformImagePair()
	{
		pDepthMap = NULL;
	}

	~TransformImagePair()
	{
		delete(pDepthMap);
	}
};


#endif //  __SPRITE_H__
