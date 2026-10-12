/*
 * xanim_parse.c - XANIM file parsing / memory mapping
 *
 * Handles reading and parsing of .xanim binary files:
 * header validation, TOC parsing, section loading, string table.
 */

#include "xanim.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* CRC32 (shared utility)                                             */
/* ------------------------------------------------------------------ */

uint32_t xanimParseCrc32(const void* pData, uint32_t iSize)
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
			for ( k = 0; k < 8; k++ )
				c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
			sTable[j] = c;
		}
		sInit = 1;
	}

	for ( i = 0; i < iSize; i++ )
		crc = sTable[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);

	return crc ^ 0xFFFFFFFFu;
}

/* ------------------------------------------------------------------ */
/* Header validation                                                  */
/* ------------------------------------------------------------------ */

int xanimParseValidateHeader(const uint8_t* pData, int iSize, xanim_header_t* pOut)
{
	if ( iSize < 64 )
		return XANIM_ERR_INVALID_FILE;

	memcpy(pOut, pData, sizeof(xanim_header_t));

	if ( pOut->magic != XANIM_MAGIC )
		return XANIM_ERR_INVALID_MAGIC;

	if ( pOut->version > XANIM_VERSION )
		return XANIM_ERR_UNSUPPORTED_VERSION;

	if ( pOut->headerSize != 64 )
		return XANIM_ERR_INVALID_FILE;

	return XANIM_OK;
}

/* ------------------------------------------------------------------ */
/* TOC parsing                                                        */
/* ------------------------------------------------------------------ */

int xanimParseTOC(const uint8_t* pData, int iSize, const xanim_header_t* pHeader,
                  xanim_toc_entry_t* pEntries, int iMaxEntries, int* pCount)
{
	uint32_t offset = pHeader->tocOffset;
	uint32_t count = pHeader->tocCount;
	uint32_t i;

	if ( count > (uint32_t)iMaxEntries )
		count = (uint32_t)iMaxEntries;

	if ( offset + count * sizeof(xanim_toc_entry_t) > (uint32_t)iSize )
		return XANIM_ERR_INVALID_FILE;

	for ( i = 0; i < count; i++ ) {
		memcpy(&pEntries[i], pData + offset + i * sizeof(xanim_toc_entry_t),
		       sizeof(xanim_toc_entry_t));
	}

	if ( pCount ) *pCount = (int)count;
	return XANIM_OK;
}

/* ------------------------------------------------------------------ */
/* String table parsing                                               */
/* ------------------------------------------------------------------ */

int xanimParseStringTable(const uint8_t* pData, int iSectionSize,
                          char*** ppStrings, int* pCount)
{
	uint32_t count, dataOffset, dataSize;
	uint32_t i;
	const uint8_t* pSection = pData;
	char** ppStr;
	const uint32_t* pOffsets;
	const char* pStrData;

	if ( iSectionSize < 12 )
		return XANIM_ERR_INVALID_FILE;

	memcpy(&count, pSection, 4);
	memcpy(&dataOffset, pSection + 4, 4);
	memcpy(&dataSize, pSection + 8, 4);

	if ( count == 0 ) {
		*ppStrings = NULL;
		*pCount = 0;
		return XANIM_OK;
	}

	if ( count > 65536 )
		return XANIM_ERR_INVALID_FILE;

	ppStr = (char**)calloc(count, sizeof(char*));
	if ( ppStr == NULL )
		return XANIM_ERR_OUT_OF_MEMORY;

	pOffsets = (const uint32_t*)(pSection + 12);
	pStrData = (const char*)(pSection + dataOffset);

	for ( i = 0; i < count; i++ ) {
		uint32_t strOff = pOffsets[i];
		const char* src;
		int len;

		if ( strOff >= dataSize ) {
			ppStr[i] = strdup("");
			continue;
		}

		src = pStrData + strOff;
		len = (int)strlen(src);
		ppStr[i] = (char*)malloc((size_t)len + 1);
		if ( ppStr[i] ) {
			memcpy(ppStr[i], src, (size_t)len + 1);
		}
	}

	*ppStrings = ppStr;
	*pCount = (int)count;
	return XANIM_OK;
}

/* ------------------------------------------------------------------ */
/* File loading                                                       */
/* ------------------------------------------------------------------ */

int xanimParseLoadFile(const char* sPath, uint8_t** ppData, int* pSize)
{
	FILE* fp;
	long size;
	uint8_t* pData;

	fp = fopen(sPath, "rb");
	if ( fp == NULL )
		return XANIM_ERR_FILE_NOT_FOUND;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if ( size < 64 ) {
		fclose(fp);
		return XANIM_ERR_INVALID_FILE;
	}

	pData = (uint8_t*)malloc((size_t)size);
	if ( pData == NULL ) {
		fclose(fp);
		return XANIM_ERR_OUT_OF_MEMORY;
	}

	if ( fread(pData, 1, (size_t)size, fp) != (size_t)size ) {
		free(pData);
		fclose(fp);
		return XANIM_ERR_IO_ERROR;
	}

	fclose(fp);
	*ppData = pData;
	*pSize = (int)size;
	return XANIM_OK;
}

void xanimParseFreeStringTable(char** ppStrings, int iCount)
{
	int i;
	if ( ppStrings == NULL ) return;
	for ( i = 0; i < iCount; i++ ) {
		if ( ppStrings[i] ) free(ppStrings[i]);
	}
	free(ppStrings);
}
