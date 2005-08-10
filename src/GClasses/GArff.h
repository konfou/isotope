/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GARFF_H__
#define __GARFF_H__

// ARFF = Attribute-Relation File Format

class GArffAttribute;
class GArffData;
class GPointerArray;

class GArffRelation
{
protected:
	char* m_szName;
	GPointerArray* m_pAttributes;
	int m_nInputCount;
	int* m_pInputIndexes;
	int m_nOutputCount;
	int* m_pOutputIndexes;

public:
	GArffRelation();
	~GArffRelation();

	// Parses a ".arff" file and returns a GArffRelation and a GArffData.  You must delete them both
	static GArffRelation* ParseFile(GArffData** ppOutData, const char* szFile, int nLen);

	// Returns the total number of attributes (both input and output) in this relation
	int GetAttributeCount();

	// Add an attribute to the relation
	void AddAttribute(GArffAttribute* pAttr);

	// Returns the number of input attributes in this relation
	int GetInputCount();

	// Returns the number of output attributes in this relation
	int GetOutputCount();

	// Returns the attribute index of the n'th input attribute
	int GetInputIndex(int n);

	// Returns the attribute index of the n'th output attribute
	int GetOutputIndex(int n);

	// Returns the attribute at the specified attribute index
	GArffAttribute* GetAttribute(int nAttribute);

	// Returns the total entropy in pData from all output attributes
	double TotalEntropyOfAllOutputs(GArffData* pData);

protected:
	double* ParseDataRow(const char* szFile, int nLen);
	void CountInputs();
};



class GArffAttribute
{
protected:
	char* m_szName;
	int m_nValues;
	char** m_szValues;
	bool m_bIsInput;

	GArffAttribute();
public:
	// To specify a continuous attribute, nValues should be 0 and
	// szValues should be NULL
	GArffAttribute(bool bIsInput, int nValues, const char** szValues);
	
	~GArffAttribute();

	// Parse the attribute section of a ".arff" file
	static GArffAttribute* Parse(const char* szFile, int nLen);

	// Returns true if this is a continuous (as opposed to discreet) attribute
	bool IsContinuous() { return m_nValues == 0; }

	// Returns the index of the specified value
	int FindEnumeratedValue(const char* szValue);

	// Returns the number of discreet values in this attribute
	int GetValueCount();

	// Returns the name of this attribute
	const char* GetName() { return m_szName; }

	// Returns the n'th discreet value that this attribute can have
	const char* GetValue(int n);

	// Returns true if this is an input attribute
	bool IsInput() { return m_bIsInput; }

	// Sets whether this is an input or output attribute.
	void SetIsInput(bool b) { m_bIsInput = b; }
};



class GArffData
{
protected:
	GPointerArray* m_pRows;

public:
	GArffData(int nGrowSize);
	~GArffData();

	// Takes ownership of pRow
	void AddRow(double* pRow);

	// Returns the number of rows
	int GetRowCount();

	// Returns a pointer to the row
	double* GetRow(int nRow);

	// you must delete the row this returns
	double* DropRow(int nRow);

	// Abandons (leaks) all the rows of data
	void DropAllRows();

	// Randomizes row order
	void ShuffleRows();

	// Splits this set of data into two sets such that this set
	// contains all rows where the value in column "nColumn" is
	// greater than dPivot and the set returned contains those
	// less-than-or-equal-to dPivot.
	GArffData* SplitByPivot(int nColumn, double dPivot);

	// Splits this set of data into a unique set for each
	// possible enumeration value of the attribute.  You are
	// responsible to delete each set of data as well as the
	// array of pointers that this returns
	GArffData** SplitByAttribute(GArffRelation* pRelation, int nAttribute);

	// Splits this set of data into two sets such that this set
	// contains "nRows" rows and the returned set contains the rest
	GArffData* SplitBySize(int nRows);

	// Steals all the rows from pData and adds them to this set.
	// (You still have to delete pData)
	void Merge(GArffData* pData);

	// Measures the entropy of this set relative to the specified attribute
	double MeasureEntropy(GArffRelation* pRelation, int nColumn);

	// Neural Nets based on sigmoids require all output values to be
	// between 0 and 1 (and preferrably not extremely close to 0 or 1).
	// This function converts all the data (both inputs and outputs)
	// to analog values in this range.  For continuous data, it merely
	// checks that the range is between .00001 and .99999 (and throws
	// if it's not).  For discreet data it uses this conversion:
	// (.5 + nVal) / nValues
	void Analogize(GArffRelation* pRelation);

	// This undoes what Analogize does
	void Unanalogize(GArffRelation* pRelation);
};


#endif // __GARFF_H__
