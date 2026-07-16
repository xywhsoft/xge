#include "ui_design_document.h"

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

static int __uiDesignBuilderReserve(ui_design_text_builder_t* pBuilder, size_t iExtra)
{
	char* sNewData;
	size_t iNeed;
	size_t iCapacity;

	if ( pBuilder == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNeed = pBuilder->iLength + iExtra + 1u;
	if ( iNeed <= pBuilder->iCapacity ) return XUI_OK;
	iCapacity = (pBuilder->iCapacity > 0u) ? pBuilder->iCapacity : 4096u;
	while ( iCapacity < iNeed ) iCapacity *= 2u;
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

static int __uiDesignBuilderAppendFormat(ui_design_text_builder_t* pBuilder, const char* sFormat, ...)
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

static xvalue __uiDesignJsonGet(xvalue pTable, const char* sKey)
{
	if ( (pTable == NULL) || (pTable->Type != XVO_DT_TABLE) || (sKey == NULL) ) return NULL;
	return xvoTableGetValue(pTable, sKey, 0);
}

static const char* __uiDesignJsonText(xvalue pValue, const char* sDefault)
{
	if ( pValue != NULL && pValue->Type == XVO_DT_TEXT ) return (const char*)xvoGetText(pValue);
	return (sDefault != NULL) ? sDefault : "";
}

static int __uiDesignJsonInt(xvalue pValue, int iDefault)
{
	if ( pValue == NULL ) return iDefault;
	if ( pValue->Type == XVO_DT_INT ) return (int)xvoGetInt(pValue);
	if ( pValue->Type == XVO_DT_FLOAT ) return (int)xvoGetFloat(pValue);
	if ( pValue->Type == XVO_DT_BOOL ) return xvoGetBool(pValue) ? 1 : 0;
	return iDefault;
}

static float __uiDesignJsonFloat(xvalue pValue, float fDefault)
{
	if ( pValue == NULL ) return fDefault;
	if ( pValue->Type == XVO_DT_FLOAT ) return (float)xvoGetFloat(pValue);
	if ( pValue->Type == XVO_DT_INT ) return (float)xvoGetInt(pValue);
	if ( pValue->Type == XVO_DT_BOOL ) return xvoGetBool(pValue) ? 1.0f : 0.0f;
	return fDefault;
}

static int __uiDesignJsonBool(xvalue pValue, int bDefault)
{
	if ( pValue == NULL ) return bDefault ? 1 : 0;
	if ( pValue->Type == XVO_DT_BOOL ) return xvoGetBool(pValue) ? 1 : 0;
	if ( pValue->Type == XVO_DT_INT ) return xvoGetInt(pValue) != 0;
	if ( pValue->Type == XVO_DT_FLOAT ) return xvoGetFloat(pValue) != 0.0;
	return bDefault ? 1 : 0;
}

static void __uiDesignCopyJsonText(char* sDst, int iCapacity, xvalue pValue, const char* sDefault)
{
	const char* sText;

	if ( (sDst == NULL) || (iCapacity <= 0) ) return;
	sText = __uiDesignJsonText(pValue, sDefault);
	snprintf(sDst, (size_t)iCapacity, "%s", (sText != NULL) ? sText : "");
	sDst[iCapacity - 1] = '\0';
}

static ui_design_node_type_t __uiDesignJsonNodeType(xvalue pNode)
{
	const char* sType;
	int iType;

	iType = __uiDesignJsonInt(__uiDesignJsonGet(pNode, "typeId"), 0);
	if ( iType > UI_DESIGN_NODE_NONE && iType <= UI_DESIGN_NODE_LAST ) return (ui_design_node_type_t)iType;
	sType = __uiDesignJsonText(__uiDesignJsonGet(pNode, "type"), "");
	for ( iType = UI_DESIGN_NODE_NONE + 1; iType <= UI_DESIGN_NODE_LAST; ++iType ) {
		if ( strcmp(sType, uiDesignNodeTypeName((ui_design_node_type_t)iType)) == 0 ) return (ui_design_node_type_t)iType;
	}
	return UI_DESIGN_NODE_NONE;
}

int uiDesignDocumentSaveModel(const ui_design_model_t* pModel, char** ppSnapshot)
{
	ui_design_text_builder_t tBuilder;
	const ui_design_node_t* pNode;
	int i;
	int j;
	int iRet;

	if ( (pModel == NULL) || (ppSnapshot == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppSnapshot = NULL;
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
		pModel->iSelectedId);
	if ( iRet != XUI_OK ) goto fail;
	for ( i = 0; i < pModel->iSelectedCount; ++i ) {
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
			"        \"rect\": {\"x\": %.6f, \"y\": %.6f, \"w\": %.6f, \"h\": %.6f},\n"
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

int uiDesignDocumentLoadModel(const char* sSnapshot, ui_design_model_t** ppModel)
{
	ui_design_model_t* pModel;
	ui_design_node_t* pNode;
	const unsigned char* pBytes;
	xvalue pRoot;
	xvalue pModelValue;
	xvalue pSelection;
	xvalue pNodes;
	xvalue pNodeValue;
	xvalue pRect;
	xvalue pProperties;
	xvalue pProperty;
	const char* sFormat;
	const char* sPropertyId;
	const char* sPropertyValue;
	uint32_t iCount;
	uint32_t iPropCount;
	uint32_t i;
	uint32_t j;
	int iSelected;
	int iMaxId;

	if ( (sSnapshot == NULL) || (ppModel == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppModel = NULL;
	pBytes = (const unsigned char*)sSnapshot;
	if ( pBytes[0] == 0xEFu && pBytes[1] == 0xBBu && pBytes[2] == 0xBFu ) sSnapshot += 3;
	pRoot = xrtParseJSON((str)(void*)sSnapshot, 0);
	if ( pRoot == NULL || pRoot->Type != XVO_DT_TABLE ) {
		if ( pRoot != NULL ) xvoUnref(pRoot);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sFormat = __uiDesignJsonText(__uiDesignJsonGet(pRoot, "format"), "");
	if ( strcmp(sFormat, UI_DESIGN_DOCUMENT_FORMAT) != 0 ) {
		xvoUnref(pRoot);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __uiDesignJsonInt(__uiDesignJsonGet(pRoot, "version"), 0) != UI_DESIGN_DOCUMENT_VERSION ) {
		xvoUnref(pRoot);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pModelValue = __uiDesignJsonGet(pRoot, "model");
	pNodes = __uiDesignJsonGet(pModelValue, "nodes");
	if ( pModelValue == NULL || pModelValue->Type != XVO_DT_TABLE || pNodes == NULL || pNodes->Type != XVO_DT_ARRAY ) {
		xvoUnref(pRoot);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pModel = (ui_design_model_t*)calloc(1u, sizeof(*pModel));
	if ( pModel == NULL ) {
		xvoUnref(pRoot);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	uiDesignModelInit(pModel);
	pModel->iNextId = __uiDesignJsonInt(__uiDesignJsonGet(pModelValue, "nextId"), 1);
	pModel->iSelectedId = __uiDesignJsonInt(__uiDesignJsonGet(pModelValue, "selectedId"), 0);
	pModel->iSelectedCount = 0;
	pSelection = __uiDesignJsonGet(pModelValue, "selection");
	if ( pSelection != NULL && pSelection->Type == XVO_DT_ARRAY ) {
		iCount = xvoArrayItemCount(pSelection);
		for ( i = 0u; i < iCount && pModel->iSelectedCount < UI_DESIGN_MAX_NODES; ++i ) {
			iSelected = __uiDesignJsonInt(xvoArrayGetValue(pSelection, i), 0);
			if ( iSelected > 0 ) pModel->arrSelectedIds[pModel->iSelectedCount++] = iSelected;
		}
	}
	iMaxId = 0;
	iCount = xvoArrayItemCount(pNodes);
	for ( i = 0u; i < iCount; ++i ) {
		if ( pModel->iNodeCount >= UI_DESIGN_MAX_NODES ) {
			free(pModel);
			xvoUnref(pRoot);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		pNodeValue = xvoArrayGetValue(pNodes, i);
		if ( pNodeValue == NULL || pNodeValue->Type != XVO_DT_TABLE ) {
			free(pModel);
			xvoUnref(pRoot);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		pNode = &pModel->arrNodes[pModel->iNodeCount++];
		memset(pNode, 0, sizeof(*pNode));
		pNode->iId = __uiDesignJsonInt(__uiDesignJsonGet(pNodeValue, "id"), 0);
		pNode->iType = __uiDesignJsonNodeType(pNodeValue);
		pNode->iParentId = __uiDesignJsonInt(__uiDesignJsonGet(pNodeValue, "parentId"), 0);
		if ( pNode->iId <= 0 || pNode->iType <= UI_DESIGN_NODE_NONE || pNode->iType > UI_DESIGN_NODE_LAST || pNode->iParentId < 0 ) {
			free(pModel);
			xvoUnref(pRoot);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		pRect = __uiDesignJsonGet(pNodeValue, "rect");
		pNode->tRect.fX = __uiDesignJsonFloat(__uiDesignJsonGet(pRect, "x"), 0.0f);
		pNode->tRect.fY = __uiDesignJsonFloat(__uiDesignJsonGet(pRect, "y"), 0.0f);
		pNode->tRect.fW = __uiDesignJsonFloat(__uiDesignJsonGet(pRect, "w"), 80.0f);
		pNode->tRect.fH = __uiDesignJsonFloat(__uiDesignJsonGet(pRect, "h"), 24.0f);
		if ( pNode->tRect.fW < 1.0f ) pNode->tRect.fW = 1.0f;
		if ( pNode->tRect.fH < 1.0f ) pNode->tRect.fH = 1.0f;
		__uiDesignCopyJsonText(pNode->sText, sizeof(pNode->sText), __uiDesignJsonGet(pNodeValue, "text"), "");
		pNode->bChecked = __uiDesignJsonBool(__uiDesignJsonGet(pNodeValue, "checked"), 0);
		pNode->bVisible = __uiDesignJsonBool(__uiDesignJsonGet(pNodeValue, "visible"), 1);
		pNode->bEnabled = __uiDesignJsonBool(__uiDesignJsonGet(pNodeValue, "enabled"), 1);
		pProperties = __uiDesignJsonGet(pNodeValue, "properties");
		if ( pProperties != NULL && pProperties->Type == XVO_DT_ARRAY ) {
			iPropCount = xvoArrayItemCount(pProperties);
			for ( j = 0u; j < iPropCount && pNode->iPropertyCount < UI_DESIGN_MAX_NODE_PROPERTIES; ++j ) {
				pProperty = xvoArrayGetValue(pProperties, j);
				if ( pProperty == NULL || pProperty->Type != XVO_DT_TABLE ) continue;
				sPropertyId = __uiDesignJsonText(__uiDesignJsonGet(pProperty, "id"), "");
				sPropertyValue = __uiDesignJsonText(__uiDesignJsonGet(pProperty, "value"), "");
				if ( sPropertyId[0] == '\0' ) continue;
				(void)uiDesignNodeSetProperty(pNode, sPropertyId, sPropertyValue);
			}
		}
		if ( pNode->iId > iMaxId ) iMaxId = pNode->iId;
	}
	if ( pModel->iNextId <= iMaxId ) pModel->iNextId = iMaxId + 1;
	if ( pModel->iNextId <= 0 ) pModel->iNextId = 1;
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
		if ( pModel->iSelectedCount < UI_DESIGN_MAX_NODES ) pModel->arrSelectedIds[pModel->iSelectedCount++] = pModel->iSelectedId;
	}
	pModel->iRevision++;
	xvoUnref(pRoot);
	*ppModel = pModel;
	return XUI_OK;
}
