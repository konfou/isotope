#ifndef __DEBUGSOURCEMANAGER_H__
#define __DEBUGSOURCEMANAGER_H__

class GAVLTree;
class GPointerArray;
class GXMLTag;
class COProject;
class EInstrArray;
class EMethod;
class Library;

class DebugSourceManager
{
protected:
	char m_szCurrentSource[512];
	GPointerArray* m_pSourceSearchPaths;
	GAVLTree* m_pSourceCacheSpecified;
	GAVLTree* m_pSourceCacheFound;
	GXMLTag* m_pCurrentTypeTag;
	COProject* m_pPartialProject;
	char* m_szDisassembly;

public:
	DebugSourceManager(Library* pLibrary);
	virtual ~DebugSourceManager();

	void AddSourceSearchPath(const char* szPath);
	const char* GetSourceFile(const char* szFilename, const char** szOutActualFilename);
	bool MakeSymbols(EInstrArray* pEInstrArray, EMethod* pEMethod);
	GXMLTag* GetCurrentTypeTag() { return m_pCurrentTypeTag; }
	void SetCurrentTypeTag(GXMLTag* pTag) { m_pCurrentTypeTag = pTag; }
	bool IsCurrentSource(const char* szSource);
	void SetCurrentSource(const char* szSource);
	const char* GetDisassembly() { return m_szDisassembly; }
};

#endif // __DEBUGSOURCEMANAGER_H__
