#define XGE_DATA_OBJECT_MAGIC 0x5847444Fu

typedef struct xge_data_slot_t {
	char* sFormat;
	void* pValue;
	size_t iSize;
	xge_data_provider_proc onRead;
	xge_data_provider_free_proc onFree;
	void* pUser;
} xge_data_slot_t;

struct xge_data_object_t {
	uint32_t iMagic;
	int iRefCount;
	xge_data_slot_t* pSlots;
	int iCount;
	int iCapacity;
};

static xge_drag_event_proc g_xgeDragEventProc;
static void* g_xgeDragEventUser;
static int g_xgeDragActive;
static xge_data_object g_xgeDragData;
static uint32_t g_xgeDragAllowedEffects;
static uint32_t g_xgeDragSuggestedEffect;
static uint64_t g_xgeDragSequence;
static int g_xgeNativeDragIn;
static int g_xgeNativeDragOut;

static void __xgeDragSessionClear(void)
{
	if ( g_xgeDragData != NULL ) xgeDataObjectRelease(g_xgeDragData);
	g_xgeDragData = NULL;
	g_xgeDragAllowedEffects = 0u;
	g_xgeDragSuggestedEffect = 0u;
	g_xgeDragActive = 0;
}

static void __xgeDragDropUnit(void)
{
	__xgeDragDropPlatformUnit();
	__xgeDragSessionClear();
	g_xgeDragSequence = 0u;
	g_xgeDragEventProc = NULL;
	g_xgeDragEventUser = NULL;
}

static int __xgeDataObjectValid(xge_data_object pData)
{
	return pData != NULL && pData->iMagic == XGE_DATA_OBJECT_MAGIC &&
		pData->iRefCount > 0;
}

static char* __xgeDataStringCopy(const char* sText)
{
	char* sCopy;
	size_t iLength;

	if ( sText == NULL || sText[0] == '\0' ) return NULL;
	iLength = strlen(sText);
	sCopy = (char*)xrtMalloc(iLength + 1u);
	if ( sCopy == NULL ) return NULL;
	memcpy(sCopy, sText, iLength + 1u);
	return sCopy;
}

static void __xgeDataSlotClear(xge_data_slot_t* pSlot)
{
	if ( pSlot == NULL ) return;
	if ( pSlot->onFree != NULL ) pSlot->onFree(pSlot->pUser);
	if ( pSlot->pValue != NULL ) xrtFree(pSlot->pValue);
	if ( pSlot->sFormat != NULL ) xrtFree(pSlot->sFormat);
	memset(pSlot, 0, sizeof(*pSlot));
}

static int __xgeDataSlotFind(xge_data_object pData, const char* sFormat)
{
	int i;

	if ( !__xgeDataObjectValid(pData) || sFormat == NULL ) return -1;
	for ( i = 0; i < pData->iCount; i++ ) {
		if ( strcmp(pData->pSlots[i].sFormat, sFormat) == 0 ) return i;
	}
	return -1;
}

static int __xgeDataSlotReserve(xge_data_object pData, int iCapacity)
{
	xge_data_slot_t* pNew;
	int iNewCapacity;

	if ( iCapacity <= pData->iCapacity ) return XGE_OK;
	iNewCapacity = pData->iCapacity > 0 ? pData->iCapacity * 2 : 4;
	while ( iNewCapacity < iCapacity ) iNewCapacity *= 2;
	pNew = (xge_data_slot_t*)xrtRealloc(pData->pSlots,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	memset(pNew + pData->iCapacity, 0,
		sizeof(*pNew) * (size_t)(iNewCapacity - pData->iCapacity));
	pData->pSlots = pNew;
	pData->iCapacity = iNewCapacity;
	return XGE_OK;
}

static int __xgeDataSlotPrepare(xge_data_object pData, const char* sFormat,
	xge_data_slot_t** ppSlot)
{
	xge_data_slot_t* pSlot;
	char* sCopy;
	int iIndex;
	int iRet;

	if ( !__xgeDataObjectValid(pData) || sFormat == NULL || sFormat[0] == '\0' ||
		ppSlot == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	iIndex = __xgeDataSlotFind(pData, sFormat);
	if ( iIndex >= 0 ) {
		pSlot = &pData->pSlots[iIndex];
		if ( pSlot->onFree != NULL ) pSlot->onFree(pSlot->pUser);
		if ( pSlot->pValue != NULL ) xrtFree(pSlot->pValue);
		pSlot->pValue = NULL;
		pSlot->iSize = 0u;
		pSlot->onRead = NULL;
		pSlot->onFree = NULL;
		pSlot->pUser = NULL;
		*ppSlot = pSlot;
		return XGE_OK;
	}
	iRet = __xgeDataSlotReserve(pData, pData->iCount + 1);
	if ( iRet != XGE_OK ) return iRet;
	sCopy = __xgeDataStringCopy(sFormat);
	if ( sCopy == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	pSlot = &pData->pSlots[pData->iCount++];
	pSlot->sFormat = sCopy;
	*ppSlot = pSlot;
	return XGE_OK;
}

int xgeDataObjectCreate(xge_data_object* ppData)
{
	xge_data_object pData;

	if ( ppData == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*ppData = NULL;
	pData = (xge_data_object)xrtMalloc(sizeof(*pData));
	if ( pData == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	memset(pData, 0, sizeof(*pData));
	pData->iMagic = XGE_DATA_OBJECT_MAGIC;
	pData->iRefCount = 1;
	*ppData = pData;
	return XGE_OK;
}

int xgeDataObjectAddRef(xge_data_object pData)
{
	if ( !__xgeDataObjectValid(pData) || pData->iRefCount == INT_MAX ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pData->iRefCount++;
	return XGE_OK;
}

void xgeDataObjectRelease(xge_data_object pData)
{
	int i;

	if ( !__xgeDataObjectValid(pData) ) return;
	pData->iRefCount--;
	if ( pData->iRefCount > 0 ) return;
	for ( i = 0; i < pData->iCount; i++ ) __xgeDataSlotClear(&pData->pSlots[i]);
	if ( pData->pSlots != NULL ) xrtFree(pData->pSlots);
	pData->iMagic = 0u;
	xrtFree(pData);
}

int xgeDataObjectSet(xge_data_object pData, const char* sFormat,
	const void* pValue, size_t iSize)
{
	xge_data_slot_t* pSlot;
	void* pCopy;
	int iRet;

	if ( iSize > 0u && pValue == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	pCopy = NULL;
	if ( iSize > 0u ) {
		pCopy = xrtMalloc(iSize);
		if ( pCopy == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
		memcpy(pCopy, pValue, iSize);
	}
	iRet = __xgeDataSlotPrepare(pData, sFormat, &pSlot);
	if ( iRet != XGE_OK ) {
		if ( pCopy != NULL ) xrtFree(pCopy);
		return iRet;
	}
	pSlot->pValue = pCopy;
	pSlot->iSize = iSize;
	return XGE_OK;
}

int xgeDataObjectSetProvider(xge_data_object pData, const char* sFormat,
	xge_data_provider_proc onRead, xge_data_provider_free_proc onFree, void* pUser)
{
	xge_data_slot_t* pSlot;
	int iRet;

	if ( onRead == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = __xgeDataSlotPrepare(pData, sFormat, &pSlot);
	if ( iRet != XGE_OK ) return iRet;
	pSlot->onRead = onRead;
	pSlot->onFree = onFree;
	pSlot->pUser = pUser;
	return XGE_OK;
}

int xgeDataObjectFormatCount(xge_data_object pData)
{
	return __xgeDataObjectValid(pData) ? pData->iCount : 0;
}

const char* xgeDataObjectFormatAt(xge_data_object pData, int iIndex)
{
	if ( !__xgeDataObjectValid(pData) || iIndex < 0 || iIndex >= pData->iCount ) return NULL;
	return pData->pSlots[iIndex].sFormat;
}

int xgeDataObjectHas(xge_data_object pData, const char* sFormat)
{
	return __xgeDataSlotFind(pData, sFormat) >= 0;
}

int xgeDataObjectGet(xge_data_object pData, const char* sFormat,
	void* pOutput, size_t iCapacity, size_t* pOutputSize)
{
	xge_data_slot_t* pSlot;
	int iIndex;

	if ( pOutputSize == NULL || (pOutput == NULL && iCapacity != 0u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOutputSize = 0u;
	iIndex = __xgeDataSlotFind(pData, sFormat);
	if ( iIndex < 0 ) return XGE_ERROR_NOT_FOUND;
	pSlot = &pData->pSlots[iIndex];
	if ( pSlot->onRead != NULL ) {
		return pSlot->onRead(sFormat, pOutput, iCapacity, pOutputSize, pSlot->pUser);
	}
	*pOutputSize = pSlot->iSize;
	if ( pOutput == NULL ) return XGE_OK;
	if ( iCapacity < pSlot->iSize ) return XGE_ERROR_BUFFER_TOO_SMALL;
	if ( pSlot->iSize > 0u ) memcpy(pOutput, pSlot->pValue, pSlot->iSize);
	return XGE_OK;
}

int xgeDragDropCapsGet(xge_drag_drop_caps_t* pCaps)
{
	if ( pCaps == NULL || (pCaps->iSize != 0u &&
		pCaps->iSize < sizeof(*pCaps)) ) return XGE_ERROR_INVALID_ARGUMENT;
	memset(pCaps, 0, sizeof(*pCaps));
	pCaps->iSize = sizeof(*pCaps);
	pCaps->bInternalOnly = 1;
	#if !defined(SOKOL_DUMMY_BACKEND)
	if ( g_xge.bSokolRunning && (g_xgeNativeDragIn || g_xgeNativeDragOut) ) {
		pCaps->bNativeDragIn = g_xgeNativeDragIn;
		pCaps->bNativeDragOut = g_xgeNativeDragOut;
		pCaps->bInternalOnly = 0;
		#if defined(_WIN32)
		pCaps->bCustomFormats = 1;
		#endif
	}
#endif
	return XGE_OK;
}

int xgeDragEventCallbackSet(xge_drag_event_proc onEvent, void* pUser)
{
	g_xgeDragEventProc = onEvent;
	g_xgeDragEventUser = pUser;
	return XGE_OK;
}

uint32_t xgeDragEventDispatch(const xge_drag_event_t* pEvent)
{
	xge_drag_event_t tEvent;
	uint32_t iAllowed;
	uint32_t iEffect;
	uint32_t iMask;

	if ( pEvent == NULL || (pEvent->iSize != 0u &&
		pEvent->iSize < sizeof(*pEvent)) ||
		pEvent->iType < XGE_DRAG_EVENT_ENTER ||
		pEvent->iType > XGE_DRAG_EVENT_CANCEL ) {
		return XGE_DRAG_EFFECT_NONE;
	}
	iMask = XGE_DRAG_EFFECT_COPY | XGE_DRAG_EFFECT_MOVE | XGE_DRAG_EFFECT_LINK;
	tEvent = *pEvent;
	tEvent.iSize = sizeof(tEvent);
	tEvent.iSequence = tEvent.iSequence != 0u ? tEvent.iSequence :
		++g_xgeDragSequence;
	tEvent.fTime = tEvent.fTime > 0.0 ? tEvent.fTime : xrtTimer();
	iAllowed = tEvent.iAllowedEffects & iMask;
	tEvent.iAllowedEffects = iAllowed;
	if ( tEvent.iSuggestedEffect != 0u &&
		((tEvent.iSuggestedEffect & iAllowed) == 0u ||
		 (tEvent.iSuggestedEffect & (tEvent.iSuggestedEffect - 1u)) != 0u) ) {
		tEvent.iSuggestedEffect = 0u;
	}
	if ( g_xgeDragEventProc == NULL ) return XGE_DRAG_EFFECT_NONE;
	iEffect = g_xgeDragEventProc(&tEvent, g_xgeDragEventUser);
	if ( iEffect == 0u || (iEffect & ~iAllowed) != 0u ||
		(iEffect & (iEffect - 1u)) != 0u ) return XGE_DRAG_EFFECT_NONE;
	return iEffect;
}

int xgeDragBegin(xge_data_object pData, uint32_t iAllowedEffects,
	uint32_t iSuggestedEffect)
{
	uint32_t iMask;

	iMask = XGE_DRAG_EFFECT_COPY | XGE_DRAG_EFFECT_MOVE | XGE_DRAG_EFFECT_LINK;
	if ( !__xgeDataObjectValid(pData) || (iAllowedEffects & iMask) == 0u ||
		(iAllowedEffects & ~iMask) != 0u || (iSuggestedEffect & ~iAllowedEffects) != 0u ||
		(iSuggestedEffect != 0u && (iSuggestedEffect & (iSuggestedEffect - 1u)) != 0u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xgeDragActive ) return XGE_ERROR_INVALID_STATE;
	if ( xgeDataObjectAddRef(pData) != XGE_OK ) return XGE_ERROR_INVALID_ARGUMENT;
	g_xgeDragData = pData;
	g_xgeDragAllowedEffects = iAllowedEffects;
	g_xgeDragSuggestedEffect = iSuggestedEffect;
	g_xgeDragActive = 1;
	if ( g_xgeNativeDragOut ) return __xgeDragDropPlatformBegin();
	return XGE_OK;
}

int xgeDragCancel(void)
{
	xge_drag_event_t tEvent;

	if ( !g_xgeDragActive ) return XGE_OK;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XGE_DRAG_EVENT_CANCEL;
	tEvent.iAllowedEffects = g_xgeDragAllowedEffects;
	tEvent.iSuggestedEffect = g_xgeDragSuggestedEffect;
	tEvent.pData = g_xgeDragData;
	if ( g_xgeDragEventProc != NULL ) {
		(void)g_xgeDragEventProc(&tEvent, g_xgeDragEventUser);
	}
	__xgeDragSessionClear();
	return XGE_OK;
}

int xgeDragIsActive(void)
{
	return g_xgeDragActive;
}

static uint32_t __xgeDragDispatchNative(int iType, float fX, float fY,
	uint32_t iModifiers, xge_data_object pData, uint32_t iAllowedEffects,
	uint32_t iSuggestedEffect)
{
	xge_drag_event_t tEvent;
	uint32_t iMask;

	iMask = XGE_DRAG_EFFECT_COPY | XGE_DRAG_EFFECT_MOVE | XGE_DRAG_EFFECT_LINK;
	iAllowedEffects &= iMask;
	if ( iSuggestedEffect != 0u &&
		((iSuggestedEffect & iAllowedEffects) == 0u ||
		 (iSuggestedEffect & (iSuggestedEffect - 1u)) != 0u) ) {
		iSuggestedEffect = 0u;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = iType;
	tEvent.iSequence = ++g_xgeDragSequence;
	tEvent.fTime = xrtTimer();
	tEvent.fX = fX;
	tEvent.fY = fY;
	tEvent.iModifiers = iModifiers;
	tEvent.iAllowedEffects = iAllowedEffects;
	tEvent.iSuggestedEffect = iSuggestedEffect;
	if ( iType == XGE_DRAG_EVENT_COMPLETE ) tEvent.iEffect = iSuggestedEffect;
	tEvent.pData = pData;
	return xgeDragEventDispatch(&tEvent);
}

static int __xgeDragUriSafe(unsigned char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
		c == '~' || c == '/' || c == ':';
}

static size_t __xgeDragFileUriSize(const char* sPath)
{
	const unsigned char* p = (const unsigned char*)sPath;
	size_t iSize = (sPath != NULL && sPath[0] == '/') ? 7u : 8u;

	while ( *p != 0u ) {
		iSize += __xgeDragUriSafe(*p) || *p == '\\' ? 1u : 3u;
		p++;
	}
	return iSize + 2u;
}

static char* __xgeDragFileUriWrite(char* pOut, const char* sPath)
{
	static const char sHex[] = "0123456789ABCDEF";
	const unsigned char* p = (const unsigned char*)sPath;

	if ( sPath[0] == '/' ) {
		memcpy(pOut, "file://", 7u);
		pOut += 7;
	} else {
		memcpy(pOut, "file:///", 8u);
		pOut += 8;
	}
	while ( *p != 0u ) {
		if ( *p == '\\' ) {
			*pOut++ = '/';
		} else if ( __xgeDragUriSafe(*p) ) {
			*pOut++ = (char)*p;
		} else {
			*pOut++ = '%';
			*pOut++ = sHex[*p >> 4];
			*pOut++ = sHex[*p & 15u];
		}
		p++;
	}
	*pOut++ = '\r';
	*pOut++ = '\n';
	return pOut;
}

static void __xgeDragDropSokolFiles(const sapp_event* pEvent)
{
	xge_data_object pData = NULL;
	char* pFiles = NULL;
	char* pUris = NULL;
	char* pWrite;
	const char* sPath;
	size_t iFilesSize = 1u;
	size_t iUrisSize = 1u;
	int iCount;
	int i;

	if ( pEvent == NULL ) return;
	iCount = sapp_get_num_dropped_files();
	if ( iCount <= 0 ) return;
	for ( i = 0; i < iCount; i++ ) {
		sPath = sapp_get_dropped_file_path(i);
		iFilesSize += strlen(sPath) + 1u;
		iUrisSize += __xgeDragFileUriSize(sPath);
	}
	pFiles = (char*)xrtMalloc(iFilesSize);
	pUris = (char*)xrtMalloc(iUrisSize);
	if ( pFiles == NULL || pUris == NULL || xgeDataObjectCreate(&pData) != XGE_OK ) {
		if ( pFiles != NULL ) xrtFree(pFiles);
		if ( pUris != NULL ) xrtFree(pUris);
		return;
	}
	pWrite = pFiles;
	for ( i = 0; i < iCount; i++ ) {
		size_t iLength;
		sPath = sapp_get_dropped_file_path(i);
		iLength = strlen(sPath) + 1u;
		memcpy(pWrite, sPath, iLength);
		pWrite += iLength;
	}
	*pWrite = '\0';
	pWrite = pUris;
	for ( i = 0; i < iCount; i++ ) {
		pWrite = __xgeDragFileUriWrite(pWrite, sapp_get_dropped_file_path(i));
	}
	*pWrite = '\0';
	(void)xgeDataObjectSet(pData, XGE_DATA_FORMAT_FILE_LIST, pFiles, iFilesSize);
	(void)xgeDataObjectSet(pData, XGE_DATA_FORMAT_URI_LIST, pUris,
		(size_t)(pWrite - pUris) + 1u);
	(void)__xgeDragDispatchNative(XGE_DRAG_EVENT_ENTER,
		pEvent->mouse_x, pEvent->mouse_y,
		__xgeModifiersFromSokol(pEvent->modifiers), pData,
		XGE_DRAG_EFFECT_COPY, XGE_DRAG_EFFECT_COPY);
	(void)__xgeDragDispatchNative(XGE_DRAG_EVENT_DROP,
		pEvent->mouse_x, pEvent->mouse_y,
		__xgeModifiersFromSokol(pEvent->modifiers), pData,
		XGE_DRAG_EFFECT_COPY, XGE_DRAG_EFFECT_COPY);
	xgeDataObjectRelease(pData);
	xrtFree(pUris);
	xrtFree(pFiles);
}

#if defined(_WIN32) || defined(_WIN64)

#ifndef COBJMACROS
#define COBJMACROS
#endif
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>

typedef struct xge_win32_drop_target_t {
	IDropTarget iface;
	LONG iRefs;
	HWND hWnd;
	xge_data_object pData;
} xge_win32_drop_target_t;

static xge_win32_drop_target_t* g_xgeWin32DropTarget;
static int g_xgeWin32OleInitialized;

static uint32_t __xgeWin32Modifiers(DWORD iKeys)
{
	uint32_t iModifiers = 0u;
	if ( (iKeys & MK_SHIFT) != 0u ) iModifiers |= XGE_KEY_MOD_SHIFT;
	if ( (iKeys & MK_CONTROL) != 0u ) iModifiers |= XGE_KEY_MOD_CTRL;
	if ( (GetKeyState(VK_MENU) & 0x8000) != 0 ) iModifiers |= XGE_KEY_MOD_ALT;
	return iModifiers;
}

static uint32_t __xgeWin32Effects(DWORD iEffect)
{
	uint32_t iResult = 0u;
	if ( (iEffect & DROPEFFECT_COPY) != 0u ) iResult |= XGE_DRAG_EFFECT_COPY;
	if ( (iEffect & DROPEFFECT_MOVE) != 0u ) iResult |= XGE_DRAG_EFFECT_MOVE;
	if ( (iEffect & DROPEFFECT_LINK) != 0u ) iResult |= XGE_DRAG_EFFECT_LINK;
	return iResult;
}

static DWORD __xgeWin32Effect(uint32_t iEffect)
{
	if ( iEffect == XGE_DRAG_EFFECT_COPY ) return DROPEFFECT_COPY;
	if ( iEffect == XGE_DRAG_EFFECT_MOVE ) return DROPEFFECT_MOVE;
	if ( iEffect == XGE_DRAG_EFFECT_LINK ) return DROPEFFECT_LINK;
	return DROPEFFECT_NONE;
}

static void __xgeWin32DropPoint(HWND hWnd, POINTL tScreen, float* pX, float* pY)
{
	POINT tPoint;
	RECT tClient;
	int iClientW;
	int iClientH;

	tPoint.x = tScreen.x;
	tPoint.y = tScreen.y;
	ScreenToClient(hWnd, &tPoint);
	GetClientRect(hWnd, &tClient);
	iClientW = tClient.right - tClient.left;
	iClientH = tClient.bottom - tClient.top;
	*pX = (iClientW > 0) ? (float)tPoint.x * (float)xgeGetWidth() /
		(float)iClientW : (float)tPoint.x;
	*pY = (iClientH > 0) ? (float)tPoint.y * (float)xgeGetHeight() /
		(float)iClientH : (float)tPoint.y;
}

static int __xgeWin32SetUnicodeText(xge_data_object pData, IDataObject* pObject)
{
	FORMATETC tFormat;
	STGMEDIUM tMedium;
	const wchar_t* sWide;
	char* sUtf8;
	int iSize;
	HRESULT iHr;

	memset(&tFormat, 0, sizeof(tFormat));
	tFormat.cfFormat = CF_UNICODETEXT;
	tFormat.dwAspect = DVASPECT_CONTENT;
	tFormat.lindex = -1;
	tFormat.tymed = TYMED_HGLOBAL;
	memset(&tMedium, 0, sizeof(tMedium));
	iHr = pObject->lpVtbl->GetData(pObject, &tFormat, &tMedium);
	if ( FAILED(iHr) ) return XGE_ERROR_NOT_FOUND;
	sWide = (const wchar_t*)GlobalLock(tMedium.hGlobal);
	if ( sWide == NULL ) {
		ReleaseStgMedium(&tMedium);
		return XGE_ERROR;
	}
	iSize = WideCharToMultiByte(CP_UTF8, 0, sWide, -1, NULL, 0, NULL, NULL);
	sUtf8 = (iSize > 0) ? (char*)xrtMalloc((size_t)iSize) : NULL;
	if ( sUtf8 == NULL || WideCharToMultiByte(CP_UTF8, 0, sWide, -1,
		sUtf8, iSize, NULL, NULL) <= 0 ) {
		if ( sUtf8 != NULL ) xrtFree(sUtf8);
		GlobalUnlock(tMedium.hGlobal);
		ReleaseStgMedium(&tMedium);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	GlobalUnlock(tMedium.hGlobal);
	ReleaseStgMedium(&tMedium);
	iHr = xgeDataObjectSet(pData, XGE_DATA_FORMAT_TEXT_UTF8,
		sUtf8, (size_t)iSize);
	xrtFree(sUtf8);
	return (int)iHr;
}

static int __xgeWin32SetFiles(xge_data_object pData, IDataObject* pObject)
{
	FORMATETC tFormat;
	STGMEDIUM tMedium;
	HDROP hDrop;
	wchar_t* sWide = NULL;
	char* pFiles = NULL;
	char* pUris = NULL;
	char* pWrite;
	int iWideLength;
	int iUtf8Length;
	UINT iCount;
	UINT i;
	size_t iFilesSize = 1u;
	size_t iUrisSize = 1u;
	HRESULT iHr;

	memset(&tFormat, 0, sizeof(tFormat));
	tFormat.cfFormat = CF_HDROP;
	tFormat.dwAspect = DVASPECT_CONTENT;
	tFormat.lindex = -1;
	tFormat.tymed = TYMED_HGLOBAL;
	memset(&tMedium, 0, sizeof(tMedium));
	iHr = pObject->lpVtbl->GetData(pObject, &tFormat, &tMedium);
	if ( FAILED(iHr) ) return XGE_ERROR_NOT_FOUND;
	hDrop = (HDROP)tMedium.hGlobal;
	iCount = DragQueryFileW(hDrop, 0xFFFFFFFFu, NULL, 0);
	for ( i = 0; i < iCount; i++ ) {
		wchar_t* pNewWide;
		iWideLength = (int)DragQueryFileW(hDrop, i, NULL, 0);
		pNewWide = (wchar_t*)xrtRealloc(sWide,
			((size_t)iWideLength + 1u) * sizeof(wchar_t));
		if ( pNewWide == NULL ) goto memory_error;
		sWide = pNewWide;
		DragQueryFileW(hDrop, i, sWide, (UINT)iWideLength + 1u);
		iUtf8Length = WideCharToMultiByte(CP_UTF8, 0, sWide, -1,
			NULL, 0, NULL, NULL);
		if ( iUtf8Length <= 0 ) continue;
		iFilesSize += (size_t)iUtf8Length;
		{
			char* sTemp = (char*)xrtMalloc((size_t)iUtf8Length);
			if ( sTemp == NULL ) goto memory_error;
			WideCharToMultiByte(CP_UTF8, 0, sWide, -1, sTemp,
				iUtf8Length, NULL, NULL);
			iUrisSize += __xgeDragFileUriSize(sTemp);
			xrtFree(sTemp);
		}
	}
	pFiles = (char*)xrtMalloc(iFilesSize);
	pUris = (char*)xrtMalloc(iUrisSize);
	if ( pFiles == NULL || pUris == NULL ) goto memory_error;
	pWrite = pFiles;
	{
		char* pUriWrite = pUris;
		for ( i = 0; i < iCount; i++ ) {
			wchar_t* pNewWide;
			iWideLength = (int)DragQueryFileW(hDrop, i, NULL, 0);
			pNewWide = (wchar_t*)xrtRealloc(sWide,
				((size_t)iWideLength + 1u) * sizeof(wchar_t));
			if ( pNewWide == NULL ) goto memory_error;
			sWide = pNewWide;
			DragQueryFileW(hDrop, i, sWide, (UINT)iWideLength + 1u);
			iUtf8Length = WideCharToMultiByte(CP_UTF8, 0, sWide, -1,
				pWrite, (int)(iFilesSize - (size_t)(pWrite - pFiles)),
				NULL, NULL);
			if ( iUtf8Length <= 0 ) continue;
			pUriWrite = __xgeDragFileUriWrite(pUriWrite, pWrite);
			pWrite += iUtf8Length;
		}
		*pWrite = '\0';
		*pUriWrite = '\0';
		(void)xgeDataObjectSet(pData, XGE_DATA_FORMAT_FILE_LIST,
			pFiles, (size_t)(pWrite - pFiles) + 1u);
		(void)xgeDataObjectSet(pData, XGE_DATA_FORMAT_URI_LIST,
			pUris, (size_t)(pUriWrite - pUris) + 1u);
	}
	if ( sWide != NULL ) xrtFree(sWide);
	xrtFree(pFiles);
	xrtFree(pUris);
	ReleaseStgMedium(&tMedium);
	return XGE_OK;

memory_error:
	if ( sWide != NULL ) xrtFree(sWide);
	if ( pFiles != NULL ) xrtFree(pFiles);
	if ( pUris != NULL ) xrtFree(pUris);
	ReleaseStgMedium(&tMedium);
	return XGE_ERROR_OUT_OF_MEMORY;
}

static int __xgeWin32SetHGlobal(xge_data_object pData, IDataObject* pObject,
	CLIPFORMAT iFormat, const char* sName)
{
	FORMATETC tFormat;
	STGMEDIUM tMedium;
	const void* pValue;
	SIZE_T iSize;
	int iRet;

	memset(&tFormat, 0, sizeof(tFormat));
	tFormat.cfFormat = iFormat;
	tFormat.dwAspect = DVASPECT_CONTENT;
	tFormat.lindex = -1;
	tFormat.tymed = TYMED_HGLOBAL;
	memset(&tMedium, 0, sizeof(tMedium));
	if ( FAILED(pObject->lpVtbl->GetData(pObject, &tFormat, &tMedium)) ) {
		return XGE_ERROR_NOT_FOUND;
	}
	pValue = GlobalLock(tMedium.hGlobal);
	iSize = GlobalSize(tMedium.hGlobal);
	if ( pValue == NULL ) {
		ReleaseStgMedium(&tMedium);
		return XGE_ERROR;
	}
	iRet = xgeDataObjectSet(pData, sName, pValue, (size_t)iSize);
	GlobalUnlock(tMedium.hGlobal);
	ReleaseStgMedium(&tMedium);
	return iRet;
}

static int __xgeWin32DataObject(xge_data_object* ppData, IDataObject* pObject)
{
	xge_data_object pData;
	IEnumFORMATETC* pEnum = NULL;
	FORMATETC tFormat;
	ULONG iFetched;
	char sName[256];
	UINT iHtml;

	if ( ppData == NULL || pObject == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*ppData = NULL;
	if ( xgeDataObjectCreate(&pData) != XGE_OK ) return XGE_ERROR_OUT_OF_MEMORY;
	(void)__xgeWin32SetUnicodeText(pData, pObject);
	(void)__xgeWin32SetFiles(pData, pObject);
	iHtml = RegisterClipboardFormatA("HTML Format");
	if ( iHtml != 0u ) (void)__xgeWin32SetHGlobal(pData, pObject,
		(CLIPFORMAT)iHtml, XGE_DATA_FORMAT_HTML);
	if ( SUCCEEDED(pObject->lpVtbl->EnumFormatEtc(pObject, DATADIR_GET, &pEnum)) &&
		pEnum != NULL ) {
		while ( pEnum->lpVtbl->Next(pEnum, 1, &tFormat, &iFetched) == S_OK ) {
			if ( tFormat.ptd != NULL ) CoTaskMemFree(tFormat.ptd);
			if ( tFormat.cfFormat == CF_UNICODETEXT ||
				tFormat.cfFormat == CF_HDROP || tFormat.cfFormat == iHtml ||
				(tFormat.tymed & TYMED_HGLOBAL) == 0u ) continue;
			if ( GetClipboardFormatNameA(tFormat.cfFormat, sName,
				(int)sizeof(sName)) <= 0 ) continue;
			(void)__xgeWin32SetHGlobal(pData, pObject,
				tFormat.cfFormat, sName);
		}
		pEnum->lpVtbl->Release(pEnum);
	}
	if ( xgeDataObjectFormatCount(pData) == 0 ) {
		xgeDataObjectRelease(pData);
		return XGE_ERROR_UNSUPPORTED;
	}
	*ppData = pData;
	return XGE_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DropQueryInterface(IDropTarget* pThis,
	REFIID pId, void** ppObject)
{
	if ( ppObject == NULL ) return E_POINTER;
	*ppObject = NULL;
	if ( IsEqualIID(pId, &IID_IUnknown) || IsEqualIID(pId, &IID_IDropTarget) ) {
		*ppObject = pThis;
		pThis->lpVtbl->AddRef(pThis);
		return S_OK;
	}
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE __xgeWin32DropAddRef(IDropTarget* pThis)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	return (ULONG)InterlockedIncrement(&pTarget->iRefs);
}

static ULONG STDMETHODCALLTYPE __xgeWin32DropRelease(IDropTarget* pThis)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	LONG iRefs = InterlockedDecrement(&pTarget->iRefs);
	if ( iRefs == 0 ) {
		if ( pTarget->pData != NULL ) xgeDataObjectRelease(pTarget->pData);
		xrtFree(pTarget);
	}
	return (ULONG)iRefs;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DropEnter(IDropTarget* pThis,
	IDataObject* pObject, DWORD iKeys, POINTL tPoint, DWORD* pEffect)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	uint32_t iAllowed;
	uint32_t iResult;
	float fX;
	float fY;

	if ( pEffect == NULL ) return E_INVALIDARG;
	if ( pTarget->pData != NULL ) xgeDataObjectRelease(pTarget->pData);
	pTarget->pData = NULL;
	if ( __xgeWin32DataObject(&pTarget->pData, pObject) != XGE_OK ) {
		*pEffect = DROPEFFECT_NONE;
		return S_OK;
	}
	iAllowed = __xgeWin32Effects(*pEffect);
	__xgeWin32DropPoint(pTarget->hWnd, tPoint, &fX, &fY);
	iResult = __xgeDragDispatchNative(XGE_DRAG_EVENT_ENTER, fX, fY,
		__xgeWin32Modifiers(iKeys), pTarget->pData, iAllowed,
		(iAllowed & XGE_DRAG_EFFECT_COPY) ? XGE_DRAG_EFFECT_COPY : 0u);
	*pEffect = __xgeWin32Effect(iResult);
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DropOver(IDropTarget* pThis,
	DWORD iKeys, POINTL tPoint, DWORD* pEffect)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	uint32_t iAllowed;
	uint32_t iResult;
	float fX;
	float fY;

	if ( pEffect == NULL ) return E_INVALIDARG;
	iAllowed = __xgeWin32Effects(*pEffect);
	__xgeWin32DropPoint(pTarget->hWnd, tPoint, &fX, &fY);
	iResult = __xgeDragDispatchNative(XGE_DRAG_EVENT_OVER, fX, fY,
		__xgeWin32Modifiers(iKeys), pTarget->pData, iAllowed,
		(iAllowed & XGE_DRAG_EFFECT_COPY) ? XGE_DRAG_EFFECT_COPY : 0u);
	*pEffect = __xgeWin32Effect(iResult);
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DropLeave(IDropTarget* pThis)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	(void)__xgeDragDispatchNative(XGE_DRAG_EVENT_LEAVE, g_xge.fMouseX,
		g_xge.fMouseY, 0u, pTarget->pData, 0u, 0u);
	if ( pTarget->pData != NULL ) xgeDataObjectRelease(pTarget->pData);
	pTarget->pData = NULL;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DropDrop(IDropTarget* pThis,
	IDataObject* pObject, DWORD iKeys, POINTL tPoint, DWORD* pEffect)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	uint32_t iAllowed;
	uint32_t iResult;
	float fX;
	float fY;

	(void)pObject;
	if ( pEffect == NULL ) return E_INVALIDARG;
	iAllowed = __xgeWin32Effects(*pEffect);
	__xgeWin32DropPoint(pTarget->hWnd, tPoint, &fX, &fY);
	iResult = __xgeDragDispatchNative(XGE_DRAG_EVENT_DROP, fX, fY,
		__xgeWin32Modifiers(iKeys), pTarget->pData, iAllowed,
		(iAllowed & XGE_DRAG_EFFECT_COPY) ? XGE_DRAG_EFFECT_COPY : 0u);
	*pEffect = __xgeWin32Effect(iResult);
	if ( pTarget->pData != NULL ) xgeDataObjectRelease(pTarget->pData);
	pTarget->pData = NULL;
	return S_OK;
}

static const IDropTargetVtbl g_xgeWin32DropTargetVtbl = {
	__xgeWin32DropQueryInterface,
	__xgeWin32DropAddRef,
	__xgeWin32DropRelease,
	__xgeWin32DropEnter,
	__xgeWin32DropOver,
	__xgeWin32DropLeave,
	__xgeWin32DropDrop
};

typedef struct xge_win32_data_object_t {
	IDataObject iface;
	LONG iRefs;
	xge_data_object pData;
	FORMATETC* pFormats;
	char** pNames;
	int iCount;
} xge_win32_data_object_t;

typedef struct xge_win32_drop_source_t {
	IDropSource iface;
	LONG iRefs;
} xge_win32_drop_source_t;

static HRESULT __xgeWin32MediumBytes(const void* pValue, size_t iSize,
	STGMEDIUM* pMedium)
{
	HGLOBAL hValue;
	void* pCopy;

	hValue = GlobalAlloc(GMEM_MOVEABLE, iSize > 0u ? iSize : 1u);
	if ( hValue == NULL ) return E_OUTOFMEMORY;
	pCopy = GlobalLock(hValue);
	if ( pCopy == NULL ) {
		GlobalFree(hValue);
		return E_OUTOFMEMORY;
	}
	if ( iSize > 0u ) memcpy(pCopy, pValue, iSize);
	GlobalUnlock(hValue);
	memset(pMedium, 0, sizeof(*pMedium));
	pMedium->tymed = TYMED_HGLOBAL;
	pMedium->hGlobal = hValue;
	return S_OK;
}

static HRESULT __xgeWin32DataGetRaw(xge_win32_data_object_t* pObject,
	const char* sFormat, STGMEDIUM* pMedium)
{
	void* pValue;
	size_t iSize;
	int iRet;

	iRet = xgeDataObjectGet(pObject->pData, sFormat, NULL, 0u, &iSize);
	if ( iRet != XGE_OK ) return DV_E_FORMATETC;
	pValue = xrtMalloc(iSize > 0u ? iSize : 1u);
	if ( pValue == NULL ) return E_OUTOFMEMORY;
	iRet = xgeDataObjectGet(pObject->pData, sFormat, pValue, iSize, &iSize);
	if ( iRet != XGE_OK ) {
		xrtFree(pValue);
		return E_FAIL;
	}
	iRet = (int)__xgeWin32MediumBytes(pValue, iSize, pMedium);
	xrtFree(pValue);
	return (HRESULT)iRet;
}

static HRESULT __xgeWin32DataGetText(xge_win32_data_object_t* pObject,
	STGMEDIUM* pMedium)
{
	char* sUtf8;
	wchar_t* sWide;
	size_t iSize;
	int iWideSize;
	int iRet;

	iRet = xgeDataObjectGet(pObject->pData, XGE_DATA_FORMAT_TEXT_UTF8,
		NULL, 0u, &iSize);
	if ( iRet != XGE_OK ) return DV_E_FORMATETC;
	sUtf8 = (char*)xrtMalloc(iSize + 1u);
	if ( sUtf8 == NULL ) return E_OUTOFMEMORY;
	iRet = xgeDataObjectGet(pObject->pData, XGE_DATA_FORMAT_TEXT_UTF8,
		sUtf8, iSize, &iSize);
	if ( iRet != XGE_OK ) {
		xrtFree(sUtf8);
		return E_FAIL;
	}
	sUtf8[iSize] = '\0';
	iWideSize = MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1, NULL, 0);
	sWide = (iWideSize > 0) ? (wchar_t*)xrtMalloc(
		(size_t)iWideSize * sizeof(wchar_t)) : NULL;
	if ( sWide == NULL || MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1,
		sWide, iWideSize) <= 0 ) {
		if ( sWide != NULL ) xrtFree(sWide);
		xrtFree(sUtf8);
		return E_OUTOFMEMORY;
	}
	iRet = (int)__xgeWin32MediumBytes(sWide,
		(size_t)iWideSize * sizeof(wchar_t), pMedium);
	xrtFree(sWide);
	xrtFree(sUtf8);
	return (HRESULT)iRet;
}

static HRESULT __xgeWin32DataGetFiles(xge_win32_data_object_t* pObject,
	STGMEDIUM* pMedium)
{
	DROPFILES* pDrop;
	char* pFiles;
	wchar_t* pWide;
	const char* pScan;
	size_t iSize;
	size_t iWideCount = 1u;
	size_t iOffset = 0u;
	int iChars;
	int iRet;
	HGLOBAL hValue;

	iRet = xgeDataObjectGet(pObject->pData, XGE_DATA_FORMAT_FILE_LIST,
		NULL, 0u, &iSize);
	if ( iRet != XGE_OK || iSize == 0u ) return DV_E_FORMATETC;
	pFiles = (char*)xrtMalloc(iSize + 1u);
	if ( pFiles == NULL ) return E_OUTOFMEMORY;
	if ( xgeDataObjectGet(pObject->pData, XGE_DATA_FORMAT_FILE_LIST,
		pFiles, iSize, &iSize) != XGE_OK ) {
		xrtFree(pFiles);
		return E_FAIL;
	}
	pFiles[iSize] = '\0';
	for ( pScan = pFiles; (size_t)(pScan - pFiles) < iSize && *pScan != '\0';
		pScan += strlen(pScan) + 1u ) {
		iChars = MultiByteToWideChar(CP_UTF8, 0, pScan, -1, NULL, 0);
		if ( iChars > 0 ) iWideCount += (size_t)iChars;
	}
	hValue = GlobalAlloc(GMEM_MOVEABLE,
		sizeof(DROPFILES) + iWideCount * sizeof(wchar_t));
	if ( hValue == NULL ) {
		xrtFree(pFiles);
		return E_OUTOFMEMORY;
	}
	pDrop = (DROPFILES*)GlobalLock(hValue);
	if ( pDrop == NULL ) {
		GlobalFree(hValue);
		xrtFree(pFiles);
		return E_OUTOFMEMORY;
	}
	memset(pDrop, 0, sizeof(*pDrop));
	pDrop->pFiles = sizeof(DROPFILES);
	pDrop->fWide = TRUE;
	pWide = (wchar_t*)((unsigned char*)pDrop + sizeof(DROPFILES));
	for ( pScan = pFiles; (size_t)(pScan - pFiles) < iSize && *pScan != '\0';
		pScan += strlen(pScan) + 1u ) {
		iChars = MultiByteToWideChar(CP_UTF8, 0, pScan, -1,
			pWide + iOffset, (int)(iWideCount - iOffset));
		if ( iChars > 0 ) iOffset += (size_t)iChars;
	}
	pWide[iOffset] = L'\0';
	GlobalUnlock(hValue);
	xrtFree(pFiles);
	memset(pMedium, 0, sizeof(*pMedium));
	pMedium->tymed = TYMED_HGLOBAL;
	pMedium->hGlobal = hValue;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataQueryInterface(IDataObject* pThis,
	REFIID pId, void** ppObject)
{
	if ( ppObject == NULL ) return E_POINTER;
	*ppObject = NULL;
	if ( IsEqualIID(pId, &IID_IUnknown) || IsEqualIID(pId, &IID_IDataObject) ) {
		*ppObject = pThis;
		pThis->lpVtbl->AddRef(pThis);
		return S_OK;
	}
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE __xgeWin32DataAddRef(IDataObject* pThis)
{
	xge_win32_data_object_t* pObject = (xge_win32_data_object_t*)pThis;
	return (ULONG)InterlockedIncrement(&pObject->iRefs);
}

static ULONG STDMETHODCALLTYPE __xgeWin32DataRelease(IDataObject* pThis)
{
	xge_win32_data_object_t* pObject = (xge_win32_data_object_t*)pThis;
	LONG iRefs = InterlockedDecrement(&pObject->iRefs);
	int i;

	if ( iRefs == 0 ) {
		for ( i = 0; i < pObject->iCount; i++ ) {
			if ( pObject->pNames[i] != NULL ) xrtFree(pObject->pNames[i]);
		}
		if ( pObject->pNames != NULL ) xrtFree(pObject->pNames);
		if ( pObject->pFormats != NULL ) xrtFree(pObject->pFormats);
		xgeDataObjectRelease(pObject->pData);
		xrtFree(pObject);
	}
	return (ULONG)iRefs;
}

static int __xgeWin32DataFormatIndex(xge_win32_data_object_t* pObject,
	CLIPFORMAT iFormat)
{
	int i;
	for ( i = 0; i < pObject->iCount; i++ ) {
		if ( pObject->pFormats[i].cfFormat == iFormat ) return i;
	}
	return -1;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataGetData(IDataObject* pThis,
	FORMATETC* pFormat, STGMEDIUM* pMedium)
{
	xge_win32_data_object_t* pObject = (xge_win32_data_object_t*)pThis;
	int iIndex;

	if ( pFormat == NULL || pMedium == NULL ||
		(pFormat->tymed & TYMED_HGLOBAL) == 0u ||
		pFormat->dwAspect != DVASPECT_CONTENT ) return DV_E_FORMATETC;
	iIndex = __xgeWin32DataFormatIndex(pObject, pFormat->cfFormat);
	if ( iIndex < 0 ) return DV_E_FORMATETC;
	if ( pFormat->cfFormat == CF_UNICODETEXT ) {
		return __xgeWin32DataGetText(pObject, pMedium);
	}
	if ( pFormat->cfFormat == CF_HDROP ) {
		return __xgeWin32DataGetFiles(pObject, pMedium);
	}
	return __xgeWin32DataGetRaw(pObject, pObject->pNames[iIndex], pMedium);
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataGetDataHere(IDataObject* pThis,
	FORMATETC* pFormat, STGMEDIUM* pMedium)
{
	(void)pThis; (void)pFormat; (void)pMedium;
	return DATA_E_FORMATETC;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataQueryGetData(IDataObject* pThis,
	FORMATETC* pFormat)
{
	xge_win32_data_object_t* pObject = (xge_win32_data_object_t*)pThis;
	if ( pFormat == NULL || (pFormat->tymed & TYMED_HGLOBAL) == 0u ||
		pFormat->dwAspect != DVASPECT_CONTENT ) return DV_E_FORMATETC;
	return __xgeWin32DataFormatIndex(pObject, pFormat->cfFormat) >= 0 ?
		S_OK : DV_E_FORMATETC;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataCanonical(IDataObject* pThis,
	FORMATETC* pInput, FORMATETC* pOutput)
{
	(void)pThis; (void)pInput;
	if ( pOutput != NULL ) pOutput->ptd = NULL;
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataSetData(IDataObject* pThis,
	FORMATETC* pFormat, STGMEDIUM* pMedium, BOOL bRelease)
{
	(void)pThis; (void)pFormat; (void)pMedium; (void)bRelease;
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataEnum(IDataObject* pThis,
	DWORD iDirection, IEnumFORMATETC** ppEnum)
{
	xge_win32_data_object_t* pObject = (xge_win32_data_object_t*)pThis;
	if ( ppEnum == NULL ) return E_POINTER;
	*ppEnum = NULL;
	if ( iDirection != DATADIR_GET ) return E_NOTIMPL;
	return SHCreateStdEnumFmtEtc((UINT)pObject->iCount,
		pObject->pFormats, ppEnum);
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataDAdvise(IDataObject* pThis,
	FORMATETC* pFormat, DWORD iFlags, IAdviseSink* pSink, DWORD* pConnection)
{
	(void)pThis; (void)pFormat; (void)iFlags; (void)pSink; (void)pConnection;
	return OLE_E_ADVISENOTSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataDUnadvise(IDataObject* pThis,
	DWORD iConnection)
{
	(void)pThis; (void)iConnection;
	return OLE_E_ADVISENOTSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DataEnumDAdvise(IDataObject* pThis,
	IEnumSTATDATA** ppEnum)
{
	(void)pThis;
	if ( ppEnum != NULL ) *ppEnum = NULL;
	return OLE_E_ADVISENOTSUPPORTED;
}

static const IDataObjectVtbl g_xgeWin32DataVtbl = {
	__xgeWin32DataQueryInterface,
	__xgeWin32DataAddRef,
	__xgeWin32DataRelease,
	__xgeWin32DataGetData,
	__xgeWin32DataGetDataHere,
	__xgeWin32DataQueryGetData,
	__xgeWin32DataCanonical,
	__xgeWin32DataSetData,
	__xgeWin32DataEnum,
	__xgeWin32DataDAdvise,
	__xgeWin32DataDUnadvise,
	__xgeWin32DataEnumDAdvise
};

static HRESULT STDMETHODCALLTYPE __xgeWin32SourceQueryInterface(IDropSource* pThis,
	REFIID pId, void** ppObject)
{
	if ( ppObject == NULL ) return E_POINTER;
	*ppObject = NULL;
	if ( IsEqualIID(pId, &IID_IUnknown) || IsEqualIID(pId, &IID_IDropSource) ) {
		*ppObject = pThis;
		pThis->lpVtbl->AddRef(pThis);
		return S_OK;
	}
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE __xgeWin32SourceAddRef(IDropSource* pThis)
{
	xge_win32_drop_source_t* pSource = (xge_win32_drop_source_t*)pThis;
	return (ULONG)InterlockedIncrement(&pSource->iRefs);
}

static ULONG STDMETHODCALLTYPE __xgeWin32SourceRelease(IDropSource* pThis)
{
	xge_win32_drop_source_t* pSource = (xge_win32_drop_source_t*)pThis;
	LONG iRefs = InterlockedDecrement(&pSource->iRefs);
	if ( iRefs == 0 ) xrtFree(pSource);
	return (ULONG)iRefs;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32SourceContinue(IDropSource* pThis,
	BOOL bEscape, DWORD iKeys)
{
	(void)pThis;
	if ( bEscape ) return DRAGDROP_S_CANCEL;
	if ( (iKeys & MK_LBUTTON) == 0u ) return DRAGDROP_S_DROP;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32SourceFeedback(IDropSource* pThis,
	DWORD iEffect)
{
	(void)pThis; (void)iEffect;
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

static const IDropSourceVtbl g_xgeWin32SourceVtbl = {
	__xgeWin32SourceQueryInterface,
	__xgeWin32SourceAddRef,
	__xgeWin32SourceRelease,
	__xgeWin32SourceContinue,
	__xgeWin32SourceFeedback
};

static CLIPFORMAT __xgeDragWin32ClipboardFormat(const char* sFormat)
{
	if ( strcmp(sFormat, XGE_DATA_FORMAT_TEXT_UTF8) == 0 ) return CF_UNICODETEXT;
	if ( strcmp(sFormat, XGE_DATA_FORMAT_FILE_LIST) == 0 ) return CF_HDROP;
	if ( strcmp(sFormat, XGE_DATA_FORMAT_HTML) == 0 ) {
		return (CLIPFORMAT)RegisterClipboardFormatA("HTML Format");
	}
	return (CLIPFORMAT)RegisterClipboardFormatA(sFormat);
}

static int __xgeWin32OutboundObjects(xge_win32_data_object_t** ppData,
	xge_win32_drop_source_t** ppSource)
{
	xge_win32_data_object_t* pData;
	xge_win32_drop_source_t* pSource;
	const char* sFormat;
	CLIPFORMAT iFormat;
	int iCount;
	int i;

	*ppData = NULL;
	*ppSource = NULL;
	iCount = xgeDataObjectFormatCount(g_xgeDragData);
	if ( iCount <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	pData = (xge_win32_data_object_t*)xrtCalloc(1u, sizeof(*pData));
	pSource = (xge_win32_drop_source_t*)xrtCalloc(1u, sizeof(*pSource));
	if ( pData == NULL || pSource == NULL ) goto memory_error;
	pData->pFormats = (FORMATETC*)xrtCalloc((size_t)iCount, sizeof(FORMATETC));
	pData->pNames = (char**)xrtCalloc((size_t)iCount, sizeof(char*));
	if ( pData->pFormats == NULL || pData->pNames == NULL ) goto memory_error;
	pData->iface.lpVtbl = (IDataObjectVtbl*)&g_xgeWin32DataVtbl;
	pData->iRefs = 1;
	pData->pData = g_xgeDragData;
	if ( xgeDataObjectAddRef(pData->pData) != XGE_OK ) goto memory_error;
	for ( i = 0; i < iCount; i++ ) {
		sFormat = xgeDataObjectFormatAt(g_xgeDragData, i);
		iFormat = __xgeDragWin32ClipboardFormat(sFormat);
		if ( iFormat == 0 ) continue;
		pData->pNames[pData->iCount] = __xgeDataStringCopy(sFormat);
		if ( pData->pNames[pData->iCount] == NULL ) goto memory_error;
		pData->pFormats[pData->iCount].cfFormat = iFormat;
		pData->pFormats[pData->iCount].dwAspect = DVASPECT_CONTENT;
		pData->pFormats[pData->iCount].lindex = -1;
		pData->pFormats[pData->iCount].tymed = TYMED_HGLOBAL;
		pData->iCount++;
	}
	if ( pData->iCount == 0 ) goto memory_error;
	pSource->iface.lpVtbl = (IDropSourceVtbl*)&g_xgeWin32SourceVtbl;
	pSource->iRefs = 1;
	*ppData = pData;
	*ppSource = pSource;
	return XGE_OK;

memory_error:
	if ( pData != NULL && pData->iface.lpVtbl != NULL ) {
		pData->iface.lpVtbl->Release(&pData->iface);
	} else if ( pData != NULL ) {
		if ( pData->pFormats != NULL ) xrtFree(pData->pFormats);
		if ( pData->pNames != NULL ) xrtFree(pData->pNames);
		xrtFree(pData);
	}
	if ( pSource != NULL ) xrtFree(pSource);
	return XGE_ERROR_OUT_OF_MEMORY;
}

static int __xgeDragDropPlatformBegin(void)
{
	xge_win32_data_object_t* pData;
	xge_win32_drop_source_t* pSource;
	DWORD iAllowed = DROPEFFECT_NONE;
	DWORD iEffect = DROPEFFECT_NONE;
	uint32_t iResult;
	HRESULT iHr;
	int iRet;

	if ( !g_xgeWin32OleInitialized || !g_xgeDragActive ) {
		return XGE_OK;
	}
	iRet = __xgeWin32OutboundObjects(&pData, &pSource);
	if ( iRet != XGE_OK ) {
		__xgeDragSessionClear();
		return iRet;
	}
	if ( (g_xgeDragAllowedEffects & XGE_DRAG_EFFECT_COPY) != 0u ) iAllowed |= DROPEFFECT_COPY;
	if ( (g_xgeDragAllowedEffects & XGE_DRAG_EFFECT_MOVE) != 0u ) iAllowed |= DROPEFFECT_MOVE;
	if ( (g_xgeDragAllowedEffects & XGE_DRAG_EFFECT_LINK) != 0u ) iAllowed |= DROPEFFECT_LINK;
	iHr = DoDragDrop(&pData->iface, &pSource->iface, iAllowed, &iEffect);
	iResult = __xgeWin32Effects(iEffect);
	if ( iHr == DRAGDROP_S_DROP ) {
		(void)__xgeDragDispatchNative(XGE_DRAG_EVENT_COMPLETE,
			g_xge.fMouseX, g_xge.fMouseY, 0u, g_xgeDragData,
			g_xgeDragAllowedEffects, iResult);
	} else {
		(void)__xgeDragDispatchNative(XGE_DRAG_EVENT_CANCEL,
			g_xge.fMouseX, g_xge.fMouseY, 0u, g_xgeDragData,
			g_xgeDragAllowedEffects, 0u);
	}
	pSource->iface.lpVtbl->Release(&pSource->iface);
	pData->iface.lpVtbl->Release(&pData->iface);
	__xgeDragSessionClear();
	return (iHr == DRAGDROP_S_DROP || iHr == DRAGDROP_S_CANCEL) ?
		XGE_OK : XGE_ERROR;
}

static void __xgeDragDropPlatformInit(void)
{
	xge_win32_drop_target_t* pTarget;
	HWND hWnd;
	HRESULT iHr;

	if ( g_xgeWin32DropTarget != NULL ) return;
	hWnd = (HWND)sapp_win32_get_hwnd();
	if ( hWnd == NULL ) return;
	iHr = OleInitialize(NULL);
	if ( SUCCEEDED(iHr) ) {
		g_xgeWin32OleInitialized = 1;
		g_xgeNativeDragOut = 1;
	}
	else if ( iHr != RPC_E_CHANGED_MODE ) return;
	pTarget = (xge_win32_drop_target_t*)xrtCalloc(1u, sizeof(*pTarget));
	if ( pTarget == NULL ) return;
	pTarget->iface.lpVtbl = (IDropTargetVtbl*)&g_xgeWin32DropTargetVtbl;
	pTarget->iRefs = 1;
	pTarget->hWnd = hWnd;
	iHr = RegisterDragDrop(hWnd, &pTarget->iface);
	if ( FAILED(iHr) ) {
		pTarget->iface.lpVtbl->Release(&pTarget->iface);
		return;
	}
	g_xgeWin32DropTarget = pTarget;
	g_xgeNativeDragIn = 1;
}

static void __xgeDragDropPlatformUnit(void)
{
	if ( g_xgeWin32DropTarget != NULL ) {
		(void)RevokeDragDrop(g_xgeWin32DropTarget->hWnd);
		g_xgeWin32DropTarget->iface.lpVtbl->Release(&g_xgeWin32DropTarget->iface);
		g_xgeWin32DropTarget = NULL;
	}
	if ( g_xgeWin32OleInitialized ) {
		OleUninitialize();
		g_xgeWin32OleInitialized = 0;
	}
	g_xgeNativeDragIn = 0;
	g_xgeNativeDragOut = 0;
}

#else

static void __xgeDragDropPlatformInit(void)
{
#if !defined(SOKOL_DUMMY_BACKEND)
	g_xgeNativeDragIn = 1;
#endif
}

static void __xgeDragDropPlatformUnit(void)
{
	g_xgeNativeDragIn = 0;
	g_xgeNativeDragOut = 0;
}

#endif
