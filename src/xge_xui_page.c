static void __xgeXuiPageSetError(xge_xui_page_t* pPage, const char* sFmt, ...)
{
	va_list args;

	if ( pPage == NULL ) {
		return;
	}
	va_start(args, sFmt);
	vsnprintf(pPage->sError, sizeof(pPage->sError), sFmt, args);
	pPage->sError[sizeof(pPage->sError) - 1] = 0;
	va_end(args);
}

static const char* __xgeXuiPageURI(xge_xui_page_t* pPage)
{
	if ( (pPage == NULL) || (pPage->tLoader.sURI == NULL) ) {
		return "<memory>";
	}
	return pPage->tLoader.sURI;
}

static void __xgeXuiPageSetPathError(xge_xui_page_t* pPage, const char* sPath, const char* sFmt, ...)
{
	va_list args;
	char sReason[160];

	if ( pPage == NULL ) {
		return;
	}
	va_start(args, sFmt);
	vsnprintf(sReason, sizeof(sReason), sFmt, args);
	sReason[sizeof(sReason) - 1] = 0;
	va_end(args);
	__xgeXuiPageSetError(pPage, "%s: %s: %s", __xgeXuiPageURI(pPage), (sPath != NULL) ? sPath : "<root>", sReason);
}

#define XGE_XUI_PAGE_MODEL_BIND_LABEL_TEXT	1
#define XGE_XUI_PAGE_MODEL_BIND_INPUT_VALUE	2
#define XGE_XUI_PAGE_MODEL_BIND_IMAGE_SRC	3

static xvalue __xgeXuiPageTableGet(xvalue pTable, const char* sKey)
{
	if ( (pTable == NULL) || (xvoType(pTable) != XVO_DT_TABLE) || (sKey == NULL) ) {
		return NULL;
	}
	return xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
}

static int __xgeXuiPageURIHasScheme(const char* sURI)
{
	const char* s;

	if ( sURI == NULL ) {
		return 0;
	}
	for ( s = sURI; *s != 0; s++ ) {
		if ( (s[0] == ':') && (s[1] == '/') && (s[2] == '/') ) {
			return 1;
		}
		if ( (*s == '/') || (*s == '\\') ) {
			return 0;
		}
	}
	return 0;
}

static void __xgeXuiPageNormalizeURI(char* sURI)
{
	char* sRead;
	char* sWrite;

	if ( sURI == NULL ) {
		return;
	}
	for ( sRead = sURI, sWrite = sURI; *sRead != 0; sRead++ ) {
		*sWrite++ = (*sRead == '\\') ? '/' : *sRead;
	}
	*sWrite = 0;
}

static void __xgeXuiPageResolveImportURI(const char* sBaseURI, const char* sImport, char* sOut, int iOutSize)
{
	const char* sEnd;
	int iBaseLen;

	if ( (sOut == NULL) || (iOutSize <= 0) ) {
		return;
	}
	sOut[0] = 0;
	if ( sImport == NULL ) {
		return;
	}
	if ( __xgeXuiPageURIHasScheme(sImport) || (sBaseURI == NULL) || (sBaseURI[0] == 0) ) {
		snprintf(sOut, (size_t)iOutSize, "%s", sImport);
		__xgeXuiPageNormalizeURI(sOut);
		return;
	}
	sEnd = strrchr(sBaseURI, '/');
	if ( sEnd == NULL ) {
		sEnd = strrchr(sBaseURI, '\\');
	}
	if ( sEnd == NULL ) {
		snprintf(sOut, (size_t)iOutSize, "%s", sImport);
		__xgeXuiPageNormalizeURI(sOut);
		return;
	}
	iBaseLen = (int)(sEnd - sBaseURI) + 1;
	if ( iBaseLen >= iOutSize ) {
		iBaseLen = iOutSize - 1;
	}
	memcpy(sOut, sBaseURI, (size_t)iBaseLen);
	sOut[iBaseLen] = 0;
	snprintf(sOut + iBaseLen, (size_t)(iOutSize - iBaseLen), "%s", sImport);
	__xgeXuiPageNormalizeURI(sOut);
}

static int __xgeXuiPageImportSeen(xge_xui_page_t* pPage, const char* sURI)
{
	int i;

	if ( (pPage == NULL) || (sURI == NULL) ) {
		return 0;
	}
	for ( i = 0; i < pPage->iImportCount; i++ ) {
		if ( strcmp(pPage->arrImportURI[i], sURI) == 0 ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeXuiPageImportInStack(const char* sURI, const char* const* arrStack, int iStackCount)
{
	int i;

	if ( (sURI == NULL) || (arrStack == NULL) ) {
		return 0;
	}
	for ( i = 0; i < iStackCount; i++ ) {
		if ( (arrStack[i] != NULL) && (strcmp(arrStack[i], sURI) == 0) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeXuiPageMergeSection(xge_xui_page_t* pPage, xvalue* ppMerged, xvalue pDoc, const char* sName)
{
	xvalue pSection;

	pSection = __xgeXuiPageTableGet(pDoc, sName);
	if ( (pSection == NULL) || (xvoType(pSection) == XVO_DT_NULL) ) {
		return XGE_OK;
	}
	if ( xvoType(pSection) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, sName, "expected object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( *ppMerged == NULL ) {
		*ppMerged = xvoCreateTable();
		if ( *ppMerged == NULL ) {
			__xgeXuiPageSetPathError(pPage, sName, "merge table allocation failed");
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xvoTableMerge(*ppMerged, pSection, TRUE) ) {
		__xgeXuiPageSetPathError(pPage, sName, "merge failed");
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int __xgeXuiPageValueExists(xvalue pVal);
static int __xgeXuiPageBuildWidget(xge_xui_page_t* pPage, xvalue pStyles, xvalue pNode, const char* sPath, xge_xui_widget* ppWidget);

static int __xgeXuiPageParseModelBinding(const char* sText, char* sKey, int iKeySize)
{
	size_t iLen;

	if ( (sText == NULL) || (sKey == NULL) || (iKeySize <= 0) ) {
		return 0;
	}
	sKey[0] = 0;
	iLen = strlen(sText);
	if ( (iLen < 4) || (sText[0] != '$') || (sText[1] != '{') || (sText[iLen - 1] != '}') ) {
		return 0;
	}
	iLen -= 3;
	if ( (iLen == 0) || (iLen >= (size_t)iKeySize) ) {
		return 0;
	}
	memcpy(sKey, sText + 2, iLen);
	sKey[iLen] = 0;
	for ( iLen = 0; sKey[iLen] != 0; iLen++ ) {
		char c = sKey[iLen];
		if ( !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '_') || (c == '.') || (c == '-')) ) {
			sKey[0] = 0;
			return 0;
		}
	}
	return 1;
}

static int __xgeXuiPageAddModelBinding(xge_xui_page_t* pPage, xge_xui_widget pWidget, void* pControl, int iKind, int iControlIndex, const char* sKey, const char* sPath)
{
	xge_xui_page_model_binding_t* pBinding;

	if ( (pPage == NULL) || (pWidget == NULL) || (pControl == NULL) || (sKey == NULL) || (sKey[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pPage->iModelBindingCount >= XGE_XUI_PAGE_MODEL_BINDING_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "model binding capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBinding = &pPage->arrModelBinding[pPage->iModelBindingCount++];
	memset(pBinding, 0, sizeof(*pBinding));
	pBinding->pWidget = pWidget;
	pBinding->pControl = pControl;
	pBinding->iKind = iKind;
	pBinding->iControlIndex = iControlIndex;
	snprintf(pBinding->sKey, sizeof(pBinding->sKey), "%s", sKey);
	pBinding->sKey[sizeof(pBinding->sKey) - 1] = 0;
	return XGE_OK;
}

static xvalue __xgeXuiPageNodeGetStyled(xvalue pNode, xvalue pStyle, const char* sKey)
{
	xvalue pVal;

	pVal = __xgeXuiPageTableGet(pNode, sKey);
	if ( (pVal != NULL) && (xvoType(pVal) != XVO_DT_NULL) ) {
		return pVal;
	}
	return __xgeXuiPageTableGet(pStyle, sKey);
}

static xvalue __xgeXuiPageTokenInTableSection(xge_xui_page_t* pPage, xvalue pTokens, const char* sSection, const char* sName, const char* sPath)
{
	xvalue pSectionTable;

	if ( (sSection == NULL) || (sName == NULL) ) {
		return NULL;
	}
	pSectionTable = __xgeXuiPageTableGet(pTokens, sSection);
	if ( !__xgeXuiPageValueExists(pSectionTable) ) {
		return NULL;
	}
	if ( xvoType(pSectionTable) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, sPath, "token section '%s' must be object", sSection);
		return NULL;
	}
	return __xgeXuiPageTableGet(pSectionTable, sName);
}

static xvalue __xgeXuiPageTokenInSection(xge_xui_page_t* pPage, const char* sSection, const char* sName, const char* sPath)
{
	return __xgeXuiPageTokenInTableSection(pPage, (xvalue)((pPage != NULL) ? pPage->pMergedTokens : NULL), sSection, sName, sPath);
}

static xvalue __xgeXuiPageContextTokenInSection(xge_xui_page_t* pPage, const char* sSection, const char* sName, const char* sPath)
{
	xge_xui_context pContext;

	pContext = (pPage != NULL) ? pPage->pContext : NULL;
	return __xgeXuiPageTokenInTableSection(pPage, (xvalue)((pContext != NULL) ? pContext->pRegisteredTokens : NULL), sSection, sName, sPath);
}

static xvalue __xgeXuiPageResolveTokenValue(xge_xui_page_t* pPage, xvalue pVal, const char* sPath)
{
	xvalue pTokens;
	xvalue pToken;
	const char* sText;
	const char* sName;

	if ( !__xgeXuiPageValueExists(pVal) || (xvoType(pVal) != XVO_DT_TEXT) ) {
		return pVal;
	}
	sText = (const char*)xvoGetText(pVal);
	if ( (sText == NULL) || (sText[0] != '@') ) {
		return pVal;
	}
	sName = sText + 1;
	if ( sName[0] == 0 ) {
		__xgeXuiPageSetPathError(pPage, sPath, "empty token reference");
		return NULL;
	}
	pTokens = (xvalue)((pPage != NULL) ? pPage->pMergedTokens : NULL);
	if ( __xgeXuiPageValueExists(pTokens) && (xvoType(pTokens) != XVO_DT_TABLE) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tokens must be object");
		return NULL;
	}
	if ( strncmp(sName, "colors.", 7) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "colors", sName + 7, sPath);
	} else if ( strncmp(sName, "color.", 6) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "colors", sName + 6, sPath);
	} else if ( strncmp(sName, "spacing.", 8) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "spacing", sName + 8, sPath);
	} else if ( strncmp(sName, "space.", 6) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "spacing", sName + 6, sPath);
	} else if ( strncmp(sName, "fonts.", 6) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "fonts", sName + 6, sPath);
	} else if ( strncmp(sName, "font.", 5) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "fonts", sName + 5, sPath);
	} else if ( strncmp(sName, "textures.", 9) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "textures", sName + 9, sPath);
	} else if ( strncmp(sName, "texture.", 8) == 0 ) {
		pToken = __xgeXuiPageTokenInSection(pPage, "textures", sName + 8, sPath);
	} else {
		pToken = __xgeXuiPageTableGet(pTokens, sName);
		if ( !__xgeXuiPageValueExists(pToken) ) {
			pToken = __xgeXuiPageTokenInSection(pPage, "colors", sName, sPath);
		}
		if ( !__xgeXuiPageValueExists(pToken) && (pPage->sError[0] == 0) ) {
			pToken = __xgeXuiPageTokenInSection(pPage, "spacing", sName, sPath);
		}
		if ( !__xgeXuiPageValueExists(pToken) && (pPage->sError[0] == 0) ) {
			pToken = __xgeXuiPageTokenInSection(pPage, "fonts", sName, sPath);
		}
		if ( !__xgeXuiPageValueExists(pToken) && (pPage->sError[0] == 0) ) {
			pToken = __xgeXuiPageTokenInSection(pPage, "textures", sName, sPath);
		}
	}
	if ( pPage->sError[0] != 0 ) {
		return NULL;
	}
	if ( !__xgeXuiPageValueExists(pToken) ) {
		if ( strncmp(sName, "colors.", 7) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "colors", sName + 7, sPath);
		} else if ( strncmp(sName, "color.", 6) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "colors", sName + 6, sPath);
		} else if ( strncmp(sName, "spacing.", 8) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "spacing", sName + 8, sPath);
		} else if ( strncmp(sName, "space.", 6) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "spacing", sName + 6, sPath);
		} else if ( strncmp(sName, "fonts.", 6) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "fonts", sName + 6, sPath);
		} else if ( strncmp(sName, "font.", 5) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "fonts", sName + 5, sPath);
		} else if ( strncmp(sName, "textures.", 9) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "textures", sName + 9, sPath);
		} else if ( strncmp(sName, "texture.", 8) == 0 ) {
			pToken = __xgeXuiPageContextTokenInSection(pPage, "textures", sName + 8, sPath);
		} else {
			xvalue pContextTokens = (xvalue)((pPage != NULL && pPage->pContext != NULL) ? pPage->pContext->pRegisteredTokens : NULL);
			pToken = __xgeXuiPageTableGet(pContextTokens, sName);
			if ( !__xgeXuiPageValueExists(pToken) ) {
				pToken = __xgeXuiPageContextTokenInSection(pPage, "colors", sName, sPath);
			}
			if ( !__xgeXuiPageValueExists(pToken) && (pPage->sError[0] == 0) ) {
				pToken = __xgeXuiPageContextTokenInSection(pPage, "spacing", sName, sPath);
			}
			if ( !__xgeXuiPageValueExists(pToken) && (pPage->sError[0] == 0) ) {
				pToken = __xgeXuiPageContextTokenInSection(pPage, "fonts", sName, sPath);
			}
			if ( !__xgeXuiPageValueExists(pToken) && (pPage->sError[0] == 0) ) {
				pToken = __xgeXuiPageContextTokenInSection(pPage, "textures", sName, sPath);
			}
		}
		if ( pPage->sError[0] != 0 ) {
			return NULL;
		}
		if ( !__xgeXuiPageValueExists(pToken) ) {
			__xgeXuiPageSetPathError(pPage, sPath, "missing token '%s'", sText);
			return NULL;
		}
	}
	return pToken;
}

static xvalue __xgeXuiPageNodeGetStyledToken(xge_xui_page_t* pPage, xvalue pNode, xvalue pStyle, const char* sKey, const char* sPath)
{
	return __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageNodeGetStyled(pNode, pStyle, sKey), sPath);
}

static int __xgeXuiPageParseNumericId(const char* sText, int* pId)
{
	const char* s;
	int iId;

	if ( sText == NULL ) {
		return 0;
	}
	iId = 0;
	for ( s = sText; *s != 0; s++ ) {
		if ( (*s < '0') || (*s > '9') ) {
			return 0;
		}
		iId = (iId * 10) + (*s - '0');
	}
	if ( s == sText ) {
		return 0;
	}
	if ( pId != NULL ) {
		*pId = iId;
	}
	return 1;
}

static int __xgeXuiPageIndexContains(xge_xui_page_t* pPage, const char* sKey, int iNumericId, int bNumeric)
{
	int i;

	if ( (pPage == NULL) || (sKey == NULL) ) {
		return 0;
	}
	for ( i = 0; i < pPage->iIndexCount; i++ ) {
		if ( (sKey != NULL) && (pPage->arrIndex[i].sKey != NULL) && (strcmp(pPage->arrIndex[i].sKey, sKey) == 0) ) {
			return 1;
		}
		if ( bNumeric && pPage->arrIndex[i].bNumeric && (pPage->arrIndex[i].iNumericId == iNumericId) ) {
			return 1;
		}
	}
	return 0;
}

static void __xgeXuiPageIndexAdd(xge_xui_page_t* pPage, const char* sKey, int iNumericId, int bNumeric, xge_xui_widget pWidget)
{
	xge_xui_page_index_t* pEntry;

	if ( (pPage == NULL) || (pWidget == NULL) || (((sKey == NULL) || (sKey[0] == 0)) && !bNumeric) ) {
		return;
	}
	if ( __xgeXuiPageIndexContains(pPage, sKey, iNumericId, bNumeric) ) {
		return;
	}
	if ( pPage->iIndexCount >= XGE_XUI_PAGE_INDEX_CAPACITY ) {
		pPage->bIndexOverflow = 1;
		return;
	}
	pEntry = &pPage->arrIndex[pPage->iIndexCount++];
	pEntry->sKey = sKey;
	pEntry->iNumericId = iNumericId;
	pEntry->bNumeric = bNumeric;
	pEntry->pWidget = pWidget;
}

static void __xgeXuiPageIndexWidget(xge_xui_page_t* pPage, xge_xui_widget pWidget)
{
	if ( (pPage == NULL) || (pWidget == NULL) ) {
		return;
	}
	if ( (pWidget->sName != NULL) && (pWidget->sName[0] != 0) ) {
		int iNumericId = 0;
		int bNumeric;

		bNumeric = __xgeXuiPageParseNumericId(pWidget->sName, &iNumericId);
		__xgeXuiPageIndexAdd(pPage, pWidget->sName, iNumericId, bNumeric, pWidget);
	}
	if ( pWidget->iId != 0 ) {
		__xgeXuiPageIndexAdd(pPage, NULL, pWidget->iId, 1, pWidget);
	}
}

static xge_xui_widget __xgeXuiPageIndexFind(xge_xui_page_t* pPage, const char* sId, int iId, int bUseNumeric)
{
	int i;

	if ( (pPage == NULL) || (sId == NULL) ) {
		return NULL;
	}
	for ( i = 0; i < pPage->iIndexCount; i++ ) {
		if ( (pPage->arrIndex[i].sKey != NULL) && (strcmp(pPage->arrIndex[i].sKey, sId) == 0) ) {
			return pPage->arrIndex[i].pWidget;
		}
		if ( bUseNumeric && pPage->arrIndex[i].bNumeric && (pPage->arrIndex[i].iNumericId == iId) ) {
			return pPage->arrIndex[i].pWidget;
		}
	}
	return NULL;
}

static int __xgeXuiPageTypeToLayout(const char* sType)
{
	if ( (sType == NULL) || (strcmp(sType, "panel") == 0) || (strcmp(sType, "absolute") == 0) ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( (strcmp(sType, "scroll") == 0) || (strcmp(sType, "scrollView") == 0) ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "virtualList") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "treeView") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "tableView") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "propertyGrid") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "breadcrumb") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "accordion") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "toast") == 0 ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	if ( strcmp(sType, "row") == 0 ) {
		return XGE_XUI_LAYOUT_ROW;
	}
	if ( strcmp(sType, "column") == 0 ) {
		return XGE_XUI_LAYOUT_COLUMN;
	}
	if ( strcmp(sType, "stack") == 0 ) {
		return XGE_XUI_LAYOUT_STACK;
	}
	if ( strcmp(sType, "grid") == 0 ) {
		return XGE_XUI_LAYOUT_GRID;
	}
	if ( strcmp(sType, "dock") == 0 ) {
		return XGE_XUI_LAYOUT_DOCK;
	}
	if ( (strcmp(sType, "button") == 0) || (strcmp(sType, "image") == 0) || (strcmp(sType, "input") == 0) || (strcmp(sType, "numericInput") == 0) || (strcmp(sType, "checkbox") == 0) || (strcmp(sType, "radio") == 0) || (strcmp(sType, "switch") == 0) || (strcmp(sType, "slider") == 0) || (strcmp(sType, "progress") == 0) || (strcmp(sType, "tabs") == 0) || (strcmp(sType, "toolbar") == 0) || (strcmp(sType, "statusBar") == 0) || (strcmp(sType, "treeView") == 0) || (strcmp(sType, "tableView") == 0) || (strcmp(sType, "propertyGrid") == 0) || (strcmp(sType, "breadcrumb") == 0) || (strcmp(sType, "accordion") == 0) || (strcmp(sType, "toast") == 0) || (strcmp(sType, "comboBox") == 0) || (strcmp(sType, "popup") == 0) || (strcmp(sType, "tooltip") == 0) || (strcmp(sType, "menu") == 0) || (strcmp(sType, "dialog") == 0) || (strcmp(sType, "messageBox") == 0) || (strcmp(sType, "label") == 0) || (strcmp(sType, "separator") == 0) ) {
		return XGE_XUI_LAYOUT_ABSOLUTE;
	}
	return XGE_XUI_LAYOUT_ABSOLUTE;
}

static int __xgeXuiPageTypeKnown(const char* sType)
{
	if ( sType == NULL ) {
		return 1;
	}
	return (strcmp(sType, "panel") == 0) || (strcmp(sType, "absolute") == 0) || (strcmp(sType, "row") == 0) || (strcmp(sType, "column") == 0) || (strcmp(sType, "stack") == 0) || (strcmp(sType, "grid") == 0) || (strcmp(sType, "dock") == 0) || (strcmp(sType, "scroll") == 0) || (strcmp(sType, "scrollView") == 0) || (strcmp(sType, "virtualList") == 0) || (strcmp(sType, "treeView") == 0) || (strcmp(sType, "tableView") == 0) || (strcmp(sType, "propertyGrid") == 0) || (strcmp(sType, "breadcrumb") == 0) || (strcmp(sType, "accordion") == 0) || (strcmp(sType, "toast") == 0) || (strcmp(sType, "button") == 0) || (strcmp(sType, "image") == 0) || (strcmp(sType, "input") == 0) || (strcmp(sType, "numericInput") == 0) || (strcmp(sType, "checkbox") == 0) || (strcmp(sType, "radio") == 0) || (strcmp(sType, "switch") == 0) || (strcmp(sType, "slider") == 0) || (strcmp(sType, "progress") == 0) || (strcmp(sType, "tabs") == 0) || (strcmp(sType, "toolbar") == 0) || (strcmp(sType, "statusBar") == 0) || (strcmp(sType, "comboBox") == 0) || (strcmp(sType, "popup") == 0) || (strcmp(sType, "tooltip") == 0) || (strcmp(sType, "menu") == 0) || (strcmp(sType, "dialog") == 0) || (strcmp(sType, "messageBox") == 0) || (strcmp(sType, "label") == 0) || (strcmp(sType, "separator") == 0);
}

static int __xgeXuiPageTextToAlign(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "start") == 0 ) {
		return XGE_XUI_ALIGN_START;
	}
	if ( strcmp(sText, "center") == 0 ) {
		return XGE_XUI_ALIGN_CENTER;
	}
	if ( strcmp(sText, "end") == 0 ) {
		return XGE_XUI_ALIGN_END;
	}
	if ( strcmp(sText, "stretch") == 0 ) {
		return XGE_XUI_ALIGN_STRETCH;
	}
	return iDefault;
}

static int __xgeXuiPageTextToDock(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "left") == 0 ) {
		return XGE_XUI_DOCK_LEFT;
	}
	if ( strcmp(sText, "top") == 0 ) {
		return XGE_XUI_DOCK_TOP;
	}
	if ( strcmp(sText, "right") == 0 ) {
		return XGE_XUI_DOCK_RIGHT;
	}
	if ( strcmp(sText, "bottom") == 0 ) {
		return XGE_XUI_DOCK_BOTTOM;
	}
	if ( (strcmp(sText, "fill") == 0) || (strcmp(sText, "center") == 0) ) {
		return XGE_XUI_DOCK_FILL;
	}
	return iDefault;
}

static int __xgeXuiPageTextToJustify(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "start") == 0 ) {
		return XGE_XUI_JUSTIFY_START;
	}
	if ( strcmp(sText, "center") == 0 ) {
		return XGE_XUI_JUSTIFY_CENTER;
	}
	if ( strcmp(sText, "end") == 0 ) {
		return XGE_XUI_JUSTIFY_END;
	}
	if ( strcmp(sText, "space-between") == 0 ) {
		return XGE_XUI_JUSTIFY_SPACE_BETWEEN;
	}
	return iDefault;
}

static uint32_t __xgeXuiPageTextToTextAlign(const char* sText, uint32_t iFlags)
{
	iFlags &= ~(uint32_t)(XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_RIGHT);
	if ( sText == NULL ) {
		return iFlags;
	}
	if ( (strcmp(sText, "center") == 0) || (strcmp(sText, "middle") == 0) ) {
		return iFlags | XGE_TEXT_ALIGN_CENTER;
	}
	if ( (strcmp(sText, "right") == 0) || (strcmp(sText, "end") == 0) ) {
		return iFlags | XGE_TEXT_ALIGN_RIGHT;
	}
	return iFlags | XGE_TEXT_ALIGN_LEFT;
}

static uint32_t __xgeXuiPageTextToTextVAlign(const char* sText, uint32_t iFlags)
{
	iFlags &= ~(uint32_t)(XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_ALIGN_BOTTOM);
	if ( sText == NULL ) {
		return iFlags;
	}
	if ( (strcmp(sText, "middle") == 0) || (strcmp(sText, "center") == 0) ) {
		return iFlags | XGE_TEXT_ALIGN_MIDDLE;
	}
	if ( (strcmp(sText, "bottom") == 0) || (strcmp(sText, "end") == 0) ) {
		return iFlags | XGE_TEXT_ALIGN_BOTTOM;
	}
	return iFlags | XGE_TEXT_ALIGN_TOP;
}

static float __xgeXuiPageValueToFloat(xvalue pVal, float fDefault)
{
	int iType;

	iType = xvoType(pVal);
	if ( iType == XVO_DT_INT ) {
		return (float)xvoGetInt(pVal);
	}
	if ( iType == XVO_DT_FLOAT ) {
		return (float)xvoGetFloat(pVal);
	}
	if ( iType == XVO_DT_TEXT ) {
		return (float)atof((const char*)xvoGetText(pVal));
	}
	return fDefault;
}

static int __xgeXuiPageValueToBool(xvalue pVal, int iDefault)
{
	int iType;

	iType = xvoType(pVal);
	if ( iType == XVO_DT_BOOL ) {
		return xvoGetBool(pVal) ? 1 : 0;
	}
	if ( iType == XVO_DT_INT ) {
		return (xvoGetInt(pVal) != 0) ? 1 : 0;
	}
	if ( iType == XVO_DT_TEXT ) {
		const char* sText = (const char*)xvoGetText(pVal);
		if ( sText == NULL ) {
			return iDefault;
		}
		if ( (strcmp(sText, "true") == 0) || (strcmp(sText, "yes") == 0) || (strcmp(sText, "1") == 0) ) {
			return 1;
		}
		if ( (strcmp(sText, "false") == 0) || (strcmp(sText, "no") == 0) || (strcmp(sText, "0") == 0) ) {
			return 0;
		}
	}
	return iDefault;
}

static xge_xui_size_t __xgeXuiPageValueToSize(xvalue pVal, xge_xui_size_t tDefault)
{
	const char* sText;
	char* sEnd;
	double fValue;
	int iType;

	iType = xvoType(pVal);
	if ( iType == XVO_DT_INT ) {
		return xgeXuiSizePx((float)xvoGetInt(pVal));
	}
	if ( iType == XVO_DT_FLOAT ) {
		return xgeXuiSizePx((float)xvoGetFloat(pVal));
	}
	if ( iType == XVO_DT_TABLE ) {
		xvalue pUnit = __xgeXuiPageTableGet(pVal, "unit");
		xvalue pValue = __xgeXuiPageTableGet(pVal, "value");
		const char* sUnit = (xvoType(pUnit) == XVO_DT_TEXT) ? (const char*)xvoGetText(pUnit) : NULL;
		float fObjValue = __xgeXuiPageValueToFloat(pValue, tDefault.fValue);
		if ( sUnit == NULL ) {
			return tDefault;
		}
		if ( strcmp(sUnit, "px") == 0 ) {
			return xgeXuiSizePx(fObjValue);
		}
		if ( strcmp(sUnit, "dip") == 0 ) {
			return xgeXuiSizeDip(fObjValue);
		}
		if ( (strcmp(sUnit, "%") == 0) || (strcmp(sUnit, "percent") == 0) ) {
			return xgeXuiSizePercent(fObjValue);
		}
		if ( strcmp(sUnit, "grow") == 0 ) {
			return xgeXuiSizeGrow(fObjValue);
		}
		if ( strcmp(sUnit, "content") == 0 ) {
			return xgeXuiSizeContent();
		}
		return tDefault;
	}
	if ( iType != XVO_DT_TEXT ) {
		return tDefault;
	}
	sText = (const char*)xvoGetText(pVal);
	if ( sText == NULL ) {
		return tDefault;
	}
	if ( strcmp(sText, "content") == 0 ) {
		return xgeXuiSizeContent();
	}
	if ( strcmp(sText, "grow") == 0 ) {
		return xgeXuiSizeGrow(1.0f);
	}
	if ( strncmp(sText, "grow:", 5) == 0 ) {
		return xgeXuiSizeGrow((float)atof(sText + 5));
	}
	fValue = strtod(sText, &sEnd);
	if ( sEnd == sText ) {
		return tDefault;
	}
	if ( strcmp(sEnd, "%") == 0 ) {
		return xgeXuiSizePercent((float)fValue);
	}
	if ( strcmp(sEnd, "dip") == 0 ) {
		return xgeXuiSizeDip((float)fValue);
	}
	if ( strcmp(sEnd, "grow") == 0 ) {
		return xgeXuiSizeGrow((float)fValue);
	}
	return xgeXuiSizePx((float)fValue);
}

static int __xgeXuiPageValueExists(xvalue pVal)
{
	return (pVal != NULL) && (xvoType(pVal) != XVO_DT_NULL);
}

static xge_xui_edges_t __xgeXuiPageValueToEdges(xvalue pVal, xge_xui_edges_t tDefault)
{
	xge_xui_edges_t tEdges;
	xge_xui_size_t tA;
	xge_xui_size_t tB;
	uint32 iCount;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return tDefault;
	}
	tEdges = tDefault;
	if ( (xvoType(pVal) == XVO_DT_INT) || (xvoType(pVal) == XVO_DT_FLOAT) || (xvoType(pVal) == XVO_DT_TEXT) || (xvoType(pVal) == XVO_DT_TABLE) ) {
		tA = __xgeXuiPageValueToSize(pVal, xgeXuiSizePx(0.0f));
		tEdges.tLeft = tA;
		tEdges.tTop = tA;
		tEdges.tRight = tA;
		tEdges.tBottom = tA;
		return tEdges;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		return tDefault;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount == 2 ) {
		tA = __xgeXuiPageValueToSize(xvoArrayGetValue(pVal, 0), xgeXuiSizePx(0.0f));
		tB = __xgeXuiPageValueToSize(xvoArrayGetValue(pVal, 1), xgeXuiSizePx(0.0f));
		tEdges.tLeft = tA;
		tEdges.tRight = tA;
		tEdges.tTop = tB;
		tEdges.tBottom = tB;
		return tEdges;
	}
	if ( iCount == 4 ) {
		tEdges.tLeft = __xgeXuiPageValueToSize(xvoArrayGetValue(pVal, 0), xgeXuiSizePx(0.0f));
		tEdges.tTop = __xgeXuiPageValueToSize(xvoArrayGetValue(pVal, 1), xgeXuiSizePx(0.0f));
		tEdges.tRight = __xgeXuiPageValueToSize(xvoArrayGetValue(pVal, 2), xgeXuiSizePx(0.0f));
		tEdges.tBottom = __xgeXuiPageValueToSize(xvoArrayGetValue(pVal, 3), xgeXuiSizePx(0.0f));
		return tEdges;
	}
	return tDefault;
}

static xge_xui_edges_t __xgeXuiPageValueToEdgesToken(xge_xui_page_t* pPage, xvalue pVal, xge_xui_edges_t tDefault, const char* sPath)
{
	xge_xui_edges_t tEdges;
	xge_xui_size_t tA;
	xge_xui_size_t tB;
	xvalue pItem;
	char sItemPath[96];
	uint32 iCount;

	pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, sPath);
	if ( pVal == NULL ) {
		return tDefault;
	}
	if ( !__xgeXuiPageValueExists(pVal) || (xvoType(pVal) != XVO_DT_ARRAY) ) {
		return __xgeXuiPageValueToEdges(pVal, tDefault);
	}
	tEdges = tDefault;
	iCount = xvoArrayItemCount(pVal);
	if ( iCount == 2 ) {
		snprintf(sItemPath, sizeof(sItemPath), "%s[0]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 0), sItemPath);
		if ( pItem == NULL ) {
			return tDefault;
		}
		tA = __xgeXuiPageValueToSize(pItem, xgeXuiSizePx(0.0f));
		snprintf(sItemPath, sizeof(sItemPath), "%s[1]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 1), sItemPath);
		if ( pItem == NULL ) {
			return tDefault;
		}
		tB = __xgeXuiPageValueToSize(pItem, xgeXuiSizePx(0.0f));
		tEdges.tLeft = tA;
		tEdges.tRight = tA;
		tEdges.tTop = tB;
		tEdges.tBottom = tB;
		return tEdges;
	}
	if ( iCount == 4 ) {
		snprintf(sItemPath, sizeof(sItemPath), "%s[0]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 0), sItemPath);
		if ( pItem == NULL ) {
			return tDefault;
		}
		tEdges.tLeft = __xgeXuiPageValueToSize(pItem, xgeXuiSizePx(0.0f));
		snprintf(sItemPath, sizeof(sItemPath), "%s[1]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 1), sItemPath);
		if ( pItem == NULL ) {
			return tDefault;
		}
		tEdges.tTop = __xgeXuiPageValueToSize(pItem, xgeXuiSizePx(0.0f));
		snprintf(sItemPath, sizeof(sItemPath), "%s[2]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 2), sItemPath);
		if ( pItem == NULL ) {
			return tDefault;
		}
		tEdges.tRight = __xgeXuiPageValueToSize(pItem, xgeXuiSizePx(0.0f));
		snprintf(sItemPath, sizeof(sItemPath), "%s[3]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 3), sItemPath);
		if ( pItem == NULL ) {
			return tDefault;
		}
		tEdges.tBottom = __xgeXuiPageValueToSize(pItem, xgeXuiSizePx(0.0f));
		return tEdges;
	}
	return tDefault;
}

static int __xgeXuiPageApplyEdges(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sField, xge_xui_edges_t* pEdges)
{
	xvalue pVal;

	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, sField);
	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	*pEdges = __xgeXuiPageValueToEdgesToken(pPage, pVal, *pEdges, sField);
	if ( (pPage != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

static int __xgeXuiPageApplyDocumentSafeArea(xge_xui_page_t* pPage, xvalue pDoc)
{
	xvalue pVal;
	xge_xui_widget pRoot;
	xge_xui_edges_t tEdges;

	if ( (pPage == NULL) || (pPage->pContext == NULL) || (pDoc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageTableGet(pDoc, "safeArea");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	pRoot = xgeXuiRoot(pPage->pContext);
	if ( pRoot == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tEdges = __xgeXuiPageValueToEdgesToken(pPage, pVal, pRoot->tStyle.tPadding, "safeArea");
	if ( pPage->sError[0] != 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->tSafeAreaPrev = pRoot->tStyle.tPadding;
	pPage->bSafeAreaApplied = 1;
	pRoot->tStyle.tPadding = tEdges;
	xgeXuiWidgetMarkLayout(pRoot);
	xgeXuiWidgetMarkPaint(pRoot);
	return XGE_OK;
}

static int __xgeXuiPageApplyAnchor(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle)
{
	xvalue pAnchor;
	xvalue pVal;
	int iAnchor;

	pAnchor = __xgeXuiPageNodeGetStyled(pNode, pStyle, "anchor");
	if ( !__xgeXuiPageValueExists(pAnchor) || (xvoType(pAnchor) != XVO_DT_TABLE) ) {
		return XGE_OK;
	}
	iAnchor = 0;
	pVal = __xgeXuiPageTableGet(pAnchor, "left");
	if ( __xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "anchor.left");
		if ( pVal == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iAnchor |= XGE_XUI_ANCHOR_LEFT;
		pWidget->tStyle.tAnchor.tLeft = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tAnchor.tLeft);
	}
	pVal = __xgeXuiPageTableGet(pAnchor, "top");
	if ( __xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "anchor.top");
		if ( pVal == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iAnchor |= XGE_XUI_ANCHOR_TOP;
		pWidget->tStyle.tAnchor.tTop = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tAnchor.tTop);
	}
	pVal = __xgeXuiPageTableGet(pAnchor, "right");
	if ( __xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "anchor.right");
		if ( pVal == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iAnchor |= XGE_XUI_ANCHOR_RIGHT;
		pWidget->tStyle.tAnchor.tRight = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tAnchor.tRight);
	}
	pVal = __xgeXuiPageTableGet(pAnchor, "bottom");
	if ( __xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "anchor.bottom");
		if ( pVal == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iAnchor |= XGE_XUI_ANCHOR_BOTTOM;
		pWidget->tStyle.tAnchor.tBottom = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tAnchor.tBottom);
	}
	pWidget->tStyle.iAnchor = iAnchor;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

static uint32_t __xgeXuiPageValueToColor(xvalue pVal, uint32_t iDefault)
{
	const char* sText;
	char* sEnd;
	uint32_t iValue;
	size_t iLen;
	int iType;

	iType = xvoType(pVal);
	if ( iType == XVO_DT_INT ) {
		return (uint32_t)xvoGetInt(pVal);
	}
	if ( iType != XVO_DT_TEXT ) {
		return iDefault;
	}
	sText = (const char*)xvoGetText(pVal);
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( sText[0] == '#' ) {
		iValue = (uint32_t)strtoul(sText + 1, &sEnd, 16);
		if ( *sEnd != 0 ) {
			return iDefault;
		}
		iLen = strlen(sText + 1);
		if ( iLen == 6 ) {
			return (iValue << 8) | 0xFFu;
		}
		if ( iLen == 8 ) {
			return iValue;
		}
		return iDefault;
	}
	return (uint32_t)strtoul(sText, NULL, 0);
}

static xge_font __xgeXuiPageValueToFont(xge_xui_page_t* pPage, xvalue pVal, const char* sPath)
{
	const char* sText;
	xvalue pFontVal;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return NULL;
	}
	if ( xvoType(pVal) == XVO_DT_POINT ) {
		return (xge_font)xvoGetPoint(pVal);
	}
	if ( xvoType(pVal) != XVO_DT_TEXT ) {
		__xgeXuiPageSetPathError(pPage, sPath, "font must be a font token");
		return NULL;
	}
	sText = (const char*)xvoGetText(pVal);
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return NULL;
	}
	if ( sText[0] == '@' ) {
		pFontVal = __xgeXuiPageResolveTokenValue(pPage, pVal, sPath);
		if ( pFontVal == NULL ) {
			return NULL;
		}
		if ( xvoType(pFontVal) == XVO_DT_POINT ) {
			return (xge_font)xvoGetPoint(pFontVal);
		}
		__xgeXuiPageSetPathError(pPage, sPath, "font token must resolve to a font pointer");
		return NULL;
	}
	pFontVal = __xgeXuiPageContextTokenInSection(pPage, "fonts", sText, sPath);
	if ( __xgeXuiPageValueExists(pFontVal) && (xvoType(pFontVal) == XVO_DT_POINT) ) {
		return (xge_font)xvoGetPoint(pFontVal);
	}
	__xgeXuiPageSetPathError(pPage, sPath, "unknown font token '%s'", sText);
	return NULL;
}

static xge_texture __xgeXuiPageValueToTexture(xge_xui_page_t* pPage, xvalue pVal, const char* sPath)
{
	const char* sText;
	xvalue pTextureVal;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return NULL;
	}
	if ( xvoType(pVal) == XVO_DT_POINT ) {
		return (xge_texture)xvoGetPoint(pVal);
	}
	if ( xvoType(pVal) != XVO_DT_TEXT ) {
		__xgeXuiPageSetPathError(pPage, sPath, "texture must be a texture token");
		return NULL;
	}
	sText = (const char*)xvoGetText(pVal);
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return NULL;
	}
	if ( sText[0] == '@' ) {
		pTextureVal = __xgeXuiPageResolveTokenValue(pPage, pVal, sPath);
		if ( pTextureVal == NULL ) {
			return NULL;
		}
		if ( xvoType(pTextureVal) == XVO_DT_POINT ) {
			return (xge_texture)xvoGetPoint(pTextureVal);
		}
		__xgeXuiPageSetPathError(pPage, sPath, "texture token must resolve to a texture pointer");
		return NULL;
	}
	pTextureVal = __xgeXuiPageContextTokenInSection(pPage, "textures", sText, sPath);
	if ( __xgeXuiPageValueExists(pTextureVal) && (xvoType(pTextureVal) == XVO_DT_POINT) ) {
		return (xge_texture)xvoGetPoint(pTextureVal);
	}
	__xgeXuiPageSetPathError(pPage, sPath, "unknown texture token '%s'", sText);
	return NULL;
}

static int __xgeXuiPageStyleStackContains(const char* const* arrStack, int iStackCount, const char* sName)
{
	int i;

	if ( (arrStack == NULL) || (sName == NULL) ) {
		return 0;
	}
	for ( i = 0; i < iStackCount; i++ ) {
		if ( (arrStack[i] != NULL) && (strcmp(arrStack[i], sName) == 0) ) {
			return 1;
		}
	}
	return 0;
}

static xvalue __xgeXuiPageResolveStyleInStack(xge_xui_page_t* pPage, xvalue pStyles, const char* sName, const char* const* arrStack, int iStackCount)
{
	xvalue pStyle;
	xvalue pParentName;
	xvalue pParentStyle;
	const char* sParent;
	const char* arrNextStack[32];
	char sPath[96];
	int i;

	if ( (pStyles == NULL) || (xvoType(pStyles) != XVO_DT_TABLE) || (sName == NULL) ) {
		return NULL;
	}
	if ( iStackCount >= (int)(sizeof(arrNextStack) / sizeof(arrNextStack[0])) ) {
		__xgeXuiPageSetPathError(pPage, "styles", "style parent chain is too deep");
		return NULL;
	}
	if ( __xgeXuiPageStyleStackContains(arrStack, iStackCount, sName) ) {
		__xgeXuiPageSetPathError(pPage, "styles", "style parent cycle at '%s'", sName);
		return NULL;
	}
	pStyle = __xgeXuiPageTableGet(pStyles, sName);
	if ( (pStyle == NULL) || (xvoType(pStyle) == XVO_DT_NULL) ) {
		__xgeXuiPageSetPathError(pPage, "styles", "missing style '%s'", sName);
		return NULL;
	}
	if ( xvoType(pStyle) != XVO_DT_TABLE ) {
		snprintf(sPath, sizeof(sPath), "styles.%s", sName);
		sPath[sizeof(sPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sPath, "expected object");
		return NULL;
	}
	for ( i = 0; i < iStackCount; i++ ) {
		arrNextStack[i] = arrStack[i];
	}
	arrNextStack[iStackCount] = sName;
	pParentName = __xgeXuiPageTableGet(pStyle, "@parent");
	if ( xvoType(pParentName) == XVO_DT_TEXT ) {
		sParent = (const char*)xvoGetText(pParentName);
		if ( (sParent != NULL) && (sParent[0] != 0) ) {
			pParentStyle = __xgeXuiPageResolveStyleInStack(pPage, pStyles, sParent, arrNextStack, iStackCount + 1);
			if ( pParentStyle != NULL ) {
				xvoTableSetParent(pStyle, pParentStyle);
			} else {
				return NULL;
			}
		}
	}
	return pStyle;
}

static xvalue __xgeXuiPageResolveStyle(xge_xui_page_t* pPage, xvalue pStyles, const char* sName)
{
	return __xgeXuiPageResolveStyleInStack(pPage, pStyles, sName, NULL, 0);
}

static xvalue __xgeXuiPageNodeStyle(xge_xui_page_t* pPage, xvalue pStyles, xvalue pNode)
{
	xvalue pVal;

	pVal = __xgeXuiPageTableGet(pNode, "style");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		return __xgeXuiPageResolveStyle(pPage, pStyles, (const char*)xvoGetText(pVal));
	}
	if ( xvoType(pVal) == XVO_DT_TABLE ) {
		return pVal;
	}
	return NULL;
}

static int __xgeXuiPageClickEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_page_click_binding_t* pBinding;

	if ( (pWidget == NULL) || (pEvent == NULL) || (pUser == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pBinding = (xge_xui_page_click_binding_t*)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_UP ) {
		if ( pEvent->iParam1 != XGE_MOUSE_LEFT ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		if ( pBinding->procClick != NULL ) {
			pBinding->procClick(pWidget, pBinding->pUser);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_TOUCH_END ) {
		if ( pBinding->procClick != NULL ) {
			pBinding->procClick(pWidget, pBinding->pUser);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && ((pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE)) ) {
		if ( pBinding->procClick != NULL ) {
			pBinding->procClick(pWidget, pBinding->pUser);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static xge_xui_binder_entry_t* __xgeXuiPageFindClickBinding(const xge_xui_binder_t* pBinder, const char* sName)
{
	int i;

	if ( (pBinder == NULL) || (sName == NULL) ) {
		return NULL;
	}
	for ( i = 0; i < pBinder->iClickCount; i++ ) {
		if ( (pBinder->arrClick[i].sName != NULL) && (strcmp(pBinder->arrClick[i].sName, sName) == 0) ) {
			return (xge_xui_binder_entry_t*)&pBinder->arrClick[i];
		}
	}
	return NULL;
}

static void __xgeXuiPageUnitWidgetControls(xge_xui_page_t* pPage, xge_xui_widget pWidget)
{
	int i;

	if ( (pPage == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < pPage->iButtonCount; i++ ) {
		if ( pPage->arrButton[i].pWidget == pWidget ) {
			xgeXuiButtonUnit(&pPage->arrButton[i]);
		}
	}
	for ( i = 0; i < pPage->iImageCount; i++ ) {
		if ( pPage->arrImage[i].pWidget == pWidget ) {
			xgeXuiImageUnit(&pPage->arrImage[i]);
			if ( pPage->arrImageTextureOwned[i] ) {
				xgeTextureFree(&pPage->arrImageTexture[i]);
				pPage->arrImageTextureOwned[i] = 0;
			}
		}
	}
	for ( i = 0; i < pPage->iInputCount; i++ ) {
		if ( pPage->arrInput[i].pWidget == pWidget ) {
			xgeXuiInputUnit(&pPage->arrInput[i]);
		}
	}
	for ( i = 0; i < pPage->iNumericInputCount; i++ ) {
		if ( pPage->arrNumericInput[i].pWidget == pWidget ) {
			xgeXuiNumericInputUnit(&pPage->arrNumericInput[i]);
		}
	}
	for ( i = 0; i < pPage->iCheckBoxCount; i++ ) {
		if ( pPage->arrCheckBox[i].pWidget == pWidget ) {
			xgeXuiCheckBoxUnit(&pPage->arrCheckBox[i]);
		}
	}
	for ( i = 0; i < pPage->iRadioCount; i++ ) {
		if ( pPage->arrRadio[i].pWidget == pWidget ) {
			xgeXuiRadioUnit(&pPage->arrRadio[i]);
		}
	}
	for ( i = 0; i < pPage->iSwitchCount; i++ ) {
		if ( pPage->arrSwitch[i].pWidget == pWidget ) {
			xgeXuiSwitchUnit(&pPage->arrSwitch[i]);
		}
	}
	for ( i = 0; i < pPage->iSliderCount; i++ ) {
		if ( pPage->arrSlider[i].pWidget == pWidget ) {
			xgeXuiSliderUnit(&pPage->arrSlider[i]);
		}
	}
	for ( i = 0; i < pPage->iProgressCount; i++ ) {
		if ( pPage->arrProgress[i].pWidget == pWidget ) {
			xgeXuiProgressUnit(&pPage->arrProgress[i]);
		}
	}
	for ( i = 0; i < pPage->iTabsCount; i++ ) {
		if ( pPage->arrTabs[i].pWidget == pWidget ) {
			xgeXuiTabsUnit(&pPage->arrTabs[i]);
		}
	}
	for ( i = 0; i < pPage->iToolbarCount; i++ ) {
		if ( pPage->arrToolbar[i].pWidget == pWidget ) {
			xgeXuiToolbarUnit(&pPage->arrToolbar[i]);
		}
	}
	for ( i = 0; i < pPage->iStatusBarCount; i++ ) {
		if ( pPage->arrStatusBar[i].pWidget == pWidget ) {
			xgeXuiStatusBarUnit(&pPage->arrStatusBar[i]);
		}
	}
	for ( i = 0; i < pPage->iComboBoxCount; i++ ) {
		if ( (pPage->arrComboBox[i] != NULL) && (pPage->arrComboBox[i]->pWidget == pWidget) ) {
			xgeXuiComboBoxUnit(pPage->arrComboBox[i]);
			free(pPage->arrComboBox[i]);
			pPage->arrComboBox[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iPopupCount; i++ ) {
		if ( (pPage->arrPopup[i] != NULL) && (pPage->arrPopup[i]->pWidget == pWidget) ) {
			xgeXuiPopupUnit(pPage->arrPopup[i]);
			free(pPage->arrPopup[i]);
			pPage->arrPopup[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iTooltipCount; i++ ) {
		if ( (pPage->arrTooltip[i] != NULL) && (pPage->arrTooltip[i]->pOwner == pWidget) ) {
			xgeXuiTooltipUnit(pPage->arrTooltip[i]);
			free(pPage->arrTooltip[i]);
			pPage->arrTooltip[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iMenuCount; i++ ) {
		if ( (pPage->arrMenu[i] != NULL) && ((pPage->arrMenuWidget[i] == pWidget) || (pPage->arrMenu[i]->pOwner == pWidget)) ) {
			xgeXuiMenuUnit(pPage->arrMenu[i]);
			free(pPage->arrMenu[i]);
			pPage->arrMenu[i] = NULL;
			pPage->arrMenuWidget[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iDialogCount; i++ ) {
		if ( (pPage->arrDialog[i] != NULL) && (pPage->arrDialog[i]->pWidget == pWidget) ) {
			xgeXuiDialogUnit(pPage->arrDialog[i]);
			free(pPage->arrDialog[i]);
			pPage->arrDialog[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iMessageBoxCount; i++ ) {
		if ( (pPage->arrMessageBox[i] != NULL) && (pPage->arrMessageBox[i]->pWidget == pWidget) ) {
			xgeXuiMessageBoxUnit(pPage->arrMessageBox[i]);
			free(pPage->arrMessageBox[i]);
			pPage->arrMessageBox[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iLabelCount; i++ ) {
		if ( pPage->arrLabel[i].pWidget == pWidget ) {
			xgeXuiLabelUnit(&pPage->arrLabel[i]);
		}
	}
	for ( i = 0; i < pPage->iSeparatorCount; i++ ) {
		if ( pPage->arrSeparator[i].pWidget == pWidget ) {
			xgeXuiSeparatorUnit(&pPage->arrSeparator[i]);
		}
	}
	for ( i = 0; i < pPage->iScrollViewCount; i++ ) {
		if ( pPage->arrScrollView[i].pWidget == pWidget ) {
			xgeXuiScrollViewUnit(&pPage->arrScrollView[i]);
		}
	}
	for ( i = 0; i < pPage->iTreeViewCount; i++ ) {
		if ( pPage->arrTreeView[i].pWidget == pWidget ) {
			xgeXuiTreeViewUnit(&pPage->arrTreeView[i]);
		}
	}
	for ( i = 0; i < pPage->iTableViewCount; i++ ) {
		if ( pPage->arrTableView[i].pWidget == pWidget ) {
			xgeXuiTableViewUnit(&pPage->arrTableView[i]);
			free(pPage->arrTableViewAdapter[i]);
			pPage->arrTableViewAdapter[i] = NULL;
		}
	}
	for ( i = 0; i < pPage->iPropertyGridCount; i++ ) {
		if ( pPage->arrPropertyGrid[i].pWidget == pWidget ) {
			xgeXuiPropertyGridUnit(&pPage->arrPropertyGrid[i]);
		}
	}
	for ( i = 0; i < pPage->iBreadcrumbCount; i++ ) {
		if ( pPage->arrBreadcrumb[i].pWidget == pWidget ) {
			xgeXuiBreadcrumbUnit(&pPage->arrBreadcrumb[i]);
		}
	}
	for ( i = 0; i < pPage->iAccordionCount; i++ ) {
		if ( pPage->arrAccordion[i].pWidget == pWidget ) {
			xgeXuiAccordionUnit(&pPage->arrAccordion[i]);
		}
	}
	for ( i = 0; i < pPage->iToastCount; i++ ) {
		if ( pPage->arrToast[i].pWidget == pWidget ) {
			xgeXuiToastUnit(&pPage->arrToast[i]);
		}
	}
	for ( i = 0; i < pPage->iVirtualListCount; i++ ) {
		if ( pPage->arrVirtualList[i].pWidget == pWidget ) {
			xgeXuiVirtualListUnit(&pPage->arrVirtualList[i]);
		}
	}
}

static void __xgeXuiPageUnitTreeControls(xge_xui_page_t* pPage, xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_xui_widget pNext;

	if ( (pPage == NULL) || (pWidget == NULL) ) {
		return;
	}
	pChild = pWidget->pFirstChild;
	while ( pChild != NULL ) {
		pNext = pChild->pNextSibling;
		__xgeXuiPageUnitTreeControls(pPage, pChild);
		pChild = pNext;
	}
	__xgeXuiPageUnitWidgetControls(pPage, pWidget);
}

static int __xgeXuiPageApplyButtonColor(xge_xui_page_t* pPage, xge_xui_widget pWidget, xge_xui_button pButton, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiWidgetMarkPaint(pWidget);
	}
	(void)pButton;
	return XGE_OK;
}

static int __xgeXuiPageApplyButtonClick(xge_xui_page_t* pPage, xge_xui_button pButton, xvalue pNode, const char* sPath)
{
	xge_xui_binder_entry_t* pEntry;
	xvalue pVal;
	const char* sName;
	char sFieldPath[128];

	pVal = __xgeXuiPageTableGet(pNode, "onClick");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.onClick", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	if ( xvoType(pVal) != XVO_DT_TEXT ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "expected event name string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sName = (const char*)xvoGetText(pVal);
	if ( (sName == NULL) || (sName[0] == 0) ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "empty event name");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pEntry = __xgeXuiPageFindClickBinding(pPage->tLoader.pBinder, sName);
	if ( pEntry == NULL ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "unregistered click event '%s'", sName);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiButtonSetClick(pButton, pEntry->procClick, pEntry->pUser);
	return XGE_OK;
}

static int __xgeXuiPageApplyButton(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_button pButton;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	uint32_t iFlags;
	char sFieldPath[128];

	if ( pPage->iButtonCount >= XGE_XUI_PAGE_BUTTON_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "button capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pPage->arrButton[pPage->iButtonCount];
	if ( xgeXuiButtonInit(pButton, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "button initialization failed");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->iButtonCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont == NULL ) {
		pFont = pButton->pFont;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pButton->sText;
	xgeXuiButtonSetText(pButton, pFont, sText);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.textColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "textColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiButtonSetTextColor(pButton, __xgeXuiPageValueToColor(pVal, pButton->iTextColor));
	}
	iFlags = pButton->iTextFlags;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "textAlign");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		iFlags = __xgeXuiPageTextToTextAlign((const char*)xvoGetText(pVal), iFlags);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "textVAlign");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		iFlags = __xgeXuiPageTextToTextVAlign((const char*)xvoGetText(pVal), iFlags);
	}
	pButton->iTextFlags = iFlags | XGE_TEXT_CLIP;
	if ( __xgeXuiPageApplyButtonColor(pPage, pWidget, pButton, pNode, pStyle, "color", "background", &pButton->iColorNormal, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyButtonColor(pPage, pWidget, pButton, pNode, pStyle, "hoverColor", NULL, &pButton->iColorHover, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyButtonColor(pPage, pWidget, pButton, pNode, pStyle, "activeColor", NULL, &pButton->iColorActive, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyButtonColor(pPage, pWidget, pButton, pNode, pStyle, "focusColor", NULL, &pButton->iColorFocus, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyButtonColor(pPage, pWidget, pButton, pNode, pStyle, "disabledColor", NULL, &pButton->iColorDisabled, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyButtonClick(pPage, pButton, pNode, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageValueToRect(xge_xui_page_t* pPage, xvalue pVal, xge_rect_t* pRect, const char* sPath)
{
	xvalue pItem;
	uint32 iCount;

	if ( (pRect == NULL) || !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_ARRAY ) {
		iCount = xvoArrayItemCount(pVal);
		if ( iCount != 4 ) {
			__xgeXuiPageSetPathError(pPage, sPath, "rect array must contain 4 values");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pRect->fX = __xgeXuiPageValueToFloat(xvoArrayGetValue(pVal, 0), pRect->fX);
		pRect->fY = __xgeXuiPageValueToFloat(xvoArrayGetValue(pVal, 1), pRect->fY);
		pRect->fW = __xgeXuiPageValueToFloat(xvoArrayGetValue(pVal, 2), pRect->fW);
		pRect->fH = __xgeXuiPageValueToFloat(xvoArrayGetValue(pVal, 3), pRect->fH);
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_TABLE ) {
		pItem = __xgeXuiPageTableGet(pVal, "x");
		if ( __xgeXuiPageValueExists(pItem) ) {
			pRect->fX = __xgeXuiPageValueToFloat(pItem, pRect->fX);
		}
		pItem = __xgeXuiPageTableGet(pVal, "y");
		if ( __xgeXuiPageValueExists(pItem) ) {
			pRect->fY = __xgeXuiPageValueToFloat(pItem, pRect->fY);
		}
		pItem = __xgeXuiPageTableGet(pVal, "w");
		if ( !__xgeXuiPageValueExists(pItem) ) {
			pItem = __xgeXuiPageTableGet(pVal, "width");
		}
		if ( __xgeXuiPageValueExists(pItem) ) {
			pRect->fW = __xgeXuiPageValueToFloat(pItem, pRect->fW);
		}
		pItem = __xgeXuiPageTableGet(pVal, "h");
		if ( !__xgeXuiPageValueExists(pItem) ) {
			pItem = __xgeXuiPageTableGet(pVal, "height");
		}
		if ( __xgeXuiPageValueExists(pItem) ) {
			pRect->fH = __xgeXuiPageValueToFloat(pItem, pRect->fH);
		}
		return XGE_OK;
	}
	__xgeXuiPageSetPathError(pPage, sPath, "rect must be array or object");
	return XGE_ERROR_INVALID_ARGUMENT;
}

static int __xgeXuiPageTextToImageMode(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "stretch") == 0 ) {
		return XGE_XUI_IMAGE_STRETCH;
	}
	if ( strcmp(sText, "fit") == 0 ) {
		return XGE_XUI_IMAGE_FIT;
	}
	if ( strcmp(sText, "center") == 0 ) {
		return XGE_XUI_IMAGE_CENTER;
	}
	return iDefault;
}

static int __xgeXuiPageApplyImage(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_image pImage;
	xge_texture pTexture;
	xvalue pVal;
	const char* sSrc;
	xge_rect_t tSrcRect;
	char sFieldPath[128];
	char sBindKey[XGE_XUI_MODEL_KEY_CAPACITY];
	int iSlot;
	int iRet;
	int bSrcBinding;

	if ( pPage->iImageCount >= XGE_XUI_PAGE_IMAGE_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "image capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSlot = pPage->iImageCount;
	pImage = &pPage->arrImage[iSlot];
	pTexture = NULL;
	bSrcBinding = 0;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.texture", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "texture");
	pTexture = __xgeXuiPageValueToTexture(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "src");
	if ( __xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.src", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		if ( xvoType(pVal) != XVO_DT_TEXT ) {
			__xgeXuiPageSetPathError(pPage, sFieldPath, "src must be string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sSrc = (const char*)xvoGetText(pVal);
		bSrcBinding = __xgeXuiPageParseModelBinding(sSrc, sBindKey, sizeof(sBindKey));
		if ( (bSrcBinding == 0) && (sSrc != NULL) && (sSrc[0] != 0) ) {
			iRet = xgeTextureLoad(&pPage->arrImageTexture[iSlot], sSrc);
			if ( iRet != XGE_OK ) {
				__xgeXuiPageSetPathError(pPage, sFieldPath, "texture load failed: %s (%d)", sSrc, iRet);
				return iRet;
			}
			pPage->arrImageTextureOwned[iSlot] = 1;
			pTexture = &pPage->arrImageTexture[iSlot];
		}
	}
	if ( xgeXuiImageInit(pImage, pWidget, pTexture) != XGE_OK ) {
		if ( pPage->arrImageTextureOwned[iSlot] ) {
			xgeTextureFree(&pPage->arrImageTexture[iSlot]);
			pPage->arrImageTextureOwned[iSlot] = 0;
		}
		__xgeXuiPageSetPathError(pPage, sPath, "image initialization failed");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->iImageCount++;
	if ( bSrcBinding ) {
		if ( __xgeXuiPageAddModelBinding(pPage, pWidget, pImage, XGE_XUI_PAGE_MODEL_BIND_IMAGE_SRC, iSlot, sBindKey, sFieldPath) != XGE_OK ) {
			return (pPage->sError[0] != 0) ? XGE_ERROR_OUT_OF_MEMORY : XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	tSrcRect = pImage->tSrc;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.source", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "source");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.srcRect", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "srcRect");
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		if ( __xgeXuiPageValueToRect(pPage, pVal, &tSrcRect, sFieldPath) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		xgeXuiImageSetSource(pImage, tSrcRect);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.color", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "color", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.tint", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "tint", sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiImageSetColor(pImage, __xgeXuiPageValueToColor(pVal, pImage->iColor));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "mode");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiImageSetMode(pImage, __xgeXuiPageTextToImageMode((const char*)xvoGetText(pVal), pImage->iMode));
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyInputColor(xge_xui_page_t* pPage, xge_xui_widget pWidget, xge_xui_input pInput, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiWidgetMarkPaint(pWidget);
	}
	(void)pInput;
	return XGE_OK;
}

static int __xgeXuiPageApplyInputSelection(xge_xui_page_t* pPage, xge_xui_input pInput, xvalue pVal, const char* sPath)
{
	xvalue pStart;
	xvalue pEnd;
	int iStart;
	int iEnd;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	iStart = 0;
	iEnd = 0;
	if ( xvoType(pVal) == XVO_DT_ARRAY ) {
		if ( xvoArrayItemCount(pVal) < 2 ) {
			__xgeXuiPageSetPathError(pPage, sPath, "selection array requires start and end");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iStart = (int)__xgeXuiPageValueToFloat(xvoArrayGetValue(pVal, 0), 0.0f);
		iEnd = (int)__xgeXuiPageValueToFloat(xvoArrayGetValue(pVal, 1), (float)iStart);
		xgeXuiInputSetSelection(pInput, iStart, iEnd);
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_TABLE ) {
		pStart = __xgeXuiPageTableGet(pVal, "start");
		pEnd = __xgeXuiPageTableGet(pVal, "end");
		iStart = (int)__xgeXuiPageValueToFloat(pStart, 0.0f);
		iEnd = (int)__xgeXuiPageValueToFloat(pEnd, (float)iStart);
		xgeXuiInputSetSelection(pInput, iStart, iEnd);
		return XGE_OK;
	}
	__xgeXuiPageSetPathError(pPage, sPath, "selection must be array or object");
	return XGE_ERROR_INVALID_ARGUMENT;
}

static int __xgeXuiPageRejectInputDeferredEvent(xge_xui_page_t* pPage, xvalue pNode, const char* sKey, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	pVal = __xgeXuiPageTableGet(pNode, sKey);
	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	__xgeXuiPageSetPathError(pPage, sFieldPath, "%s is not supported yet", sKey);
	return XGE_ERROR_INVALID_ARGUMENT;
}

static int __xgeXuiPageApplyInput(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_input pInput;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	char sFieldPath[128];
	char sBindKey[XGE_XUI_MODEL_KEY_CAPACITY];
	int bValueBinding;
	int iValueField;

	if ( pPage->iInputCount >= XGE_XUI_PAGE_INPUT_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "input capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pInput = &pPage->arrInput[pPage->iInputCount];
	if ( xgeXuiInputInit(pInput, pPage->pContext, pWidget, pFont) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "input initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iInputCount++;
	bValueBinding = 0;
	iValueField = 1;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "value");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
		iValueField = 0;
	}
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", iValueField ? "value" : "text");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "%s must be string", iValueField ? "value" : "text");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		bValueBinding = __xgeXuiPageParseModelBinding(sText, sBindKey, sizeof(sBindKey));
		xgeXuiInputSetText(pInput, bValueBinding ? "" : sText);
		if ( bValueBinding ) {
			snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", iValueField ? "value" : "text");
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			if ( __xgeXuiPageAddModelBinding(pPage, pWidget, pInput, XGE_XUI_PAGE_MODEL_BIND_INPUT_VALUE, -1, sBindKey, sFieldPath) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "placeholder");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.placeholder", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "placeholder must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiInputSetPlaceholder(pInput, (const char*)xvoGetText(pVal));
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "textColor", NULL, &pInput->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "backgroundColor", "background", &pInput->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "focusColor", NULL, &pInput->iFocusColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "cursorColor", NULL, &pInput->iCursorColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "placeholderColor", NULL, &pInput->iPlaceholderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "selectionColor", NULL, &pInput->iSelectionColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "disabledTextColor", NULL, &pInput->iDisabledTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "disabledBackgroundColor", NULL, &pInput->iDisabledBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "password");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiInputSetPassword(pInput, __xgeXuiPageValueToBool(pVal, pInput->bPassword));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "readonly");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiInputSetReadonly(pInput, __xgeXuiPageValueToBool(pVal, pInput->bReadonly));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "disabled");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiInputSetDisabled(pInput, __xgeXuiPageValueToBool(pVal, pInput->bDisabled));
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selection", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "selection");
	if ( __xgeXuiPageApplyInputSelection(pPage, pInput, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSubmit", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyNumericInput(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_numeric_input pNumeric;
	xge_xui_input pInput;
	xvalue pVal;
	xge_font pFont;
	float fMin;
	float fMax;
	const char* sText;
	char sFieldPath[128];
	char sBindKey[XGE_XUI_MODEL_KEY_CAPACITY];

	if ( pPage->iNumericInputCount >= XGE_XUI_PAGE_NUMERIC_INPUT_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "numericInput capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pNumeric = &pPage->arrNumericInput[pPage->iNumericInputCount];
	if ( xgeXuiNumericInputInit(pNumeric, pPage->pContext, pWidget, pFont) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "numericInput initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iNumericInputCount++;
	pInput = &pNumeric->tInput;
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "textColor", NULL, &pInput->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "backgroundColor", "background", &pInput->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "focusColor", NULL, &pInput->iFocusColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "cursorColor", NULL, &pInput->iCursorColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "placeholderColor", NULL, &pInput->iPlaceholderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "selectionColor", NULL, &pInput->iSelectionColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "disabledTextColor", NULL, &pInput->iDisabledTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyInputColor(pPage, pWidget, pInput, pNode, pStyle, "disabledBackgroundColor", NULL, &pInput->iDisabledBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "placeholder");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.placeholder", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "placeholder must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiInputSetPlaceholder(pInput, (const char*)xvoGetText(pVal));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "integer");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiNumericInputSetInteger(pNumeric, __xgeXuiPageValueToBool(pVal, pNumeric->bInteger));
	}
	fMin = pNumeric->fMin;
	fMax = pNumeric->fMax;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.min", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "min", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMin = __xgeXuiPageValueToFloat(pVal, fMin);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.max", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "max", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMax = __xgeXuiPageValueToFloat(pVal, fMax);
	}
	if ( (fMin != pNumeric->fMin) || (fMax != pNumeric->fMax) ) {
		xgeXuiNumericInputSetRange(pNumeric, fMin, fMax);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.step", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "step", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiNumericInputSetStep(pNumeric, __xgeXuiPageValueToFloat(pVal, pNumeric->fStep));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "spinner");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiNumericInputSetSpinnerVisible(pNumeric, __xgeXuiPageValueToBool(pVal, pNumeric->bShowSpinner));
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.value", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "value", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "text", sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		if ( xvoType(pVal) == XVO_DT_TEXT ) {
			sText = (const char*)xvoGetText(pVal);
			if ( __xgeXuiPageParseModelBinding(sText, sBindKey, sizeof(sBindKey)) ) {
				__xgeXuiPageSetPathError(pPage, sFieldPath, "numericInput model binding is not supported yet");
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		}
		xgeXuiNumericInputSetValue(pNumeric, __xgeXuiPageValueToFloat(pVal, pNumeric->fValue));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "readonly");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiInputSetReadonly(pInput, __xgeXuiPageValueToBool(pVal, pInput->bReadonly));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "disabled");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiInputSetDisabled(pInput, __xgeXuiPageValueToBool(pVal, pInput->bDisabled));
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSubmit", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyToggleColor(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiWidgetMarkPaint(pWidget);
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyCheckBox(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_checkbox pCheckBox;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	char sFieldPath[128];

	if ( pPage->iCheckBoxCount >= XGE_XUI_PAGE_CHECKBOX_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "checkbox capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pCheckBox = &pPage->arrCheckBox[pPage->iCheckBoxCount];
	if ( xgeXuiCheckBoxInit(pCheckBox, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "checkbox initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iCheckBoxCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pCheckBox->sText;
	xgeXuiCheckBoxSetText(pCheckBox, (pFont != NULL) ? pFont : pCheckBox->pFont, sText);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "checked");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "value");
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiCheckBoxSetChecked(pCheckBox, __xgeXuiPageValueToBool(pVal, pCheckBox->bChecked));
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pCheckBox->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", "background", &pCheckBox->iColorNormal, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pCheckBox->iColorHover, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pCheckBox->iColorActive, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pCheckBox->iColorFocus, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pCheckBox->iColorDisabled, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "boxColor", NULL, &pCheckBox->iColorBox, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "checkedColor", NULL, &pCheckBox->iColorChecked, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath);
}

static int __xgeXuiPageApplyRadio(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_radio pRadio;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	char sFieldPath[128];

	if ( pPage->iRadioCount >= XGE_XUI_PAGE_RADIO_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "radio capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pRadio = &pPage->arrRadio[pPage->iRadioCount];
	if ( xgeXuiRadioInit(pRadio, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "radio initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iRadioCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pRadio->sText;
	xgeXuiRadioSetText(pRadio, (pFont != NULL) ? pFont : pRadio->pFont, sText);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "value");
	if ( __xgeXuiPageValueExists(pVal) ) {
		pRadio->iValue = (int)__xgeXuiPageValueToFloat(pVal, (float)pRadio->iValue);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "checked");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiRadioSetChecked(pRadio, __xgeXuiPageValueToBool(pVal, pRadio->bChecked));
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pRadio->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", "background", &pRadio->iColorNormal, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pRadio->iColorHover, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pRadio->iColorActive, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pRadio->iColorFocus, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pRadio->iColorDisabled, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "ringColor", NULL, &pRadio->iColorRing, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "checkedColor", NULL, &pRadio->iColorChecked, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath);
}

static int __xgeXuiPageApplySwitch(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_switch pSwitch;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	char sFieldPath[128];

	if ( pPage->iSwitchCount >= XGE_XUI_PAGE_SWITCH_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "switch capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSwitch = &pPage->arrSwitch[pPage->iSwitchCount];
	if ( xgeXuiSwitchInit(pSwitch, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "switch initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iSwitchCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pSwitch->sText;
	xgeXuiSwitchSetText(pSwitch, (pFont != NULL) ? pFont : pSwitch->pFont, sText);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "checked");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "value");
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiSwitchSetChecked(pSwitch, __xgeXuiPageValueToBool(pVal, pSwitch->bChecked));
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pSwitch->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", "background", &pSwitch->iColorNormal, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pSwitch->iColorHover, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pSwitch->iColorActive, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pSwitch->iColorFocus, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pSwitch->iColorDisabled, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "trackColor", NULL, &pSwitch->iColorTrack, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "checkedColor", NULL, &pSwitch->iColorChecked, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "knobColor", NULL, &pSwitch->iColorKnob, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath);
}

static int __xgeXuiPageApplySlider(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_slider pSlider;
	xvalue pVal;
	float fMin;
	float fMax;
	char sFieldPath[128];

	if ( pPage->iSliderCount >= XGE_XUI_PAGE_SLIDER_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "slider capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSlider = &pPage->arrSlider[pPage->iSliderCount];
	if ( xgeXuiSliderInit(pSlider, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "slider initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iSliderCount++;
	fMin = pSlider->fMin;
	fMax = pSlider->fMax;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.min", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "min", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMin = __xgeXuiPageValueToFloat(pVal, fMin);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.max", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "max", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMax = __xgeXuiPageValueToFloat(pVal, fMax);
	}
	if ( (fMin != pSlider->fMin) || (fMax != pSlider->fMax) ) {
		xgeXuiSliderSetRange(pSlider, fMin, fMax);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.value", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "value", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiSliderSetValue(pSlider, __xgeXuiPageValueToFloat(pVal, pSlider->fValue));
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "trackColor", NULL, &pSlider->iColorTrack, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "fillColor", NULL, &pSlider->iColorFill, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "knobColor", NULL, &pSlider->iColorKnob, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pSlider->iColorFocus, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pSlider->iColorDisabled, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath);
}

static int __xgeXuiPageApplyProgress(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_progress pProgress;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	float fMin;
	float fMax;
	char sFieldPath[128];

	if ( pPage->iProgressCount >= XGE_XUI_PAGE_PROGRESS_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "progress capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pProgress = &pPage->arrProgress[pPage->iProgressCount];
	if ( xgeXuiProgressInit(pProgress, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "progress initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iProgressCount++;
	fMin = pProgress->fMin;
	fMax = pProgress->fMax;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.min", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "min", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMin = __xgeXuiPageValueToFloat(pVal, fMin);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.max", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "max", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMax = __xgeXuiPageValueToFloat(pVal, fMax);
	}
	if ( (fMin != pProgress->fMin) || (fMax != pProgress->fMax) ) {
		xgeXuiProgressSetRange(pProgress, fMin, fMax);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.value", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "value", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiProgressSetValue(pProgress, __xgeXuiPageValueToFloat(pVal, pProgress->fValue));
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pProgress->sText;
	xgeXuiProgressSetText(pProgress, pFont, sText);
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "trackColor", NULL, &pProgress->iColorTrack, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "fillColor", NULL, &pProgress->iColorFill, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pProgress->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyTabsItems(xge_xui_page_t* pPage, xge_xui_tabs pTabs, xvalue pVal, const char* sPath)
{
	static const char* arrItems[XGE_XUI_PAGE_TABS_CAPACITY][32];
	xvalue pItem;
	uint32 i;
	uint32 iCount;
	int iSlot;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "items must be array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > 32 ) {
		__xgeXuiPageSetPathError(pPage, sPath, "items capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSlot = pPage->iTabsCount - 1;
	if ( (iSlot < 0) || (iSlot >= XGE_XUI_PAGE_TABS_CAPACITY) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tabs slot invalid");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "items", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( pItem == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_TEXT ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "item must be string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrItems[iSlot][i] = (const char*)xvoGetText(pItem);
	}
	xgeXuiTabsSetItems(pTabs, arrItems[iSlot], (int)iCount);
	return XGE_OK;
}

static int __xgeXuiPageApplyTabs(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_tabs pTabs;
	xvalue pVal;
	xge_font pFont;
	float fTabWidth;
	float fTabHeight;
	char sFieldPath[128];

	if ( pPage->iTabsCount >= XGE_XUI_PAGE_TABS_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tabs capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTabs = &pPage->arrTabs[pPage->iTabsCount];
	if ( xgeXuiTabsInit(pTabs, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tabs initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iTabsCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiTabsSetFont(pTabs, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.items", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "items");
	if ( __xgeXuiPageApplyTabsItems(pPage, pTabs, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "selected");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "value");
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiTabsSetSelected(pTabs, (int)__xgeXuiPageValueToFloat(pVal, (float)pTabs->iSelected));
	}
	fTabWidth = pTabs->fTabWidth;
	fTabHeight = pTabs->fTabHeight;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.tabWidth", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "tabWidth", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fTabWidth = __xgeXuiPageValueToFloat(pVal, fTabWidth);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.tabHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "tabHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fTabHeight = __xgeXuiPageValueToFloat(pVal, fTabHeight);
	}
	if ( (fTabWidth != pTabs->fTabWidth) || (fTabHeight != pTabs->fTabHeight) ) {
		xgeXuiTabsSetTabSize(pTabs, fTabWidth, fTabHeight);
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pTabs->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "tabColor", NULL, &pTabs->iTabColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pTabs->iHoverColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pTabs->iActiveColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pTabs->iFocusColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pTabs->iDisabledColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pTabs->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeTextColor", NULL, &pTabs->iActiveTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageTextToToolbarItemType(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "toggle") == 0 ) {
		return XGE_XUI_TOOLBAR_ITEM_TOGGLE;
	}
	if ( (strcmp(sText, "separator") == 0) || (strcmp(sText, "sep") == 0) ) {
		return XGE_XUI_TOOLBAR_ITEM_SEPARATOR;
	}
	return XGE_XUI_TOOLBAR_ITEM_BUTTON;
}

static int __xgeXuiPageApplyToolbarItems(xge_xui_page_t* pPage, xge_xui_toolbar pToolbar, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	int iType;
	char sItemPath[128];
	const char* arrText[XGE_XUI_TOOLBAR_CAPACITY];
	int arrTypes[XGE_XUI_TOOLBAR_CAPACITY];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toolbar items must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_TOOLBAR_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toolbar items capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(arrText, 0, sizeof(arrText));
	memset(arrTypes, 0, sizeof(arrTypes));
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "items", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrText[i] = "";
		arrTypes[i] = XGE_XUI_TOOLBAR_ITEM_BUTTON;
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			arrText[i] = (const char*)xvoGetText(pItem);
		} else if ( xvoType(pItem) == XVO_DT_TABLE ) {
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "text"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				arrText[i] = (const char*)xvoGetText(pField);
			}
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "type"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				arrTypes[i] = __xgeXuiPageTextToToolbarItemType((const char*)xvoGetText(pField), arrTypes[i]);
			}
		} else {
			__xgeXuiPageSetPathError(pPage, sItemPath, "toolbar item must be text or object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	xgeXuiToolbarSetItems(pToolbar, arrText, arrTypes, (int)iCount);
	for ( i = 0; i < iCount; i++ ) {
		pItem = xvoArrayGetValue(pVal, i);
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			continue;
		}
		pField = __xgeXuiPageTableGet(pItem, "enabled");
		if ( __xgeXuiPageValueExists(pField) ) {
			xgeXuiToolbarSetItemEnabled(pToolbar, (int)i, __xgeXuiPageValueToBool(pField, 1));
		}
		pField = __xgeXuiPageTableGet(pItem, "checked");
		if ( __xgeXuiPageValueExists(pField) ) {
			xgeXuiToolbarSetItemChecked(pToolbar, (int)i, __xgeXuiPageValueToBool(pField, 0));
		}
		iType = pToolbar->arrItems[i].iType;
		if ( iType == XGE_XUI_TOOLBAR_ITEM_SEPARATOR ) {
			xgeXuiToolbarSetItemEnabled(pToolbar, (int)i, 0);
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyToolbar(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_toolbar pToolbar;
	xvalue pVal;
	xge_font pFont;
	float fItemWidth;
	float fItemHeight;
	float fSeparatorSize;
	char sFieldPath[128];

	if ( pPage->iToolbarCount >= XGE_XUI_PAGE_TOOLBAR_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toolbar capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pToolbar = &pPage->arrToolbar[pPage->iToolbarCount];
	if ( xgeXuiToolbarInit(pToolbar, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toolbar initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iToolbarCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiToolbarSetFont(pToolbar, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.items", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "items");
	if ( __xgeXuiPageApplyToolbarItems(pPage, pToolbar, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "orientation");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiToolbarSetOrientation(pToolbar, (strcmp((const char*)xvoGetText(pVal), "vertical") == 0) ? XGE_XUI_SEPARATOR_VERTICAL : XGE_XUI_SEPARATOR_HORIZONTAL);
	}
	fItemWidth = pToolbar->fItemWidth;
	fItemHeight = pToolbar->fItemHeight;
	fSeparatorSize = pToolbar->fSeparatorSize;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemWidth", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemWidth", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fItemWidth = __xgeXuiPageValueToFloat(pVal, fItemWidth);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fItemHeight = __xgeXuiPageValueToFloat(pVal, fItemHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.separatorSize", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "separatorSize", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fSeparatorSize = __xgeXuiPageValueToFloat(pVal, fSeparatorSize);
	}
	xgeXuiToolbarSetItemSize(pToolbar, fItemWidth, fItemHeight, fSeparatorSize);
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pToolbar->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "itemColor", "color", &pToolbar->iItemColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pToolbar->iHoverColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pToolbar->iActiveColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "checkedColor", NULL, &pToolbar->iCheckedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pToolbar->iFocusColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pToolbar->iDisabledColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "separatorColor", NULL, &pToolbar->iSeparatorColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pToolbar->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledTextColor", NULL, &pToolbar->iDisabledTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageTextToStatusBarSection(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "center") == 0 ) {
		return XGE_XUI_STATUS_BAR_SECTION_CENTER;
	}
	if ( strcmp(sText, "right") == 0 ) {
		return XGE_XUI_STATUS_BAR_SECTION_RIGHT;
	}
	return XGE_XUI_STATUS_BAR_SECTION_LEFT;
}

static int __xgeXuiPageApplyStatusBarItems(xge_xui_page_t* pPage, xge_xui_status_bar pStatusBar, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	int iSection;
	int iIndex;
	float fWidth;
	float fMin;
	float fMax;
	float fValue;
	const char* sText;
	const char* sType;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "statusBar items must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_STATUS_BAR_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "statusBar items capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "items", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			iIndex = xgeXuiStatusBarAddText(pStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, (const char*)xvoGetText(pItem), 0.0f, 0);
			if ( iIndex < 0 ) {
				__xgeXuiPageSetPathError(pPage, sItemPath, "statusBar item capacity exceeded");
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			continue;
		}
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "statusBar item must be text or object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iSection = XGE_XUI_STATUS_BAR_SECTION_LEFT;
		pField = __xgeXuiPageTableGet(pItem, "section");
		if ( xvoType(pField) == XVO_DT_TEXT ) {
			iSection = __xgeXuiPageTextToStatusBarSection((const char*)xvoGetText(pField), iSection);
		}
		fWidth = 0.0f;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "width"), sItemPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( __xgeXuiPageValueExists(pField) ) {
			fWidth = __xgeXuiPageValueToFloat(pField, fWidth);
		}
		sType = "text";
		pField = __xgeXuiPageTableGet(pItem, "type");
		if ( xvoType(pField) == XVO_DT_TEXT ) {
			sType = (const char*)xvoGetText(pField);
		}
		if ( (strcmp(sType, "progress") == 0) ) {
			fMin = 0.0f;
			fMax = 1.0f;
			fValue = 0.0f;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "min"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				fMin = __xgeXuiPageValueToFloat(pField, fMin);
			}
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "max"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				fMax = __xgeXuiPageValueToFloat(pField, fMax);
			}
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "value"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				fValue = __xgeXuiPageValueToFloat(pField, fValue);
			}
			iIndex = xgeXuiStatusBarAddProgress(pStatusBar, iSection, fMin, fMax, fValue, fWidth);
		} else if ( (strcmp(sType, "spacer") == 0) ) {
			iIndex = xgeXuiStatusBarAddSpacer(pStatusBar, iSection, fWidth);
		} else {
			sText = "";
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "text"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				sText = (const char*)xvoGetText(pField);
			}
			iIndex = xgeXuiStatusBarAddText(pStatusBar, iSection, sText, fWidth, __xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "clickable"), 0));
		}
		if ( iIndex < 0 ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "statusBar item capacity exceeded");
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pField = __xgeXuiPageTableGet(pItem, "enabled");
		if ( __xgeXuiPageValueExists(pField) ) {
			xgeXuiStatusBarSetItemEnabled(pStatusBar, iIndex, __xgeXuiPageValueToBool(pField, 1));
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyStatusBar(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_status_bar pStatusBar;
	xvalue pVal;
	xge_font pFont;
	float fHeight;
	float fGap;
	float fItemPadding;
	char sFieldPath[128];

	if ( pPage->iStatusBarCount >= XGE_XUI_PAGE_STATUS_BAR_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "statusBar capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pStatusBar = &pPage->arrStatusBar[pPage->iStatusBarCount];
	if ( xgeXuiStatusBarInit(pStatusBar, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "statusBar initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iStatusBarCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiStatusBarSetFont(pStatusBar, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.items", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "items");
	if ( __xgeXuiPageApplyStatusBarItems(pPage, pStatusBar, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fHeight = pStatusBar->fHeight;
	fGap = pStatusBar->fGap;
	fItemPadding = pStatusBar->fItemPadding;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.barHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "barHeight", sFieldPath);
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.statusHeight", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "statusHeight", sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fHeight = __xgeXuiPageValueToFloat(pVal, fHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemGap", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemGap", sFieldPath);
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.gap", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "gap", sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fGap = __xgeXuiPageValueToFloat(pVal, fGap);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemPadding", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemPadding", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fItemPadding = __xgeXuiPageValueToFloat(pVal, fItemPadding);
	}
	xgeXuiStatusBarSetMetrics(pStatusBar, fHeight, fGap, fItemPadding);
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pStatusBar->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "borderColor", NULL, &pStatusBar->iBorderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "itemColor", "color", &pStatusBar->iItemColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pStatusBar->iHoverColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pStatusBar->iActiveColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pStatusBar->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledTextColor", NULL, &pStatusBar->iDisabledTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "progressTrackColor", NULL, &pStatusBar->iProgressTrackColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "progressFillColor", NULL, &pStatusBar->iProgressFillColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageApplyComboBoxItems(xge_xui_page_t* pPage, xge_xui_combo_box pCombo, xvalue pVal, const char* sPath)
{
	static const char* arrItems[XGE_XUI_PAGE_COMBO_BOX_CAPACITY][32];
	xvalue pItem;
	uint32 i;
	uint32 iCount;
	int iSlot;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "items must be array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > 32 ) {
		__xgeXuiPageSetPathError(pPage, sPath, "items capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSlot = pPage->iComboBoxCount - 1;
	if ( (iSlot < 0) || (iSlot >= XGE_XUI_PAGE_COMBO_BOX_CAPACITY) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "comboBox slot invalid");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "items", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( pItem == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_TEXT ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "item must be string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrItems[iSlot][i] = (const char*)xvoGetText(pItem);
	}
	xgeXuiComboBoxSetItems(pCombo, arrItems[iSlot], (int)iCount);
	return XGE_OK;
}

static int __xgeXuiPageApplyComboBox(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_combo_box pCombo;
	xvalue pVal;
	xge_font pFont;
	float fDropDownHeight;
	char sFieldPath[128];

	if ( pPage->iComboBoxCount >= XGE_XUI_PAGE_COMBO_BOX_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "comboBox capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pCombo = (xge_xui_combo_box)malloc(sizeof(xge_xui_combo_box_t));
	if ( pCombo == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "comboBox allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pCombo, 0, sizeof(*pCombo));
	if ( xgeXuiComboBoxInit(pCombo, pPage->pContext, pWidget) != XGE_OK ) {
		free(pCombo);
		__xgeXuiPageSetPathError(pPage, sPath, "comboBox initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrComboBox[pPage->iComboBoxCount] = pCombo;
	pPage->iComboBoxCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiComboBoxSetFont(pCombo, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.items", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "items");
	if ( __xgeXuiPageApplyComboBoxItems(pPage, pCombo, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selected", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "selected", sFieldPath);
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.value", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "value", sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiComboBoxSetSelected(pCombo, (int)__xgeXuiPageValueToFloat(pVal, (float)pCombo->iSelected));
	}
	fDropDownHeight = pCombo->fDropDownHeight;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.dropDownHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "dropDownHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fDropDownHeight = __xgeXuiPageValueToFloat(pVal, fDropDownHeight);
	}
	if ( fDropDownHeight != pCombo->fDropDownHeight ) {
		xgeXuiComboBoxSetDropDownHeight(pCombo, fDropDownHeight);
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", "background", &pCombo->iColorNormal, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "hoverColor", NULL, &pCombo->iColorHover, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "activeColor", NULL, &pCombo->iColorActive, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "focusColor", NULL, &pCombo->iColorFocus, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledColor", NULL, &pCombo->iColorDisabled, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pCombo->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "popupColor", NULL, &pCombo->iPopupColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiComboBoxSetColors(pCombo, pCombo->iColorNormal, pCombo->iColorHover, pCombo->iColorActive, pCombo->iColorFocus, pCombo->iColorDisabled, pCombo->iTextColor, pCombo->iPopupColor);
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onChange", sPath);
}

static int __xgeXuiPageApplyPopup(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_popup pPopup;
	xge_xui_widget pOwner;
	xvalue pVal;
	const char* sOwnerId;
	int bCloseOnOutside;
	int bCloseOnEscape;
	char sFieldPath[128];

	if ( pPage->iPopupCount >= XGE_XUI_PAGE_POPUP_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "popup capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPopup = (xge_xui_popup)malloc(sizeof(xge_xui_popup_t));
	if ( pPopup == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "popup allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pPopup, 0, sizeof(*pPopup));
	if ( xgeXuiPopupInit(pPopup, pPage->pContext, pWidget) != XGE_OK ) {
		free(pPopup);
		__xgeXuiPageSetPathError(pPage, sPath, "popup initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrPopup[pPage->iPopupCount] = pPopup;
	pPage->iPopupCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.owner", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "owner");
	if ( __xgeXuiPageValueExists(pVal) ) {
		if ( xvoType(pVal) != XVO_DT_TEXT ) {
			__xgeXuiPageSetPathError(pPage, sFieldPath, "owner must be string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sOwnerId = (const char*)xvoGetText(pVal);
		pOwner = __xgeXuiPageIndexFind(pPage, sOwnerId, 0, 0);
		if ( pOwner == NULL ) {
			__xgeXuiPageSetPathError(pPage, sFieldPath, "owner widget not found");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		xgeXuiPopupSetOwner(pPopup, pOwner);
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pPopup->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", NULL, &pPopup->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiPopupSetBackground(pPopup, pPopup->iBackgroundColor);
	bCloseOnOutside = pPopup->bCloseOnOutside;
	bCloseOnEscape = pPopup->bCloseOnEscape;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "closeOnOutside");
	if ( __xgeXuiPageValueExists(pVal) ) {
		bCloseOnOutside = __xgeXuiPageValueToBool(pVal, bCloseOnOutside);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "closeOnEscape");
	if ( __xgeXuiPageValueExists(pVal) ) {
		bCloseOnEscape = __xgeXuiPageValueToBool(pVal, bCloseOnEscape);
	}
	xgeXuiPopupSetAutoClose(pPopup, bCloseOnOutside, bCloseOnEscape);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "open");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiPopupSetOpen(pPopup, __xgeXuiPageValueToBool(pVal, pPopup->bOpen));
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onClose", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyTooltip(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_tooltip pTooltip;
	xge_xui_widget pOwner;
	xvalue pVal;
	xge_font pFont;
	const char* sOwnerId;
	const char* sText;
	float fOffsetX;
	float fOffsetY;
	char sFieldPath[128];

	(void)pWidget;
	if ( pPage->iTooltipCount >= XGE_XUI_PAGE_TOOLTIP_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tooltip capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.owner", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "owner");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "owner is required");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pVal) != XVO_DT_TEXT ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "owner must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sOwnerId = (const char*)xvoGetText(pVal);
	pOwner = __xgeXuiPageIndexFind(pPage, sOwnerId, 0, 0);
	if ( pOwner == NULL ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "owner widget not found");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTooltip = (xge_xui_tooltip)malloc(sizeof(xge_xui_tooltip_t));
	if ( pTooltip == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tooltip allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pTooltip, 0, sizeof(*pTooltip));
	if ( xgeXuiTooltipInit(pTooltip, pPage->pContext, pOwner) != XGE_OK ) {
		free(pTooltip);
		__xgeXuiPageSetPathError(pPage, sPath, "tooltip initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrTooltip[pPage->iTooltipCount] = pTooltip;
	pPage->iTooltipCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pTooltip->sText;
	xgeXuiTooltipSetText(pTooltip, (pFont != NULL) ? pFont : pTooltip->pFont, sText);
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pTooltip->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", NULL, &pTooltip->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pTooltip->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiTooltipSetColors(pTooltip, pTooltip->iBackgroundColor, pTooltip->iTextColor);
	fOffsetX = pTooltip->fOffsetX;
	fOffsetY = pTooltip->fOffsetY;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.offsetX", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "offsetX", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fOffsetX = __xgeXuiPageValueToFloat(pVal, fOffsetX);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.offsetY", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "offsetY", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fOffsetY = __xgeXuiPageValueToFloat(pVal, fOffsetY);
	}
	xgeXuiTooltipSetOffset(pTooltip, fOffsetX, fOffsetY);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "enabled");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiTooltipSetEnabled(pTooltip, __xgeXuiPageValueToBool(pVal, pTooltip->bEnabled));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "open");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiTooltipSetOpen(pTooltip, __xgeXuiPageValueToBool(pVal, xgeXuiTooltipIsOpen(pTooltip)));
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onOpen", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onClose", sPath);
}

static int __xgeXuiPageApplyMenuItems(xge_xui_page_t* pPage, xge_xui_menu pMenu, xvalue pVal, const char* sPath)
{
	static const char* arrItems[XGE_XUI_PAGE_MENU_CAPACITY][32];
	xvalue pItem;
	uint32 i;
	uint32 iCount;
	int iSlot;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "items must be array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > 32 ) {
		__xgeXuiPageSetPathError(pPage, sPath, "items capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSlot = pPage->iMenuCount - 1;
	if ( (iSlot < 0) || (iSlot >= XGE_XUI_PAGE_MENU_CAPACITY) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "menu slot invalid");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "items", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( pItem == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_TEXT ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "item must be string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrItems[iSlot][i] = (const char*)xvoGetText(pItem);
	}
	xgeXuiMenuSetItems(pMenu, arrItems[iSlot], (int)iCount);
	return XGE_OK;
}

static int __xgeXuiPageApplyMenuEnabledItems(xge_xui_page_t* pPage, xge_xui_menu pMenu, xvalue pVal, const char* sPath)
{
	static int arrEnabled[XGE_XUI_PAGE_MENU_CAPACITY][32];
	xvalue pItem;
	uint32 i;
	uint32 iCount;
	int iSlot;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "enabledItems must be array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > 32 ) {
		__xgeXuiPageSetPathError(pPage, sPath, "enabledItems capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSlot = pPage->iMenuCount - 1;
	if ( (iSlot < 0) || (iSlot >= XGE_XUI_PAGE_MENU_CAPACITY) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "menu slot invalid");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "enabledItems", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( pItem == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrEnabled[iSlot][i] = __xgeXuiPageValueToBool(pItem, 1);
	}
	xgeXuiMenuSetEnabledItems(pMenu, arrEnabled[iSlot], (int)iCount);
	return XGE_OK;
}

static int __xgeXuiPageApplyMenu(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_menu pMenu;
	xge_xui_widget pOwner;
	xvalue pVal;
	xge_font pFont;
	const char* sOwnerId;
	float fWidth;
	float fMaxHeight;
	float fX;
	float fY;
	char sFieldPath[128];

	if ( pPage->iMenuCount >= XGE_XUI_PAGE_MENU_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "menu capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pOwner = pWidget;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.owner", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "owner");
	if ( __xgeXuiPageValueExists(pVal) ) {
		if ( xvoType(pVal) != XVO_DT_TEXT ) {
			__xgeXuiPageSetPathError(pPage, sFieldPath, "owner must be string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sOwnerId = (const char*)xvoGetText(pVal);
		pOwner = __xgeXuiPageIndexFind(pPage, sOwnerId, 0, 0);
		if ( pOwner == NULL ) {
			__xgeXuiPageSetPathError(pPage, sFieldPath, "owner widget not found");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	pMenu = (xge_xui_menu)malloc(sizeof(xge_xui_menu_t));
	if ( pMenu == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "menu allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pMenu, 0, sizeof(*pMenu));
	if ( xgeXuiMenuInit(pMenu, pPage->pContext, pOwner) != XGE_OK ) {
		free(pMenu);
		__xgeXuiPageSetPathError(pPage, sPath, "menu initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrMenu[pPage->iMenuCount] = pMenu;
	pPage->arrMenuWidget[pPage->iMenuCount] = pWidget;
	pPage->iMenuCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiMenuSetFont(pMenu, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.items", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "items");
	if ( __xgeXuiPageApplyMenuItems(pPage, pMenu, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.enabledItems", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "enabledItems");
	if ( __xgeXuiPageApplyMenuEnabledItems(pPage, pMenu, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fWidth = pMenu->fWidth;
	fMaxHeight = pMenu->fMaxHeight;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.width", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "menuWidth", sFieldPath);
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "width", sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fWidth = __xgeXuiPageValueToFloat(pVal, fWidth);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.maxHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "maxHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fMaxHeight = __xgeXuiPageValueToFloat(pVal, fMaxHeight);
	}
	xgeXuiMenuSetSize(pMenu, fWidth, fMaxHeight);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pMenu->fItemHeight = __xgeXuiPageValueToFloat(pVal, pMenu->fItemHeight);
		xgeXuiListViewSetItemHeight(&pMenu->tList, pMenu->fItemHeight);
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pMenu->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", NULL, &pMenu->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "rowColor", NULL, &pMenu->iRowColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "selectedColor", NULL, &pMenu->iSelectedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "textColor", NULL, &pMenu->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "disabledTextColor", NULL, &pMenu->iDisabledTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiMenuSetColors(pMenu, pMenu->iBackgroundColor, pMenu->iRowColor, pMenu->iSelectedColor, pMenu->iTextColor, pMenu->iDisabledTextColor);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "open");
	if ( __xgeXuiPageValueExists(pVal) && __xgeXuiPageValueToBool(pVal, 0) ) {
		fX = pOwner->tRect.fX;
		fY = pOwner->tRect.fY + pOwner->tRect.fH;
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.x", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "x", sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( __xgeXuiPageValueExists(pVal) ) {
			fX = __xgeXuiPageValueToFloat(pVal, fX);
		}
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.y", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "y", sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( __xgeXuiPageValueExists(pVal) ) {
			fY = __xgeXuiPageValueToFloat(pVal, fY);
		}
		xgeXuiMenuOpen(pMenu, fX, fY);
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageApplyDialog(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_dialog pDialog;
	xvalue pVal;
	xge_font pFont;
	const char* sTitle;
	char sFieldPath[128];

	if ( pPage->iDialogCount >= XGE_XUI_PAGE_DIALOG_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "dialog capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pDialog = (xge_xui_dialog)malloc(sizeof(xge_xui_dialog_t));
	if ( pDialog == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "dialog allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pDialog, 0, sizeof(*pDialog));
	if ( xgeXuiDialogInit(pDialog, pPage->pContext, pWidget) != XGE_OK ) {
		free(pDialog);
		__xgeXuiPageSetPathError(pPage, sPath, "dialog initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrDialog[pPage->iDialogCount] = pDialog;
	pPage->iDialogCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.title", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "title");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "title must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sTitle = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : pDialog->sTitle;
	xgeXuiDialogSetTitle(pDialog, pFont, sTitle);
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backdropColor", NULL, &pDialog->iBackdropColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pDialog->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "color", NULL, &pDialog->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "titleColor", NULL, &pDialog->iTitleColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "closeColor", NULL, &pDialog->iCloseColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiDialogSetColors(pDialog, pDialog->iBackdropColor, pDialog->iBackgroundColor, pDialog->iTitleColor, pDialog->iCloseColor);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "modal");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiDialogSetModal(pDialog, __xgeXuiPageValueToBool(pVal, pDialog->bModal));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "closeOnEscape");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiDialogSetCloseOnEscape(pDialog, __xgeXuiPageValueToBool(pVal, pDialog->bCloseOnEscape));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "showClose");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiDialogSetShowClose(pDialog, __xgeXuiPageValueToBool(pVal, pDialog->bShowClose));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "open");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiDialogSetOpen(pDialog, __xgeXuiPageValueToBool(pVal, pDialog->bOpen));
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onClose", sPath);
}

static int __xgeXuiPageTextToMessageBoxType(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "warning") == 0 ) {
		return XGE_XUI_MESSAGE_BOX_WARNING;
	}
	if ( strcmp(sText, "error") == 0 ) {
		return XGE_XUI_MESSAGE_BOX_ERROR;
	}
	if ( strcmp(sText, "question") == 0 ) {
		return XGE_XUI_MESSAGE_BOX_QUESTION;
	}
	return XGE_XUI_MESSAGE_BOX_INFO;
}

static int __xgeXuiPageTextToMessageBoxButtons(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( (strcmp(sText, "okCancel") == 0) || (strcmp(sText, "OKCancel") == 0) ) {
		return XGE_XUI_MESSAGE_BOX_OK_CANCEL;
	}
	if ( (strcmp(sText, "yesNo") == 0) || (strcmp(sText, "YesNo") == 0) ) {
		return XGE_XUI_MESSAGE_BOX_YES_NO;
	}
	if ( (strcmp(sText, "yesNoCancel") == 0) || (strcmp(sText, "YesNoCancel") == 0) ) {
		return XGE_XUI_MESSAGE_BOX_YES_NO_CANCEL;
	}
	return XGE_XUI_MESSAGE_BOX_OK;
}

static int __xgeXuiPageApplyMessageBox(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_message_box pBox;
	xvalue pVal;
	xge_font pFont;
	const char* sTitle;
	const char* sMessage;
	char sFieldPath[128];

	if ( pPage->iMessageBoxCount >= XGE_XUI_PAGE_MESSAGE_BOX_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "messageBox capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBox = (xge_xui_message_box)malloc(sizeof(xge_xui_message_box_t));
	if ( pBox == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "messageBox allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pBox, 0, sizeof(*pBox));
	if ( xgeXuiMessageBoxInit(pBox, pPage->pContext, pWidget) != XGE_OK ) {
		free(pBox);
		__xgeXuiPageSetPathError(pPage, sPath, "messageBox initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrMessageBox[pPage->iMessageBoxCount] = pBox;
	pPage->iMessageBoxCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "title");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.title", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "title must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sTitle = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : "";
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "message");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	}
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.message", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "message must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sMessage = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : "";
	xgeXuiMessageBoxSetText(pBox, pFont, sTitle, sMessage);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "messageType");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "kind");
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "typeName");
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiMessageBoxSetType(pBox, __xgeXuiPageTextToMessageBoxType((const char*)xvoGetText(pVal), pBox->iType));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "buttons");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiMessageBoxSetButtons(pBox, __xgeXuiPageTextToMessageBoxButtons((const char*)xvoGetText(pVal), pBox->iButtons));
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backdropColor", NULL, &pBox->tDialog.iBackdropColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "backgroundColor", "background", &pBox->tDialog.iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "titleColor", NULL, &pBox->tDialog.iTitleColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "closeColor", NULL, &pBox->tDialog.iCloseColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "messageColor", "textColor", &pBox->iMessageColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "buttonColor", NULL, &pBox->iButtonColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "buttonHoverColor", NULL, &pBox->iButtonHoverColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToggleColor(pPage, pWidget, pNode, pStyle, "buttonTextColor", NULL, &pBox->iButtonTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiMessageBoxSetColors(pBox, pBox->tDialog.iBackdropColor, pBox->tDialog.iBackgroundColor, pBox->tDialog.iTitleColor, pBox->tDialog.iCloseColor, pBox->iMessageColor, pBox->iButtonColor, pBox->iButtonHoverColor, pBox->iButtonTextColor);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "open");
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiMessageBoxSetOpen(pBox, __xgeXuiPageValueToBool(pVal, xgeXuiMessageBoxIsOpen(pBox)));
	}
	if ( __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onResult", sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onClose", sPath);
}

static int __xgeXuiPageApplyLabel(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_label pLabel;
	xvalue pVal;
	xge_font pFont;
	const char* sText;
	uint32_t iFlags;
	char sFieldPath[128];
	char sBindKey[XGE_XUI_MODEL_KEY_CAPACITY];
	int bTextBinding;

	if ( pPage->iLabelCount >= XGE_XUI_PAGE_LABEL_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "label capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pVal != NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "text");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "text must be string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : "";
	bTextBinding = __xgeXuiPageParseModelBinding(sText, sBindKey, sizeof(sBindKey));
	pLabel = &pPage->arrLabel[pPage->iLabelCount];
	if ( xgeXuiLabelInit(pLabel, pWidget, pFont, bTextBinding ? "" : sText) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "label initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iLabelCount++;
	if ( bTextBinding ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.text", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		if ( __xgeXuiPageAddModelBinding(pPage, pWidget, pLabel, XGE_XUI_PAGE_MODEL_BIND_LABEL_TEXT, -1, sBindKey, sFieldPath) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.textColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "textColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.color", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "color", sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiLabelSetColor(pLabel, __xgeXuiPageValueToColor(pVal, pLabel->iColor));
	}
	iFlags = pLabel->iTextFlags;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "textAlign");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		iFlags = __xgeXuiPageTextToTextAlign((const char*)xvoGetText(pVal), iFlags);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "textVAlign");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		iFlags = __xgeXuiPageTextToTextVAlign((const char*)xvoGetText(pVal), iFlags);
	}
	xgeXuiLabelSetAlign(pLabel, iFlags);
	return XGE_OK;
}

static int __xgeXuiPageApplySeparator(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_separator pSeparator;
	xvalue pVal;
	const char* sText;
	char sFieldPath[128];

	if ( pPage->iSeparatorCount >= XGE_XUI_PAGE_SEPARATOR_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "separator capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSeparator = &pPage->arrSeparator[pPage->iSeparatorCount];
	if ( xgeXuiSeparatorInit(pSeparator, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "separator initialization failed");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->iSeparatorCount++;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "orientation");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		xgeXuiSeparatorSetOrientation(pSeparator, (sText != NULL && (strcmp(sText, "vertical") == 0)) ? XGE_XUI_SEPARATOR_VERTICAL : XGE_XUI_SEPARATOR_HORIZONTAL);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.thickness", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "thickness", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiSeparatorSetThickness(pSeparator, __xgeXuiPageValueToFloat(pVal, pSeparator->fThickness));
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.color", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "color", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.background", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "background", sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiSeparatorSetColor(pSeparator, __xgeXuiPageValueToColor(pVal, pSeparator->iColor));
	}
	return XGE_OK;
}

static int __xgeXuiPageValueToVec2Token(xge_xui_page_t* pPage, xvalue pVal, float* pX, float* pY, const char* sPath)
{
	xvalue pItem;
	xvalue pXVal;
	xvalue pYVal;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, sPath);
	if ( pVal == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pVal) == XVO_DT_ARRAY ) {
		if ( xvoArrayItemCount(pVal) < 2 ) {
			__xgeXuiPageSetPathError(pPage, sPath, "expected [x, y]");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		snprintf(sItemPath, sizeof(sItemPath), "%s[0]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 0), sItemPath);
		if ( pItem == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		*pX = __xgeXuiPageValueToFloat(pItem, *pX);
		snprintf(sItemPath, sizeof(sItemPath), "%s[1]", sPath);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, 1), sItemPath);
		if ( pItem == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		*pY = __xgeXuiPageValueToFloat(pItem, *pY);
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_TABLE ) {
		pXVal = __xgeXuiPageTableGet(pVal, "x");
		if ( !__xgeXuiPageValueExists(pXVal) ) {
			pXVal = __xgeXuiPageTableGet(pVal, "width");
		}
		if ( !__xgeXuiPageValueExists(pXVal) ) {
			pXVal = __xgeXuiPageTableGet(pVal, "w");
		}
		pYVal = __xgeXuiPageTableGet(pVal, "y");
		if ( !__xgeXuiPageValueExists(pYVal) ) {
			pYVal = __xgeXuiPageTableGet(pVal, "height");
		}
		if ( !__xgeXuiPageValueExists(pYVal) ) {
			pYVal = __xgeXuiPageTableGet(pVal, "h");
		}
		if ( __xgeXuiPageValueExists(pXVal) ) {
			snprintf(sItemPath, sizeof(sItemPath), "%s.x", sPath);
			sItemPath[sizeof(sItemPath) - 1] = 0;
			pItem = __xgeXuiPageResolveTokenValue(pPage, pXVal, sItemPath);
			if ( pItem == NULL ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			*pX = __xgeXuiPageValueToFloat(pItem, *pX);
		}
		if ( __xgeXuiPageValueExists(pYVal) ) {
			snprintf(sItemPath, sizeof(sItemPath), "%s.y", sPath);
			sItemPath[sizeof(sItemPath) - 1] = 0;
			pItem = __xgeXuiPageResolveTokenValue(pPage, pYVal, sItemPath);
			if ( pItem == NULL ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			*pY = __xgeXuiPageValueToFloat(pItem, *pY);
		}
		return XGE_OK;
	}
	__xgeXuiPageSetPathError(pPage, sPath, "expected array or object");
	return XGE_ERROR_INVALID_ARGUMENT;
}

static int __xgeXuiPageApplyScrollViewColor(xge_xui_page_t* pPage, xge_xui_scroll_view pScroll, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
		if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiWidgetMarkPaint(pScroll->pWidget);
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyScrollView(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_scroll_view pScroll;
	xvalue pVal;
	float fContentW;
	float fContentH;
	float fScrollX;
	float fScrollY;
	char sFieldPath[128];

	if ( pPage->iScrollViewCount >= XGE_XUI_PAGE_SCROLL_VIEW_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "scrollView capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pScroll = &pPage->arrScrollView[pPage->iScrollViewCount];
	if ( xgeXuiScrollViewInit(pScroll, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "scrollView initialization failed");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->iScrollViewCount++;
	fContentW = pScroll->fContentW;
	fContentH = pScroll->fContentH;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "contentSize");
	if ( __xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.contentSize", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		if ( __xgeXuiPageValueToVec2Token(pPage, pVal, &fContentW, &fContentH, sFieldPath) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.contentWidth", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "contentWidth", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fContentW = __xgeXuiPageValueToFloat(pVal, fContentW);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.contentHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "contentHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fContentH = __xgeXuiPageValueToFloat(pVal, fContentH);
	}
	xgeXuiScrollViewSetContentSize(pScroll, fContentW, fContentH);
	fScrollX = pScroll->fScrollX;
	fScrollY = pScroll->fScrollY;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "offset");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "scrollOffset");
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "contentOffset");
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.offset", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		if ( __xgeXuiPageValueToVec2Token(pPage, pVal, &fScrollX, &fScrollY, sFieldPath) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.scrollX", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "scrollX", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fScrollX = __xgeXuiPageValueToFloat(pVal, fScrollX);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.scrollY", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "scrollY", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fScrollY = __xgeXuiPageValueToFloat(pVal, fScrollY);
	}
	xgeXuiScrollViewSetOffset(pScroll, fScrollX, fScrollY);
	if ( __xgeXuiPageApplyScrollViewColor(pPage, pScroll, pNode, pStyle, "backgroundColor", "background", &pScroll->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyScrollViewColor(pPage, pScroll, pNode, pStyle, "barColor", NULL, &pScroll->iBarColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyScrollViewColor(pPage, pScroll, pNode, pStyle, "thumbColor", NULL, &pScroll->iThumbColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageVirtualListCount(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_page_virtual_list_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (xge_xui_page_virtual_list_adapter_t*)pUser;
	return (pAdapter != NULL) ? pAdapter->iItemCount : 0;
}

static int __xgeXuiPageApplyTreeViewColor(xge_xui_page_t* pPage, xge_xui_tree_view pTree, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiTreeViewSetColors(pTree, pTree->iBackgroundColor, pTree->iRowColor, pTree->iSelectedColor, pTree->iTextColor, pTree->iBarColor, pTree->iThumbColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageValueToTreeNodeId(xge_xui_page_t* pPage, xvalue pVal, int iDefault, const char* sPath)
{
	const char* sText;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return iDefault;
	}
	if ( xvoType(pVal) == XVO_DT_INT ) {
		return (int)xvoGetInt(pVal);
	}
	if ( xvoType(pVal) == XVO_DT_FLOAT ) {
		return (int)xvoGetFloat(pVal);
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		if ( (sText != NULL) && (sText[0] != 0) ) {
			return (int)strtol(sText, NULL, 10);
		}
	}
	__xgeXuiPageSetPathError(pPage, sPath, "treeView node id must be number or numeric text");
	return iDefault;
}

static int __xgeXuiPageApplyTreeViewNodes(xge_xui_page_t* pPage, xge_xui_tree_view pTree, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	int iId;
	int iParent;
	int iExpanded;
	const char* sText;
	char sItemPath[128];
	char sFieldPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "treeView nodes must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_TREE_VIEW_NODE_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "treeView nodes capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "nodes", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "treeView node must be object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.id", 120, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "id"), sFieldPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( !__xgeXuiPageValueExists(pField) ) {
			__xgeXuiPageSetPathError(pPage, sFieldPath, "treeView node id is required");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iId = __xgeXuiPageValueToTreeNodeId(pPage, pField, -1, sFieldPath);
		if ( pPage->sError[0] != 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.parent", 120, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "parent"), sFieldPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iParent = __xgeXuiPageValueToTreeNodeId(pPage, pField, -1, sFieldPath);
		if ( pPage->sError[0] != 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sText = "";
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "text"), sItemPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pField) == XVO_DT_TEXT ) {
			sText = (const char*)xvoGetText(pField);
		}
		if ( xgeXuiTreeViewAddNode(pTree, iId, iParent, sText) < 0 ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "treeView node add failed");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pField = __xgeXuiPageTableGet(pItem, "expanded");
		if ( __xgeXuiPageValueExists(pField) ) {
			iExpanded = __xgeXuiPageValueToBool(pField, 0);
			xgeXuiTreeViewSetNodeExpanded(pTree, iId, iExpanded);
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyTreeView(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_tree_view pTree;
	xvalue pVal;
	xge_font pFont;
	float fItemHeight;
	float fIndent;
	float fScrollY;
	char sFieldPath[128];

	if ( pPage->iTreeViewCount >= XGE_XUI_PAGE_TREE_VIEW_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "treeView capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTree = &pPage->arrTreeView[pPage->iTreeViewCount];
	if ( xgeXuiTreeViewInit(pTree, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "treeView initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iTreeViewCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiTreeViewSetFont(pTree, pFont);
	}
	fItemHeight = pTree->fItemHeight;
	fIndent = pTree->fIndent;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fItemHeight = __xgeXuiPageValueToFloat(pVal, fItemHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.indent", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "indent", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fIndent = __xgeXuiPageValueToFloat(pVal, fIndent);
	}
	xgeXuiTreeViewSetMetrics(pTree, fItemHeight, fIndent);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.nodes", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "nodes");
	if ( __xgeXuiPageApplyTreeViewNodes(pPage, pTree, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selected", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "selected", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiTreeViewSetSelected(pTree, __xgeXuiPageValueToTreeNodeId(pPage, pVal, -1, sFieldPath));
	}
	fScrollY = pTree->fScrollY;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.scrollY", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "scrollY", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fScrollY = __xgeXuiPageValueToFloat(pVal, fScrollY);
	}
	xgeXuiTreeViewSetScroll(pTree, fScrollY);
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "backgroundColor", "background", &pTree->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "rowColor", "color", &pTree->iRowColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "selectedColor", NULL, &pTree->iSelectedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "textColor", NULL, &pTree->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "barColor", NULL, &pTree->iBarColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "thumbColor", NULL, &pTree->iThumbColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "expanderColor", NULL, &pTree->iExpanderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTreeViewColor(pPage, pTree, pNode, pStyle, "disabledTextColor", NULL, &pTree->iDisabledTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageTableViewCount(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_page_table_view_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (xge_xui_page_table_view_adapter_t*)pUser;
	return (pAdapter != NULL) ? pAdapter->iRowCount : 0;
}

static int __xgeXuiPageTableViewCell(xge_xui_widget pWidget, int iRow, int iColumn, char* sBuffer, int iBufferSize, void* pUser)
{
	xge_xui_page_table_view_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (xge_xui_page_table_view_adapter_t*)pUser;
	if ( (sBuffer == NULL) || (iBufferSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBuffer[0] = 0;
	if ( (pAdapter == NULL) || (iRow < 0) || (iRow >= pAdapter->iRowCount) || (iColumn < 0) || (iColumn >= pAdapter->iColumnCount) ) {
		return XGE_OK;
	}
	snprintf(sBuffer, (size_t)iBufferSize, "%s", pAdapter->arrCell[iRow][iColumn]);
	return XGE_OK;
}

static int __xgeXuiPageValueToCellText(xvalue pVal, char* sBuffer, int iBufferSize)
{
	const char* sText;

	if ( (sBuffer == NULL) || (iBufferSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBuffer[0] = 0;
	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		snprintf(sBuffer, (size_t)iBufferSize, "%s", (sText != NULL) ? sText : "");
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_INT ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%lld", (long long)xvoGetInt(pVal));
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_FLOAT ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%.6g", xvoGetFloat(pVal));
		return XGE_OK;
	}
	if ( xvoType(pVal) == XVO_DT_BOOL ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%s", xvoGetBool(pVal) ? "true" : "false");
		return XGE_OK;
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyTableViewColor(xge_xui_page_t* pPage, xge_xui_table_view pTable, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiTableViewSetColors(pTable, pTable->iBackgroundColor, pTable->iHeaderColor, pTable->iRowColor, pTable->iSelectedColor, pTable->iGridColor, pTable->iTextColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyTableViewColumns(xge_xui_page_t* pPage, xge_xui_table_view pTable, xvalue pVal, const char* sPath)
{
	xge_xui_table_view_column_t arrColumns[XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	char sItemPath[128];
	char sFieldPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tableView columns must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tableView column capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(arrColumns, 0, sizeof(arrColumns));
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "columns", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrColumns[i].iId = (int)i;
		arrColumns[i].sTitle = "";
		arrColumns[i].fWidth = 80.0f;
		arrColumns[i].fMinWidth = 30.0f;
		arrColumns[i].iAlign = XGE_TEXT_ALIGN_LEFT;
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			arrColumns[i].sTitle = (const char*)xvoGetText(pItem);
		} else if ( xvoType(pItem) == XVO_DT_TABLE ) {
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "title"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				arrColumns[i].sTitle = (const char*)xvoGetText(pField);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.id", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "id"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				arrColumns[i].iId = (int)__xgeXuiPageValueToFloat(pField, (float)i);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.width", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "width"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				arrColumns[i].fWidth = __xgeXuiPageValueToFloat(pField, arrColumns[i].fWidth);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.minWidth", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "minWidth"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				arrColumns[i].fMinWidth = __xgeXuiPageValueToFloat(pField, arrColumns[i].fMinWidth);
			}
			pField = __xgeXuiPageTableGet(pItem, "align");
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				arrColumns[i].iAlign = __xgeXuiPageTextToTextAlign((const char*)xvoGetText(pField), arrColumns[i].iAlign);
			}
		} else {
			__xgeXuiPageSetPathError(pPage, sItemPath, "tableView column must be text or object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	xgeXuiTableViewSetColumns(pTable, arrColumns, (int)iCount);
	return XGE_OK;
}

static int __xgeXuiPageApplyTableViewRows(xge_xui_page_t* pPage, xge_xui_page_table_view_adapter_t* pAdapter, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pCell;
	uint32 i;
	uint32 j;
	uint32 iCount;
	uint32 iCellCount;
	char sItemPath[128];
	char sCellPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tableView rows must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tableView row capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pAdapter->iRowCount = (int)iCount;
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "rows", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_ARRAY ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "tableView row must be an array");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iCellCount = xvoArrayItemCount(pItem);
		if ( iCellCount > (uint32)pAdapter->iColumnCount ) {
			iCellCount = (uint32)pAdapter->iColumnCount;
		}
		for ( j = 0; j < iCellCount; j++ ) {
			snprintf(sCellPath, sizeof(sCellPath), "%.*s[%u]", 100, sItemPath, j);
			sCellPath[sizeof(sCellPath) - 1] = 0;
			pCell = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pItem, j), sCellPath);
			if ( (pCell == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			__xgeXuiPageValueToCellText(pCell, pAdapter->arrCell[i][j], XGE_XUI_PAGE_TABLE_VIEW_CELL_CAPACITY);
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyTableView(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_table_view pTable;
	xge_xui_page_table_view_adapter_t* pAdapter;
	xvalue pVal;
	xge_font pFont;
	float fHeaderHeight;
	float fRowHeight;
	float fScrollY;
	char sFieldPath[128];

	if ( pPage->iTableViewCount >= XGE_XUI_PAGE_TABLE_VIEW_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tableView capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTable = &pPage->arrTableView[pPage->iTableViewCount];
	pAdapter = (xge_xui_page_table_view_adapter_t*)calloc(1, sizeof(*pAdapter));
	if ( pAdapter == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "tableView adapter allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiTableViewInit(pTable, pPage->pContext, pWidget) != XGE_OK ) {
		free(pAdapter);
		__xgeXuiPageSetPathError(pPage, sPath, "tableView initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->arrTableViewAdapter[pPage->iTableViewCount] = pAdapter;
	pPage->iTableViewCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiTableViewSetFont(pTable, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.columns", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "columns");
	if ( __xgeXuiPageApplyTableViewColumns(pPage, pTable, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pAdapter->iColumnCount = pTable->iColumnCount;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.rows", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "rows");
	if ( __xgeXuiPageApplyTableViewRows(pPage, pAdapter, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiTableViewSetAdapter(pTable, __xgeXuiPageTableViewCount, __xgeXuiPageTableViewCell, pAdapter);
	fHeaderHeight = pTable->fHeaderHeight;
	fRowHeight = pTable->fRowHeight;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.headerHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "headerHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fHeaderHeight = __xgeXuiPageValueToFloat(pVal, fHeaderHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.rowHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "rowHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fRowHeight = __xgeXuiPageValueToFloat(pVal, fRowHeight);
	}
	xgeXuiTableViewSetMetrics(pTable, fHeaderHeight, fRowHeight);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selected", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "selected", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiTableViewSetSelected(pTable, (int)__xgeXuiPageValueToFloat(pVal, -1.0f));
	}
	fScrollY = pTable->fScrollY;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.scrollY", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "scrollY", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fScrollY = __xgeXuiPageValueToFloat(pVal, fScrollY);
	}
	xgeXuiTableViewSetScroll(pTable, fScrollY);
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "backgroundColor", "background", &pTable->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "headerColor", NULL, &pTable->iHeaderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "rowColor", "color", &pTable->iRowColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "selectedColor", NULL, &pTable->iSelectedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "gridColor", NULL, &pTable->iGridColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "textColor", NULL, &pTable->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.headerTextColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "headerTextColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pTable->iHeaderTextColor = __xgeXuiPageValueToColor(pVal, pTable->iHeaderTextColor);
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "barColor", NULL, &pTable->iBarColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyTableViewColor(pPage, pTable, pNode, pStyle, "thumbColor", NULL, &pTable->iThumbColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.headerTextColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "headerTextColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pTable->iHeaderTextColor = __xgeXuiPageValueToColor(pVal, pTable->iHeaderTextColor);
	}
	return (__xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath) == XGE_OK) ? __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSort", sPath) : XGE_ERROR_INVALID_ARGUMENT;
}

static int __xgeXuiPageApplyPropertyGridColor(xge_xui_page_t* pPage, xge_xui_property_grid pGrid, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiPropertyGridSetColors(pGrid, pGrid->iBackgroundColor, pGrid->iCategoryColor, pGrid->iRowColor, pGrid->iSelectedColor, pGrid->iGridColor, pGrid->iTextColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageValueToPropertyGridEditor(xge_xui_page_t* pPage, xvalue pVal, int iDefault, const char* sPath)
{
	const char* sText;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return iDefault;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		if ( (sText == NULL) || (sText[0] == 0) || (strcmp(sText, "text") == 0) ) {
			return XGE_XUI_PROPERTY_GRID_EDITOR_TEXT;
		}
		if ( strcmp(sText, "number") == 0 ) {
			return XGE_XUI_PROPERTY_GRID_EDITOR_NUMBER;
		}
		if ( (strcmp(sText, "bool") == 0) || (strcmp(sText, "boolean") == 0) ) {
			return XGE_XUI_PROPERTY_GRID_EDITOR_BOOL;
		}
		if ( strcmp(sText, "enum") == 0 ) {
			return XGE_XUI_PROPERTY_GRID_EDITOR_ENUM;
		}
		if ( strcmp(sText, "color") == 0 ) {
			return XGE_XUI_PROPERTY_GRID_EDITOR_COLOR;
		}
		__xgeXuiPageSetPathError(pPage, sPath, "propertyGrid editor is invalid");
		return iDefault;
	}
	if ( (xvoType(pVal) == XVO_DT_INT) || (xvoType(pVal) == XVO_DT_FLOAT) ) {
		return (int)__xgeXuiPageValueToFloat(pVal, (float)iDefault);
	}
	__xgeXuiPageSetPathError(pPage, sPath, "propertyGrid editor must be text or number");
	return iDefault;
}

static int __xgeXuiPageApplyPropertyGridProperties(xge_xui_page_t* pPage, xge_xui_property_grid pGrid, xvalue pVal, int iCategory, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	const char* sName;
	const char* sValue;
	int iEditor;
	int iProp;
	char sItemPath[128];
	char sFieldPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "propertyGrid properties must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "properties", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "propertyGrid property must be object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.name", 100, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "name"), sFieldPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sName = (xvoType(pField) == XVO_DT_TEXT) ? (const char*)xvoGetText(pField) : "";
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.value", 100, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "value"), sFieldPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sValue = (xvoType(pField) == XVO_DT_TEXT) ? (const char*)xvoGetText(pField) : "";
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.editor", 100, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "editor"), sFieldPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iEditor = __xgeXuiPageValueToPropertyGridEditor(pPage, pField, XGE_XUI_PROPERTY_GRID_EDITOR_TEXT, sFieldPath);
		if ( pPage->sError[0] != 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iProp = xgeXuiPropertyGridAddProperty(pGrid, iCategory, sName, sValue, iEditor);
		if ( iProp < 0 ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "propertyGrid property add failed");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		xgeXuiPropertyGridSetPropertyFlags(
			pGrid,
			iProp,
			__xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "readonly"), __xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "readOnly"), 0)),
			__xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "changed"), __xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "defaultChanged"), 0)),
			__xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "error"), 0));
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyPropertyGridCategories(xge_xui_page_t* pPage, xge_xui_property_grid pGrid, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	const char* sName;
	int iCategory;
	char sItemPath[128];
	char sFieldPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "propertyGrid categories must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	xgeXuiPropertyGridClear(pGrid);
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "categories", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "propertyGrid category must be object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.name", 100, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "name"), sFieldPath);
		if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sName = (xvoType(pField) == XVO_DT_TEXT) ? (const char*)xvoGetText(pField) : "";
		iCategory = xgeXuiPropertyGridAddCategory(pGrid, sName, __xgeXuiPageValueToBool(__xgeXuiPageTableGet(pItem, "expanded"), 1));
		if ( iCategory < 0 ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "propertyGrid category add failed");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.properties", 100, sItemPath);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		if ( __xgeXuiPageApplyPropertyGridProperties(pPage, pGrid, __xgeXuiPageTableGet(pItem, "properties"), iCategory, sFieldPath) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyPropertyGrid(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_property_grid pGrid;
	xvalue pVal;
	xge_font pFont;
	float fRowHeight;
	float fNameWidth;
	char sFieldPath[128];

	if ( pPage->iPropertyGridCount >= XGE_XUI_PAGE_PROPERTY_GRID_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "propertyGrid capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pGrid = &pPage->arrPropertyGrid[pPage->iPropertyGridCount];
	if ( xgeXuiPropertyGridInit(pGrid, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "propertyGrid initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iPropertyGridCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiPropertyGridSetFont(pGrid, pFont);
	}
	fRowHeight = pGrid->fRowHeight;
	fNameWidth = pGrid->fNameWidth;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.rowHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "rowHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fRowHeight = __xgeXuiPageValueToFloat(pVal, fRowHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.nameWidth", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "nameWidth", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fNameWidth = __xgeXuiPageValueToFloat(pVal, fNameWidth);
	}
	xgeXuiPropertyGridSetMetrics(pGrid, fRowHeight, fNameWidth);
	if ( __xgeXuiPageApplyPropertyGridColor(pPage, pGrid, pNode, pStyle, "backgroundColor", "background", &pGrid->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyPropertyGridColor(pPage, pGrid, pNode, pStyle, "categoryColor", NULL, &pGrid->iCategoryColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyPropertyGridColor(pPage, pGrid, pNode, pStyle, "rowColor", "color", &pGrid->iRowColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyPropertyGridColor(pPage, pGrid, pNode, pStyle, "selectedColor", NULL, &pGrid->iSelectedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyPropertyGridColor(pPage, pGrid, pNode, pStyle, "gridColor", NULL, &pGrid->iGridColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyPropertyGridColor(pPage, pGrid, pNode, pStyle, "textColor", NULL, &pGrid->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.valueColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "valueColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pGrid->iValueColor = __xgeXuiPageValueToColor(pVal, pGrid->iValueColor);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.readonlyColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "readonlyColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pGrid->iReadonlyColor = __xgeXuiPageValueToColor(pVal, pGrid->iReadonlyColor);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.changedColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "changedColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pGrid->iChangedColor = __xgeXuiPageValueToColor(pVal, pGrid->iChangedColor);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.errorColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "errorColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pGrid->iErrorColor = __xgeXuiPageValueToColor(pVal, pGrid->iErrorColor);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.categories", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "categories");
	if ( __xgeXuiPageApplyPropertyGridCategories(pPage, pGrid, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selected", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "selected", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiPropertyGridSetSelected(pGrid, (int)__xgeXuiPageValueToFloat(pVal, -1.0f));
	}
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageApplyBreadcrumbColor(xge_xui_page_t* pPage, xge_xui_breadcrumb pBreadcrumb, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiBreadcrumbSetColors(pBreadcrumb, pBreadcrumb->iBackgroundColor, pBreadcrumb->iSegmentColor, pBreadcrumb->iSelectedColor, pBreadcrumb->iBorderColor, pBreadcrumb->iTextColor, pBreadcrumb->iSeparatorColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyBreadcrumbSegments(xge_xui_page_t* pPage, xge_xui_breadcrumb pBreadcrumb, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	const char* sText;
	int iId;
	char sItemPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "breadcrumb segments must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_BREADCRUMB_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "breadcrumb segment capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiBreadcrumbClear(pBreadcrumb);
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "segments", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sText = "";
		iId = (int)i;
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			sText = (const char*)xvoGetText(pItem);
		} else if ( xvoType(pItem) == XVO_DT_TABLE ) {
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "text"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				sText = (const char*)xvoGetText(pField);
			}
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "id"), sItemPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				iId = (int)__xgeXuiPageValueToFloat(pField, (float)iId);
			}
		} else {
			__xgeXuiPageSetPathError(pPage, sItemPath, "breadcrumb segment must be text or object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		xgeXuiBreadcrumbAddSegment(pBreadcrumb, sText, iId);
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyBreadcrumb(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_breadcrumb pBreadcrumb;
	xvalue pVal;
	xge_font pFont;
	float fPaddingX;
	float fSeparatorWidth;
	char sFieldPath[128];

	if ( pPage->iBreadcrumbCount >= XGE_XUI_PAGE_BREADCRUMB_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "breadcrumb capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBreadcrumb = &pPage->arrBreadcrumb[pPage->iBreadcrumbCount];
	if ( xgeXuiBreadcrumbInit(pBreadcrumb, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "breadcrumb initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iBreadcrumbCount++;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sPath);
	if ( pFont != NULL ) {
		xgeXuiBreadcrumbSetFont(pBreadcrumb, pFont);
	}
	fPaddingX = pBreadcrumb->fSegmentPaddingX;
	fSeparatorWidth = pBreadcrumb->fSeparatorWidth;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.paddingX", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "paddingX", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fPaddingX = __xgeXuiPageValueToFloat(pVal, fPaddingX);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.separatorWidth", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "separatorWidth", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fSeparatorWidth = __xgeXuiPageValueToFloat(pVal, fSeparatorWidth);
	}
	xgeXuiBreadcrumbSetMetrics(pBreadcrumb, fPaddingX, fSeparatorWidth);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.segments", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "segments");
	if ( __xgeXuiPageApplyBreadcrumbSegments(pPage, pBreadcrumb, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selected", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "selected", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiBreadcrumbSetSelected(pBreadcrumb, (int)__xgeXuiPageValueToFloat(pVal, -1.0f));
	}
	if ( __xgeXuiPageApplyBreadcrumbColor(pPage, pBreadcrumb, pNode, pStyle, "backgroundColor", "background", &pBreadcrumb->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyBreadcrumbColor(pPage, pBreadcrumb, pNode, pStyle, "segmentColor", "color", &pBreadcrumb->iSegmentColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyBreadcrumbColor(pPage, pBreadcrumb, pNode, pStyle, "selectedColor", NULL, &pBreadcrumb->iSelectedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyBreadcrumbColor(pPage, pBreadcrumb, pNode, pStyle, "borderColor", NULL, &pBreadcrumb->iBorderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyBreadcrumbColor(pPage, pBreadcrumb, pNode, pStyle, "textColor", NULL, &pBreadcrumb->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyBreadcrumbColor(pPage, pBreadcrumb, pNode, pStyle, "separatorColor", NULL, &pBreadcrumb->iSeparatorColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageApplyAccordionColor(xge_xui_page_t* pPage, xge_xui_accordion pAccordion, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiAccordionSetColors(pAccordion, pAccordion->iBackgroundColor, pAccordion->iHeaderColor, pAccordion->iExpandedColor, pAccordion->iContentColor, pAccordion->iBorderColor, pAccordion->iTextColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageValueToAccordionMode(xge_xui_page_t* pPage, xvalue pVal, int iDefault, const char* sPath)
{
	const char* sText;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return iDefault;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		if ( (sText == NULL) || (sText[0] == 0) || (strcmp(sText, "multiple") == 0) ) {
			return XGE_XUI_ACCORDION_MODE_MULTIPLE;
		}
		if ( strcmp(sText, "single") == 0 ) {
			return XGE_XUI_ACCORDION_MODE_SINGLE;
		}
		__xgeXuiPageSetPathError(pPage, sPath, "accordion mode must be single or multiple");
		return iDefault;
	}
	if ( (xvoType(pVal) == XVO_DT_INT) || (xvoType(pVal) == XVO_DT_FLOAT) ) {
		return ((int)__xgeXuiPageValueToFloat(pVal, (float)iDefault) == XGE_XUI_ACCORDION_MODE_SINGLE) ? XGE_XUI_ACCORDION_MODE_SINGLE : XGE_XUI_ACCORDION_MODE_MULTIPLE;
	}
	__xgeXuiPageSetPathError(pPage, sPath, "accordion mode must be text or number");
	return iDefault;
}

static int __xgeXuiPageApplyAccordionSections(xge_xui_page_t* pPage, xge_xui_accordion pAccordion, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	const char* sTitle;
	const char* sText;
	float fContentHeight;
	int bExpanded;
	int iId;
	char sItemPath[128];
	char sFieldPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "accordion sections must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_ACCORDION_SECTION_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "accordion section capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiAccordionClear(pAccordion);
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "sections", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sTitle = "";
		sText = "";
		fContentHeight = 48.0f;
		bExpanded = 0;
		iId = (int)i;
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			sTitle = (const char*)xvoGetText(pItem);
		} else if ( xvoType(pItem) == XVO_DT_TABLE ) {
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.title", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "title"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				sTitle = (const char*)xvoGetText(pField);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.text", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "text"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				sText = (const char*)xvoGetText(pField);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.contentHeight", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "contentHeight"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				fContentHeight = __xgeXuiPageValueToFloat(pField, fContentHeight);
			}
			pField = __xgeXuiPageTableGet(pItem, "expanded");
			if ( __xgeXuiPageValueExists(pField) ) {
				bExpanded = __xgeXuiPageValueToBool(pField, 0);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.id", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "id"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				iId = (int)__xgeXuiPageValueToFloat(pField, (float)iId);
			}
		} else {
			__xgeXuiPageSetPathError(pPage, sItemPath, "accordion section must be text or object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xgeXuiAccordionAddSection(pAccordion, sTitle, sText, fContentHeight, bExpanded, iId) < 0 ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "accordion section add failed");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyAccordion(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_accordion pAccordion;
	xvalue pVal;
	xge_font pFont;
	float fHeaderHeight;
	float fSpacing;
	float fContentPadding;
	char sFieldPath[128];

	if ( pPage->iAccordionCount >= XGE_XUI_PAGE_ACCORDION_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "accordion capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pAccordion = &pPage->arrAccordion[pPage->iAccordionCount];
	if ( xgeXuiAccordionInit(pAccordion, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "accordion initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iAccordionCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pFont != NULL ) {
		xgeXuiAccordionSetFont(pAccordion, pFont);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.mode", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "mode", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiAccordionSetMode(pAccordion, __xgeXuiPageValueToAccordionMode(pPage, pVal, pAccordion->iMode, sFieldPath));
	if ( pPage->sError[0] != 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fHeaderHeight = pAccordion->fHeaderHeight;
	fSpacing = pAccordion->fSpacing;
	fContentPadding = pAccordion->fContentPadding;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.headerHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "headerHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fHeaderHeight = __xgeXuiPageValueToFloat(pVal, fHeaderHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.spacing", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "spacing", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fSpacing = __xgeXuiPageValueToFloat(pVal, fSpacing);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.contentPadding", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "contentPadding", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fContentPadding = __xgeXuiPageValueToFloat(pVal, fContentPadding);
	}
	xgeXuiAccordionSetMetrics(pAccordion, fHeaderHeight, fSpacing, fContentPadding);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.sections", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "sections");
	if ( __xgeXuiPageApplyAccordionSections(pPage, pAccordion, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.selected", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "selected", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pAccordion->iSelected = (int)__xgeXuiPageValueToFloat(pVal, -1.0f);
	}
	if ( __xgeXuiPageApplyAccordionColor(pPage, pAccordion, pNode, pStyle, "backgroundColor", "background", &pAccordion->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyAccordionColor(pPage, pAccordion, pNode, pStyle, "headerColor", "color", &pAccordion->iHeaderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyAccordionColor(pPage, pAccordion, pNode, pStyle, "expandedColor", NULL, &pAccordion->iExpandedColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyAccordionColor(pPage, pAccordion, pNode, pStyle, "contentColor", NULL, &pAccordion->iContentColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyAccordionColor(pPage, pAccordion, pNode, pStyle, "borderColor", NULL, &pAccordion->iBorderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyAccordionColor(pPage, pAccordion, pNode, pStyle, "textColor", NULL, &pAccordion->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.contentTextColor", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "contentTextColor", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		pAccordion->iContentTextColor = __xgeXuiPageValueToColor(pVal, pAccordion->iContentTextColor);
		xgeXuiWidgetMarkPaint(pAccordion->pWidget);
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onSelect", sPath);
}

static int __xgeXuiPageApplyToastColor(xge_xui_page_t* pPage, xge_xui_toast pToast, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiToastSetColors(pToast, pToast->iBackgroundColor, pToast->iBorderColor, pToast->iTextColor, pToast->iMutedTextColor, pToast->iInfoColor, pToast->iSuccessColor, pToast->iWarningColor, pToast->iErrorColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageValueToToastType(xge_xui_page_t* pPage, xvalue pVal, int iDefault, const char* sPath)
{
	const char* sText;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return iDefault;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		if ( (sText == NULL) || (sText[0] == 0) || (strcmp(sText, "info") == 0) ) {
			return XGE_XUI_TOAST_TYPE_INFO;
		}
		if ( strcmp(sText, "success") == 0 ) {
			return XGE_XUI_TOAST_TYPE_SUCCESS;
		}
		if ( strcmp(sText, "warning") == 0 ) {
			return XGE_XUI_TOAST_TYPE_WARNING;
		}
		if ( strcmp(sText, "error") == 0 ) {
			return XGE_XUI_TOAST_TYPE_ERROR;
		}
		__xgeXuiPageSetPathError(pPage, sPath, "toast type must be info, success, warning or error");
		return iDefault;
	}
	if ( (xvoType(pVal) == XVO_DT_INT) || (xvoType(pVal) == XVO_DT_FLOAT) ) {
		return (int)__xgeXuiPageValueToFloat(pVal, (float)iDefault);
	}
	__xgeXuiPageSetPathError(pPage, sPath, "toast type must be text or number");
	return iDefault;
}

static int __xgeXuiPageValueToToastPlacement(xge_xui_page_t* pPage, xvalue pVal, int iDefault, const char* sPath)
{
	const char* sText;

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return iDefault;
	}
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		if ( (sText == NULL) || (sText[0] == 0) || (strcmp(sText, "topRight") == 0) || (strcmp(sText, "top-right") == 0) ) {
			return XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT;
		}
		if ( (strcmp(sText, "topLeft") == 0) || (strcmp(sText, "top-left") == 0) ) {
			return XGE_XUI_TOAST_PLACEMENT_TOP_LEFT;
		}
		if ( (strcmp(sText, "bottomRight") == 0) || (strcmp(sText, "bottom-right") == 0) ) {
			return XGE_XUI_TOAST_PLACEMENT_BOTTOM_RIGHT;
		}
		if ( (strcmp(sText, "bottomLeft") == 0) || (strcmp(sText, "bottom-left") == 0) ) {
			return XGE_XUI_TOAST_PLACEMENT_BOTTOM_LEFT;
		}
		if ( (strcmp(sText, "topCenter") == 0) || (strcmp(sText, "top-center") == 0) ) {
			return XGE_XUI_TOAST_PLACEMENT_TOP_CENTER;
		}
		if ( (strcmp(sText, "bottomCenter") == 0) || (strcmp(sText, "bottom-center") == 0) ) {
			return XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER;
		}
		__xgeXuiPageSetPathError(pPage, sPath, "toast placement is invalid");
		return iDefault;
	}
	if ( (xvoType(pVal) == XVO_DT_INT) || (xvoType(pVal) == XVO_DT_FLOAT) ) {
		return (int)__xgeXuiPageValueToFloat(pVal, (float)iDefault);
	}
	__xgeXuiPageSetPathError(pPage, sPath, "toast placement must be text or number");
	return iDefault;
}

static int __xgeXuiPageApplyToastItems(xge_xui_page_t* pPage, xge_xui_toast pToast, xvalue pVal, const char* sPath)
{
	xvalue pItem;
	xvalue pField;
	uint32 i;
	uint32 iCount;
	int iType;
	float fDuration;
	const char* sTitle;
	const char* sMessage;
	char sItemPath[128];
	char sFieldPath[128];

	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( xvoType(pVal) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toast items must be an array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pVal);
	if ( iCount > XGE_XUI_TOAST_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toast item capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiToastClear(pToast);
	for ( i = 0; i < iCount; i++ ) {
		snprintf(sItemPath, sizeof(sItemPath), "%.*s[%u]", 100, (sPath != NULL) ? sPath : "items", i);
		sItemPath[sizeof(sItemPath) - 1] = 0;
		pItem = __xgeXuiPageResolveTokenValue(pPage, xvoArrayGetValue(pVal, i), sItemPath);
		if ( (pItem == NULL) && (pPage->sError[0] != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iType = XGE_XUI_TOAST_TYPE_INFO;
		sTitle = "";
		sMessage = "";
		fDuration = 3.0f;
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			sTitle = (const char*)xvoGetText(pItem);
		} else if ( xvoType(pItem) == XVO_DT_TABLE ) {
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.type", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "type"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			iType = __xgeXuiPageValueToToastType(pPage, pField, iType, sFieldPath);
			if ( pPage->sError[0] != 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.title", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "title"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				sTitle = (const char*)xvoGetText(pField);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.message", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "message"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( !__xgeXuiPageValueExists(pField) ) {
				pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "text"), sFieldPath);
				if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
					return XGE_ERROR_INVALID_ARGUMENT;
				}
			}
			if ( xvoType(pField) == XVO_DT_TEXT ) {
				sMessage = (const char*)xvoGetText(pField);
			}
			snprintf(sFieldPath, sizeof(sFieldPath), "%.*s.duration", 100, sItemPath);
			sFieldPath[sizeof(sFieldPath) - 1] = 0;
			pField = __xgeXuiPageResolveTokenValue(pPage, __xgeXuiPageTableGet(pItem, "duration"), sFieldPath);
			if ( (pField == NULL) && (pPage->sError[0] != 0) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( __xgeXuiPageValueExists(pField) ) {
				fDuration = __xgeXuiPageValueToFloat(pField, fDuration);
			}
		} else {
			__xgeXuiPageSetPathError(pPage, sItemPath, "toast item must be text or object");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( xgeXuiToastShow(pToast, iType, sTitle, sMessage, fDuration) < 0 ) {
			__xgeXuiPageSetPathError(pPage, sItemPath, "toast item add failed");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyToast(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, const char* sPath)
{
	xge_xui_toast pToast;
	xvalue pVal;
	xge_font pFont;
	float fToastWidth;
	float fToastHeight;
	float fSpacing;
	char sFieldPath[128];

	if ( pPage->iToastCount >= XGE_XUI_PAGE_TOAST_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toast capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.font", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "font");
	pFont = __xgeXuiPageValueToFont(pPage, pVal, sFieldPath);
	if ( (pFont == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pToast = &pPage->arrToast[pPage->iToastCount];
	if ( xgeXuiToastInit(pToast, pPage->pContext, pWidget, pFont) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "toast initialization failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPage->iToastCount++;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.placement", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "placement", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiToastSetPlacement(pToast, __xgeXuiPageValueToToastPlacement(pPage, pVal, pToast->iPlacement, sFieldPath));
	if ( pPage->sError[0] != 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fToastWidth = pToast->fToastWidth;
	fToastHeight = pToast->fToastHeight;
	fSpacing = pToast->fSpacing;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.toastWidth", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "toastWidth", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fToastWidth = __xgeXuiPageValueToFloat(pVal, fToastWidth);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.toastHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "toastHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fToastHeight = __xgeXuiPageValueToFloat(pVal, fToastHeight);
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.spacing", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "spacing", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fSpacing = __xgeXuiPageValueToFloat(pVal, fSpacing);
	}
	xgeXuiToastSetMetrics(pToast, fToastWidth, fToastHeight, fSpacing);
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "backgroundColor", "background", &pToast->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "borderColor", NULL, &pToast->iBorderColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "textColor", NULL, &pToast->iTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "mutedTextColor", NULL, &pToast->iMutedTextColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "infoColor", NULL, &pToast->iInfoColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "successColor", NULL, &pToast->iSuccessColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "warningColor", NULL, &pToast->iWarningColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "errorColor", NULL, &pToast->iErrorColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "closeColor", NULL, &pToast->iCloseColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyToastColor(pPage, pToast, pNode, pStyle, "closeHoverColor", NULL, &pToast->iCloseHoverColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.items", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "items");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.toasts", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "toasts");
	}
	if ( __xgeXuiPageApplyToastItems(pPage, pToast, pVal, sFieldPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeXuiPageRejectInputDeferredEvent(pPage, pNode, "onClose", sPath);
}

static xge_xui_widget __xgeXuiPageVirtualListCreate(xge_xui_widget pListWidget, int iSlot, void* pUser)
{
	xge_xui_page_virtual_list_adapter_t* pAdapter;
	xge_xui_widget pWidget;
	char sPath[128];

	(void)pListWidget;
	pAdapter = (xge_xui_page_virtual_list_adapter_t*)pUser;
	if ( (pAdapter == NULL) || (pAdapter->pPage == NULL) || (pAdapter->pTemplate == NULL) ) {
		return NULL;
	}
	snprintf(sPath, sizeof(sPath), "%s.itemTemplate.slot[%d]", pAdapter->sPath, iSlot);
	sPath[sizeof(sPath) - 1] = 0;
	pWidget = NULL;
	if ( __xgeXuiPageBuildWidget(pAdapter->pPage, (xvalue)pAdapter->pStyles, (xvalue)pAdapter->pTemplate, sPath, &pWidget) != XGE_OK ) {
		return NULL;
	}
	return pWidget;
}

static void __xgeXuiPageVirtualListBind(xge_xui_widget pItemWidget, int iIndex, void* pUser)
{
	(void)pUser;
	if ( pItemWidget != NULL ) {
		pItemWidget->pInternal = (void*)(intptr_t)(iIndex + 1);
	}
}

static int __xgeXuiPageApplyVirtualListColor(xge_xui_page_t* pPage, xge_xui_virtual_list pList, xvalue pNode, xvalue pStyle, const char* sKey, const char* sFallbackKey, uint32_t* pColor, const char* sPath)
{
	xvalue pVal;
	char sFieldPath[128];

	snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sKey);
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sKey, sFieldPath);
	if ( (!__xgeXuiPageValueExists(pVal)) && (sFallbackKey != NULL) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.%s", (sPath != NULL) ? sPath : "tree", sFallbackKey);
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, sFallbackKey, sFieldPath);
	}
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		*pColor = __xgeXuiPageValueToColor(pVal, *pColor);
		xgeXuiVirtualListSetColors(pList, pList->iBackgroundColor, pList->iBarColor, pList->iThumbColor);
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyVirtualList(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, xvalue pStyles, const char* sPath)
{
	xge_xui_virtual_list pList;
	xge_xui_page_virtual_list_adapter_t* pAdapter;
	xvalue pVal;
	xvalue pTemplate;
	xvalue pChildren;
	const char* sTemplateName;
	char sFieldPath[128];
	float fScrollY;

	if ( pPage->iVirtualListCount >= XGE_XUI_PAGE_VIRTUAL_LIST_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sPath, "virtualList capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pChildren = __xgeXuiPageTableGet(pNode, "children");
	if ( __xgeXuiPageValueExists(pChildren) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "virtualList children are not supported; use itemTemplate");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTemplate = __xgeXuiPageNodeGetStyled(pNode, pStyle, "itemTemplate");
	if ( !__xgeXuiPageValueExists(pTemplate) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "itemTemplate is required");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pTemplate) == XVO_DT_TEXT ) {
		sTemplateName = (const char*)xvoGetText(pTemplate);
		pTemplate = __xgeXuiPageTableGet((xvalue)pPage->pMergedTemplates, sTemplateName);
		if ( !__xgeXuiPageValueExists(pTemplate) ) {
			__xgeXuiPageSetPathError(pPage, sPath, "missing itemTemplate '%s'", (sTemplateName != NULL) ? sTemplateName : "");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( xvoType(pTemplate) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, sPath, "itemTemplate must be object or template name");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pList = &pPage->arrVirtualList[pPage->iVirtualListCount];
	pAdapter = &pPage->arrVirtualListAdapter[pPage->iVirtualListCount];
	if ( xgeXuiVirtualListInit(pList, pPage->pContext, pWidget) != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, sPath, "virtualList initialization failed");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->iVirtualListCount++;
	memset(pAdapter, 0, sizeof(*pAdapter));
	pAdapter->pPage = pPage;
	pAdapter->pStyles = pStyles;
	pAdapter->pTemplate = pTemplate;
	snprintf(pAdapter->sPath, sizeof(pAdapter->sPath), "%s", (sPath != NULL) ? sPath : "tree");
	pAdapter->sPath[sizeof(pAdapter->sPath) - 1] = 0;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemCount", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemCount", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeXuiPageValueExists(pVal) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "itemCount is required");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pAdapter->iItemCount = (int)__xgeXuiPageValueToFloat(pVal, 0.0f);
	if ( pAdapter->iItemCount < 0 ) {
		pAdapter->iItemCount = 0;
	}
	xgeXuiVirtualListSetAdapter(pList, __xgeXuiPageVirtualListCount, __xgeXuiPageVirtualListCreate, __xgeXuiPageVirtualListBind, pAdapter);
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.itemHeight", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "itemHeight", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		xgeXuiVirtualListSetItemHeight(pList, __xgeXuiPageValueToFloat(pVal, pList->fItemHeight));
	}
	fScrollY = pList->fScrollY;
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.scrollY", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "scrollY", sFieldPath);
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fScrollY = __xgeXuiPageValueToFloat(pVal, fScrollY);
	}
	xgeXuiVirtualListSetScroll(pList, fScrollY);
	if ( __xgeXuiPageApplyVirtualListColor(pPage, pList, pNode, pStyle, "backgroundColor", "background", &pList->iBackgroundColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyVirtualListColor(pPage, pList, pNode, pStyle, "barColor", NULL, &pList->iBarColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyVirtualListColor(pPage, pList, pNode, pStyle, "thumbColor", NULL, &pList->iThumbColor, sPath) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageApplyControl(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle, xvalue pStyles, const char* sType, const char* sPath)
{
	if ( (sType == NULL) || (strcmp(sType, "panel") == 0) || (strcmp(sType, "absolute") == 0) || (strcmp(sType, "row") == 0) || (strcmp(sType, "column") == 0) || (strcmp(sType, "stack") == 0) || (strcmp(sType, "grid") == 0) || (strcmp(sType, "dock") == 0) ) {
		return XGE_OK;
	}
	if ( (strcmp(sType, "scroll") == 0) || (strcmp(sType, "scrollView") == 0) ) {
		return __xgeXuiPageApplyScrollView(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "virtualList") == 0 ) {
		return __xgeXuiPageApplyVirtualList(pPage, pWidget, pNode, pStyle, pStyles, sPath);
	}
	if ( strcmp(sType, "treeView") == 0 ) {
		return __xgeXuiPageApplyTreeView(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "tableView") == 0 ) {
		return __xgeXuiPageApplyTableView(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "propertyGrid") == 0 ) {
		return __xgeXuiPageApplyPropertyGrid(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "breadcrumb") == 0 ) {
		return __xgeXuiPageApplyBreadcrumb(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "accordion") == 0 ) {
		return __xgeXuiPageApplyAccordion(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "toast") == 0 ) {
		return __xgeXuiPageApplyToast(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "button") == 0 ) {
		return __xgeXuiPageApplyButton(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "image") == 0 ) {
		return __xgeXuiPageApplyImage(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "input") == 0 ) {
		return __xgeXuiPageApplyInput(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "numericInput") == 0 ) {
		return __xgeXuiPageApplyNumericInput(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "checkbox") == 0 ) {
		return __xgeXuiPageApplyCheckBox(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "radio") == 0 ) {
		return __xgeXuiPageApplyRadio(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "switch") == 0 ) {
		return __xgeXuiPageApplySwitch(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "slider") == 0 ) {
		return __xgeXuiPageApplySlider(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "progress") == 0 ) {
		return __xgeXuiPageApplyProgress(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "tabs") == 0 ) {
		return __xgeXuiPageApplyTabs(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "toolbar") == 0 ) {
		return __xgeXuiPageApplyToolbar(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "statusBar") == 0 ) {
		return __xgeXuiPageApplyStatusBar(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "comboBox") == 0 ) {
		return __xgeXuiPageApplyComboBox(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "popup") == 0 ) {
		return __xgeXuiPageApplyPopup(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "tooltip") == 0 ) {
		return __xgeXuiPageApplyTooltip(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "menu") == 0 ) {
		return __xgeXuiPageApplyMenu(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "dialog") == 0 ) {
		return __xgeXuiPageApplyDialog(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "messageBox") == 0 ) {
		return __xgeXuiPageApplyMessageBox(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "label") == 0 ) {
		return __xgeXuiPageApplyLabel(pPage, pWidget, pNode, pStyle, sPath);
	}
	if ( strcmp(sType, "separator") == 0 ) {
		return __xgeXuiPageApplySeparator(pPage, pWidget, pNode, pStyle, sPath);
	}
	return XGE_ERROR_INVALID_ARGUMENT;
}

static int __xgeXuiPageApplyEvents(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, const char* sType, const char* sPath)
{
	xge_xui_binder_entry_t* pEntry;
	xge_xui_page_click_binding_t* pBinding;
	xvalue pVal;
	const char* sName;
	char sFieldPath[128];

	if ( (pPage == NULL) || (pWidget == NULL) || (pNode == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageTableGet(pNode, "script");
	if ( __xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.script", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "scripts are not supported");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageTableGet(pNode, "onClickScript");
	if ( __xgeXuiPageValueExists(pVal) ) {
		snprintf(sFieldPath, sizeof(sFieldPath), "%s.onClickScript", (sPath != NULL) ? sPath : "tree");
		sFieldPath[sizeof(sFieldPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sFieldPath, "scripts are not supported");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageTableGet(pNode, "onClick");
	if ( !__xgeXuiPageValueExists(pVal) ) {
		return XGE_OK;
	}
	if ( (sType != NULL) && (strcmp(sType, "button") == 0) ) {
		return XGE_OK;
	}
	snprintf(sFieldPath, sizeof(sFieldPath), "%s.onClick", (sPath != NULL) ? sPath : "tree");
	sFieldPath[sizeof(sFieldPath) - 1] = 0;
	if ( xvoType(pVal) != XVO_DT_TEXT ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "expected event name string");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sName = (const char*)xvoGetText(pVal);
	if ( (sName == NULL) || (sName[0] == 0) ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "empty event name");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pWidget->pUser != NULL) || (pWidget->procEvent != NULL) ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "onClick is not supported by this control type");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pEntry = __xgeXuiPageFindClickBinding(pPage->tLoader.pBinder, sName);
	if ( pEntry == NULL ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "unregistered click event '%s'", sName);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pPage->iClickBindingCount >= XGE_XUI_PAGE_CLICK_BINDING_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, sFieldPath, "click binding capacity exceeded");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBinding = &pPage->arrClickBinding[pPage->iClickBindingCount++];
	pBinding->procClick = pEntry->procClick;
	pBinding->pUser = pEntry->pUser;
	pWidget->procEvent = __xgeXuiPageClickEventProc;
	pWidget->pUser = pBinding;
	pWidget->iFlags |= XGE_XUI_WIDGET_FOCUSABLE;
	return XGE_OK;
}

static int __xgeXuiPageApplyStyle(xge_xui_page_t* pPage, xge_xui_widget pWidget, xvalue pNode, xvalue pStyle)
{
	xvalue pVal;
	xvalue pGrid;
	const char* sText;
	xge_xui_size_t tWidth;
	xge_xui_size_t tHeight;
	int iGridColumns;
	float fGridRowHeight;
	float fGridColumnGap;
	float fGridRowGap;
	int iAlignX;
	int iAlignY;

	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "layout");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiWidgetSetLayout(pWidget, __xgeXuiPageTypeToLayout((const char*)xvoGetText(pVal)));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "dock");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiWidgetSetDock(pWidget, __xgeXuiPageTextToDock((const char*)xvoGetText(pVal), pWidget->tStyle.iDock));
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "width", "width");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tWidth = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tWidth);
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "height", "height");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tHeight = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tHeight);
	xgeXuiWidgetSetSize(pWidget, tWidth, tHeight);
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "minWidth", "minWidth");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tWidth = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tMinWidth);
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "minHeight", "minHeight");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tHeight = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tMinHeight);
	xgeXuiWidgetSetMinSize(pWidget, tWidth, tHeight);
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "maxWidth", "maxWidth");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tWidth = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tMaxWidth);
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "maxHeight", "maxHeight");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tHeight = __xgeXuiPageValueToSize(pVal, pWidget->tStyle.tMaxHeight);
	xgeXuiWidgetSetMaxSize(pWidget, tWidth, tHeight);
	if ( __xgeXuiPageApplyEdges(pPage, pWidget, pNode, pStyle, "margin", &pWidget->tStyle.tMargin) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyEdges(pPage, pWidget, pNode, pStyle, "padding", &pWidget->tStyle.tPadding) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageApplyAnchor(pPage, pWidget, pNode, pStyle) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "gap", "gap");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pVal != NULL) && (xvoType(pVal) != XVO_DT_NULL) ) {
		xgeXuiWidgetSetGap(pWidget, __xgeXuiPageValueToFloat(pVal, pWidget->tStyle.fGap));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "align");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		iAlignX = __xgeXuiPageTextToAlign(sText, pWidget->tStyle.iAlignX);
		iAlignY = __xgeXuiPageTextToAlign(sText, pWidget->tStyle.iAlignY);
		xgeXuiWidgetSetAlign(pWidget, iAlignX, iAlignY);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "alignX");
	iAlignX = (xvoType(pVal) == XVO_DT_TEXT) ? __xgeXuiPageTextToAlign((const char*)xvoGetText(pVal), pWidget->tStyle.iAlignX) : pWidget->tStyle.iAlignX;
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "alignY");
	iAlignY = (xvoType(pVal) == XVO_DT_TEXT) ? __xgeXuiPageTextToAlign((const char*)xvoGetText(pVal), pWidget->tStyle.iAlignY) : pWidget->tStyle.iAlignY;
	xgeXuiWidgetSetAlign(pWidget, iAlignX, iAlignY);
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "justify");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiWidgetSetJustify(pWidget, __xgeXuiPageTextToJustify((const char*)xvoGetText(pVal), pWidget->tStyle.iJustify));
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "background", "background");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pVal != NULL) && (xvoType(pVal) != XVO_DT_NULL) ) {
		xgeXuiWidgetSetBackground(pWidget, __xgeXuiPageValueToColor(pVal, pWidget->tStyle.iBackgroundColor));
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "radius", "radius");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pVal != NULL) && (xvoType(pVal) != XVO_DT_NULL) ) {
		xgeXuiWidgetSetRadius(pWidget, __xgeXuiPageValueToFloat(pVal, pWidget->tStyle.fRadius));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "clip");
	if ( (pVal != NULL) && (xvoType(pVal) != XVO_DT_NULL) ) {
		xgeXuiWidgetSetClip(pWidget, __xgeXuiPageValueToBool(pVal, pWidget->tStyle.iClip));
	}
	iGridColumns = pWidget->tStyle.iGridColumns;
	fGridRowHeight = pWidget->tStyle.fGridRowHeight;
	fGridColumnGap = pWidget->tStyle.fGridColumnGap;
	fGridRowGap = pWidget->tStyle.fGridRowGap;
	pGrid = __xgeXuiPageNodeGetStyled(pNode, pStyle, "grid");
	if ( __xgeXuiPageValueExists(pGrid) && (xvoType(pGrid) == XVO_DT_TABLE) ) {
		pVal = __xgeXuiPageTableGet(pGrid, "columns");
		if ( xvoType(pVal) == XVO_DT_INT ) {
			iGridColumns = (int)xvoGetInt(pVal);
		}
		pVal = __xgeXuiPageTableGet(pGrid, "rowHeight");
		if ( __xgeXuiPageValueExists(pVal) ) {
			pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "grid.rowHeight");
			if ( pVal == NULL ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			fGridRowHeight = __xgeXuiPageValueToFloat(pVal, fGridRowHeight);
		}
		pVal = __xgeXuiPageTableGet(pGrid, "columnGap");
		if ( __xgeXuiPageValueExists(pVal) ) {
			pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "grid.columnGap");
			if ( pVal == NULL ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			fGridColumnGap = __xgeXuiPageValueToFloat(pVal, fGridColumnGap);
		}
		pVal = __xgeXuiPageTableGet(pGrid, "rowGap");
		if ( __xgeXuiPageValueExists(pVal) ) {
			pVal = __xgeXuiPageResolveTokenValue(pPage, pVal, "grid.rowGap");
			if ( pVal == NULL ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			fGridRowGap = __xgeXuiPageValueToFloat(pVal, fGridRowGap);
		}
		pVal = __xgeXuiPageTableGet(pGrid, "columnSpan");
		if ( xvoType(pVal) == XVO_DT_INT ) {
			xgeXuiWidgetSetGridColumnSpan(pWidget, (int)xvoGetInt(pVal));
		}
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "columns");
	if ( xvoType(pVal) == XVO_DT_INT ) {
		iGridColumns = (int)xvoGetInt(pVal);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "gridColumns");
	if ( xvoType(pVal) == XVO_DT_INT ) {
		iGridColumns = (int)xvoGetInt(pVal);
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "rowHeight", "rowHeight");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fGridRowHeight = __xgeXuiPageValueToFloat(pVal, fGridRowHeight);
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "columnGap", "columnGap");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fGridColumnGap = __xgeXuiPageValueToFloat(pVal, fGridColumnGap);
	}
	pVal = __xgeXuiPageNodeGetStyledToken(pPage, pNode, pStyle, "rowGap", "rowGap");
	if ( (pVal == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageValueExists(pVal) ) {
		fGridRowGap = __xgeXuiPageValueToFloat(pVal, fGridRowGap);
	}
	if ( (iGridColumns != pWidget->tStyle.iGridColumns) || (fGridRowHeight != pWidget->tStyle.fGridRowHeight) || (fGridColumnGap != pWidget->tStyle.fGridColumnGap) || (fGridRowGap != pWidget->tStyle.fGridRowGap) ) {
		xgeXuiWidgetSetGrid(pWidget, iGridColumns, fGridRowHeight, fGridColumnGap, fGridRowGap);
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "gridColumnSpan");
	if ( xvoType(pVal) == XVO_DT_INT ) {
		xgeXuiWidgetSetGridColumnSpan(pWidget, (int)xvoGetInt(pVal));
	}
	pVal = __xgeXuiPageNodeGetStyled(pNode, pStyle, "columnSpan");
	if ( xvoType(pVal) == XVO_DT_INT ) {
		xgeXuiWidgetSetGridColumnSpan(pWidget, (int)xvoGetInt(pVal));
	}
	return XGE_OK;
}

static int __xgeXuiPageBuildWidget(xge_xui_page_t* pPage, xvalue pStyles, xvalue pNode, const char* sPath, xge_xui_widget* ppWidget)
{
	xvalue pVal;
	xvalue pChildren;
	xvalue pChildNode;
	xvalue pStyle;
	xge_xui_widget pWidget;
	xge_xui_widget pChild;
	const char* sType;
	const char* sText;
	char sChildPath[96];
	char sChildrenPath[96];
	uint32 i;
	uint32 iCount;
	int iRet;

	if ( ppWidget != NULL ) {
		*ppWidget = NULL;
	}
	if ( (pPage == NULL) || (pNode == NULL) || (ppWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pNode) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, sPath, "expected object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		__xgeXuiPageSetPathError(pPage, sPath, "widget allocation failed");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pVal = __xgeXuiPageTableGet(pNode, "type");
	if ( __xgeXuiPageValueExists(pVal) && (xvoType(pVal) != XVO_DT_TEXT) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "type must be string");
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sType = (xvoType(pVal) == XVO_DT_TEXT) ? (const char*)xvoGetText(pVal) : "panel";
	if ( !__xgeXuiPageTypeKnown(sType) ) {
		__xgeXuiPageSetPathError(pPage, sPath, "unknown widget type '%s'", (sType != NULL) ? sType : "");
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetSetLayout(pWidget, __xgeXuiPageTypeToLayout(sType));
	pStyle = __xgeXuiPageNodeStyle(pPage, pStyles, pNode);
	if ( (pStyle == NULL) && (pPage->sError[0] != 0) ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeXuiPageApplyStyle(pPage, pWidget, pNode, pStyle);
	if ( iRet != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return iRet;
	}
	iRet = __xgeXuiPageApplyControl(pPage, pWidget, pNode, pStyle, pStyles, sType, sPath);
	if ( iRet != XGE_OK ) {
		__xgeXuiPageUnitTreeControls(pPage, pWidget);
		xgeXuiWidgetFree(pWidget);
		return iRet;
	}
	iRet = __xgeXuiPageApplyEvents(pPage, pWidget, pNode, sType, sPath);
	if ( iRet != XGE_OK ) {
		__xgeXuiPageUnitTreeControls(pPage, pWidget);
		xgeXuiWidgetFree(pWidget);
		return iRet;
	}
	pWidget->iStyleVersion = pPage->iStyleVersion;
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_STYLE;
	pVal = __xgeXuiPageTableGet(pNode, "id");
	if ( xvoType(pVal) == XVO_DT_INT ) {
		xgeXuiWidgetSetId(pWidget, (int)xvoGetInt(pVal));
	} else if ( xvoType(pVal) == XVO_DT_TEXT ) {
		xgeXuiWidgetSetName(pWidget, (const char*)xvoGetText(pVal));
	}
	pVal = __xgeXuiPageTableGet(pNode, "name");
	if ( xvoType(pVal) == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pVal);
		if ( sText != NULL ) {
			xgeXuiWidgetSetName(pWidget, sText);
		}
	}
	__xgeXuiPageIndexWidget(pPage, pWidget);
	pChildren = __xgeXuiPageTableGet(pNode, "children");
	if ( (pChildren != NULL) && (xvoType(pChildren) != XVO_DT_NULL) ) {
		if ( xvoType(pChildren) != XVO_DT_ARRAY ) {
			__xgeXuiPageUnitTreeControls(pPage, pWidget);
			xgeXuiWidgetFree(pWidget);
			snprintf(sChildrenPath, sizeof(sChildrenPath), "%s.children", sPath);
			sChildrenPath[sizeof(sChildrenPath) - 1] = 0;
			__xgeXuiPageSetPathError(pPage, sChildrenPath, "expected array");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iCount = xvoArrayItemCount(pChildren);
		for ( i = 0; i < iCount; i++ ) {
			pChildNode = xvoArrayGetValue(pChildren, i);
			snprintf(sChildPath, sizeof(sChildPath), "%s.children[%u]", sPath, i);
			sChildPath[sizeof(sChildPath) - 1] = 0;
			iRet = __xgeXuiPageBuildWidget(pPage, pStyles, pChildNode, sChildPath, &pChild);
			if ( iRet != XGE_OK ) {
				__xgeXuiPageUnitTreeControls(pPage, pWidget);
				xgeXuiWidgetFree(pWidget);
				return iRet;
			}
			iRet = xgeXuiWidgetAdd(pWidget, pChild);
			if ( iRet != XGE_OK ) {
				__xgeXuiPageUnitTreeControls(pPage, pChild);
				__xgeXuiPageUnitTreeControls(pPage, pWidget);
				xgeXuiWidgetFree(pChild);
				xgeXuiWidgetFree(pWidget);
				__xgeXuiPageSetPathError(pPage, sChildPath, "add failed (%d)", iRet);
				return iRet;
			}
		}
	}
	*ppWidget = pWidget;
	return XGE_OK;
}

static uint32_t __xgeXuiPageNextStyleVersion(uint32_t iVersion)
{
	iVersion++;
	if ( iVersion == 0 ) {
		iVersion = 1;
	}
	return iVersion;
}

static int __xgeXuiPageRefreshWidgetStyle(xge_xui_page_t* pPage, xvalue pStyles, xvalue pNode, const char* sPath, xge_xui_widget pWidget)
{
	xvalue pStyle;
	xvalue pChildren;
	xvalue pChildNode;
	xvalue pType;
	xge_xui_widget pChild;
	const char* sType;
	char sChildPath[160];
	uint32 iCount;
	uint32 i;
	int iRet;

	if ( (pPage == NULL) || (pNode == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pNode) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, sPath, "expected object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pStyle = __xgeXuiPageNodeStyle(pPage, pStyles, pNode);
	if ( (pStyle == NULL) && (pPage->sError[0] != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeXuiPageApplyStyle(pPage, pWidget, pNode, pStyle);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pWidget->iStyleVersion = pPage->iStyleVersion;
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_STYLE;
	pType = __xgeXuiPageTableGet(pNode, "type");
	sType = (xvoType(pType) == XVO_DT_TEXT) ? (const char*)xvoGetText(pType) : "panel";
	if ( (sType != NULL) && ((strcmp(sType, "virtualList") == 0) || (strcmp(sType, "treeView") == 0)) ) {
		return XGE_OK;
	}
	pChildren = __xgeXuiPageTableGet(pNode, "children");
	if ( !__xgeXuiPageValueExists(pChildren) ) {
		return XGE_OK;
	}
	if ( xvoType(pChildren) != XVO_DT_ARRAY ) {
		snprintf(sChildPath, sizeof(sChildPath), "%s.children", (sPath != NULL) ? sPath : "tree");
		sChildPath[sizeof(sChildPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sChildPath, "expected array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = xvoArrayItemCount(pChildren);
	pChild = pWidget->pFirstChild;
	for ( i = 0; i < iCount; i++ ) {
		if ( pChild == NULL ) {
			snprintf(sChildPath, sizeof(sChildPath), "%s.children[%u]", (sPath != NULL) ? sPath : "tree", i);
			sChildPath[sizeof(sChildPath) - 1] = 0;
			__xgeXuiPageSetPathError(pPage, sChildPath, "widget tree is shorter than XSON tree");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pChildNode = xvoArrayGetValue(pChildren, i);
		snprintf(sChildPath, sizeof(sChildPath), "%s.children[%u]", (sPath != NULL) ? sPath : "tree", i);
		sChildPath[sizeof(sChildPath) - 1] = 0;
		iRet = __xgeXuiPageRefreshWidgetStyle(pPage, pStyles, pChildNode, sChildPath, pChild);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pChild = pChild->pNextSibling;
	}
	if ( pChild != NULL ) {
		snprintf(sChildPath, sizeof(sChildPath), "%s.children", (sPath != NULL) ? sPath : "tree");
		sChildPath[sizeof(sChildPath) - 1] = 0;
		__xgeXuiPageSetPathError(pPage, sChildPath, "widget tree is longer than XSON tree");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeXuiPageLoadImportDocument(xge_xui_page_t* pPage, const char* sURI, const char* const* arrStack, int iStackCount)
{
	xge_resource_t* pResource;
	xvalue pDoc;
	int iSlot;
	int iRet;

	if ( (pPage == NULL) || (sURI == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiPageImportSeen(pPage, sURI) ) {
		return XGE_OK;
	}
	if ( __xgeXuiPageImportInStack(sURI, arrStack, iStackCount) ) {
		__xgeXuiPageSetPathError(pPage, "imports", "import cycle at %s", sURI);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pPage->iImportCount >= XGE_XUI_PAGE_IMPORT_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, "imports", "too many imports");
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSlot = pPage->iImportCount;
	pResource = &pPage->arrImportResource[iSlot];
	memset(pResource, 0, sizeof(*pResource));
	iRet = xgeResourceLoad(sURI, pResource);
	if ( iRet != XGE_OK ) {
		__xgeXuiPageSetPathError(pPage, "imports", "resource load failed: %s (%d)", sURI, iRet);
		return iRet;
	}
	pDoc = xrtParseXSON((str)(void*)pResource->pData, (size_t)pResource->iSize);
	if ( (pDoc == NULL) || (xvoType(pDoc) == XVO_DT_NULL) ) {
		xvoUnref(pDoc);
		xgeResourceFree(pResource);
		__xgeXuiPageSetPathError(pPage, "imports", "XSON parse failed: %s", sURI);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xvoType(pDoc) != XVO_DT_TABLE ) {
		xvoUnref(pDoc);
		xgeResourceFree(pResource);
		__xgeXuiPageSetPathError(pPage, "imports", "root must be an object: %s", sURI);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(pPage->arrImportURI[iSlot], sizeof(pPage->arrImportURI[iSlot]), "%s", sURI);
	pPage->arrImportURI[iSlot][sizeof(pPage->arrImportURI[iSlot]) - 1] = 0;
	pPage->arrImportDocument[iSlot] = pDoc;
	pPage->iImportCount++;
	return XGE_OK;
}

static int __xgeXuiPageLoadImports(xge_xui_page_t* pPage, xvalue pDoc, const char* sURI, const char* const* arrStack, int iStackCount)
{
	const char* arrNextStack[XGE_XUI_PAGE_IMPORT_CAPACITY + 1];
	xvalue pImports;
	xvalue pImportVal;
	xvalue pImportDoc;
	const char* sImport;
	char sResolved[XGE_XUI_PAGE_IMPORT_URI_CAPACITY];
	char sImportPath[64];
	uint32 i;
	uint32 iCount;
	int iRet;

	pImports = __xgeXuiPageTableGet(pDoc, "imports");
	if ( (pImports == NULL) || (xvoType(pImports) == XVO_DT_NULL) ) {
		return XGE_OK;
	}
	if ( xvoType(pImports) != XVO_DT_ARRAY ) {
		__xgeXuiPageSetPathError(pPage, "imports", "expected array");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iStackCount >= XGE_XUI_PAGE_IMPORT_CAPACITY ) {
		__xgeXuiPageSetPathError(pPage, "imports", "import chain is too deep");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < (uint32)iStackCount; i++ ) {
		arrNextStack[i] = arrStack[i];
	}
	arrNextStack[iStackCount] = (sURI != NULL) ? sURI : "<memory>";
	iCount = xvoArrayItemCount(pImports);
	for ( i = 0; i < iCount; i++ ) {
		pImportVal = xvoArrayGetValue(pImports, i);
		if ( xvoType(pImportVal) != XVO_DT_TEXT ) {
			snprintf(sImportPath, sizeof(sImportPath), "imports[%u]", i);
			sImportPath[sizeof(sImportPath) - 1] = 0;
			__xgeXuiPageSetPathError(pPage, sImportPath, "expected string");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		sImport = (const char*)xvoGetText(pImportVal);
		__xgeXuiPageResolveImportURI(sURI, sImport, sResolved, (int)sizeof(sResolved));
		if ( __xgeXuiPageImportSeen(pPage, sResolved) ) {
			continue;
		}
		iRet = __xgeXuiPageLoadImportDocument(pPage, sResolved, arrNextStack, iStackCount + 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pImportDoc = (xvalue)pPage->arrImportDocument[pPage->iImportCount - 1];
		iRet = __xgeXuiPageLoadImports(pPage, pImportDoc, sResolved, arrNextStack, iStackCount + 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeXuiPageMergeSection(pPage, (xvalue*)&pPage->pMergedStyles, pImportDoc, "styles");
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeXuiPageMergeSection(pPage, (xvalue*)&pPage->pMergedTokens, pImportDoc, "tokens");
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeXuiPageMergeSection(pPage, (xvalue*)&pPage->pMergedTemplates, pImportDoc, "templates");
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeXuiPageLoadDocument(xge_xui_context pContext, const void* pData, int iSize, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage, const char* sURI)
{
	xvalue pDoc;
	xvalue pTree;
	xvalue pStyles;
	xge_xui_widget pRoot;
	int iRet;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pData == NULL) || (iSize < 0) || (pPage == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiPageUnload(pPage);
	memset(pPage, 0, sizeof(*pPage));
	pPage->pContext = pContext;
	pPage->tLoader.sURI = sURI;
	pPage->tLoader.pBinder = pBinder;
	pDoc = xrtParseXSON((str)(void*)pData, (size_t)iSize);
	if ( (pDoc == NULL) || (((xvalue)pDoc)->Type == XVO_DT_NULL) ) {
		xvoUnref(pDoc);
		__xgeXuiPageSetPathError(pPage, "<root>", "XSON parse failed");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->pDocument = pDoc;
	pPage->tLoader.pDocument = pDoc;
	if ( xvoType(pDoc) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, "<root>", "root must be an object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTree = __xgeXuiPageTableGet(pDoc, "tree");
	if ( (pTree == NULL) || (xvoType(pTree) == XVO_DT_NULL) ) {
		__xgeXuiPageSetPathError(pPage, "tree", "missing tree");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pStyles = __xgeXuiPageTableGet(pDoc, "styles");
	if ( (pStyles != NULL) && (xvoType(pStyles) == XVO_DT_NULL) ) {
		pStyles = NULL;
	}
	if ( (pStyles != NULL) && (xvoType(pStyles) != XVO_DT_TABLE) ) {
		__xgeXuiPageSetPathError(pPage, "styles", "expected object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeXuiPageLoadImports(pPage, pDoc, sURI, NULL, 0);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeXuiPageMergeSection(pPage, (xvalue*)&pPage->pMergedStyles, pDoc, "styles");
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeXuiPageMergeSection(pPage, (xvalue*)&pPage->pMergedTokens, pDoc, "tokens");
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeXuiPageMergeSection(pPage, (xvalue*)&pPage->pMergedTemplates, pDoc, "templates");
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeXuiPageApplyDocumentSafeArea(pPage, pDoc);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pStyles = (xvalue)pPage->pMergedStyles;
	pPage->iStyleVersion = 1;
	pPage->iThemeVersion = xgeXuiGetThemeVersion(pContext);
	iRet = __xgeXuiPageBuildWidget(pPage, pStyles, pTree, "tree", &pRoot);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeXuiWidgetAdd(xgeXuiRoot(pContext), pRoot);
	if ( iRet != XGE_OK ) {
		xgeXuiWidgetFree(pRoot);
		__xgeXuiPageSetPathError(pPage, "tree", "attach root failed (%d)", iRet);
		return iRet;
	}
	pPage->pRoot = pRoot;
	pPage->sError[0] = 0;
	return XGE_OK;
}

static xge_xui_widget __xgeXuiPageFindWidget(xge_xui_widget pWidget, const char* sId, int iId, int bUseNumeric)
{
	xge_xui_widget pChild;
	xge_xui_widget pFound;

	if ( (pWidget == NULL) || (sId == NULL) ) {
		return NULL;
	}
	if ( ((pWidget->sName != NULL) && (strcmp(pWidget->sName, sId) == 0)) || (bUseNumeric && (pWidget->iId == iId)) ) {
		return pWidget;
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		pFound = __xgeXuiPageFindWidget(pChild, sId, iId, bUseNumeric);
		if ( pFound != NULL ) {
			return pFound;
		}
	}
	return NULL;
}

static void __xgeXuiPageRollbackFailedLoad(xge_xui_page_t* pPage, xge_xui_context pContext)
{
	char sError[XGE_XUI_PAGE_ERROR_CAPACITY];

	if ( pPage == NULL ) {
		return;
	}
	snprintf(sError, sizeof(sError), "%s", pPage->sError);
	sError[sizeof(sError) - 1] = 0;
	xgeXuiPageUnload(pPage);
	pPage->pContext = pContext;
	snprintf(pPage->sError, sizeof(pPage->sError), "%s", sError);
	pPage->sError[sizeof(pPage->sError) - 1] = 0;
}

int xgeXuiPageLoad(xge_xui_context pContext, const char* sURI, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (sURI == NULL) || (pPage == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tResource, 0, sizeof(tResource));
	iRet = xgeResourceLoad(sURI, &tResource);
	if ( iRet != XGE_OK ) {
		xgeXuiPageUnload(pPage);
		memset(pPage, 0, sizeof(*pPage));
		pPage->pContext = pContext;
		__xgeXuiPageSetError(pPage, "%s: resource load failed (%d)", sURI, iRet);
		return iRet;
	}
	iRet = __xgeXuiPageLoadDocument(pContext, tResource.pData, tResource.iSize, pBinder, pPage, sURI);
	pPage->tLoader.tResource = tResource;
	if ( iRet != XGE_OK ) {
		__xgeXuiPageRollbackFailedLoad(pPage, pContext);
	}
	return iRet;
}

int xgeXuiPageLoadMemory(xge_xui_context pContext, const void* pData, int iSize, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pData == NULL) || (iSize < 0) || (pPage == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tResource, 0, sizeof(tResource));
	iRet = xgeResourceLoadMemory(pData, iSize, &tResource);
	if ( iRet != XGE_OK ) {
		xgeXuiPageUnload(pPage);
		memset(pPage, 0, sizeof(*pPage));
		pPage->pContext = pContext;
		__xgeXuiPageSetError(pPage, "<memory>: resource copy failed (%d)", iRet);
		return iRet;
	}
	iRet = __xgeXuiPageLoadDocument(pContext, tResource.pData, tResource.iSize, pBinder, pPage, NULL);
	pPage->tLoader.tResource = tResource;
	if ( iRet != XGE_OK ) {
		__xgeXuiPageRollbackFailedLoad(pPage, pContext);
	}
	return iRet;
}

void xgeXuiPageUnload(xge_xui_page_t* pPage)
{
	int i;

	if ( pPage == NULL ) {
		return;
	}
	if ( pPage->pRoot != NULL ) {
		__xgeXuiPageUnitTreeControls(pPage, pPage->pRoot);
		xgeXuiWidgetRemove(pPage->pRoot);
		xgeXuiWidgetFree(pPage->pRoot);
	}
	if ( pPage->bSafeAreaApplied && (pPage->pContext != NULL) && (pPage->pContext->pRoot != NULL) ) {
		pPage->pContext->pRoot->tStyle.tPadding = pPage->tSafeAreaPrev;
		xgeXuiWidgetMarkLayout(pPage->pContext->pRoot);
		xgeXuiWidgetMarkPaint(pPage->pContext->pRoot);
	}
	if ( pPage->pMergedStyles != NULL ) {
		xvoUnref((xvalue)pPage->pMergedStyles);
	}
	if ( pPage->pMergedTokens != NULL ) {
		xvoUnref((xvalue)pPage->pMergedTokens);
	}
	if ( pPage->pMergedTemplates != NULL ) {
		xvoUnref((xvalue)pPage->pMergedTemplates);
	}
	for ( i = 0; i < pPage->iImportCount; i++ ) {
		if ( pPage->arrImportDocument[i] != NULL ) {
			xvoUnref((xvalue)pPage->arrImportDocument[i]);
		}
		xgeResourceFree(&pPage->arrImportResource[i]);
	}
	if ( pPage->pDocument != NULL ) {
		xvoUnref((xvalue)pPage->pDocument);
	}
	xgeResourceFree(&pPage->tLoader.tResource);
	memset(pPage, 0, sizeof(*pPage));
}

xge_xui_widget xgeXuiPageRoot(xge_xui_page_t* pPage)
{
	if ( pPage == NULL ) {
		return NULL;
	}
	return pPage->pRoot;
}

xge_xui_widget xgeXuiPageFind(xge_xui_page_t* pPage, const char* sId)
{
	int iId;
	int bUseNumeric;
	xge_xui_widget pFound;

	if ( (pPage == NULL) || (sId == NULL) || (pPage->pRoot == NULL) ) {
		return NULL;
	}
	iId = 0;
	bUseNumeric = __xgeXuiPageParseNumericId(sId, &iId);
	if ( pPage->iIndexCount > 0 ) {
		pFound = __xgeXuiPageIndexFind(pPage, sId, iId, bUseNumeric);
		if ( (pFound != NULL) || !pPage->bIndexOverflow ) {
			return pFound;
		}
	}
	return __xgeXuiPageFindWidget(pPage->pRoot, sId, (int)iId, bUseNumeric);
}

uint32_t xgeXuiPageStyleVersion(xge_xui_page_t* pPage)
{
	if ( pPage == NULL ) {
		return 0;
	}
	return pPage->iStyleVersion;
}

int xgeXuiPageRefreshStyle(xge_xui_page_t* pPage)
{
	xvalue pDoc;
	xvalue pTree;
	int iRet;

	if ( (pPage == NULL) || (pPage->pContext == NULL) || (pPage->pRoot == NULL) || (pPage->pDocument == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pDoc = (xvalue)pPage->pDocument;
	if ( xvoType(pDoc) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, "xui", "document must be object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTree = __xgeXuiPageTableGet(pDoc, "tree");
	if ( xvoType(pTree) != XVO_DT_TABLE ) {
		__xgeXuiPageSetPathError(pPage, "tree", "expected object");
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPage->sError[0] = 0;
	pPage->iStyleVersion = __xgeXuiPageNextStyleVersion(pPage->iStyleVersion);
	iRet = __xgeXuiPageRefreshWidgetStyle(pPage, (xvalue)pPage->pMergedStyles, pTree, "tree", pPage->pRoot);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pPage->iThemeVersion = xgeXuiGetThemeVersion(pPage->pContext);
	return XGE_OK;
}

int xgeXuiPageSyncStyle(xge_xui_page_t* pPage)
{
	uint32_t iThemeVersion;

	if ( (pPage == NULL) || (pPage->pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iThemeVersion = xgeXuiGetThemeVersion(pPage->pContext);
	if ( pPage->iThemeVersion == iThemeVersion ) {
		return XGE_OK;
	}
	return xgeXuiPageRefreshStyle(pPage);
}

static uint32_t __xgeXuiModelNextVersion(uint32_t iVersion)
{
	iVersion++;
	if ( iVersion == 0 ) {
		iVersion = 1;
	}
	return iVersion;
}

static int __xgeXuiModelFindEntry(const xge_xui_model_t* pModel, const char* sKey)
{
	int i;

	if ( (pModel == NULL) || (sKey == NULL) || (sKey[0] == 0) ) {
		return -1;
	}
	for ( i = 0; i < pModel->iEntryCount; i++ ) {
		if ( strcmp(pModel->arrEntry[i].sKey, sKey) == 0 ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiPageApplyModel(xge_xui_page_t* pPage, const xge_xui_model_t* pModel)
{
	xge_xui_page_model_binding_t* pBinding;
	const char* sValue;
	xge_xui_image pImage;
	int i;
	int iRet;

	if ( (pPage == NULL) || (pModel == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pPage->iModelVersion == pModel->iVersion ) {
		return XGE_OK;
	}
	pPage->sError[0] = 0;
	for ( i = 0; i < pPage->iModelBindingCount; i++ ) {
		pBinding = &pPage->arrModelBinding[i];
		sValue = xgeXuiModelGetText(pModel, pBinding->sKey);
		if ( sValue == NULL ) {
			__xgeXuiPageSetPathError(pPage, pBinding->sKey, "model key not found");
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( pBinding->iKind == XGE_XUI_PAGE_MODEL_BIND_LABEL_TEXT ) {
			xgeXuiLabelSetText((xge_xui_label)pBinding->pControl, sValue);
		} else if ( pBinding->iKind == XGE_XUI_PAGE_MODEL_BIND_INPUT_VALUE ) {
			xgeXuiInputSetText((xge_xui_input)pBinding->pControl, sValue);
		} else if ( pBinding->iKind == XGE_XUI_PAGE_MODEL_BIND_IMAGE_SRC ) {
			if ( (pBinding->iControlIndex < 0) || (pBinding->iControlIndex >= pPage->iImageCount) ) {
				__xgeXuiPageSetPathError(pPage, pBinding->sKey, "image binding target is invalid");
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pImage = (xge_xui_image)pBinding->pControl;
			xgeXuiImageSetTexture(pImage, NULL);
			if ( pPage->arrImageTextureOwned[pBinding->iControlIndex] ) {
				xgeTextureFree(&pPage->arrImageTexture[pBinding->iControlIndex]);
				pPage->arrImageTextureOwned[pBinding->iControlIndex] = 0;
			}
			if ( sValue[0] != 0 ) {
				iRet = xgeTextureLoad(&pPage->arrImageTexture[pBinding->iControlIndex], sValue);
				if ( iRet != XGE_OK ) {
					__xgeXuiPageSetPathError(pPage, pBinding->sKey, "texture load failed: %s (%d)", sValue, iRet);
					return iRet;
				}
				pPage->arrImageTextureOwned[pBinding->iControlIndex] = 1;
				xgeXuiImageSetTexture(pImage, &pPage->arrImageTexture[pBinding->iControlIndex]);
			}
		}
	}
	pPage->iModelVersion = pModel->iVersion;
	return XGE_OK;
}

const char* xgeXuiPageGetError(xge_xui_page_t* pPage)
{
	if ( pPage == NULL ) {
		return "";
	}
	return pPage->sError;
}

void xgeXuiBinderInit(xge_xui_binder_t* pBinder)
{
	if ( pBinder == NULL ) {
		return;
	}
	memset(pBinder, 0, sizeof(*pBinder));
}

int xgeXuiBinderSetClick(xge_xui_binder_t* pBinder, const char* sName, xge_xui_click_proc procClick, void* pUser)
{
	int i;

	if ( (pBinder == NULL) || (sName == NULL) || (procClick == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pBinder->iClickCount; i++ ) {
		if ( (pBinder->arrClick[i].sName != NULL) && (strcmp(pBinder->arrClick[i].sName, sName) == 0) ) {
			pBinder->arrClick[i].procClick = procClick;
			pBinder->arrClick[i].pUser = pUser;
			return XGE_OK;
		}
	}
	if ( pBinder->iClickCount >= XGE_XUI_BINDER_ENTRY_CAPACITY ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBinder->arrClick[pBinder->iClickCount].sName = sName;
	pBinder->arrClick[pBinder->iClickCount].procClick = procClick;
	pBinder->arrClick[pBinder->iClickCount].pUser = pUser;
	pBinder->iClickCount++;
	return XGE_OK;
}

void xgeXuiModelInit(xge_xui_model_t* pModel)
{
	if ( pModel == NULL ) {
		return;
	}
	memset(pModel, 0, sizeof(*pModel));
	pModel->iVersion = 1;
}

int xgeXuiModelSetText(xge_xui_model_t* pModel, const char* sKey, const char* sValue)
{
	int iIndex;
	int bNewEntry;

	if ( (pModel == NULL) || (sKey == NULL) || (sKey[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sValue == NULL ) {
		sValue = "";
	}
	if ( (strlen(sKey) >= XGE_XUI_MODEL_KEY_CAPACITY) || (strlen(sValue) >= XGE_XUI_MODEL_VALUE_CAPACITY) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeXuiModelFindEntry(pModel, sKey);
	bNewEntry = 0;
	if ( iIndex < 0 ) {
		if ( pModel->iEntryCount >= XGE_XUI_MODEL_ENTRY_CAPACITY ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iIndex = pModel->iEntryCount++;
		bNewEntry = 1;
		memset(&pModel->arrEntry[iIndex], 0, sizeof(pModel->arrEntry[iIndex]));
		snprintf(pModel->arrEntry[iIndex].sKey, sizeof(pModel->arrEntry[iIndex].sKey), "%s", sKey);
	}
	if ( !bNewEntry && (strcmp(pModel->arrEntry[iIndex].sValue, sValue) == 0) ) {
		return XGE_OK;
	}
	snprintf(pModel->arrEntry[iIndex].sValue, sizeof(pModel->arrEntry[iIndex].sValue), "%s", sValue);
	pModel->iVersion = __xgeXuiModelNextVersion(pModel->iVersion);
	return XGE_OK;
}

int xgeXuiModelSetInt(xge_xui_model_t* pModel, const char* sKey, int iValue)
{
	char sValue[32];

	snprintf(sValue, sizeof(sValue), "%d", iValue);
	return xgeXuiModelSetText(pModel, sKey, sValue);
}

int xgeXuiModelSetFloat(xge_xui_model_t* pModel, const char* sKey, float fValue)
{
	char sValue[64];

	snprintf(sValue, sizeof(sValue), "%g", fValue);
	return xgeXuiModelSetText(pModel, sKey, sValue);
}

const char* xgeXuiModelGetText(const xge_xui_model_t* pModel, const char* sKey)
{
	int iIndex;

	iIndex = __xgeXuiModelFindEntry(pModel, sKey);
	if ( iIndex < 0 ) {
		return NULL;
	}
	return pModel->arrEntry[iIndex].sValue;
}

uint32_t xgeXuiModelVersion(const xge_xui_model_t* pModel)
{
	if ( pModel == NULL ) {
		return 0;
	}
	return pModel->iVersion;
}
