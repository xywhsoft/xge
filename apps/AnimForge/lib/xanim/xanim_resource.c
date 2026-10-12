/*
 * xanim_resource.c - Texture decode and upload
 *
 * Manages embedded resources (textures, fonts, audio) from
 * the .xanim Resource section. Handles decode and GPU upload.
 */

#include "xanim.h"
#include "xge.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Resource entry (runtime)                                           */
/* ------------------------------------------------------------------ */

typedef struct xanim_rt_resource_t {
	uint32_t iId;
	uint32_t iType;       /* XANIM_RES_TEXTURE / FONT / AUDIO */
	uint32_t iFormat;     /* pixel format */
	uint16_t iWidth;
	uint16_t iHeight;
	uint32_t iDataSize;
	void*    pData;       /* decoded pixel/audio data (owned) */
	uint32_t iGpuHandle;  /* GPU texture handle (0 = not uploaded) */
	int      bUploaded;
} xanim_rt_resource_t;

#define XANIM_MAX_RESOURCES  256

static xanim_rt_resource_t s_arrResources[XANIM_MAX_RESOURCES];
static int s_iResourceCount = 0;

/* ------------------------------------------------------------------ */
/* Init / Clear                                                       */
/* ------------------------------------------------------------------ */

void xanimResourceInit(void)
{
	memset(s_arrResources, 0, sizeof(s_arrResources));
	s_iResourceCount = 0;
}

void xanimResourceClear(void)
{
	int i;
	for ( i = 0; i < s_iResourceCount; i++ ) {
		if ( s_arrResources[i].pData ) {
			free(s_arrResources[i].pData);
			s_arrResources[i].pData = NULL;
		}
		/* GPU texture cleanup would go here */
	}
	s_iResourceCount = 0;
}

/* ------------------------------------------------------------------ */
/* Resource registration from parsed section                          */
/* ------------------------------------------------------------------ */

int xanimResourceAdd(uint32_t iId, uint32_t iType, uint32_t iFormat,
                     uint16_t iW, uint16_t iH,
                     const void* pData, uint32_t iDataSize)
{
	xanim_rt_resource_t* pRes;

	if ( s_iResourceCount >= XANIM_MAX_RESOURCES )
		return -1;

	pRes = &s_arrResources[s_iResourceCount];
	memset(pRes, 0, sizeof(*pRes));

	pRes->iId = iId;
	pRes->iType = iType;
	pRes->iFormat = iFormat;
	pRes->iWidth = iW;
	pRes->iHeight = iH;
	pRes->iDataSize = iDataSize;

	if ( pData && iDataSize > 0 ) {
		pRes->pData = malloc(iDataSize);
		if ( pRes->pData == NULL )
			return -2;
		memcpy(pRes->pData, pData, iDataSize);
	}

	s_iResourceCount++;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Resource lookup                                                    */
/* ------------------------------------------------------------------ */

xanim_rt_resource_t* xanimResourceFind(uint32_t iId)
{
	int i;
	for ( i = 0; i < s_iResourceCount; i++ ) {
		if ( s_arrResources[i].iId == iId )
			return &s_arrResources[i];
	}
	return NULL;
}

int xanimResourceGetCount(void)
{
	return s_iResourceCount;
}

/* ------------------------------------------------------------------ */
/* GPU upload                                                         */
/* ------------------------------------------------------------------ */

int xanimResourceUpload(uint32_t iId)
{
	xanim_rt_resource_t* pRes = xanimResourceFind(iId);

	if ( pRes == NULL )
		return -1;

	if ( pRes->bUploaded )
		return 0; /* already uploaded */

	if ( pRes->iType != XANIM_RES_TEXTURE )
		return -2; /* only textures get GPU upload */

	if ( pRes->pData == NULL || pRes->iDataSize == 0 )
		return -3;

	/*
	 * Full implementation would call:
	 * xgeTextureCreateFromMemory(pRes->pData, pRes->iWidth, pRes->iHeight,
	 *                            XGE_TEX_RGBA8, &pRes->iGpuHandle);
	 * For now, mark as uploaded with placeholder handle.
	 */
	pRes->iGpuHandle = iId; /* placeholder */
	pRes->bUploaded = 1;

	return 0;
}

int xanimResourceUploadAll(void)
{
	int i, failures = 0;
	for ( i = 0; i < s_iResourceCount; i++ ) {
		if ( s_arrResources[i].iType == XANIM_RES_TEXTURE && !s_arrResources[i].bUploaded ) {
			if ( xanimResourceUpload(s_arrResources[i].iId) != 0 )
				failures++;
		}
	}
	return failures;
}

/* ------------------------------------------------------------------ */
/* Texture access for rendering                                       */
/* ------------------------------------------------------------------ */

uint32_t xanimResourceGetGpuHandle(uint32_t iId)
{
	xanim_rt_resource_t* pRes = xanimResourceFind(iId);
	if ( pRes == NULL || !pRes->bUploaded )
		return 0;
	return pRes->iGpuHandle;
}

void* xanimResourceGetData(uint32_t iId, uint32_t* pSize)
{
	xanim_rt_resource_t* pRes = xanimResourceFind(iId);
	if ( pRes == NULL )
		return NULL;
	if ( pSize ) *pSize = pRes->iDataSize;
	return pRes->pData;
}
