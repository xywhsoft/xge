#include "ui_design_document.h"
#include "src/xui_xrt_port.h"

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ui_design_text_builder_t {
	char* sData;
	size_t iLength;
	size_t iCapacity;
} ui_design_text_builder_t;

#if defined(__GNUC__) || defined(__clang__)
#define UI_DESIGN_PRINTF_FORMAT(iFormat, iArgs) __attribute__((format(printf, iFormat, iArgs)))
#else
#define UI_DESIGN_PRINTF_FORMAT(iFormat, iArgs)
#endif

static int __uiDesignBuilderReserve(ui_design_text_builder_t* pBuilder, size_t iExtra)
{
	char* sNewData;
	size_t iNeed;
	size_t iCapacity;
	size_t iMaxCapacity;

	if ( pBuilder == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBuilder->iLength > UI_DESIGN_DOCUMENT_MAX_BYTES ||
	     iExtra > UI_DESIGN_DOCUMENT_MAX_BYTES - pBuilder->iLength ) {
		return XUI_ERROR_BUFFER_TOO_SMALL;
	}
	iNeed = pBuilder->iLength + iExtra + 1u;
	if ( iNeed <= pBuilder->iCapacity ) return XUI_OK;
	iMaxCapacity = (size_t)UI_DESIGN_DOCUMENT_MAX_BYTES + 1u;
	iCapacity = (pBuilder->iCapacity > 0u) ? pBuilder->iCapacity : 4096u;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > iMaxCapacity / 2u ) {
			iCapacity = iMaxCapacity;
			break;
		}
		iCapacity *= 2u;
	}
	sNewData = (char*)realloc(pBuilder->sData, iCapacity);
	if ( sNewData == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pBuilder->sData = sNewData;
	pBuilder->iCapacity = iCapacity;
	return XUI_OK;
}

static int __uiDesignBuilderAppendRaw(ui_design_text_builder_t* pBuilder, const char* sText)
{
	size_t n;
	int iRet;

	if ( sText == NULL ) sText = "";
	n = strlen(sText);
	iRet = __uiDesignBuilderReserve(pBuilder, n);
	if ( iRet != XUI_OK ) return iRet;
	memcpy(pBuilder->sData + pBuilder->iLength, sText, n);
	pBuilder->iLength += n;
	pBuilder->sData[pBuilder->iLength] = '\0';
	return XUI_OK;
}

static int UI_DESIGN_PRINTF_FORMAT(2, 3) __uiDesignBuilderAppendFormat(ui_design_text_builder_t* pBuilder, const char* sFormat, ...)
{
	char sStack[512];
	char* sHeap;
	va_list args;
	va_list argsCopy;
	int n;
	int iRet;

	if ( (pBuilder == NULL) || (sFormat == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	va_start(args, sFormat);
	va_copy(argsCopy, args);
	n = vsnprintf(sStack, sizeof(sStack), sFormat, args);
	va_end(args);
	if ( n < 0 ) {
		va_end(argsCopy);
		return XUI_ERROR;
	}
	if ( n < (int)sizeof(sStack) ) {
		va_end(argsCopy);
		return __uiDesignBuilderAppendRaw(pBuilder, sStack);
	}
	sHeap = (char*)malloc((size_t)n + 1u);
	if ( sHeap == NULL ) {
		va_end(argsCopy);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	(void)vsnprintf(sHeap, (size_t)n + 1u, sFormat, argsCopy);
	va_end(argsCopy);
	iRet = __uiDesignBuilderAppendRaw(pBuilder, sHeap);
	free(sHeap);
	return iRet;
}

static int __uiDesignBuilderAppendJsonString(ui_design_text_builder_t* pBuilder, const char* sText)
{
	unsigned char c;
	char sEscaped[8];
	int iRet;

	if ( sText == NULL ) sText = "";
	iRet = __uiDesignBuilderAppendRaw(pBuilder, "\"");
	if ( iRet != XUI_OK ) return iRet;
	for ( ; *sText != '\0'; ++sText ) {
		c = (unsigned char)*sText;
		sEscaped[0] = '\0';
		switch ( c ) {
		case '\"':
			snprintf(sEscaped, sizeof(sEscaped), "\\\"");
			break;
		case '\\':
			snprintf(sEscaped, sizeof(sEscaped), "\\\\");
			break;
		case '\b':
			snprintf(sEscaped, sizeof(sEscaped), "\\b");
			break;
		case '\f':
			snprintf(sEscaped, sizeof(sEscaped), "\\f");
			break;
		case '\n':
			snprintf(sEscaped, sizeof(sEscaped), "\\n");
			break;
		case '\r':
			snprintf(sEscaped, sizeof(sEscaped), "\\r");
			break;
		case '\t':
			snprintf(sEscaped, sizeof(sEscaped), "\\t");
			break;
		default:
			if ( c < 0x20u ) {
				snprintf(sEscaped, sizeof(sEscaped), "\\u%04x", (unsigned int)c);
			}
			break;
		}
		if ( sEscaped[0] != '\0' ) {
			iRet = __uiDesignBuilderAppendRaw(pBuilder, sEscaped);
		} else {
			char sChar[2] = {(char)c, '\0'};
			iRet = __uiDesignBuilderAppendRaw(pBuilder, sChar);
		}
		if ( iRet != XUI_OK ) return iRet;
	}
	return __uiDesignBuilderAppendRaw(pBuilder, "\"");
}

static xvalue* __uiDesignJsonGet(xvalue* pTable, const char* sKey)
{
	if ( (pTable == NULL) || (!xuiXrtValueIsObject(pTable)) || (sKey == NULL) ) return NULL;
	return xuiXrtValueObjectGet(pTable, sKey, 0);
}

static const char* __uiDesignJsonText(xvalue* pValue, const char* sDefault)
{
	if ( pValue != NULL && xuiXrtValueIsText(pValue) ) return (const char*)xuiXrtValueGetText(pValue);
	return (sDefault != NULL) ? sDefault : "";
}

static int __uiDesignJsonReadInt(xvalue* pValue, int* pResult)
{
	double fValue;
	int64_t iValue;
	int iResult;

	if ( pValue == NULL || pResult == NULL ) return 0;
	if ( xuiXrtValueIsInt(pValue) ) {
		iValue = (int64_t)xuiXrtValueGetInt(pValue);
		if ( iValue < INT_MIN || iValue > INT_MAX ) return 0;
		*pResult = (int)iValue;
		return 1;
	}
	if ( xuiXrtValueType(pValue) != XVALUE_FLOAT ) return 0;
	fValue = xuiXrtValueGetFloat(pValue);
	if ( fValue < (double)INT_MIN || fValue > (double)INT_MAX ) return 0;
	iResult = (int)fValue;
	if ( (double)iResult != fValue ) return 0;
	*pResult = iResult;
	return 1;
}

static int __uiDesignJsonInt(xvalue* pValue, int iDefault)
{
	int iResult;

	return __uiDesignJsonReadInt(pValue, &iResult) ? iResult : iDefault;
}

static int __uiDesignJsonBool(xvalue* pValue, int bDefault)
{
	if ( pValue == NULL ) return bDefault ? 1 : 0;
	if ( xuiXrtValueIsBool(pValue) ) return xuiXrtValueGetBool(pValue) ? 1 : 0;
	if ( xuiXrtValueIsInt(pValue) ) return xuiXrtValueGetInt(pValue) != 0;
	if ( xuiXrtValueType(pValue) == XVALUE_FLOAT ) return xuiXrtValueGetFloat(pValue) != 0.0;
	return bDefault ? 1 : 0;
}

static ui_design_node_type_t __uiDesignJsonNodeType(xvalue* pNode)
{
	const char* sType;
	int iType;

	sType = __uiDesignJsonText(__uiDesignJsonGet(pNode, "type"), "");
	if ( sType[0] != '\0' ) {
		for ( iType = UI_DESIGN_NODE_NONE + 1; iType <= UI_DESIGN_NODE_LAST; ++iType ) {
			if ( strcmp(sType, uiDesignNodeTypeName((ui_design_node_type_t)iType)) == 0 ) return (ui_design_node_type_t)iType;
		}
		return UI_DESIGN_NODE_NONE;
	}
	iType = __uiDesignJsonInt(__uiDesignJsonGet(pNode, "typeId"), 0);
	if ( iType > UI_DESIGN_NODE_NONE && iType <= UI_DESIGN_NODE_LAST ) return (ui_design_node_type_t)iType;
	return UI_DESIGN_NODE_NONE;
}

static int __uiDesignDocumentValidateModel(const ui_design_model_t* pModel);

static int __uiDesignDocumentSave(const ui_design_model_t* pModel, char** ppSnapshot, int bSelection)
{
	ui_design_text_builder_t tBuilder;
	const ui_design_node_t* pNode;
	int i;
	int j;
	int iRet;

	if ( (pModel == NULL) || (ppSnapshot == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppSnapshot = NULL;
	iRet = __uiDesignDocumentValidateModel(pModel);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tBuilder, 0, sizeof(tBuilder));
	iRet = __uiDesignBuilderAppendFormat(&tBuilder,
		"{\n"
		"  \"format\": \"%s\",\n"
		"  \"version\": %d,\n"
		"  \"model\": {\n"
		"    \"nextId\": %d,\n"
		"    \"selectedId\": %d,\n"
		"    \"selection\": [",
		UI_DESIGN_DOCUMENT_FORMAT,
		UI_DESIGN_DOCUMENT_VERSION,
		pModel->iNextId,
		bSelection ? pModel->iSelectedId : 0);
	if ( iRet != XUI_OK ) goto fail;
	for ( i = 0; bSelection && i < pModel->iSelectedCount; ++i ) {
		iRet = __uiDesignBuilderAppendFormat(&tBuilder, "%s%d", (i == 0) ? "" : ", ", pModel->arrSelectedIds[i]);
		if ( iRet != XUI_OK ) goto fail;
	}
	iRet = __uiDesignBuilderAppendRaw(&tBuilder, "],\n    \"nodes\": [\n");
	if ( iRet != XUI_OK ) goto fail;
	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		pNode = &pModel->arrNodes[i];
		iRet = __uiDesignBuilderAppendFormat(&tBuilder,
			"%s"
			"      {\n"
			"        \"id\": %d,\n"
			"        \"type\": ",
			(i == 0) ? "" : ",\n",
			pNode->iId);
		if ( iRet != XUI_OK ) goto fail;
		iRet = __uiDesignBuilderAppendJsonString(&tBuilder, uiDesignNodeTypeName(pNode->iType));
		if ( iRet != XUI_OK ) goto fail;
		iRet = __uiDesignBuilderAppendFormat(&tBuilder,
			",\n"
			"        \"typeId\": %d,\n"
			"        \"parentId\": %d,\n"
			"        \"rect\": {\"x\": %d, \"y\": %d, \"w\": %d, \"h\": %d},\n"
			"        \"text\": ",
			(int)pNode->iType,
			pNode->iParentId,
			pNode->tRect.fX,
			pNode->tRect.fY,
			pNode->tRect.fW,
			pNode->tRect.fH);
		if ( iRet != XUI_OK ) goto fail;
		iRet = __uiDesignBuilderAppendJsonString(&tBuilder, pNode->sText);
		if ( iRet != XUI_OK ) goto fail;
		iRet = __uiDesignBuilderAppendFormat(&tBuilder,
			",\n"
			"        \"checked\": %s,\n"
			"        \"visible\": %s,\n"
			"        \"enabled\": %s,\n"
			"        \"properties\": [",
			pNode->bChecked ? "true" : "false",
			pNode->bVisible ? "true" : "false",
			pNode->bEnabled ? "true" : "false");
		if ( iRet != XUI_OK ) goto fail;
		for ( j = 0; j < pNode->iPropertyCount; ++j ) {
			iRet = __uiDesignBuilderAppendFormat(&tBuilder, "%s\n          {\"id\": ", (j == 0) ? "" : ",");
			if ( iRet != XUI_OK ) goto fail;
			iRet = __uiDesignBuilderAppendJsonString(&tBuilder, pNode->arrProperties[j].sId);
			if ( iRet != XUI_OK ) goto fail;
			iRet = __uiDesignBuilderAppendRaw(&tBuilder, ", \"value\": ");
			if ( iRet != XUI_OK ) goto fail;
			iRet = __uiDesignBuilderAppendJsonString(&tBuilder, pNode->arrProperties[j].sValue);
			if ( iRet != XUI_OK ) goto fail;
			iRet = __uiDesignBuilderAppendRaw(&tBuilder, "}");
			if ( iRet != XUI_OK ) goto fail;
		}
		iRet = __uiDesignBuilderAppendRaw(&tBuilder, (pNode->iPropertyCount > 0) ? "\n        ]\n      }" : "]\n      }");
		if ( iRet != XUI_OK ) goto fail;
	}
	iRet = __uiDesignBuilderAppendRaw(&tBuilder, "\n    ]\n  }\n}\n");
	if ( iRet != XUI_OK ) goto fail;
	*ppSnapshot = tBuilder.sData;
	return XUI_OK;
fail:
	free(tBuilder.sData);
	return iRet;
}

int uiDesignDocumentSaveModel(const ui_design_model_t* pModel, char** ppSnapshot)
{
	return __uiDesignDocumentSave(pModel, ppSnapshot, 1);
}

int uiDesignDocumentSaveContent(const ui_design_model_t* pModel, char** ppSnapshot)
{
	return __uiDesignDocumentSave(pModel, ppSnapshot, 0);
}

static int __uiDesignNodeHasPropertyId(const ui_design_node_t* pNode, const char* sId)
{
	int i;

	if ( (pNode == NULL) || (sId == NULL) ) return 0;
	for ( i = 0; i < pNode->iPropertyCount; ++i ) {
		if ( strcmp(pNode->arrProperties[i].sId, sId) == 0 ) return 1;
	}
	return 0;
}

static int __uiDesignDocumentValidateModel(const ui_design_model_t* pModel)
{
	const ui_design_node_t* pNode;
	const ui_design_node_t* pParent;
	int i;
	int j;
	int k;
	int iDepth;

	if ( pModel == NULL || pModel->iNodeCount < 0 || pModel->iNodeCount > UI_DESIGN_MAX_NODES ||
	     pModel->iNextId <= 0 || pModel->iNextId >= INT_MAX ||
	     pModel->iSelectedCount < 0 || pModel->iSelectedCount > UI_DESIGN_MAX_NODES ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		pNode = &pModel->arrNodes[i];
		if ( pNode->iId <= 0 || pNode->iType <= UI_DESIGN_NODE_NONE || pNode->iType > UI_DESIGN_NODE_LAST ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( pNode->tRect.fX < -(int)XUI_LAYOUT_UNBOUNDED || pNode->tRect.fX > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->tRect.fY < -(int)XUI_LAYOUT_UNBOUNDED || pNode->tRect.fY > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->tRect.fW < 1 || pNode->tRect.fW > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->tRect.fH < 1 || pNode->tRect.fH > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->iPropertyCount < 0 || pNode->iPropertyCount > UI_DESIGN_MAX_NODE_PROPERTIES ||
		     pNode->iPropertyCapacity < pNode->iPropertyCount ||
		     (pNode->iPropertyCount > 0 && pNode->arrProperties == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
		for ( j = 0; j < pNode->iPropertyCount; ++j ) {
			if ( pNode->arrProperties[j].sId[0] == '\0' ||
			     memchr(pNode->arrProperties[j].sId, '\0', sizeof(pNode->arrProperties[j].sId)) == NULL ||
			     memchr(pNode->arrProperties[j].sValue, '\0', sizeof(pNode->arrProperties[j].sValue)) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
			for ( k = j + 1; k < pNode->iPropertyCount; ++k ) {
				if ( strcmp(pNode->arrProperties[j].sId, pNode->arrProperties[k].sId) == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
			}
		}
		for ( j = i + 1; j < pModel->iNodeCount; ++j ) {
			if ( pModel->arrNodes[j].iId == pNode->iId ) return XUI_ERROR_INVALID_ARGUMENT;
		}
		pParent = pNode;
		iDepth = 0;
		while ( pParent->iParentId != 0 ) {
			pParent = uiDesignModelGetNodeConst(pModel, pParent->iParentId);
			if ( pParent == NULL || !uiDesignNodeTypeIsContainer(pParent->iType) ) return XUI_ERROR_INVALID_ARGUMENT;
			if ( pParent->iId == pNode->iId || ++iDepth > pModel->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( pModel->iSelectedId != 0 && uiDesignModelGetNodeConst(pModel, pModel->iSelectedId) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pModel->iSelectedCount; ++i ) {
		if ( uiDesignModelGetNodeConst(pModel, pModel->arrSelectedIds[i]) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		for ( j = i + 1; j < pModel->iSelectedCount; ++j ) {
			if ( pModel->arrSelectedIds[j] == pModel->arrSelectedIds[i] ) return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	return XUI_OK;
}

static void __uiDesignDocumentFreeModel(ui_design_model_t* pModel)
{
	if ( pModel == NULL ) return;
	uiDesignModelDestroy(pModel);
	free(pModel);
}

int uiDesignDocumentLoadModel(const char* sSnapshot, ui_design_model_t** ppModel)
{
	ui_design_model_t* pModel;
	ui_design_node_t* pNode;
	const unsigned char* pBytes;
	xvalue* pRoot;
	xvalue* pModelValue;
	xvalue* pSelection;
	xvalue* pNodes;
	xvalue* pNodeValue;
	xvalue* pRect;
	xvalue* pProperties;
	xvalue* pProperty;
	xvalue* pTextValue;
	const char* sFormat;
	const char* sText;
	const char* sPropertyId;
	const char* sPropertyValue;
	size_t iSnapshotLength;
	uint32_t iCount;
	uint32_t iPropCount;
	uint32_t i;
	uint32_t j;
	int iSelected;
	int iMaxId;
	int iVersion;
	int iRet;

	if ( (sSnapshot == NULL) || (ppModel == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppModel = NULL;
	iSnapshotLength = strlen(sSnapshot);
	if ( iSnapshotLength == 0u || iSnapshotLength > UI_DESIGN_DOCUMENT_MAX_BYTES ) return XUI_ERROR_INVALID_ARGUMENT;
	pBytes = (const unsigned char*)sSnapshot;
	if ( iSnapshotLength >= 3u && pBytes[0] == 0xEFu && pBytes[1] == 0xBBu && pBytes[2] == 0xBFu ) sSnapshot += 3;
	pRoot = xrtJsonParse(xuiXrtText(sSnapshot, 0));
	if ( pRoot == NULL || !xuiXrtValueIsObject(pRoot) ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pModel = NULL;
	iRet = XUI_ERROR_INVALID_ARGUMENT;
	sFormat = __uiDesignJsonText(__uiDesignJsonGet(pRoot, "format"), "");
	if ( strcmp(sFormat, UI_DESIGN_DOCUMENT_FORMAT) != 0 ) goto fail;
	if ( !__uiDesignJsonReadInt(__uiDesignJsonGet(pRoot, "version"), &iVersion) || iVersion != UI_DESIGN_DOCUMENT_VERSION ) goto fail;
	pModelValue = __uiDesignJsonGet(pRoot, "model");
	pNodes = __uiDesignJsonGet(pModelValue, "nodes");
	if ( pModelValue == NULL || !xuiXrtValueIsObject(pModelValue) || pNodes == NULL || !xuiXrtValueIsArray(pNodes) ) goto fail;
	iCount = xrtValueCount(pNodes);
	if ( iCount > UI_DESIGN_MAX_NODES ) {
		iRet = XUI_ERROR_OUT_OF_MEMORY;
		goto fail;
	}
	pModel = (ui_design_model_t*)calloc(1u, sizeof(*pModel));
	if ( pModel == NULL ) {
		iRet = XUI_ERROR_OUT_OF_MEMORY;
		goto fail;
	}
	uiDesignModelInit(pModel);
	pTextValue = __uiDesignJsonGet(pModelValue, "nextId");
	if ( pTextValue != NULL && !__uiDesignJsonReadInt(pTextValue, &pModel->iNextId) ) goto fail;
	pTextValue = __uiDesignJsonGet(pModelValue, "selectedId");
	if ( pTextValue != NULL && !__uiDesignJsonReadInt(pTextValue, &pModel->iSelectedId) ) goto fail;
	pSelection = __uiDesignJsonGet(pModelValue, "selection");
	if ( pSelection != NULL ) {
		if ( !xuiXrtValueIsArray(pSelection) || xrtValueCount(pSelection) > UI_DESIGN_MAX_NODES ) goto fail;
		for ( i = 0u; i < xrtValueCount(pSelection); ++i ) {
			if ( !__uiDesignJsonReadInt(xuiXrtValueArrayGet(pSelection, i), &iSelected) ) goto fail;
			if ( iSelected <= 0 || uiDesignModelIsSelected(pModel, iSelected) ) continue;
			pModel->arrSelectedIds[pModel->iSelectedCount++] = iSelected;
		}
	}
	iMaxId = 0;
	for ( i = 0u; i < iCount; ++i ) {
		pNodeValue = xuiXrtValueArrayGet(pNodes, i);
		if ( pNodeValue == NULL || !xuiXrtValueIsObject(pNodeValue) ) goto fail;
		pNode = &pModel->arrNodes[pModel->iNodeCount++];
		memset(pNode, 0, sizeof(*pNode));
		if ( !__uiDesignJsonReadInt(__uiDesignJsonGet(pNodeValue, "id"), &pNode->iId) ||
		     !__uiDesignJsonReadInt(__uiDesignJsonGet(pNodeValue, "parentId"), &pNode->iParentId) ) goto fail;
		pNode->iType = __uiDesignJsonNodeType(pNodeValue);
		if ( pNode->iId <= 0 || pNode->iId >= INT_MAX || pNode->iType <= UI_DESIGN_NODE_NONE ||
		     pNode->iType > UI_DESIGN_NODE_LAST || pNode->iParentId < 0 ) goto fail;
		pRect = __uiDesignJsonGet(pNodeValue, "rect");
		if ( pRect == NULL || !xuiXrtValueIsObject(pRect) ) goto fail;
		if ( !__uiDesignJsonReadInt(__uiDesignJsonGet(pRect, "x"), &pNode->tRect.fX) ||
		     !__uiDesignJsonReadInt(__uiDesignJsonGet(pRect, "y"), &pNode->tRect.fY) ||
		     !__uiDesignJsonReadInt(__uiDesignJsonGet(pRect, "w"), &pNode->tRect.fW) ||
		     !__uiDesignJsonReadInt(__uiDesignJsonGet(pRect, "h"), &pNode->tRect.fH) ) goto fail;
		if ( pNode->tRect.fX < -(int)XUI_LAYOUT_UNBOUNDED || pNode->tRect.fX > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->tRect.fY < -(int)XUI_LAYOUT_UNBOUNDED || pNode->tRect.fY > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->tRect.fW < 1 || pNode->tRect.fW > (int)XUI_LAYOUT_UNBOUNDED ||
		     pNode->tRect.fH < 1 || pNode->tRect.fH > (int)XUI_LAYOUT_UNBOUNDED ) goto fail;
		pTextValue = __uiDesignJsonGet(pNodeValue, "text");
		if ( pTextValue != NULL && !xuiXrtValueIsText(pTextValue) ) goto fail;
		sText = __uiDesignJsonText(pTextValue, "");
		if ( strlen(sText) >= sizeof(pNode->sText) ) goto fail;
		snprintf(pNode->sText, sizeof(pNode->sText), "%s", sText);
		pNode->bChecked = __uiDesignJsonBool(__uiDesignJsonGet(pNodeValue, "checked"), 0);
		pNode->bVisible = __uiDesignJsonBool(__uiDesignJsonGet(pNodeValue, "visible"), 1);
		pNode->bEnabled = __uiDesignJsonBool(__uiDesignJsonGet(pNodeValue, "enabled"), 1);
		pProperties = __uiDesignJsonGet(pNodeValue, "properties");
		if ( pProperties != NULL ) {
			if ( !xuiXrtValueIsArray(pProperties) ) goto fail;
			iPropCount = xrtValueCount(pProperties);
			if ( iPropCount > UI_DESIGN_MAX_NODE_PROPERTIES ) goto fail;
			for ( j = 0u; j < iPropCount; ++j ) {
				pProperty = xuiXrtValueArrayGet(pProperties, j);
				if ( pProperty == NULL || !xuiXrtValueIsObject(pProperty) ) goto fail;
				pTextValue = __uiDesignJsonGet(pProperty, "id");
				if ( pTextValue == NULL || !xuiXrtValueIsText(pTextValue) ) goto fail;
				sPropertyId = __uiDesignJsonText(pTextValue, "");
				pTextValue = __uiDesignJsonGet(pProperty, "value");
				if ( pTextValue != NULL && !xuiXrtValueIsText(pTextValue) ) goto fail;
				sPropertyValue = __uiDesignJsonText(pTextValue, "");
				if ( sPropertyId[0] == '\0' || strlen(sPropertyId) >= UI_DESIGN_PROPERTY_ID_CAPACITY ||
				     strlen(sPropertyValue) >= UI_DESIGN_PROPERTY_VALUE_CAPACITY ||
				     __uiDesignNodeHasPropertyId(pNode, sPropertyId) ) goto fail;
				iRet = uiDesignNodeSetProperty(pNode, sPropertyId, sPropertyValue);
				if ( iRet != XUI_OK ) goto fail;
			}
		}
		if ( pNode->iId > iMaxId ) iMaxId = pNode->iId;
	}
	iRet = __uiDesignDocumentValidateModel(pModel);
	if ( iRet != XUI_OK ) goto fail;
	if ( iMaxId >= INT_MAX - 1 ) goto fail;
	if ( pModel->iNextId <= iMaxId ) pModel->iNextId = iMaxId + 1;
	if ( pModel->iNextId <= 0 ) pModel->iNextId = 1;
	if ( pModel->iNextId >= INT_MAX ) goto fail;
	if ( pModel->iSelectedId != 0 && uiDesignModelGetNode(pModel, pModel->iSelectedId) == NULL ) pModel->iSelectedId = 0;
	for ( i = 0u; i < (uint32_t)pModel->iSelectedCount; ) {
		if ( uiDesignModelGetNode(pModel, pModel->arrSelectedIds[i]) == NULL ) {
			memmove(&pModel->arrSelectedIds[i], &pModel->arrSelectedIds[i + 1], (size_t)(pModel->iSelectedCount - (int)i - 1) * sizeof(pModel->arrSelectedIds[0]));
			pModel->iSelectedCount--;
		} else {
			i++;
		}
	}
	if ( pModel->iSelectedId == 0 && pModel->iSelectedCount > 0 ) pModel->iSelectedId = pModel->arrSelectedIds[pModel->iSelectedCount - 1];
	if ( pModel->iSelectedId != 0 && !uiDesignModelIsSelected(pModel, pModel->iSelectedId) ) {
		pModel->arrSelectedIds[pModel->iSelectedCount++] = pModel->iSelectedId;
	}
	pModel->iRevision++;
	xrtValueRelease(pRoot);
	*ppModel = pModel;
	return XUI_OK;

fail:
	__uiDesignDocumentFreeModel(pModel);
	xrtValueRelease(pRoot);
	return iRet;
}
