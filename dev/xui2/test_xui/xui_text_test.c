#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_text_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static const char* __xuiTextTestFontPath(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiTextTestLineEquals(xui_text_layout pLayout, int iLine, const char* sExpected)
{
	xui_text_line_t tLine;
	const char* sText;
	int iExpectedSize;

	if ( xuiTextLayoutGetLine(pLayout, iLine, &tLine) != XUI_OK ) {
		return 0;
	}
	sText = xuiTextLayoutGetText(pLayout);
	iExpectedSize = (int)strlen(sExpected);
	return (tLine.iTextSize == iExpectedSize) &&
	       (strncmp(sText + tLine.iTextOffset, sExpected, (size_t)iExpectedSize) == 0);
}

static int __xuiTextTestStatusAllowed(int iRet)
{
	return (iRet == XUI_OK) ||
	       (iRet == XUI_ERROR_NOT_INITIALIZED) ||
	       (iRet == XUI_ERROR_UNSUPPORTED) ||
	       (iRet == XUI_ERROR_BACKEND_FAILED) ||
	       (iRet == XUI_ERROR_GPU_FAILED);
}

int main(void)
{
	xge_desc_t tXgeDesc;
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_font pFont;
	xui_text_layout pLayout;
	xui_text_layout_desc_t tDesc;
	xui_font_metrics_t tMetrics;
	xui_vec2_t tMeasure;
	xui_vec2_t tLayoutSize;
	xui_text_line_t tLine;
	xui_surface_desc_t tSurfaceDesc;
	xui_surface pTarget;
	const char* sFontPath;
	int iRet;
	int iFailed;
	int iXgeReady;

	pContext = NULL;
	pFont = NULL;
	pLayout = NULL;
	pTarget = NULL;
	iFailed = 0;
	iXgeReady = 0;

	sFontPath = __xuiTextTestFontPath();
	XUI_TEST_CHECK(sFontPath != NULL, "font path not found");

	memset(&tXgeDesc, 0, sizeof(tXgeDesc));
	tXgeDesc.iWidth = 256;
	tXgeDesc.iHeight = 128;
	tXgeDesc.sTitle = "xui text test";
	tXgeDesc.iFlags = XGE_INIT_OFFSCREEN;
	tXgeDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tXgeDesc);
	XUI_TEST_CHECK(iRet == XGE_OK, "xgeInit failed");
	iXgeReady = 1;

	tProxy = xuiProxyXge();
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetProxy(pContext, &tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetProxy failed");
	iRet = tProxy.fontLoadFile(&tProxy, &pFont, sFontPath, 18.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pFont != NULL), "font load failed");
	iRet = tProxy.fontGetMetrics(&tProxy, pFont, &tMetrics);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMetrics.fLineHeight > 0.0f), "font metrics failed");

	iRet = tProxy.textMeasure(&tProxy, pFont, "alpha beta", &tMeasure);
	XUI_TEST_CHECK(iRet == XUI_OK, "text measure failed");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "alpha beta gamma";
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = tMeasure.fX + 0.5f;
	tDesc.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.fLineGap = 2.0f;
	tDesc.fParagraphGap = 6.0f;
	iRet = xuiTextLayoutCreate(pContext, &pLayout, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pLayout != NULL), "word layout create failed");
	XUI_TEST_CHECK(xuiTextLayoutGetLineCount(pLayout) == 2, "word wrap line count failed");
	XUI_TEST_CHECK(__xuiTextTestLineEquals(pLayout, 0, "alpha beta"), "word wrap line 0 failed");
	XUI_TEST_CHECK(__xuiTextTestLineEquals(pLayout, 1, "gamma"), "word wrap line 1 failed");
	iRet = xuiTextLayoutGetLine(pLayout, 0, &tLine);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tLine.iBreakType == XUI_TEXT_BREAK_WRAP), "word wrap break type failed");
	tLayoutSize = xuiTextLayoutGetSize(pLayout);
	XUI_TEST_CHECK((tLayoutSize.fX <= tDesc.fMaxWidth + 1.0f) && (tLayoutSize.fY > (tMetrics.fLineHeight * 2.0f)), "word layout size failed");

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 256;
	tSurfaceDesc.iHeight = 128;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = tProxy.surfaceCreate(&tProxy, &pTarget, &tSurfaceDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pTarget != NULL), "target create failed");
	iRet = tProxy.surfaceClear(&tProxy, pTarget, XUI_COLOR_RGBA(0, 0, 0, 0));
	XUI_TEST_CHECK(__xuiTextTestStatusAllowed(iRet), "target clear failed");
	iRet = xuiTextLayoutDraw(pLayout, pTarget, (xui_rect_t){0.0f, 0.0f, 240.0f, 96.0f}, XUI_COLOR_WHITE, XUI_TEXT_CLIP);
	XUI_TEST_CHECK(__xuiTextTestStatusAllowed(iRet), "layout draw failed");
	xuiTextLayoutDestroy(pLayout);
	pLayout = NULL;

	iRet = tProxy.textMeasure(&tProxy, pFont, "ab", &tMeasure);
	XUI_TEST_CHECK(iRet == XUI_OK, "char measure failed");
	tDesc.sText = "abcd";
	tDesc.fMaxWidth = tMeasure.fX + 0.5f;
	tDesc.iWrapMode = XUI_TEXT_WRAP_CHAR;
	iRet = xuiTextLayoutCreate(pContext, &pLayout, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiTextLayoutGetLineCount(pLayout) >= 2), "char wrap failed");
	xuiTextLayoutDestroy(pLayout);
	pLayout = NULL;

	tDesc.sText = "abcd";
	tDesc.fMaxWidth = 1.0f;
	tDesc.iWrapMode = XUI_TEXT_WRAP_NONE;
	iRet = xuiTextLayoutCreate(pContext, &pLayout, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiTextLayoutGetLineCount(pLayout) == 1), "nowrap count failed");
	tLayoutSize = xuiTextLayoutGetSize(pLayout);
	XUI_TEST_CHECK(tLayoutSize.fX > tDesc.fMaxWidth, "nowrap width failed");
	xuiTextLayoutDestroy(pLayout);
	pLayout = NULL;

	tDesc.sText = "a\n\nb";
	tDesc.fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	iRet = xuiTextLayoutCreate(pContext, &pLayout, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiTextLayoutGetLineCount(pLayout) == 3), "newline line count failed");
	iRet = xuiTextLayoutGetLine(pLayout, 1, &tLine);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tLine.iTextSize == 0) && (tLine.iBreakType == XUI_TEXT_BREAK_NEWLINE), "empty paragraph line failed");
	xuiTextLayoutDestroy(pLayout);
	pLayout = NULL;

	tDesc.sText = "alpha beta gamma";
	tDesc.fMaxWidth = 1.0f;
	tDesc.fMaxHeight = tMetrics.fLineHeight + 1.0f;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	iRet = xuiTextLayoutCreate(pContext, &pLayout, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiTextLayoutGetLineCount(pLayout) == 1) && xuiTextLayoutGetTruncated(pLayout), "max height truncation failed");
	xuiTextLayoutDestroy(pLayout);
	pLayout = NULL;

	iRet = xuiTextMeasureLayout(pContext, &tDesc, &tLayoutSize);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tLayoutSize.fY > 0.0f), "measure layout failed");

cleanup:
	if ( pLayout != NULL ) {
		xuiTextLayoutDestroy(pLayout);
	}
	if ( pTarget != NULL ) {
		tProxy.surfaceDestroy(&tProxy, pTarget);
	}
	if ( pFont != NULL ) {
		tProxy.fontDestroy(&tProxy, pFont);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iXgeReady ) {
		xgeUnit();
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_text_test passed\n");
	return 0;
}
