#include "xui_config.h"
#if XUI_ENABLE_PROGRESS
#include "xui_internal.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_PROGRESS_ANIMATION_FRAME_SECONDS (1.0f / 60.0f)
#define XUI_PROGRESS_MAX_TILE_PIECES 16384u

typedef struct xui_progress_data_t {
	char* sTextTemplate;
	char* sDisplayText;
	size_t iDisplayCapacity;
	xui_font pFont;
	float fMin;
	float fMax;
	float fValue;
	uint32_t iTrackColor;
	uint32_t iFillColor;
	uint32_t iTextColor;
	uint32_t iFillTextColor;
	uint32_t iTextFlags;
	int iFillDirection;
	int iFillPatchMode;
	xui_nine_patch_t tTrackPatch;
	xui_nine_patch_t tFillPatch;
	uint32_t iTrackSurfaceGeneration;
	uint32_t iFillSurfaceGeneration;
	float fMarqueePhase;
	float fMarqueeInterval;
	int bHasTrackPatch;
	int bHasFillPatch;
	int bTrackSurfaceGenerationValid;
	int bFillSurfaceGenerationValid;
	int bTemplateFormatted;
	int bTemplateDynamic;
	int bIndeterminate;
} xui_progress_data_t;

typedef struct xui_progress_resolved_t {
	xui_font pFont;
	uint32_t iTrackColor;
	uint32_t iFillColor;
	uint32_t iTextColor;
	uint32_t iFillTextColor;
	uint32_t iTextFlags;
	int iFillDirection;
	int iFillPatchMode;
} xui_progress_resolved_t;

static uint32_t __xuiProgressColorAlpha(uint32_t iColor)
{
	return iColor & 0xffu;
}

static float __xuiProgressClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static int __xuiProgressDirectionValid(int iDirection)
{
	return (iDirection == XUI_PROGRESS_LEFT_TO_RIGHT) ||
	       (iDirection == XUI_PROGRESS_RIGHT_TO_LEFT) ||
	       (iDirection == XUI_PROGRESS_BOTTOM_TO_TOP) ||
	       (iDirection == XUI_PROGRESS_TOP_TO_BOTTOM);
}

static int __xuiProgressPatchModeValid(int iMode)
{
	return (iMode == XUI_PROGRESS_FILL_STRETCH) ||
	       (iMode == XUI_PROGRESS_FILL_REVEAL);
}

static int __xuiProgressFloatValid(float fValue)
{
	return isfinite(fValue);
}

static int __xuiProgressGeometryIntValid(int iValue)
{
	const int iBound = (int)XUI_LAYOUT_UNBOUNDED;
	return iValue >= -iBound && iValue <= iBound;
}

static int __xuiProgressNonNegativeGeometryIntValid(int iValue)
{
	return iValue >= 0 && iValue <= (int)XUI_LAYOUT_UNBOUNDED;
}

static int __xuiProgressSourceRectValid(xui_rect_t tRect)
{
	const int64_t iBound = (int64_t)XUI_LAYOUT_UNBOUNDED;
	int64_t iRight;
	int64_t iBottom;

	if ( !__xuiProgressGeometryIntValid(tRect.fX) ||
	     !__xuiProgressGeometryIntValid(tRect.fY) ||
	     !__xuiProgressNonNegativeGeometryIntValid(tRect.fW) ||
	     !__xuiProgressNonNegativeGeometryIntValid(tRect.fH) ) {
		return 0;
	}
	iRight = (int64_t)tRect.fX + (int64_t)tRect.fW;
	iBottom = (int64_t)tRect.fY + (int64_t)tRect.fH;
	return ((tRect.fW == 0) == (tRect.fH == 0)) &&
	       iRight >= -iBound && iRight <= iBound &&
	       iBottom >= -iBound && iBottom <= iBound;
}

static int __xuiProgressRectEqual(xui_rect_t tA, xui_rect_t tB)
{
	return tA.fX == tB.fX && tA.fY == tB.fY &&
	       tA.fW == tB.fW && tA.fH == tB.fH;
}

static int __xuiProgressPatchEqual(const xui_nine_patch_t* pA,
	const xui_nine_patch_t* pB)
{
	return pA != NULL && pB != NULL &&
	       pA->pSurface == pB->pSurface &&
	       __xuiProgressRectEqual(pA->tSrc, pB->tSrc) &&
	       pA->tSlice.fLeft == pB->tSlice.fLeft &&
	       pA->tSlice.fTop == pB->tSlice.fTop &&
	       pA->tSlice.fRight == pB->tSlice.fRight &&
	       pA->tSlice.fBottom == pB->tSlice.fBottom &&
	       pA->iColor == pB->iColor &&
	       pA->iMode == pB->iMode;
}

static int __xuiProgressPatchValid(const xui_nine_patch_t* pPatch)
{
	int64_t iHorizontalSlice;
	int64_t iVerticalSlice;

	if ( pPatch == NULL ) {
		return 1;
	}
	if ( (pPatch->iSize != 0) && (pPatch->iSize < sizeof(*pPatch)) ) {
		return 0;
	}
	if ( (pPatch->pSurface == NULL) ||
	     !__xuiProgressSourceRectValid(pPatch->tSrc) ||
	     !__xuiProgressNonNegativeGeometryIntValid(pPatch->tSlice.fLeft) ||
	     !__xuiProgressNonNegativeGeometryIntValid(pPatch->tSlice.fTop) ||
	     !__xuiProgressNonNegativeGeometryIntValid(pPatch->tSlice.fRight) ||
	     !__xuiProgressNonNegativeGeometryIntValid(pPatch->tSlice.fBottom) ) {
		return 0;
	}
	iHorizontalSlice = (int64_t)pPatch->tSlice.fLeft +
		(int64_t)pPatch->tSlice.fRight;
	iVerticalSlice = (int64_t)pPatch->tSlice.fTop +
		(int64_t)pPatch->tSlice.fBottom;
	if ( iHorizontalSlice > (int64_t)XUI_LAYOUT_UNBOUNDED ||
	     iVerticalSlice > (int64_t)XUI_LAYOUT_UNBOUNDED ) return 0;
	return (pPatch->iMode == XUI_NINE_PATCH_STRETCH) ||
	       (pPatch->iMode == XUI_NINE_PATCH_TILE);
}

static int __xuiProgressDescValid(const xui_progress_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( !__xuiProgressFloatValid(pDesc->fMin) ||
	     !__xuiProgressFloatValid(pDesc->fMax) ||
	     !__xuiProgressFloatValid(pDesc->fValue) ) {
		return 0;
	}
	if ( (pDesc->iFillDirection != 0) && !__xuiProgressDirectionValid(pDesc->iFillDirection) ) {
		return 0;
	}
	if ( (pDesc->iFillPatchMode != 0) && !__xuiProgressPatchModeValid(pDesc->iFillPatchMode) ) {
		return 0;
	}
	if ( (pDesc->bHasTrackPatch && !__xuiProgressPatchValid(&pDesc->tTrackPatch)) ||
	     (pDesc->bHasFillPatch && !__xuiProgressPatchValid(&pDesc->tFillPatch)) ) {
		return 0;
	}
	return 1;
}

static void __xuiProgressNormalizeRange(float* pMin, float* pMax)
{
	float fSwap;

	if ( (*pMax) < (*pMin) ) {
		fSwap = *pMin;
		*pMin = *pMax;
		*pMax = fSwap;
	}
	if ( (*pMax) == (*pMin) ) {
		*pMax = (*pMin) + 1.0f;
		if ( !isfinite(*pMax) || (*pMax == *pMin) ) {
			*pMax = nextafterf(*pMin, INFINITY);
			if ( !isfinite(*pMax) ) {
				*pMax = *pMin;
				*pMin = nextafterf(*pMax, -INFINITY);
			}
		}
	}
}

static float __xuiProgressRateData(const xui_progress_data_t* pData)
{
	double fRate;

	if ( (pData == NULL) || (pData->fMax == pData->fMin) ) {
		return 0.0f;
	}
	fRate = ((double)pData->fValue - (double)pData->fMin) /
		((double)pData->fMax - (double)pData->fMin);
	if ( fRate <= 0.0 ) return 0.0f;
	if ( fRate >= 1.0 ) return 1.0f;
	return (float)fRate;
}

static int __xuiProgressParsePercentFormat(const char* sText,
	size_t* pLength, int* pWidth, int* pPrecision, int* pZeroPad,
	char* pConversion)
{
	size_t i;
	int iWidth;
	int iPrecision;
	int bZeroPad;
	char cConversion;

	if ( (sText == NULL) || (sText[0] != '%') || (sText[1] == '%') ) {
		return 0;
	}
	i = 1u;
	bZeroPad = 0;
	if ( sText[i] == '0' ) {
		bZeroPad = 1;
		i++;
	}
	iWidth = 0;
	while ( sText[i] >= '0' && sText[i] <= '9' ) {
		if ( iWidth < 64 ) iWidth = iWidth * 10 + (sText[i] - '0');
		if ( iWidth > 64 ) iWidth = 64;
		i++;
	}
	iPrecision = -1;
	if ( sText[i] == '.' ) {
		i++;
		iPrecision = 0;
		while ( sText[i] >= '0' && sText[i] <= '9' ) {
			if ( iPrecision < 12 ) iPrecision = iPrecision * 10 + (sText[i] - '0');
			if ( iPrecision > 12 ) iPrecision = 12;
			i++;
		}
	}
	cConversion = sText[i];
	if ( cConversion != 'f' && cConversion != 'F' &&
	     cConversion != 'g' && cConversion != 'G' &&
	     cConversion != 'e' && cConversion != 'E' ) {
		return 0;
	}
	if ( pLength != NULL ) *pLength = i + 1u;
	if ( pWidth != NULL ) *pWidth = iWidth;
	if ( pPrecision != NULL ) *pPrecision = iPrecision;
	if ( pZeroPad != NULL ) *pZeroPad = bZeroPad;
	if ( pConversion != NULL ) *pConversion = cConversion;
	return 1;
}

static int __xuiProgressTemplateIsDynamic(const char* sTextTemplate)
{
	size_t i;

	if ( sTextTemplate == NULL ) {
		return 0;
	}
	for ( i = 0u; sTextTemplate[i] != 0; i++ ) {
		if ( sTextTemplate[i] == '{' ) {
			if ( sTextTemplate[i + 1] == '{' ) {
				i++;
				continue;
			}
			if ( strncmp(sTextTemplate + i, "{value}", 7u) == 0 ||
			     strncmp(sTextTemplate + i, "{min}", 5u) == 0 ||
			     strncmp(sTextTemplate + i, "{max}", 5u) == 0 ||
			     strncmp(sTextTemplate + i, "{percent}", 9u) == 0 ) {
				return 1;
			}
			continue;
		}
		if ( sTextTemplate[i] == '}' && sTextTemplate[i + 1] == '}' ) {
			i++;
			continue;
		}
		if ( sTextTemplate[i] != '%' ) {
			continue;
		}
		if ( sTextTemplate[i + 1] == '%' ) {
			i++;
			continue;
		}
		if ( __xuiProgressParsePercentFormat(sTextTemplate + i,
			NULL, NULL, NULL, NULL, NULL) ) return 1;
	}
	return 0;
}

static int __xuiProgressTemplateNeedsFormatting(const char* sTextTemplate)
{
	size_t i;

	if ( sTextTemplate == NULL ) return 0;
	if ( __xuiProgressTemplateIsDynamic(sTextTemplate) ) return 1;
	for ( i = 0u; sTextTemplate[i] != '\0'; i++ ) {
		if ( ((sTextTemplate[i] == '{' || sTextTemplate[i] == '}') &&
		      sTextTemplate[i + 1] == sTextTemplate[i]) ||
		     (sTextTemplate[i] == '%' && sTextTemplate[i + 1] == '%') ) {
			return 1;
		}
	}
	return 0;
}

static char* __xuiProgressCopyText(const char* sText)
{
	return xuiInternalStringDuplicateNullable(sText);
}

typedef struct xui_progress_text_writer_t {
	char* sText;
	size_t iLength;
	size_t iCapacity;
} xui_progress_text_writer_t;

typedef struct xui_progress_prepared_text_t {
	char* sTemplate;
	char* sDisplay;
	size_t iDisplayCapacity;
	int bFormatted;
	int bDynamic;
} xui_progress_prepared_text_t;

static int __xuiProgressWriterAppend(xui_progress_text_writer_t* pWriter,
	const char* sText, size_t iTextLength)
{
	size_t iNeed;

	if ( pWriter == NULL || (iTextLength > 0u && sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iTextLength > SIZE_MAX - pWriter->iLength - 1u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iNeed = pWriter->iLength + iTextLength + 1u;
	if ( pWriter->sText != NULL ) {
		if ( iNeed > pWriter->iCapacity ) return XUI_ERROR_OUT_OF_MEMORY;
		if ( iTextLength > 0u ) {
			memcpy(pWriter->sText + pWriter->iLength, sText, iTextLength);
		}
		pWriter->sText[pWriter->iLength + iTextLength] = '\0';
	}
	pWriter->iLength += iTextLength;
	return XUI_OK;
}

static int __xuiProgressWriterAppendNumber(xui_progress_text_writer_t* pWriter,
	double fValue)
{
	char sNumber[64];
	int iLength;

	iLength = snprintf(sNumber, sizeof(sNumber), "%.9g", fValue);
	if ( iLength < 0 || (size_t)iLength >= sizeof(sNumber) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiProgressWriterAppend(pWriter, sNumber, (size_t)iLength);
}

static int __xuiProgressFormatTemplateInto(const xui_progress_data_t* pData,
	char* sOutput, size_t iCapacity, size_t* pLength)
{
	xui_progress_text_writer_t tWriter;
	const char* sTemplate;
	char sFormat[32];
	char sNumber[128];
	size_t iTokenLength;
	size_t i;
	int iRet;
	int iWidth;
	int iPrecision;
	int bZeroPad;
	char cConversion;
	double fRate;
	double fPercent;

	if ( pData == NULL || pData->sTextTemplate == NULL || pLength == NULL ||
	     (sOutput != NULL && iCapacity == 0u) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tWriter, 0, sizeof(tWriter));
	tWriter.sText = sOutput;
	tWriter.iCapacity = iCapacity;
	if ( sOutput != NULL ) sOutput[0] = '\0';
	sTemplate = pData->sTextTemplate;
	fRate = pData->fMax != pData->fMin ?
		((double)pData->fValue - (double)pData->fMin) /
		((double)pData->fMax - (double)pData->fMin) : 0.0;
	if ( fRate < 0.0 ) fRate = 0.0;
	if ( fRate > 1.0 ) fRate = 1.0;
	fPercent = fRate * 100.0;

	for ( i = 0u; sTemplate[i] != '\0'; ) {
		if ( sTemplate[i] == '{' ) {
			if ( sTemplate[i + 1] == '{' ) {
				iRet = __xuiProgressWriterAppend(&tWriter, "{", 1u);
				i += 2u;
			} else if ( strncmp(sTemplate + i, "{value}", 7u) == 0 ) {
				iRet = __xuiProgressWriterAppendNumber(&tWriter, pData->fValue);
				i += 7u;
			} else if ( strncmp(sTemplate + i, "{min}", 5u) == 0 ) {
				iRet = __xuiProgressWriterAppendNumber(&tWriter, pData->fMin);
				i += 5u;
			} else if ( strncmp(sTemplate + i, "{max}", 5u) == 0 ) {
				iRet = __xuiProgressWriterAppendNumber(&tWriter, pData->fMax);
				i += 5u;
			} else if ( strncmp(sTemplate + i, "{percent}", 9u) == 0 ) {
				iRet = __xuiProgressWriterAppendNumber(&tWriter, fPercent);
				i += 9u;
			} else {
				iRet = __xuiProgressWriterAppend(&tWriter, sTemplate + i, 1u);
				i++;
			}
			if ( iRet != XUI_OK ) return iRet;
			continue;
		}
		if ( sTemplate[i] == '}' && sTemplate[i + 1] == '}' ) {
			iRet = __xuiProgressWriterAppend(&tWriter, "}", 1u);
			if ( iRet != XUI_OK ) return iRet;
			i += 2u;
			continue;
		}
		if ( sTemplate[i] != '%' ) {
			iRet = __xuiProgressWriterAppend(&tWriter, sTemplate + i, 1u);
			if ( iRet != XUI_OK ) return iRet;
			i++;
			continue;
		}
		if ( sTemplate[i + 1] == '%' ) {
			iRet = __xuiProgressWriterAppend(&tWriter, "%", 1u);
			if ( iRet != XUI_OK ) return iRet;
			i += 2u;
			continue;
		}
		if ( __xuiProgressParsePercentFormat(sTemplate + i, &iTokenLength,
			&iWidth, &iPrecision, &bZeroPad, &cConversion) ) {
			if ( iPrecision >= 0 && iWidth > 0 ) {
				(void)snprintf(sFormat, sizeof(sFormat), "%%%s%d.%d%c",
					bZeroPad ? "0" : "", iWidth, iPrecision, cConversion);
			} else if ( iPrecision >= 0 ) {
				(void)snprintf(sFormat, sizeof(sFormat), "%%.%d%c", iPrecision, cConversion);
			} else if ( iWidth > 0 ) {
				(void)snprintf(sFormat, sizeof(sFormat), "%%%s%d%c",
					bZeroPad ? "0" : "", iWidth, cConversion);
			} else {
				(void)snprintf(sFormat, sizeof(sFormat), "%%%c", cConversion);
			}
			iRet = snprintf(sNumber, sizeof(sNumber), sFormat, fPercent);
			if ( iRet < 0 || (size_t)iRet >= sizeof(sNumber) ) {
				return XUI_ERROR_INVALID_ARGUMENT;
			}
			iRet = __xuiProgressWriterAppend(&tWriter, sNumber, (size_t)iRet);
			if ( iRet != XUI_OK ) return iRet;
			i += iTokenLength;
			continue;
		}
		iRet = __xuiProgressWriterAppend(&tWriter, "%", 1u);
		if ( iRet != XUI_OK ) return iRet;
		i++;
	}
	*pLength = tWriter.iLength;
	return XUI_OK;
}

static int __xuiProgressCapacityForLength(size_t iLength, size_t* pCapacity)
{
	size_t iNeed;
	size_t iCapacity;

	if ( pCapacity == NULL || iLength == SIZE_MAX ) return XUI_ERROR_OUT_OF_MEMORY;
	iNeed = iLength + 1u;
	iCapacity = 64u;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > SIZE_MAX / 2u ) {
			iCapacity = iNeed;
			break;
		}
		iCapacity *= 2u;
	}
	*pCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiProgressBuildDisplayText(const xui_progress_data_t* pData,
	char** psText, size_t* pCapacity)
{
	char* sNew;
	size_t iLength;
	size_t iCapacity;
	int iRet;

	if ( pData == NULL || psText == NULL || pCapacity == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*psText = NULL;
	*pCapacity = 0u;
	if ( pData->sTextTemplate == NULL ) return XUI_OK;
	if ( pData->bTemplateFormatted ) {
		iRet = __xuiProgressFormatTemplateInto(pData, NULL, 0u, &iLength);
		if ( iRet != XUI_OK ) return iRet;
	} else {
		iLength = strlen(pData->sTextTemplate);
	}
	if ( pData->bTemplateDynamic ) {
		iRet = __xuiProgressCapacityForLength(iLength, &iCapacity);
		if ( iRet != XUI_OK ) return iRet;
	} else {
		if ( iLength == SIZE_MAX ) return XUI_ERROR_OUT_OF_MEMORY;
		iCapacity = iLength + 1u;
	}
	sNew = (char*)xrtMalloc(iCapacity);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( pData->bTemplateFormatted ) {
		iRet = __xuiProgressFormatTemplateInto(pData, sNew, iCapacity, &iLength);
		if ( iRet != XUI_OK ) {
			xrtFree(sNew);
			return iRet;
		}
	} else {
		memcpy(sNew, pData->sTextTemplate, iLength + 1u);
	}
	*psText = sNew;
	*pCapacity = iCapacity;
	return XUI_OK;
}

static void __xuiProgressPreparedTextClear(xui_progress_prepared_text_t* pPrepared)
{
	if ( pPrepared == NULL ) return;
	if ( pPrepared->sTemplate != NULL ) xrtFree(pPrepared->sTemplate);
	if ( pPrepared->sDisplay != NULL ) xrtFree(pPrepared->sDisplay);
	memset(pPrepared, 0, sizeof(*pPrepared));
}

static int __xuiProgressPrepareText(const xui_progress_data_t* pData,
	const char* sTextTemplate, xui_progress_prepared_text_t* pPrepared)
{
	xui_progress_data_t tCandidate;
	int iRet;

	if ( pData == NULL || pPrepared == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pPrepared, 0, sizeof(*pPrepared));
	pPrepared->sTemplate = __xuiProgressCopyText(sTextTemplate);
	if ( sTextTemplate != NULL && pPrepared->sTemplate == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPrepared->bFormatted = __xuiProgressTemplateNeedsFormatting(pPrepared->sTemplate);
	pPrepared->bDynamic = __xuiProgressTemplateIsDynamic(pPrepared->sTemplate);
	tCandidate = *pData;
	tCandidate.sTextTemplate = pPrepared->sTemplate;
	tCandidate.bTemplateFormatted = pPrepared->bFormatted;
	tCandidate.bTemplateDynamic = pPrepared->bDynamic;
	iRet = __xuiProgressBuildDisplayText(&tCandidate, &pPrepared->sDisplay,
		&pPrepared->iDisplayCapacity);
	if ( iRet != XUI_OK ) __xuiProgressPreparedTextClear(pPrepared);
	return iRet;
}

static void __xuiProgressCommitPreparedText(xui_progress_data_t* pData,
	xui_progress_prepared_text_t* pPrepared)
{
	if ( pData == NULL || pPrepared == NULL ) return;
	if ( pData->sTextTemplate != NULL ) xrtFree(pData->sTextTemplate);
	if ( pData->sDisplayText != NULL ) xrtFree(pData->sDisplayText);
	pData->sTextTemplate = pPrepared->sTemplate;
	pData->sDisplayText = pPrepared->sDisplay;
	pData->iDisplayCapacity = pPrepared->iDisplayCapacity;
	pData->bTemplateFormatted = pPrepared->bFormatted;
	pData->bTemplateDynamic = pPrepared->bDynamic;
	memset(pPrepared, 0, sizeof(*pPrepared));
}

static int __xuiProgressSetTextTemplateData(xui_progress_data_t* pData,
	const char* sTextTemplate)
{
	xui_progress_prepared_text_t tPrepared;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiProgressPrepareText(pData, sTextTemplate, &tPrepared);
	if ( iRet != XUI_OK ) return iRet;
	__xuiProgressCommitPreparedText(pData, &tPrepared);
	return XUI_OK;
}

static int __xuiProgressPrepareDynamicDisplay(const xui_progress_data_t* pCandidate,
	const xui_progress_data_t* pCurrent, char** psDisplay, size_t* pCapacity,
	int* pReuseCurrent)
{
	char* sDisplay;
	size_t iLength;
	size_t iCapacity;
	int iRet;

	if ( pCandidate == NULL || pCurrent == NULL || psDisplay == NULL ||
	     pCapacity == NULL || pReuseCurrent == NULL ||
	     !pCandidate->bTemplateDynamic ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiProgressFormatTemplateInto(pCandidate, NULL, 0u, &iLength);
	if ( iRet != XUI_OK ) return iRet;
	if ( pCurrent->sDisplayText != NULL &&
	     iLength < pCurrent->iDisplayCapacity ) {
		*psDisplay = pCurrent->sDisplayText;
		*pCapacity = pCurrent->iDisplayCapacity;
		*pReuseCurrent = 1;
		return XUI_OK;
	}
	iRet = __xuiProgressCapacityForLength(iLength, &iCapacity);
	if ( iRet != XUI_OK ) return iRet;
	sDisplay = (char*)xrtMalloc(iCapacity);
	if ( sDisplay == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = __xuiProgressFormatTemplateInto(pCandidate, sDisplay, iCapacity,
		&iLength);
	if ( iRet != XUI_OK ) {
		xrtFree(sDisplay);
		return iRet;
	}
	*psDisplay = sDisplay;
	*pCapacity = iCapacity;
	*pReuseCurrent = 0;
	return XUI_OK;
}

static xui_progress_data_t* __xuiProgressGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "progress");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_progress_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiProgressStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiProgressStyleInt(xui_widget pWidget, const char* sName, int* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) &&
	     ((tProperty.tValue.iType == XUI_STYLE_VALUE_INT) || (tProperty.tValue.iType == XUI_STYLE_VALUE_BOOL)) ) {
		*pValue = tProperty.tValue.iInt;
		return 1;
	}
	return 0;
}

static xui_font __xuiProgressStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_context pContext;
	xui_font pFont;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) ) {
		pContext = xuiWidgetGetContext(pWidget);
		pFont = xuiFindFont(pContext, tProperty.tValue.sText);
		if ( pFont != NULL ) {
			return pFont;
		}
	}
	return pBaseFont;
}

static void __xuiProgressResolve(xui_widget pWidget, const xui_progress_data_t* pData, xui_progress_resolved_t* pResolved)
{
	int iValue;

	memset(pResolved, 0, sizeof(*pResolved));
	pResolved->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	pResolved->iTrackColor = pData->iTrackColor;
	pResolved->iFillColor = pData->iFillColor;
	pResolved->iTextColor = pData->iTextColor;
	pResolved->iFillTextColor = pData->iFillTextColor;
	pResolved->iTextFlags = pData->iTextFlags | XUI_TEXT_CLIP;
	pResolved->iFillDirection = pData->iFillDirection;
	pResolved->iFillPatchMode = pData->iFillPatchMode;

	(void)__xuiProgressStyleColor(pWidget, "text.color", &pResolved->iTextColor);
	(void)__xuiProgressStyleColor(pWidget, "progress.track_color", &pResolved->iTrackColor);
	(void)__xuiProgressStyleColor(pWidget, "progress.fill_color", &pResolved->iFillColor);
	(void)__xuiProgressStyleColor(pWidget, "progress.text_color", &pResolved->iTextColor);
	(void)__xuiProgressStyleColor(pWidget, "progress.fill_text_color", &pResolved->iFillTextColor);
	if ( __xuiProgressStyleInt(pWidget, "text.flags", &iValue) ) {
		pResolved->iTextFlags = (uint32_t)iValue;
	}
	if ( __xuiProgressStyleInt(pWidget, "progress.text_flags", &iValue) ) {
		pResolved->iTextFlags = (uint32_t)iValue;
	}
	if ( __xuiProgressStyleInt(pWidget, "progress.fill_direction", &iValue) && __xuiProgressDirectionValid(iValue) ) {
		pResolved->iFillDirection = iValue;
	}
	if ( __xuiProgressStyleInt(pWidget, "progress.fill_patch_mode", &iValue) && __xuiProgressPatchModeValid(iValue) ) {
		pResolved->iFillPatchMode = iValue;
	}
	pResolved->pFont = __xuiProgressStyleFont(pWidget, pResolved->pFont);
	pResolved->iTextFlags |= XUI_TEXT_CLIP;
	if ( !__xuiProgressDirectionValid(pResolved->iFillDirection) ) {
		pResolved->iFillDirection = XUI_PROGRESS_LEFT_TO_RIGHT;
	}
	if ( !__xuiProgressPatchModeValid(pResolved->iFillPatchMode) ) {
		pResolved->iFillPatchMode = XUI_PROGRESS_FILL_STRETCH;
	}
}

static xui_rect_t __xuiProgressSurfaceSrc(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc)
{
	xui_surface_desc_t tDesc;

	if ( (tSrc.fW == 0) && (tSrc.fH == 0) ) {
		memset(&tDesc, 0, sizeof(tDesc));
		if ( (pProxy != NULL) &&
		     (pProxy->surfaceGetDesc != NULL) &&
		     (pProxy->surfaceGetDesc(pProxy, pSurface, &tDesc) == XUI_OK) &&
		     (tDesc.iWidth > 0) && (tDesc.iHeight > 0) &&
		     (tDesc.iWidth <= (int)XUI_LAYOUT_UNBOUNDED) &&
		     (tDesc.iHeight <= (int)XUI_LAYOUT_UNBOUNDED) ) {
			tSrc.fX = 0.0f;
			tSrc.fY = 0.0f;
			tSrc.fW = (float)tDesc.iWidth;
			tSrc.fH = (float)tDesc.iHeight;
		}
	}
	return tSrc;
}

static void __xuiProgressScaleFixed(float fTotal, float* pA, float* pB, float* pMid)
{
	float fSum;

	fSum = *pA + *pB;
	if ( fSum > fTotal ) {
		if ( fSum > 0.0f ) {
			*pA = (*pA) * fTotal / fSum;
			*pB = fTotal - *pA;
		} else {
			*pA = 0.0f;
			*pB = 0.0f;
		}
		*pMid = 0.0f;
	} else {
		*pMid = fTotal - fSum;
	}
}

static int __xuiProgressDrawSurfacePiece(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor)
{
	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ||
	     (__xuiProgressColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawSurface(pProxy, pDraw, pSurface, tSrc, tDst, iColor, 0);
}

static int __xuiProgressDrawTilePiece(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor)
{
	xui_rect_t tTileSrc;
	xui_rect_t tTileDst;
	double fColumns;
	double fRows;
	float fX;
	float fY;
	float fW;
	float fH;
	uint32_t iColumn;
	uint32_t iRow;
	uint32_t iColumns;
	uint32_t iRows;
	int iRet;

	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) ||
	     (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ||
	     (__xuiProgressColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	fColumns = ceil((double)tDst.fW / (double)tSrc.fW);
	fRows = ceil((double)tDst.fH / (double)tSrc.fH);
	if ( !isfinite(fColumns) || !isfinite(fRows) ||
	     (fColumns <= 0.0) || (fRows <= 0.0) ||
	     (fColumns > (double)XUI_PROGRESS_MAX_TILE_PIECES) ||
	     (fRows > (double)XUI_PROGRESS_MAX_TILE_PIECES) ||
	     (fColumns * fRows > (double)XUI_PROGRESS_MAX_TILE_PIECES) ) {
		return __xuiProgressDrawSurfacePiece(pProxy, pDraw, pSurface,
			tSrc, tDst, iColor);
	}
	iColumns = (uint32_t)fColumns;
	iRows = (uint32_t)fRows;
	for ( iRow = 0u; iRow < iRows; iRow++ ) {
		fY = (float)((double)iRow * (double)tSrc.fH);
		fH = tSrc.fH;
		if ( fY + fH > tDst.fH ) {
			fH = tDst.fH - fY;
		}
		for ( iColumn = 0u; iColumn < iColumns; iColumn++ ) {
			fX = (float)((double)iColumn * (double)tSrc.fW);
			fW = tSrc.fW;
			if ( fX + fW > tDst.fW ) {
				fW = tDst.fW - fX;
			}
			tTileSrc = tSrc;
			tTileDst = tDst;
			tTileSrc.fW = fW;
			tTileSrc.fH = fH;
			tTileDst.fX += fX;
			tTileDst.fY += fY;
			tTileDst.fW = fW;
			tTileDst.fH = fH;
			iRet = __xuiProgressDrawSurfacePiece(pProxy, pDraw, pSurface, tTileSrc, tTileDst, iColor);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiProgressDrawNinePatch(xui_proxy pProxy, xui_draw_context pDraw, const xui_nine_patch_t* pPatch, xui_rect_t tDst)
{
	xui_rect_t tSrc;
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
	float fMidW;
	float fMidH;
	int x;
	int y;
	int iIndex;
	int bTile;
	int iRet;

	if ( (pProxy == NULL) || (pDraw == NULL) || (pPatch == NULL) || (pPatch->pSurface == NULL) ||
	     (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tSrc = __xuiProgressSurfaceSrc(pProxy, pPatch->pSurface, pPatch->tSrc);
	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) ) {
		return XUI_OK;
	}
	fSrcLeft = pPatch->tSlice.fLeft;
	fSrcTop = pPatch->tSlice.fTop;
	fSrcRight = pPatch->tSlice.fRight;
	fSrcBottom = pPatch->tSlice.fBottom;
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
	__xuiProgressScaleFixed(tDst.fW, &fDstLeft, &fDstRight, &fMidW);
	__xuiProgressScaleFixed(tDst.fH, &fDstTop, &fDstBottom, &fMidH);
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
			iIndex = y * 3 + x;
			arrSrc[iIndex] = (xui_rect_t){arrSrcX[x], arrSrcY[y], arrSrcX[x + 1] - arrSrcX[x], arrSrcY[y + 1] - arrSrcY[y]};
			arrDst[iIndex] = (xui_rect_t){arrDstX[x], arrDstY[y], arrDstX[x + 1] - arrDstX[x], arrDstY[y + 1] - arrDstY[y]};
		}
	}
	for ( iIndex = 0; iIndex < 9; iIndex++ ) {
		bTile = (pPatch->iMode == XUI_NINE_PATCH_TILE) && (iIndex != 0) && (iIndex != 2) && (iIndex != 6) && (iIndex != 8);
		if ( bTile ) {
			iRet = __xuiProgressDrawTilePiece(pProxy, pDraw, pPatch->pSurface, arrSrc[iIndex], arrDst[iIndex], pPatch->iColor);
		} else {
			iRet = __xuiProgressDrawSurfacePiece(pProxy, pDraw, pPatch->pSurface, arrSrc[iIndex], arrDst[iIndex], pPatch->iColor);
		}
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiProgressPatchIsRevealSimple(const xui_nine_patch_t* pPatch)
{
	return (pPatch != NULL) &&
	       (pPatch->tSlice.fLeft <= 0.0f) &&
	       (pPatch->tSlice.fTop <= 0.0f) &&
	       (pPatch->tSlice.fRight <= 0.0f) &&
	       (pPatch->tSlice.fBottom <= 0.0f);
}

static int __xuiProgressDrawRevealPatch(xui_proxy pProxy, xui_draw_context pDraw, const xui_nine_patch_t* pPatch, xui_rect_t tFill, int iDirection, float fRate)
{
	xui_rect_t tSrc;

	if ( (pPatch == NULL) || (pPatch->pSurface == NULL) || (tFill.fW <= 0.0f) || (tFill.fH <= 0.0f) || (fRate <= 0.0f) ) {
		return XUI_OK;
	}
	tSrc = __xuiProgressSurfaceSrc(pProxy, pPatch->pSurface, pPatch->tSrc);
	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) ) {
		return XUI_OK;
	}
	if ( iDirection == XUI_PROGRESS_RIGHT_TO_LEFT ) {
		tSrc.fX += tSrc.fW * (1.0f - fRate);
		tSrc.fW *= fRate;
	} else if ( iDirection == XUI_PROGRESS_BOTTOM_TO_TOP ) {
		tSrc.fY += tSrc.fH * (1.0f - fRate);
		tSrc.fH *= fRate;
	} else if ( iDirection == XUI_PROGRESS_TOP_TO_BOTTOM ) {
		tSrc.fH *= fRate;
	} else {
		tSrc.fW *= fRate;
	}
	return __xuiProgressDrawSurfacePiece(pProxy, pDraw, pPatch->pSurface, tSrc, tFill, pPatch->iColor);
}

static void __xuiProgressNormalizePatch(xui_nine_patch_t* pPatch)
{
	if ( pPatch == NULL ) {
		return;
	}
	if ( pPatch->iSize == 0 ) {
		pPatch->iSize = sizeof(*pPatch);
	}
	if ( pPatch->iColor == 0 ) {
		pPatch->iColor = XUI_COLOR_WHITE;
	}
}

static int __xuiProgressReadSurfaceGeneration(xui_widget pWidget, xui_surface pSurface, uint32_t* pGeneration)
{
	xui_proxy pProxy;

	if ( (pWidget == NULL) || (pSurface == NULL) || (pGeneration == NULL) ) {
		return 0;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	return (pProxy != NULL) && (pProxy->surfaceGetGeneration != NULL) &&
		(pProxy->surfaceGetGeneration(pProxy, pSurface, pGeneration) == XUI_OK);
}

static int __xuiProgressUpdatePatchGeneration(xui_widget pWidget, int bHasPatch,
	const xui_nine_patch_t* pPatch, uint32_t* pGeneration, int* pGenerationValid)
{
	uint32_t iGeneration;
	int bValid;

	if ( !bHasPatch || (pPatch == NULL) || (pPatch->pSurface == NULL) ) {
		*pGenerationValid = 0;
		return 0;
	}
	bValid = __xuiProgressReadSurfaceGeneration(pWidget, pPatch->pSurface, &iGeneration);
	if ( !bValid ) {
		*pGenerationValid = 0;
		return 0;
	}
	if ( *pGenerationValid && (*pGeneration == iGeneration) ) {
		return 0;
	}
	*pGeneration = iGeneration;
	*pGenerationValid = 1;
	return 1;
}

static xui_rect_t __xuiProgressContentRect(xui_widget pWidget)
{
	return xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
}

static xui_rect_t __xuiProgressFillRectFromData(xui_widget pWidget, const xui_progress_data_t* pData, int iFillDirection)
{
	xui_rect_t tContent;
	xui_rect_t tFill;
	float fRate;
	float fEdge;

	tContent = __xuiProgressContentRect(pWidget);
	tFill = tContent;
	fRate = __xuiProgressRateData(pData);
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) || (fRate <= 0.0f) ) {
		return (xui_rect_t){tContent.fX, tContent.fY, 0.0f, 0.0f};
	}
	if ( fRate >= 1.0f ) {
		return tContent;
	}
	if ( iFillDirection == XUI_PROGRESS_RIGHT_TO_LEFT ) {
		fEdge = xuiInternalSnapPixel(tContent.fX + tContent.fW * (1.0f - fRate));
		tFill.fX = fEdge;
		tFill.fW = (tContent.fX + tContent.fW) - fEdge;
	} else if ( iFillDirection == XUI_PROGRESS_BOTTOM_TO_TOP ) {
		fEdge = xuiInternalSnapPixel(tContent.fY + tContent.fH * (1.0f - fRate));
		tFill.fY = fEdge;
		tFill.fH = (tContent.fY + tContent.fH) - fEdge;
	} else if ( iFillDirection == XUI_PROGRESS_TOP_TO_BOTTOM ) {
		fEdge = xuiInternalSnapPixel(tContent.fY + tContent.fH * fRate);
		tFill.fH = fEdge - tContent.fY;
	} else {
		fEdge = xuiInternalSnapPixel(tContent.fX + tContent.fW * fRate);
		tFill.fW = fEdge - tContent.fX;
	}
	if ( tFill.fW < 0.0f ) {
		tFill.fW = 0.0f;
	}
	if ( tFill.fH < 0.0f ) {
		tFill.fH = 0.0f;
	}
	return tFill;
}

static xui_rect_t __xuiProgressMarqueeRectFromData(xui_widget pWidget,
	const xui_progress_data_t* pData, int iFillDirection)
{
	xui_rect_t tContent;
	xui_rect_t tFill;
	float fLength;
	float fSegment;
	float fStart;
	float fEnd;
	int bVertical;

	tContent = __xuiProgressContentRect(pWidget);
	tFill = tContent;
	bVertical = (iFillDirection == XUI_PROGRESS_BOTTOM_TO_TOP) ||
		(iFillDirection == XUI_PROGRESS_TOP_TO_BOTTOM);
	fLength = bVertical ? tContent.fH : tContent.fW;
	if ( (pData == NULL) || (fLength <= 0.0f) ||
	     (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return (xui_rect_t){tContent.fX, tContent.fY, 0, 0};
	}
	fSegment = fLength * 0.3f;
	if ( fSegment < 1.0f ) fSegment = 1.0f;
	if ( fSegment > fLength ) fSegment = fLength;
	fStart = pData->fMarqueePhase * (fLength + fSegment) - fSegment;
	fEnd = fStart + fSegment;
	if ( fStart < 0.0f ) fStart = 0.0f;
	if ( fEnd > fLength ) fEnd = fLength;
	if ( fEnd <= fStart ) return (xui_rect_t){tContent.fX, tContent.fY, 0, 0};
	if ( bVertical ) {
		if ( iFillDirection == XUI_PROGRESS_BOTTOM_TO_TOP ) {
			tFill.fY = tContent.fY + fLength - fEnd;
		} else {
			tFill.fY = tContent.fY + fStart;
		}
		tFill.fH = fEnd - fStart;
	} else {
		if ( iFillDirection == XUI_PROGRESS_RIGHT_TO_LEFT ) {
			tFill.fX = tContent.fX + fLength - fEnd;
		} else {
			tFill.fX = tContent.fX + fStart;
		}
		tFill.fW = fEnd - fStart;
	}
	return xuiInternalSnapRect(tFill);
}

static int __xuiProgressRectContains(xui_rect_t tOuter, xui_rect_t tInner)
{
	return (tInner.fX >= tOuter.fX) &&
	       (tInner.fY >= tOuter.fY) &&
	       ((tInner.fX + tInner.fW) <= (tOuter.fX + tOuter.fW)) &&
	       ((tInner.fY + tInner.fH) <= (tOuter.fY + tOuter.fH));
}

static xui_rect_t __xuiProgressTextVisualRect(xui_proxy pProxy, xui_font pFont, const char* sText, xui_rect_t tContent, uint32_t iFlags)
{
	xui_vec2_t tSize;
	xui_rect_t tText;

	tText = tContent;
	memset(&tSize, 0, sizeof(tSize));
	if ( (pProxy == NULL) || (pProxy->textMeasure == NULL) ||
	     (pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) != XUI_OK) ||
	     (tSize.fX <= 0.0f) || (tSize.fY <= 0.0f) ) {
		return tText;
	}
	if ( tSize.fX > tContent.fW ) {
		tSize.fX = tContent.fW;
	}
	if ( tSize.fY > tContent.fH ) {
		tSize.fY = tContent.fH;
	}
	tText.fW = tSize.fX;
	tText.fH = tSize.fY;
	if ( (iFlags & XUI_TEXT_ALIGN_RIGHT) == XUI_TEXT_ALIGN_RIGHT ) {
		tText.fX = tContent.fX + tContent.fW - tText.fW;
	} else if ( (iFlags & XUI_TEXT_ALIGN_CENTER) == XUI_TEXT_ALIGN_CENTER ) {
		tText.fX = tContent.fX + (tContent.fW - tText.fW) * 0.5f;
	} else {
		tText.fX = tContent.fX;
	}
	if ( (iFlags & XUI_TEXT_ALIGN_BOTTOM) == XUI_TEXT_ALIGN_BOTTOM ) {
		tText.fY = tContent.fY + tContent.fH - tText.fH;
	} else if ( (iFlags & XUI_TEXT_ALIGN_MIDDLE) == XUI_TEXT_ALIGN_MIDDLE ) {
		tText.fY = tContent.fY + (tContent.fH - tText.fH) * 0.5f;
	} else {
		tText.fY = tContent.fY;
	}
	return xuiInternalSnapRect(tText);
}

static xui_rect_t __xuiProgressIntersectRect(xui_rect_t tA, xui_rect_t tB)
{
	float fLeft = fmaxf(tA.fX, tB.fX);
	float fTop = fmaxf(tA.fY, tB.fY);
	float fRight = fminf(tA.fX + tA.fW, tB.fX + tB.fW);
	float fBottom = fminf(tA.fY + tA.fH, tB.fY + tB.fH);

	if ( (fRight <= fLeft) || (fBottom <= fTop) ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	return (xui_rect_t){fLeft, fTop, fRight - fLeft, fBottom - fTop};
}

static int __xuiProgressDrawFillText(xui_proxy pProxy, xui_draw_context pDraw,
	xui_font pFont, const char* sText, xui_rect_t tContent, xui_rect_t tFill,
	uint32_t iColor, uint32_t iTextFlags)
{
	xui_rect_t tOldClip;
	xui_rect_t tClip;
	xui_rect_t tTextRect;
	int bHadOldClip;
	int iRet;
	int iRestore;

	if ( pProxy->drawClipGet == NULL || pProxy->drawClipSet == NULL ||
	     pProxy->drawClipClear == NULL ) {
		tTextRect = __xuiProgressTextVisualRect(pProxy, pFont, sText,
			tContent, iTextFlags);
		if ( !__xuiProgressRectContains(tFill, tTextRect) ) return XUI_OK;
		return pProxy->drawText(pProxy, pDraw,
			&(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont,
			.sText=sText, .iTextSize=-1,
			.iFlags=XUI_TEXT_SHAPE_DEFAULT |
				((iTextFlags & XUI_TEXT_RTL) ? XUI_TEXT_SHAPE_RTL : 0)},
			tContent, iColor, iTextFlags);
	}
	memset(&tOldClip, 0, sizeof(tOldClip));
	bHadOldClip = 0;
	iRet = pProxy->drawClipGet(pProxy, pDraw, &tOldClip, &bHadOldClip);
	if ( iRet != XUI_OK ) return iRet;
	tClip = bHadOldClip ? __xuiProgressIntersectRect(tFill, tOldClip) : tFill;
	if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) return XUI_OK;
	iRet = pProxy->drawClipSet(pProxy, pDraw, xuiInternalSnapRect(tClip));
	if ( iRet != XUI_OK ) return iRet;
	iRet = pProxy->drawText(pProxy, pDraw,
		&(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont,
		.sText=sText, .iTextSize=-1,
		.iFlags=XUI_TEXT_SHAPE_DEFAULT |
			((iTextFlags & XUI_TEXT_RTL) ? XUI_TEXT_SHAPE_RTL : 0)},
		tContent, iColor, iTextFlags);
	iRestore = bHadOldClip ? pProxy->drawClipSet(pProxy, pDraw, tOldClip) :
		pProxy->drawClipClear(pProxy, pDraw);
	return (iRet != XUI_OK) ? iRet : iRestore;
}

static int __xuiProgressContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_progress_data_t* pData;
	xui_progress_resolved_t tResolved;

	if ( (pWidget == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = (xui_progress_data_t*)pUser;
	if ( pData == NULL ) {
		pData = __xuiProgressGetData(pWidget);
	}
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiProgressResolve(pWidget, pData, &tResolved);
	if ( (tResolved.iFillDirection == XUI_PROGRESS_BOTTOM_TO_TOP) ||
	     (tResolved.iFillDirection == XUI_PROGRESS_TOP_TO_BOTTOM) ) {
		pSize->fX = 12.0f;
		pSize->fY = 120.0f;
	} else {
		pSize->fX = 120.0f;
		pSize->fY = 12.0f;
	}
	if ( (tConstraint.fX >= 0.0f) &&
	     (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) &&
	     (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static int __xuiProgressCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_progress_data_t* pData;
	xui_progress_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tContent;
	xui_rect_t tFill;
	float fRate;
	int iRet;
	int bFillVisible;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiProgressResolve(pWidget, pData, &tResolved);
	tContent = __xuiProgressContentRect(pWidget);
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return XUI_OK;
	}
	if ( pData->bHasTrackPatch ) {
		iRet = __xuiProgressDrawNinePatch(pProxy, pDraw, &pData->tTrackPatch, tContent);
	} else if ( __xuiProgressColorAlpha(tResolved.iTrackColor) != 0 ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tContent, tResolved.iTrackColor);
	} else {
		iRet = XUI_OK;
	}
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	fRate = pData->bIndeterminate ? 1.0f : __xuiProgressRateData(pData);
	tFill = pData->bIndeterminate ?
		__xuiProgressMarqueeRectFromData(pWidget, pData, tResolved.iFillDirection) :
		__xuiProgressFillRectFromData(pWidget, pData, tResolved.iFillDirection);
	bFillVisible = (tFill.fW > 0.0f) && (tFill.fH > 0.0f);
	if ( pData->bHasFillPatch && bFillVisible ) {
		if ( (tResolved.iFillPatchMode == XUI_PROGRESS_FILL_REVEAL) && __xuiProgressPatchIsRevealSimple(&pData->tFillPatch) ) {
			iRet = __xuiProgressDrawRevealPatch(pProxy, pDraw, &pData->tFillPatch, tFill, tResolved.iFillDirection, fRate);
		} else {
			iRet = __xuiProgressDrawNinePatch(pProxy, pDraw, &pData->tFillPatch, tFill);
		}
	} else if ( bFillVisible && (__xuiProgressColorAlpha(tResolved.iFillColor) != 0) ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, tResolved.iFillColor);
	}
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (tResolved.pFont != NULL) && (pProxy->drawText != NULL) &&
	     (pData->sDisplayText != NULL) && (pData->sDisplayText[0] != '\0') &&
	     (__xuiProgressColorAlpha(tResolved.iTextColor) != 0) ) {
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->sDisplayText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((tResolved.iTextFlags) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tContent, tResolved.iTextColor, tResolved.iTextFlags);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( (tResolved.pFont != NULL) && (pProxy->drawText != NULL) &&
	     (pData->sDisplayText != NULL) && (pData->sDisplayText[0] != '\0') &&
	     bFillVisible && (__xuiProgressColorAlpha(tResolved.iFillTextColor) != 0) &&
	     (pData->bHasFillPatch || (__xuiProgressColorAlpha(tResolved.iFillColor) != 0)) ) {
		iRet = __xuiProgressDrawFillText(pProxy, pDraw, tResolved.pFont,
			pData->sDisplayText, tContent, tFill, tResolved.iFillTextColor,
			tResolved.iTextFlags);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiProgressInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_progress_data_t* pData;
	const xui_progress_desc_t* pDesc;
	xui_theme_t tTheme;
	xui_context pContext;
	float fMin;
	float fMax;
	int iRet;

	(void)pUser;
	pData = (xui_progress_data_t*)pTypeData;
	pDesc = (const xui_progress_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiProgressDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	memset(&tTheme, 0, sizeof(tTheme));
	tTheme.iSize = sizeof(tTheme);
	(void)xuiGetTheme(pContext, &tTheme);
	fMin = (pDesc != NULL) ? pDesc->fMin : 0.0f;
	fMax = (pDesc != NULL) ? pDesc->fMax : 1.0f;
	if ( (pDesc == NULL) || ((fMin == 0.0f) && (fMax == 0.0f)) ) {
		fMin = 0.0f;
		fMax = 1.0f;
	}
	__xuiProgressNormalizeRange(&fMin, &fMax);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pData->fMin = fMin;
	pData->fMax = fMax;
	pData->fValue = __xuiProgressClampFloat((pDesc != NULL) ? pDesc->fValue : 0.0f, pData->fMin, pData->fMax);
	pData->iTrackColor = (pDesc != NULL && pDesc->iTrackColor != 0) ? pDesc->iTrackColor : XUI_COLOR_RGBA(216, 236, 248, 255);
	pData->iFillColor = (pDesc != NULL && pDesc->iFillColor != 0) ? pDesc->iFillColor : XUI_COLOR_RGBA(46, 124, 214, 255);
	pData->iTextColor = (pDesc != NULL && pDesc->iTextColor != 0) ? pDesc->iTextColor : ((tTheme.iTextColor != 0) ? tTheme.iTextColor : XUI_COLOR_RGBA(36, 52, 70, 255));
	pData->iFillTextColor = (pDesc != NULL && pDesc->iFillTextColor != 0) ? pDesc->iFillTextColor : XUI_COLOR_WHITE;
	pData->iTextFlags = ((pDesc != NULL && pDesc->iTextFlags != 0) ? pDesc->iTextFlags : (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) | XUI_TEXT_CLIP;
	pData->iFillDirection = (pDesc != NULL && pDesc->iFillDirection != 0) ? pDesc->iFillDirection : XUI_PROGRESS_LEFT_TO_RIGHT;
	pData->iFillPatchMode = (pDesc != NULL && pDesc->iFillPatchMode != 0) ? pDesc->iFillPatchMode : XUI_PROGRESS_FILL_STRETCH;
	pData->fMarqueePhase = 0.5f;
	pData->fMarqueeInterval = 1.5f;
	if ( (pDesc != NULL) && pDesc->bHasTrackPatch ) {
		pData->tTrackPatch = pDesc->tTrackPatch;
		__xuiProgressNormalizePatch(&pData->tTrackPatch);
		pData->bHasTrackPatch = 1;
	}
	if ( (pDesc != NULL) && pDesc->bHasFillPatch ) {
		pData->tFillPatch = pDesc->tFillPatch;
		__xuiProgressNormalizePatch(&pData->tFillPatch);
		pData->bHasFillPatch = 1;
	}
	pData->bTrackSurfaceGenerationValid = __xuiProgressReadSurfaceGeneration(pWidget,
		pData->bHasTrackPatch ? pData->tTrackPatch.pSurface : NULL,
		&pData->iTrackSurfaceGeneration);
	pData->bFillSurfaceGenerationValid = __xuiProgressReadSurfaceGeneration(pWidget,
		pData->bHasFillPatch ? pData->tFillPatch.pSurface : NULL,
		&pData->iFillSurfaceGeneration);
	iRet = __xuiProgressSetTextTemplateData(pData, (pDesc != NULL) ? pDesc->sText : NULL);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	return iRet;
}

static void __xuiProgressRequestAnimationFrame(xui_widget pWidget,
	float fDelaySeconds)
{
	xui_proxy pProxy;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->requestFrame != NULL) ) {
		pProxy->requestFrame(pProxy, fDelaySeconds);
	}
}

static int __xuiProgressUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_progress_data_t* pData;
	double fPhase;
	int bTrackChanged;
	int bFillChanged;
	int bAnimationChanged;
	int bAnimate;

	pData = (xui_progress_data_t*)pUser;
	if ( __xuiProgressGetData(pWidget) != pData ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bTrackChanged = __xuiProgressUpdatePatchGeneration(pWidget,
		pData->bHasTrackPatch, &pData->tTrackPatch,
		&pData->iTrackSurfaceGeneration, &pData->bTrackSurfaceGenerationValid);
	bFillChanged = __xuiProgressUpdatePatchGeneration(pWidget,
		pData->bHasFillPatch, &pData->tFillPatch,
		&pData->iFillSurfaceGeneration, &pData->bFillSurfaceGenerationValid);
	bAnimationChanged = 0;
	bAnimate = pData->bIndeterminate && xuiWidgetGetEffectiveVisible(pWidget);
	if ( bAnimate ) {
		__xuiProgressRequestAnimationFrame(pWidget,
			XUI_PROGRESS_ANIMATION_FRAME_SECONDS);
	}
	if ( bAnimate && isfinite(fDelta) && (fDelta > 0.0f) &&
	     (pData->fMarqueeInterval > 0.0f) ) {
		fPhase = fmod((double)pData->fMarqueePhase +
			(double)fDelta / (double)pData->fMarqueeInterval, 1.0);
		if ( fPhase < 0.0 ) fPhase += 1.0;
		if ( isfinite(fPhase) && (pData->fMarqueePhase != (float)fPhase) ) {
			pData->fMarqueePhase = (float)fPhase;
			bAnimationChanged = 1;
		}
	}
	if ( !bTrackChanged && !bFillChanged && !bAnimationChanged ) {
		return XUI_OK;
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiProgressDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_progress_data_t* pData;

	(void)pWidget;
	(void)pUser;
	pData = (xui_progress_data_t*)pTypeData;
	if ( pData == NULL ) {
		return;
	}
	if ( pData->sTextTemplate != NULL ) {
		xrtFree(pData->sTextTemplate);
	}
	if ( pData->sDisplayText != NULL ) {
		xrtFree(pData->sDisplayText);
	}
	memset(pData, 0, sizeof(*pData));
}

static void __xuiProgressDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiProgressDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static void __xuiProgressRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) {
		return;
	}
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiProgressRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	__xuiProgressRegisterStyleProperty(pContext, pType, "text.color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiProgressRegisterStyleProperty(pContext, pType, "text.flags", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiProgressRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.track_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.fill_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.text_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.fill_text_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.text_flags", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.fill_direction", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiProgressRegisterStyleProperty(pContext, pType, "progress.fill_patch_mode", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
}

static int __xuiProgressAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_PROGRESS;
	pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	if ( (pData->sDisplayText != NULL) && (pData->sDisplayText[0] != '\0') ) {
		pNode->sValue = pData->sDisplayText;
	}
	if ( !pData->bIndeterminate ) {
		pNode->tRange.iSize = sizeof(pNode->tRange);
		pNode->tRange.fValue = pData->fValue;
		pNode->tRange.fMin = pData->fMin;
		pNode->tRange.fMax = pData->fMax;
	}
	return XUI_OK;
}

static const xui_internal_accessibility_adapter_t g_xuiProgressAccessible = {
	.getNode = __xuiProgressAccessibleNode,
	.performAction = NULL
};

XUI_API xui_widget_type xuiProgressGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "progress");
	if ( pType != NULL ) {
		__xuiProgressRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiProgressAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "progress";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_progress_data_t);
	tDesc.onInit = __xuiProgressInit;
	tDesc.onDestroy = __xuiProgressDestroy;
	tDesc.onContentMeasure = __xuiProgressContentMeasure;
	tDesc.onCacheRender = __xuiProgressCacheRender;
	tDesc.onUpdate = __xuiProgressUpdate;
	__xuiProgressDefaultLayout(&tDesc.tLayout);
	__xuiProgressDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiProgressRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiProgressAccessible;
	return pType;
}

XUI_API int xuiProgressCreate(xui_context pContext, xui_widget* ppWidget, const xui_progress_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiProgressDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiProgressGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiProgressSetRange(xui_widget pWidget, float fMin, float fMax)
{
	xui_progress_data_t* pData;
	xui_progress_data_t tCandidate;
	char* sNewDisplay;
	size_t iNewCapacity;
	size_t iFormattedLength;
	int bReuseDisplay;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( (pData == NULL) || !__xuiProgressFloatValid(fMin) || !__xuiProgressFloatValid(fMax) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiProgressNormalizeRange(&fMin, &fMax);
	tCandidate = *pData;
	tCandidate.fMin = fMin;
	tCandidate.fMax = fMax;
	tCandidate.fValue = __xuiProgressClampFloat(pData->fValue, fMin, fMax);
	if ( (pData->fMin == tCandidate.fMin) &&
	     (pData->fMax == tCandidate.fMax) &&
	     (pData->fValue == tCandidate.fValue) ) {
		return XUI_OK;
	}
	sNewDisplay = NULL;
	iNewCapacity = 0u;
	bReuseDisplay = 0;
	if ( pData->bTemplateDynamic ) {
		iRet = __xuiProgressPrepareDynamicDisplay(&tCandidate, pData,
			&sNewDisplay, &iNewCapacity, &bReuseDisplay);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		if ( pData->bTemplateDynamic && !bReuseDisplay && sNewDisplay != NULL ) {
			xrtFree(sNewDisplay);
		}
		return iRet;
	}
	if ( pData->bTemplateDynamic ) {
		if ( bReuseDisplay ) {
			iRet = __xuiProgressFormatTemplateInto(&tCandidate, sNewDisplay,
				iNewCapacity, &iFormattedLength);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			if ( pData->sDisplayText != NULL ) xrtFree(pData->sDisplayText);
			pData->sDisplayText = sNewDisplay;
			pData->iDisplayCapacity = iNewCapacity;
		}
	}
	pData->fMin = tCandidate.fMin;
	pData->fMax = tCandidate.fMax;
	pData->fValue = tCandidate.fValue;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiProgressGetRange(xui_widget pWidget, float* pMin, float* pMax)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pMin != NULL ) {
		*pMin = pData->fMin;
	}
	if ( pMax != NULL ) {
		*pMax = pData->fMax;
	}
	return XUI_OK;
}

XUI_API int xuiProgressSetValue(xui_widget pWidget, float fValue)
{
	xui_progress_data_t* pData;
	xui_progress_data_t tCandidate;
	char* sNewDisplay;
	size_t iNewCapacity;
	size_t iFormattedLength;
	int bReuseDisplay;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( (pData == NULL) || !__xuiProgressFloatValid(fValue) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fValue = __xuiProgressClampFloat(fValue, pData->fMin, pData->fMax);
	if ( pData->fValue == fValue ) {
		return XUI_OK;
	}
	tCandidate = *pData;
	tCandidate.fValue = fValue;
	sNewDisplay = NULL;
	iNewCapacity = 0u;
	bReuseDisplay = 0;
	if ( pData->bTemplateDynamic ) {
		iRet = __xuiProgressPrepareDynamicDisplay(&tCandidate, pData,
			&sNewDisplay, &iNewCapacity, &bReuseDisplay);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		if ( pData->bTemplateDynamic && !bReuseDisplay && sNewDisplay != NULL ) {
			xrtFree(sNewDisplay);
		}
		return iRet;
	}
	if ( pData->bTemplateDynamic ) {
		if ( bReuseDisplay ) {
			iRet = __xuiProgressFormatTemplateInto(&tCandidate, sNewDisplay,
				iNewCapacity, &iFormattedLength);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			if ( pData->sDisplayText != NULL ) xrtFree(pData->sDisplayText);
			pData->sDisplayText = sNewDisplay;
			pData->iDisplayCapacity = iNewCapacity;
		}
	}
	pData->fValue = fValue;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return XUI_OK;
}

XUI_API float xuiProgressGetValue(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->fValue : 0.0f;
}

XUI_API float xuiProgressGetRate(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return __xuiProgressRateData(pData);
}

XUI_API int xuiProgressSetIndeterminate(xui_widget pWidget, int bIndeterminate)
{
	xui_progress_data_t* pData;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bIndeterminate = bIndeterminate ? 1 : 0;
	if ( pData->bIndeterminate == bIndeterminate ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->bIndeterminate = bIndeterminate;
	if ( bIndeterminate ) pData->fMarqueePhase = 0.5f;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bIndeterminate ) {
		__xuiProgressRequestAnimationFrame(pWidget, 0.0f);
	}
	return XUI_OK;
}

XUI_API int xuiProgressGetIndeterminate(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->bIndeterminate : 0;
}

XUI_API int xuiProgressSetMarqueeInterval(xui_widget pWidget, float fSeconds)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	if ( (pData == NULL) || !__xuiProgressFloatValid(fSeconds) || (fSeconds <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fMarqueeInterval == fSeconds ) return XUI_OK;
	pData->fMarqueeInterval = fSeconds;
	if ( pData->bIndeterminate ) {
		__xuiProgressRequestAnimationFrame(pWidget, 0.0f);
	}
	return XUI_OK;
}

XUI_API float xuiProgressGetMarqueeInterval(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->fMarqueeInterval : 0.0f;
}

XUI_API int xuiProgressSetText(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_progress_data_t* pData;
	xui_progress_prepared_text_t tPrepared;
	int bTextSame;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bTextSame = ((pData->sTextTemplate == NULL) && (sText == NULL)) ||
		((pData->sTextTemplate != NULL) && (sText != NULL) &&
		 strcmp(pData->sTextTemplate, sText) == 0);
	if ( bTextSame ) {
		return xuiProgressSetFont(pWidget, pFont);
	}
	iRet = __xuiProgressPrepareText(pData, sText, &tPrepared);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		__xuiProgressPreparedTextClear(&tPrepared);
		return iRet;
	}
	__xuiProgressCommitPreparedText(pData, &tPrepared);
	pData->pFont = pFont;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiProgressSetTextTemplate(xui_widget pWidget, const char* sTextTemplate)
{
	xui_progress_data_t* pData;
	xui_progress_prepared_text_t tPrepared;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( ((pData->sTextTemplate == NULL) && (sTextTemplate == NULL)) ||
	     ((pData->sTextTemplate != NULL) && (sTextTemplate != NULL) && (strcmp(pData->sTextTemplate, sTextTemplate) == 0)) ) {
		return XUI_OK;
	}
	iRet = __xuiProgressPrepareText(pData, sTextTemplate, &tPrepared);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		__xuiProgressPreparedTextClear(&tPrepared);
		return iRet;
	}
	__xuiProgressCommitPreparedText(pData, &tPrepared);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API const char* xuiProgressGetTextTemplate(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->sTextTemplate : NULL;
}

XUI_API const char* xuiProgressGetDisplayText(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->sDisplayText : NULL;
}

XUI_API int xuiProgressSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_progress_data_t* pData;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->pFont == pFont ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->pFont = pFont;
	return XUI_OK;
}

XUI_API xui_font xuiProgressGetFont(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiProgressSetTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_progress_data_t* pData;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iTextColor == iColor ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iTextColor = iColor;
	return XUI_OK;
}

XUI_API uint32_t xuiProgressGetTextColor(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iTextColor : 0;
}

XUI_API int xuiProgressSetFillTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_progress_data_t* pData;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iFillTextColor == iColor ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iFillTextColor = iColor;
	return XUI_OK;
}

XUI_API uint32_t xuiProgressGetFillTextColor(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iFillTextColor : 0;
}

XUI_API int xuiProgressSetTextFlags(xui_widget pWidget, uint32_t iTextFlags)
{
	xui_progress_data_t* pData;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iTextFlags |= XUI_TEXT_CLIP;
	if ( pData->iTextFlags == iTextFlags ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iTextFlags = iTextFlags;
	return XUI_OK;
}

XUI_API uint32_t xuiProgressGetTextFlags(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iTextFlags : 0;
}

XUI_API int xuiProgressSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill)
{
	xui_progress_data_t* pData;
	int iRet;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pData->iTrackColor == iTrack) && (pData->iFillColor == iFill) ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iTrackColor = iTrack;
	pData->iFillColor = iFill;
	return XUI_OK;
}

XUI_API uint32_t xuiProgressGetTrackColor(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iTrackColor : 0;
}

XUI_API uint32_t xuiProgressGetFillColor(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iFillColor : 0;
}

XUI_API int xuiProgressSetFillDirection(xui_widget pWidget, int iFillDirection)
{
	xui_progress_data_t* pData;
	int iRet;

	if ( !__xuiProgressDirectionValid(iFillDirection) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iFillDirection == iFillDirection ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE |
		XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iFillDirection = iFillDirection;
	return XUI_OK;
}

XUI_API int xuiProgressGetFillDirection(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iFillDirection : XUI_PROGRESS_LEFT_TO_RIGHT;
}

XUI_API int xuiProgressSetTrackPatch(xui_widget pWidget, const xui_nine_patch_t* pPatch)
{
	xui_progress_data_t* pData;
	xui_nine_patch_t tPatch;
	uint32_t iGeneration;
	int bGenerationValid;
	int iRet;

	if ( !__xuiProgressPatchValid(pPatch) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pPatch != NULL ) {
		tPatch = *pPatch;
		__xuiProgressNormalizePatch(&tPatch);
		if ( pData->bHasTrackPatch &&
		     __xuiProgressPatchEqual(&pData->tTrackPatch, &tPatch) ) {
			iGeneration = pData->iTrackSurfaceGeneration;
			bGenerationValid = pData->bTrackSurfaceGenerationValid;
			if ( !__xuiProgressUpdatePatchGeneration(pWidget, 1,
				&pData->tTrackPatch, &iGeneration,
				&bGenerationValid) ) return XUI_OK;
			iRet = xuiWidgetInvalidate(pWidget,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			pData->iTrackSurfaceGeneration = iGeneration;
			pData->bTrackSurfaceGenerationValid = bGenerationValid;
			return XUI_OK;
		}
		iGeneration = 0u;
		bGenerationValid = __xuiProgressReadSurfaceGeneration(pWidget,
			tPatch.pSurface, &iGeneration);
		iRet = xuiWidgetInvalidate(pWidget,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		pData->tTrackPatch = tPatch;
		pData->bHasTrackPatch = 1;
		pData->iTrackSurfaceGeneration = iGeneration;
		pData->bTrackSurfaceGenerationValid = bGenerationValid;
	} else {
		if ( !pData->bHasTrackPatch ) return XUI_OK;
		iRet = xuiWidgetInvalidate(pWidget,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		memset(&pData->tTrackPatch, 0, sizeof(pData->tTrackPatch));
		pData->bHasTrackPatch = 0;
		pData->bTrackSurfaceGenerationValid = 0;
	}
	return XUI_OK;
}

XUI_API int xuiProgressClearTrackPatch(xui_widget pWidget)
{
	return xuiProgressSetTrackPatch(pWidget, NULL);
}

XUI_API int xuiProgressHasTrackPatch(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->bHasTrackPatch : 0;
}

XUI_API int xuiProgressGetTrackPatch(xui_widget pWidget, xui_nine_patch_t* pPatch)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pPatch != NULL ) {
		if ( pData->bHasTrackPatch ) {
			*pPatch = pData->tTrackPatch;
		} else {
			memset(pPatch, 0, sizeof(*pPatch));
		}
	}
	return XUI_OK;
}

XUI_API int xuiProgressSetFillPatch(xui_widget pWidget, const xui_nine_patch_t* pPatch)
{
	xui_progress_data_t* pData;
	xui_nine_patch_t tPatch;
	uint32_t iGeneration;
	int bGenerationValid;
	int iRet;

	if ( !__xuiProgressPatchValid(pPatch) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pPatch != NULL ) {
		tPatch = *pPatch;
		__xuiProgressNormalizePatch(&tPatch);
		if ( pData->bHasFillPatch &&
		     __xuiProgressPatchEqual(&pData->tFillPatch, &tPatch) ) {
			iGeneration = pData->iFillSurfaceGeneration;
			bGenerationValid = pData->bFillSurfaceGenerationValid;
			if ( !__xuiProgressUpdatePatchGeneration(pWidget, 1,
				&pData->tFillPatch, &iGeneration,
				&bGenerationValid) ) return XUI_OK;
			iRet = xuiWidgetInvalidate(pWidget,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			pData->iFillSurfaceGeneration = iGeneration;
			pData->bFillSurfaceGenerationValid = bGenerationValid;
			return XUI_OK;
		}
		iGeneration = 0u;
		bGenerationValid = __xuiProgressReadSurfaceGeneration(pWidget,
			tPatch.pSurface, &iGeneration);
		iRet = xuiWidgetInvalidate(pWidget,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		pData->tFillPatch = tPatch;
		pData->bHasFillPatch = 1;
		pData->iFillSurfaceGeneration = iGeneration;
		pData->bFillSurfaceGenerationValid = bGenerationValid;
	} else {
		if ( !pData->bHasFillPatch ) return XUI_OK;
		iRet = xuiWidgetInvalidate(pWidget,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		memset(&pData->tFillPatch, 0, sizeof(pData->tFillPatch));
		pData->bHasFillPatch = 0;
		pData->bFillSurfaceGenerationValid = 0;
	}
	return XUI_OK;
}

XUI_API int xuiProgressClearFillPatch(xui_widget pWidget)
{
	return xuiProgressSetFillPatch(pWidget, NULL);
}

XUI_API int xuiProgressHasFillPatch(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->bHasFillPatch : 0;
}

XUI_API int xuiProgressGetFillPatch(xui_widget pWidget, xui_nine_patch_t* pPatch)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pPatch != NULL ) {
		if ( pData->bHasFillPatch ) {
			*pPatch = pData->tFillPatch;
		} else {
			memset(pPatch, 0, sizeof(*pPatch));
		}
	}
	return XUI_OK;
}

XUI_API int xuiProgressSetFillPatchMode(xui_widget pWidget, int iMode)
{
	xui_progress_data_t* pData;
	int iRet;

	if ( !__xuiProgressPatchModeValid(iMode) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iFillPatchMode == iMode ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iFillPatchMode = iMode;
	return XUI_OK;
}

XUI_API int xuiProgressGetFillPatchMode(xui_widget pWidget)
{
	xui_progress_data_t* pData;

	pData = __xuiProgressGetData(pWidget);
	return (pData != NULL) ? pData->iFillPatchMode : XUI_PROGRESS_FILL_STRETCH;
}

XUI_API xui_rect_t xuiProgressGetFillRect(xui_widget pWidget)
{
	xui_progress_data_t* pData;
	xui_progress_resolved_t tResolved;

	pData = __xuiProgressGetData(pWidget);
	if ( pData == NULL ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	__xuiProgressResolve(pWidget, pData, &tResolved);
	return pData->bIndeterminate ?
		__xuiProgressMarqueeRectFromData(pWidget, pData, tResolved.iFillDirection) :
		__xuiProgressFillRectFromData(pWidget, pData, tResolved.iFillDirection);
}

#endif
