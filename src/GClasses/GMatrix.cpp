/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMatrix.h"
#include "GMacros.h"
#include "GVector.h"

// These tell which library to use for computing eigen vectors. You should uncomment only one of them. Octave is known to give good results, but it's GPL'ed. The other two are still rather experimental.
//#define OCTAVE
//#define ARPACK
//#define EIGEN


GMatrix::GMatrix(int nRows, int nColumns)
{
	m_nRows = nRows;
	m_nColumns = nColumns;
	int nSize = nRows * nColumns;
	if(nSize > 0)
	{
		m_pData = new double[nSize];
		int n;
		for(n = 0; n < nSize; n++)
			m_pData[n] = 0;
	}
	else
		m_pData = NULL;
}

GMatrix::~GMatrix()
{
	delete(m_pData);
}

void GMatrix::SetToIdentity()
{
	int nRow, nColumn;
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		for(nColumn = 0; nColumn < m_nColumns; nColumn++)
			Set(nRow, nColumn, 0);
	}
	int nMin = MIN(m_nColumns, m_nRows);
	for(nColumn = 0; nColumn < nMin; nColumn++)
		Set(nColumn, nColumn, 1);
}

void GMatrix::Resize(int nRows, int nColumns)
{
	if(m_nRows == nRows && m_nColumns == nColumns)
		return;
	delete(m_pData);
	m_pData = new double[nRows * nColumns];
	m_nRows = nRows;
	m_nColumns = nColumns;
}

void GMatrix::Copy(const GMatrix* pMatrix)
{
	Resize(pMatrix->m_nRows, pMatrix->m_nColumns);
	memcpy(m_pData, pMatrix->m_pData, m_nRows * m_nColumns * sizeof(double));
}

void GMatrix::Multiply(GMatrix* pA, GMatrix* pB)
{
	if(pA->m_nColumns != pB->m_nRows)
	{
		GAssert(false, "Incompatible sizes--can't multiply");
		return;
	}
	Resize(pA->m_nRows, pB->m_nColumns);
	double dSum;
	int nPos;
	int nRow;
	int nColumn;
	for(nRow = 0; nRow < pA->m_nRows; nRow++)
	{
		for(nColumn = 0; nColumn < pB->m_nColumns; nColumn++)
		{
			dSum = 0;
			for(nPos = 0; nPos < pA->m_nColumns; nPos++)
				dSum += pA->Get(nRow, nPos) * pB->Get(nPos, nColumn);
			Set(nRow, nColumn, dSum);
		}
	}
}

// todo: this algorithm isn't very numerically stable because the
// values will tend to get too large or too small and lose data
double GMatrix::GetDeterminant()
{
	if(m_nRows != m_nColumns)
	{
		GAssert(false, "expected a square matrix");
		return 0;
	}
	double dDet = 0;
	double d;
	int col, i;
	for(col = 0; col < m_nColumns; col++)
	{
		d = 1;
		for(i = 0; i < m_nRows; i++)
			d *= Get(i, (col + i) % m_nColumns);
		dDet += d;
		d = 1;
		for(i = 0; i < m_nRows; i++)
			d *= Get(i, (col - i + m_nColumns) % m_nColumns);
		dDet -= d;
	}
	return dDet;
}

void GMatrix::Print()
{
	int nRow;
	int nColumn;
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		printf("%f", Get(nRow, 0));
		for(nColumn = 1; nColumn < m_nColumns; nColumn++)
			printf("\t%f", Get(nRow, nColumn));
		printf("\n");
	}
}

void GMatrix::PrintCorners(int n)
{
	int nRow, nCol;
	for(nRow = 0; nRow < n && nRow < m_nRows; nRow++)
	{
		printf("%f", Get(nRow, 0));
		for(nCol = 1; nCol < n && nCol < m_nColumns; nCol++)
			printf("\t%f", Get(nRow, nCol));
		if(nCol < m_nColumns - n)
			printf("\t...");
		for(nCol = MAX(nCol, m_nColumns - n); nCol < m_nColumns; nCol++)
			printf("\t%f", Get(nRow, nCol));
		printf("\n");
	}
	if(nRow < m_nRows - n)
		printf("...\n");
	for(nRow = MAX(nRow, m_nRows - n); nRow < m_nRows; nRow++)
	{
		printf("%f", Get(nRow, 0));
		for(nCol = 1; nCol < n && nCol < m_nColumns; nCol++)
			printf("\t%f", Get(nRow, nCol));
		if(nCol < m_nColumns - n)
			printf("\t...");
		for(nCol = MAX(nCol, m_nColumns - n); nCol < m_nColumns; nCol++)
			printf("\t%f", Get(nRow, nCol));
		printf("\n");
	}
}

void GMatrix::Transpose()
{
	double* pNewData = new double[m_nRows * m_nColumns];
	int nRow, nCol;
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		for(nCol = 0; nCol < m_nColumns; nCol++)
			pNewData[nCol * m_nRows + nRow] = m_pData[nRow * m_nColumns + nCol];
	}
	delete(m_pData);
	m_pData = pNewData;
	int nTmp = m_nRows;
	m_nRows = m_nColumns;
	m_nColumns = nTmp;
}

double GMatrix::ComputeTrace()
{
	int nMin = m_nColumns;
	if(m_nRows < nMin)
		nMin = m_nRows;
	double dSum = 0;
	int n;
	for(n = 0; n < nMin; n++)
		dSum += Get(n, n);
	return dSum;
}
/*
void GMatrix::Invert()
{
	GAssert(m_nRows == m_nColumns, "only square matrices supported");
	GAssert(m_nRows > 1, "needs to be at least 2x2");
	for(int i=1; i < actualsize; i++)
		data[i] /= data[0]; // normalize row 0
	for(int i=1; i < actualsize; i++)
	{ 
		for(int j=i; j < actualsize; j++)
		{ // do a column of L
			D sum = 0.0;
			for (int k = 0; k < i; k++)  
				sum += data[j*maxsize+k] * data[k*maxsize+i];
			data[j*maxsize+i] -= sum;
		}
		if(i == actualsize-1)
			continue;
		for(int j=i+1; j < actualsize; j++)
		{  // do a row of U
			D sum = 0.0;
			for (int k = 0; k < i; k++)
				sum += data[i*maxsize+k]*data[k*maxsize+j];
			data[i*maxsize+j] = (data[i*maxsize+j]-sum) / data[i*maxsize+i];
		}
	}
	for( int i = 0; i < actualsize; i++ )  // invert L
	{
		for( int j = i; j < actualsize; j++ )
		{
			D x = 1.0;
			if ( i != j )
			{
				x = 0.0;
				for ( int k = i; k < j; k++ ) 
					x -= data[j*maxsize+k]*data[k*maxsize+i];
			}
			data[j*maxsize+i] = x / data[j*maxsize+j];
		}
	}
	for( int i = 0; i < actualsize; i++ )   // invert U
	{
		for ( int j = i; j < actualsize; j++ )
		{
			if ( i == j ) continue;
			D sum = 0.0;
			for ( int k = i; k < j; k++ )
				sum += data[k*maxsize+j]*( (i==k) ? 1.0 : data[i*maxsize+k] );
			data[i*maxsize+j] = -sum;
		}
	}
	for( int i = 0; i < actualsize; i++ )   // final inversion
	{
		for ( int j = 0; j < actualsize; j++ )
		{
			D sum = 0.0;
			for ( int k = ((i>j)?i:j); k < actualsize; k++ )  
				sum += ((j==k)?1.0:data[j*maxsize+k])*data[k*maxsize+i];
			data[j*maxsize+i] = sum;
		}
	}
}
*/

void GMatrix::Solve(double* pVector)
{
	// Subtract out the non-diagonals
	GAssert(m_nRows == m_nColumns, "Expected a square matrix");
	int nRow, nCol, i;
	double dVal;
	for(nCol = 0; nCol < m_nColumns; nCol++)
	{
		for(nRow = 0; nRow < m_nRows; nRow++)
		{
			if(nRow == nCol)
				continue;
			dVal = m_pData[nRow * m_nColumns + nCol] / m_pData[nCol * m_nColumns + nCol];
			for(i = 0; i < m_nColumns; i++)
				m_pData[nRow * m_nColumns + i] -= dVal * m_pData[nCol * m_nColumns + i];
			pVector[nRow] -= dVal * pVector[nCol];
		}
	}

	// Normalize to make the diagonals one
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		dVal = m_pData[m_nColumns * nRow + nRow];
		for(nCol = 0; nCol < m_nColumns; nCol++)
			m_pData[m_nColumns * nRow + nCol] /= dVal;
		pVector[nRow] /= dVal;
	}
}

int GMatrix::CountNonZeroElements()
{
	int nCount = 0;
	int nRow, nCol;
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		for(nCol = 0; nCol < m_nColumns; nCol++)
		{
			if(Get(nRow, nCol) != 0)
				nCount++;
		}
	}
	return nCount;
}

#ifdef EIGEN
typedef double REAL;
int eigen(int vec, int ortho, int ev_norm, int n, REAL** mat, REAL** eivec, REAL* valre, REAL* valim, int* cnt);

void GMatrix::ComputeEigenVectors(GVector* pOutEigenValues, GMatrix* pOutEigenVectors)
{
	// Convert the matrix to a form that the eigen function can handle
	GAssert(m_nRows == m_nColumns, "expected a square matrix");
	int row, col;
	REAL** pMat = new REAL*[m_nRows];
	for(row = 0; row < m_nRows; row++)
	{
		pMat[row] = new REAL[m_nColumns];
		for(col = 0; col < m_nColumns; col++)
			pMat[row][col] = (REAL)Get(row, col);
	}

	// Allocate space for the output eigen vectors and eigen values
	REAL** pEigenVectors = new REAL*[m_nRows];
	for(row = 0; row < m_nRows; row++)
		pEigenVectors[row] = new REAL[m_nRows];
	REAL* pEigenValuesReal = new REAL[m_nRows];
	REAL* pEigenValuesImag = new REAL[m_nRows];
	int* pCounts = new int[m_nRows];

	// Compute the eigen values and vectors
	int res = eigen(1, 1, 0, m_nRows, pMat, pEigenVectors, pEigenValuesReal, pEigenValuesImag, pCounts);

	// Copy the results
	pOutEigenValues->SetData(pEigenValuesReal, m_nRows, true);
	pOutEigenVectors->Resize(m_nRows, m_nColumns);
	for(row = 0; row < m_nRows; row++)
	{
		for(col = 0; col < m_nColumns; col++)
			pOutEigenVectors->Set(row, col, pEigenVectors[col][row]);
	}

	// Clean up
	delete(pCounts);
	delete(pEigenValuesImag);
	for(row = 0; row < m_nRows; row++)
		delete(pEigenVectors[row]);
	delete(pEigenVectors);
	for(row = 0; row < m_nRows; row++)
		delete(pMat[row]);
	delete(pMat);
}
#else // EIGEN
#ifdef ARPACK

#include <arpack++/armat.h>
#include <arpack++/arcomp.h>
#include <arpack++/arlsmat.h>
#include <arpack++/arlnsmat.h>
#include <arpack++/arlssym.h>
#include <arpack++/arlgsym.h>
#include <arpack++/arlgnsym.h>
#include <arpack++/arlscomp.h>
#include <arpack++/arlgcomp.h>

//#include <arpack++/arbsnsym.h>
//#include <arpack++/ardsnsym.h>
#include <arpack++/arlsnsym.h>
//#include <arpack++/arusnsym.h>

void ArpackError::Set(ArpackError::ErrorCode eCode, char const* szMessage)
{
	fprintf(stderr, "Got an error: [%d] %s\n", eCode, szMessage);
}

void MemoryOverflow()
{
	fprintf(stderr, "memory overflow\n");
}

typedef double ARFLOAT;

void GMatrix::ComputeEigenVectors(GVector* pOutEigenValues, GMatrix* pOutEigenVectors)
{
	// Allocate space for a sparse matrix
	printf("allocating space for the matrix...\n");
	int nRows = m_nRows;
	GAssert(m_nRows == m_nColumns, "expected a square matrix");
	int nNonZeroElements = CountNonZeroElements();
	int* pRowIndexes = new int[nNonZeroElements];
	int* pColIndexes = new int[nRows + 1];
	ARFLOAT* pMatrix = new ARFLOAT[nNonZeroElements];

	// Make the matrix
	printf("making the matrix...\n");
	int col, row;
	int j = 0; // matrix position
	pColIndexes[0] = 0;
	for(col = 0; col < m_nColumns; col++)
	{
		for(row = 0; row < m_nRows; row++)
		{
			if(Get(row, col) != 0)
			{
				pRowIndexes[j] = row;
				pMatrix[j++] = Get(row, col);
			}
		}
		pColIndexes[col + 1] = j;
	}

	// Allocate space for the output
	int nMaxEigenVals = 100;
	printf("allocating space for the output...\n");
	ARFLOAT* pEigValReal = new ARFLOAT[nRows];
	ARFLOAT* pEigValImag = new ARFLOAT[nRows];
	ARFLOAT* pEigVec = new ARFLOAT[nMaxEigenVals * nRows];

	// Create an ARPACK++ matrix
	printf("creating an arpack++ matrix...\n");
	ARluNonSymMatrix<ARFLOAT, ARFLOAT> matrix(nRows, nNonZeroElements, pMatrix, pRowIndexes, pColIndexes);

	// Construct the eigenvalue solver
	printf("constructing eigen solver...\n");
	ARluNonSymStdEig<ARFLOAT> solver(nMaxEigenVals, matrix, "SR", 0, 0.00001, 500000, NULL, true);

	// Find the eigenvalues and vectors
	printf("solving for eigen values and vectors...\n");
	int nEigenValues = solver.EigenValVectors(pEigVec, pEigValReal, pEigValImag);
	printf("Number of eigen values computed: %d\n", nEigenValues);

	// Copy the data into the output space
	pOutEigenVectors->Resize(m_nRows, m_nColumns);
	pOutEigenValues->Resize(m_nRows);
	for(j = 0; j < nEigenValues; j++)
	{
		row = m_nRows - 1 - j;
		for(col = 0; col < m_nColumns; col++)
			pOutEigenVectors->Set(row, col, pEigVec[m_nRows * j + col]);
		pOutEigenValues->Set(row, pEigValReal[j]);
	}

	// Clean up
	delete(pEigVec);
	delete(pEigValImag);
	delete(pEigValReal);
}
#else // ARPACK
#ifdef OCTAVE

#define EIGHT_BYTE_INT long long int
#define GCC_ATTR_NORETURN
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
//#define SIZEOF_LONG 4
#include <octave/octave/Matrix.h>
#include <octave/octave/EIG.h>

void GMatrix::ComputeEigenVectors(GVector* pOutEigenValues, GMatrix* pOutEigenVectors)
{
	Matrix mTmp(m_nRows, m_nColumns);
	int row, col;
	for(row = 0; row < m_nRows; row++)
	{
		for(col = 0; col < m_nColumns; col++)
			mTmp.elem(row, col) = Get(row, col);
	}
	EIG eig(mTmp);
	ComplexColumnVector eigenvalues = eig.eigenvalues();
	ComplexMatrix eigenvectors = eig.eigenvectors();
	pOutEigenVectors->Resize(m_nRows, m_nColumns);
	pOutEigenValues->Resize(m_nRows);
	for(row = 0; row < m_nRows; row++)
	{
		for(col = 0; col < m_nColumns; col++)
			pOutEigenVectors->Set(row, col, eigenvectors.elem(col, row).real());
		pOutEigenValues->Set(row, (double)eigenvalues.elem(row).real());
	}
}

#else // OCTAVE

void GMatrix::ComputeEigenVectors(GVector* pOutEigenValues, GMatrix* pOutEigenVectors)
{
	const char* szMessage = "No method is available for computing eigen vectors. (You need to uncomment one of the #defines at the top of this file and link to the corresponding library.)";
	GAssert(false, szMessage);
	throw(szMessage);
}

#endif // !OCTAVE
#endif // !ARPACK
#endif // !EIGEN
