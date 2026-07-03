#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_theme_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_theme pTheme;
	xui_code_style_t tStyle;
	xui_code_style_t tCustom;
	int iFailed;
	int iRet;

	pTheme = NULL;
	iFailed = 0;

	iRet = xuiCodeThemeCreate(&pTheme);
	XUI_TEST_CHECK(iRet == XUI_OK && pTheme != NULL, "theme create");

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

	memset(&tCustom, 0, sizeof(tCustom));
	tCustom.iSize = sizeof(tCustom);
	tCustom.iForeground = XUI_COLOR_RGBA(1, 2, 3, 255);
	tCustom.iBackground = XUI_COLOR_RGBA(4, 5, 6, 255);
	tCustom.iFlags = XUI_CODE_STYLE_UNDERLINE;
	iRet = xuiCodeThemeSetStyle(pTheme, XUI_CODE_STYLE_IDENTIFIER, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_OK, "set style");
	iRet = xuiCodeThemeMapTokenKind(pTheme, XUI_CODE_TOKEN_NUMBER, XUI_CODE_STYLE_IDENTIFIER);
	XUI_TEST_CHECK(iRet == XUI_OK, "map number token");
	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pTheme, XUI_CODE_TOKEN_NUMBER, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "mapped token style");
	XUI_TEST_CHECK(tStyle.iForeground == tCustom.iForeground, "mapped foreground");
	XUI_TEST_CHECK(tStyle.iBackground == tCustom.iBackground, "mapped background");
	XUI_TEST_CHECK(tStyle.iFlags == XUI_CODE_STYLE_UNDERLINE, "mapped flags");

	iRet = xuiCodeThemeSetStyle(pTheme, XUI_CODE_STYLE_CUSTOM_BASE, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "custom base out of table");
	iRet = xuiCodeThemeMapTokenKind(pTheme, XUI_CODE_TOKEN_CUSTOM_BASE, XUI_CODE_STYLE_DEFAULT);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "custom token out of table");

cleanup:
	xuiCodeThemeDestroy(pTheme);
	if ( iFailed ) return 1;
	printf("xui_code_theme_test passed\n");
	return 0;
}
