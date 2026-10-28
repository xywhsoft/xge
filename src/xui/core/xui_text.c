#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_text_internal.h"
#include "xui_text_item_internal.h"
#include "xui_text_display.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_TEXT_LAYOUT_MAGIC 0x58554954u

#ifndef XUI_TEXT_TEST_COUNT
#define XUI_TEXT_TEST_COUNT(field, count) ((void)0)
#endif

#include "xui_text_break.inl"
#include "xui_text_carets.inl"

void xuiInternalUtf8TrimIncomplete(char* sText)
{
	size_t iLength;
	size_t iStart;
	size_t iExpected;
	unsigned char iLead;

	if ( sText == NULL || sText[0] == '\0' ) return;
	iLength = strlen(sText);
	iStart = iLength - 1u;
	while ( iStart > 0u && (((unsigned char)sText[iStart] & 0xc0u) == 0x80u) ) {
		iStart--;
	}
	iLead = (unsigned char)sText[iStart];
	if ( iLead < 0x80u ) iExpected = 1u;
	else if ( (iLead & 0xe0u) == 0xc0u ) iExpected = 2u;
	else if ( (iLead & 0xf0u) == 0xe0u ) iExpected = 3u;
	else if ( (iLead & 0xf8u) == 0xf0u ) iExpected = 4u;
	else iExpected = 1u;
	if ( iExpected > iLength - iStart ) {
		sText[iStart] = '\0';
	}
}

size_t xuiInternalUtf8CopyGraphemes(char* sDst, size_t iCapacity, const char* sSource)
{
	size_t iSourceSize;
	int iTextSize;
	int iAt;
	int iNext;

	if ( sDst == NULL || iCapacity == 0u ) return 0u;
	if ( sSource == NULL ) sSource = "";
	iSourceSize = strlen(sSource);
	iTextSize = (iSourceSize > (size_t)INT_MAX) ? INT_MAX : (int)iSourceSize;
	iAt = 0;
	while ( iAt < iTextSize ) {
		iNext = xuiInternalTextGraphemeNext(sSource, iTextSize, iAt);
		if ( iNext <= iAt || iNext > iTextSize ) iNext = iAt + 1;
		if ( (size_t)iNext >= iCapacity ) break;
		iAt = iNext;
	}
	if ( iAt > 0 ) memmove(sDst, sSource, (size_t)iAt);
	sDst[iAt] = '\0';
	return (size_t)iAt;
}

int xuiInternalTextBreakMap(const char* text, int bytes, unsigned char* boundaries)
{
    if (!text || bytes < 0 || !boundaries) return XUI_ERROR_INVALID_ARGUMENT;
    return __xuiTextBreakMap(text, bytes, boundaries);
}

int xuiInternalTextNextHardLine(const char* sText, int iSize, int iStart, int* pEnd, int* pNext)
{
	size_t iAt;
	if ( sText == NULL || iSize < 0 || iStart < 0 || iStart > iSize || pEnd == NULL || pNext == NULL )
		return XUI_ERROR_INVALID_ARGUMENT;
	iAt = (size_t)iStart;
	while ( iAt < (size_t)iSize ) {
		size_t iBefore = iAt;
		/* Printable ASCII cannot be a mandatory control; keep long source
		 * and code-line scans on the allocation-free byte path. */
		if ( (unsigned char)sText[iAt] >= 32 && (unsigned char)sText[iAt] < 128 ) {
			iAt++;
			continue;
		}
		utf32_t iScalar = __xuiTextBreakDecode(sText, (size_t)iSize, &iAt);
		enum LineBreakClass iClass = __xuiTextBreakClass(iScalar);
		if ( iClass == LBP_BK || iClass == LBP_NL || iClass == LBP_CR || iClass == LBP_LF ) {
			if ( iClass == LBP_CR && iAt < (size_t)iSize && sText[iAt] == '\n' ) iAt++;
			*pEnd = (int)iBefore;
			*pNext = (int)iAt;
			return XUI_OK;
		}
	}
	*pEnd = *pNext = iSize;
	return XUI_OK;
}

struct xui_text_layout_t {
	uint32_t iMagic;
	xui_context pContext;
	uint32_t iDpiGeneration;
	int bRefreshing;
	xui_text_layout_desc_t tDesc;
	xui_font_metrics_t tMetrics;
	xui_text_shape_t tShape;
	double* pClusterAdvances;
	unsigned char* pBreaks;
	float fHyphenWidth;
	int iTrimStart;
	int iTrimEnd;
	int iSkipStart;
	int iSkipEnd;
	char* sText;
	int iTextSize;
	xui_text_line_t* pLines;
	int iLineCount;
	int iLineCapacity;
	xui_vec2_t tSize;
	int bTruncated;
	float fNextY;
	char* pScratch;
	int iScratchCapacity;
};

static int __xuiTextLayoutValid(xui_text_layout pLayout)
{
	return (pLayout != NULL) && (pLayout->iMagic == XUI_TEXT_LAYOUT_MAGIC);
}

static int __xuiTextFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static int __xuiTextWrapModeValid(int iWrapMode)
{
	return (iWrapMode == XUI_TEXT_WRAP_NONE) ||
	       (iWrapMode == XUI_TEXT_WRAP_WORD) ||
	       (iWrapMode == XUI_TEXT_WRAP_CHAR);
}

static int __xuiTextMaxFinite(float fValue)
{
	return (fValue > 0.0f) && (fValue < XUI_LAYOUT_UNBOUNDED);
}

static float __xuiTextMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiTextDescValid(const xui_text_layout_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 0;
	}
	if ( pDesc->iSize < sizeof(*pDesc) ) {
		return 0;
	}
	if ( (pDesc->sText == NULL) || (pDesc->pFont == NULL) || (pDesc->iTextSize < -1) ) {
		return 0;
	}
	if ( !__xuiTextWrapModeValid(pDesc->iWrapMode) ||
	     !__xuiTextFloatValid(pDesc->fMaxWidth) ||
	     !__xuiTextFloatValid(pDesc->fMaxHeight) ||
	     !__xuiTextFloatValid(pDesc->fLineGap) ||
	     !__xuiTextFloatValid(pDesc->fParagraphGap) ) {
		return 0;
	}
	return 1;
}

static void __xuiTextLayoutClearGeometry(xui_text_layout pLayout)
{
	xuiTextShapeFree(&pLayout->tShape);
	xrtFree(pLayout->pClusterAdvances);
	pLayout->pClusterAdvances = NULL;
	xrtFree(pLayout->pBreaks);
	pLayout->pBreaks = NULL;
	pLayout->fHyphenWidth = 0;
	pLayout->iTrimStart = pLayout->iTrimEnd = 0;
	pLayout->iSkipStart = pLayout->iSkipEnd = 0;
	if ( pLayout->pLines != NULL ) {
		xrtFree(pLayout->pLines);
		pLayout->pLines = NULL;
	}
	pLayout->iDpiGeneration = 0;
	pLayout->iLineCount = 0;
	pLayout->iLineCapacity = 0;
	pLayout->tSize.fX = 0.0f;
	pLayout->tSize.fY = 0.0f;
	pLayout->bTruncated = 0;
	pLayout->fNextY = 0.0f;
}

static void __xuiTextLayoutClear(xui_text_layout pLayout)
{
	__xuiTextLayoutClearGeometry(pLayout);
	if ( pLayout->sText != NULL ) {
		xrtFree(pLayout->sText);
		pLayout->sText = NULL;
	}
	pLayout->iTextSize = 0;
}

static int __xuiTextLineReserve(xui_text_layout pLayout, int iCapacity)
{
	xui_text_line_t* pLines;

	if ( iCapacity <= pLayout->iLineCapacity ) {
		return XUI_OK;
	}
	if ( iCapacity < (pLayout->iLineCapacity * 2) ) {
		iCapacity = pLayout->iLineCapacity * 2;
	}
	if ( iCapacity < 8 ) {
		iCapacity = 8;
	}
	pLines = (xui_text_line_t*)xrtRealloc(pLayout->pLines, sizeof(*pLines) * (size_t)iCapacity);
	if ( pLines == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pLayout->pLines = pLines;
	pLayout->iLineCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTextScratchReserve(xui_text_layout pLayout, int iCapacity)
{
	char* pScratch;

	if ( iCapacity <= pLayout->iScratchCapacity ) {
		return XUI_OK;
	}
	pScratch = (char*)xrtRealloc(pLayout->pScratch, (size_t)iCapacity);
	if ( pScratch == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pLayout->pScratch = pScratch;
	pLayout->iScratchCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTextBuildClusterIndex(xui_text_layout pLayout)
{
	const xui_text_cluster_t* pClusters = pLayout->tShape.pClusters;
	int iCount = pLayout->tShape.iClusterCount;
	int i;

	/* Unordered or non-finite custom shapes retain their legacy scan semantics. */
	for ( i = 0; i < iCount; i++ ) {
		XUI_TEXT_TEST_COUNT(iIndexBuildSteps, 1);
		if ( !__xuiTextFloatValid(pClusters[i].fAdvance) ) return XUI_OK;
		if ( (i > 0) && ((pClusters[i].iTextStart < pClusters[i - 1].iTextStart) ||
		     (pClusters[i].iTextEnd < pClusters[i - 1].iTextEnd)) ) return XUI_OK;
	}
	if ( iCount <= 0 ) return XUI_OK;
	if ( (size_t)iCount >= SIZE_MAX / sizeof(*pLayout->pClusterAdvances) ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pLayout->pClusterAdvances = (double*)xrtMalloc(((size_t)iCount + 1u) * sizeof(*pLayout->pClusterAdvances));
	if ( pLayout->pClusterAdvances == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pLayout->pClusterAdvances[0] = 0.0;
	for ( i = 0; i < iCount; i++ ) {
		double fPrefix = pLayout->pClusterAdvances[i];
		double fAdvance = pClusters[i].fAdvance;
		double fNext = fPrefix + fAdvance;
		XUI_TEXT_TEST_COUNT(iIndexBuildSteps, 1);
		/* Do not let extreme custom advances erase small ranges through cancellation. */
		if ( (fNext - fPrefix != fAdvance) || (fNext - fAdvance != fPrefix) ) {
			xrtFree(pLayout->pClusterAdvances);
			pLayout->pClusterAdvances = NULL;
			return XUI_OK;
		}
		pLayout->pClusterAdvances[i + 1] = fNext;
	}
	return XUI_OK;
}

static int __xuiTextClusterAfter(xui_text_layout pLayout, int iOffset, int bNext)
{
	int iLo = 0;
	int iHi = pLayout->tShape.iClusterCount;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		if ( bNext ) { XUI_TEXT_TEST_COUNT(iNextProbes, 1); }
		else { XUI_TEXT_TEST_COUNT(iRangeProbes, 1); }
		if ( pLayout->tShape.pClusters[iMid].iTextEnd <= iOffset ) iLo = iMid + 1;
		else iHi = iMid;
	}
	return iLo;
}

static int __xuiTextBuildBreakIndex(xui_text_layout pLayout)
{
	if ( pLayout->pBreaks != NULL || pLayout->iTextSize <= 0 ) return XUI_OK;
	pLayout->pBreaks = (unsigned char*)xrtCalloc((size_t)pLayout->iTextSize + 1u, 1);
	if ( pLayout->pBreaks == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	return __xuiTextBreakMap(pLayout->sText, pLayout->iTextSize, pLayout->pBreaks);
}

#include "xui_text_projection.inl"

int xuiInternalTextCopyDisplay(const char* text, int bytes, char* output)
{
    return __xuiTextCopyDisplay(text,bytes,output);
}

int xuiInternalTextShapeProjection(xui_context context, xui_font font,
    const char* text, int bytes, const char* language, xui_text_shape_t* shape)
{
    struct xui_text_layout_t projection = {0}; int result;
    if (!text || bytes < 0 || !shape) return XUI_ERROR_INVALID_ARGUMENT;
    if (xuiInternalTextCopyDisplay(text, bytes, NULL) == bytes) {
        uint32_t flags=XUI_TEXT_SHAPE_DEFAULT;
        xui_proxy proxy=xuiInternalContextGetProxy(context);
        int i;
        /* Start with printable ASCII: no deletion/source remapping, BiDi or
         * mandatory break. Other projections retain their existing paint path. */
        if(proxy && proxy->drawTextShapeRange){
            for(i=0;i<bytes;i++)if((unsigned char)text[i]<32 || (unsigned char)text[i]>126)break;
            if(i==bytes)flags|=XUI_TEXT_SHAPE_RETAIN_PAINT;
        }
        result = xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=text, .iTextSize=bytes, .sLanguage=language, .iFlags=flags}, shape);
        if (result == XUI_OK) result = xuiInternalTextShapeCaretFragments(text, bytes, shape);
        return result;
    }
    projection.pContext = context; projection.tDesc.pFont = font;
    projection.sText = (char*)text; projection.iTextSize = bytes;
    projection.pBreaks = xrtCalloc((size_t)bytes + 1, 1);
    if (!projection.pBreaks) return XUI_ERROR_OUT_OF_MEMORY;
    result = __xuiTextBreakMap(text, bytes, projection.pBreaks);
    if (result == XUI_OK) result = __xuiTextShapeProjection(&projection, 1, language);
    xrtFree(projection.pBreaks);
    if (result == XUI_OK) *shape = projection.tShape;
    else xuiTextShapeFree(&projection.tShape);
    return result;
}

static void __xuiTextChunkFree(xui_internal_text_chunk_t* pChunk)
{
	if ( pChunk == NULL ) return;
	xuiTextShapeFree(&pChunk->tShape);
	xrtFree(pChunk->sText);
	xrtFree(pChunk->pAdvances);
	memset(pChunk, 0, sizeof(*pChunk));
}

void xuiInternalTextChunkCacheClear(xui_internal_text_chunk_cache_t* pCache)
{
	int i;
	if ( pCache == NULL ) return;
	for ( i = 0; i < pCache->iChunkCount; i++ ) __xuiTextChunkFree(&pCache->pChunks[i]);
	xrtFree(pCache->pChunks);
	xrtFree(pCache->pChunkAdvances);
	memset(pCache, 0, sizeof(*pCache));
}

static int __xuiTextChunkReserve(xui_internal_text_chunk_cache_t* pCache, int iNeed)
{
	xui_internal_text_chunk_t* pChunks;
	int iCapacity;
	if ( pCache == NULL || iNeed < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNeed <= pCache->iChunkCapacity ) return XUI_OK;
	iCapacity = pCache->iChunkCapacity > 0 ? pCache->iChunkCapacity : 8;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > INT_MAX / 2 ) return XUI_ERROR_OUT_OF_MEMORY;
		iCapacity *= 2;
	}
	pChunks = (xui_internal_text_chunk_t*)xrtRealloc(pCache->pChunks,
		(size_t)iCapacity * sizeof(*pChunks));
	if ( pChunks == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pChunks + pCache->iChunkCapacity, 0,
		(size_t)(iCapacity - pCache->iChunkCapacity) * sizeof(*pChunks));
	pCache->pChunks = pChunks;
	pCache->iChunkCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTextChunkBoundaryAtOrAfter(const char* sText, int iTextSize,
	int iStart, int iTarget)
{
	int iAt = iStart;
	int iNext;
	if ( iTarget > iTextSize ) iTarget = iTextSize;
	while ( iAt < iTarget ) {
		iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iAt);
		if ( iNext <= iAt || iNext > iTextSize ) return iTextSize;
		iAt = iNext;
	}
	return iAt;
}

static int __xuiTextChunkCanShapeIsolated(const char* sText, int iTextSize)
{
	int i;
	if ( sText == NULL || iTextSize < 0 ) return 0;
	for ( i = 0; i < iTextSize; i++ ) {
		unsigned char iByte = (unsigned char)sText[i];
		if ( iByte < 0x20u || iByte > 0x7eu ) return 0;
	}
	return 1;
}

static int __xuiTextChunkBuildOne(xui_context pContext, xui_font pFont,
	const char* sText, int iTextSize, int iStart, int iLimit, int bContext,
	xui_internal_text_chunk_t* pChunk)
{
	xui_text_item_t tItem;
	double fAdvance;
	int iTarget;
	int iWindowEnd;
	int iCoreBytes;
	int i;
	int iRet;
	if ( pContext == NULL || pFont == NULL || sText == NULL || pChunk == NULL ||
	     iStart < 0 || iStart >= iLimit || iLimit > iTextSize ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pChunk, 0, sizeof(*pChunk));
	iTarget = __xuiTextChunkBoundaryAtOrAfter(sText, iTextSize, iStart,
		iStart + XUI_INTERNAL_TEXT_CHUNK_BYTES);
	if ( iTarget > iLimit ) iTarget = iLimit;
	if ( iTarget <= iStart ) iTarget = iLimit;
	iWindowEnd = __xuiTextChunkBoundaryAtOrAfter(sText, iTextSize, iTarget,
		iTarget + XUI_INTERNAL_TEXT_CHUNK_LOOKAHEAD);
	if ( iWindowEnd < iTarget ) iWindowEnd = iTarget;
	pChunk->sText = (char*)xrtMalloc((size_t)(iWindowEnd - iStart) + 1u);
	if ( pChunk->sText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memcpy(pChunk->sText, sText + iStart, (size_t)(iWindowEnd - iStart));
	pChunk->sText[iWindowEnd - iStart] = '\0';
	memset(&tItem, 0, sizeof(tItem));
	tItem.iSize = sizeof(tItem);
	tItem.pFont = pFont;
	tItem.sText = pChunk->sText;
	tItem.iTextSize = iWindowEnd - iStart;
	tItem.iFlags = XUI_TEXT_SHAPE_DEFAULT | XUI_TEXT_SHAPE_RETAIN_PAINT;
	if ( bContext ) {
		tItem.sContext = sText;
		tItem.iContextSize = iTextSize;
		tItem.iContextOffset = iStart;
	}
	iRet = xuiTextShape(pContext, &tItem, &pChunk->tShape);
	if ( iRet != XUI_OK ) {
		__xuiTextChunkFree(pChunk);
		return iRet;
	}
	iRet = xuiInternalTextShapeCaretFragments(pChunk->sText,
		iWindowEnd - iStart, &pChunk->tShape);
	if ( iRet != XUI_OK ) {
		__xuiTextChunkFree(pChunk);
		return iRet;
	}
	iCoreBytes = iTarget - iStart;
	if ( iTarget < iLimit ) {
		int iChosen = 0;
		for ( i = 0; i < pChunk->tShape.iClusterCount; i++ ) {
			const xui_text_cluster_t* pCluster = &pChunk->tShape.pClusters[i];
			if ( pCluster->iTextEnd > iCoreBytes ) break;
			iChosen = pCluster->iTextEnd;
		}
		if ( iChosen <= 0 && pChunk->tShape.iClusterCount > 0 )
			iChosen = pChunk->tShape.pClusters[0].iTextEnd;
		if ( iChosen > 0 ) iCoreBytes = iChosen;
	}
	if ( iCoreBytes <= 0 || iStart + iCoreBytes > iLimit ) {
		__xuiTextChunkFree(pChunk);
		return XUI_ERROR_INVALID_STATE;
	}
	pChunk->pAdvances = (double*)xrtMalloc(
		((size_t)pChunk->tShape.iClusterCount + 1u) * sizeof(*pChunk->pAdvances));
	if ( pChunk->pAdvances == NULL ) {
		__xuiTextChunkFree(pChunk);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pChunk->pAdvances[0] = 0.0;
	pChunk->iCoreClusterCount = 0;
	for ( i = 0; i < pChunk->tShape.iClusterCount; i++ ) {
		const xui_text_cluster_t* pCluster = &pChunk->tShape.pClusters[i];
		if ( !isfinite(pCluster->fAdvance) || pCluster->fAdvance < 0.0f ||
		     pCluster->iTextStart < 0 || pCluster->iTextEnd <= pCluster->iTextStart ||
		     pCluster->iTextEnd > iWindowEnd - iStart ) {
			__xuiTextChunkFree(pChunk);
			return XUI_ERROR_INVALID_STATE;
		}
		fAdvance = pChunk->pAdvances[i] + (double)pCluster->fAdvance;
		if ( fAdvance < pChunk->pAdvances[i] ) {
			__xuiTextChunkFree(pChunk);
			return XUI_ERROR_INVALID_STATE;
		}
		pChunk->pAdvances[i + 1] = fAdvance;
		if ( pCluster->iTextEnd <= iCoreBytes ) pChunk->iCoreClusterCount = i + 1;
		else if ( pCluster->iTextStart < iCoreBytes ) {
			__xuiTextChunkFree(pChunk);
			return XUI_ERROR_UNSUPPORTED;
		}
	}
	if ( pChunk->iCoreClusterCount <= 0 ) {
		__xuiTextChunkFree(pChunk);
		return XUI_ERROR_UNSUPPORTED;
	}
	pChunk->iStart = iStart;
	pChunk->iEnd = iStart + iCoreBytes;
	pChunk->iShapeStart = iStart;
	pChunk->iShapeEnd = iWindowEnd;
	pChunk->fWidth = (float)pChunk->pAdvances[pChunk->iCoreClusterCount];
	return XUI_OK;
}

static int __xuiTextChunkBuildRange(xui_context pContext,
	xui_internal_text_chunk_cache_t* pCache, xui_font pFont,
	const char* sText, int iTextSize, int iStart, int iEnd, int bContext)
{
	xui_internal_text_chunk_t tChunk;
	int iAt = iStart;
	int iRet;
	while ( iAt < iEnd ) {
		iRet = __xuiTextChunkBuildOne(pContext, pFont, sText, iTextSize,
			iAt, iEnd, bContext, &tChunk);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiTextChunkReserve(pCache, pCache->iChunkCount + 1);
		if ( iRet != XUI_OK ) {
			__xuiTextChunkFree(&tChunk);
			return iRet;
		}
		pCache->pChunks[pCache->iChunkCount++] = tChunk;
		iAt = tChunk.iEnd;
	}
	return XUI_OK;
}

static int __xuiTextChunkRebuildAdvances(xui_internal_text_chunk_cache_t* pCache)
{
	double* pAdvances;
	int i;
	if ( pCache == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pAdvances = (double*)xrtRealloc(pCache->pChunkAdvances,
		((size_t)pCache->iChunkCount + 1u) * sizeof(*pAdvances));
	if ( pAdvances == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pCache->pChunkAdvances = pAdvances;
	pAdvances[0] = 0.0;
	for ( i = 0; i < pCache->iChunkCount; i++ )
		pAdvances[i + 1] = pAdvances[i] + (double)pCache->pChunks[i].fWidth;
	return XUI_OK;
}

int xuiInternalTextChunkCacheEnsure(xui_context pContext,
	xui_internal_text_chunk_cache_t* pCache, xui_font pFont,
	const char* sText, int iTextSize, uint32_t iDpiGeneration)
{
	xui_proxy_caps_t tCaps;
	int bContext = 0;
	int iRet;
	if ( pContext == NULL || pCache == NULL || pFont == NULL || sText == NULL ||
	     iTextSize < XUI_INTERNAL_TEXT_CHUNK_THRESHOLD ) return XUI_ERROR_UNSUPPORTED;
	if ( pCache->bValid && pCache->pFont == pFont && pCache->iTextSize == iTextSize &&
	     pCache->iDpiGeneration == iDpiGeneration ) return XUI_OK;
	if ( xuiInternalTextCopyDisplay(sText, iTextSize, NULL) != iTextSize )
		return XUI_ERROR_UNSUPPORTED;
	xuiInternalTextChunkCacheClear(pCache);
	memset(&tCaps, 0, sizeof(tCaps));
	tCaps.iSize = sizeof(tCaps);
	if ( xuiGetProxyCaps(pContext, &tCaps) == XUI_OK )
		bContext = (tCaps.iCaps & XUI_PROXY_CAP_TEXT_CONTEXT) != 0;
	if ( !bContext && !__xuiTextChunkCanShapeIsolated(sText, iTextSize) )
		return XUI_ERROR_UNSUPPORTED;
	iRet = __xuiTextChunkBuildRange(pContext, pCache, pFont, sText, iTextSize,
		0, iTextSize, bContext);
	if ( iRet == XUI_OK ) iRet = __xuiTextChunkRebuildAdvances(pCache);
	if ( iRet != XUI_OK ) {
		xuiInternalTextChunkCacheClear(pCache);
		return iRet;
	}
	pCache->iTextSize = iTextSize;
	pCache->pFont = pFont;
	pCache->iDpiGeneration = iDpiGeneration;
	pCache->bValid = 1;
	return XUI_OK;
}

static int __xuiTextChunkFindOffset(const xui_internal_text_chunk_cache_t* pCache,
	int iOffset)
{
	int iLo = 0;
	int iHi = pCache->iChunkCount;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		if ( pCache->pChunks[iMid].iEnd < iOffset ) iLo = iMid + 1;
		else iHi = iMid;
	}
	if ( iLo >= pCache->iChunkCount ) return pCache->iChunkCount - 1;
	return iLo;
}

int xuiInternalTextChunkCachePatch(xui_context pContext,
	xui_internal_text_chunk_cache_t* pCache, const char* sText, int iTextSize,
	int iEditStart, int iOldEnd, int iNewEnd)
{
	xui_internal_text_chunk_cache_t tBuilt;
	xui_internal_text_chunk_t* pCombined;
	xui_proxy_caps_t tCaps;
	int iOldSize;
	int iDelta;
	int iFirst;
	int iLast;
	int iRegionStart;
	int iRegionEnd;
	int iTailCount;
	int iCombinedCount;
	int bContext = 0;
	int i;
	int iRet;
	if ( pContext == NULL || pCache == NULL || sText == NULL || !pCache->bValid )
		return XUI_ERROR_INVALID_STATE;
	iOldSize = pCache->iTextSize;
	if ( iEditStart < 0 || iOldEnd < iEditStart || iOldEnd > iOldSize ||
	     iNewEnd < iEditStart || iNewEnd > iTextSize ||
	     iTextSize != iOldSize + iNewEnd - iOldEnd ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iTextSize < XUI_INTERNAL_TEXT_CHUNK_THRESHOLD ||
	     xuiInternalTextCopyDisplay(sText + iEditStart,
		iNewEnd - iEditStart, NULL) != iNewEnd - iEditStart ) {
		xuiInternalTextChunkCacheClear(pCache);
		return XUI_ERROR_UNSUPPORTED;
	}
	iDelta = iNewEnd - iOldEnd;
	iFirst = __xuiTextChunkFindOffset(pCache, iEditStart);
	iLast = __xuiTextChunkFindOffset(pCache, iOldEnd);
	if ( iFirst > 0 ) iFirst--;
	if ( iLast + 1 < pCache->iChunkCount ) iLast++;
	iRegionStart = pCache->pChunks[iFirst].iStart;
	iRegionEnd = pCache->pChunks[iLast].iEnd + iDelta;
	if ( iRegionEnd < iRegionStart || iRegionEnd > iTextSize ) return XUI_ERROR_INVALID_STATE;
	memset(&tCaps, 0, sizeof(tCaps));
	tCaps.iSize = sizeof(tCaps);
	if ( xuiGetProxyCaps(pContext, &tCaps) == XUI_OK )
		bContext = (tCaps.iCaps & XUI_PROXY_CAP_TEXT_CONTEXT) != 0;
	if ( !bContext && !__xuiTextChunkCanShapeIsolated(sText + iEditStart,
	     iNewEnd - iEditStart) ) {
		xuiInternalTextChunkCacheClear(pCache);
		return XUI_ERROR_UNSUPPORTED;
	}
	memset(&tBuilt, 0, sizeof(tBuilt));
	iRet = __xuiTextChunkBuildRange(pContext, &tBuilt, pCache->pFont,
		sText, iTextSize, iRegionStart, iRegionEnd, bContext);
	if ( iRet != XUI_OK ) {
		xuiInternalTextChunkCacheClear(&tBuilt);
		return iRet;
	}
	iTailCount = pCache->iChunkCount - iLast - 1;
	iCombinedCount = iFirst + tBuilt.iChunkCount + iTailCount;
	pCombined = (xui_internal_text_chunk_t*)xrtCalloc(
		(size_t)(iCombinedCount > 0 ? iCombinedCount : 1), sizeof(*pCombined));
	if ( pCombined == NULL ) {
		xuiInternalTextChunkCacheClear(&tBuilt);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( iFirst > 0 ) memcpy(pCombined, pCache->pChunks,
		(size_t)iFirst * sizeof(*pCombined));
	if ( tBuilt.iChunkCount > 0 ) memcpy(pCombined + iFirst, tBuilt.pChunks,
		(size_t)tBuilt.iChunkCount * sizeof(*pCombined));
	if ( iTailCount > 0 ) {
		memcpy(pCombined + iFirst + tBuilt.iChunkCount,
			pCache->pChunks + iLast + 1, (size_t)iTailCount * sizeof(*pCombined));
		for ( i = iFirst + tBuilt.iChunkCount; i < iCombinedCount; i++ ) {
			pCombined[i].iStart += iDelta;
			pCombined[i].iEnd += iDelta;
			pCombined[i].iShapeStart += iDelta;
			pCombined[i].iShapeEnd += iDelta;
		}
	}
	for ( i = iFirst; i <= iLast; i++ ) __xuiTextChunkFree(&pCache->pChunks[i]);
	xrtFree(pCache->pChunks);
	xrtFree(pCache->pChunkAdvances);
	xrtFree(tBuilt.pChunks);
	xrtFree(tBuilt.pChunkAdvances);
	pCache->pChunks = pCombined;
	pCache->pChunkAdvances = NULL;
	pCache->iChunkCount = iCombinedCount;
	pCache->iChunkCapacity = iCombinedCount;
	pCache->iTextSize = iTextSize;
	iRet = __xuiTextChunkRebuildAdvances(pCache);
	if ( iRet != XUI_OK ) {
		xuiInternalTextChunkCacheClear(pCache);
		return iRet;
	}
	pCache->bValid = 1;
	return XUI_OK;
}

int xuiInternalTextChunkCacheAdvance(const xui_internal_text_chunk_cache_t* pCache,
	int iOffset, float* pAdvance)
{
	const xui_internal_text_chunk_t* pChunk;
	int iChunk;
	int iLocal;
	int iLo;
	int iHi;
	if ( pCache == NULL || pAdvance == NULL || !pCache->bValid ||
	     pCache->pChunkAdvances == NULL ) return XUI_ERROR_UNSUPPORTED;
	*pAdvance = 0.0f;
	if ( iOffset <= 0 ) return XUI_OK;
	if ( iOffset >= pCache->iTextSize ) {
		*pAdvance = (float)pCache->pChunkAdvances[pCache->iChunkCount];
		return XUI_OK;
	}
	iChunk = __xuiTextChunkFindOffset(pCache, iOffset);
	pChunk = &pCache->pChunks[iChunk];
	iLocal = iOffset - pChunk->iStart;
	iLo = 0;
	iHi = pChunk->iCoreClusterCount;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		if ( pChunk->tShape.pClusters[iMid].iTextEnd <= iLocal ) iLo = iMid + 1;
		else iHi = iMid;
	}
	*pAdvance = (float)(pCache->pChunkAdvances[iChunk] + pChunk->pAdvances[iLo]);
	return XUI_OK;
}

int xuiInternalTextChunkCacheHitTest(const xui_internal_text_chunk_cache_t* pCache,
	float fAdvance, int bGlyph, int* pOffset)
{
	const xui_internal_text_chunk_t* pChunk;
	double fLocal;
	int iChunk;
	int iLo;
	int iHi;
	if ( pCache == NULL || pOffset == NULL || !pCache->bValid ||
	     pCache->pChunkAdvances == NULL ) return XUI_ERROR_UNSUPPORTED;
	*pOffset = 0;
	if ( fAdvance <= 0.0f ) return XUI_OK;
	if ( (double)fAdvance >= pCache->pChunkAdvances[pCache->iChunkCount] ) {
		*pOffset = pCache->iTextSize;
		return XUI_OK;
	}
	iLo = 0;
	iHi = pCache->iChunkCount;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		if ( pCache->pChunkAdvances[iMid + 1] <= (double)fAdvance ) iLo = iMid + 1;
		else iHi = iMid;
	}
	iChunk = iLo;
	pChunk = &pCache->pChunks[iChunk];
	fLocal = (double)fAdvance - pCache->pChunkAdvances[iChunk];
	iLo = 0;
	iHi = pChunk->iCoreClusterCount;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		if ( pChunk->pAdvances[iMid + 1] <= fLocal ) iLo = iMid + 1;
		else iHi = iMid;
	}
	if ( iLo >= pChunk->iCoreClusterCount ) *pOffset = pChunk->iEnd;
	else {
		double fLeft = pChunk->pAdvances[iLo];
		double fRight = pChunk->pAdvances[iLo + 1];
		*pOffset = pChunk->iStart +
			((bGlyph || fLocal < (fLeft + fRight) * 0.5) ?
			 pChunk->tShape.pClusters[iLo].iTextStart :
			 pChunk->tShape.pClusters[iLo].iTextEnd);
	}
	return XUI_OK;
}

float xuiInternalTextChunkCacheWidth(const xui_internal_text_chunk_cache_t* pCache)
{
	if ( pCache == NULL || !pCache->bValid || pCache->pChunkAdvances == NULL ) return 0.0f;
	return (float)pCache->pChunkAdvances[pCache->iChunkCount];
}

static int __xuiInternalTextChunkCacheDraw(xui_proxy pProxy,
	xui_draw_context pDraw,
	const xui_internal_text_chunk_cache_t* pCache, xui_rect_t tViewport,
	float fOriginX, float fY, float fHeight, uint32_t iColor, uint32_t iFlags,
	const xui_text_paint_span_t* pSpans, int iSpanCount)
{
	xui_text_paint_span_t arrLocal[16];
	xui_text_paint_span_t* pLocal = arrLocal;
	xui_rect_t tOldClip;
	xui_rect_t tClip;
	int bOldClip = 0;
	int iRet;
	int iRestore;
	int iFirst;
	int iSpan;
	int i;
	if ( pProxy == NULL || pDraw == NULL || pCache == NULL || !pCache->bValid ||
	     pProxy->drawClipGet == NULL ||
	     pProxy->drawClipSet == NULL || pProxy->drawClipClear == NULL ) return XUI_ERROR_UNSUPPORTED;
	if ( iSpanCount < 0 || (iSpanCount > 0 && pSpans == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iSpanCount > 0 && pProxy->drawTextShapeRangeSpans == NULL ) return XUI_ERROR_UNSUPPORTED;
	if ( iSpanCount == 0 && pProxy->drawTextShapeRange == NULL ) return XUI_ERROR_UNSUPPORTED;
	for ( i = 0; i < iSpanCount; i++ ) {
		if ( pSpans[i].iSize < sizeof(pSpans[i]) || pSpans[i].iStart < 0 ||
		     pSpans[i].iEnd < pSpans[i].iStart ||
		     pSpans[i].iEnd > pCache->iTextSize ||
		     (i > 0 && pSpans[i].iStart < pSpans[i - 1].iEnd) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( tViewport.fW <= 0.0f || tViewport.fH <= 0.0f ) return XUI_OK;
	if ( iSpanCount > (int)(sizeof(arrLocal) / sizeof(arrLocal[0])) ) {
		pLocal = (xui_text_paint_span_t*)xrtMalloc(
			(size_t)iSpanCount * sizeof(*pLocal));
		if ( pLocal == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = pProxy->drawClipGet(pProxy, pDraw, &tOldClip, &bOldClip);
	if ( iRet != XUI_OK ) goto cleanup;
	tClip = tViewport;
	if ( bOldClip ) {
		float fLeft = fmaxf(tClip.fX, tOldClip.fX);
		float fTop = fmaxf(tClip.fY, tOldClip.fY);
		float fRight = fminf(tClip.fX + tClip.fW, tOldClip.fX + tOldClip.fW);
		float fBottom = fminf(tClip.fY + tClip.fH, tOldClip.fY + tOldClip.fH);
		tClip = (xui_rect_t){fLeft, fTop, fRight - fLeft, fBottom - fTop};
	}
	if ( tClip.fW <= 0.0f || tClip.fH <= 0.0f ) {
		iRet = XUI_OK;
		goto cleanup;
	}
	iRet = pProxy->drawClipSet(pProxy, pDraw, tClip);
	if ( iRet != XUI_OK ) goto cleanup;
	iRet = XUI_OK;
	iFirst = 0;
	if ( fOriginX < tViewport.fX ) {
		int iLo = 0;
		int iHi = pCache->iChunkCount;
		while ( iLo < iHi ) {
			int iMid = iLo + (iHi - iLo) / 2;
			if ( fOriginX + (float)pCache->pChunkAdvances[iMid + 1] < tViewport.fX )
				iLo = iMid + 1;
			else iHi = iMid;
		}
		iFirst = iLo;
	}
	for ( i = iFirst; i < pCache->iChunkCount; i++ ) {
		const xui_internal_text_chunk_t* pChunk = &pCache->pChunks[i];
		float fChunkX = fOriginX + (float)pCache->pChunkAdvances[i];
		float fChunkRight = fOriginX + (float)pCache->pChunkAdvances[i + 1];
		xui_rect_t tRect;
		int iLocalCount = 0;
		if ( fChunkRight < tViewport.fX ) continue;
		if ( fChunkX > tViewport.fX + tViewport.fW ) break;
		if ( pChunk->tShape.pPaint == NULL ) {
			iRet = XUI_ERROR_UNSUPPORTED;
			break;
		}
		tRect = (xui_rect_t){fChunkX, fY,
			pChunk->fWidth > 0.0f ? pChunk->fWidth : 1.0f, fHeight};
		if ( iSpanCount > 0 ) {
			for ( iSpan = 0; iSpan < iSpanCount; iSpan++ ) {
				int iStart = pSpans[iSpan].iStart;
				int iEnd = pSpans[iSpan].iEnd;
				if ( iEnd <= pChunk->iStart ) continue;
				if ( iStart >= pChunk->iEnd ) break;
				if ( iStart < pChunk->iStart ) iStart = pChunk->iStart;
				if ( iEnd > pChunk->iEnd ) iEnd = pChunk->iEnd;
				if ( iEnd <= iStart ) continue;
				pLocal[iLocalCount] = pSpans[iSpan];
				pLocal[iLocalCount].iStart = iStart - pChunk->iStart;
				pLocal[iLocalCount].iEnd = iEnd - pChunk->iStart;
				iLocalCount++;
			}
			iRet = pProxy->drawTextShapeRangeSpans(pProxy, pDraw,
				&pChunk->tShape, 0, pChunk->iEnd - pChunk->iStart,
				tRect, iColor, iFlags & ~XUI_TEXT_CLIP, 0.0f,
				pLocal, iLocalCount);
		} else {
			iRet = pProxy->drawTextShapeRange(pProxy, pDraw, &pChunk->tShape,
				0, pChunk->iEnd - pChunk->iStart, tRect, iColor,
				iFlags & ~XUI_TEXT_CLIP);
		}
		if ( iRet != XUI_OK ) break;
	}
	iRestore = bOldClip ? pProxy->drawClipSet(pProxy, pDraw, tOldClip) :
		pProxy->drawClipClear(pProxy, pDraw);
	if ( iRet == XUI_OK ) iRet = iRestore;

cleanup:
	if ( pLocal != arrLocal ) xrtFree(pLocal);
	return iRet;
}

int xuiInternalTextChunkCacheDraw(xui_proxy pProxy, xui_draw_context pDraw,
	const xui_internal_text_chunk_cache_t* pCache, xui_rect_t tViewport,
	float fOriginX, float fY, float fHeight, uint32_t iColor, uint32_t iFlags)
{
	return __xuiInternalTextChunkCacheDraw(pProxy, pDraw, pCache, tViewport,
		fOriginX, fY, fHeight, iColor, iFlags, NULL, 0);
}

int xuiInternalTextChunkCacheDrawSpans(xui_proxy pProxy,
	xui_draw_context pDraw, const xui_internal_text_chunk_cache_t* pCache,
	xui_rect_t tViewport, float fOriginX, float fY, float fHeight,
	uint32_t iColor, uint32_t iFlags,
	const xui_text_paint_span_t* pSpans, int iSpanCount)
{
	return __xuiInternalTextChunkCacheDraw(pProxy, pDraw, pCache, tViewport,
		fOriginX, fY, fHeight, iColor, iFlags, pSpans, iSpanCount);
}

int xuiInternalTextDisplayGraphemes(const char* text, int bytes, unsigned char* boundaries)
{
    int count, i, at = 0; char* work;
    if (!text || bytes < 0 || !boundaries) return XUI_ERROR_INVALID_ARGUMENT;
    count = xuiInternalTextCopyDisplay(text, bytes, NULL);
    if (count == bytes) return XUI_OK;
    work = xrtMalloc((size_t)count * 2 + 1);
    if (!work) return XUI_ERROR_OUT_OF_MEMORY;
    xuiInternalTextCopyDisplay(text, bytes, work);
    if (count) set_graphemebreaks(work, (size_t)count, work + count, __xuiTextBreakDecode);
    for (i = 0; i < bytes; i++) {
        if (at > 0 && at < count && work[count + at - 1] != GRAPHEMEBREAK_BREAK)
            boundaries[i] &= (unsigned char)~XUI_LB_GRAPHEME;
        if (!(boundaries[i] & XUI_LB_INVISIBLE)) at++;
    }
    xrtFree(work); return XUI_OK;
}

static int __xuiTextMeasureRange(xui_text_layout pLayout, const char* sStart, const char* sEnd, xui_vec2_t* pSize)
{
	int iStart;
	int iEnd;
	int i;

	pSize->fX = 0.0f;
	pSize->fY = pLayout->tMetrics.fLineHeight;
	if ( sEnd <= sStart ) {
		return XUI_OK;
	}
	iStart = (int)(sStart - pLayout->sText);
	iEnd = (int)(sEnd - pLayout->sText);
	if ( pLayout->pClusterAdvances != NULL ) {
		int iFirst = __xuiTextClusterAfter(pLayout, iStart, 0);
		int iLo = iFirst;
		int iHi = pLayout->tShape.iClusterCount;
		while ( iLo < iHi ) {
			int iMid = iLo + (iHi - iLo) / 2;
			XUI_TEXT_TEST_COUNT(iRangeProbes, 1);
			if ( pLayout->tShape.pClusters[iMid].iTextStart < iEnd ) iLo = iMid + 1;
			else iHi = iMid;
		}
		pSize->fX = (float)(pLayout->pClusterAdvances[iLo] - pLayout->pClusterAdvances[iFirst]);
		return XUI_OK;
	}
	for ( i = 0; i < pLayout->tShape.iClusterCount; i++ ) {
		xui_text_cluster_t* pCluster = &pLayout->tShape.pClusters[i];
		XUI_TEXT_TEST_COUNT(iRangeProbes, 1);
		if ( pCluster->iTextEnd <= iStart ) continue;
		if ( pCluster->iTextStart >= iEnd ) break;
		pSize->fX += pCluster->fAdvance;
	}
	return XUI_OK;
}

static const char* __xuiTextNextCluster(xui_text_layout pLayout, const char* sAt, const char* sEnd)
{
	int iOffset = (int)(sAt - pLayout->sText);
	int iLimit = (int)(sEnd - pLayout->sText);
	int i;
	if ( pLayout->pClusterAdvances != NULL ) {
		i = __xuiTextClusterAfter(pLayout, iOffset, 1);
		if ( i < pLayout->tShape.iClusterCount ) {
			const xui_text_cluster_t* pCluster = &pLayout->tShape.pClusters[i];
			if ( pCluster->iTextStart < iLimit ) {
				return pLayout->sText + (pCluster->iTextEnd < iLimit ? pCluster->iTextEnd : iLimit);
			}
		}
		return sAt < sEnd ? sAt + 1 : sEnd;
	}
	for ( i = 0; i < pLayout->tShape.iClusterCount; i++ ) {
		xui_text_cluster_t* pCluster = &pLayout->tShape.pClusters[i];
		XUI_TEXT_TEST_COUNT(iNextProbes, 1);
		if ( pCluster->iTextEnd <= iOffset ) continue;
		if ( pCluster->iTextStart >= iLimit ) break;
		return pLayout->sText + (pCluster->iTextEnd < iLimit ? pCluster->iTextEnd : iLimit);
	}
	return sAt < sEnd ? sAt + 1 : sEnd;
}

static const char* __xuiTextNextLayoutCluster(xui_text_layout pLayout, const char* sAt, const char* sEnd)
{
	const char* sNext = sAt;
	do {
		const char* sPrevious = sNext;
		sNext = __xuiTextNextCluster(pLayout, sNext, sEnd);
		if ( sNext <= sPrevious ) return sEnd;
	} while ( sNext < sEnd && pLayout->pBreaks != NULL &&
		!(pLayout->pBreaks[sNext - pLayout->sText] & XUI_LB_GRAPHEME) );
	return sNext;
}

static const char* __xuiTextPrevUtf8(const char* sBegin, const char* sAt)
{
	const unsigned char* p;

	if ( sAt <= sBegin ) {
		return sBegin;
	}
	p = (const unsigned char*)sAt - 1;
	while ( ((const char*)p > sBegin) && ((*p & 0xC0u) == 0x80u) ) {
		p--;
	}
	return (const char*)p;
}

static int __xuiTextAsciiSpaceAt(const char* sAt, const char* sEnd)
{
	if ( sAt >= sEnd ) {
		return 0;
	}
	return (*sAt == ' ') || (*sAt == '\t');
}

static const char* __xuiTextTrimEndSpaces(xui_text_layout pLayout, const char* sBegin, const char* sEnd)
{
	const char* sPrev;
	const char* sOriginalEnd = sEnd;

	while ( sEnd > sBegin ) {
		XUI_TEXT_TEST_COUNT(iWhitespaceVisits, 1);
		if ( (sBegin <= pLayout->sText + pLayout->iTrimStart) &&
		     (sEnd > pLayout->sText + pLayout->iTrimStart) &&
		     (sEnd <= pLayout->sText + pLayout->iTrimEnd) ) {
			sEnd = pLayout->sText + pLayout->iTrimStart;
			continue;
		}
		sPrev = __xuiTextPrevUtf8(sBegin, sEnd);
		if ( !__xuiTextAsciiSpaceAt(sPrev, sEnd) ) {
			break;
		}
		sEnd = sPrev;
	}
	if ( sEnd < sOriginalEnd ) {
		pLayout->iTrimStart = (int)(sEnd - pLayout->sText);
		pLayout->iTrimEnd = (int)(sOriginalEnd - pLayout->sText);
	}
	return sEnd;
}

static const char* __xuiTextSkipSpaces(xui_text_layout pLayout, const char* sAt, const char* sEnd)
{
	const char* sStart = sAt;
	while ( (sAt < sEnd) && __xuiTextAsciiSpaceAt(sAt, sEnd) ) {
		XUI_TEXT_TEST_COUNT(iWhitespaceVisits, 1);
		if ( (sAt >= pLayout->sText + pLayout->iSkipStart) &&
		     (sAt < pLayout->sText + pLayout->iSkipEnd) ) {
			const char* sCachedEnd = pLayout->sText + pLayout->iSkipEnd;
			sAt = sCachedEnd < sEnd ? sCachedEnd : sEnd;
			continue;
		}
		sAt++;
	}
	if ( sAt > sStart ) {
		pLayout->iSkipStart = (int)(sStart - pLayout->sText);
		pLayout->iSkipEnd = (int)(sAt - pLayout->sText);
	}
	return sAt;
}

static const char* __xuiTextFindNewline(xui_text_layout pLayout, const char* sStart, const char* sEnd, const char** psAfter)
{
	const char* sScan;

	for ( sScan = sStart; sScan < sEnd; sScan++ ) {
		if ( *sScan == '\n' ) {
			*psAfter = sScan + 1;
			return sScan;
		}
		if ( *sScan == '\r' ) {
			if ( ((sScan + 1) < sEnd) && (sScan[1] == '\n') ) {
				*psAfter = sScan + 2;
			} else {
				*psAfter = sScan + 1;
			}
			return sScan;
		}
		if ( pLayout->pBreaks != NULL && (pLayout->pBreaks[sScan - pLayout->sText] & XUI_LB_HARD) ) {
			size_t iAt = (size_t)(sScan - pLayout->sText);
			(void)__xuiTextBreakDecode(pLayout->sText, (size_t)pLayout->iTextSize, &iAt);
			*psAfter = pLayout->sText + iAt;
			return sScan;
		}
	}
	*psAfter = sEnd;
	return sEnd;
}

static int __xuiTextAddLine(xui_text_layout pLayout, const char* sStart, const char* sEnd, int iBreakType)
{
	xui_text_line_t tLine;
	xui_vec2_t tMeasure;
	float fLineBottom;
	int iRet;

	if ( pLayout->bTruncated ) {
		return XUI_OK;
	}
	if ( sEnd < sStart ) {
		sEnd = sStart;
	}
	fLineBottom = pLayout->fNextY + pLayout->tMetrics.fLineHeight;
	if ( __xuiTextMaxFinite(pLayout->tDesc.fMaxHeight) && (fLineBottom > pLayout->tDesc.fMaxHeight) ) {
		pLayout->bTruncated = 1;
		return XUI_OK;
	}
	iRet = __xuiTextMeasureRange(pLayout, sStart, sEnd, &tMeasure);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiTextLineReserve(pLayout, pLayout->iLineCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	memset(&tLine, 0, sizeof(tLine));
	tLine.iSize = sizeof(tLine);
	tLine.iTextOffset = (int)(sStart - pLayout->sText);
	tLine.iTextSize = (int)(sEnd - sStart);
	tLine.iBreakType = iBreakType;
	tLine.fY = pLayout->fNextY;
	tLine.fW = tMeasure.fX;
	if ( iBreakType == XUI_TEXT_BREAK_WRAP && pLayout->pBreaks != NULL &&
	     (pLayout->pBreaks[sEnd - pLayout->sText] & XUI_LB_HYPHEN_USED) ) tLine.fW += pLayout->fHyphenWidth;
	tLine.fH = pLayout->tMetrics.fLineHeight;
	tLine.fBaseline = pLayout->tMetrics.fAscent;
	pLayout->pLines[pLayout->iLineCount++] = tLine;
	pLayout->tSize.fX = __xuiTextMaxFloat(pLayout->tSize.fX, tLine.fW);
	pLayout->tSize.fY = tLine.fY + tLine.fH;
	pLayout->fNextY = pLayout->tSize.fY + ((iBreakType == XUI_TEXT_BREAK_NEWLINE) ? pLayout->tDesc.fParagraphGap : pLayout->tDesc.fLineGap);
	return XUI_OK;
}

static int __xuiTextLayoutNoWrapParagraph(xui_text_layout pLayout, const char* sStart, const char* sEnd, int iBreakType)
{
	return __xuiTextAddLine(pLayout, sStart, __xuiTextTrimEndSpaces(pLayout, sStart, sEnd), iBreakType);
}

static int __xuiTextLayoutCharParagraph(xui_text_layout pLayout, const char* sStart, const char* sEnd, int iBreakType)
{
	const char* sLineStart;
	const char* sScan;
	const char* sNext;
	const char* sMeasureEnd;
	xui_vec2_t tMeasure;
	int iRet;

	if ( sStart >= sEnd ) {
		return __xuiTextAddLine(pLayout, sStart, sStart, iBreakType);
	}
	if ( !__xuiTextMaxFinite(pLayout->tDesc.fMaxWidth) ) {
		return __xuiTextLayoutNoWrapParagraph(pLayout, sStart, sEnd, iBreakType);
	}
	sLineStart = sStart;
	sScan = sStart;
	while ( sScan < sEnd ) {
		XUI_TEXT_TEST_COUNT(iWrapSteps, 1);
		sNext = __xuiTextNextLayoutCluster(pLayout, sScan, sEnd);
		if ( sNext <= sScan ) break;
		sMeasureEnd = __xuiTextTrimEndSpaces(pLayout, sLineStart, sNext);
		iRet = __xuiTextMeasureRange(pLayout, sLineStart, sMeasureEnd, &tMeasure);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		if ( (tMeasure.fX > pLayout->tDesc.fMaxWidth) && (sLineStart < sScan) ) {
			iRet = __xuiTextAddLine(pLayout, sLineStart, __xuiTextTrimEndSpaces(pLayout, sLineStart, sScan), XUI_TEXT_BREAK_WRAP);
			if ( iRet != XUI_OK ) return iRet;
			sLineStart = sScan;
			if ( pLayout->bTruncated ) {
				return XUI_OK;
			}
			sScan = sLineStart;
			continue;
		}
		if ( (tMeasure.fX > pLayout->tDesc.fMaxWidth) && (sLineStart == sScan) ) {
			iRet = __xuiTextAddLine(pLayout, sLineStart, sNext, XUI_TEXT_BREAK_WRAP);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
			if ( pLayout->bTruncated ) {
				return XUI_OK;
			}
			sLineStart = sNext;
			sScan = sLineStart;
			continue;
		}
		sScan = sNext;
	}
	if ( !pLayout->bTruncated && (sLineStart <= sEnd) ) {
		return __xuiTextAddLine(pLayout, sLineStart, __xuiTextTrimEndSpaces(pLayout, sLineStart, sEnd), iBreakType);
	}
	return XUI_OK;
}

static int __xuiTextLayoutWordParagraph(xui_text_layout pLayout, const char* sStart, const char* sEnd, int iBreakType)
{
	const char* sLineStart = sStart;
	const char* sScan = sStart;
	const char* sNormal = NULL;
	const char* sEmergency = NULL;
	if ( sStart >= sEnd || !__xuiTextMaxFinite(pLayout->tDesc.fMaxWidth) ) {
		return __xuiTextLayoutNoWrapParagraph(pLayout, sStart, sEnd, iBreakType);
	}
	while ( sScan < sEnd ) {
		const char* sNext = __xuiTextNextLayoutCluster(pLayout, sScan, sEnd);
		const char* sTrim = __xuiTextTrimEndSpaces(pLayout, sLineStart, sNext);
		unsigned char iBoundary = pLayout->pBreaks[sNext - pLayout->sText];
		xui_vec2_t tMeasure;
		float fBreakWidth;
		int iRet;
		XUI_TEXT_TEST_COUNT(iWrapSteps, 1);
		iRet = __xuiTextMeasureRange(pLayout, sLineStart, sTrim, &tMeasure);
		if ( iRet != XUI_OK ) return iRet;
		fBreakWidth = tMeasure.fX;
		if ( sNext < sEnd && (iBoundary & XUI_LB_SOFT_HYPHEN) ) fBreakWidth += pLayout->fHyphenWidth;
		if ( tMeasure.fX > pLayout->tDesc.fMaxWidth ) {
			const char* sCut = sNormal != NULL ? sNormal : sEmergency;
			if ( sCut == NULL && sNext < sEnd && (iBoundary & (XUI_LB_NORMAL | XUI_LB_EMERGENCY)) ) sCut = sNext;
			if ( sCut != NULL ) {
				const char* sLineEnd = __xuiTextTrimEndSpaces(pLayout, sLineStart, sCut);
				/* An over-wide protected unit is kept intact. Emergency breaks
				 * inside alphabetic/numeric runs never bypass GL/WJ or kinsoku. */
				if ( sLineEnd == sCut && (pLayout->pBreaks[sCut - pLayout->sText] & XUI_LB_SOFT_HYPHEN) ) {
					pLayout->pBreaks[sCut - pLayout->sText] |= XUI_LB_HYPHEN_USED;
				}
				iRet = __xuiTextAddLine(pLayout, sLineStart, sLineEnd, XUI_TEXT_BREAK_WRAP);
				if ( iRet != XUI_OK || pLayout->bTruncated ) return iRet;
				sLineStart = __xuiTextSkipSpaces(pLayout, sCut, sEnd);
				sScan = sLineStart;
				sNormal = sEmergency = NULL;
				continue;
			}
		} else {
			if ( (iBoundary & XUI_LB_NORMAL) && fBreakWidth <= pLayout->tDesc.fMaxWidth ) sNormal = sNext;
			if ( iBoundary & XUI_LB_EMERGENCY ) sEmergency = sNext;
		}
		sScan = sNext;
	}
	return __xuiTextAddLine(pLayout, sLineStart, __xuiTextTrimEndSpaces(pLayout, sLineStart, sEnd), iBreakType);
}

static int __xuiTextLayoutParagraph(xui_text_layout pLayout, const char* sStart, const char* sEnd, int iBreakType)
{
	if ( pLayout->tDesc.iWrapMode == XUI_TEXT_WRAP_NONE ) {
		return __xuiTextLayoutNoWrapParagraph(pLayout, sStart, sEnd, iBreakType);
	}
	if ( pLayout->tDesc.iWrapMode == XUI_TEXT_WRAP_WORD && pLayout->pBreaks != NULL ) {
		return __xuiTextLayoutWordParagraph(pLayout, sStart, sEnd, iBreakType);
	}
	return __xuiTextLayoutCharParagraph(pLayout, sStart, sEnd, iBreakType);
}

static void __xuiTextLayoutAlignLines(xui_text_layout pLayout)
{
	uint32_t iHorizontal;
	int i;

	iHorizontal = pLayout->tDesc.iFlags & (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_RIGHT);
	for ( i = 0; i < pLayout->iLineCount; i++ ) {
		if ( iHorizontal == XUI_TEXT_ALIGN_RIGHT ) {
			pLayout->pLines[i].fX = pLayout->tSize.fX - pLayout->pLines[i].fW;
		} else if ( iHorizontal == XUI_TEXT_ALIGN_CENTER ) {
			pLayout->pLines[i].fX = (pLayout->tSize.fX - pLayout->pLines[i].fW) * 0.5f;
		} else {
			pLayout->pLines[i].fX = 0.0f;
		}
	}
}

static int __xuiTextLayoutBuild(xui_text_layout pLayout)
{
	const char* sScan;
	const char* sEnd;
	const char* sParagraphEnd;
	const char* sAfterNewline;
	int iRet;
	int iBreakType;

	iRet = __xuiTextBuildBreakIndex(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextBuildClusterIndex(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	sScan = pLayout->sText;
	sEnd = pLayout->sText + pLayout->iTextSize;
	while ( sScan < sEnd ) {
		sParagraphEnd = __xuiTextFindNewline(pLayout, sScan, sEnd, &sAfterNewline);
		iBreakType = (sParagraphEnd < sEnd) ? XUI_TEXT_BREAK_NEWLINE : XUI_TEXT_BREAK_END;
		iRet = __xuiTextLayoutParagraph(pLayout, sScan, sParagraphEnd, iBreakType);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		if ( pLayout->bTruncated ) {
			break;
		}
		sScan = sAfterNewline;
		if ( (sParagraphEnd < sEnd) && (sScan == sEnd) ) {
			iRet = __xuiTextAddLine(pLayout, sScan, sScan, XUI_TEXT_BREAK_END);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
			break;
		}
	}
	__xuiTextLayoutAlignLines(pLayout);
	return XUI_OK;
}

static uint32_t __xuiTextMergeFlags(uint32_t iBaseFlags, uint32_t iOverrideFlags)
{
	uint32_t iHorizontal;
	uint32_t iVertical;
	uint32_t iFlags;

	iHorizontal = iOverrideFlags & (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_RIGHT);
	if ( iHorizontal == 0 ) {
		iHorizontal = iBaseFlags & (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_RIGHT);
	}
	iVertical = iOverrideFlags & (XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_ALIGN_BOTTOM);
	if ( iVertical == 0 ) {
		iVertical = iBaseFlags & (XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_ALIGN_BOTTOM);
	}
	iFlags = (iBaseFlags | iOverrideFlags) & ~(XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_ALIGN_BOTTOM);
	return iFlags | iHorizontal | iVertical;
}

static float __xuiTextVerticalOffset(xui_text_layout pLayout, xui_rect_t tRect, uint32_t iFlags)
{
	if ( (iFlags & XUI_TEXT_ALIGN_BOTTOM) == XUI_TEXT_ALIGN_BOTTOM ) {
		return tRect.fH - pLayout->tSize.fY;
	}
	if ( (iFlags & XUI_TEXT_ALIGN_MIDDLE) == XUI_TEXT_ALIGN_MIDDLE ) {
		return (tRect.fH - pLayout->tSize.fY) * 0.5f;
	}
	return 0.0f;
}

XUI_API void xuiTextShapeFree(xui_text_shape_t* pShape)
{
	if ( pShape == NULL ) return;
	if ( pShape->pPaint != NULL && pShape->paintFree != NULL ) pShape->paintFree(pShape->pPaint);
	if ( pShape->pClusters != NULL ) xrtFree(pShape->pClusters);
	if ( pShape->pCarets != NULL ) xrtFree(pShape->pCarets);
	memset(pShape, 0, sizeof(*pShape));
}

XUI_API int xuiTextShape(xui_context pContext, const xui_text_item_t* pTextItem, xui_text_shape_t* pShape)
{
	xui_text_item_t normalized;
	xui_font pFont;
	const char* sText;
	int iTextSize;
	uint32_t iFlags;
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;
	xui_vec2_t tSize;
	xui_proxy_caps_t tCaps;
	int iAt;
	int iNext;
	int iCount;
	int i;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) || pShape == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pShape, 0, sizeof(*pShape));
	iRet = __xuiTextItemNormalize(pTextItem, &normalized);
	if (iRet != XUI_OK) return iRet;
	pFont=normalized.pFont;sText=normalized.sText;iTextSize=normalized.iTextSize;iFlags=normalized.iFlags;
	pShape->iSize = sizeof(*pShape);
	pShape->iFlags = iFlags;
	pShape->iTextSize = iTextSize;
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	iRet = xuiGetProxyCaps(pContext, &tCaps);
	if ( iRet != XUI_OK ) return iRet;
	if ( (normalized.sContext && !(tCaps.iCaps & XUI_PROXY_CAP_TEXT_CONTEXT)) ||
	     (normalized.iScript && !(tCaps.iCaps & XUI_PROXY_CAP_TEXT_SCRIPT)) ||
	     (normalized.sLanguage && !(tCaps.iCaps & XUI_PROXY_CAP_TEXT_LANGUAGE)) ||
	     ((iFlags & XUI_TEXT_SHAPE_RTL) && !(tCaps.iCaps & XUI_PROXY_CAP_TEXT_RTL)) ||
	     ((iFlags & XUI_TEXT_SHAPE_RANGE) && !(tCaps.iCaps & XUI_PROXY_CAP_TEXT_RANGE)) )
		return XUI_ERROR_UNSUPPORTED;
	if ( pProxy->textShape != NULL ) return pProxy->textShape(pProxy, &normalized, pShape);
	/* Isolated measurements cannot supply contextual shaping. */
	if ( (iFlags & (XUI_TEXT_SHAPE_RTL|XUI_TEXT_SHAPE_RANGE)) || pTextItem->sContext || pTextItem->iScript || pTextItem->sLanguage )
		return XUI_ERROR_UNSUPPORTED;

	memset(&tMetrics, 0, sizeof(tMetrics));
	iRet = pProxy->fontGetMetrics(pProxy, pFont, &tMetrics);
	if ( iRet != XUI_OK ) return iRet;
	iCount = 0;
	for ( iAt = 0; iAt < iTextSize; iAt = xuiInternalTextGraphemeNext(sText, iTextSize, iAt) ) iCount++;
	if ( iCount > 0 ) {
		pShape->pClusters = (xui_text_cluster_t*)xrtCalloc((size_t)iCount, sizeof(*pShape->pClusters));
		if ( pShape->pClusters == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	}
	pShape->iClusterCount = iCount;
	pShape->fAscent = tMetrics.fAscent;
	pShape->fDescent = tMetrics.fDescent;
	pShape->fLineHeight = tMetrics.fLineHeight;
	pShape->fHeight = tMetrics.fLineHeight;
	i = 0;
	for ( iAt = 0; iAt < iTextSize; iAt = iNext, i++ ) {
		char sLocal[32];
		char* sMeasure = sLocal;
		int iBytes;
		iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iAt);
		pShape->pClusters[i].iSize = sizeof(pShape->pClusters[i]);
		pShape->pClusters[i].iTextStart = iAt;
		pShape->pClusters[i].iTextEnd = iNext;
		iBytes = iNext - iAt;
		if ( iBytes + 1 > (int)sizeof(sLocal) ) {
			sMeasure = (char*)xrtMalloc((size_t)iBytes + 1u);
			if ( sMeasure == NULL ) { xuiTextShapeFree(pShape); return XUI_ERROR_OUT_OF_MEMORY; }
		}
		memcpy(sMeasure, sText + iAt, (size_t)iBytes);
		sMeasure[iBytes] = 0;
		iRet = pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sMeasure, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize);
		if ( sMeasure != sLocal ) xrtFree(sMeasure);
		if ( iRet != XUI_OK ) { xuiTextShapeFree(pShape); return iRet; }
		pShape->pClusters[i].fAdvance = tSize.fX;
		pShape->fWidth += tSize.fX;
	}
	return XUI_OK;
}

XUI_API int xuiTextLayoutCreate(xui_context pContext, xui_text_layout* ppLayout, const xui_text_layout_desc_t* pDesc)
{
	xui_text_layout pLayout;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) || (ppLayout == NULL) || !__xuiTextDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppLayout = NULL;
	if ( xuiInternalContextGetProxy(pContext) == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	pLayout = (xui_text_layout)xrtCalloc(1, sizeof(*pLayout));
	if ( pLayout == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pLayout->iMagic = XUI_TEXT_LAYOUT_MAGIC;
	pLayout->pContext = pContext;
	iRet = xuiTextLayoutReset(pLayout, pDesc);
	if ( iRet != XUI_OK ) {
		xuiTextLayoutDestroy(pLayout);
		return iRet;
	}
	*ppLayout = pLayout;
	return XUI_OK;
}

XUI_API void xuiTextLayoutDestroy(xui_text_layout pLayout)
{
	if ( !__xuiTextLayoutValid(pLayout) ) {
		return;
	}
	__xuiTextLayoutClear(pLayout);
	if ( pLayout->pScratch != NULL ) {
		xrtFree(pLayout->pScratch);
		pLayout->pScratch = NULL;
	}
	pLayout->iMagic = 0;
	xrtFree(pLayout);
}

static int __xuiTextLayoutCompute(xui_text_layout pLayout, xui_proxy pProxy)
{
	uint32_t iDpiGeneration = pLayout->pContext->iDpiGeneration;
	int iRet, i;

	iRet = pProxy->fontGetMetrics(pProxy, pLayout->tDesc.pFont, &pLayout->tMetrics);
	if ( iRet != XUI_OK ) return iRet;
	if ( pLayout->tMetrics.fLineHeight <= 0.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTextBuildBreakIndex(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextShapeProjection(pLayout, 0, NULL);
	if ( iRet != XUI_OK ) return iRet;
	if ( pLayout->tShape.fLineHeight > 0.0f ) {
		pLayout->tMetrics.fAscent = pLayout->tShape.fAscent;
		pLayout->tMetrics.fDescent = pLayout->tShape.fDescent;
		pLayout->tMetrics.fLineHeight = pLayout->tShape.fLineHeight;
	}
	if ( pLayout->tDesc.iWrapMode == XUI_TEXT_WRAP_WORD && __xuiTextMaxFinite(pLayout->tDesc.fMaxWidth) ) {
		for ( i = 0; i + 1 < pLayout->iTextSize; i++ ) {
			if ( (unsigned char)pLayout->sText[i] == 0xc2u && (unsigned char)pLayout->sText[i + 1] == 0xadu ) {
				xui_text_shape_t tHyphen;
				int j;
				iRet = xuiTextShape(pLayout->pContext, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pLayout->tDesc.pFont, .sText="-", .iTextSize=1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tHyphen);
				if ( iRet == XUI_OK ) {
					for ( j = 0; j < tHyphen.iClusterCount; j++ ) pLayout->fHyphenWidth += tHyphen.pClusters[j].fAdvance;
					if ( !__xuiTextFloatValid(pLayout->fHyphenWidth) ) iRet = XUI_ERROR_INVALID_ARGUMENT;
				}
				xuiTextShapeFree(&tHyphen);
				if ( iRet != XUI_OK ) return iRet;
				break;
			}
		}
	}
	iRet = __xuiTextLayoutBuild(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	if ( pLayout->pContext->iDpiGeneration != iDpiGeneration ) return XUI_ERROR_INVALID_STATE;
	pLayout->iDpiGeneration = iDpiGeneration;
	return XUI_OK;
}

XUI_API int xuiTextLayoutReset(xui_text_layout pLayout, const xui_text_layout_desc_t* pDesc)
{
	xui_proxy pProxy;
	int iTextSize;
	int iRet;

	if ( !__xuiTextLayoutValid(pLayout) || !__xuiTextDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayout->bRefreshing ) return XUI_ERROR_INVALID_STATE;
	if ( !xuiInternalContextIsValid(pLayout->pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pLayout->pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iTextSize = pDesc->iTextSize;
	if ( iTextSize < 0 ) {
		iTextSize = (int)strlen(pDesc->sText);
	}
	__xuiTextLayoutClear(pLayout);
	pLayout->tDesc = *pDesc;
	pLayout->tDesc.iSize = sizeof(pLayout->tDesc);
	pLayout->iTextSize = iTextSize;
	pLayout->sText = (char*)xrtMalloc((size_t)iTextSize + 1u);
	if ( pLayout->sText == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pLayout->sText, pDesc->sText, (size_t)iTextSize);
	pLayout->sText[iTextSize] = 0;
	pLayout->bRefreshing = 1;
	iRet = __xuiTextLayoutCompute(pLayout, pProxy);
	pLayout->bRefreshing = 0;
	return iRet;
}

static int __xuiTextLayoutEnsureDpi(xui_text_layout pLayout)
{
	struct xui_text_layout_t tNext;
	xui_proxy pProxy;
	int iRet;

	if ( !__xuiTextLayoutValid(pLayout) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pLayout->bRefreshing ) return XUI_ERROR_INVALID_STATE;
	if ( !xuiInternalContextIsValid(pLayout->pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pLayout->iDpiGeneration == pLayout->pContext->iDpiGeneration ) return XUI_OK;
	if ( pLayout->sText == NULL ) return XUI_ERROR_INVALID_STATE;
	pProxy = xuiInternalContextGetProxy(pLayout->pContext);
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;

	/* Keep borrowed GetText pointers stable and publish only a complete rebuild. */
	memset(&tNext, 0, sizeof(tNext));
	tNext.iMagic = pLayout->iMagic;
	tNext.pContext = pLayout->pContext;
	tNext.tDesc = pLayout->tDesc;
	tNext.sText = pLayout->sText;
	tNext.iTextSize = pLayout->iTextSize;
	tNext.pScratch = pLayout->pScratch;
	tNext.iScratchCapacity = pLayout->iScratchCapacity;
	pLayout->bRefreshing = 1;
	iRet = __xuiTextLayoutCompute(&tNext, pProxy);
	pLayout->bRefreshing = 0;
	if ( iRet != XUI_OK ) {
		__xuiTextLayoutClearGeometry(&tNext);
		return iRet;
	}
	__xuiTextLayoutClearGeometry(pLayout);
	*pLayout = tNext;
	return XUI_OK;
}

XUI_API xui_vec2_t xuiTextLayoutGetSize(xui_text_layout pLayout)
{
	xui_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( __xuiTextLayoutEnsureDpi(pLayout) == XUI_OK ) {
		tSize = pLayout->tSize;
	}
	return tSize;
}

XUI_API int xuiTextLayoutGetLineCount(xui_text_layout pLayout)
{
	return __xuiTextLayoutEnsureDpi(pLayout) == XUI_OK ? pLayout->iLineCount : 0;
}

XUI_API int xuiTextLayoutGetLine(xui_text_layout pLayout, int iIndex, xui_text_line_t* pLine)
{
	int iRet;
	if ( !__xuiTextLayoutValid(pLayout) || (pLine == NULL) || (iIndex < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTextLayoutEnsureDpi(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	if ( iIndex >= pLayout->iLineCount ) return XUI_ERROR_INVALID_ARGUMENT;
	*pLine = pLayout->pLines[iIndex];
	return XUI_OK;
}

XUI_API const char* xuiTextLayoutGetText(xui_text_layout pLayout)
{
	if ( !__xuiTextLayoutValid(pLayout) ) {
		return NULL;
	}
	(void)__xuiTextLayoutEnsureDpi(pLayout);
	return pLayout->sText;
}

XUI_API int xuiTextLayoutGetTruncated(xui_text_layout pLayout)
{
	return __xuiTextLayoutEnsureDpi(pLayout) == XUI_OK ? pLayout->bTruncated : 0;
}

static int __xuiTextLayoutDisplayLine(xui_text_layout pLayout, const xui_text_line_t* pLine, const char** ppText, int* pSize)
{
	int i, iEnd, iSize = 0, iRet;
	/* Replacing the two-byte SHY by one byte never increases source length. */
	if ( pLine->iTextSize == INT_MAX ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = __xuiTextScratchReserve(pLayout, pLine->iTextSize + 1);
	if ( iRet != XUI_OK ) return iRet;
	iEnd = pLine->iTextOffset + pLine->iTextSize;
	for ( i = pLine->iTextOffset; i < iEnd; i++ ) {
		if ( pLayout->pBreaks == NULL || !(pLayout->pBreaks[i] & XUI_LB_INVISIBLE) ) pLayout->pScratch[iSize++] = pLayout->sText[i];
	}
	if ( pLine->iBreakType == XUI_TEXT_BREAK_WRAP && pLayout->pBreaks != NULL &&
	     (pLayout->pBreaks[iEnd] & XUI_LB_HYPHEN_USED) ) pLayout->pScratch[iSize++] = '-';
	pLayout->pScratch[iSize] = 0;
	*ppText = pLayout->pScratch;
	*pSize = iSize;
	return XUI_OK;
}

int xuiInternalTextLayoutGetDisplayLine(xui_text_layout pLayout, int iIndex, const char** ppText, int* pSize)
{
	xui_text_line_t tLine;
	int iRet;
	if ( ppText == NULL || pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppText = NULL;
	*pSize = 0;
	iRet = xuiTextLayoutGetLine(pLayout, iIndex, &tLine);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTextLayoutDisplayLine(pLayout, &tLine, ppText, pSize);
}

int xuiInternalTextLayoutLineAdvance(xui_text_layout pLayout, int iLine, int iOffset, float* pAdvance)
{
	xui_text_line_t tLine;
	xui_vec2_t tSize;
	int iRet, iEnd;
	if ( pAdvance == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pAdvance = 0.0f;
	iRet = xuiTextLayoutGetLine(pLayout, iLine, &tLine);
	if ( iRet != XUI_OK ) return iRet;
	iEnd = tLine.iTextOffset + tLine.iTextSize;
	if ( iOffset <= tLine.iTextOffset ) return XUI_OK;
	if ( iOffset >= iEnd ) { *pAdvance = tLine.fW; return XUI_OK; }
	iRet = __xuiTextMeasureRange(pLayout, pLayout->sText + tLine.iTextOffset,
		pLayout->sText + iOffset, &tSize);
	if ( iRet == XUI_OK ) *pAdvance = tSize.fX;
	return iRet;
}

int xuiInternalTextLayoutLineHitTest(xui_text_layout pLayout, int iLine,
	float fAdvance, int bGlyph, int* pOffset)
{
	const xui_text_line_t* pLine;
	const xui_text_cluster_t* pClusters;
	double fBase;
	int iFirst;
	int iLast;
	int iLo;
	int iHi;
	int iCluster;
	int iGroupFirst;
	int iGroupLast;
	int iStart;
	int iEnd;
	int iRet;

	if ( pOffset == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pOffset = 0;
	iRet = __xuiTextLayoutEnsureDpi(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	if ( iLine < 0 || iLine >= pLayout->iLineCount ) return XUI_ERROR_INVALID_ARGUMENT;
	pLine = &pLayout->pLines[iLine];
	iStart = pLine->iTextOffset;
	iEnd = iStart + pLine->iTextSize;
	*pOffset = iStart;
	if ( fAdvance <= 0.0f || iEnd <= iStart ) return XUI_OK;
	if ( fAdvance >= pLine->fW ) {
		*pOffset = iEnd;
		return XUI_OK;
	}
	if ( pLayout->pClusterAdvances == NULL || pLayout->tShape.pClusters == NULL ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	pClusters = pLayout->tShape.pClusters;
	iFirst = __xuiTextClusterAfter(pLayout, iStart, 0);
	iLo = iFirst;
	iHi = pLayout->tShape.iClusterCount;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		if ( pClusters[iMid].iTextStart < iEnd ) iLo = iMid + 1;
		else iHi = iMid;
	}
	iLast = iLo;
	if ( iFirst >= iLast ) {
		*pOffset = iEnd;
		return XUI_OK;
	}
	fBase = pLayout->pClusterAdvances[iFirst];
	iLo = iFirst;
	iHi = iLast;
	while ( iLo < iHi ) {
		int iMid = iLo + (iHi - iLo) / 2;
		double fRight = pLayout->pClusterAdvances[iMid + 1] - fBase;
		if ( fRight <= (double)fAdvance ) iLo = iMid + 1;
		else iHi = iMid;
	}
	iCluster = iLo;
	if ( iCluster >= iLast ) {
		*pOffset = iEnd;
		return XUI_OK;
	}
	iGroupFirst = iCluster;
	while ( iGroupFirst > iFirst && pLayout->pBreaks != NULL &&
	       !(pLayout->pBreaks[pClusters[iGroupFirst].iTextStart] & XUI_LB_GRAPHEME) ) {
		iGroupFirst--;
	}
	iGroupLast = iCluster;
	while ( iGroupLast + 1 < iLast && pLayout->pBreaks != NULL &&
	       !(pLayout->pBreaks[pClusters[iGroupLast].iTextEnd] & XUI_LB_GRAPHEME) ) {
		iGroupLast++;
	}
	{
		double fLeft = pLayout->pClusterAdvances[iGroupFirst] - fBase;
		double fRight = pLayout->pClusterAdvances[iGroupLast + 1] - fBase;
		int iBefore = pClusters[iGroupFirst].iTextStart;
		int iAfter = pClusters[iGroupLast].iTextEnd;
		if ( iBefore < iStart ) iBefore = iStart;
		if ( iAfter > iEnd ) iAfter = iEnd;
		*pOffset = (bGlyph || (double)fAdvance < (fLeft + fRight) * 0.5) ? iBefore : iAfter;
	}
	return XUI_OK;
}

int xuiInternalTextLayoutNextCaret(xui_text_layout pLayout, int iOffset, int iLimit, int* pNext)
{
	int iRet;
	if ( pNext == NULL || iOffset < 0 || iLimit < iOffset ) return XUI_ERROR_INVALID_ARGUMENT;
	*pNext = iOffset;
	iRet = __xuiTextLayoutEnsureDpi(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	if ( iLimit > pLayout->iTextSize ) return XUI_ERROR_INVALID_ARGUMENT;
	*pNext = (int)(__xuiTextNextLayoutCluster(pLayout, pLayout->sText + iOffset,
		pLayout->sText + iLimit) - pLayout->sText);
	return XUI_OK;
}

XUI_API int xuiTextLayoutDraw(xui_text_layout pLayout, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_proxy pProxy;
	xui_text_line_t* pLine;
	xui_rect_t tLineRect;
	uint32_t iMergedFlags;
	uint32_t iLineFlags;
	float fOffsetY;
	float fLineBottom;
	int i;
	int iRet;

	if ( !__xuiTextLayoutValid(pLayout) || (pTarget == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTextLayoutEnsureDpi(pLayout);
	if ( iRet != XUI_OK ) return iRet;
	pProxy = xuiInternalContextGetProxy(pLayout->pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iMergedFlags = __xuiTextMergeFlags(pLayout->tDesc.iFlags, iFlags);
	iLineFlags = (iMergedFlags & (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_CLIP | XUI_TEXT_UNDERLINE | XUI_TEXT_RTL)) | XUI_TEXT_ALIGN_TOP;
	fOffsetY = __xuiTextVerticalOffset(pLayout, tRect, iMergedFlags);
	for ( i = 0; i < pLayout->iLineCount; i++ ) {
		const char* sDisplay;
		int iDisplaySize;
		pLine = &pLayout->pLines[i];
		if ( pLine->iTextSize <= 0 ) {
			continue;
		}
		iRet = __xuiTextLayoutDisplayLine(pLayout, pLine, &sDisplay, &iDisplaySize);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		tLineRect.fX = tRect.fX;
		tLineRect.fY = tRect.fY + fOffsetY + pLine->fY;
		tLineRect.fW = tRect.fW;
		tLineRect.fH = pLine->fH;
		if ( (iMergedFlags & XUI_TEXT_CLIP) != 0 ) {
			fLineBottom = tLineRect.fY + tLineRect.fH;
			if ( (fLineBottom <= tRect.fY) || (tLineRect.fY >= (tRect.fY + tRect.fH)) ) {
				continue;
			}
			if ( tLineRect.fY < tRect.fY ) {
				tLineRect.fH -= tRect.fY - tLineRect.fY;
				tLineRect.fY = tRect.fY;
			}
			if ( (tLineRect.fY + tLineRect.fH) > (tRect.fY + tRect.fH) ) {
				tLineRect.fH = (tRect.fY + tRect.fH) - tLineRect.fY;
			}
			if ( tLineRect.fH <= 0.0f ) {
				continue;
			}
		}
		iRet = pProxy->textDraw(pProxy, pTarget, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pLayout->tDesc.pFont, .sText=sDisplay, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((iLineFlags) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLineRect, iColor, iLineFlags);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

XUI_API int xuiTextMeasureLayout(xui_context pContext, const xui_text_layout_desc_t* pDesc, xui_vec2_t* pSize)
{
	xui_text_layout pLayout;
	int iRet;

	if ( (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pContext, &pLayout, pDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	*pSize = xuiTextLayoutGetSize(pLayout);
	xuiTextLayoutDestroy(pLayout);
	return XUI_OK;
}

#endif
