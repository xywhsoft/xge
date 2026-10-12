#include "xui_document_ui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_edit_contract_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_edit_contract_state_t {
	xui_widget pLastWidget;
	int iTextChanged;
	int iSelectionChanged;
	int iValidationChanged;
	int bLastValid;
} xui_edit_contract_state_t;

static int __xuiEditContractEvent(xui_widget pWidget, const xui_edit_event_t* pEvent, void* pUser)
{
	xui_edit_contract_state_t* pState = (xui_edit_contract_state_t*)pUser;
	if ( pState == NULL || pEvent == NULL || pEvent->iSize < sizeof(*pEvent) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState->pLastWidget = pWidget;
	if ( pEvent->iType == XUI_EDIT_EVENT_TEXT_CHANGED ) pState->iTextChanged++;
	if ( pEvent->iType == XUI_EDIT_EVENT_SELECTION_CHANGED ) pState->iSelectionChanged++;
	if ( pEvent->iType == XUI_EDIT_EVENT_VALIDATION_CHANGED ) {
		pState->iValidationChanged++;
		pState->bLastValid = pEvent->bValid;
	}
	return XUI_OK;
}

static void __xuiEditContractDestroy(xui_widget* ppWidget)
{
	if ( ppWidget != NULL && *ppWidget != NULL ) {
		xuiWidgetDestroy(*ppWidget);
		*ppWidget = NULL;
	}
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_edit_contract_state_t tEventState;
	xui_context pContext = NULL;
	xui_font pFont = NULL;
	xui_widget pWidget = NULL;
	xui_document pDocument = NULL;
	xui_input_desc_t tInput;
	xui_text_edit_desc_t tTextEdit;
	xui_code_edit_desc_t tCodeEdit;
	xui_doc_editor_desc_t tDocumentEditor;
	xui_doc_desc_t tMarkdown;
	xui_document_snapshot pSnapshot = NULL;
	xui_doc_command_state_t tCommand;
	xui_doc_range_t tMatch;
	char sSource[64];
	uint64_t iSourceBytes;
	xui_terminal_desc_t tTerminal;
	xui_numeric_input_desc_t tNumeric;
	xui_tag_input_desc_t tTag;
	xui_combobox_desc_t tCombo;
	xui_edit_behavior_t tBehavior;
	const char* arrItems[] = {"one", "two"};
	uint32_t iCaps;
	int iStart;
	int iEnd;
	int iFailed = 0;
	int iRet;

	memset(&tEventState, 0, sizeof(tEventState));
	xuiTestProxyInit(&tProxyState);
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxyState.tProxy.fontLoadFile(&tProxyState.tProxy, &pFont, "test.ttf", 14.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	XUI_TEST_CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK, "default font");

	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.sText = "abc";
	tInput.pFont = pFont;
	XUI_TEST_CHECK(xuiInputCreate(pContext, &pWidget, &tInput) == XUI_OK, "input create");
	iCaps = xuiEditGetCapabilities(pWidget);
	XUI_TEST_CHECK((iCaps & (XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_CLIPBOARD |
		XUI_EDIT_CAP_UNDO | XUI_EDIT_CAP_READONLY | XUI_EDIT_CAP_IME | XUI_EDIT_CAP_FORM_VALUE)) ==
		(XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_CLIPBOARD |
		 XUI_EDIT_CAP_UNDO | XUI_EDIT_CAP_READONLY | XUI_EDIT_CAP_IME | XUI_EDIT_CAP_FORM_VALUE),
		"input capabilities");
	XUI_TEST_CHECK(xuiEditSetEvent(pWidget, __xuiEditContractEvent, &tEventState) == XUI_OK, "input event");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "hello") == XUI_OK && strcmp(xuiEditGetText(pWidget), "hello") == 0,
		"input generic text");
	XUI_TEST_CHECK(tEventState.iTextChanged == 1 && tEventState.pLastWidget == pWidget, "input text event");
	XUI_TEST_CHECK(xuiEditSetSelection(pWidget, 1, 4) == XUI_OK, "input selection set");
	XUI_TEST_CHECK(xuiEditGetSelection(pWidget, &iStart, &iEnd) == XUI_OK && iStart == 1 && iEnd == 4,
		"input selection get");
	XUI_TEST_CHECK(tEventState.iSelectionChanged == 1, "input selection event");
	memset(&tBehavior, 0, sizeof(tBehavior));
	tBehavior.iSize = sizeof(tBehavior);
	tBehavior.iTabBehavior = XUI_EDIT_TAB_INSERT;
	tBehavior.iEnterBehavior = XUI_EDIT_ENTER_COMMIT;
	tBehavior.iEscapeBehavior = XUI_EDIT_ESCAPE_REVERT;
	tBehavior.bSelectAllOnFocus = 1;
	tBehavior.bCommitOnBlur = 1;
	XUI_TEST_CHECK(xuiEditSetBehavior(pWidget, &tBehavior) == XUI_OK, "input behavior set");
	memset(&tBehavior, 0, sizeof(tBehavior));
	tBehavior.iSize = sizeof(tBehavior);
	XUI_TEST_CHECK(xuiEditGetBehavior(pWidget, &tBehavior) == XUI_OK &&
		tBehavior.iTabBehavior == XUI_EDIT_TAB_INSERT && tBehavior.bCommitOnBlur, "input behavior get");
	XUI_TEST_CHECK(xuiInputSetError(pWidget, 1) == XUI_OK && tEventState.iValidationChanged == 1 &&
		!tEventState.bLastValid, "input validation event");
	XUI_TEST_CHECK(xuiEditSetReadonly(pWidget, 1) == XUI_OK && xuiEditIsReadonly(pWidget), "input readonly");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "programmatic") == XUI_OK, "readonly programmatic text");
	__xuiEditContractDestroy(&pWidget);

	memset(&tTextEdit, 0, sizeof(tTextEdit));
	tTextEdit.iSize = sizeof(tTextEdit);
	tTextEdit.sText = "line one\nline two";
	tTextEdit.pFont = pFont;
	XUI_TEST_CHECK(xuiTextEditCreate(pContext, &pWidget, &tTextEdit) == XUI_OK, "text edit create");
	XUI_TEST_CHECK((xuiEditGetCapabilities(pWidget) & XUI_EDIT_CAP_MULTILINE) != 0u, "text edit multiline");
	XUI_TEST_CHECK(xuiEditSetSelection(pWidget, 0, 4) == XUI_OK && xuiEditHasSelection(pWidget), "text edit selection");
	__xuiEditContractDestroy(&pWidget);

	memset(&tCodeEdit, 0, sizeof(tCodeEdit));
	tCodeEdit.iSize = sizeof(tCodeEdit);
	tCodeEdit.sText = "int main(void) {}";
	tCodeEdit.pFont = pFont;
	XUI_TEST_CHECK(xuiCodeEditCreate(pContext, &pWidget, &tCodeEdit) == XUI_OK, "code edit create");
	XUI_TEST_CHECK((xuiEditGetCapabilities(pWidget) & (XUI_EDIT_CAP_CODE | XUI_EDIT_CAP_MULTILINE)) ==
		(XUI_EDIT_CAP_CODE | XUI_EDIT_CAP_MULTILINE), "code edit capabilities");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "return 0;") == XUI_OK && strcmp(xuiEditGetText(pWidget), "return 0;") == 0,
		"code edit generic text");
	__xuiEditContractDestroy(&pWidget);

	XUI_TEST_CHECK(xuiDocumentCreate(NULL, &pDocument) == XUI_OK, "document create");
	memset(&tDocumentEditor, 0, sizeof(tDocumentEditor));
	tDocumentEditor.iSize = sizeof(tDocumentEditor);
	tDocumentEditor.tView.iSize = sizeof(tDocumentEditor.tView);
	tDocumentEditor.tView.pDocument = pDocument;
	tDocumentEditor.tView.tRenderer.iSize = sizeof(tDocumentEditor.tView.tRenderer);
	tDocumentEditor.tView.tRenderer.tFonts.normal = pFont;
	XUI_TEST_CHECK(xuiDocumentEditorCreate(pContext, &tDocumentEditor, &pWidget) == XUI_OK,
		"document editor create");
	XUI_TEST_CHECK(xuiDocumentEditorInsertText(pWidget, "rich", 4) == XUI_OK,
		"document editor insert");
	XUI_TEST_CHECK((xuiEditGetCapabilities(pWidget) &
		(XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_UNDO |
		 XUI_EDIT_CAP_IME | XUI_EDIT_CAP_MULTILINE | XUI_EDIT_CAP_STRUCTURED)) ==
		(XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_UNDO |
		 XUI_EDIT_CAP_IME | XUI_EDIT_CAP_MULTILINE | XUI_EDIT_CAP_STRUCTURED),
		"document editor capabilities");
	XUI_TEST_CHECK(xuiEditGetText(pWidget) != NULL && !strcmp(xuiEditGetText(pWidget), "rich\n"),
		"document editor generic text");
	memset(&tCommand, 0, sizeof(tCommand)); tCommand.iSize = sizeof(tCommand);
	XUI_TEST_CHECK(xuiDocumentEditorQueryCommand(pWidget, XUI_DOC_EDIT_BOLD, &tCommand) == XUI_OK &&
		tCommand.bEnabled, "document editor rich command");
	XUI_TEST_CHECK(xuiDocumentViewFind(pWidget, "rich", 4, 0, 1, &tMatch) == XUI_OK,
		"document editor find");
	XUI_TEST_CHECK(xuiEditSetReadonly(pWidget, 1) == XUI_OK && xuiEditIsReadonly(pWidget),
		"document editor readonly");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "blocked") == XUI_ERROR_INVALID_STATE &&
		!strcmp(xuiEditGetText(pWidget), "rich\n"), "readonly document editor blocks writes");
	XUI_TEST_CHECK(xuiEditSetReadonly(pWidget, 0) == XUI_OK, "document editor writable");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "updated") == XUI_OK &&
		!strcmp(xuiEditGetText(pWidget), "updated\n"), "document editor generic text replacement");
	XUI_TEST_CHECK(xuiEditCanUndo(pWidget) && xuiEditUndo(pWidget) == XUI_OK &&
		!strcmp(xuiEditGetText(pWidget), "rich\n"), "document editor generic undo");
	__xuiEditContractDestroy(&pWidget);
	xuiDocumentRelease(pDocument); pDocument = NULL;

	memset(&tMarkdown, 0, sizeof(tMarkdown));
	tMarkdown.iSize = sizeof(tMarkdown); tMarkdown.iProfile = XUI_DOCUMENT_MARKDOWN;
	XUI_TEST_CHECK(xuiDocumentCreate(&tMarkdown, &pDocument) == XUI_OK &&
		xuiDocumentLoadMarkdown(pDocument, "# old\n", 6) == XUI_OK, "markdown document create");
	tDocumentEditor.tView.pDocument = pDocument;
	tDocumentEditor.iMode = XUI_DOC_SOURCE_TEXT;
	XUI_TEST_CHECK(xuiDocumentEditorCreate(pContext, &tDocumentEditor, &pWidget) == XUI_OK,
		"markdown source editor create");
	XUI_TEST_CHECK(xuiEditGetText(pWidget) != NULL && !strcmp(xuiEditGetText(pWidget), "# old\n"),
		"markdown generic source text");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "# next\n") == XUI_OK,
		"markdown generic source replacement");
	XUI_TEST_CHECK(xuiDocumentAcquireSnapshot(pDocument, &pSnapshot) == XUI_OK &&
		xuiDocumentSnapshotCopySource(pSnapshot, sSource, sizeof(sSource), &iSourceBytes) == XUI_OK &&
		!strcmp(sSource, "# next\n"), "markdown source persisted");
	xuiDocumentSnapshotRelease(pSnapshot); pSnapshot = NULL;
	XUI_TEST_CHECK(xuiDocumentViewSetMode(pWidget, XUI_DOC_VISUAL) == XUI_OK &&
		xuiDocumentViewFind(pWidget, "next", 4, 0, 1, &tMatch) == XUI_OK,
		"markdown visual projection and find");
	XUI_TEST_CHECK(xuiEditCanUndo(pWidget) && xuiEditUndo(pWidget) == XUI_OK,
		"markdown shared history after mode switch");
	XUI_TEST_CHECK(xuiDocumentAcquireSnapshot(pDocument, &pSnapshot) == XUI_OK &&
		xuiDocumentSnapshotCopySource(pSnapshot, sSource, sizeof(sSource), &iSourceBytes) == XUI_OK &&
		!strcmp(sSource, "# old\n"), "markdown source restored");
	xuiDocumentSnapshotRelease(pSnapshot); pSnapshot = NULL;
	__xuiEditContractDestroy(&pWidget);
	xuiDocumentRelease(pDocument); pDocument = NULL;

	memset(&tTerminal, 0, sizeof(tTerminal));
	tTerminal.iSize = sizeof(tTerminal);
	tTerminal.pFont = pFont;
	tTerminal.iColumns = 20;
	tTerminal.iRows = 4;
	XUI_TEST_CHECK(xuiTerminalCreate(pContext, &pWidget, &tTerminal) == XUI_OK, "terminal create");
	iCaps = xuiEditGetCapabilities(pWidget);
	XUI_TEST_CHECK((iCaps & XUI_EDIT_CAP_TERMINAL) != 0u && (iCaps & XUI_EDIT_CAP_TEXT) == 0u,
		"terminal capability boundary");
	XUI_TEST_CHECK(xuiEditSetSelection(pWidget, 0, 1) == XUI_OK && xuiEditHasSelection(pWidget),
		"terminal selection adapter");
	__xuiEditContractDestroy(&pWidget);

	memset(&tNumeric, 0, sizeof(tNumeric));
	tNumeric.iSize = sizeof(tNumeric);
	tNumeric.pFont = pFont;
	tNumeric.fMin = 0.0f;
	tNumeric.fMax = 100.0f;
	tNumeric.fStep = 1.0f;
	tNumeric.fValue = 10.0f;
	XUI_TEST_CHECK(xuiNumericInputCreate(pContext, &pWidget, &tNumeric) == XUI_OK, "numeric input create");
	XUI_TEST_CHECK((xuiEditGetCapabilities(pWidget) & XUI_EDIT_CAP_FORM_VALUE) != 0u, "numeric delegate capability");
	XUI_TEST_CHECK(xuiEditSetReadonly(pWidget, 1) == XUI_OK && xuiEditIsReadonly(pWidget), "numeric delegate readonly");
	__xuiEditContractDestroy(&pWidget);

	memset(&tTag, 0, sizeof(tTag));
	tTag.iSize = sizeof(tTag);
	tTag.sText = "draft";
	tTag.pFont = pFont;
	XUI_TEST_CHECK(xuiTagInputCreate(pContext, &pWidget, &tTag) == XUI_OK, "tag input create");
	memset(&tEventState, 0, sizeof(tEventState));
	XUI_TEST_CHECK(xuiEditSetEvent(pWidget, __xuiEditContractEvent, &tEventState) == XUI_OK, "tag event");
	XUI_TEST_CHECK(xuiEditSetText(pWidget, "tag text") == XUI_OK && tEventState.pLastWidget == pWidget,
		"tag delegate event host");
	__xuiEditContractDestroy(&pWidget);

	memset(&tCombo, 0, sizeof(tCombo));
	tCombo.iSize = sizeof(tCombo);
	tCombo.arrItems = arrItems;
	tCombo.iItemCount = 2;
	tCombo.iSelected = 0;
	tCombo.iMode = XUI_COMBOBOX_MODE_EDIT;
	tCombo.pFont = pFont;
	XUI_TEST_CHECK(xuiComboBoxCreate(pContext, &pWidget, &tCombo) == XUI_OK, "editable combo create");
	XUI_TEST_CHECK((xuiEditGetCapabilities(pWidget) & XUI_EDIT_CAP_TEXT) != 0u, "editable combo delegate");
	__xuiEditContractDestroy(&pWidget);

	tCombo.iMode = XUI_COMBOBOX_MODE_SELECT;
	XUI_TEST_CHECK(xuiComboBoxCreate(pContext, &pWidget, &tCombo) == XUI_OK, "select combo create");
	XUI_TEST_CHECK(xuiEditGetCapabilities(pWidget) == 0u, "select combo is not editor");

cleanup:
	__xuiEditContractDestroy(&pWidget);
	if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
	if ( pDocument != NULL ) xuiDocumentRelease(pDocument);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( pFont != NULL && tProxyState.tProxy.fontDestroy != NULL ) {
		tProxyState.tProxy.fontDestroy(&tProxyState.tProxy, pFont);
	}
	if ( !iFailed ) printf("xui_edit_contract_test passed\n");
	return iFailed ? 1 : 0;
}
