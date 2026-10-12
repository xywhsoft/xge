/*
 * af_onion.c - Onion skin rendering
 *
 * Renders previous/next frames as semi-transparent overlays
 * for animation reference. Previous frames tinted red,
 * next frames tinted blue (or custom colors).
 */

#include "af_onion.h"
#include "af_draw_helper.h"
#include "../core/af_app.h"
#include "../anim/af_timeline.h"
#include "af_stage.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* State                                                              */
/* ------------------------------------------------------------------ */

static af_onion_config_t s_tConfig;

/* ------------------------------------------------------------------ */
/* Init                                                               */
/* ------------------------------------------------------------------ */

void afOnionInit(af_onion_config_t* pConfig)
{
	memset(pConfig, 0, sizeof(*pConfig));
	pConfig->bEnabled = 0;
	pConfig->iPrevFrames = 3;
	pConfig->iNextFrames = 3;
	pConfig->fPrevAlpha = 0.3f;
	pConfig->fNextAlpha = 0.3f;
	pConfig->iPrevColor = XUI_COLOR_RGBA(255, 80, 80, 255);   /* red tint */
	pConfig->iNextColor = XUI_COLOR_RGBA(80, 80, 255, 255);   /* blue tint */
	pConfig->bColorMode = 1;

	/* Also update global state */
	s_tConfig = *pConfig;
}

void afOnionSetEnabled(af_onion_config_t* pConfig, int bEnabled)
{
	pConfig->bEnabled = bEnabled;
	s_tConfig.bEnabled = bEnabled;
}

void afOnionSetRange(af_onion_config_t* pConfig, int iPrev, int iNext)
{
	if ( iPrev < 0 ) iPrev = 0;
	if ( iNext < 0 ) iNext = 0;
	if ( iPrev > AF_ONION_MAX_FRAMES ) iPrev = AF_ONION_MAX_FRAMES;
	if ( iNext > AF_ONION_MAX_FRAMES ) iNext = AF_ONION_MAX_FRAMES;

	pConfig->iPrevFrames = iPrev;
	pConfig->iNextFrames = iNext;
	s_tConfig = *pConfig;
}

void afOnionSetAlpha(af_onion_config_t* pConfig, float fPrev, float fNext)
{
	if ( fPrev < 0.0f ) fPrev = 0.0f;
	if ( fPrev > 1.0f ) fPrev = 1.0f;
	if ( fNext < 0.0f ) fNext = 0.0f;
	if ( fNext > 1.0f ) fNext = 1.0f;

	pConfig->fPrevAlpha = fPrev;
	pConfig->fNextAlpha = fNext;
	s_tConfig = *pConfig;
}

void afOnionSetColors(af_onion_config_t* pConfig, uint32_t iPrev, uint32_t iNext)
{
	pConfig->iPrevColor = iPrev;
	pConfig->iNextColor = iNext;
	s_tConfig = *pConfig;
}

/* ------------------------------------------------------------------ */
/* Rendering                                                          */
/* ------------------------------------------------------------------ */

static void __afOnionRenderFrame(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                                 af_doc pDoc, uint32_t iFrame, uint32_t iTintColor, float fAlpha)
{
	int li;

	for ( li = 0; li < pDoc->tTimeline.iLayerCount; li++ ) {
		af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[li];
		af_keyframe_t* pKf;
		int ei;

		/* Skip hidden layers */
		if ( !(pLayer->iFlags & AF_LAYER_FLAG_VISIBLE) )
			continue;

		/* Get owning keyframe for this frame */
		pKf = afTimelineGetOwningKeyframe(pLayer, iFrame);
		if ( pKf == NULL )
			continue;
		if ( pKf->iFlags & AF_KF_BLANK )
			continue;

		/* Render each element with tint */
		for ( ei = 0; ei < pKf->iElementCount; ei++ ) {
			af_element_t* pElem = &pKf->arrElements[ei];
			af_symbol_t* pSym;
			float sx, sy;

			if ( !(pElem->iFlags & AF_ELEM_FLAG_VISIBLE) )
				continue;

			pSym = afDocFindSymbol(pDoc, pElem->iSymbolId);
			if ( pSym == NULL )
				continue;

			afStageWorldToScreen(pView, pElem->fTx, pElem->fTy, &sx, &sy);

			/* Render as tinted outline/rectangle */
			if ( pSym->iType == AF_SYMBOL_GRAPHIC ) {
				float w = pSym->fBoundsW * pElem->fScaleX * pView->fZoom;
				float h = pSym->fBoundsH * pElem->fScaleY * pView->fZoom;
				int tintR = (iTintColor >> 24) & 0xFF;
				int tintG = (iTintColor >> 16) & 0xFF;
				int tintB = (iTintColor >> 8) & 0xFF;
				int alpha = (int)(fAlpha * pElem->fOpacity * 255.0f);
				uint32_t col = ((uint32_t)tintR << 24) | ((uint32_t)tintG << 16) |
				               ((uint32_t)tintB << 8) | (uint32_t)alpha;

				afDrawStrokeRect(pProxy, pCtx, sx, sy, w, h, 1.5f, col);
			} else {
				float w = pSym->fBoundsW * pElem->fScaleX * pView->fZoom;
				float h = pSym->fBoundsH * pElem->fScaleY * pView->fZoom;
				int alpha = (int)(fAlpha * pElem->fOpacity * 128.0f);
				uint32_t col = (iTintColor & 0xFFFFFF00) | (uint32_t)alpha;

				afDrawFillRect(pProxy, pCtx, sx, sy, w, h, col);
			}
		}
	}
}

void afOnionRender(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                   af_doc pDoc, uint32_t iCurrentFrame, const af_onion_config_t* pConfig)
{
	int i;
	uint32_t totalFrames = pDoc->iFrameCount;

	if ( !pConfig->bEnabled )
		return;

	/* Render previous frames (red tint, fading out) */
	for ( i = 1; i <= pConfig->iPrevFrames; i++ ) {
		int frame = (int)iCurrentFrame - i;
		float alpha;

		if ( frame < 0 ) {
			if ( pDoc->iFlags & 0x0001 ) /* loop flag */
				frame += (int)totalFrames;
			else
				break;
		}

		/* Fade: closer frames are more opaque */
		alpha = pConfig->fPrevAlpha * (1.0f - (float)(i - 1) / (float)pConfig->iPrevFrames);
		if ( alpha < 0.02f )
			break;

		__afOnionRenderFrame(pProxy, pCtx, pView, pDoc, (uint32_t)frame,
		                     pConfig->iPrevColor, alpha);
	}

	/* Render next frames (blue tint, fading out) */
	for ( i = 1; i <= pConfig->iNextFrames; i++ ) {
		uint32_t frame = iCurrentFrame + (uint32_t)i;
		float alpha;

		if ( frame >= totalFrames ) {
			if ( pDoc->iFlags & 0x0001 )
				frame %= totalFrames;
			else
				break;
		}

		alpha = pConfig->fNextAlpha * (1.0f - (float)(i - 1) / (float)pConfig->iNextFrames);
		if ( alpha < 0.02f )
			break;

		__afOnionRenderFrame(pProxy, pCtx, pView, pDoc, frame,
		                     pConfig->iNextColor, alpha);
	}
}

/* ------------------------------------------------------------------ */
/* Toggle helpers                                                     */
/* ------------------------------------------------------------------ */

void afOnionToggle(af_onion_config_t* pConfig)
{
	pConfig->bEnabled = !pConfig->bEnabled;
	s_tConfig.bEnabled = pConfig->bEnabled;
}

int afOnionIsEnabled(const af_onion_config_t* pConfig)
{
	return pConfig->bEnabled;
}
