#include "xui_internal.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define XUI_CONTEXT_REQUIRED_CAPS \
	(XUI_PROXY_CAP_SURFACE_TARGET | \
	 XUI_PROXY_CAP_SURFACE_READ | \
	 XUI_PROXY_CAP_SURFACE_QUAD | \
	 XUI_PROXY_CAP_SURFACE_CLEAR_RECT | \
	 XUI_PROXY_CAP_SURFACE_SAMPLER | \
	 XUI_PROXY_CAP_DRAW_CONTEXT | \
	 XUI_PROXY_CAP_SHAPE | \
	 XUI_PROXY_CAP_FONT_TTF | \
	 XUI_PROXY_CAP_FONT_XRF | \
	 XUI_PROXY_CAP_TEXT)

struct xui_resource_t {
	uint32_t iMagic;
	xui_context pContext;
	xui_resource pNext;
	char* sName;
	int iKind;
	void* pHandle;
	void* pUser;
	void (*onDestroy)(xui_context pContext, void* pHandle, void* pUser);
	uint32_t iGeneration;
	int iRefCount;
	xui_resource_dependency_t* pDependencies;
};

struct xui_resource_dependency_t {
	xui_resource pResource;
	xui_resource_dependency_t* pNext;
};

struct xui_path_t {
	uint32_t iMagic;
	xui_path_command_t* pCommands;
	int iCommandCount;
	int iCommandCapacity;
};

typedef struct xui_vector_icon_def_t {
	const char* sName;
	const char* sPath;
	float fViewWidth;
	float fViewHeight;
} xui_vector_icon_def_t;

static const xui_vector_icon_def_t g_arrXuiVectorIcons[] = {
	{"check", "M20 6 L9 17 L4 12", 24.0f, 24.0f},
	{"close", "M6 6 L18 18 M18 6 L6 18", 24.0f, 24.0f},
	{"chevron_down", "M6 9 L12 15 L18 9", 24.0f, 24.0f},
	{"search", "M10 18 C5.58 18 2 14.42 2 10 C2 5.58 5.58 2 10 2 C14.42 2 18 5.58 18 10 C18 12.1 17.19 14 15.88 15.88 L22 22", 24.0f, 24.0f},
	{"user", "M20 21 C20 17.69 16.42 15 12 15 C7.58 15 4 17.69 4 21 M12 12 C9.79 12 8 10.21 8 8 C8 5.79 9.79 4 12 4 C14.21 4 16 5.79 16 8 C16 10.21 14.21 12 12 12", 24.0f, 24.0f},
	{"lock", "M7 11 V8 C7 5.24 9.24 3 12 3 C14.76 3 17 5.24 17 8 V11 M6 11 H18 V21 H6 Z", 24.0f, 24.0f},
	{"eye", "M2 12 C5 7 8.5 5 12 5 C15.5 5 19 7 22 12 C19 17 15.5 19 12 19 C8.5 19 5 17 2 12 Z M12 15 C10.34 15 9 13.66 9 12 C9 10.34 10.34 9 12 9 C13.66 9 15 10.34 15 12 C15 13.66 13.66 15 12 15 Z", 24.0f, 24.0f}
};

static int __xuiPathBuildFillMeshWithRule(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fTolerance, int* pVertexCount, int* pIndexCount);
static int __xuiPathBuildDashedStrokeMeshWithStyle(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, int iLineJoin, int iLineCap, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, int* pVertexCount, int* pIndexCount);

static int __xuiContextValid(xui_context pContext)
{
	return (pContext != NULL) && (pContext->iMagic == XUI_CONTEXT_MAGIC);
}

static int __xuiResourceValid(xui_resource pResource)
{
	return (pResource != NULL) && (pResource->iMagic == XUI_RESOURCE_MAGIC);
}

static char* __xuiCoreStringDuplicate(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) {
		return NULL;
	}
	iSize = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize);
	return sCopy;
}

static int __xuiPainterValid(xui_painter pPainter)
{
	return (pPainter != NULL) && (pPainter->iMagic == XUI_PAINTER_MAGIC);
}

static int __xuiPathValid(xui_path pPath)
{
	return (pPath != NULL) && (pPath->iMagic == XUI_PATH_MAGIC);
}

static int __xuiPathReserve(xui_path pPath, int iNeeded)
{
	xui_path_command_t* pCommands;
	int iCapacity;

	if ( !__xuiPathValid(pPath) || (iNeeded < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pPath->iCommandCapacity ) {
		return XUI_OK;
	}
	iCapacity = (pPath->iCommandCapacity > 0) ? pPath->iCommandCapacity : 16;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pCommands = (xui_path_command_t*)xrtRealloc(pPath->pCommands, sizeof(*pCommands) * (size_t)iCapacity);
	if ( pCommands == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPath->pCommands = pCommands;
	pPath->iCommandCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiPathAddCommand(xui_path pPath, int iCommand, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC)
{
	xui_path_command_t* pCommand;
	int iRet;

	if ( !__xuiPathValid(pPath) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiPathReserve(pPath, pPath->iCommandCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pCommand = &pPath->pCommands[pPath->iCommandCount++];
	memset(pCommand, 0, sizeof(*pCommand));
	pCommand->iCommand = iCommand;
	pCommand->arrPoints[0] = tA;
	pCommand->arrPoints[1] = tB;
	pCommand->arrPoints[2] = tC;
	return XUI_OK;
}

static int __xuiPathFlattenAdd(xui_vec2_t* pPoints, int iCapacity, int* pCount, xui_vec2_t tPoint)
{
	if ( pCount == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pPoints != NULL) && (*pCount < iCapacity) ) {
		pPoints[*pCount] = tPoint;
	}
	(*pCount)++;
	return XUI_OK;
}

static int __xuiPathFlattenSteps(float fTolerance)
{
	int iSteps;

	if ( fTolerance <= 0.0f ) {
		fTolerance = 1.0f;
	}
	iSteps = (int)(16.0f / fTolerance);
	if ( iSteps < 4 ) iSteps = 4;
	if ( iSteps > 64 ) iSteps = 64;
	return iSteps;
}

static float __xuiPathSqrt(float fValue)
{
	float fGuess;
	int i;

	if ( fValue <= 0.0f ) {
		return 0.0f;
	}
	fGuess = (fValue > 1.0f) ? fValue : 1.0f;
	for ( i = 0; i < 8; i++ ) {
		fGuess = 0.5f * (fGuess + (fValue / fGuess));
	}
	return fGuess;
}

static int __xuiFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_CONTEXT_MAX_VIEWPORT);
}

static int __xuiRectValid(xui_rect_i_t tRect)
{
	return (tRect.iW > 0) && (tRect.iH > 0);
}

static int __xuiContextCeilPositive(float fValue)
{
	if ( fValue <= 0.0f ) {
		return 0;
	}
	return xuiInternalPixelCeil(fValue);
}

static xui_rect_i_t __xuiRectUnion(xui_rect_i_t tA, xui_rect_i_t tB)
{
	xui_rect_i_t tRet;
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;

	iLeft = (tA.iX < tB.iX) ? tA.iX : tB.iX;
	iTop = (tA.iY < tB.iY) ? tA.iY : tB.iY;
	iRight = ((tA.iX + tA.iW) > (tB.iX + tB.iW)) ? (tA.iX + tA.iW) : (tB.iX + tB.iW);
	iBottom = ((tA.iY + tA.iH) > (tB.iY + tB.iH)) ? (tA.iY + tA.iH) : (tB.iY + tB.iH);
	tRet.iX = iLeft;
	tRet.iY = iTop;
	tRet.iW = iRight - iLeft;
	tRet.iH = iBottom - iTop;
	return tRet;
}

static int __xuiRectTouchesOrIntersects(xui_rect_i_t tA, xui_rect_i_t tB)
{
	if ( (tA.iX > (tB.iX + tB.iW)) || (tB.iX > (tA.iX + tA.iW)) ) {
		return 0;
	}
	if ( (tA.iY > (tB.iY + tB.iH)) || (tB.iY > (tA.iY + tA.iH)) ) {
		return 0;
	}
	return 1;
}

static int __xuiContextClipToViewport(xui_context pContext, xui_rect_i_t* pRect)
{
	int iWidth;
	int iHeight;
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;

	iWidth = __xuiContextCeilPositive(pContext->fViewportWidth);
	iHeight = __xuiContextCeilPositive(pContext->fViewportHeight);
	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return 1;
	}
	iLeft = pRect->iX;
	iTop = pRect->iY;
	iRight = pRect->iX + pRect->iW;
	iBottom = pRect->iY + pRect->iH;
	if ( iLeft < 0 ) {
		iLeft = 0;
	}
	if ( iTop < 0 ) {
		iTop = 0;
	}
	if ( iRight > iWidth ) {
		iRight = iWidth;
	}
	if ( iBottom > iHeight ) {
		iBottom = iHeight;
	}
	pRect->iX = iLeft;
	pRect->iY = iTop;
	pRect->iW = iRight - iLeft;
	pRect->iH = iBottom - iTop;
	return __xuiRectValid(*pRect);
}

static int __xuiContextDamageReserve(xui_context pContext, int iCapacity)
{
	xui_rect_i_t* pDamage;

	if ( iCapacity <= pContext->iDamageCapacity ) {
		return XUI_OK;
	}
	if ( iCapacity < (pContext->iDamageCapacity * 2) ) {
		iCapacity = pContext->iDamageCapacity * 2;
	}
	if ( pContext->pDamage == pContext->arrInlineDamage ) {
		pDamage = (xui_rect_i_t*)xrtMalloc(sizeof(*pDamage) * (size_t)iCapacity);
		if ( pDamage == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pDamage, pContext->pDamage, sizeof(*pDamage) * (size_t)pContext->iDamageCount);
	} else {
		pDamage = (xui_rect_i_t*)xrtRealloc(pContext->pDamage, sizeof(*pDamage) * (size_t)iCapacity);
		if ( pDamage == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	pContext->pDamage = pDamage;
	pContext->iDamageCapacity = iCapacity;
	return XUI_OK;
}

static void __xuiContextBumpGeneration(xui_context pContext)
{
	pContext->iGeneration++;
	if ( pContext->iGeneration == 0 ) {
		pContext->iGeneration = 1;
	}
}

static void __xuiContextRemoveDamage(xui_context pContext, int iIndex)
{
	int iMoveCount;

	iMoveCount = pContext->iDamageCount - iIndex - 1;
	if ( iMoveCount > 0 ) {
		memmove(&pContext->pDamage[iIndex], &pContext->pDamage[iIndex + 1], sizeof(pContext->pDamage[0]) * (size_t)iMoveCount);
	}
	pContext->iDamageCount--;
}

static int __xuiContextAddDamage(xui_context pContext, xui_rect_i_t tRect)
{
	int i;
	int j;
	int iRet;

	if ( !__xuiRectValid(tRect) ) {
		return XUI_OK;
	}
	if ( !__xuiContextClipToViewport(pContext, &tRect) ) {
		return XUI_OK;
	}
	for ( i = 0; i < pContext->iDamageCount; i++ ) {
		if ( __xuiRectTouchesOrIntersects(pContext->pDamage[i], tRect) ) {
			pContext->pDamage[i] = __xuiRectUnion(pContext->pDamage[i], tRect);
			j = i + 1;
			while ( j < pContext->iDamageCount ) {
				if ( __xuiRectTouchesOrIntersects(pContext->pDamage[i], pContext->pDamage[j]) ) {
					pContext->pDamage[i] = __xuiRectUnion(pContext->pDamage[i], pContext->pDamage[j]);
					__xuiContextRemoveDamage(pContext, j);
				} else {
					j++;
				}
			}
			__xuiContextBumpGeneration(pContext);
			return XUI_OK;
		}
	}
	iRet = __xuiContextDamageReserve(pContext, pContext->iDamageCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pContext->pDamage[pContext->iDamageCount++] = tRect;
	__xuiContextBumpGeneration(pContext);
	return XUI_OK;
}

int xuiInternalContextIsValid(xui_context pContext)
{
	return __xuiContextValid(pContext);
}

int xuiInternalContextHasProxy(xui_context pContext)
{
	return __xuiContextValid(pContext) && pContext->bHasProxy;
}

xui_proxy xuiInternalContextGetProxy(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) || !pContext->bHasProxy ) {
		return NULL;
	}
	return &pContext->tProxy;
}

int xuiInternalContextInvalidateRect(xui_context pContext, xui_rect_i_t tRect)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiContextAddDamage(pContext, tRect);
}

int xuiInternalContextInvalidateAll(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return xuiInvalidateAll(pContext);
}

void xuiInternalContextBumpGeneration(xui_context pContext)
{
	if ( __xuiContextValid(pContext) ) {
		__xuiContextBumpGeneration(pContext);
	}
}

void xuiInternalContextDestroyRenderTree(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	if ( pContext->pRenderNodes != NULL ) {
		xrtFree(pContext->pRenderNodes);
	}
	pContext->pRenderNodes = NULL;
	pContext->iRenderNodeCount = 0;
	pContext->iRenderNodeCapacity = 0;
	pContext->iRenderTreeGeneration = 0;
}

void xuiInternalContextDestroyInput(xui_context pContext)
{
	int i;

	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	for ( i = 0; i < pContext->iHotkeyCount; i++ ) {
		if ( pContext->pHotkeys[i].sCommand != NULL ) {
			xrtFree(pContext->pHotkeys[i].sCommand);
		}
	}
	if ( pContext->pHotkeys != pContext->arrInlineHotkeys ) {
		xrtFree(pContext->pHotkeys);
	}
	pContext->pHotkeys = pContext->arrInlineHotkeys;
	pContext->iHotkeyCount = 0;
	pContext->iHotkeyCapacity = XUI_CONTEXT_HOTKEY_INLINE;
	pContext->pDragWidget = NULL;
	pContext->bDragActive = 0;
	pContext->pLastClickWidget = NULL;
	memset(pContext->arrPointerStates, 0, sizeof(pContext->arrPointerStates));
	pContext->iPointerStateCount = 0;
	pContext->iInputPointerId = XUI_POINTER_ID_MOUSE;
	pContext->iInputPointerType = XUI_POINTER_TYPE_MOUSE;
	pContext->iInputDispatchDepth = 0;
	if ( pContext->sActiveTooltipText != NULL ) {
		xrtFree(pContext->sActiveTooltipText);
		pContext->sActiveTooltipText = NULL;
	}
	pContext->pTooltipOwner = NULL;
	pContext->pTooltipPopupWidget = NULL;
	pContext->bTooltipOpen = 0;
	pContext->fTooltipHoverTime = 0.0f;
	memset(&pContext->tActiveTooltip, 0, sizeof(pContext->tActiveTooltip));
	pContext->bImeEnabled = 0;
	pContext->bHasImeCandidateRect = 0;
}

static void __xuiResourceDestroyOne(xui_resource pResource)
{
	xui_resource_dependency_t* pDependency;
	xui_resource_dependency_t* pNextDependency;

	if ( !__xuiResourceValid(pResource) ) {
		return;
	}
	pDependency = pResource->pDependencies;
	while ( pDependency != NULL ) {
		pNextDependency = pDependency->pNext;
		xrtFree(pDependency);
		pDependency = pNextDependency;
	}
	pResource->pDependencies = NULL;
	if ( pResource->onDestroy != NULL ) {
		pResource->onDestroy(pResource->pContext, pResource->pHandle, pResource->pUser);
	}
	if ( pResource->sName != NULL ) {
		xrtFree(pResource->sName);
	}
	pResource->iMagic = 0;
	xrtFree(pResource);
}

static void __xuiResourceRemoveDependencyReferences(xui_context pContext, xui_resource pRemoved)
{
	xui_resource pResource;
	xui_resource_dependency_t* pDependency;
	xui_resource_dependency_t* pPrev;
	xui_resource_dependency_t* pNext;

	for ( pResource = pContext->pResources; pResource != NULL; pResource = pResource->pNext ) {
		if ( pResource == pRemoved ) {
			continue;
		}
		pPrev = NULL;
		pDependency = pResource->pDependencies;
		while ( pDependency != NULL ) {
			pNext = pDependency->pNext;
			if ( pDependency->pResource == pRemoved ) {
				if ( pPrev != NULL ) {
					pPrev->pNext = pNext;
				} else {
					pResource->pDependencies = pNext;
				}
				xrtFree(pDependency);
				pResource->iGeneration = pContext->iNextResourceGeneration++;
				if ( pContext->iNextResourceGeneration == 0 ) {
					pContext->iNextResourceGeneration = 1;
				}
			} else {
				pPrev = pDependency;
			}
			pDependency = pNext;
		}
	}
}

void xuiInternalContextDestroyResources(xui_context pContext)
{
	xui_resource pResource;
	xui_resource pNext;

	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	pResource = pContext->pResources;
	while ( pResource != NULL ) {
		pNext = pResource->pNext;
		__xuiResourceDestroyOne(pResource);
		pResource = pNext;
	}
	pContext->pResources = NULL;
}

static int __xuiContextWidgetContains(xui_widget pRoot, xui_widget pWidget)
{
	xui_widget pScan;

	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	for ( pScan = pWidget; pScan != NULL; pScan = pScan->pParent ) {
		if ( pScan == pRoot ) {
			return 1;
		}
	}
	return 0;
}

static void __xuiContextClearOverlayOwners(xui_widget pScan, xui_widget pRemoved)
{
	xui_widget pChild;

	if ( pScan == NULL ) {
		return;
	}
	if ( __xuiContextWidgetContains(pRemoved, pScan->pOverlayOwner) ) {
		pScan->pOverlayOwner = NULL;
	}
	for ( pChild = pScan->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xuiContextClearOverlayOwners(pChild, pRemoved);
	}
}

void xuiInternalContextDetachWidget(xui_context pContext, xui_widget pWidget)
{
	int i;

	if ( !__xuiContextValid(pContext) || (pWidget == NULL) ) {
		return;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pHoverWidget) ) {
		if ( pContext->pHoverWidget != NULL ) {
			pContext->pHoverWidget->iInputState &= ~XUI_WIDGET_STATE_HOVER;
		}
		pContext->pHoverWidget = NULL;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pActiveWidget) ) {
		if ( pContext->pActiveWidget != NULL ) {
			pContext->pActiveWidget->iInputState &= ~XUI_WIDGET_STATE_ACTIVE;
		}
		pContext->pActiveWidget = NULL;
		pContext->iActiveButton = 0;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pFocusWidget) ) {
		if ( pContext->pFocusWidget != NULL ) {
			pContext->pFocusWidget->iInputState &= ~XUI_WIDGET_STATE_FOCUS;
		}
		pContext->pFocusWidget = NULL;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pPointerCaptureWidget) ) {
		pContext->pPointerCaptureWidget = NULL;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pDragWidget) ) {
		pContext->pDragWidget = NULL;
		pContext->bDragActive = 0;
		pContext->iDragButton = 0;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pLastClickWidget) ) {
		pContext->pLastClickWidget = NULL;
		pContext->iLastClickButton = 0;
		pContext->fLastClickTime = 0.0;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pContextPressWidget) ) {
		xuiInternalContextPressCancel(pContext);
	}
	for ( i = 0; i < XUI_POINTER_MAX; i++ ) {
		xui_pointer_state_t* pState = &pContext->arrPointerStates[i];
		if ( !pState->bAllocated ) {
			continue;
		}
		if ( __xuiContextWidgetContains(pWidget, pState->pHoverWidget) ) {
			pState->pHoverWidget = NULL;
		}
		if ( __xuiContextWidgetContains(pWidget, pState->pActiveWidget) ) {
			pState->pActiveWidget = NULL;
			pState->iActiveButton = 0;
		}
		if ( __xuiContextWidgetContains(pWidget, pState->pPointerCaptureWidget) ) {
			pState->pPointerCaptureWidget = NULL;
		}
		if ( __xuiContextWidgetContains(pWidget, pState->pDragWidget) ) {
			pState->pDragWidget = NULL;
			pState->bDragActive = 0;
			pState->iDragButton = 0;
		}
		if ( __xuiContextWidgetContains(pWidget, pState->pLastClickWidget) ) {
			pState->pLastClickWidget = NULL;
			pState->iLastClickButton = 0;
			pState->fLastClickTime = 0.0;
		}
		if ( __xuiContextWidgetContains(pWidget, pState->pContextPressWidget) ) {
			pState->pContextPressWidget = NULL;
			pState->bContextPressActive = 0;
			pState->bContextPressMoved = 0;
			pState->bContextPressFired = 0;
			pState->fContextPressTime = 0.0f;
		}
	}
	xuiInternalTooltipDetachWidget(pContext, pWidget);
	for ( i = 0; i < pContext->iHotkeyCount; i++ ) {
		if ( __xuiContextWidgetContains(pWidget, pContext->pHotkeys[i].pWidget) ) {
			pContext->pHotkeys[i].pWidget = NULL;
		}
	}
	__xuiContextClearOverlayOwners(pContext->pOverlayRoot, pWidget);
	for ( i = pContext->iEventRead; i < pContext->iEventCount; i++ ) {
		if ( __xuiContextWidgetContains(pWidget, pContext->pEvents[i].pTarget) ) {
			pContext->pEvents[i].pTarget = NULL;
		}
		if ( __xuiContextWidgetContains(pWidget, pContext->pEvents[i].pRelated) ) {
			pContext->pEvents[i].pRelated = NULL;
		}
	}
}

static xui_rect_i_t __xuiContextFullRect(xui_context pContext)
{
	xui_rect_i_t tRect;

	tRect.iX = 0;
	tRect.iY = 0;
	tRect.iW = __xuiContextCeilPositive(pContext->fViewportWidth);
	tRect.iH = __xuiContextCeilPositive(pContext->fViewportHeight);
	return tRect;
}

static int __xuiProxyRequiredCallbacks(const xui_proxy_t* pProxy)
{
	return (pProxy->getCaps != NULL) &&
	       (pProxy->clipboardSetText != NULL) &&
	       (pProxy->clipboardGetText != NULL) &&
	       (pProxy->imeGetEnabled != NULL) &&
	       (pProxy->imeSetEnabled != NULL) &&
	       (pProxy->imeSetCandidateRect != NULL) &&
	       (pProxy->surfaceCreate != NULL) &&
	       (pProxy->surfaceCreateRGBA != NULL) &&
	       (pProxy->surfaceLoadFile != NULL) &&
	       (pProxy->surfaceLoadMemory != NULL) &&
	       (pProxy->surfaceUpdateRGBA != NULL) &&
	       (pProxy->surfaceReadRGBA != NULL) &&
	       (pProxy->surfaceGetDesc != NULL) &&
	       (pProxy->surfaceDraw != NULL) &&
	       (pProxy->surfaceClear != NULL) &&
	       (pProxy->surfaceClearRect != NULL) &&
	       (pProxy->surfaceDrawTo != NULL) &&
	       (pProxy->surfaceDrawQuad != NULL) &&
	       (pProxy->surfaceDrawQuadTo != NULL) &&
	       (pProxy->surfaceGetSampler != NULL) &&
	       (pProxy->surfaceSetSampler != NULL) &&
	       (pProxy->surfaceGetGeneration != NULL) &&
	       (pProxy->surfaceDestroy != NULL) &&
	       (pProxy->shapePoint != NULL) &&
	       (pProxy->shapeLine != NULL) &&
	       (pProxy->shapeTriangleFill != NULL) &&
	       (pProxy->shapeTriangleStroke != NULL) &&
	       (pProxy->shapeRectFill != NULL) &&
	       (pProxy->shapeRectStroke != NULL) &&
	       (pProxy->shapeCircleFill != NULL) &&
	       (pProxy->shapeCircleStroke != NULL) &&
	       (pProxy->shapeRoundRectFill != NULL) &&
	       (pProxy->shapeRoundRectStroke != NULL) &&
	       (pProxy->fontLoadFile != NULL) &&
	       (pProxy->fontLoadMemory != NULL) &&
	       (pProxy->fontGetMetrics != NULL) &&
	       (pProxy->fontDestroy != NULL) &&
	       (pProxy->textMeasure != NULL) &&
	       (pProxy->textDraw != NULL) &&
	       (pProxy->drawBegin != NULL) &&
	       (pProxy->drawEnd != NULL) &&
	       (pProxy->drawClearRect != NULL) &&
	       (pProxy->drawSurface != NULL) &&
	       (pProxy->drawSurfaceQuad != NULL) &&
	       (pProxy->drawPoint != NULL) &&
	       (pProxy->drawLine != NULL) &&
	       (pProxy->drawTriangleFill != NULL) &&
	       (pProxy->drawTriangleStroke != NULL) &&
	       (pProxy->drawRectFill != NULL) &&
	       (pProxy->drawRectStroke != NULL) &&
	       (pProxy->drawCircleFill != NULL) &&
	       (pProxy->drawCircleStroke != NULL) &&
	       (pProxy->drawRoundRectFill != NULL) &&
	       (pProxy->drawRoundRectStroke != NULL) &&
	       (pProxy->drawText != NULL);
}

static int __xuiContextValidateProxy(const xui_proxy_t* pProxy, xui_proxy_caps_t* pCaps)
{
	int iRet;

	if ( (pProxy == NULL) || (pCaps == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy->iSize < sizeof(*pProxy)) || (pProxy->iVersion != XUI_PROXY_VERSION) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( !__xuiProxyRequiredCallbacks(pProxy) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	memset(pCaps, 0, sizeof(*pCaps));
	iRet = pProxy->getCaps((xui_proxy)pProxy, pCaps);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pCaps->iSize < sizeof(*pCaps) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( (pCaps->iCaps & XUI_CONTEXT_REQUIRED_CAPS) != XUI_CONTEXT_REQUIRED_CAPS ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( (pCaps->iSurfaceFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     (pCaps->iInternalAlpha != XUI_SURFACE_ALPHA_PREMULTIPLIED) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( ((pCaps->tDefaultSampler.iMinFilter != XUI_SURFACE_FILTER_NEAREST) &&
	      (pCaps->tDefaultSampler.iMinFilter != XUI_SURFACE_FILTER_LINEAR)) ||
	     ((pCaps->tDefaultSampler.iMagFilter != XUI_SURFACE_FILTER_NEAREST) &&
	      (pCaps->tDefaultSampler.iMagFilter != XUI_SURFACE_FILTER_LINEAR)) ||
	     ((pCaps->tDefaultSampler.iWrapS != XUI_SURFACE_WRAP_CLAMP) &&
	      (pCaps->tDefaultSampler.iWrapS != XUI_SURFACE_WRAP_REPEAT)) ||
	     ((pCaps->tDefaultSampler.iWrapT != XUI_SURFACE_WRAP_CLAMP) &&
	      (pCaps->tDefaultSampler.iWrapT != XUI_SURFACE_WRAP_REPEAT)) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	return XUI_OK;
}

XUI_API int xuiCreate(xui_context* ppContext)
{
	xui_context pContext;

	if ( ppContext == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppContext = NULL;
	pContext = (xui_context)xrtMalloc(sizeof(*pContext));
	if ( pContext == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memset(pContext, 0, sizeof(*pContext));
	pContext->iMagic = XUI_CONTEXT_MAGIC;
	pContext->fDpiScale = 1.0f;
	pContext->iGeneration = 1;
	pContext->iNextStylePropertyId = 1;
	pContext->iNextResourceGeneration = 1;
	pContext->pDamage = pContext->arrInlineDamage;
	pContext->iDamageCapacity = XUI_CONTEXT_DAMAGE_INLINE;
	pContext->pEvents = pContext->arrInlineEvents;
	pContext->iEventCapacity = XUI_CONTEXT_EVENT_INLINE;
	pContext->pHotkeys = pContext->arrInlineHotkeys;
	pContext->iHotkeyCapacity = XUI_CONTEXT_HOTKEY_INLINE;
	pContext->pFonts = pContext->arrInlineFonts;
	pContext->iFontCapacity = XUI_CONTEXT_FONT_INLINE;
	xuiThemeDefault(&pContext->tTheme);
	pContext->tChromeStyle.iSize = sizeof(pContext->tChromeStyle);
	pContext->tChromeStyle.iBackdropColor = XUI_COLOR_RGBA(18, 20, 24, 255);
	pContext->tChromeStyle.iPopupColor = XUI_COLOR_RGBA(248, 249, 251, 255);
	pContext->tChromeStyle.iPopupBorderColor = XUI_COLOR_RGBA(120, 126, 138, 255);
	pContext->tChromeStyle.iTooltipColor = XUI_COLOR_RGBA(28, 32, 38, 245);
	pContext->tChromeStyle.iTooltipTextColor = XUI_COLOR_RGBA(248, 250, 252, 255);
	pContext->tChromeStyle.iModalOverlayColor = XUI_COLOR_RGBA(0, 0, 0, 96);
	pContext->tChromeStyle.fRadius = 4.0f;
	pContext->tChromeStyle.fBorderWidth = 1.0f;
	pContext->tChromeStyle.fShadowSize = 8.0f;
	*ppContext = pContext;
	return XUI_OK;
}

static void __xuiContextDestroyFonts(xui_context pContext)
{
	int i;

	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	for ( i = 0; i < pContext->iFontCount; i++ ) {
		if ( pContext->pFonts[i].sName != NULL ) {
			xrtFree(pContext->pFonts[i].sName);
		}
	}
	if ( pContext->pFonts != pContext->arrInlineFonts ) {
		xrtFree(pContext->pFonts);
	}
	pContext->pFonts = pContext->arrInlineFonts;
	pContext->iFontCount = 0;
	pContext->iFontCapacity = XUI_CONTEXT_FONT_INLINE;
	pContext->pDefaultFont = NULL;
}

XUI_API void xuiDestroy(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	if ( pContext->pRoot != NULL ) {
		xuiWidgetDestroy(pContext->pRoot);
		pContext->pRoot = NULL;
	}
	if ( pContext->pOverlayRoot != NULL ) {
		xuiWidgetDestroy(pContext->pOverlayRoot);
		pContext->pOverlayRoot = NULL;
	}
	if ( pContext->pDamage != pContext->arrInlineDamage ) {
		xrtFree(pContext->pDamage);
	}
	if ( pContext->pEvents != pContext->arrInlineEvents ) {
		xrtFree(pContext->pEvents);
	}
	xuiInternalContextDestroyInput(pContext);
	xuiInternalContextDestroyRenderTree(pContext);
	xuiInternalContextDestroyResources(pContext);
	xuiInternalContextDestroyStyles(pContext);
	xuiInternalContextDestroyWidgetTypes(pContext);
	__xuiContextDestroyFonts(pContext);
	pContext->iMagic = 0;
	xrtFree(pContext);
}

XUI_API int xuiSetProxy(xui_context pContext, const xui_proxy_t* pProxy)
{
	xui_proxy_caps_t tCaps;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bHasProxy ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	iRet = __xuiContextValidateProxy(pProxy, &tCaps);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pContext->tProxy = *pProxy;
	pContext->tProxyCaps = tCaps;
	pContext->bHasProxy = 1;
	__xuiContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API int xuiGetProxy(xui_context pContext, xui_proxy_t* pProxy)
{
	if ( !__xuiContextValid(pContext) || (pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pContext->bHasProxy ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	*pProxy = pContext->tProxy;
	return XUI_OK;
}

XUI_API int xuiGetProxyCaps(xui_context pContext, xui_proxy_caps_t* pCaps)
{
	if ( !__xuiContextValid(pContext) || (pCaps == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pContext->bHasProxy ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	*pCaps = pContext->tProxyCaps;
	return XUI_OK;
}

XUI_API int xuiSetViewportSize(xui_context pContext, float fWidth, float fHeight)
{
	if ( !__xuiContextValid(pContext) || !__xuiFloatValid(fWidth) || !__xuiFloatValid(fHeight) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pContext->fViewportWidth == fWidth) && (pContext->fViewportHeight == fHeight) ) {
		return XUI_OK;
	}
	pContext->fViewportWidth = fWidth;
	pContext->fViewportHeight = fHeight;
	pContext->iDamageCount = 0;
	if ( (__xuiContextCeilPositive(fWidth) <= 0) || (__xuiContextCeilPositive(fHeight) <= 0) ) {
		__xuiContextBumpGeneration(pContext);
		return XUI_OK;
	}
	return __xuiContextAddDamage(pContext, __xuiContextFullRect(pContext));
}

XUI_API xui_vec2_t xuiGetViewportSize(xui_context pContext)
{
	xui_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( __xuiContextValid(pContext) ) {
		tSize.fX = pContext->fViewportWidth;
		tSize.fY = pContext->fViewportHeight;
	}
	return tSize;
}

XUI_API int xuiSetVirtualDpi(xui_context pContext, float fDpiScale)
{
	if ( !__xuiContextValid(pContext) || (fDpiScale != fDpiScale) || (fDpiScale <= 0.0f) || (fDpiScale > 64.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->fDpiScale == fDpiScale ) {
		return XUI_OK;
	}
	pContext->fDpiScale = fDpiScale;
	pContext->iDamageCount = 0;
	if ( (__xuiContextCeilPositive(pContext->fViewportWidth) <= 0) ||
	     (__xuiContextCeilPositive(pContext->fViewportHeight) <= 0) ) {
		__xuiContextBumpGeneration(pContext);
		return XUI_OK;
	}
	return __xuiContextAddDamage(pContext, __xuiContextFullRect(pContext));
}

XUI_API float xuiGetVirtualDpi(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return 0.0f;
	}
	return pContext->fDpiScale;
}

XUI_API void xuiThemeDefault(xui_theme_t* pTheme)
{
	if ( pTheme == NULL ) {
		return;
	}
	memset(pTheme, 0, sizeof(*pTheme));
	pTheme->iSize = sizeof(*pTheme);
	pTheme->iTextColor = XUI_COLOR_RGBA(34, 37, 41, 255);
	pTheme->iBackgroundColor = XUI_COLOR_RGBA(245, 247, 250, 255);
	pTheme->iPanelColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pTheme->iBorderColor = XUI_COLOR_RGBA(188, 195, 205, 255);
	pTheme->iAccentColor = XUI_COLOR_RGBA(0, 112, 210, 255);
	pTheme->iSelectionColor = XUI_COLOR_RGBA(0, 112, 210, 72);
	pTheme->iStateNormalColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pTheme->iStateHoverColor = XUI_COLOR_RGBA(236, 244, 252, 255);
	pTheme->iStateActiveColor = XUI_COLOR_RGBA(214, 232, 248, 255);
	pTheme->iStateFocusColor = XUI_COLOR_RGBA(0, 112, 210, 255);
	pTheme->iStateDisabledColor = XUI_COLOR_RGBA(162, 168, 176, 255);
	pTheme->fRadius = 4.0f;
	pTheme->fPadding = 6.0f;
	pTheme->fSpacing = 6.0f;
	pTheme->fBorderWidth = 1.0f;
}

static int __xuiCoreSetColorToken(xui_context pContext, const char* sName, uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return xuiStyleSetToken(pContext, sName, &tValue);
}

static int __xuiCoreSetFloatToken(xui_context pContext, const char* sName, float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return xuiStyleSetToken(pContext, sName, &tValue);
}

XUI_API int xuiSetTheme(xui_context pContext, const xui_theme_t* pTheme)
{
	xui_theme_t tTheme;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pTheme == NULL) ||
	     ((pTheme->iSize != 0) && (pTheme->iSize < sizeof(*pTheme))) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tTheme = *pTheme;
	tTheme.iSize = sizeof(tTheme);
	pContext->tTheme = tTheme;
	pContext->pDefaultFont = tTheme.pFont;
	iRet = xuiStyleBeginUpdate(pContext);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiCoreSetColorToken(pContext, "theme.text", tTheme.iTextColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.background", tTheme.iBackgroundColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.panel", tTheme.iPanelColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.border", tTheme.iBorderColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.accent", tTheme.iAccentColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.selection", tTheme.iSelectionColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.normal", tTheme.iStateNormalColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.hover", tTheme.iStateHoverColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.active", tTheme.iStateActiveColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.focus", tTheme.iStateFocusColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.disabled", tTheme.iStateDisabledColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.radius", tTheme.fRadius);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.padding", tTheme.fPadding);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.spacing", tTheme.fSpacing);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.border_width", tTheme.fBorderWidth);
	if ( iRet != XUI_OK ) {
		(void)xuiStyleEndUpdate(pContext);
		return iRet;
	}
	return xuiStyleEndUpdate(pContext);
}

XUI_API int xuiGetTheme(xui_context pContext, xui_theme_t* pTheme)
{
	if ( !__xuiContextValid(pContext) || (pTheme == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pTheme = pContext->tTheme;
	pTheme->iSize = sizeof(*pTheme);
	return XUI_OK;
}

XUI_API int xuiSetChromeStyle(xui_context pContext, const xui_chrome_style_t* pChrome)
{
	xui_chrome_style_t tChrome;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pChrome == NULL) ||
	     ((pChrome->iSize != 0) && (pChrome->iSize < sizeof(*pChrome))) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tChrome = *pChrome;
	tChrome.iSize = sizeof(tChrome);
	pContext->tChromeStyle = tChrome;
	iRet = xuiStyleBeginUpdate(pContext);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiCoreSetColorToken(pContext, "chrome.backdrop", tChrome.iBackdropColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.popup", tChrome.iPopupColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.popup_border", tChrome.iPopupBorderColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.tooltip", tChrome.iTooltipColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.tooltip_text", tChrome.iTooltipTextColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.modal_overlay", tChrome.iModalOverlayColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "chrome.radius", tChrome.fRadius);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "chrome.border_width", tChrome.fBorderWidth);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "chrome.shadow_size", tChrome.fShadowSize);
	if ( iRet != XUI_OK ) {
		(void)xuiStyleEndUpdate(pContext);
		return iRet;
	}
	return xuiStyleEndUpdate(pContext);
}

XUI_API int xuiGetChromeStyle(xui_context pContext, xui_chrome_style_t* pChrome)
{
	if ( !__xuiContextValid(pContext) || (pChrome == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pChrome = pContext->tChromeStyle;
	pChrome->iSize = sizeof(*pChrome);
	return XUI_OK;
}

XUI_API int xuiSetDefaultFont(xui_context pContext, xui_font pFont)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext->pDefaultFont = pFont;
	pContext->tTheme.pFont = pFont;
	xuiInternalContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API xui_font xuiGetDefaultFont(xui_context pContext)
{
	return __xuiContextValid(pContext) ? pContext->pDefaultFont : NULL;
}

static int __xuiCoreReserveFonts(xui_context pContext, int iCapacity)
{
	xui_font_entry_t* pFonts;

	if ( iCapacity <= pContext->iFontCapacity ) {
		return XUI_OK;
	}
	if ( iCapacity < (pContext->iFontCapacity * 2) ) {
		iCapacity = pContext->iFontCapacity * 2;
	}
	if ( pContext->pFonts == pContext->arrInlineFonts ) {
		pFonts = (xui_font_entry_t*)xrtMalloc(sizeof(*pFonts) * (size_t)iCapacity);
		if ( pFonts == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pFonts, pContext->pFonts, sizeof(*pFonts) * (size_t)pContext->iFontCount);
	} else {
		pFonts = (xui_font_entry_t*)xrtRealloc(pContext->pFonts, sizeof(*pFonts) * (size_t)iCapacity);
		if ( pFonts == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	pContext->pFonts = pFonts;
	pContext->iFontCapacity = iCapacity;
	return XUI_OK;
}

XUI_API int xuiRegisterFont(xui_context pContext, const char* sName, xui_font pFont)
{
	char* sCopy;
	int i;
	int iRet;

	if ( !__xuiContextValid(pContext) || (sName == NULL) || (sName[0] == '\0') || (pFont == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pContext->iFontCount; i++ ) {
		if ( strcmp(pContext->pFonts[i].sName, sName) == 0 ) {
			pContext->pFonts[i].pFont = pFont;
			xuiInternalContextBumpGeneration(pContext);
			return XUI_OK;
		}
	}
	iRet = __xuiCoreReserveFonts(pContext, pContext->iFontCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	sCopy = __xuiCoreStringDuplicate(sName);
	if ( sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pContext->pFonts[pContext->iFontCount].sName = sCopy;
	pContext->pFonts[pContext->iFontCount].pFont = pFont;
	pContext->iFontCount++;
	xuiInternalContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API xui_font xuiFindFont(xui_context pContext, const char* sName)
{
	int i;

	if ( !__xuiContextValid(pContext) || (sName == NULL) || (sName[0] == '\0') ) {
		return NULL;
	}
	for ( i = 0; i < pContext->iFontCount; i++ ) {
		if ( strcmp(pContext->pFonts[i].sName, sName) == 0 ) {
			return pContext->pFonts[i].pFont;
		}
	}
	return NULL;
}

XUI_API void xuiClearFonts(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	__xuiContextDestroyFonts(pContext);
	xuiInternalContextBumpGeneration(pContext);
}

XUI_API int xuiInvalidateRect(xui_context pContext, xui_rect_i_t tRect)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiContextAddDamage(pContext, tRect);
}

XUI_API int xuiInvalidateAll(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext->iDamageCount = 0;
	return __xuiContextAddDamage(pContext, __xuiContextFullRect(pContext));
}

XUI_API int xuiHasDamage(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return 0;
	}
	return pContext->iDamageCount > 0;
}

XUI_API int xuiGetDamageRects(xui_context pContext, xui_rect_i_t* pRects, int iCapacity)
{
	int i;
	int iCopyCount;

	if ( !__xuiContextValid(pContext) || (iCapacity < 0) || ((iCapacity > 0) && (pRects == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iCopyCount = pContext->iDamageCount;
	if ( iCopyCount > iCapacity ) {
		iCopyCount = iCapacity;
	}
	for ( i = 0; i < iCopyCount; i++ ) {
		pRects[i] = pContext->pDamage[i];
	}
	return pContext->iDamageCount;
}

XUI_API void xuiClearDamage(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	pContext->iDamageCount = 0;
}

XUI_API int xuiResourceSet(xui_context pContext, xui_resource* ppResource, const xui_resource_desc_t* pDesc)
{
	xui_resource pResource;
	xui_resource pExisting;
	xui_resource pPrev;
	char* sName;

	if ( !__xuiContextValid(pContext) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->sName == NULL) || (pDesc->sName[0] == '\0') || (pDesc->iKind <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( ppResource != NULL ) {
		*ppResource = NULL;
	}
	sName = __xuiCoreStringDuplicate(pDesc->sName);
	if ( sName == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPrev = NULL;
	for ( pExisting = pContext->pResources; pExisting != NULL; pExisting = pExisting->pNext ) {
		if ( strcmp(pExisting->sName, pDesc->sName) == 0 ) {
			if ( pPrev != NULL ) {
				pPrev->pNext = pExisting->pNext;
			} else {
				pContext->pResources = pExisting->pNext;
			}
			__xuiResourceDestroyOne(pExisting);
			break;
		}
		pPrev = pExisting;
	}
	pResource = (xui_resource)xrtCalloc(1, sizeof(*pResource));
	if ( pResource == NULL ) {
		xrtFree(sName);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pResource->iMagic = XUI_RESOURCE_MAGIC;
	pResource->pContext = pContext;
	pResource->sName = sName;
	pResource->iKind = pDesc->iKind;
	pResource->pHandle = pDesc->pHandle;
	pResource->pUser = pDesc->pUser;
	pResource->onDestroy = pDesc->onDestroy;
	pResource->iGeneration = pContext->iNextResourceGeneration++;
	pResource->iRefCount = 1;
	if ( pContext->iNextResourceGeneration == 0 ) {
		pContext->iNextResourceGeneration = 1;
	}
	pResource->pNext = pContext->pResources;
	pContext->pResources = pResource;
	__xuiContextBumpGeneration(pContext);
	if ( ppResource != NULL ) {
		*ppResource = pResource;
	}
	return XUI_OK;
}

XUI_API xui_resource xuiResourceFind(xui_context pContext, const char* sName)
{
	xui_resource pResource;

	if ( !__xuiContextValid(pContext) || (sName == NULL) || (sName[0] == '\0') ) {
		return NULL;
	}
	for ( pResource = pContext->pResources; pResource != NULL; pResource = pResource->pNext ) {
		if ( strcmp(pResource->sName, sName) == 0 ) {
			return pResource;
		}
	}
	return NULL;
}

XUI_API int xuiResourceRemove(xui_resource pResource)
{
	xui_context pContext;
	xui_resource pScan;
	xui_resource pPrev;

	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = pResource->pContext;
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pPrev = NULL;
	for ( pScan = pContext->pResources; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan == pResource ) {
			if ( pPrev != NULL ) {
				pPrev->pNext = pScan->pNext;
			} else {
				pContext->pResources = pScan->pNext;
			}
			__xuiResourceRemoveDependencyReferences(pContext, pScan);
			__xuiResourceDestroyOne(pScan);
			__xuiContextBumpGeneration(pContext);
			return XUI_OK;
		}
		pPrev = pScan;
	}
	return XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiResourceTouch(xui_resource pResource)
{
	xui_context pContext;
	xui_resource pScan;
	xui_resource_dependency_t* pDependency;

	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = pResource->pContext;
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pResource->iGeneration = pContext->iNextResourceGeneration++;
	if ( pContext->iNextResourceGeneration == 0 ) {
		pContext->iNextResourceGeneration = 1;
	}
	for ( pScan = pContext->pResources; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan == pResource ) {
			continue;
		}
		for ( pDependency = pScan->pDependencies; pDependency != NULL; pDependency = pDependency->pNext ) {
			if ( pDependency->pResource == pResource ) {
				pScan->iGeneration = pContext->iNextResourceGeneration++;
				if ( pContext->iNextResourceGeneration == 0 ) {
					pContext->iNextResourceGeneration = 1;
				}
				break;
			}
		}
	}
	__xuiContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API const char* xuiResourceGetName(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->sName : NULL;
}

XUI_API int xuiResourceGetKind(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->iKind : 0;
}

XUI_API void* xuiResourceGetHandle(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->pHandle : NULL;
}

XUI_API uint32_t xuiResourceGetGeneration(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->iGeneration : 0;
}

XUI_API int xuiResourceAddRef(xui_resource pResource)
{
	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pResource->iRefCount++;
	if ( pResource->iRefCount <= 0 ) {
		pResource->iRefCount = 1;
		return XUI_ERROR_UNSUPPORTED;
	}
	return XUI_OK;
}

XUI_API int xuiResourceRelease(xui_resource pResource)
{
	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pResource->iRefCount--;
	if ( pResource->iRefCount > 0 ) {
		return XUI_OK;
	}
	return xuiResourceRemove(pResource);
}

XUI_API int xuiResourceGetRefCount(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->iRefCount : 0;
}

XUI_API int xuiResourceAddDependency(xui_resource pResource, xui_resource pDependency)
{
	xui_resource_dependency_t* pScan;
	xui_resource_dependency_t* pNewDependency;

	if ( !__xuiResourceValid(pResource) ||
	     !__xuiResourceValid(pDependency) ||
	     (pResource->pContext != pDependency->pContext) ||
	     (pResource == pDependency) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( pScan = pResource->pDependencies; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan->pResource == pDependency ) {
			return XUI_OK;
		}
	}
	pNewDependency = (xui_resource_dependency_t*)xrtCalloc(1, sizeof(*pNewDependency));
	if ( pNewDependency == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pNewDependency->pResource = pDependency;
	pNewDependency->pNext = pResource->pDependencies;
	pResource->pDependencies = pNewDependency;
	return xuiResourceTouch(pResource);
}

XUI_API void xuiResourceClearDependencies(xui_resource pResource)
{
	xui_resource_dependency_t* pDependency;
	xui_resource_dependency_t* pNext;

	if ( !__xuiResourceValid(pResource) ) {
		return;
	}
	pDependency = pResource->pDependencies;
	while ( pDependency != NULL ) {
		pNext = pDependency->pNext;
		xrtFree(pDependency);
		pDependency = pNext;
	}
	pResource->pDependencies = NULL;
	(void)xuiResourceTouch(pResource);
}

XUI_API int xuiResourceGetDependencyCount(xui_resource pResource)
{
	xui_resource_dependency_t* pDependency;
	int iCount;

	if ( !__xuiResourceValid(pResource) ) {
		return 0;
	}
	iCount = 0;
	for ( pDependency = pResource->pDependencies; pDependency != NULL; pDependency = pDependency->pNext ) {
		iCount++;
	}
	return iCount;
}

XUI_API xui_resource xuiResourceGetDependency(xui_resource pResource, int iIndex)
{
	xui_resource_dependency_t* pDependency;
	int i;

	if ( !__xuiResourceValid(pResource) || (iIndex < 0) ) {
		return NULL;
	}
	i = 0;
	for ( pDependency = pResource->pDependencies; pDependency != NULL; pDependency = pDependency->pNext ) {
		if ( i == iIndex ) {
			return pDependency->pResource;
		}
		i++;
	}
	return NULL;
}

XUI_API int xuiPainterBegin(xui_context pContext, xui_surface pTarget, xui_painter* ppPainter)
{
	xui_painter pPainter;
	xui_proxy pProxy;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pTarget == NULL) || (ppPainter == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppPainter = NULL;
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	pPainter = (xui_painter)xrtCalloc(1, sizeof(*pPainter));
	if ( pPainter == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPainter->iMagic = XUI_PAINTER_MAGIC;
	pPainter->pContext = pContext;
	pPainter->pTarget = pTarget;
	iRet = pProxy->drawBegin(pProxy, &pPainter->pDraw, pTarget);
	if ( iRet != XUI_OK ) {
		pPainter->iMagic = 0;
		xrtFree(pPainter);
		return iRet;
	}
	*ppPainter = pPainter;
	return XUI_OK;
}

XUI_API int xuiPainterEnd(xui_painter pPainter)
{
	xui_proxy pProxy;
	int iRet;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	if ( pProxy == NULL ) {
		iRet = XUI_ERROR_NOT_INITIALIZED;
	} else {
		iRet = pProxy->drawEnd(pProxy, pPainter->pDraw);
	}
	pPainter->iMagic = 0;
	xrtFree(pPainter);
	return iRet;
}

XUI_API xui_draw_context xuiPainterGetDrawContext(xui_painter pPainter)
{
	return __xuiPainterValid(pPainter) ? pPainter->pDraw : NULL;
}

XUI_API int xuiPainterClearRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRectOut(tRect);
	return (pProxy != NULL) ? pProxy->drawClearRect(pProxy, pPainter->pDraw, tRect, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawSurface(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pSurface == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( ((iColor & 0xffu) == 0u) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tDst = xuiInternalSnapRect(tDst);
	return (pProxy != NULL) ? pProxy->drawSurface(pProxy, pPainter->pDraw, pSurface, tSrc, tDst, iColor, iFlags) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawSurfaceQuad(xui_painter pPainter, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pSurface == NULL) || (pVertices == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( ((pVertices[0].iColor | pVertices[1].iColor | pVertices[2].iColor | pVertices[3].iColor) & 0xffu) == 0u ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	return (pProxy != NULL) ? pProxy->drawSurfaceQuad(pProxy, pPainter->pDraw, pSurface, pVertices, iFlags) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawMeshTriangles(xui_painter pPainter, const xui_mesh_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pVertices == NULL) || (iVertexCount <= 0) ||
	     (pIndices == NULL) || (iIndexCount <= 0) || ((iIndexCount % 3) != 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	{
		int i;
		uint32_t iAlphaMask;

		iAlphaMask = 0u;
		for ( i = 0; i < iVertexCount; i++ ) {
			iAlphaMask |= pVertices[i].iColor;
		}
		if ( (iAlphaMask & 0xffu) == 0u ) {
			return XUI_OK;
		}
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( pProxy->drawMeshTriangles == NULL ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	return pProxy->drawMeshTriangles(pProxy, pPainter->pDraw, pVertices, iVertexCount, pIndices, iIndexCount, iFlags);
}

XUI_API int xuiPainterFillPath(xui_painter pPainter, xui_path pPath, uint32_t iColor, float fTolerance)
{
	xui_mesh_vertex_t* pVertices;
	uint32_t* pIndices;
	int iVertexCount;
	int iIndexCount;
	int iRet;

	if ( !__xuiPainterValid(pPainter) || !__xuiPathValid(pPath) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iColor & 0xffu) == 0u ) {
		return XUI_OK;
	}
	iRet = xuiPathBuildFillMesh(pPath, NULL, 0, NULL, 0, iColor, fTolerance, &iVertexCount, &iIndexCount);
	if ( iRet == XUI_OK && (iVertexCount == 0 || iIndexCount == 0) ) {
		return XUI_OK;
	}
	if ( iRet != XUI_ERROR_BUFFER_TOO_SMALL ) {
		return iRet;
	}
	pVertices = (xui_mesh_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
	pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		if ( pVertices != NULL ) xrtFree(pVertices);
		if ( pIndices != NULL ) xrtFree(pIndices);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = xuiPathBuildFillMesh(pPath, pVertices, iVertexCount, pIndices, iIndexCount, iColor, fTolerance, &iVertexCount, &iIndexCount);
	if ( iRet == XUI_OK ) {
		iRet = xuiPainterDrawMeshTriangles(pPainter, pVertices, iVertexCount, pIndices, iIndexCount, 0);
	}
	xrtFree(pIndices);
	xrtFree(pVertices);
	return iRet;
}

XUI_API int xuiPainterDrawPath(xui_painter pPainter, xui_path pPath, const xui_path_style_t* pStyle, float fTolerance)
{
	xui_mesh_vertex_t* pVertices;
	uint32_t* pIndices;
	int iVertexCount;
	int iIndexCount;
	int iRet;

	if ( !__xuiPainterValid(pPainter) || !__xuiPathValid(pPath) || (pStyle == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle->iFillColor & 0xffu) != 0u ) {
		iRet = __xuiPathBuildFillMeshWithRule(pPath, NULL, 0, NULL, 0, pStyle->iFillColor, pStyle->iFillRule, fTolerance, &iVertexCount, &iIndexCount);
		if ( iRet == XUI_OK && (iVertexCount == 0 || iIndexCount == 0) ) {
			return XUI_OK;
		}
		if ( iRet != XUI_ERROR_BUFFER_TOO_SMALL ) {
			return iRet;
		}
		pVertices = (xui_mesh_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
		pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
		if ( (pVertices == NULL) || (pIndices == NULL) ) {
			if ( pVertices != NULL ) xrtFree(pVertices);
			if ( pIndices != NULL ) xrtFree(pIndices);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		iRet = __xuiPathBuildFillMeshWithRule(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->iFillColor, pStyle->iFillRule, fTolerance, &iVertexCount, &iIndexCount);
		if ( iRet == XUI_OK ) {
			iRet = xuiPainterDrawMeshTriangles(pPainter, pVertices, iVertexCount, pIndices, iIndexCount, 0);
		}
		xrtFree(pIndices);
		xrtFree(pVertices);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( ((pStyle->iStrokeColor & 0xffu) != 0u) && (pStyle->fStrokeWidth > 0.0f) ) {
		iRet = __xuiPathBuildDashedStrokeMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, pStyle->iStrokeColor, pStyle->iLineJoin, pStyle->iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
		if ( iRet == XUI_OK && (iVertexCount == 0 || iIndexCount == 0) ) {
			return XUI_OK;
		}
		if ( iRet != XUI_ERROR_BUFFER_TOO_SMALL ) {
			return iRet;
		}
		pVertices = (xui_mesh_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
		pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
		if ( (pVertices == NULL) || (pIndices == NULL) ) {
			if ( pVertices != NULL ) xrtFree(pVertices);
			if ( pIndices != NULL ) xrtFree(pIndices);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		iRet = __xuiPathBuildDashedStrokeMeshWithStyle(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->fStrokeWidth, pStyle->iStrokeColor, pStyle->iLineJoin, pStyle->iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
		if ( iRet == XUI_OK ) {
			iRet = xuiPainterDrawMeshTriangles(pPainter, pVertices, iVertexCount, pIndices, iIndexCount, 0);
		}
		xrtFree(pIndices);
		xrtFree(pVertices);
		return iRet;
	}
	(void)pStyle;
	return XUI_OK;
}

XUI_API int xuiPainterFillRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || ((iColor & 0xffu) == 0u) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	return (pProxy != NULL) ? pProxy->drawRectFill(pProxy, pPainter->pDraw, tRect, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterStrokeRect(xui_painter pPainter, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || ((iColor & 0xffu) == 0u) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	fWidth = xuiInternalSnapSize(fWidth);
	return (pProxy != NULL) ? pProxy->drawRectStroke(pProxy, pPainter->pDraw, tRect, fWidth, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterFillRoundRect(xui_painter pPainter, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || ((iColor & 0xffu) == 0u) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	fRadius = xuiInternalSnapPixel(fRadius);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( fRadius <= 0.0f ) {
		return pProxy->drawRectFill(pProxy, pPainter->pDraw, tRect, iColor);
	}
	return pProxy->drawRoundRectFill(pProxy, pPainter->pDraw, tRect, fRadius, iColor);
}

XUI_API int xuiPainterStrokeRoundRect(xui_painter pPainter, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || ((iColor & 0xffu) == 0u) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	fWidth = xuiInternalSnapSize(fWidth);
	tRect = xuiInternalSnapRect(tRect);
	fRadius = xuiInternalSnapPixel(fRadius);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( fRadius <= 0.0f ) {
		return pProxy->drawRectStroke(pProxy, pPainter->pDraw, tRect, fWidth, iColor);
	}
	return pProxy->drawRoundRectStroke(pProxy, pPainter->pDraw, tRect, fRadius, fWidth, iColor);
}

XUI_API int xuiVectorIconGetCount(void)
{
	return (int)(sizeof(g_arrXuiVectorIcons) / sizeof(g_arrXuiVectorIcons[0]));
}

XUI_API const char* xuiVectorIconGetName(int iIndex)
{
	if ( (iIndex < 0) || (iIndex >= xuiVectorIconGetCount()) ) {
		return NULL;
	}
	return g_arrXuiVectorIcons[iIndex].sName;
}

static const xui_vector_icon_def_t* __xuiVectorIconFind(const char* sName)
{
	int i;

	if ( sName == NULL ) {
		return NULL;
	}
	for ( i = 0; i < xuiVectorIconGetCount(); i++ ) {
		if ( strcmp(g_arrXuiVectorIcons[i].sName, sName) == 0 ) {
			return &g_arrXuiVectorIcons[i];
		}
	}
	return NULL;
}

static xui_vec2_t __xuiVectorIconMapPoint(xui_vec2_t tPoint, float fScale, float fOffsetX, float fOffsetY)
{
	tPoint.fX = fOffsetX + (tPoint.fX * fScale);
	tPoint.fY = fOffsetY + (tPoint.fY * fScale);
	return tPoint;
}

static int __xuiVectorIconBuildPath(const xui_vector_icon_def_t* pIcon, xui_rect_t tRect, xui_path* ppPath)
{
	xui_path pSource;
	xui_path pTarget;
	xui_path_command_t tCommand;
	float fScale;
	float fIconW;
	float fIconH;
	float fOffsetX;
	float fOffsetY;
	int i;
	int iRet;

	if ( (pIcon == NULL) || (ppPath == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppPath = NULL;
	pSource = NULL;
	pTarget = NULL;
	iRet = xuiPathCreate(&pSource);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPathParseSvg(pSource, pIcon->sPath);
	if ( iRet != XUI_OK ) {
		xuiPathDestroy(pSource);
		return iRet;
	}
	iRet = xuiPathCreate(&pTarget);
	if ( iRet != XUI_OK ) {
		xuiPathDestroy(pSource);
		return iRet;
	}
	fScale = (tRect.fW / pIcon->fViewWidth < tRect.fH / pIcon->fViewHeight) ? (tRect.fW / pIcon->fViewWidth) : (tRect.fH / pIcon->fViewHeight);
	fIconW = pIcon->fViewWidth * fScale;
	fIconH = pIcon->fViewHeight * fScale;
	fOffsetX = tRect.fX + (tRect.fW - fIconW) * 0.5f;
	fOffsetY = tRect.fY + (tRect.fH - fIconH) * 0.5f;
	for ( i = 0; i < xuiPathGetCommandCount(pSource); i++ ) {
		iRet = xuiPathGetCommand(pSource, i, &tCommand);
		if ( iRet != XUI_OK ) break;
		if ( tCommand.iCommand == XUI_PATH_CMD_MOVE ) {
			xui_vec2_t p = __xuiVectorIconMapPoint(tCommand.arrPoints[0], fScale, fOffsetX, fOffsetY);
			iRet = xuiPathMoveTo(pTarget, p.fX, p.fY);
		} else if ( tCommand.iCommand == XUI_PATH_CMD_LINE ) {
			xui_vec2_t p = __xuiVectorIconMapPoint(tCommand.arrPoints[0], fScale, fOffsetX, fOffsetY);
			iRet = xuiPathLineTo(pTarget, p.fX, p.fY);
		} else if ( tCommand.iCommand == XUI_PATH_CMD_QUAD ) {
			xui_vec2_t c = __xuiVectorIconMapPoint(tCommand.arrPoints[0], fScale, fOffsetX, fOffsetY);
			xui_vec2_t p = __xuiVectorIconMapPoint(tCommand.arrPoints[1], fScale, fOffsetX, fOffsetY);
			iRet = xuiPathQuadTo(pTarget, c.fX, c.fY, p.fX, p.fY);
		} else if ( tCommand.iCommand == XUI_PATH_CMD_CUBIC ) {
			xui_vec2_t c1 = __xuiVectorIconMapPoint(tCommand.arrPoints[0], fScale, fOffsetX, fOffsetY);
			xui_vec2_t c2 = __xuiVectorIconMapPoint(tCommand.arrPoints[1], fScale, fOffsetX, fOffsetY);
			xui_vec2_t p = __xuiVectorIconMapPoint(tCommand.arrPoints[2], fScale, fOffsetX, fOffsetY);
			iRet = xuiPathCubicTo(pTarget, c1.fX, c1.fY, c2.fX, c2.fY, p.fX, p.fY);
		} else if ( tCommand.iCommand == XUI_PATH_CMD_CLOSE ) {
			iRet = xuiPathClose(pTarget);
		}
		if ( iRet != XUI_OK ) break;
	}
	xuiPathDestroy(pSource);
	if ( iRet != XUI_OK ) {
		xuiPathDestroy(pTarget);
		return iRet;
	}
	*ppPath = pTarget;
	return XUI_OK;
}

XUI_API int xuiPainterDrawVectorIcon(xui_painter pPainter, const char* sName, xui_rect_t tRect, uint32_t iColor)
{
	const xui_vector_icon_def_t* pIcon;
	xui_path pPath;
	xui_path_style_t tStyle;
	float fScale;
	int iRet;

	if ( !__xuiPainterValid(pPainter) || (sName == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pIcon = __xuiVectorIconFind(sName);
	if ( pIcon == NULL ) {
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	if ( (iColor & 0xffu) == 0u ) {
		return XUI_OK;
	}
	pPath = NULL;
	iRet = __xuiVectorIconBuildPath(pIcon, tRect, &pPath);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	fScale = (tRect.fW / pIcon->fViewWidth < tRect.fH / pIcon->fViewHeight) ? (tRect.fW / pIcon->fViewWidth) : (tRect.fH / pIcon->fViewHeight);
	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iStrokeColor = iColor;
	tStyle.fStrokeWidth = 2.0f * fScale;
	if ( tStyle.fStrokeWidth < 1.0f ) {
		tStyle.fStrokeWidth = 1.0f;
	}
	tStyle.iLineJoin = XUI_PATH_JOIN_ROUND;
	tStyle.iLineCap = XUI_PATH_CAP_ROUND;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tStyle, 1.0f);
	xuiPathDestroy(pPath);
	return iRet;
}

XUI_API int xuiPainterDrawText(xui_painter pPainter, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pFont == NULL) || (sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (sText[0] == '\0') || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || ((iColor & 0xffu) == 0u) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	return (pProxy != NULL) ? pProxy->drawText(pProxy, pPainter->pDraw, pFont, sText, tRect, iColor, iFlags) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawNinePatch(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, xui_thickness_t tSlice, uint32_t iColor, uint32_t iFlags)
{
	xui_rect_t arrSrc[9];
	xui_rect_t arrDst[9];
	float arrSrcX[4];
	float arrSrcY[4];
	float arrDstX[4];
	float arrDstY[4];
	float fSrcLeft;
	float fSrcTop;
	float fSrcRight;
	float fSrcBottom;
	float fDstLeft;
	float fDstTop;
	float fDstRight;
	float fDstBottom;
	int x;
	int y;
	int iIndex;
	int iRet;

	if ( !__xuiPainterValid(pPainter) || (pSurface == NULL) ||
	     (tSrc.fW < 0.0f) || (tSrc.fH < 0.0f) || (tDst.fW < 0.0f) || (tDst.fH < 0.0f) ||
	     (tSlice.fLeft < 0.0f) || (tSlice.fTop < 0.0f) || (tSlice.fRight < 0.0f) || (tSlice.fBottom < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( ((iColor & 0xffu) == 0u) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XUI_OK;
	}
	fSrcLeft = tSlice.fLeft;
	fSrcTop = tSlice.fTop;
	fSrcRight = tSlice.fRight;
	fSrcBottom = tSlice.fBottom;
	if ( fSrcLeft + fSrcRight > tSrc.fW ) {
		fSrcRight = tSrc.fW - fSrcLeft;
		if ( fSrcRight < 0.0f ) {
			fSrcRight = 0.0f;
			fSrcLeft = tSrc.fW;
		}
	}
	if ( fSrcTop + fSrcBottom > tSrc.fH ) {
		fSrcBottom = tSrc.fH - fSrcTop;
		if ( fSrcBottom < 0.0f ) {
			fSrcBottom = 0.0f;
			fSrcTop = tSrc.fH;
		}
	}
	fDstLeft = fSrcLeft;
	fDstTop = fSrcTop;
	fDstRight = fSrcRight;
	fDstBottom = fSrcBottom;
	if ( fDstLeft + fDstRight > tDst.fW ) {
		fDstRight = tDst.fW - fDstLeft;
		if ( fDstRight < 0.0f ) {
			fDstRight = 0.0f;
			fDstLeft = tDst.fW;
		}
	}
	if ( fDstTop + fDstBottom > tDst.fH ) {
		fDstBottom = tDst.fH - fDstTop;
		if ( fDstBottom < 0.0f ) {
			fDstBottom = 0.0f;
			fDstTop = tDst.fH;
		}
	}
	arrSrcX[0] = tSrc.fX;
	arrSrcX[1] = tSrc.fX + fSrcLeft;
	arrSrcX[2] = tSrc.fX + tSrc.fW - fSrcRight;
	arrSrcX[3] = tSrc.fX + tSrc.fW;
	arrSrcY[0] = tSrc.fY;
	arrSrcY[1] = tSrc.fY + fSrcTop;
	arrSrcY[2] = tSrc.fY + tSrc.fH - fSrcBottom;
	arrSrcY[3] = tSrc.fY + tSrc.fH;
	arrDstX[0] = tDst.fX;
	arrDstX[1] = tDst.fX + fDstLeft;
	arrDstX[2] = tDst.fX + tDst.fW - fDstRight;
	arrDstX[3] = tDst.fX + tDst.fW;
	arrDstY[0] = tDst.fY;
	arrDstY[1] = tDst.fY + fDstTop;
	arrDstY[2] = tDst.fY + tDst.fH - fDstBottom;
	arrDstY[3] = tDst.fY + tDst.fH;
	for ( y = 0; y < 3; y++ ) {
		for ( x = 0; x < 3; x++ ) {
			iIndex = (y * 3) + x;
			arrSrc[iIndex].fX = arrSrcX[x];
			arrSrc[iIndex].fY = arrSrcY[y];
			arrSrc[iIndex].fW = arrSrcX[x + 1] - arrSrcX[x];
			arrSrc[iIndex].fH = arrSrcY[y + 1] - arrSrcY[y];
			arrDst[iIndex].fX = arrDstX[x];
			arrDst[iIndex].fY = arrDstY[y];
			arrDst[iIndex].fW = arrDstX[x + 1] - arrDstX[x];
			arrDst[iIndex].fH = arrDstY[y + 1] - arrDstY[y];
		}
	}
	for ( iIndex = 0; iIndex < 9; iIndex++ ) {
		if ( (arrSrc[iIndex].fW <= 0.0f) || (arrSrc[iIndex].fH <= 0.0f) ||
		     (arrDst[iIndex].fW <= 0.0f) || (arrDst[iIndex].fH <= 0.0f) ) {
			continue;
		}
		iRet = xuiPainterDrawSurface(pPainter, pSurface, arrSrc[iIndex], arrDst[iIndex], iColor, iFlags);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

XUI_API int xuiPathCreate(xui_path* ppPath)
{
	xui_path pPath;

	if ( ppPath == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppPath = NULL;
	pPath = (xui_path)xrtCalloc(1, sizeof(*pPath));
	if ( pPath == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPath->iMagic = XUI_PATH_MAGIC;
	*ppPath = pPath;
	return XUI_OK;
}

XUI_API void xuiPathDestroy(xui_path pPath)
{
	if ( !__xuiPathValid(pPath) ) {
		return;
	}
	if ( pPath->pCommands != NULL ) {
		xrtFree(pPath->pCommands);
	}
	pPath->iMagic = 0;
	xrtFree(pPath);
}

XUI_API int xuiPathClear(xui_path pPath)
{
	if ( !__xuiPathValid(pPath) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pPath->iCommandCount = 0;
	return XUI_OK;
}

XUI_API int xuiPathMoveTo(xui_path pPath, float fX, float fY)
{
	return __xuiPathAddCommand(pPath, XUI_PATH_CMD_MOVE, (xui_vec2_t){fX, fY}, (xui_vec2_t){0.0f, 0.0f}, (xui_vec2_t){0.0f, 0.0f});
}

XUI_API int xuiPathLineTo(xui_path pPath, float fX, float fY)
{
	return __xuiPathAddCommand(pPath, XUI_PATH_CMD_LINE, (xui_vec2_t){fX, fY}, (xui_vec2_t){0.0f, 0.0f}, (xui_vec2_t){0.0f, 0.0f});
}

XUI_API int xuiPathQuadTo(xui_path pPath, float fCX, float fCY, float fX, float fY)
{
	return __xuiPathAddCommand(pPath, XUI_PATH_CMD_QUAD, (xui_vec2_t){fCX, fCY}, (xui_vec2_t){fX, fY}, (xui_vec2_t){0.0f, 0.0f});
}

XUI_API int xuiPathCubicTo(xui_path pPath, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY)
{
	return __xuiPathAddCommand(pPath, XUI_PATH_CMD_CUBIC, (xui_vec2_t){fC1X, fC1Y}, (xui_vec2_t){fC2X, fC2Y}, (xui_vec2_t){fX, fY});
}

XUI_API int xuiPathClose(xui_path pPath)
{
	return __xuiPathAddCommand(pPath, XUI_PATH_CMD_CLOSE, (xui_vec2_t){0.0f, 0.0f}, (xui_vec2_t){0.0f, 0.0f}, (xui_vec2_t){0.0f, 0.0f});
}

static int __xuiPathSvgIsSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f');
}

static int __xuiPathSvgIsCommand(char c)
{
	return (c == 'M') || (c == 'm') || (c == 'L') || (c == 'l') ||
	       (c == 'H') || (c == 'h') || (c == 'V') || (c == 'v') ||
	       (c == 'Q') || (c == 'q') || (c == 'C') || (c == 'c') ||
	       (c == 'Z') || (c == 'z');
}

static int __xuiPathSvgCommandLower(char c)
{
	if ( (c >= 'A') && (c <= 'Z') ) {
		return c - 'A' + 'a';
	}
	return c;
}

static int __xuiPathSvgCommandRelative(char c)
{
	return (c >= 'a') && (c <= 'z');
}

static void __xuiPathSvgSkipSeparators(const char** ppText)
{
	const char* pText;

	pText = *ppText;
	while ( (*pText == ',') || __xuiPathSvgIsSpace(*pText) ) {
		pText++;
	}
	*ppText = pText;
}

static int __xuiPathSvgHasNumber(const char* sText)
{
	__xuiPathSvgSkipSeparators(&sText);
	return (*sText != '\0') && !__xuiPathSvgIsCommand(*sText);
}

static int __xuiPathSvgReadNumber(const char** ppText, float* pValue)
{
	const char* pText;
	char* pEnd;
	double fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	pText = *ppText;
	__xuiPathSvgSkipSeparators(&pText);
	if ( (*pText == '\0') || __xuiPathSvgIsCommand(*pText) ) {
		return 0;
	}
	fValue = strtod(pText, &pEnd);
	if ( pEnd == pText ) {
		return 0;
	}
	*pValue = (float)fValue;
	*ppText = pEnd;
	return 1;
}

XUI_API int xuiPathParseSvg(xui_path pPath, const char* sPath)
{
	const char* pText;
	xui_vec2_t tCurrent;
	xui_vec2_t tStart;
	char cCommand;
	int bHasCommand;
	int iRet;

	if ( !__xuiPathValid(pPath) || (sPath == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiPathClear(pPath);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pText = sPath;
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	cCommand = 0;
	bHasCommand = 0;
	for ( ;; ) {
		int bRelative;
		int iCommand;

		__xuiPathSvgSkipSeparators(&pText);
		if ( *pText == '\0' ) {
			return XUI_OK;
		}
		if ( __xuiPathSvgIsCommand(*pText) ) {
			cCommand = *pText++;
			bHasCommand = 1;
		} else if ( !bHasCommand ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		bRelative = __xuiPathSvgCommandRelative(cCommand);
		iCommand = __xuiPathSvgCommandLower(cCommand);
		if ( iCommand == 'z' ) {
			iRet = xuiPathClose(pPath);
			if ( iRet != XUI_OK ) return iRet;
			tCurrent = tStart;
			bHasCommand = 0;
			continue;
		}
		if ( iCommand == 'm' ) {
			float fX;
			float fY;
			int bFirst;

			if ( !__xuiPathSvgReadNumber(&pText, &fX) || !__xuiPathSvgReadNumber(&pText, &fY) ) {
				return XUI_ERROR_INVALID_ARGUMENT;
			}
			if ( bRelative ) {
				fX += tCurrent.fX;
				fY += tCurrent.fY;
			}
			iRet = xuiPathMoveTo(pPath, fX, fY);
			if ( iRet != XUI_OK ) return iRet;
			tCurrent = (xui_vec2_t){fX, fY};
			tStart = tCurrent;
			bFirst = 0;
			while ( __xuiPathSvgHasNumber(pText) ) {
				if ( !__xuiPathSvgReadNumber(&pText, &fX) || !__xuiPathSvgReadNumber(&pText, &fY) ) {
					return XUI_ERROR_INVALID_ARGUMENT;
				}
				if ( bRelative ) {
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xuiPathLineTo(pPath, fX, fY);
				if ( iRet != XUI_OK ) return iRet;
				tCurrent = (xui_vec2_t){fX, fY};
				bFirst = 1;
			}
			(void)bFirst;
		} else if ( iCommand == 'l' ) {
			while ( __xuiPathSvgHasNumber(pText) ) {
				float fX;
				float fY;
				if ( !__xuiPathSvgReadNumber(&pText, &fX) || !__xuiPathSvgReadNumber(&pText, &fY) ) return XUI_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xuiPathLineTo(pPath, fX, fY);
				if ( iRet != XUI_OK ) return iRet;
				tCurrent = (xui_vec2_t){fX, fY};
			}
		} else if ( iCommand == 'h' ) {
			while ( __xuiPathSvgHasNumber(pText) ) {
				float fX;
				if ( !__xuiPathSvgReadNumber(&pText, &fX) ) return XUI_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) fX += tCurrent.fX;
				iRet = xuiPathLineTo(pPath, fX, tCurrent.fY);
				if ( iRet != XUI_OK ) return iRet;
				tCurrent.fX = fX;
			}
		} else if ( iCommand == 'v' ) {
			while ( __xuiPathSvgHasNumber(pText) ) {
				float fY;
				if ( !__xuiPathSvgReadNumber(&pText, &fY) ) return XUI_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) fY += tCurrent.fY;
				iRet = xuiPathLineTo(pPath, tCurrent.fX, fY);
				if ( iRet != XUI_OK ) return iRet;
				tCurrent.fY = fY;
			}
		} else if ( iCommand == 'q' ) {
			while ( __xuiPathSvgHasNumber(pText) ) {
				float fCX;
				float fCY;
				float fX;
				float fY;
				if ( !__xuiPathSvgReadNumber(&pText, &fCX) || !__xuiPathSvgReadNumber(&pText, &fCY) ||
				     !__xuiPathSvgReadNumber(&pText, &fX) || !__xuiPathSvgReadNumber(&pText, &fY) ) return XUI_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fCX += tCurrent.fX;
					fCY += tCurrent.fY;
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xuiPathQuadTo(pPath, fCX, fCY, fX, fY);
				if ( iRet != XUI_OK ) return iRet;
				tCurrent = (xui_vec2_t){fX, fY};
			}
		} else if ( iCommand == 'c' ) {
			while ( __xuiPathSvgHasNumber(pText) ) {
				float fC1X;
				float fC1Y;
				float fC2X;
				float fC2Y;
				float fX;
				float fY;
				if ( !__xuiPathSvgReadNumber(&pText, &fC1X) || !__xuiPathSvgReadNumber(&pText, &fC1Y) ||
				     !__xuiPathSvgReadNumber(&pText, &fC2X) || !__xuiPathSvgReadNumber(&pText, &fC2Y) ||
				     !__xuiPathSvgReadNumber(&pText, &fX) || !__xuiPathSvgReadNumber(&pText, &fY) ) return XUI_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fC1X += tCurrent.fX;
					fC1Y += tCurrent.fY;
					fC2X += tCurrent.fX;
					fC2Y += tCurrent.fY;
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xuiPathCubicTo(pPath, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
				if ( iRet != XUI_OK ) return iRet;
				tCurrent = (xui_vec2_t){fX, fY};
			}
		} else {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
}

XUI_API int xuiPathGetCommandCount(xui_path pPath)
{
	return __xuiPathValid(pPath) ? pPath->iCommandCount : 0;
}

XUI_API int xuiPathGetCommand(xui_path pPath, int iIndex, xui_path_command_t* pCommand)
{
	if ( !__xuiPathValid(pPath) || (pCommand == NULL) || (iIndex < 0) || (iIndex >= pPath->iCommandCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pCommand = pPath->pCommands[iIndex];
	return XUI_OK;
}

XUI_API int xuiPathFlatten(xui_path pPath, xui_vec2_t* pPoints, int iCapacity, float fTolerance)
{
	xui_path_command_t* pCommand;
	xui_vec2_t tCurrent;
	xui_vec2_t tStart;
	xui_vec2_t tPoint;
	float t;
	float u;
	int bHasCurrent;
	int iCount;
	int iSteps;
	int i;
	int j;

	if ( !__xuiPathValid(pPath) || (iCapacity < 0) || ((pPoints == NULL) && (iCapacity > 0)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	bHasCurrent = 0;
	iCount = 0;
	iSteps = __xuiPathFlattenSteps(fTolerance);
	for ( i = 0; i < pPath->iCommandCount; i++ ) {
		pCommand = &pPath->pCommands[i];
		if ( pCommand->iCommand == XUI_PATH_CMD_MOVE ) {
			tCurrent = pCommand->arrPoints[0];
			tStart = tCurrent;
			bHasCurrent = 1;
			__xuiPathFlattenAdd(pPoints, iCapacity, &iCount, tCurrent);
		} else if ( (pCommand->iCommand == XUI_PATH_CMD_LINE) && bHasCurrent ) {
			tCurrent = pCommand->arrPoints[0];
			__xuiPathFlattenAdd(pPoints, iCapacity, &iCount, tCurrent);
		} else if ( (pCommand->iCommand == XUI_PATH_CMD_QUAD) && bHasCurrent ) {
			for ( j = 1; j <= iSteps; j++ ) {
				t = (float)j / (float)iSteps;
				u = 1.0f - t;
				tPoint.fX = u * u * tCurrent.fX + 2.0f * u * t * pCommand->arrPoints[0].fX + t * t * pCommand->arrPoints[1].fX;
				tPoint.fY = u * u * tCurrent.fY + 2.0f * u * t * pCommand->arrPoints[0].fY + t * t * pCommand->arrPoints[1].fY;
				__xuiPathFlattenAdd(pPoints, iCapacity, &iCount, tPoint);
			}
			tCurrent = pCommand->arrPoints[1];
		} else if ( (pCommand->iCommand == XUI_PATH_CMD_CUBIC) && bHasCurrent ) {
			for ( j = 1; j <= iSteps; j++ ) {
				t = (float)j / (float)iSteps;
				u = 1.0f - t;
				tPoint.fX = u * u * u * tCurrent.fX + 3.0f * u * u * t * pCommand->arrPoints[0].fX + 3.0f * u * t * t * pCommand->arrPoints[1].fX + t * t * t * pCommand->arrPoints[2].fX;
				tPoint.fY = u * u * u * tCurrent.fY + 3.0f * u * u * t * pCommand->arrPoints[0].fY + 3.0f * u * t * t * pCommand->arrPoints[1].fY + t * t * t * pCommand->arrPoints[2].fY;
				__xuiPathFlattenAdd(pPoints, iCapacity, &iCount, tPoint);
			}
			tCurrent = pCommand->arrPoints[2];
		} else if ( (pCommand->iCommand == XUI_PATH_CMD_CLOSE) && bHasCurrent ) {
			if ( (tCurrent.fX != tStart.fX) || (tCurrent.fY != tStart.fY) ) {
				__xuiPathFlattenAdd(pPoints, iCapacity, &iCount, tStart);
			}
			tCurrent = tStart;
		}
	}
	return iCount;
}

static int __xuiPathFillRuleValid(int iFillRule)
{
	return (iFillRule == XUI_PATH_FILL_NON_ZERO) || (iFillRule == XUI_PATH_FILL_EVEN_ODD);
}

static int __xuiPathBuildFillMeshWithRule(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	xui_vec2_t* pPoints;
	int iPointCount;
	int iTriangleCount;
	int i;

	if ( !__xuiPathValid(pPath) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) && (iVertexCapacity > 0)) || ((pIndices == NULL) && (iIndexCapacity > 0)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiPathFillRuleValid(iFillRule) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	iPointCount = xuiPathFlatten(pPath, NULL, 0, fTolerance);
	if ( iPointCount < 3 ) {
		return XUI_OK;
	}
	pPoints = (xui_vec2_t*)xrtMalloc(sizeof(*pPoints) * (size_t)iPointCount);
	if ( pPoints == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	(void)xuiPathFlatten(pPath, pPoints, iPointCount, fTolerance);
	if ( (iPointCount > 1) &&
	     (pPoints[iPointCount - 1].fX == pPoints[0].fX) &&
	     (pPoints[iPointCount - 1].fY == pPoints[0].fY) ) {
		iPointCount--;
	}
	if ( iPointCount < 3 ) {
		xrtFree(pPoints);
		return XUI_OK;
	}
	iTriangleCount = iPointCount - 2;
	*pVertexCount = iPointCount;
	*pIndexCount = iTriangleCount * 3;
	if ( (iVertexCapacity < iPointCount) || (iIndexCapacity < iTriangleCount * 3) ) {
		xrtFree(pPoints);
		return XUI_ERROR_BUFFER_TOO_SMALL;
	}
	for ( i = 0; i < iPointCount; i++ ) {
		pVertices[i].fX = pPoints[i].fX;
		pVertices[i].fY = pPoints[i].fY;
		pVertices[i].iColor = iColor;
	}
	for ( i = 0; i < iTriangleCount; i++ ) {
		pIndices[(i * 3) + 0] = 0u;
		pIndices[(i * 3) + 1] = (uint32_t)(i + 1);
		pIndices[(i * 3) + 2] = (uint32_t)(i + 2);
	}
	xrtFree(pPoints);
	return XUI_OK;
}

XUI_API int xuiPathBuildFillMesh(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return __xuiPathBuildFillMeshWithRule(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, XUI_PATH_FILL_NON_ZERO, fTolerance, pVertexCount, pIndexCount);
}

#define XUI_PATH_ROUND_CAP_SEGMENTS 8
#define XUI_PATH_ROUND_JOIN_SEGMENTS 8

static int __xuiPathStrokeSegmentVertexCount(int iLineCap)
{
	return (iLineCap == XUI_PATH_CAP_ROUND) ? (4 + ((XUI_PATH_ROUND_CAP_SEGMENTS + 2) * 2)) : 4;
}

static int __xuiPathStrokeSegmentIndexCount(int iLineCap)
{
	return (iLineCap == XUI_PATH_CAP_ROUND) ? (6 + (XUI_PATH_ROUND_CAP_SEGMENTS * 6)) : 6;
}

static int __xuiPathStrokeJoinVertexCount(int iLineJoin)
{
	if ( iLineJoin == XUI_PATH_JOIN_ROUND ) {
		return XUI_PATH_ROUND_JOIN_SEGMENTS + 2;
	}
	return (iLineJoin == XUI_PATH_JOIN_MITER) ? 4 : 3;
}

static int __xuiPathStrokeJoinIndexCount(int iLineJoin)
{
	if ( iLineJoin == XUI_PATH_JOIN_ROUND ) {
		return XUI_PATH_ROUND_JOIN_SEGMENTS * 3;
	}
	return (iLineJoin == XUI_PATH_JOIN_MITER) ? 6 : 3;
}

static void __xuiPathStrokeWriteRoundCap(xui_mesh_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xui_vec2_t tCenter, float fUX, float fUY, float fNX, float fNY, int bEndCap, uint32_t iColor)
{
	int iCenter;
	int iArcBase;
	int i;

	iCenter = *pVertexWrite;
	iArcBase = iCenter + 1;
	pVertices[iCenter] = (xui_mesh_vertex_t){tCenter.fX, tCenter.fY, iColor};
	for ( i = 0; i <= XUI_PATH_ROUND_CAP_SEGMENTS; i++ ) {
		float fT = (float)i / (float)XUI_PATH_ROUND_CAP_SEGMENTS;
		float fAngle = fT * 3.14159265358979323846f;
		float fC = cosf(fAngle);
		float fS = sinf(fAngle);
		float fX;
		float fY;

		if ( bEndCap ) {
			fX = tCenter.fX - (fNX * fC) + (fUX * fS);
			fY = tCenter.fY - (fNY * fC) + (fUY * fS);
		} else {
			fX = tCenter.fX + (fNX * fC) - (fUX * fS);
			fY = tCenter.fY + (fNY * fC) - (fUY * fS);
		}
		pVertices[iArcBase + i] = (xui_mesh_vertex_t){fX, fY, iColor};
	}
	for ( i = 0; i < XUI_PATH_ROUND_CAP_SEGMENTS; i++ ) {
		pIndices[*pIndexWrite + 0] = (uint32_t)iCenter;
		pIndices[*pIndexWrite + 1] = (uint32_t)(iArcBase + i);
		pIndices[*pIndexWrite + 2] = (uint32_t)(iArcBase + i + 1);
		*pIndexWrite += 3;
	}
	*pVertexWrite += XUI_PATH_ROUND_CAP_SEGMENTS + 2;
}

static void __xuiPathStrokeWriteSegment(xui_mesh_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xui_vec2_t tA, xui_vec2_t tB, float fHalf, uint32_t iColor, int iLineCap)
{
	float fDX = tB.fX - tA.fX;
	float fDY = tB.fY - tA.fY;
	float fLen = __xuiPathSqrt(fDX * fDX + fDY * fDY);
	float fUX;
	float fUY;
	float fNX;
	float fNY;
	int iBase;

	if ( fLen <= 0.000001f ) {
		return;
	}
	fUX = (fDX / fLen) * fHalf;
	fUY = (fDY / fLen) * fHalf;
	fNX = (-fDY / fLen) * fHalf;
	fNY = (fDX / fLen) * fHalf;
	if ( iLineCap == XUI_PATH_CAP_SQUARE ) {
		tA.fX -= fUX;
		tA.fY -= fUY;
		tB.fX += fUX;
		tB.fY += fUY;
	}
	iBase = *pVertexWrite;
	pVertices[iBase + 0] = (xui_mesh_vertex_t){tA.fX + fNX, tA.fY + fNY, iColor};
	pVertices[iBase + 1] = (xui_mesh_vertex_t){tA.fX - fNX, tA.fY - fNY, iColor};
	pVertices[iBase + 2] = (xui_mesh_vertex_t){tB.fX + fNX, tB.fY + fNY, iColor};
	pVertices[iBase + 3] = (xui_mesh_vertex_t){tB.fX - fNX, tB.fY - fNY, iColor};
	pIndices[*pIndexWrite + 0] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 1] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 3] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 4] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 5] = (uint32_t)(iBase + 3);
	*pVertexWrite += 4;
	*pIndexWrite += 6;
	if ( iLineCap == XUI_PATH_CAP_ROUND ) {
		__xuiPathStrokeWriteRoundCap(pVertices, pIndices, pVertexWrite, pIndexWrite, tA, fUX, fUY, fNX, fNY, 0, iColor);
		__xuiPathStrokeWriteRoundCap(pVertices, pIndices, pVertexWrite, pIndexWrite, tB, fUX, fUY, fNX, fNY, 1, iColor);
	}
}

static int __xuiPathStrokeLineIntersection(xui_vec2_t tP0, float fD0X, float fD0Y, xui_vec2_t tP1, float fD1X, float fD1Y, xui_vec2_t* pOut)
{
	float fDenom;
	float fT;

	fDenom = (fD0X * fD1Y) - (fD0Y * fD1X);
	if ( (fDenom > -0.000001f) && (fDenom < 0.000001f) ) {
		return 0;
	}
	fT = (((tP1.fX - tP0.fX) * fD1Y) - ((tP1.fY - tP0.fY) * fD1X)) / fDenom;
	pOut->fX = tP0.fX + (fD0X * fT);
	pOut->fY = tP0.fY + (fD0Y * fT);
	return 1;
}

static void __xuiPathStrokeWriteBevelJoin(xui_mesh_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xui_vec2_t tPoint, xui_vec2_t tOuterA, xui_vec2_t tOuterB, uint32_t iColor)
{
	int iBase;

	iBase = *pVertexWrite;
	pVertices[iBase + 0] = (xui_mesh_vertex_t){tPoint.fX, tPoint.fY, iColor};
	pVertices[iBase + 1] = (xui_mesh_vertex_t){tOuterA.fX, tOuterA.fY, iColor};
	pVertices[iBase + 2] = (xui_mesh_vertex_t){tOuterB.fX, tOuterB.fY, iColor};
	pIndices[*pIndexWrite + 0] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 1] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 2] = (uint32_t)(iBase + 2);
	*pVertexWrite += 3;
	*pIndexWrite += 3;
}

static void __xuiPathStrokeWriteMiterJoin(xui_mesh_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xui_vec2_t tPoint, xui_vec2_t tOuterA, xui_vec2_t tOuterB, xui_vec2_t tMiter, uint32_t iColor)
{
	int iBase;

	iBase = *pVertexWrite;
	pVertices[iBase + 0] = (xui_mesh_vertex_t){tPoint.fX, tPoint.fY, iColor};
	pVertices[iBase + 1] = (xui_mesh_vertex_t){tOuterA.fX, tOuterA.fY, iColor};
	pVertices[iBase + 2] = (xui_mesh_vertex_t){tMiter.fX, tMiter.fY, iColor};
	pVertices[iBase + 3] = (xui_mesh_vertex_t){tOuterB.fX, tOuterB.fY, iColor};
	pIndices[*pIndexWrite + 0] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 1] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 3] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 4] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 5] = (uint32_t)(iBase + 3);
	*pVertexWrite += 4;
	*pIndexWrite += 6;
}

static void __xuiPathStrokeWriteRoundJoin(xui_mesh_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xui_vec2_t tPoint, float fHalf, float fStartAngle, float fEndAngle, uint32_t iColor)
{
	int iCenter;
	int iArcBase;
	int i;

	iCenter = *pVertexWrite;
	iArcBase = iCenter + 1;
	pVertices[iCenter] = (xui_mesh_vertex_t){tPoint.fX, tPoint.fY, iColor};
	for ( i = 0; i <= XUI_PATH_ROUND_JOIN_SEGMENTS; i++ ) {
		float fT = (float)i / (float)XUI_PATH_ROUND_JOIN_SEGMENTS;
		float fAngle = fStartAngle + ((fEndAngle - fStartAngle) * fT);
		pVertices[iArcBase + i] = (xui_mesh_vertex_t){tPoint.fX + (cosf(fAngle) * fHalf), tPoint.fY + (sinf(fAngle) * fHalf), iColor};
	}
	for ( i = 0; i < XUI_PATH_ROUND_JOIN_SEGMENTS; i++ ) {
		pIndices[*pIndexWrite + 0] = (uint32_t)iCenter;
		pIndices[*pIndexWrite + 1] = (uint32_t)(iArcBase + i);
		pIndices[*pIndexWrite + 2] = (uint32_t)(iArcBase + i + 1);
		*pIndexWrite += 3;
	}
	*pVertexWrite += XUI_PATH_ROUND_JOIN_SEGMENTS + 2;
}

static void __xuiPathStrokeWriteJoin(xui_mesh_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xui_vec2_t tPrev, xui_vec2_t tPoint, xui_vec2_t tNext, float fHalf, uint32_t iColor, int iLineJoin)
{
	float fD0X;
	float fD0Y;
	float fD1X;
	float fD1Y;
	float fLen0;
	float fLen1;
	float fN0X;
	float fN0Y;
	float fN1X;
	float fN1Y;
	float fCross;
	float fSide;
	float fStartAngle;
	float fEndAngle;
	xui_vec2_t tOuterA;
	xui_vec2_t tOuterB;
	xui_vec2_t tMiter;

	fD0X = tPoint.fX - tPrev.fX;
	fD0Y = tPoint.fY - tPrev.fY;
	fD1X = tNext.fX - tPoint.fX;
	fD1Y = tNext.fY - tPoint.fY;
	fLen0 = __xuiPathSqrt((fD0X * fD0X) + (fD0Y * fD0Y));
	fLen1 = __xuiPathSqrt((fD1X * fD1X) + (fD1Y * fD1Y));
	if ( (fLen0 <= 0.000001f) || (fLen1 <= 0.000001f) ) {
		return;
	}
	fD0X /= fLen0;
	fD0Y /= fLen0;
	fD1X /= fLen1;
	fD1Y /= fLen1;
	fCross = (fD0X * fD1Y) - (fD0Y * fD1X);
	if ( (fCross > -0.000001f) && (fCross < 0.000001f) ) {
		return;
	}
	fN0X = -fD0Y;
	fN0Y = fD0X;
	fN1X = -fD1Y;
	fN1Y = fD1X;
	fSide = (fCross > 0.0f) ? -1.0f : 1.0f;
	tOuterA = (xui_vec2_t){tPoint.fX + (fN0X * fHalf * fSide), tPoint.fY + (fN0Y * fHalf * fSide)};
	tOuterB = (xui_vec2_t){tPoint.fX + (fN1X * fHalf * fSide), tPoint.fY + (fN1Y * fHalf * fSide)};
	if ( iLineJoin == XUI_PATH_JOIN_ROUND ) {
		fStartAngle = atan2f(tOuterA.fY - tPoint.fY, tOuterA.fX - tPoint.fX);
		fEndAngle = atan2f(tOuterB.fY - tPoint.fY, tOuterB.fX - tPoint.fX);
		if ( fCross > 0.0f ) {
			while ( fEndAngle > fStartAngle ) fEndAngle -= 6.28318530717958647692f;
		} else {
			while ( fEndAngle < fStartAngle ) fEndAngle += 6.28318530717958647692f;
		}
		__xuiPathStrokeWriteRoundJoin(pVertices, pIndices, pVertexWrite, pIndexWrite, tPoint, fHalf, fStartAngle, fEndAngle, iColor);
		return;
	}
	if ( iLineJoin == XUI_PATH_JOIN_MITER ) {
		xui_vec2_t tLineA = {tPoint.fX + (fN0X * fHalf * fSide), tPoint.fY + (fN0Y * fHalf * fSide)};
		xui_vec2_t tLineB = {tPoint.fX + (fN1X * fHalf * fSide), tPoint.fY + (fN1Y * fHalf * fSide)};
		if ( __xuiPathStrokeLineIntersection(tLineA, fD0X, fD0Y, tLineB, fD1X, fD1Y, &tMiter) ) {
			float fMX = tMiter.fX - tPoint.fX;
			float fMY = tMiter.fY - tPoint.fY;
			float fMLen = __xuiPathSqrt((fMX * fMX) + (fMY * fMY));
			if ( fMLen <= (fHalf * 4.0f) ) {
				__xuiPathStrokeWriteMiterJoin(pVertices, pIndices, pVertexWrite, pIndexWrite, tPoint, tOuterA, tOuterB, tMiter, iColor);
				return;
			}
		}
	}
	__xuiPathStrokeWriteBevelJoin(pVertices, pIndices, pVertexWrite, pIndexWrite, tPoint, tOuterA, tOuterB, iColor);
}

static int __xuiPathDashPatternValid(const float* pDashPattern, int iDashCount, float* pTotal)
{
	float fTotal;
	int i;

	if ( pTotal != NULL ) {
		*pTotal = 0.0f;
	}
	if ( (pDashPattern == NULL) || (iDashCount <= 0) ) {
		return 1;
	}
	fTotal = 0.0f;
	for ( i = 0; i < iDashCount; i++ ) {
		if ( pDashPattern[i] <= 0.0f ) {
			return 0;
		}
		fTotal += pDashPattern[i];
	}
	if ( fTotal <= 0.0f ) {
		return 0;
	}
	if ( pTotal != NULL ) {
		*pTotal = fTotal;
	}
	return 1;
}

static void __xuiPathDashAdvance(const float* pDashPattern, int iDashCount, int* pDashIndex, float* pDashRemaining, int* pDashOn, float fAmount)
{
	while ( fAmount > 0.000001f ) {
		if ( fAmount < *pDashRemaining ) {
			*pDashRemaining -= fAmount;
			return;
		}
		fAmount -= *pDashRemaining;
		*pDashIndex = (*pDashIndex + 1) % iDashCount;
		*pDashRemaining = pDashPattern[*pDashIndex];
		*pDashOn = ((*pDashIndex % 2) == 0);
	}
}

static int __xuiPathBuildDashedStrokeMeshWithStyle(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, int iLineJoin, int iLineCap, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	xui_vec2_t* pPoints;
	float fHalf;
	float fDashTotal;
	float fDashRemaining;
	int iPointCount;
	int iSegmentCount;
	int iJoinCount;
	int iVertexWrite;
	int iIndexWrite;
	int iVertexCount;
	int iIndexCount;
	int iDashIndex;
	int bDashOn;
	int bDashed;
	int i;

	if ( !__xuiPathValid(pPath) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (fWidth <= 0.0f) || (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) && (iVertexCapacity > 0)) || ((pIndices == NULL) && (iIndexCapacity > 0)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iLineCap != XUI_PATH_CAP_BUTT) && (iLineCap != XUI_PATH_CAP_SQUARE) && (iLineCap != XUI_PATH_CAP_ROUND) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iLineJoin != XUI_PATH_JOIN_MITER) && (iLineJoin != XUI_PATH_JOIN_BEVEL) && (iLineJoin != XUI_PATH_JOIN_ROUND) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiPathDashPatternValid(pDashPattern, iDashCount, &fDashTotal) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	iPointCount = xuiPathFlatten(pPath, NULL, 0, fTolerance);
	if ( iPointCount < 2 ) {
		return XUI_OK;
	}
	pPoints = (xui_vec2_t*)xrtMalloc(sizeof(*pPoints) * (size_t)iPointCount);
	if ( pPoints == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	(void)xuiPathFlatten(pPath, pPoints, iPointCount, fTolerance);
	bDashed = (pDashPattern != NULL) && (iDashCount > 0);
	iDashIndex = 0;
	fDashRemaining = bDashed ? pDashPattern[0] : 0.0f;
	bDashOn = 1;
	if ( bDashed && (fDashOffset > 0.0f) ) {
		while ( fDashOffset >= fDashTotal ) {
			fDashOffset -= fDashTotal;
		}
		__xuiPathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fDashOffset);
	}
	iSegmentCount = 0;
	for ( i = 1; i < iPointCount; i++ ) {
		float fDX = pPoints[i].fX - pPoints[i - 1].fX;
		float fDY = pPoints[i].fY - pPoints[i - 1].fY;
		float fLen = __xuiPathSqrt(fDX * fDX + fDY * fDY);
		if ( fLen <= 0.000001f ) {
			continue;
		}
		if ( !bDashed ) {
			iSegmentCount++;
		} else {
			float fRemain = fLen;
			while ( fRemain > 0.000001f ) {
				float fStep = (fRemain < fDashRemaining) ? fRemain : fDashRemaining;
				if ( bDashOn ) {
					iSegmentCount++;
				}
				__xuiPathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fStep);
				fRemain -= fStep;
			}
		}
	}
	if ( iSegmentCount <= 0 ) {
		xrtFree(pPoints);
		return XUI_OK;
	}
	iJoinCount = 0;
	if ( !bDashed ) {
		for ( i = 1; i < (iPointCount - 1); i++ ) {
			float fD0X = pPoints[i].fX - pPoints[i - 1].fX;
			float fD0Y = pPoints[i].fY - pPoints[i - 1].fY;
			float fD1X = pPoints[i + 1].fX - pPoints[i].fX;
			float fD1Y = pPoints[i + 1].fY - pPoints[i].fY;
			float fLen0 = __xuiPathSqrt((fD0X * fD0X) + (fD0Y * fD0Y));
			float fLen1 = __xuiPathSqrt((fD1X * fD1X) + (fD1Y * fD1Y));
			if ( (fLen0 > 0.000001f) && (fLen1 > 0.000001f) ) {
				float fCross;
				fD0X /= fLen0;
				fD0Y /= fLen0;
				fD1X /= fLen1;
				fD1Y /= fLen1;
				fCross = (fD0X * fD1Y) - (fD0Y * fD1X);
				if ( (fCross > -0.000001f) && (fCross < 0.000001f) ) {
					continue;
				}
				iJoinCount++;
			}
		}
	}
	iVertexCount = (iSegmentCount * __xuiPathStrokeSegmentVertexCount(iLineCap)) + (iJoinCount * __xuiPathStrokeJoinVertexCount(iLineJoin));
	iIndexCount = (iSegmentCount * __xuiPathStrokeSegmentIndexCount(iLineCap)) + (iJoinCount * __xuiPathStrokeJoinIndexCount(iLineJoin));
	*pVertexCount = iVertexCount;
	*pIndexCount = iIndexCount;
	if ( (iVertexCapacity < iVertexCount) || (iIndexCapacity < iIndexCount) ) {
		xrtFree(pPoints);
		return XUI_ERROR_BUFFER_TOO_SMALL;
	}
	fHalf = fWidth * 0.5f;
	iVertexWrite = 0;
	iIndexWrite = 0;
	iDashIndex = 0;
	fDashRemaining = bDashed ? pDashPattern[0] : 0.0f;
	bDashOn = 1;
	if ( bDashed && (fDashOffset > 0.0f) ) {
		while ( fDashOffset >= fDashTotal ) {
			fDashOffset -= fDashTotal;
		}
		__xuiPathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fDashOffset);
	}
	iSegmentCount = 0;
	for ( i = 1; i < iPointCount; i++ ) {
		xui_vec2_t tA = pPoints[i - 1];
		xui_vec2_t tB = pPoints[i];
		float fDX = tB.fX - tA.fX;
		float fDY = tB.fY - tA.fY;
		float fLen = __xuiPathSqrt(fDX * fDX + fDY * fDY);
		if ( fLen <= 0.000001f ) {
			continue;
		}
		if ( !bDashed ) {
			__xuiPathStrokeWriteSegment(pVertices, pIndices, &iVertexWrite, &iIndexWrite, tA, tB, fHalf, iColor, iLineCap);
			iSegmentCount++;
		} else {
			float fDone = 0.0f;
			float fRemain = fLen;
			while ( fRemain > 0.000001f ) {
				float fStep = (fRemain < fDashRemaining) ? fRemain : fDashRemaining;
				if ( bDashOn ) {
					float fT0 = fDone / fLen;
					float fT1 = (fDone + fStep) / fLen;
					xui_vec2_t tS = {tA.fX + fDX * fT0, tA.fY + fDY * fT0};
					xui_vec2_t tE = {tA.fX + fDX * fT1, tA.fY + fDY * fT1};
					__xuiPathStrokeWriteSegment(pVertices, pIndices, &iVertexWrite, &iIndexWrite, tS, tE, fHalf, iColor, iLineCap);
					iSegmentCount++;
				}
				__xuiPathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fStep);
				fDone += fStep;
				fRemain -= fStep;
			}
		}
	}
	if ( !bDashed ) {
		for ( i = 1; i < (iPointCount - 1); i++ ) {
			__xuiPathStrokeWriteJoin(pVertices, pIndices, &iVertexWrite, &iIndexWrite, pPoints[i - 1], pPoints[i], pPoints[i + 1], fHalf, iColor, iLineJoin);
		}
	}
	*pVertexCount = iVertexWrite;
	*pIndexCount = iIndexWrite;
	xrtFree(pPoints);
	return XUI_OK;
}

XUI_API int xuiPathBuildDashedStrokeMesh(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return __xuiPathBuildDashedStrokeMeshWithStyle(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, fWidth, iColor, XUI_PATH_JOIN_MITER, XUI_PATH_CAP_BUTT, pDashPattern, iDashCount, fDashOffset, fTolerance, pVertexCount, pIndexCount);
}

XUI_API int xuiPathBuildStrokeMesh(xui_path pPath, xui_mesh_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return xuiPathBuildDashedStrokeMesh(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, fWidth, iColor, NULL, 0, 0.0f, fTolerance, pVertexCount, pIndexCount);
}

static int __xuiCoreEffectiveCachePolicy(xui_widget pWidget)
{
	if ( pWidget->tCachePolicy.iPolicy == XUI_CACHE_POLICY_AUTO ) {
		return (pWidget->iChildCount > 0) ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
	}
	if ( pWidget->tCachePolicy.iPolicy == XUI_CACHE_POLICY_SUBTREE_TILED ) {
		return XUI_CACHE_POLICY_SUBTREE;
	}
	if ( pWidget->tCachePolicy.iPolicy == XUI_CACHE_POLICY_DISPLAY_LIST ) {
		return XUI_CACHE_POLICY_SELF;
	}
	return pWidget->tCachePolicy.iPolicy;
}

static xui_widget_cache_slot_t* __xuiCoreFindCacheSlot(xui_widget pWidget, uint32_t iStateId)
{
	int i;

	if ( pWidget == NULL ) {
		return NULL;
	}
	for ( i = 0; i < pWidget->iCacheCount; i++ ) {
		if ( pWidget->pCacheSlots[i].iStateId == iStateId ) {
			return &pWidget->pCacheSlots[i];
		}
	}
	return NULL;
}

static int __xuiCoreFloatRectValid(xui_rect_t tRect)
{
	return (tRect.fW > 0.0f) && (tRect.fH > 0.0f);
}

static xui_rect_t __xuiCoreRectFromInt(xui_rect_i_t tRect)
{
	return xuiInternalRectFromInt(tRect);
}

static xui_rect_t __xuiCoreIntersectRect(xui_rect_t tA, xui_rect_t tB)
{
	xui_rect_t tRet;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	fLeft = (tA.fX > tB.fX) ? tA.fX : tB.fX;
	fTop = (tA.fY > tB.fY) ? tA.fY : tB.fY;
	fRight = ((tA.fX + tA.fW) < (tB.fX + tB.fW)) ? (tA.fX + tA.fW) : (tB.fX + tB.fW);
	fBottom = ((tA.fY + tA.fH) < (tB.fY + tB.fH)) ? (tA.fY + tA.fH) : (tB.fY + tB.fH);
	tRet.fX = fLeft;
	tRet.fY = fTop;
	tRet.fW = fRight - fLeft;
	tRet.fH = fBottom - fTop;
	if ( tRet.fW < 0.0f ) {
		tRet.fW = 0.0f;
	}
	if ( tRet.fH < 0.0f ) {
		tRet.fH = 0.0f;
	}
	return tRet;
}

static int __xuiCoreClipDamageToTarget(const xui_surface_desc_t* pDesc, xui_rect_i_t* pRect)
{
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;

	iLeft = pRect->iX;
	iTop = pRect->iY;
	iRight = pRect->iX + pRect->iW;
	iBottom = pRect->iY + pRect->iH;
	if ( iLeft < 0 ) {
		iLeft = 0;
	}
	if ( iTop < 0 ) {
		iTop = 0;
	}
	if ( iRight > pDesc->iWidth ) {
		iRight = pDesc->iWidth;
	}
	if ( iBottom > pDesc->iHeight ) {
		iBottom = pDesc->iHeight;
	}
	pRect->iX = iLeft;
	pRect->iY = iTop;
	pRect->iW = iRight - iLeft;
	pRect->iH = iBottom - iTop;
	return __xuiRectValid(*pRect);
}

static int __xuiCoreRectContains(xui_rect_t tOuter, xui_rect_t tInner)
{
	return (tInner.fX >= tOuter.fX) &&
	       (tInner.fY >= tOuter.fY) &&
	       ((tInner.fX + tInner.fW) <= (tOuter.fX + tOuter.fW)) &&
	       ((tInner.fY + tInner.fH) <= (tOuter.fY + tOuter.fH));
}

static int __xuiCoreNodeCoveredBySubtreeCache(const xui_render_node_t* pNode)
{
	xui_widget pParent;
	xui_widget_cache_slot_t* pSlot;
	xui_rect_t tParentRect;

	for ( pParent = pNode->pWidget->pParent; pParent != NULL; pParent = pParent->pParent ) {
		if ( __xuiCoreEffectiveCachePolicy(pParent) != XUI_CACHE_POLICY_SUBTREE ) {
			continue;
		}
		pSlot = __xuiCoreFindCacheSlot(pParent, pParent->iStateId);
		if ( (pSlot == NULL) || (pSlot->pSurface == NULL) ) {
			continue;
		}
		tParentRect = xuiWidgetGetWorldRect(pParent);
		if ( __xuiCoreRectContains(tParentRect, pNode->tPaintRect) ) {
			return 1;
		}
	}
	return 0;
}

static int __xuiCoreDrawRenderNode(xui_context pContext, xui_surface pTarget, const xui_render_node_t* pNode, xui_rect_i_t tDamage)
{
	xui_widget_cache_slot_t* pSlot;
	xui_proxy pProxy;
	xui_rect_t tDamageRect;
	xui_rect_t tClipRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	float fScaleX;
	float fScaleY;
	int iPolicy;
	int iRet;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) || !pNode->pWidget->bVisible ) {
		return XUI_OK;
	}
	iPolicy = pNode->iCachePolicy;
	if ( iPolicy == XUI_CACHE_POLICY_NONE ) {
		return XUI_OK;
	}
	pSlot = __xuiCoreFindCacheSlot(pNode->pWidget, pNode->iStateId);
	if ( (pSlot == NULL) || (pSlot->pSurface == NULL) ||
	     (pSlot->iWidth <= 0) || (pSlot->iHeight <= 0) ||
	     (pNode->tWorldRect.fW <= 0.0f) || (pNode->tWorldRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tDamageRect = __xuiCoreRectFromInt(tDamage);
	tClipRect = __xuiCoreIntersectRect(pNode->tPaintRect, tDamageRect);
	tClipRect = xuiInternalSnapRect(tClipRect);
	if ( !__xuiCoreFloatRectValid(tClipRect) ) {
		return XUI_OK;
	}
	fScaleX = (float)pSlot->iWidth / pNode->tWorldRect.fW;
	fScaleY = (float)pSlot->iHeight / pNode->tWorldRect.fH;
	tSrc.fX = (tClipRect.fX - pNode->tWorldRect.fX) * fScaleX;
	tSrc.fY = (tClipRect.fY - pNode->tWorldRect.fY) * fScaleY;
	tSrc.fW = tClipRect.fW * fScaleX;
	tSrc.fH = tClipRect.fH * fScaleY;
	tSrc = xuiInternalSnapRect(tSrc);
	tDst = xuiInternalSnapRect(tClipRect);
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = pProxy->surfaceDrawTo(pProxy, pTarget, pSlot->pSurface, tSrc, tDst, XUI_COLOR_WHITE, 0);
	if ( iRet == XUI_OK ) {
		pContext->tRenderStats.iDrawnCaches++;
	}
	return iRet;
}

static void __xuiCoreClearRenderDirtyRecursive(xui_widget pWidget)
{
	xui_widget pChild;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->iDirtyFlags &= ~(XUI_WIDGET_DIRTY_RENDER | XUI_WIDGET_DIRTY_TREE);
	pWidget->iSubtreeDirtyFlags &= ~(XUI_WIDGET_DIRTY_RENDER | XUI_WIDGET_DIRTY_TREE);
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xuiCoreClearRenderDirtyRecursive(pChild);
	}
}

static int __xuiCoreComposeRenderTree(xui_context pContext, xui_surface pTarget, const xui_surface_desc_t* pTargetDesc, const xui_rect_i_t* pRects, int iRectCount)
{
	xui_rect_i_t tDamage;
	const xui_render_node_t* pNode;
	int i;
	int n;
	int iRet;

	for ( i = 0; i < iRectCount; i++ ) {
		tDamage = pRects[i];
		if ( !__xuiCoreClipDamageToTarget(pTargetDesc, &tDamage) ) {
			continue;
		}
		for ( n = 0; n < pContext->iRenderNodeCount; n++ ) {
			pNode = &pContext->pRenderNodes[n];
			if ( __xuiCoreNodeCoveredBySubtreeCache(pNode) ) {
				pContext->tRenderStats.iSkippedWidgets++;
				continue;
			}
			iRet = __xuiCoreDrawRenderNode(pContext, pTarget, pNode, tDamage);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	if ( pContext->pRoot != NULL ) {
		__xuiCoreClearRenderDirtyRecursive(pContext->pRoot);
	}
	if ( pContext->pOverlayRoot != NULL ) {
		__xuiCoreClearRenderDirtyRecursive(pContext->pOverlayRoot);
	}
	return XUI_OK;
}

XUI_API int xuiRender(xui_context pContext, xui_surface pTarget, const xui_rect_i_t* pRects, int iRectCount)
{
	xui_surface_desc_t tDesc;
	xui_rect_i_t tInlineRect;
	const xui_rect_i_t* pRenderRects;
	int iRenderRectCount;
	int i;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pTarget == NULL) || (iRectCount < 0) || ((iRectCount > 0) && (pRects == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pContext->bHasProxy ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	for ( i = 0; i < iRectCount; i++ ) {
		if ( !__xuiRectValid(pRects[i]) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	iRet = pContext->tProxy.surfaceGetDesc(&pContext->tProxy, pTarget, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (tDesc.iKind != XUI_SURFACE_KIND_TEXTURE) ||
	     (tDesc.iFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     ((tDesc.iFlags & XUI_SURFACE_USAGE_TARGET) == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->pRoot != NULL ) {
		iRet = xuiRenderPrepare(pContext);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		iRet = xuiBuildRenderTree(pContext);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		if ( iRectCount > 0 ) {
			pRenderRects = pRects;
			iRenderRectCount = iRectCount;
		} else if ( pContext->iDamageCount > 0 ) {
			pRenderRects = pContext->pDamage;
			iRenderRectCount = pContext->iDamageCount;
		} else {
			tInlineRect.iX = 0;
			tInlineRect.iY = 0;
			tInlineRect.iW = tDesc.iWidth;
			tInlineRect.iH = tDesc.iHeight;
			pRenderRects = &tInlineRect;
			iRenderRectCount = 1;
		}
		iRet = __xuiCoreComposeRenderTree(pContext, pTarget, &tDesc, pRenderRects, iRenderRectCount);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	pContext->iDamageCount = 0;
	return XUI_OK;
}
