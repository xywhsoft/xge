#include "xui_config.h"
#if XUI_ENABLE_FILE_DIALOG
#include "xui_internal.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>
#endif

#define XUI_FILE_DIALOG_MAGIC 0x58464447u
#define XUI_FILE_DIALOG_ENTRY_GROW 32
#define XUI_FILE_DIALOG_FILTER_GROW 8
#define XUI_FILE_DIALOG_ROOT_GROW 16
#define XUI_FILE_DIALOG_MIN_WIDTH 560.0f
#define XUI_FILE_DIALOG_MIN_HEIGHT 430.0f
#define XUI_FILE_DIALOG_ICON_FOLDER "file_dialog_folder"
#define XUI_FILE_DIALOG_ICON_FILE "file_dialog_file"
#define XUI_FILE_DIALOG_ICON_DOCUMENT "file_dialog_document"
#define XUI_FILE_DIALOG_ICON_IMAGE "file_dialog_image"
#define XUI_FILE_DIALOG_ICON_SCRIPT "file_dialog_script"
#define XUI_FILE_DIALOG_ICON_UP "file_dialog_up"
#define XUI_FILE_DIALOG_ICON_REFRESH "file_dialog_refresh"
#define XUI_FILE_DIALOG_ICON_USER "file_dialog_user"
#define XUI_FILE_DIALOG_ICON_DOCUMENTS "file_dialog_documents"
#define XUI_FILE_DIALOG_ICON_PICTURES "file_dialog_pictures"
#define XUI_FILE_DIALOG_ICON_MUSIC "file_dialog_music"
#define XUI_FILE_DIALOG_ICON_VIDEOS "file_dialog_videos"
#define XUI_FILE_DIALOG_ICON_DOWNLOADS "file_dialog_downloads"
#define XUI_FILE_DIALOG_ICON_DESKTOP "file_dialog_desktop"
#define XUI_FILE_DIALOG_ICON_COMPUTER "file_dialog_computer"
#define XUI_FILE_DIALOG_ICON_DRIVE "file_dialog_drive"

enum {
	XUI_FILE_DIALOG_ROOT_USER = 1,
	XUI_FILE_DIALOG_ROOT_DOCUMENTS,
	XUI_FILE_DIALOG_ROOT_PICTURES,
	XUI_FILE_DIALOG_ROOT_MUSIC,
	XUI_FILE_DIALOG_ROOT_VIDEOS,
	XUI_FILE_DIALOG_ROOT_DOWNLOADS,
	XUI_FILE_DIALOG_ROOT_DESKTOP,
	XUI_FILE_DIALOG_ROOT_COMPUTER = 100,
	XUI_FILE_DIALOG_ROOT_DRIVE_FIRST = 1000
};

typedef struct xui_file_dialog_entry_t {
	char* sName;
	char* sPath;
	int bDir;
} xui_file_dialog_entry_t;

typedef struct xui_file_dialog_root_t {
	int iNodeId;
	int iParentId;
	char* sName;
	char* sPath;
	const char* sIconAsset;
	int bExpanded;
	int bShowInRoot;
} xui_file_dialog_root_t;

struct xui_file_dialog_t {
	uint32_t iMagic;
	xui_context pContext;
	xui_widget pWindow;
	xui_widget pBackdrop;
	xui_widget pPathRow;
	xui_widget pListRow;
	xui_widget pFieldRow;
	xui_widget pButtonRow;
	xui_widget pButtonSpacer;
	xui_widget pPathBar;
	xui_widget pPathBreadcrumb;
	xui_widget pRootList;
	xui_widget pFileList;
	uint32_t iWindowStyleHash, iListStyleHash;
	uint32_t arrListBase[8];
	int bWindowPaintReady, bListPaintReady;
	xui_widget pPathInput;
	xui_widget pNameInput;
	xui_widget pFilterCombo;
	xui_widget pUpButton;
	xui_widget pRefreshButton;
	xui_widget pOkButton;
	xui_widget pCancelButton;
	xui_msgbox pOverwriteBox;
	xui_widget pPathLabel;
	xui_widget pNameLabel;
	xui_msgtip pPathTip;
	xui_font pFont;
	xui_widget_event_proc onPathInputBlur;
	void* pPathInputBlurUser;
	xui_widget_event_proc onWindowEvent;
	void* pWindowEventUser;
	xui_file_dialog_result_proc onResult;
	void* pResultUser;
	xui_file_dialog_context_proc onContext;
	void* pContextUser;
	char* sCurrentDir;
	char* sFileName;
	char* sFilter;
	char* sResultPath;
	char* sPendingSavePath;
	char** arrFilterNames;
	char** arrFilterPatterns;
	int iFilterCount;
	int iFilterCapacity;
	int iSelectedFilter;
	xui_file_dialog_root_t* arrRoots;
	int iRootCount;
	int iRootCapacity;
	xui_file_dialog_entry_t* arrEntries;
	int iEntryCount;
	int iEntryCapacity;
	int iSelectedEntry;
	int iMode;
	int iResult;
	int iModalZ;
	int bModal;
	int bOpen;
	int bPathEditMode;
	int bPathApplying;
	int bInternalNameChange;
	float fWidth;
	float fHeight;
};

static int __xuiFileDialogRefreshInternal(xui_file_dialog pDialog, int bRefreshRoots);
static int __xuiFileDialogSyncRootTree(xui_file_dialog pDialog);

static int __xuiFileDialogValid(xui_file_dialog pDialog)
{
	return (pDialog != NULL) && (pDialog->iMagic == XUI_FILE_DIALOG_MAGIC);
}

static void __xuiFileDialogDestroyOrphan(xui_widget pWidget)
{
	if ( xuiInternalWidgetIsValid(pWidget) && xuiWidgetGetParent(pWidget) == NULL ) {
		xuiWidgetDestroy(pWidget);
	}
}

static uint32_t __xuiFileDialogStyleColor(xui_widget pWidget, const char* sName, uint32_t iBase)
{
	xui_style_property_t tProperty;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	if ( xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty) == XUI_OK &&
		tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR ) return tProperty.tValue.iColor;
	return iBase;
}

static xui_widget_type __xuiFileDialogType(xui_context pContext, const char* sName, xui_widget_type pParent)
{
	xui_widget_type pType = xuiWidgetFindType(pContext, sName);
	xui_widget_type_desc_t tDesc;
	if ( pType != NULL ) return pType;
	if ( pParent == NULL ) return NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = sName;
	tDesc.pParent = pParent;
	tDesc.iTypeDataSize = pParent->iTypeDataSize + sizeof(xui_file_dialog);
	return xuiWidgetRegisterType(pContext, &pType, &tDesc) == XUI_OK ? pType : NULL;
}

static void __xuiFileDialogRegisterColor(xui_context pContext, xui_widget_type pType, const char* sName, int bPrivate)
{
	xui_style_property_info_t tInfo;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.pWidgetType = pType;
	tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
	tInfo.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	tInfo.iFlags = XUI_STYLE_PROPERTY_INHERITED | (bPrivate ? XUI_STYLE_PROPERTY_PRIVATE : 0);
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiFileDialogThemeDefaults(xui_context pContext, xui_widget_type pType,
	const char* const* arrNames, const char* const* arrTokens, int iCount)
{
	xui_style_property_t arrProperties[5];
	xui_style_desc_t tStyle;
	int i;
	memset(arrProperties, 0, sizeof(arrProperties));
	memset(&tStyle, 0, sizeof(tStyle));
	for ( i = 0; i < iCount; ++i ) {
		__xuiFileDialogRegisterColor(pContext, pType, arrNames[i], 1);
		arrProperties[i].iSize = sizeof(arrProperties[i]);
		arrProperties[i].sName = arrNames[i];
		arrProperties[i].tValue.iSize = sizeof(arrProperties[i].tValue);
		arrProperties[i].tValue.iType = XUI_STYLE_VALUE_TOKEN;
		arrProperties[i].tValue.sText = arrTokens[i];
	}
	tStyle.iSize = sizeof(tStyle);
	tStyle.pProperties = arrProperties;
	tStyle.iPropertyCount = iCount;
	(void)xuiStyleSetType(pContext, pType, &tStyle);
}

static void __xuiFileDialogRegisterColors(xui_context pContext, xui_widget_type pPathType, xui_widget_type pListType)
{
	static const char* arrNames[] = {"file_dialog.row.color", "file_dialog.row.hover_color",
		"file_dialog.row.selected_color", "file_dialog.focus.color", "file_dialog.text.color",
		"file_dialog.text.disabled_color", "file_dialog.text.selected_color", "file_dialog.icon.color"};
	static const char* arrPathNames[] = {"file_dialog.theme.panel", "file_dialog.theme.border"};
	static const char* arrPathTokens[] = {"theme.panel", "theme.border"};
	static const char* arrListNames[] = {"file_dialog.theme.text", "file_dialog.theme.disabled",
		"file_dialog.theme.hover", "file_dialog.theme.selected", "file_dialog.theme.focus"};
	static const char* arrListTokens[] = {"theme.text", "theme.state.disabled", "theme.state.hover", "theme.selection", "theme.state.focus"};
	size_t i;
	__xuiFileDialogRegisterColor(pContext, pPathType, "file_dialog.path.background.color", 0);
	__xuiFileDialogRegisterColor(pContext, pPathType, "file_dialog.path.border.color", 0);
	for ( i = 0; i < sizeof(arrNames) / sizeof(arrNames[0]); ++i )
		__xuiFileDialogRegisterColor(pContext, pListType, arrNames[i], 0);
	/* Private ancestor rules keep token dependencies when a public type rule is replaced. */
	__xuiFileDialogThemeDefaults(pContext, pPathType->pParent, arrPathNames, arrPathTokens, 2);
	__xuiFileDialogThemeDefaults(pContext, pListType->pParent, arrListNames, arrListTokens, 5);
}

static uint32_t __xuiFileDialogRowColor(xui_file_dialog pDialog, int iIndex, uint32_t iBase,
	const char* sNative, const char* sName, const char* sTheme)
{
	if ( sTheme != NULL && iBase == pDialog->arrListBase[iIndex] )
		iBase = __xuiFileDialogStyleColor(pDialog->pFileList, sTheme, iBase);
	iBase = __xuiFileDialogStyleColor(pDialog->pFileList, sNative, iBase);
	return __xuiFileDialogStyleColor(pDialog->pFileList, sName, iBase);
}

static int __xuiFileDialogListPreparePaint(xui_widget pWidget)
{
	xui_context pContext = xuiWidgetGetContext(pWidget);
	xui_widget_type pType = xuiWidgetFindType(pContext, "file-dialog-list");
	xui_widget_type pParent = pType->pParent;
	xui_file_dialog pDialog;
	uint32_t iHash;
	int iRet;
	for ( ; pParent != NULL && pParent->onPreparePaint == NULL; pParent = pParent->pParent ) {}
	if ( pParent != NULL ) {
		iRet = pParent->onPreparePaint(pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( !xuiInternalWidgetIsValid(pWidget) || xuiInternalContextDestroyPending(pContext) || !xuiWidgetGetVisible(pWidget) ) return XUI_OK;
	}
	memcpy(&pDialog, (char*)xuiWidgetGetTypeData(pWidget) + pType->pParent->iTypeDataSize, sizeof(pDialog));
	if ( !__xuiFileDialogValid(pDialog) ) return XUI_OK;
	iHash = xuiWidgetGetStyleHash(pWidget);
	if ( !pDialog->bListPaintReady || pDialog->iListStyleHash != iHash ) {
		iRet = xuiInternalWidgetInvalidatePaintIfNeeded(
			xuiListViewGetViewportWidget(pWidget));
		if ( iRet != XUI_OK ) return iRet;
		pDialog->bListPaintReady = 1;
		pDialog->iListStyleHash = iHash;
	}
	return XUI_OK;
}

static void __xuiFileDialogResolveChildren(xui_widget pWidget)
{
	xui_widget pChild;
	for ( pChild = xuiWidgetGetFirstChild(pWidget); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild) ) {
		(void)xuiWidgetResolveStyle(pChild);
		__xuiFileDialogResolveChildren(pChild);
	}
}

static int __xuiFileDialogWindowPreparePaint(xui_widget pWidget)
{
	xui_context pContext = xuiWidgetGetContext(pWidget);
	xui_widget_type pType = xuiWidgetFindType(pContext, "file-dialog");
	xui_widget_type pParent = pType->pParent;
	xui_file_dialog pDialog;
	uint32_t iHash;
	int iRet;
	/* Window's hook is not automatically chained for derived service types. */
	for ( ; pParent != NULL && pParent->onPreparePaint == NULL; pParent = pParent->pParent ) {}
	if ( pParent != NULL ) {
		iRet = pParent->onPreparePaint(pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( !xuiInternalWidgetIsValid(pWidget) || xuiInternalContextDestroyPending(pContext) || !xuiWidgetGetVisible(pWidget) ) return XUI_OK;
	}
	memcpy(&pDialog, (char*)xuiWidgetGetTypeData(pWidget) + pType->pParent->iTypeDataSize, sizeof(pDialog));
	if ( !__xuiFileDialogValid(pDialog) ) return XUI_OK;
	iHash = xuiWidgetGetStyleHash(pWidget);
	if ( !pDialog->bWindowPaintReady || pDialog->iWindowStyleHash != iHash ) {
		__xuiFileDialogResolveChildren(pWidget);
		iRet = xuiInternalWidgetInvalidatePaintIfNeeded(pDialog->pPathBar);
		if ( iRet == XUI_OK ) iRet = xuiInternalWidgetInvalidatePaintIfNeeded(
			xuiTreeViewGetViewportWidget(pDialog->pRootList));
		if ( iRet == XUI_OK ) iRet = xuiInternalWidgetInvalidatePaintIfNeeded(
			xuiListViewGetViewportWidget(pDialog->pFileList));
		if ( iRet != XUI_OK ) return iRet;
		pDialog->bWindowPaintReady = 1;
		pDialog->iWindowStyleHash = iHash;
	}
	return XUI_OK;
}

static char* __xuiFileDialogDup(const char* sText)
{
	char* sCopy;
	size_t iLen;

	if ( sText == NULL ) {
		sText = "";
	}
	iLen = strlen(sText);
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iLen + 1u);
	return sCopy;
}

static int __xuiFileDialogSetString(char** psDst, const char* sText)
{
	char* sNew;

	if ( psDst == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sNew = __xuiFileDialogDup(sText);
	if ( sNew == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( *psDst != NULL ) {
		xrtFree(*psDst);
	}
	*psDst = sNew;
	return XUI_OK;
}

static int __xuiFileDialogGrowCapacity(int iCurrent, int iNeed, int iGrow,
	size_t iElementSize, int* pCapacity)
{
	int iCapacity;

	if ( iCurrent < 0 || iNeed < 0 || iGrow <= 0 || iElementSize == 0u ||
	     pCapacity == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iCapacity = iCurrent > 0 ? iCurrent : iGrow;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > INT_MAX - iGrow ) {
			iCapacity = iNeed;
			break;
		}
		iCapacity += iGrow;
	}
	if ( iCapacity < iNeed || (size_t)iCapacity > SIZE_MAX / iElementSize )
		return XUI_ERROR_LIMIT_EXCEEDED;
	*pCapacity = iCapacity;
	return XUI_OK;
}

static char* __xuiFileDialogDupRange(const char* sText, size_t iLen)
{
	char* sCopy;

	if ( sText == NULL ) {
		return __xuiFileDialogDup("");
	}
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iLen);
	sCopy[iLen] = 0;
	return sCopy;
}

static int __xuiFileDialogIsSlash(char c)
{
	return c == '/' || c == '\\';
}

static int __xuiFileDialogPathRootLen(const char* sPath)
{
	size_t iLen;
	size_t iPos;
	int iSlashCount;

	if ( sPath == NULL || sPath[0] == 0 ) {
		return 0;
	}
	iLen = strlen(sPath);
	if ( iLen >= 2u && sPath[1] == ':' ) {
		return (iLen >= 3u && __xuiFileDialogIsSlash(sPath[2])) ? 3 : 2;
	}
	if ( __xuiFileDialogIsSlash(sPath[0]) ) {
		if ( iLen >= 2u && __xuiFileDialogIsSlash(sPath[1]) ) {
			iSlashCount = 0;
			for ( iPos = 0u; iPos < iLen; iPos++ ) {
				if ( __xuiFileDialogIsSlash(sPath[iPos]) ) {
					iSlashCount++;
					if ( iSlashCount >= 4 ) return (int)(iPos + 1u);
				}
			}
		}
		return 1;
	}
	return 0;
}

static int __xuiFileDialogModeValid(int iMode)
{
	return (iMode == XUI_FILE_DIALOG_MODE_OPEN_FILE) ||
	       (iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE) ||
	       (iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER);
}

static int __xuiFileDialogDescValid(const xui_file_dialog_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iMode != 0) && !__xuiFileDialogModeValid(pDesc->iMode) ) {
		return 0;
	}
	if ( !isfinite(pDesc->fWidth) || !isfinite(pDesc->fHeight) ||
	     (pDesc->fWidth < 0.0f) || (pDesc->fHeight < 0.0f) ) {
		return 0;
	}
	return 1;
}

static const char* __xuiFileDialogDefaultTitle(xui_context pContext, int iMode)
{
	if ( iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE ) return xuiTranslate(pContext, XUI_TR_FILE_SAVE_FILE);
	if ( iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) return xuiTranslate(pContext, XUI_TR_FILE_SELECT_FOLDER);
	return xuiTranslate(pContext, XUI_TR_FILE_OPEN_FILE);
}

static const char* __xuiFileDialogOkText(xui_context pContext, int iMode)
{
	if ( iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE ) return xuiTranslate(pContext, XUI_TR_FILE_SAVE);
	if ( iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) return xuiTranslate(pContext, XUI_TR_FILE_SELECT);
	return xuiTranslate(pContext, XUI_TR_FILE_OPEN);
}

static int __xuiFileDialogTextEquals(const char* sLeft, const char* sRight, size_t iRightSize)
{
	size_t i;

	if ( sLeft == NULL || sRight == NULL ) {
		return 0;
	}
	if ( strlen(sLeft) != iRightSize ) {
		return 0;
	}
	for ( i = 0; i < iRightSize; i++ ) {
		if ( tolower((unsigned char)sLeft[i]) != tolower((unsigned char)sRight[i]) ) {
			return 0;
		}
	}
	return 1;
}

static int __xuiFileDialogExtIn(const char* sExt, const char* const* arrExts, int iCount)
{
	int i;

	if ( sExt == NULL || arrExts == NULL || iCount <= 0 ) {
		return 0;
	}
	if ( sExt[0] == '.' ) {
		sExt++;
	}
	for ( i = 0; i < iCount; i++ ) {
		if ( __xuiFileDialogTextEquals(sExt, arrExts[i], strlen(arrExts[i])) ) {
			return 1;
		}
	}
	return 0;
}

static void __xuiFileDialogTrimSpan(const char** psText, size_t* pLen)
{
	const char* sText;
	size_t iLen;

	if ( psText == NULL || pLen == NULL || *psText == NULL ) {
		return;
	}
	sText = *psText;
	iLen = *pLen;
	while ( iLen > 0u && isspace((unsigned char)sText[0]) ) {
		sText++;
		iLen--;
	}
	while ( iLen > 0u && isspace((unsigned char)sText[iLen - 1u]) ) {
		iLen--;
	}
	*psText = sText;
	*pLen = iLen;
}

static const char* __xuiFileDialogIconAsset(const xui_file_dialog_entry_t* pEntry)
{
	static const char* const arrDocumentExts[] = {
		"txt", "text", "md", "markdown", "json", "xson", "xml", "yaml", "yml",
		"ini", "cfg", "conf", "log", "csv", "tsv", "rtf", "doc", "docx",
		"odt", "pdf", "html", "htm", "xhtml", "css", "scss", "less"
	};
	static const char* const arrImageExts[] = {
		"bmp", "dib", "jpg", "jpeg", "jfif", "png", "apng", "gif", "webp",
		"ico", "cur", "svg", "tif", "tiff", "tga", "psd", "avif", "heic",
		"heif", "raw", "dds", "exr", "hdr"
	};
	static const char* const arrScriptExts[] = {
		"c", "h", "cc", "cpp", "cxx", "hpp", "hxx", "m", "mm", "cs", "java",
		"js", "jsx", "ts", "tsx", "py", "pyw", "lua", "xs", "go", "rs",
		"php", "rb", "pl", "pm", "sh", "bash", "zsh", "ps1", "bat", "cmd",
		"sql", "swift", "kt", "kts", "scala", "r", "vue", "svelte", "glsl",
		"hlsl", "metal", "asm", "s"
	};
	const char* sExt;

	if ( pEntry == NULL ) {
		return XUI_FILE_DIALOG_ICON_FILE;
	}
	if ( pEntry->bDir ) {
		return XUI_FILE_DIALOG_ICON_FOLDER;
	}
	sExt = strrchr(pEntry->sName != NULL ? pEntry->sName : "", '.');
	if ( sExt != NULL ) {
		if ( __xuiFileDialogExtIn(sExt, arrDocumentExts, (int)(sizeof(arrDocumentExts) / sizeof(arrDocumentExts[0]))) ) {
			return XUI_FILE_DIALOG_ICON_DOCUMENT;
		}
		if ( __xuiFileDialogExtIn(sExt, arrImageExts, (int)(sizeof(arrImageExts) / sizeof(arrImageExts[0]))) ) {
			return XUI_FILE_DIALOG_ICON_IMAGE;
		}
		if ( __xuiFileDialogExtIn(sExt, arrScriptExts, (int)(sizeof(arrScriptExts) / sizeof(arrScriptExts[0]))) ) {
			return XUI_FILE_DIALOG_ICON_SCRIPT;
		}
	}
	return XUI_FILE_DIALOG_ICON_FILE;
}

static float __xuiFileDialogMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiFileDialogSetButtonIcon(xui_file_dialog pDialog, xui_widget pButton, const char* sAsset)
{
	xui_surface pAtlas;
	xui_rect_t tSrc;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pButton == NULL || sAsset == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiBuiltinAssetGetAtlas(pDialog->pContext, &pAtlas);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiBuiltinAssetGetRect(sAsset, &tSrc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiButtonSetIcon(pButton, pAtlas, tSrc);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetIconLayout(pButton, XUI_BUTTON_ICON_LEFT, 16.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetIconColor(pButton, XUI_COLOR_WHITE);
	return iRet;
}

static int __xuiFileDialogDrawFileItem(xui_widget pWidget, int iIndex, xui_draw_context pDraw, xui_rect_t tRow, int iState, void* pUser)
{
	xui_file_dialog pDialog;
	xui_file_dialog_entry_t* pEntry;
	xui_proxy pProxy;
	xui_surface pAtlas;
	xui_rect_t tFill;
	xui_rect_t tIconSrc;
	xui_rect_t tIconDst;
	xui_rect_t tText;
	const char* sAsset;
	uint32_t iTextColor;
	uint32_t iRow, iHover, iSelected, iFocus, iDisabled, iSelectedText;
	int iRet;

	pDialog = (xui_file_dialog)pUser;
	if ( !__xuiFileDialogValid(pDialog) || pWidget == NULL || pDraw == NULL || iIndex < 0 || iIndex >= pDialog->iEntryCount ) {
		return 0;
	}
	pEntry = &pDialog->arrEntries[iIndex];
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL || pProxy->drawSurface == NULL || pProxy->drawText == NULL ||
	     pProxy->drawRectFill == NULL ) {
		return 0;
	}
	iRet = xuiBuiltinAssetGetAtlas(xuiWidgetGetContext(pWidget), &pAtlas);
	if ( iRet != XUI_OK || pAtlas == NULL ) {
		return 0;
	}
	sAsset = __xuiFileDialogIconAsset(pEntry);
	iRet = xuiBuiltinAssetGetRect(sAsset, &tIconSrc);
	if ( iRet != XUI_OK ) {
		return 0;
	}
	(void)xuiListViewGetColors(pWidget, NULL, NULL, &iFocus, &iRow, &iHover, &iSelected, &iTextColor, &iDisabled);
	iSelectedText = iTextColor == pDialog->arrListBase[6] ? XUI_COLOR_WHITE : iTextColor;
	iFocus = __xuiFileDialogRowColor(pDialog, 2, iFocus, "listview.focus.color", "file_dialog.focus.color", "file_dialog.theme.focus");
	iRow = __xuiFileDialogRowColor(pDialog, 3, iRow, "listview.row.color", "file_dialog.row.color", NULL);
	iHover = __xuiFileDialogRowColor(pDialog, 4, iHover, "listview.row.hover_color", "file_dialog.row.hover_color", "file_dialog.theme.hover");
	iSelected = __xuiFileDialogRowColor(pDialog, 5, iSelected, "listview.row.selected_color", "file_dialog.row.selected_color", "file_dialog.theme.selected");
	iTextColor = __xuiFileDialogRowColor(pDialog, 6, iTextColor, "listview.text.color", "file_dialog.text.color", "file_dialog.theme.text");
	iDisabled = __xuiFileDialogRowColor(pDialog, 7, iDisabled, "listview.text.disabled_color", "file_dialog.text.disabled_color", "file_dialog.theme.disabled");
	iSelectedText = __xuiFileDialogStyleColor(pWidget, "listview.text.selected_color", iSelectedText);
	iSelectedText = __xuiFileDialogStyleColor(pWidget, "file_dialog.text.selected_color", iSelectedText);
	if ( (iRow & 0xffu) != 0u ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tRow, iRow);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iState & XUI_LIST_ITEM_SELECTED) != 0 ) {
		tFill = xuiInternalSnapRect((xui_rect_t){tRow.fX + 3.0f, tRow.fY + 2.0f, __xuiFileDialogMaxFloat(1.0f, tRow.fW - 6.0f), __xuiFileDialogMaxFloat(1.0f, tRow.fH - 4.0f)});
		iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, iSelected);
		if ( iRet != XUI_OK ) return iRet;
	} else if ( (iState & XUI_LIST_ITEM_HOVER) != 0 ) {
		tFill = xuiInternalSnapRect((xui_rect_t){tRow.fX + 3.0f, tRow.fY + 2.0f, __xuiFileDialogMaxFloat(1.0f, tRow.fW - 6.0f), __xuiFileDialogMaxFloat(1.0f, tRow.fH - 4.0f)});
		iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, iHover);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( ((iState & XUI_LIST_ITEM_FOCUS) != 0) &&
	     (xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget) &&
	     ((iState & XUI_LIST_ITEM_DISABLED) == 0) &&
	     (pProxy->drawRectStroke != NULL) ) {
		tFill = xuiInternalSnapRect((xui_rect_t){tRow.fX + 3.0f, tRow.fY + 2.0f, __xuiFileDialogMaxFloat(1.0f, tRow.fW - 6.0f), __xuiFileDialogMaxFloat(1.0f, tRow.fH - 4.0f)});
		iRet = pProxy->drawRectStroke(pProxy, pDraw, tFill, 1.0f, iFocus);
		if ( iRet != XUI_OK ) return iRet;
	}
	tIconDst = xuiInternalSnapRect((xui_rect_t){tRow.fX + 8.0f, tRow.fY + (tRow.fH - 16.0f) * 0.5f, 16.0f, 16.0f});
	iRet = pProxy->drawSurface(pProxy, pDraw, pAtlas, tIconSrc, tIconDst,
		__xuiFileDialogStyleColor(pWidget, "file_dialog.icon.color", XUI_COLOR_WHITE), 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( (iState & XUI_LIST_ITEM_SELECTED) != 0 ) {
		iTextColor = iSelectedText;
	}
	if ( (iState & XUI_LIST_ITEM_DISABLED) != 0 ) iTextColor = iDisabled;
	if ( pDialog->pFont != NULL ) {
		tText = xuiInternalSnapRect((xui_rect_t){tRow.fX + 30.0f, tRow.fY, __xuiFileDialogMaxFloat(1.0f, tRow.fW - 38.0f), tRow.fH});
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pDialog->pFont, .sText=pEntry->sName != NULL ? pEntry->sName : "", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	return 1;
}

static void __xuiFileDialogEntryFree(xui_file_dialog_entry_t* pEntry)
{
	if ( pEntry == NULL ) {
		return;
	}
	if ( pEntry->sName != NULL ) {
		xrtFree(pEntry->sName);
	}
	if ( pEntry->sPath != NULL ) {
		xrtFree(pEntry->sPath);
	}
	memset(pEntry, 0, sizeof(*pEntry));
}

static void __xuiFileDialogClearEntries(xui_file_dialog pDialog)
{
	int i;

	if ( pDialog == NULL ) {
		return;
	}
	for ( i = 0; i < pDialog->iEntryCount; i++ ) {
		__xuiFileDialogEntryFree(&pDialog->arrEntries[i]);
	}
	pDialog->iEntryCount = 0;
	pDialog->iSelectedEntry = -1;
}

static void __xuiFileDialogReleaseEntryStorage(xui_file_dialog pDialog)
{
	if ( pDialog == NULL ) return;
	__xuiFileDialogClearEntries(pDialog);
	if ( pDialog->arrEntries != NULL ) xrtFree(pDialog->arrEntries);
	pDialog->arrEntries = NULL;
	pDialog->iEntryCapacity = 0;
}

static void __xuiFileDialogFreeRootItems(xui_file_dialog pDialog)
{
	int i;

	if ( pDialog == NULL ) {
		return;
	}
	for ( i = 0; i < pDialog->iRootCount; i++ ) {
		if ( pDialog->arrRoots[i].sName != NULL ) xrtFree(pDialog->arrRoots[i].sName);
		if ( pDialog->arrRoots[i].sPath != NULL ) xrtFree(pDialog->arrRoots[i].sPath);
	}
	if ( pDialog->arrRoots != NULL ) xrtFree(pDialog->arrRoots);
	pDialog->arrRoots = NULL;
	pDialog->iRootCount = 0;
	pDialog->iRootCapacity = 0;
}

static void __xuiFileDialogFreeFilters(xui_file_dialog pDialog)
{
	int i;

	if ( pDialog == NULL ) {
		return;
	}
	for ( i = 0; i < pDialog->iFilterCount; i++ ) {
		if ( pDialog->arrFilterNames != NULL && pDialog->arrFilterNames[i] != NULL ) {
			xrtFree(pDialog->arrFilterNames[i]);
		}
		if ( pDialog->arrFilterPatterns != NULL && pDialog->arrFilterPatterns[i] != NULL ) {
			xrtFree(pDialog->arrFilterPatterns[i]);
		}
	}
	if ( pDialog->arrFilterNames != NULL ) xrtFree(pDialog->arrFilterNames);
	if ( pDialog->arrFilterPatterns != NULL ) xrtFree(pDialog->arrFilterPatterns);
	pDialog->arrFilterNames = NULL;
	pDialog->arrFilterPatterns = NULL;
	pDialog->iFilterCount = 0;
	pDialog->iFilterCapacity = 0;
	pDialog->iSelectedFilter = -1;
}

static int __xuiFileDialogEnsureFilterCapacity(xui_file_dialog pDialog, int iNeed)
{
	char** arrNames;
	char** arrPatterns;
	int iNewCapacity;
	int iRet;

	if ( pDialog == NULL || iNeed < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeed <= pDialog->iFilterCapacity ) {
		return XUI_OK;
	}
	iRet = __xuiFileDialogGrowCapacity(pDialog->iFilterCapacity, iNeed,
		XUI_FILE_DIALOG_FILTER_GROW, sizeof(char*), &iNewCapacity);
	if ( iRet != XUI_OK ) return iRet;
	arrNames = (char**)xrtRealloc(pDialog->arrFilterNames, sizeof(char*) * (size_t)iNewCapacity);
	if ( arrNames == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDialog->arrFilterNames = arrNames;
	arrPatterns = (char**)xrtRealloc(pDialog->arrFilterPatterns, sizeof(char*) * (size_t)iNewCapacity);
	if ( arrPatterns == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDialog->arrFilterPatterns = arrPatterns;
	memset(pDialog->arrFilterNames + pDialog->iFilterCapacity, 0, sizeof(char*) * (size_t)(iNewCapacity - pDialog->iFilterCapacity));
	memset(pDialog->arrFilterPatterns + pDialog->iFilterCapacity, 0, sizeof(char*) * (size_t)(iNewCapacity - pDialog->iFilterCapacity));
	pDialog->iFilterCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiFileDialogAddFilterRange(xui_file_dialog pDialog, const char* sName, size_t iNameLen, const char* sPattern, size_t iPatternLen)
{
	char* sNewName;
	char* sNewPattern;
	int iRet;

	if ( pDialog == NULL || sPattern == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiFileDialogTrimSpan(&sName, &iNameLen);
	__xuiFileDialogTrimSpan(&sPattern, &iPatternLen);
	if ( iPatternLen == 0u ) {
		return XUI_OK;
	}
	if ( sName == NULL || iNameLen == 0u ) {
		sName = sPattern;
		iNameLen = iPatternLen;
	}
	iRet = __xuiFileDialogEnsureFilterCapacity(pDialog, pDialog->iFilterCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	sNewName = __xuiFileDialogDupRange(sName, iNameLen);
	sNewPattern = __xuiFileDialogDupRange(sPattern, iPatternLen);
	if ( sNewName == NULL || sNewPattern == NULL ) {
		if ( sNewName != NULL ) xrtFree(sNewName);
		if ( sNewPattern != NULL ) xrtFree(sNewPattern);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDialog->arrFilterNames[pDialog->iFilterCount] = sNewName;
	pDialog->arrFilterPatterns[pDialog->iFilterCount] = sNewPattern;
	pDialog->iFilterCount++;
	return XUI_OK;
}

static int __xuiFileDialogAddFilter(xui_file_dialog pDialog, const char* sName, const char* sPattern)
{
	return __xuiFileDialogAddFilterRange(pDialog, sName, sName != NULL ? strlen(sName) : 0u, sPattern, sPattern != NULL ? strlen(sPattern) : 0u);
}

static int __xuiFileDialogParsePipeFilters(xui_file_dialog pDialog, const char* sSpec)
{
	const char* p;
	const char* sName;
	const char* sPattern;
	size_t iNameLen;
	size_t iPatternLen;
	int iRet;

	p = sSpec;
	while ( p != NULL && *p != 0 ) {
		sName = p;
		while ( *p != 0 && *p != '|' ) p++;
		iNameLen = (size_t)(p - sName);
		if ( *p != '|' ) break;
		p++;
		sPattern = p;
		while ( *p != 0 && *p != '|' ) p++;
		iPatternLen = (size_t)(p - sPattern);
		iRet = __xuiFileDialogAddFilterRange(pDialog, sName, iNameLen, sPattern, iPatternLen);
		if ( iRet != XUI_OK ) return iRet;
		if ( *p == '|' ) p++;
	}
	return XUI_OK;
}

static int __xuiFileDialogSetFilters(xui_file_dialog pDialog, const char* sSpec)
{
	xui_file_dialog_t tFilters;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tFilters, 0, sizeof(tFilters));
	tFilters.iMagic = XUI_FILE_DIALOG_MAGIC;
	tFilters.iSelectedFilter = -1;
	if ( sSpec != NULL && sSpec[0] != 0 ) {
		iRet = __xuiFileDialogParsePipeFilters(&tFilters, sSpec);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( tFilters.iFilterCount == 0 ) {
		iRet = __xuiFileDialogAddFilter(&tFilters, "All Files (*.*)", "*.*");
		if ( iRet != XUI_OK ) goto cleanup;
	}
	tFilters.iSelectedFilter = 0;
	iRet = __xuiFileDialogSetString(&tFilters.sFilter, tFilters.arrFilterPatterns[0]);
	if ( iRet != XUI_OK ) goto cleanup;
	__xuiFileDialogFreeFilters(pDialog);
	if ( pDialog->sFilter != NULL ) xrtFree(pDialog->sFilter);
	pDialog->arrFilterNames = tFilters.arrFilterNames;
	pDialog->arrFilterPatterns = tFilters.arrFilterPatterns;
	pDialog->iFilterCount = tFilters.iFilterCount;
	pDialog->iFilterCapacity = tFilters.iFilterCapacity;
	pDialog->iSelectedFilter = tFilters.iSelectedFilter;
	pDialog->sFilter = tFilters.sFilter;
	tFilters.arrFilterNames = NULL;
	tFilters.arrFilterPatterns = NULL;
	tFilters.iFilterCount = 0;
	tFilters.iFilterCapacity = 0;
	tFilters.sFilter = NULL;
	return XUI_OK;

cleanup:
	__xuiFileDialogFreeFilters(&tFilters);
	if ( tFilters.sFilter != NULL ) xrtFree(tFilters.sFilter);
	return iRet;
}

static int __xuiFileDialogEnsureEntryCapacity(xui_file_dialog pDialog, int iNeed)
{
	xui_file_dialog_entry_t* pEntries;
	int iNewCapacity;
	int iRet;

	if ( pDialog == NULL || iNeed < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeed <= pDialog->iEntryCapacity ) {
		return XUI_OK;
	}
	iRet = __xuiFileDialogGrowCapacity(pDialog->iEntryCapacity, iNeed,
		XUI_FILE_DIALOG_ENTRY_GROW, sizeof(*pEntries), &iNewCapacity);
	if ( iRet != XUI_OK ) return iRet;
	pEntries = (xui_file_dialog_entry_t*)xrtRealloc(pDialog->arrEntries, sizeof(xui_file_dialog_entry_t) * (size_t)iNewCapacity);
	if ( pEntries == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDialog->arrEntries = pEntries;
	memset(pDialog->arrEntries + pDialog->iEntryCapacity, 0, sizeof(xui_file_dialog_entry_t) * (size_t)(iNewCapacity - pDialog->iEntryCapacity));
	pDialog->iEntryCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiFileDialogFilterMatch(xui_file_dialog pDialog, const char* sName)
{
	const char* sFilter;
	const char* p;
	size_t iTokenLen;

	if ( pDialog == NULL || sName == NULL ) {
		return 0;
	}
	sFilter = pDialog->sFilter;
	if ( sFilter == NULL || sFilter[0] == 0 || strcmp(sFilter, "*") == 0 || strcmp(sFilter, "*.*") == 0 ) {
		return 1;
	}
	p = sFilter;
	while ( *p != 0 ) {
		const char* sStart;
		while ( (*p == ';') || (*p == ',') || isspace((unsigned char)*p) ) p++;
		sStart = p;
		while ( (*p != 0) && (*p != ';') && (*p != ',') && !isspace((unsigned char)*p) ) p++;
		iTokenLen = (size_t)(p - sStart);
		if ( iTokenLen == 0u ) {
			continue;
		}
		if ( (iTokenLen == 1u && sStart[0] == '*') ||
		     (iTokenLen == 3u && sStart[0] == '*' && sStart[1] == '.' && sStart[2] == '*') ) {
			return 1;
		}
		if ( xrtStrGlob(xuiXrtText(sName, 0u), xuiXrtText(sStart, iTokenLen), XSTR_GLOB_CASE_ASCII) ) {
			return 1;
		}
	}
	return 0;
}

static int __xuiFileDialogAddEntry(xui_file_dialog pDialog, const char* sName, const char* sPath, int bDir)
{
	xui_file_dialog_entry_t* pEntry;
	int iRet;

	if ( pDialog == NULL || sName == NULL || sPath == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER && !bDir ) {
		return XUI_OK;
	}
	if ( !bDir && !__xuiFileDialogFilterMatch(pDialog, sName) ) {
		return XUI_OK;
	}
	iRet = __xuiFileDialogEnsureEntryCapacity(pDialog, pDialog->iEntryCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pEntry = &pDialog->arrEntries[pDialog->iEntryCount];
	pEntry->sName = __xuiFileDialogDup(sName);
	pEntry->sPath = __xuiFileDialogDup(sPath);
	pEntry->bDir = bDir ? 1 : 0;
	if ( pEntry->sName == NULL || pEntry->sPath == NULL ) {
		__xuiFileDialogEntryFree(pEntry);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDialog->iEntryCount++;
	return XUI_OK;
}

static int __xuiFileDialogEntryCompare(const void* pLeft, const void* pRight)
{
	const xui_file_dialog_entry_t* pA = (const xui_file_dialog_entry_t*)pLeft;
	const xui_file_dialog_entry_t* pB = (const xui_file_dialog_entry_t*)pRight;
	int iCompare;

	if ( pA->bDir != pB->bDir ) return pA->bDir ? -1 : 1;
	iCompare = xrtStrCaseCompare(xuiXrtText(pA->sName, 0u), xuiXrtText(pB->sName, 0u));
	if ( iCompare != 0 ) return iCompare;
	return strcmp(pA->sName, pB->sName);
}

static int __xuiFileDialogSortEntries(xui_file_dialog pDialog)
{
	if ( pDialog == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDialog->iEntryCount > 1 ) {
		qsort(pDialog->arrEntries, (size_t)pDialog->iEntryCount,
			sizeof(*pDialog->arrEntries), __xuiFileDialogEntryCompare);
	}
	return XUI_OK;
}

static int __xuiFileDialogBuildEntries(xui_file_dialog pDialog, const char* sDir,
	xui_file_dialog pEntries)
{
	xdir tDir;
	xdirentry tEntry;
	xdirnext iNext;
	xfileinfo tInfo;
	char* sPath;
	size_t i;
	int bDir;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || sDir == NULL || pEntries == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = XUI_OK;
	memset(pEntries, 0, sizeof(*pEntries));
	pEntries->iMode = pDialog->iMode;
	pEntries->sFilter = pDialog->sFilter;
	pEntries->iSelectedEntry = -1;
	if ( sDir[0] == 0 ) {
		for ( i = 0; i < (size_t)pDialog->iRootCount; ++i ) {
			xui_file_dialog_root_t* pRoot = &pDialog->arrRoots[i];
			if ( pRoot->bShowInRoot && pRoot->sPath[0] != 0 ) {
				iRet = __xuiFileDialogAddEntry(pEntries, pRoot->sName, pRoot->sPath, 1);
				if ( iRet != XUI_OK ) goto cleanup;
			}
		}
	} else {
		if ( !xrtDirExists((str)sDir) ) return XUI_ERROR_FILE_NOT_FOUND;
		tDir = xrtDirOpen(sDir, 0u);
		if ( tDir == NULL ) return XUI_ERROR_BACKEND_FAILED;
		iNext = XDIR_NEXT_END;
		while ( (iNext = xrtDirNext(tDir, &tEntry)) == XDIR_NEXT_ITEM ) {
			sPath = xrtDirEntryPath(tDir, &tEntry);
			if ( sPath == NULL ) {
				iRet = XUI_ERROR_OUT_OF_MEMORY;
				break;
			}
			bDir = tEntry.Info.Type == XFILE_TYPE_DIRECTORY;
			if ( tEntry.Info.Type == XFILE_TYPE_NONE || tEntry.Info.Type == XFILE_TYPE_LINK ) {
				if ( xrtPathStat(sPath, true, &tInfo) ) {
					bDir = tInfo.Type == XFILE_TYPE_DIRECTORY;
				} else {
					xrtClearError();
				}
			}
			iRet = __xuiFileDialogAddEntry(pEntries, tEntry.Name.Data, sPath, bDir);
			xrtFree(sPath);
			if ( iRet != XUI_OK ) break;
		}
		if ( iNext == XDIR_NEXT_ERROR && iRet == XUI_OK ) iRet = XUI_ERROR_BACKEND_FAILED;
		if ( !xrtDirClose(tDir) && iRet == XUI_OK ) iRet = XUI_ERROR_BACKEND_FAILED;
		if ( iRet != XUI_OK ) goto cleanup;
	}
	iRet = __xuiFileDialogSortEntries(pEntries);
	if ( iRet != XUI_OK ) goto cleanup;
	return XUI_OK;

cleanup:
	__xuiFileDialogReleaseEntryStorage(pEntries);
	return iRet;
}

static const char* __xuiFileDialogEntryText(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) return "";
	return pDialog->arrEntries[iIndex].sName != NULL ? pDialog->arrEntries[iIndex].sName : "";
}

static int __xuiFileDialogReplaceEntries(xui_file_dialog pDialog, xui_file_dialog pEntries)
{
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pEntries == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiListViewSetVirtualItems(pDialog->pFileList, pEntries->iEntryCount,
		__xuiFileDialogEntryText, pDialog);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiListViewSetSelected(pDialog->pFileList, -1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiFileDialogReleaseEntryStorage(pDialog);
	pDialog->arrEntries = pEntries->arrEntries;
	pDialog->iEntryCount = pEntries->iEntryCount;
	pDialog->iEntryCapacity = pEntries->iEntryCapacity;
	pDialog->iSelectedEntry = -1;
	pEntries->arrEntries = NULL;
	pEntries->iEntryCount = 0;
	pEntries->iEntryCapacity = 0;
	return XUI_OK;
}

static int __xuiFileDialogSetNameText(xui_file_dialog pDialog, const char* sText)
{
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->pNameInput == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDialog->bInternalNameChange = 1;
	iRet = xuiInputSetText(pDialog->pNameInput, sText != NULL ? sText : "");
	pDialog->bInternalNameChange = 0;
	return iRet;
}

static void __xuiFileDialogClearSelection(xui_file_dialog pDialog)
{
	if ( !__xuiFileDialogValid(pDialog) ) {
		return;
	}
	pDialog->iSelectedEntry = -1;
	if ( pDialog->pFileList != NULL ) {
		(void)xuiListViewSetSelected(pDialog->pFileList, -1);
	}
}

static int __xuiFileDialogSetSelectedName(xui_file_dialog pDialog, int iIndex)
{
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE && pDialog->arrEntries[iIndex].bDir ) {
		return XUI_OK;
	}
	return __xuiFileDialogSetNameText(pDialog, pDialog->arrEntries[iIndex].sName);
}

static int __xuiFileDialogEnsureRootCapacity(xui_file_dialog pDialog, int iNeed)
{
	xui_file_dialog_root_t* arrRoots;
	int iCapacity;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || iNeed < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNeed <= pDialog->iRootCapacity ) return XUI_OK;
	iRet = __xuiFileDialogGrowCapacity(pDialog->iRootCapacity, iNeed,
		XUI_FILE_DIALOG_ROOT_GROW, sizeof(*arrRoots), &iCapacity);
	if ( iRet != XUI_OK ) return iRet;
	arrRoots = (xui_file_dialog_root_t*)xrtRealloc(pDialog->arrRoots,
		sizeof(*arrRoots) * (size_t)iCapacity);
	if ( arrRoots == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pDialog->arrRoots = arrRoots;
	memset(pDialog->arrRoots + pDialog->iRootCapacity, 0,
		sizeof(*arrRoots) * (size_t)(iCapacity - pDialog->iRootCapacity));
	pDialog->iRootCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiFileDialogAddRoot(xui_file_dialog pDialog, int iNodeId, int iParentId,
	const char* sName, const char* sPath, const char* sIconAsset, int bExpanded, int bShowInRoot)
{
	xui_file_dialog_root_t* pRoot;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || iNodeId < 0 || sName == NULL || sIconAsset == NULL )
		return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiFileDialogEnsureRootCapacity(pDialog, pDialog->iRootCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	pRoot = &pDialog->arrRoots[pDialog->iRootCount];
	memset(pRoot, 0, sizeof(*pRoot));
	pRoot->sName = __xuiFileDialogDup(sName);
	pRoot->sPath = __xuiFileDialogDup(sPath);
	if ( pRoot->sName == NULL || pRoot->sPath == NULL ) {
		if ( pRoot->sName != NULL ) xrtFree(pRoot->sName);
		if ( pRoot->sPath != NULL ) xrtFree(pRoot->sPath);
		memset(pRoot, 0, sizeof(*pRoot));
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pRoot->iNodeId = iNodeId;
	pRoot->iParentId = iParentId;
	pRoot->sIconAsset = sIconAsset;
	pRoot->bExpanded = bExpanded ? 1 : 0;
	pRoot->bShowInRoot = bShowInRoot ? 1 : 0;
	pDialog->iRootCount++;
	return XUI_OK;
}

static xui_file_dialog_root_t* __xuiFileDialogFindRoot(xui_file_dialog pDialog, int iNodeId, int* pIndex)
{
	int i;
	if ( !__xuiFileDialogValid(pDialog) ) return NULL;
	for ( i = 0; i < pDialog->iRootCount; ++i ) {
		if ( pDialog->arrRoots[i].iNodeId == iNodeId ) {
			if ( pIndex != NULL ) *pIndex = i;
			return &pDialog->arrRoots[i];
		}
	}
	return NULL;
}

static int __xuiFileDialogRootProc(str sPath, size_t iSize, int bDir, ptr pData, ptr Param)
{
	xui_file_dialog pDialog = (xui_file_dialog)Param;
	int iParentId;
	int iNodeId;

	(void)iSize;
	(void)bDir;
	(void)pData;
	if ( !__xuiFileDialogValid(pDialog) ) return TRUE;
#if defined(_WIN32)
	iParentId = XUI_FILE_DIALOG_ROOT_COMPUTER;
#else
	iParentId = -1;
#endif
	iNodeId = XUI_FILE_DIALOG_ROOT_DRIVE_FIRST + pDialog->iRootCount;
	return __xuiFileDialogAddRoot(pDialog, iNodeId, iParentId, (const char*)sPath,
		(const char*)sPath, XUI_FILE_DIALOG_ICON_DRIVE, 0, 1) != XUI_OK;
}

#if defined(_WIN32)
static char* __xuiFileDialogKnownFolder(const KNOWNFOLDERID* pId)
{
	PWSTR sWide = NULL;
	char* sPath = NULL;
	if ( pId != NULL && SUCCEEDED(SHGetKnownFolderPath(pId, KF_FLAG_DEFAULT, NULL, &sWide)) && sWide != NULL ) {
		sPath = xrtUtf16To8((const uint16*)sWide, NULL);
	}
	if ( sWide != NULL ) CoTaskMemFree(sWide);
	return sPath;
}

static char* __xuiFileDialogKnownFolderOrFallback(const KNOWNFOLDERID* pId,
	const char* sProfile, const char* sFallback)
{
	char* sPath = __xuiFileDialogKnownFolder(pId);
	if ( sPath == NULL && sProfile != NULL && sFallback != NULL )
		sPath = xrtPathJoin(sProfile, sFallback);
	return sPath;
}
#endif

static int __xuiFileDialogBuildRoots(xui_file_dialog pDialog)
{
	xui_file_dialog_t tRootsStore;
	xui_file_dialog pRoots;
	xdirroots tRoots;
	size_t i;
	int iRet = XUI_OK;

	if ( !__xuiFileDialogValid(pDialog) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tRootsStore, 0, sizeof(tRootsStore));
	tRootsStore.iMagic = XUI_FILE_DIALOG_MAGIC;
	tRootsStore.pContext = pDialog->pContext;
	pRoots = &tRootsStore;
#if defined(_WIN32)
	{
		static const struct {
			int iNodeId;
			int iTranslation;
			const KNOWNFOLDERID* pFolderId;
			const char* sFallback;
			const char* sIcon;
		} arrKnown[] = {
			{XUI_FILE_DIALOG_ROOT_DOCUMENTS, XUI_TR_FILE_DOCUMENTS, &FOLDERID_Documents, "Documents", XUI_FILE_DIALOG_ICON_DOCUMENTS},
			{XUI_FILE_DIALOG_ROOT_PICTURES, XUI_TR_FILE_PICTURES, &FOLDERID_Pictures, "Pictures", XUI_FILE_DIALOG_ICON_PICTURES},
			{XUI_FILE_DIALOG_ROOT_MUSIC, XUI_TR_FILE_MUSIC, &FOLDERID_Music, "Music", XUI_FILE_DIALOG_ICON_MUSIC},
			{XUI_FILE_DIALOG_ROOT_VIDEOS, XUI_TR_FILE_VIDEOS, &FOLDERID_Videos, "Videos", XUI_FILE_DIALOG_ICON_VIDEOS},
			{XUI_FILE_DIALOG_ROOT_DOWNLOADS, XUI_TR_FILE_DOWNLOADS, &FOLDERID_Downloads, "Downloads", XUI_FILE_DIALOG_ICON_DOWNLOADS},
			{XUI_FILE_DIALOG_ROOT_DESKTOP, XUI_TR_FILE_DESKTOP, &FOLDERID_Desktop, "Desktop", XUI_FILE_DIALOG_ICON_DESKTOP}
		};
		char* sProfile = __xuiFileDialogKnownFolder(&FOLDERID_Profile);
		if ( sProfile == NULL ) sProfile = xrtPathHome();
		if ( sProfile != NULL ) {
			iRet = __xuiFileDialogAddRoot(pRoots, XUI_FILE_DIALOG_ROOT_USER, -1,
				xuiTranslate(pRoots->pContext, XUI_TR_FILE_USER), sProfile,
				XUI_FILE_DIALOG_ICON_USER, 1, 0);
			for ( i = 0; i < sizeof(arrKnown) / sizeof(arrKnown[0]) && iRet == XUI_OK; ++i ) {
				char* sPath = __xuiFileDialogKnownFolderOrFallback(arrKnown[i].pFolderId,
					sProfile, arrKnown[i].sFallback);
				if ( sPath != NULL ) {
					iRet = __xuiFileDialogAddRoot(pRoots, arrKnown[i].iNodeId,
						XUI_FILE_DIALOG_ROOT_USER,
						xuiTranslate(pRoots->pContext, arrKnown[i].iTranslation), sPath,
						arrKnown[i].sIcon, 0, 0);
					xrtFree(sPath);
				}
			}
			xrtFree(sProfile);
		}
		if ( iRet == XUI_OK ) {
			iRet = __xuiFileDialogAddRoot(pRoots, XUI_FILE_DIALOG_ROOT_COMPUTER, -1,
				xuiTranslate(pRoots->pContext, XUI_TR_FILE_THIS_PC), "",
				XUI_FILE_DIALOG_ICON_COMPUTER, 1, 0);
		}
	}
#endif
	memset(&tRoots, 0, sizeof(tRoots));
	if ( iRet == XUI_OK ) {
		if ( !xrtDirRoots(&tRoots) ) {
			iRet = xrtErrorIs(xrtGetError(), XERR_MEMORY) ?
				XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR_BACKEND_FAILED;
		} else {
			for ( i = 0; i < tRoots.Count; ++i ) {
				if ( __xuiFileDialogRootProc(tRoots.Items[i], strlen(tRoots.Items[i]), TRUE,
					NULL, pRoots) ) {
					iRet = XUI_ERROR_OUT_OF_MEMORY;
					break;
				}
			}
			xrtDirRootsFree(&tRoots);
		}
	}
	if ( iRet == XUI_OK && pDialog->pRootList != NULL ) {
		pRoots->pRootList = pDialog->pRootList;
		iRet = __xuiFileDialogSyncRootTree(pRoots);
	}
	if ( iRet == XUI_OK ) {
		__xuiFileDialogFreeRootItems(pDialog);
		pDialog->arrRoots = pRoots->arrRoots;
		pDialog->iRootCount = pRoots->iRootCount;
		pDialog->iRootCapacity = pRoots->iRootCapacity;
		pRoots->arrRoots = NULL;
		pRoots->iRootCount = 0;
		pRoots->iRootCapacity = 0;
	}
	__xuiFileDialogFreeRootItems(pRoots);
	return iRet;
}

static int __xuiFileDialogSyncRootTree(xui_file_dialog pDialog)
{
	xui_tree_view_node_t* arrNodes;
	int iCount;
	int i;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->pRootList == NULL )
		return XUI_ERROR_INVALID_ARGUMENT;
	iCount = pDialog->iRootCount;
	if ( iCount <= 0 ) return xuiTreeViewSetNodes(pDialog->pRootList, NULL, 0);
	if ( (size_t)iCount > SIZE_MAX / sizeof(*arrNodes) ) return XUI_ERROR_LIMIT_EXCEEDED;
	arrNodes = (xui_tree_view_node_t*)xrtCalloc((size_t)iCount, sizeof(*arrNodes));
	if ( arrNodes == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	for ( i = 0; i < iCount; ++i ) {
		xui_file_dialog_root_t* pRoot = &pDialog->arrRoots[i];
		arrNodes[i].iId = pRoot->iNodeId;
		arrNodes[i].iParent = pRoot->iParentId;
		arrNodes[i].sText = pRoot->sName;
		arrNodes[i].bExpanded = pRoot->bExpanded;
		arrNodes[i].bEnabled = 1;
		arrNodes[i].bIconReserved = 1;
	}
	iRet = xuiTreeViewSetNodes(pDialog->pRootList, arrNodes, iCount);
	xrtFree(arrNodes);
	return iRet;
}

static int __xuiFileDialogSyncRootSelection(xui_file_dialog pDialog)
{
	int iSelected = -1;
	int i;
	if ( !__xuiFileDialogValid(pDialog) || pDialog->pRootList == NULL )
		return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pDialog->iRootCount; ++i ) {
		xui_file_dialog_root_t* pRoot = &pDialog->arrRoots[i];
		if ( pRoot->sPath[0] != 0 && pDialog->sCurrentDir != NULL &&
			strcmp(pRoot->sPath, pDialog->sCurrentDir) == 0 ) {
			iSelected = pRoot->iNodeId;
			break;
		}
	}
	return xuiTreeViewSetSelected(pDialog->pRootList, iSelected);
}

static int __xuiFileDialogDrawRootExpander(xui_proxy pProxy, xui_draw_context pDraw,
	xui_rect_t tRect, int bExpanded, uint32_t iColor)
{
	xui_vec2_t tA, tB, tC;
	float fCX, fCY;
	if ( pProxy->drawTriangleFill == NULL || (iColor & 0xffu) == 0u ) return XUI_OK;
	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	if ( bExpanded ) {
		tA = (xui_vec2_t){fCX - 4.0f, fCY - 2.0f};
		tB = (xui_vec2_t){fCX + 4.0f, fCY - 2.0f};
		tC = (xui_vec2_t){fCX, fCY + 3.0f};
	} else {
		tA = (xui_vec2_t){fCX - 2.0f, fCY - 4.0f};
		tB = (xui_vec2_t){fCX - 2.0f, fCY + 4.0f};
		tC = (xui_vec2_t){fCX + 3.0f, fCY};
	}
	return pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
}

static int __xuiFileDialogDrawRootItem(xui_widget pWidget, int iNodeId, int iVisible,
	const xui_tree_view_node_t* pNode, xui_draw_context pDraw, xui_rect_t tRow,
	int iState, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	xui_file_dialog_root_t* pRoot;
	xui_proxy pProxy;
	xui_surface pAtlas;
	xui_rect_t tFill, tExpander, tIcon, tIconSrc, tText;
	float fIndent = 18.0f, fPadding = 6.0f;
	float fTextX;
	uint32_t iFocus, iRow, iHover, iSelected, iText, iDisabled, iExpander;
	int iRet;

	(void)iVisible;
	if ( !__xuiFileDialogValid(pDialog) || pWidget == NULL || pNode == NULL || pDraw == NULL ) return 0;
	pRoot = __xuiFileDialogFindRoot(pDialog, iNodeId, NULL);
	if ( pRoot == NULL ) return 0;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL || pProxy->drawSurface == NULL || pProxy->drawText == NULL ||
		pProxy->drawRectFill == NULL ) return 0;
	if ( xuiBuiltinAssetGetAtlas(xuiWidgetGetContext(pWidget), &pAtlas) != XUI_OK ||
		xuiBuiltinAssetGetRect(pRoot->sIconAsset, &tIconSrc) != XUI_OK ) return 0;
	(void)xuiTreeViewGetMetrics(pWidget, NULL, &fIndent, &fPadding, NULL);
	(void)xuiTreeViewGetColors(pWidget, NULL, NULL, &iFocus, &iRow, &iHover,
		&iSelected, &iText, &iDisabled);
	(void)xuiTreeViewGetDecorationColors(pWidget, &iExpander, NULL, NULL);
	iRow = __xuiFileDialogStyleColor(pWidget, "treeview.row.color", iRow);
	iHover = __xuiFileDialogStyleColor(pWidget, "treeview.row.hover_color", iHover);
	iSelected = __xuiFileDialogStyleColor(pWidget, "treeview.row.selected_color", iSelected);
	iFocus = __xuiFileDialogStyleColor(pWidget, "treeview.focus.color", iFocus);
	iText = __xuiFileDialogStyleColor(pWidget, "treeview.text.color", iText);
	iDisabled = __xuiFileDialogStyleColor(pWidget, "treeview.text.disabled_color", iDisabled);
	iExpander = __xuiFileDialogStyleColor(pWidget, "treeview.expander.color", iExpander);
	if ( (iRow & 0xffu) != 0u ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tRow, iRow);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iState & XUI_TREE_ITEM_SELECTED) != 0 ) {
		tFill = xuiInternalSnapRect((xui_rect_t){tRow.fX + 3.0f, tRow.fY + 2.0f,
			__xuiFileDialogMaxFloat(1.0f, tRow.fW - 6.0f),
			__xuiFileDialogMaxFloat(1.0f, tRow.fH - 4.0f)});
		iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, iSelected);
		if ( iRet != XUI_OK ) return iRet;
		iText = __xuiFileDialogStyleColor(pWidget, "treeview.text.selected_color", XUI_COLOR_WHITE);
	} else if ( (iState & XUI_TREE_ITEM_HOVER) != 0 ) {
		tFill = xuiInternalSnapRect((xui_rect_t){tRow.fX + 3.0f, tRow.fY + 2.0f,
			__xuiFileDialogMaxFloat(1.0f, tRow.fW - 6.0f),
			__xuiFileDialogMaxFloat(1.0f, tRow.fH - 4.0f)});
		iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, iHover);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iState & XUI_TREE_ITEM_FOCUS) != 0 &&
		xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget &&
		pProxy->drawRectStroke != NULL ) {
		tFill = xuiInternalSnapRect((xui_rect_t){tRow.fX + 3.0f, tRow.fY + 2.0f,
			__xuiFileDialogMaxFloat(1.0f, tRow.fW - 6.0f),
			__xuiFileDialogMaxFloat(1.0f, tRow.fH - 4.0f)});
		iRet = pProxy->drawRectStroke(pProxy, pDraw, tFill, 1.0f, iFocus);
		if ( iRet != XUI_OK ) return iRet;
	}
	tExpander = xuiInternalSnapRect((xui_rect_t){tRow.fX + fPadding +
		(float)pNode->iDepth * fIndent, tRow.fY + (tRow.fH - 14.0f) * 0.5f,
		14.0f, 14.0f});
	if ( (iState & XUI_TREE_ITEM_HAS_CHILDREN) != 0 ) {
		iRet = __xuiFileDialogDrawRootExpander(pProxy, pDraw, tExpander,
			(iState & XUI_TREE_ITEM_EXPANDED) != 0, iExpander);
		if ( iRet != XUI_OK ) return iRet;
	}
	tIcon = xuiInternalSnapRect((xui_rect_t){tExpander.fX + tExpander.fW + 4.0f,
		tRow.fY + (tRow.fH - 16.0f) * 0.5f, 16.0f, 16.0f});
	iRet = pProxy->drawSurface(pProxy, pDraw, pAtlas, tIconSrc, tIcon, XUI_COLOR_WHITE, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( (iState & XUI_TREE_ITEM_DISABLED) != 0 ) iText = iDisabled;
	fTextX = tIcon.fX + tIcon.fW + 5.0f;
	tText = xuiInternalSnapRect((xui_rect_t){fTextX, tRow.fY,
		__xuiFileDialogMaxFloat(1.0f, tRow.fW - fTextX - fPadding), tRow.fH});
	iRet = pProxy->drawText(pProxy, pDraw,
		&(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pDialog->pFont,
		.sText=pRoot->sName, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT},
		tText, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	return 1;
}

static int __xuiFileDialogPathBarRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_proxy pProxy;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	(void)pUser;
	if ( pWidget == NULL || pDraw == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	tRect = xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
	if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tRect,
			__xuiFileDialogStyleColor(pWidget, "file_dialog.path.background.color",
				__xuiFileDialogStyleColor(pWidget, "file_dialog.theme.panel", XUI_COLOR_WHITE)));
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pProxy->drawRectStroke != NULL ) {
		iRet = pProxy->drawRectStroke(pProxy, pDraw, tRect, 1.0f,
			__xuiFileDialogStyleColor(pWidget, "file_dialog.path.border.color",
				__xuiFileDialogStyleColor(pWidget, "file_dialog.theme.border", XUI_COLOR_RGBA(0, 0, 0, 255))));
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiFileDialogBreadcrumbHit(xui_widget pBreadcrumb, float fX, float fY)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fLocalX;
	float fLocalY;
	int i;
	int iCount;

	if ( pBreadcrumb == NULL ) {
		return -1;
	}
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	fLocalX = fX - tWorld.fX;
	fLocalY = fY - tWorld.fY;
	iCount = xuiBreadcrumbGetItemCount(pBreadcrumb);
	for ( i = 0; i < iCount; i++ ) {
		tItem = xuiBreadcrumbGetItemRect(pBreadcrumb, i);
		if ( xuiBreadcrumbGetItemClickable(pBreadcrumb, i) &&
		     fLocalX >= tItem.fX && fLocalY >= tItem.fY &&
		     fLocalX < tItem.fX + tItem.fW && fLocalY < tItem.fY + tItem.fH ) {
			return i;
		}
	}
	return -1;
}

static int __xuiFileDialogSetPathEditMode(xui_file_dialog pDialog, int bEdit)
{
	int bNewEdit;
	int bOldEdit;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bNewEdit = bEdit ? 1 : 0;
	bOldEdit = pDialog->bPathEditMode;
	if ( bNewEdit == bOldEdit ) return XUI_OK;
	pDialog->bPathEditMode = bNewEdit;
	iRet = XUI_OK;
	if ( pDialog->pPathInput != NULL ) {
		iRet = xuiWidgetSetVisible(pDialog->pPathInput, bNewEdit);
	}
	if ( iRet == XUI_OK && pDialog->pPathBreadcrumb != NULL ) {
		iRet = xuiWidgetSetVisible(pDialog->pPathBreadcrumb, !bNewEdit);
	}
	if ( iRet == XUI_OK ) {
		if ( bNewEdit ) {
			iRet = xuiInputSetText(pDialog->pPathInput,
				pDialog->sCurrentDir != NULL ? pDialog->sCurrentDir : "");
			if ( iRet == XUI_OK ) iRet = xuiInputSetError(pDialog->pPathInput, 0);
			if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pDialog->pContext, pDialog->pPathInput);
		} else if ( !pDialog->bPathApplying &&
			xuiGetFocusWidget(pDialog->pContext) == pDialog->pPathInput ) {
			iRet = xuiSetFocusWidget(pDialog->pContext, NULL);
		}
	}
	if ( iRet == XUI_OK && pDialog->pPathBar != NULL ) {
		iRet = xuiWidgetInvalidate(pDialog->pPathBar,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( iRet != XUI_OK ) {
		pDialog->bPathEditMode = bOldEdit;
		if ( pDialog->pPathInput != NULL ) (void)xuiWidgetSetVisible(pDialog->pPathInput, bOldEdit);
		if ( pDialog->pPathBreadcrumb != NULL ) (void)xuiWidgetSetVisible(pDialog->pPathBreadcrumb, !bOldEdit);
		if ( pDialog->pPathBar != NULL ) {
			(void)xuiWidgetInvalidate(pDialog->pPathBar,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	}
	return iRet;
}

static int __xuiFileDialogShowPathError(xui_file_dialog pDialog, const char* sPath)
{
	char sMessage[512];

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	(void)xuiInputSetError(pDialog->pPathInput, 1);
	snprintf(sMessage, sizeof(sMessage), xuiTranslate(pDialog->pContext, XUI_TR_FILE_PATH_NOT_EXIST_FMT), (sPath != NULL && sPath[0] != 0) ? sPath : xuiTranslate(pDialog->pContext, XUI_TR_FILE_EMPTY));
	xuiInternalUtf8TrimIncomplete(sMessage);
	if ( pDialog->pPathTip != NULL ) {
		(void)xuiMsgTipShow(pDialog->pPathTip, XUI_MSGTIP_ICON_ERROR, sMessage, 2.2f);
	}
	return XUI_OK;
}

static int __xuiFileDialogShowFolderError(xui_file_dialog pDialog)
{
	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	(void)xuiInputSetError(pDialog->pNameInput, 1);
	if ( pDialog->pPathTip != NULL ) {
		(void)xuiMsgTipShow(pDialog->pPathTip, XUI_MSGTIP_ICON_ERROR, xuiTranslate(pDialog->pContext, XUI_TR_FILE_SELECT_FOLDER_FIRST), 2.2f);
	}
	return XUI_OK;
}

static int __xuiFileDialogUpdateBreadcrumb(xui_file_dialog pDialog)
{
	const char* sPath;
	char sLabel[512];
	size_t iLen;
	size_t iStart;
	size_t iEnd;
	int iRootLen;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->pPathBreadcrumb == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiBreadcrumbClearItems(pDialog->pPathBreadcrumb);
	if ( iRet != XUI_OK ) return iRet;
	sPath = pDialog->sCurrentDir != NULL ? pDialog->sCurrentDir : "";
	if ( sPath[0] == 0 ) {
		return xuiBreadcrumbAddItem(pDialog->pPathBreadcrumb, "Root", 1, 0);
	}
	iLen = strlen(sPath);
	iRootLen = __xuiFileDialogPathRootLen(sPath);
	if ( iRootLen > 0 ) {
		size_t iCopy = (size_t)iRootLen;
		if ( iCopy >= sizeof(sLabel) ) iCopy = sizeof(sLabel) - 1u;
		memcpy(sLabel, sPath, iCopy);
		sLabel[iCopy] = 0;
		iRet = xuiBreadcrumbAddItem(pDialog->pPathBreadcrumb, sLabel, 1, iRootLen);
		if ( iRet != XUI_OK ) return iRet;
	}
	iStart = (size_t)iRootLen;
	while ( iStart < iLen && __xuiFileDialogIsSlash(sPath[iStart]) ) {
		iStart++;
	}
	while ( iStart < iLen && xuiBreadcrumbGetItemCount(pDialog->pPathBreadcrumb) < XUI_BREADCRUMB_MAX_ITEMS ) {
		iEnd = iStart;
		while ( iEnd < iLen && !__xuiFileDialogIsSlash(sPath[iEnd]) ) {
			iEnd++;
		}
		if ( iEnd > iStart ) {
			size_t iCopy = iEnd - iStart;
			if ( iCopy >= sizeof(sLabel) ) iCopy = sizeof(sLabel) - 1u;
			memcpy(sLabel, sPath + iStart, iCopy);
			sLabel[iCopy] = 0;
			iRet = xuiBreadcrumbAddItem(pDialog->pPathBreadcrumb, sLabel, 1, (int)iEnd);
			if ( iRet != XUI_OK ) return iRet;
		}
		iStart = iEnd;
		while ( iStart < iLen && __xuiFileDialogIsSlash(sPath[iStart]) ) {
			iStart++;
		}
	}
	if ( xuiBreadcrumbGetItemCount(pDialog->pPathBreadcrumb) == 0 ) {
		return xuiBreadcrumbAddItem(pDialog->pPathBreadcrumb, sPath, 1, (int)iLen);
	}
	return XUI_OK;
}

static int __xuiFileDialogBuildCurrentPath(xui_file_dialog pDialog, const char* sNameOrPath, char** psPath)
{
	if ( pDialog == NULL || sNameOrPath == NULL || sNameOrPath[0] == 0 || psPath == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*psPath = NULL;
	if ( xrtPathIsAbs(sNameOrPath) || pDialog->sCurrentDir == NULL || pDialog->sCurrentDir[0] == 0 ) {
		*psPath = __xuiFileDialogDup(sNameOrPath);
	} else {
		*psPath = xrtPathJoin(pDialog->sCurrentDir, sNameOrPath);
	}
	return (*psPath != NULL) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

static int __xuiFileDialogNameHasExtension(const char* sNameOrPath)
{
	const char* sLastSep;
	const char* sDot;

	if ( sNameOrPath == NULL || sNameOrPath[0] == 0 ) {
		return 0;
	}
	sLastSep = strrchr(sNameOrPath, '/');
	if ( strrchr(sNameOrPath, '\\') != NULL && (sLastSep == NULL || strrchr(sNameOrPath, '\\') > sLastSep) ) {
		sLastSep = strrchr(sNameOrPath, '\\');
	}
	sDot = strrchr(sNameOrPath, '.');
	return (sDot != NULL) && (sLastSep == NULL || sDot > sLastSep);
}

static char* __xuiFileDialogDefaultSaveExtension(xui_file_dialog pDialog)
{
	const char* sPattern;
	const char* sStart;
	const char* sEnd;
	const char* sDot;
	size_t iLen;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->iSelectedFilter < 0 || pDialog->iSelectedFilter >= pDialog->iFilterCount ) {
		return NULL;
	}
	sPattern = pDialog->arrFilterPatterns[pDialog->iSelectedFilter];
	if ( sPattern == NULL ) {
		return NULL;
	}
	sStart = sPattern;
	while ( *sStart != 0 ) {
		while ( *sStart == ';' || *sStart == ',' || isspace((unsigned char)*sStart) ) {
			sStart++;
		}
		sEnd = sStart;
		while ( *sEnd != 0 && *sEnd != ';' && *sEnd != ',' && !isspace((unsigned char)*sEnd) ) {
			sEnd++;
		}
		iLen = (size_t)(sEnd - sStart);
		if ( iLen > 2u && sStart[0] == '*' && sStart[1] == '.' ) {
			if ( iLen == 3u && sStart[2] == '*' ) {
				return NULL;
			}
			sDot = sStart + 1;
			return __xuiFileDialogDupRange(sDot, iLen - 1u);
		}
		sStart = sEnd;
	}
	return NULL;
}

static int __xuiFileDialogBuildSavePath(xui_file_dialog pDialog, const char* sNameOrPath, char** psPath)
{
	char* sNameWithExt;
	char* sExt;
	size_t iNameLen;
	size_t iExtLen;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || sNameOrPath == NULL || sNameOrPath[0] == 0 || psPath == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*psPath = NULL;
	if ( __xuiFileDialogNameHasExtension(sNameOrPath) ) {
		return __xuiFileDialogBuildCurrentPath(pDialog, sNameOrPath, psPath);
	}
	sExt = __xuiFileDialogDefaultSaveExtension(pDialog);
	if ( sExt == NULL || sExt[0] == 0 ) {
		if ( sExt != NULL ) xrtFree(sExt);
		return __xuiFileDialogBuildCurrentPath(pDialog, sNameOrPath, psPath);
	}
	iNameLen = strlen(sNameOrPath);
	iExtLen = strlen(sExt);
	sNameWithExt = (char*)xrtMalloc(iNameLen + iExtLen + 1u);
	if ( sNameWithExt == NULL ) {
		xrtFree(sExt);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sNameWithExt, sNameOrPath, iNameLen);
	memcpy(sNameWithExt + iNameLen, sExt, iExtLen + 1u);
	iRet = __xuiFileDialogBuildCurrentPath(pDialog, sNameWithExt, psPath);
	xrtFree(sNameWithExt);
	xrtFree(sExt);
	return iRet;
}

static char* __xuiFileDialogParentDir(const char* sDir)
{
	size_t iLen;
	size_t iEnd;
	size_t iPos;
	size_t iRootLen;
	char* sParent;

	if ( sDir == NULL || sDir[0] == 0 ) {
		return __xuiFileDialogDup("");
	}
	iLen = strlen(sDir);
	iEnd = iLen;
	while ( iEnd > 0u && (sDir[iEnd - 1u] == '/' || sDir[iEnd - 1u] == '\\') ) {
		iEnd--;
	}
	if ( iEnd == 0u ) {
		return __xuiFileDialogDup("");
	}
	iRootLen = (size_t)__xuiFileDialogPathRootLen(sDir);
	if ( iEnd <= iRootLen ) {
		return __xuiFileDialogDup("");
	}
	iPos = iEnd;
	while ( iPos > 0u && sDir[iPos - 1u] != '/' && sDir[iPos - 1u] != '\\' ) {
		iPos--;
	}
	if ( iPos == 0u ) {
		return __xuiFileDialogDup("");
	}
	if ( iPos <= iRootLen ) {
		iPos = iRootLen;
	} else {
		iPos--;
	}
	sParent = (char*)xrtMalloc(iPos + 1u);
	if ( sParent == NULL ) {
		return NULL;
	}
	memcpy(sParent, sDir, iPos);
	sParent[iPos] = 0;
	return sParent;
}

static int __xuiFileDialogNotify(xui_file_dialog pDialog, int iResult)
{
	pDialog->iResult = iResult;
	if ( pDialog->onResult != NULL ) {
		pDialog->onResult(pDialog, iResult, pDialog->sResultPath != NULL ? pDialog->sResultPath : "", pDialog->pResultUser);
	}
	return XUI_OK;
}

static int __xuiFileDialogFinishWithPath(xui_file_dialog pDialog, const char* sPath)
{
	char* sNewPath;
	char* sOldPath;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || sPath == NULL || sPath[0] == 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sNewPath = __xuiFileDialogDup(sPath);
	if ( sNewPath == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOldPath = pDialog->sResultPath;
	pDialog->sResultPath = sNewPath;
	iRet = xuiFileDialogSetOpen(pDialog, 0);
	if ( iRet != XUI_OK ) {
		pDialog->sResultPath = sOldPath;
		xrtFree(sNewPath);
		return iRet;
	}
	if ( sOldPath != NULL ) xrtFree(sOldPath);
	return __xuiFileDialogNotify(pDialog, XUI_FILE_DIALOG_RESULT_OK);
}

static void __xuiFileDialogOverwriteResult(xui_msgbox pBox, int iResult, void* pUser)
{
	xui_file_dialog pDialog;

	(void)pBox;
	pDialog = (xui_file_dialog)pUser;
	if ( !__xuiFileDialogValid(pDialog) ) {
		return;
	}
	if ( iResult == XUI_MSGBOX_RESULT_YES && pDialog->sPendingSavePath != NULL && pDialog->sPendingSavePath[0] != 0 ) {
		(void)__xuiFileDialogFinishWithPath(pDialog, pDialog->sPendingSavePath);
	}
}

static int __xuiFileDialogShowOverwriteConfirm(xui_file_dialog pDialog, const char* sPath)
{
	xui_msgbox_desc_t tDesc;
	xui_msgbox pNewBox;
	char* sNewPath;
	char sMessage[768];
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || sPath == NULL || sPath[0] == 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sNewPath = __xuiFileDialogDup(sPath);
	if ( sNewPath == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	snprintf(sMessage, sizeof(sMessage), xuiTranslate(pDialog->pContext, XUI_TR_FILE_OVERWRITE_MESSAGE_FMT), sPath);
	xuiInternalUtf8TrimIncomplete(sMessage);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = xuiTranslate(pDialog->pContext, XUI_TR_FILE_CONFIRM_SAVE_AS);
	tDesc.sMessage = sMessage;
	tDesc.pFont = pDialog->pFont;
	tDesc.iType = XUI_MSGBOX_ICON_QUEST;
	tDesc.iButtons = XUI_MSGBOX_BUTTON_YES_NO;
	pNewBox = NULL;
	iRet = xuiMsgBoxCreate(pDialog->pContext, &pNewBox, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiMsgBoxSetResult(pNewBox, __xuiFileDialogOverwriteResult, pDialog);
	if ( iRet == XUI_OK ) iRet = xuiMsgBoxSetOpen(pNewBox, 1);
	if ( iRet != XUI_OK ) {
		if ( pNewBox != NULL ) xuiMsgBoxDestroy(pNewBox);
		xrtFree(sNewPath);
		return iRet;
	}
	if ( pDialog->pOverwriteBox != NULL ) xuiMsgBoxDestroy(pDialog->pOverwriteBox);
	if ( pDialog->sPendingSavePath != NULL ) xrtFree(pDialog->sPendingSavePath);
	pDialog->pOverwriteBox = pNewBox;
	pDialog->sPendingSavePath = sNewPath;
	return XUI_OK;
}

static int __xuiFileDialogCommitSavePath(xui_file_dialog pDialog, const char* sPath)
{
	if ( !__xuiFileDialogValid(pDialog) || sPath == NULL || sPath[0] == 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xrtFileExists((str)sPath) ) {
		return __xuiFileDialogShowOverwriteConfirm(pDialog, sPath);
	}
	return __xuiFileDialogFinishWithPath(pDialog, sPath);
}

static void __xuiFileDialogRootSelect(xui_widget pWidget, int iNodeId, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	xui_file_dialog_root_t* pRoot;
	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) ) return;
	pRoot = __xuiFileDialogFindRoot(pDialog, iNodeId, NULL);
	if ( pRoot != NULL && pRoot->sPath != NULL && pRoot->sPath[0] != 0 )
		(void)xuiFileDialogSetDirectory(pDialog, pRoot->sPath);
}

static void __xuiFileDialogFileSelect(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;

	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) {
		return;
	}
	pDialog->iSelectedEntry = iIndex;
	(void)__xuiFileDialogSetSelectedName(pDialog, iIndex);
}

static void __xuiFileDialogFileActivate(xui_widget pWidget, int iIndex, int iButton, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;

	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) ||
	     iButton != XUI_POINTER_BUTTON_LEFT ||
	     iIndex < 0 || iIndex >= pDialog->iEntryCount ) return;
	pDialog->iSelectedEntry = iIndex;
	if ( pDialog->arrEntries[iIndex].bDir ) {
		(void)xuiFileDialogSetDirectory(pDialog, pDialog->arrEntries[iIndex].sPath);
		return;
	}
	if ( pDialog->iMode != XUI_FILE_DIALOG_MODE_SAVE_FILE ) {
		(void)xuiFileDialogCommit(pDialog);
	}
}

static int __xuiFileDialogRootContext(xui_widget pWidget, int iNodeId, int iVisibleIndex,
	float fX, float fY, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	xui_file_dialog_root_t* pRoot;
	int iIndex = -1;

	(void)pWidget;
	(void)iVisibleIndex;
	if ( !__xuiFileDialogValid(pDialog) || pDialog->onContext == NULL ) return XUI_OK;
	pRoot = __xuiFileDialogFindRoot(pDialog, iNodeId, &iIndex);
	if ( pRoot == NULL || pRoot->sPath == NULL || pRoot->sPath[0] == 0 ) return XUI_OK;
	return pDialog->onContext(pDialog, iIndex, pRoot->sPath, 1,
		fX, fY, pDialog->pContextUser);
}

static int __xuiFileDialogListContext(xui_widget pWidget, int iIndex, float fX, float fY, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	const char* sPath = "";
	int bDirectory = 0;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->onContext == NULL ) return XUI_OK;
	if ( pWidget == pDialog->pFileList && iIndex >= 0 && iIndex < pDialog->iEntryCount ) {
		pDialog->iSelectedEntry = iIndex;
		sPath = pDialog->arrEntries[iIndex].sPath;
		bDirectory = pDialog->arrEntries[iIndex].bDir;
		(void)__xuiFileDialogSetSelectedName(pDialog, iIndex);
	}
	return pDialog->onContext(pDialog, iIndex, sPath, bDirectory,
		fX, fY, pDialog->pContextUser);
}

static void __xuiFileDialogPathChange(xui_widget pWidget, const char* sText, void* pUser)
{
	(void)sText;
	(void)pUser;
	(void)xuiInputSetError(pWidget, 0);
}

static void __xuiFileDialogNameChange(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	(void)sText;
	if ( __xuiFileDialogValid(pDialog) ) {
		if ( !pDialog->bInternalNameChange ) {
			__xuiFileDialogClearSelection(pDialog);
		}
	}
	(void)xuiInputSetError(pWidget, 0);
}

static int __xuiFileDialogSyncPathInput(xui_file_dialog pDialog)
{
	const char* sPath;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pDialog->bPathEditMode ) {
		return XUI_OK;
	}
	sPath = xuiInputGetText(pDialog->pPathInput);
	if ( sPath == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sPath[0] == 0 || xrtDirExists((str)sPath) ) {
		if ( pDialog->sCurrentDir == NULL || strcmp(pDialog->sCurrentDir, sPath) != 0 ) {
			iRet = xuiFileDialogSetDirectory(pDialog, sPath);
			if ( iRet != XUI_OK ) return iRet;
		}
		(void)xuiInputSetError(pDialog->pPathInput, 0);
		(void)__xuiFileDialogSetPathEditMode(pDialog, 0);
		return XUI_OK;
	}
	(void)__xuiFileDialogShowPathError(pDialog, sPath);
	return XUI_ERROR_FILE_NOT_FOUND;
}

static void __xuiFileDialogUpClick(xui_widget pWidget, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;

	(void)pWidget;
	if ( __xuiFileDialogSyncPathInput(pDialog) == XUI_OK ) {
		(void)xuiFileDialogGoUp(pDialog);
	}
}

static int __xuiFileDialogCommitPathInput(xui_file_dialog pDialog)
{
	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiFileDialogSyncPathInput(pDialog);
}

static int __xuiFileDialogCommitNameInput(xui_file_dialog pDialog)
{
	const char* sName;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) {
		(void)xuiInputSetError(pDialog->pNameInput, 0);
		return xuiFileDialogCommit(pDialog);
	}
	sName = xuiInputGetText(pDialog->pNameInput);
	if ( sName == NULL || sName[0] == 0 ) {
		(void)xuiInputSetError(pDialog->pNameInput, 1);
		return XUI_OK;
	}
	iRet = xuiFileDialogCommit(pDialog);
	return iRet == XUI_ERROR_FILE_NOT_FOUND ? XUI_OK : iRet;
}

static void __xuiFileDialogDefaultAction(xui_widget pWidget, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) ) {
		return;
	}
	if ( xuiGetFocusWidget(pDialog->pContext) == pDialog->pPathInput ) {
		(void)__xuiFileDialogCommitPathInput(pDialog);
	} else {
		(void)__xuiFileDialogCommitNameInput(pDialog);
	}
}

static void __xuiFileDialogBreadcrumbClick(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	char* sPath;
	size_t iLen;
	int iRet;

	(void)pWidget;
	(void)iIndex;
	if ( !__xuiFileDialogValid(pDialog) || iValue < 0 ) {
		return;
	}
	iLen = strlen(pDialog->sCurrentDir != NULL ? pDialog->sCurrentDir : "");
	if ( (size_t)iValue > iLen ) {
		return;
	}
	if ( iValue == 0 ) {
		(void)xuiFileDialogSetDirectory(pDialog, "");
		return;
	}
	sPath = (char*)xrtMalloc((size_t)iValue + 1u);
	if ( sPath == NULL ) {
		return;
	}
	memcpy(sPath, pDialog->sCurrentDir, (size_t)iValue);
	sPath[iValue] = 0;
	iRet = xuiFileDialogSetDirectory(pDialog, sPath);
	xrtFree(sPath);
	(void)iRet;
}

static void __xuiFileDialogFilterSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	char* sNewFilter;
	char* sOldFilter;
	int iOldFilter;
	int iRet;

	(void)pWidget;
	(void)iValue;
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iFilterCount ) {
		return;
	}
	if ( iIndex == pDialog->iSelectedFilter ) return;
	sNewFilter = __xuiFileDialogDup(pDialog->arrFilterPatterns[iIndex]);
	if ( sNewFilter == NULL ) {
		(void)xuiComboBoxSetSelected(pDialog->pFilterCombo, pDialog->iSelectedFilter);
		return;
	}
	sOldFilter = pDialog->sFilter;
	iOldFilter = pDialog->iSelectedFilter;
	pDialog->iSelectedFilter = iIndex;
	pDialog->sFilter = sNewFilter;
	iRet = __xuiFileDialogRefreshInternal(pDialog, 0);
	if ( iRet != XUI_OK ) {
		pDialog->iSelectedFilter = iOldFilter;
		pDialog->sFilter = sOldFilter;
		xrtFree(sNewFilter);
		(void)xuiComboBoxSetSelected(pDialog->pFilterCombo, iOldFilter);
	} else if ( sOldFilter != NULL ) {
		xrtFree(sOldFilter);
	}
}

static int __xuiFileDialogPathBarEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	int bLeftButton;

	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) || pEvent == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE &&
	     pEvent->iType == XUI_EVENT_POINTER_DOWN &&
	     bLeftButton &&
	     !pDialog->bPathEditMode &&
	     __xuiFileDialogBreadcrumbHit(pDialog->pPathBreadcrumb, pEvent->fX, pEvent->fY) < 0 ) {
		(void)__xuiFileDialogSetPathEditMode(pDialog, 1);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiFileDialogPathInputBlurEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pEvent == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->onPathInputBlur != NULL ) {
		iRet = pDialog->onPathInputBlur(pWidget, pEvent, pDialog->pPathInputBlurUser);
		if ( iRet < 0 ) return iRet;
	} else {
		iRet = XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_BLUR && xuiWidgetGetVisible(pDialog->pPathInput) && !pDialog->bPathApplying ) {
		pDialog->bPathApplying = 1;
		(void)__xuiFileDialogCommitPathInput(pDialog);
		pDialog->bPathApplying = 0;
	}
	return iRet;
}

static int __xuiFileDialogWindowPointerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	xui_rect_t tPath;
	int bLeftButton;
	int iRet;

	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) || pEvent == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	if ( pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ||
	     pEvent->iType != XUI_EVENT_POINTER_DOWN ||
	     !bLeftButton ||
	     !xuiWidgetGetVisible(pDialog->pPathInput) ) {
		return XUI_OK;
	}
	tPath = xuiWidgetGetWorldRect(pDialog->pPathBar);
	if ( pEvent->fX >= tPath.fX && pEvent->fY >= tPath.fY &&
	     pEvent->fX < tPath.fX + tPath.fW && pEvent->fY < tPath.fY + tPath.fH ) {
		return XUI_OK;
	}
	pDialog->bPathApplying = 1;
	iRet = __xuiFileDialogCommitPathInput(pDialog);
	pDialog->bPathApplying = 0;
	if ( iRet != XUI_OK ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiFileDialogBackdropEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;

	(void)pWidget;
	if ( !__xuiFileDialogValid(pDialog) || pEvent == NULL || !pDialog->bOpen || !pDialog->bModal ) {
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		(void)xuiSetFocusWidget(pDialog->pContext, pDialog->pWindow);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiFileDialogWindowEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;
	xui_context pContext = xuiWidgetGetContext(pWidget);
	int iRet = XUI_OK;

	if ( __xuiFileDialogValid(pDialog) && pDialog->onWindowEvent != NULL ) {
		iRet = pDialog->onWindowEvent(pWidget, pEvent, pDialog->pWindowEventUser);
		if ( iRet < 0 ) return iRet;
	}
	if ( !xuiInternalWidgetIsValid(pWidget) || xuiInternalContextDestroyPending(pContext) ) return iRet;
	if ( __xuiFileDialogValid(pDialog) && pDialog->bOpen && pDialog->bModal ) {
		int iLayerRet = xuiInternalWidgetSetTreeLayer(pWidget, XUI_LAYER_MODAL, pDialog->iModalZ + 1);
		if ( iLayerRet != XUI_OK ) return iLayerRet;
	}
	return iRet;
}

static void __xuiFileDialogCancelAction(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)xuiFileDialogCancel((xui_file_dialog)pUser);
}

static void __xuiFileDialogRefreshClick(xui_widget pWidget, void* pUser)
{
	xui_file_dialog pDialog = (xui_file_dialog)pUser;

	(void)pWidget;
	if ( __xuiFileDialogSyncPathInput(pDialog) == XUI_OK ) {
		(void)xuiFileDialogRefresh(pDialog);
	}
}

static void __xuiFileDialogOkClick(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)__xuiFileDialogCommitNameInput((xui_file_dialog)pUser);
}

static void __xuiFileDialogCancelClick(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)xuiFileDialogCancel((xui_file_dialog)pUser);
}

static void __xuiFileDialogWindowClose(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)xuiFileDialogCancel((xui_file_dialog)pUser);
}

static int __xuiFileDialogLayoutInitial(xui_file_dialog pDialog)
{
	xui_size_t tViewport;
	float fW;
	float fH;
	float fX;
	float fY;

	if ( pDialog == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fW = pDialog->fWidth;
	fH = pDialog->fHeight;
	fW = __xuiFileDialogMaxFloat(XUI_FILE_DIALOG_MIN_WIDTH, fW);
	fH = __xuiFileDialogMaxFloat(XUI_FILE_DIALOG_MIN_HEIGHT, fH);
	tViewport = xuiGetViewportSize(pDialog->pContext);
	if ( tViewport.iW > 0 ) {
		fW = (fW > (float)tViewport.iW - 24.0f) ? __xuiFileDialogMaxFloat(XUI_FILE_DIALOG_MIN_WIDTH, (float)tViewport.iW - 24.0f) : fW;
	}
	if ( tViewport.iH > 0 ) {
		fH = (fH > (float)tViewport.iH - 24.0f) ? __xuiFileDialogMaxFloat(XUI_FILE_DIALOG_MIN_HEIGHT, (float)tViewport.iH - 24.0f) : fH;
	}
	fX = ((float)tViewport.iW > fW) ? (((float)tViewport.iW - fW) * 0.5f) : 12.0f;
	fY = ((float)tViewport.iH > fH) ? (((float)tViewport.iH - fH) * 0.5f) : 12.0f;
	return xuiWidgetSetRect(pDialog->pWindow, xuiRectFromFloatNearest(fX, fY, fW, fH));
}

static int __xuiFileDialogApplyOverlay(xui_file_dialog pDialog)
{
	xui_size_t tViewport;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->pWindow == NULL || pDialog->pBackdrop == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->bOpen && pDialog->bModal ) {
		if ( xuiWidgetGetParent(pDialog->pBackdrop) == NULL ) {
			pDialog->iModalZ = xuiInternalOverlayNextZ(pDialog->pContext, XUI_LAYER_MODAL);
		}
		tViewport = xuiGetViewportSize(pDialog->pContext);
		iRet = xuiWidgetSetRect(pDialog->pBackdrop, (xui_rect_t){
			0.0f, 0.0f,
			tViewport.iW > 0 ? (float)tViewport.iW : 0.0f,
			tViewport.iH > 0 ? (float)tViewport.iH : 0.0f
		});
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pDialog->pBackdrop, 1);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pDialog->pBackdrop, 1);
		if ( iRet == XUI_OK ) iRet = xuiOverlayAttach(pDialog->pContext, pDialog->pWindow,
			pDialog->pBackdrop, XUI_LAYER_MODAL, pDialog->iModalZ);
		if ( iRet == XUI_OK ) iRet = xuiInternalWidgetSetTreeLayer(pDialog->pWindow,
			XUI_LAYER_MODAL, pDialog->iModalZ + 1);
		if ( iRet != XUI_OK ) return iRet;
		pDialog->pWindow->bModalFocus = 1;
		return xuiOverlayBringToFront(pDialog->pWindow);
	}
	pDialog->pWindow->bModalFocus = 0;
	iRet = xuiWidgetSetVisible(pDialog->pBackdrop, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pDialog->pBackdrop, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( xuiWidgetGetParent(pDialog->pBackdrop) != NULL ) {
		iRet = xuiOverlayDetach(pDialog->pBackdrop);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pDialog->bOpen ) {
		iRet = xuiInternalWidgetSetTreeLayer(pDialog->pWindow,
			XUI_LAYER_FLOATING, XUI_WINDOW_Z_TOPMOST);
		if ( iRet == XUI_OK ) iRet = xuiOverlayBringToFront(pDialog->pWindow);
		return iRet;
	}
	return XUI_OK;
}

XUI_API int xuiFileDialogCreate(xui_context pContext, xui_file_dialog* ppDialog, const xui_file_dialog_desc_t* pDesc)
{
	xui_file_dialog pDialog;
	xui_window_desc_t tWindow;
	xui_label_desc_t tLabel;
	xui_input_desc_t tInput;
	xui_button_desc_t tButton;
	xui_list_view_desc_t tList;
	xui_tree_view_desc_t tTree;
	xui_combobox_desc_t tCombo;
	xui_breadcrumb_desc_t tBreadcrumb;
	xui_msgtip_desc_t tTip;
	xui_widget pClient;
	xui_widget_type pWindowType, pPathType, pListType;
	void* pWindowData;
	void* pListData;
	xui_cache_policy_t tPathPolicy;
	const char* sTitle;
	const char* sInitialDir;
	int iRet;

	if ( ppDialog == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppDialog = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiFileDialogDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pWindowType = __xuiFileDialogType(pContext, "file-dialog", xuiWindowGetType(pContext));
	pPathType = __xuiFileDialogType(pContext, "file-dialog-path-bar",
		__xuiFileDialogType(pContext, "file-dialog-path-theme", xuiWidgetGetBaseType()));
	pListType = __xuiFileDialogType(pContext, "file-dialog-list",
		__xuiFileDialogType(pContext, "file-dialog-list-theme", xuiListViewGetType(pContext)));
	if ( pWindowType == NULL || pPathType == NULL || pListType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiFileDialogRegisterColors(pContext, pPathType, pListType);
	pDialog = (xui_file_dialog)xrtCalloc(1, sizeof(*pDialog));
	if ( pDialog == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDialog->iMagic = XUI_FILE_DIALOG_MAGIC;
	pDialog->pContext = pContext;
	pDialog->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pDialog->iMode = (pDesc != NULL && pDesc->iMode != 0) ? pDesc->iMode : XUI_FILE_DIALOG_MODE_OPEN_FILE;
	pDialog->fWidth = (pDesc != NULL && pDesc->fWidth > 0.0f) ? pDesc->fWidth : 720.0f;
	pDialog->fHeight = (pDesc != NULL && pDesc->fHeight > 0.0f) ? pDesc->fHeight : 520.0f;
	pDialog->iSelectedEntry = -1;
	pDialog->iResult = XUI_FILE_DIALOG_RESULT_NONE;
	pDialog->bModal = (pDesc != NULL && pDesc->bModal) ? 1 : 0;
	pDialog->onResult = (pDesc != NULL) ? pDesc->onResult : NULL;
	pDialog->pResultUser = (pDesc != NULL) ? pDesc->pResultUser : NULL;
	iRet = __xuiFileDialogSetFilters(pDialog, (pDesc != NULL) ? pDesc->sFilter : NULL);
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogSetString(&pDialog->sFileName, (pDesc != NULL) ? pDesc->sFileName : NULL);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(pDialog);
		return iRet;
	}
	memset(&tWindow, 0, sizeof(tWindow));
	tWindow.iSize = sizeof(tWindow);
	sTitle = (pDesc != NULL && pDesc->sTitle != NULL) ? pDesc->sTitle : __xuiFileDialogDefaultTitle(pContext, pDialog->iMode);
	tWindow.sTitle = sTitle;
	tWindow.pFont = pDialog->pFont;
	tWindow.bClosed = 1;
	tWindow.bTopMost = 1;
	tWindow.bHideCollapse = 1;
	tWindow.bHideMaximize = 1;
	tWindow.fMinWidth = XUI_FILE_DIALOG_MIN_WIDTH;
	tWindow.fMinHeight = XUI_FILE_DIALOG_MIN_HEIGHT;
	iRet = xuiWidgetCreateTyped(pContext, pWindowType, &pDialog->pWindow, &tWindow);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(pDialog);
		return iRet;
	}
	iRet = xuiWidgetCreate(pContext, &pDialog->pBackdrop);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(pDialog);
		return iRet;
	}
	memset(&tLabel, 0, sizeof(tLabel));
	tLabel.iSize = sizeof(tLabel);
	tLabel.pFont = pDialog->pFont;
	tLabel.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tLabel.sText = xuiTranslate(pContext, XUI_TR_FILE_PATH);
	iRet = xuiLabelCreate(pContext, &pDialog->pPathLabel, &tLabel);
	tLabel.sText = (pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER) ? xuiTranslate(pContext, XUI_TR_FILE_FOLDER) : xuiTranslate(pContext, XUI_TR_FILE_FILE);
	if ( iRet == XUI_OK ) iRet = xuiLabelCreate(pContext, &pDialog->pNameLabel, &tLabel);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pDialog->pPathRow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pDialog->pListRow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pDialog->pFieldRow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pDialog->pButtonRow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pDialog->pButtonSpacer);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreateTyped(pContext, pPathType, &pDialog->pPathBar, NULL);
	memset(&tBreadcrumb, 0, sizeof(tBreadcrumb));
	tBreadcrumb.iSize = sizeof(tBreadcrumb);
	tBreadcrumb.pFont = pDialog->pFont;
	tBreadcrumb.sSeparator = ">";
	tBreadcrumb.fGap = 6.0f;
	tBreadcrumb.fPaddingX = 0.0f;
	tBreadcrumb.fPaddingY = 4.0f;
	if ( iRet == XUI_OK ) iRet = xuiBreadcrumbCreate(pContext, &pDialog->pPathBreadcrumb, &tBreadcrumb);
	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.pFont = pDialog->pFont;
	tInput.bReadonly = 0;
	if ( iRet == XUI_OK ) iRet = xuiInputCreate(pContext, &pDialog->pPathInput, &tInput);
	tInput.bReadonly = 0;
	if ( iRet == XUI_OK ) iRet = xuiInputCreate(pContext, &pDialog->pNameInput, &tInput);
	memset(&tCombo, 0, sizeof(tCombo));
	tCombo.iSize = sizeof(tCombo);
	tCombo.arrItems = (const char**)pDialog->arrFilterNames;
	tCombo.iItemCount = pDialog->iFilterCount;
	tCombo.iSelected = pDialog->iSelectedFilter;
	tCombo.iMode = XUI_COMBOBOX_MODE_SELECT;
	tCombo.pFont = pDialog->pFont;
	tCombo.fPopupMaxHeight = 220.0f;
	if ( iRet == XUI_OK ) iRet = xuiComboBoxCreate(pContext, &pDialog->pFilterCombo, &tCombo);
	memset(&tButton, 0, sizeof(tButton));
	tButton.iSize = sizeof(tButton);
	tButton.pFont = pDialog->pFont;
	tButton.fBorderWidth = 1.0f;
	tButton.sText = "";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pContext, &pDialog->pUpButton, &tButton);
	tButton.sText = "";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pContext, &pDialog->pRefreshButton, &tButton);
	tButton.sText = __xuiFileDialogOkText(pContext, pDialog->iMode);
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pContext, &pDialog->pOkButton, &tButton);
	tButton.sText = xuiTranslate(pContext, XUI_TR_FILE_CANCEL);
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pContext, &pDialog->pCancelButton, &tButton);
	memset(&tTree, 0, sizeof(tTree));
	tTree.iSize = sizeof(tTree);
	tTree.pFont = pDialog->pFont;
	tTree.fItemHeight = 26.0f;
	tTree.fIndent = 17.0f;
	tTree.fPadding = 5.0f;
	tTree.iSelectedId = -1;
	if ( iRet == XUI_OK ) iRet = xuiTreeViewCreate(pContext, &pDialog->pRootList, &tTree);
	memset(&tList, 0, sizeof(tList));
	tList.iSize = sizeof(tList);
	tList.pFont = pDialog->pFont;
	tList.iSelectionMode = XUI_SELECTION_SINGLE;
	tList.fItemHeight = 24.0f;
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreateTyped(pContext, pListType, &pDialog->pFileList, &tList);
	memset(&tTip, 0, sizeof(tTip));
	tTip.iSize = sizeof(tTip);
	tTip.pFont = pDialog->pFont;
	tTip.iType = XUI_MSGTIP_ICON_ERROR;
	tTip.sText = "";
	tTip.fDuration = 2.2f;
	if ( iRet == XUI_OK ) iRet = xuiMsgTipCreate(pContext, &pDialog->pPathTip, &tTip);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(pDialog);
		return iRet;
	}
	#define XUI_FILE_DIALOG_SETUP(...) \
		do { if ( iRet == XUI_OK ) iRet = (__VA_ARGS__); } while ( 0 )
	pClient = xuiWindowGetClientWidget(pDialog->pWindow);
	if ( pClient == NULL ) iRet = XUI_ERROR_INVALID_STATE;
	XUI_FILE_DIALOG_SETUP(xuiWindowSetClose(pDialog->pWindow, __xuiFileDialogWindowClose, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetGetEventCallback(pDialog->pWindow, &pDialog->onWindowEvent, &pDialog->pWindowEventUser));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventCallback(pDialog->pWindow, __xuiFileDialogWindowEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetVisible(pDialog->pBackdrop, 0));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEnabled(pDialog->pBackdrop, 0));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFocusable(pDialog->pBackdrop, 0));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetTabStop(pDialog->pBackdrop, 0));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pBackdrop, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_ENTER, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_LEAVE, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_MOVE, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_DOWN, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_UP, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_CLICK, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_POINTER_WHEEL, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pBackdrop, XUI_EVENT_CONTEXT_MENU, __xuiFileDialogBackdropEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWindowAddChild(pDialog->pWindow, pDialog->pPathRow));
	XUI_FILE_DIALOG_SETUP(xuiWindowAddChild(pDialog->pWindow, pDialog->pListRow));
	XUI_FILE_DIALOG_SETUP(xuiWindowAddChild(pDialog->pWindow, pDialog->pFieldRow));
	XUI_FILE_DIALOG_SETUP(xuiWindowAddChild(pDialog->pWindow, pDialog->pButtonRow));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pPathRow, pDialog->pPathLabel));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pPathRow, pDialog->pPathBar));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pPathRow, pDialog->pUpButton));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pPathRow, pDialog->pRefreshButton));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pPathBar, pDialog->pPathBreadcrumb));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pPathBar, pDialog->pPathInput));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pListRow, pDialog->pRootList));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pListRow, pDialog->pFileList));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pFieldRow, pDialog->pNameLabel));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pFieldRow, pDialog->pNameInput));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pFieldRow, pDialog->pFilterCombo));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pButtonRow, pDialog->pButtonSpacer));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pButtonRow, pDialog->pOkButton));
	XUI_FILE_DIALOG_SETUP(xuiWidgetAddChild(pDialog->pButtonRow, pDialog->pCancelButton));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetLayoutType(pClient, XUI_LAYOUT_COLUMN));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pClient, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPadding(pClient, (xui_thickness_t){14.0f, 12.0f, 14.0f, 14.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetGap(pClient, 8.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetLayoutType(pDialog->pPathRow, XUI_LAYOUT_ROW));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetLayoutType(pDialog->pListRow, XUI_LAYOUT_ROW));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetLayoutType(pDialog->pFieldRow, XUI_LAYOUT_ROW));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetLayoutType(pDialog->pButtonRow, XUI_LAYOUT_ROW));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetLayoutType(pDialog->pPathBar, XUI_LAYOUT_OVERLAY));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetGap(pDialog->pPathRow, 8.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetGap(pDialog->pListRow, 8.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetGap(pDialog->pFieldRow, 8.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetGap(pDialog->pButtonRow, 8.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pPathRow, XUI_SIZE_FILL, XUI_SIZE_FIXED));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pListRow, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pFieldRow, XUI_SIZE_FILL, XUI_SIZE_FIXED));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pButtonRow, XUI_SIZE_FILL, XUI_SIZE_FIXED));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pPathRow, (xui_vec2_t){0.0f, 32.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pFieldRow, (xui_vec2_t){0.0f, 28.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pButtonRow, (xui_vec2_t){0.0f, 28.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFlex(pDialog->pListRow, 1.0f, 1.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetMinSize(pDialog->pListRow, (xui_vec2_t){0.0f, 72.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pPathLabel, (xui_vec2_t){58.0f, 32.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pNameLabel, (xui_vec2_t){58.0f, 28.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pPathBar, (xui_vec2_t){0.0f, 32.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pRootList, (xui_vec2_t){168.0f, 0.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pFilterCombo, (xui_vec2_t){210.0f, 28.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pUpButton, (xui_vec2_t){32.0f, 32.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pRefreshButton, (xui_vec2_t){32.0f, 32.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pOkButton, (xui_vec2_t){78.0f, 28.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetPreferredSize(pDialog->pCancelButton, (xui_vec2_t){74.0f, 28.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pPathBar, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pPathBreadcrumb, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pPathInput, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pRootList, XUI_SIZE_FIXED, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pFileList, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pNameInput, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetSizeMode(pDialog->pButtonSpacer, XUI_SIZE_FILL, XUI_SIZE_FILL));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFlex(pDialog->pPathBar, 1.0f, 1.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFlex(pDialog->pFileList, 1.0f, 1.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFlex(pDialog->pNameInput, 1.0f, 1.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFlex(pDialog->pButtonSpacer, 1.0f, 1.0f));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetMargin(pDialog->pPathBreadcrumb, (xui_thickness_t){8.0f, 0.0f, 8.0f, 0.0f}));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetFocusable(pDialog->pPathBar, 0));
	memset(&tPathPolicy, 0, sizeof(tPathPolicy));
	tPathPolicy.iSize = sizeof(tPathPolicy);
	tPathPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPathPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetCachePolicy(pDialog->pPathBar, &tPathPolicy));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetCacheRenderCallback(pDialog->pPathBar, __xuiFileDialogPathBarRender, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventCallback(pDialog->pPathBar, __xuiFileDialogPathBarEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetVisible(pDialog->pPathInput, 0));
	XUI_FILE_DIALOG_SETUP(xuiButtonSetClick(pDialog->pUpButton, __xuiFileDialogUpClick, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiButtonSetClick(pDialog->pRefreshButton, __xuiFileDialogRefreshClick, pDialog));
	XUI_FILE_DIALOG_SETUP(__xuiFileDialogSetButtonIcon(pDialog, pDialog->pUpButton, XUI_FILE_DIALOG_ICON_UP));
	XUI_FILE_DIALOG_SETUP(__xuiFileDialogSetButtonIcon(pDialog, pDialog->pRefreshButton, XUI_FILE_DIALOG_ICON_REFRESH));
	XUI_FILE_DIALOG_SETUP(xuiButtonSetClick(pDialog->pOkButton, __xuiFileDialogOkClick, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiButtonSetClick(pDialog->pCancelButton, __xuiFileDialogCancelClick, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pWindow, XUI_EVENT_POINTER_DOWN, __xuiFileDialogWindowPointerEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetDefaultAction(pDialog->pWindow, __xuiFileDialogDefaultAction, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetCancelAction(pDialog->pWindow, __xuiFileDialogCancelAction, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetDefaultAction(pDialog->pPathInput, __xuiFileDialogDefaultAction, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetCancelAction(pDialog->pPathInput, __xuiFileDialogCancelAction, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetGetEventHandler(pDialog->pPathInput, XUI_EVENT_BLUR, &pDialog->onPathInputBlur, &pDialog->pPathInputBlurUser));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetEventHandler(pDialog->pPathInput, XUI_EVENT_BLUR, __xuiFileDialogPathInputBlurEvent, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetDefaultAction(pDialog->pNameInput, __xuiFileDialogDefaultAction, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiWidgetSetCancelAction(pDialog->pNameInput, __xuiFileDialogCancelAction, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiInputSetChange(pDialog->pPathInput, __xuiFileDialogPathChange, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiInputSetChange(pDialog->pNameInput, __xuiFileDialogNameChange, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiBreadcrumbSetClick(pDialog->pPathBreadcrumb, __xuiFileDialogBreadcrumbClick, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiComboBoxSetSelect(pDialog->pFilterCombo, __xuiFileDialogFilterSelect, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiTreeViewSetSelect(pDialog->pRootList, __xuiFileDialogRootSelect, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiListViewSetSelect(pDialog->pFileList, __xuiFileDialogFileSelect, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiListViewSetActivate(pDialog->pFileList, __xuiFileDialogFileActivate, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiTreeViewSetContextMenu(pDialog->pRootList, __xuiFileDialogRootContext, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiListViewSetContextMenu(pDialog->pFileList, __xuiFileDialogListContext, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiTreeViewSetItemRenderer(pDialog->pRootList, __xuiFileDialogDrawRootItem, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiListViewSetItemRenderer(pDialog->pFileList, __xuiFileDialogDrawFileItem, pDialog));
	XUI_FILE_DIALOG_SETUP(xuiListViewGetColors(pDialog->pFileList, &pDialog->arrListBase[0], &pDialog->arrListBase[1],
		&pDialog->arrListBase[2], &pDialog->arrListBase[3], &pDialog->arrListBase[4], &pDialog->arrListBase[5],
		&pDialog->arrListBase[6], &pDialog->arrListBase[7]));
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(pDialog);
		return iRet;
	}
	pWindowType->onPreparePaint = __xuiFileDialogWindowPreparePaint;
	pListType->onPreparePaint = __xuiFileDialogListPreparePaint;
	pWindowData = xuiWidgetGetTypeData(pDialog->pWindow);
	pListData = xuiWidgetGetTypeData(pDialog->pFileList);
	if ( pWindowData == NULL || pListData == NULL ) {
		xuiFileDialogDestroy(pDialog);
		return XUI_ERROR_INVALID_STATE;
	}
	memcpy((char*)pWindowData + pWindowType->pParent->iTypeDataSize, &pDialog, sizeof(pDialog));
	memcpy((char*)pListData + pListType->pParent->iTypeDataSize, &pDialog, sizeof(pDialog));
	if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) {
		XUI_FILE_DIALOG_SETUP(xuiWidgetSetVisible(pDialog->pFilterCombo, 0));
		XUI_FILE_DIALOG_SETUP(xuiInputSetReadonly(pDialog->pNameInput, 1));
	}
	XUI_FILE_DIALOG_SETUP(__xuiFileDialogLayoutInitial(pDialog));
	#undef XUI_FILE_DIALOG_SETUP
	sInitialDir = (pDesc != NULL && pDesc->sInitialDir != NULL) ? pDesc->sInitialDir : ".";
	if ( iRet == XUI_OK ) iRet = xuiFileDialogSetDirectory(pDialog, sInitialDir);
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogSetPathEditMode(pDialog, 0);
	if ( iRet == XUI_OK ) {
		iRet = __xuiFileDialogSetNameText(
			pDialog,
			(pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER) ? "" : (pDialog->sFileName != NULL ? pDialog->sFileName : "")
		);
	}
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(pDialog);
		return iRet;
	}
	*ppDialog = pDialog;
	return XUI_OK;
}

XUI_API void xuiFileDialogDestroy(xui_file_dialog pDialog)
{
	if ( !__xuiFileDialogValid(pDialog) ) {
		return;
	}
	(void)xuiFileDialogSetOpen(pDialog, 0);
	if ( pDialog->pPathTip != NULL ) {
		xuiMsgTipDestroy(pDialog->pPathTip);
		pDialog->pPathTip = NULL;
	}
	if ( pDialog->pOverwriteBox != NULL ) {
		xuiMsgBoxDestroy(pDialog->pOverwriteBox);
		pDialog->pOverwriteBox = NULL;
	}
	if ( pDialog->pBackdrop != NULL ) {
		xuiWidgetDestroy(pDialog->pBackdrop);
		pDialog->pBackdrop = NULL;
	}
	__xuiFileDialogDestroyOrphan(pDialog->pPathLabel);
	__xuiFileDialogDestroyOrphan(pDialog->pNameLabel);
	__xuiFileDialogDestroyOrphan(pDialog->pPathBreadcrumb);
	__xuiFileDialogDestroyOrphan(pDialog->pPathInput);
	__xuiFileDialogDestroyOrphan(pDialog->pNameInput);
	__xuiFileDialogDestroyOrphan(pDialog->pFilterCombo);
	__xuiFileDialogDestroyOrphan(pDialog->pUpButton);
	__xuiFileDialogDestroyOrphan(pDialog->pRefreshButton);
	__xuiFileDialogDestroyOrphan(pDialog->pOkButton);
	__xuiFileDialogDestroyOrphan(pDialog->pCancelButton);
	__xuiFileDialogDestroyOrphan(pDialog->pRootList);
	__xuiFileDialogDestroyOrphan(pDialog->pFileList);
	__xuiFileDialogDestroyOrphan(pDialog->pButtonSpacer);
	__xuiFileDialogDestroyOrphan(pDialog->pPathBar);
	__xuiFileDialogDestroyOrphan(pDialog->pPathRow);
	__xuiFileDialogDestroyOrphan(pDialog->pListRow);
	__xuiFileDialogDestroyOrphan(pDialog->pFieldRow);
	__xuiFileDialogDestroyOrphan(pDialog->pButtonRow);
	if ( pDialog->pWindow != NULL ) {
		xuiWidgetDestroy(pDialog->pWindow);
	}
	__xuiFileDialogFreeRootItems(pDialog);
	__xuiFileDialogReleaseEntryStorage(pDialog);
	if ( pDialog->sCurrentDir != NULL ) xrtFree(pDialog->sCurrentDir);
	if ( pDialog->sFileName != NULL ) xrtFree(pDialog->sFileName);
	if ( pDialog->sFilter != NULL ) xrtFree(pDialog->sFilter);
	if ( pDialog->sResultPath != NULL ) xrtFree(pDialog->sResultPath);
	if ( pDialog->sPendingSavePath != NULL ) xrtFree(pDialog->sPendingSavePath);
	__xuiFileDialogFreeFilters(pDialog);
	pDialog->iMagic = 0;
	xrtFree(pDialog);
}

XUI_API int xuiOpenFileDialog(xui_context pContext, xui_file_dialog* ppDialog, const xui_file_dialog_desc_t* pDesc)
{
	xui_file_dialog_desc_t tDesc;
	int iRet;
	if ( ppDialog == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppDialog = NULL;
	if ( !__xuiFileDialogDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDesc != NULL ) {
		tDesc = *pDesc;
	} else {
		memset(&tDesc, 0, sizeof(tDesc));
	}
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = XUI_FILE_DIALOG_MODE_OPEN_FILE;
	tDesc.bModal = 1;
	iRet = xuiFileDialogCreate(pContext, ppDialog, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiFileDialogSetOpen(*ppDialog, 1);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(*ppDialog);
		*ppDialog = NULL;
	}
	return iRet;
}

XUI_API int xuiSaveFileDialog(xui_context pContext, xui_file_dialog* ppDialog, const xui_file_dialog_desc_t* pDesc)
{
	xui_file_dialog_desc_t tDesc;
	int iRet;
	if ( ppDialog == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppDialog = NULL;
	if ( !__xuiFileDialogDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDesc != NULL ) tDesc = *pDesc; else memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = XUI_FILE_DIALOG_MODE_SAVE_FILE;
	tDesc.bModal = 1;
	iRet = xuiFileDialogCreate(pContext, ppDialog, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiFileDialogSetOpen(*ppDialog, 1);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(*ppDialog);
		*ppDialog = NULL;
	}
	return iRet;
}

XUI_API int xuiSelectFolderDialog(xui_context pContext, xui_file_dialog* ppDialog, const xui_file_dialog_desc_t* pDesc)
{
	xui_file_dialog_desc_t tDesc;
	int iRet;
	if ( ppDialog == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppDialog = NULL;
	if ( !__xuiFileDialogDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDesc != NULL ) tDesc = *pDesc; else memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = XUI_FILE_DIALOG_MODE_SELECT_FOLDER;
	tDesc.bModal = 1;
	iRet = xuiFileDialogCreate(pContext, ppDialog, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiFileDialogSetOpen(*ppDialog, 1);
	if ( iRet != XUI_OK ) {
		xuiFileDialogDestroy(*ppDialog);
		*ppDialog = NULL;
	}
	return iRet;
}

XUI_API int xuiFileDialogSetOpen(xui_file_dialog pDialog, int bOpen)
{
	int bNewOpen;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bNewOpen = bOpen ? 1 : 0;
	if ( pDialog->bOpen == bNewOpen ) {
		if ( bNewOpen ) {
			iRet = xuiWindowSetOpen(pDialog->pWindow, 1);
			if ( iRet == XUI_OK ) iRet = __xuiFileDialogApplyOverlay(pDialog);
			return iRet;
		}
		return XUI_OK;
	}
	if ( !bNewOpen && pDialog->pOverwriteBox != NULL && xuiMsgBoxIsOpen(pDialog->pOverwriteBox) ) {
		iRet = xuiMsgBoxSetOpen(pDialog->pOverwriteBox, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( bNewOpen ) {
		pDialog->bOpen = 1;
		/* Prepare the modal focus scope before Window activation so nested
		 * dialogs can receive focus above an existing modal surface. */
		iRet = __xuiFileDialogApplyOverlay(pDialog);
		if ( iRet == XUI_OK ) iRet = xuiWindowSetOpen(pDialog->pWindow, 1);
		if ( iRet != XUI_OK ) {
			pDialog->bOpen = 0;
			(void)__xuiFileDialogApplyOverlay(pDialog);
			(void)xuiWindowSetOpen(pDialog->pWindow, 0);
			return iRet;
		}
	} else {
		iRet = xuiWindowSetOpen(pDialog->pWindow, 0);
		if ( iRet != XUI_OK ) return iRet;
		pDialog->bOpen = 0;
		iRet = __xuiFileDialogApplyOverlay(pDialog);
		if ( iRet != XUI_OK ) {
			pDialog->bOpen = 1;
			(void)xuiWindowSetOpen(pDialog->pWindow, 1);
			(void)__xuiFileDialogApplyOverlay(pDialog);
			return iRet;
		}
	}
	if ( bNewOpen && pDialog->iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE && pDialog->pNameInput != NULL ) {
		iRet = xuiSetFocusWidget(pDialog->pContext, pDialog->pNameInput);
		if ( iRet == XUI_OK ) iRet = xuiInputSelectAll(pDialog->pNameInput);
		if ( iRet != XUI_OK ) {
			pDialog->bOpen = 0;
			(void)__xuiFileDialogApplyOverlay(pDialog);
			(void)xuiWindowSetOpen(pDialog->pWindow, 0);
			return iRet;
		}
	}
	if ( bNewOpen ) {
		pDialog->iResult = XUI_FILE_DIALOG_RESULT_NONE;
		if ( pDialog->sResultPath != NULL ) {
			xrtFree(pDialog->sResultPath);
			pDialog->sResultPath = NULL;
		}
		if ( pDialog->sPendingSavePath != NULL ) {
			xrtFree(pDialog->sPendingSavePath);
			pDialog->sPendingSavePath = NULL;
		}
	}
	return XUI_OK;
}

XUI_API int xuiFileDialogIsOpen(xui_file_dialog pDialog)
{
	return __xuiFileDialogValid(pDialog) ? pDialog->bOpen : 0;
}

XUI_API int xuiFileDialogSetModal(xui_file_dialog pDialog, int bModal)
{
	int bOldModal;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) return XUI_ERROR_INVALID_ARGUMENT;
	bModal = bModal ? 1 : 0;
	if ( pDialog->bModal == bModal ) return XUI_OK;
	bOldModal = pDialog->bModal;
	pDialog->bModal = bModal;
	iRet = __xuiFileDialogApplyOverlay(pDialog);
	if ( iRet != XUI_OK ) {
		pDialog->bModal = bOldModal;
		(void)__xuiFileDialogApplyOverlay(pDialog);
		return iRet;
	}
	return XUI_OK;
}

XUI_API int xuiFileDialogIsModal(xui_file_dialog pDialog)
{
	return __xuiFileDialogValid(pDialog) ? pDialog->bModal : 0;
}

XUI_API int xuiFileDialogSetResult(xui_file_dialog pDialog, xui_file_dialog_result_proc onResult, void* pUser)
{
	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDialog->onResult = onResult;
	pDialog->pResultUser = pUser;
	return XUI_OK;
}

XUI_API int xuiFileDialogSetContextMenu(xui_file_dialog pDialog, xui_file_dialog_context_proc onContext, void* pUser)
{
	if ( !__xuiFileDialogValid(pDialog) ) return XUI_ERROR_INVALID_ARGUMENT;
	pDialog->onContext = onContext;
	pDialog->pContextUser = pUser;
	return XUI_OK;
}

XUI_API int xuiFileDialogSetFilter(xui_file_dialog pDialog, const char* sFilter)
{
	xui_file_dialog_t tOldFilters;
	int iRestore;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tOldFilters, 0, sizeof(tOldFilters));
	tOldFilters.arrFilterNames = pDialog->arrFilterNames;
	tOldFilters.arrFilterPatterns = pDialog->arrFilterPatterns;
	tOldFilters.iFilterCount = pDialog->iFilterCount;
	tOldFilters.iFilterCapacity = pDialog->iFilterCapacity;
	tOldFilters.iSelectedFilter = pDialog->iSelectedFilter;
	tOldFilters.sFilter = pDialog->sFilter;
	pDialog->arrFilterNames = NULL;
	pDialog->arrFilterPatterns = NULL;
	pDialog->iFilterCount = 0;
	pDialog->iFilterCapacity = 0;
	pDialog->iSelectedFilter = -1;
	pDialog->sFilter = NULL;
	iRet = __xuiFileDialogSetFilters(pDialog, sFilter);
	if ( iRet == XUI_OK && pDialog->pFilterCombo != NULL ) {
		iRet = xuiComboBoxSetItems(pDialog->pFilterCombo,
			(const char**)pDialog->arrFilterNames, pDialog->iFilterCount);
		if ( iRet == XUI_OK ) {
			iRet = xuiComboBoxSetSelected(pDialog->pFilterCombo, pDialog->iSelectedFilter);
		}
	}
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogRefreshInternal(pDialog, 0);
	if ( iRet == XUI_OK ) {
		__xuiFileDialogFreeFilters(&tOldFilters);
		if ( tOldFilters.sFilter != NULL ) xrtFree(tOldFilters.sFilter);
		return XUI_OK;
	}
	__xuiFileDialogFreeFilters(pDialog);
	if ( pDialog->sFilter != NULL ) xrtFree(pDialog->sFilter);
	pDialog->arrFilterNames = tOldFilters.arrFilterNames;
	pDialog->arrFilterPatterns = tOldFilters.arrFilterPatterns;
	pDialog->iFilterCount = tOldFilters.iFilterCount;
	pDialog->iFilterCapacity = tOldFilters.iFilterCapacity;
	pDialog->iSelectedFilter = tOldFilters.iSelectedFilter;
	pDialog->sFilter = tOldFilters.sFilter;
	if ( pDialog->pFilterCombo != NULL ) {
		iRestore = xuiComboBoxSetItems(pDialog->pFilterCombo,
			(const char**)pDialog->arrFilterNames, pDialog->iFilterCount);
		if ( iRestore == XUI_OK ) {
			(void)xuiComboBoxSetSelected(pDialog->pFilterCombo, pDialog->iSelectedFilter);
		}
	}
	return iRet;
}

XUI_API const char* xuiFileDialogGetFilter(xui_file_dialog pDialog)
{
	return __xuiFileDialogValid(pDialog) && pDialog->sFilter != NULL ? pDialog->sFilter : "";
}

XUI_API int xuiFileDialogSetDirectory(xui_file_dialog pDialog, const char* sDir)
{
	xui_file_dialog_t tEntries;
	char* sNewDir;
	char* sOldDir;
	char* sOldName;
	int bChanged;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sDir == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sDir[0] != 0 && !xrtDirExists((str)sDir) ) return XUI_ERROR_FILE_NOT_FOUND;
	if ( pDialog->iRootCount == 0 ) {
		iRet = __xuiFileDialogBuildRoots(pDialog);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiFileDialogBuildEntries(pDialog, sDir, &tEntries);
	if ( iRet != XUI_OK ) return iRet;
	sNewDir = __xuiFileDialogDup(sDir);
	if ( sNewDir == NULL ) {
		__xuiFileDialogReleaseEntryStorage(&tEntries);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	bChanged = pDialog->sCurrentDir == NULL || strcmp(pDialog->sCurrentDir, sDir) != 0;
	sOldName = NULL;
	if ( bChanged && pDialog->iMode != XUI_FILE_DIALOG_MODE_SAVE_FILE ) {
		sOldName = __xuiFileDialogDup(xuiInputGetText(pDialog->pNameInput));
		if ( sOldName == NULL ) {
			xrtFree(sNewDir);
			__xuiFileDialogReleaseEntryStorage(&tEntries);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	sOldDir = pDialog->sCurrentDir;
	pDialog->sCurrentDir = sNewDir;
	iRet = xuiInputSetText(pDialog->pPathInput, sNewDir);
	if ( iRet == XUI_OK ) iRet = xuiInputSetError(pDialog->pPathInput, 0);
	if ( iRet == XUI_OK ) iRet = xuiInputSetError(pDialog->pNameInput, 0);
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogUpdateBreadcrumb(pDialog);
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogSyncRootSelection(pDialog);
	if ( iRet == XUI_OK && sOldName != NULL ) iRet = __xuiFileDialogSetNameText(pDialog, "");
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogReplaceEntries(pDialog, &tEntries);
	if ( iRet != XUI_OK ) {
		pDialog->sCurrentDir = sOldDir;
		(void)xuiInputSetText(pDialog->pPathInput, sOldDir != NULL ? sOldDir : "");
		(void)__xuiFileDialogUpdateBreadcrumb(pDialog);
		(void)__xuiFileDialogSyncRootSelection(pDialog);
		if ( sOldName != NULL ) (void)__xuiFileDialogSetNameText(pDialog, sOldName);
		xrtFree(sNewDir);
		if ( sOldName != NULL ) xrtFree(sOldName);
		__xuiFileDialogReleaseEntryStorage(&tEntries);
		return iRet;
	}
	if ( sOldDir != NULL ) xrtFree(sOldDir);
	if ( sOldName != NULL ) xrtFree(sOldName);
	__xuiFileDialogReleaseEntryStorage(&tEntries);
	return XUI_OK;
}

XUI_API const char* xuiFileDialogGetDirectory(xui_file_dialog pDialog)
{
	return __xuiFileDialogValid(pDialog) && pDialog->sCurrentDir != NULL ? pDialog->sCurrentDir : "";
}

XUI_API int xuiFileDialogGoUp(xui_file_dialog pDialog)
{
	char* sParent;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || pDialog->sCurrentDir == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->sCurrentDir[0] == 0 ) {
		return XUI_OK;
	}
	sParent = __xuiFileDialogParentDir(pDialog->sCurrentDir);
	if ( sParent == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = xuiFileDialogSetDirectory(pDialog, sParent);
	xrtFree(sParent);
	return iRet;
}

static int __xuiFileDialogRefreshInternal(xui_file_dialog pDialog, int bRefreshRoots)
{
	xui_file_dialog_t tEntries;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( bRefreshRoots || pDialog->iRootCount == 0 ) {
		iRet = __xuiFileDialogBuildRoots(pDialog);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiFileDialogSyncRootSelection(pDialog);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFileDialogBuildEntries(pDialog,
		pDialog->sCurrentDir != NULL ? pDialog->sCurrentDir : "", &tEntries);
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogReplaceEntries(pDialog, &tEntries);
	__xuiFileDialogReleaseEntryStorage(&tEntries);
	return iRet;
}

XUI_API int xuiFileDialogRefresh(xui_file_dialog pDialog)
{
	return __xuiFileDialogRefreshInternal(pDialog, 1);
}

XUI_API int xuiFileDialogSelectIndex(xui_file_dialog pDialog, int iIndex)
{
	int iOldIndex;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iOldIndex = pDialog->iSelectedEntry;
	pDialog->iSelectedEntry = iIndex;
	iRet = xuiListViewSetSelected(pDialog->pFileList, iIndex);
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogSetSelectedName(pDialog, iIndex);
	if ( iRet != XUI_OK ) {
		pDialog->iSelectedEntry = iOldIndex;
		(void)xuiListViewSetSelected(pDialog->pFileList, iOldIndex);
	}
	return iRet;
}

XUI_API int xuiFileDialogCommit(xui_file_dialog pDialog)
{
	const char* sName;
	char* sPath;
	xui_file_dialog_entry_t* pEntry;
	int iRet;

	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDialog->iSelectedEntry >= 0 && pDialog->iSelectedEntry < pDialog->iEntryCount ) {
		pEntry = &pDialog->arrEntries[pDialog->iSelectedEntry];
		if ( pEntry->bDir && pDialog->iMode != XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) {
			return xuiFileDialogSetDirectory(pDialog, pEntry->sPath);
		}
		if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) {
			return __xuiFileDialogFinishWithPath(pDialog,
				pEntry->bDir ? pEntry->sPath : pDialog->sCurrentDir);
		} else if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE ) {
			return __xuiFileDialogCommitSavePath(pDialog, pEntry->sPath);
		} else {
			return __xuiFileDialogFinishWithPath(pDialog, pEntry->sPath);
		}
	} else if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) {
		if ( pDialog->sCurrentDir == NULL || pDialog->sCurrentDir[0] == 0 ) {
			return __xuiFileDialogShowFolderError(pDialog);
		}
		return __xuiFileDialogFinishWithPath(pDialog,
			pDialog->sCurrentDir != NULL ? pDialog->sCurrentDir : "");
	} else {
		sName = xuiInputGetText(pDialog->pNameInput);
		if ( pDialog->iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE ) {
			sPath = NULL;
			iRet = __xuiFileDialogBuildSavePath(pDialog, sName, &sPath);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiFileDialogCommitSavePath(pDialog, sPath);
			xrtFree(sPath);
			return iRet;
		}
		sPath = NULL;
		iRet = __xuiFileDialogBuildCurrentPath(pDialog, sName, &sPath);
		if ( iRet != XUI_OK ) return iRet;
		if ( xrtDirExists((str)sPath) ) {
			iRet = xuiFileDialogSetDirectory(pDialog, sPath);
			xrtFree(sPath);
			return iRet;
		}
		if ( !xrtFileExists((str)sPath) ) {
			(void)xuiInputSetError(pDialog->pNameInput, 1);
			xrtFree(sPath);
			return XUI_ERROR_FILE_NOT_FOUND;
		}
		(void)xuiInputSetError(pDialog->pNameInput, 0);
		iRet = __xuiFileDialogFinishWithPath(pDialog, sPath);
		xrtFree(sPath);
		return iRet;
	}
}

XUI_API int xuiFileDialogCancel(xui_file_dialog pDialog)
{
	int iRet;
	if ( !__xuiFileDialogValid(pDialog) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiFileDialogSetOpen(pDialog, 0);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiFileDialogNotify(pDialog, XUI_FILE_DIALOG_RESULT_CANCEL);
}

XUI_API int xuiFileDialogGetResult(xui_file_dialog pDialog)
{
	return __xuiFileDialogValid(pDialog) ? pDialog->iResult : XUI_FILE_DIALOG_RESULT_NONE;
}

XUI_API const char* xuiFileDialogGetResultPath(xui_file_dialog pDialog)
{
	return __xuiFileDialogValid(pDialog) && pDialog->sResultPath != NULL ? pDialog->sResultPath : "";
}

XUI_API xui_widget xuiFileDialogGetWindowWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pWindow : NULL; }
XUI_API xui_widget xuiFileDialogGetRootListWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pRootList : NULL; }
XUI_API xui_widget xuiFileDialogGetFileListWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pFileList : NULL; }
XUI_API xui_widget xuiFileDialogGetPathBreadcrumbWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pPathBreadcrumb : NULL; }
XUI_API xui_widget xuiFileDialogGetPathInputWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pPathInput : NULL; }
XUI_API xui_widget xuiFileDialogGetNameInputWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pNameInput : NULL; }
XUI_API xui_widget xuiFileDialogGetFilterComboWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pFilterCombo : NULL; }
XUI_API xui_widget xuiFileDialogGetUpButtonWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pUpButton : NULL; }
XUI_API xui_widget xuiFileDialogGetRefreshButtonWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pRefreshButton : NULL; }
XUI_API xui_widget xuiFileDialogGetOkButtonWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pOkButton : NULL; }
XUI_API xui_widget xuiFileDialogGetCancelButtonWidget(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pCancelButton : NULL; }
XUI_API xui_msgbox xuiFileDialogGetOverwriteMsgBox(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->pOverwriteBox : NULL; }
XUI_API int xuiFileDialogGetFilterCount(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->iFilterCount : 0; }

XUI_API const char* xuiFileDialogGetFilterName(xui_file_dialog pDialog, int iIndex)
{
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iFilterCount ) return "";
	return pDialog->arrFilterNames[iIndex] != NULL ? pDialog->arrFilterNames[iIndex] : "";
}

XUI_API const char* xuiFileDialogGetFilterPattern(xui_file_dialog pDialog, int iIndex)
{
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iFilterCount ) return "";
	return pDialog->arrFilterPatterns[iIndex] != NULL ? pDialog->arrFilterPatterns[iIndex] : "";
}

XUI_API int xuiFileDialogGetEntryCount(xui_file_dialog pDialog) { return __xuiFileDialogValid(pDialog) ? pDialog->iEntryCount : 0; }

XUI_API const char* xuiFileDialogGetEntryName(xui_file_dialog pDialog, int iIndex)
{
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) return "";
	return pDialog->arrEntries[iIndex].sName;
}

XUI_API const char* xuiFileDialogGetEntryPath(xui_file_dialog pDialog, int iIndex)
{
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) return "";
	return pDialog->arrEntries[iIndex].sPath;
}

XUI_API int xuiFileDialogEntryIsDir(xui_file_dialog pDialog, int iIndex)
{
	if ( !__xuiFileDialogValid(pDialog) || iIndex < 0 || iIndex >= pDialog->iEntryCount ) return 0;
	return pDialog->arrEntries[iIndex].bDir;
}

#endif
