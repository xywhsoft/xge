#include "tools/UIDesign/src/ui_design_app.h"
#include "tools/UIDesign/src/ui_design_codegen.h"
#include "tools/UIDesign/src/ui_design_document.h"
#include "tools/UIDesign/src/ui_design_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_CHECK(expr, message) do { \
	if ( !(expr) ) { \
		fprintf(stderr, "uidesign-test failed: %s (line %d)\n", message, __LINE__); \
		return 1; \
	} \
} while ( 0 )

static int __testWriteFile(const char* sPath, const char* sText)
{
	FILE* pFile;
	size_t iLength;

	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) return 0;
	iLength = strlen(sText);
	if ( fwrite(sText, 1u, iLength, pFile) != iLength ) {
		fclose(pFile);
		return 0;
	}
	return fclose(pFile) == 0;
}

static char* __testReadFile(const char* sPath)
{
	FILE* pFile;
	char* sText;
	long iSize;

	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) return NULL;
	if ( fseek(pFile, 0, SEEK_END) != 0 ) { fclose(pFile); return NULL; }
	iSize = ftell(pFile);
	if ( iSize < 0 || fseek(pFile, 0, SEEK_SET) != 0 ) { fclose(pFile); return NULL; }
	sText = (char*)malloc((size_t)iSize + 1u);
	if ( sText == NULL ) { fclose(pFile); return NULL; }
	if ( fread(sText, 1u, (size_t)iSize, pFile) != (size_t)iSize ) { free(sText); fclose(pFile); return NULL; }
	sText[iSize] = 0;
	fclose(pFile);
	return sText;
}

static int __testModelsEqual(const ui_design_model_t* pA, const ui_design_model_t* pB)
{
	const ui_design_node_t* pNodeA;
	const ui_design_node_t* pNodeB;
	int i;
	int j;

	if ( pA->iNodeCount != pB->iNodeCount || pA->iNextId != pB->iNextId ||
	     pA->iSelectedId != pB->iSelectedId || pA->iSelectedCount != pB->iSelectedCount ) return 0;
	for ( i = 0; i < pA->iSelectedCount; ++i ) if ( pA->arrSelectedIds[i] != pB->arrSelectedIds[i] ) return 0;
	for ( i = 0; i < pA->iNodeCount; ++i ) {
		pNodeA = &pA->arrNodes[i];
		pNodeB = &pB->arrNodes[i];
		if ( pNodeA->iId != pNodeB->iId || pNodeA->iType != pNodeB->iType || pNodeA->iParentId != pNodeB->iParentId ||
		     memcmp(&pNodeA->tRect, &pNodeB->tRect, sizeof(pNodeA->tRect)) != 0 || strcmp(pNodeA->sText, pNodeB->sText) != 0 ||
		     pNodeA->bChecked != pNodeB->bChecked || pNodeA->bVisible != pNodeB->bVisible || pNodeA->bEnabled != pNodeB->bEnabled ||
		     pNodeA->iPropertyCount != pNodeB->iPropertyCount ) return 0;
		for ( j = 0; j < pNodeA->iPropertyCount; ++j ) {
			if ( strcmp(pNodeA->arrProperties[j].sId, pNodeB->arrProperties[j].sId) != 0 ||
			     strcmp(pNodeA->arrProperties[j].sValue, pNodeB->arrProperties[j].sValue) != 0 ) return 0;
		}
	}
	return 1;
}

static int __testReplaceOnce(char* sText, const char* sNeedle, const char* sReplacement)
{
	char* pFound;
	size_t iNeedle;
	size_t iReplacement;

	pFound = strstr(sText, sNeedle);
	if ( pFound == NULL ) return 0;
	iNeedle = strlen(sNeedle);
	iReplacement = strlen(sReplacement);
	if ( iNeedle != iReplacement ) return 0;
	memcpy(pFound, sReplacement, iReplacement);
	return 1;
}

static int __testInvalidDocuments(const char* sSnapshot)
{
	static const char* sOverflowId =
		"{\"format\":\"xui.uidesign\",\"version\":1,\"model\":{\"nextId\":2,\"selectedId\":0,\"selection\":[],\"nodes\":["
		"{\"id\":4294967297,\"type\":\"widget\",\"typeId\":1,\"parentId\":0,\"rect\":{\"x\":0,\"y\":0,\"w\":10,\"h\":10},"
		"\"text\":\"\",\"checked\":false,\"visible\":true,\"enabled\":true,\"properties\":[]}]}}";
	ui_design_model_t* pLoaded;
	char* sInvalid;
	int iRet;

	sInvalid = (char*)malloc(strlen(sSnapshot) + 1u);
	TEST_CHECK(sInvalid != NULL, "allocate invalid snapshot");
	strcpy(sInvalid, sSnapshot);
	TEST_CHECK(__testReplaceOnce(sInvalid, "\"id\": 2,", "\"id\": 1,"), "find second id");
	pLoaded = NULL;
	iRet = uiDesignDocumentLoadModel(sInvalid, &pLoaded);
	TEST_CHECK(iRet != XUI_OK && pLoaded == NULL, "reject duplicate node id");
	strcpy(sInvalid, sSnapshot);
	TEST_CHECK(__testReplaceOnce(sInvalid, "\"parentId\": 0,", "\"parentId\": 1,"), "find root parent");
	pLoaded = NULL;
	iRet = uiDesignDocumentLoadModel(sInvalid, &pLoaded);
	TEST_CHECK(iRet != XUI_OK && pLoaded == NULL, "reject parent cycle");
	strcpy(sInvalid, sSnapshot);
	TEST_CHECK(__testReplaceOnce(sInvalid, "\"type\": \"widget\"", "\"type\": \"future\""), "find symbolic type");
	pLoaded = NULL;
	iRet = uiDesignDocumentLoadModel(sInvalid, &pLoaded);
	TEST_CHECK(iRet != XUI_OK && pLoaded == NULL, "symbolic type wins over typeId");
	pLoaded = NULL;
	iRet = uiDesignDocumentLoadModel(sOverflowId, &pLoaded);
	TEST_CHECK(iRet != XUI_OK && pLoaded == NULL, "reject overflowing numeric fields");
	free(sInvalid);
	return 0;
}

static int __testBuildModel(ui_design_model_t* pModel)
{
	ui_design_node_t* pButton;
	ui_design_property_value_t* pStableProperties;
	char sId[48];
	char sValue[48];
	int iWidget;
	int iPanel;
	int iLabel;
	int iButton;
	int iInput;
	int iCheckBox;
	int iImage;
	int iTabs;
	int i;

	uiDesignModelInit(pModel);
	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_WIDGET, 0, 11.0f, 12.0f, &iWidget) == XUI_OK, "add widget");
	TEST_CHECK(uiDesignModelSetRect(pModel, iWidget, (xui_rect_t){11, 12, 760, 520}) == XUI_OK, "widget rect");
	TEST_CHECK(uiDesignModelSetText(pModel, iWidget, "root_widget") == XUI_OK, "widget text");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iWidget, "layout.type", "0") == XUI_OK, "widget layout");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_PANEL, iWidget, 20.0f, 24.0f, &iPanel) == XUI_OK, "add panel");
	TEST_CHECK(uiDesignModelSetRect(pModel, iPanel, (xui_rect_t){20, 24, 350, 220}) == XUI_OK, "panel rect");
	TEST_CHECK(uiDesignModelSetText(pModel, iPanel, "Settings \"A\"") == XUI_OK, "panel text");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iPanel, "appearance.clientColor", "#F8FAFC") == XUI_OK, "panel color");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_LABEL, iPanel, 16.0f, 42.0f, &iLabel) == XUI_OK, "add label");
	TEST_CHECK(uiDesignModelSetRect(pModel, iLabel, (xui_rect_t){16, 42, 210, 28}) == XUI_OK, "label rect");
	TEST_CHECK(uiDesignModelSetText(pModel, iLabel, "Hello, XUI!\nRound-trip") == XUI_OK, "label text");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_BUTTON, iPanel, 16.0f, 82.0f, &iButton) == XUI_OK, "add button");
	TEST_CHECK(uiDesignModelSetRect(pModel, iButton, (xui_rect_t){16, 82, 128, 34}) == XUI_OK, "button rect");
	TEST_CHECK(uiDesignModelSetText(pModel, iButton, "Apply") == XUI_OK, "button text");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iButton, "event.onClick", "settings.apply") == XUI_OK, "button event");
	pButton = uiDesignModelGetNode(pModel, iButton);
	TEST_CHECK(pButton != NULL && pButton->pRuntime == NULL, "runtime starts lazy");
	pStableProperties = pButton->arrProperties;
	for ( i = 0; i < 48; ++i ) {
		snprintf(sId, sizeof(sId), "test.property.%02d", i);
		snprintf(sValue, sizeof(sValue), "value-%02d", i);
		TEST_CHECK(uiDesignModelSetProperty(pModel, iButton, sId, sValue) == XUI_OK, "append stable property");
	}
	TEST_CHECK(pButton->arrProperties == pStableProperties, "property storage address remains stable");
	TEST_CHECK(uiDesignNodeEnsureRuntime(pButton) == XUI_OK && pButton->pRuntime != NULL, "lazy runtime allocation");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_INPUT, iPanel, 16.0f, 130.0f, &iInput) == XUI_OK, "add input");
	TEST_CHECK(uiDesignModelSetText(pModel, iInput, "guest") == XUI_OK, "input text");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iInput, "text.placeholder", "User name") == XUI_OK, "input placeholder");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_CHECKBOX, iPanel, 16.0f, 174.0f, &iCheckBox) == XUI_OK, "add checkbox");
	TEST_CHECK(uiDesignModelSetText(pModel, iCheckBox, "Remember me") == XUI_OK, "checkbox text");
	TEST_CHECK(uiDesignModelSetChecked(pModel, iCheckBox, 1) == XUI_OK, "checkbox state");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_TABS, iWidget, 390.0f, 24.0f, &iTabs) == XUI_OK, "add tabs");
	TEST_CHECK(uiDesignModelSetRect(pModel, iTabs, (xui_rect_t){390, 24, 330, 260}) == XUI_OK, "tabs rect");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iTabs, "data.items", "General|true|false|||||\nPreview|true|true|||||") == XUI_OK, "tabs data");

	TEST_CHECK(uiDesignModelAddNode(pModel, UI_DESIGN_NODE_IMAGE, iTabs, 12.0f, 18.0f, &iImage) == XUI_OK, "add image");
	TEST_CHECK(uiDesignModelSetRect(pModel, iImage, (xui_rect_t){12, 18, 180, 120}) == XUI_OK, "image rect");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iImage, "layout.tabPage", "1") == XUI_OK, "image page");
	TEST_CHECK(uiDesignModelSetProperty(pModel, iImage, "data.source", "xui_image_file.png") == XUI_OK, "image source");

	TEST_CHECK(uiDesignModelSetSelected(pModel, iButton) == XUI_OK, "primary selection");
	TEST_CHECK(uiDesignModelAddSelection(pModel, iImage) == XUI_OK, "multi selection");
	return 0;
}

static int __testDocumentSizeLimit(void)
{
	ui_design_model_t tModel;
	char sId[32];
	char sValue[UI_DESIGN_PROPERTY_VALUE_CAPACITY];
	char* sSnapshot;
	int iNode;
	int i;
	int j;

	uiDesignModelInit(&tModel);
	memset(sValue, 'x', sizeof(sValue) - 1u);
	sValue[sizeof(sValue) - 1u] = 0;
	for ( i = 0; i < 128; ++i ) {
		TEST_CHECK(uiDesignModelAddNode(&tModel, UI_DESIGN_NODE_WIDGET, 0, 0.0f, 0.0f, &iNode) == XUI_OK, "add size-limit node");
		for ( j = 0; j < UI_DESIGN_MAX_NODE_PROPERTIES; ++j ) {
			snprintf(sId, sizeof(sId), "property.%03d", j);
			TEST_CHECK(uiDesignModelSetProperty(&tModel, iNode, sId, sValue) == XUI_OK, "fill size-limit property");
		}
	}
	sSnapshot = NULL;
	TEST_CHECK(uiDesignDocumentSaveModel(&tModel, &sSnapshot) == XUI_ERROR_BUFFER_TOO_SMALL && sSnapshot == NULL,
		"reject document larger than load limit");
	uiDesignModelDestroy(&tModel);
	return 0;
}

int main(int argc, char** argv)
{
	ui_design_model_t tModel;
	ui_design_model_t tUnsupported;
	ui_design_model_t* pLoaded;
	ui_design_node_t* pLoadedButton;
	char* sSnapshot;
	char* sRoundTrip;
	char* sGenerated;
	char sHeaderPath[1024];
	int iRadio;
	int iRet;

	TEST_CHECK(argc == 3, "expected fixture and generated C paths");
	TEST_CHECK(sizeof(ui_design_node_t) <= 256u, "node metadata size budget");
	TEST_CHECK(sizeof(ui_design_model_t) <= 100000u, "empty model size budget");
	TEST_CHECK(sizeof(ui_design_app_t) <= 1024u * 1024u, "empty app size budget");
	TEST_CHECK(__testDocumentSizeLimit() == 0, "document size limit");
	TEST_CHECK(__testBuildModel(&tModel) == 0, "build model");
	sSnapshot = NULL;
	TEST_CHECK(uiDesignDocumentSaveModel(&tModel, &sSnapshot) == XUI_OK && sSnapshot != NULL, "serialize model");
	TEST_CHECK(strstr(sSnapshot, "\"rect\": {\"x\": 11, \"y\": 12, \"w\": 760, \"h\": 520}") != NULL, "integer rect serialization");
	pLoaded = NULL;
	TEST_CHECK(uiDesignDocumentLoadModel(sSnapshot, &pLoaded) == XUI_OK && pLoaded != NULL, "load model");
	TEST_CHECK(__testModelsEqual(&tModel, pLoaded), "round-trip preserves persistent model");
	pLoadedButton = uiDesignModelGetNode(pLoaded, 4);
	TEST_CHECK(pLoadedButton != NULL && pLoadedButton->pRuntime == NULL, "loaded runtime remains lazy");
	sRoundTrip = NULL;
	TEST_CHECK(uiDesignDocumentSaveModel(pLoaded, &sRoundTrip) == XUI_OK, "re-serialize model");
	TEST_CHECK(strcmp(sSnapshot, sRoundTrip) == 0, "save-open-save is stable");
	TEST_CHECK(__testInvalidDocuments(sSnapshot) == 0, "invalid document checks");
	TEST_CHECK(__testWriteFile(argv[1], sSnapshot), "write preview fixture");
	iRet = uiDesignCodegenExport(pLoaded, argv[2], "milestone_view", sHeaderPath, sizeof(sHeaderPath));
	TEST_CHECK(iRet == XUI_OK, "export supported controls");
	TEST_CHECK(uiDesignCodegenExport(pLoaded, sHeaderPath, "milestone_view", NULL, 0) == XUI_ERROR_INVALID_ARGUMENT,
	           "reject header path as source output");
	sGenerated = __testReadFile(argv[2]);
	TEST_CHECK(sGenerated != NULL, "read generated source");
	TEST_CHECK(strstr(sGenerated, "xuiWidgetCreate") != NULL && strstr(sGenerated, "xuiPanelCreate") != NULL &&
	           strstr(sGenerated, "xuiLabelCreate") != NULL && strstr(sGenerated, "xuiButtonCreate") != NULL &&
	           strstr(sGenerated, "xuiInputCreate") != NULL && strstr(sGenerated, "xuiCheckBoxCreate") != NULL &&
	           strstr(sGenerated, "xuiImageCreate") != NULL && strstr(sGenerated, "xuiTabsCreate") != NULL,
	           "generated source covers eight controls");
	TEST_CHECK(strstr(sGenerated, "settings.apply") != NULL && strstr(sGenerated, "xuiButtonSetClick") != NULL, "generated button action");
	free(sGenerated);

	uiDesignModelInit(&tUnsupported);
	TEST_CHECK(uiDesignModelAddNode(&tUnsupported, UI_DESIGN_NODE_RADIO, 0, 0.0f, 0.0f, &iRadio) == XUI_OK, "add unsupported node");
	TEST_CHECK(uiDesignCodegenExport(&tUnsupported, argv[2], "unsupported", NULL, 0) == XUI_ERROR_UNSUPPORTED, "reject partial export");
	uiDesignModelDestroy(&tUnsupported);

	printf("uidesign-test passed: node=%zu model=%zu app=%zu runtime=%zu snapshot=%zu\n",
		sizeof(ui_design_node_t), sizeof(ui_design_model_t), sizeof(ui_design_app_t), sizeof(ui_design_node_runtime_t), strlen(sSnapshot));
	free(sRoundTrip);
	free(sSnapshot);
	uiDesignModelDestroy(pLoaded);
	free(pLoaded);
	uiDesignModelDestroy(&tModel);
	return 0;
}
