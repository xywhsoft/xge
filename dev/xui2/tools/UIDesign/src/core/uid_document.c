#include "uid_document.h"
#include <stdio.h>
#include <string.h>

static void UIDesignCopyText(char* sDst, int iDstSize, const char* sSrc)
{
	size_t iLen;

	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	iLen = strlen(sSrc);
	if ( iLen >= (size_t)iDstSize ) {
		iLen = (size_t)iDstSize - 1u;
	}
	memmove(sDst, sSrc, iLen);
	sDst[iLen] = 0;
}

void UIDesignDocumentInit(uid_document_t* pDocument)
{
	if ( pDocument == NULL ) {
		return;
	}
	memset(pDocument, 0, sizeof(*pDocument));
	strcpy(pDocument->sName, "Untitled");
	pDocument->iNextNodeId = 1;
	pDocument->iSelectedNode = -1;
}

void UIDesignDocumentUnit(uid_document_t* pDocument)
{
	(void)pDocument;
}

void UIDesignDocumentMarkDirty(uid_document_t* pDocument)
{
	if ( pDocument == NULL ) {
		return;
	}
	pDocument->iRevision++;
	pDocument->bDirty = (pDocument->iRevision != pDocument->iCleanRevision);
}

xge_rect_t UIDesignDocumentDefaultRect(const char* sType)
{
	xge_rect_t tRect;

	tRect = (xge_rect_t){ 0.0f, 0.0f, 140.0f, 26.0f };
	if ( sType == NULL ) {
		return tRect;
	}
	if ( strcmp(sType, "button") == 0 ) {
		tRect.fW = 96.0f;
		tRect.fH = 30.0f;
	} else if ( strcmp(sType, "input") == 0 ) {
		tRect.fW = 180.0f;
		tRect.fH = 30.0f;
	} else if ( strcmp(sType, "panel") == 0 ) {
		tRect.fW = 220.0f;
		tRect.fH = 120.0f;
	}
	return tRect;
}

uid_node_t* UIDesignDocumentAddNode(uid_document_t* pDocument, const char* sType)
{
	uid_node_t* pNode;
	int iIndex;
	int iId;
	float fOffset;

	if ( (pDocument == NULL) || (sType == NULL) || (pDocument->iNodeCount >= UID_DOCUMENT_NODE_CAPACITY) ) {
		return NULL;
	}
	iIndex = pDocument->iNodeCount++;
	iId = pDocument->iNextNodeId++;
	pNode = &pDocument->arrNodes[iIndex];
	memset(pNode, 0, sizeof(*pNode));
	pNode->iId = iId;
	UIDesignCopyText(pNode->sType, sizeof(pNode->sType), sType);
	snprintf(pNode->sName, sizeof(pNode->sName), "%s%d", sType, iId);
	pNode->tRect = UIDesignDocumentDefaultRect(sType);
	if ( strcmp(sType, "button") == 0 ) {
		UIDesignCopyText(pNode->sText, sizeof(pNode->sText), "Button");
	} else if ( strcmp(sType, "input") == 0 ) {
		UIDesignCopyText(pNode->sText, sizeof(pNode->sText), "Input");
	} else if ( strcmp(sType, "panel") == 0 ) {
		UIDesignCopyText(pNode->sText, sizeof(pNode->sText), "Panel");
	} else {
		UIDesignCopyText(pNode->sText, sizeof(pNode->sText), "Label");
	}
	fOffset = (float)((iIndex % 8) * 18);
	pNode->tRect.fX = 34.0f + fOffset;
	pNode->tRect.fY = 46.0f + fOffset;
	pDocument->iSelectedNode = iIndex;
	UIDesignDocumentMarkDirty(pDocument);
	return pNode;
}

uid_node_t* UIDesignDocumentGetSelected(uid_document_t* pDocument)
{
	if ( (pDocument == NULL) || (pDocument->iSelectedNode < 0) || (pDocument->iSelectedNode >= pDocument->iNodeCount) ) {
		return NULL;
	}
	return &pDocument->arrNodes[pDocument->iSelectedNode];
}
