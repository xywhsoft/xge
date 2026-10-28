#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

struct xui_code_margin_model_t {
	xui_code_margin_desc_t* pMargins;
	int iCount;
	int iCapacity;
};

static int __xuiCodeMarginReserve(xui_code_margin_model pModel, int iCapacity)
{
	xui_code_margin_desc_t* pNew;
	int iNewCapacity;

	if ( pModel == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pModel->iCapacity ) return XUI_OK;
	iNewCapacity = (pModel->iCapacity > 0) ? pModel->iCapacity : 8;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) {
			iNewCapacity = iCapacity;
			break;
		}
		iNewCapacity *= 2;
	}
	if ( (size_t)iNewCapacity > SIZE_MAX / sizeof(*pNew) ) return XUI_ERROR_LIMIT_EXCEEDED;
	pNew = (xui_code_margin_desc_t*)xrtRealloc(pModel->pMargins,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pModel->pMargins = pNew;
	pModel->iCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeMarginRectValid(xui_rect_t tRect)
{
	return isfinite((double)tRect.fX) && isfinite((double)tRect.fY) &&
		isfinite((double)tRect.fW) && isfinite((double)tRect.fH) &&
		tRect.fW >= 0.0f && tRect.fH >= 0.0f;
}

static int __xuiCodeMarginFind(xui_code_margin_model pModel, int iId)
{
	int i;

	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( pModel->pMargins[i].iId == iId ) return i;
	}
	return -1;
}

static int __xuiCodeMarginVisible(const xui_code_margin_desc_t* pDesc)
{
	return (pDesc->iFlags & XUI_CODE_MARGIN_VISIBLE) != 0 && pDesc->fWidth > 0.0f;
}

static void __xuiCodeMarginCopyInfo(xui_code_margin_info_t* pInfo, const xui_code_margin_desc_t* pDesc, xui_rect_t tRect)
{
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = sizeof(*pInfo);
	pInfo->iId = pDesc->iId;
	pInfo->iKind = pDesc->iKind;
	pInfo->fWidth = pDesc->fWidth;
	pInfo->iFlags = pDesc->iFlags;
	pInfo->tRect = tRect;
	pInfo->onRender = pDesc->onRender;
	pInfo->onEvent = pDesc->onEvent;
	pInfo->pUser = pDesc->pUser;
}

static xui_code_margin_desc_t __xuiCodeMarginDefault(int iId, int iKind, float fWidth, uint32_t iFlags)
{
	xui_code_margin_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iId = iId;
	tDesc.iKind = iKind;
	tDesc.fWidth = fWidth;
	tDesc.iFlags = iFlags | XUI_CODE_MARGIN_VISIBLE;
	return tDesc;
}

XUI_API int xuiCodeMarginModelCreate(xui_code_margin_model* ppModel)
{
	xui_code_margin_model pModel;

	if ( ppModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppModel = NULL;
	pModel = (xui_code_margin_model)xrtMalloc(sizeof(*pModel));
	if ( pModel == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pModel, 0, sizeof(*pModel));
	*ppModel = pModel;
	return XUI_OK;
}

XUI_API void xuiCodeMarginModelDestroy(xui_code_margin_model pModel)
{
	if ( pModel == NULL ) return;
	xrtFree(pModel->pMargins);
	xrtFree(pModel);
}

XUI_API void xuiCodeMarginModelClear(xui_code_margin_model pModel)
{
	if ( pModel == NULL ) return;
	pModel->iCount = 0;
}

XUI_API int xuiCodeMarginModelLoadDefaults(xui_code_margin_model pModel, int bLineNumber, int bMarker, int bFold, int bDiagnostic)
{
	xui_code_margin_model_t tNew;
	xui_code_margin_desc_t tDesc;
	int iRet;

	if ( pModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tNew, 0, sizeof(tNew));
	tDesc = __xuiCodeMarginDefault(1, XUI_CODE_MARGIN_CHANGE, 4.0f, 0);
	iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
	if ( iRet != XUI_OK ) goto cleanup;
	if ( bMarker ) {
		tDesc = __xuiCodeMarginDefault(2, XUI_CODE_MARGIN_MARKER, 16.0f, XUI_CODE_MARGIN_CLICKABLE);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( bFold ) {
		tDesc = __xuiCodeMarginDefault(3, XUI_CODE_MARGIN_FOLD, 14.0f, XUI_CODE_MARGIN_CLICKABLE);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( bLineNumber ) {
		tDesc = __xuiCodeMarginDefault(4, XUI_CODE_MARGIN_LINE_NUMBER, 44.0f, 0);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( bDiagnostic ) {
		tDesc = __xuiCodeMarginDefault(5, XUI_CODE_MARGIN_DIAGNOSTIC, 14.0f, XUI_CODE_MARGIN_CLICKABLE);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	xrtFree(pModel->pMargins);
	pModel->pMargins = tNew.pMargins;
	pModel->iCount = tNew.iCount;
	pModel->iCapacity = tNew.iCapacity;
	return XUI_OK;

cleanup:
	xrtFree(tNew.pMargins);
	return iRet;
}

XUI_API int xuiCodeMarginModelAdd(xui_code_margin_model pModel, const xui_code_margin_desc_t* pDesc)
{
	int iIndex;
	int iRet;

	if ( (pModel == NULL) || (pDesc == NULL) || (pDesc->iId <= 0) ||
	     (pDesc->iKind <= 0) || !isfinite((double)pDesc->fWidth) ||
	     (pDesc->fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, pDesc->iId);
	if ( iIndex >= 0 ) {
		pModel->pMargins[iIndex] = *pDesc;
		pModel->pMargins[iIndex].iSize = sizeof(pModel->pMargins[iIndex]);
		return XUI_OK;
	}
	if ( pModel->iCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	iRet = __xuiCodeMarginReserve(pModel, pModel->iCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	pModel->pMargins[pModel->iCount] = *pDesc;
	pModel->pMargins[pModel->iCount].iSize = sizeof(pModel->pMargins[pModel->iCount]);
	pModel->iCount++;
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelRemove(xui_code_margin_model pModel, int iId)
{
	int iIndex;

	if ( (pModel == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, iId);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
	memmove(&pModel->pMargins[iIndex], &pModel->pMargins[iIndex + 1], sizeof(pModel->pMargins[iIndex]) * (size_t)(pModel->iCount - iIndex - 1));
	pModel->iCount--;
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelSetWidth(xui_code_margin_model pModel, int iId, float fWidth)
{
	int iIndex;

	if ( (pModel == NULL) || (iId <= 0) || !isfinite((double)fWidth) ||
	     (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, iId);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
	pModel->pMargins[iIndex].fWidth = fWidth;
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelSetVisible(xui_code_margin_model pModel, int iId, int bVisible)
{
	int iIndex;

	if ( (pModel == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, iId);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
	if ( bVisible ) pModel->pMargins[iIndex].iFlags |= XUI_CODE_MARGIN_VISIBLE;
	else pModel->pMargins[iIndex].iFlags &= ~XUI_CODE_MARGIN_VISIBLE;
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelGetCount(xui_code_margin_model pModel)
{
	return (pModel != NULL) ? pModel->iCount : 0;
}

XUI_API int xuiCodeMarginModelGet(xui_code_margin_model pModel, int iIndex, xui_code_margin_info_t* pInfo)
{
	xui_rect_t tRect;

	if ( (pModel == NULL) || (pInfo == NULL) || (iIndex < 0) || (iIndex >= pModel->iCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tRect, 0, sizeof(tRect));
	__xuiCodeMarginCopyInfo(pInfo, &pModel->pMargins[iIndex], tRect);
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelGetTotalWidth(xui_code_margin_model pModel, float* pWidth)
{
	double fWidth;
	int i;

	if ( (pModel == NULL) || (pWidth == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fWidth = 0.0f;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( __xuiCodeMarginVisible(&pModel->pMargins[i]) ) {
			fWidth += (double)pModel->pMargins[i].fWidth;
			if ( fWidth > FLT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		}
	}
	*pWidth = (float)fWidth;
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelGetRect(xui_code_margin_model pModel, int iId, xui_rect_t tViewport, xui_rect_t* pRect)
{
	float fCurrentX;
	int i;

	if ( (pModel == NULL) || (iId <= 0) || (pRect == NULL) ||
	     !__xuiCodeMarginRectValid(tViewport) ) return XUI_ERROR_INVALID_ARGUMENT;
	fCurrentX = tViewport.fX;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( pModel->pMargins[i].iId == iId ) {
			if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) return XUI_ERROR_UNSUPPORTED;
			*pRect = (xui_rect_t){ fCurrentX, tViewport.fY,
				pModel->pMargins[i].fWidth, tViewport.fH };
			return XUI_OK;
		}
		if ( __xuiCodeMarginVisible(&pModel->pMargins[i]) ) {
			if ( (double)fCurrentX + pModel->pMargins[i].fWidth > FLT_MAX ) {
				return XUI_ERROR_LIMIT_EXCEEDED;
			}
			fCurrentX += pModel->pMargins[i].fWidth;
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeMarginModelLayout(xui_code_margin_model pModel, xui_rect_t tViewport, xui_code_margin_info_t* pInfos, int iInfoCapacity, int* pInfoCount, xui_rect_t* pTextRect)
{
	xui_rect_t tRect;
	double fX;
	double fTotal;
	int i;
	int iCount;

	if ( (pModel == NULL) || (pInfoCount == NULL) || (iInfoCapacity < 0) ||
	     (pInfos == NULL && iInfoCapacity > 0) ||
	     !__xuiCodeMarginRectValid(tViewport) ) return XUI_ERROR_INVALID_ARGUMENT;
	fX = tViewport.fX;
	iCount = 0;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) continue;
		if ( fX > FLT_MAX || fX < -FLT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		tRect = (xui_rect_t){ (float)fX, tViewport.fY,
			pModel->pMargins[i].fWidth, tViewport.fH };
		if ( pInfos != NULL && iCount < iInfoCapacity ) __xuiCodeMarginCopyInfo(&pInfos[iCount], &pModel->pMargins[i], tRect);
		fX += pModel->pMargins[i].fWidth;
		iCount++;
	}
	*pInfoCount = iCount;
	if ( pTextRect != NULL ) {
		fTotal = fX - (double)tViewport.fX;
		if ( fX > FLT_MAX || fX < -FLT_MAX || fTotal > FLT_MAX ) {
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		*pTextRect = (xui_rect_t){ (float)fX, tViewport.fY,
			(float)((double)tViewport.fW - fTotal), tViewport.fH };
		if ( pTextRect->fW < 0.0f ) pTextRect->fW = 0.0f;
	}
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelHitTest(xui_code_margin_model pModel, xui_rect_t tViewport, float fX, float fY, float fLineHeight, float fScrollY, xui_code_margin_hit_t* pHit)
{
	xui_rect_t tRect;
	double fCurrentX;
	double fLine;
	int i;

	if ( (pModel == NULL) || (pHit == NULL) ||
	     !__xuiCodeMarginRectValid(tViewport) ||
	     !isfinite((double)fX) || !isfinite((double)fY) ||
	     !isfinite((double)fLineHeight) || fLineHeight <= 0.0f ||
	     !isfinite((double)fScrollY) || fScrollY < 0.0f ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pHit, 0, sizeof(*pHit));
	pHit->iSize = sizeof(*pHit);
	if ( fY < tViewport.fY || fY >= tViewport.fY + tViewport.fH ) return XUI_ERROR_UNSUPPORTED;
	fCurrentX = tViewport.fX;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) continue;
		if ( fCurrentX > FLT_MAX || fCurrentX < -FLT_MAX ) {
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		tRect = (xui_rect_t){ (float)fCurrentX, tViewport.fY,
			pModel->pMargins[i].fWidth, tViewport.fH };
		if ( fX >= tRect.fX && fX < tRect.fX + tRect.fW ) {
			pHit->iId = pModel->pMargins[i].iId;
			pHit->iKind = pModel->pMargins[i].iKind;
			pHit->iIndex = i;
			fLine = ((double)fY - tViewport.fY + fScrollY) / fLineHeight;
			if ( fLine < 0.0 || fLine > INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
			pHit->iLine = (int)fLine;
			pHit->tRect = tRect;
			return XUI_OK;
		}
		fCurrentX += pModel->pMargins[i].fWidth;
	}
	return XUI_ERROR_UNSUPPORTED;
}

#endif
