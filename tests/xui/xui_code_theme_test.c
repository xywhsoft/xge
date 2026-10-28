#include "xui.h"

#include <stdio.h>
#include <string.h>

static int g_iThemeFailAllocation = -1;
static int g_bThemeAllocationFailed = 0;

static void* xuiTestThemeMalloc(size_t iSize)
{
	if ( g_iThemeFailAllocation >= 0 && g_iThemeFailAllocation-- == 0 ) {
		g_bThemeAllocationFailed = 1;
		return NULL;
	}
	return xrtMalloc(iSize);
}

static void* xuiTestThemeRealloc(void* pMemory, size_t iSize)
{
	if ( g_iThemeFailAllocation >= 0 && g_iThemeFailAllocation-- == 0 ) {
		g_bThemeAllocationFailed = 1;
		return NULL;
	}
	return xrtRealloc(pMemory, iSize);
}

#define xrtMalloc xuiTestThemeMalloc
#define xrtRealloc xuiTestThemeRealloc
#include "../../src/xui/code/xui_code_theme.c"
#undef xrtMalloc
#undef xrtRealloc

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_theme_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static xui_code_style_t xuiTestStyle(uint32_t iForeground)
{
	xui_code_style_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iForeground = iForeground;
	return tStyle;
}

int main(void)
{
	xui_code_theme pTheme;
	xui_code_theme pCopy;
	xui_code_style_t tStyle;
	xui_code_style_t tCustom;
	xui_code_style_t tPreserved;
	int iFailed;
	int iRet;
	int iFail;
	int bCopySucceeded;

	pTheme = NULL;
	pCopy = NULL;
	iFailed = 0;

	iRet = xuiCodeThemeCreate(&pTheme);
	XUI_TEST_CHECK(iRet == XUI_OK && pTheme != NULL, "theme create");
	iRet = xuiCodeThemeCreate(&pCopy);
	XUI_TEST_CHECK(iRet == XUI_OK && pCopy != NULL, "copy theme create");

	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetStyle(pTheme, XUI_CODE_STYLE_KEYWORD, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "keyword style");
	XUI_TEST_CHECK(tStyle.iSize == sizeof(tStyle), "keyword style size");
	XUI_TEST_CHECK((tStyle.iFlags & XUI_CODE_STYLE_BOLD) != 0, "keyword bold");
	XUI_TEST_CHECK(tStyle.iForeground != 0, "keyword foreground");

	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pTheme, XUI_CODE_TOKEN_COMMENT, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "comment token style");
	XUI_TEST_CHECK((tStyle.iFlags & XUI_CODE_STYLE_ITALIC) != 0, "comment italic");

	tCustom = xuiTestStyle(XUI_COLOR_RGBA(1, 2, 3, 255));
	tCustom.iBackground = XUI_COLOR_RGBA(4, 5, 6, 255);
	tCustom.iFlags = XUI_CODE_STYLE_UNDERLINE;
	iRet = xuiCodeThemeSetStyle(pTheme, XUI_CODE_STYLE_IDENTIFIER, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_OK, "set built-in style");
	iRet = xuiCodeThemeMapTokenKind(pTheme, XUI_CODE_TOKEN_NUMBER,
		XUI_CODE_STYLE_IDENTIFIER);
	XUI_TEST_CHECK(iRet == XUI_OK, "map number token");
	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pTheme, XUI_CODE_TOKEN_NUMBER, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK && tStyle.iForeground == tCustom.iForeground &&
		tStyle.iBackground == tCustom.iBackground &&
		tStyle.iFlags == XUI_CODE_STYLE_UNDERLINE, "mapped built-in token style");

	iRet = xuiCodeThemeSetStyle(pTheme, XUI_CODE_STYLE_CUSTOM_BASE + 7, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_OK, "set custom style");
	iRet = xuiCodeThemeMapTokenKind(pTheme, XUI_CODE_TOKEN_CUSTOM_BASE + 9,
		XUI_CODE_STYLE_CUSTOM_BASE + 7);
	XUI_TEST_CHECK(iRet == XUI_OK, "map custom token");
	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pTheme, XUI_CODE_TOKEN_CUSTOM_BASE + 9, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK && tStyle.iForeground == tCustom.iForeground,
		"custom token style");

	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pTheme, XUI_CODE_TOKEN_CUSTOM_BASE + 10, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "unmapped custom token fallback");
	XUI_TEST_CHECK(tStyle.iForeground == XUI_COLOR_RGBA(36, 41, 47, 255),
		"unmapped custom token uses default");
	iRet = xuiCodeThemeMapTokenKind(pTheme, XUI_CODE_TOKEN_CUSTOM_BASE + 11,
		XUI_CODE_STYLE_CUSTOM_BASE + 99);
	XUI_TEST_CHECK(iRet == XUI_ERROR_NOT_FOUND, "undefined custom style rejected");
	iRet = xuiCodeThemeSetStyle(pTheme, XUI_CODE_STYLE_COUNT, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reserved style id rejected");
	iRet = xuiCodeThemeMapTokenKind(pTheme, XUI_CODE_TOKEN_ERROR + 1,
		XUI_CODE_STYLE_DEFAULT);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reserved token id rejected");
	tStyle = tCustom;
	tStyle.iSize = sizeof(tStyle) - 1;
	iRet = xuiCodeThemeSetStyle(pTheme, XUI_CODE_STYLE_DEFAULT, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "short style structure rejected");

	tPreserved = xuiTestStyle(XUI_COLOR_RGBA(90, 80, 70, 255));
	iRet = xuiCodeThemeSetStyle(pCopy, XUI_CODE_STYLE_CUSTOM_BASE + 1, &tPreserved);
	XUI_TEST_CHECK(iRet == XUI_OK, "set preserved destination style");
	bCopySucceeded = 0;
	for ( iFail = 0; iFail < 4; iFail++ ) {
		g_iThemeFailAllocation = iFail;
		g_bThemeAllocationFailed = 0;
		iRet = xuiCodeThemeCopy(pCopy, pTheme);
		g_iThemeFailAllocation = -1;
		if ( iRet == XUI_OK ) {
			bCopySucceeded = 1;
			break;
		}
		XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && g_bThemeAllocationFailed,
			"copy allocation failure");
		memset(&tStyle, 0, sizeof(tStyle));
		iRet = xuiCodeThemeGetStyle(pCopy, XUI_CODE_STYLE_CUSTOM_BASE + 1, &tStyle);
		XUI_TEST_CHECK(iRet == XUI_OK && tStyle.iForeground == tPreserved.iForeground,
			"failed copy preserves destination");
	}
	XUI_TEST_CHECK(bCopySucceeded, "theme copy eventually succeeds");
	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pCopy, XUI_CODE_TOKEN_CUSTOM_BASE + 9, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK && tStyle.iForeground == tCustom.iForeground,
		"copied custom mapping");
	XUI_TEST_CHECK(xuiCodeThemeCopy(pCopy, pCopy) == XUI_OK, "self copy");

	iRet = xuiCodeThemeSetDefault(pCopy);
	XUI_TEST_CHECK(iRet == XUI_OK, "theme reset");
	iRet = xuiCodeThemeGetStyle(pCopy, XUI_CODE_STYLE_CUSTOM_BASE + 7, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_ERROR_NOT_FOUND, "theme reset clears custom styles");
	g_iThemeFailAllocation = 0;
	g_bThemeAllocationFailed = 0;
	iRet = xuiCodeThemeSetStyle(pCopy, XUI_CODE_STYLE_CUSTOM_BASE + 1, &tPreserved);
	g_iThemeFailAllocation = -1;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && g_bThemeAllocationFailed,
		"custom style reserve failure");
	iRet = xuiCodeThemeGetStyle(pCopy, XUI_CODE_STYLE_CUSTOM_BASE + 1, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_ERROR_NOT_FOUND,
		"custom style reserve failure leaves theme unchanged");
	iRet = xuiCodeThemeSetStyle(pCopy, XUI_CODE_STYLE_CUSTOM_BASE + 1, &tPreserved);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom style after reserve failure");
	g_iThemeFailAllocation = 0;
	g_bThemeAllocationFailed = 0;
	iRet = xuiCodeThemeMapTokenKind(pCopy, XUI_CODE_TOKEN_CUSTOM_BASE + 1,
		XUI_CODE_STYLE_CUSTOM_BASE + 1);
	g_iThemeFailAllocation = -1;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && g_bThemeAllocationFailed,
		"custom token reserve failure");
	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pCopy, XUI_CODE_TOKEN_CUSTOM_BASE + 1, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		tStyle.iForeground == XUI_COLOR_RGBA(36, 41, 47, 255),
		"custom token reserve failure leaves default fallback");

cleanup:
	g_iThemeFailAllocation = -1;
	xuiCodeThemeDestroy(pCopy);
	xuiCodeThemeDestroy(pTheme);
	if ( iFailed ) return 1;
	printf("xui_code_theme_test passed\n");
	return 0;
}
