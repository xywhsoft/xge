#ifndef XGE_NO_TEXT
#include "xge_text_context.h"
typedef struct xge_glyph_run_backend_t {
	int iFontCount;
	int iFontCapacity;
	xge_font* ppFonts;
	xge_emoji_pack pEmojiPack;
#if XGE_ENABLE_HARFBUZZ
	int iGlyphCapacity, iCaretCapacity;
	char* pGraphemeBreaks;
	char* pContextGraphemeBreaks;
	int iContextGraphemeBytes;
	unsigned char* pScriptMap;
	int iScriptMapBytes;
	int bScriptsChecked;
	const char *pFallbackUnitEnd, *pFallbackWordEnd;
	xge_font pFallbackUnitFont; /* Borrowed during this synchronous shape call. */
#endif
} xge_glyph_run_backend_t;

static int __xgeGlyphRunKeepFont(xge_glyph_run_backend_t* pBackend, xge_font pFont)
{
	xge_font* ppFonts;
	int iCapacity;
	int i;

	if ( (pBackend == NULL) || (pFont == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pBackend->iFontCount; i++ ) {
		if ( pBackend->ppFonts[i] == pFont ) return XGE_OK;
	}
	if ( pBackend->iFontCount >= pBackend->iFontCapacity ) {
		iCapacity = (pBackend->iFontCapacity > 0) ? pBackend->iFontCapacity * 2 : 4;
		ppFonts = (xge_font*)xrtRealloc(pBackend->ppFonts, sizeof(*ppFonts) * (size_t)iCapacity);
		if ( ppFonts == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
		pBackend->ppFonts = ppFonts;
		pBackend->iFontCapacity = iCapacity;
	}
	if ( xgeFontAddRef(pFont) <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	pBackend->ppFonts[pBackend->iFontCount++] = pFont;
	return XGE_OK;
}

#if XGE_ENABLE_EMOJI
static int __xgeGlyphRunKeepEmojiPack(xge_glyph_run_backend_t* pBackend, xge_emoji_pack pPack)
{
	if ( (pBackend == NULL) || (pPack == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( pBackend->pEmojiPack == pPack ) return XGE_OK;
	if ( pBackend->pEmojiPack != NULL ) return XGE_ERROR_INVALID_STATE;
	if ( xgeEmojiPackAddRef(pPack) <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	pBackend->pEmojiPack = pPack;
	return XGE_OK;
}

#endif
#if XGE_ENABLE_HARFBUZZ
#include "xge_text_opentype.c"
#endif

static float __xgeGlyphRunLineAdvance(const xge_glyph_run_t* run, int start)
{
	float advance = 0;
	while ( start < run->iGlyphCount && !(run->pGlyphs[start].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) )
		advance += run->pGlyphs[start++].fAdvanceX;
	return advance;
}
static int __xgeGlyphRunVisualPositions(xge_glyph_run_t* run)
{
	int first = 0, rtl = (run->iFlags & XGE_TEXT_SHAPE_RTL) != 0;
	float width = 0;
	if ( !rtl ) {
		float pen = 0; int i;
		for ( i = 0; i < run->iGlyphCount; i++ ) {
			run->pGlyphs[i].fVisualX = pen;
			if ( run->pGlyphs[i].iFlags & XGE_GLYPH_POSITION_LINE_BREAK ) pen = 0;
			else pen += run->pGlyphs[i].fAdvanceX;
		}
		return XGE_OK;
	}
	while ( first < run->iGlyphCount ) {
		float total = __xgeGlyphRunLineAdvance(run, first), logical = 0;
		int i = first;
		if ( !isfinite(total) ) return XGE_ERROR_INVALID_STATE;
		if ( total > width ) width = total;
		while ( i < run->iGlyphCount && !(run->pGlyphs[i].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) ) {
			int finish = i + 1, j; float advance = run->pGlyphs[i].fAdvanceX, pen;
			while ( finish < run->iGlyphCount && !(run->pGlyphs[finish].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) &&
			        run->pGlyphs[finish].iCluster == run->pGlyphs[i].iCluster && run->pGlyphs[finish].iClusterEnd == run->pGlyphs[i].iClusterEnd )
				advance += run->pGlyphs[finish++].fAdvanceX;
			pen = rtl ? total - logical - advance : logical;
			for ( j = i; j < finish; j++ ) {
				xge_glyph_position_t* glyph = &run->pGlyphs[j]; float right;
				xge_glyph_metrics_t metrics;
				glyph->fVisualX = pen;
				if ( glyph->iItemKind == XGE_TEXT_ITEM_EMOJI ) right = pen + glyph->fOffsetX + glyph->fEmojiWidth;
				else {
					int result = xgeFontGlyphGetByIndex(glyph->pFont, glyph->iGlyph, &metrics);
					if ( result != XGE_OK ) return result;
					right = pen + glyph->fOffsetX + metrics.fX1;
				}
				if ( right > width ) width = right;
				pen += glyph->fAdvanceX;
			}
			logical += advance; i = finish;
		}
		first = i < run->iGlyphCount ? i + 1 : i;
	}
	if ( rtl ) run->fWidth = width;
	return XGE_OK;
}

int xgeTextShape(const xge_text_shape_desc_t* pDesc, xge_glyph_run_t* pRun)
{
	xge_text_shape_desc_t normalized;
	xge_glyph_run_backend_t* pBackend;
	xge_glyph_position_t* pPosition;
	xge_glyph_metrics_t tMetrics;
	xge_emoji_match_t tEmojiMatch;
	xge_emoji_pack pEmojiPack;
	xge_font pGlyphFont;
	xge_font pPreviousFont;
	stbtt_fontinfo* pInfo;
	const char* sScan;
	const char* sEnd;
	const char* sBefore;
	uint32_t iCodepoint;
	float fLineWidth;
	float fLineRight;
	float fMaxWidth;
	float fGlyphRight;
	float fKerning;
	float fEmojiAdvance;
	float fEmojiWidth;
	float fEmojiHeight;
	float fEmojiAbove;
	float fEmojiBelow;
	float fMaxLineGap;
	int iTextSize;
	int iGlyph;
	int iPreviousGlyph;
	int iLineCount;
	int iEmojiPresentation;
	int iEmojiLinePolicy;
	int iRet;

	if ( (pDesc == NULL) || (pRun == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->pFont == NULL) || (pDesc->sText == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	memset(pRun, 0, sizeof(*pRun));
#if !XGE_ENABLE_HARFBUZZ
    if ((pDesc->iFlags & XGE_TEXT_SHAPE_RTL) || pDesc->sContext || pDesc->iScript || pDesc->sLanguage)
		return XGE_ERROR_UNSUPPORTED;
#endif
	iTextSize = pDesc->iTextSize;
	if ( iTextSize < -1 ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( iTextSize < 0 ) {
		size_t bytes = strlen(pDesc->sText);
		if ( bytes > INT_MAX ) return XGE_ERROR_INVALID_ARGUMENT;
		iTextSize = (int)bytes;
	}
	iRet = __xgeTextContextNormalize(pDesc, iTextSize, &normalized);
	if (iRet != XGE_OK) return iRet;
	pDesc = &normalized;
	pRun->iSize = sizeof(*pRun);
	pRun->iFlags = pDesc->iFlags;
	pRun->iTextSize = iTextSize;
	pRun->fAscent = pDesc->pFont->fAscent;
	pRun->fDescent = pDesc->pFont->fDescent;
	pRun->fLineHeight = pDesc->pFont->fLineHeight;
	pRun->fHeight = pDesc->pFont->fLineHeight;
	iEmojiPresentation = pDesc->iEmojiPresentation;
	if ( (iEmojiPresentation < XGE_EMOJI_PRESENTATION_AUTO) ||
	     (iEmojiPresentation > XGE_EMOJI_PRESENTATION_DISABLED) ) {
		iEmojiPresentation = XGE_EMOJI_PRESENTATION_AUTO;
	}
	iEmojiLinePolicy = (pDesc->iEmojiLinePolicy == XGE_EMOJI_LINE_EXPAND)
		? XGE_EMOJI_LINE_EXPAND : XGE_EMOJI_LINE_STABLE;
	pEmojiPack = pDesc->pEmojiPack;
	if ( iTextSize == 0 ) return XGE_OK;
	pRun->pGlyphs = (xge_glyph_position_t*)xrtCalloc((size_t)iTextSize, sizeof(*pRun->pGlyphs));
	pBackend = (xge_glyph_run_backend_t*)xrtCalloc(1, sizeof(*pBackend));
	if ( (pRun->pGlyphs == NULL) || (pBackend == NULL) ) {
		xrtFree(pRun->pGlyphs);
		xrtFree(pBackend);
		memset(pRun, 0, sizeof(*pRun));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pRun->pBackend = pBackend;
#if XGE_ENABLE_HARFBUZZ
	pBackend->iGlyphCapacity = iTextSize;
#endif
	sScan = pDesc->sText;
	sEnd = pDesc->sText + iTextSize;
	fLineWidth = 0.0f;
	fLineRight = 0.0f;
	fMaxWidth = 0.0f;
	iPreviousGlyph = -1;
	pPreviousFont = NULL;
	iLineCount = 1;
	fMaxLineGap = pDesc->pFont->fLineGap;
	while ( sScan < sEnd ) {
		sBefore = sScan;
		iRet = __xgeTextUTF8DecodeBounded(&sScan, sEnd, &iCodepoint);
		if ( iRet != XGE_OK ) {
			xgeGlyphRunFree(pRun);
			return iRet;
		}
		if ( iCodepoint == '\r' ) {
			if ( (sScan < sEnd) && (*sScan == '\n') ) continue;
			iCodepoint = '\n';
		}
		pPosition = &pRun->pGlyphs[pRun->iGlyphCount];
		pPosition->iCodepoint = iCodepoint;
		pPosition->iCluster = (uint32_t)(sBefore - pDesc->sText);
		pPosition->iClusterEnd = (uint32_t)(sScan - pDesc->sText);
		pPosition->iItemKind = XGE_TEXT_ITEM_GLYPH;
		if ( iCodepoint == '\n' ) {
			pPosition->iGlyph = -1;
			pPosition->iFlags = XGE_GLYPH_POSITION_LINE_BREAK;
			pPosition->pFont = pDesc->pFont;
			pRun->iGlyphCount++;
			if ( fLineRight > fMaxWidth ) fMaxWidth = fLineRight;
			fLineWidth = 0.0f;
			fLineRight = 0.0f;
			iPreviousGlyph = -1;
			pPreviousFont = NULL;
			iLineCount++;
			continue;
		}
#if XGE_ENABLE_EMOJI
		if ( ((pDesc->iFlags & XGE_TEXT_SHAPE_EMOJI) != 0) &&
		     (iEmojiPresentation != XGE_EMOJI_PRESENTATION_TEXT) &&
		     (iEmojiPresentation != XGE_EMOJI_PRESENTATION_DISABLED) &&
		     ((pEmojiPack != NULL) || __xgeEmojiMayStart(iCodepoint)) ) {
			if ( pEmojiPack == NULL ) {
				pEmojiPack = __xgeEmojiDefaultAcquire();
				if ( pEmojiPack != NULL ) {
					if ( __xgeGlyphRunKeepEmojiPack(pBackend, pEmojiPack) != XGE_OK ) {
						xgeEmojiPackFree(pEmojiPack);
						xgeGlyphRunFree(pRun);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					xgeEmojiPackFree(pEmojiPack);
					pEmojiPack = pBackend->pEmojiPack;
				}
			}
		}
		if ( ((pDesc->iFlags & XGE_TEXT_SHAPE_EMOJI) != 0) && (pEmojiPack != NULL) &&
		     (__xgeEmojiMatchForText(pEmojiPack, sBefore, sEnd, iEmojiPresentation, &tEmojiMatch) == XGE_OK) ) {
			if ( __xgeGlyphRunKeepEmojiPack(pBackend, pEmojiPack) != XGE_OK ) {
				xgeGlyphRunFree(pRun);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			pGlyphFont = __xgeFontResolveCodepoint(pDesc->pFont, iCodepoint, &iGlyph);
			if ( (pGlyphFont != NULL) && (__xgeGlyphRunKeepFont(pBackend, pGlyphFont) != XGE_OK) ) {
				xgeGlyphRunFree(pRun);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			__xgeEmojiResolveLayout(
				pDesc->pFont, &tEmojiMatch.tMetrics, pDesc->fEmojiScale, iEmojiLinePolicy,
				&fEmojiAdvance, &fEmojiWidth, &fEmojiHeight, &fEmojiAbove, &fEmojiBelow
			);
			sScan = sBefore + tEmojiMatch.iTextSize;
			pPosition->iClusterEnd = (uint32_t)(sScan - pDesc->sText);
			pPosition->iItemKind = XGE_TEXT_ITEM_EMOJI;
			pPosition->iGlyph = (pGlyphFont != NULL) ? iGlyph : -1;
			pPosition->pFont = pGlyphFont;
			pPosition->iEmojiId = tEmojiMatch.iEmojiId;
			pPosition->fAdvanceX = fEmojiAdvance;
			pPosition->fEmojiWidth = fEmojiWidth;
			pPosition->fEmojiHeight = fEmojiHeight;
			pPosition->fOffsetX = (fEmojiAdvance - fEmojiWidth) * 0.5f;
			pPosition->fOffsetY = -fEmojiAbove;
			pRun->iGlyphCount++;
			fGlyphRight = fLineWidth + pPosition->fOffsetX + fEmojiWidth;
			if ( fGlyphRight > fLineRight ) fLineRight = fGlyphRight;
			fLineWidth += fEmojiAdvance;
			if ( fLineWidth > fLineRight ) fLineRight = fLineWidth;
			if ( iEmojiLinePolicy == XGE_EMOJI_LINE_EXPAND ) {
				if ( fEmojiAbove > pRun->fAscent ) pRun->fAscent = fEmojiAbove;
				if ( -fEmojiBelow < pRun->fDescent ) pRun->fDescent = -fEmojiBelow;
				pRun->fLineHeight = pRun->fAscent - pRun->fDescent + fMaxLineGap;
			}
			iPreviousGlyph = -1;
			pPreviousFont = NULL;
			continue;
		}
#endif

#if XGE_ENABLE_HARFBUZZ
		iRet = __xgeGlyphRunOpenType(pDesc, pRun, sBefore, sEnd, &sScan, &fLineWidth, &fLineRight, &fMaxLineGap);
		if ( iRet == XGE_OK ) {
			iPreviousGlyph = -1; pPreviousFont = NULL;
			continue;
		}
		if ( iRet != XGE_ERROR_NOT_FOUND ) { xgeGlyphRunFree(pRun); return iRet; }
#endif
		pGlyphFont = __xgeFontResolveCodepoint(pDesc->pFont, iCodepoint, &iGlyph);
		if ( pGlyphFont == NULL ) continue;
		iRet = xgeFontGlyphGetByIndex(pGlyphFont, iGlyph, &tMetrics);
		if ( iRet != XGE_OK ) continue;
		if ( __xgeGlyphRunKeepFont(pBackend, pGlyphFont) != XGE_OK ) {
			xgeGlyphRunFree(pRun);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( pGlyphFont->fAscent > pRun->fAscent ) pRun->fAscent = pGlyphFont->fAscent;
		if ( pGlyphFont->fDescent < pRun->fDescent ) pRun->fDescent = pGlyphFont->fDescent;
		if ( pGlyphFont->fLineGap > fMaxLineGap ) fMaxLineGap = pGlyphFont->fLineGap;
		pRun->fLineHeight = pRun->fAscent - pRun->fDescent + fMaxLineGap;
		if ( ((pDesc->iFlags & XGE_TEXT_SHAPE_KERNING) != 0) && (iPreviousGlyph >= 0) && (pPreviousFont == pGlyphFont) ) {
			pInfo = __xgeFontInfo(pGlyphFont);
			if ( pInfo != NULL ) {
				fKerning = (float)stbtt_GetGlyphKernAdvance(pInfo, iPreviousGlyph, iGlyph) * pGlyphFont->fScale;
				pRun->pGlyphs[pRun->iGlyphCount - 1].fAdvanceX += fKerning;
				fLineWidth += fKerning;
			}
		}
		pPosition->iGlyph = iGlyph;
		pPosition->pFont = pGlyphFont;
		pPosition->fAdvanceX = tMetrics.fAdvanceX;
		pRun->iGlyphCount++;
		fGlyphRight = fLineWidth + tMetrics.fX1;
		if ( fGlyphRight > fLineRight ) fLineRight = fGlyphRight;
		fLineWidth += tMetrics.fAdvanceX;
		if ( fLineWidth > fLineRight ) fLineRight = fLineWidth;
		iPreviousGlyph = iGlyph;
		pPreviousFont = pGlyphFont;
	}
	if ( fLineRight > fMaxWidth ) fMaxWidth = fLineRight;
	pRun->fWidth = fMaxWidth;
	pRun->fHeight = pRun->fLineHeight * (float)iLineCount;
	iRet = __xgeGlyphRunVisualPositions(pRun);
	if ( iRet != XGE_OK ) { xgeGlyphRunFree(pRun); return iRet; }
#if XGE_ENABLE_HARFBUZZ
	/* These maps and borrowed source pointers only serve synchronous shaping.
	 * Caret/hit/paint use the published glyphs and caret stops. Retained rows
	 * must not each keep a full paragraph's temporary script map alive. */
	xrtFree(pBackend->pGraphemeBreaks);pBackend->pGraphemeBreaks=NULL;
	xrtFree(pBackend->pContextGraphemeBreaks);pBackend->pContextGraphemeBreaks=NULL;pBackend->iContextGraphemeBytes=0;
	xrtFree(pBackend->pScriptMap);pBackend->pScriptMap=NULL;pBackend->iScriptMapBytes=0;
	pBackend->pFallbackUnitEnd=pBackend->pFallbackWordEnd=NULL;
	pBackend->pFallbackUnitFont=NULL;
#endif
	return XGE_OK;
}

void xgeGlyphRunFree(xge_glyph_run_t* pRun)
{
	xge_glyph_run_backend_t* pBackend;
	int i;

	if ( pRun == NULL ) return;
	pBackend = (xge_glyph_run_backend_t*)pRun->pBackend;
	if ( pBackend != NULL ) {
		for ( i = 0; i < pBackend->iFontCount; i++ ) xgeFontFree(pBackend->ppFonts[i]);
#if XGE_ENABLE_EMOJI
		xgeEmojiPackFree(pBackend->pEmojiPack);
#endif
#if XGE_ENABLE_HARFBUZZ
		xrtFree(pBackend->pGraphemeBreaks);
		xrtFree(pBackend->pContextGraphemeBreaks);
		xrtFree(pBackend->pScriptMap);
#endif
		xrtFree(pBackend->ppFonts);
		xrtFree(pBackend);
	}
	xrtFree(pRun->pGlyphs);
	xrtFree(pRun->pCarets);
	memset(pRun, 0, sizeof(*pRun));
}

size_t xgeGlyphRunRetainedBytes(const xge_glyph_run_t* run)
{
	xge_glyph_run_backend_t* backend;
	size_t bytes=0;
	if(!run)return 0;
	backend=(xge_glyph_run_backend_t*)run->pBackend;
	if(!backend)return 0;
	bytes=sizeof(*backend)+(size_t)backend->iFontCapacity*sizeof(*backend->ppFonts);
#if XGE_ENABLE_HARFBUZZ
	if(run->pGlyphs)bytes+=(size_t)backend->iGlyphCapacity*sizeof(*run->pGlyphs);
	if(run->pCarets)bytes+=(size_t)backend->iCaretCapacity*sizeof(*run->pCarets);
	if(backend->pGraphemeBreaks)bytes+=(size_t)run->iTextSize;
	if(backend->pContextGraphemeBreaks)bytes+=(size_t)backend->iContextGraphemeBytes;
	if(backend->pScriptMap)bytes+=(size_t)backend->iScriptMapBytes;
#else
	if(run->pGlyphs)bytes+=(size_t)run->iTextSize*sizeof(*run->pGlyphs);
#endif
	return bytes;
}

xge_vec2_t xgeGlyphRunMeasure(const xge_glyph_run_t* pRun)
{
	xge_vec2_t tSize;

	tSize.fX = (pRun != NULL) ? pRun->fWidth : 0.0f;
	tSize.fY = (pRun != NULL) ? pRun->fHeight : 0.0f;
	return tSize;
}

int xgeGlyphRunHitTest(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t* pCluster, int* pTrailing)
{
	const xge_glyph_position_t* pPosition;
	float fPenX;
	int iTargetLine;
	int iLine;
	int i;
	int j;
	float fAdvance;
	double line;

	if ( (pRun == NULL) || (pCluster == NULL) || (pTrailing == NULL) || !isfinite(fX) || !isfinite(fY) ) return XGE_ERROR_INVALID_ARGUMENT;
	line = (pRun->fLineHeight > 0.0f) ? floor((double)fY / pRun->fLineHeight) : 0;
	iTargetLine = line <= 0 ? 0 : line >= INT_MAX ? INT_MAX : (int)line;
	if ( pRun->iFlags & XGE_TEXT_SHAPE_RTL ) {
		float nearest = INFINITY; int found = 0;
		iLine = 0; *pCluster = (uint32_t)pRun->iTextSize; *pTrailing = 0;
		for ( i = 0; i < pRun->iGlyphCount; i = j ) {
			pPosition = &pRun->pGlyphs[i]; j = i + 1;
			if ( pPosition->iFlags & XGE_GLYPH_POSITION_LINE_BREAK ) {
				if ( iLine == iTargetLine ) {
					if ( !found ) *pCluster = pPosition->iCluster;
					return XGE_OK;
				}
				iLine++; continue;
			}
			fAdvance = pPosition->fAdvanceX;
			while ( j < pRun->iGlyphCount && !(pRun->pGlyphs[j].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) &&
			        pRun->pGlyphs[j].iCluster == pPosition->iCluster && pRun->pGlyphs[j].iClusterEnd == pPosition->iClusterEnd )
				fAdvance += pRun->pGlyphs[j++].fAdvanceX;
			if ( iLine == iTargetLine ) {
				float left = pPosition->fVisualX, right = left + fAdvance;
				float distance = fX < left ? left - fX : fX > right ? fX - right : 0;
				if ( distance < nearest ) {
					nearest = distance; found = 1; *pCluster = pPosition->iCluster;
					*pTrailing = fX < left + fAdvance * .5f;
				}
			}
		}
		return XGE_OK;
	}
	fPenX = 0.0f;
	iLine = 0;
	for ( i = 0; i < pRun->iGlyphCount; i++ ) {
		pPosition = &pRun->pGlyphs[i];
		if ( (pPosition->iFlags & XGE_GLYPH_POSITION_LINE_BREAK) != 0 ) {
			if ( iLine == iTargetLine ) {
				*pCluster = pPosition->iCluster;
				*pTrailing = 0;
				return XGE_OK;
			}
			iLine++;
			fPenX = 0.0f;
			continue;
		}
		if ( iLine != iTargetLine ) continue;
		fAdvance = pPosition->fAdvanceX;
		for ( j = i + 1; j < pRun->iGlyphCount &&
		      (pRun->pGlyphs[j].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) == 0 &&
		      pRun->pGlyphs[j].iCluster == pPosition->iCluster &&
		      pRun->pGlyphs[j].iClusterEnd == pPosition->iClusterEnd; j++ ) {
			fAdvance += pRun->pGlyphs[j].fAdvanceX;
		}
		if ( fX <= (fPenX + fAdvance * 0.5f) ) {
			*pCluster = pPosition->iCluster;
			*pTrailing = 0;
			return XGE_OK;
		}
		if ( fX <= (fPenX + fAdvance) ) {
			*pCluster = pPosition->iCluster;
			*pTrailing = 1;
			return XGE_OK;
		}
		fPenX += fAdvance;
		i = j - 1;
	}
	*pCluster = (uint32_t)pRun->iTextSize;
	*pTrailing = 0;
	return XGE_OK;
}

static void __xgeGlyphRunPrepareAtlas(const xge_glyph_run_t* pRun)
{
	xge_glyph_run_backend_t* pBackend;
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_t tGlyph;
	int i;
	int j;

	for ( i = 0; i < pRun->iGlyphCount; i++ ) {
		if ( ((pRun->pGlyphs[i].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) == 0) &&
		     (pRun->pGlyphs[i].iItemKind == XGE_TEXT_ITEM_GLYPH) ) {
			(void)xgeFontGlyphAtlasGetByIndex(pRun->pGlyphs[i].pFont, pRun->pGlyphs[i].iGlyph, &tGlyph);
		}
	}
	pBackend = (xge_glyph_run_backend_t*)pRun->pBackend;
	if ( pBackend == NULL ) return;
	for ( i = 0; i < pBackend->iFontCount; i++ ) {
		pPages = (xge_glyph_atlas_page_t*)pBackend->ppFonts[i]->tAtlas.pPages;
		for ( j = 0; (pPages != NULL) && (j < pBackend->ppFonts[i]->tAtlas.iPageCount); j++ ) {
			if ( pPages[j].bDirty || (pPages[j].tTexture.iBackendId == 0) ) (void)__xgeFontAtlasUploadPage(pBackend->ppFonts[i], j);
		}
	}
}

static uint32_t __xgeGlyphRunSpanColor(const xge_glyph_position_t* pPosition,
	uint32_t iColor, const xge_text_paint_span_t* pSpans, int iSpanCount)
{
	uint32_t iClusterEnd;
	int i;

	if ( pPosition == NULL || pSpans == NULL || iSpanCount <= 0 ) return iColor;
	iClusterEnd = pPosition->iClusterEnd;
	if ( iClusterEnd <= pPosition->iCluster ) iClusterEnd = pPosition->iCluster + 1u;
	for ( i = 0; i < iSpanCount; i++ ) {
		const xge_text_paint_span_t* pSpan = &pSpans[i];
		int iStart = pSpan->iStart;
		int iEnd = pSpan->iEnd;
		if ( iEnd <= 0 || iEnd <= iStart ) continue;
		if ( iStart < 0 ) iStart = 0;
		if ( (uint32_t)iEnd <= pPosition->iCluster || (uint32_t)iStart >= iClusterEnd ) continue;
		iColor = pSpan->iColor;
	}
	return iColor;
}

/* Crop destination and UV together: adjacent pieces sample the same atlas glyph
 * and obey the caller's existing world transform and clip without changing either. */
static void __xgeGlyphRunDrawPaint(const xge_glyph_run_t* pRun,
	const xge_glyph_position_t* pPosition, const xge_draw_t* pDraw, float fClusterX, float fClusterAdvance,
	uint32_t iColor, const xge_text_paint_span_t* pSpans, int iSpanCount)
{
	int iLow = 0, iHigh = pRun->iCaretCount, i;
	float fLeft = pDraw->tDst.fX;
	float fRight = fLeft + pDraw->tDst.fW;
	uint32_t iStart = pPosition->iCluster;
	int bRtl = (pRun->iFlags & XGE_TEXT_SHAPE_RTL) != 0;
	float fCursor = bRtl ? fRight : fLeft;
	if ( pSpans == NULL || iSpanCount <= 0 || pRun->pCarets == NULL || iHigh <= 0 ) {
		xgeDrawEx(pDraw);
		return;
	}
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		if ( pRun->pCarets[iMid].iTextOffset <= iStart ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	if ( iLow == pRun->iCaretCount || pRun->pCarets[iLow].iTextOffset >= pPosition->iClusterEnd ) {
		xgeDrawEx(pDraw);
		return;
	}
	for ( i = iLow; ; i++ ) {
		xge_glyph_position_t tPart = *pPosition;
		xge_draw_t tPartDraw = *pDraw;
		int bLast = i == pRun->iCaretCount || pRun->pCarets[i].iTextOffset >= pPosition->iClusterEnd;
		float fEnd = bLast ? (bRtl ? fLeft : fRight) : fClusterX +
			(bRtl ? fClusterAdvance - pRun->pCarets[i].fAdvance : pRun->pCarets[i].fAdvance);
		if ( fEnd > fRight ) fEnd = fRight;
		if ( fEnd < pDraw->tDst.fX ) fEnd = pDraw->tDst.fX;
		tPart.iCluster = iStart;
		tPart.iClusterEnd = bLast ? pPosition->iClusterEnd : pRun->pCarets[i].iTextOffset;
		if ( (bRtl && fEnd < fCursor) || (!bRtl && fEnd > fCursor) ) {
			float fRatio = pDraw->tSrc.fW / pDraw->tDst.fW;
			tPartDraw.tDst.fX = bRtl ? fEnd : fCursor;
			tPartDraw.tDst.fW = fabsf(fEnd - fCursor);
			tPartDraw.tSrc.fX += (tPartDraw.tDst.fX - pDraw->tDst.fX) * fRatio;
			tPartDraw.tSrc.fW = tPartDraw.tDst.fW * fRatio;
			tPartDraw.iColor = __xgeGlyphRunSpanColor(&tPart, iColor, pSpans, iSpanCount);
			xgeDrawEx(&tPartDraw);
		}
		if ( bLast ) break;
		fCursor = fEnd;
		iStart = tPart.iClusterEnd;
	}
}

static void __xgeGlyphRunDrawSpans(const xge_glyph_run_t* pRun, float fX, float fY,
	uint32_t iColor, uint32_t iFlags, const xge_text_paint_span_t* pSpans, int iSpanCount)
{
	const xge_glyph_position_t* pPosition;
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_t tGlyph;
	xge_draw_t tDraw;
	float fPenX;
	float fPenY;
	float fClusterX = 0.0f;
	float fClusterAdvance = 0.0f, fDrawX, fOriginX;
	xge_glyph_run_backend_t* pBackend;
	int i;
	int keep_x = (iFlags & XGE_DRAW_TEXT_SUBPIXEL_X) != 0;
	iFlags &= ~XGE_DRAW_TEXT_SUBPIXEL_X; /* Never forward text-only flags to quads. */

	if ( (pRun == NULL) || (pRun->pGlyphs == NULL) ) return;
	pBackend = (xge_glyph_run_backend_t*)pRun->pBackend;
	__xgeGlyphRunPrepareAtlas(pRun);
	fPenX = fX;
	fPenY = fY + pRun->fAscent;
	if ( (iFlags & XGE_DRAW_SCREEN_SPACE) != 0 ) {
		if (!keep_x) fPenX = __xgeTextSnapPixel(fPenX);
		fPenY = __xgeTextSnapPixel(fPenY);
	}
	fOriginX = fPenX;
	for ( i = 0; i < pRun->iGlyphCount; i++ ) {
		pPosition = &pRun->pGlyphs[i];
		if ( (pPosition->iFlags & XGE_GLYPH_POSITION_LINE_BREAK) != 0 ) {
			fPenX = fX;
			fPenY += pRun->fLineHeight;
			continue;
		}
		if ( i == 0 || (pRun->pGlyphs[i - 1].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) != 0 ||
		     pRun->pGlyphs[i - 1].iCluster != pPosition->iCluster ||
		     pRun->pGlyphs[i - 1].iClusterEnd != pPosition->iClusterEnd ) {
			int j;
			fClusterX = (pRun->iFlags & XGE_TEXT_SHAPE_RTL) ? fOriginX + pPosition->fVisualX : fPenX;
			fClusterAdvance = pPosition->fAdvanceX;
			for ( j = i + 1; j < pRun->iGlyphCount && !(pRun->pGlyphs[j].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) &&
			      pRun->pGlyphs[j].iCluster == pPosition->iCluster && pRun->pGlyphs[j].iClusterEnd == pPosition->iClusterEnd; j++ )
				fClusterAdvance += pRun->pGlyphs[j].fAdvanceX;
		}
		fDrawX = (pRun->iFlags & XGE_TEXT_SHAPE_RTL) ? fOriginX + pPosition->fVisualX : fPenX;
#if XGE_ENABLE_EMOJI
		if ( pPosition->iItemKind == XGE_TEXT_ITEM_EMOJI ) {
			xge_rect_t tEmojiRect;
			int iEmojiRet = XGE_ERROR_RESOURCE_FAILED;
			tEmojiRect.fX = fDrawX + pPosition->fOffsetX;
			tEmojiRect.fY = fPenY + pPosition->fOffsetY;
			tEmojiRect.fW = pPosition->fEmojiWidth;
			tEmojiRect.fH = pPosition->fEmojiHeight;
			if ( (pBackend != NULL) && (pBackend->pEmojiPack != NULL) ) {
				iEmojiRet = __xgeEmojiDraw(
					pBackend->pEmojiPack, pPosition->iEmojiId, tEmojiRect,
					((iFlags & XGE_DRAW_SCREEN_SPACE) != 0)
				);
			}
			if ( iEmojiRet == XGE_OK ) {
				fPenX += pPosition->fAdvanceX;
				continue;
			}
		}
#endif
		if ( xgeFontGlyphAtlasGetByIndex(pPosition->pFont, pPosition->iGlyph, &tGlyph) == XGE_OK &&
		     (tGlyph.iPage >= 0) && (tGlyph.iWidth > 0) && (tGlyph.iHeight > 0) ) {
			pPages = (xge_glyph_atlas_page_t*)pPosition->pFont->tAtlas.pPages;
			if ( (pPosition->iItemKind == XGE_TEXT_ITEM_EMOJI) &&
			     (pPages[tGlyph.iPage].bDirty || (pPages[tGlyph.iPage].tTexture.iBackendId == 0)) ) {
				(void)__xgeFontAtlasUploadPage(pPosition->pFont, tGlyph.iPage);
			}
			memset(&tDraw, 0, sizeof(tDraw));
			tDraw.pTexture = &pPages[tGlyph.iPage].tTexture;
			tDraw.tSrc.fX = (float)tGlyph.iX;
			tDraw.tSrc.fY = (float)tGlyph.iY;
			tDraw.tSrc.fW = (float)tGlyph.iWidth;
			tDraw.tSrc.fH = (float)tGlyph.iHeight;
			tDraw.tDst.fX = fDrawX + tGlyph.fOffsetX +
				((pPosition->iItemKind == XGE_TEXT_ITEM_GLYPH) ? pPosition->fOffsetX : 0.0f);
			tDraw.tDst.fY = fPenY + tGlyph.fOffsetY +
				((pPosition->iItemKind == XGE_TEXT_ITEM_GLYPH) ? pPosition->fOffsetY : 0.0f);
			tDraw.tDst.fW = (float)tGlyph.iWidth;
			tDraw.tDst.fH = (float)tGlyph.iHeight;
			tDraw.iColor = __xgeGlyphRunSpanColor(pPosition, iColor, pSpans, iSpanCount);
			tDraw.iFlags = iFlags;
			__xgeGlyphRunDrawPaint(pRun, pPosition, &tDraw, fClusterX, fClusterAdvance, iColor, pSpans, iSpanCount);
		}
		fPenX += pPosition->fAdvanceX;
	}
}

void xgeGlyphRunDraw(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags)
{
	__xgeGlyphRunDrawSpans(pRun, fX, fY, iColor, iFlags, NULL, 0);
}

void xgeGlyphRunDrawSpans(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor,
	uint32_t iFlags, const xge_text_paint_span_t* pSpans, int iSpanCount)
{
	__xgeGlyphRunDrawSpans(pRun, fX, fY, iColor, iFlags, pSpans, iSpanCount);
}

static void __xgeGlyphRunDecorationLine(float fX0, float fX1, float fY, const xge_text_decoration_t* pDecoration, float fDefaultThickness)
{
	float fThickness;
	float fWavelength;
	float fAmplitude;
	float fX;
	float fNext;
	float fDirection;
	uint32_t iColor;
	int bScreenSpace;

	if ( (pDecoration == NULL) || (fX1 <= fX0) ) return;
	iColor = pDecoration->iColor;
	fThickness = (pDecoration->fThickness > 0.0f) ? pDecoration->fThickness : fDefaultThickness;
	bScreenSpace = ((pDecoration->iFlags & XGE_TEXT_DECORATION_SCREEN_SPACE) != 0);
	if ( pDecoration->iType == XGE_TEXT_DECORATION_SQUIGGLE ) {
		fWavelength = (pDecoration->fWavelength > 1.0f) ? pDecoration->fWavelength : 4.0f;
		fAmplitude = (pDecoration->fAmplitude > 0.0f) ? pDecoration->fAmplitude : 1.5f;
		fX = fX0 - fmodf(fabsf(pDecoration->fPhase), fWavelength);
		fDirection = -1.0f;
		while ( fX < fX1 ) {
			fNext = fX + fWavelength * 0.5f;
			if ( fNext > fX1 ) fNext = fX1;
			if ( bScreenSpace ) xgeShapeLinePx(fX, fY + fDirection * fAmplitude, fNext, fY - fDirection * fAmplitude, fThickness, iColor);
			else xgeShapeLine(fX, fY + fDirection * fAmplitude, fNext, fY - fDirection * fAmplitude, fThickness, iColor);
			fDirection = -fDirection;
			fX = fNext;
		}
		return;
	}
	if ( (pDecoration->iType == XGE_TEXT_DECORATION_DOTTED) || (pDecoration->iType == XGE_TEXT_DECORATION_DASHED) ) {
		fWavelength = (pDecoration->fWavelength > 1.0f) ? pDecoration->fWavelength : ((pDecoration->iType == XGE_TEXT_DECORATION_DOTTED) ? 3.0f : 7.0f);
		fX = fX0 - fmodf(fabsf(pDecoration->fPhase), fWavelength);
		while ( fX < fX1 ) {
			fNext = fX + ((pDecoration->iType == XGE_TEXT_DECORATION_DOTTED) ? fThickness : fWavelength * 0.65f);
			if ( fNext > fX1 ) fNext = fX1;
			if ( bScreenSpace ) xgeShapeLinePx(fX, fY, fNext, fY, fThickness, iColor);
			else xgeShapeLine(fX, fY, fNext, fY, fThickness, iColor);
			fX += fWavelength;
		}
		return;
	}
	if ( bScreenSpace ) xgeShapeLinePx(fX0, fY, fX1, fY, fThickness, iColor);
	else xgeShapeLine(fX0, fY, fX1, fY, fThickness, iColor);
}

static float __xgeGlyphRunRangeAdvance(const xge_glyph_run_t* pRun,
	uint32_t iStart, uint32_t iEnd, uint32_t iByte, float fAdvance, int bRoundUp)
{
	int iLow = 0, iHigh = pRun->iCaretCount;
	if ( iByte <= iStart ) return 0.0f;
	if ( iByte >= iEnd ) return fAdvance;
	if ( pRun->pCarets == NULL ) return bRoundUp ? fAdvance : 0.0f;
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		if ( pRun->pCarets[iMid].iTextOffset < iByte ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	if ( iLow < pRun->iCaretCount && pRun->pCarets[iLow].iTextOffset == iByte ) return pRun->pCarets[iLow].fAdvance;
	if ( bRoundUp ) return iLow < pRun->iCaretCount && pRun->pCarets[iLow].iTextOffset < iEnd
		? pRun->pCarets[iLow].fAdvance : fAdvance;
	return iLow > 0 && pRun->pCarets[iLow - 1].iTextOffset > iStart
		? pRun->pCarets[iLow - 1].fAdvance : 0.0f;
}

void xgeGlyphRunDrawDecorated(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags, const xge_text_decoration_t* pDecorations, int iDecorationCount)
{
	xge_font_metrics_t tMetrics;
	const xge_text_decoration_t* pDecoration;
	const xge_glyph_position_t* pPosition;
	float fPenX;
	float fSegmentStart;
	float fSegmentEnd;
	float fLineTop;
	float fBaseline;
	float fOffset;
	float fAdvance;
	float fLineAdvance;
	uint32_t iGlyphEnd;
	int iRangeStart;
	int iRangeEnd;
	int bSegment;
	int i;
	int j;
	int iGroupEnd;

	if ( pRun == NULL ) return;
	xgeGlyphRunDraw(pRun, fX, fY, iColor, iFlags);
	if ( (pDecorations == NULL) || (iDecorationCount <= 0) || (pRun->iGlyphCount <= 0) ) return;
	for ( j = 0; j < iDecorationCount; j++ ) {
		pDecoration = &pDecorations[j];
		if ( (pDecoration->iSize != 0) && (pDecoration->iSize < sizeof(*pDecoration)) ) continue;
		iRangeStart = 0;
		iRangeEnd = pRun->iTextSize;
		if ( (pDecoration->iFlags & XGE_TEXT_DECORATION_RANGE) != 0 ) {
			iRangeStart = pDecoration->iStart;
			iRangeEnd = pDecoration->iEnd;
			if ( iRangeStart < 0 ) iRangeStart = 0;
			if ( iRangeEnd > pRun->iTextSize ) iRangeEnd = pRun->iTextSize;
			if ( iRangeEnd <= iRangeStart ) continue;
		}
		fPenX = fX;
		fLineTop = fY;
		fLineAdvance = __xgeGlyphRunLineAdvance(pRun, 0);
		fSegmentStart = fX;
		fSegmentEnd = fX;
		bSegment = 0;
		memset(&tMetrics, 0, sizeof(tMetrics));
		for ( i = 0; i <= pRun->iGlyphCount; i++ ) {
			if ( i == pRun->iGlyphCount ||
			     ((pRun->pGlyphs[i].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) != 0) ) {
				if ( bSegment ) {
					fBaseline = fLineTop + pRun->fAscent;
					if ( pDecoration->iType == XGE_TEXT_DECORATION_OVERLINE ) fOffset = -pRun->fAscent;
					else if ( pDecoration->iType == XGE_TEXT_DECORATION_STRIKE ) fOffset = tMetrics.fStrikePosition;
					else fOffset = tMetrics.fUnderlinePosition;
					if ( pDecoration->fOffset != 0.0f ) fOffset = pDecoration->fOffset;
					__xgeGlyphRunDecorationLine(
						(pRun->iFlags & XGE_TEXT_SHAPE_RTL) ? fX + fLineAdvance - (fSegmentEnd - fX) : fSegmentStart,
						(pRun->iFlags & XGE_TEXT_SHAPE_RTL) ? fX + fLineAdvance - (fSegmentStart - fX) : fSegmentEnd,
						fBaseline + fOffset, pDecoration,
						(pDecoration->iType == XGE_TEXT_DECORATION_STRIKE)
							? tMetrics.fStrikeThickness : tMetrics.fUnderlineThickness
					);
					bSegment = 0;
				}
				if ( i < pRun->iGlyphCount ) {
					fPenX = fX;
					fLineTop += pRun->fLineHeight;
					fLineAdvance = __xgeGlyphRunLineAdvance(pRun, i + 1);
				}
				continue;
			}
			pPosition = &pRun->pGlyphs[i];
			fAdvance = pPosition->fAdvanceX;
			for ( iGroupEnd = i + 1; iGroupEnd < pRun->iGlyphCount &&
			      (pRun->pGlyphs[iGroupEnd].iFlags & XGE_GLYPH_POSITION_LINE_BREAK) == 0 &&
			      pRun->pGlyphs[iGroupEnd].iCluster == pPosition->iCluster &&
			      pRun->pGlyphs[iGroupEnd].iClusterEnd == pPosition->iClusterEnd; iGroupEnd++ ) {
				fAdvance += pRun->pGlyphs[iGroupEnd].fAdvanceX;
			}
			iGlyphEnd = (pPosition->iClusterEnd > pPosition->iCluster)
				? pPosition->iClusterEnd : ((i + 1 < pRun->iGlyphCount)
					? pRun->pGlyphs[i + 1].iCluster : (uint32_t)pRun->iTextSize);
			if ( ((int)iGlyphEnd > iRangeStart) && ((int)pPosition->iCluster < iRangeEnd) ) {
				if ( !bSegment ) {
					fSegmentStart = fPenX + __xgeGlyphRunRangeAdvance(pRun,
						pPosition->iCluster, iGlyphEnd, (uint32_t)iRangeStart, fAdvance, 0);
					memset(&tMetrics, 0, sizeof(tMetrics));
					if ( xgeFontGetMetrics(pPosition->pFont, &tMetrics) != XGE_OK ) {
						tMetrics.fUnderlinePosition = 1.0f;
						tMetrics.fUnderlineThickness = 1.0f;
						tMetrics.fStrikePosition = -pRun->fAscent * 0.35f;
						tMetrics.fStrikeThickness = 1.0f;
					}
					bSegment = 1;
				}
				fSegmentEnd = fPenX + __xgeGlyphRunRangeAdvance(pRun,
					pPosition->iCluster, iGlyphEnd, (uint32_t)iRangeEnd, fAdvance, 1);
			}
			fPenX += fAdvance;
			i = iGroupEnd - 1;
		}
	}
}
#else

int xgeTextShape(const xge_text_shape_desc_t* pDesc, xge_glyph_run_t* pRun) { (void)pDesc; (void)pRun; return XGE_ERROR_UNSUPPORTED; }
void xgeGlyphRunFree(xge_glyph_run_t* pRun) { if ( pRun != NULL ) memset(pRun, 0, sizeof(*pRun)); }
size_t xgeGlyphRunRetainedBytes(const xge_glyph_run_t* pRun) { (void)pRun; return 0; }
xge_vec2_t xgeGlyphRunMeasure(const xge_glyph_run_t* pRun) { xge_vec2_t t = {0.0f, 0.0f}; (void)pRun; return t; }
int xgeGlyphRunHitTest(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t* pCluster, int* pTrailing) { (void)pRun; (void)fX; (void)fY; (void)pCluster; (void)pTrailing; return XGE_ERROR_UNSUPPORTED; }
void xgeGlyphRunDraw(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags) { (void)pRun; (void)fX; (void)fY; (void)iColor; (void)iFlags; }
void xgeGlyphRunDrawSpans(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags, const xge_text_paint_span_t* pSpans, int iSpanCount) { (void)pRun; (void)fX; (void)fY; (void)iColor; (void)iFlags; (void)pSpans; (void)iSpanCount; }
void xgeGlyphRunDrawDecorated(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags, const xge_text_decoration_t* pDecorations, int iDecorationCount) { (void)pRun; (void)fX; (void)fY; (void)iColor; (void)iFlags; (void)pDecorations; (void)iDecorationCount; }

#endif
