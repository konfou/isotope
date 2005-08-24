/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GDAFTLEARNER_H__
#define __GDAFTLEARNER_H__

class GDaftNode;
class GImage;

// Uses the Divide And Fit Technique for learning
class GDaftLearner
{
protected:
	GDaftNode* m_pRed;
	GDaftNode* m_pGreen;
	GDaftNode* m_pBlue;
	GDaftNode* m_pAlpha;
	int m_nControlPoints;
	double m_dAcceptableError;

public:
	GDaftLearner();
	~GDaftLearner();

	void SetControlPoints(int n) { m_nControlPoints = n; }
	void SetAcceptableError(double d) { m_dAcceptableError = d; }
	void Train(GImage* pImage);
//	void Eval(GArffRelation* pRelation, double* pRow);

protected:
	GDaftNode* BuildBranch(GImage* pImage, int* pPixels, int nPixelCount, int nChannel);
};



#endif // __GDAFTLEARNER_H__
