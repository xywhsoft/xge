#include "mapedit_xui2_mapdoc.h"
#include "xui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int __mapdocReadFileBytes(const char* sPath, char** ppText, long* pSize)
{
	FILE* fp;
	long n;
	char* p;
	if ( sPath == NULL || ppText == NULL || pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppText = NULL;
	*pSize = 0;
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	if ( fseek(fp, 0, SEEK_END) != 0 ) {
		fclose(fp);
		return XUI_ERROR;
	}
	n = ftell(fp);
	if ( n <= 0 ) {
		fclose(fp);
		return XUI_ERROR;
	}
	if ( fseek(fp, 0, SEEK_SET) != 0 ) {
		fclose(fp);
		return XUI_ERROR;
	}
	p = (char*)malloc((size_t)n + 1u);
	if ( p == NULL ) {
		fclose(fp);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( fread(p, 1, (size_t)n, fp) != (size_t)n ) {
		free(p);
		fclose(fp);
		return XUI_ERROR;
	}
	fclose(fp);
	p[n] = 0;
	*ppText = p;
	*pSize = n;
	return XUI_OK;
}

static int __mapdocNextIntToken(const char* sText, long n, long* pOffset, int* pValue)
{
	long i;
	int sign;
	int value;
	if ( sText == NULL || pOffset == NULL || pValue == NULL ) return 0;
	i = *pOffset;
	while ( i < n ) {
		if ( (sText[i] >= '0' && sText[i] <= '9') || sText[i] == '-' ) break;
		i++;
	}
	if ( i >= n ) return 0;
	sign = 1;
	if ( sText[i] == '-' ) {
		sign = -1;
		i++;
	}
	value = 0;
	while ( i < n && sText[i] >= '0' && sText[i] <= '9' ) {
		value = (value * 10) + (sText[i] - '0');
		i++;
	}
	*pOffset = i;
	*pValue = value * sign;
	return 1;
}

static const char* __mapdocFindLargestIntArray(const char* sText, long n, long* pStart, long* pEnd, int* pCount)
{
	long i;
	const char* pBest;
	long iBestStart;
	long iBestEnd;
	int iBestCount;
	if ( sText == NULL || pStart == NULL || pEnd == NULL || pCount == NULL ) return NULL;
	pBest = NULL;
	iBestStart = 0;
	iBestEnd = 0;
	iBestCount = 0;
	for ( i = 0; i < n; i++ ) {
		long j;
		long off;
		long close;
		int count;
		int value;
		if ( sText[i] != '[' ) continue;
		close = i + 1;
		while ( close < n && sText[close] != ']' ) close++;
		if ( close >= n ) break;
		j = i + 1;
		count = 0;
		while ( j < close ) {
			off = j;
			if ( __mapdocNextIntToken(sText, close, &off, &value) ) {
				if ( off <= j || off > close ) break;
				count++;
				j = off;
			} else {
				j++;
			}
		}
		if ( count > iBestCount ) {
			pBest = sText + i + 1;
			iBestStart = i + 1;
			iBestEnd = close;
			iBestCount = count;
		}
		i = close;
	}
	*pStart = iBestStart;
	*pEnd = iBestEnd;
	*pCount = iBestCount;
	return pBest;
}

static int __mapdocParseTextValueAfterKey(const char* sText, const char* sKey, char* sOut, int iOutSize)
{
	const char* p;
	const char* q;
	int n;
	if ( sText == NULL || sKey == NULL || sOut == NULL || iOutSize <= 0 ) return 0;
	sOut[0] = 0;
	p = strstr(sText, sKey);
	if ( p == NULL ) return 0;
	p = strchr(p + strlen(sKey), ':');
	if ( p == NULL ) return 0;
	p = strchr(p, '"');
	if ( p == NULL ) return 0;
	p++;
	q = strchr(p, '"');
	if ( q == NULL || q <= p ) return 0;
	n = (int)(q - p);
	if ( n >= iOutSize ) n = iOutSize - 1;
	memcpy(sOut, p, (size_t)n);
	sOut[n] = 0;
	return 1;
}

static int __mapdocParseIntValueAfterKey(const char* sText, long n, const char* sKey, int* pValue)
{
	const char* p;
	long off;
	if ( sText == NULL || sKey == NULL || pValue == NULL ) return 0;
	p = strstr(sText, sKey);
	if ( p == NULL ) return 0;
	off = (long)(p - sText) + (long)strlen(sKey);
	return __mapdocNextIntToken(sText, n, &off, pValue);
}

void mapeditXui2MapDocInit(mapedit_xui2_mapdoc_t* pDoc)
{
	if ( pDoc == NULL ) return;
	memset(pDoc, 0, sizeof(*pDoc));
	pDoc->iLayerCount = 4;
}

void mapeditXui2MapDocUnit(mapedit_xui2_mapdoc_t* pDoc)
{
	if ( pDoc == NULL ) return;
	free(pDoc->pTiles);
	memset(pDoc, 0, sizeof(*pDoc));
}

int mapeditXui2MapDocLoad(mapedit_xui2_mapdoc_t* pDoc, const char* sPath)
{
	char* sText;
	long n;
	long arrStart;
	long arrEnd;
	long off;
	int count;
	int value;
	int i;
	int ret;
	int* pTiles;
	if ( pDoc == NULL || sPath == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sText = NULL;
	n = 0;
	ret = __mapdocReadFileBytes(sPath, &sText, &n);
	if ( ret != XUI_OK ) return ret;
	mapeditXui2MapDocUnit(pDoc);
	mapeditXui2MapDocInit(pDoc);
	snprintf(pDoc->sPath, sizeof(pDoc->sPath), "%s", sPath);
	(void)__mapdocParseTextValueAfterKey(sText, "鍦板浘鍚嶇О", pDoc->sName, (int)sizeof(pDoc->sName));
	(void)__mapdocParseTextValueAfterKey(sText, "鍥鹃泦", pDoc->sTileset, (int)sizeof(pDoc->sTileset));
	(void)__mapdocParseIntValueAfterKey(sText, n, "鍦板浘楂樺害", &pDoc->iHeight);
	(void)__mapdocParseIntValueAfterKey(sText, n, "鍦板浘瀹藉害", &pDoc->iWidth);
	(void)__mapdocParseIntValueAfterKey(sText, n, "褰撳墠鐘舵€?", &pDoc->iState);
	if ( pDoc->iWidth <= 0 || pDoc->iHeight <= 0 ) {
		off = 0;
		(void)__mapdocNextIntToken(sText, n, &off, &pDoc->iHeight);
		(void)__mapdocNextIntToken(sText, n, &off, &pDoc->iWidth);
	}
	arrStart = 0;
	arrEnd = 0;
	count = 0;
	if ( __mapdocFindLargestIntArray(sText, n, &arrStart, &arrEnd, &count) == NULL || count <= 0 ) {
		free(sText);
		return XUI_ERROR;
	}
	pTiles = (int*)calloc((size_t)count, sizeof(int));
	if ( pTiles == NULL ) {
		free(sText);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	off = arrStart;
	for ( i = 0; i < count && off < arrEnd; i++ ) {
		if ( !__mapdocNextIntToken(sText, arrEnd, &off, &value) ) break;
		pTiles[i] = value;
		if ( value != 0 ) pDoc->iNonZeroTileCount++;
	}
	pDoc->pTiles = pTiles;
	pDoc->iTileCount = i;
	if ( pDoc->iWidth > 0 && pDoc->iHeight > 0 ) {
		int layers = pDoc->iTileCount / (pDoc->iWidth * pDoc->iHeight);
		if ( layers > 0 ) pDoc->iLayerCount = layers;
	}
	if ( pDoc->sName[0] == 0 ) snprintf(pDoc->sName, sizeof(pDoc->sName), "default");
	if ( pDoc->sTileset[0] == 0 ) snprintf(pDoc->sTileset, sizeof(pDoc->sTileset), "default.xson");
	free(sText);
	return (pDoc->iWidth > 0 && pDoc->iHeight > 0 && pDoc->iTileCount > 0) ? XUI_OK : XUI_ERROR;
}

int mapeditXui2MapDocSaveSmoke(const mapedit_xui2_mapdoc_t* pDoc, const char* sPath)
{
	FILE* fp;
	int i;
	if ( pDoc == NULL || sPath == NULL || pDoc->iWidth <= 0 || pDoc->iHeight <= 0 || pDoc->iTileCount <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	fp = fopen(sPath, "wb");
	if ( fp == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	fprintf(fp, "{\n");
	fprintf(fp, "\t\"name\":\t\"%s\",\n", pDoc->sName[0] ? pDoc->sName : "default");
	fprintf(fp, "\t\"tileset\":\t\"%s\",\n", pDoc->sTileset[0] ? pDoc->sTileset : "default.xson");
	fprintf(fp, "\t\"width\":\t%d,\n", pDoc->iWidth);
	fprintf(fp, "\t\"height\":\t%d,\n", pDoc->iHeight);
	fprintf(fp, "\t\"layers\":\t%d,\n", pDoc->iLayerCount);
	fprintf(fp, "\t\"state\":\t%d,\n", pDoc->iState);
	fprintf(fp, "\t\"tiles\":\t[");
	for ( i = 0; i < pDoc->iTileCount; i++ ) {
		if ( i > 0 ) fprintf(fp, ",");
		if ( (i % 32) == 0 ) fprintf(fp, "\n\t\t");
		fprintf(fp, "%d", pDoc->pTiles[i]);
	}
	fprintf(fp, "\n\t]\n");
	fprintf(fp, "}\n");
	fclose(fp);
	return XUI_OK;
}

int mapeditXui2MapDocTileIndex(const mapedit_xui2_mapdoc_t* pDoc, int iLayer, int x, int y)
{
	int index;
	if ( pDoc == NULL || pDoc->iWidth <= 0 || pDoc->iHeight <= 0 || iLayer < 0 || x < 0 || y < 0 || x >= pDoc->iWidth || y >= pDoc->iHeight ) {
		return -1;
	}
	index = (iLayer * pDoc->iHeight + y) * pDoc->iWidth + x;
	return (index >= 0 && index < pDoc->iTileCount) ? index : -1;
}

int mapeditXui2MapDocSetTile(mapedit_xui2_mapdoc_t* pDoc, int iLayer, int x, int y, int iTile)
{
	int index;
	int oldTile;
	if ( pDoc == NULL || pDoc->pTiles == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	index = mapeditXui2MapDocTileIndex(pDoc, iLayer, x, y);
	if ( index < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	oldTile = pDoc->pTiles[index];
	if ( oldTile == 0 && iTile != 0 ) pDoc->iNonZeroTileCount++;
	if ( oldTile != 0 && iTile == 0 && pDoc->iNonZeroTileCount > 0 ) pDoc->iNonZeroTileCount--;
	pDoc->pTiles[index] = iTile;
	return XUI_OK;
}
