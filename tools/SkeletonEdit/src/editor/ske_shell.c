#include "ske_app.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#define SKE_PI 3.14159265358979323846f
#define SKE_RAD_TO_DEG 57.29577951308232f
#define SKE_DEG_TO_RAD 0.017453292519943295f

static void SkeletonEditRefreshProperties(ske_app_t* pApp);
static void SkeletonEditCanvasResetView(ske_app_t* pApp);
static int SkeletonEditNewDocument(ske_app_t* pApp);
static void SkeletonEditResetLayout(ske_app_t* pApp);
static void SkeletonEditTextureCacheClear(ske_app_t* pApp);

#ifdef _WIN32
static int SkeletonEditUtf8ToWide(const char* sText, wchar_t* sOut, int iOutCount)
{
	int iLen;

	if ( (sText == NULL) || (sOut == NULL) || (iOutCount <= 0) ) {
		return 0;
	}
	sOut[0] = 0;
	iLen = MultiByteToWideChar(CP_UTF8, 0, sText, -1, sOut, iOutCount);
	if ( iLen > 0 ) {
		return iLen;
	}
	return MultiByteToWideChar(CP_ACP, 0, sText, -1, sOut, iOutCount);
}

static int SkeletonEditWideToUtf8(const wchar_t* sText, char* sOut, int iOutCount)
{
	int iLen;

	if ( (sText == NULL) || (sOut == NULL) || (iOutCount <= 0) ) {
		return 0;
	}
	sOut[0] = 0;
	iLen = WideCharToMultiByte(CP_UTF8, 0, sText, -1, sOut, iOutCount, NULL, NULL);
	if ( iLen > 0 ) {
		return iLen;
	}
	return WideCharToMultiByte(CP_ACP, 0, sText, -1, sOut, iOutCount, NULL, NULL);
}

static int SkeletonEditGetEnvUtf8(const wchar_t* sName, char* sOut, size_t iOutSize)
{
	wchar_t sValueW[1024];
	DWORD iLen;

	if ( (sName == NULL) || (sOut == NULL) || (iOutSize == 0u) ) {
		return 0;
	}
	sOut[0] = 0;
	iLen = GetEnvironmentVariableW(sName, sValueW, (DWORD)(sizeof(sValueW) / sizeof(sValueW[0])));
	if ( iLen == 0 || iLen >= (sizeof(sValueW) / sizeof(sValueW[0])) ) {
		return 0;
	}
	return SkeletonEditWideToUtf8(sValueW, sOut, (int)iOutSize) > 0;
}
#endif

static xge_xui_menu_item_t g_arrFileMenu[] = {
	{ "New", "Ctrl+N", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_NEW, 0, NULL, NULL },
	{ "Open", "Ctrl+O", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_OPEN, 0, NULL, NULL },
	{ "Save", "Ctrl+S", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_SAVE, 0, NULL, NULL },
	{ "Save As", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_SAVE_AS, 0, NULL, NULL },
	{ "Import Image", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_IMPORT_IMAGE, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Exit", "Alt+F4", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_EXIT, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrEditMenu[] = {
	{ "Undo", "Ctrl+Z", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_UNDO, 0, NULL, NULL },
	{ "Redo", "Ctrl+Y", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_REDO, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Delete", "Del", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_DELETE, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrViewMenu[] = {
	{ "Reset Layout", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_RESET_LAYOUT, 0, NULL, NULL },
	{ "Frame All", "F", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_FRAME_ALL, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrSkeletonMenu[] = {
	{ "Add Bone", "B", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_ADD_BONE, 0, NULL, NULL },
	{ "Add Slot", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_ADD_SLOT, 0, NULL, NULL },
	{ "Add Attachment", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_ADD_ATTACHMENT, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrAnimationMenu[] = {
	{ "Add Animation", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_ADD_ANIMATION, 0, NULL, NULL },
	{ "Add Key", "K", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_ADD_KEY, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Play", "Space", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_PLAY, 0, NULL, NULL },
	{ "Stop", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_STOP, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrHelpMenu[] = {
	{ "About SkeletonEdit", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, SKE_CMD_ABOUT, 0, NULL, NULL }
};

static const char* g_arrToolbarText[] = {
	"New", "Open", "Save", "Import", "", "Select", "Bone", "Slot", "Attach", "Key", "", "Play"
};

static const int g_arrToolbarType[] = {
	XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON,
	XGE_XUI_TOOLBAR_ITEM_SEPARATOR,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_SEPARATOR,
	XGE_XUI_TOOLBAR_ITEM_BUTTON
};

static const char* SkeletonEditModeName(int iMode)
{
	switch ( iMode ) {
	case SKE_MODE_BONE:
		return "Bone";
	case SKE_MODE_SLOT:
		return "Slot";
	case SKE_MODE_ATTACHMENT:
		return "Attachment";
	case SKE_MODE_KEY:
		return "Key";
	case SKE_MODE_PLAY:
		return "Play";
	case SKE_MODE_SELECT:
	default:
		return "Select";
	}
}

static const char* SkeletonEditImageNameFromNode(int iNode)
{
	(void)iNode;
	return "";
}

static int SkeletonEditImageIndexFromNode(int iNode)
{
	iNode -= SKE_IMAGE_NODE_BASE;
	return (iNode >= 0 && iNode < SKE_IMAGE_CAPACITY) ? iNode : -1;
}

static const char* SkeletonEditImageNameFromAppNode(const ske_app_t* pApp, int iNode)
{
	int iIndex;

	if ( pApp == NULL ) {
		return "";
	}
	iIndex = SkeletonEditImageIndexFromNode(iNode);
	if ( iIndex < 0 || iIndex >= pApp->iImageCount ) {
		return "";
	}
	return pApp->arrImageNames[iIndex];
}

static int SkeletonEditIsImageFileName(const char* sName)
{
	const char* sExt;

	if ( (sName == NULL) || (sName[0] == 0) ) {
		return 0;
	}
	sExt = strrchr(sName, '.');
	if ( sExt == NULL ) {
		return 0;
	}
#ifdef _WIN32
	return (_stricmp(sExt, ".png") == 0 || _stricmp(sExt, ".jpg") == 0 || _stricmp(sExt, ".jpeg") == 0 || _stricmp(sExt, ".bmp") == 0);
#else
	return (strcmp(sExt, ".png") == 0 || strcmp(sExt, ".jpg") == 0 || strcmp(sExt, ".jpeg") == 0 || strcmp(sExt, ".bmp") == 0);
#endif
}

static const char* SkeletonEditPathBaseName(const char* sPath)
{
	const char* sSlash;
	const char* sBackslash;

	if ( sPath == NULL ) {
		return "";
	}
	sSlash = strrchr(sPath, '/');
	sBackslash = strrchr(sPath, '\\');
	if ( sBackslash > sSlash ) {
		sSlash = sBackslash;
	}
	return (sSlash != NULL) ? (sSlash + 1) : sPath;
}

static void SkeletonEditPathDirName(const char* sPath, char* sDir, size_t iDirSize)
{
	const char* sSlash;
	const char* sBackslash;
	size_t iLen;

	if ( (sDir == NULL) || (iDirSize == 0u) ) {
		return;
	}
	sDir[0] = 0;
	if ( (sPath == NULL) || (sPath[0] == 0) ) {
		return;
	}
	sSlash = strrchr(sPath, '/');
	sBackslash = strrchr(sPath, '\\');
	if ( sBackslash > sSlash ) {
		sSlash = sBackslash;
	}
	if ( sSlash == NULL ) {
		snprintf(sDir, iDirSize, ".");
		return;
	}
	iLen = (size_t)(sSlash - sPath);
	if ( iLen >= iDirSize ) {
		iLen = iDirSize - 1u;
	}
	memcpy(sDir, sPath, iLen);
	sDir[iLen] = 0;
}

static void SkeletonEditEnsureProjectContext(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->sDocumentPath[0] == 0 ) {
		snprintf(pApp->sDocumentPath, sizeof(pApp->sDocumentPath), "projects/hero.xskel");
	}
	if ( pApp->sProjectDir[0] == 0 ) {
		SkeletonEditPathDirName(pApp->sDocumentPath, pApp->sProjectDir, sizeof(pApp->sProjectDir));
	}
	if ( pApp->sProjectDir[0] == 0 ) {
		snprintf(pApp->sProjectDir, sizeof(pApp->sProjectDir), "projects");
	}
}

static void SkeletonEditSetDocumentPath(ske_app_t* pApp, const char* sPath)
{
	if ( (pApp == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return;
	}
	snprintf(pApp->sDocumentPath, sizeof(pApp->sDocumentPath), "%s", sPath);
	SkeletonEditPathDirName(pApp->sDocumentPath, pApp->sProjectDir, sizeof(pApp->sProjectDir));
	if ( pApp->sProjectDir[0] == 0 ) {
		snprintf(pApp->sProjectDir, sizeof(pApp->sProjectDir), ".");
	}
}

static void SkeletonEditProjectPath(ske_app_t* pApp, const char* sName, char* sOut, size_t iOutSize)
{
	if ( (sOut == NULL) || (iOutSize == 0u) ) {
		return;
	}
	SkeletonEditEnsureProjectContext(pApp);
	snprintf(sOut, iOutSize, "%s/%s", pApp->sProjectDir, (sName != NULL) ? sName : "");
}

static void SkeletonEditTextureCacheClear(ske_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < pApp->iImageTextureCount; i++ ) {
		if ( pApp->arrImageTextures[i].bLoaded ) {
			xgeTextureFree(&pApp->arrImageTextures[i].tTexture);
		}
	}
	memset(pApp->arrImageTextures, 0, sizeof(pApp->arrImageTextures));
	pApp->iImageTextureCount = 0;
}

static xge_texture SkeletonEditTextureForImage(ske_app_t* pApp, const char* sName)
{
	int i;
	ske_image_texture_t* pSlot;
	char sPath[1024];

	if ( (pApp == NULL) || (sName == NULL) || (sName[0] == 0) ) {
		return NULL;
	}
	for ( i = 0; i < pApp->iImageTextureCount; i++ ) {
#ifdef _WIN32
		if ( _stricmp(pApp->arrImageTextures[i].sName, sName) == 0 ) {
#else
		if ( strcmp(pApp->arrImageTextures[i].sName, sName) == 0 ) {
#endif
			return pApp->arrImageTextures[i].bLoaded ? &pApp->arrImageTextures[i].tTexture : NULL;
		}
	}
	if ( pApp->iImageTextureCount >= SKE_IMAGE_CAPACITY ) {
		return NULL;
	}
	pSlot = &pApp->arrImageTextures[pApp->iImageTextureCount++];
	memset(pSlot, 0, sizeof(*pSlot));
	snprintf(pSlot->sName, sizeof(pSlot->sName), "%s", sName);
	SkeletonEditProjectPath(pApp, sName, sPath, sizeof(sPath));
#ifdef _WIN32
	{
		wchar_t sPathW[1024];
		FILE* fp;
		long iSize;
		void* pData;
		int iRet;

		fp = NULL;
		if ( SkeletonEditUtf8ToWide(sPath, sPathW, (int)(sizeof(sPathW) / sizeof(sPathW[0]))) > 0 ) {
			fp = _wfopen(sPathW, L"rb");
		}
		if ( fp != NULL ) {
			if ( fseek(fp, 0, SEEK_END) == 0 ) {
				iSize = ftell(fp);
				if ( iSize > 0 && fseek(fp, 0, SEEK_SET) == 0 ) {
					pData = malloc((size_t)iSize);
					if ( pData != NULL ) {
						if ( fread(pData, 1, (size_t)iSize, fp) == (size_t)iSize ) {
							iRet = xgeTextureLoadMemoryEx(&pSlot->tTexture, pData, (int)iSize, XGE_IMAGE_PREMULTIPLIED);
							free(pData);
							fclose(fp);
							if ( iRet == XGE_OK ) {
								pSlot->bLoaded = 1;
								return &pSlot->tTexture;
							}
							pSlot->bFailed = 1;
							return NULL;
						}
						free(pData);
					}
				}
			}
			fclose(fp);
			pSlot->bFailed = 1;
			return NULL;
		}
	}
#endif
	if ( xgeTextureLoadEx(&pSlot->tTexture, sPath, XGE_IMAGE_PREMULTIPLIED) == XGE_OK ) {
		pSlot->bLoaded = 1;
		return &pSlot->tTexture;
	}
	pSlot->bFailed = 1;
	return NULL;
}

static int SkeletonEditCopyFileBytes(const char* sSrc, const char* sDst)
{
	FILE* fpSrc;
	FILE* fpDst;
	char arrBuffer[8192];
	size_t iRead;

	if ( (sSrc == NULL) || (sDst == NULL) || (sSrc[0] == 0) || (sDst[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
#ifdef _WIN32
	{
		wchar_t sSrcW[1024];
		wchar_t sDstW[1024];

		if ( SkeletonEditUtf8ToWide(sSrc, sSrcW, (int)(sizeof(sSrcW) / sizeof(sSrcW[0]))) <= 0 ||
		     SkeletonEditUtf8ToWide(sDst, sDstW, (int)(sizeof(sDstW) / sizeof(sDstW[0]))) <= 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		fpSrc = _wfopen(sSrcW, L"rb");
		if ( fpSrc == NULL ) {
			return XGE_ERROR_FILE_NOT_FOUND;
		}
		fpDst = _wfopen(sDstW, L"wb");
	}
#else
	fpSrc = fopen(sSrc, "rb");
	if ( fpSrc == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	fpDst = fopen(sDst, "wb");
#endif
	if ( fpDst == NULL ) {
		fclose(fpSrc);
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	while ( (iRead = fread(arrBuffer, 1, sizeof(arrBuffer), fpSrc)) > 0u ) {
		if ( fwrite(arrBuffer, 1, iRead, fpDst) != iRead ) {
			fclose(fpDst);
			fclose(fpSrc);
			return XGE_ERROR;
		}
	}
	fclose(fpDst);
	fclose(fpSrc);
	return XGE_OK;
}

static void SkeletonEditAddImageResource(ske_app_t* pApp, const char* sName)
{
	int i;

	if ( (pApp == NULL) || (sName == NULL) || (sName[0] == 0) || !SkeletonEditIsImageFileName(sName) ) {
		return;
	}
	for ( i = 0; i < pApp->iImageCount; i++ ) {
#ifdef _WIN32
		if ( _stricmp(pApp->arrImageNames[i], sName) == 0 ) {
#else
		if ( strcmp(pApp->arrImageNames[i], sName) == 0 ) {
#endif
			return;
		}
	}
	if ( pApp->iImageCount >= SKE_IMAGE_CAPACITY ) {
		return;
	}
	snprintf(pApp->arrImageNames[pApp->iImageCount], sizeof(pApp->arrImageNames[pApp->iImageCount]), "%s", sName);
	pApp->iImageCount++;
}

static int SkeletonEditScanProjectImages(ske_app_t* pApp)
{
#ifdef _WIN32
	WIN32_FIND_DATAW tFind;
	HANDLE hFind;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pApp->iImageCount = 0;
	{
		char sPattern[1024];
		wchar_t sPatternW[1024];
		SkeletonEditEnsureProjectContext(pApp);
		snprintf(sPattern, sizeof(sPattern), "%s\\*.*", pApp->sProjectDir);
		if ( SkeletonEditUtf8ToWide(sPattern, sPatternW, (int)(sizeof(sPatternW) / sizeof(sPatternW[0]))) <= 0 ) {
			hFind = INVALID_HANDLE_VALUE;
		} else {
			hFind = FindFirstFileW(sPatternW, &tFind);
		}
	}
	if ( hFind != INVALID_HANDLE_VALUE ) {
		do {
			if ( (tFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
				char sName[512];
				if ( SkeletonEditWideToUtf8(tFind.cFileName, sName, (int)sizeof(sName)) > 0 ) {
					SkeletonEditAddImageResource(pApp, sName);
				}
			}
		} while ( FindNextFileW(hFind, &tFind) );
		FindClose(hFind);
	}
	if ( pApp->iImageCount == 0 ) {
		SkeletonEditAddImageResource(pApp, "hero_body.png");
		SkeletonEditAddImageResource(pApp, "hero_arm.png");
	}
	return XGE_OK;
#else
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pApp->iImageCount = 0;
	SkeletonEditAddImageResource(pApp, "hero_body.png");
	SkeletonEditAddImageResource(pApp, "hero_arm.png");
	return XGE_OK;
#endif
}

static int SkeletonEditRefreshAssetTree(ske_app_t* pApp)
{
	int i;

	if ( (pApp == NULL) || !pApp->bAssetTreeReady ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	SkeletonEditScanProjectImages(pApp);
	xgeXuiTreeViewClear(&pApp->tAssetTree);
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 100, -1, "DefaultSkeleton");
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 101, 100, "Skeletons");
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 102, 101, SkeletonEditPathBaseName(pApp->sDocumentPath));
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 110, 100, "Images");
	for ( i = 0; i < pApp->iImageCount; i++ ) {
		xgeXuiTreeViewAddNode(&pApp->tAssetTree, SKE_IMAGE_NODE_BASE + i, 110, pApp->arrImageNames[i]);
	}
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 120, 100, "Animations");
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 121, 120, "idle");
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 122, 120, "walk");
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 130, 100, "Skins");
	xgeXuiTreeViewAddNode(&pApp->tAssetTree, 131, 130, "default");
	xgeXuiTreeViewSetNodeExpanded(&pApp->tAssetTree, 100, 1);
	xgeXuiTreeViewSetNodeExpanded(&pApp->tAssetTree, 101, 1);
	xgeXuiTreeViewSetNodeExpanded(&pApp->tAssetTree, 110, 1);
	xgeXuiTreeViewSetNodeExpanded(&pApp->tAssetTree, 120, 1);
	if ( pApp->iSelectedTreeNode < 0 ) {
		xgeXuiTreeViewSetSelected(&pApp->tAssetTree, 102);
		pApp->iSelectedTreeNode = 102;
	} else {
		xgeXuiTreeViewSetSelected(&pApp->tAssetTree, pApp->iSelectedTreeNode);
	}
	return XGE_OK;
}

static ske_bone_t* SkeletonEditSelectedBone(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->tDocument.iSelectedType != SKE_SELECTION_BONE) ||
	     (pApp->tDocument.iSelectedIndex < 0) || (pApp->tDocument.iSelectedIndex >= pApp->tDocument.iBoneCount) ) {
		return NULL;
	}
	return &pApp->tDocument.arrBones[pApp->tDocument.iSelectedIndex];
}

static ske_slot_t* SkeletonEditSelectedSlot(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->tDocument.iSelectedType != SKE_SELECTION_SLOT) ||
	     (pApp->tDocument.iSelectedIndex < 0) || (pApp->tDocument.iSelectedIndex >= pApp->tDocument.iSlotCount) ) {
		return NULL;
	}
	return &pApp->tDocument.arrSlots[pApp->tDocument.iSelectedIndex];
}

static ske_attachment_t* SkeletonEditSelectedAttachment(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->tDocument.iSelectedType != SKE_SELECTION_ATTACHMENT) ||
	     (pApp->tDocument.iSelectedIndex < 0) || (pApp->tDocument.iSelectedIndex >= pApp->tDocument.iAttachmentCount) ) {
		return NULL;
	}
	return &pApp->tDocument.arrAttachments[pApp->tDocument.iSelectedIndex];
}

static ske_animation_t* SkeletonEditSelectedAnimation(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->tDocument.iSelectedType != SKE_SELECTION_ANIMATION) ||
	     (pApp->tDocument.iSelectedIndex < 0) || (pApp->tDocument.iSelectedIndex >= pApp->tDocument.iAnimationCount) ) {
		return NULL;
	}
	return &pApp->tDocument.arrAnimations[pApp->tDocument.iSelectedIndex];
}

static ske_keyframe_t* SkeletonEditSelectedKeyframe(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->tDocument.iSelectedType != SKE_SELECTION_KEYFRAME) ||
	     (pApp->tDocument.iSelectedIndex < 0) || (pApp->tDocument.iSelectedIndex >= pApp->tDocument.iKeyframeCount) ) {
		return NULL;
	}
	return &pApp->tDocument.arrKeyframes[pApp->tDocument.iSelectedIndex];
}

static void SkeletonEditHistoryPushDocument(ske_document_t* pStack, int* pCount, const ske_document_t* pDoc)
{
	int i;

	if ( (pStack == NULL) || (pCount == NULL) || (pDoc == NULL) ) {
		return;
	}
	if ( *pCount >= SKE_HISTORY_CAPACITY ) {
		for ( i = 1; i < SKE_HISTORY_CAPACITY; i++ ) {
			pStack[i - 1] = pStack[i];
		}
		*pCount = SKE_HISTORY_CAPACITY - 1;
	}
	pStack[*pCount] = *pDoc;
	(*pCount)++;
}

static void SkeletonEditHistoryPushUndo(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	SkeletonEditHistoryPushDocument(pApp->arrUndo, &pApp->iUndoCount, &pApp->tDocument);
	pApp->iRedoCount = 0;
}

static void SkeletonEditPropertyPushUndo(ske_app_t* pApp, const char* sId)
{
	if ( (pApp == NULL) || (sId == NULL) ) {
		return;
	}
	if ( pApp->bPropertyUndoOpen != 0 && strcmp(pApp->sPropertyUndoId, sId) == 0 ) {
		return;
	}
	SkeletonEditHistoryPushUndo(pApp);
	pApp->bPropertyUndoOpen = 1;
	snprintf(pApp->sPropertyUndoId, sizeof(pApp->sPropertyUndoId), "%s", sId);
}

static int SkeletonEditHistoryUndo(ske_app_t* pApp)
{
	ske_document_t tCurrent;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->iUndoCount <= 0 ) {
		SkeletonEditStatusSet(pApp, "Nothing to undo");
		return XGE_OK;
	}
	tCurrent = pApp->tDocument;
	SkeletonEditHistoryPushDocument(pApp->arrRedo, &pApp->iRedoCount, &tCurrent);
	pApp->iUndoCount--;
	pApp->tDocument = pApp->arrUndo[pApp->iUndoCount];
	SkeletonEditStatusSet(pApp, "Undo");
	return XGE_OK;
}

static int SkeletonEditHistoryRedo(ske_app_t* pApp)
{
	ske_document_t tCurrent;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->iRedoCount <= 0 ) {
		SkeletonEditStatusSet(pApp, "Nothing to redo");
		return XGE_OK;
	}
	tCurrent = pApp->tDocument;
	SkeletonEditHistoryPushDocument(pApp->arrUndo, &pApp->iUndoCount, &tCurrent);
	pApp->iRedoCount--;
	pApp->tDocument = pApp->arrRedo[pApp->iRedoCount];
	SkeletonEditStatusSet(pApp, "Redo");
	return XGE_OK;
}

static void SkeletonEditBoneWorld(ske_app_t* pApp, int iBone, float* pX0, float* pY0, float* pX1, float* pY1, float* pRotation)
{
	ske_bone_t* pBone;
	float fPX0;
	float fPY0;
	float fPX1;
	float fPY1;
	float fParentRotation;
	float fRotation;
	float fX0;
	float fY0;

	if ( (pApp == NULL) || (iBone < 0) || (iBone >= pApp->tDocument.iBoneCount) ) {
		if ( pX0 != NULL ) *pX0 = 0.0f;
		if ( pY0 != NULL ) *pY0 = 0.0f;
		if ( pX1 != NULL ) *pX1 = 0.0f;
		if ( pY1 != NULL ) *pY1 = 0.0f;
		if ( pRotation != NULL ) *pRotation = 0.0f;
		return;
	}
	pBone = &pApp->tDocument.arrBones[iBone];
	fX0 = pBone->fLocalX;
	fY0 = pBone->fLocalY;
	fRotation = pBone->fRotation;
	if ( pBone->iParent >= 0 ) {
		SkeletonEditBoneWorld(pApp, pBone->iParent, &fPX0, &fPY0, &fPX1, &fPY1, &fParentRotation);
		fX0 = fPX1;
		fY0 = fPY1;
		fRotation += fParentRotation;
	}
	if ( pX0 != NULL ) *pX0 = fX0;
	if ( pY0 != NULL ) *pY0 = fY0;
	if ( pX1 != NULL ) *pX1 = fX0 + cosf(fRotation) * pBone->fLength;
	if ( pY1 != NULL ) *pY1 = fY0 + sinf(fRotation) * pBone->fLength;
	if ( pRotation != NULL ) *pRotation = fRotation;
}

static float SkeletonEditCanvasZoom(ske_app_t* pApp)
{
	if ( pApp == NULL || pApp->fCanvasZoom <= 0.0f ) {
		return 1.0f;
	}
	return pApp->fCanvasZoom;
}

static void SkeletonEditDocToScreen(ske_app_t* pApp, xge_rect_t r, float fX, float fY, float* pSX, float* pSY)
{
	float fZoom;

	fZoom = SkeletonEditCanvasZoom(pApp);
	if ( pSX != NULL ) {
		*pSX = r.fX + r.fW * 0.5f + ((pApp != NULL) ? pApp->fCanvasPanX : 0.0f) + fX * fZoom;
	}
	if ( pSY != NULL ) {
		*pSY = r.fY + r.fH * 0.58f + ((pApp != NULL) ? pApp->fCanvasPanY : 0.0f) + fY * fZoom;
	}
}

static void SkeletonEditScreenToDoc(ske_app_t* pApp, xge_rect_t r, float fSX, float fSY, float* pX, float* pY)
{
	float fZoom;

	fZoom = SkeletonEditCanvasZoom(pApp);
	if ( pX != NULL ) {
		*pX = (fSX - (r.fX + r.fW * 0.5f) - ((pApp != NULL) ? pApp->fCanvasPanX : 0.0f)) / fZoom;
	}
	if ( pY != NULL ) {
		*pY = (fSY - (r.fY + r.fH * 0.58f) - ((pApp != NULL) ? pApp->fCanvasPanY : 0.0f)) / fZoom;
	}
}

static float SkeletonEditDistancePointToSegment(float fPX, float fPY, float fX0, float fY0, float fX1, float fY1)
{
	float fDX;
	float fDY;
	float fLen2;
	float fT;
	float fX;
	float fY;
	float fOX;
	float fOY;

	fDX = fX1 - fX0;
	fDY = fY1 - fY0;
	fLen2 = fDX * fDX + fDY * fDY;
	if ( fLen2 <= 0.0001f ) {
		fOX = fPX - fX0;
		fOY = fPY - fY0;
		return sqrtf(fOX * fOX + fOY * fOY);
	}
	fT = ((fPX - fX0) * fDX + (fPY - fY0) * fDY) / fLen2;
	if ( fT < 0.0f ) fT = 0.0f;
	if ( fT > 1.0f ) fT = 1.0f;
	fX = fX0 + fDX * fT;
	fY = fY0 + fDY * fT;
	fOX = fPX - fX;
	fOY = fPY - fY;
	return sqrtf(fOX * fOX + fOY * fOY);
}

static int SkeletonEditAddBoneFromDrag(ske_app_t* pApp, int iParent, float fStartDocX, float fStartDocY, float fEndDocX, float fEndDocY)
{
	ske_bone_t* pBone;
	float fParentRot;
	float fDX;
	float fDY;
	float fLength;
	float fWorldRotation;
	char sName[SKE_NAME_CAPACITY];

	if ( (pApp == NULL) || (pApp->tDocument.iBoneCount >= SKE_BONE_CAPACITY) ) {
		return XGE_ERROR;
	}
	fDX = fEndDocX - fStartDocX;
	fDY = fEndDocY - fStartDocY;
	fLength = sqrtf(fDX * fDX + fDY * fDY);
	if ( fLength < 8.0f ) {
		fLength = 48.0f;
		fDX = 0.0f;
		fDY = -fLength;
	}
	fWorldRotation = atan2f(fDY, fDX);
	fParentRot = 0.0f;
	if ( iParent >= 0 ) {
		SkeletonEditBoneWorld(pApp, iParent, NULL, NULL, NULL, NULL, &fParentRot);
	}
	SkeletonEditHistoryPushUndo(pApp);
	pBone = &pApp->tDocument.arrBones[pApp->tDocument.iBoneCount];
	memset(pBone, 0, sizeof(*pBone));
	pBone->iId = pApp->tDocument.iNextBoneId++;
	snprintf(sName, sizeof(sName), "bone.%d", pBone->iId);
	snprintf(pBone->sName, sizeof(pBone->sName), "%s", sName);
	pBone->iParent = iParent;
	pBone->fLocalX = (iParent >= 0) ? 0.0f : fStartDocX;
	pBone->fLocalY = (iParent >= 0) ? 0.0f : fStartDocY;
	pBone->fRotation = fWorldRotation - fParentRot;
	pBone->fLength = fLength;
	pBone->bConnected = (iParent >= 0) ? 1 : 0;
	pBone->bVisible = 1;
	pApp->tDocument.iSelectedType = SKE_SELECTION_BONE;
	pApp->tDocument.iSelectedIndex = pApp->tDocument.iBoneCount;
	pApp->tDocument.iActiveBoneIndex = pApp->tDocument.iBoneCount;
	pApp->tDocument.iBoneCount++;
	pApp->tDocument.bDirty = 1;
	return XGE_OK;
}

static int SkeletonEditBindSelectedImage(ske_app_t* pApp)
{
	ske_bone_t* pBone;
	ske_attachment_t* pAttachment;
	const char* sImage;

	if ( (pApp == NULL) || (pApp->tDocument.iAttachmentCount >= SKE_ATTACHMENT_CAPACITY) ) {
		return XGE_ERROR;
	}
	pBone = SkeletonEditSelectedBone(pApp);
	if ( pBone == NULL ) {
		if ( pApp->tDocument.iActiveBoneIndex >= 0 && pApp->tDocument.iActiveBoneIndex < pApp->tDocument.iBoneCount ) {
			pBone = &pApp->tDocument.arrBones[pApp->tDocument.iActiveBoneIndex];
		} else if ( pApp->tDocument.iBoneCount <= 0 ) {
			return XGE_ERROR;
		} else {
			pBone = &pApp->tDocument.arrBones[0];
		}
	}
	sImage = SkeletonEditImageNameFromAppNode(pApp, pApp->tDocument.iSelectedImageNode);
	if ( sImage[0] == 0 ) {
		if ( pApp->iImageCount <= 0 ) {
			return XGE_ERROR;
		}
		sImage = pApp->arrImageNames[0];
	}
	SkeletonEditHistoryPushUndo(pApp);
	pAttachment = &pApp->tDocument.arrAttachments[pApp->tDocument.iAttachmentCount];
	memset(pAttachment, 0, sizeof(*pAttachment));
	pAttachment->iId = pApp->tDocument.iNextAttachmentId++;
	pAttachment->iBone = (int)(pBone - pApp->tDocument.arrBones);
	pApp->tDocument.iActiveBoneIndex = pAttachment->iBone;
	snprintf(pAttachment->sName, sizeof(pAttachment->sName), "%.56s.attach", pBone->sName);
	snprintf(pAttachment->sImage, sizeof(pAttachment->sImage), "%s", sImage);
	pAttachment->fScaleX = 1.0f;
	pAttachment->fScaleY = 1.0f;
	pAttachment->iDrawOrder = pApp->tDocument.iAttachmentCount;
	pApp->tDocument.iSelectedType = SKE_SELECTION_ATTACHMENT;
	pApp->tDocument.iSelectedIndex = pApp->tDocument.iAttachmentCount;
	pApp->tDocument.iAttachmentCount++;
	pApp->tDocument.bDirty = 1;
	return XGE_OK;
}

static int SkeletonEditAddSlot(ske_app_t* pApp)
{
	ske_slot_t* pSlot;
	int iBone;
	int iAttachment;

	if ( (pApp == NULL) || (pApp->tDocument.iSlotCount >= SKE_SLOT_CAPACITY) ) {
		return XGE_ERROR;
	}
	iBone = pApp->tDocument.iActiveBoneIndex;
	if ( iBone < 0 || iBone >= pApp->tDocument.iBoneCount ) {
		iBone = 0;
	}
	iAttachment = (pApp->tDocument.iSelectedType == SKE_SELECTION_ATTACHMENT) ? pApp->tDocument.iSelectedIndex : -1;
	if ( iAttachment < 0 || iAttachment >= pApp->tDocument.iAttachmentCount ) {
		iAttachment = (pApp->tDocument.iAttachmentCount > 0) ? pApp->tDocument.iAttachmentCount - 1 : -1;
	}
	SkeletonEditHistoryPushUndo(pApp);
	pSlot = &pApp->tDocument.arrSlots[pApp->tDocument.iSlotCount];
	memset(pSlot, 0, sizeof(*pSlot));
	pSlot->iId = pApp->tDocument.iNextSlotId++;
	pSlot->iBone = iBone;
	snprintf(pSlot->sName, sizeof(pSlot->sName), "slot.%d", pSlot->iId);
	pSlot->iAttachment = iAttachment;
	pSlot->iDrawOrder = pApp->tDocument.iSlotCount;
	pSlot->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pSlot->bVisible = 1;
	pApp->tDocument.iSelectedType = SKE_SELECTION_SLOT;
	pApp->tDocument.iSelectedIndex = pApp->tDocument.iSlotCount;
	pApp->tDocument.iActiveSlotIndex = pApp->tDocument.iSlotCount;
	pApp->tDocument.iActiveBoneIndex = iBone;
	pApp->tDocument.iSlotCount++;
	pApp->tDocument.bDirty = 1;
	return XGE_OK;
}

static int SkeletonEditSelectImageByName(ske_app_t* pApp, const char* sName)
{
	int i;

	if ( (pApp == NULL) || (sName == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pApp->iImageCount; i++ ) {
#ifdef _WIN32
		if ( _stricmp(pApp->arrImageNames[i], sName) == 0 ) {
#else
		if ( strcmp(pApp->arrImageNames[i], sName) == 0 ) {
#endif
			pApp->iSelectedTreeNode = SKE_IMAGE_NODE_BASE + i;
			pApp->tDocument.iSelectedType = SKE_SELECTION_IMAGE;
			pApp->tDocument.iSelectedImageNode = pApp->iSelectedTreeNode;
			pApp->tDocument.iSelectedIndex = -1;
			xgeXuiTreeViewSetSelected(&pApp->tAssetTree, pApp->iSelectedTreeNode);
			return XGE_OK;
		}
	}
	return XGE_ERROR_FILE_NOT_FOUND;
}

static int SkeletonEditPickImagePath(char* sPath, size_t iPathSize)
{
	const char* sEnv;

	if ( (sPath == NULL) || (iPathSize == 0u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sPath[0] = 0;
#ifdef _WIN32
	if ( SkeletonEditGetEnvUtf8(L"XGE_SKELETONEDIT_IMPORT_IMAGE", sPath, iPathSize) ) {
		return XGE_OK;
	}
#endif
	sEnv = getenv("XGE_SKELETONEDIT_IMPORT_IMAGE");
	if ( (sEnv != NULL) && (sEnv[0] != 0) ) {
		snprintf(sPath, iPathSize, "%s", sEnv);
		return XGE_OK;
	}
#ifdef _WIN32
	{
		OPENFILENAMEW tOfn;
		wchar_t sPathW[1024];
		memset(&tOfn, 0, sizeof(tOfn));
		sPathW[0] = 0;
		tOfn.lStructSize = sizeof(tOfn);
		tOfn.lpstrFile = sPathW;
		tOfn.nMaxFile = (DWORD)(sizeof(sPathW) / sizeof(sPathW[0]));
		tOfn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0PNG Files\0*.png\0All Files\0*.*\0";
		tOfn.lpstrTitle = L"Import Image";
		tOfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		if ( GetOpenFileNameW(&tOfn) ) {
			return (SkeletonEditWideToUtf8(sPathW, sPath, (int)iPathSize) > 0) ? XGE_OK : XGE_ERROR;
		}
	}
#endif
	return XGE_ERROR_FILE_NOT_FOUND;
}

static int SkeletonEditPickSkeletonOpenPath(char* sPath, size_t iPathSize)
{
	const char* sEnv;

	if ( (sPath == NULL) || (iPathSize == 0u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sPath[0] = 0;
#ifdef _WIN32
	if ( SkeletonEditGetEnvUtf8(L"XGE_SKELETONEDIT_OPEN_FILE", sPath, iPathSize) ) {
		return XGE_OK;
	}
#endif
	sEnv = getenv("XGE_SKELETONEDIT_OPEN_FILE");
	if ( (sEnv != NULL) && (sEnv[0] != 0) ) {
		snprintf(sPath, iPathSize, "%s", sEnv);
		return XGE_OK;
	}
#ifdef _WIN32
	{
		OPENFILENAMEW tOfn;
		wchar_t sPathW[1024];
		memset(&tOfn, 0, sizeof(tOfn));
		sPathW[0] = 0;
		tOfn.lStructSize = sizeof(tOfn);
		tOfn.lpstrFile = sPathW;
		tOfn.nMaxFile = (DWORD)(sizeof(sPathW) / sizeof(sPathW[0]));
		tOfn.lpstrFilter = L"XGE Skeleton Files\0*.xskel\0All Files\0*.*\0";
		tOfn.lpstrTitle = L"Open Skeleton";
		tOfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		if ( GetOpenFileNameW(&tOfn) ) {
			return (SkeletonEditWideToUtf8(sPathW, sPath, (int)iPathSize) > 0) ? XGE_OK : XGE_ERROR;
		}
	}
#endif
	return XGE_ERROR_FILE_NOT_FOUND;
}

static int SkeletonEditPickSkeletonSavePath(char* sPath, size_t iPathSize)
{
	if ( (sPath == NULL) || (iPathSize == 0u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
#ifdef _WIN32
	{
		OPENFILENAMEW tOfn;
		wchar_t sPathW[1024];
		memset(&tOfn, 0, sizeof(tOfn));
		if ( SkeletonEditUtf8ToWide(sPath, sPathW, (int)(sizeof(sPathW) / sizeof(sPathW[0]))) <= 0 ) {
			sPathW[0] = 0;
		}
		tOfn.lStructSize = sizeof(tOfn);
		tOfn.lpstrFile = sPathW;
		tOfn.nMaxFile = (DWORD)(sizeof(sPathW) / sizeof(sPathW[0]));
		tOfn.lpstrFilter = L"XGE Skeleton Files\0*.xskel\0All Files\0*.*\0";
		tOfn.lpstrDefExt = L"xskel";
		tOfn.lpstrTitle = L"Save Skeleton";
		tOfn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		if ( GetSaveFileNameW(&tOfn) ) {
			return (SkeletonEditWideToUtf8(sPathW, sPath, (int)iPathSize) > 0) ? XGE_OK : XGE_ERROR;
		}
	}
#endif
	return XGE_ERROR_FILE_NOT_FOUND;
}

static int SkeletonEditOpenDocument(ske_app_t* pApp)
{
	char sPath[1024];

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( SkeletonEditPickSkeletonOpenPath(sPath, sizeof(sPath)) != XGE_OK ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( SkeletonEditDocumentLoad(&pApp->tDocument, sPath) != XGE_OK ) {
		return XGE_ERROR;
	}
	SkeletonEditSetDocumentPath(pApp, sPath);
	SkeletonEditTextureCacheClear(pApp);
	pApp->iUndoCount = 0;
	pApp->iRedoCount = 0;
	SkeletonEditCanvasResetView(pApp);
	SkeletonEditRefreshAssetTree(pApp);
	SkeletonEditRefreshProperties(pApp);
	SkeletonEditStatusSet(pApp, "Document opened");
	SkeletonEditStatusRefresh(pApp);
	xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
	return XGE_OK;
}

static int SkeletonEditSaveDocument(ske_app_t* pApp, int bSaveAs)
{
	char sPath[1024];

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	SkeletonEditEnsureProjectContext(pApp);
	snprintf(sPath, sizeof(sPath), "%s", pApp->sDocumentPath);
	if ( bSaveAs || sPath[0] == 0 ) {
		if ( SkeletonEditPickSkeletonSavePath(sPath, sizeof(sPath)) != XGE_OK ) {
			return XGE_ERROR_FILE_NOT_FOUND;
		}
		SkeletonEditSetDocumentPath(pApp, sPath);
	}
	pApp->tDocument.bDirty = 0;
	if ( SkeletonEditDocumentSave(&pApp->tDocument, pApp->sDocumentPath) != XGE_OK ) {
		pApp->tDocument.bDirty = 1;
		return XGE_ERROR;
	}
	SkeletonEditRefreshAssetTree(pApp);
	SkeletonEditRefreshProperties(pApp);
	SkeletonEditStatusSet(pApp, "Document saved");
	return XGE_OK;
}

static int SkeletonEditNewDocument(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	SkeletonEditDocumentResetDefault(&pApp->tDocument);
	SkeletonEditSetDocumentPath(pApp, "projects/hero.xskel");
	SkeletonEditTextureCacheClear(pApp);
	pApp->iUndoCount = 0;
	pApp->iRedoCount = 0;
	pApp->iMode = SKE_MODE_SELECT;
	pApp->iCurrentFrame = 0;
	pApp->fPlaybackAccum = 0.0f;
	pApp->iSelectedTreeNode = 101;
	SkeletonEditCanvasResetView(pApp);
	SkeletonEditRefreshAssetTree(pApp);
	SkeletonEditRefreshProperties(pApp);
	if ( pApp->bTimelineReady ) {
		xgeXuiTimelineViewSetCurrentFrame(&pApp->tTimeline, 0);
	}
	SkeletonEditStatusSet(pApp, "New document created");
	SkeletonEditStatusRefresh(pApp);
	if ( pApp->pCanvasWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
	}
	return XGE_OK;
}

static void SkeletonEditResetLayout(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pCenterPane != NULL ) {
		xgeXuiDockPaneSetActiveIndex(pApp->pCenterPane, 0);
	}
	if ( pApp->pAssetPane != NULL ) {
		xgeXuiDockPaneSetActiveIndex(pApp->pAssetPane, 0);
	}
	if ( pApp->pPropertyPane != NULL ) {
		xgeXuiDockPaneSetActiveIndex(pApp->pPropertyPane, 0);
	}
	SkeletonEditCanvasResetView(pApp);
	if ( pApp->pDockHost != NULL ) {
		xgeXuiWidgetMarkLayout(pApp->pDockHost);
		xgeXuiWidgetMarkPaint(pApp->pDockHost);
	}
	if ( pApp->pCanvasWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
	}
	SkeletonEditStatusSet(pApp, "Layout reset");
}

static int SkeletonEditImportImage(ske_app_t* pApp)
{
	char sSrc[1024];
	char sDst[1024];
	const char* sName;
	int iRet;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
#ifdef _WIN32
	SkeletonEditEnsureProjectContext(pApp);
	{
		wchar_t sProjectDirW[1024];
		if ( SkeletonEditUtf8ToWide(pApp->sProjectDir, sProjectDirW, (int)(sizeof(sProjectDirW) / sizeof(sProjectDirW[0]))) > 0 ) {
			CreateDirectoryW(sProjectDirW, NULL);
		}
	}
#endif
	iRet = SkeletonEditPickImagePath(sSrc, sizeof(sSrc));
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	sName = SkeletonEditPathBaseName(sSrc);
	if ( !SkeletonEditIsImageFileName(sName) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	SkeletonEditProjectPath(pApp, sName, sDst, sizeof(sDst));
#ifdef _WIN32
	{
		wchar_t sSrcW[1024];
		wchar_t sDstW[1024];
		wchar_t sSrcFull[1024];
		wchar_t sDstFull[1024];
		DWORD iSrcLen;
		DWORD iDstLen;

		if ( SkeletonEditUtf8ToWide(sSrc, sSrcW, (int)(sizeof(sSrcW) / sizeof(sSrcW[0]))) <= 0 ||
		     SkeletonEditUtf8ToWide(sDst, sDstW, (int)(sizeof(sDstW) / sizeof(sDstW[0]))) <= 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iSrcLen = GetFullPathNameW(sSrcW, (DWORD)(sizeof(sSrcFull) / sizeof(sSrcFull[0])), sSrcFull, NULL);
		iDstLen = GetFullPathNameW(sDstW, (DWORD)(sizeof(sDstFull) / sizeof(sDstFull[0])), sDstFull, NULL);
		if ( iSrcLen == 0 || iSrcLen >= (sizeof(sSrcFull) / sizeof(sSrcFull[0])) || iDstLen == 0 || iDstLen >= (sizeof(sDstFull) / sizeof(sDstFull[0])) || _wcsicmp(sSrcFull, sDstFull) != 0 ) {
			iRet = SkeletonEditCopyFileBytes(sSrc, sDst);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
#else
	if ( strcmp(sSrc, sDst) != 0 ) {
		iRet = SkeletonEditCopyFileBytes(sSrc, sDst);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
#endif
	SkeletonEditScanProjectImages(pApp);
	SkeletonEditTextureCacheClear(pApp);
	SkeletonEditRefreshAssetTree(pApp);
	SkeletonEditSelectImageByName(pApp, sName);
	SkeletonEditRefreshProperties(pApp);
	SkeletonEditStatusSet(pApp, "Image imported");
	return XGE_OK;
}

static int SkeletonEditAddAnimation(ske_app_t* pApp)
{
	ske_animation_t* pAnim;

	if ( (pApp == NULL) || (pApp->tDocument.iAnimationCount >= SKE_ANIMATION_CAPACITY) ) {
		return XGE_ERROR;
	}
	SkeletonEditHistoryPushUndo(pApp);
	pAnim = &pApp->tDocument.arrAnimations[pApp->tDocument.iAnimationCount];
	memset(pAnim, 0, sizeof(*pAnim));
	pAnim->iId = pApp->tDocument.iNextAnimationId++;
	snprintf(pAnim->sName, sizeof(pAnim->sName), "anim.%d", pAnim->iId);
	pAnim->iFrameStart = 0;
	pAnim->iFrameEnd = 48;
	pAnim->bLoop = 1;
	pApp->tDocument.iSelectedType = SKE_SELECTION_ANIMATION;
	pApp->tDocument.iSelectedIndex = pApp->tDocument.iAnimationCount;
	pApp->tDocument.iActiveAnimationIndex = pApp->tDocument.iAnimationCount;
	pApp->tDocument.iAnimationCount++;
	pApp->tDocument.bDirty = 1;
	return XGE_OK;
}

static int SkeletonEditAddKeyframe(ske_app_t* pApp)
{
	ske_keyframe_t* pKey;
	ske_bone_t* pBone;
	int iBone;
	int iAnimation;

	if ( (pApp == NULL) || (pApp->tDocument.iKeyframeCount >= SKE_KEYFRAME_CAPACITY) ) {
		return XGE_ERROR;
	}
	iBone = pApp->tDocument.iActiveBoneIndex;
	if ( iBone < 0 || iBone >= pApp->tDocument.iBoneCount ) {
		iBone = 0;
	}
	iAnimation = pApp->tDocument.iActiveAnimationIndex;
	if ( iAnimation < 0 || iAnimation >= pApp->tDocument.iAnimationCount ) {
		if ( pApp->tDocument.iAnimationCount <= 0 ) {
			if ( SkeletonEditAddAnimation(pApp) != XGE_OK ) {
				return XGE_ERROR;
			}
		}
		iAnimation = pApp->tDocument.iActiveAnimationIndex;
	}
	pBone = &pApp->tDocument.arrBones[iBone];
	SkeletonEditHistoryPushUndo(pApp);
	pKey = &pApp->tDocument.arrKeyframes[pApp->tDocument.iKeyframeCount];
	memset(pKey, 0, sizeof(*pKey));
	pKey->iId = pApp->tDocument.iNextKeyframeId++;
	pKey->iAnimation = iAnimation;
	pKey->iBone = iBone;
	pKey->iFrame = pApp->iCurrentFrame;
	pKey->fLocalX = pBone->fLocalX;
	pKey->fLocalY = pBone->fLocalY;
	pKey->fRotation = pBone->fRotation;
	pKey->fLength = pBone->fLength;
	pApp->tDocument.iSelectedType = SKE_SELECTION_KEYFRAME;
	pApp->tDocument.iSelectedIndex = pApp->tDocument.iKeyframeCount;
	pApp->tDocument.iKeyframeCount++;
	pApp->tDocument.bDirty = 1;
	return XGE_OK;
}

static int SkeletonEditHitBone(ske_app_t* pApp, xge_rect_t r, float fSX, float fSY)
{
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	float fSX0;
	float fSY0;
	float fSX1;
	float fSY1;
	float fDist;
	float fBest;
	int iBest;
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	fBest = 14.0f;
	iBest = -1;
	for ( i = 0; i < pApp->tDocument.iBoneCount; i++ ) {
		SkeletonEditBoneWorld(pApp, i, &fX0, &fY0, &fX1, &fY1, NULL);
		SkeletonEditDocToScreen(pApp, r, fX0, fY0, &fSX0, &fSY0);
		SkeletonEditDocToScreen(pApp, r, fX1, fY1, &fSX1, &fSY1);
		fDist = SkeletonEditDistancePointToSegment(fSX, fSY, fSX0, fSY0, fSX1, fSY1);
		if ( fDist < fBest ) {
			fBest = fDist;
			iBest = i;
		}
	}
	return iBest;
}

static int SkeletonEditHitBoneEnd(ske_app_t* pApp, xge_rect_t r, float fSX, float fSY)
{
	float fX1;
	float fY1;
	float fSX1;
	float fSY1;
	float fDX;
	float fDY;
	float fDist;
	float fBest;
	int iBest;
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	fBest = 12.0f;
	iBest = -1;
	for ( i = 0; i < pApp->tDocument.iBoneCount; i++ ) {
		SkeletonEditBoneWorld(pApp, i, NULL, NULL, &fX1, &fY1, NULL);
		SkeletonEditDocToScreen(pApp, r, fX1, fY1, &fSX1, &fSY1);
		fDX = fSX - fSX1;
		fDY = fSY - fSY1;
		fDist = sqrtf(fDX * fDX + fDY * fDY);
		if ( fDist < fBest ) {
			fBest = fDist;
			iBest = i;
		}
	}
	return iBest;
}

static int SkeletonEditHitAttachment(ske_app_t* pApp, xge_rect_t r, float fSX, float fSY)
{
	ske_attachment_t* pAttachment;
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	float fCX;
	float fCY;
	float fSCX;
	float fSCY;
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	for ( i = pApp->tDocument.iAttachmentCount - 1; i >= 0; i-- ) {
		pAttachment = &pApp->tDocument.arrAttachments[i];
		if ( pAttachment->iBone < 0 || pAttachment->iBone >= pApp->tDocument.iBoneCount ) {
			continue;
		}
		SkeletonEditBoneWorld(pApp, pAttachment->iBone, &fX0, &fY0, &fX1, &fY1, NULL);
		fCX = (fX0 + fX1) * 0.5f + pAttachment->fOffsetX;
		fCY = (fY0 + fY1) * 0.5f + pAttachment->fOffsetY;
		SkeletonEditDocToScreen(pApp, r, fCX, fCY, &fSCX, &fSCY);
		if ( fSX >= fSCX - 24.0f && fSX <= fSCX + 24.0f && fSY >= fSCY - 18.0f && fSY <= fSCY + 18.0f ) {
			return i;
		}
	}
	return -1;
}

static void SkeletonEditSetBoneEndFromDoc(ske_app_t* pApp, int iBone, float fEndDocX, float fEndDocY)
{
	ske_bone_t* pBone;
	float fX0;
	float fY0;
	float fDX;
	float fDY;
	float fWorldRotation;
	float fParentRotation;
	float fLength;

	if ( (pApp == NULL) || (iBone < 0) || (iBone >= pApp->tDocument.iBoneCount) ) {
		return;
	}
	pBone = &pApp->tDocument.arrBones[iBone];
	SkeletonEditBoneWorld(pApp, iBone, &fX0, &fY0, NULL, NULL, NULL);
	fDX = fEndDocX - fX0;
	fDY = fEndDocY - fY0;
	fLength = sqrtf(fDX * fDX + fDY * fDY);
	if ( fLength < 4.0f ) {
		fLength = 4.0f;
	}
	fWorldRotation = atan2f(fDY, fDX);
	fParentRotation = 0.0f;
	if ( pBone->iParent >= 0 ) {
		SkeletonEditBoneWorld(pApp, pBone->iParent, NULL, NULL, NULL, NULL, &fParentRotation);
	}
	pBone->fLength = fLength;
	pBone->fRotation = fWorldRotation - fParentRotation;
	pApp->tDocument.bDirty = 1;
}

static int SkeletonEditHitRootStart(ske_app_t* pApp, xge_rect_t r, float fSX, float fSY)
{
	float fX0;
	float fY0;
	float fSX0;
	float fSY0;
	float fDX;
	float fDY;
	float fDist;
	float fBest;
	int iBest;
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	fBest = 12.0f;
	iBest = -1;
	for ( i = 0; i < pApp->tDocument.iBoneCount; i++ ) {
		if ( pApp->tDocument.arrBones[i].iParent >= 0 ) {
			continue;
		}
		SkeletonEditBoneWorld(pApp, i, &fX0, &fY0, NULL, NULL, NULL);
		SkeletonEditDocToScreen(pApp, r, fX0, fY0, &fSX0, &fSY0);
		fDX = fSX - fSX0;
		fDY = fSY - fSY0;
		fDist = sqrtf(fDX * fDX + fDY * fDY);
		if ( fDist < fBest ) {
			fBest = fDist;
			iBest = i;
		}
	}
	return iBest;
}

static int SkeletonEditBoneHasDependents(ske_app_t* pApp, int iBone)
{
	int i;

	if ( (pApp == NULL) || (iBone < 0) || (iBone >= pApp->tDocument.iBoneCount) ) {
		return 1;
	}
	for ( i = 0; i < pApp->tDocument.iBoneCount; i++ ) {
		if ( pApp->tDocument.arrBones[i].iParent == iBone ) {
			return 1;
		}
	}
	for ( i = 0; i < pApp->tDocument.iAttachmentCount; i++ ) {
		if ( pApp->tDocument.arrAttachments[i].iBone == iBone ) {
			return 1;
		}
	}
	return 0;
}

static int SkeletonEditDeleteSelected(ske_app_t* pApp)
{
	int i;
	int iIndex;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->tDocument.iSelectedType == SKE_SELECTION_ATTACHMENT ) {
		iIndex = pApp->tDocument.iSelectedIndex;
		if ( iIndex < 0 || iIndex >= pApp->tDocument.iAttachmentCount ) {
			return XGE_ERROR;
		}
		SkeletonEditHistoryPushUndo(pApp);
		for ( i = iIndex; i < pApp->tDocument.iAttachmentCount - 1; i++ ) {
			pApp->tDocument.arrAttachments[i] = pApp->tDocument.arrAttachments[i + 1];
		}
		pApp->tDocument.iAttachmentCount--;
		pApp->tDocument.iSelectedType = SKE_SELECTION_BONE;
		pApp->tDocument.iSelectedIndex = pApp->tDocument.iActiveBoneIndex;
		if ( pApp->tDocument.iSelectedIndex < 0 || pApp->tDocument.iSelectedIndex >= pApp->tDocument.iBoneCount ) {
			pApp->tDocument.iSelectedType = SKE_SELECTION_NONE;
			pApp->tDocument.iSelectedIndex = -1;
		}
		pApp->tDocument.bDirty = 1;
		SkeletonEditStatusSet(pApp, "Attachment deleted");
		return XGE_OK;
	}
	if ( pApp->tDocument.iSelectedType == SKE_SELECTION_SLOT ) {
		iIndex = pApp->tDocument.iSelectedIndex;
		if ( iIndex < 0 || iIndex >= pApp->tDocument.iSlotCount ) {
			return XGE_ERROR;
		}
		SkeletonEditHistoryPushUndo(pApp);
		for ( i = iIndex; i < pApp->tDocument.iSlotCount - 1; i++ ) {
			pApp->tDocument.arrSlots[i] = pApp->tDocument.arrSlots[i + 1];
		}
		pApp->tDocument.iSlotCount--;
		pApp->tDocument.iActiveSlotIndex = (pApp->tDocument.iSlotCount > 0) ? 0 : -1;
		pApp->tDocument.iSelectedType = SKE_SELECTION_NONE;
		pApp->tDocument.iSelectedIndex = -1;
		pApp->tDocument.bDirty = 1;
		SkeletonEditStatusSet(pApp, "Slot deleted");
		return XGE_OK;
	}
	if ( pApp->tDocument.iSelectedType == SKE_SELECTION_KEYFRAME ) {
		iIndex = pApp->tDocument.iSelectedIndex;
		if ( iIndex < 0 || iIndex >= pApp->tDocument.iKeyframeCount ) {
			return XGE_ERROR;
		}
		SkeletonEditHistoryPushUndo(pApp);
		for ( i = iIndex; i < pApp->tDocument.iKeyframeCount - 1; i++ ) {
			pApp->tDocument.arrKeyframes[i] = pApp->tDocument.arrKeyframes[i + 1];
		}
		pApp->tDocument.iKeyframeCount--;
		pApp->tDocument.iSelectedType = SKE_SELECTION_NONE;
		pApp->tDocument.iSelectedIndex = -1;
		pApp->tDocument.bDirty = 1;
		SkeletonEditStatusSet(pApp, "Keyframe deleted");
		return XGE_OK;
	}
	if ( pApp->tDocument.iSelectedType == SKE_SELECTION_ANIMATION ) {
		iIndex = pApp->tDocument.iSelectedIndex;
		if ( iIndex < 0 || iIndex >= pApp->tDocument.iAnimationCount ) {
			return XGE_ERROR;
		}
		for ( i = 0; i < pApp->tDocument.iKeyframeCount; i++ ) {
			if ( pApp->tDocument.arrKeyframes[i].iAnimation == iIndex ) {
				SkeletonEditStatusSet(pApp, "Delete blocked: animation has keys");
				return XGE_OK;
			}
		}
		SkeletonEditHistoryPushUndo(pApp);
		for ( i = iIndex; i < pApp->tDocument.iAnimationCount - 1; i++ ) {
			pApp->tDocument.arrAnimations[i] = pApp->tDocument.arrAnimations[i + 1];
		}
		pApp->tDocument.iAnimationCount--;
		for ( i = 0; i < pApp->tDocument.iKeyframeCount; i++ ) {
			if ( pApp->tDocument.arrKeyframes[i].iAnimation > iIndex ) {
				pApp->tDocument.arrKeyframes[i].iAnimation--;
			}
		}
		pApp->tDocument.iActiveAnimationIndex = (pApp->tDocument.iAnimationCount > 0) ? 0 : -1;
		pApp->tDocument.iSelectedType = SKE_SELECTION_NONE;
		pApp->tDocument.iSelectedIndex = -1;
		pApp->tDocument.bDirty = 1;
		SkeletonEditStatusSet(pApp, "Animation deleted");
		return XGE_OK;
	}
	if ( pApp->tDocument.iSelectedType == SKE_SELECTION_BONE ) {
		iIndex = pApp->tDocument.iSelectedIndex;
		if ( iIndex < 0 || iIndex >= pApp->tDocument.iBoneCount ) {
			return XGE_ERROR;
		}
		if ( SkeletonEditBoneHasDependents(pApp, iIndex) ) {
			SkeletonEditStatusSet(pApp, "Delete blocked: bone has children or attachments");
			return XGE_OK;
		}
		SkeletonEditHistoryPushUndo(pApp);
		for ( i = iIndex; i < pApp->tDocument.iBoneCount - 1; i++ ) {
			pApp->tDocument.arrBones[i] = pApp->tDocument.arrBones[i + 1];
		}
		pApp->tDocument.iBoneCount--;
		for ( i = 0; i < pApp->tDocument.iBoneCount; i++ ) {
			if ( pApp->tDocument.arrBones[i].iParent > iIndex ) {
				pApp->tDocument.arrBones[i].iParent--;
			}
		}
		pApp->tDocument.iSelectedType = SKE_SELECTION_BONE;
		pApp->tDocument.iSelectedIndex = (iIndex > 0) ? iIndex - 1 : 0;
		if ( pApp->tDocument.iBoneCount <= 0 ) {
			pApp->tDocument.iSelectedType = SKE_SELECTION_NONE;
			pApp->tDocument.iSelectedIndex = -1;
			pApp->tDocument.iActiveBoneIndex = -1;
		} else {
			pApp->tDocument.iActiveBoneIndex = pApp->tDocument.iSelectedIndex;
		}
		pApp->tDocument.bDirty = 1;
		SkeletonEditStatusSet(pApp, "Bone deleted");
		return XGE_OK;
	}
	SkeletonEditStatusSet(pApp, "Nothing to delete");
	return XGE_OK;
}

static int SkeletonEditCanvasContains(xge_rect_t r, float fX, float fY)
{
	return (fX >= r.fX) && (fY >= r.fY) && (fX <= r.fX + r.fW) && (fY <= r.fY + r.fH);
}

static void SkeletonEditCanvasResetView(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->fCanvasPanX = 0.0f;
	pApp->fCanvasPanY = 0.0f;
	pApp->fCanvasZoom = 1.0f;
}

static void SkeletonEditCanvasZoomAt(ske_app_t* pApp, xge_rect_t r, float fSX, float fSY, float fWheelY)
{
	float fDocX;
	float fDocY;
	float fOldZoom;
	float fNewZoom;

	if ( pApp == NULL || fWheelY == 0.0f ) {
		return;
	}
	fOldZoom = SkeletonEditCanvasZoom(pApp);
	SkeletonEditScreenToDoc(pApp, r, fSX, fSY, &fDocX, &fDocY);
	fNewZoom = fOldZoom * ((fWheelY > 0.0f) ? 1.12f : 0.89285713f);
	if ( fNewZoom < 0.25f ) fNewZoom = 0.25f;
	if ( fNewZoom > 4.0f ) fNewZoom = 4.0f;
	pApp->fCanvasZoom = fNewZoom;
	pApp->fCanvasPanX = fSX - (r.fX + r.fW * 0.5f) - fDocX * fNewZoom;
	pApp->fCanvasPanY = fSY - (r.fY + r.fH * 0.58f) - fDocY * fNewZoom;
}

int SkeletonEditCanvasHandleEvent(ske_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t r;
	ske_attachment_t* pAttachment;
	ske_bone_t* pBone;
	float fDocX;
	float fDocY;
	float fStartX;
	float fStartY;
	float fEndX;
	float fEndY;
	float fParentEndX;
	float fParentEndY;
	int iHit;
	int iAttachmentHit;
	int iEndHit;
	int iRootHit;

	if ( (pApp == NULL) || (pEvent == NULL) || (pApp->pCanvasWidget == NULL) ) {
		return XGE_OK;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_DOWN) &&
	     (pEvent->iType != XGE_EVENT_MOUSE_UP) &&
	     (pEvent->iType != XGE_EVENT_MOUSE_MOVE) &&
	     (pEvent->iType != XGE_EVENT_MOUSE_WHEEL) ) {
		return XGE_OK;
	}
	r = pApp->pCanvasWidget->tRect;
	if ( pEvent->iType == XGE_EVENT_MOUSE_WHEEL && SkeletonEditCanvasContains(r, pEvent->fX, pEvent->fY) ) {
		SkeletonEditCanvasZoomAt(pApp, r, pEvent->fX, pEvent->fY, pEvent->fDY);
		SkeletonEditStatusSet(pApp, "Canvas zoomed");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasPan && pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		pApp->fCanvasPanX = pApp->fPanStartX + (pEvent->fX - pApp->fPanStartMouseX);
		pApp->fCanvasPanY = pApp->fPanStartY + (pEvent->fY - pApp->fPanStartMouseY);
		SkeletonEditStatusSet(pApp, "Canvas panned");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasPan && pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_MIDDLE ) {
		pApp->bCanvasPan = 0;
		SkeletonEditStatusSet(pApp, "Canvas view placed");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasDragAttachment && pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		if ( pApp->iDragAttachmentIndex >= 0 && pApp->iDragAttachmentIndex < pApp->tDocument.iAttachmentCount ) {
			pAttachment = &pApp->tDocument.arrAttachments[pApp->iDragAttachmentIndex];
			SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &fDocX, &fDocY);
			pAttachment->fOffsetX = pApp->fDragStartOffsetX + (fDocX - pApp->fDragStartDocX);
			pAttachment->fOffsetY = pApp->fDragStartOffsetY + (fDocY - pApp->fDragStartDocY);
			pApp->tDocument.bDirty = 1;
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Attachment moved");
			SkeletonEditStatusRefresh(pApp);
			xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
			xgeXuiRefreshRequest(&pApp->tXui);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasDragAttachment && pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT ) {
		pApp->bCanvasDragAttachment = 0;
		pApp->iDragAttachmentIndex = -1;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Attachment placed");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasDragRoot && pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		if ( pApp->iDragBoneIndex >= 0 && pApp->iDragBoneIndex < pApp->tDocument.iBoneCount ) {
			pBone = &pApp->tDocument.arrBones[pApp->iDragBoneIndex];
			SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &fDocX, &fDocY);
			pBone->fLocalX = pApp->fDragStartRootX + (fDocX - pApp->fDragStartDocX);
			pBone->fLocalY = pApp->fDragStartRootY + (fDocY - pApp->fDragStartDocY);
			pApp->tDocument.bDirty = 1;
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Root moved");
			SkeletonEditStatusRefresh(pApp);
			xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
			xgeXuiRefreshRequest(&pApp->tXui);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasDragRoot && pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT ) {
		pApp->bCanvasDragRoot = 0;
		pApp->iDragBoneIndex = -1;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Root placed");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasDragBoneEnd && pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &fDocX, &fDocY);
		SkeletonEditSetBoneEndFromDoc(pApp, pApp->iDragBoneIndex, fDocX, fDocY);
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Bone length adjusted");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasDragBoneEnd && pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT ) {
		SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &fDocX, &fDocY);
		SkeletonEditSetBoneEndFromDoc(pApp, pApp->iDragBoneIndex, fDocX, fDocY);
		pApp->bCanvasDragBoneEnd = 0;
		pApp->iDragBoneIndex = -1;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Bone adjusted");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasCreateBone && pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		pApp->fCreateEndX = pEvent->fX;
		pApp->fCreateEndY = pEvent->fY;
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->bCanvasCreateBone && pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT ) {
		SkeletonEditScreenToDoc(pApp, r, pApp->fCreateStartX, pApp->fCreateStartY, &fStartX, &fStartY);
		SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &fEndX, &fEndY);
		SkeletonEditAddBoneFromDrag(pApp, pApp->iCreateParent, fStartX, fStartY, fEndX, fEndY);
		pApp->bCanvasCreateBone = 0;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Bone created");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType != XGE_EVENT_MOUSE_DOWN ) {
		return XGE_OK;
	}
	if ( !SkeletonEditCanvasContains(r, pEvent->fX, pEvent->fY) ) {
		return XGE_OK;
	}
	if ( pEvent->iParam1 == XGE_MOUSE_MIDDLE ) {
		pApp->bCanvasPan = 1;
		pApp->fPanStartMouseX = pEvent->fX;
		pApp->fPanStartMouseY = pEvent->fY;
		pApp->fPanStartX = pApp->fCanvasPanX;
		pApp->fPanStartY = pApp->fCanvasPanY;
		SkeletonEditStatusSet(pApp, "Panning canvas");
		SkeletonEditStatusRefresh(pApp);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iParam1 != XGE_MOUSE_LEFT ) {
		return XGE_OK;
	}
	if ( pApp->iMode == SKE_MODE_BONE ) {
		pApp->iCreateParent = (pApp->tDocument.iSelectedType == SKE_SELECTION_BONE) ? pApp->tDocument.iSelectedIndex : -1;
		if ( pApp->iCreateParent >= 0 ) {
			SkeletonEditBoneWorld(pApp, pApp->iCreateParent, NULL, NULL, &fParentEndX, &fParentEndY, NULL);
			SkeletonEditDocToScreen(pApp, r, fParentEndX, fParentEndY, &pApp->fCreateStartX, &pApp->fCreateStartY);
		} else {
			pApp->fCreateStartX = pEvent->fX;
			pApp->fCreateStartY = pEvent->fY;
		}
		pApp->fCreateEndX = pEvent->fX;
		pApp->fCreateEndY = pEvent->fY;
		pApp->bCanvasCreateBone = 1;
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iRootHit = SkeletonEditHitRootStart(pApp, r, pEvent->fX, pEvent->fY);
	if ( iRootHit >= 0 ) {
		pBone = &pApp->tDocument.arrBones[iRootHit];
		pApp->tDocument.iSelectedType = SKE_SELECTION_BONE;
		pApp->tDocument.iSelectedIndex = iRootHit;
		pApp->tDocument.iActiveBoneIndex = iRootHit;
		SkeletonEditHistoryPushUndo(pApp);
		pApp->bCanvasDragRoot = 1;
		pApp->iDragBoneIndex = iRootHit;
		SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &pApp->fDragStartDocX, &pApp->fDragStartDocY);
		pApp->fDragStartRootX = pBone->fLocalX;
		pApp->fDragStartRootY = pBone->fLocalY;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Dragging root");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iEndHit = SkeletonEditHitBoneEnd(pApp, r, pEvent->fX, pEvent->fY);
	if ( iEndHit >= 0 ) {
		pApp->tDocument.iSelectedType = SKE_SELECTION_BONE;
		pApp->tDocument.iSelectedIndex = iEndHit;
		pApp->tDocument.iActiveBoneIndex = iEndHit;
		SkeletonEditHistoryPushUndo(pApp);
		pApp->bCanvasDragBoneEnd = 1;
		pApp->iDragBoneIndex = iEndHit;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Dragging bone end");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iAttachmentHit = SkeletonEditHitAttachment(pApp, r, pEvent->fX, pEvent->fY);
	if ( iAttachmentHit >= 0 ) {
		pAttachment = &pApp->tDocument.arrAttachments[iAttachmentHit];
		pApp->tDocument.iSelectedType = SKE_SELECTION_ATTACHMENT;
		pApp->tDocument.iSelectedIndex = iAttachmentHit;
		if ( pAttachment->iBone >= 0 ) {
			pApp->tDocument.iActiveBoneIndex = pAttachment->iBone;
		}
		SkeletonEditHistoryPushUndo(pApp);
		pApp->bCanvasDragAttachment = 1;
		pApp->iDragAttachmentIndex = iAttachmentHit;
		SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &pApp->fDragStartDocX, &pApp->fDragStartDocY);
		pApp->fDragStartOffsetX = pAttachment->fOffsetX;
		pApp->fDragStartOffsetY = pAttachment->fOffsetY;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Dragging attachment");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	SkeletonEditScreenToDoc(pApp, r, pEvent->fX, pEvent->fY, &fDocX, &fDocY);
	(void)fDocX;
	(void)fDocY;
	iHit = SkeletonEditHitBone(pApp, r, pEvent->fX, pEvent->fY);
	if ( iHit >= 0 ) {
		pApp->tDocument.iSelectedType = SKE_SELECTION_BONE;
		pApp->tDocument.iSelectedIndex = iHit;
		pApp->tDocument.iActiveBoneIndex = iHit;
		SkeletonEditRefreshProperties(pApp);
		SkeletonEditStatusSet(pApp, "Bone selected");
		SkeletonEditStatusRefresh(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_OK;
}

static xge_xui_widget SkeletonEditCreateWidget(int iLayout, float fW, float fH, uint32_t iBackground, uint32_t iBorder)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pWidget, iLayout);
	xgeXuiWidgetSetSize(pWidget, (fW < 0.0f) ? xgeXuiSizeGrow(1.0f) : xgeXuiSizePx(fW), (fH < 0.0f) ? xgeXuiSizeGrow(1.0f) : xgeXuiSizePx(fH));
	xgeXuiWidgetSetBackground(pWidget, iBackground);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, iBorder);
	return pWidget;
}

static int SkeletonEditAdd(xge_xui_widget pParent, xge_xui_widget pChild)
{
	if ( (pParent == NULL) || (pChild == NULL) ) {
		return XGE_ERROR;
	}
	return xgeXuiWidgetAdd(pParent, pChild);
}

static int SkeletonEditAddLabel(ske_app_t* pApp, xge_xui_widget pParent, const char* sText, float fH, uint32_t iTextColor, uint32_t iBackground)
{
	ske_label_slot_t* pSlot;

	if ( (pApp == NULL) || (pParent == NULL) || (pApp->iLabelCount >= SKE_LABEL_CAPACITY) ) {
		return XGE_ERROR;
	}
	pSlot = &pApp->arrLabels[pApp->iLabelCount++];
	memset(pSlot, 0, sizeof(*pSlot));
	pSlot->pWidget = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, fH, iBackground, XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetPaddingPx(pSlot->pWidget, 8.0f, 0.0f, 8.0f, 0.0f);
	if ( xgeXuiLabelInit(&pSlot->tLabel, pSlot->pWidget, xgeXuiGetDefaultFont(&pApp->tXui), sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pSlot->tLabel, iTextColor);
	xgeXuiLabelSetAlign(&pSlot->tLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	pSlot->bReady = 1;
	return SkeletonEditAdd(pParent, pSlot->pWidget);
}

static int SkeletonEditInitDockWindow(ske_app_t* pApp, int iIndex, const char* sTitle, xge_xui_widget pClient, int bClosable)
{
	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= SKE_DOCK_WINDOW_COUNT) || (pClient == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiDockWindowInit(&pApp->arrDockWindow[iIndex], &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->iDockWindowReadyCount <= iIndex ) {
		pApp->iDockWindowReadyCount = iIndex + 1;
	}
	xgeXuiDockWindowSetTitle(&pApp->arrDockWindow[iIndex], sTitle);
	xgeXuiDockWindowSetClosable(&pApp->arrDockWindow[iIndex], bClosable);
	xgeXuiDockWindowSetClientWidget(&pApp->arrDockWindow[iIndex], pClient);
	return XGE_OK;
}

static int SkeletonEditPropertyAdd(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, int iFlags)
{
	xge_xui_property_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sId = sId;
	tDesc.sName = sName;
	tDesc.sDescription = sDescription;
	tDesc.iType = iType;
	tDesc.sValue = sValue;
	tDesc.sDefaultValue = sValue;
	tDesc.iFlags = iFlags;
	return xgeXuiPropertyGridAddProperty(pGrid, iCategory, &tDesc);
}

static void SkeletonEditSetRotationEditorStep(xge_xui_property_grid pGrid, int iProperty)
{
	xge_xui_table_grid_editor_config_t tConfig;

	if ( (pGrid == NULL) || (iProperty < 0) ) {
		return;
	}
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.iEnumSelected = -1;
	tConfig.iPrecision = 2;
	tConfig.fStep = 1.0f;
	xgeXuiPropertyGridSetEditorConfig(pGrid, iProperty, &tConfig);
}

static void SkeletonEditRefreshProperties(ske_app_t* pApp)
{
	int iDoc;
	int iSel;
	ske_bone_t* pBone;
	ske_slot_t* pSlot;
	ske_attachment_t* pAttachment;
	ske_animation_t* pAnim;
	ske_keyframe_t* pKey;
	char sText[128];
	char sParent[32];
	char sLength[32];
	char sRotation[32];
	char sOffsetX[32];
	char sOffsetY[32];
	char sUndo[32];
	char sRedo[32];
	char sCount[32];
	int iProp;

	if ( (pApp == NULL) || !pApp->bPropertyGridReady ) {
		return;
	}
	xgeXuiPropertyGridClear(&pApp->tPropertyGrid);
	iDoc = xgeXuiPropertyGridAddCategory(&pApp->tPropertyGrid, "document", "Document", 1);
	if ( iDoc >= 0 ) {
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iDoc, "document.name", "Name", "Current skeleton document.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pApp->tDocument.sName, XGE_XUI_PROPERTY_FLAG_READONLY);
		snprintf(sText, sizeof(sText), "%d", pApp->tDocument.iFrameRate);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iDoc, "document.fps", "Frame Rate", "Animation frame rate.", XGE_XUI_TABLE_CELL_TYPE_INT, sText, 0);
		snprintf(sCount, sizeof(sCount), "%d/%d/%d/%d/%d",
			pApp->tDocument.iBoneCount,
			pApp->tDocument.iSlotCount,
			pApp->tDocument.iAttachmentCount,
			pApp->tDocument.iAnimationCount,
			pApp->tDocument.iKeyframeCount);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iDoc, "document.counts", "Counts", "Bones/slots/attachments/animations/keyframes.", XGE_XUI_TABLE_CELL_TYPE_TEXT, sCount, XGE_XUI_PROPERTY_FLAG_READONLY);
		snprintf(sUndo, sizeof(sUndo), "%d", pApp->iUndoCount);
		snprintf(sRedo, sizeof(sRedo), "%d", pApp->iRedoCount);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iDoc, "document.undo", "Undo", "Available undo snapshots.", XGE_XUI_TABLE_CELL_TYPE_INT, sUndo, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iDoc, "document.redo", "Redo", "Available redo snapshots.", XGE_XUI_TABLE_CELL_TYPE_INT, sRedo, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iDoc, "document.dirty", "Dirty", "Document dirty state.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pApp->tDocument.bDirty ? "true" : "false", XGE_XUI_PROPERTY_FLAG_READONLY);
	}
	iSel = xgeXuiPropertyGridAddCategory(&pApp->tPropertyGrid, "selection", "Selection", 1);
	if ( iSel < 0 ) {
		xgeXuiWidgetMarkPaint(pApp->pPropertyGridWidget);
		return;
	}
	pBone = SkeletonEditSelectedBone(pApp);
	if ( pBone != NULL ) {
		snprintf(sParent, sizeof(sParent), "%d", pBone->iParent);
		snprintf(sLength, sizeof(sLength), "%.2f", pBone->fLength);
		snprintf(sRotation, sizeof(sRotation), "%.2f", pBone->fRotation * SKE_RAD_TO_DEG);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "bone.name", "Name", "Bone name.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pBone->sName, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "bone.parent", "Parent", "Parent bone index.", XGE_XUI_TABLE_CELL_TYPE_INT, sParent, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "bone.length", "Length", "Bone length in canvas units.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sLength, 0);
		iProp = SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "bone.rotation", "Rotation", "Local rotation in degrees.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sRotation, 0);
		SkeletonEditSetRotationEditorStep(&pApp->tPropertyGrid, iProp);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "bone.connected", "Connected", "Starts at parent end.", XGE_XUI_TABLE_CELL_TYPE_BOOL, pBone->bConnected ? "true" : "false", XGE_XUI_PROPERTY_FLAG_READONLY);
	} else if ( (pSlot = SkeletonEditSelectedSlot(pApp)) != NULL ) {
		snprintf(sParent, sizeof(sParent), "%d", pSlot->iBone);
		snprintf(sLength, sizeof(sLength), "%d", pSlot->iAttachment);
		snprintf(sText, sizeof(sText), "%d", pSlot->iDrawOrder);
		snprintf(sUndo, sizeof(sUndo), "%u", (unsigned int)pSlot->iColor);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "slot.name", "Name", "Slot name.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pSlot->sName, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "slot.bone", "Bone", "Target bone index.", XGE_XUI_TABLE_CELL_TYPE_INT, sParent, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "slot.attachment", "Attachment", "Active attachment index.", XGE_XUI_TABLE_CELL_TYPE_INT, sLength, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "slot.drawOrder", "Draw Order", "Rendering order.", XGE_XUI_TABLE_CELL_TYPE_INT, sText, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "slot.color", "Color", "Packed RGBA color.", XGE_XUI_TABLE_CELL_TYPE_INT, sUndo, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "slot.visible", "Visible", "Slot visibility.", XGE_XUI_TABLE_CELL_TYPE_BOOL, pSlot->bVisible ? "true" : "false", 0);
	} else if ( (pAttachment = SkeletonEditSelectedAttachment(pApp)) != NULL ) {
		snprintf(sParent, sizeof(sParent), "%d", pAttachment->iBone);
		snprintf(sLength, sizeof(sLength), "%d", pAttachment->iDrawOrder);
		snprintf(sOffsetX, sizeof(sOffsetX), "%.2f", pAttachment->fOffsetX);
		snprintf(sOffsetY, sizeof(sOffsetY), "%.2f", pAttachment->fOffsetY);
		snprintf(sRotation, sizeof(sRotation), "%.2f", pAttachment->fRotation * SKE_RAD_TO_DEG);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.name", "Name", "Attachment name.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pAttachment->sName, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.image", "Image", "Bound image resource.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pAttachment->sImage, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.bone", "Bone", "Target bone index.", XGE_XUI_TABLE_CELL_TYPE_INT, sParent, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.offsetX", "Offset X", "Attachment offset from the bone midpoint.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sOffsetX, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.offsetY", "Offset Y", "Attachment offset from the bone midpoint.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sOffsetY, 0);
		iProp = SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.rotation", "Rotation", "Attachment local rotation in degrees.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sRotation, 0);
		SkeletonEditSetRotationEditorStep(&pApp->tPropertyGrid, iProp);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "attachment.drawOrder", "Draw Order", "Rendering order.", XGE_XUI_TABLE_CELL_TYPE_INT, sLength, 0);
	} else if ( pApp->tDocument.iSelectedType == SKE_SELECTION_IMAGE ) {
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "image.name", "Image", "Selected image resource.", XGE_XUI_TABLE_CELL_TYPE_TEXT, SkeletonEditImageNameFromAppNode(pApp, pApp->tDocument.iSelectedImageNode), XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "image.action", "Action", "Use Attach mode or Add Attachment to bind this image to the selected bone.", XGE_XUI_TABLE_CELL_TYPE_TEXT, "Bind to selected bone", XGE_XUI_PROPERTY_FLAG_READONLY);
	} else if ( (pAnim = SkeletonEditSelectedAnimation(pApp)) != NULL ) {
		snprintf(sParent, sizeof(sParent), "%d", pAnim->iFrameStart);
		snprintf(sLength, sizeof(sLength), "%d", pAnim->iFrameEnd);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "animation.name", "Name", "Animation name.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pAnim->sName, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "animation.start", "Start", "Start frame.", XGE_XUI_TABLE_CELL_TYPE_INT, sParent, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "animation.end", "End", "End frame.", XGE_XUI_TABLE_CELL_TYPE_INT, sLength, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "animation.loop", "Loop", "Loop playback.", XGE_XUI_TABLE_CELL_TYPE_BOOL, pAnim->bLoop ? "true" : "false", 0);
	} else if ( (pKey = SkeletonEditSelectedKeyframe(pApp)) != NULL ) {
		snprintf(sParent, sizeof(sParent), "%d", pKey->iAnimation);
		snprintf(sLength, sizeof(sLength), "%d", pKey->iBone);
		snprintf(sText, sizeof(sText), "%d", pKey->iFrame);
		snprintf(sOffsetX, sizeof(sOffsetX), "%.2f", pKey->fLocalX);
		snprintf(sOffsetY, sizeof(sOffsetY), "%.2f", pKey->fLocalY);
		snprintf(sRotation, sizeof(sRotation), "%.2f", pKey->fRotation * SKE_RAD_TO_DEG);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "key.animation", "Animation", "Animation index.", XGE_XUI_TABLE_CELL_TYPE_INT, sParent, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "key.bone", "Bone", "Bone index.", XGE_XUI_TABLE_CELL_TYPE_INT, sLength, XGE_XUI_PROPERTY_FLAG_READONLY);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "key.frame", "Frame", "Keyframe frame.", XGE_XUI_TABLE_CELL_TYPE_INT, sText, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "key.x", "X", "Captured local X.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sOffsetX, 0);
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "key.y", "Y", "Captured local Y.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sOffsetY, 0);
		iProp = SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "key.rotation", "Rotation", "Captured local rotation.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sRotation, 0);
		SkeletonEditSetRotationEditorStep(&pApp->tPropertyGrid, iProp);
	} else {
		SkeletonEditPropertyAdd(&pApp->tPropertyGrid, iSel, "selection.none", "Object", "Current selection.", XGE_XUI_TABLE_CELL_TYPE_TEXT, "None", XGE_XUI_PROPERTY_FLAG_READONLY);
	}
	xgeXuiWidgetMarkPaint(pApp->pPropertyGridWidget);
}

void SkeletonEditPropertiesRequestRefresh(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->bPropertyRefreshPending = 1;
	if ( pApp->pPropertyGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pPropertyGridWidget);
	}
}

void SkeletonEditPropertiesFlushRefresh(ske_app_t* pApp)
{
	if ( (pApp == NULL) || !pApp->bPropertyRefreshPending || !pApp->bPropertyGridReady ) {
		return;
	}
	if ( xgeXuiPropertyGridIsEditing(&pApp->tPropertyGrid) ) {
		return;
	}
	pApp->bPropertyRefreshPending = 0;
	pApp->bPropertyUndoOpen = 0;
	pApp->sPropertyUndoId[0] = 0;
	SkeletonEditRefreshProperties(pApp);
}

static void SkeletonEditPropertyChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	ske_app_t* pApp;
	ske_bone_t* pBone;
	ske_slot_t* pSlot;
	ske_attachment_t* pAttachment;
	ske_animation_t* pAnim;
	ske_keyframe_t* pKey;
	float fValue;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pApp = (ske_app_t*)pUser;
	if ( (pApp == NULL) || (sId == NULL) || (sValue == NULL) ) {
		return;
	}
#define SkeletonEditHistoryPushUndo(pUndoApp) SkeletonEditPropertyPushUndo((pUndoApp), sId)
	pBone = SkeletonEditSelectedBone(pApp);
	if ( pBone != NULL ) {
		if ( strcmp(sId, "bone.name") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			snprintf(pBone->sName, sizeof(pBone->sName), "%s", sValue);
		} else if ( strcmp(sId, "bone.length") == 0 ) {
			fValue = (float)atof(sValue);
			if ( fValue < 1.0f ) fValue = 1.0f;
			if ( fValue > 2000.0f ) fValue = 2000.0f;
			SkeletonEditHistoryPushUndo(pApp);
			pBone->fLength = fValue;
		} else if ( strcmp(sId, "bone.rotation") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pBone->fRotation = (float)atof(sValue) * SKE_DEG_TO_RAD;
		} else {
			return;
		}
		pApp->tDocument.bDirty = 1;
	} else if ( (pSlot = SkeletonEditSelectedSlot(pApp)) != NULL ) {
		if ( strcmp(sId, "slot.name") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			snprintf(pSlot->sName, sizeof(pSlot->sName), "%s", sValue);
		} else if ( strcmp(sId, "slot.attachment") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pSlot->iAttachment = atoi(sValue);
			if ( pSlot->iAttachment >= pApp->tDocument.iAttachmentCount ) pSlot->iAttachment = pApp->tDocument.iAttachmentCount - 1;
			if ( pSlot->iAttachment < -1 ) pSlot->iAttachment = -1;
		} else if ( strcmp(sId, "slot.drawOrder") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pSlot->iDrawOrder = atoi(sValue);
		} else if ( strcmp(sId, "slot.color") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pSlot->iColor = (uint32_t)strtoul(sValue, NULL, 10);
		} else if ( strcmp(sId, "slot.visible") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pSlot->bVisible = (strcmp(sValue, "true") == 0 || strcmp(sValue, "1") == 0) ? 1 : 0;
		} else {
			return;
		}
		pApp->tDocument.bDirty = 1;
	} else if ( (pAttachment = SkeletonEditSelectedAttachment(pApp)) != NULL ) {
		if ( strcmp(sId, "attachment.name") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			snprintf(pAttachment->sName, sizeof(pAttachment->sName), "%s", sValue);
		} else if ( strcmp(sId, "attachment.offsetX") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAttachment->fOffsetX = (float)atof(sValue);
		} else if ( strcmp(sId, "attachment.offsetY") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAttachment->fOffsetY = (float)atof(sValue);
		} else if ( strcmp(sId, "attachment.rotation") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAttachment->fRotation = (float)atof(sValue) * SKE_DEG_TO_RAD;
		} else if ( strcmp(sId, "attachment.drawOrder") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAttachment->iDrawOrder = atoi(sValue);
		} else {
			return;
		}
		pApp->tDocument.bDirty = 1;
	} else if ( strcmp(sId, "document.fps") == 0 ) {
		SkeletonEditHistoryPushUndo(pApp);
		pApp->tDocument.iFrameRate = atoi(sValue);
		if ( pApp->tDocument.iFrameRate <= 0 ) {
			pApp->tDocument.iFrameRate = 24;
		}
		pApp->tDocument.bDirty = 1;
	} else if ( (pAnim = SkeletonEditSelectedAnimation(pApp)) != NULL ) {
		if ( strcmp(sId, "animation.name") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			snprintf(pAnim->sName, sizeof(pAnim->sName), "%s", sValue);
		} else if ( strcmp(sId, "animation.start") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAnim->iFrameStart = atoi(sValue);
		} else if ( strcmp(sId, "animation.end") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAnim->iFrameEnd = atoi(sValue);
		} else if ( strcmp(sId, "animation.loop") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pAnim->bLoop = (strcmp(sValue, "true") == 0 || strcmp(sValue, "1") == 0) ? 1 : 0;
		} else {
			return;
		}
		pApp->tDocument.bDirty = 1;
	} else if ( (pKey = SkeletonEditSelectedKeyframe(pApp)) != NULL ) {
		if ( strcmp(sId, "key.frame") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pKey->iFrame = atoi(sValue);
		} else if ( strcmp(sId, "key.x") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pKey->fLocalX = (float)atof(sValue);
		} else if ( strcmp(sId, "key.y") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pKey->fLocalY = (float)atof(sValue);
		} else if ( strcmp(sId, "key.rotation") == 0 ) {
			SkeletonEditHistoryPushUndo(pApp);
			pKey->fRotation = (float)atof(sValue) * SKE_DEG_TO_RAD;
		} else {
			return;
		}
		pApp->tDocument.bDirty = 1;
	} else {
		return;
	}
	SkeletonEditPropertiesRequestRefresh(pApp);
	SkeletonEditStatusRefresh(pApp);
	xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
	xgeXuiRefreshRequest(&pApp->tXui);
#undef SkeletonEditHistoryPushUndo
}

void SkeletonEditStatusSet(ske_app_t* pApp, const char* sText)
{
	if ( (pApp == NULL) || !pApp->bStatusReady || (pApp->iStatusMain < 0) ) {
		return;
	}
	xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusMain, (sText != NULL) ? sText : "Ready");
	xgeXuiWidgetMarkPaint(pApp->pStatusHost);
}

void SkeletonEditStatusRefresh(ske_app_t* pApp)
{
	char sMode[64];
	char sSelection[96];
	char sFrame[64];

	if ( (pApp == NULL) || !pApp->bStatusReady ) {
		return;
	}
	snprintf(sMode, sizeof(sMode), "Mode: %s", SkeletonEditModeName(pApp->iMode));
	if ( pApp->tDocument.iSelectedType == SKE_SELECTION_BONE && pApp->tDocument.iSelectedIndex >= 0 && pApp->tDocument.iSelectedIndex < pApp->tDocument.iBoneCount ) {
		snprintf(sSelection, sizeof(sSelection), "Bone: %s", pApp->tDocument.arrBones[pApp->tDocument.iSelectedIndex].sName);
	} else if ( pApp->tDocument.iSelectedType == SKE_SELECTION_SLOT && pApp->tDocument.iSelectedIndex >= 0 && pApp->tDocument.iSelectedIndex < pApp->tDocument.iSlotCount ) {
		snprintf(sSelection, sizeof(sSelection), "Slot: %s", pApp->tDocument.arrSlots[pApp->tDocument.iSelectedIndex].sName);
	} else if ( pApp->tDocument.iSelectedType == SKE_SELECTION_ATTACHMENT && pApp->tDocument.iSelectedIndex >= 0 && pApp->tDocument.iSelectedIndex < pApp->tDocument.iAttachmentCount ) {
		snprintf(sSelection, sizeof(sSelection), "Attachment: %s", pApp->tDocument.arrAttachments[pApp->tDocument.iSelectedIndex].sName);
	} else if ( pApp->tDocument.iSelectedType == SKE_SELECTION_IMAGE ) {
		snprintf(sSelection, sizeof(sSelection), "Image: %s", SkeletonEditImageNameFromAppNode(pApp, pApp->tDocument.iSelectedImageNode));
	} else if ( pApp->tDocument.iSelectedType == SKE_SELECTION_ANIMATION && pApp->tDocument.iSelectedIndex >= 0 && pApp->tDocument.iSelectedIndex < pApp->tDocument.iAnimationCount ) {
		snprintf(sSelection, sizeof(sSelection), "Animation: %s", pApp->tDocument.arrAnimations[pApp->tDocument.iSelectedIndex].sName);
	} else if ( pApp->tDocument.iSelectedType == SKE_SELECTION_KEYFRAME && pApp->tDocument.iSelectedIndex >= 0 && pApp->tDocument.iSelectedIndex < pApp->tDocument.iKeyframeCount ) {
		snprintf(sSelection, sizeof(sSelection), "Key: frame %d", pApp->tDocument.arrKeyframes[pApp->tDocument.iSelectedIndex].iFrame);
	} else {
		snprintf(sSelection, sizeof(sSelection), "Selection: None");
	}
	snprintf(sFrame, sizeof(sFrame), "Frame: %d", pApp->iCurrentFrame);
	xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusProject, "Project: DefaultSkeleton");
	xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusMode, sMode);
	xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusSelection, sSelection);
	xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusFrame, sFrame);
	xgeXuiWidgetMarkPaint(pApp->pStatusHost);
}

static void SkeletonEditMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	SkeletonEditCommandExecute((ske_app_t*)pUser, iValue);
}

static void SkeletonEditToolbarSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	static const int arrCommands[] = {
		SKE_CMD_NEW, SKE_CMD_OPEN, SKE_CMD_SAVE, SKE_CMD_IMPORT_IMAGE, 0,
		SKE_CMD_MODE_SELECT, SKE_CMD_MODE_BONE, SKE_CMD_MODE_SLOT, SKE_CMD_MODE_ATTACHMENT,
		SKE_CMD_MODE_KEY, 0, SKE_CMD_PLAY
	};

	(void)pWidget;
	if ( (iIndex >= 0) && (iIndex < (int)(sizeof(arrCommands) / sizeof(arrCommands[0]))) && (arrCommands[iIndex] != 0) ) {
		SkeletonEditCommandExecute((ske_app_t*)pUser, arrCommands[iIndex]);
	}
}

static void SkeletonEditTreeSelect(xge_xui_widget pWidget, int iNodeId, void* pUser)
{
	ske_app_t* pApp;

	(void)pWidget;
	pApp = (ske_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iSelectedTreeNode = iNodeId;
	if ( SkeletonEditImageIndexFromNode(iNodeId) >= 0 ) {
		pApp->tDocument.iSelectedType = SKE_SELECTION_IMAGE;
		pApp->tDocument.iSelectedImageNode = iNodeId;
		pApp->tDocument.iSelectedIndex = -1;
		SkeletonEditRefreshProperties(pApp);
	}
	SkeletonEditStatusSet(pApp, "Asset selected");
	SkeletonEditStatusRefresh(pApp);
}

static int SkeletonEditFrameChanging(xge_xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	(void)pUser;
	return 1;
}

static void SkeletonEditFrameChanged(xge_xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	ske_app_t* pApp;

	(void)pWidget;
	(void)iOldFrame;
	pApp = (ske_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iCurrentFrame = iNewFrame;
	SkeletonEditStatusRefresh(pApp);
}

static void SkeletonEditCanvasPaint(xge_xui_widget pWidget, void* pUser)
{
	ske_app_t* pApp;
	xge_rect_t r;
	float cx;
	float cy;
	float x;
	float y;
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	float fSX0;
	float fSY0;
	float fSX1;
	float fSY1;
	float fRotation;
	float fGrid;
	float fGridStart;
	ske_bone_t* pBone;
	ske_attachment_t* pAttachment;
	xge_texture pTexture;
	uint32_t iBoneColor;
	int i;

	pApp = (ske_app_t*)pUser;
	if ( pWidget == NULL ) {
		return;
	}
	r = pWidget->tRect;
	xgeShapeRectFillPx(r, XGE_COLOR_RGBA(242, 245, 248, 255));
	SkeletonEditDocToScreen(pApp, r, 0.0f, 0.0f, &cx, &cy);
	fGrid = 32.0f * SkeletonEditCanvasZoom(pApp);
	if ( fGrid < 8.0f ) fGrid = 8.0f;
	if ( fGrid > 96.0f ) fGrid = 96.0f;
	fGridStart = cx - floorf((cx - r.fX) / fGrid) * fGrid;
	for ( x = fGridStart; x < r.fX + r.fW; x += fGrid ) {
		xgeShapeLinePx(x, r.fY, x, r.fY + r.fH, 1.0f, XGE_COLOR_RGBA(220, 226, 232, 255));
	}
	fGridStart = cy - floorf((cy - r.fY) / fGrid) * fGrid;
	for ( y = fGridStart; y < r.fY + r.fH; y += fGrid ) {
		xgeShapeLinePx(r.fX, y, r.fX + r.fW, y, 1.0f, XGE_COLOR_RGBA(220, 226, 232, 255));
	}
	xgeShapeLinePx(cx - 120.0f, cy, cx + 120.0f, cy, 1.5f, XGE_COLOR_RGBA(170, 184, 196, 255));
	xgeShapeLinePx(cx, cy - 150.0f, cx, cy + 90.0f, 1.5f, XGE_COLOR_RGBA(170, 184, 196, 255));
	if ( pApp != NULL ) {
		for ( i = 0; i < pApp->tDocument.iAttachmentCount; i++ ) {
			pAttachment = &pApp->tDocument.arrAttachments[i];
			if ( pAttachment->iBone < 0 || pAttachment->iBone >= pApp->tDocument.iBoneCount ) {
				continue;
			}
			SkeletonEditBoneWorld(pApp, pAttachment->iBone, &fX0, &fY0, &fX1, &fY1, &fRotation);
			SkeletonEditDocToScreen(pApp, r, (fX0 + fX1) * 0.5f + pAttachment->fOffsetX, (fY0 + fY1) * 0.5f + pAttachment->fOffsetY, &fSX0, &fSY0);
			pTexture = SkeletonEditTextureForImage(pApp, pAttachment->sImage);
			if ( (pTexture != NULL) && (pTexture->iWidth > 0) && (pTexture->iHeight > 0) ) {
				xge_draw_t tDraw;
				float fZoom;
				float fScaleX;
				float fScaleY;

				fZoom = SkeletonEditCanvasZoom(pApp);
				fScaleX = (pAttachment->fScaleX == 0.0f) ? 1.0f : pAttachment->fScaleX;
				fScaleY = (pAttachment->fScaleY == 0.0f) ? 1.0f : pAttachment->fScaleY;
				memset(&tDraw, 0, sizeof(tDraw));
				tDraw.pTexture = pTexture;
				tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pTexture->iWidth, (float)pTexture->iHeight };
				tDraw.tDst.fX = fSX0;
				tDraw.tDst.fY = fSY0;
				tDraw.tDst.fW = (float)pTexture->iWidth * fZoom * fScaleX;
				tDraw.tDst.fH = (float)pTexture->iHeight * fZoom * fScaleY;
				tDraw.tOrigin.fX = (pAttachment->fPivotX != 0.0f) ? (pAttachment->fPivotX * fZoom * fScaleX) : (tDraw.tDst.fW * 0.5f);
				tDraw.tOrigin.fY = (pAttachment->fPivotY != 0.0f) ? (pAttachment->fPivotY * fZoom * fScaleY) : (tDraw.tDst.fH * 0.5f);
				tDraw.fRotation = fRotation + pAttachment->fRotation;
				tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
				tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
				xgeDrawEx(&tDraw);
				xgeShapeRectStrokePx((xge_rect_t){ fSX0 - tDraw.tOrigin.fX, fSY0 - tDraw.tOrigin.fY, tDraw.tDst.fW, tDraw.tDst.fH }, 1.5f,
					(pApp->tDocument.iSelectedType == SKE_SELECTION_ATTACHMENT && pApp->tDocument.iSelectedIndex == i) ? XGE_COLOR_RGBA(238, 150, 40, 255) : XGE_COLOR_RGBA(178, 126, 38, 160));
			} else {
				xgeShapeRectFillPx((xge_rect_t){ fSX0 - 24.0f, fSY0 - 18.0f, 48.0f, 36.0f }, XGE_COLOR_RGBA(238, 208, 130, 132));
				xgeShapeRectStrokePx((xge_rect_t){ fSX0 - 24.0f, fSY0 - 18.0f, 48.0f, 36.0f }, 1.5f,
					(pApp->tDocument.iSelectedType == SKE_SELECTION_ATTACHMENT && pApp->tDocument.iSelectedIndex == i) ? XGE_COLOR_RGBA(238, 150, 40, 255) : XGE_COLOR_RGBA(178, 126, 38, 220));
			}
		}
		for ( i = 0; i < pApp->tDocument.iBoneCount; i++ ) {
			pBone = &pApp->tDocument.arrBones[i];
			if ( !pBone->bVisible ) {
				continue;
			}
			SkeletonEditBoneWorld(pApp, i, &fX0, &fY0, &fX1, &fY1, NULL);
			SkeletonEditDocToScreen(pApp, r, fX0, fY0, &fSX0, &fSY0);
			SkeletonEditDocToScreen(pApp, r, fX1, fY1, &fSX1, &fSY1);
			iBoneColor = (pApp->tDocument.iSelectedType == SKE_SELECTION_BONE && pApp->tDocument.iSelectedIndex == i) ? XGE_COLOR_RGBA(238, 150, 40, 255) : XGE_COLOR_RGBA(58, 124, 184, 255);
			xgeShapeLinePx(fSX0, fSY0, fSX1, fSY1, 5.0f, iBoneColor);
			xgeShapeCircleFillPx(fSX0, fSY0, 4.5f, XGE_COLOR_RGBA(28, 76, 128, 255));
			xgeShapeCircleFillPx(fSX1, fSY1, 5.5f, iBoneColor);
		}
		if ( pApp->bCanvasCreateBone ) {
			xgeShapeLinePx(pApp->fCreateStartX, pApp->fCreateStartY, pApp->fCreateEndX, pApp->fCreateEndY, 3.0f, XGE_COLOR_RGBA(240, 170, 56, 255));
			xgeShapeCircleStrokePx(pApp->fCreateEndX, pApp->fCreateEndY, 7.0f, 2.0f, XGE_COLOR_RGBA(240, 170, 56, 255));
		}
	}
	xgeShapeRectStrokePx(r, 1.0f, XGE_COLOR_RGBA(148, 164, 180, 255));
}

static int SkeletonEditInitMenuBar(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pMenuBar == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiMenuInit(&pApp->tFileMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tEditMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tViewMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tSkeletonMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tAnimationMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tHelpMenu, &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuSetItems(&pApp->tFileMenu, g_arrFileMenu, (int)(sizeof(g_arrFileMenu) / sizeof(g_arrFileMenu[0])));
	xgeXuiMenuSetItems(&pApp->tEditMenu, g_arrEditMenu, (int)(sizeof(g_arrEditMenu) / sizeof(g_arrEditMenu[0])));
	xgeXuiMenuSetItems(&pApp->tViewMenu, g_arrViewMenu, (int)(sizeof(g_arrViewMenu) / sizeof(g_arrViewMenu[0])));
	xgeXuiMenuSetItems(&pApp->tSkeletonMenu, g_arrSkeletonMenu, (int)(sizeof(g_arrSkeletonMenu) / sizeof(g_arrSkeletonMenu[0])));
	xgeXuiMenuSetItems(&pApp->tAnimationMenu, g_arrAnimationMenu, (int)(sizeof(g_arrAnimationMenu) / sizeof(g_arrAnimationMenu[0])));
	xgeXuiMenuSetItems(&pApp->tHelpMenu, g_arrHelpMenu, (int)(sizeof(g_arrHelpMenu) / sizeof(g_arrHelpMenu[0])));
	xgeXuiMenuSetSelect(&pApp->tFileMenu, SkeletonEditMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tEditMenu, SkeletonEditMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tViewMenu, SkeletonEditMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tSkeletonMenu, SkeletonEditMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tAnimationMenu, SkeletonEditMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tHelpMenu, SkeletonEditMenuSelect, pApp);
	if ( xgeXuiMenuBarInit(&pApp->tMenuBar, &pApp->tXui, pApp->pMenuBar) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuBarSetFont(&pApp->tMenuBar, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiMenuBarSetSelect(&pApp->tMenuBar, SkeletonEditMenuSelect, pApp);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "File", &pApp->tFileMenu, 1);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "Edit", &pApp->tEditMenu, 2);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "View", &pApp->tViewMenu, 3);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "Skeleton", &pApp->tSkeletonMenu, 4);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "Animation", &pApp->tAnimationMenu, 5);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "Help", &pApp->tHelpMenu, 6);
	pApp->bMenuReady = 1;
	return XGE_OK;
}

static int SkeletonEditInitToolbar(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pToolbar == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiToolbarInit(&pApp->tToolbar, &pApp->tXui, pApp->pToolbar) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiToolbarSetFont(&pApp->tToolbar, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiToolbarSetItems(&pApp->tToolbar, g_arrToolbarText, g_arrToolbarType, (int)(sizeof(g_arrToolbarText) / sizeof(g_arrToolbarText[0])));
	xgeXuiToolbarSetItemSize(&pApp->tToolbar, 74.0f, 28.0f, 10.0f);
	xgeXuiToolbarSetGroupGap(&pApp->tToolbar, 8.0f);
	xgeXuiToolbarSetSelect(&pApp->tToolbar, SkeletonEditToolbarSelect, pApp);
	xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 5, 1);
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 3, "Import image into the project");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 5, "Select bones, slots and attachments");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 6, "Create or edit bones");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 7, "Create or edit slots");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 8, "Bind image attachments");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 9, "Add animation key");
	pApp->bToolbarReady = 1;
	return XGE_OK;
}

static int SkeletonEditInitStatusBar(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pStatusHost == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pStatusHost) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, xgeXuiGetDefaultFont(&pApp->tXui));
	pApp->iStatusMain = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 160.0f, 0);
	pApp->iStatusProject = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Project: DefaultSkeleton", 260.0f, 0);
	pApp->iStatusMode = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Mode: Select", 150.0f, 0);
	xgeXuiStatusBarAddFlexibleSpacer(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 1.0f);
	pApp->iStatusSelection = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "Selection: root", 210.0f, 0);
	pApp->iStatusFrame = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "Frame: 0", 110.0f, 0);
	pApp->bStatusReady = 1;
	SkeletonEditStatusRefresh(pApp);
	return XGE_OK;
}

static int SkeletonEditInitAssetTree(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pAssetTreeWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiTreeViewInit(&pApp->tAssetTree, &pApp->tXui, pApp->pAssetTreeWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTreeViewSetFont(&pApp->tAssetTree, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiTreeViewSetMetrics(&pApp->tAssetTree, 24.0f, 18.0f);
	xgeXuiTreeViewSetScrollbarMode(&pApp->tAssetTree, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiTreeViewSetColors(&pApp->tAssetTree,
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(206, 229, 246, 255),
		XGE_COLOR_RGBA(42, 62, 78, 255),
		XGE_COLOR_RGBA(232, 238, 244, 255),
		XGE_COLOR_RGBA(154, 176, 194, 255));
	xgeXuiTreeViewSetSelect(&pApp->tAssetTree, SkeletonEditTreeSelect, pApp);
	pApp->iSelectedTreeNode = 102;
	pApp->bAssetTreeReady = 1;
	return SkeletonEditRefreshAssetTree(pApp);
}

static int SkeletonEditInitPropertyGrid(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pPropertyGridWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiPropertyGridInit(&pApp->tPropertyGrid, &pApp->tXui, pApp->pPropertyGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bPropertyGridReady = 1;
	xgeXuiPropertyGridSetFont(&pApp->tPropertyGrid, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiPropertyGridSetMetrics(&pApp->tPropertyGrid, 104.0f, 24.0f, 25.0f);
	xgeXuiPropertyGridSetDescriptionMode(&pApp->tPropertyGrid, XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	xgeXuiPropertyGridSetEditMode(&pApp->tPropertyGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiPropertyGridSetScrollbarMode(&pApp->tPropertyGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiPropertyGridSetColors(&pApp->tPropertyGrid,
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(228, 238, 246, 255),
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(210, 230, 244, 255),
		XGE_COLOR_RGBA(172, 204, 228, 255),
		XGE_COLOR_RGBA(36, 68, 96, 255));
	xgeXuiPropertyGridSetChange(&pApp->tPropertyGrid, SkeletonEditPropertyChange, pApp);
	SkeletonEditRefreshProperties(pApp);
	return XGE_OK;
}

static int SkeletonEditInitTimeline(ske_app_t* pApp)
{
	int iRoot;
	int iBody;
	int iHead;
	int iArmL;
	int iArmR;
	int iSlotBody;
	int iEvents;

	if ( (pApp == NULL) || (pApp->pTimelineWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiTimelineViewInit(&pApp->tTimeline, &pApp->tXui, pApp->pTimelineWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bTimelineReady = 1;
	xgeXuiTimelineViewSetFont(&pApp->tTimeline, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiTimelineViewSetFrameCount(&pApp->tTimeline, 96);
	xgeXuiTimelineViewSetFrameRate(&pApp->tTimeline, 24);
	xgeXuiTimelineViewSetMetrics(&pApp->tTimeline, 132.0f, 12.0f, 24.0f, 24.0f);
	xgeXuiTimelineViewSetScrollbarMode(&pApp->tTimeline, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiTimelineViewSetCurrentFrameProc(&pApp->tTimeline, SkeletonEditFrameChanging, SkeletonEditFrameChanged, pApp);
	iRoot = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 1, "root");
	iBody = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 2, "body");
	iHead = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 3, "head");
	iArmL = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 4, "arm.L");
	iArmR = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 5, "arm.R");
	iSlotBody = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 6, "slot.body");
	iEvents = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 7, "events");
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iRoot, 0, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iBody, 0, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iBody, 24, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iHead, 12, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iArmL, 8, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iArmR, 8, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iSlotBody, 0, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, iEvents, 36, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewAddSpan(&pApp->tTimeline, iBody, 201, 0, 48, XGE_XUI_TIMELINE_SPAN_MOTION, "idle sway");
	xgeXuiTimelineViewAddSpan(&pApp->tTimeline, iArmL, 202, 8, 32, XGE_XUI_TIMELINE_SPAN_MOTION, "arm swing");
	xgeXuiTimelineViewAddSpan(&pApp->tTimeline, iArmR, 203, 8, 32, XGE_XUI_TIMELINE_SPAN_MOTION, "arm swing");
	xgeXuiTimelineViewSetCurrentFrame(&pApp->tTimeline, 0);
	return XGE_OK;
}

int SkeletonEditCommandExecute(ske_app_t* pApp, int iCommand)
{
	int iModeIndex;
	const char* sModeStatus;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iModeIndex = -1;
	sModeStatus = NULL;
	switch ( iCommand ) {
	case SKE_CMD_NEW:
		if ( SkeletonEditNewDocument(pApp) != XGE_OK ) {
			SkeletonEditStatusSet(pApp, "New document failed");
		}
		iModeIndex = 5;
		break;
	case SKE_CMD_EXIT:
		xgeQuit();
		return XGE_OK;
	case SKE_CMD_OPEN:
		if ( SkeletonEditOpenDocument(pApp) != XGE_OK ) {
			SkeletonEditStatusSet(pApp, "Open failed");
		}
		break;
	case SKE_CMD_SAVE:
		if ( SkeletonEditSaveDocument(pApp, 0) != XGE_OK ) {
			SkeletonEditStatusSet(pApp, "Save failed");
		}
		break;
	case SKE_CMD_SAVE_AS:
		if ( SkeletonEditSaveDocument(pApp, 1) != XGE_OK ) {
			SkeletonEditStatusSet(pApp, "Save As failed");
		}
		break;
	case SKE_CMD_IMPORT_IMAGE:
		if ( SkeletonEditImportImage(pApp) != XGE_OK ) {
			SkeletonEditStatusSet(pApp, "Import image failed");
		}
		break;
	case SKE_CMD_UNDO:
		SkeletonEditHistoryUndo(pApp);
		SkeletonEditRefreshProperties(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		break;
	case SKE_CMD_REDO:
		SkeletonEditHistoryRedo(pApp);
		SkeletonEditRefreshProperties(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		break;
	case SKE_CMD_DELETE:
		SkeletonEditDeleteSelected(pApp);
		SkeletonEditRefreshProperties(pApp);
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		break;
	case SKE_CMD_RESET_LAYOUT:
		SkeletonEditResetLayout(pApp);
		break;
	case SKE_CMD_FRAME_ALL:
		SkeletonEditCanvasResetView(pApp);
		SkeletonEditStatusSet(pApp, "Canvas view reset");
		xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		break;
	case SKE_CMD_MODE_SELECT:
		pApp->iMode = SKE_MODE_SELECT;
		pApp->fPlaybackAccum = 0.0f;
		iModeIndex = 5;
		sModeStatus = "Select mode";
		break;
	case SKE_CMD_MODE_BONE:
		pApp->iMode = SKE_MODE_BONE;
		pApp->fPlaybackAccum = 0.0f;
		iModeIndex = 6;
		sModeStatus = "Bone mode";
		break;
	case SKE_CMD_ADD_BONE:
		{
			float fX0;
			float fY0;
			float fX1;
			float fY1;
			int iParent;

			iParent = (pApp->tDocument.iActiveBoneIndex >= 0 && pApp->tDocument.iActiveBoneIndex < pApp->tDocument.iBoneCount) ? pApp->tDocument.iActiveBoneIndex : -1;
			if ( iParent >= 0 ) {
				SkeletonEditBoneWorld(pApp, iParent, NULL, NULL, &fX0, &fY0, NULL);
				fX1 = fX0 + 48.0f;
				fY1 = fY0 - 32.0f;
			} else {
				fX0 = 0.0f;
				fY0 = 0.0f;
				fX1 = 0.0f;
				fY1 = -64.0f;
			}
			SkeletonEditAddBoneFromDrag(pApp, iParent, fX0, fY0, fX1, fY1);
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Bone added");
			xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
			pApp->iMode = SKE_MODE_BONE;
			iModeIndex = 6;
		}
		break;
	case SKE_CMD_MODE_SLOT:
		pApp->iMode = SKE_MODE_SLOT;
		pApp->fPlaybackAccum = 0.0f;
		iModeIndex = 7;
		sModeStatus = "Slot mode";
		break;
	case SKE_CMD_ADD_SLOT:
		if ( SkeletonEditAddSlot(pApp) == XGE_OK ) {
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Slot added");
			xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		} else {
			SkeletonEditStatusSet(pApp, "Add slot failed");
		}
		pApp->iMode = SKE_MODE_SLOT;
			iModeIndex = 7;
		break;
	case SKE_CMD_MODE_ATTACHMENT:
		if ( pApp->tDocument.iSelectedType == SKE_SELECTION_IMAGE ) {
			if ( SkeletonEditBindSelectedImage(pApp) == XGE_OK ) {
				SkeletonEditRefreshProperties(pApp);
				SkeletonEditStatusSet(pApp, "Image bound to bone");
				xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
			} else {
				SkeletonEditStatusSet(pApp, "Bind image failed: select a bone first");
			}
			pApp->iMode = SKE_MODE_ATTACHMENT;
			iModeIndex = 8;
		} else {
			pApp->iMode = SKE_MODE_ATTACHMENT;
			iModeIndex = 8;
			sModeStatus = "Attachment mode";
		}
		break;
	case SKE_CMD_ADD_ATTACHMENT:
		if ( SkeletonEditBindSelectedImage(pApp) == XGE_OK ) {
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Image bound to bone");
			xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
		} else {
			SkeletonEditStatusSet(pApp, "Bind image failed");
		}
		pApp->iMode = SKE_MODE_ATTACHMENT;
		iModeIndex = 8;
		break;
	case SKE_CMD_MODE_KEY:
		pApp->iMode = SKE_MODE_KEY;
		pApp->fPlaybackAccum = 0.0f;
		iModeIndex = 9;
		sModeStatus = "Key mode";
		break;
	case SKE_CMD_ADD_ANIMATION:
		if ( SkeletonEditAddAnimation(pApp) == XGE_OK ) {
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Animation added");
		} else {
			SkeletonEditStatusSet(pApp, "Add animation failed");
		}
		break;
	case SKE_CMD_ADD_KEY:
		if ( SkeletonEditAddKeyframe(pApp) == XGE_OK ) {
			SkeletonEditRefreshProperties(pApp);
			SkeletonEditStatusSet(pApp, "Keyframe added");
		} else {
			SkeletonEditStatusSet(pApp, "Add key failed");
		}
		pApp->iMode = SKE_MODE_KEY;
		iModeIndex = 9;
		break;
	case SKE_CMD_PLAY:
		pApp->iMode = SKE_MODE_PLAY;
		pApp->fPlaybackAccum = 0.0f;
		iModeIndex = 11;
		sModeStatus = "Playback started";
		break;
	case SKE_CMD_STOP:
		pApp->iMode = SKE_MODE_SELECT;
		pApp->fPlaybackAccum = 0.0f;
		iModeIndex = 5;
		sModeStatus = "Playback stopped";
		break;
	case SKE_CMD_ABOUT:
		SkeletonEditStatusSet(pApp, "SkeletonEdit - XGE static skeleton editor");
		break;
	default:
		SkeletonEditStatusSet(pApp, "Command is wired");
		break;
	}
	if ( iModeIndex >= 0 && pApp->bToolbarReady ) {
		xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 5, 0);
		xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 6, 0);
		xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 7, 0);
		xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 8, 0);
		xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 9, 0);
		if ( iModeIndex >= 5 && iModeIndex <= 9 ) {
			xgeXuiToolbarSetItemChecked(&pApp->tToolbar, iModeIndex, 1);
		}
		SkeletonEditRefreshProperties(pApp);
		if ( sModeStatus != NULL ) {
			SkeletonEditStatusSet(pApp, sModeStatus);
		}
	}
	SkeletonEditStatusRefresh(pApp);
	xgeXuiRefreshRequest(&pApp->tXui);
	return XGE_OK;
}

int SkeletonEditShellBuild(ske_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pApp->pMenuBar = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 26.0f, XGE_COLOR_RGBA(242, 246, 250, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	pApp->pToolbar = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 34.0f, XGE_COLOR_RGBA(234, 241, 247, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	pApp->pDockHost = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(224, 229, 235, 255), XGE_COLOR_RGBA(154, 174, 192, 255));
	pApp->pStatusHost = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 24.0f, XGE_COLOR_RGBA(238, 244, 249, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	if ( SkeletonEditAdd(pApp->pRoot, pApp->pMenuBar) != XGE_OK ||
	     SkeletonEditAdd(pApp->pRoot, pApp->pToolbar) != XGE_OK ||
	     SkeletonEditAdd(pApp->pRoot, pApp->pDockHost) != XGE_OK ||
	     SkeletonEditAdd(pApp->pRoot, pApp->pStatusHost) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( SkeletonEditInitMenuBar(pApp) != XGE_OK ||
	     SkeletonEditInitToolbar(pApp) != XGE_OK ||
	     SkeletonEditInitStatusBar(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiDockLayoutInit(&pApp->tDockLayout, &pApp->tXui, pApp->pDockHost) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bDockLayoutReady = 1;

	pApp->pAssetPanel = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pAssetTreeWidget = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pPropertyPanel = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pPropertyGridWidget = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pCenterPanel = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(218, 225, 232, 255), XGE_COLOR_RGBA(154, 174, 192, 255));
	pApp->pTimelineWidget = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, 214.0f, XGE_COLOR_RGBA(247, 250, 253, 255), XGE_COLOR_RGBA(170, 188, 204, 255));
	pApp->pCanvasWidget = SkeletonEditCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(242, 245, 248, 255), XGE_COLOR_RGBA(148, 164, 180, 255));
	if ( pApp->pAssetPanel == NULL || pApp->pAssetTreeWidget == NULL ||
	     pApp->pPropertyPanel == NULL || pApp->pPropertyGridWidget == NULL ||
	     pApp->pCenterPanel == NULL || pApp->pTimelineWidget == NULL || pApp->pCanvasWidget == NULL ) {
		return XGE_ERROR;
	}
	if ( SkeletonEditAddLabel(pApp, pApp->pAssetPanel, "Asset Tree", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     SkeletonEditAdd(pApp->pAssetPanel, pApp->pAssetTreeWidget) != XGE_OK ||
	     SkeletonEditAddLabel(pApp, pApp->pPropertyPanel, "Properties", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     SkeletonEditAdd(pApp->pPropertyPanel, pApp->pPropertyGridWidget) != XGE_OK ||
	     SkeletonEditAdd(pApp->pCenterPanel, pApp->pTimelineWidget) != XGE_OK ||
	     SkeletonEditAdd(pApp->pCenterPanel, pApp->pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetPaint(pApp->pCanvasWidget, SkeletonEditCanvasPaint, pApp);
	if ( SkeletonEditInitAssetTree(pApp) != XGE_OK ||
	     SkeletonEditInitPropertyGrid(pApp) != XGE_OK ||
	     SkeletonEditInitTimeline(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( SkeletonEditInitDockWindow(pApp, SKE_DOCK_CENTER, "Skeleton Editor", pApp->pCenterPanel, 0) != XGE_OK ||
	     SkeletonEditInitDockWindow(pApp, SKE_DOCK_ASSETS, "Assets", pApp->pAssetPanel, 1) != XGE_OK ||
	     SkeletonEditInitDockWindow(pApp, SKE_DOCK_PROPERTIES, "Properties", pApp->pPropertyPanel, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pCenterPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[SKE_DOCK_CENTER], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);
	pApp->pAssetPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[SKE_DOCK_ASSETS], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_LEFT, 0.22f);
	pApp->pPropertyPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[SKE_DOCK_PROPERTIES], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_RIGHT, 0.28f);
	if ( (pApp->pCenterPane == NULL) || (pApp->pAssetPane == NULL) || (pApp->pPropertyPane == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pCenterPane, 0);
	xgeXuiDockPaneSetActiveIndex(pApp->pAssetPane, 0);
	xgeXuiDockPaneSetActiveIndex(pApp->pPropertyPane, 0);
	SkeletonEditStatusSet(pApp, "Skeleton editor shell ready");
	SkeletonEditStatusRefresh(pApp);
	return XGE_OK;
}

void SkeletonEditShellUnit(ske_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	SkeletonEditTextureCacheClear(pApp);
	if ( pApp->bTimelineReady ) {
		xgeXuiTimelineViewUnit(&pApp->tTimeline);
		pApp->bTimelineReady = 0;
	}
	if ( pApp->bPropertyGridReady ) {
		xgeXuiPropertyGridUnit(&pApp->tPropertyGrid);
		pApp->bPropertyGridReady = 0;
	}
	if ( pApp->bAssetTreeReady ) {
		xgeXuiTreeViewUnit(&pApp->tAssetTree);
		pApp->bAssetTreeReady = 0;
	}
	if ( pApp->bStatusReady ) {
		xgeXuiStatusBarUnit(&pApp->tStatusBar);
		pApp->bStatusReady = 0;
	}
	if ( pApp->bToolbarReady ) {
		xgeXuiToolbarUnit(&pApp->tToolbar);
		pApp->bToolbarReady = 0;
	}
	if ( pApp->bMenuReady ) {
		xgeXuiMenuBarUnit(&pApp->tMenuBar);
		xgeXuiMenuUnit(&pApp->tHelpMenu);
		xgeXuiMenuUnit(&pApp->tAnimationMenu);
		xgeXuiMenuUnit(&pApp->tSkeletonMenu);
		xgeXuiMenuUnit(&pApp->tViewMenu);
		xgeXuiMenuUnit(&pApp->tEditMenu);
		xgeXuiMenuUnit(&pApp->tFileMenu);
		pApp->bMenuReady = 0;
	}
	for ( i = pApp->iDockWindowReadyCount - 1; i >= 0; i-- ) {
		xgeXuiDockWindowUnit(&pApp->arrDockWindow[i]);
	}
	if ( pApp->bDockLayoutReady ) {
		xgeXuiDockLayoutUnit(&pApp->tDockLayout);
		pApp->bDockLayoutReady = 0;
	}
	pApp->iDockWindowReadyCount = 0;
}
