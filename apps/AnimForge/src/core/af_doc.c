/*
 * af_doc.c - AnimForge document model implementation
 */

#include "af_doc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Document lifecycle                                                 */
/* ------------------------------------------------------------------ */

int afDocInit(af_doc pDoc)
{
	if ( pDoc == NULL ) return -1;
	memset(pDoc, 0, sizeof(af_doc_t));
	pDoc->fFrameRate = 24.0f;
	pDoc->iFrameCount = 120;
	pDoc->fStageWidth = 800.0f;
	pDoc->fStageHeight = 600.0f;
	pDoc->iBackgroundColor = 0xFFFFFFFFu;
	pDoc->iNextSymbolId = 1;
	pDoc->iNextInstanceId = 1;
	pDoc->iNextCurveId = 1;
	pDoc->iNextResourceId = 1;
	pDoc->iNextLayerId = 1;
	return 0;
}

void afDocClear(af_doc pDoc)
{
	int i;
	if ( pDoc == NULL ) return;
	/* Free resource data */
	for ( i = 0; i < pDoc->iResourceCount; i++ ) {
		if ( pDoc->arrResources[i].pData != NULL ) {
			free(pDoc->arrResources[i].pData);
		}
	}
	afDocInit(pDoc);
}

int afDocSetStage(af_doc pDoc, float fWidth, float fHeight, float fFrameRate, uint32_t iFrameCount)
{
	if ( pDoc == NULL ) return -1;
	if ( fWidth <= 0.0f || fHeight <= 0.0f ) return -2;
	if ( fFrameRate <= 0.0f ) return -2;
	if ( iFrameCount == 0 ) return -2;
	pDoc->fStageWidth = fWidth;
	pDoc->fStageHeight = fHeight;
	pDoc->fFrameRate = fFrameRate;
	pDoc->iFrameCount = iFrameCount;
	pDoc->bModified = 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Symbol management                                                  */
/* ------------------------------------------------------------------ */

int afDocAddSymbol(af_doc pDoc, int iType, const char* sName, uint32_t* pId)
{
	af_symbol_t* pSym;
	if ( pDoc == NULL ) return -1;
	if ( pDoc->iSymbolCount >= AF_DOC_MAX_SYMBOLS ) return -3;
	pSym = &pDoc->arrSymbols[pDoc->iSymbolCount];
	memset(pSym, 0, sizeof(af_symbol_t));
	pSym->iId = pDoc->iNextSymbolId++;
	pSym->iType = iType;
	if ( sName != NULL ) {
		strncpy(pSym->sName, sName, AF_DOC_MAX_NAME - 1);
	}
	pSym->iTimelineIndex = (iType == AF_SYMBOL_MOVIE_CLIP) ? 0 : -1;
	pDoc->iSymbolCount++;
	pDoc->bModified = 1;
	if ( pId != NULL ) *pId = pSym->iId;
	return 0;
}

int afDocRemoveSymbol(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return -1;
	for ( i = 0; i < pDoc->iSymbolCount; i++ ) {
		if ( pDoc->arrSymbols[i].iId == iId ) {
			memmove(&pDoc->arrSymbols[i], &pDoc->arrSymbols[i + 1],
				(size_t)(pDoc->iSymbolCount - i - 1) * sizeof(af_symbol_t));
			pDoc->iSymbolCount--;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4; /* not found */
}

af_symbol_t* afDocFindSymbol(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return NULL;
	for ( i = 0; i < pDoc->iSymbolCount; i++ ) {
		if ( pDoc->arrSymbols[i].iId == iId ) {
			return &pDoc->arrSymbols[i];
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Resource management                                                */
/* ------------------------------------------------------------------ */

int afDocAddResource(af_doc pDoc, int iType, const char* sName, void* pData, int iDataSize, int iWidth, int iHeight, uint32_t* pId)
{
	af_resource_t* pRes;
	if ( pDoc == NULL ) return -1;
	if ( pDoc->iResourceCount >= AF_DOC_MAX_RESOURCES ) return -3;
	pRes = &pDoc->arrResources[pDoc->iResourceCount];
	memset(pRes, 0, sizeof(af_resource_t));
	pRes->iId = pDoc->iNextResourceId++;
	pRes->iType = iType;
	pRes->iWidth = iWidth;
	pRes->iHeight = iHeight;
	pRes->pData = pData;
	pRes->iDataSize = iDataSize;
	if ( sName != NULL ) {
		strncpy(pRes->sName, sName, AF_DOC_MAX_NAME - 1);
	}
	pDoc->iResourceCount++;
	pDoc->bModified = 1;
	if ( pId != NULL ) *pId = pRes->iId;
	return 0;
}

int afDocRemoveResource(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return -1;
	for ( i = 0; i < pDoc->iResourceCount; i++ ) {
		if ( pDoc->arrResources[i].iId == iId ) {
			if ( pDoc->arrResources[i].pData != NULL ) {
				free(pDoc->arrResources[i].pData);
			}
			memmove(&pDoc->arrResources[i], &pDoc->arrResources[i + 1],
				(size_t)(pDoc->iResourceCount - i - 1) * sizeof(af_resource_t));
			pDoc->iResourceCount--;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4;
}

af_resource_t* afDocFindResource(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return NULL;
	for ( i = 0; i < pDoc->iResourceCount; i++ ) {
		if ( pDoc->arrResources[i].iId == iId ) {
			return &pDoc->arrResources[i];
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Layer management                                                   */
/* ------------------------------------------------------------------ */

int afDocAddLayer(af_doc pDoc, const char* sName, int iType, uint32_t* pId)
{
	af_layer_t* pLayer;
	if ( pDoc == NULL ) return -1;
	if ( pDoc->tTimeline.iLayerCount >= AF_DOC_MAX_LAYERS ) return -3;
	pLayer = &pDoc->tTimeline.arrLayers[pDoc->tTimeline.iLayerCount];
	memset(pLayer, 0, sizeof(af_layer_t));
	pLayer->iId = pDoc->iNextLayerId++;
	pLayer->iType = iType;
	pLayer->iFlags = AF_LAYER_FLAG_VISIBLE;
	pLayer->iParentLayer = 0xFFFFFFFFu;
	pLayer->iColor = 0x007ACCFFu;
	if ( sName != NULL ) {
		strncpy(pLayer->sName, sName, AF_DOC_MAX_NAME - 1);
	} else {
		snprintf(pLayer->sName, AF_DOC_MAX_NAME, "Layer %u", pLayer->iId);
	}
	pDoc->tTimeline.iLayerCount++;
	pDoc->bModified = 1;
	if ( pId != NULL ) *pId = pLayer->iId;
	return 0;
}

int afDocRemoveLayer(af_doc pDoc, uint32_t iId)
{
	int i;
	af_timeline_t* pTl;
	if ( pDoc == NULL ) return -1;
	pTl = &pDoc->tTimeline;
	for ( i = 0; i < pTl->iLayerCount; i++ ) {
		if ( pTl->arrLayers[i].iId == iId ) {
			memmove(&pTl->arrLayers[i], &pTl->arrLayers[i + 1],
				(size_t)(pTl->iLayerCount - i - 1) * sizeof(af_layer_t));
			pTl->iLayerCount--;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4;
}

af_layer_t* afDocFindLayer(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return NULL;
	for ( i = 0; i < pDoc->tTimeline.iLayerCount; i++ ) {
		if ( pDoc->tTimeline.arrLayers[i].iId == iId ) {
			return &pDoc->tTimeline.arrLayers[i];
		}
	}
	return NULL;
}

int afDocMoveLayer(af_doc pDoc, uint32_t iId, int iDirection)
{
	int i;
	af_timeline_t* pTl;
	af_layer_t tmp;
	if ( pDoc == NULL ) return -1;
	pTl = &pDoc->tTimeline;
	for ( i = 0; i < pTl->iLayerCount; i++ ) {
		if ( pTl->arrLayers[i].iId == iId ) {
			int j = i + iDirection;
			if ( j < 0 || j >= pTl->iLayerCount ) return -2;
			tmp = pTl->arrLayers[i];
			pTl->arrLayers[i] = pTl->arrLayers[j];
			pTl->arrLayers[j] = tmp;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4;
}

/* ------------------------------------------------------------------ */
/* Keyframe management                                                */
/* ------------------------------------------------------------------ */

int afDocSetKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, uint32_t iFlags)
{
	af_layer_t* pLayer;
	af_keyframe_t* pKf;
	int i;
	if ( pDoc == NULL ) return -1;
	pLayer = afDocFindLayer(pDoc, iLayerId);
	if ( pLayer == NULL ) return -4;
	/* Check if keyframe already exists */
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		if ( pLayer->arrKeyframes[i].iFrame == iFrame ) {
			pLayer->arrKeyframes[i].iFlags = iFlags;
			pDoc->bModified = 1;
			return 0;
		}
	}
	/* Add new keyframe */
	if ( pLayer->iKeyframeCount >= (int)(sizeof(pLayer->arrKeyframes) / sizeof(pLayer->arrKeyframes[0])) ) return -3;
	pKf = &pLayer->arrKeyframes[pLayer->iKeyframeCount];
	memset(pKf, 0, sizeof(af_keyframe_t));
	pKf->iFrame = iFrame;
	pKf->iFlags = iFlags;
	pLayer->iKeyframeCount++;
	pDoc->bModified = 1;
	return 0;
}

int afDocClearKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	af_layer_t* pLayer;
	int i;
	if ( pDoc == NULL ) return -1;
	pLayer = afDocFindLayer(pDoc, iLayerId);
	if ( pLayer == NULL ) return -4;
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		if ( pLayer->arrKeyframes[i].iFrame == iFrame ) {
			memmove(&pLayer->arrKeyframes[i], &pLayer->arrKeyframes[i + 1],
				(size_t)(pLayer->iKeyframeCount - i - 1) * sizeof(af_keyframe_t));
			pLayer->iKeyframeCount--;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4;
}

af_keyframe_t* afDocGetKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	af_layer_t* pLayer;
	int i;
	if ( pDoc == NULL ) return NULL;
	pLayer = afDocFindLayer(pDoc, iLayerId);
	if ( pLayer == NULL ) return NULL;
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		if ( pLayer->arrKeyframes[i].iFrame == iFrame ) {
			return &pLayer->arrKeyframes[i];
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Element management                                                 */
/* ------------------------------------------------------------------ */

int afDocAddElement(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, uint32_t iSymbolId, uint32_t* pInstanceId)
{
	af_keyframe_t* pKf;
	af_element_t* pElem;
	if ( pDoc == NULL ) return -1;
	pKf = afDocGetKeyframe(pDoc, iLayerId, iFrame);
	if ( pKf == NULL ) {
		/* Auto-create keyframe */
		if ( afDocSetKeyframe(pDoc, iLayerId, iFrame, 0) != 0 ) return -3;
		pKf = afDocGetKeyframe(pDoc, iLayerId, iFrame);
		if ( pKf == NULL ) return -3;
	}
	if ( pKf->iElementCount >= 64 ) return -3;
	pElem = &pKf->arrElements[pKf->iElementCount];
	memset(pElem, 0, sizeof(af_element_t));
	pElem->iSymbolId = iSymbolId;
	pElem->iInstanceId = pDoc->iNextInstanceId++;
	pElem->fScaleX = 1.0f;
	pElem->fScaleY = 1.0f;
	pElem->fOpacity = 1.0f;
	pElem->iTweenId = 0xFFFFFFFFu;
	pElem->iFlags = AF_ELEM_FLAG_VISIBLE;
	pElem->iDepth = (uint32_t)pKf->iElementCount;
	pKf->iElementCount++;
	pDoc->bModified = 1;
	if ( pInstanceId != NULL ) *pInstanceId = pElem->iInstanceId;
	return 0;
}

int afDocRemoveElement(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, uint32_t iInstanceId)
{
	af_keyframe_t* pKf;
	int i;
	if ( pDoc == NULL ) return -1;
	pKf = afDocGetKeyframe(pDoc, iLayerId, iFrame);
	if ( pKf == NULL ) return -4;
	for ( i = 0; i < pKf->iElementCount; i++ ) {
		if ( pKf->arrElements[i].iInstanceId == iInstanceId ) {
			memmove(&pKf->arrElements[i], &pKf->arrElements[i + 1],
				(size_t)(pKf->iElementCount - i - 1) * sizeof(af_element_t));
			pKf->iElementCount--;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4;
}

af_element_t* afDocFindElement(af_doc pDoc, uint32_t iInstanceId)
{
	int li, ki, ei;
	af_timeline_t* pTl;
	if ( pDoc == NULL ) return NULL;
	pTl = &pDoc->tTimeline;
	for ( li = 0; li < pTl->iLayerCount; li++ ) {
		af_layer_t* pLayer = &pTl->arrLayers[li];
		for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
			af_keyframe_t* pKf = &pLayer->arrKeyframes[ki];
			for ( ei = 0; ei < pKf->iElementCount; ei++ ) {
				if ( pKf->arrElements[ei].iInstanceId == iInstanceId ) {
					return &pKf->arrElements[ei];
				}
			}
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Curve management                                                   */
/* ------------------------------------------------------------------ */

int afDocAddCurve(af_doc pDoc, uint32_t iElementId, uint32_t iChannelMask, uint32_t* pId)
{
	af_curve_t* pCurve;
	if ( pDoc == NULL ) return -1;
	if ( pDoc->iCurveCount >= AF_DOC_MAX_CURVES ) return -3;
	pCurve = &pDoc->arrCurves[pDoc->iCurveCount];
	memset(pCurve, 0, sizeof(af_curve_t));
	pCurve->iId = pDoc->iNextCurveId++;
	pCurve->iElementId = iElementId;
	pCurve->iChannelMask = iChannelMask;
	pDoc->iCurveCount++;
	pDoc->bModified = 1;
	if ( pId != NULL ) *pId = pCurve->iId;
	return 0;
}

int afDocRemoveCurve(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return -1;
	for ( i = 0; i < pDoc->iCurveCount; i++ ) {
		if ( pDoc->arrCurves[i].iId == iId ) {
			memmove(&pDoc->arrCurves[i], &pDoc->arrCurves[i + 1],
				(size_t)(pDoc->iCurveCount - i - 1) * sizeof(af_curve_t));
			pDoc->iCurveCount--;
			pDoc->bModified = 1;
			return 0;
		}
	}
	return -4;
}

af_curve_t* afDocFindCurve(af_doc pDoc, uint32_t iId)
{
	int i;
	if ( pDoc == NULL ) return NULL;
	for ( i = 0; i < pDoc->iCurveCount; i++ ) {
		if ( pDoc->arrCurves[i].iId == iId ) {
			return &pDoc->arrCurves[i];
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Action management                                                  */
/* ------------------------------------------------------------------ */

int afDocAddAction(af_doc pDoc, uint32_t iFrame, uint32_t iLayerId, uint32_t iType, int32_t iParam, const char* sName)
{
	af_action_t* pAct;
	if ( pDoc == NULL ) return -1;
	if ( pDoc->iActionCount >= AF_DOC_MAX_ACTIONS ) return -3;
	pAct = &pDoc->arrActions[pDoc->iActionCount];
	memset(pAct, 0, sizeof(af_action_t));
	pAct->iFrame = iFrame;
	pAct->iLayerId = iLayerId;
	pAct->iType = iType;
	pAct->iParam = iParam;
	if ( sName != NULL ) {
		strncpy(pAct->sName, sName, AF_DOC_MAX_NAME - 1);
	}
	pDoc->iActionCount++;
	pDoc->bModified = 1;
	return 0;
}

int afDocRemoveAction(af_doc pDoc, int iIndex)
{
	if ( pDoc == NULL ) return -1;
	if ( iIndex < 0 || iIndex >= pDoc->iActionCount ) return -2;
	memmove(&pDoc->arrActions[iIndex], &pDoc->arrActions[iIndex + 1],
		(size_t)(pDoc->iActionCount - iIndex - 1) * sizeof(af_action_t));
	pDoc->iActionCount--;
	pDoc->bModified = 1;
	return 0;
}
