/*
 * af_file.c - AnimForge .xanim file serialization implementation
 *
 * Buffer-based writer/reader for the .xanim binary format.
 * Serializes af_doc_t <-> xanim on-disk structures.
 */

#include "af_file.h"
#include "xanim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ------------------------------------------------------------------ */
/* Growable write buffer                                              */
/* ------------------------------------------------------------------ */

typedef struct af_wbuf_t {
	uint8_t* pData;
	uint32_t iSize;
	uint32_t iCapacity;
} af_wbuf_t;

static int __wbufInit(af_wbuf_t* pBuf, uint32_t iInitial)
{
	pBuf->pData = (uint8_t*)malloc(iInitial);
	if ( pBuf->pData == NULL ) return -1;
	pBuf->iSize = 0;
	pBuf->iCapacity = iInitial;
	return 0;
}

static void __wbufFree(af_wbuf_t* pBuf)
{
	if ( pBuf->pData != NULL ) { free(pBuf->pData); pBuf->pData = NULL; }
	pBuf->iSize = 0;
	pBuf->iCapacity = 0;
}

static int __wbufEnsure(af_wbuf_t* pBuf, uint32_t iExtra)
{
	uint32_t iNeeded = pBuf->iSize + iExtra;
	if ( iNeeded <= pBuf->iCapacity ) return 0;
	while ( pBuf->iCapacity < iNeeded ) pBuf->iCapacity *= 2;
	uint8_t* pNew = (uint8_t*)realloc(pBuf->pData, pBuf->iCapacity);
	if ( pNew == NULL ) return -1;
	pBuf->pData = pNew;
	return 0;
}

static int __wbufWrite(af_wbuf_t* pBuf, const void* pData, uint32_t iSize)
{
	if ( __wbufEnsure(pBuf, iSize) != 0 ) return -1;
	memcpy(pBuf->pData + pBuf->iSize, pData, iSize);
	pBuf->iSize += iSize;
	return 0;
}

static int __wbufWriteU32(af_wbuf_t* pBuf, uint32_t v)
{
	return __wbufWrite(pBuf, &v, 4);
}

static int __wbufWriteF32(af_wbuf_t* pBuf, float v)
{
	return __wbufWrite(pBuf, &v, 4);
}

static uint32_t __wbufTell(af_wbuf_t* pBuf)
{
	return pBuf->iSize;
}

static void __wbufPad(af_wbuf_t* pBuf, uint32_t iAlign)
{
	uint32_t iRem = pBuf->iSize % iAlign;
	if ( iRem != 0 ) {
		uint32_t iPad = iAlign - iRem;
		uint8_t zero = 0;
		uint32_t i;
		for ( i = 0; i < iPad; i++ ) __wbufWrite(pBuf, &zero, 1);
	}
}

/* Patch a uint32 at a given offset */
static void __wbufPatchU32(af_wbuf_t* pBuf, uint32_t iOffset, uint32_t v)
{
	if ( iOffset + 4 <= pBuf->iSize ) {
		memcpy(pBuf->pData + iOffset, &v, 4);
	}
}

/* ------------------------------------------------------------------ */
/* String table builder                                               */
/* ------------------------------------------------------------------ */

#define AF_STRTAB_MAX 4096

typedef struct af_strtab_t {
	char*    arrStrings[AF_STRTAB_MAX];
	int      iCount;
} af_strtab_t;

static void __strtabInit(af_strtab_t* pTab)
{
	pTab->iCount = 0;
}

static void __strtabFree(af_strtab_t* pTab)
{
	int i;
	for ( i = 0; i < pTab->iCount; i++ ) {
		if ( pTab->arrStrings[i] != NULL ) free(pTab->arrStrings[i]);
	}
	pTab->iCount = 0;
}

/* Returns string index. Adds if not found. */
static uint32_t __strtabAdd(af_strtab_t* pTab, const char* s)
{
	int i;
	if ( s == NULL || s[0] == '\0' ) return 0xFFFFFFFFu;
	for ( i = 0; i < pTab->iCount; i++ ) {
		if ( strcmp(pTab->arrStrings[i], s) == 0 ) return (uint32_t)i;
	}
	if ( pTab->iCount >= AF_STRTAB_MAX ) return 0xFFFFFFFFu;
	pTab->arrStrings[pTab->iCount] = strdup(s);
	return (uint32_t)(pTab->iCount++);
}

/* ------------------------------------------------------------------ */
/* CRC32 (simple table-based)                                         */
/* ------------------------------------------------------------------ */

static uint32_t __afCrc32(const void* pData, uint32_t iSize)
{
	static uint32_t sTable[256];
	static int sInit = 0;
	const uint8_t* p = (const uint8_t*)pData;
	uint32_t crc = 0xFFFFFFFFu;
	uint32_t i;
	if ( !sInit ) {
		uint32_t j, c;
		for ( i = 0; i < 256; i++ ) {
			c = i;
			for ( j = 0; j < 8; j++ ) {
				c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
			}
			sTable[i] = c;
		}
		sInit = 1;
	}
	for ( i = 0; i < iSize; i++ ) {
		crc = sTable[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
	}
	return crc ^ 0xFFFFFFFFu;
}

/* ------------------------------------------------------------------ */
/* Save: serialize af_doc_t to .xanim                                  */
/* ------------------------------------------------------------------ */

int afFileSave(af_doc pDoc, const char* sPath)
{
	af_wbuf_t buf;
	af_strtab_t strtab;
	xanim_header_t header;
	xanim_toc_entry_t toc[7];
	uint32_t iTocOffset;
	uint32_t iSectionOffsets[7];
	uint32_t iSectionSizes[7];
	FILE* pFile;
	int i, j, k;

	if ( pDoc == NULL || sPath == NULL ) return -1;

	if ( __wbufInit(&buf, 1024 * 1024) != 0 ) return -1;
	__strtabInit(&strtab);

	/* Build string table from document names */
	for ( i = 0; i < pDoc->iSymbolCount; i++ )
		__strtabAdd(&strtab, pDoc->arrSymbols[i].sName);
	for ( i = 0; i < pDoc->iResourceCount; i++ )
		__strtabAdd(&strtab, pDoc->arrResources[i].sName);
	for ( i = 0; i < pDoc->tTimeline.iLayerCount; i++ ) {
		af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[i];
		__strtabAdd(&strtab, pLayer->sName);
		for ( j = 0; j < pLayer->iKeyframeCount; j++ ) {
			if ( pLayer->arrKeyframes[j].sLabel[0] != '\0' )
				__strtabAdd(&strtab, pLayer->arrKeyframes[j].sLabel);
		}
	}
	for ( i = 0; i < pDoc->iActionCount; i++ )
		__strtabAdd(&strtab, pDoc->arrActions[i].sName);

	/* Reserve space for header + TOC (will patch later) */
	memset(&header, 0, sizeof(header));
	__wbufWrite(&buf, &header, XANIM_HEADER_SIZE);
	iTocOffset = __wbufTell(&buf);
	memset(toc, 0, sizeof(toc));
	__wbufWrite(&buf, toc, sizeof(toc)); /* 7 entries placeholder */

	/* --- Section 1: Meta --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[0] = __wbufTell(&buf);
	{
		xanim_meta_t meta;
		memset(&meta, 0, sizeof(meta));
		meta.iAuthorIndex = 0xFFFFFFFFu;
		meta.iDescriptionIndex = 0xFFFFFFFFu;
		meta.iBackgroundColor = pDoc->iBackgroundColor;
		__wbufWrite(&buf, &meta, sizeof(meta));
	}
	iSectionSizes[0] = __wbufTell(&buf) - iSectionOffsets[0];

	/* --- Section 2: Resource --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[1] = __wbufTell(&buf);
	{
		uint32_t iCount = (uint32_t)pDoc->iResourceCount;
		__wbufWriteU32(&buf, iCount);
		for ( i = 0; i < pDoc->iResourceCount; i++ ) {
			af_resource_t* pRes = &pDoc->arrResources[i];
			xanim_resource_entry_t entry;
			memset(&entry, 0, sizeof(entry));
			entry.iId = pRes->iId;
			entry.iType = (uint32_t)pRes->iType;
			entry.iFormat = (uint32_t)pRes->iFormat;
			entry.iWidth = (uint16_t)pRes->iWidth;
			entry.iHeight = (uint16_t)pRes->iHeight;
			entry.iDataOffset = 0; /* data follows entries */
			entry.iDataSize = (uint32_t)pRes->iDataSize;
			entry.iNameIndex = __strtabAdd(&strtab, pRes->sName);
			__wbufWrite(&buf, &entry, sizeof(entry));
		}
		/* Write resource data blobs */
		for ( i = 0; i < pDoc->iResourceCount; i++ ) {
			af_resource_t* pRes = &pDoc->arrResources[i];
			if ( pRes->pData != NULL && pRes->iDataSize > 0 ) {
				__wbufWrite(&buf, pRes->pData, (uint32_t)pRes->iDataSize);
			}
		}
	}
	iSectionSizes[1] = __wbufTell(&buf) - iSectionOffsets[1];

	/* --- Section 3: Symbol --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[2] = __wbufTell(&buf);
	{
		uint32_t iCount = (uint32_t)pDoc->iSymbolCount;
		__wbufWriteU32(&buf, iCount);
		for ( i = 0; i < pDoc->iSymbolCount; i++ ) {
			af_symbol_t* pSym = &pDoc->arrSymbols[i];
			xanim_symbol_t xs;
			memset(&xs, 0, sizeof(xs));
			xs.iId = pSym->iId;
			xs.iType = (uint32_t)pSym->iType;
			xs.iNameIndex = __strtabAdd(&strtab, pSym->sName);
			xs.fBoundsX = pSym->fBoundsX;
			xs.fBoundsY = pSym->fBoundsY;
			xs.fBoundsW = pSym->fBoundsW;
			xs.fBoundsH = pSym->fBoundsH;
			xs.iChildCount = (uint32_t)pSym->iChildCount;
			xs.iChildOffset = 0;
			xs.iTimelineIndex = (pSym->iTimelineIndex >= 0) ? (uint32_t)pSym->iTimelineIndex : 0xFFFFFFFFu;
			xs.iDataOffset = 0;
			xs.iDataSize = 0;
			__wbufWrite(&buf, &xs, sizeof(xs));

			/* Write children */
			for ( j = 0; j < pSym->iChildCount; j++ ) {
				af_symbol_child_t* pChild = &pSym->arrChildren[j];
				xanim_symbol_child_t xc;
				xc.iSymbolId = pChild->iSymbolId;
				xc.fTx = pChild->fTx;
				xc.fTy = pChild->fTy;
				xc.fScaleX = pChild->fScaleX;
				xc.fScaleY = pChild->fScaleY;
				xc.fRotation = pChild->fRotation;
				xc.fOpacity = pChild->fOpacity;
				xc.iDepth = pChild->iDepth;
				__wbufWrite(&buf, &xc, sizeof(xc));
			}

			/* Write type-specific payload */
			if ( pSym->iType == AF_SYMBOL_SPRITE ) {
				xanim_sprite_data_t sd;
				sd.iResourceId = pSym->iResourceId;
				sd.fNineX1 = pSym->fNineX1;
				sd.fNineY1 = pSym->fNineY1;
				sd.fNineX2 = pSym->fNineX2;
				sd.fNineY2 = pSym->fNineY2;
				sd.iFilter = XANIM_FILTER_LINEAR;
				__wbufWrite(&buf, &sd, sizeof(sd));
			} else if ( pSym->iType == AF_SYMBOL_GRAPHIC ) {
				xanim_graphic_data_t gd;
				gd.iShapeCount = (uint32_t)pSym->iShapeCount;
				__wbufWrite(&buf, &gd, sizeof(gd));
				for ( j = 0; j < pSym->iShapeCount; j++ ) {
					af_shape_record_t* pShape = &pSym->arrShapes[j];
					xanim_shape_record_t sr;
					/* Serialize path to command stream */
					uint32_t iPathBytes = 0;
					int iPtIdx = 0;
					/* Calculate path size */
					for ( k = 0; k < pShape->tPath.iCommandCount; k++ ) {
						uint8_t cmd = pShape->tPath.arrCommands[k];
						iPathBytes += 1; /* opcode */
						if ( cmd == AF_PATH_CMD_MOVE_TO || cmd == AF_PATH_CMD_LINE_TO ) iPathBytes += 8;
						else if ( cmd == AF_PATH_CMD_QUAD_TO ) iPathBytes += 16;
						else if ( cmd == AF_PATH_CMD_CUBIC_TO ) iPathBytes += 24;
					}
					memset(&sr, 0, sizeof(sr));
					sr.iPathSize = iPathBytes;
					sr.iFillType = (uint8_t)pShape->tFill.iType;
					sr.iStrokeType = (uint8_t)pShape->tStroke.iType;
					sr.iFillRule = (uint8_t)pShape->iFillRule;
					sr.fStrokeWidth = pShape->fStrokeWidth;
					sr.iLineCap = (uint32_t)pShape->iLineCap;
					sr.iLineJoin = (uint32_t)pShape->iLineJoin;
					sr.fMiterLimit = pShape->fMiterLimit;
					__wbufWrite(&buf, &sr, sizeof(sr));
					/* Write path commands */
					iPtIdx = 0;
					for ( k = 0; k < pShape->tPath.iCommandCount; k++ ) {
						uint8_t cmd = pShape->tPath.arrCommands[k];
						__wbufWrite(&buf, &cmd, 1);
						if ( cmd == AF_PATH_CMD_MOVE_TO || cmd == AF_PATH_CMD_LINE_TO ) {
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[iPtIdx * 2]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[iPtIdx * 2 + 1]);
							iPtIdx++;
						} else if ( cmd == AF_PATH_CMD_QUAD_TO ) {
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[iPtIdx * 2]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[iPtIdx * 2 + 1]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[(iPtIdx + 1) * 2]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[(iPtIdx + 1) * 2 + 1]);
							iPtIdx += 2;
						} else if ( cmd == AF_PATH_CMD_CUBIC_TO ) {
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[iPtIdx * 2]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[iPtIdx * 2 + 1]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[(iPtIdx + 1) * 2]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[(iPtIdx + 1) * 2 + 1]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[(iPtIdx + 2) * 2]);
							__wbufWriteF32(&buf, pShape->tPath.arrPoints[(iPtIdx + 2) * 2 + 1]);
							iPtIdx += 3;
						}
					}
					/* Write fill paint */
					if ( pShape->tFill.iType == AF_PAINT_SOLID ) {
						__wbufWriteU32(&buf, pShape->tFill.iColor);
					} else if ( pShape->tFill.iType == AF_PAINT_LINEAR || pShape->tFill.iType == AF_PAINT_RADIAL ) {
						xanim_gradient_t grad;
						grad.fX1 = pShape->tFill.fX1;
						grad.fY1 = pShape->tFill.fY1;
						grad.fX2 = pShape->tFill.fX2;
						grad.fY2 = pShape->tFill.fY2;
						grad.fRadius = pShape->tFill.fRadius;
						grad.iStopCount = (uint32_t)pShape->tFill.iStopCount;
						__wbufWrite(&buf, &grad, sizeof(grad));
						for ( k = 0; k < pShape->tFill.iStopCount; k++ ) {
							xanim_gradient_stop_t stop;
							stop.fOffset = pShape->tFill.arrStops[k].fOffset;
							stop.iColor = pShape->tFill.arrStops[k].iColor;
							__wbufWrite(&buf, &stop, sizeof(stop));
						}
					}
					/* Write stroke paint */
					if ( pShape->tStroke.iType == AF_PAINT_SOLID ) {
						__wbufWriteU32(&buf, pShape->tStroke.iColor);
					} else if ( pShape->tStroke.iType == AF_PAINT_LINEAR || pShape->tStroke.iType == AF_PAINT_RADIAL ) {
						xanim_gradient_t grad;
						grad.fX1 = pShape->tStroke.fX1;
						grad.fY1 = pShape->tStroke.fY1;
						grad.fX2 = pShape->tStroke.fX2;
						grad.fY2 = pShape->tStroke.fY2;
						grad.fRadius = pShape->tStroke.fRadius;
						grad.iStopCount = (uint32_t)pShape->tStroke.iStopCount;
						__wbufWrite(&buf, &grad, sizeof(grad));
						for ( k = 0; k < pShape->tStroke.iStopCount; k++ ) {
							xanim_gradient_stop_t stop;
							stop.fOffset = pShape->tStroke.arrStops[k].fOffset;
							stop.iColor = pShape->tStroke.arrStops[k].iColor;
							__wbufWrite(&buf, &stop, sizeof(stop));
						}
					}
				}
			}
		}
	}
	iSectionSizes[2] = __wbufTell(&buf) - iSectionOffsets[2];

	/* --- Section 4: Timeline --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[3] = __wbufTell(&buf);
	{
		uint32_t iLayerCount = (uint32_t)pDoc->tTimeline.iLayerCount;
		__wbufWriteU32(&buf, iLayerCount);
		for ( i = 0; i < pDoc->tTimeline.iLayerCount; i++ ) {
			af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[i];
			xanim_layer_t xl;
			memset(&xl, 0, sizeof(xl));
			xl.iId = pLayer->iId;
			xl.iNameIndex = __strtabAdd(&strtab, pLayer->sName);
			xl.iType = (uint32_t)pLayer->iType;
			xl.iParentLayer = pLayer->iParentLayer;
			xl.iKeyframeCount = (uint32_t)pLayer->iKeyframeCount;
			xl.iKeyframeOffset = 0;
			xl.iFlags = pLayer->iFlags;
			xl.iColor = pLayer->iColor;
			__wbufWrite(&buf, &xl, sizeof(xl));
			/* Write keyframes */
			for ( j = 0; j < pLayer->iKeyframeCount; j++ ) {
				af_keyframe_t* pKf = &pLayer->arrKeyframes[j];
				xanim_keyframe_t xk;
				memset(&xk, 0, sizeof(xk));
				xk.iFrame = pKf->iFrame;
				xk.iElementCount = (uint32_t)pKf->iElementCount;
				xk.iElementOffset = 0;
				xk.iLabelIndex = __strtabAdd(&strtab, pKf->sLabel);
				xk.iFlags = pKf->iFlags;
				__wbufWrite(&buf, &xk, sizeof(xk));
				/* Write elements */
				for ( k = 0; k < pKf->iElementCount; k++ ) {
					af_element_t* pElem = &pKf->arrElements[k];
					xanim_element_t xe;
					memset(&xe, 0, sizeof(xe));
					xe.iSymbolId = pElem->iSymbolId;
					xe.iInstanceId = pElem->iInstanceId;
					xe.fTx = pElem->fTx;
					xe.fTy = pElem->fTy;
					xe.fScaleX = pElem->fScaleX;
					xe.fScaleY = pElem->fScaleY;
					xe.fRotation = pElem->fRotation;
					xe.fSkewX = pElem->fSkewX;
					xe.fSkewY = pElem->fSkewY;
					xe.fOpacity = pElem->fOpacity;
					xe.fPivotX = pElem->fPivotX;
					xe.fPivotY = pElem->fPivotY;
					xe.iColorTransform = pElem->iColorTransform;
					xe.iTweenId = pElem->iTweenId;
					xe.iBlendMode = pElem->iBlendMode;
					xe.iFlags = pElem->iFlags;
					xe.iDepth = pElem->iDepth;
					__wbufWrite(&buf, &xe, sizeof(xe));
				}
			}
		}
	}
	iSectionSizes[3] = __wbufTell(&buf) - iSectionOffsets[3];

	/* --- Section 5: Curves --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[4] = __wbufTell(&buf);
	{
		uint32_t iCount = (uint32_t)pDoc->iCurveCount;
		__wbufWriteU32(&buf, iCount);
		for ( i = 0; i < pDoc->iCurveCount; i++ ) {
			af_curve_t* pCurve = &pDoc->arrCurves[i];
			xanim_curve_t xc;
			xc.iId = pCurve->iId;
			xc.iElementId = pCurve->iElementId;
			xc.iChannelMask = pCurve->iChannelMask;
			xc.iKeyCount = (uint32_t)pCurve->iKeyCount;
			xc.iKeyOffset = 0;
			__wbufWrite(&buf, &xc, sizeof(xc));
			for ( j = 0; j < pCurve->iKeyCount; j++ ) {
				af_curve_key_t* pKey = &pCurve->arrKeys[j];
				xanim_curve_key_t xk;
				xk.iFrame = pKey->iFrame;
				xk.iInterpType = pKey->iInterpType;
				memcpy(xk.arrValues, pKey->arrValues, sizeof(xk.arrValues));
				xk.fEaseIn = pKey->fEaseIn;
				xk.fEaseOut = pKey->fEaseOut;
				memcpy(xk.arrBezier, pKey->arrBezier, sizeof(xk.arrBezier));
				__wbufWrite(&buf, &xk, sizeof(xk));
			}
		}
	}
	iSectionSizes[4] = __wbufTell(&buf) - iSectionOffsets[4];

	/* --- Section 6: Actions --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[5] = __wbufTell(&buf);
	{
		uint32_t iCount = (uint32_t)pDoc->iActionCount;
		__wbufWriteU32(&buf, iCount);
		for ( i = 0; i < pDoc->iActionCount; i++ ) {
			af_action_t* pAct = &pDoc->arrActions[i];
			xanim_action_t xa;
			xa.iFrame = pAct->iFrame;
			xa.iLayerId = pAct->iLayerId;
			xa.iType = pAct->iType;
			xa.iParam = pAct->iParam;
			xa.iNameIndex = __strtabAdd(&strtab, pAct->sName);
			__wbufWrite(&buf, &xa, sizeof(xa));
		}
	}
	iSectionSizes[5] = __wbufTell(&buf) - iSectionOffsets[5];

	/* --- Section 7: String Table --- */
	__wbufPad(&buf, 4);
	iSectionOffsets[6] = __wbufTell(&buf);
	{
		xanim_string_table_t st;
		uint32_t iDataStart;
		uint32_t iDataSize = 0;
		/* Calculate data size */
		for ( i = 0; i < strtab.iCount; i++ ) {
			iDataSize += (uint32_t)strlen(strtab.arrStrings[i]) + 1;
		}
		st.iCount = (uint32_t)strtab.iCount;
		st.iDataOffset = sizeof(xanim_string_table_t) + (uint32_t)strtab.iCount * 4;
		st.iDataSize = iDataSize;
		__wbufWrite(&buf, &st, sizeof(st));
		/* Write offsets */
		iDataStart = 0;
		for ( i = 0; i < strtab.iCount; i++ ) {
			__wbufWriteU32(&buf, iDataStart);
			iDataStart += (uint32_t)strlen(strtab.arrStrings[i]) + 1;
		}
		/* Write string data */
		for ( i = 0; i < strtab.iCount; i++ ) {
			__wbufWrite(&buf, strtab.arrStrings[i], (uint32_t)strlen(strtab.arrStrings[i]) + 1);
		}
	}
	iSectionSizes[6] = __wbufTell(&buf) - iSectionOffsets[6];

	/* --- Patch TOC --- */
	for ( i = 0; i < 7; i++ ) {
		toc[i].iSectionType = (uint32_t)(i + 1);
		toc[i].iOffset = iSectionOffsets[i];
		toc[i].iSize = iSectionSizes[i];
		toc[i].iReserved = 0;
	}
	memcpy(buf.pData + iTocOffset, toc, sizeof(toc));

	/* --- Patch Header --- */
	memset(&header, 0, sizeof(header));
	header.iMagic = XANIM_MAGIC;
	header.iVersion = (uint16_t)((XANIM_VERSION_MAJOR << 8) | XANIM_VERSION_MINOR);
	header.iHeaderSize = XANIM_HEADER_SIZE;
	header.iFlags = pDoc->iFlags;
	header.fFrameRate = pDoc->fFrameRate;
	header.iFrameCount = pDoc->iFrameCount;
	header.fStageWidth = pDoc->fStageWidth;
	header.fStageHeight = pDoc->fStageHeight;
	header.iSymbolCount = (uint32_t)pDoc->iSymbolCount;
	header.iLayerCount = (uint32_t)pDoc->tTimeline.iLayerCount;
	header.iResourceCount = (uint32_t)pDoc->iResourceCount;
	header.iTocOffset = iTocOffset;
	header.iTocCount = 7;
	header.iCrc32 = 0;
	memcpy(buf.pData, &header, XANIM_HEADER_SIZE);

	/* Compute CRC32 (with crc field zeroed) */
	header.iCrc32 = __afCrc32(buf.pData, buf.iSize);
	memcpy(buf.pData + offsetof(xanim_header_t, iCrc32), &header.iCrc32, 4);

	/* Write to file */
	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		__wbufFree(&buf);
		__strtabFree(&strtab);
		return -1;
	}
	fwrite(buf.pData, 1, buf.iSize, pFile);
	fclose(pFile);

	__wbufFree(&buf);
	__strtabFree(&strtab);
	return 0;
}

/* ------------------------------------------------------------------ */
/* Load: deserialize .xanim to af_doc_t                                */
/* ------------------------------------------------------------------ */

/* Read helper */
typedef struct af_rbuf_t {
	const uint8_t* pData;
	uint32_t iSize;
	uint32_t iPos;
} af_rbuf_t;

static int __rbufRead(af_rbuf_t* pBuf, void* pOut, uint32_t iSize)
{
	if ( pBuf->iPos + iSize > pBuf->iSize ) return -1;
	memcpy(pOut, pBuf->pData + pBuf->iPos, iSize);
	pBuf->iPos += iSize;
	return 0;
}

static uint32_t __rbufReadU32(af_rbuf_t* pBuf)
{
	uint32_t v = 0;
	__rbufRead(pBuf, &v, 4);
	return v;
}

static float __rbufReadF32(af_rbuf_t* pBuf)
{
	float v = 0.0f;
	__rbufRead(pBuf, &v, 4);
	return v;
}

static void __rbufSeek(af_rbuf_t* pBuf, uint32_t iPos)
{
	pBuf->iPos = iPos;
}

/* String table lookup */
typedef struct af_strtab_r_t {
	const char** arrStrings;
	int iCount;
} af_strtab_r_t;

static const char* __strtabGet(af_strtab_r_t* pTab, uint32_t iIndex)
{
	if ( iIndex == 0xFFFFFFFFu ) return "";
	if ( (int)iIndex >= pTab->iCount ) return "";
	return pTab->arrStrings[iIndex];
}

int afFileLoad(af_doc pDoc, const char* sPath)
{
	FILE* pFile;
	uint8_t* pData;
	long iFileSize;
	af_rbuf_t rbuf;
	xanim_header_t header;
	xanim_toc_entry_t toc[7];
	af_strtab_r_t strtab;
	uint32_t iStringSection = 0xFFFFFFFFu;
	int i, j, k;

	if ( pDoc == NULL || sPath == NULL ) return -1;

	/* Read entire file */
	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) return -1;
	fseek(pFile, 0, SEEK_END);
	iFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	if ( iFileSize < XANIM_HEADER_SIZE ) { fclose(pFile); return -1; }
	pData = (uint8_t*)malloc((size_t)iFileSize);
	if ( pData == NULL ) { fclose(pFile); return -1; }
	fread(pData, 1, (size_t)iFileSize, pFile);
	fclose(pFile);

	rbuf.pData = pData;
	rbuf.iSize = (uint32_t)iFileSize;
	rbuf.iPos = 0;

	/* Read header */
	if ( __rbufRead(&rbuf, &header, XANIM_HEADER_SIZE) != 0 ) { free(pData); return -1; }
	if ( header.iMagic != XANIM_MAGIC ) { free(pData); return -1; }

	/* Read TOC */
	__rbufSeek(&rbuf, header.iTocOffset);
	memset(toc, 0, sizeof(toc));
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		__rbufRead(&rbuf, &toc[i], sizeof(xanim_toc_entry_t));
		if ( toc[i].iSectionType == XANIM_SECTION_STRING )
			iStringSection = (uint32_t)i;
	}

	/* Load string table first */
	memset(&strtab, 0, sizeof(strtab));
	if ( iStringSection != 0xFFFFFFFFu ) {
		xanim_string_table_t st;
		uint32_t* pOffsets;
		const char* pCharData;
		__rbufSeek(&rbuf, toc[iStringSection].iOffset);
		__rbufRead(&rbuf, &st, sizeof(st));
		if ( st.iCount > 0 ) {
			strtab.iCount = (int)st.iCount;
			strtab.arrStrings = (const char**)calloc(st.iCount, sizeof(char*));
			pOffsets = (uint32_t*)(pData + rbuf.iPos);
			pCharData = (const char*)(pData + toc[iStringSection].iOffset + st.iDataOffset);
			for ( i = 0; i < (int)st.iCount; i++ ) {
				strtab.arrStrings[i] = pCharData + pOffsets[i];
			}
		}
	}

	/* Initialize document with header info */
	afDocClear(pDoc);
	afDocInit(pDoc);
	pDoc->fFrameRate = header.fFrameRate;
	pDoc->iFrameCount = header.iFrameCount;
	pDoc->fStageWidth = header.fStageWidth;
	pDoc->fStageHeight = header.fStageHeight;
	pDoc->iFlags = header.iFlags;

	/* Load Meta section */
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		if ( toc[i].iSectionType == XANIM_SECTION_META ) {
			xanim_meta_t meta;
			__rbufSeek(&rbuf, toc[i].iOffset);
			if ( __rbufRead(&rbuf, &meta, sizeof(meta)) == 0 ) {
				pDoc->iBackgroundColor = meta.iBackgroundColor;
			}
			break;
		}
	}

	/* Load Resources */
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		if ( toc[i].iSectionType == XANIM_SECTION_RESOURCE ) {
			uint32_t iCount;
			__rbufSeek(&rbuf, toc[i].iOffset);
			iCount = __rbufReadU32(&rbuf);
			for ( j = 0; j < (int)iCount && j < AF_DOC_MAX_RESOURCES; j++ ) {
				xanim_resource_entry_t entry;
				af_resource_t* pRes;
				__rbufRead(&rbuf, &entry, sizeof(entry));
				pRes = &pDoc->arrResources[pDoc->iResourceCount];
				memset(pRes, 0, sizeof(af_resource_t));
				pRes->iId = entry.iId;
				pRes->iType = (int)entry.iType;
				pRes->iFormat = (int)entry.iFormat;
				pRes->iWidth = entry.iWidth;
				pRes->iHeight = entry.iHeight;
				pRes->iDataSize = (int)entry.iDataSize;
				snprintf(pRes->sName, AF_DOC_MAX_NAME, "%s", __strtabGet(&strtab, entry.iNameIndex));
				pDoc->iResourceCount++;
				if ( entry.iId >= pDoc->iNextResourceId )
					pDoc->iNextResourceId = entry.iId + 1;
			}
			break;
		}
	}

	/* Load Symbols */
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		if ( toc[i].iSectionType == XANIM_SECTION_SYMBOL ) {
			uint32_t iCount;
			__rbufSeek(&rbuf, toc[i].iOffset);
			iCount = __rbufReadU32(&rbuf);
			for ( j = 0; j < (int)iCount && j < AF_DOC_MAX_SYMBOLS; j++ ) {
				xanim_symbol_t xs;
				af_symbol_t* pSym;
				__rbufRead(&rbuf, &xs, sizeof(xs));
				pSym = &pDoc->arrSymbols[pDoc->iSymbolCount];
				memset(pSym, 0, sizeof(af_symbol_t));
				pSym->iId = xs.iId;
				pSym->iType = (int)xs.iType;
				snprintf(pSym->sName, AF_DOC_MAX_NAME, "%s", __strtabGet(&strtab, xs.iNameIndex));
				pSym->fBoundsX = xs.fBoundsX;
				pSym->fBoundsY = xs.fBoundsY;
				pSym->fBoundsW = xs.fBoundsW;
				pSym->fBoundsH = xs.fBoundsH;
				pSym->iTimelineIndex = (xs.iTimelineIndex == 0xFFFFFFFFu) ? -1 : (int)xs.iTimelineIndex;
				/* Read children */
				pSym->iChildCount = (int)xs.iChildCount;
				for ( k = 0; k < (int)xs.iChildCount && k < AF_DOC_MAX_CHILDREN; k++ ) {
					xanim_symbol_child_t xc;
					__rbufRead(&rbuf, &xc, sizeof(xc));
					pSym->arrChildren[k].iSymbolId = xc.iSymbolId;
					pSym->arrChildren[k].fTx = xc.fTx;
					pSym->arrChildren[k].fTy = xc.fTy;
					pSym->arrChildren[k].fScaleX = xc.fScaleX;
					pSym->arrChildren[k].fScaleY = xc.fScaleY;
					pSym->arrChildren[k].fRotation = xc.fRotation;
					pSym->arrChildren[k].fOpacity = xc.fOpacity;
					pSym->arrChildren[k].iDepth = xc.iDepth;
				}
				/* Read type-specific payload */
				if ( xs.iType == XANIM_SYMBOL_SPRITE ) {
					xanim_sprite_data_t sd;
					__rbufRead(&rbuf, &sd, sizeof(sd));
					pSym->iResourceId = sd.iResourceId;
					pSym->fNineX1 = sd.fNineX1;
					pSym->fNineY1 = sd.fNineY1;
					pSym->fNineX2 = sd.fNineX2;
					pSym->fNineY2 = sd.fNineY2;
				} else if ( xs.iType == XANIM_SYMBOL_GRAPHIC ) {
					xanim_graphic_data_t gd;
					memset(&gd, 0, sizeof(gd));
					__rbufRead(&rbuf, &gd, sizeof(gd));
					pSym->iShapeCount = (int)gd.iShapeCount;
					for ( k = 0; k < (int)gd.iShapeCount && k < AF_DOC_MAX_SHAPE_RECORDS; k++ ) {
						xanim_shape_record_t sr;
						af_shape_record_t* pShape = &pSym->arrShapes[k];
						uint32_t iPathEnd;
						memset(pShape, 0, sizeof(af_shape_record_t));
						__rbufRead(&rbuf, &sr, sizeof(sr));
						pShape->tFill.iType = sr.iFillType;
						pShape->tStroke.iType = sr.iStrokeType;
						pShape->iFillRule = sr.iFillRule;
						pShape->fStrokeWidth = sr.fStrokeWidth;
						pShape->iLineCap = (int)sr.iLineCap;
						pShape->iLineJoin = (int)sr.iLineJoin;
						pShape->fMiterLimit = sr.fMiterLimit;
						/* Parse path commands */
						iPathEnd = rbuf.iPos + sr.iPathSize;
						while ( rbuf.iPos < iPathEnd ) {
							uint8_t cmd = 0;
							__rbufRead(&rbuf, &cmd, 1);
							if ( cmd == XANIM_PATH_MOVE_TO || cmd == XANIM_PATH_LINE_TO ) {
								float x = __rbufReadF32(&rbuf);
								float y = __rbufReadF32(&rbuf);
								if ( pShape->tPath.iCommandCount < AF_DOC_MAX_PATH_CMDS &&
								     pShape->tPath.iPointCount < AF_DOC_MAX_PATH_PTS ) {
									pShape->tPath.arrCommands[pShape->tPath.iCommandCount++] = cmd;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2] = x;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2 + 1] = y;
									pShape->tPath.iPointCount++;
								}
							} else if ( cmd == XANIM_PATH_QUAD_TO ) {
								float cx = __rbufReadF32(&rbuf);
								float cy = __rbufReadF32(&rbuf);
								float x = __rbufReadF32(&rbuf);
								float y = __rbufReadF32(&rbuf);
								if ( pShape->tPath.iCommandCount < AF_DOC_MAX_PATH_CMDS &&
								     pShape->tPath.iPointCount + 2 <= AF_DOC_MAX_PATH_PTS ) {
									pShape->tPath.arrCommands[pShape->tPath.iCommandCount++] = cmd;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2] = cx;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2 + 1] = cy;
									pShape->tPath.iPointCount++;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2] = x;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2 + 1] = y;
									pShape->tPath.iPointCount++;
								}
							} else if ( cmd == XANIM_PATH_CUBIC_TO ) {
								float c1x = __rbufReadF32(&rbuf);
								float c1y = __rbufReadF32(&rbuf);
								float c2x = __rbufReadF32(&rbuf);
								float c2y = __rbufReadF32(&rbuf);
								float x = __rbufReadF32(&rbuf);
								float y = __rbufReadF32(&rbuf);
								if ( pShape->tPath.iCommandCount < AF_DOC_MAX_PATH_CMDS &&
								     pShape->tPath.iPointCount + 3 <= AF_DOC_MAX_PATH_PTS ) {
									pShape->tPath.arrCommands[pShape->tPath.iCommandCount++] = cmd;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2] = c1x;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2 + 1] = c1y;
									pShape->tPath.iPointCount++;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2] = c2x;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2 + 1] = c2y;
									pShape->tPath.iPointCount++;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2] = x;
									pShape->tPath.arrPoints[pShape->tPath.iPointCount * 2 + 1] = y;
									pShape->tPath.iPointCount++;
								}
							} else if ( cmd == XANIM_PATH_CLOSE ) {
								if ( pShape->tPath.iCommandCount < AF_DOC_MAX_PATH_CMDS )
									pShape->tPath.arrCommands[pShape->tPath.iCommandCount++] = cmd;
							}
						}
						/* Read fill paint data */
						if ( sr.iFillType == XANIM_PAINT_SOLID ) {
							pShape->tFill.iColor = __rbufReadU32(&rbuf);
						} else if ( sr.iFillType == XANIM_PAINT_LINEAR || sr.iFillType == XANIM_PAINT_RADIAL ) {
							xanim_gradient_t grad;
							__rbufRead(&rbuf, &grad, sizeof(grad));
							pShape->tFill.fX1 = grad.fX1;
							pShape->tFill.fY1 = grad.fY1;
							pShape->tFill.fX2 = grad.fX2;
							pShape->tFill.fY2 = grad.fY2;
							pShape->tFill.fRadius = grad.fRadius;
							pShape->tFill.iStopCount = (int)grad.iStopCount;
							for ( int s = 0; s < (int)grad.iStopCount && s < AF_DOC_MAX_GRADIENT_STOPS; s++ ) {
								xanim_gradient_stop_t stop;
								__rbufRead(&rbuf, &stop, sizeof(stop));
								pShape->tFill.arrStops[s].fOffset = stop.fOffset;
								pShape->tFill.arrStops[s].iColor = stop.iColor;
							}
						}
						/* Read stroke paint data */
						if ( sr.iStrokeType == XANIM_PAINT_SOLID ) {
							pShape->tStroke.iColor = __rbufReadU32(&rbuf);
						} else if ( sr.iStrokeType == XANIM_PAINT_LINEAR || sr.iStrokeType == XANIM_PAINT_RADIAL ) {
							xanim_gradient_t grad;
							__rbufRead(&rbuf, &grad, sizeof(grad));
							pShape->tStroke.fX1 = grad.fX1;
							pShape->tStroke.fY1 = grad.fY1;
							pShape->tStroke.fX2 = grad.fX2;
							pShape->tStroke.fY2 = grad.fY2;
							pShape->tStroke.fRadius = grad.fRadius;
							pShape->tStroke.iStopCount = (int)grad.iStopCount;
							for ( int s = 0; s < (int)grad.iStopCount && s < AF_DOC_MAX_GRADIENT_STOPS; s++ ) {
								xanim_gradient_stop_t stop;
								__rbufRead(&rbuf, &stop, sizeof(stop));
								pShape->tStroke.arrStops[s].fOffset = stop.fOffset;
								pShape->tStroke.arrStops[s].iColor = stop.iColor;
							}
						}
					}
				}
				pDoc->iSymbolCount++;
				if ( xs.iId >= pDoc->iNextSymbolId )
					pDoc->iNextSymbolId = xs.iId + 1;
			}
			break;
		}
	}

	/* Load Timeline */
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		if ( toc[i].iSectionType == XANIM_SECTION_TIMELINE ) {
			uint32_t iLayerCount;
			__rbufSeek(&rbuf, toc[i].iOffset);
			iLayerCount = __rbufReadU32(&rbuf);
			pDoc->tTimeline.iLayerCount = 0;
			for ( j = 0; j < (int)iLayerCount && j < AF_DOC_MAX_LAYERS; j++ ) {
				xanim_layer_t xl;
				af_layer_t* pLayer;
				__rbufRead(&rbuf, &xl, sizeof(xl));
				pLayer = &pDoc->tTimeline.arrLayers[pDoc->tTimeline.iLayerCount];
				memset(pLayer, 0, sizeof(af_layer_t));
				pLayer->iId = xl.iId;
				snprintf(pLayer->sName, AF_DOC_MAX_NAME, "%s", __strtabGet(&strtab, xl.iNameIndex));
				pLayer->iType = (int)xl.iType;
				pLayer->iParentLayer = xl.iParentLayer;
				pLayer->iFlags = xl.iFlags;
				pLayer->iColor = xl.iColor;
				pLayer->iKeyframeCount = 0;
				/* Read keyframes */
				for ( k = 0; k < (int)xl.iKeyframeCount; k++ ) {
					xanim_keyframe_t xk;
					af_keyframe_t* pKf;
					int m;
					__rbufRead(&rbuf, &xk, sizeof(xk));
					if ( pLayer->iKeyframeCount >= (int)(AF_DOC_MAX_KEYFRAMES / AF_DOC_MAX_LAYERS) ) break;
					pKf = &pLayer->arrKeyframes[pLayer->iKeyframeCount];
					memset(pKf, 0, sizeof(af_keyframe_t));
					pKf->iFrame = xk.iFrame;
					pKf->iFlags = xk.iFlags;
					snprintf(pKf->sLabel, AF_DOC_MAX_NAME, "%s", __strtabGet(&strtab, xk.iLabelIndex));
					pKf->iElementCount = 0;
					/* Read elements */
					for ( m = 0; m < (int)xk.iElementCount && m < 64; m++ ) {
						xanim_element_t xe;
						af_element_t* pElem;
						__rbufRead(&rbuf, &xe, sizeof(xe));
						pElem = &pKf->arrElements[pKf->iElementCount];
						memset(pElem, 0, sizeof(af_element_t));
						pElem->iSymbolId = xe.iSymbolId;
						pElem->iInstanceId = xe.iInstanceId;
						pElem->fTx = xe.fTx;
						pElem->fTy = xe.fTy;
						pElem->fScaleX = xe.fScaleX;
						pElem->fScaleY = xe.fScaleY;
						pElem->fRotation = xe.fRotation;
						pElem->fSkewX = xe.fSkewX;
						pElem->fSkewY = xe.fSkewY;
						pElem->fOpacity = xe.fOpacity;
						pElem->fPivotX = xe.fPivotX;
						pElem->fPivotY = xe.fPivotY;
						pElem->iColorTransform = xe.iColorTransform;
						pElem->iTweenId = xe.iTweenId;
						pElem->iBlendMode = xe.iBlendMode;
						pElem->iFlags = xe.iFlags;
						pElem->iDepth = xe.iDepth;
						pKf->iElementCount++;
						if ( xe.iInstanceId >= pDoc->iNextInstanceId )
							pDoc->iNextInstanceId = xe.iInstanceId + 1;
					}
					pLayer->iKeyframeCount++;
				}
				pDoc->tTimeline.iLayerCount++;
				if ( xl.iId >= pDoc->iNextLayerId )
					pDoc->iNextLayerId = xl.iId + 1;
			}
			break;
		}
	}

	/* Load Curves */
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		if ( toc[i].iSectionType == XANIM_SECTION_CURVE ) {
			uint32_t iCount;
			__rbufSeek(&rbuf, toc[i].iOffset);
			iCount = __rbufReadU32(&rbuf);
			pDoc->iCurveCount = 0;
			for ( j = 0; j < (int)iCount && j < AF_DOC_MAX_CURVES; j++ ) {
				xanim_curve_t xc;
				af_curve_t* pCurve;
				__rbufRead(&rbuf, &xc, sizeof(xc));
				pCurve = &pDoc->arrCurves[pDoc->iCurveCount];
				memset(pCurve, 0, sizeof(af_curve_t));
				pCurve->iId = xc.iId;
				pCurve->iElementId = xc.iElementId;
				pCurve->iChannelMask = xc.iChannelMask;
				pCurve->iKeyCount = 0;
				for ( k = 0; k < (int)xc.iKeyCount && k < AF_CURVE_MAX_KEYS; k++ ) {
					xanim_curve_key_t xk;
					af_curve_key_t* pKey;
					__rbufRead(&rbuf, &xk, sizeof(xk));
					pKey = &pCurve->arrKeys[pCurve->iKeyCount];
					pKey->iFrame = xk.iFrame;
					pKey->iInterpType = xk.iInterpType;
					memcpy(pKey->arrValues, xk.arrValues, sizeof(pKey->arrValues));
					pKey->fEaseIn = xk.fEaseIn;
					pKey->fEaseOut = xk.fEaseOut;
					memcpy(pKey->arrBezier, xk.arrBezier, sizeof(pKey->arrBezier));
					pCurve->iKeyCount++;
				}
				pDoc->iCurveCount++;
				if ( xc.iId >= pDoc->iNextCurveId )
					pDoc->iNextCurveId = xc.iId + 1;
			}
			break;
		}
	}

	/* Load Actions */
	for ( i = 0; i < (int)header.iTocCount && i < 7; i++ ) {
		if ( toc[i].iSectionType == XANIM_SECTION_ACTION ) {
			uint32_t iCount;
			__rbufSeek(&rbuf, toc[i].iOffset);
			iCount = __rbufReadU32(&rbuf);
			pDoc->iActionCount = 0;
			for ( j = 0; j < (int)iCount && j < AF_DOC_MAX_ACTIONS; j++ ) {
				xanim_action_t xa;
				af_action_t* pAct;
				__rbufRead(&rbuf, &xa, sizeof(xa));
				pAct = &pDoc->arrActions[pDoc->iActionCount];
				memset(pAct, 0, sizeof(af_action_t));
				pAct->iFrame = xa.iFrame;
				pAct->iLayerId = xa.iLayerId;
				pAct->iType = xa.iType;
				pAct->iParam = xa.iParam;
				snprintf(pAct->sName, AF_DOC_MAX_NAME, "%s", __strtabGet(&strtab, xa.iNameIndex));
				pDoc->iActionCount++;
			}
			break;
		}
	}

	/* Store file path */
	snprintf(pDoc->sFilePath, sizeof(pDoc->sFilePath), "%s", sPath);
	pDoc->bModified = 0;

	/* Cleanup */
	if ( strtab.arrStrings != NULL ) free((void*)strtab.arrStrings);
	free(pData);
	return 0;
}
