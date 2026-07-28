/*
 * xanim_symbol.c - Symbol instantiation and caching
 *
 * Manages runtime symbol instances: creation from parsed data,
 * instance tree building, and render caching for static symbols.
 */

#include "xanim.h"
#include "xge.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Symbol instance                                                    */
/* ------------------------------------------------------------------ */

typedef struct xanim_symbol_instance_t {
	uint32_t iSymbolId;
	uint32_t iInstanceId;
	uint32_t iParentInstance;
	float    fTx, fTy;
	float    fScaleX, fScaleY;
	float    fRotation;
	float    fOpacity;
	int      bVisible;
	int      iChildCount;
	struct xanim_symbol_instance_t** ppChildren;
} xanim_symbol_instance_t;

/* ------------------------------------------------------------------ */
/* Instance pool                                                      */
/* ------------------------------------------------------------------ */

#define XANIM_MAX_INSTANCES  4096

typedef struct xanim_instance_pool_t {
	xanim_symbol_instance_t arrInstances[XANIM_MAX_INSTANCES];
	int iCount;
	uint32_t iNextId;
} xanim_instance_pool_t;

static xanim_instance_pool_t s_tPool;

void xanimSymbolInitPool(void)
{
	memset(&s_tPool, 0, sizeof(s_tPool));
	s_tPool.iNextId = 1;
}

void xanimSymbolClearPool(void)
{
	int i;
	for ( i = 0; i < s_tPool.iCount; i++ ) {
		if ( s_tPool.arrInstances[i].ppChildren ) {
			free(s_tPool.arrInstances[i].ppChildren);
			s_tPool.arrInstances[i].ppChildren = NULL;
		}
	}
	s_tPool.iCount = 0;
}

/* ------------------------------------------------------------------ */
/* Instance creation                                                  */
/* ------------------------------------------------------------------ */

xanim_symbol_instance_t* xanimSymbolCreateInstance(uint32_t iSymbolId, uint32_t iParent)
{
	xanim_symbol_instance_t* pInst;

	if ( s_tPool.iCount >= XANIM_MAX_INSTANCES )
		return NULL;

	pInst = &s_tPool.arrInstances[s_tPool.iCount];
	memset(pInst, 0, sizeof(*pInst));

	pInst->iSymbolId = iSymbolId;
	pInst->iInstanceId = s_tPool.iNextId++;
	pInst->iParentInstance = iParent;
	pInst->fScaleX = 1.0f;
	pInst->fScaleY = 1.0f;
	pInst->fOpacity = 1.0f;
	pInst->bVisible = 1;

	s_tPool.iCount++;
	return pInst;
}

xanim_symbol_instance_t* xanimSymbolFindInstance(uint32_t iInstanceId)
{
	int i;
	for ( i = 0; i < s_tPool.iCount; i++ ) {
		if ( s_tPool.arrInstances[i].iInstanceId == iInstanceId )
			return &s_tPool.arrInstances[i];
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Instance transform update                                          */
/* ------------------------------------------------------------------ */

void xanimSymbolSetInstanceTransform(xanim_symbol_instance_t* pInst,
                                     float fTx, float fTy,
                                     float fScaleX, float fScaleY,
                                     float fRotation, float fOpacity)
{
	if ( pInst == NULL ) return;
	pInst->fTx = fTx;
	pInst->fTy = fTy;
	pInst->fScaleX = fScaleX;
	pInst->fScaleY = fScaleY;
	pInst->fRotation = fRotation;
	pInst->fOpacity = fOpacity;
}

void xanimSymbolSetInstanceVisible(xanim_symbol_instance_t* pInst, int bVisible)
{
	if ( pInst ) pInst->bVisible = bVisible;
}

/* ------------------------------------------------------------------ */
/* Render cache for static GRAPHIC symbols                            */
/* ------------------------------------------------------------------ */

typedef struct xanim_render_cache_t {
	uint32_t iSymbolId;
	int      bValid;
	/* In full implementation: RenderTarget / texture handle */
	uint32_t iTextureHandle;
	int      iWidth, iHeight;
} xanim_render_cache_t;

#define XANIM_MAX_CACHE  256

static xanim_render_cache_t s_arrCache[XANIM_MAX_CACHE];
static int s_iCacheCount = 0;

void xanimSymbolClearCache(void)
{
	s_iCacheCount = 0;
	memset(s_arrCache, 0, sizeof(s_arrCache));
}

xanim_render_cache_t* xanimSymbolGetCache(uint32_t iSymbolId)
{
	int i;
	for ( i = 0; i < s_iCacheCount; i++ ) {
		if ( s_arrCache[i].iSymbolId == iSymbolId && s_arrCache[i].bValid )
			return &s_arrCache[i];
	}
	return NULL;
}

int xanimSymbolCreateCache(uint32_t iSymbolId, int iW, int iH)
{
	if ( s_iCacheCount >= XANIM_MAX_CACHE )
		return -1;

	s_arrCache[s_iCacheCount].iSymbolId = iSymbolId;
	s_arrCache[s_iCacheCount].bValid = 1;
	s_arrCache[s_iCacheCount].iWidth = iW;
	s_arrCache[s_iCacheCount].iHeight = iH;
	s_arrCache[s_iCacheCount].iTextureHandle = 0; /* placeholder */
	s_iCacheCount++;
	return 0;
}

void xanimSymbolInvalidateCache(uint32_t iSymbolId)
{
	int i;
	for ( i = 0; i < s_iCacheCount; i++ ) {
		if ( s_arrCache[i].iSymbolId == iSymbolId )
			s_arrCache[i].bValid = 0;
	}
}
