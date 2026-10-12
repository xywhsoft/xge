#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr, message) do { \
	if ( !(expr) ) { \
		printf("xui_code_word_wrap_test failed: %s\n", message); \
		iFailed = 1; \
		goto cleanup; \
	} \
} while ( 0 )

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pEditor = NULL;
	xui_font pFont = NULL;
	xui_code_edit_desc_t tDesc;
	xui_code_edit_text_hit_t tHit;
	xui_rect_t tFirst;
	xui_rect_t tWrapped;
	xui_rect_t tRoundTrip;
	const char* sText =
		"alpha beta gamma delta epsilon zeta eta theta iota kappa\n"
		"tab\tChinese: \xE4\xB8\xAD\xE6\x96\x87 emoji: \xF0\x9F\x98\x80 tail";
	int iFailed = 0;
	int iRet;
	int iWrappedOffset = 24;
	int iUtf8Offset;
	int iEmojiOffset;

	xuiTestProxyInit(&tState);
	iRet = xuiCreate(&pContext);
	CHECK(iRet == XUI_OK, "context create");
	CHECK(xuiSetProxy(pContext, &tState.tProxy) == XUI_OK, "proxy set");
	CHECK(tState.tProxy.fontLoadFile(&tState.tProxy, &pFont,
		"code-wrap.ttf", 14.0f, 0) == XUI_OK, "font create");
	CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK, "default font");
	CHECK(xuiInputViewport(pContext, 360.0f, 240.0f) == XUI_OK, "viewport");
	CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "root create");
	CHECK(xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 360, 240}) == XUI_OK,
		"root rect");
	CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pFont;
	tDesc.bWordWrap = 1;
	tDesc.iTabColumns = 4;
	iRet = xuiCodeEditCreate(pContext, &pEditor, &tDesc);
	CHECK(iRet == XUI_OK, "editor create");
	CHECK(xuiWidgetSetRect(pEditor, (xui_rect_t){20, 20, 150, 180}) == XUI_OK,
		"editor rect");
	CHECK(xuiWidgetAddChild(pRoot, pEditor) == XUI_OK, "editor attach");
	CHECK(xuiLayout(pContext) == XUI_OK, "layout");
	CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "update");

	CHECK(xuiCodeEditGetWordWrap(pEditor), "word wrap enabled");
	CHECK(!xuiWidgetGetVisible(xuiCodeEditGetHScrollBarWidget(pEditor)),
		"word wrap disables horizontal scrollbar");
	CHECK(xuiCodeEditGetTextOffsetRect(pEditor, 0, &tFirst) == XUI_OK,
		"first offset geometry");
	CHECK(xuiCodeEditGetTextOffsetRect(pEditor, iWrappedOffset, &tWrapped) == XUI_OK,
		"wrapped offset geometry");
	CHECK(tWrapped.fY > tFirst.fY, "long logical line creates visual rows");

	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	CHECK(xuiCodeEditHitTestText(pEditor, (float)tWrapped.fX,
		(float)(tWrapped.fY + tWrapped.fH / 2), &tHit) == XUI_OK,
		"wrapped hit test");
	CHECK(tHit.iByteOffset == iWrappedOffset, "offset geometry hit round trip");
	CHECK(tHit.iVisualLine > 0 && tHit.iLine == 0,
		"visual line remains mapped to logical line");
	CHECK(xuiCodeEditGetTextOffsetRect(pEditor, tHit.iByteOffset,
		&tRoundTrip) == XUI_OK && tRoundTrip.fX == tWrapped.fX &&
		tRoundTrip.fY == tWrapped.fY, "stable geometry round trip");

	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	CHECK(xuiCodeEditHitTestText(pEditor, 168.0f,
		(float)(tFirst.fY + tFirst.fH / 2), &tHit) == XUI_OK,
		"padding returns nearest insertion");
	CHECK(!tHit.bInsideText, "padding is not reported as text");

	iUtf8Offset = (int)(strstr(sText, "\xE4\xB8\xAD") - sText);
	CHECK(xuiCodeEditGetTextOffsetRect(pEditor, iUtf8Offset, &tRoundTrip) == XUI_OK,
		"UTF-8 offset geometry");
	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	CHECK(xuiCodeEditHitTestText(pEditor, (float)tRoundTrip.fX,
		(float)(tRoundTrip.fY + tRoundTrip.fH / 2), &tHit) == XUI_OK &&
		tHit.iByteOffset == iUtf8Offset && tHit.iClusterEnd - tHit.iClusterStart == 3,
		"Chinese grapheme geometry round trip");

	iEmojiOffset = (int)(strstr(sText, "\xF0\x9F\x98\x80") - sText);
	CHECK(xuiCodeEditGetTextOffsetRect(pEditor, iEmojiOffset, &tRoundTrip) == XUI_OK,
		"emoji offset geometry");
	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	CHECK(xuiCodeEditHitTestText(pEditor, (float)tRoundTrip.fX,
		(float)(tRoundTrip.fY + tRoundTrip.fH / 2), &tHit) == XUI_OK &&
		tHit.iByteOffset == iEmojiOffset && tHit.iClusterEnd - tHit.iClusterStart == 4,
		"emoji grapheme geometry round trip");

cleanup:
	xuiDestroy(pContext);
	if ( pFont != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	if ( iFailed ) return 1;
	printf("xui_code_word_wrap_test passed\n");
	return 0;
}
