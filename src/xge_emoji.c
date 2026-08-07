typedef struct xge_emoji_entry_t {
	char* sSequence;
	int iSequenceSize;
	unsigned char* pSvgData;
	int iSvgSize;
	xge_emoji_metrics_t tMetrics;
	xge_svg pSvg;
} xge_emoji_entry_t;

typedef struct xge_emoji_trie_node_t {
	uint32_t iCodepoint;
	int iChild;
	int iSibling;
	int iEntry;
} xge_emoji_trie_node_t;

struct xge_emoji_pack_t {
	int iRefCount;
	xge_emoji_entry_t* pEntries;
	int iEntryCount;
	int iEntryCapacity;
	xge_emoji_trie_node_t* pNodes;
	int iNodeCount;
	int iNodeCapacity;
};

typedef struct xge_builtin_emoji_source_t {
	const unsigned char* pSequence;
	int iSequenceSize;
	const unsigned char* pSvg;
	int iSvgSize;
	uint32_t iFlags;
} xge_builtin_emoji_source_t;

#include "xge_builtin_emoji.inc"

static xge_emoji_pack g_xgeBuiltinEmojiPack;
static xge_emoji_pack g_xgeDefaultEmojiPack;

static int __xgeEmojiUTF8Decode(const char** ppScan, const char* sEnd, uint32_t* pCodepoint)
{
	const unsigned char* s;
	uint32_t c;
	int iLength;
	int i;

	if ( (ppScan == NULL) || (*ppScan == NULL) || (pCodepoint == NULL) || (*ppScan >= sEnd) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	s = (const unsigned char*)*ppScan;
	if ( s[0] < 0x80u ) {
		*pCodepoint = s[0];
		*ppScan += 1;
		return XGE_OK;
	}
	if ( (s[0] & 0xe0u) == 0xc0u ) {
		c = s[0] & 0x1fu;
		iLength = 2;
	} else if ( (s[0] & 0xf0u) == 0xe0u ) {
		c = s[0] & 0x0fu;
		iLength = 3;
	} else if ( (s[0] & 0xf8u) == 0xf0u ) {
		c = s[0] & 0x07u;
		iLength = 4;
	} else {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( *ppScan + iLength > sEnd ) return XGE_ERROR_RESOURCE_FAILED;
	for ( i = 1; i < iLength; i++ ) {
		if ( (s[i] & 0xc0u) != 0x80u ) return XGE_ERROR_RESOURCE_FAILED;
		c = (c << 6) | (uint32_t)(s[i] & 0x3fu);
	}
	if ( ((iLength == 2) && (c < 0x80u)) ||
	     ((iLength == 3) && (c < 0x800u)) ||
	     ((iLength == 4) && (c < 0x10000u)) ||
	     (c > 0x10ffffu) || ((c >= 0xd800u) && (c <= 0xdfffu)) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	*pCodepoint = c;
	*ppScan += iLength;
	return XGE_OK;
}

static int __xgeEmojiEnsureEntries(xge_emoji_pack pPack, int iRequired)
{
	xge_emoji_entry_t* pEntries;
	int iCapacity;

	if ( iRequired <= pPack->iEntryCapacity ) return XGE_OK;
	iCapacity = (pPack->iEntryCapacity > 0) ? pPack->iEntryCapacity * 2 : 16;
	while ( iCapacity < iRequired ) iCapacity *= 2;
	pEntries = (xge_emoji_entry_t*)xrtRealloc(pPack->pEntries, sizeof(*pEntries) * (size_t)iCapacity);
	if ( pEntries == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	memset(pEntries + pPack->iEntryCapacity, 0, sizeof(*pEntries) * (size_t)(iCapacity - pPack->iEntryCapacity));
	pPack->pEntries = pEntries;
	pPack->iEntryCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeEmojiEnsureNodes(xge_emoji_pack pPack, int iRequired)
{
	xge_emoji_trie_node_t* pNodes;
	int iCapacity;

	if ( iRequired <= pPack->iNodeCapacity ) return XGE_OK;
	iCapacity = (pPack->iNodeCapacity > 0) ? pPack->iNodeCapacity * 2 : 32;
	while ( iCapacity < iRequired ) iCapacity *= 2;
	pNodes = (xge_emoji_trie_node_t*)xrtRealloc(pPack->pNodes, sizeof(*pNodes) * (size_t)iCapacity);
	if ( pNodes == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	memset(pNodes + pPack->iNodeCapacity, 0, sizeof(*pNodes) * (size_t)(iCapacity - pPack->iNodeCapacity));
	pPack->pNodes = pNodes;
	pPack->iNodeCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeEmojiFindChild(xge_emoji_pack pPack, int iParent, uint32_t iCodepoint)
{
	int iNode;

	iNode = pPack->pNodes[iParent].iChild;
	while ( iNode >= 0 ) {
		if ( pPack->pNodes[iNode].iCodepoint == iCodepoint ) return iNode;
		iNode = pPack->pNodes[iNode].iSibling;
	}
	return -1;
}

static xge_emoji_metrics_t __xgeEmojiMetricsDefault(void)
{
	xge_emoji_metrics_t tMetrics;

	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.fAdvanceEm = 1.0f;
	tMetrics.fWidthEm = 1.0f;
	tMetrics.fHeightEm = 1.0f;
	tMetrics.fBaselineRatio = 0.82f;
	return tMetrics;
}

int xgeEmojiPackCreate(xge_emoji_pack* ppPack)
{
	xge_emoji_pack pPack;

	if ( ppPack == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*ppPack = NULL;
	pPack = (xge_emoji_pack)xrtCalloc(1, sizeof(*pPack));
	if ( pPack == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	if ( __xgeEmojiEnsureNodes(pPack, 1) != XGE_OK ) {
		xrtFree(pPack);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPack->iRefCount = 1;
	pPack->iNodeCount = 1;
	pPack->pNodes[0].iChild = -1;
	pPack->pNodes[0].iSibling = -1;
	pPack->pNodes[0].iEntry = -1;
	*ppPack = pPack;
	return XGE_OK;
}

int xgeEmojiPackAddRef(xge_emoji_pack pPack)
{
	if ( (pPack == NULL) || (pPack->iRefCount <= 0) ) return XGE_ERROR_INVALID_ARGUMENT;
	pPack->iRefCount++;
	return pPack->iRefCount;
}

void xgeEmojiPackFree(xge_emoji_pack pPack)
{
	int i;

	if ( (pPack == NULL) || (pPack->iRefCount <= 0) ) return;
	pPack->iRefCount--;
	if ( pPack->iRefCount > 0 ) return;
	for ( i = 0; i < pPack->iEntryCount; i++ ) {
		xrtFree(pPack->pEntries[i].sSequence);
		xrtFree(pPack->pEntries[i].pSvgData);
		xgeSvgDestroy(pPack->pEntries[i].pSvg);
	}
	xrtFree(pPack->pEntries);
	xrtFree(pPack->pNodes);
	xrtFree(pPack);
}

int xgeEmojiPackAddSvgMemory(xge_emoji_pack pPack, const char* sSequence, const void* pSvgData, int iSvgSize, const xge_emoji_metrics_t* pMetrics, uint32_t* pEmojiId)
{
	xge_emoji_metrics_t tMetrics;
	xge_emoji_entry_t* pEntry;
	unsigned char* pSvgCopy;
	char* sSequenceCopy;
	const char* sScan;
	const char* sEnd;
	uint32_t iCodepoint;
	int iCodepointCount;
	int iParent;
	int iNode;
	int iEntry;
	int iRet;

	if ( (pPack == NULL) || (sSequence == NULL) || (*sSequence == 0) ||
	     (pSvgData == NULL) || (iSvgSize <= 0) ) return XGE_ERROR_INVALID_ARGUMENT;
	sEnd = sSequence + strlen(sSequence);
	sScan = sSequence;
	iCodepointCount = 0;
	while ( sScan < sEnd ) {
		if ( __xgeEmojiUTF8Decode(&sScan, sEnd, &iCodepoint) != XGE_OK ) return XGE_ERROR_RESOURCE_FAILED;
		iCodepointCount++;
	}
	tMetrics = __xgeEmojiMetricsDefault();
	if ( pMetrics != NULL ) {
		if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return XGE_ERROR_INVALID_ARGUMENT;
		tMetrics = *pMetrics;
		tMetrics.iSize = sizeof(tMetrics);
		if ( !(tMetrics.fAdvanceEm > 0.0f) ) tMetrics.fAdvanceEm = 1.0f;
		if ( !(tMetrics.fWidthEm > 0.0f) ) tMetrics.fWidthEm = 1.0f;
		if ( !(tMetrics.fHeightEm > 0.0f) ) tMetrics.fHeightEm = 1.0f;
		if ( !(tMetrics.fBaselineRatio > 0.0f) || !(tMetrics.fBaselineRatio < 1.0f) ) tMetrics.fBaselineRatio = 0.82f;
	}
	iRet = __xgeEmojiEnsureEntries(pPack, pPack->iEntryCount + 1);
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xgeEmojiEnsureNodes(pPack, pPack->iNodeCount + iCodepointCount);
	if ( iRet != XGE_OK ) return iRet;
	pSvgCopy = (unsigned char*)xrtMalloc((size_t)iSvgSize);
	sSequenceCopy = (char*)xrtMalloc(strlen(sSequence) + 1u);
	if ( (pSvgCopy == NULL) || (sSequenceCopy == NULL) ) {
		xrtFree(pSvgCopy);
		xrtFree(sSequenceCopy);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pSvgCopy, pSvgData, (size_t)iSvgSize);
	memcpy(sSequenceCopy, sSequence, strlen(sSequence) + 1u);

	iParent = 0;
	sScan = sSequence;
	while ( sScan < sEnd ) {
		(void)__xgeEmojiUTF8Decode(&sScan, sEnd, &iCodepoint);
		iNode = __xgeEmojiFindChild(pPack, iParent, iCodepoint);
		if ( iNode < 0 ) {
			iNode = pPack->iNodeCount++;
			pPack->pNodes[iNode].iCodepoint = iCodepoint;
			pPack->pNodes[iNode].iChild = -1;
			pPack->pNodes[iNode].iSibling = pPack->pNodes[iParent].iChild;
			pPack->pNodes[iNode].iEntry = -1;
			pPack->pNodes[iParent].iChild = iNode;
		}
		iParent = iNode;
	}
	iEntry = pPack->pNodes[iParent].iEntry;
	if ( iEntry >= 0 ) {
		pEntry = &pPack->pEntries[iEntry];
		xrtFree(pEntry->sSequence);
		xrtFree(pEntry->pSvgData);
		xgeSvgDestroy(pEntry->pSvg);
		memset(pEntry, 0, sizeof(*pEntry));
	} else {
		iEntry = pPack->iEntryCount++;
		pPack->pNodes[iParent].iEntry = iEntry;
	}
	pEntry = &pPack->pEntries[iEntry];
	pEntry->sSequence = sSequenceCopy;
	pEntry->iSequenceSize = (int)strlen(sSequence);
	pEntry->pSvgData = pSvgCopy;
	pEntry->iSvgSize = iSvgSize;
	pEntry->tMetrics = tMetrics;
	if ( pEmojiId != NULL ) *pEmojiId = (uint32_t)iEntry + 1u;
	return XGE_OK;
}

int xgeEmojiPackMatch(xge_emoji_pack pPack, const char* sText, int iTextSize, xge_emoji_match_t* pMatch)
{
	const char* sScan;
	const char* sEnd;
	const char* sBestEnd;
	uint32_t iCodepoint;
	int iNode;
	int iBestEntry;

	if ( (pPack == NULL) || (sText == NULL) || (pMatch == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (pMatch->iSize != 0) && (pMatch->iSize < sizeof(*pMatch)) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( iTextSize < 0 ) iTextSize = (int)strlen(sText);
	if ( iTextSize <= 0 ) return XGE_ERROR_NOT_FOUND;
	sScan = sText;
	sEnd = sText + iTextSize;
	sBestEnd = NULL;
	iNode = 0;
	iBestEntry = -1;
	while ( sScan < sEnd ) {
		const char* sBefore = sScan;
		if ( __xgeEmojiUTF8Decode(&sScan, sEnd, &iCodepoint) != XGE_OK ) break;
		iNode = __xgeEmojiFindChild(pPack, iNode, iCodepoint);
		if ( iNode < 0 ) {
			sScan = sBefore;
			break;
		}
		if ( pPack->pNodes[iNode].iEntry >= 0 ) {
			iBestEntry = pPack->pNodes[iNode].iEntry;
			sBestEnd = sScan;
		}
	}
	if ( iBestEntry < 0 ) return XGE_ERROR_NOT_FOUND;
	memset(pMatch, 0, sizeof(*pMatch));
	pMatch->iSize = sizeof(*pMatch);
	pMatch->iEmojiId = (uint32_t)iBestEntry + 1u;
	pMatch->iTextSize = (int)(sBestEnd - sText);
	pMatch->tMetrics = pPack->pEntries[iBestEntry].tMetrics;
	return XGE_OK;
}

int xgeEmojiPackLoadBuiltin(xge_emoji_pack* ppPack)
{
	xge_emoji_metrics_t tMetrics;
	char sSequence[64];
	int iRet;
	int i;

	if ( ppPack == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*ppPack = NULL;
	if ( g_xgeBuiltinEmojiPack == NULL ) {
		iRet = xgeEmojiPackCreate(&g_xgeBuiltinEmojiPack);
		if ( iRet != XGE_OK ) return iRet;
		for ( i = 0; i < (int)(sizeof(g_xgeBuiltinEmojiSources) / sizeof(g_xgeBuiltinEmojiSources[0])); i++ ) {
			if ( g_xgeBuiltinEmojiSources[i].iSequenceSize >= (int)sizeof(sSequence) ) {
				xgeEmojiPackFree(g_xgeBuiltinEmojiPack);
				g_xgeBuiltinEmojiPack = NULL;
				return XGE_ERROR_RESOURCE_FAILED;
			}
			memcpy(sSequence, g_xgeBuiltinEmojiSources[i].pSequence, (size_t)g_xgeBuiltinEmojiSources[i].iSequenceSize);
			sSequence[g_xgeBuiltinEmojiSources[i].iSequenceSize] = 0;
			tMetrics = __xgeEmojiMetricsDefault();
			tMetrics.iFlags = g_xgeBuiltinEmojiSources[i].iFlags;
			iRet = xgeEmojiPackAddSvgMemory(
				g_xgeBuiltinEmojiPack, sSequence,
				g_xgeBuiltinEmojiSources[i].pSvg, g_xgeBuiltinEmojiSources[i].iSvgSize,
				&tMetrics, NULL
			);
			if ( iRet != XGE_OK ) {
				xgeEmojiPackFree(g_xgeBuiltinEmojiPack);
				g_xgeBuiltinEmojiPack = NULL;
				return iRet;
			}
		}
	}
	iRet = xgeEmojiPackAddRef(g_xgeBuiltinEmojiPack);
	if ( iRet <= 0 ) return iRet;
	*ppPack = g_xgeBuiltinEmojiPack;
	return XGE_OK;
}

int xgeEmojiPackSetDefault(xge_emoji_pack pPack)
{
	if ( pPack != NULL ) {
		if ( xgeEmojiPackAddRef(pPack) <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeEmojiPackFree(g_xgeDefaultEmojiPack);
	g_xgeDefaultEmojiPack = pPack;
	return XGE_OK;
}

static xge_emoji_pack __xgeEmojiDefaultGetBorrowed(void)
{
	xge_emoji_pack pPack;

	if ( g_xgeDefaultEmojiPack != NULL ) return g_xgeDefaultEmojiPack;
	pPack = NULL;
	if ( xgeEmojiPackLoadBuiltin(&pPack) != XGE_OK ) return NULL;
	if ( xgeEmojiPackSetDefault(pPack) != XGE_OK ) {
		xgeEmojiPackFree(pPack);
		return NULL;
	}
	xgeEmojiPackFree(pPack);
	return g_xgeDefaultEmojiPack;
}

int xgeEmojiPackGetDefault(xge_emoji_pack* ppPack)
{
	xge_emoji_pack pPack;

	if ( ppPack == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*ppPack = NULL;
	pPack = __xgeEmojiDefaultGetBorrowed();
	if ( pPack == NULL ) return XGE_ERROR_RESOURCE_FAILED;
	if ( xgeEmojiPackAddRef(pPack) <= 0 ) return XGE_ERROR_INVALID_STATE;
	*ppPack = pPack;
	return XGE_OK;
}

void xgeEmojiPackClearDefault(void)
{
	xgeEmojiPackFree(g_xgeDefaultEmojiPack);
	g_xgeDefaultEmojiPack = NULL;
}

static void __xgeEmojiGlobalClear(void)
{
	xgeEmojiPackClearDefault();
	xgeEmojiPackFree(g_xgeBuiltinEmojiPack);
	g_xgeBuiltinEmojiPack = NULL;
}

static int __xgeEmojiMayStart(uint32_t iCodepoint)
{
	return ((iCodepoint >= 0x2300u) && (iCodepoint <= 0x27ffu)) ||
	       ((iCodepoint >= 0x1f000u) && (iCodepoint <= 0x1faffu));
}

static int __xgeEmojiMatchForText(xge_emoji_pack pPack, const char* sText, const char* sEnd, int iPresentation, xge_emoji_match_t* pMatch)
{
	const char* sNext;
	uint32_t iNext;
	int iRet;

	if ( (iPresentation == XGE_EMOJI_PRESENTATION_TEXT) ||
	     (iPresentation == XGE_EMOJI_PRESENTATION_DISABLED) ) return XGE_ERROR_NOT_FOUND;
	if ( pPack == NULL ) pPack = __xgeEmojiDefaultGetBorrowed();
	if ( pPack == NULL ) return XGE_ERROR_NOT_FOUND;
	memset(pMatch, 0, sizeof(*pMatch));
	pMatch->iSize = sizeof(*pMatch);
	iRet = xgeEmojiPackMatch(pPack, sText, (int)(sEnd - sText), pMatch);
	if ( iRet != XGE_OK ) return iRet;

	sNext = sText + pMatch->iTextSize;
	if ( sNext < sEnd ) {
		const char* sAfter = sNext;
		if ( __xgeEmojiUTF8Decode(&sAfter, sEnd, &iNext) == XGE_OK ) {
			if ( iNext == 0xfe0eu ) return XGE_ERROR_NOT_FOUND;
			if ( iNext == 0xfe0fu ) pMatch->iTextSize = (int)(sAfter - sText);
		}
	}
	if ( (iPresentation == XGE_EMOJI_PRESENTATION_AUTO) &&
	     ((pMatch->tMetrics.iFlags & XGE_EMOJI_ENTRY_TEXT_DEFAULT) != 0) ) {
		return XGE_ERROR_NOT_FOUND;
	}
	return XGE_OK;
}

static void __xgeEmojiResolveLayout(xge_font pFont, const xge_emoji_metrics_t* pMetrics, float fEmojiScale, int iLinePolicy, float* pAdvance, float* pWidth, float* pHeight, float* pAbove, float* pBelow)
{
	float fPixelSize;
	float fRatio;
	float fScale;
	float fFit;

	fPixelSize = (pFont->fSize > 0.0f) ? pFont->fSize : pFont->fLineHeight;
	fScale = (fEmojiScale > 0.0f) ? fEmojiScale : 1.0f;
	*pHeight = fPixelSize * fScale * pMetrics->fHeightEm;
	*pWidth = fPixelSize * fScale * pMetrics->fWidthEm;
	*pAdvance = fPixelSize * fScale * pMetrics->fAdvanceEm;
	fRatio = pMetrics->fBaselineRatio;
	*pAbove = *pHeight * fRatio;
	*pBelow = *pHeight - *pAbove;
	if ( iLinePolicy == XGE_EMOJI_LINE_STABLE ) {
		fFit = 1.0f;
		if ( (*pAbove > pFont->fAscent) && (*pAbove > 0.0f) ) fFit = pFont->fAscent / *pAbove;
		if ( (*pBelow > -pFont->fDescent) && (*pBelow > 0.0f) ) {
			float fDescentFit = -pFont->fDescent / *pBelow;
			if ( fDescentFit < fFit ) fFit = fDescentFit;
		}
		if ( fFit < 1.0f ) {
			*pHeight *= fFit;
			*pWidth *= fFit;
			*pAdvance *= fFit;
			*pAbove *= fFit;
			*pBelow *= fFit;
		}
	}
}

static int __xgeEmojiDraw(xge_emoji_pack pPack, uint32_t iEmojiId, xge_rect_t tDst, int bScreenSpace)
{
	xge_emoji_entry_t* pEntry;
	int iRet;

	if ( (pPack == NULL) || (iEmojiId == 0) || (iEmojiId > (uint32_t)pPack->iEntryCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pEntry = &pPack->pEntries[iEmojiId - 1u];
	if ( pEntry->pSvg == NULL ) {
		iRet = xgeSvgCreate(&pEntry->pSvg);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeSvgLoadMemory(pEntry->pSvg, pEntry->pSvgData, pEntry->iSvgSize);
		if ( iRet != XGE_OK ) {
			xgeSvgDestroy(pEntry->pSvg);
			pEntry->pSvg = NULL;
			return iRet;
		}
	}
	return bScreenSpace ? xgeSvgDrawPx(pEntry->pSvg, tDst, 0.35f) : xgeSvgDraw(pEntry->pSvg, tDst, 0.35f);
}
