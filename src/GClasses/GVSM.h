#ifndef __GVSM_H__
#define __GVSM_H__

class GConstStringHashTable;
class GStemmer;
class GStringHeap;
class GPointerArray;


typedef void (*ProcessWordFunc)(void* pThis, const char* szWord, int nLen);

// The Vector Space Model for information retrieval represents
// each term in a text document as a weight value in an orthogonal
// dimension. This class is designed to be used in two passes:
// First you build up the vocabulary by feeding it a representative
// collection of documents, then you obtain the vector for each document.
class GVSM
{
protected:
	GStemmer* m_pStemmer;
	GConstStringHashTable* m_pStopWords;
	GConstStringHashTable* m_pVocabulary;
	GStringHeap* m_pStringHeap;
	GPointerArray* m_pWords;
	bool m_bVocabularyLocked;
	double* m_pCurrentVector;
	int m_nVocabSizeBeforeThisDoc;
	int* m_pWordCount;
	int m_nDocumentCount;

public:
	GVSM();
	~GVSM();

	// Parses all the words in the file and calls pProcessWordFunc for each one
	static void ExtractWords(const char* pFile, int nSize, ProcessWordFunc pProcessWordFunc, void* pThis);

	// Extract all the words from the document and add all the non-stop-words to the vocabulary
	void AddDocumentToVocabulary(const char* szText, int nLength);

	// Returns the number of words in the vocabulary
	int GetVocabSize();

	// Returns a word in the vocab
	const char* GetVocabWord(int nIndex);

	// Returns the maximum number of occurrences of a word in any training document
	int GetMaxWordFrequency(int nIndex);

	// Returns the number of training documents which contain a word
	int GetNumberOfDocsContainingWord(int nIndex);

	// Return the number of documents used to train the vocabulary
	int GetTrainingDocumentCount();

	// Returns the index of the stem word.  Returns -1 if not in the vocabulary.
	int FindStemIndex(const char* szStem);

	// pOutVector should be an array of n doubles where n = GetVocabSize().
	void GetVector(double* pOutVector, const char* szText, int nLength);

	// Internal helper method--don't call it
	void AddWordToVocabulary(const char* szWord, int nLen);

	// Internal helper method--don't call it
	void AddWordToVector(const char* szWord, int nLen);
};

#endif // __GVSM_H__
