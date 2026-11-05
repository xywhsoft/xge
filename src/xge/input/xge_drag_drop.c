#include "xge_data_object.c"

static xge_drag_event_proc g_xgeDragEventProc;
static xge_drag_event_checked_proc g_xgeDragEventChecked;
static void* g_xgeDragEventUser;
static xspinlock g_xgeDragCallbackSpin=XRT_SPIN_INIT;
static xge_callback_node_t* g_xgeDragEventOwner;
static int g_xgeDragActive;
static xge_data_object g_xgeDragData;
static uint32_t g_xgeDragAllowedEffects;
static uint32_t g_xgeDragSuggestedEffect;
static uint64_t g_xgeDragSequence;
static uint64_t g_xgeDragSessionId;
static uint64_t g_xgeDragSessionThread;
static uint64_t g_xgeDragCancellingSession;
static int g_xgeNativeDragIn;
static int g_xgeNativeDragOut;

static int __xgeDragSessionClear(void)
{
	xge_data_object data=g_xgeDragData;
	uint32_t allowed=g_xgeDragAllowedEffects,suggested=g_xgeDragSuggestedEffect;
	uint64_t thread=g_xgeDragSessionThread;
	g_xgeDragData = NULL;
	g_xgeDragAllowedEffects = 0u;
	g_xgeDragSuggestedEffect = 0u;
	g_xgeDragActive = 0;
	g_xgeDragSessionThread=0;
	if(!data)return XGE_OK;
	if(!xgeDataObjectIsOwned(data)){xgeDataObjectRelease(data);return XGE_OK;}
	bool consumed=false;int status=xgeDataObjectReleaseOwnerChecked(data,&consumed);
	if(!consumed){
		/* A preflight failure invoked no callback; preserve the actual global
		 * owner for a checked retry instead of losing its carrier. */
		g_xgeDragData=data;g_xgeDragAllowedEffects=allowed;g_xgeDragSuggestedEffect=suggested;
		g_xgeDragActive=1;g_xgeDragSessionThread=thread;
	}
	return status;
}

static int __xgeDragDropUnit(void)
{
	__xgeDragDropPlatformUnit();
	int result=__xgeDragSessionClear();
	g_xgeDragSequence = 0u;
	(void)xgeDragEventCallbackSetOwned(NULL,NULL,NULL);
	return result;
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
	return xgeDragEventCallbackSetOwned(onEvent,pUser,NULL);
}

static int __xgeDragEventCallbackSet(xge_drag_event_proc proc,xge_drag_event_checked_proc checked,void* user,const xge_callback_owner_t* owner)
{
	if((proc || checked) && xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))return XGE_ERROR_INVALID_STATE;
	xge_callback_node_t* next=NULL;int result=__xgeCallbackNodePrepare(proc!=NULL || checked!=NULL,false,owner,&next);
	if(result!=XGE_OK)return result;
	(void)xrtSpinLock(&g_xgeDragCallbackSpin);
	if((proc || checked) && xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE)){
		(void)xrtSpinUnlock(&g_xgeDragCallbackSpin);(void)__xgeCallbackNodeRelease(next);return XGE_ERROR_INVALID_STATE;
	}
	xge_callback_node_t* old=g_xgeDragEventOwner;g_xgeDragEventOwner=next;
	g_xgeDragEventProc=proc;g_xgeDragEventChecked=checked;g_xgeDragEventUser=(proc || checked) ? user:NULL;
	(void)xrtSpinUnlock(&g_xgeDragCallbackSpin);return __xgeCallbackNodeRelease(old);
}
int xgeDragEventCallbackSetOwned(xge_drag_event_proc proc,void* user,const xge_callback_owner_t* owner)
{return __xgeDragEventCallbackSet(proc,NULL,user,owner);}
int xgeDragEventCallbackSetCheckedOwned(xge_drag_event_checked_proc proc,void* user,const xge_callback_owner_t* owner)
{return __xgeDragEventCallbackSet(NULL,proc,user,owner);}

int xgeDragEventDispatchChecked(const xge_drag_event_t* pEvent,uint32_t* output)
{
	xge_drag_event_t tEvent;
	uint32_t iAllowed;
	uint32_t iEffect;
	uint32_t iMask;
	if(xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))return XGE_ERROR_INVALID_STATE;

	if ( output==NULL || pEvent == NULL || (pEvent->iSize != 0u &&
		pEvent->iSize < sizeof(*pEvent)) ||
		pEvent->iType < XGE_DRAG_EVENT_ENTER ||
		pEvent->iType > XGE_DRAG_EVENT_CANCEL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iMask = XGE_DRAG_EFFECT_COPY | XGE_DRAG_EFFECT_MOVE | XGE_DRAG_EFFECT_LINK;
	tEvent = *pEvent;
	xge_data_operation_t dataCall={0};
	if(tEvent.pData){int pin=__xgeDataOperationBegin(tEvent.pData,&dataCall);if(pin!=XGE_OK)return pin;}
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
	(void)xrtSpinLock(&g_xgeDragCallbackSpin);
	xge_drag_event_proc proc=g_xgeDragEventProc;xge_drag_event_checked_proc checked=g_xgeDragEventChecked;
	void* user=g_xgeDragEventUser;xge_callback_node_t* node=g_xgeDragEventOwner;
	if(!__xgeCallbackNodePin(node)){(void)xrtSpinUnlock(&g_xgeDragCallbackSpin);return dataCall.pObject ? __xgeDataOperationEnd(&dataCall,XGE_ERROR_INVALID_STATE):XGE_ERROR_INVALID_STATE;}
	(void)xrtSpinUnlock(&g_xgeDragCallbackSpin);
	xge_callback_snapshot_t snapshot={0};int result=__xgeCallbackSnapshotBegin(node,false,&snapshot);
	if(result!=XGE_OK)return dataCall.pObject ? __xgeDataOperationEnd(&dataCall,result):result;
	iEffect=XGE_DRAG_EFFECT_NONE;
	result=checked ? checked(&tEvent,&iEffect,user):XGE_OK;
	if(!checked && proc)iEffect=proc(&tEvent,user);
	if(result>0)result=XGE_ERROR_CALLBACK_FAILED;
	xerror* first=result!=XGE_OK ? xrtTakeError():NULL;
	__xgeCallbackOutcome(__xgeCallbackSnapshotEnd(&snapshot),&result,&first);xrtSetErrorTake(first);
	if(dataCall.pObject)result=__xgeDataOperationEnd(&dataCall,result);
	if(result!=XGE_OK)return result;
	if ( iEffect == 0u || (iEffect & ~iAllowed) != 0u ||
		(iEffect & (iEffect - 1u)) != 0u )iEffect=XGE_DRAG_EFFECT_NONE;
	*output=iEffect;return XGE_OK;
}

uint32_t xgeDragEventDispatch(const xge_drag_event_t* pEvent)
{
	/* Preserve the raw API's ordinary no-effect cases. Managed declarations
	 * use the checked entry to distinguish invalid input and lifecycle refusal. */
	if(!pEvent || (pEvent->iSize!=0u && pEvent->iSize<sizeof(*pEvent)) ||
		pEvent->iType<XGE_DRAG_EVENT_ENTER || pEvent->iType>XGE_DRAG_EVENT_CANCEL ||
		xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))return XGE_DRAG_EFFECT_NONE;
	uint32_t effect=XGE_DRAG_EFFECT_NONE;int result=xgeDragEventDispatchChecked(pEvent,&effect);
	if(result!=XGE_OK && !xrtGetError())xrtSetErrorKind(XERR_STATE);
	return result==XGE_OK ? effect:XGE_DRAG_EFFECT_NONE;
}

static int __xgeDragBegin(xge_data_object pData,uint32_t iAllowedEffects,uint32_t iSuggestedEffect,const xn_transfer_services_v1* services)
{
	uint32_t iMask;
	if(xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))return XGE_ERROR_INVALID_STATE;
	if(services){int status=__xgeTransferValidate(services);if(status!=XGE_OK)return status;}
	if(g_xgeNativeDragOut && xgeDataObjectIsOwned(pData) && !services)return XGE_ERROR_UNSUPPORTED;

	iMask = XGE_DRAG_EFFECT_COPY | XGE_DRAG_EFFECT_MOVE | XGE_DRAG_EFFECT_LINK;
	if ( !__xgeDataObjectValid(pData) || (iAllowedEffects & iMask) == 0u ||
		(iAllowedEffects & ~iMask) != 0u || (iSuggestedEffect & ~iAllowedEffects) != 0u ||
		(iSuggestedEffect != 0u && (iSuggestedEffect & (iSuggestedEffect - 1u)) != 0u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xgeDragActive ) return XGE_ERROR_INVALID_STATE;
	if(g_xgeDragSessionId==UINT64_MAX)return XGE_ERROR_INVALID_STATE;
	int acquired;
	if(xgeDataObjectIsOwned(pData)){xge_data_object shared=NULL;acquired=xgeDataObjectShareOwned(pData,&shared);}
	else acquired=xgeDataObjectAddRef(pData);
	if(acquired!=XGE_OK)return acquired;
	g_xgeDragData = pData;
	g_xgeDragAllowedEffects = iAllowedEffects;
	g_xgeDragSuggestedEffect = iSuggestedEffect;
	g_xgeDragActive = 1;
	g_xgeDragSessionThread=xrtThreadCurrentId();++g_xgeDragSessionId;
	if ( g_xgeNativeDragOut ) return __xgeDragDropPlatformBegin(services);
	return XGE_OK;
}
int xgeDragBegin(xge_data_object data,uint32_t allowed,uint32_t suggested)
{
	xerror* ambient=xrtTakeError();xn_transfer_services_v1 services;
	int status=__xgeDragBegin(data,allowed,suggested,__xgeTransferCurrent(&services) ? &services:NULL);
	xge_data_outcome_t outcome={0};__xgeDataOwnedOutcomeBegin(&outcome,status);return __xgeDataOwnedOutcomeEnd(&outcome,ambient);
}
int xgeDragBeginWithTransfer(xge_data_object data,uint32_t allowed,uint32_t suggested,const xn_transfer_services_v1* services)
{
	xerror* ambient=xrtTakeError();int status=__xgeTransferValidate(services);
	if(status==XGE_OK)status=xgeDataObjectIsOwned(data) ? __xgeDragBegin(data,allowed,suggested,services):XGE_ERROR_INVALID_ARGUMENT;
	xge_data_outcome_t outcome={0};__xgeDataOwnedOutcomeBegin(&outcome,status);return __xgeDataOwnedOutcomeEnd(&outcome,ambient);
}

int xgeDragCancel(void)
{
	xge_drag_event_t tEvent;

	if ( !g_xgeDragActive ) return XGE_OK;
	if(g_xgeDragSessionThread!=xrtThreadCurrentId())return XGE_ERROR_BUSY;
	uint64_t session=g_xgeDragSessionId;
	if(g_xgeDragCancellingSession==session)return XGE_OK;
	g_xgeDragCancellingSession=session;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XGE_DRAG_EVENT_CANCEL;
	tEvent.iAllowedEffects = g_xgeDragAllowedEffects;
	tEvent.iSuggestedEffect = g_xgeDragSuggestedEffect;
	tEvent.pData = g_xgeDragData;
	uint32_t ignored=XGE_DRAG_EFFECT_NONE;int status=xgeDragEventDispatchChecked(&tEvent,&ignored);
	if(g_xgeDragCancellingSession==session)g_xgeDragCancellingSession=0;
	xerror* first=status!=XGE_OK ? xrtTakeError():NULL;
	int cleanup=g_xgeDragSessionId==session ? __xgeDragSessionClear():XGE_OK;
	if(status!=XGE_OK){xrtClearError();xrtSetErrorTake(first);return status;}
	return cleanup;
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
	if ( pFiles == NULL || pUris == NULL || xgeDataObjectCreateOwned(&pData) != XGE_OK ) {
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
	uint64_t iCreator;
	uint64_t iRevision;
	HWND hWnd;
	xge_data_object pData;
} xge_win32_drop_target_t;

static xge_win32_drop_target_t* g_xgeWin32DropTarget;
static IDropTarget* g_xgeWin32RegisteredTarget;
static int g_xgeWin32OleInitialized;
static int __xgeWin32ReleaseDataReference(xge_data_object data,bool* consumed)
{
	if(!data){*consumed=true;return XGE_OK;}
	if(xgeDataObjectIsOwned(data))return xgeDataObjectReleaseOwnerChecked(data,consumed);
	xgeDataObjectRelease(data);*consumed=true;return XGE_OK;
}
static bool __xgeWin32TargetReady(const void* raw)
{const xge_win32_drop_target_t* target=raw;return target->iCreator==xrtThreadCurrentId();}
static bool __xgeWin32TargetTrace(const void* raw,xn_transfer_ownership_visitor visit,void* user)
{
	const xge_win32_drop_target_t* target=raw;
	if(!target->pData)return true;
	xrtownershipref ref=xgeDataObjectOwnership(target->pData);
	return ref.Data && visit(ref,user);
}
static int __xgeWin32TargetReplace(xge_win32_drop_target_t* target,xge_data_object next)
{
	if(target->iRevision==UINT64_MAX)return XGE_ERROR_INVALID_STATE;
	xrtownershipscope mutation={0};
	if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
	xge_data_object previous=target->pData;target->pData=next;++target->iRevision;
	if(!xrtOwnershipScopeEnd(&mutation))abort();
	bool consumed=false;int status=__xgeWin32ReleaseDataReference(previous,&consumed);
	/* All target entries execute on their creator, outside Freeze. Existing
	 * actual owner release consumes even when semantic cleanup reports error. */
	if(!consumed)abort();
	return status;
}
static int __xgeWin32TargetReleaseChecked(void* raw,bool* consumed)
{
	if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
	*consumed=false;xge_win32_drop_target_t* target=raw;
	if(!__xgeWin32TargetReady(target))return XGE_STATUS_DEFERRED;
	xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
	xge_data_object previous=target->pData;target->pData=NULL;
	if(!xrtOwnershipScopeEnd(&mutation))abort();
	bool released=false;int status=__xgeWin32ReleaseDataReference(previous,&released);
	if(!released){if(!xrtOwnershipMutationBegin(&mutation))abort();target->pData=previous;if(!xrtOwnershipScopeEnd(&mutation))abort();return status;}
	*consumed=true;xrtFree(target);return status;
}

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
	if ( xgeDataObjectCreateOwned(&pData) != XGE_OK ) return XGE_ERROR_OUT_OF_MEMORY;
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
		bool consumed=false;(void)__xgeWin32TargetReleaseChecked(pTarget,&consumed);
		if(!consumed){InterlockedIncrement(&pTarget->iRefs);return 1u;}
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
	if(g_xgeWin32DropTarget!=pTarget){*pEffect=DROPEFFECT_NONE;return S_OK;}
	xge_data_object incoming=NULL;
	int status=__xgeWin32DataObject(&incoming,pObject);
	if(status==XGE_OK && g_xgeWin32DropTarget!=pTarget)status=XGE_ERROR_INVALID_STATE;
	uint64_t expected=pTarget->iRevision==UINT64_MAX ? 0:pTarget->iRevision+1u;
	if(status==XGE_OK)status=__xgeWin32TargetReplace(pTarget,incoming);
	if ( status != XGE_OK ) {
		if(incoming && pTarget->pData!=incoming)xgeDataObjectRelease(incoming);
		*pEffect = DROPEFFECT_NONE;
		return S_OK;
	}
	if(pTarget->iRevision!=expected){*pEffect=DROPEFFECT_NONE;return S_OK;}
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
	if(g_xgeWin32DropTarget!=pTarget){*pEffect=DROPEFFECT_NONE;return S_OK;}
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
	if(g_xgeWin32DropTarget!=pTarget)return S_OK;
	uint64_t revision=pTarget->iRevision;
	(void)__xgeDragDispatchNative(XGE_DRAG_EVENT_LEAVE, g_xge.fMouseX,
		g_xge.fMouseY, 0u, pTarget->pData, 0u, 0u);
	if(pTarget->iRevision==revision)(void)__xgeWin32TargetReplace(pTarget,NULL);
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE __xgeWin32DropDrop(IDropTarget* pThis,
	IDataObject* pObject, DWORD iKeys, POINTL tPoint, DWORD* pEffect)
{
	xge_win32_drop_target_t* pTarget = (xge_win32_drop_target_t*)pThis;
	uint64_t revision=pTarget->iRevision;
	uint32_t iAllowed;
	uint32_t iResult;
	float fX;
	float fY;

	(void)pObject;
	if ( pEffect == NULL ) return E_INVALIDARG;
	if(g_xgeWin32DropTarget!=pTarget){*pEffect=DROPEFFECT_NONE;return S_OK;}
	iAllowed = __xgeWin32Effects(*pEffect);
	__xgeWin32DropPoint(pTarget->hWnd, tPoint, &fX, &fY);
	iResult = __xgeDragDispatchNative(XGE_DRAG_EVENT_DROP, fX, fY,
		__xgeWin32Modifiers(iKeys), pTarget->pData, iAllowed,
		(iAllowed & XGE_DRAG_EFFECT_COPY) ? XGE_DRAG_EFFECT_COPY : 0u);
	*pEffect = __xgeWin32Effect(iResult);
	if(pTarget->iRevision==revision)(void)__xgeWin32TargetReplace(pTarget,NULL);
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
	uint64_t iCreator;
	xge_data_object pData;
	FORMATETC* pFormats;
	char** pNames;
	int iCount;
	HGLOBAL* pPrepared; /* Immutable clipboard-only representation handles. */
	xge_callback_run_outcome_t* pFailure; /* Borrowed only within DoDragDrop. */
} xge_win32_data_object_t;

typedef struct xge_win32_drop_source_t {
	IDropSource iface;
	LONG iRefs;
	uint64_t iCreator;
} xge_win32_drop_source_t;
static bool __xgeWin32DataReady(const void* raw)
{const xge_win32_data_object_t* data=raw;return data->iCreator==xrtThreadCurrentId();}
static bool __xgeWin32DataTrace(const void* raw,xn_transfer_ownership_visitor visit,void* user)
{
	const xge_win32_data_object_t* data=raw;if(!data->pData)return true;
	xrtownershipref ref=xgeDataObjectOwnership(data->pData);return ref.Data && visit(ref,user);
}
static int __xgeWin32DataReleaseChecked(void* raw,bool* consumed)
{
	if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
	*consumed=false;xge_win32_data_object_t* data=raw;
	if(!__xgeWin32DataReady(data))return XGE_STATUS_DEFERRED;
	xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
	xge_data_object previous=data->pData;data->pData=NULL;
	if(!xrtOwnershipScopeEnd(&mutation))abort();
	bool released=false;int status=__xgeWin32ReleaseDataReference(previous,&released);
	if(!released){if(!xrtOwnershipMutationBegin(&mutation))abort();data->pData=previous;if(!xrtOwnershipScopeEnd(&mutation))abort();return status;}
	for(int i=0;i<data->iCount;++i){if(data->pNames && data->pNames[i])xrtFree(data->pNames[i]);if(data->pPrepared && data->pPrepared[i])GlobalFree(data->pPrepared[i]);}
	if(!data->pPrepared)xrtFree(data->pFormats);
	xrtFree(data->pNames);*consumed=true;xrtFree(data);return status;
}
static bool __xgeWin32SourceReady(const void* raw)
{const xge_win32_drop_source_t* source=raw;return source->iCreator==xrtThreadCurrentId();}
static bool __xgeWin32SourceTrace(const void* raw,xn_transfer_ownership_visitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static int __xgeWin32SourceReleaseChecked(void* raw,bool* consumed)
{
	if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
	*consumed=false;if(!__xgeWin32SourceReady(raw))return XGE_STATUS_DEFERRED;
	*consumed=true;xrtFree(raw);return XGE_OK;
}

static HGLOBAL __xgeWin32ClipboardHtml(const void*,size_t);
static HRESULT __xgeWin32DataFailure(xge_win32_data_object_t* object,int status,HRESULT result)
{
	if(!__xgeWin32DataReady(object))return RPC_E_WRONG_THREAD;
	__xgeCallbackRunRemember(object->pFailure,status<0 ? status:XGE_ERROR_CALLBACK_FAILED);return result;
}
static HRESULT __xgeWin32MediumBytes(const void* pValue, size_t iSize,
	STGMEDIUM* pMedium)
{
	HGLOBAL hValue;
	void* pCopy;

	hValue = GlobalAlloc(GMEM_MOVEABLE, iSize);
	if ( hValue == NULL ) return E_OUTOFMEMORY;
	if(iSize==0u){memset(pMedium,0,sizeof(*pMedium));pMedium->tymed=TYMED_HGLOBAL;pMedium->hGlobal=hValue;return S_OK;}
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
	if ( iRet != XGE_OK ) return __xgeWin32DataFailure(pObject,iRet,DV_E_FORMATETC);
	pValue = xrtMalloc(iSize > 0u ? iSize : 1u);
	if ( pValue == NULL ) return __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
	iRet = xgeDataObjectGet(pObject->pData, sFormat, pValue, iSize, &iSize);
	if ( iRet != XGE_OK ) {
		xrtFree(pValue);
		return __xgeWin32DataFailure(pObject,iRet,E_FAIL);
	}
	iRet = (int)__xgeWin32MediumBytes(pValue, iSize, pMedium);
	xrtFree(pValue);
	return FAILED((HRESULT)iRet) ? __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,(HRESULT)iRet):(HRESULT)iRet;
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
	if ( iRet != XGE_OK ) return __xgeWin32DataFailure(pObject,iRet,DV_E_FORMATETC);
	if(iSize>(size_t)INT_MAX-1u)return __xgeWin32DataFailure(pObject,XGE_ERROR_INVALID_ARGUMENT,DV_E_FORMATETC);
	sUtf8 = (char*)xrtMalloc(iSize + 1u);
	if ( sUtf8 == NULL ) return __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
	iRet = xgeDataObjectGet(pObject->pData, XGE_DATA_FORMAT_TEXT_UTF8,
		sUtf8, iSize, &iSize);
	if ( iRet != XGE_OK ) {
		xrtFree(sUtf8);
		return __xgeWin32DataFailure(pObject,iRet,E_FAIL);
	}
	if(iSize && memchr(sUtf8,0,iSize)){xrtFree(sUtf8);return __xgeWin32DataFailure(pObject,XGE_ERROR_INVALID_ARGUMENT,DV_E_FORMATETC);}
	sUtf8[iSize] = '\0';
	iWideSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, sUtf8, -1, NULL, 0);
	if(iWideSize<=0){xrtFree(sUtf8);return __xgeWin32DataFailure(pObject,XGE_ERROR_INVALID_ARGUMENT,DV_E_FORMATETC);}
	sWide = (iWideSize > 0) ? (wchar_t*)xrtMalloc(
		(size_t)iWideSize * sizeof(wchar_t)) : NULL;
	if ( sWide == NULL || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, sUtf8, -1,
		sWide, iWideSize) <= 0 ) {
		if ( sWide != NULL ) xrtFree(sWide);
		xrtFree(sUtf8);
		return __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
	}
	iRet = (int)__xgeWin32MediumBytes(sWide,
		(size_t)iWideSize * sizeof(wchar_t), pMedium);
	xrtFree(sWide);
	xrtFree(sUtf8);
	return FAILED((HRESULT)iRet) ? __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,(HRESULT)iRet):(HRESULT)iRet;
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
	if ( iRet != XGE_OK || iSize == 0u || iSize>(size_t)INT_MAX-1u ) return __xgeWin32DataFailure(pObject,iRet!=XGE_OK ? iRet:XGE_ERROR_INVALID_ARGUMENT,DV_E_FORMATETC);
	pFiles = (char*)xrtMalloc(iSize + 1u);
	if ( pFiles == NULL ) return __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
	if ( (iRet=xgeDataObjectGet(pObject->pData, XGE_DATA_FORMAT_FILE_LIST,
		pFiles, iSize, &iSize)) != XGE_OK ) {
		xrtFree(pFiles);
		return __xgeWin32DataFailure(pObject,iRet,E_FAIL);
	}
	pFiles[iSize] = '\0';
	for ( pScan = pFiles; (size_t)(pScan - pFiles) < iSize && *pScan != '\0';
		pScan += strlen(pScan) + 1u ) {
		iChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pScan, -1, NULL, 0);
		if(iChars<=0 || iWideCount>(size_t)INT_MAX-(size_t)iChars){xrtFree(pFiles);return __xgeWin32DataFailure(pObject,XGE_ERROR_INVALID_ARGUMENT,DV_E_FORMATETC);}
		iWideCount += (size_t)iChars;
	}
	hValue = GlobalAlloc(GMEM_MOVEABLE,
		sizeof(DROPFILES) + iWideCount * sizeof(wchar_t));
	if ( hValue == NULL ) {
		xrtFree(pFiles);
		return __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
	}
	pDrop = (DROPFILES*)GlobalLock(hValue);
	if ( pDrop == NULL ) {
		GlobalFree(hValue);
		xrtFree(pFiles);
		return __xgeWin32DataFailure(pObject,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
	}
	memset(pDrop, 0, sizeof(*pDrop));
	pDrop->pFiles = sizeof(DROPFILES);
	pDrop->fWide = TRUE;
	pWide = (wchar_t*)((unsigned char*)pDrop + sizeof(DROPFILES));
	for ( pScan = pFiles; (size_t)(pScan - pFiles) < iSize && *pScan != '\0';
		pScan += strlen(pScan) + 1u ) {
		iChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pScan, -1,
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

static HRESULT __xgeWin32DataGetHtml(xge_win32_data_object_t* object,STGMEDIUM* medium)
{
    size_t required=0,written=0;int status=xgeDataObjectGet(object->pData,XGE_DATA_FORMAT_HTML,NULL,0,&required);
    if(status!=XGE_OK)return __xgeWin32DataFailure(object,status,E_FAIL);
    void* bytes=xrtMalloc(required ? required:1u);
    if(!bytes)return __xgeWin32DataFailure(object,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
    status=xgeDataObjectGet(object->pData,XGE_DATA_FORMAT_HTML,bytes,required,&written);
    if(status!=XGE_OK || written!=required){xrtFree(bytes);return __xgeWin32DataFailure(object,status!=XGE_OK ? status:XGE_ERROR_INVALID_STATE,E_FAIL);}
    HGLOBAL memory=__xgeWin32ClipboardHtml(bytes,written);xrtFree(bytes);
    if(!memory)return __xgeWin32DataFailure(object,XGE_ERROR_OUT_OF_MEMORY,E_OUTOFMEMORY);
    memset(medium,0,sizeof(*medium));medium->tymed=TYMED_HGLOBAL;medium->hGlobal=memory;return S_OK;
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
    xge_win32_data_object_t* data=(xge_win32_data_object_t*)pThis;
    LONG refs=InterlockedDecrement(&data->iRefs);
    if(refs==0){bool consumed=false;(void)__xgeWin32DataReleaseChecked(data,&consumed);
        if(!consumed){InterlockedIncrement(&data->iRefs);return 1;}}
    return (ULONG)refs;
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
	if(pMedium)memset(pMedium,0,sizeof(*pMedium));
	if(!__xgeWin32DataReady(pObject))return RPC_E_WRONG_THREAD;

	if ( pFormat == NULL || pMedium == NULL ||
		(pFormat->tymed & TYMED_HGLOBAL) == 0u ||
		pFormat->dwAspect != DVASPECT_CONTENT ) return DV_E_FORMATETC;
	if(pFormat->lindex!=-1)return DV_E_LINDEX;
	if(pFormat->ptd)return DV_E_DVTARGETDEVICE;
	iIndex = __xgeWin32DataFormatIndex(pObject, pFormat->cfFormat);
	if ( iIndex < 0 ) return DV_E_FORMATETC;
    if(pObject->pPrepared){
        HGLOBAL original=pObject->pPrepared[iIndex];
        HGLOBAL copy=GlobalSize(original)==0u ? GlobalAlloc(GMEM_MOVEABLE,0):(HGLOBAL)OleDuplicateData(original,pFormat->cfFormat,0);
        if(!copy)return E_OUTOFMEMORY;
        memset(pMedium,0,sizeof(*pMedium));pMedium->tymed=TYMED_HGLOBAL;pMedium->hGlobal=copy;
        return S_OK;
    }
	if ( pFormat->cfFormat == CF_UNICODETEXT ) {
		return __xgeWin32DataGetText(pObject, pMedium);
	}
	if ( pFormat->cfFormat == CF_HDROP ) {
		return __xgeWin32DataGetFiles(pObject, pMedium);
	}
	if(strcmp(pObject->pNames[iIndex],XGE_DATA_FORMAT_HTML)==0)return __xgeWin32DataGetHtml(pObject,pMedium);
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
	if(pFormat->lindex!=-1)return DV_E_LINDEX;
	if(pFormat->ptd)return DV_E_DVTARGETDEVICE;
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
    xge_win32_drop_source_t* source=(xge_win32_drop_source_t*)pThis;
    LONG refs=InterlockedDecrement(&source->iRefs);
    if(refs==0){bool consumed=false;(void)__xgeWin32SourceReleaseChecked(source,&consumed);
        if(!consumed){InterlockedIncrement(&source->iRefs);return 1;}}
    return (ULONG)refs;
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

static int __xgeWin32OutboundObjects(xge_data_object original,xge_win32_data_object_t** ppData,
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
	iCount = xgeDataObjectFormatCount(original);
	if ( iCount <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	pData = (xge_win32_data_object_t*)xrtCalloc(1u, sizeof(*pData));
	pSource = (xge_win32_drop_source_t*)xrtCalloc(1u, sizeof(*pSource));
	if ( pData == NULL || pSource == NULL ) goto memory_error;
	pData->pFormats = (FORMATETC*)xrtCalloc((size_t)iCount, sizeof(FORMATETC));
	pData->pNames = (char**)xrtCalloc((size_t)iCount, sizeof(char*));
	if ( pData->pFormats == NULL || pData->pNames == NULL ) goto memory_error;
	pData->iface.lpVtbl = (IDataObjectVtbl*)&g_xgeWin32DataVtbl;
	pData->iRefs = 1;
	pData->iCreator=xrtThreadCurrentId();
	if ( xgeDataObjectAddRef(original) != XGE_OK ) goto memory_error;
	pData->pData = original;
	for ( i = 0; i < iCount; i++ ) {
		sFormat = xgeDataObjectFormatAt(original, i);
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
	pSource->iCreator=xrtThreadCurrentId();
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

static xn_transfer_native_owner_v1 __xgeWin32DataOwner(const xge_win32_data_object_t* data)
{xn_transfer_native_owner_v1 owner={sizeof(owner),XN_TRANSFER_NATIVE_OWNER_V1_TAG,data->iCreator,__xgeWin32DataReady,__xgeWin32DataTrace,__xgeWin32DataReleaseChecked};return owner;}
static xn_transfer_native_owner_v1 __xgeWin32SourceOwner(const xge_win32_drop_source_t* source)
{xn_transfer_native_owner_v1 owner={sizeof(owner),XN_TRANSFER_NATIVE_OWNER_V1_TAG,source->iCreator,__xgeWin32SourceReady,__xgeWin32SourceTrace,__xgeWin32SourceReleaseChecked};return owner;}
static xn_transfer_native_owner_v1 __xgeWin32TargetOwner(const xge_win32_drop_target_t* target)
{xn_transfer_native_owner_v1 owner={sizeof(owner),XN_TRANSFER_NATIVE_OWNER_V1_TAG,target->iCreator,__xgeWin32TargetReady,__xgeWin32TargetTrace,__xgeWin32TargetReleaseChecked};return owner;}

static int __xgeDragDropPlatformBegin(const xn_transfer_services_v1* services)
{
    if(!g_xgeWin32OleInitialized || !g_xgeDragActive)return XGE_OK;
    uint64_t session=g_xgeDragSessionId;
    uint32_t allowed=g_xgeDragAllowedEffects;
    xge_win32_data_object_t* nativeData=NULL;xge_win32_drop_source_t* nativeSource=NULL;
    IDataObject* data=NULL;IDropSource* source=NULL;
    int status=__xgeWin32OutboundObjects(g_xgeDragData,&nativeData,&nativeSource);
    if(status==XGE_OK){data=&nativeData->iface;source=&nativeSource->iface;}
    if(status==XGE_OK && services){
        xn_transfer_native_owner_v1 owner=__xgeWin32DataOwner(nativeData);void* wrapped=NULL;
        int wrappedStatus=services->wrapDataObject(services,data,&owner,&wrapped);
        if(wrappedStatus==0 && wrapped)data=(IDataObject*)wrapped;
        if(wrappedStatus!=0 || !wrapped || xrtGetError())status=XGE_ERROR_BACKEND_FAILED;
        if(status==XGE_OK){owner=__xgeWin32SourceOwner(nativeSource);wrapped=NULL;
            wrappedStatus=services->wrapDropSource(services,source,&owner,&wrapped);
            if(wrappedStatus==0 && wrapped)source=(IDropSource*)wrapped;
            if(wrappedStatus!=0 || !wrapped || xrtGetError())status=XGE_ERROR_BACKEND_FAILED;}
    }
    if(status==XGE_OK && (!g_xgeDragActive || g_xgeDragSessionId!=session))status=XGE_ERROR_INVALID_STATE;
    if(status==XGE_OK && !__xgeWin32DataReady(nativeData))status=XGE_ERROR_BUSY;
    xge_callback_run_outcome_t callFailure={0};
    DWORD effect=DROPEFFECT_NONE;HRESULT hr=E_FAIL;
    if(status==XGE_OK){
        DWORD effects=0;if(allowed&XGE_DRAG_EFFECT_COPY)effects|=DROPEFFECT_COPY;
        if(allowed&XGE_DRAG_EFFECT_MOVE)effects|=DROPEFFECT_MOVE;
        if(allowed&XGE_DRAG_EFFECT_LINK)effects|=DROPEFFECT_LINK;
        nativeData->pFailure=&callFailure;
        hr=DoDragDrop(data,source,effects,&effect);
        nativeData->pFailure=NULL;
        if(hr!=DRAGDROP_S_DROP && hr!=DRAGDROP_S_CANCEL)status=XGE_ERROR_BACKEND_FAILED;
    }
    if(callFailure.iFirstStatus){status=__xgeCallbackRunTake(&callFailure);}
    xerror* first=status!=XGE_OK ? xrtTakeError():NULL;int outcome=status;
    if(status==XGE_OK && g_xgeDragActive && g_xgeDragSessionId==session){
        xge_drag_event_t event={0};event.iSize=sizeof(event);
        event.iType=hr==DRAGDROP_S_DROP ? XGE_DRAG_EVENT_COMPLETE:XGE_DRAG_EVENT_CANCEL;
        event.pData=nativeData->pData;event.iAllowedEffects=allowed;
        event.iSuggestedEffect=__xgeWin32Effects(effect);event.fX=g_xge.fMouseX;event.fY=g_xge.fMouseY;
        uint32_t ignored=0;__xgeCallbackOutcome(xgeDragEventDispatchChecked(&event,&ignored),&outcome,&first);
    }
    if(source){source->lpVtbl->Release(source);__xgeCallbackOutcome(XGE_OK,&outcome,&first);}
    if(data){data->lpVtbl->Release(data);__xgeCallbackOutcome(XGE_OK,&outcome,&first);}
    if(g_xgeDragActive && g_xgeDragSessionId==session)__xgeCallbackOutcome(__xgeDragSessionClear(),&outcome,&first);
    xrtSetErrorTake(first);return outcome;
}

static void __xgeDragDropPlatformInit(void)
{
    if(g_xgeWin32RegisteredTarget)return;
    HWND window=(HWND)sapp_win32_get_hwnd();if(!window)return;
    HRESULT hr=OleInitialize(NULL);
    if(SUCCEEDED(hr)){g_xgeWin32OleInitialized=1;g_xgeNativeDragOut=1;}
    else if(hr!=RPC_E_CHANGED_MODE)return;
    xge_win32_drop_target_t* target=xrtCalloc(1,sizeof(*target));if(!target)return;
    target->iface.lpVtbl=(IDropTargetVtbl*)&g_xgeWin32DropTargetVtbl;
    target->iRefs=1;target->iCreator=xrtThreadCurrentId();target->hWnd=window;
    IDropTarget* registered=&target->iface;xn_transfer_services_v1 services;
    if(__xgeTransferCurrent(&services)){
        xn_transfer_native_owner_v1 owner=__xgeWin32TargetOwner(target);void* wrapped=NULL;
        int wrappedStatus=services.wrapDropTarget(&services,registered,&owner,&wrapped);
        if(wrappedStatus==0 && wrapped)registered=(IDropTarget*)wrapped;
        if(wrappedStatus!=0 || !wrapped || xrtGetError()){
            xerror* first=xrtTakeError();registered->lpVtbl->Release(registered);xrtClearError();xrtSetErrorTake(first);return;}
    }
    hr=RegisterDragDrop(window,registered);
    if(FAILED(hr)){registered->lpVtbl->Release(registered);return;}
    g_xgeWin32DropTarget=target;g_xgeWin32RegisteredTarget=registered;g_xgeNativeDragIn=1;
}

static void __xgeDragDropPlatformUnit(void)
{
    IDropTarget* registered=g_xgeWin32RegisteredTarget;
    HWND window=g_xgeWin32DropTarget ? g_xgeWin32DropTarget->hWnd:NULL;
    g_xgeWin32RegisteredTarget=NULL;g_xgeWin32DropTarget=NULL;
    if(registered){if(window)(void)RevokeDragDrop(window);registered->lpVtbl->Release(registered);}
    if(g_xgeWin32OleInitialized){OleUninitialize();g_xgeWin32OleInitialized=0;}
    g_xgeNativeDragIn=0;g_xgeNativeDragOut=0;
}

#else

static int __xgeDragDropPlatformBegin(const xn_transfer_services_v1* services)
{
	(void)services;return XGE_ERROR_UNSUPPORTED;
}

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
