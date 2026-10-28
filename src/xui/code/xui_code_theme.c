#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#define XUI_CODE_THEME_TOKEN_BUILTIN_COUNT (XUI_CODE_TOKEN_ERROR + 1)

typedef struct xui_code_theme_style_entry_t {
	int iStyleId;
	xui_code_style_t tStyle;
} xui_code_theme_style_entry_t;

typedef struct xui_code_theme_token_entry_t {
	int iTokenKind;
	int iStyleId;
} xui_code_theme_token_entry_t;

struct xui_code_theme_t {
	xui_code_style_t arrStyles[XUI_CODE_STYLE_COUNT];
	int arrTokenStyle[XUI_CODE_THEME_TOKEN_BUILTIN_COUNT];
	xui_code_theme_style_entry_t* pCustomStyles;
	int iCustomStyleCount;
	int iCustomStyleCapacity;
	xui_code_theme_token_entry_t* pCustomTokens;
	int iCustomTokenCount;
	int iCustomTokenCapacity;
};

static int __xuiCodeThemeStyleIdValid(int iStyleId)
{
	return (iStyleId >= 0 && iStyleId < XUI_CODE_STYLE_COUNT) ||
		iStyleId >= XUI_CODE_STYLE_CUSTOM_BASE;
}

static int __xuiCodeThemeTokenKindValid(int iTokenKind)
{
	return (iTokenKind >= 0 && iTokenKind < XUI_CODE_THEME_TOKEN_BUILTIN_COUNT) ||
		iTokenKind >= XUI_CODE_TOKEN_CUSTOM_BASE;
}

static int __xuiCodeThemeReserve(void** ppItems, int* pCapacity,
	int iRequired, size_t iItemSize)
{
	void* pNew;
	int iCapacity;

	if ( ppItems == NULL || pCapacity == NULL || iRequired < 0 || iItemSize == 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iRequired <= *pCapacity ) return XUI_OK;
	iCapacity = (*pCapacity > 0) ? *pCapacity : 8;
	while ( iCapacity < iRequired ) {
		if ( iCapacity > INT_MAX / 2 ) {
			iCapacity = iRequired;
			break;
		}
		iCapacity *= 2;
	}
	if ( (size_t)iCapacity > SIZE_MAX / iItemSize ) return XUI_ERROR_LIMIT_EXCEEDED;
	pNew = xrtRealloc(*ppItems, (size_t)iCapacity * iItemSize);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	*ppItems = pNew;
	*pCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeThemeFindStyle(xui_code_theme pTheme, int iStyleId,
	int* pInsertIndex)
{
	int iLow;
	int iHigh;

	iLow = 0;
	iHigh = pTheme->iCustomStyleCount;
	while ( iLow < iHigh ) {
		int iMiddle = iLow + (iHigh - iLow) / 2;
		if ( pTheme->pCustomStyles[iMiddle].iStyleId < iStyleId ) iLow = iMiddle + 1;
		else iHigh = iMiddle;
	}
	if ( pInsertIndex != NULL ) *pInsertIndex = iLow;
	return iLow < pTheme->iCustomStyleCount &&
		pTheme->pCustomStyles[iLow].iStyleId == iStyleId;
}

static int __xuiCodeThemeFindToken(xui_code_theme pTheme, int iTokenKind,
	int* pInsertIndex)
{
	int iLow;
	int iHigh;

	iLow = 0;
	iHigh = pTheme->iCustomTokenCount;
	while ( iLow < iHigh ) {
		int iMiddle = iLow + (iHigh - iLow) / 2;
		if ( pTheme->pCustomTokens[iMiddle].iTokenKind < iTokenKind ) iLow = iMiddle + 1;
		else iHigh = iMiddle;
	}
	if ( pInsertIndex != NULL ) *pInsertIndex = iLow;
	return iLow < pTheme->iCustomTokenCount &&
		pTheme->pCustomTokens[iLow].iTokenKind == iTokenKind;
}

static int __xuiCodeThemeStyleExists(xui_code_theme pTheme, int iStyleId)
{
	if ( iStyleId >= 0 && iStyleId < XUI_CODE_STYLE_COUNT ) return 1;
	if ( iStyleId < XUI_CODE_STYLE_CUSTOM_BASE ) return 0;
	return __xuiCodeThemeFindStyle(pTheme, iStyleId, NULL);
}

XUI_API int xuiCodeThemeCreate(xui_code_theme* ppTheme)
{
	xui_code_theme pTheme;
	int iRet;

	if ( ppTheme == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppTheme = NULL;
	pTheme = (xui_code_theme)xrtMalloc(sizeof(*pTheme));
	if ( pTheme == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pTheme, 0, sizeof(*pTheme));
	iRet = xuiCodeThemeSetDefault(pTheme);
	if ( iRet != XUI_OK ) {
		xuiCodeThemeDestroy(pTheme);
		return iRet;
	}
	*ppTheme = pTheme;
	return XUI_OK;
}

XUI_API void xuiCodeThemeDestroy(xui_code_theme pTheme)
{
	if ( pTheme == NULL ) return;
	xrtFree(pTheme->pCustomStyles);
	xrtFree(pTheme->pCustomTokens);
	xrtFree(pTheme);
}

XUI_API int xuiCodeThemeSetDefault(xui_code_theme pTheme)
{
	int i;

	if ( pTheme == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	xrtFree(pTheme->pCustomStyles);
	xrtFree(pTheme->pCustomTokens);
	pTheme->pCustomStyles = NULL;
	pTheme->iCustomStyleCount = 0;
	pTheme->iCustomStyleCapacity = 0;
	pTheme->pCustomTokens = NULL;
	pTheme->iCustomTokenCount = 0;
	pTheme->iCustomTokenCapacity = 0;
	memset(pTheme->arrStyles, 0, sizeof(pTheme->arrStyles));
	for ( i = 0; i < XUI_CODE_STYLE_COUNT; i++ ) {
		pTheme->arrStyles[i].iSize = sizeof(pTheme->arrStyles[i]);
		pTheme->arrStyles[i].iForeground = XUI_COLOR_RGBA(36, 41, 47, 255);
		pTheme->arrStyles[i].iBackground = 0;
	}
	pTheme->arrStyles[XUI_CODE_STYLE_DEFAULT].iForeground = XUI_COLOR_RGBA(36, 41, 47, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_KEYWORD].iForeground = XUI_COLOR_RGBA(9, 105, 218, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_KEYWORD].iFlags = XUI_CODE_STYLE_BOLD;
	pTheme->arrStyles[XUI_CODE_STYLE_TYPE].iForeground = XUI_COLOR_RGBA(130, 80, 223, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_NUMBER].iForeground = XUI_COLOR_RGBA(17, 99, 41, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_STRING].iForeground = XUI_COLOR_RGBA(10, 91, 129, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_CHAR].iForeground = XUI_COLOR_RGBA(10, 91, 129, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_COMMENT].iForeground = XUI_COLOR_RGBA(110, 119, 129, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_COMMENT].iFlags = XUI_CODE_STYLE_ITALIC;
	pTheme->arrStyles[XUI_CODE_STYLE_PREPROCESSOR].iForeground = XUI_COLOR_RGBA(149, 56, 18, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_OPERATOR].iForeground = XUI_COLOR_RGBA(87, 96, 106, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_BRACE].iForeground = XUI_COLOR_RGBA(87, 96, 106, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_ERROR].iForeground = XUI_COLOR_RGBA(207, 34, 46, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_SELECTION].iBackground = XUI_COLOR_RGBA(74, 144, 239, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_CURRENT_LINE].iBackground = XUI_COLOR_RGBA(246, 248, 250, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_SEARCH_RESULT].iBackground = XUI_COLOR_RGBA(255, 235, 128, 190);
	pTheme->arrStyles[XUI_CODE_STYLE_MATCHED_BRACE].iBackground = XUI_COLOR_RGBA(221, 244, 255, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_MARGIN].iForeground = XUI_COLOR_RGBA(87, 96, 106, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_MARGIN].iBackground = XUI_COLOR_RGBA(246, 248, 250, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_LINE_NUMBER].iForeground = XUI_COLOR_RGBA(110, 119, 129, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_FOLD_ICON].iForeground = XUI_COLOR_RGBA(87, 96, 106, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_MARKER].iForeground = XUI_COLOR_RGBA(9, 105, 218, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_DIAGNOSTIC_ERROR].iForeground = XUI_COLOR_RGBA(207, 34, 46, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_DIAGNOSTIC_WARNING].iForeground = XUI_COLOR_RGBA(154, 103, 0, 255);
	pTheme->arrStyles[XUI_CODE_STYLE_DIAGNOSTIC_INFO].iForeground = XUI_COLOR_RGBA(9, 105, 218, 255);
	for ( i = 0; i < XUI_CODE_THEME_TOKEN_BUILTIN_COUNT; i++ ) {
		pTheme->arrTokenStyle[i] = XUI_CODE_STYLE_DEFAULT;
	}
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_KEYWORD] = XUI_CODE_STYLE_KEYWORD;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_TYPE] = XUI_CODE_STYLE_TYPE;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_IDENTIFIER] = XUI_CODE_STYLE_IDENTIFIER;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_NUMBER] = XUI_CODE_STYLE_NUMBER;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_STRING] = XUI_CODE_STYLE_STRING;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_CHAR] = XUI_CODE_STYLE_CHAR;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_COMMENT] = XUI_CODE_STYLE_COMMENT;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_PREPROCESSOR] = XUI_CODE_STYLE_PREPROCESSOR;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_OPERATOR] = XUI_CODE_STYLE_OPERATOR;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_BRACE] = XUI_CODE_STYLE_BRACE;
	pTheme->arrTokenStyle[XUI_CODE_TOKEN_ERROR] = XUI_CODE_STYLE_ERROR;
	return XUI_OK;
}

XUI_API int xuiCodeThemeSetStyle(xui_code_theme pTheme, int iStyleId,
	const xui_code_style_t* pStyle)
{
	xui_code_style_t tStyle;
	int iIndex;
	int iRet;

	if ( pTheme == NULL || !__xuiCodeThemeStyleIdValid(iStyleId) ||
		pStyle == NULL || pStyle->iSize < sizeof(*pStyle) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tStyle = *pStyle;
	tStyle.iSize = sizeof(tStyle);
	if ( iStyleId < XUI_CODE_STYLE_COUNT ) {
		pTheme->arrStyles[iStyleId] = tStyle;
		return XUI_OK;
	}
	if ( __xuiCodeThemeFindStyle(pTheme, iStyleId, &iIndex) ) {
		pTheme->pCustomStyles[iIndex].tStyle = tStyle;
		return XUI_OK;
	}
	if ( pTheme->iCustomStyleCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	iRet = __xuiCodeThemeReserve((void**)&pTheme->pCustomStyles,
		&pTheme->iCustomStyleCapacity, pTheme->iCustomStyleCount + 1,
		sizeof(*pTheme->pCustomStyles));
	if ( iRet != XUI_OK ) return iRet;
	memmove(&pTheme->pCustomStyles[iIndex + 1],
		&pTheme->pCustomStyles[iIndex],
		(size_t)(pTheme->iCustomStyleCount - iIndex) * sizeof(*pTheme->pCustomStyles));
	pTheme->pCustomStyles[iIndex].iStyleId = iStyleId;
	pTheme->pCustomStyles[iIndex].tStyle = tStyle;
	pTheme->iCustomStyleCount++;
	return XUI_OK;
}

XUI_API int xuiCodeThemeGetStyle(xui_code_theme pTheme, int iStyleId,
	xui_code_style_t* pStyle)
{
	int iIndex;

	if ( pTheme == NULL || !__xuiCodeThemeStyleIdValid(iStyleId) || pStyle == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iStyleId < XUI_CODE_STYLE_COUNT ) {
		*pStyle = pTheme->arrStyles[iStyleId];
	} else {
		if ( !__xuiCodeThemeFindStyle(pTheme, iStyleId, &iIndex) ) return XUI_ERROR_NOT_FOUND;
		*pStyle = pTheme->pCustomStyles[iIndex].tStyle;
	}
	pStyle->iSize = sizeof(*pStyle);
	return XUI_OK;
}

XUI_API int xuiCodeThemeMapTokenKind(xui_code_theme pTheme, int iTokenKind,
	int iStyleId)
{
	int iIndex;
	int iRet;

	if ( pTheme == NULL || !__xuiCodeThemeTokenKindValid(iTokenKind) ||
		!__xuiCodeThemeStyleIdValid(iStyleId) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiCodeThemeStyleExists(pTheme, iStyleId) ) return XUI_ERROR_NOT_FOUND;
	if ( iTokenKind < XUI_CODE_THEME_TOKEN_BUILTIN_COUNT ) {
		pTheme->arrTokenStyle[iTokenKind] = iStyleId;
		return XUI_OK;
	}
	if ( __xuiCodeThemeFindToken(pTheme, iTokenKind, &iIndex) ) {
		pTheme->pCustomTokens[iIndex].iStyleId = iStyleId;
		return XUI_OK;
	}
	if ( pTheme->iCustomTokenCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	iRet = __xuiCodeThemeReserve((void**)&pTheme->pCustomTokens,
		&pTheme->iCustomTokenCapacity, pTheme->iCustomTokenCount + 1,
		sizeof(*pTheme->pCustomTokens));
	if ( iRet != XUI_OK ) return iRet;
	memmove(&pTheme->pCustomTokens[iIndex + 1],
		&pTheme->pCustomTokens[iIndex],
		(size_t)(pTheme->iCustomTokenCount - iIndex) * sizeof(*pTheme->pCustomTokens));
	pTheme->pCustomTokens[iIndex].iTokenKind = iTokenKind;
	pTheme->pCustomTokens[iIndex].iStyleId = iStyleId;
	pTheme->iCustomTokenCount++;
	return XUI_OK;
}

XUI_API int xuiCodeThemeGetTokenStyle(xui_code_theme pTheme, int iTokenKind,
	xui_code_style_t* pStyle)
{
	int iIndex;
	int iStyleId;

	if ( pTheme == NULL || !__xuiCodeThemeTokenKindValid(iTokenKind) || pStyle == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iTokenKind < XUI_CODE_THEME_TOKEN_BUILTIN_COUNT ) {
		iStyleId = pTheme->arrTokenStyle[iTokenKind];
	} else if ( __xuiCodeThemeFindToken(pTheme, iTokenKind, &iIndex) ) {
		iStyleId = pTheme->pCustomTokens[iIndex].iStyleId;
	} else {
		iStyleId = XUI_CODE_STYLE_DEFAULT;
	}
	return xuiCodeThemeGetStyle(pTheme, iStyleId, pStyle);
}

XUI_API int xuiCodeThemeCopy(xui_code_theme pDst, xui_code_theme pSrc)
{
	xui_code_theme_style_entry_t* pStyles;
	xui_code_theme_token_entry_t* pTokens;

	if ( pDst == NULL || pSrc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDst == pSrc ) return XUI_OK;
	pStyles = NULL;
	pTokens = NULL;
	if ( pSrc->iCustomStyleCount > 0 ) {
		if ( (size_t)pSrc->iCustomStyleCount > SIZE_MAX / sizeof(*pStyles) ) {
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		pStyles = (xui_code_theme_style_entry_t*)xrtMalloc(
			(size_t)pSrc->iCustomStyleCount * sizeof(*pStyles));
		if ( pStyles == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		memcpy(pStyles, pSrc->pCustomStyles,
			(size_t)pSrc->iCustomStyleCount * sizeof(*pStyles));
	}
	if ( pSrc->iCustomTokenCount > 0 ) {
		if ( (size_t)pSrc->iCustomTokenCount > SIZE_MAX / sizeof(*pTokens) ) {
			xrtFree(pStyles);
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		pTokens = (xui_code_theme_token_entry_t*)xrtMalloc(
			(size_t)pSrc->iCustomTokenCount * sizeof(*pTokens));
		if ( pTokens == NULL ) {
			xrtFree(pStyles);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pTokens, pSrc->pCustomTokens,
			(size_t)pSrc->iCustomTokenCount * sizeof(*pTokens));
	}
	xrtFree(pDst->pCustomStyles);
	xrtFree(pDst->pCustomTokens);
	memcpy(pDst->arrStyles, pSrc->arrStyles, sizeof(pDst->arrStyles));
	memcpy(pDst->arrTokenStyle, pSrc->arrTokenStyle, sizeof(pDst->arrTokenStyle));
	pDst->pCustomStyles = pStyles;
	pDst->iCustomStyleCount = pSrc->iCustomStyleCount;
	pDst->iCustomStyleCapacity = pSrc->iCustomStyleCount;
	pDst->pCustomTokens = pTokens;
	pDst->iCustomTokenCount = pSrc->iCustomTokenCount;
	pDst->iCustomTokenCapacity = pSrc->iCustomTokenCount;
	return XUI_OK;
}

#endif
