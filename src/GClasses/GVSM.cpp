#include "GVSM.h"
#include "GHashTable.h"
#include "GStemmer.h"
#include "GArray.h"
#include <math.h>

const char* g_szStopWords[] = 
{
	"a",
	"about",
	"all",
	"also",
	"although",
	"an",
	"and",
	"any",
	"are",
	"as",
	"at",
	"be",
	"but",
	"by",
	"can",
	"did",
	"each",
	"every",
	"for",
	"from",
	"had",
	"have",
	"he",
	"her",
	"him",
	"his",
	"how",
	"i",
	"if",
	"in",
	"is",
	"it",
	"its",
	"my",
	"nbsp",
	"next",
	"no",
	"not",
	"of",
	"on",
	"one",
	"or",
	"our",
	"out",
	"quite",
	"really",
	"so",
	"some",
	"that",
	"the",
	"them",
	"then",
	"there",
	"this",
	"to",
	"too",
	"use",
	"very",
	"was",
	"we",
	"what",
	"when",
	"where",
	"who",
	"will",
	"with",
	"you",
};

struct GVSMWordStats
{
	int m_nMaxFrequency;
	int m_nDocsContainingWord;
};

GVSM::GVSM()
{
	m_pStemmer = new GStemmer();
	m_pStopWords = new GConstStringHashTable(113, false);
	int i;
	for(i = 0; i < (int)(sizeof(g_szStopWords) / sizeof(const char*)); i++)
		m_pStopWords->Add(g_szStopWords[i], NULL);
	m_pVocabulary = new GConstStringHashTable(1031, false);
	m_pStringHeap = new GStringHeap(1024);
	m_pWords = new GPointerArray(1024);
	m_pCurrentVector = NULL;
	m_nVocabSizeBeforeThisDoc = 0;
	m_pWordCount = NULL;
	m_nDocumentCount = 0;
}

GVSM::~GVSM()
{
	delete(m_pStemmer);
	delete(m_pStopWords);
	delete(m_pVocabulary);
	delete(m_pStringHeap);
	delete(m_pWords);
	delete(m_pWordCount);
}

/*static*/ void GVSM::ExtractWords(const char* pFile, int nSize, ProcessWordFunc pProcessWordFunc, void* pThis)
{
	int nPos = 0;
	int nWordStart;
	while(true)
	{
		// Skip whitespace
		while(nPos < nSize && pFile[nPos] < 'A')
			nPos++;
		nWordStart = nPos;

		// Find the end of the word
		while(nPos < nSize && pFile[nPos] >= 'A')
			nPos++;

		// Add the word
		pProcessWordFunc(pThis, &pFile[nWordStart], nPos - nWordStart);

		// Check for end of file
		if(nPos >= nSize)
			break;
	}
}

void GVSM::AddWordToVocabulary(const char* szWord, int nLen)
{
	if(nLen < 4)
		return;

	// Find the stem
	const char* szStem = m_pStemmer->GetStem(szWord, nLen);

	// Check for stop words
	void* pValue;
	if(m_pStopWords->Get(szStem, &pValue))
		return;

	// Check for existing words
	int nIndex;
	if(m_pVocabulary->Get(szStem, (void**)&nIndex))
	{
		struct GVSMWordStats* pWordStats = (struct GVSMWordStats*)m_pWords->GetPointer(nIndex);
		if(nIndex < m_nVocabSizeBeforeThisDoc)
		{
			// This word was introduced by a previous document
			if(m_pWordCount[nIndex] == 0)
				pWordStats->m_nDocsContainingWord++;
			m_pWordCount[nIndex]++;
			if(m_pWordCount[nIndex] > pWordStats->m_nMaxFrequency)
				pWordStats->m_nMaxFrequency = m_pWordCount[nIndex];
		}
		else
		{
			// This word was previously introduced by this document
			pWordStats->m_nMaxFrequency++;
		}
	}
	else
	{
		// This is a new vocabulary word
		char* pNewWord = m_pStringHeap->Allocate(sizeof(struct GVSMWordStats) + strlen(szStem) + 1);
		struct GVSMWordStats* pWordStats = (struct GVSMWordStats*)pNewWord;
		pWordStats->m_nMaxFrequency = 1;
		pWordStats->m_nDocsContainingWord = 1;
		strcpy(pNewWord + sizeof(struct GVSMWordStats), szStem);
		int nIndex = m_pWords->GetSize();
		m_pWords->AddPointer(pNewWord);
		m_pVocabulary->Add(pNewWord + sizeof(struct GVSMWordStats), (const void*)nIndex);
	}
}

void AddWordToVocab(void* pThis, const char* szWord, int nLen)
{
	((GVSM*)pThis)->AddWordToVocabulary(szWord, nLen);
}

void GVSM::AddDocumentToVocabulary(const char* szText, int nLength)
{
	m_nVocabSizeBeforeThisDoc = GetVocabSize();
	m_pWordCount = new int[m_nVocabSizeBeforeThisDoc]; // todo: it would scale better if we didn't allocate this every time a document is added
	memset(m_pWordCount, '\0', sizeof(int) * m_nVocabSizeBeforeThisDoc);
	m_nDocumentCount++;
	ExtractWords(szText, nLength, AddWordToVocab, this);
	delete(m_pWordCount);
	m_pWordCount = NULL;
}

int GVSM::GetVocabSize()
{
	return m_pWords->GetSize();
}

int GVSM::FindStemIndex(const char* szStem)
{
	int nIndex;
	if(m_pVocabulary->Get(szStem, (void**)&nIndex))
		return nIndex;
	else
		return -1;
}

const char* GVSM::GetVocabWord(int nIndex)
{
	return (const char*)m_pWords->GetPointer(nIndex) + sizeof(struct GVSMWordStats);
}

int GVSM::GetMaxWordFrequency(int nIndex)
{
	return ((struct GVSMWordStats*)m_pWords->GetPointer(nIndex))->m_nMaxFrequency;
}

int GVSM::GetNumberOfDocsContainingWord(int nIndex)
{
	return ((struct GVSMWordStats*)m_pWords->GetPointer(nIndex))->m_nDocsContainingWord;
}

int GVSM::GetTrainingDocumentCount()
{
	return m_nDocumentCount;
}

void GVSM::AddWordToVector(const char* szWord, int nLen)
{
	const char* szStem = m_pStemmer->GetStem(szWord, nLen);
	int nIndex = FindStemIndex(szStem);
	if(nIndex < 0)
		return;
	struct GVSMWordStats* pWordStats = (struct GVSMWordStats*)m_pWords->GetPointer(nIndex);
	m_pCurrentVector[nIndex] += ((1.0 / pWordStats->m_nMaxFrequency) * log(m_nDocumentCount / pWordStats->m_nDocsContainingWord));
}

void AddWordToVec(void* pThis, const char* szWord, int nLen)
{
	((GVSM*)pThis)->AddWordToVector(szWord, nLen);
}

void GVSM::GetVector(double* pOutVector, const char* szText, int nLength)
{
	int nWords = GetVocabSize();
	int i;
	for(i = 0; i < nWords; i++)
		pOutVector[i] = 0;
	m_pCurrentVector = pOutVector;
	ExtractWords(szText, nLength, AddWordToVec, this);
	m_pCurrentVector = NULL;
}
