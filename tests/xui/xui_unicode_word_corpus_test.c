#include "../src/xui/core/xui_unicode_core.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { \
	if (!(e)) { \
		fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); \
		exit(1); \
	} \
} while (0)

typedef struct word_reader_t {
	const unsigned char* pText;
	int64_t iLength;
} word_reader_t;

static int read_byte64(void* pUser, int64_t iOffset, unsigned char* pByte)
{
	word_reader_t* pReader = (word_reader_t*)pUser;
	CHECK(pReader != NULL && pByte != NULL);
	CHECK(iOffset >= 0 && iOffset < pReader->iLength);
	*pByte = pReader->pText[iOffset];
	return 1;
}

static int read_byte32(void* pUser, int iOffset, unsigned char* pByte)
{
	return read_byte64(pUser, iOffset, pByte);
}

static int encode_utf8(uint32_t iCodepoint, unsigned char* pText)
{
	if ( iCodepoint < 0x80u ) {
		pText[0] = (unsigned char)iCodepoint;
		return 1;
	}
	if ( iCodepoint < 0x800u ) {
		pText[0] = (unsigned char)(0xC0u | (iCodepoint >> 6));
		pText[1] = (unsigned char)(0x80u | (iCodepoint & 0x3Fu));
		return 2;
	}
	if ( iCodepoint < 0x10000u ) {
		pText[0] = (unsigned char)(0xE0u | (iCodepoint >> 12));
		pText[1] = (unsigned char)(0x80u | ((iCodepoint >> 6) & 0x3Fu));
		pText[2] = (unsigned char)(0x80u | (iCodepoint & 0x3Fu));
		return 3;
	}
	pText[0] = (unsigned char)(0xF0u | (iCodepoint >> 18));
	pText[1] = (unsigned char)(0x80u | ((iCodepoint >> 12) & 0x3Fu));
	pText[2] = (unsigned char)(0x80u | ((iCodepoint >> 6) & 0x3Fu));
	pText[3] = (unsigned char)(0x80u | (iCodepoint & 0x3Fu));
	return 4;
}

int main(int argc, char** argv)
{
	unsigned char sText[4096];
	unsigned char aExpected[4097];
	int aOffsets[2048];
	char sLine[4096];
	unsigned iCase = 0;
	unsigned iBoundaryCount = 0;
	unsigned iMismatch = 0;
	FILE* pFile;

	CHECK(argc == 2);
	pFile = fopen(argv[1], "rb");
	CHECK(pFile != NULL);
	while ( fgets(sLine, sizeof(sLine), pFile) != NULL ) {
		word_reader_t tReader;
		char* pComment;
		char* pToken;
		int iLength = 0;
		int iOffsetCount = 0;
		int i;

		pComment = strchr(sLine, '#');
		if ( pComment != NULL ) *pComment = '\0';
		memset(aExpected, 0, sizeof(aExpected));
		pToken = strtok(sLine, " \t\r\n");
		if ( pToken == NULL ) continue;
		do {
			if ( (unsigned char)pToken[0] == 0xC3u ) {
				CHECK((unsigned char)pToken[1] == 0xB7u ||
					(unsigned char)pToken[1] == 0x97u);
				CHECK(iOffsetCount < (int)(sizeof(aOffsets) / sizeof(aOffsets[0])));
				aExpected[iLength] = (unsigned char)((unsigned char)pToken[1] == 0xB7u);
				aOffsets[iOffsetCount++] = iLength;
			} else {
				char* pEnd;
				unsigned long iCodepoint = strtoul(pToken, &pEnd, 16);
				CHECK(*pEnd == '\0' && iCodepoint <= 0x10FFFFu);
				CHECK(iLength + 4 < (int)sizeof(sText));
				iLength += encode_utf8((uint32_t)iCodepoint, sText + iLength);
			}
		} while ( (pToken = strtok(NULL, " \t\r\n")) != NULL );

		tReader.pText = sText;
		tReader.iLength = iLength;
		iCase++;
		for ( i = 0; i < iOffsetCount; i++ ) {
			int iOffset = aOffsets[i];
			int iExpected = aExpected[iOffset] != 0;
			int iPointer = xuiInternalTextWordBoundary((const char*)sText, iLength,
				iOffset, XUI_INTERNAL_WORD_NATURAL);
			int iReader32 = xuiInternalTextWordBoundaryRead(read_byte32, &tReader,
				iLength, iOffset, XUI_INTERNAL_WORD_NATURAL);
			int iReader = xuiInternalTextWordBoundary64Read(read_byte64, &tReader,
				iLength, iOffset, XUI_INTERNAL_WORD_NATURAL);
			iBoundaryCount++;
			CHECK(iPointer == iReader32 && iPointer == iReader);
			if ( iPointer != iExpected ) {
				if ( iMismatch < 20 ) {
					fprintf(stderr, "case=%u offset=%d expected=%d actual=%d\n",
						iCase, iOffset, iExpected, iPointer);
				}
				iMismatch++;
			}
		}
	}
	CHECK(!ferror(pFile));
	CHECK(fclose(pFile) == 0);
	CHECK(iCase == 1944);
	printf("Unicode 17 word corpus: %u cases, %u boundaries, %u mismatches\n",
		iCase, iBoundaryCount, iMismatch);
	return iMismatch == 0 ? 0 : 2;
}
