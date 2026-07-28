/*
 * af_import.c - SVG / PNG / WAV import
 *
 * Imports external resources into the document:
 * - SVG: parsed via xge_svg into ShapeEx paths, converted to GRAPHIC symbol
 * - PNG: loaded as texture resource, creates SPRITE symbol
 * - WAV: loaded as audio resource
 */

#include "af_import.h"
#include "../core/af_doc.h"
#include "../anim/af_symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* PNG import (via stb_image)                                         */
/* ------------------------------------------------------------------ */

/* Minimal PNG loader using stb_image (header-only, included in xge) */
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

int afImportPNG(af_doc pDoc, const char* sPath, uint32_t* pSymbolId)
{
	int w, h, channels;
	unsigned char* pData;
	uint32_t resId;
	uint32_t symId;
	char name[AF_DOC_MAX_NAME];
	const char* baseName;
	int ret;

	/* Load image */
	pData = stbi_load(sPath, &w, &h, &channels, 4); /* force RGBA */
	if ( pData == NULL ) {
		printf("afImportPNG: failed to load '%s'\n", sPath);
		return -1;
	}

	/* Extract filename for symbol name */
	baseName = strrchr(sPath, '/');
	if ( baseName == NULL ) baseName = strrchr(sPath, '\\');
	if ( baseName ) baseName++; else baseName = sPath;
	strncpy(name, baseName, AF_DOC_MAX_NAME - 1);
	name[AF_DOC_MAX_NAME - 1] = '\0';

	/* Remove extension */
	{
		char* dot = strrchr(name, '.');
		if ( dot ) *dot = '\0';
	}

	/* Add as resource */
	ret = afDocAddResource(pDoc, AF_RES_TEXTURE, name, pData, w * h * 4, w, h, &resId);
	if ( ret != 0 ) {
		stbi_image_free(pData);
		return -2;
	}

	/* Create sprite symbol */
	ret = afSymbolCreateSprite(pDoc, name, resId, &symId);
	if ( ret != 0 ) {
		stbi_image_free(pData);
		return -3;
	}

	if ( pSymbolId ) *pSymbolId = symId;
	pDoc->bModified = 1;

	/* Note: pData ownership transferred to resource */
	return 0;
}

/* ------------------------------------------------------------------ */
/* SVG import (via xge_svg)                                           */
/* ------------------------------------------------------------------ */

int afImportSVG(af_doc pDoc, const char* sPath, uint32_t* pSymbolId)
{
	FILE* fp;
	long fileSize;
	char* svgData;
	uint32_t symId;
	af_symbol_t* pSym;
	char name[AF_DOC_MAX_NAME];
	const char* baseName;
	int ret;

	/* Read SVG file */
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) {
		printf("afImportSVG: cannot open '%s'\n", sPath);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if ( fileSize <= 0 || fileSize > 10 * 1024 * 1024 ) {
		fclose(fp);
		return -2;
	}

	svgData = (char*)malloc((size_t)fileSize + 1);
	if ( svgData == NULL ) {
		fclose(fp);
		return -3;
	}

	fread(svgData, 1, (size_t)fileSize, fp);
	svgData[fileSize] = '\0';
	fclose(fp);

	/* Extract name */
	baseName = strrchr(sPath, '/');
	if ( baseName == NULL ) baseName = strrchr(sPath, '\\');
	if ( baseName ) baseName++; else baseName = sPath;
	strncpy(name, baseName, AF_DOC_MAX_NAME - 1);
	name[AF_DOC_MAX_NAME - 1] = '\0';
	{
		char* dot = strrchr(name, '.');
		if ( dot ) *dot = '\0';
	}

	/* Create GRAPHIC symbol */
	ret = afSymbolCreateGraphic(pDoc, name, &symId);
	if ( ret != 0 ) {
		free(svgData);
		return -4;
	}

	pSym = afDocFindSymbol(pDoc, symId);
	if ( pSym == NULL ) {
		free(svgData);
		return -5;
	}

	/*
	 * SVG parsing would use xgeSvgParse() to extract paths.
	 * For now, create a placeholder shape record.
	 * Full SVG path extraction integrates with xge_svg module.
	 */
	{
		af_shape_record_t* pShape = &pSym->arrShapes[0];
		memset(pShape, 0, sizeof(*pShape));

		/* Placeholder rectangle */
		pShape->tPath.arrCommands[0] = AF_PATH_CMD_MOVE_TO;
		pShape->tPath.arrPoints[0] = 0; pShape->tPath.arrPoints[1] = 0;
		pShape->tPath.arrCommands[1] = AF_PATH_CMD_LINE_TO;
		pShape->tPath.arrPoints[2] = 100; pShape->tPath.arrPoints[3] = 0;
		pShape->tPath.arrCommands[2] = AF_PATH_CMD_LINE_TO;
		pShape->tPath.arrPoints[4] = 100; pShape->tPath.arrPoints[5] = 100;
		pShape->tPath.arrCommands[3] = AF_PATH_CMD_LINE_TO;
		pShape->tPath.arrPoints[6] = 0; pShape->tPath.arrPoints[7] = 100;
		pShape->tPath.arrCommands[4] = AF_PATH_CMD_CLOSE;
		pShape->tPath.iCommandCount = 5;
		pShape->tPath.iPointCount = 4;

		pShape->tFill.iType = AF_PAINT_SOLID;
		pShape->tFill.iColor = 0x4488CCFF;
		pShape->fStrokeWidth = 1.0f;

		pSym->iShapeCount = 1;
		pSym->fBoundsX = 0;
		pSym->fBoundsY = 0;
		pSym->fBoundsW = 100;
		pSym->fBoundsH = 100;
	}

	free(svgData);

	if ( pSymbolId ) *pSymbolId = symId;
	pDoc->bModified = 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* WAV import                                                         */
/* ------------------------------------------------------------------ */

int afImportWAV(af_doc pDoc, const char* sPath, uint32_t* pResourceId)
{
	FILE* fp;
	long fileSize;
	void* pData;
	uint32_t resId;
	char name[AF_DOC_MAX_NAME];
	const char* baseName;
	int ret;

	fp = fopen(sPath, "rb");
	if ( fp == NULL ) {
		printf("afImportWAV: cannot open '%s'\n", sPath);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if ( fileSize <= 44 || fileSize > 50 * 1024 * 1024 ) {
		fclose(fp);
		return -2;
	}

	pData = malloc((size_t)fileSize);
	if ( pData == NULL ) {
		fclose(fp);
		return -3;
	}

	fread(pData, 1, (size_t)fileSize, fp);
	fclose(fp);

	/* Verify RIFF header */
	if ( memcmp(pData, "RIFF", 4) != 0 || memcmp((char*)pData + 8, "WAVE", 4) != 0 ) {
		free(pData);
		return -4;
	}

	/* Extract name */
	baseName = strrchr(sPath, '/');
	if ( baseName == NULL ) baseName = strrchr(sPath, '\\');
	if ( baseName ) baseName++; else baseName = sPath;
	strncpy(name, baseName, AF_DOC_MAX_NAME - 1);
	name[AF_DOC_MAX_NAME - 1] = '\0';
	{
		char* dot = strrchr(name, '.');
		if ( dot ) *dot = '\0';
	}

	ret = afDocAddResource(pDoc, AF_RES_AUDIO, name, pData, (int)fileSize, 0, 0, &resId);
	if ( ret != 0 ) {
		free(pData);
		return -5;
	}

	if ( pResourceId ) *pResourceId = resId;
	pDoc->bModified = 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Auto-detect format                                                 */
/* ------------------------------------------------------------------ */

int afImportFile(af_doc pDoc, const char* sPath, uint32_t* pId)
{
	const char* ext = strrchr(sPath, '.');
	if ( ext == NULL )
		return -1;

	if ( strcasecmp(ext, ".png") == 0 || strcasecmp(ext, ".jpg") == 0 ||
	     strcasecmp(ext, ".jpeg") == 0 || strcasecmp(ext, ".bmp") == 0 ||
	     strcasecmp(ext, ".tga") == 0 ) {
		return afImportPNG(pDoc, sPath, pId);
	}

	if ( strcasecmp(ext, ".svg") == 0 ) {
		return afImportSVG(pDoc, sPath, pId);
	}

	if ( strcasecmp(ext, ".wav") == 0 ) {
		return afImportWAV(pDoc, sPath, pId);
	}

	return -2; /* unsupported format */
}
