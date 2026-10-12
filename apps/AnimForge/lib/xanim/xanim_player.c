/*
 * xanim_player.c - XANIM runtime player implementation
 *
 * Standalone playback library for .xanim files.
 * Depends on XGE (ShapeEx) for rendering.
 * This is external plugin code - not part of xge internals.
 */

#include "xanim.h"
#include "xge.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Internal structures                                                */
/* ------------------------------------------------------------------ */

#define XANIM_MAX_LAYERS     64
#define XANIM_MAX_SYMBOLS    1024
#define XANIM_MAX_CURVES     4096
#define XANIM_MAX_STRINGS    4096

typedef struct xanim_rt_layer_t {
	uint32_t iId;
	uint32_t iType;
	uint32_t iFlags;
	int      iKeyframeCount;
	xanim_keyframe_t* pKeyframes;   /* owned array */
	xanim_element_t*  pElements;    /* owned flat array for all keyframes */
} xanim_rt_layer_t;

typedef struct xanim_rt_curve_t {
	uint32_t iId;
	uint32_t iElementId;
	uint32_t iChannelMask;
	int      iKeyCount;
	xanim_curve_key_t* pKeys;       /* owned array */
} xanim_rt_curve_t;

struct xanim_player_t {
	/* File data (owned) */
	uint8_t* pData;
	int      iDataSize;

	/* Parsed header */
	xanim_header_t tHeader;

	/* String table */
	int      iStringCount;
	char**   ppStrings;             /* owned array of owned strings */

	/* Symbols (parsed from section) */
	int      iSymbolCount;
	xanim_symbol_t* pSymbols;       /* points into pData */

	/* Layers */
	int      iLayerCount;
	xanim_rt_layer_t arrLayers[XANIM_MAX_LAYERS];

	/* Curves */
	int      iCurveCount;
	xanim_rt_curve_t arrCurves[XANIM_MAX_CURVES];

	/* Playback state */
	int      bPlaying;
	int      bLoop;
	float    fSpeed;
	uint32_t iCurrentFrame;
	double   fAccum;

	/* Event callback */
	xanim_event_proc pfnEvent;
	void*    pEventUser;
};

/* ------------------------------------------------------------------ */
/* CRC32                                                              */
/* ------------------------------------------------------------------ */

uint32_t xanimCrc32(const void* pData, uint32_t iSize)
{
	static uint32_t sTable[256];
	static int sInit = 0;
	const uint8_t* p = (const uint8_t*)pData;
	uint32_t crc = 0xFFFFFFFFu;
	uint32_t i;

	if ( !sInit ) {
		uint32_t j, k;
		for ( j = 0; j < 256; j++ ) {
			uint32_t c = j;
			for ( k = 0; k < 8; k++ ) {
				c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
			}
			sTable[j] = c;
		}
		sInit = 1;
	}

	for ( i = 0; i < iSize; i++ ) {
		crc = sTable[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
	}
	return crc ^ 0xFFFFFFFFu;
}

/* ------------------------------------------------------------------ */
/* Easing (minimal runtime version)                                   */
/* ------------------------------------------------------------------ */

static float __xanimEase(float fT, uint32_t iType, const float* pBezier)
{
	if ( fT <= 0.0f ) return 0.0f;
	if ( fT >= 1.0f ) return 1.0f;
	switch ( iType ) {
	case XANIM_INTERP_HOLD:   return 0.0f;
	case XANIM_INTERP_LINEAR: return fT;
	case XANIM_INTERP_EASE_IN: return fT * fT * fT;
	case XANIM_INTERP_EASE_OUT: { float u = fT - 1.0f; return u * u * u + 1.0f; }
	case XANIM_INTERP_EASE_BOTH:
		if ( fT < 0.5f ) return 4.0f * fT * fT * fT;
		{ float u = 2.0f * fT - 2.0f; return 0.5f * u * u * u + 1.0f; }
	case XANIM_INTERP_BEZIER:
		if ( pBezier != NULL ) {
			/* Newton-Raphson solve */
			float u = fT;
			int i;
			for ( i = 0; i < 8; i++ ) {
				float bx = 3.0f*(1.0f-u)*(1.0f-u)*u*pBezier[0] + 3.0f*(1.0f-u)*u*u*pBezier[2] + u*u*u;
				float dx = 3.0f*(1.0f-u)*(1.0f-u)*pBezier[0] + 6.0f*(1.0f-u)*u*(pBezier[2]-pBezier[0]) + 3.0f*u*u*(1.0f-pBezier[2]);
				float err = bx - fT;
				if ( fabsf(err) < 1e-6f || fabsf(dx) < 1e-6f ) break;
				u -= err / dx;
			}
			if ( u < 0.0f ) u = 0.0f;
			if ( u > 1.0f ) u = 1.0f;
			return 3.0f*(1.0f-u)*(1.0f-u)*u*pBezier[1] + 3.0f*(1.0f-u)*u*u*pBezier[3] + u*u*u;
		}
		return fT;
	default: break;
	}
	/* Elastic/bounce/back fallback to smooth */
	if ( fT < 0.5f ) return 4.0f * fT * fT * fT;
	{ float u = 2.0f * fT - 2.0f; return 0.5f * u * u * u + 1.0f; }
}

/* ------------------------------------------------------------------ */
/* Parsing helpers                                                    */
/* ------------------------------------------------------------------ */

static const uint8_t* __getSection(xanim_player_t* p, uint32_t iType, uint32_t* pSize)
{
	uint32_t i;
	const xanim_toc_entry_t* pToc;
	if ( p->pData == NULL ) return NULL;
	pToc = (const xanim_toc_entry_t*)(p->pData + p->tHeader.iTocOffset);
	for ( i = 0; i < p->tHeader.iTocCount; i++ ) {
		if ( pToc[i].iSectionType == iType ) {
			if ( pSize != NULL ) *pSize = pToc[i].iSize;
			return p->pData + pToc[i].iOffset;
		}
	}
	return NULL;
}

static const char* __getString(xanim_player_t* p, uint32_t iIndex)
{
	if ( iIndex == 0xFFFFFFFFu ) return "";
	if ( (int)iIndex >= p->iStringCount ) return "";
	return p->ppStrings[iIndex];
}

static int __parseStringTable(xanim_player_t* p)
{
	uint32_t iSize = 0;
	const uint8_t* pSec = __getSection(p, XANIM_SECTION_STRING, &iSize);
	const xanim_string_table_t* pSt;
	const uint32_t* pOffsets;
	const char* pDataArea;
	uint32_t i;

	if ( pSec == NULL ) return 0;
	pSt = (const xanim_string_table_t*)pSec;
	p->iStringCount = (int)pSt->iCount;
	if ( p->iStringCount <= 0 ) return 0;

	p->ppStrings = (char**)calloc((size_t)p->iStringCount, sizeof(char*));
	if ( p->ppStrings == NULL ) return -1;

	pOffsets = (const uint32_t*)(pSec + sizeof(xanim_string_table_t));
	pDataArea = (const char*)(pSec + pSt->iDataOffset);

	for ( i = 0; i < pSt->iCount; i++ ) {
		const char* s = pDataArea + pOffsets[i];
		size_t len = strlen(s);
		p->ppStrings[i] = (char*)malloc(len + 1);
		if ( p->ppStrings[i] != NULL ) {
			memcpy(p->ppStrings[i], s, len + 1);
		}
	}
	return 0;
}

static int __parseTimeline(xanim_player_t* p)
{
	uint32_t iSize = 0;
	const uint8_t* pSec = __getSection(p, XANIM_SECTION_TIMELINE, &iSize);
	const xanim_layer_t* pLayers;
	uint32_t i;

	if ( pSec == NULL ) return 0;
	pLayers = (const xanim_layer_t*)pSec;
	p->iLayerCount = (int)p->tHeader.iLayerCount;
	if ( p->iLayerCount > XANIM_MAX_LAYERS ) p->iLayerCount = XANIM_MAX_LAYERS;

	for ( i = 0; i < (uint32_t)p->iLayerCount; i++ ) {
		const xanim_layer_t* pSrc = &pLayers[i];
		xanim_rt_layer_t* pDst = &p->arrLayers[i];
		pDst->iId = pSrc->iId;
		pDst->iType = pSrc->iType;
		pDst->iFlags = pSrc->iFlags;
		pDst->iKeyframeCount = (int)pSrc->iKeyframeCount;

		if ( pSrc->iKeyframeCount > 0 && pSrc->iKeyframeOffset < iSize ) {
			pDst->pKeyframes = (xanim_keyframe_t*)malloc(sizeof(xanim_keyframe_t) * pSrc->iKeyframeCount);
			if ( pDst->pKeyframes != NULL ) {
				memcpy(pDst->pKeyframes, pSec + pSrc->iKeyframeOffset, sizeof(xanim_keyframe_t) * pSrc->iKeyframeCount);
			}
		}
	}
	return 0;
}

static int __parseCurves(xanim_player_t* p)
{
	uint32_t iSize = 0;
	const uint8_t* pSec = __getSection(p, XANIM_SECTION_CURVE, &iSize);
	const xanim_curve_t* pCurves;
	uint32_t i;

	if ( pSec == NULL ) return 0;
	pCurves = (const xanim_curve_t*)pSec;
	/* Count curves by scanning */
	p->iCurveCount = 0;
	for ( i = 0; i < XANIM_MAX_CURVES; i++ ) {
		const xanim_curve_t* pSrc = &pCurves[i];
		if ( (const uint8_t*)pSrc >= pSec + iSize ) break;
		if ( pSrc->iId == 0 && pSrc->iKeyCount == 0 ) break;

		{
			xanim_rt_curve_t* pDst = &p->arrCurves[p->iCurveCount];
			pDst->iId = pSrc->iId;
			pDst->iElementId = pSrc->iElementId;
			pDst->iChannelMask = pSrc->iChannelMask;
			pDst->iKeyCount = (int)pSrc->iKeyCount;
			if ( pSrc->iKeyCount > 0 && pSrc->iKeyOffset < iSize ) {
				pDst->pKeys = (xanim_curve_key_t*)malloc(sizeof(xanim_curve_key_t) * pSrc->iKeyCount);
				if ( pDst->pKeys != NULL ) {
					memcpy(pDst->pKeys, pSec + pSrc->iKeyOffset, sizeof(xanim_curve_key_t) * pSrc->iKeyCount);
				}
			}
			p->iCurveCount++;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Lifecycle                                                          */
/* ------------------------------------------------------------------ */

int xanimPlayerCreate(xanim_player* ppPlayer)
{
	xanim_player_t* p;
	if ( ppPlayer == NULL ) return -1;
	p = (xanim_player_t*)calloc(1, sizeof(xanim_player_t));
	if ( p == NULL ) return -2;
	p->bLoop = 1;
	p->fSpeed = 1.0f;
	*ppPlayer = p;
	return 0;
}

void xanimPlayerDestroy(xanim_player pPlayer)
{
	if ( pPlayer == NULL ) return;
	xanimPlayerUnload(pPlayer);
	free(pPlayer);
}

void xanimPlayerUnload(xanim_player pPlayer)
{
	int i;
	if ( pPlayer == NULL ) return;

	/* Free strings */
	if ( pPlayer->ppStrings != NULL ) {
		for ( i = 0; i < pPlayer->iStringCount; i++ ) {
			free(pPlayer->ppStrings[i]);
		}
		free(pPlayer->ppStrings);
		pPlayer->ppStrings = NULL;
	}
	/* Free layers */
	for ( i = 0; i < pPlayer->iLayerCount; i++ ) {
		free(pPlayer->arrLayers[i].pKeyframes);
		free(pPlayer->arrLayers[i].pElements);
		pPlayer->arrLayers[i].pKeyframes = NULL;
		pPlayer->arrLayers[i].pElements = NULL;
	}
	/* Free curves */
	for ( i = 0; i < pPlayer->iCurveCount; i++ ) {
		free(pPlayer->arrCurves[i].pKeys);
		pPlayer->arrCurves[i].pKeys = NULL;
	}
	/* Free file data */
	free(pPlayer->pData);
	pPlayer->pData = NULL;
	pPlayer->iDataSize = 0;
	pPlayer->iLayerCount = 0;
	pPlayer->iCurveCount = 0;
	pPlayer->iStringCount = 0;
	pPlayer->iSymbolCount = 0;
	pPlayer->bPlaying = 0;
	pPlayer->iCurrentFrame = 0;
}

int xanimPlayerLoadMemory(xanim_player pPlayer, const void* pData, int iSize)
{
	xanim_header_t* pHdr;

	if ( pPlayer == NULL || pData == NULL || iSize < XANIM_HEADER_SIZE ) return -1;

	xanimPlayerUnload(pPlayer);

	pPlayer->pData = (uint8_t*)malloc((size_t)iSize);
	if ( pPlayer->pData == NULL ) return -2;
	memcpy(pPlayer->pData, pData, (size_t)iSize);
	pPlayer->iDataSize = iSize;

	pHdr = (xanim_header_t*)pPlayer->pData;
	if ( pHdr->iMagic != XANIM_MAGIC ) {
		xanimPlayerUnload(pPlayer);
		return -3;
	}
	pPlayer->tHeader = *pHdr;

	/* Parse sections */
	__parseStringTable(pPlayer);
	__parseTimeline(pPlayer);
	__parseCurves(pPlayer);

	/* Symbols point into data */
	{
		uint32_t symSize = 0;
		const uint8_t* pSymSec = __getSection(pPlayer, XANIM_SECTION_SYMBOL, &symSize);
		if ( pSymSec != NULL ) {
			pPlayer->pSymbols = (xanim_symbol_t*)pSymSec;
			pPlayer->iSymbolCount = (int)pPlayer->tHeader.iSymbolCount;
		}
	}

	pPlayer->iCurrentFrame = 0;
	pPlayer->bPlaying = 0;
	return 0;
}

int xanimPlayerLoad(xanim_player pPlayer, const char* sPath)
{
	FILE* pFile;
	long iSize;
	uint8_t* pData;
	int ret;

	if ( pPlayer == NULL || sPath == NULL ) return -1;
	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) return -2;
	fseek(pFile, 0, SEEK_END);
	iSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	if ( iSize < XANIM_HEADER_SIZE ) {
		fclose(pFile);
		return -3;
	}
	pData = (uint8_t*)malloc((size_t)iSize);
	if ( pData == NULL ) {
		fclose(pFile);
		return -4;
	}
	if ( fread(pData, 1, (size_t)iSize, pFile) != (size_t)iSize ) {
		free(pData);
		fclose(pFile);
		return -5;
	}
	fclose(pFile);

	ret = xanimPlayerLoadMemory(pPlayer, pData, (int)iSize);
	free(pData);
	return ret;
}

/* ------------------------------------------------------------------ */
/* Playback control                                                   */
/* ------------------------------------------------------------------ */

int xanimPlayerPlay(xanim_player pPlayer)
{
	if ( pPlayer == NULL ) return -1;
	pPlayer->bPlaying = 1;
	pPlayer->fAccum = 0.0;
	return 0;
}

int xanimPlayerStop(xanim_player pPlayer)
{
	if ( pPlayer == NULL ) return -1;
	pPlayer->bPlaying = 0;
	pPlayer->iCurrentFrame = 0;
	pPlayer->fAccum = 0.0;
	return 0;
}

int xanimPlayerPause(xanim_player pPlayer)
{
	if ( pPlayer == NULL ) return -1;
	pPlayer->bPlaying = 0;
	return 0;
}

int xanimPlayerGotoFrame(xanim_player pPlayer, uint32_t iFrame)
{
	if ( pPlayer == NULL ) return -1;
	if ( iFrame >= pPlayer->tHeader.iFrameCount ) iFrame = pPlayer->tHeader.iFrameCount - 1;
	pPlayer->iCurrentFrame = iFrame;
	pPlayer->fAccum = 0.0;
	return 0;
}

int xanimPlayerGotoLabel(xanim_player pPlayer, const char* sLabel)
{
	int li, ki;
	if ( pPlayer == NULL || sLabel == NULL ) return -1;
	/* Search keyframe labels */
	for ( li = 0; li < pPlayer->iLayerCount; li++ ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[li];
		for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
			uint32_t iLabelIdx = pLayer->pKeyframes[ki].iLabelIndex;
			if ( iLabelIdx != 0xFFFFFFFFu ) {
				const char* s = __getString(pPlayer, iLabelIdx);
				if ( strcmp(s, sLabel) == 0 ) {
					return xanimPlayerGotoFrame(pPlayer, pLayer->pKeyframes[ki].iFrame);
				}
			}
		}
	}
	return -4;
}

void xanimPlayerSetLoop(xanim_player pPlayer, int bLoop)
{
	if ( pPlayer != NULL ) pPlayer->bLoop = bLoop;
}

void xanimPlayerSetSpeed(xanim_player pPlayer, float fSpeed)
{
	if ( pPlayer != NULL && fSpeed > 0.0f ) pPlayer->fSpeed = fSpeed;
}

int xanimPlayerIsPlaying(xanim_player pPlayer)
{
	return (pPlayer != NULL) ? pPlayer->bPlaying : 0;
}

uint32_t xanimPlayerGetFrame(xanim_player pPlayer)
{
	return (pPlayer != NULL) ? pPlayer->iCurrentFrame : 0;
}

uint32_t xanimPlayerGetFrameCount(xanim_player pPlayer)
{
	return (pPlayer != NULL) ? pPlayer->tHeader.iFrameCount : 0;
}

float xanimPlayerGetFrameRate(xanim_player pPlayer)
{
	return (pPlayer != NULL) ? pPlayer->tHeader.fFrameRate : 0.0f;
}

/* ------------------------------------------------------------------ */
/* Update                                                             */
/* ------------------------------------------------------------------ */

int xanimPlayerUpdate(xanim_player pPlayer, float fDelta)
{
	double fFrameDur;
	if ( pPlayer == NULL ) return -1;
	if ( !pPlayer->bPlaying ) return 0;
	if ( pPlayer->tHeader.fFrameRate <= 0.0f ) return 0;

	fFrameDur = 1.0 / (double)(pPlayer->tHeader.fFrameRate * pPlayer->fSpeed);
	pPlayer->fAccum += (double)fDelta;

	while ( pPlayer->fAccum >= fFrameDur ) {
		pPlayer->fAccum -= fFrameDur;
		if ( pPlayer->iCurrentFrame < pPlayer->tHeader.iFrameCount - 1 ) {
			pPlayer->iCurrentFrame++;
		} else {
			if ( pPlayer->bLoop ) {
				pPlayer->iCurrentFrame = 0;
			} else {
				pPlayer->bPlaying = 0;
				pPlayer->fAccum = 0.0;
				break;
			}
		}
		/* Fire actions/events at this frame */
		if ( pPlayer->pfnEvent != NULL ) {
			uint32_t iSize = 0;
			const uint8_t* pSec = __getSection(pPlayer, XANIM_SECTION_ACTION, &iSize);
			if ( pSec != NULL ) {
				const xanim_action_t* pActs = (const xanim_action_t*)pSec;
				uint32_t ai, actCount = iSize / sizeof(xanim_action_t);
				for ( ai = 0; ai < actCount; ai++ ) {
					if ( pActs[ai].iFrame == pPlayer->iCurrentFrame && pActs[ai].iType == XANIM_ACTION_EVENT ) {
						pPlayer->pfnEvent(pPlayer, pPlayer->iCurrentFrame, __getString(pPlayer, pActs[ai].iNameIndex), pPlayer->pEventUser);
					}
				}
			}
		}
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Curve evaluation                                                   */
/* ------------------------------------------------------------------ */

static xanim_rt_curve_t* __findCurve(xanim_player_t* p, uint32_t iId)
{
	int i;
	for ( i = 0; i < p->iCurveCount; i++ ) {
		if ( p->arrCurves[i].iId == iId ) return &p->arrCurves[i];
	}
	return NULL;
}

static void __evalCurve(xanim_rt_curve_t* pCurve, float fFrame, float* pOut)
{
	int i;
	const xanim_curve_key_t* pPrev;
	const xanim_curve_key_t* pNext;
	float fT, fEased;

	if ( pCurve == NULL || pCurve->iKeyCount == 0 ) return;

	if ( fFrame <= (float)pCurve->pKeys[0].iFrame ) {
		memcpy(pOut, pCurve->pKeys[0].arrValues, sizeof(float) * XANIM_CHANNEL_COUNT);
		return;
	}
	if ( fFrame >= (float)pCurve->pKeys[pCurve->iKeyCount - 1].iFrame ) {
		memcpy(pOut, pCurve->pKeys[pCurve->iKeyCount - 1].arrValues, sizeof(float) * XANIM_CHANNEL_COUNT);
		return;
	}

	pPrev = &pCurve->pKeys[0];
	pNext = &pCurve->pKeys[1];
	for ( i = 1; i < pCurve->iKeyCount; i++ ) {
		if ( (float)pCurve->pKeys[i].iFrame >= fFrame ) {
			pPrev = &pCurve->pKeys[i - 1];
			pNext = &pCurve->pKeys[i];
			break;
		}
	}

	{
		float fSpan = (float)(pNext->iFrame - pPrev->iFrame);
		if ( fSpan <= 0.0f ) { memcpy(pOut, pNext->arrValues, sizeof(float)*XANIM_CHANNEL_COUNT); return; }
		fT = (fFrame - (float)pPrev->iFrame) / fSpan;
	}

	fEased = __xanimEase(fT, pPrev->iInterpType, pPrev->arrBezier);

	for ( i = 0; i < XANIM_CHANNEL_COUNT; i++ ) {
		if ( (pCurve->iChannelMask & (1u << i)) != 0 ) {
			pOut[i] = pPrev->arrValues[i] + (pNext->arrValues[i] - pPrev->arrValues[i]) * fEased;
		} else {
			pOut[i] = pPrev->arrValues[i];
		}
	}
}

/* ------------------------------------------------------------------ */
/* Render                                                             */
/* ------------------------------------------------------------------ */

int xanimPlayerRender(xanim_player pPlayer, float fX, float fY)
{
	int li;
	if ( pPlayer == NULL || pPlayer->pData == NULL ) return -1;

	/* Render layers bottom to top */
	for ( li = 0; li < pPlayer->iLayerCount; li++ ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[li];
		xanim_keyframe_t* pKfA = NULL;
		xanim_keyframe_t* pKfB = NULL;
		int ki;

		if ( (pLayer->iFlags & XANIM_LAYER_FLAG_VISIBLE) == 0 ) continue;
		if ( pLayer->iType == XANIM_LAYER_GUIDE ) continue;

		/* Find surrounding keyframes */
		for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
			xanim_keyframe_t* pKf = &pLayer->pKeyframes[ki];
			if ( pKf->iFrame <= pPlayer->iCurrentFrame ) {
				if ( pKfA == NULL || pKf->iFrame > pKfA->iFrame ) pKfA = pKf;
			}
			if ( pKf->iFrame > pPlayer->iCurrentFrame ) {
				if ( pKfB == NULL || pKf->iFrame < pKfB->iFrame ) pKfB = pKf;
			}
		}

		if ( pKfA == NULL ) continue;
		if ( (pKfA->iFlags & XANIM_KF_BLANK) != 0 ) continue;

		/* For each element on the keyframe, render its symbol */
		/* Elements are stored after the timeline section - use offset */
		{
			uint32_t iSize = 0;
			const uint8_t* pTlSec = __getSection(pPlayer, XANIM_SECTION_TIMELINE, &iSize);
			if ( pTlSec == NULL ) continue;

			if ( pKfA->iElementOffset < iSize && pKfA->iElementCount > 0 ) {
				const xanim_element_t* pElems = (const xanim_element_t*)(pTlSec + pKfA->iElementOffset);
				int ei;

				for ( ei = 0; ei < (int)pKfA->iElementCount; ei++ ) {
					const xanim_element_t* pElem = &pElems[ei];
					float fTx = pElem->fTx, fTy = pElem->fTy;
					float fScaleX = pElem->fScaleX, fScaleY = pElem->fScaleY;
					float fRot = pElem->fRotation;
					float fOpacity = pElem->fOpacity;

					if ( (pElem->iFlags & XANIM_ELEM_FLAG_VISIBLE) == 0 ) continue;

					/* Apply curve interpolation if tween exists */
					if ( pElem->iTweenId != 0xFFFFFFFFu && pKfB != NULL ) {
						xanim_rt_curve_t* pCurve = __findCurve(pPlayer, pElem->iTweenId);
						if ( pCurve != NULL ) {
							float arrVals[XANIM_CHANNEL_COUNT];
							__evalCurve(pCurve, (float)pPlayer->iCurrentFrame, arrVals);
							fTx = arrVals[XANIM_CHANNEL_X];
							fTy = arrVals[XANIM_CHANNEL_Y];
							fScaleX = arrVals[XANIM_CHANNEL_SCALE_X];
							fScaleY = arrVals[XANIM_CHANNEL_SCALE_Y];
							fRot = arrVals[XANIM_CHANNEL_ROTATION];
							fOpacity = arrVals[XANIM_CHANNEL_OPACITY];
						}
					}

					/* Render symbol at interpolated transform */
					{
						xge_shape_ex pShape;
						xge_shape_ex_matrix_t tMatrix;

						xgeShapeExMatrixIdentity(&tMatrix);
						xgeShapeExMatrixTranslate(&tMatrix, fX + fTx, fY + fTy);
						if ( fRot != 0.0f ) {
							/* Rotation via matrix would need rotate function; use scale for now */
						}
						xgeShapeExMatrixScale(&tMatrix, fScaleX, fScaleY);

						/* Find symbol and render its graphic data */
						if ( pPlayer->pSymbols != NULL ) {
							int si;
							for ( si = 0; si < pPlayer->iSymbolCount; si++ ) {
								if ( pPlayer->pSymbols[si].iId == pElem->iSymbolId ) {
									/* Render graphic symbol shapes */
									uint32_t symSize = 0;
									const uint8_t* pSymSec = __getSection(pPlayer, XANIM_SECTION_SYMBOL, &symSize);
									if ( pSymSec != NULL && pPlayer->pSymbols[si].iDataOffset < symSize ) {
										const uint8_t* pPayload = pSymSec + pPlayer->pSymbols[si].iDataOffset;
										const xanim_graphic_data_t* pGfx = (const xanim_graphic_data_t*)pPayload;
										uint32_t shi;
										const uint8_t* pPtr = pPayload + sizeof(xanim_graphic_data_t);

										for ( shi = 0; shi < pGfx->iShapeCount; shi++ ) {
											const xanim_shape_record_t* pRec = (const xanim_shape_record_t*)pPtr;
											const uint8_t* pPathData = pPtr + sizeof(xanim_shape_record_t);

											if ( xgeShapeExCreate(&pShape) == XGE_OK ) {
												/* Replay path commands */
												const uint8_t* pCmd = pPathData;
												const uint8_t* pEnd = pPathData + pRec->iPathSize;
												while ( pCmd < pEnd ) {
													uint8_t op = *pCmd++;
													switch ( op ) {
													case XANIM_PATH_MOVE_TO: {
														float px, py;
														memcpy(&px, pCmd, 4); pCmd += 4;
														memcpy(&py, pCmd, 4); pCmd += 4;
														xgeShapeExMoveTo(pShape, px, py);
														break;
													}
													case XANIM_PATH_LINE_TO: {
														float px, py;
														memcpy(&px, pCmd, 4); pCmd += 4;
														memcpy(&py, pCmd, 4); pCmd += 4;
														xgeShapeExLineTo(pShape, px, py);
														break;
													}
													case XANIM_PATH_QUAD_TO: {
														float cx, cy, px, py;
														memcpy(&cx, pCmd, 4); pCmd += 4;
														memcpy(&cy, pCmd, 4); pCmd += 4;
														memcpy(&px, pCmd, 4); pCmd += 4;
														memcpy(&py, pCmd, 4); pCmd += 4;
														xgeShapeExQuadTo(pShape, cx, cy, px, py);
														break;
													}
													case XANIM_PATH_CUBIC_TO: {
														float c1x, c1y, c2x, c2y, px, py;
														memcpy(&c1x, pCmd, 4); pCmd += 4;
														memcpy(&c1y, pCmd, 4); pCmd += 4;
														memcpy(&c2x, pCmd, 4); pCmd += 4;
														memcpy(&c2y, pCmd, 4); pCmd += 4;
														memcpy(&px, pCmd, 4); pCmd += 4;
														memcpy(&py, pCmd, 4); pCmd += 4;
														xgeShapeExCubicTo(pShape, c1x, c1y, c2x, c2y, px, py);
														break;
													}
													case XANIM_PATH_CLOSE:
														xgeShapeExClose(pShape);
														break;
													default:
														pCmd = pEnd; /* unknown, bail */
														break;
													}
												}

												/* Apply fill */
												if ( pRec->iFillType == XANIM_PAINT_SOLID ) {
													uint32_t iColor;
													memcpy(&iColor, pPathData + pRec->iPathSize, 4);
													/* Apply opacity */
													{
														uint32_t a = (uint32_t)((float)(iColor & 0xFF) * fOpacity);
														iColor = (iColor & 0xFFFFFF00u) | (a & 0xFF);
													}
													xgeShapeExFillColor(pShape, iColor);
												}
												/* Apply stroke */
												if ( pRec->iStrokeType == XANIM_PAINT_SOLID ) {
													uint32_t iColor;
													const uint8_t* pStrokeData = pPathData + pRec->iPathSize;
													if ( pRec->iFillType == XANIM_PAINT_SOLID ) pStrokeData += 4;
													memcpy(&iColor, pStrokeData, 4);
													xgeShapeExStrokeColor(pShape, iColor);
													xgeShapeExStrokeWidth(pShape, pRec->fStrokeWidth);
												}

												xgeShapeExTransformSet(pShape, &tMatrix);
												xgeShapeExDraw(pShape, fOpacity);
												xgeShapeExDestroy(pShape);
											}

											/* Advance to next shape record */
											pPtr += sizeof(xanim_shape_record_t) + pRec->iPathSize;
											if ( pRec->iFillType == XANIM_PAINT_SOLID ) pPtr += 4;
											if ( pRec->iStrokeType == XANIM_PAINT_SOLID ) pPtr += 4;
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* Query                                                              */
/* ------------------------------------------------------------------ */

int xanimPlayerGetSize(xanim_player pPlayer, float* pW, float* pH)
{
	if ( pPlayer == NULL ) return -1;
	if ( pW != NULL ) *pW = pPlayer->tHeader.fStageWidth;
	if ( pH != NULL ) *pH = pPlayer->tHeader.fStageHeight;
	return 0;
}

int xanimPlayerGetSymbolCount(xanim_player pPlayer)
{
	return (pPlayer != NULL) ? pPlayer->iSymbolCount : 0;
}

void xanimPlayerSetEventCallback(xanim_player pPlayer, xanim_event_proc proc, void* pUser)
{
	if ( pPlayer == NULL ) return;
	pPlayer->pfnEvent = proc;
	pPlayer->pEventUser = pUser;
}

/* ------------------------------------------------------------------ */
/* Extended render / hit-test / instance control                      */
/* ------------------------------------------------------------------ */

int xanimPlayerRenderEx(xanim_player pPlayer, const void* pMatrix)
{
	/* TODO: apply parent matrix then delegate to normal render path */
	(void)pMatrix;
	return xanimPlayerRender(pPlayer, 0.0f, 0.0f);
}

int xanimPlayerGetInstanceBounds(xanim_player pPlayer, uint32_t iInstanceId,
                                 float* pX, float* pY, float* pW, float* pH)
{
	int iLayer, iKf, iElem;
	if ( pPlayer == NULL ) return -1;

	for ( iLayer = 0; iLayer < pPlayer->iLayerCount; iLayer++ ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[iLayer];
		for ( iKf = 0; iKf < pLayer->iKeyframeCount; iKf++ ) {
			xanim_keyframe_t* pKf = &pLayer->pKeyframes[iKf];
			for ( iElem = 0; iElem < (int)pKf->iElementCount; iElem++ ) {
				xanim_element_t* pEl = &pLayer->pElements[pKf->iElementOffset + iElem];
				if ( pEl->iInstanceId == iInstanceId ) {
					/* Find symbol bounds */
					int iSym;
					for ( iSym = 0; iSym < pPlayer->iSymbolCount; iSym++ ) {
						if ( pPlayer->pSymbols[iSym].iId == pEl->iSymbolId ) {
							if ( pX ) *pX = pEl->fTx + pPlayer->pSymbols[iSym].fBoundsX;
							if ( pY ) *pY = pEl->fTy + pPlayer->pSymbols[iSym].fBoundsY;
							if ( pW ) *pW = pPlayer->pSymbols[iSym].fBoundsW * pEl->fScaleX;
							if ( pH ) *pH = pPlayer->pSymbols[iSym].fBoundsH * pEl->fScaleY;
							return 0;
						}
					}
				}
			}
		}
	}
	return -1; /* not found */
}

int xanimPlayerHitTest(xanim_player pPlayer, float fX, float fY, uint32_t* pInstanceId)
{
	int iLayer, iKf, iElem;
	if ( pPlayer == NULL ) return -1;

	/* Top-most first: iterate layers in reverse, elements in reverse depth */
	for ( iLayer = pPlayer->iLayerCount - 1; iLayer >= 0; iLayer-- ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[iLayer];
		if ( !(pLayer->iFlags & XANIM_LAYER_FLAG_VISIBLE) ) continue;
		for ( iKf = 0; iKf < pLayer->iKeyframeCount; iKf++ ) {
			xanim_keyframe_t* pKf = &pLayer->pKeyframes[iKf];
			if ( pKf->iFrame > pPlayer->iCurrentFrame ) break;
			for ( iElem = (int)pKf->iElementCount - 1; iElem >= 0; iElem-- ) {
				xanim_element_t* pEl = &pLayer->pElements[pKf->iElementOffset + iElem];
				int iSym;
				for ( iSym = 0; iSym < pPlayer->iSymbolCount; iSym++ ) {
					if ( pPlayer->pSymbols[iSym].iId == pEl->iSymbolId ) {
						float bx = pEl->fTx + pPlayer->pSymbols[iSym].fBoundsX * pEl->fScaleX;
						float by = pEl->fTy + pPlayer->pSymbols[iSym].fBoundsY * pEl->fScaleY;
						float bw = pPlayer->pSymbols[iSym].fBoundsW * pEl->fScaleX;
						float bh = pPlayer->pSymbols[iSym].fBoundsH * pEl->fScaleY;
						if ( fX >= bx && fX <= bx + bw && fY >= by && fY <= by + bh ) {
							if ( pInstanceId ) *pInstanceId = pEl->iInstanceId;
							return 0;
						}
						break;
					}
				}
			}
		}
	}
	return -1; /* no hit */
}

int xanimPlayerInstanceSetVisible(xanim_player pPlayer, uint32_t iInstanceId, int bVisible)
{
	int iLayer, iKf, iElem;
	if ( pPlayer == NULL ) return -1;
	for ( iLayer = 0; iLayer < pPlayer->iLayerCount; iLayer++ ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[iLayer];
		for ( iKf = 0; iKf < pLayer->iKeyframeCount; iKf++ ) {
			xanim_keyframe_t* pKf = &pLayer->pKeyframes[iKf];
			for ( iElem = 0; iElem < (int)pKf->iElementCount; iElem++ ) {
				xanim_element_t* pEl = &pLayer->pElements[pKf->iElementOffset + iElem];
				if ( pEl->iInstanceId == iInstanceId ) {
					if ( bVisible ) pEl->iFlags |= XANIM_ELEM_FLAG_VISIBLE;
					else pEl->iFlags &= ~XANIM_ELEM_FLAG_VISIBLE;
					return 0;
				}
			}
		}
	}
	return -1;
}

int xanimPlayerInstanceSetOpacity(xanim_player pPlayer, uint32_t iInstanceId, float fOpacity)
{
	int iLayer, iKf, iElem;
	if ( pPlayer == NULL ) return -1;
	for ( iLayer = 0; iLayer < pPlayer->iLayerCount; iLayer++ ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[iLayer];
		for ( iKf = 0; iKf < pLayer->iKeyframeCount; iKf++ ) {
			xanim_keyframe_t* pKf = &pLayer->pKeyframes[iKf];
			for ( iElem = 0; iElem < (int)pKf->iElementCount; iElem++ ) {
				xanim_element_t* pEl = &pLayer->pElements[pKf->iElementOffset + iElem];
				if ( pEl->iInstanceId == iInstanceId ) {
					pEl->fOpacity = fOpacity;
					return 0;
				}
			}
		}
	}
	return -1;
}

int xanimPlayerInstanceSetTransform(xanim_player pPlayer, uint32_t iInstanceId, const void* pMatrix)
{
	/* TODO: decompose matrix into tx/ty/scale/rotation and apply */
	(void)pMatrix;
	int iLayer, iKf, iElem;
	if ( pPlayer == NULL ) return -1;
	for ( iLayer = 0; iLayer < pPlayer->iLayerCount; iLayer++ ) {
		xanim_rt_layer_t* pLayer = &pPlayer->arrLayers[iLayer];
		for ( iKf = 0; iKf < pLayer->iKeyframeCount; iKf++ ) {
			xanim_keyframe_t* pKf = &pLayer->pKeyframes[iKf];
			for ( iElem = 0; iElem < (int)pKf->iElementCount; iElem++ ) {
				xanim_element_t* pEl = &pLayer->pElements[pKf->iElementOffset + iElem];
				if ( pEl->iInstanceId == iInstanceId ) {
					return 0; /* found - matrix decomposition pending */
				}
			}
		}
	}
	return -1;
}
