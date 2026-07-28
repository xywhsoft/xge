/*
 * af_symbol.c - Symbol definition and instance management
 *
 * Higher-level Symbol operations: creation wizards, nesting,
 * instance management, bounds calculation, and Symbol editing mode.
 */

#include "af_symbol.h"
#include "../core/af_doc.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Symbol creation helpers                                            */
/* ------------------------------------------------------------------ */

int afSymbolCreateGraphic(af_doc pDoc, const char* sName, uint32_t* pId)
{
	return afDocAddSymbol(pDoc, AF_SYMBOL_GRAPHIC, sName, pId);
}

int afSymbolCreateSprite(af_doc pDoc, const char* sName, uint32_t iResourceId, uint32_t* pId)
{
	uint32_t id;
	af_symbol_t* pSym;
	int ret;

	ret = afDocAddSymbol(pDoc, AF_SYMBOL_SPRITE, sName, &id);
	if ( ret != 0 ) return ret;

	pSym = afDocFindSymbol(pDoc, id);
	if ( pSym == NULL ) return -1;

	pSym->iResourceId = iResourceId;

	/* Set bounds from resource dimensions */
	{
		af_resource_t* pRes = afDocFindResource(pDoc, iResourceId);
		if ( pRes != NULL ) {
			pSym->fBoundsW = (float)pRes->iWidth;
			pSym->fBoundsH = (float)pRes->iHeight;
		}
	}

	if ( pId ) *pId = id;
	return 0;
}

int afSymbolCreateGroup(af_doc pDoc, const char* sName, uint32_t* pId)
{
	return afDocAddSymbol(pDoc, AF_SYMBOL_GROUP, sName, pId);
}

int afSymbolCreateMovieClip(af_doc pDoc, const char* sName, uint32_t* pId)
{
	uint32_t id;
	af_symbol_t* pSym;
	int ret;

	ret = afDocAddSymbol(pDoc, AF_SYMBOL_MOVIE_CLIP, sName, &id);
	if ( ret != 0 ) return ret;

	pSym = afDocFindSymbol(pDoc, id);
	if ( pSym == NULL ) return -1;

	pSym->iTimelineIndex = -1; /* no nested timeline yet */

	if ( pId ) *pId = id;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Group child management                                             */
/* ------------------------------------------------------------------ */

int afSymbolAddChild(af_doc pDoc, uint32_t iGroupId, uint32_t iChildSymbolId,
                     float fTx, float fTy, uint32_t iDepth)
{
	af_symbol_t* pGroup = afDocFindSymbol(pDoc, iGroupId);
	af_symbol_child_t* pChild;

	if ( pGroup == NULL ) return -1;
	if ( pGroup->iType != AF_SYMBOL_GROUP && pGroup->iType != AF_SYMBOL_MOVIE_CLIP )
		return -2;
	if ( pGroup->iChildCount >= AF_DOC_MAX_CHILDREN )
		return -3;

	pChild = &pGroup->arrChildren[pGroup->iChildCount];
	memset(pChild, 0, sizeof(*pChild));
	pChild->iSymbolId = iChildSymbolId;
	pChild->fTx = fTx;
	pChild->fTy = fTy;
	pChild->fScaleX = 1.0f;
	pChild->fScaleY = 1.0f;
	pChild->fRotation = 0.0f;
	pChild->fOpacity = 1.0f;
	pChild->iDepth = iDepth;
	pGroup->iChildCount++;

	return 0;
}

int afSymbolRemoveChild(af_doc pDoc, uint32_t iGroupId, int iChildIndex)
{
	af_symbol_t* pGroup = afDocFindSymbol(pDoc, iGroupId);
	int i;

	if ( pGroup == NULL ) return -1;
	if ( iChildIndex < 0 || iChildIndex >= pGroup->iChildCount )
		return -2;

	/* Shift remaining children down */
	for ( i = iChildIndex; i < pGroup->iChildCount - 1; i++ ) {
		pGroup->arrChildren[i] = pGroup->arrChildren[i + 1];
	}
	pGroup->iChildCount--;
	return 0;
}

int afSymbolMoveChild(af_doc pDoc, uint32_t iGroupId, int iFrom, int iTo)
{
	af_symbol_t* pGroup = afDocFindSymbol(pDoc, iGroupId);
	af_symbol_child_t tmp;

	if ( pGroup == NULL ) return -1;
	if ( iFrom < 0 || iFrom >= pGroup->iChildCount ) return -2;
	if ( iTo < 0 || iTo >= pGroup->iChildCount ) return -2;

	tmp = pGroup->arrChildren[iFrom];
	if ( iFrom < iTo ) {
		int i;
		for ( i = iFrom; i < iTo; i++ )
			pGroup->arrChildren[i] = pGroup->arrChildren[i + 1];
	} else {
		int i;
		for ( i = iFrom; i > iTo; i-- )
			pGroup->arrChildren[i] = pGroup->arrChildren[i - 1];
	}
	pGroup->arrChildren[iTo] = tmp;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Shape record management for GRAPHIC symbols                        */
/* ------------------------------------------------------------------ */

int afSymbolAddShape(af_doc pDoc, uint32_t iSymbolId, const af_shape_record_t* pShape, int* pIndex)
{
	af_symbol_t* pSym = afDocFindSymbol(pDoc, iSymbolId);
	if ( pSym == NULL ) return -1;
	if ( pSym->iType != AF_SYMBOL_GRAPHIC ) return -2;
	if ( pSym->iShapeCount >= AF_DOC_MAX_SHAPE_RECORDS ) return -3;

	pSym->arrShapes[pSym->iShapeCount] = *pShape;
	if ( pIndex ) *pIndex = pSym->iShapeCount;
	pSym->iShapeCount++;
	return 0;
}

int afSymbolRemoveShape(af_doc pDoc, uint32_t iSymbolId, int iShapeIndex)
{
	af_symbol_t* pSym = afDocFindSymbol(pDoc, iSymbolId);
	int i;

	if ( pSym == NULL ) return -1;
	if ( iShapeIndex < 0 || iShapeIndex >= pSym->iShapeCount ) return -2;

	for ( i = iShapeIndex; i < pSym->iShapeCount - 1; i++ )
		pSym->arrShapes[i] = pSym->arrShapes[i + 1];
	pSym->iShapeCount--;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Bounds calculation                                                 */
/* ------------------------------------------------------------------ */

static void __afPathBounds(const af_path_t* pPath, float* pMinX, float* pMinY, float* pMaxX, float* pMaxY)
{
	int i;
	if ( pPath->iPointCount == 0 ) return;

	for ( i = 0; i < pPath->iPointCount; i++ ) {
		float x = pPath->arrPoints[i * 2];
		float y = pPath->arrPoints[i * 2 + 1];
		if ( x < *pMinX ) *pMinX = x;
		if ( y < *pMinY ) *pMinY = y;
		if ( x > *pMaxX ) *pMaxX = x;
		if ( y > *pMaxY ) *pMaxY = y;
	}
}

int afSymbolRecalcBounds(af_doc pDoc, uint32_t iSymbolId)
{
	af_symbol_t* pSym = afDocFindSymbol(pDoc, iSymbolId);
	float minX = 1e30f, minY = 1e30f, maxX = -1e30f, maxY = -1e30f;
	int i;

	if ( pSym == NULL ) return -1;

	if ( pSym->iType == AF_SYMBOL_GRAPHIC ) {
		for ( i = 0; i < pSym->iShapeCount; i++ ) {
			__afPathBounds(&pSym->arrShapes[i].tPath, &minX, &minY, &maxX, &maxY);
		}
	} else if ( pSym->iType == AF_SYMBOL_SPRITE ) {
		af_resource_t* pRes = afDocFindResource(pDoc, pSym->iResourceId);
		if ( pRes ) {
			minX = 0; minY = 0;
			maxX = (float)pRes->iWidth;
			maxY = (float)pRes->iHeight;
		}
	} else if ( pSym->iType == AF_SYMBOL_GROUP || pSym->iType == AF_SYMBOL_MOVIE_CLIP ) {
		for ( i = 0; i < pSym->iChildCount; i++ ) {
			af_symbol_child_t* pChild = &pSym->arrChildren[i];
			af_symbol_t* pChildSym = afDocFindSymbol(pDoc, pChild->iSymbolId);
			if ( pChildSym ) {
				float cx = pChild->fTx + pChildSym->fBoundsX;
				float cy = pChild->fTy + pChildSym->fBoundsY;
				float cw = pChildSym->fBoundsW * pChild->fScaleX;
				float ch = pChildSym->fBoundsH * pChild->fScaleY;
				if ( cx < minX ) minX = cx;
				if ( cy < minY ) minY = cy;
				if ( cx + cw > maxX ) maxX = cx + cw;
				if ( cy + ch > maxY ) maxY = cy + ch;
			}
		}
	}

	if ( minX > maxX ) { minX = 0; maxX = 0; }
	if ( minY > maxY ) { minY = 0; maxY = 0; }

	pSym->fBoundsX = minX;
	pSym->fBoundsY = minY;
	pSym->fBoundsW = maxX - minX;
	pSym->fBoundsH = maxY - minY;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Instance placement on stage                                        */
/* ------------------------------------------------------------------ */

int afSymbolInstantiate(af_doc pDoc, uint32_t iSymbolId, uint32_t iLayerId,
                        uint32_t iFrame, float fX, float fY, uint32_t* pInstanceId)
{
	uint32_t instId;
	af_element_t* pElem;
	int ret;

	ret = afDocAddElement(pDoc, iLayerId, iFrame, iSymbolId, &instId);
	if ( ret != 0 ) return ret;

	pElem = afDocFindElement(pDoc, instId);
	if ( pElem ) {
		pElem->fTx = fX;
		pElem->fTy = fY;
		pElem->fScaleX = 1.0f;
		pElem->fScaleY = 1.0f;
		pElem->fOpacity = 1.0f;
	}

	if ( pInstanceId ) *pInstanceId = instId;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Symbol duplication                                                 */
/* ------------------------------------------------------------------ */

int afSymbolDuplicate(af_doc pDoc, uint32_t iSrcId, const char* sNewName, uint32_t* pNewId)
{
	af_symbol_t* pSrc = afDocFindSymbol(pDoc, iSrcId);
	uint32_t newId;
	af_symbol_t* pDst;
	int ret;

	if ( pSrc == NULL ) return -1;

	ret = afDocAddSymbol(pDoc, pSrc->iType, sNewName, &newId);
	if ( ret != 0 ) return ret;

	pDst = afDocFindSymbol(pDoc, newId);
	if ( pDst == NULL ) return -1;

	/* Copy all fields except id and name */
	pDst->iType = pSrc->iType;
	pDst->fBoundsX = pSrc->fBoundsX;
	pDst->fBoundsY = pSrc->fBoundsY;
	pDst->fBoundsW = pSrc->fBoundsW;
	pDst->fBoundsH = pSrc->fBoundsH;
	pDst->iShapeCount = pSrc->iShapeCount;
	memcpy(pDst->arrShapes, pSrc->arrShapes, sizeof(af_shape_record_t) * pSrc->iShapeCount);
	pDst->iResourceId = pSrc->iResourceId;
	pDst->fNineX1 = pSrc->fNineX1;
	pDst->fNineY1 = pSrc->fNineY1;
	pDst->fNineX2 = pSrc->fNineX2;
	pDst->fNineY2 = pSrc->fNineY2;
	pDst->iChildCount = pSrc->iChildCount;
	memcpy(pDst->arrChildren, pSrc->arrChildren, sizeof(af_symbol_child_t) * pSrc->iChildCount);
	pDst->iTimelineIndex = pSrc->iTimelineIndex;

	if ( pNewId ) *pNewId = newId;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Convert selection to Symbol                                        */
/* ------------------------------------------------------------------ */

int afSymbolConvertToSymbol(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame,
                            int iType, const char* sName, uint32_t* pSymbolId)
{
	af_keyframe_t* pKf;
	uint32_t symId;
	af_symbol_t* pSym;
	int ret, i;

	pKf = afDocGetKeyframe(pDoc, iLayerId, iFrame);
	if ( pKf == NULL || pKf->iElementCount == 0 ) return -1;

	ret = afDocAddSymbol(pDoc, iType, sName, &symId);
	if ( ret != 0 ) return ret;

	pSym = afDocFindSymbol(pDoc, symId);
	if ( pSym == NULL ) return -1;

	/* Move elements into symbol as children (for GROUP/MOVIE_CLIP) */
	if ( iType == AF_SYMBOL_GROUP || iType == AF_SYMBOL_MOVIE_CLIP ) {
		for ( i = 0; i < pKf->iElementCount && pSym->iChildCount < AF_DOC_MAX_CHILDREN; i++ ) {
			af_element_t* pElem = &pKf->arrElements[i];
			af_symbol_child_t* pChild = &pSym->arrChildren[pSym->iChildCount];
			pChild->iSymbolId = pElem->iSymbolId;
			pChild->fTx = pElem->fTx;
			pChild->fTy = pElem->fTy;
			pChild->fScaleX = pElem->fScaleX;
			pChild->fScaleY = pElem->fScaleY;
			pChild->fRotation = pElem->fRotation;
			pChild->fOpacity = pElem->fOpacity;
			pChild->iDepth = pElem->iDepth;
			pSym->iChildCount++;
		}
		/* Replace all elements with a single instance of the new symbol */
		pKf->iElementCount = 1;
		memset(&pKf->arrElements[0], 0, sizeof(af_element_t));
		pKf->arrElements[0].iSymbolId = symId;
		pKf->arrElements[0].iInstanceId = pDoc->iNextInstanceId++;
		pKf->arrElements[0].fScaleX = 1.0f;
		pKf->arrElements[0].fScaleY = 1.0f;
		pKf->arrElements[0].fOpacity = 1.0f;
		pKf->arrElements[0].iTweenId = 0xFFFFFFFFu;
		pKf->arrElements[0].iFlags = AF_ELEM_FLAG_VISIBLE;
	}

	afSymbolRecalcBounds(pDoc, symId);
	if ( pSymbolId ) *pSymbolId = symId;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Query helpers                                                      */
/* ------------------------------------------------------------------ */

int afSymbolGetUsageCount(af_doc pDoc, uint32_t iSymbolId)
{
	int count = 0;
	int li, ki, ei;

	for ( li = 0; li < pDoc->tTimeline.iLayerCount; li++ ) {
		af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[li];
		for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
			af_keyframe_t* pKf = &pLayer->arrKeyframes[ki];
			for ( ei = 0; ei < pKf->iElementCount; ei++ ) {
				if ( pKf->arrElements[ei].iSymbolId == iSymbolId )
					count++;
			}
		}
	}
	return count;
}

const char* afSymbolTypeName(int iType)
{
	switch ( iType ) {
	case AF_SYMBOL_GRAPHIC:    return "Graphic";
	case AF_SYMBOL_SPRITE:     return "Sprite";
	case AF_SYMBOL_GROUP:      return "Group";
	case AF_SYMBOL_MOVIE_CLIP: return "MovieClip";
	default:                   return "Unknown";
	}
}
