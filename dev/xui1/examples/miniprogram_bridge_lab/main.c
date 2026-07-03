#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct miniprogram_bridge_lab_t {
	int iFrameLimit;
	int iFrameCallbacks;
	int iFrameCallsFull;
	int iFrameCallsSimple;
	int iRequestFrameCalls;
	int iAudioCommandCalls;
	int iAudioCommandSum;
	int iLoadCalls;
	int iFreeCalls;
	int bBridgeBeforeInitOK;
	int bInitDescOK;
	int bBackendOK;
	int bCapsOK;
	int bResourceBeforeOK;
	int bTouchArrayOK;
	int bTouchOneOK;
	int bTextOK;
	int bResizeOK;
	int bRequestOK;
	int bAudioOK;
	int bFrameOK;
	int bUnitOK;
	int bInitSimpleOK;
	int bBridgeAfterInitOK;
	int bResourceAfterOK;
	int bRequestAfterOK;
	int bFrameAfterOK;
	int bUnitAfterOK;
	char sLastResource[64];
} miniprogram_bridge_lab_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int CheckResourceText(const xge_resource_t* pResource, const char* sExpected)
{
	size_t iExpected;

	if ( (pResource == NULL) || (pResource->pData == NULL) || (sExpected == NULL) ) {
		return 0;
	}
	iExpected = strlen(sExpected);
	if ( pResource->iSize != (int)iExpected ) {
		return 0;
	}
	return (memcmp(pResource->pData, sExpected, iExpected) == 0) ? 1 : 0;
}

static int HostRequestFrame(void* pUser)
{
	miniprogram_bridge_lab_t* pLab;

	pLab = (miniprogram_bridge_lab_t*)pUser;
	pLab->iRequestFrameCalls++;
	return XGE_OK;
}

static int HostLoadResource(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	miniprogram_bridge_lab_t* pLab;
	const char* sText;
	size_t iLen;
	char* sCopy;

	pLab = (miniprogram_bridge_lab_t*)pUser;
	if ( (sURI == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( strcmp(sURI, "res://bridge-note.txt") == 0 ) {
		sText = "bridge-before-init";
	} else if ( strcmp(sURI, "res://bridge-after.txt") == 0 ) {
		sText = "bridge-after-init";
	} else {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	iLen = strlen(sText);
	sCopy = (char*)malloc(iLen);
	if ( sCopy == NULL ) {
		return XGE_ERROR_BACKEND_FAILED;
	}
	memcpy(sCopy, sText, iLen);
	*ppData = sCopy;
	*pSize = (int)iLen;
	pLab->iLoadCalls++;
	snprintf(pLab->sLastResource, (int)sizeof(pLab->sLastResource), "%s", sURI);
	return XGE_OK;
}

static void HostFreeResource(void* pData, void* pUser)
{
	miniprogram_bridge_lab_t* pLab;

	pLab = (miniprogram_bridge_lab_t*)pUser;
	if ( pData != NULL ) {
		free(pData);
	}
	pLab->iFreeCalls++;
}

static int HostAudioCommand(int iCommand, int iHandle, const void* pData, int iSize, void* pUser)
{
	miniprogram_bridge_lab_t* pLab;

	(void)pData;
	pLab = (miniprogram_bridge_lab_t*)pUser;
	pLab->iAudioCommandCalls++;
	pLab->iAudioCommandSum = iCommand + iHandle + iSize;
	return XGE_OK;
}

static int MiniProgramFrameProc(void* pUser)
{
	miniprogram_bridge_lab_t* pLab;

	pLab = (miniprogram_bridge_lab_t*)pUser;
	pLab->iFrameCallbacks++;
	xgeClear(XGE_COLOR_RGBA(18, 24, 36, 255));
	xgeShapeRectFill((xge_rect_t){ 8.0f, 8.0f, 48.0f, 24.0f }, XGE_COLOR_RGBA(64, 128, 220, 255));
	return 0;
}

static int StartManualRuntime(int iWidth, int iHeight)
{
	xge_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = iWidth;
	tDesc.iHeight = iHeight;
	tDesc.sTitle = "XGE MiniProgram Bridge Lab";
	tDesc.iFlags = 0;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	tDesc.iTargetFPS = 60;
	return xgeInit(&tDesc);
}

static void PrintSummary(const miniprogram_bridge_lab_t* pLab)
{
	printf("miniprogram-bridge-lab final-summary desc=%d/%d/%d/%d before=%d/%d/%d/%d/%d/%d/%d frames=%d/%d simple=%d/%d/%d/%d/%d/%d callbacks=%d bridge(load=%d free=%d request=%d audio=%d sum=%d last=%s)\n",
		pLab->bBridgeBeforeInitOK,
		pLab->bInitDescOK,
		pLab->bBackendOK,
		pLab->bCapsOK,
		pLab->bResourceBeforeOK,
		pLab->bTouchArrayOK,
		pLab->bTouchOneOK,
		pLab->bTextOK,
		pLab->bResizeOK,
		pLab->bRequestOK,
		pLab->bAudioOK,
		pLab->bFrameOK,
		pLab->iFrameCallsFull,
		pLab->bInitSimpleOK,
		pLab->bBridgeAfterInitOK,
		pLab->bResourceAfterOK,
		pLab->bRequestAfterOK,
		pLab->bFrameAfterOK,
		pLab->bUnitAfterOK,
		pLab->iFrameCallbacks,
		pLab->iLoadCalls,
		pLab->iFreeCalls,
		pLab->iRequestFrameCalls,
		pLab->iAudioCommandCalls,
		pLab->iAudioCommandSum,
		pLab->sLastResource[0] ? pLab->sLastResource : "none");
}

static int RunDescPhase(miniprogram_bridge_lab_t* pLab)
{
	xge_miniprogram_bridge_t tBridge;
	xge_miniprogram_desc_t tMiniDesc;
	xge_miniprogram_touch_t tTouch;
	xge_platform_caps_t tCaps;
	xge_touch_point_t tPoint;
	xge_resource_t tResource;
	int i;
	int iRet;

	memset(&tBridge, 0, sizeof(tBridge));
	memset(&tMiniDesc, 0, sizeof(tMiniDesc));
	memset(&tCaps, 0, sizeof(tCaps));
	memset(&tPoint, 0, sizeof(tPoint));
	memset(&tResource, 0, sizeof(tResource));
	memset(&tTouch, 0, sizeof(tTouch));

	iRet = StartManualRuntime(640, 360);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tBridge.request_frame = HostRequestFrame;
	tBridge.load_resource = HostLoadResource;
	tBridge.free_resource = HostFreeResource;
	tBridge.audio_command = HostAudioCommand;
	tBridge.pUser = pLab;
	pLab->bBridgeBeforeInitOK = (xgeMiniProgramSetBridge(&tBridge) == XGE_OK);
	tMiniDesc.pCanvas = (void*)0x1;
	tMiniDesc.pWebGLContext = (void*)0x2;
	tMiniDesc.iWidth = 640;
	tMiniDesc.iHeight = 360;
	tMiniDesc.fDevicePixelRatio = 2.0f;
	tMiniDesc.pUser = pLab;
	pLab->bInitDescOK = (xgeMiniProgramInit(&tMiniDesc) == XGE_OK);
	pLab->bBackendOK = pLab->bInitDescOK &&
		(xgePlatformBackendGet().iType == XGE_PLATFORM_BACKEND_MINIPROGRAM) &&
		(xgeGraphicsBackendGet().iType == XGE_GPU_BACKEND_WEBGL2) &&
		(xgeGetWidth() == 640) &&
		(xgeGetHeight() == 360);
	pLab->bCapsOK = pLab->bInitDescOK &&
		(xgePlatformCapsGet(&tCaps) == XGE_OK) &&
		tCaps.bMiniProgram &&
		tCaps.bWASM &&
		tCaps.bTouch &&
		(tCaps.iGraphicsBackend == XGE_GPU_BACKEND_WEBGL2);

	if ( xgeRun(MiniProgramFrameProc, pLab) != XGE_OK ) {
		xgeMiniProgramUnit();
		xgeUnit();
		return XGE_ERROR_BACKEND_FAILED;
	}
	if ( xgeResourceLoad("res://bridge-note.txt", &tResource) == XGE_OK ) {
		pLab->bResourceBeforeOK = CheckResourceText(&tResource, "bridge-before-init");
		xgeResourceFree(&tResource);
	}

	tTouch.iId = 7;
	tTouch.fX = 12.0f;
	tTouch.fY = 34.0f;
	tTouch.fForce = 1.0f;
	pLab->bTouchArrayOK = (xgeMiniProgramTouch(XGE_TOUCH_BEGIN, &tTouch, 1) == XGE_OK) &&
		(xgeTouchGetCount() == 1) &&
		(xgeTouchFind(7, &tPoint) == XGE_OK) &&
		(tPoint.fX == 12.0f) &&
		(tPoint.fY == 34.0f);
	pLab->bTouchOneOK = (xgeMiniProgramTouchOne(XGE_TOUCH_MOVE, 8, 56.0f, 78.0f, 1.0f) == XGE_OK) &&
		(xgeTouchFind(8, &tPoint) == XGE_OK) &&
		(tPoint.fX == 56.0f) &&
		(tPoint.fY == 78.0f);
	pLab->bTextOK = (xgeMiniProgramText('Z') == XGE_OK) && (xgeTextGet() == 'Z');
	pLab->bResizeOK = (xgeMiniProgramResize(320, 200, 1.5f) == XGE_OK) &&
		(xgeGetWidth() == 320) &&
		(xgeGetHeight() == 200);
	pLab->bRequestOK = (xgeMiniProgramRequestFrame() == XGE_OK) && (pLab->iRequestFrameCalls == 1);
	pLab->bAudioOK = (xgeMiniProgramAudioCommand(10, 5, "ok", 2) == XGE_OK) &&
		(pLab->iAudioCommandCalls == 1) &&
		(pLab->iAudioCommandSum == 17);
	pLab->bFrameOK = 1;
	for ( i = 0; i < pLab->iFrameLimit; i++ ) {
		iRet = xgeMiniProgramFrame((double)i / 60.0);
		if ( iRet != 1 ) {
			pLab->bFrameOK = 0;
			break;
		}
		pLab->iFrameCallsFull++;
	}
	xgeMiniProgramUnit();
	pLab->bUnitOK = (xgeMiniProgramFrame(0.0) == XGE_ERROR_NOT_INITIALIZED);
	xgeUnit();
	return XGE_OK;
}

static int RunSimplePhase(miniprogram_bridge_lab_t* pLab)
{
	xge_miniprogram_bridge_t tBridge;
	xge_resource_t tResource;
	int i;
	int iRet;
	int iRequestBefore;

	memset(&tBridge, 0, sizeof(tBridge));
	memset(&tResource, 0, sizeof(tResource));

	iRet = StartManualRuntime(240, 160);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pLab->bInitSimpleOK = (xgeMiniProgramInitSimple(240, 160, 1.0f) == XGE_OK) &&
		(xgeGetWidth() == 240) &&
		(xgeGetHeight() == 160);
	tBridge.request_frame = HostRequestFrame;
	tBridge.load_resource = HostLoadResource;
	tBridge.free_resource = HostFreeResource;
	tBridge.audio_command = HostAudioCommand;
	tBridge.pUser = pLab;
	pLab->bBridgeAfterInitOK = (xgeMiniProgramSetBridge(&tBridge) == XGE_OK);

	if ( xgeRun(MiniProgramFrameProc, pLab) != XGE_OK ) {
		xgeMiniProgramUnit();
		xgeUnit();
		return XGE_ERROR_BACKEND_FAILED;
	}
	if ( xgeResourceLoad("res://bridge-after.txt", &tResource) == XGE_OK ) {
		pLab->bResourceAfterOK = CheckResourceText(&tResource, "bridge-after-init");
		xgeResourceFree(&tResource);
	}
	iRequestBefore = pLab->iRequestFrameCalls;
	pLab->bRequestAfterOK = (xgeMiniProgramRequestFrame() == XGE_OK) &&
		(pLab->iRequestFrameCalls == (iRequestBefore + 1));
	pLab->bFrameAfterOK = 1;
	for ( i = 0; i < pLab->iFrameLimit; i++ ) {
		iRet = xgeMiniProgramFrame((double)(i + 100) / 60.0);
		if ( iRet != 1 ) {
			pLab->bFrameAfterOK = 0;
			break;
		}
		pLab->iFrameCallsSimple++;
	}
	xgeMiniProgramUnit();
	pLab->bUnitAfterOK = (xgeMiniProgramFrame(0.0) == XGE_ERROR_NOT_INITIALIZED);
	xgeUnit();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	miniprogram_bridge_lab_t tLab;
	int i;
	int iRet;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_MINIPROGRAM_BRIDGE_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		}
	}

	iRet = RunDescPhase(&tLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "miniprogram-bridge-lab desc phase failed: %d\n", iRet);
		return 1;
	}
	iRet = RunSimplePhase(&tLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "miniprogram-bridge-lab simple phase failed: %d\n", iRet);
		return 2;
	}

	PrintSummary(&tLab);
	printf("miniprogram-bridge-lab summary frames=%d/%d\n", tLab.iFrameCallsFull, tLab.iFrameCallsSimple);
	return 0;
}
