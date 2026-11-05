#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_text_internal.h"
#include "xui_text_item_internal.h"
#include "xui_proxy_resource.h"
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

int xuiInternalTextBufferReserve(char** ppBuffer, int* pCapacity, int iRequired)
{
	char* pNew;
	int iNewCapacity;
	int iGrowth;

	if ( ppBuffer == NULL || pCapacity == NULL || iRequired <= 0 )
		return XUI_ERROR_INVALID_ARGUMENT;
	if ( *pCapacity < 0 || (*pCapacity > 0 && *ppBuffer == NULL) )
		return XUI_ERROR_INVALID_STATE;
	if ( iRequired <= *pCapacity ) return XUI_OK;
	iNewCapacity = *pCapacity;
	if ( iNewCapacity < 32 ) iNewCapacity = 32;
	while ( iNewCapacity < iRequired ) {
		iGrowth = iNewCapacity / 2;
		if ( iGrowth < 32 ) iGrowth = 32;
		if ( iNewCapacity > INT_MAX - iGrowth ) {
			iNewCapacity = iRequired;
			break;
		}
		iNewCapacity += iGrowth;
	}
	pNew = (char*)xrtRealloc(*ppBuffer, (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	*ppBuffer = pNew;
	*pCapacity = iNewCapacity;
	return XUI_OK;
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

#define XUI_TEXT_LAYOUT_PAYLOAD_FIELDS \
    bool bOwnedFont,bOwnedText; \
    uint32_t iDpiGeneration; \
    xui_text_layout_desc_t tDesc; \
    xui_font_metrics_t tMetrics; \
    xui_text_shape_t tShape; \
    double* pClusterAdvances; \
    unsigned char* pBreaks; \
    float fHyphenWidth; \
    int iTrimStart,iTrimEnd,iSkipStart,iSkipEnd; \
    char* sText; \
    int iTextSize; \
    xui_text_line_t* pLines; \
    int iLineCount,iLineCapacity; \
    xui_vec2_t tSize; \
    int bTruncated; \
    float fNextY;
typedef struct xui_text_layout_payload_t { XUI_TEXT_LAYOUT_PAYLOAD_FIELDS } xui_text_layout_payload_t;
struct xui_text_layout_t {
    uint32_t iMagic;
    bool bDestroying,bRetiring;
    xui_context pContext;
    uint64_t iCreatorThread;
    xui_lifetime_link_t tContextLink;
    xui_lifetime_link_t* pLifetimeLinks;
    int iCalls,bRefreshing;
    /* Only payload ownership moves. Watches, callbacks and native identity
     * remain attached to this one allocation throughout a failed rebuild. */
    union { xui_text_layout_payload_t active; struct { XUI_TEXT_LAYOUT_PAYLOAD_FIELDS }; };
    xui_text_layout_payload_t draft;
    char* pScratch;
    int iScratchCapacity;
};
#undef XUI_TEXT_LAYOUT_PAYLOAD_FIELDS

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

static void __xuiTextCleanupPrefixBegin(xui_context context,xui_cleanup_receipt_t* receipt,xui_cleanup_prefix_t* prefix)
{
    if(receipt->status!=XUI_OK)
        xuiInternalCleanupAdd(receipt,xuiInternalCleanupPrefixBegin(context,prefix,receipt->status,receipt->error));
}
static void __xuiTextCleanupPrefixEnd(xui_cleanup_receipt_t* receipt,xui_cleanup_prefix_t* prefix)
{xuiInternalCleanupAdd(receipt,xuiInternalCleanupPrefixEnd(prefix));}

/* A failed rebuild remains a bounded second real payload, never a stack
 * owner or a hidden retirement queue. A pure wait preserves its entire slot.
 * A consuming callback failure is independent from whether it completed. */
static int __xuiTextLayoutPayloadClear(xui_context context,xui_text_layout_payload_t* payload,bool* complete,bool* changed)
{
    *complete=false;if(changed)*changed=false;
    bool consumed=false,called=false;
    int status=xuiInternalTextShapeFreeOutcome(&payload->tShape,&consumed,&called);
    if(!consumed && !called && status==XUI_ERROR_BUSY)return XUI_STATUS_DEFERRED;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    if(called)xuiInternalContextCleanupOutcome(context,status,&receipt.status,&receipt.error);
    else xuiInternalCleanupAdd(&receipt,status);
    if(!consumed)return xuiInternalCleanupEnd(NULL,&receipt);
    if(changed)*changed=true;
    xrtFree(payload->pClusterAdvances);payload->pClusterAdvances=NULL;
    xrtFree(payload->pBreaks);payload->pBreaks=NULL;
    xrtFree(payload->pLines);payload->pLines=NULL;
    payload->iLineCount=payload->iLineCapacity=0;
    if(payload->bOwnedFont && payload->tDesc.pFont){
        bool released=false;
        status=xuiFontReleaseOwnerChecked(payload->tDesc.pFont,&released);
        if(!released && (status==XUI_STATUS_DEFERRED || status==XUI_ERROR_BUSY)) {
            int result=xuiInternalCleanupEnd(NULL,&receipt);
            return result==XUI_OK ? XUI_STATUS_DEFERRED:result;
        }
        xuiInternalContextCleanupOutcome(context,status,&receipt.status,&receipt.error);
        if(!released)return xuiInternalCleanupEnd(NULL,&receipt);
        payload->bOwnedFont=false;
    }
    if(payload->bOwnedText)xrtFree(payload->sText);
    memset(payload,0,sizeof(*payload));*complete=true;
    return xuiInternalCleanupEnd(NULL,&receipt);
}
static void __xuiTextLayoutSwapPayload(xui_text_layout layout)
{ xui_text_layout_payload_t saved=layout->active;layout->active=layout->draft;layout->draft=saved; }
static bool __xuiTextLayoutPayloadTrace(const xui_text_layout_payload_t* payload,xrtownershipvisitor visit,void* user)
{
    if(payload->bOwnedFont && payload->tDesc.pFont && !visit(xuiFontOwnership(payload->tDesc.pFont),user))return false;
    return xuiInternalTextShapeTraceOwnership(&payload->tShape,visit,user);
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
	if ( pLayout == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiInternalTextBufferReserve(&pLayout->pScratch,
		&pLayout->iScratchCapacity, iCapacity);
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
    /* The caller receives the real partial owner even on a later mapping
     * allocation failure. Its persistent slot decides when cleanup can run. */
    *shape = projection.tShape;
    return result;
}

void xuiInternalTextChunkCacheClear(xui_internal_text_chunk_cache_t* cache)
{
    /* Legacy void caller still retains the actual cache on a pure wait.
     * Checked owners must inspect completion before releasing its carrier. */
    if(cache)(void)xuiInternalTextChunkCacheClearChecked(NULL,cache);
}

bool xuiInternalTextChunkCacheTraceOwnership(const xui_internal_text_chunk_cache_t* cache,xrtownershipvisitor visit,void* user)
{
    if(!cache || !visit)return false;
    for(int i=0;i<cache->iChunkCount;++i)if(!xuiInternalTextShapeTraceOwnership(&cache->pChunks[i].tShape,visit,user))return false;
    return true;
}
int xuiInternalTextChunkCacheReady(const xui_internal_text_chunk_cache_t* cache,bool managed)
{
    if(!cache)return XUI_ERROR_INVALID_ARGUMENT;
    if(cache->iPreparation)return XUI_ERROR_BUSY;
    for(int i=0;i<cache->iChunkCount;++i){int status=xuiInternalTextShapeReady(&cache->pChunks[i].tShape,managed);if(status!=XUI_OK)return status;}
    return XUI_OK;
}
static int __xuiTextChunkCacheClear(xui_context context,xui_internal_text_chunk_cache_t* cache,bool deferred,bool* complete)
{
    if(!cache || !complete)return XUI_ERROR_INVALID_ARGUMENT;
    *complete=false;
    if(cache->iPreparation)return deferred ? XUI_STATUS_DEFERRED:XUI_ERROR_BUSY;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    int previousValid=cache->bValid;
    cache->iPreparation=1;bool waiting=false,changed=false;
    for(int i=0;i<cache->iChunkCount;++i){
        xui_internal_text_chunk_t* chunk=&cache->pChunks[i];bool consumed=false,called=false;
        int status=xuiInternalTextShapeFreeOutcome(&chunk->tShape,&consumed,&called);
        if(!consumed && !called && status==XUI_ERROR_BUSY){waiting=true;continue;}
        if(called)xuiInternalContextCleanupOutcome(context,status,&receipt.status,&receipt.error);
        else xuiInternalCleanupAdd(&receipt,status);
        if(consumed){
            if(chunk->sText || chunk->pAdvances)changed=true;
            xrtFree(chunk->sText);xrtFree(chunk->pAdvances);chunk->sText=NULL;chunk->pAdvances=NULL;
        }
        else waiting=true;
    }
    cache->iPreparation=0;cache->bValid=changed ? 0:previousValid;
    if(!waiting){
        xrtFree(cache->pChunks);xrtFree(cache->pChunkAdvances);memset(cache,0,sizeof(*cache));*complete=true;
    }
    int status=xuiInternalCleanupEnd(NULL,&receipt);
    return status!=XUI_OK ? status:waiting ? (deferred ? XUI_STATUS_DEFERRED:XUI_ERROR_BUSY):XUI_OK;
}
int xuiInternalTextChunkCacheClearChecked(xui_context context,xui_internal_text_chunk_cache_t* cache)
{bool complete=false;return __xuiTextChunkCacheClear(context,cache,false,&complete);}
int xuiInternalTextChunkCacheDestroyPrepare(xui_context context,xui_internal_text_chunk_cache_t* cache,bool* complete)
{return __xuiTextChunkCacheClear(context,cache,true,complete);}

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
		iStart + (iTextSize-iStart<XUI_INTERNAL_TEXT_CHUNK_BYTES ? iTextSize-iStart:XUI_INTERNAL_TEXT_CHUNK_BYTES));
	if ( iTarget > iLimit ) iTarget = iLimit;
	if ( iTarget <= iStart ) iTarget = iLimit;
	iWindowEnd = __xuiTextChunkBoundaryAtOrAfter(sText, iTextSize, iTarget,
		iTarget + (iTextSize-iTarget<XUI_INTERNAL_TEXT_CHUNK_LOOKAHEAD ? iTextSize-iTarget:XUI_INTERNAL_TEXT_CHUNK_LOOKAHEAD));
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
		return iRet;
	}
	iRet = xuiInternalTextShapeCaretFragments(pChunk->sText,
		iWindowEnd - iStart, &pChunk->tShape);
	if ( iRet != XUI_OK ) {
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
		return XUI_ERROR_INVALID_STATE;
	}
	pChunk->pAdvances = (double*)xrtMalloc(
		((size_t)pChunk->tShape.iClusterCount + 1u) * sizeof(*pChunk->pAdvances));
	if ( pChunk->pAdvances == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pChunk->pAdvances[0] = 0.0;
	pChunk->iCoreClusterCount = 0;
	for ( i = 0; i < pChunk->tShape.iClusterCount; i++ ) {
		const xui_text_cluster_t* pCluster = &pChunk->tShape.pClusters[i];
		if ( !isfinite(pCluster->fAdvance) || pCluster->fAdvance < 0.0f ||
		     pCluster->iTextStart < 0 || pCluster->iTextEnd <= pCluster->iTextStart ||
		     pCluster->iTextEnd > iWindowEnd - iStart ) {
				return XUI_ERROR_INVALID_STATE;
		}
		fAdvance = pChunk->pAdvances[i] + (double)pCluster->fAdvance;
		if ( fAdvance < pChunk->pAdvances[i] ) {
				return XUI_ERROR_INVALID_STATE;
		}
		pChunk->pAdvances[i + 1] = fAdvance;
		if ( pCluster->iTextEnd <= iCoreBytes ) pChunk->iCoreClusterCount = i + 1;
		else if ( pCluster->iTextStart < iCoreBytes ) {
				return XUI_ERROR_UNSUPPORTED;
		}
	}
	if ( pChunk->iCoreClusterCount <= 0 ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	pChunk->iStart = iStart;
	pChunk->iEnd = iStart + iCoreBytes;
	pChunk->iShapeStart = iStart;
	pChunk->iShapeEnd = iWindowEnd;
	pChunk->fWidth = (float)pChunk->pAdvances[pChunk->iCoreClusterCount];
	return XUI_OK;
}

static int __xuiTextChunkBuildRange(xui_context context,
    xui_internal_text_chunk_cache_t* cache,xui_font font,
    const char* text,int bytes,int start,int end,int useContext)
{
    int at=start;
    while(at<end){
        if(cache->iChunkCount==INT_MAX)return XUI_ERROR_OUT_OF_MEMORY;
        int status=__xuiTextChunkReserve(cache,cache->iChunkCount+1);
        if(status!=XUI_OK)return status;
        /* Publish an actual owning slot before invoking the producer. Any
         * subsequent caret/index/realloc OOM leaves its paint in this same
         * directory. No temporary chunk ever owns a successful busy paint. */
        xui_internal_text_chunk_t* chunk=&cache->pChunks[cache->iChunkCount++];
        status=__xuiTextChunkBuildOne(context,font,text,bytes,at,end,useContext,chunk);
        if(status!=XUI_OK)return status;
        at=chunk->iEnd;
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
    if(pCache->iPreparation)return XUI_ERROR_BUSY;
	if ( pCache->bValid && pCache->pFont == pFont && pCache->iTextSize == iTextSize &&
	     pCache->iDpiGeneration == iDpiGeneration ) return XUI_OK;
	if ( xuiInternalTextCopyDisplay(sText, iTextSize, NULL) != iTextSize )
		return XUI_ERROR_UNSUPPORTED;
    iRet=xuiInternalTextChunkCacheClearChecked(pContext,pCache);if(iRet!=XUI_OK)return iRet;
	memset(&tCaps, 0, sizeof(tCaps));
	tCaps.iSize = sizeof(tCaps);
	if ( xuiGetProxyCaps(pContext, &tCaps) == XUI_OK )
		bContext = (tCaps.iCaps & XUI_PROXY_CAP_TEXT_CONTEXT) != 0;
	if ( !bContext && !__xuiTextChunkCanShapeIsolated(sText, iTextSize) )
		return XUI_ERROR_UNSUPPORTED;
    pCache->pFont=pFont;pCache->iTextSize=iTextSize;pCache->iDpiGeneration=iDpiGeneration;
    pCache->iPreparation=1;
	iRet = __xuiTextChunkBuildRange(pContext, pCache, pFont, sText, iTextSize,
		0, iTextSize, bContext);
	if ( iRet == XUI_OK ) iRet = __xuiTextChunkRebuildAdvances(pCache);
    pCache->iPreparation=0;
    if(iRet!=XUI_OK){
        xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,iRet);
        xui_cleanup_prefix_t prefix={0};__xuiTextCleanupPrefixBegin(pContext,&receipt,&prefix);
        xuiInternalCleanupAdd(&receipt,xuiInternalTextChunkCacheClearChecked(pContext,pCache));
        __xuiTextCleanupPrefixEnd(&receipt,&prefix);
        return xuiInternalCleanupEnd(NULL,&receipt);
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

void xuiInternalTextChunkCacheCancelEdit(xui_internal_text_chunk_cache_t* cache)
{
    if(cache && cache->iPreparation==2){cache->iPreparation=0;cache->bValid=0;}
}
int xuiInternalTextChunkCachePrepareEdit(xui_context context,xui_internal_text_chunk_cache_t* cache,
    int start,int oldEnd,int newEnd)
{
    if(!cache || !context)return XUI_ERROR_INVALID_ARGUMENT;
    if(cache->iPreparation)return XUI_ERROR_BUSY;
    if(start<0 || oldEnd<start || newEnd<start)return XUI_ERROR_INVALID_ARGUMENT;
    if(!cache->bValid)return xuiInternalTextChunkCacheClearChecked(context,cache);
    if(oldEnd>cache->iTextSize || newEnd-start>INT_MAX-(cache->iTextSize-(oldEnd-start)))return XUI_ERROR_INVALID_ARGUMENT;
    int first=__xuiTextChunkFindOffset(cache,start),last=__xuiTextChunkFindOffset(cache,oldEnd);
    if(first<0 || last<first)return XUI_ERROR_INVALID_STATE;
    if(first>0)--first;
    if(last+1<cache->iChunkCount)++last;
    /* Reserve before callbacks: a cleanup hook cannot recursively Ensure,
     * Patch, Draw, or cancel a half-consumed window. All Ready checks precede
     * the first physical owner consumption. */
    cache->iPreparation=1;
    for(int i=first;i<=last;++i){
        int ready=xuiInternalTextShapeReady(&cache->pChunks[i].tShape,context->pOwnedProxy!=NULL);
        if(ready!=XUI_OK){cache->iPreparation=0;return ready;}
    }
    cache->bValid=0;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    bool waiting=false;
    for(int i=first;i<=last;++i){
        bool consumed=false,called=false;
        int status=xuiInternalTextShapeFreeOutcome(&cache->pChunks[i].tShape,&consumed,&called);
        if(!consumed && !called && status==XUI_ERROR_BUSY){waiting=true;continue;}
        if(called)xuiInternalContextCleanupOutcome(context,status,&receipt.status,&receipt.error);
        else xuiInternalCleanupAdd(&receipt,status);
        if(!consumed)waiting=true;
    }
    cache->iPreparation=receipt.status==XUI_OK && !waiting ? 2:0;
    if(receipt.status==XUI_OK && waiting)receipt.status=XUI_ERROR_BUSY;
    cache->iEditStart=start;cache->iEditOldEnd=oldEnd;cache->iEditNewEnd=newEnd;
    return xuiInternalCleanupEnd(NULL,&receipt);
}

int xuiInternalTextChunkCachePatch(xui_context context,
    xui_internal_text_chunk_cache_t* cache,const char* text,int bytes,
    int start,int oldEnd,int newEnd)
{
    if(!context || !cache || !text)return XUI_ERROR_INVALID_STATE;
    if(cache->iPreparation==1)return XUI_ERROR_BUSY;
    if(cache->iPreparation==2){
        if(cache->iEditStart!=start || cache->iEditOldEnd!=oldEnd || cache->iEditNewEnd!=newEnd)return XUI_ERROR_INVALID_STATE;
    }else if(!cache->bValid)return XUI_ERROR_INVALID_STATE;
    int oldSize=cache->iTextSize;
    if(start<0 || oldEnd<start || oldEnd>oldSize || newEnd<start || newEnd>bytes ||
       (int64_t)bytes!=(int64_t)oldSize+newEnd-oldEnd)return XUI_ERROR_INVALID_ARGUMENT;
    if(bytes<XUI_INTERNAL_TEXT_CHUNK_THRESHOLD ||
       xuiInternalTextCopyDisplay(text+start,newEnd-start,NULL)!=newEnd-start){
        xuiInternalTextChunkCacheCancelEdit(cache);
        int status=xuiInternalTextChunkCacheClearChecked(context,cache);
        return status!=XUI_OK ? status:XUI_ERROR_UNSUPPORTED;
    }
    /* Older native callers may still call Patch after their edit. They get
     * the same real cleanup reservation; new mutable controls prepare before
     * editing so a pure wait leaves their source content unchanged. */
    if(cache->iPreparation==0){
        int status=xuiInternalTextChunkCachePrepareEdit(context,cache,start,oldEnd,newEnd);
        if(status!=XUI_OK)return status;
    }
    int oldCount=cache->iChunkCount,delta=newEnd-oldEnd;
    int first=__xuiTextChunkFindOffset(cache,start),last=__xuiTextChunkFindOffset(cache,oldEnd);
    if(first>0)--first;
    if(last+1<oldCount)++last;
    if(first<0 || last<first){cache->iPreparation=0;return XUI_ERROR_INVALID_STATE;}
    int regionStart=cache->pChunks[first].iStart,regionEnd=cache->pChunks[last].iEnd+delta;
    if(regionEnd<regionStart || regionEnd>bytes){cache->iPreparation=0;return XUI_ERROR_INVALID_STATE;}
    cache->iPreparation=1;cache->bValid=0;
    xui_proxy_caps_t caps={.iSize=sizeof(caps)};int useContext=0;
    if(xuiGetProxyCaps(context,&caps)==XUI_OK)useContext=(caps.iCaps&XUI_PROXY_CAP_TEXT_CONTEXT)!=0;
    int status=!useContext && !__xuiTextChunkCanShapeIsolated(text+start,newEnd-start) ? XUI_ERROR_UNSUPPORTED:XUI_OK;
    if(status==XUI_OK)status=__xuiTextChunkBuildRange(context,cache,cache->pFont,text,bytes,regionStart,regionEnd,useContext);
    /* Failure keeps both old and appended candidate owners in this one
     * invalid cache. The ordinary retry/DestroyPrepare drains that directory. */
    if(status!=XUI_OK){cache->iPreparation=0;return status;}
    int built=cache->iChunkCount-oldCount,tail=oldCount-last-1;
    if(built>INT_MAX-first-tail){cache->iPreparation=0;return XUI_ERROR_OUT_OF_MEMORY;}
    int count=first+built+tail;
    if((size_t)count>SIZE_MAX/sizeof(*cache->pChunks) || (size_t)count>=SIZE_MAX/sizeof(double)){
        cache->iPreparation=0;return XUI_ERROR_OUT_OF_MEMORY;
    }
    xui_internal_text_chunk_t* combined=xrtCalloc((size_t)(count ? count:1),sizeof(*combined));
    double* advances=xrtMalloc(((size_t)count+1)*sizeof(*advances));
    if(!combined || !advances){xrtFree(combined);xrtFree(advances);cache->iPreparation=0;return XUI_ERROR_OUT_OF_MEMORY;}
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    bool waiting=false;
    for(int i=first;i<=last;++i){
        xui_internal_text_chunk_t* chunk=&cache->pChunks[i];bool consumed=false,called=false;
        int result=xuiInternalTextShapeFreeOutcome(&chunk->tShape,&consumed,&called);
        if(!consumed && !called && result==XUI_ERROR_BUSY){waiting=true;continue;}
        if(called)xuiInternalContextCleanupOutcome(context,result,&receipt.status,&receipt.error);
        else xuiInternalCleanupAdd(&receipt,result);
        if(consumed){xrtFree(chunk->sText);xrtFree(chunk->pAdvances);chunk->sText=NULL;chunk->pAdvances=NULL;}
        else waiting=true;
    }
    if(waiting){
        xrtFree(combined);xrtFree(advances);cache->iPreparation=0;
        if(receipt.status==XUI_OK)receipt.status=XUI_ERROR_BUSY;
        return xuiInternalCleanupEnd(NULL,&receipt);
    }
    if(first)memcpy(combined,cache->pChunks,(size_t)first*sizeof(*combined));
    if(built)memcpy(combined+first,cache->pChunks+oldCount,(size_t)built*sizeof(*combined));
    if(tail)memcpy(combined+first+built,cache->pChunks+last+1,(size_t)tail*sizeof(*combined));
    for(int i=first+built;i<count;++i){combined[i].iStart+=delta;combined[i].iEnd+=delta;combined[i].iShapeStart+=delta;combined[i].iShapeEnd+=delta;}
    advances[0]=0;for(int i=0;i<count;++i)advances[i+1]=advances[i]+combined[i].fWidth;
    /* Every retained slot moves once; source arrays contain no extra owner. */
    xrtFree(cache->pChunks);xrtFree(cache->pChunkAdvances);
    cache->pChunks=combined;cache->pChunkAdvances=advances;cache->iChunkCount=cache->iChunkCapacity=count;
    cache->iTextSize=bytes;cache->iPreparation=0;cache->bValid=receipt.status==XUI_OK;
    return xuiInternalCleanupEnd(NULL,&receipt);
}

int xuiInternalTextChunkCacheAdvance(const xui_internal_text_chunk_cache_t* pCache,
	int iOffset, float* pAdvance)
{
	const xui_internal_text_chunk_t* pChunk;
	int iChunk;
	int iLocal;
	int iLo;
	int iHi;
	if ( pCache == NULL || pAdvance == NULL || (!pCache->bValid || pCache->iPreparation) ||
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
	if ( pCache == NULL || pOffset == NULL || (!pCache->bValid || pCache->iPreparation) ||
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
	if ( pCache == NULL || (!pCache->bValid || pCache->iPreparation) || pCache->pChunkAdvances == NULL ) return 0.0f;
	return (float)pCache->pChunkAdvances[pCache->iChunkCount];
}

static int __xuiInternalTextChunkCacheDraw(xui_proxy pProxy,
	xui_draw_context pDraw,
	const xui_internal_text_chunk_cache_t* pCache, xui_rect_t tViewport,
	float fOriginX, float fY, float fHeight, uint32_t iColor, uint32_t iFlags,
	const xui_text_paint_span_t* pSpans, int iSpanCount)
{
	xui_text_paint_span_t arrLocal[128];
	xui_text_paint_span_t* pDynamic = NULL;
	xui_text_paint_span_t* pLocal;
	xui_rect_t tOldClip;
	xui_rect_t tClip;
	int bOldClip = 0;
	int iDynamicCapacity = 0;
	int iRet;
	int iRestore;
	int iFirst;
	int iSpanFirst;
	int iSpanEnd;
	int iSpan;
	int i;
	if ( pProxy == NULL || pDraw == NULL || pCache == NULL || (!pCache->bValid || pCache->iPreparation) ||
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
	iSpanFirst = 0;
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
			while ( iSpanFirst < iSpanCount &&
			        pSpans[iSpanFirst].iEnd <= pChunk->iStart ) {
				iSpanFirst++;
			}
			iSpanEnd = iSpanFirst;
			while ( iSpanEnd < iSpanCount &&
			        pSpans[iSpanEnd].iStart < pChunk->iEnd ) {
				iSpanEnd++;
			}
			if ( iSpanEnd - iSpanFirst >
			     (int)(sizeof(arrLocal) / sizeof(arrLocal[0])) ) {
				int iNeed = iSpanEnd - iSpanFirst;
				xui_text_paint_span_t* pNew;

				if ( (size_t)iNeed > SIZE_MAX / sizeof(*pNew) ) {
					iRet = XUI_ERROR_LIMIT_EXCEEDED;
					break;
				}
				if ( iNeed > iDynamicCapacity ) {
					pNew = (xui_text_paint_span_t*)xrtRealloc(pDynamic,
						(size_t)iNeed * sizeof(*pNew));
					if ( pNew == NULL ) {
						iRet = XUI_ERROR_OUT_OF_MEMORY;
						break;
					}
					pDynamic = pNew;
					iDynamicCapacity = iNeed;
				}
				pLocal = pDynamic;
			} else {
				pLocal = arrLocal;
			}
			for ( iSpan = iSpanFirst; iSpan < iSpanEnd; iSpan++ ) {
				int iStart = pSpans[iSpan].iStart;
				int iEnd = pSpans[iSpan].iEnd;
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
	xrtFree(pDynamic);
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

static bool __xuiTextPaintProtocol(const xui_text_paint_ownership_v1_t* protocol)
{
    return protocol && protocol->size==sizeof(*protocol) &&
        protocol->AbiTag==XUI_TEXT_PAINT_OWNERSHIP_V1_TAG &&
        protocol->DropReady && protocol->TraceRaw && protocol->DropChecked;
}
bool xuiInternalTextShapeTraceOwnership(const xui_text_shape_t* shape,xrtownershipvisitor visit,void* user)
{
    if(!visit)return false;
    if(!shape || !shape->pPaint)return true;
    return __xuiTextPaintProtocol(shape->pPaintOwnership) &&
        shape->pPaintOwnership->TraceRaw(shape->pPaint,visit,user);
}
int xuiInternalTextShapeReady(const xui_text_shape_t* shape,bool managed)
{
    if(!shape || !shape->pPaint)return XUI_OK;
    if(!shape->pPaintOwnership)return managed ? XUI_ERROR_UNSUPPORTED:XUI_OK;
    if(!__xuiTextPaintProtocol(shape->pPaintOwnership))return XUI_ERROR_INVALID_ARGUMENT;
    return shape->pPaintOwnership->DropReady(shape->pPaint) ? XUI_OK:XUI_ERROR_BUSY;
}
int xuiInternalTextShapeFreeOutcome(xui_text_shape_t* pShape,bool* pConsumed,bool* pCalledDrop)
{
    if(!pConsumed || !pCalledDrop)return XUI_ERROR_INVALID_ARGUMENT;
    *pConsumed=false;*pCalledDrop=false;
    if(!pShape)return XUI_ERROR_INVALID_ARGUMENT;
    int ready=xuiInternalTextShapeReady(pShape,false);
    if(ready!=XUI_OK)return ready; /* no owner, arrays or fields consumed */
    /* Detach before arbitrary cleanup: recursive observation never sees the
     * same owning paint/arrays twice. The caller exclusively owns the output
     * record; a cleanup hook must not replace that record while it is active. */
    xui_text_shape_t retired=*pShape;memset(pShape,0,sizeof(*pShape));
    xerror* prior=xrtTakeError();int result=XUI_OK;bool consumed=retired.pPaint==NULL;
    if(retired.pPaint){
        if(retired.pPaintOwnership){
            *pCalledDrop=true;
            result=retired.pPaintOwnership->DropChecked(retired.pPaint);consumed=true;
        }else if(retired.paintFreeChecked){
            *pCalledDrop=true;
            result=retired.paintFreeChecked(retired.pPaint,&consumed);
            if(!consumed && result==XUI_OK)result=XUI_ERROR_INVALID_STATE;
        }else if(retired.paintFree){*pCalledDrop=true;retired.paintFree(retired.pPaint);consumed=true;}
        else result=XUI_ERROR_INVALID_STATE;
    }
    xerror* failure=xrtTakeError();
    if(result==XUI_OK && failure)result=XUI_ERROR_CALLBACK_FAILED;
    xrtFree(retired.pClusters);xrtFree(retired.pCarets);
    xerror* secondary=xrtTakeError();
    if(result==XUI_OK && secondary){result=XUI_ERROR_CALLBACK_FAILED;failure=secondary;}
    else xrtErrorFree(secondary);
    if(!consumed){
        pShape->iSize=retired.iSize;pShape->pPaint=retired.pPaint;
        pShape->iPaintBytes=retired.iPaintBytes;pShape->paintFree=retired.paintFree;
        pShape->paintFreeChecked=retired.paintFreeChecked;
        pShape->pPaintOwnership=retired.pPaintOwnership;
    }
    /* A pre-existing primary diagnostic has priority, independently of the
     * explicit cleanup result, including a false result with NULL text. */
    if(prior){xrtErrorFree(failure);failure=prior;}
    *pConsumed=consumed;
    xrtSetErrorTake(failure);return result;
}
XUI_API int xuiTextShapeFreeChecked(xui_text_shape_t* shape)
{
    bool consumed=false,called=false;int status=xuiInternalTextShapeFreeOutcome(shape,&consumed,&called);
    return !consumed && !called && status==XUI_ERROR_BUSY ? XUI_STATUS_DEFERRED:status;
}
XUI_API void xuiTextShapeFree(xui_text_shape_t* pShape)
{
    if(pShape)(void)xuiTextShapeFreeChecked(pShape);
}

static int __xuiTextShapeWithProxy(xui_proxy pProxy,const xui_proxy_caps_t* caps,const xui_text_item_t* pTextItem,xui_text_shape_t* pShape)
{
	xui_text_item_t normalized;
	xui_font pFont;
	const char* sText;
	int iTextSize;
	uint32_t iFlags;
	xui_font_metrics_t tMetrics;
	xui_vec2_t tSize;
	int iAt;
	int iNext;
	int iCount;
	int i;
	int iRet;

	if (pProxy==NULL || caps==NULL || pShape==NULL) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pShape, 0, sizeof(*pShape));
	iRet = __xuiTextItemNormalize(pTextItem, &normalized);
	if (iRet != XUI_OK) return iRet;
	pFont=normalized.pFont;sText=normalized.sText;iTextSize=normalized.iTextSize;iFlags=normalized.iFlags;
	pShape->iSize = sizeof(*pShape);
	pShape->iFlags = iFlags;
	pShape->iTextSize = iTextSize;
	if ( (normalized.sContext && !(caps->iCaps & XUI_PROXY_CAP_TEXT_CONTEXT)) ||
	     (normalized.iScript && !(caps->iCaps & XUI_PROXY_CAP_TEXT_SCRIPT)) ||
	     (normalized.sLanguage && !(caps->iCaps & XUI_PROXY_CAP_TEXT_LANGUAGE)) ||
	     ((iFlags & XUI_TEXT_SHAPE_RTL) && !(caps->iCaps & XUI_PROXY_CAP_TEXT_RTL)) ||
	     ((iFlags & XUI_TEXT_SHAPE_RANGE) && !(caps->iCaps & XUI_PROXY_CAP_TEXT_RANGE)) )
		return XUI_ERROR_UNSUPPORTED;
	if ( pProxy->textShape != NULL ) return pProxy->textShape(pProxy, &normalized, pShape);
	/* Isolated measurements cannot supply contextual shaping. */
	if ( (iFlags & (XUI_TEXT_SHAPE_RTL|XUI_TEXT_SHAPE_RANGE)) || pTextItem->sContext || pTextItem->iScript || pTextItem->sLanguage )
		return XUI_ERROR_UNSUPPORTED;

    if(!pProxy->fontGetMetrics || !pProxy->textMeasure)return XUI_ERROR_UNSUPPORTED;
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

XUI_API int xuiTextShape(xui_context context,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    if(!xuiInternalContextIsValid(context) || !shape)return XUI_ERROR_INVALID_ARGUMENT;
    memset(shape,0,sizeof(*shape));
    xui_proxy proxy=xuiInternalContextGetProxy(context);if(!proxy)return XUI_ERROR_NOT_INITIALIZED;
    xui_proxy_caps_t caps;int status=xuiGetProxyCaps(context,&caps);if(status!=XUI_OK)return status;
    return __xuiTextShapeWithProxy(proxy,&caps,item,shape);
}
XUI_API int xuiTextShapeGetClusters(const xui_text_shape_t* shape,const xui_text_cluster_t** output,size_t* count)
{
    if(!output || !count)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;*count=0;
    if(!shape || shape->iSize<sizeof(*shape) || shape->iClusterCount<0 ||
        (shape->iClusterCount && !shape->pClusters))return XUI_ERROR_INVALID_ARGUMENT;
    *output=shape->pClusters;*count=(size_t)shape->iClusterCount;return XUI_OK;
}
XUI_API int xuiTextShapeGetCarets(const xui_text_shape_t* shape,const xui_text_caret_t** output,size_t* count)
{
    if(!output || !count)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;*count=0;
    if(!shape || shape->iSize<sizeof(*shape) || shape->iCaretCount<0 ||
        (shape->iCaretCount && !shape->pCarets))return XUI_ERROR_INVALID_ARGUMENT;
    *output=shape->pCarets;*count=(size_t)shape->iCaretCount;return XUI_OK;
}
XUI_API int xuiTextShapeGetCluster(const xui_text_shape_t* shape,size_t index,xui_text_cluster_t* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    const xui_text_cluster_t* items=NULL;size_t count=0;
    int status=xuiTextShapeGetClusters(shape,&items,&count);if(status!=XUI_OK)return status;
    if(index>=count)return XUI_ERROR_INVALID_ARGUMENT;
    *output=items[index];return XUI_OK;
}
XUI_API int xuiTextShapeGetCaret(const xui_text_shape_t* shape,size_t index,xui_text_caret_t* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    const xui_text_caret_t* items=NULL;size_t count=0;
    int status=xuiTextShapeGetCarets(shape,&items,&count);if(status!=XUI_OK)return status;
    if(index>=count)return XUI_ERROR_INVALID_ARGUMENT;
    *output=items[index];return XUI_OK;
}
bool xuiInternalOwnedTextShapeReady(const void* payload)
{return xuiInternalTextShapeReady(payload,true)==XUI_OK;}
bool xuiInternalOwnedTextShapeTrace(const void* payload,xrtownershipvisitor visit,void* user)
{return xuiInternalTextShapeTraceOwnership(payload,visit,user);}
int xuiInternalOwnedTextShapeFinalize(xui_proxy proxy,void* payload)
{
    (void)proxy;
    /* Kind5 is admitted only with PaintV1. Its immutable record has exactly
     * one arrays/paint owner. Ready is stable for its caller's operation. */
    return xuiTextShapeFreeChecked(payload);
}
XUI_API int xuiTextShapeCreateOwned(xui_proxy proxy,const xui_text_item_t* item,const xui_text_shape_t** output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;
    if(!xuiProxyIsOwned(proxy) || !xuiInternalProxyReady(proxy))return XUI_ERROR_INVALID_STATE;
    if(!item || !item->pFont)return XUI_ERROR_INVALID_ARGUMENT;
    xui_proxy fontProxy=xuiFontGetProxy(item->pFont);
    if(!fontProxy || fontProxy->_pCapability!=proxy->_pCapability)return XUI_ERROR_INVALID_ARGUMENT;
    if(!proxy->getCaps)return XUI_ERROR_UNSUPPORTED;
    xui_proxy_caps_t caps={.iSize=sizeof(caps)};
    int status=proxy->getCaps(proxy,&caps);if(status!=XUI_OK)return status;
    xui_text_shape_t* shape=NULL;
    status=xuiInternalTextShapeAllocateOwned(proxy,&shape);if(status!=XUI_OK)return status;
    status=__xuiTextShapeWithProxy(proxy,&caps,item,shape);
    /* Readiness controls cleanup, not validity: an immutable shape with a
     * valid PaintV1 can be returned while an external native lease is busy. */
    if(status==XUI_OK && shape->pPaint && !__xuiTextPaintProtocol(shape->pPaintOwnership))status=XUI_ERROR_UNSUPPORTED;
    if(status!=XUI_OK){
        xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
        bool consumed=false;xuiInternalCleanupAdd(&receipt,xuiTextShapeReleaseOwnerChecked(shape,&consumed));
        /* Native output ownership is independent from status. A partial
         * producer result which cannot retire yet remains an explicit owner;
         * it must never be discarded on a failed factory path. */
        if(!consumed)*output=shape;
        return xuiInternalCleanupEnd(NULL,&receipt);
    }
    *output=shape;return XUI_OK;
}

static void __xuiTextLayoutContextGone(void* raw)
{
    xui_text_layout layout=raw;layout->pContext=NULL;
}
XUI_API xui_context xuiTextLayoutGetContext(xui_text_layout layout)
{return __xuiTextLayoutValid(layout) && !layout->bDestroying ? layout->pContext:NULL;}
int xuiInternalTextLayoutDestroyReady(xui_text_layout layout)
{
    if(!__xuiTextLayoutValid(layout))return XUI_ERROR_INVALID_ARGUMENT;
    if(layout->iCreatorThread!=xrtThreadCurrentId() || layout->bRefreshing || layout->bDestroying || layout->iCalls)return XUI_ERROR_BUSY;
    bool managed=layout->pContext && layout->pContext->pOwnedProxy;
    int status=xuiInternalTextShapeReady(&layout->tShape,managed || layout->bOwnedFont);
    return status==XUI_OK ? xuiInternalTextShapeReady(&layout->draft.tShape,managed || layout->draft.bOwnedFont):status;
}
XUI_API bool xuiTextLayoutOwnershipBusy(xui_text_layout layout)
{return xuiInternalTextLayoutDestroyReady(layout)!=XUI_OK;}
XUI_API bool xuiTextLayoutTraceOwnership(xui_text_layout layout,xrtownershipvisitor visit,void* user)
{
    /* Pure frozen graph discovery is distinct from cleanup readiness. A
     * known busy paint must remain traceable so preparation returns BUSY,
     * rather than misclassifying this explicit owner as opaque. */
    if(!visit || !__xuiTextLayoutValid(layout) || layout->bRefreshing || layout->bDestroying || layout->iCalls)return false;
    return __xuiTextLayoutPayloadTrace(&layout->active,visit,user) && __xuiTextLayoutPayloadTrace(&layout->draft,visit,user);
}
XUI_API int xuiTextLayoutRequestClose(xui_text_layout layout)
{
    if(!__xuiTextLayoutValid(layout) || layout->bDestroying)return XUI_ERROR_INVALID_ARGUMENT;
    if(layout->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    layout->bRetiring=true;return XUI_OK;
}

XUI_API int xuiTextLayoutLifetimeWatch(xui_text_layout layout,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidated,void* user)
{
    if(!__xuiTextLayoutValid(layout) || layout->bDestroying || !link || !invalidated)return XUI_ERROR_INVALID_ARGUMENT;
    if(layout->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(link->ppPrevious || link->pNext || link->onInvalidate || link->pUser)return XUI_ERROR_INVALID_STATE;
    link->pNext=layout->pLifetimeLinks;link->ppPrevious=&layout->pLifetimeLinks;link->onInvalidate=invalidated;link->pUser=user;
    if(link->pNext)link->pNext->ppPrevious=&link->pNext;
    layout->pLifetimeLinks=link;return XUI_OK;
}
XUI_API int xuiTextLayoutCreate(xui_context context,xui_text_layout* output,const xui_text_layout_desc_t* desc)
{
    if(!output || !xuiInternalContextIsValid(context) || !__xuiTextDescValid(desc))return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;if(!xuiInternalContextGetProxy(context))return XUI_ERROR_NOT_INITIALIZED;
    xui_text_layout layout=xrtCalloc(1,sizeof(*layout));if(!layout)return XUI_ERROR_OUT_OF_MEMORY;
    layout->iMagic=XUI_TEXT_LAYOUT_MAGIC;layout->pContext=context;layout->iCreatorThread=xrtThreadCurrentId();
    int result=xuiContextLifetimeWatch(context,&layout->tContextLink,__xuiTextLayoutContextGone,layout);
    if(result==XUI_OK)result=xuiTextLayoutReset(layout,desc);
    if(result!=XUI_OK){
        xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,result);
        xui_context cleanupContext=layout->pContext;xui_call_scope_t cleanupScope={0};
        if(cleanupContext)xuiInternalCleanupAdd(&receipt,xuiCallScopeBegin(cleanupContext,&cleanupScope));
        xui_cleanup_prefix_t prefix={0};__xuiTextCleanupPrefixBegin(cleanupContext,&receipt,&prefix);
        bool consumed=false;int cleanupStatus=xuiTextLayoutDestroyOwnerChecked(layout,&consumed);
        if(cleanupStatus!=XUI_STATUS_DEFERRED)xuiInternalContextCleanupOutcome(cleanupContext,cleanupStatus,&receipt.status,&receipt.error);
        __xuiTextCleanupPrefixEnd(&receipt,&prefix);
        receipt.status=xuiInternalCallScopeEndOutcome(&cleanupScope,receipt.status,&receipt.error);
        if(!consumed)*output=layout; /* status does not erase native ownership */
        return xuiInternalCleanupEnd(NULL,&receipt);
    }
    *output=layout;return XUI_OK;
}
XUI_API int xuiTextLayoutDestroyOwnerChecked(xui_text_layout layout,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;if(!__xuiTextLayoutValid(layout))return XUI_ERROR_INVALID_ARGUMENT;
    if(layout->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(layout->bDestroying)return XUI_ERROR_INVALID_STATE;
    layout->bRetiring=true;
    if(layout->bRefreshing || layout->iCalls)return XUI_STATUS_DEFERRED;
    xui_context context=layout->pContext;xui_call_scope_t scope={0};
    if(context){int status=xuiInternalCleanupCallScopeBegin(context,&scope);if(status!=XUI_OK)return status;}
    layout->bRefreshing=1;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    bool draftDone=false,activeDone=false;
    int status=__xuiTextLayoutPayloadClear(context,&layout->draft,&draftDone,NULL);
    if(status!=XUI_STATUS_DEFERRED)xuiInternalCleanupAdd(&receipt,status);
    /* Draft may borrow active text/font during a failed DPI rebuild. Clear
     * active only after draft is fully gone; no borrowed key can outlive it. */
    if(draftDone){
        status=__xuiTextLayoutPayloadClear(context,&layout->active,&activeDone,NULL);
        if(status!=XUI_STATUS_DEFERRED)xuiInternalCleanupAdd(&receipt,status);
    }
    if(!draftDone || !activeDone){
        layout->bRefreshing=0;
        receipt.status=xuiInternalCallScopeEndOutcome(&scope,receipt.status,&receipt.error);
        int result=xuiInternalCleanupEnd(NULL,&receipt);
        return result==XUI_OK ? XUI_STATUS_DEFERRED:result;
    }
    layout->bDestroying=true;*consumed=true;
    xuiLifetimeUnwatch(&layout->tContextLink);xuiInternalLifetimeInvalidateAll(&layout->pLifetimeLinks);
    xuiInternalContextCleanupOutcome(context,XUI_OK,&receipt.status,&receipt.error);
    xrtFree(layout->pScratch);layout->iMagic=0;xrtFree(layout);
    receipt.status=xuiInternalCallScopeEndOutcome(&scope,receipt.status,&receipt.error);
    return xuiInternalCleanupEnd(NULL,&receipt);
}
XUI_API int xuiTextLayoutDestroyChecked(xui_text_layout layout)
{bool consumed=false;return xuiTextLayoutDestroyOwnerChecked(layout,&consumed);}
XUI_API void xuiTextLayoutDestroy(xui_text_layout layout)
{if(layout)(void)xuiTextLayoutDestroyChecked(layout);}
int xuiInternalTextLayoutDestroy(xui_context context,xui_text_layout layout)
{
    int status=layout ? xuiTextLayoutDestroyChecked(layout):XUI_OK;
    if(status<0)xuiInternalContextRecordFailure(context,status);
    return status;
}

static int __xuiTextLayoutCompute(xui_text_layout pLayout, xui_proxy pProxy)
{
	xui_context context=pLayout->pContext;
    uint32_t iDpiGeneration=context->iDpiGeneration;
	int iRet, i;

	iRet = pProxy->fontGetMetrics(pProxy, pLayout->tDesc.pFont, &pLayout->tMetrics);
	if ( iRet != XUI_OK ) return iRet;
    if(context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
	if ( pLayout->tMetrics.fLineHeight <= 0.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTextBuildBreakIndex(pLayout);
	if ( iRet != XUI_OK ) return iRet;
    if(context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
	iRet = __xuiTextShapeProjection(pLayout, 0, NULL);
	if ( iRet != XUI_OK ) return iRet;
    if(context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
	if ( pLayout->tShape.fLineHeight > 0.0f ) {
		pLayout->tMetrics.fAscent = pLayout->tShape.fAscent;
		pLayout->tMetrics.fDescent = pLayout->tShape.fDescent;
		pLayout->tMetrics.fLineHeight = pLayout->tShape.fLineHeight;
	}
	if ( pLayout->tDesc.iWrapMode == XUI_TEXT_WRAP_WORD && __xuiTextMaxFinite(pLayout->tDesc.fMaxWidth) ) {
		for ( i = 0; i + 1 < pLayout->iTextSize; i++ ) {
			if ( (unsigned char)pLayout->sText[i] == 0xc2u && (unsigned char)pLayout->sText[i + 1] == 0xadu ) {
                xui_vec2_t hyphenSize={0};
                if(!pProxy->textMeasure)return XUI_ERROR_UNSUPPORTED;
                xui_text_item_t hyphen={.iSize=sizeof(hyphen),.pFont=pLayout->tDesc.pFont,.sText="-",.iTextSize=1};
                iRet=pProxy->textMeasure(pProxy,&hyphen,&hyphenSize);
                if(iRet==XUI_OK){pLayout->fHyphenWidth=hyphenSize.fX;
                    if(!__xuiTextFloatValid(pLayout->fHyphenWidth))iRet=XUI_ERROR_INVALID_ARGUMENT;}
				if ( iRet != XUI_OK ) return iRet;
    if(context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
				break;
			}
		}
	}
	iRet = __xuiTextLayoutBuild(pLayout);
	if ( iRet != XUI_OK ) return iRet;
    if(context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
	if ( context->bCloseRequested || context->bDestroyPending || context->bDestroying || context->iDpiGeneration != iDpiGeneration ) return XUI_ERROR_INVALID_STATE;
	pLayout->iDpiGeneration = iDpiGeneration;
	return XUI_OK;
}

static int __xuiTextLayoutEnd(xui_text_layout layout,xui_context context,xui_call_scope_t* scope,xui_cleanup_receipt_t* receipt,bool report)
{
    if(receipt->status==XUI_OK && (layout->pContext!=context || context->bCloseRequested || context->bDestroyPending || context->bDestroying))
        xuiInternalCleanupAdd(receipt,XUI_ERROR_INVALID_STATE);
    layout->bRefreshing=0;
    /* Value-returning accessors cannot return the operation status. */
    if(report)xuiInternalContextRecordOutcome(context,receipt->status,receipt->error);
    /* Context teardown may destroy an internally owned layout at this point. */
    receipt->status=xuiInternalCallScopeEndOutcome(scope,receipt->status,&receipt->error);
    return xuiInternalCleanupEnd(NULL,receipt);
}
/* Build only in a persistent payload. Failure can retain one busy draft;
 * active output stays usable, and the next reset first retires that draft. */
static int __xuiTextLayoutRebuild(xui_text_layout layout,const xui_text_layout_desc_t* desc,bool dpi,bool report)
{
    xui_context context=layout->pContext;
    xui_proxy proxy=xuiInternalContextGetProxy(context);if(!proxy)return XUI_ERROR_NOT_INITIALIZED;
    xui_call_scope_t scope={0};int result=xuiCallScopeBegin(context,&scope);if(result!=XUI_OK)return result;
    layout->bRefreshing=1;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    bool complete=false;
    result=__xuiTextLayoutPayloadClear(context,&layout->draft,&complete,NULL);
    if(!complete){
        xuiInternalCleanupAdd(&receipt,result==XUI_STATUS_DEFERRED ? XUI_ERROR_BUSY:result);
        return __xuiTextLayoutEnd(layout,context,&scope,&receipt,report);
    }
    xuiInternalCleanupAdd(&receipt,result);
    if(receipt.status==XUI_OK && (layout->bRetiring || context->bCloseRequested || context->bDestroyPending || context->bDestroying))xuiInternalCleanupAdd(&receipt,XUI_ERROR_INVALID_STATE);
    if(receipt.status!=XUI_OK)return __xuiTextLayoutEnd(layout,context,&scope,&receipt,report);
    xui_text_layout_payload_t* candidate=&layout->draft;
    candidate->tDesc=*desc;candidate->tDesc.iSize=sizeof(candidate->tDesc);
    if(dpi){
        candidate->sText=layout->sText;candidate->iTextSize=layout->iTextSize;
        /* Borrowed from active until commit, never a duplicate owner. */
    }else{
        size_t bytes=desc->iTextSize<0 ? strlen(desc->sText):(size_t)desc->iTextSize;
        candidate->iTextSize=(int)bytes;
        candidate->sText=xrtMalloc(bytes+1);
        if(!candidate->sText)result=XUI_ERROR_OUT_OF_MEMORY;
        else {candidate->bOwnedText=true;memcpy(candidate->sText,desc->sText,bytes);candidate->sText[bytes]=0;}
        if(result==XUI_OK){
            result=xuiInternalFontAdmit(context,desc->pFont);
            if(result==XUI_OK)candidate->bOwnedFont=context->pOwnedProxy!=NULL;
        }
    }
    candidate->tDesc.sText=candidate->sText;
    bool swapped=false;
    if(result==XUI_OK){__xuiTextLayoutSwapPayload(layout);swapped=true;result=__xuiTextLayoutCompute(layout,proxy);}
    if(result==XUI_OK && (layout->bRetiring || layout->pContext!=context || context->bCloseRequested || context->bDestroyPending || context->bDestroying))result=XUI_ERROR_INVALID_STATE;
    if(result==XUI_OK && xrtGetError())result=XUI_ERROR_CALLBACK_FAILED;
    xuiInternalCleanupAdd(&receipt,result);
    if(result==XUI_OK){
        /* A DPI candidate borrows these immutable inputs. Move their real
         * ownership before retiring the old geometry; never AddRef or copy. */
        if(dpi){layout->bOwnedFont=layout->draft.bOwnedFont;layout->draft.bOwnedFont=false;
            layout->bOwnedText=layout->draft.bOwnedText;layout->draft.bOwnedText=false;}
        bool changed=false;
        int clear=__xuiTextLayoutPayloadClear(context,&layout->draft,&complete,&changed);
        if(!complete && !changed){
            if(dpi){layout->draft.bOwnedFont=layout->bOwnedFont;layout->bOwnedFont=false;
                layout->draft.bOwnedText=layout->bOwnedText;layout->bOwnedText=false;}
            __xuiTextLayoutSwapPayload(layout);
            xuiInternalCleanupAdd(&receipt,clear==XUI_STATUS_DEFERRED ? XUI_ERROR_BUSY:clear);
        }else {
            /* Cleanup may already have consumed old geometry before a
             * later font waits. Keep new active output and the real partial
             * draft; never restore a payload whose owner was consumed. */
            xuiInternalCleanupAdd(&receipt,clear==XUI_STATUS_DEFERRED ? XUI_ERROR_BUSY:clear);
        }
    }else{
        if(swapped)__xuiTextLayoutSwapPayload(layout);
        xui_cleanup_prefix_t prefix={0};__xuiTextCleanupPrefixBegin(context,&receipt,&prefix);
        int clear=__xuiTextLayoutPayloadClear(context,&layout->draft,&complete,NULL);
        if(clear!=XUI_STATUS_DEFERRED)xuiInternalCleanupAdd(&receipt,clear);
        __xuiTextCleanupPrefixEnd(&receipt,&prefix);
    }
    return __xuiTextLayoutEnd(layout,context,&scope,&receipt,report);
}
XUI_API int xuiTextLayoutReset(xui_text_layout layout,const xui_text_layout_desc_t* desc)
{
    if(!__xuiTextLayoutValid(layout) || !__xuiTextDescValid(desc))return XUI_ERROR_INVALID_ARGUMENT;
    if(layout->bRetiring)return XUI_ERROR_INVALID_STATE;
    if(layout->bRefreshing || layout->bDestroying || layout->iCalls || layout->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!xuiInternalContextIsValid(layout->pContext))return XUI_ERROR_INVALID_STATE;
    xui_text_layout_desc_t snapshot=*desc;
    size_t bytes=snapshot.iTextSize<0 ? strlen(snapshot.sText):(size_t)snapshot.iTextSize;
    if(bytes>INT_MAX)return XUI_ERROR_INVALID_ARGUMENT;
    int ready=xuiInternalTextShapeReady(&layout->tShape,layout->bOwnedFont);
    if(ready!=XUI_OK)return ready;
    return __xuiTextLayoutRebuild(layout,&snapshot,false,false);
}
static int __xuiTextLayoutEnsureDpi(xui_text_layout layout,bool report)
{
    if(!__xuiTextLayoutValid(layout))return XUI_ERROR_INVALID_ARGUMENT;
    if(layout->bRetiring)return XUI_ERROR_INVALID_STATE;
    if(layout->bRefreshing || layout->bDestroying || layout->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xui_context context=layout->pContext;
    if(!xuiInternalContextIsValid(context))return XUI_ERROR_INVALID_STATE;
    if(layout->iDpiGeneration==context->iDpiGeneration)return XUI_OK;
    if(layout->iCalls)return XUI_ERROR_BUSY;
    if(!layout->sText)return XUI_ERROR_INVALID_STATE;
    xui_text_layout_desc_t snapshot=layout->tDesc;
    return __xuiTextLayoutRebuild(layout,&snapshot,true,report);
}

XUI_API xui_vec2_t xuiTextLayoutGetSize(xui_text_layout pLayout)
{
	xui_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( __xuiTextLayoutEnsureDpi(pLayout,true) == XUI_OK ) {
		tSize = pLayout->tSize;
	}
	return tSize;
}

XUI_API int xuiTextLayoutGetLineCount(xui_text_layout pLayout)
{
	return __xuiTextLayoutEnsureDpi(pLayout,true) == XUI_OK ? pLayout->iLineCount : 0;
}

XUI_API int xuiTextLayoutGetLine(xui_text_layout pLayout, int iIndex, xui_text_line_t* pLine)
{
	int iRet;
	if ( !__xuiTextLayoutValid(pLayout) || (pLine == NULL) || (iIndex < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTextLayoutEnsureDpi(pLayout,false);
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
    if(__xuiTextLayoutEnsureDpi(pLayout,true)!=XUI_OK)return NULL;
	return pLayout->sText;
}

XUI_API int xuiTextLayoutGetTruncated(xui_text_layout pLayout)
{
	return __xuiTextLayoutEnsureDpi(pLayout,true) == XUI_OK ? pLayout->bTruncated : 0;
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
	iRet = __xuiTextLayoutEnsureDpi(pLayout,true);
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
	iRet = __xuiTextLayoutEnsureDpi(pLayout,true);
	if ( iRet != XUI_OK ) return iRet;
	if ( iLimit > pLayout->iTextSize ) return XUI_ERROR_INVALID_ARGUMENT;
	*pNext = (int)(__xuiTextNextLayoutCluster(pLayout, pLayout->sText + iOffset,
		pLayout->sText + iLimit) - pLayout->sText);
	return XUI_OK;
}

static int __xuiTextLayoutPixelLineRect(xui_rect_t tRect, float fOffsetY,
	const xui_text_line_t* pLine, uint32_t iFlags, xui_rect_t* pLineRect)
{
	double fTop;
	double fBottom;
	double fClipTop;
	double fClipBottom;
	long long iHeight;
	int iTop;
	int iBottom;

	if ( pLine == NULL || pLineRect == NULL ) return 0;
	fTop = (double)tRect.fY + (double)fOffsetY + (double)pLine->fY;
	fBottom = fTop + (double)pLine->fH;
	if ( !isfinite(fTop) || !isfinite(fBottom) || fBottom <= fTop ) return 0;
	if ( (iFlags & XUI_TEXT_CLIP) != 0 ) {
		fClipTop = (double)tRect.fY;
		fClipBottom = fClipTop + (double)tRect.fH;
		if ( fBottom <= fClipTop || fTop >= fClipBottom ) return 0;
		if ( fTop < fClipTop ) fTop = fClipTop;
		if ( fBottom > fClipBottom ) fBottom = fClipBottom;
	}
	if ( fTop <= (double)INT_MIN ) iTop = INT_MIN;
	else if ( fTop >= (double)INT_MAX ) iTop = INT_MAX;
	else iTop = (int)floor(fTop);
	if ( fBottom <= (double)INT_MIN ) iBottom = INT_MIN;
	else if ( fBottom >= (double)INT_MAX ) iBottom = INT_MAX;
	else iBottom = (int)ceil(fBottom);
	iHeight = (long long)iBottom - (long long)iTop;
	if ( iHeight <= 0 ) return 0;
	if ( iHeight > INT_MAX ) iHeight = INT_MAX;
	pLineRect->fX = tRect.fX;
	pLineRect->fY = iTop;
	pLineRect->fW = tRect.fW;
	pLineRect->fH = (int)iHeight;
	return 1;
}

static int __xuiTextLayoutDrawBody(xui_text_layout pLayout, xui_surface pTarget,
	xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor,
	uint32_t iFlags, int bSkipEmptyDisplay)
{
	xui_proxy pProxy;
	xui_text_line_t* pLine;
	xui_text_item_t tItem;
	xui_rect_t tLineRect;
	uint32_t iMergedFlags;
	uint32_t iLineFlags;
	float fOffsetY;
	int i;
	int iRet;

	if ( !__xuiTextLayoutValid(pLayout) || ((pTarget == NULL) == (pDraw == NULL)) ||
	     (tRect.fW <= 0) || (tRect.fH <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pLayout->pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( (pDraw != NULL && pProxy->drawText == NULL) ||
	     (pTarget != NULL && pProxy->textDraw == NULL) ) {
		return XUI_ERROR_UNSUPPORTED;
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
		if ( bSkipEmptyDisplay && iDisplaySize <= 0 ) continue;
		if ( !__xuiTextLayoutPixelLineRect(tRect, fOffsetY, pLine,
		     iMergedFlags, &tLineRect) ) continue;
		memset(&tItem, 0, sizeof(tItem));
		tItem.iSize = sizeof(tItem);
		tItem.pFont = pLayout->tDesc.pFont;
		tItem.sText = sDisplay;
		tItem.iTextSize = iDisplaySize;
		tItem.iFlags = XUI_TEXT_SHAPE_DEFAULT |
			((iLineFlags & XUI_TEXT_RTL) ? XUI_TEXT_SHAPE_RTL : 0);
		if ( pDraw != NULL ) {
			iRet = pProxy->drawText(pProxy, pDraw, &tItem, tLineRect,
				iColor, iLineFlags);
		} else {
			iRet = pProxy->textDraw(pProxy, pTarget, &tItem, tLineRect,
				iColor, iLineFlags);
		}
		if ( iRet != XUI_OK ) return iRet;
        if(!pLayout->pContext || pLayout->pContext->bCloseRequested || pLayout->pContext->bDestroyPending || pLayout->pContext->bDestroying)return XUI_ERROR_INVALID_STATE;
	}
	return XUI_OK;
}

static int __xuiTextLayoutDraw(xui_text_layout layout,xui_surface target,xui_draw_context draw,
    xui_rect_t rect,uint32_t color,uint32_t flags,int skipEmpty)
{
    int result=__xuiTextLayoutEnsureDpi(layout,false);if(result!=XUI_OK)return result;
    xui_context context=layout->pContext;xui_call_scope_t scope={0};
    result=xuiCallScopeBegin(context,&scope);if(result!=XUI_OK)return result;
    ++layout->iCalls;
    result=__xuiTextLayoutDrawBody(layout,target,draw,rect,color,flags,skipEmpty);
    --layout->iCalls;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,result);
    return __xuiTextLayoutEnd(layout,context,&scope,&receipt,false);
}

int xuiInternalTextLayoutDrawContext(xui_text_layout pLayout,
	xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor,
	uint32_t iFlags)
{
	return __xuiTextLayoutDraw(pLayout, NULL, pDraw, tRect, iColor,
		iFlags, 1);
}

XUI_API int xuiTextLayoutDraw(xui_text_layout pLayout, xui_surface pTarget,
	xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	return __xuiTextLayoutDraw(pLayout, pTarget, NULL, tRect, iColor,
		iFlags, 0);
}

int xuiInternalTextLayoutEnsure(xui_context context,xui_text_layout* slot,const xui_text_layout_desc_t* desc)
{
    if(!slot || !__xuiTextDescValid(desc) || !xuiInternalContextIsValid(context))return XUI_ERROR_INVALID_ARGUMENT;
    if(context->bCloseRequested)return XUI_ERROR_INVALID_STATE;
    if(*slot && ((*slot)->bRetiring || (*slot)->pContext!=context)){
        bool consumed=false;int status=xuiTextLayoutDestroyOwnerChecked(*slot,&consumed);
        if(consumed)*slot=NULL;
        if(status!=XUI_OK)return status==XUI_STATUS_DEFERRED ? XUI_ERROR_BUSY:status;
    }
    return *slot ? xuiTextLayoutReset(*slot,desc):xuiTextLayoutCreate(context,slot,desc);
}
bool xuiInternalTextMeasureCacheTrace(xui_context context,xrtownershipvisitor visit,void* user)
{
    if(!context || !visit || context->bMeasureLayoutActive)return false;
    return !context->pMeasureLayout || xuiTextLayoutTraceOwnership(context->pMeasureLayout,visit,user);
}
int xuiInternalTextMeasureCacheReady(xui_context context)
{
    if(!context)return XUI_ERROR_INVALID_ARGUMENT;
    if(context->bMeasureLayoutActive)return XUI_ERROR_BUSY;
    return context->pMeasureLayout ? xuiInternalTextLayoutDestroyReady(context->pMeasureLayout):XUI_OK;
}
int xuiInternalTextMeasureCachePrepare(xui_context context,bool* complete)
{
    if(!context || !complete)return XUI_ERROR_INVALID_ARGUMENT;
    *complete=false;if(context->bMeasureLayoutActive)return XUI_STATUS_DEFERRED;
    if(!context->pMeasureLayout){*complete=true;return XUI_OK;}
    context->bMeasureLayoutActive=true;
    bool consumed=false;int status=xuiTextLayoutDestroyOwnerChecked(context->pMeasureLayout,&consumed);
    if(consumed)context->pMeasureLayout=NULL;
    context->bMeasureLayoutActive=false;*complete=consumed;
    return status;
}
XUI_API int xuiTextMeasureLayout(xui_context context,const xui_text_layout_desc_t* desc,xui_vec2_t* size)
{
    if(!size || !__xuiTextDescValid(desc) || !xuiInternalContextIsValid(context))return XUI_ERROR_INVALID_ARGUMENT;
    *size=(xui_vec2_t){0};
    if(context->bMeasureLayoutActive)return XUI_ERROR_BUSY;
    xui_call_scope_t scope={0};int result=xuiCallScopeBegin(context,&scope);
    if(result!=XUI_OK)return result;
    context->bMeasureLayoutActive=true;
    if(context->pMeasureLayout && context->pMeasureLayout->bRetiring){
        bool consumed=false;result=xuiTextLayoutDestroyOwnerChecked(context->pMeasureLayout,&consumed);
        if(consumed)context->pMeasureLayout=NULL;
        if(result==XUI_STATUS_DEFERRED)result=XUI_ERROR_BUSY;
    }
    if(result==XUI_OK){
        if(context->pMeasureLayout)result=xuiTextLayoutReset(context->pMeasureLayout,desc);
        else result=xuiTextLayoutCreate(context,&context->pMeasureLayout,desc);
    }
    if(result==XUI_OK && (context->bCloseRequested || context->bDestroyPending || context->bDestroying))result=XUI_ERROR_INVALID_STATE;
    if(result==XUI_OK)*size=context->pMeasureLayout->tSize;
    xerror* first=xrtTakeError();context->bMeasureLayoutActive=false;
    result=xuiInternalCallScopeEndOutcome(&scope,result,&first);xrtSetErrorTake(first);return result;
}

#endif
