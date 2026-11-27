#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 820
#define DEMO_H 520
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DEMO_PADDING_BUTTON_COUNT 4

#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_LEFT_ALT 342
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#define XGE_KEY_RIGHT_ALT 346
#endif

#ifndef XGE_KEY_F3
#define XGE_KEY_F3 292
#endif

typedef struct xui_codeedit_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pCodeEdit;
	xui_widget arrPaddingButtons[DEMO_PADDING_BUTTON_COUNT];
	int iLineNumberPadding;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bRenderOK;
	int bLexerOK;
	int bInputOK;
	int bMenuOK;
	int bScrollOK;
	int bFindOK;
	int bCompletionOK;
	int bMinimapOK;
} xui_codeedit_demo_t;

static const char* g_sCodeEditDemoText =
	"#include <stdio.h>\n"
	"\n"
	"static int add(int a, int b) {\n"
	"\treturn a + b;\n"
	"}\n"
	"\n"
	"int main(void) {\n"
	"\tprintf(\"hello xui codeedit: %d\\n\", add(2, 3));\n"
	"\treturn 0;\n"
	"}\n";

static int __xuiCodeEditDemoCompletion(xui_widget_t* pWidget, int iOffset,
	const char* sPrefix, xui_code_completion_item_t* pItems,
	int iItemCapacity, int* pItemCount, void* pUser)
{
	static const char* arrLabels[] = {"printf", "puts", "perror", "sizeof"};
	int i;
	int iCount;

	(void)pWidget;
	(void)iOffset;
	(void)sPrefix;
	(void)pUser;
	if ( pItemCount == NULL || iItemCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = (int)(sizeof(arrLabels) / sizeof(arrLabels[0]));
	if ( iCount > iItemCapacity ) iCount = iItemCapacity;
	if ( pItems != NULL ) {
		for ( i = 0; i < iCount; i++ ) {
			memset(&pItems[i], 0, sizeof(pItems[i]));
			pItems[i].iSize = sizeof(pItems[i]);
			pItems[i].sLabel = arrLabels[i];
			pItems[i].sInsertText = arrLabels[i];
			pItems[i].sDetail = "C completion";
			pItems[i].sDocumentation = "Locally filtered completion candidate.";
			pItems[i].sFilterText = arrLabels[i];
			pItems[i].sSortText = arrLabels[i];
			pItems[i].sCommitCharacters = "(.";
			pItems[i].iKind = XUI_CODE_TOKEN_IDENTIFIER;
			pItems[i].iSortOrder = i;
		}
	}
	*pItemCount = iCount;
	return XUI_OK;
}

static int __xuiCodeEditDemoSignature(xui_widget_t* pWidget, int iOffset,
	xui_code_signature_help_t* pHelp, void* pUser)
{
	static xui_code_signature_parameter_t arrParameters[2];

	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	if ( pHelp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrParameters, 0, sizeof(arrParameters));
	arrParameters[0].iSize = sizeof(arrParameters[0]);
	arrParameters[0].sLabel = "format";
	arrParameters[0].tLabelRange = (xui_code_range_t){7, 13};
	arrParameters[1].iSize = sizeof(arrParameters[1]);
	arrParameters[1].sLabel = "...";
	arrParameters[1].tLabelRange = (xui_code_range_t){15, 18};
	pHelp->iSize = sizeof(*pHelp);
	pHelp->sLabel = "printf(format, ...)";
	pHelp->sDocumentation = "Writes formatted output to stdout.";
	pHelp->pParameters = arrParameters;
	pHelp->iParameterCount = 2;
	pHelp->iActiveParameter = 0;
	return XUI_OK;
}

static int __xuiCodeEditDemoInput(xui_widget_t* pWidget,
	const xui_code_input_event_t* pEvent, xui_code_input_action_t* pAction,
	void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent == NULL || pAction == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase != XUI_CODE_INPUT_COMMITTED || pEvent->iTextSize != 1 ) return XUI_OK;
	if ( pEvent->sText[0] == '.' ) {
		pAction->iFlags |= XUI_CODE_INPUT_ACTION_SHOW_COMPLETION;
	} else if ( pEvent->sText[0] == '(' || pEvent->sText[0] == ',' ) {
		pAction->iFlags |= XUI_CODE_INPUT_ACTION_SHOW_SIGNATURE;
	} else if ( pEvent->sText[0] == ')' ) {
		pAction->iFlags |= XUI_CODE_INPUT_ACTION_CLOSE_ASSIST;
	}
	return XUI_OK;
}

static void __xuiCodeEditUsage(void)
{
	printf("usage: xui_codeedit [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiCodeEditParseArgs(xui_codeedit_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiCodeEditUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCodeEditFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\consola.ttf",
		"C:\\Windows\\Fonts\\CascadiaMono.ttf",
		"C:\\Windows\\Fonts\\CascadiaCode.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf"
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

static const char* __xuiCodeEditFindFallbackTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\NotoSansSC-VF.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc"
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

static int __xuiCodeEditRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_codeedit_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tHeader;

	(void)iStateId;
	pDemo = (xui_codeedit_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 238, 245, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tHeader = (xui_rect_t){24.0f, 18.0f, tRect.fW - 48.0f, 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tHeader, XUI_COLOR_RGBA(247, 250, 255, 255));
	}
	if ( pDemo->tProxy.drawText != NULL && pDemo->pFont != NULL ) {
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw,
			&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),
				.pFont=pDemo->pFont, .sText="Line number padding",
				.iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT},
			(xui_rect_t){38.0f, 18.0f, 142.0f, 44.0f},
			XUI_COLOR_RGBA(51, 65, 85, 255),
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static void __xuiCodeEditDemoRefreshPaddingButtons(xui_codeedit_demo_t* pDemo)
{
	static const int arrPadding[DEMO_PADDING_BUTTON_COUNT] = {2, 4, 6, 8};
	int i;

	if ( pDemo == NULL ) return;
	for ( i = 0; i < DEMO_PADDING_BUTTON_COUNT; i++ ) {
		if ( pDemo->arrPaddingButtons[i] == NULL ) continue;
		if ( arrPadding[i] == pDemo->iLineNumberPadding ) {
			(void)xuiButtonSetTextColor(pDemo->arrPaddingButtons[i], XUI_COLOR_WHITE);
			(void)xuiButtonSetColors(pDemo->arrPaddingButtons[i],
				XUI_COLOR_RGBA(59, 130, 246, 255),
				XUI_COLOR_RGBA(37, 99, 235, 255),
				XUI_COLOR_RGBA(29, 78, 216, 255),
				XUI_COLOR_RGBA(59, 130, 246, 255),
				XUI_COLOR_RGBA(148, 163, 184, 255));
			(void)xuiButtonSetBorder(pDemo->arrPaddingButtons[i], 1.0f,
				XUI_COLOR_RGBA(37, 99, 235, 255));
		} else {
			(void)xuiButtonSetTextColor(pDemo->arrPaddingButtons[i],
				XUI_COLOR_RGBA(51, 65, 85, 255));
			(void)xuiButtonSetColors(pDemo->arrPaddingButtons[i],
				XUI_COLOR_RGBA(248, 250, 252, 255),
				XUI_COLOR_RGBA(226, 232, 240, 255),
				XUI_COLOR_RGBA(203, 213, 225, 255),
				XUI_COLOR_RGBA(241, 245, 249, 255),
				XUI_COLOR_RGBA(241, 245, 249, 255));
			(void)xuiButtonSetBorder(pDemo->arrPaddingButtons[i], 1.0f,
				XUI_COLOR_RGBA(203, 213, 225, 255));
		}
	}
}

static void __xuiCodeEditDemoPaddingClick(xui_widget pWidget, void* pUser)
{
	static const int arrPadding[DEMO_PADDING_BUTTON_COUNT] = {2, 4, 6, 8};
	xui_codeedit_demo_t* pDemo = (xui_codeedit_demo_t*)pUser;
	int i;

	if ( pDemo == NULL || pDemo->pCodeEdit == NULL ) return;
	for ( i = 0; i < DEMO_PADDING_BUTTON_COUNT; i++ ) {
		if ( pDemo->arrPaddingButtons[i] != pWidget ) continue;
		pDemo->iLineNumberPadding = arrPadding[i];
		(void)xuiCodeEditSetLineNumberLayout(pDemo->pCodeEdit, 1,
			(float)arrPadding[i], XUI_TEXT_ALIGN_RIGHT);
		__xuiCodeEditDemoRefreshPaddingButtons(pDemo);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCodeEdit);
		break;
	}
}

static int __xuiCodeEditDemoCreatePaddingButtons(xui_codeedit_demo_t* pDemo)
{
	static const char* arrLabels[DEMO_PADDING_BUTTON_COUNT] = {"2 px", "4 px", "6 px", "8 px"};
	xui_button_desc_t tDesc;
	int i;
	int iRet;

	for ( i = 0; i < DEMO_PADDING_BUTTON_COUNT; i++ ) {
		memset(&tDesc, 0, sizeof(tDesc));
		tDesc.iSize = sizeof(tDesc);
		tDesc.sText = arrLabels[i];
		tDesc.pFont = pDemo->pFont;
		tDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
		tDesc.fBorderWidth = 1.0f;
		iRet = xuiButtonCreate(pDemo->pContext,
			&pDemo->arrPaddingButtons[i], &tDesc);
		if ( iRet != XUI_OK ) return iRet;
		(void)xuiWidgetSetRect(pDemo->arrPaddingButtons[i],
			(xui_rect_t){184.0f + (float)i * 58.0f, 27.0f, 50.0f, 26.0f});
		(void)xuiWidgetSetFocusable(pDemo->arrPaddingButtons[i], 0);
		iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->arrPaddingButtons[i]);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiButtonSetClick(pDemo->arrPaddingButtons[i],
			__xuiCodeEditDemoPaddingClick, pDemo);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->iLineNumberPadding = 2;
	__xuiCodeEditDemoRefreshPaddingButtons(pDemo);
	return XUI_OK;
}

static int __xuiCodeEditDemoAddDiagnostic(xui_code_diagnostic_t* pDiagnostics,
	int iCount, int iCapacity, const char* sText, const char* sNeedle,
	int iSeverity, const char* sCode, const char* sMessage)
{
	const char* sMatch;
	xui_code_diagnostic_t* pDiagnostic;

	if ( pDiagnostics == NULL || iCount < 0 || iCount >= iCapacity ||
	     sText == NULL || sNeedle == NULL ) return iCount;
	sMatch = strstr(sText, sNeedle);
	if ( sMatch == NULL ) return iCount;
	pDiagnostic = &pDiagnostics[iCount];
	memset(pDiagnostic, 0, sizeof(*pDiagnostic));
	pDiagnostic->iSize = sizeof(*pDiagnostic);
	pDiagnostic->tRange.iStart = (int)(sMatch - sText);
	pDiagnostic->tRange.iEnd = pDiagnostic->tRange.iStart + (int)strlen(sNeedle);
	pDiagnostic->iSeverity = iSeverity;
	pDiagnostic->sCode = sCode;
	pDiagnostic->sMessage = sMessage;
	pDiagnostic->sSource = "xui_codeedit";
	return iCount + 1;
}

static int __xuiCodeEditCreateUi(xui_codeedit_demo_t* pDemo)
{
	xui_code_edit_desc_t tDesc;
	xui_code_fold_range_t arrFolds[16];
	xui_code_diagnostic_t arrDiagnostics[4];
	xui_find_options_t tFind;
	xui_code_document pDocument;
	const char* sText;
	const char* sMatch;
	int iDiagnosticCount;
	int iFoldCount;
	int iFindCount;
	int iMainLine;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCodeEditRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sText = g_sCodeEditDemoText;
	tDesc.sLanguage = "c";
	tDesc.bShowLineNumbers = 1;
	tDesc.bShowFoldMargin = 1;
	tDesc.bShowMarkerMargin = 1;
	tDesc.bShowDiagnosticMargin = 1;
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	iRet = xuiCodeEditCreate(pDemo->pContext, &pDemo->pCodeEdit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pCodeEdit, (xui_rect_t){28.0f, 72.0f, DEMO_W - 56.0f, DEMO_H - 106.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCodeEdit);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditDemoCreatePaddingButtons(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiCodeEditSetLineNumberLayout(pDemo->pCodeEdit, 1, 2.0f,
		XUI_TEXT_ALIGN_RIGHT);
	(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCodeEdit);
	(void)xuiCodeProviderSetCompletion(xuiCodeEditGetProviders(pDemo->pCodeEdit),
		__xuiCodeEditDemoCompletion, pDemo);
	(void)xuiCodeProviderSetSignature(xuiCodeEditGetProviders(pDemo->pCodeEdit),
		__xuiCodeEditDemoSignature, pDemo);
	(void)xuiCodeEditSetInputHandler(pDemo->pCodeEdit,
		__xuiCodeEditDemoInput, pDemo);
	(void)xuiCodeEditSetCompletionOptions(pDemo->pCodeEdit, 1, 1, 64, 0.075f);
	(void)xuiCodeEditSetMinimap(pDemo->pCodeEdit, 1, 92.0f);

	pDocument = xuiCodeEditGetDocument(pDemo->pCodeEdit);
	sText = xuiCodeEditGetText(pDemo->pCodeEdit);
	sMatch = strstr(sText, "hello");
	if ( sMatch != NULL ) {
		(void)xuiCodeDocumentReplace(pDocument,
			(int)(sMatch - sText), (int)(sMatch - sText) + 5, "Hello");
	}
	sText = xuiCodeEditGetText(pDemo->pCodeEdit);
	sMatch = strchr(sText, '\n');
	if ( sMatch != NULL ) {
		(void)xuiCodeDocumentInsert(pDocument, (int)(sMatch - sText) + 1,
			"// Added line: Change margin is enabled by default.\n");
	}
	sText = xuiCodeEditGetText(pDemo->pCodeEdit);
	iMainLine = 0;
	sMatch = strstr(sText, "int main");
	if ( sMatch != NULL ) {
		(void)xuiCodeDocumentOffsetToLineColumn(pDocument,
			(int)(sMatch - sText), &iMainLine, NULL);
	}
	(void)xuiCodeAnnotationSetMarker(xuiCodeEditGetAnnotations(pDemo->pCodeEdit),
		iMainLine, XUI_CODE_MARKER_BOOKMARK, 0u, "main", 1u);
	memset(arrDiagnostics, 0, sizeof(arrDiagnostics));
	iDiagnosticCount = 0;
	iDiagnosticCount = __xuiCodeEditDemoAddDiagnostic(arrDiagnostics,
		iDiagnosticCount, 4, sText, "#include", XUI_CODE_DIAGNOSTIC_INFO,
		"demo.include", "Information diagnostic with the default icon.");
	iDiagnosticCount = __xuiCodeEditDemoAddDiagnostic(arrDiagnostics,
		iDiagnosticCount, 4, sText, "static int add", XUI_CODE_DIAGNOSTIC_HINT,
		"demo.hint", "Hint diagnostic with the default icon.");
	iDiagnosticCount = __xuiCodeEditDemoAddDiagnostic(arrDiagnostics,
		iDiagnosticCount, 4, sText, "printf", XUI_CODE_DIAGNOSTIC_WARNING,
		"demo.printf", "Warning diagnostic: prefer the project logger.");
	iDiagnosticCount = __xuiCodeEditDemoAddDiagnostic(arrDiagnostics,
		iDiagnosticCount, 4, sText, "return 0", XUI_CODE_DIAGNOSTIC_ERROR,
		"demo.return", "Error diagnostic icon demonstration.");
	(void)xuiCodeAnnotationSetDiagnostics(xuiCodeEditGetAnnotations(pDemo->pCodeEdit),
		arrDiagnostics, iDiagnosticCount);
	sMatch = strchr(sText, '\n');
	if ( sMatch != NULL ) {
		int iInlineOffset = (int)(sMatch - sText);
		if ( xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pDemo->pCodeEdit),
			pDocument, iInlineOffset, iInlineOffset) == XUI_OK ) {
			(void)xuiCodeEditSetInlineCompletion(pDemo->pCodeEdit, iInlineOffset,
				" /* Press Tab to accept. */");
		}
	}
	if ( xuiCodeFoldCBuildRanges(sText, -1, arrFolds, 16, &iFoldCount) == XUI_OK && iFoldCount > 0 ) {
		(void)xuiCodeFoldStateSetRanges(xuiCodeEditGetFoldState(pDemo->pCodeEdit), arrFolds, iFoldCount);
	}
	memset(&tFind, 0, sizeof(tFind));
	tFind.iSize = sizeof(tFind);
	tFind.sPattern = "printf";
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE;
	iFindCount = 0;
	(void)xuiCodeEditFindAll(pDemo->pCodeEdit, &tFind, &iFindCount);
	return XUI_OK;
}

static void __xuiCodeEditLayout(xui_codeedit_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H});
	(void)xuiWidgetSetRect(pDemo->pCodeEdit, (xui_rect_t){28.0f, 72.0f, DEMO_W - 56.0f, DEMO_H - 106.0f});
	(void)xuiLayout(pDemo->pContext);
}

static int __xuiCodeEditMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_BACKSPACE: return XUI_KEY_BACKSPACE;
	case XGE_KEY_DELETE: return XUI_KEY_DELETE;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_PAGE_UP: return XUI_KEY_PAGE_UP;
	case XGE_KEY_PAGE_DOWN: return XUI_KEY_PAGE_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_F3: return XUI_KEY_F3;
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	default: return 0;
	}
}

static void __xuiCodeEditRunChecks(xui_codeedit_demo_t* pDemo, int bExerciseInput)
{
	xui_find_options_t tFind;
	xui_rect_t tRect;
	xui_widget pFindWindow;
	float fScrollX;
	float fScrollY;
	int iCount;
	int iStart;
	int iEnd;
	const char* sCompletion;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pCodeEdit != NULL) &&
		(xuiCodeEditGetDocument(pDemo->pCodeEdit) != NULL) &&
		(strcmp(xuiCodeEditGetLanguage(pDemo->pCodeEdit), "c") == 0);
	tRect = xuiWidgetGetRect(pDemo->pCodeEdit);
	pDemo->bLayoutOK = (tRect.fW > 700.0f) && (tRect.fH > 350.0f);
	pDemo->bLexerOK = xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pDemo->pCodeEdit)) > 0;
	pDemo->bMenuOK = xuiCodeEditGetMenuWidget(pDemo->pCodeEdit) != NULL;
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCodeEdit);
		(void)xuiCodeEditSetScroll(pDemo->pCodeEdit, 0.0f, 36.0f);
		(void)xuiInputText(pDemo->pContext, '!');
		(void)xuiInputText(pDemo->pContext, 0x4F60u);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		memset(&tFind, 0, sizeof(tFind));
		tFind.iSize = sizeof(tFind);
		tFind.sPattern = "return";
		tFind.iFlags = XUI_FIND_CASE_SENSITIVE;
		iCount = 0;
		pDemo->bFindOK = (xuiCodeEditFindAll(pDemo->pCodeEdit, &tFind, &iCount) == XUI_OK) && (iCount >= 2);
		pDemo->bFindOK = pDemo->bFindOK && (xuiCodeEditFindNext(pDemo->pCodeEdit, &tFind) == XUI_OK);
		iStart = 0;
		iEnd = 0;
		(void)xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pDemo->pCodeEdit), &iStart, &iEnd);
		pDemo->bFindOK = pDemo->bFindOK && (iEnd > iStart);
		pDemo->bFindOK = pDemo->bFindOK && (xuiCodeEditOpenFind(pDemo->pCodeEdit) == XUI_OK);
		pFindWindow = xuiCodeEditGetFindWindow(pDemo->pCodeEdit);
		pDemo->bFindOK = pDemo->bFindOK && (pFindWindow != NULL) && xuiWindowIsOpen(pFindWindow);
		if ( pFindWindow != NULL ) (void)xuiWindowSetOpen(pFindWindow, 0);
		sCompletion = strstr(xuiCodeEditGetText(pDemo->pCodeEdit), "printf");
		if ( sCompletion != NULL ) {
			iStart = (int)(sCompletion - xuiCodeEditGetText(pDemo->pCodeEdit)) + 2;
			(void)xuiCodeSelectionGotoOffset(xuiCodeEditGetSelection(pDemo->pCodeEdit),
				xuiCodeEditGetDocument(pDemo->pCodeEdit), iStart, 0);
			pDemo->bCompletionOK = xuiCodeEditShowCompletion(pDemo->pCodeEdit) == XUI_OK &&
				xuiCodeEditIsCompletionOpen(pDemo->pCodeEdit) &&
				xuiCodeEditGetCompletionCount(pDemo->pCodeEdit) > 0;
			(void)xuiCodeEditCancelCompletion(pDemo->pCodeEdit);
		}
		pDemo->bExerciseDone = 1;
	}
	(void)xuiCodeEditGetScroll(pDemo->pCodeEdit, &fScrollX, &fScrollY);
	pDemo->bScrollOK = (xuiCodeEditGetScrollModel(pDemo->pCodeEdit) != NULL) &&
		(xuiCodeEditGetHScrollBarWidget(pDemo->pCodeEdit) != NULL) &&
		(xuiCodeEditGetVScrollBarWidget(pDemo->pCodeEdit) != NULL) &&
		(fScrollY >= 0.0f);
	pDemo->bInputOK = !bExerciseInput ||
		(strchr(xuiCodeEditGetText(pDemo->pCodeEdit), '!') != NULL &&
		 strstr(xuiCodeEditGetText(pDemo->pCodeEdit), "\xE4\xBD\xA0") != NULL);
	pDemo->bMinimapOK =
		(xuiCodeEditGetDisplayOptions(pDemo->pCodeEdit) & XUI_CODE_EDIT_SHOW_MINIMAP) != 0 &&
		xuiCodeEditGetMinimapRect(pDemo->pCodeEdit).fW > 0.0f;
	if ( !bExerciseInput ) {
		pDemo->bFindOK = xuiCodeEditGetFindResultCount(pDemo->pCodeEdit) > 0;
		pDemo->bCompletionOK = 1;
	}
}

static int __xuiCodeEditCreateAssets(xui_codeedit_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	const char* sFallbackPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_W, (float)DEMO_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_W;
	tSurfaceDesc.iHeight = DEMO_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiCodeEditFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	sFallbackPath = __xuiCodeEditFindFallbackTtf();
	if ( sFallbackPath != NULL ) {
		(void)xgeFontFallbackSet(sFallbackPath, 14.0f);
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	xgeFontFallbackClear();
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiCodeEditCreateUi(pDemo);
}

static void __xuiCodeEditDestroyAssets(xui_codeedit_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
}

static int __xuiCodeEditFrame(void* pUser)
{
	xui_codeedit_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int bAutoRun;
	int iRet;

	pDemo = (xui_codeedit_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( pDemo->pContext == NULL ) {
		iRet = __xuiCodeEditCreateAssets(pDemo);
		if ( iRet != XUI_OK ) {
			printf("xui_codeedit: create assets failed: %d\n", iRet);
			return iRet;
		}
	}
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiProxyXgePumpInputRect(pDemo->pContext,
		(xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_W, (float)DEMO_H});
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCodeEditLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCodeEditRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_W, DEMO_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bRenderOK = 1;
	xgeClear(XUI_COLOR_RGBA(20, 24, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_W, (float)DEMO_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_codeedit final-summary frames=%d create=%d layout=%d render=%d lexer=%d input=%d menu=%d scroll=%d find=%d completion=%d minimap=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bRenderOK, pDemo->bLexerOK, pDemo->bInputOK, pDemo->bMenuOK, pDemo->bScrollOK, pDemo->bFindOK, pDemo->bCompletionOK, pDemo->bMinimapOK,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_codeedit_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;
	int bAutoRun;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCodeEditParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCodeEditUsage();
		return 1;
	}
	bAutoRun = (tDemo.iMaxFrames > 0) || (tDemo.fMaxSeconds > 0.0);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W + 20;
	tDesc.iHeight = DEMO_H + 50;
	tDesc.sTitle = "XUI CodeEdit";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_codeedit: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = xgeRun(__xuiCodeEditFrame, &tDemo);
	__xuiCodeEditDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bRenderOK &&
		tDemo.bLexerOK && tDemo.bMenuOK && tDemo.bScrollOK && tDemo.bFindOK &&
		tDemo.bCompletionOK && tDemo.bMinimapOK && (!bAutoRun || tDemo.bInputOK)) ? 0 : 1;
}
