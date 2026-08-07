#include <stdio.h>
#include <string.h>

#include "../xge.c"

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("test_tsf_candidate_ui failed: %s\n", (message)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct test_candidate_t {
	ITfCandidateListUIElementBehavior tInterface;
	LONG iRefCount;
	UINT iSelection;
	UINT iSetSelection;
	int bFinalized;
} test_candidate_t;

typedef struct test_ui_manager_t {
	ITfUIElementMgr tInterface;
	LONG iRefCount;
	test_candidate_t* pCandidate;
} test_ui_manager_t;

static test_candidate_t* __testCandidate(ITfCandidateListUIElementBehavior* pInterface)
{
	return (test_candidate_t*)((char*)pInterface - offsetof(test_candidate_t, tInterface));
}

static test_ui_manager_t* __testManager(ITfUIElementMgr* pInterface)
{
	return (test_ui_manager_t*)((char*)pInterface - offsetof(test_ui_manager_t, tInterface));
}

static HRESULT STDMETHODCALLTYPE __testCandidateQuery(ITfCandidateListUIElementBehavior* pInterface,
	REFIID pIid, void** ppObject)
{
	if ( ppObject == NULL ) return E_POINTER;
	*ppObject = NULL;
	if ( IsEqualIID(pIid, &IID_IUnknown) ||
	     IsEqualIID(pIid, &XGE_IID_ITfCandidateListUIElement) ||
	     IsEqualIID(pIid, &XGE_IID_ITfCandidateListUIElementBehavior) ) {
		*ppObject = pInterface;
		InterlockedIncrement(&__testCandidate(pInterface)->iRefCount);
		return S_OK;
	}
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE __testCandidateAddRef(ITfCandidateListUIElementBehavior* pInterface)
{
	return (ULONG)InterlockedIncrement(&__testCandidate(pInterface)->iRefCount);
}

static ULONG STDMETHODCALLTYPE __testCandidateRelease(ITfCandidateListUIElementBehavior* pInterface)
{
	return (ULONG)InterlockedDecrement(&__testCandidate(pInterface)->iRefCount);
}

static HRESULT STDMETHODCALLTYPE __testCandidateDescription(ITfCandidateListUIElementBehavior* pInterface, BSTR* pText)
{
	(void)pInterface;
	if ( pText == NULL ) return E_POINTER;
	*pText = SysAllocString(L"test candidates");
	return (*pText != NULL) ? S_OK : E_OUTOFMEMORY;
}

static HRESULT STDMETHODCALLTYPE __testCandidateGuid(ITfCandidateListUIElementBehavior* pInterface, GUID* pGuid)
{
	(void)pInterface;
	if ( pGuid == NULL ) return E_POINTER;
	*pGuid = XGE_IID_ITfCandidateListUIElement;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateShow(ITfCandidateListUIElementBehavior* pInterface, BOOL bShow)
{
	(void)pInterface; (void)bShow;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateIsShown(ITfCandidateListUIElementBehavior* pInterface, BOOL* pShown)
{
	(void)pInterface;
	if ( pShown == NULL ) return E_POINTER;
	*pShown = TRUE;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateFlags(ITfCandidateListUIElementBehavior* pInterface, DWORD* pFlags)
{
	(void)pInterface;
	if ( pFlags == NULL ) return E_POINTER;
	*pFlags = TF_CLUIE_COUNT | TF_CLUIE_SELECTION | TF_CLUIE_STRING |
	          TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateDocument(ITfCandidateListUIElementBehavior* pInterface,
	ITfDocumentMgr** ppDocument)
{
	(void)pInterface;
	if ( ppDocument == NULL ) return E_POINTER;
	*ppDocument = NULL;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateCount(ITfCandidateListUIElementBehavior* pInterface, UINT* pCount)
{
	(void)pInterface;
	if ( pCount == NULL ) return E_POINTER;
	*pCount = 3;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateSelection(ITfCandidateListUIElementBehavior* pInterface, UINT* pSelection)
{
	if ( pSelection == NULL ) return E_POINTER;
	*pSelection = __testCandidate(pInterface)->iSelection;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateString(ITfCandidateListUIElementBehavior* pInterface,
	UINT iIndex, BSTR* pText)
{
	static const WCHAR* arrText[] = { L"ni", L"\x4F60", L"\x59AE\x597D" };
	(void)pInterface;
	if ( pText == NULL ) return E_POINTER;
	if ( iIndex >= 3 ) return E_INVALIDARG;
	*pText = SysAllocString(arrText[iIndex]);
	return (*pText != NULL) ? S_OK : E_OUTOFMEMORY;
}

static HRESULT STDMETHODCALLTYPE __testCandidatePages(ITfCandidateListUIElementBehavior* pInterface,
	UINT* pPages, UINT iCapacity, UINT* pCount)
{
	(void)pInterface;
	if ( pCount == NULL ) return E_POINTER;
	*pCount = 2;
	if ( pPages == NULL || iCapacity < 2 ) return E_INVALIDARG;
	pPages[0] = 0;
	pPages[1] = 2;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateSetPages(ITfCandidateListUIElementBehavior* pInterface,
	UINT* pPages, UINT iCount)
{
	(void)pInterface; (void)pPages; (void)iCount;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateCurrentPage(ITfCandidateListUIElementBehavior* pInterface,
	UINT* pPage)
{
	(void)pInterface;
	if ( pPage == NULL ) return E_POINTER;
	*pPage = 0;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateSetSelection(ITfCandidateListUIElementBehavior* pInterface,
	UINT iSelection)
{
	__testCandidate(pInterface)->iSetSelection = iSelection;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateFinalize(ITfCandidateListUIElementBehavior* pInterface)
{
	__testCandidate(pInterface)->bFinalized = 1;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testCandidateAbort(ITfCandidateListUIElementBehavior* pInterface)
{
	(void)pInterface;
	return S_OK;
}

static const ITfCandidateListUIElementBehaviorVtbl g_testCandidateVtbl = {
	__testCandidateQuery,
	__testCandidateAddRef,
	__testCandidateRelease,
	__testCandidateDescription,
	__testCandidateGuid,
	__testCandidateShow,
	__testCandidateIsShown,
	__testCandidateFlags,
	__testCandidateDocument,
	__testCandidateCount,
	__testCandidateSelection,
	__testCandidateString,
	__testCandidatePages,
	__testCandidateSetPages,
	__testCandidateCurrentPage,
	__testCandidateSetSelection,
	__testCandidateFinalize,
	__testCandidateAbort
};

static HRESULT STDMETHODCALLTYPE __testManagerQuery(ITfUIElementMgr* pInterface, REFIID pIid, void** ppObject)
{
	if ( ppObject == NULL ) return E_POINTER;
	*ppObject = NULL;
	if ( IsEqualIID(pIid, &IID_IUnknown) || IsEqualIID(pIid, &IID_ITfUIElementMgr) ) {
		*ppObject = pInterface;
		InterlockedIncrement(&__testManager(pInterface)->iRefCount);
		return S_OK;
	}
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE __testManagerAddRef(ITfUIElementMgr* pInterface)
{
	return (ULONG)InterlockedIncrement(&__testManager(pInterface)->iRefCount);
}

static ULONG STDMETHODCALLTYPE __testManagerRelease(ITfUIElementMgr* pInterface)
{
	return (ULONG)InterlockedDecrement(&__testManager(pInterface)->iRefCount);
}

static HRESULT STDMETHODCALLTYPE __testManagerBegin(ITfUIElementMgr* pInterface, ITfUIElement* pElement,
	WINBOOL* pShow, DWORD* pId)
{
	(void)pInterface; (void)pElement; (void)pShow; (void)pId;
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE __testManagerUpdate(ITfUIElementMgr* pInterface, DWORD iId)
{
	(void)pInterface; (void)iId;
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE __testManagerEnd(ITfUIElementMgr* pInterface, DWORD iId)
{
	(void)pInterface; (void)iId;
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE __testManagerGet(ITfUIElementMgr* pInterface, DWORD iId,
	ITfUIElement** ppElement)
{
	test_ui_manager_t* pManager;
	(void)iId;
	if ( ppElement == NULL ) return E_POINTER;
	pManager = __testManager(pInterface);
	*ppElement = (ITfUIElement*)&pManager->pCandidate->tInterface;
	__testCandidateAddRef(&pManager->pCandidate->tInterface);
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __testManagerEnum(ITfUIElementMgr* pInterface, IEnumTfUIElements** ppItems)
{
	(void)pInterface;
	if ( ppItems != NULL ) *ppItems = NULL;
	return E_NOTIMPL;
}

static ITfUIElementMgrVtbl g_testManagerVtbl = {
	__testManagerQuery,
	__testManagerAddRef,
	__testManagerRelease,
	__testManagerBegin,
	__testManagerUpdate,
	__testManagerEnd,
	__testManagerGet,
	__testManagerEnum
};

int main(void)
{
	xge_desc_t tDesc;
	test_candidate_t tCandidate;
	test_ui_manager_t tManager;
	xge_ime_candidate_info_t tInfo;
	xge_input_event_t tEvent;
	WINBOOL bShow;
	char sText[16];
	int iFailed;

	iFailed = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	TEST_CHECK(xgeInit(&tDesc) == XGE_OK, "xgeInit");
	memset(&tCandidate, 0, sizeof(tCandidate));
	tCandidate.tInterface.lpVtbl = &g_testCandidateVtbl;
	tCandidate.iRefCount = 1;
	tCandidate.iSelection = 1;
	memset(&tManager, 0, sizeof(tManager));
	tManager.tInterface.lpVtbl = &g_testManagerVtbl;
	tManager.iRefCount = 1;
	tManager.pCandidate = &tCandidate;
	g_xgeWin32Ime.pUiElementMgr = &tManager.tInterface;
	g_xgeWin32Ime.iMode = XGE_IME_MODE_FULL;
	g_xgeWin32Ime.bCandidatePresenterReady = 1;
	g_xgeWin32Ime.iCandidateElementId = TF_INVALID_UIELEMENTID;
	g_xgeWin32Ime.iCandidateSelection = -1;

	bShow = TRUE;
	TEST_CHECK(__xgeTsfUiSinkBegin(&g_xgeWin32Ime.tUiElementSink.tSink, 42, &bShow) == S_OK,
	           "candidate begin");
	TEST_CHECK(!bShow, "full mode hides native candidate UI");
	TEST_CHECK(xgeImeCandidateGetInfo(&tInfo) == XGE_OK, "candidate info");
	TEST_CHECK(tInfo.bVisible && tInfo.bCanSelect && tInfo.iCount == 3,
	           "candidate state");
	TEST_CHECK(tInfo.iSelection == 1 && tInfo.iPageStart == 0 && tInfo.iPageSize == 2,
	           "candidate page");
	TEST_CHECK(xgeImeCandidateGetText(1, sText, (int)sizeof(sText)) == 3 &&
	           strcmp(sText, "\xE4\xBD\xA0") == 0, "candidate UTF-8 text");
	TEST_CHECK(xgeImeCandidateSelect(2) == XGE_OK && tCandidate.iSetSelection == 2,
	           "candidate selection");
	TEST_CHECK(xgeImeCandidateFinalize() == XGE_OK && tCandidate.bFinalized,
	           "candidate finalize");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
	           tEvent.iType == XGE_EVENT_IME_CANDIDATE_START, "candidate start event");
	TEST_CHECK(__xgeTsfUiSinkEnd(&g_xgeWin32Ime.tUiElementSink.tSink, 42) == S_OK,
	           "candidate end");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
	           tEvent.iType == XGE_EVENT_IME_CANDIDATE_END, "candidate end event");

	g_xgeWin32Ime.bCandidatePresenterReady = 0;
	bShow = FALSE;
	TEST_CHECK(__xgeTsfUiSinkBegin(&g_xgeWin32Ime.tUiElementSink.tSink, 43, &bShow) == S_OK && bShow,
	           "native candidate remains visible without presenter");
	TEST_CHECK(__xgeTsfUiSinkEnd(&g_xgeWin32Ime.tUiElementSink.tSink, 43) == S_OK,
	           "fallback candidate end");

cleanup:
	g_xgeWin32Ime.pUiElementMgr = NULL;
	__xgeTsfCandidateClose(0);
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_tsf_candidate_ui passed\n");
	return 0;
}
