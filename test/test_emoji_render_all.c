#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID_COLUMNS 48
#define CELL_SIZE 20
#define TARGET_WIDTH (GRID_COLUMNS * CELL_SIZE)
#define TARGET_HEIGHT (41 * CELL_SIZE)

typedef struct emoji_render_test_t {
	xge_font_t tFont;
	xge_render_target_t tTarget;
	char* pSequences;
	int* pOffsets;
	int* pLengths;
	int iCount;
	int iResult;
} emoji_render_test_t;

static int utf8_append(char* sText, int iCapacity, int* pSize, unsigned long iCodepoint)
{
	unsigned char bytes[4];
	int count;

	if ( iCodepoint <= 0x7fu ) {
		bytes[0] = (unsigned char)iCodepoint;
		count = 1;
	} else if ( iCodepoint <= 0x7ffu ) {
		bytes[0] = (unsigned char)(0xc0u | (iCodepoint >> 6));
		bytes[1] = (unsigned char)(0x80u | (iCodepoint & 0x3fu));
		count = 2;
	} else if ( iCodepoint <= 0xffffu ) {
		bytes[0] = (unsigned char)(0xe0u | (iCodepoint >> 12));
		bytes[1] = (unsigned char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		bytes[2] = (unsigned char)(0x80u | (iCodepoint & 0x3fu));
		count = 3;
	} else {
		bytes[0] = (unsigned char)(0xf0u | (iCodepoint >> 18));
		bytes[1] = (unsigned char)(0x80u | ((iCodepoint >> 12) & 0x3fu));
		bytes[2] = (unsigned char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		bytes[3] = (unsigned char)(0x80u | (iCodepoint & 0x3fu));
		count = 4;
	}
	if ( *pSize + count >= iCapacity ) return 0;
	memcpy(sText + *pSize, bytes, (size_t)count);
	*pSize += count;
	sText[*pSize] = 0;
	return 1;
}

static int load_sequences(emoji_render_test_t* pTest)
{
	FILE* file;
	char line[512];
	char sequence[160];
	char* token;
	char* end;
	unsigned long codepoint;
	int total_size;
	int sequence_size;
	int capacity;

	file = fopen("res/emoji/twemoji_core/17.0/manifest.txt", "rb");
	if ( file == NULL ) return 0;
	capacity = 2048;
	pTest->pOffsets = (int*)malloc((size_t)capacity * sizeof(int));
	pTest->pLengths = (int*)malloc((size_t)capacity * sizeof(int));
	pTest->pSequences = (char*)malloc((size_t)capacity * 160u);
	if ( (pTest->pOffsets == NULL) || (pTest->pLengths == NULL) || (pTest->pSequences == NULL) ) {
		fclose(file);
		return 0;
	}
	total_size = 0;
	while ( fgets(line, sizeof(line), file) != NULL ) {
		end = strchr(line, '|');
		if ( (end == NULL) || (pTest->iCount >= capacity) ) {
			fclose(file);
			return 0;
		}
		*end = 0;
		sequence[0] = 0;
		sequence_size = 0;
		token = strtok(line, " \t\r\n");
		while ( token != NULL ) {
			codepoint = strtoul(token, &end, 16);
			if ( (*end != 0) || !utf8_append(sequence, (int)sizeof(sequence), &sequence_size, codepoint) ) {
				fclose(file);
				return 0;
			}
			token = strtok(NULL, " \t\r\n");
		}
		pTest->pOffsets[pTest->iCount] = total_size;
		pTest->pLengths[pTest->iCount] = sequence_size;
		memcpy(pTest->pSequences + total_size, sequence, (size_t)sequence_size + 1u);
		total_size += sequence_size + 1;
		pTest->iCount++;
	}
	fclose(file);
	return pTest->iCount == 1923;
}

static int render_frame(void* pUser)
{
	emoji_render_test_t* pTest = (emoji_render_test_t*)pUser;
	xge_text_shape_desc_t desc;
	xge_glyph_run_t run;
	xge_pass_t pass;
	int i;

	if ( xgeBegin() != XGE_OK ) return XGE_ERROR;
	xgePassInit(&pass, &pTest->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(255, 255, 255, 255));
	if ( xgePassBegin(&pass) != XGE_OK ) return XGE_ERROR;
	for ( i = 0; i < pTest->iCount; i++ ) {
		memset(&desc, 0, sizeof(desc));
		memset(&run, 0, sizeof(run));
		desc.iSize = sizeof(desc);
		desc.pFont = &pTest->tFont;
		desc.sText = pTest->pSequences + pTest->pOffsets[i];
		desc.iTextSize = pTest->pLengths[i];
		desc.iFlags = XGE_TEXT_SHAPE_DEFAULT;
		desc.iEmojiPresentation = XGE_EMOJI_PRESENTATION_COLOR;
		desc.iEmojiLinePolicy = XGE_EMOJI_LINE_STABLE;
		if ( (xgeTextShape(&desc, &run) != XGE_OK) || (run.iGlyphCount != 1) ||
		     (run.pGlyphs[0].iItemKind != XGE_TEXT_ITEM_EMOJI) ) {
			fprintf(stderr, "emoji render-all shape failed at manifest entry %d\n", i + 1);
			pTest->iResult = 1;
			xgeGlyphRunFree(&run);
			break;
		}
		xgeGlyphRunDraw(
			&run, (float)((i % GRID_COLUMNS) * CELL_SIZE),
			(float)((i / GRID_COLUMNS) * CELL_SIZE), 0xffffffffu, XGE_DRAW_SCREEN_SPACE
		);
		xgeGlyphRunFree(&run);
	}
	if ( xgePassEnd(&pass) != XGE_OK ) pTest->iResult = 1;
	xgeEnd();
	xgeQuit();
	return XGE_OK;
}

int main(void)
{
	static const char* fonts[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf"
	};
	emoji_render_test_t test;
	xge_desc_t desc;
	FILE* file;
	int i;

	memset(&test, 0, sizeof(test));
	if ( !load_sequences(&test) ) return 1;
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = TARGET_WIDTH;
	desc.iHeight = TARGET_HEIGHT;
	desc.iFlags = XGE_INIT_OFFSCREEN;
	if ( xgeInit(&desc) != XGE_OK ) return 1;
	for ( i = 0; i < (int)(sizeof(fonts) / sizeof(fonts[0])); i++ ) {
		file = fopen(fonts[i], "rb");
		if ( file != NULL ) {
			fclose(file);
			if ( xgeFontLoad(&test.tFont, fonts[i], 18.0f) == XGE_OK ) break;
		}
	}
	if ( i == (int)(sizeof(fonts) / sizeof(fonts[0])) ||
	     xgeRenderTargetCreate(&test.tTarget, TARGET_WIDTH, TARGET_HEIGHT) != XGE_OK ) {
		xgeUnit();
		return 1;
	}
	(void)xgeRun(render_frame, &test);
	xgeRenderTargetFree(&test.tTarget);
	xgeFontFree(&test.tFont);
	xgeUnit();
	free(test.pSequences);
	free(test.pOffsets);
	free(test.pLengths);
	printf("emoji render-all test %s (%d entries)\n", test.iResult ? "failed" : "passed", test.iCount);
	return test.iResult;
}
