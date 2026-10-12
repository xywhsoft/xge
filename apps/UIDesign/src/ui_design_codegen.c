#include "ui_design_codegen.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UI_DESIGN_CODEGEN_MAX_BYTES (16u * 1024u * 1024u)
#define UI_DESIGN_CODEGEN_SYMBOL_CAPACITY 128

typedef struct ui_design_codegen_builder_t {
	char* sData;
	size_t iLength;
	size_t iCapacity;
	int iError;
} ui_design_codegen_builder_t;

typedef struct ui_design_codegen_tabs_t {
	char arrTitle[XUI_TABS_PAGE_CAPACITY][256];
	int arrEnabled[XUI_TABS_PAGE_CAPACITY];
	int arrDirty[XUI_TABS_PAGE_CAPACITY];
	int iCount;
} ui_design_codegen_tabs_t;

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
static void __uiDesignCodegenAppendFormat(ui_design_codegen_builder_t* pBuilder, const char* sFormat, ...)
{
	va_list tArgs;
	va_list tCopy;
	char* pNew;
	size_t iRequired;
	size_t iCapacity;
	int iWritten;

	if ( pBuilder == NULL || sFormat == NULL || pBuilder->iError != XUI_OK ) return;
	va_start(tArgs, sFormat);
	va_copy(tCopy, tArgs);
	iWritten = vsnprintf(NULL, 0, sFormat, tCopy);
	va_end(tCopy);
	if ( iWritten < 0 ) {
		pBuilder->iError = XUI_ERROR;
		va_end(tArgs);
		return;
	}
	iRequired = pBuilder->iLength + (size_t)iWritten + 1u;
	if ( iRequired > UI_DESIGN_CODEGEN_MAX_BYTES ) {
		pBuilder->iError = XUI_ERROR_BUFFER_TOO_SMALL;
		va_end(tArgs);
		return;
	}
	if ( iRequired > pBuilder->iCapacity ) {
		iCapacity = pBuilder->iCapacity > 0u ? pBuilder->iCapacity : 4096u;
		while ( iCapacity < iRequired ) iCapacity *= 2u;
		pNew = (char*)realloc(pBuilder->sData, iCapacity);
		if ( pNew == NULL ) {
			pBuilder->iError = XUI_ERROR_OUT_OF_MEMORY;
			va_end(tArgs);
			return;
		}
		pBuilder->sData = pNew;
		pBuilder->iCapacity = iCapacity;
	}
	(void)vsnprintf(pBuilder->sData + pBuilder->iLength, pBuilder->iCapacity - pBuilder->iLength, sFormat, tArgs);
	va_end(tArgs);
	pBuilder->iLength += (size_t)iWritten;
}

static void __uiDesignCodegenAppendCString(ui_design_codegen_builder_t* pBuilder, const char* sText)
{
	const unsigned char* pCursor;

	if ( pBuilder == NULL || pBuilder->iError != XUI_OK ) return;
	if ( sText == NULL ) sText = "";
	__uiDesignCodegenAppendFormat(pBuilder, "\"");
	pCursor = (const unsigned char*)sText;
	while ( *pCursor != 0u && pBuilder->iError == XUI_OK ) {
		switch ( *pCursor ) {
		case '\\': __uiDesignCodegenAppendFormat(pBuilder, "\\\\"); break;
		case '"': __uiDesignCodegenAppendFormat(pBuilder, "\\\""); break;
		case '\n': __uiDesignCodegenAppendFormat(pBuilder, "\\n"); break;
		case '\r': __uiDesignCodegenAppendFormat(pBuilder, "\\r"); break;
		case '\t': __uiDesignCodegenAppendFormat(pBuilder, "\\t"); break;
		default:
			if ( *pCursor < 32u || *pCursor == 127u ) {
				__uiDesignCodegenAppendFormat(pBuilder, "\\%03o", (unsigned int)*pCursor);
			} else {
				__uiDesignCodegenAppendFormat(pBuilder, "%c", (int)*pCursor);
			}
			break;
		}
		++pCursor;
	}
	__uiDesignCodegenAppendFormat(pBuilder, "\"");
}

int uiDesignCodegenSupportsControl(ui_design_node_type_t iType)
{
	switch ( iType ) {
	case UI_DESIGN_NODE_WIDGET:
	case UI_DESIGN_NODE_PANEL:
	case UI_DESIGN_NODE_LABEL:
	case UI_DESIGN_NODE_BUTTON:
	case UI_DESIGN_NODE_INPUT:
	case UI_DESIGN_NODE_CHECKBOX:
	case UI_DESIGN_NODE_IMAGE:
	case UI_DESIGN_NODE_TABS:
		return 1;
	default:
		return 0;
	}
}

static int __uiDesignCodegenContainer(ui_design_node_type_t iType)
{
	return iType == UI_DESIGN_NODE_WIDGET || iType == UI_DESIGN_NODE_PANEL || iType == UI_DESIGN_NODE_TABS;
}

static const char* __uiDesignCodegenTypeToken(ui_design_node_type_t iType)
{
	switch ( iType ) {
	case UI_DESIGN_NODE_WIDGET: return "widget";
	case UI_DESIGN_NODE_PANEL: return "panel";
	case UI_DESIGN_NODE_LABEL: return "label";
	case UI_DESIGN_NODE_BUTTON: return "button";
	case UI_DESIGN_NODE_INPUT: return "input";
	case UI_DESIGN_NODE_CHECKBOX: return "checkbox";
	case UI_DESIGN_NODE_IMAGE: return "image";
	case UI_DESIGN_NODE_TABS: return "tabs";
	default: return "node";
	}
}

static void __uiDesignCodegenSanitize(const char* sSource, char* sTarget, size_t iCapacity, int bUpper)
{
	size_t iRead;
	size_t iWrite;
	int c;

	if ( sTarget == NULL || iCapacity == 0u ) return;
	iWrite = 0u;
	if ( sSource == NULL || sSource[0] == 0 ) sSource = "uidesign_view";
	if ( isdigit((unsigned char)sSource[0]) && iWrite + 1u < iCapacity ) sTarget[iWrite++] = '_';
	for ( iRead = 0u; sSource[iRead] != 0 && iWrite + 1u < iCapacity; ++iRead ) {
		c = (unsigned char)sSource[iRead];
		if ( !isalnum(c) && c != '_' ) c = '_';
		if ( bUpper ) c = toupper(c);
		else c = tolower(c);
		sTarget[iWrite++] = (char)c;
	}
	if ( iWrite == 0u ) {
		strncpy(sTarget, bUpper ? "UIDESIGN_VIEW" : "uidesign_view", iCapacity - 1u);
		sTarget[iCapacity - 1u] = 0;
		return;
	}
	sTarget[iWrite] = 0;
}

static const char* __uiDesignCodegenBaseName(const char* sPath)
{
	const char* sBase;
	const char* p;

	sBase = sPath;
	for ( p = sPath; p != NULL && *p != 0; ++p ) {
		if ( *p == '/' || *p == '\\' ) sBase = p + 1;
	}
	return sBase;
}

static int __uiDesignCodegenHeaderPath(const char* sCPath, char* sHeaderPath, size_t iCapacity)
{
	const char* sBase;
	const char* sDot;
	size_t iPrefix;

	if ( sCPath == NULL || sHeaderPath == NULL || iCapacity == 0u ) return XUI_ERROR_INVALID_ARGUMENT;
	sBase = __uiDesignCodegenBaseName(sCPath);
	sDot = strrchr(sBase, '.');
	if ( sDot != NULL && sDot[1] != 0 && sDot[2] == 0 &&
	     tolower((unsigned char)sDot[1]) == 'h' ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sDot == NULL ) sDot = sCPath + strlen(sCPath);
	iPrefix = (size_t)(sDot - sCPath);
	if ( iPrefix + 3u > iCapacity ) return XUI_ERROR_BUFFER_TOO_SMALL;
	memcpy(sHeaderPath, sCPath, iPrefix);
	memcpy(sHeaderPath + iPrefix, ".h", 3u);
	return XUI_OK;
}

static int __uiDesignCodegenOrder(const ui_design_model_t* pModel, int* arrOrder)
{
	unsigned char arrDone[UI_DESIGN_MAX_NODES];
	const ui_design_node_t* pParent;
	int iCount;
	int iProgress;
	int i;

	memset(arrDone, 0, sizeof(arrDone));
	iCount = 0;
	while ( iCount < pModel->iNodeCount ) {
		iProgress = 0;
		for ( i = 0; i < pModel->iNodeCount; ++i ) {
			if ( arrDone[i] ) continue;
			if ( !uiDesignCodegenSupportsControl(pModel->arrNodes[i].iType) ) return XUI_ERROR_UNSUPPORTED;
			if ( pModel->arrNodes[i].iParentId != 0 ) {
				pParent = uiDesignModelGetNodeConst(pModel, pModel->arrNodes[i].iParentId);
				if ( pParent == NULL || !__uiDesignCodegenContainer(pParent->iType) ) return XUI_ERROR_INVALID_ARGUMENT;
				{
					int j;
					int bParentDone = 0;
					for ( j = 0; j < pModel->iNodeCount; ++j ) {
						if ( &pModel->arrNodes[j] == pParent ) {
							bParentDone = arrDone[j] != 0u;
							break;
						}
					}
					if ( !bParentDone ) continue;
				}
			}
			arrDone[i] = 1u;
			arrOrder[iCount++] = i;
			iProgress = 1;
		}
		if ( !iProgress ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	return XUI_OK;
}

static int __uiDesignCodegenBoolText(const char* sText, int bDefault)
{
	char sLower[16];
	size_t i;

	if ( sText == NULL || sText[0] == 0 ) return bDefault;
	for ( i = 0u; sText[i] != 0 && i + 1u < sizeof(sLower); ++i ) sLower[i] = (char)tolower((unsigned char)sText[i]);
	sLower[i] = 0;
	if ( strcmp(sLower, "true") == 0 || strcmp(sLower, "yes") == 0 || strcmp(sLower, "on") == 0 || strcmp(sLower, "1") == 0 ) return 1;
	if ( strcmp(sLower, "false") == 0 || strcmp(sLower, "no") == 0 || strcmp(sLower, "off") == 0 || strcmp(sLower, "0") == 0 ) return 0;
	return bDefault;
}

static void __uiDesignCodegenTabs(const ui_design_node_t* pNode, ui_design_codegen_tabs_t* pTabs)
{
	const char* sCursor;
	const char* sEnd;
	const char* sField;
	size_t iLength;
	int iField;

	memset(pTabs, 0, sizeof(*pTabs));
	sCursor = uiDesignNodeGetProperty(pNode, "data.items", "");
	while ( *sCursor != 0 && pTabs->iCount < XUI_TABS_PAGE_CAPACITY ) {
		while ( *sCursor == '\r' || *sCursor == '\n' ) ++sCursor;
		if ( *sCursor == 0 ) break;
		pTabs->arrEnabled[pTabs->iCount] = 1;
		sEnd = sCursor;
		while ( *sEnd != 0 && *sEnd != '\r' && *sEnd != '\n' ) ++sEnd;
		sField = sCursor;
		for ( iField = 0; iField < 3; ++iField ) {
			const char* sSeparator = sField;
			while ( sSeparator < sEnd && *sSeparator != '|' ) ++sSeparator;
			iLength = (size_t)(sSeparator - sField);
			if ( iField == 0 ) {
				if ( iLength >= sizeof(pTabs->arrTitle[0]) ) iLength = sizeof(pTabs->arrTitle[0]) - 1u;
				memcpy(pTabs->arrTitle[pTabs->iCount], sField, iLength);
				pTabs->arrTitle[pTabs->iCount][iLength] = 0;
			} else {
				char sValue[16];
				if ( iLength >= sizeof(sValue) ) iLength = sizeof(sValue) - 1u;
				memcpy(sValue, sField, iLength);
				sValue[iLength] = 0;
				if ( iField == 1 ) pTabs->arrEnabled[pTabs->iCount] = __uiDesignCodegenBoolText(sValue, 1);
				else pTabs->arrDirty[pTabs->iCount] = __uiDesignCodegenBoolText(sValue, 0);
			}
			if ( sSeparator >= sEnd ) break;
			sField = sSeparator + 1;
		}
		if ( pTabs->arrTitle[pTabs->iCount][0] != 0 ) ++pTabs->iCount;
		sCursor = sEnd;
	}
	if ( pTabs->iCount == 0 ) {
		strcpy(pTabs->arrTitle[0], "Page");
		pTabs->arrEnabled[0] = 1;
		pTabs->iCount = 1;
	}
}

static void __uiDesignCodegenMember(char* sMember, size_t iCapacity, const ui_design_node_t* pNode)
{
	snprintf(sMember, iCapacity, "%s_%d", __uiDesignCodegenTypeToken(pNode->iType), pNode->iId);
}

static void __uiDesignCodegenEmitHeader(ui_design_codegen_builder_t* pHeader, const ui_design_model_t* pModel,
	const char* sSymbol, const char* sGuard)
{
	char sMember[64];
	int i;

	__uiDesignCodegenAppendFormat(pHeader,
		"/* Generated by XUI UI Design. Edit the .xui design and export again. */\n"
		"#ifndef %s_H\n#define %s_H\n\n#include \"xui.h\"\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
		"typedef struct %s_callbacks_t {\n"
		"\tvoid (*onAction)(const char* sAction, xui_widget pWidget, void* pUser);\n"
		"\txui_surface (*resolveSurface)(const char* sPath, void* pUser);\n"
		"\tvoid* pUser;\n"
		"} %s_callbacks_t;\n\n"
		"typedef struct %s_view_t {\n\txui_widget pRoot;\n\t%s_callbacks_t tCallbacks;\n",
		sGuard, sGuard, sSymbol, sSymbol, sSymbol, sSymbol);
	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		__uiDesignCodegenMember(sMember, sizeof(sMember), &pModel->arrNodes[i]);
		__uiDesignCodegenAppendFormat(pHeader, "\txui_widget %s;\n", sMember);
	}
	__uiDesignCodegenAppendFormat(pHeader,
		"} %s_view_t;\n\n"
		"int %s_create(xui_context pContext, xui_widget pParent, xui_font pFont, const %s_callbacks_t* pCallbacks, %s_view_t* pView);\n"
		"void %s_destroy(%s_view_t* pView);\n\n"
		"#ifdef __cplusplus\n}\n#endif\n\n#endif\n",
		sSymbol, sSymbol, sSymbol, sSymbol, sSymbol, sSymbol);
}

static void __uiDesignCodegenEmitTabsData(ui_design_codegen_builder_t* pSource, const ui_design_model_t* pModel, const char* sSymbol)
{
	ui_design_codegen_tabs_t tTabs;
	const ui_design_node_t* pNode;
	int i;
	int j;

	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		pNode = &pModel->arrNodes[i];
		if ( pNode->iType != UI_DESIGN_NODE_TABS ) continue;
		__uiDesignCodegenTabs(pNode, &tTabs);
		__uiDesignCodegenAppendFormat(pSource, "static const char* %s_tabs_%d_items[] = {", sSymbol, pNode->iId);
		for ( j = 0; j < tTabs.iCount; ++j ) {
			if ( j != 0 ) __uiDesignCodegenAppendFormat(pSource, ", ");
			__uiDesignCodegenAppendCString(pSource, tTabs.arrTitle[j]);
		}
		__uiDesignCodegenAppendFormat(pSource, "};\nstatic const int %s_tabs_%d_enabled[] = {", sSymbol, pNode->iId);
		for ( j = 0; j < tTabs.iCount; ++j ) __uiDesignCodegenAppendFormat(pSource, "%s%d", j != 0 ? ", " : "", tTabs.arrEnabled[j]);
		__uiDesignCodegenAppendFormat(pSource, "};\nstatic const int %s_tabs_%d_dirty[] = {", sSymbol, pNode->iId);
		for ( j = 0; j < tTabs.iCount; ++j ) __uiDesignCodegenAppendFormat(pSource, "%s%d", j != 0 ? ", " : "", tTabs.arrDirty[j]);
		__uiDesignCodegenAppendFormat(pSource, "};\n\n");
	}
}

static void __uiDesignCodegenEmitCallbacks(ui_design_codegen_builder_t* pSource, const ui_design_model_t* pModel, const char* sSymbol)
{
	const ui_design_node_t* pNode;
	const char* sAction;
	int i;

	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		pNode = &pModel->arrNodes[i];
		if ( pNode->iType != UI_DESIGN_NODE_BUTTON ) continue;
		sAction = uiDesignNodeGetProperty(pNode, "event.onClick", "");
		if ( sAction[0] == 0 ) continue;
		__uiDesignCodegenAppendFormat(pSource, "static void %s_button_%d_click(xui_widget pWidget, void* pUser)\n{\n\t%s_view_t* pView = (%s_view_t*)pUser;\n\tif ( pView != NULL && pView->tCallbacks.onAction != NULL ) pView->tCallbacks.onAction(", sSymbol, pNode->iId, sSymbol, sSymbol);
		__uiDesignCodegenAppendCString(pSource, sAction);
		__uiDesignCodegenAppendFormat(pSource, ", pWidget, pView->tCallbacks.pUser);\n}\n\n");
	}
}

static void __uiDesignCodegenEmitCreateNode(ui_design_codegen_builder_t* pSource, const ui_design_model_t* pModel,
	const ui_design_node_t* pNode, const char* sSymbol)
{
	ui_design_codegen_tabs_t tTabs;
	const ui_design_node_t* pParent;
	const char* sAction;
	const char* sImage;
	char sMember[64];
	char sParentMember[64];
	int iPage;

	__uiDesignCodegenMember(sMember, sizeof(sMember), pNode);
	__uiDesignCodegenAppendFormat(pSource, "\t/* %s #%d */\n\t{\n", uiDesignNodeTypeName(pNode->iType), pNode->iId);
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_WIDGET:
		__uiDesignCodegenAppendFormat(pSource, "\t\tiRet = xuiWidgetCreate(pContext, &pView->%s);\n", sMember);
		break;
	case UI_DESIGN_NODE_PANEL:
		__uiDesignCodegenAppendFormat(pSource, "\t\txui_panel_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\ttDesc.sTitle = ");
		__uiDesignCodegenAppendCString(pSource, pNode->sText);
		__uiDesignCodegenAppendFormat(pSource,
			";\n\t\ttDesc.pFont = pFont;\n\t\ttDesc.iTitleColor = 0x%08Xu;\n\t\ttDesc.iDisabledTitleColor = 0x%08Xu;\n"
			"\t\ttDesc.iBackgroundColor = 0x%08Xu;\n\t\ttDesc.iHeaderColor = 0x%08Xu;\n\t\ttDesc.iClientColor = 0x%08Xu;\n"
			"\t\ttDesc.iBorderColor = 0x%08Xu;\n\t\ttDesc.iTitleFlags = %d;\n\t\ttDesc.fHeaderHeight = %.6ff;\n"
			"\t\ttDesc.fHeaderGap = %.6ff;\n\t\ttDesc.fIconSize = %.6ff;\n\t\ttDesc.fBorderWidth = %.6ff;\n\t\ttDesc.bClipClient = %d;\n"
			"\t\tiRet = xuiPanelCreate(pContext, &pView->%s, &tDesc);\n",
			uiDesignNodeGetPropertyColor(pNode, "appearance.titleColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.disabledTitleColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 251, 255, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 242, 251, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE),
			uiDesignNodeGetPropertyColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			uiDesignNodeGetPropertyInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP),
			uiDesignNodeGetPropertyFloat(pNode, "metrics.headerHeight", 28.0f),
			uiDesignNodeGetPropertyFloat(pNode, "metrics.headerGap", 0.0f),
			uiDesignNodeGetPropertyFloat(pNode, "metrics.iconSize", 16.0f),
			uiDesignNodeGetPropertyFloat(pNode, "appearance.borderWidth", 1.0f),
			uiDesignNodeGetPropertyBool(pNode, "behavior.clientClip", 1), sMember);
		break;
	case UI_DESIGN_NODE_LABEL:
		__uiDesignCodegenAppendFormat(pSource, "\t\txui_label_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\ttDesc.sText = ");
		__uiDesignCodegenAppendCString(pSource, pNode->sText);
		__uiDesignCodegenAppendFormat(pSource,
			";\n\t\ttDesc.pFont = pFont;\n\t\ttDesc.iTextColor = 0x%08Xu;\n\t\ttDesc.iDisabledTextColor = 0x%08Xu;\n"
			"\t\ttDesc.iTextFlags = %d;\n\t\ttDesc.iWrapMode = %d;\n\t\ttDesc.bUnderline = %d;\n\t\ttDesc.fLineGap = %.6ff;\n\t\ttDesc.fParagraphGap = %.6ff;\n"
			"\t\tiRet = xuiLabelCreate(pContext, &pView->%s, &tDesc);\n",
			uiDesignNodeGetPropertyColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			uiDesignNodeGetPropertyInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP),
			uiDesignNodeGetPropertyInt(pNode, "text.wrapMode", XUI_TEXT_WRAP_NONE),
			uiDesignNodeGetPropertyBool(pNode, "text.underline", 0),
			uiDesignNodeGetPropertyFloat(pNode, "text.lineGap", 0.0f),
			uiDesignNodeGetPropertyFloat(pNode, "text.paragraphGap", 0.0f), sMember);
		break;
	case UI_DESIGN_NODE_BUTTON:
		__uiDesignCodegenAppendFormat(pSource, "\t\txui_button_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\ttDesc.sText = ");
		__uiDesignCodegenAppendCString(pSource, pNode->sText);
		__uiDesignCodegenAppendFormat(pSource,
			";\n\t\ttDesc.pFont = pFont;\n\t\ttDesc.iTextColor = 0x%08Xu;\n\t\ttDesc.iDisabledTextColor = 0x%08Xu;\n\t\ttDesc.iTextFlags = %d;\n"
			"\t\ttDesc.iNormalColor = 0x%08Xu;\n\t\ttDesc.iHoverColor = 0x%08Xu;\n\t\ttDesc.iActiveColor = 0x%08Xu;\n\t\ttDesc.iFocusColor = 0x%08Xu;\n"
			"\t\ttDesc.iDisabledColor = 0x%08Xu;\n\t\ttDesc.iCheckedColor = 0x%08Xu;\n\t\ttDesc.fBorderWidth = %.6ff;\n\t\ttDesc.iBorderColor = 0x%08Xu;\n"
			"\t\tiRet = xuiButtonCreate(pContext, &pView->%s, &tDesc);\n",
			uiDesignNodeGetPropertyColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			uiDesignNodeGetPropertyInt(pNode, "text.flags", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP),
			uiDesignNodeGetPropertyColor(pNode, "appearance.normalColor", XUI_COLOR_RGBA(248, 251, 255, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(238, 242, 247, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.checkedColor", XUI_COLOR_RGBA(217, 234, 254, 255)),
			uiDesignNodeGetPropertyFloat(pNode, "appearance.borderWidth", 1.0f),
			uiDesignNodeGetPropertyColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)), sMember);
		break;
	case UI_DESIGN_NODE_INPUT:
		__uiDesignCodegenAppendFormat(pSource, "\t\txui_input_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\ttDesc.sText = ");
		__uiDesignCodegenAppendCString(pSource, pNode->sText);
		__uiDesignCodegenAppendFormat(pSource, ";\n\t\ttDesc.sPlaceholder = ");
		__uiDesignCodegenAppendCString(pSource, uiDesignNodeGetProperty(pNode, "text.placeholder", "Input text"));
		__uiDesignCodegenAppendFormat(pSource,
			";\n\t\ttDesc.pFont = pFont;\n\t\ttDesc.iMaxLength = %d;\n\t\ttDesc.iTextAlign = %d;\n\t\ttDesc.bPassword = %d;\n\t\ttDesc.bReadonly = %d;\n\t\ttDesc.bError = %d;\n"
			"\t\ttDesc.iTextColor = 0x%08Xu;\n\t\ttDesc.iPlaceholderColor = 0x%08Xu;\n\t\ttDesc.iDisabledTextColor = 0x%08Xu;\n\t\ttDesc.iBackgroundColor = 0x%08Xu;\n"
			"\t\ttDesc.iBorderColor = 0x%08Xu;\n\t\ttDesc.iFocusBorderColor = 0x%08Xu;\n\t\ttDesc.fBorderWidth = %.6ff;\n\t\tiRet = xuiInputCreate(pContext, &pView->%s, &tDesc);\n",
			uiDesignNodeGetPropertyInt(pNode, "text.maxLength", 0), uiDesignNodeGetPropertyInt(pNode, "text.align", XUI_INPUT_ALIGN_LEFT),
			uiDesignNodeGetPropertyBool(pNode, "behavior.password", 0), uiDesignNodeGetPropertyBool(pNode, "behavior.readonly", 0),
			uiDesignNodeGetPropertyBool(pNode, "behavior.error", 0), uiDesignNodeGetPropertyColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			uiDesignNodeGetPropertyColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			uiDesignNodeGetPropertyFloat(pNode, "metrics.borderWidth", 1.0f), sMember);
		break;
	case UI_DESIGN_NODE_CHECKBOX:
		__uiDesignCodegenAppendFormat(pSource, "\t\txui_checkbox_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\ttDesc.sText = ");
		__uiDesignCodegenAppendCString(pSource, pNode->sText);
		__uiDesignCodegenAppendFormat(pSource,
			";\n\t\ttDesc.pFont = pFont;\n\t\ttDesc.iTextColor = 0x%08Xu;\n\t\ttDesc.iDisabledTextColor = 0x%08Xu;\n\t\ttDesc.iTextFlags = %d;\n"
			"\t\ttDesc.bChecked = %d;\n\t\ttDesc.fIndicatorSize = %.6ff;\n\t\ttDesc.fGap = %.6ff;\n\t\ttDesc.bUseBuiltinAtlas = %d;\n"
			"\t\tiRet = xuiCheckBoxCreate(pContext, &pView->%s, &tDesc);\n",
			uiDesignNodeGetPropertyColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			uiDesignNodeGetPropertyInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP),
			pNode->bChecked, uiDesignNodeGetPropertyFloat(pNode, "metrics.indicatorSize", 18.0f),
			uiDesignNodeGetPropertyFloat(pNode, "metrics.gap", 7.0f), uiDesignNodeGetPropertyBool(pNode, "behavior.useBuiltinAtlas", 1), sMember);
		break;
	case UI_DESIGN_NODE_IMAGE:
		sImage = uiDesignNodeGetProperty(pNode, "data.source", "");
		__uiDesignCodegenAppendFormat(pSource, "\t\txui_image_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\tif ( pView->tCallbacks.resolveSurface != NULL ) tDesc.pSurface = pView->tCallbacks.resolveSurface(");
		__uiDesignCodegenAppendCString(pSource, sImage);
		__uiDesignCodegenAppendFormat(pSource,
			", pView->tCallbacks.pUser);\n\t\ttDesc.tSrc = (xui_rect_t){%d, %d, %d, %d};\n\t\ttDesc.iColor = 0x%08Xu;\n\t\ttDesc.iMode = %d;\n\t\ttDesc.iAlignX = %d;\n\t\ttDesc.iAlignY = %d;\n"
			"\t\tiRet = xuiImageCreate(pContext, &pView->%s, &tDesc);\n",
			(int)uiDesignNodeGetPropertyFloat(pNode, "source.x", 0.0f), (int)uiDesignNodeGetPropertyFloat(pNode, "source.y", 0.0f),
			(int)uiDesignNodeGetPropertyFloat(pNode, "source.w", 0.0f), (int)uiDesignNodeGetPropertyFloat(pNode, "source.h", 0.0f),
			uiDesignNodeGetPropertyColor(pNode, "appearance.tintColor", XUI_COLOR_WHITE), uiDesignNodeGetPropertyInt(pNode, "behavior.mode", 2),
			uiDesignNodeGetPropertyInt(pNode, "behavior.alignX", XUI_ALIGN_CENTER), uiDesignNodeGetPropertyInt(pNode, "behavior.alignY", XUI_ALIGN_CENTER), sMember);
		break;
	case UI_DESIGN_NODE_TABS:
		__uiDesignCodegenTabs(pNode, &tTabs);
		__uiDesignCodegenAppendFormat(pSource,
			"\t\txui_tabs_desc_t tDesc;\n\t\tmemset(&tDesc, 0, sizeof(tDesc));\n\t\ttDesc.iSize = sizeof(tDesc);\n\t\ttDesc.arrItems = %s_tabs_%d_items;\n"
			"\t\ttDesc.arrEnabled = %s_tabs_%d_enabled;\n\t\ttDesc.arrDirty = %s_tabs_%d_dirty;\n\t\ttDesc.iItemCount = %d;\n\t\ttDesc.iSelected = %d;\n"
			"\t\ttDesc.iPlacement = %d;\n\t\ttDesc.bScrollable = %d;\n\t\ttDesc.bCloseButtons = %d;\n\t\ttDesc.pFont = pFont;\n\t\ttDesc.fTabWidth = %.6ff;\n\t\ttDesc.fTabHeight = %.6ff;\n"
			"\t\ttDesc.iBackgroundColor = 0x%08Xu;\n\t\ttDesc.iTabColor = 0x%08Xu;\n\t\ttDesc.iActiveColor = 0x%08Xu;\n\t\ttDesc.iTextColor = 0x%08Xu;\n\t\ttDesc.iActiveTextColor = 0x%08Xu;\n"
			"\t\ttDesc.iBorderColor = 0x%08Xu;\n\t\ttDesc.iClientColor = 0x%08Xu;\n\t\tiRet = xuiTabsCreate(pContext, &pView->%s, &tDesc);\n",
			sSymbol, pNode->iId, sSymbol, pNode->iId, sSymbol, pNode->iId, tTabs.iCount,
			uiDesignNodeGetPropertyInt(pNode, "data.selected", 0), uiDesignNodeGetPropertyInt(pNode, "behavior.placement", XUI_TABS_PLACEMENT_TOP),
			uiDesignNodeGetPropertyBool(pNode, "behavior.scrollable", 1), uiDesignNodeGetPropertyBool(pNode, "behavior.closeButtons", 0),
			uiDesignNodeGetPropertyFloat(pNode, "metrics.tabWidth", 92.0f), uiDesignNodeGetPropertyFloat(pNode, "metrics.tabHeight", 30.0f),
			uiDesignNodeGetPropertyColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			uiDesignNodeGetPropertyColor(pNode, "appearance.tabColor", XUI_COLOR_RGBA(234, 241, 248, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.activeColor", XUI_COLOR_WHITE),
			uiDesignNodeGetPropertyColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			uiDesignNodeGetPropertyColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE), sMember);
		break;
	default:
		break;
	}
	__uiDesignCodegenAppendFormat(pSource,
		"\t\tif ( iRet != XUI_OK ) goto fail;\n"
		"\t\tiRet = xuiWidgetSetRect(pView->%s, (xui_rect_t){%d, %d, %d, %d});\n"
		"\t\tif ( iRet != XUI_OK ) { xuiWidgetDestroy(pView->%s); pView->%s = NULL; goto fail; }\n"
		"\t\t(void)xuiWidgetSetVisible(pView->%s, %d);\n\t\t(void)xuiWidgetSetEnabled(pView->%s, %d);\n",
		sMember, pNode->tRect.fX, pNode->tRect.fY, pNode->tRect.fW, pNode->tRect.fH,
		sMember, sMember, sMember, pNode->bVisible, sMember, pNode->bEnabled);
	if ( pNode->iType == UI_DESIGN_NODE_WIDGET ) {
		__uiDesignCodegenAppendFormat(pSource, "\t\t(void)xuiWidgetSetLayoutType(pView->%s, %d);\n", sMember, uiDesignNodeGetPropertyInt(pNode, "layout.type", XUI_LAYOUT_MANUAL));
	} else if ( pNode->iType == UI_DESIGN_NODE_PANEL ) {
		__uiDesignCodegenAppendFormat(pSource, "\t\t(void)xuiWidgetSetLayoutType(xuiPanelGetClientWidget(pView->%s), %d);\n", sMember, uiDesignNodeGetPropertyInt(pNode, "layout.type", XUI_LAYOUT_MANUAL));
	}
	if ( pNode->iType == UI_DESIGN_NODE_BUTTON ) {
		sAction = uiDesignNodeGetProperty(pNode, "event.onClick", "");
		if ( sAction[0] != 0 ) __uiDesignCodegenAppendFormat(pSource, "\t\t(void)xuiButtonSetClick(pView->%s, %s_button_%d_click, pView);\n", sMember, sSymbol, pNode->iId);
	}
	pParent = pNode->iParentId != 0 ? uiDesignModelGetNodeConst(pModel, pNode->iParentId) : NULL;
	if ( pParent == NULL ) {
		__uiDesignCodegenAppendFormat(pSource,
			"\t\tif ( pView->pRoot == NULL ) pView->pRoot = pView->%s;\n"
			"\t\tif ( pParent != NULL ) { iRet = xuiWidgetAddChild(pParent, pView->%s); if ( iRet != XUI_OK ) { xuiWidgetDestroy(pView->%s); pView->%s = NULL; goto fail; } }\n",
			sMember, sMember, sMember, sMember);
	} else {
		__uiDesignCodegenMember(sParentMember, sizeof(sParentMember), pParent);
		if ( pParent->iType == UI_DESIGN_NODE_PANEL ) {
			__uiDesignCodegenAppendFormat(pSource, "\t\tiRet = xuiPanelAddChild(pView->%s, pView->%s);\n", sParentMember, sMember);
		} else if ( pParent->iType == UI_DESIGN_NODE_TABS ) {
			__uiDesignCodegenTabs(pParent, &tTabs);
			iPage = uiDesignNodeGetPropertyInt(pNode, "layout.tabPage", uiDesignNodeGetPropertyInt(pParent, "data.selected", 0));
			if ( iPage < 0 ) iPage = 0;
			if ( iPage >= tTabs.iCount ) iPage = tTabs.iCount - 1;
			__uiDesignCodegenAppendFormat(pSource, "\t\tiRet = xuiTabsAddPageChild(pView->%s, %d, pView->%s);\n", sParentMember, iPage, sMember);
		} else {
			__uiDesignCodegenAppendFormat(pSource, "\t\tiRet = xuiWidgetAddChild(pView->%s, pView->%s);\n", sParentMember, sMember);
		}
		__uiDesignCodegenAppendFormat(pSource, "\t\tif ( iRet != XUI_OK ) { xuiWidgetDestroy(pView->%s); pView->%s = NULL; goto fail; }\n", sMember, sMember);
	}
	__uiDesignCodegenAppendFormat(pSource, "\t}\n\n");
}

static void __uiDesignCodegenEmitSource(ui_design_codegen_builder_t* pSource, const ui_design_model_t* pModel,
	const int* arrOrder, const char* sSymbol, const char* sHeaderBase)
{
	char sMember[64];
	int i;

	__uiDesignCodegenAppendFormat(pSource, "/* Generated by XUI UI Design. */\n#include ");
	__uiDesignCodegenAppendCString(pSource, sHeaderBase);
	__uiDesignCodegenAppendFormat(pSource, "\n\n#include <string.h>\n\n");
	__uiDesignCodegenEmitTabsData(pSource, pModel, sSymbol);
	__uiDesignCodegenEmitCallbacks(pSource, pModel, sSymbol);
	__uiDesignCodegenAppendFormat(pSource, "void %s_destroy(%s_view_t* pView)\n{\n\tif ( pView == NULL ) return;\n", sSymbol, sSymbol);
	for ( i = pModel->iNodeCount - 1; i >= 0; --i ) {
		if ( pModel->arrNodes[i].iParentId != 0 ) continue;
		__uiDesignCodegenMember(sMember, sizeof(sMember), &pModel->arrNodes[i]);
		__uiDesignCodegenAppendFormat(pSource, "\tif ( pView->%s != NULL ) xuiWidgetDestroy(pView->%s);\n", sMember, sMember);
	}
	__uiDesignCodegenAppendFormat(pSource, "\tmemset(pView, 0, sizeof(*pView));\n}\n\n");
	__uiDesignCodegenAppendFormat(pSource,
		"int %s_create(xui_context pContext, xui_widget pParent, xui_font pFont, const %s_callbacks_t* pCallbacks, %s_view_t* pView)\n"
		"{\n\tint iRet = XUI_OK;\n\n\tif ( pContext == NULL || pView == NULL ) return XUI_ERROR_INVALID_ARGUMENT;\n\t(void)pFont;\n\tif ( 0 ) goto fail;\n\tmemset(pView, 0, sizeof(*pView));\n"
		"\tif ( pCallbacks != NULL ) pView->tCallbacks = *pCallbacks;\n\n",
		sSymbol, sSymbol, sSymbol);
	for ( i = 0; i < pModel->iNodeCount; ++i ) __uiDesignCodegenEmitCreateNode(pSource, pModel, &pModel->arrNodes[arrOrder[i]], sSymbol);
	__uiDesignCodegenAppendFormat(pSource, "\treturn XUI_OK;\n\nfail:\n\t%s_destroy(pView);\n\treturn iRet;\n}\n", sSymbol);
}

static int __uiDesignCodegenWrite(const char* sPath, const char* sData, size_t iLength)
{
	FILE* pFile;
	size_t iWritten;

	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) return XUI_ERROR;
	iWritten = fwrite(sData, 1u, iLength, pFile);
	if ( fclose(pFile) != 0 || iWritten != iLength ) return XUI_ERROR;
	return XUI_OK;
}

int uiDesignCodegenExport(const ui_design_model_t* pModel, const char* sCPath,
	const char* sSymbolPrefix, char* sHeaderPath, int iHeaderPathCapacity)
{
	ui_design_codegen_builder_t tHeader;
	ui_design_codegen_builder_t tSource;
	char sLocalHeader[1024];
	char sBaseSymbol[UI_DESIGN_CODEGEN_SYMBOL_CAPACITY];
	char sSymbol[UI_DESIGN_CODEGEN_SYMBOL_CAPACITY];
	char sGuard[UI_DESIGN_CODEGEN_SYMBOL_CAPACITY];
	char sHeaderBase[1024];
	const char* sFileBase;
	const char* sDot;
	int arrOrder[UI_DESIGN_MAX_NODES];
	int iRet;
	size_t iLength;

	if ( pModel == NULL || sCPath == NULL || sCPath[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pModel->iNodeCount < 0 || pModel->iNodeCount > UI_DESIGN_MAX_NODES ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignCodegenOrder(pModel, arrOrder);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignCodegenHeaderPath(sCPath, sLocalHeader, sizeof(sLocalHeader));
	if ( iRet != XUI_OK ) return iRet;
	if ( sHeaderPath != NULL ) {
		if ( iHeaderPathCapacity <= 0 || strlen(sLocalHeader) + 1u > (size_t)iHeaderPathCapacity ) return XUI_ERROR_BUFFER_TOO_SMALL;
		strcpy(sHeaderPath, sLocalHeader);
	}
	if ( sSymbolPrefix == NULL || sSymbolPrefix[0] == 0 ) {
		sFileBase = __uiDesignCodegenBaseName(sCPath);
		sDot = strrchr(sFileBase, '.');
		iLength = sDot != NULL ? (size_t)(sDot - sFileBase) : strlen(sFileBase);
		if ( iLength >= sizeof(sBaseSymbol) ) iLength = sizeof(sBaseSymbol) - 1u;
		memcpy(sBaseSymbol, sFileBase, iLength);
		sBaseSymbol[iLength] = 0;
		sSymbolPrefix = sBaseSymbol;
	}
	__uiDesignCodegenSanitize(sSymbolPrefix, sSymbol, sizeof(sSymbol), 0);
	__uiDesignCodegenSanitize(sSymbolPrefix, sGuard, sizeof(sGuard), 1);
	sFileBase = __uiDesignCodegenBaseName(sLocalHeader);
	if ( strlen(sFileBase) + 1u > sizeof(sHeaderBase) ) return XUI_ERROR_BUFFER_TOO_SMALL;
	strcpy(sHeaderBase, sFileBase);
	memset(&tHeader, 0, sizeof(tHeader));
	memset(&tSource, 0, sizeof(tSource));
	tHeader.iError = XUI_OK;
	tSource.iError = XUI_OK;
	__uiDesignCodegenEmitHeader(&tHeader, pModel, sSymbol, sGuard);
	__uiDesignCodegenEmitSource(&tSource, pModel, arrOrder, sSymbol, sHeaderBase);
	if ( tHeader.iError != XUI_OK || tSource.iError != XUI_OK ) {
		iRet = tHeader.iError != XUI_OK ? tHeader.iError : tSource.iError;
		free(tHeader.sData);
		free(tSource.sData);
		return iRet;
	}
	iRet = __uiDesignCodegenWrite(sLocalHeader, tHeader.sData, tHeader.iLength);
	if ( iRet == XUI_OK ) iRet = __uiDesignCodegenWrite(sCPath, tSource.sData, tSource.iLength);
	free(tHeader.sData);
	free(tSource.sData);
	return iRet;
}
