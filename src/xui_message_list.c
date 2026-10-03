#include "../xui_config.h"
#if XUI_ENABLE_MESSAGE_LIST
#include "xui_internal.h"
#include "xui_text_internal.h"
#include "xui_document_layout_internal.h"
#include "xui_document_accessible_internal.h"
#include "../xui_document_ui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>

#define XUI_MESSAGE_LIST_DEFAULT_WIDTH 360.0f
#define XUI_MESSAGE_LIST_DEFAULT_HEIGHT 160.0f

#ifndef XUI_MESSAGE_LIST_AUDIT_STEP
#define XUI_MESSAGE_LIST_AUDIT_STEP(name) ((void)0)
#endif

typedef struct xui_message_text_caret_t {
	int iOffset;
	float fX;
} xui_message_text_caret_t;

typedef struct xui_message_accessible_document_node_t {
	xui_doc_node_id iDocumentNodeId;
	uint64_t iAccessibleId;
	uint64_t iParentAccessibleId;
	char* sValue;
} xui_message_accessible_document_node_t;

typedef struct xui_message_document_binding_t {
	xui_widget pList;
	xui_document pDocument;
	xui_document_renderer pRenderer;
	uint64_t iSubscription;
	int iIndex;
	int bNeedsSync;
	xui_doc_range_t tSelection;
	xui_doc_node_id iPressedNode;
	xui_message_accessible_document_node_t* arrAccessibleNodes;
	uint64_t iAccessibleNodeCount;
	uint64_t iAccessibleIdentity;
	uint64_t iAccessibleRevision;
	xui_message_document_desc_t tDesc;
	xui_font pResolvedFont;
	uint32_t iResolvedTextColor;
} xui_message_document_binding_t;

typedef struct xui_message_node_data_t {
	uint64_t iAccessibleId;
	uint64_t iAccessibleDocumentId;
	uint64_t iAccessibleDocumentRevision;
	char* sAccessibleDocumentText;
	char* sId;
	char* sSender;
	char* sTime;
	char* sText;
	char* sParentId;
	char* sTitle;
	xui_message_document_binding_t* pDocumentBinding;
	int bDocumentSizeExact;
	int iType;
	int iFlags;
	int iAuxiliaryKind;
	void* pUser;
	xui_rect_t tNodeRect;
	xui_rect_t tBubbleRect;
	xui_rect_t tHeaderRect;
	xui_rect_t tTextRect;
	xui_text_layout pTextLayout;
	xui_font pTextLayoutFont;
	const char* sTextLayoutSource;
	float fTextLayoutWidth;
	uint32_t iTextDpiGeneration;
	xui_message_text_caret_t* arrTextCarets;
	int iTextCaretCount;
	int iTextCaretCapacity;
	int iTextCaretLine;
	xui_text_layout pTitleLayout;
	xui_font pTitleLayoutFont;
	const char* sTitleLayoutSource;
	float fTitleLayoutWidth;
	uint32_t iTitleLanguageRevision;
	uint32_t iTitleDpiGeneration;
	xui_vec2_t tMeasuredText;
	xui_vec2_t tMeasuredTitle;
	float fMetaWidth;
	float fNextY;
	int iSelectablePrefix;
	int bMeasureDirty;
} xui_message_node_data_t;

typedef struct xui_message_document_anchor_t {
	xui_message_document_binding_t* pBinding;
	xui_doc_position_t tPosition;
	uint8_t* arrMeasuredPrefix;
	size_t iPrefixCount;
	size_t iOldBlockCount;
	float fScreenY;
	float fLineFraction;
	int iNode;
	int bPrefixApplied;
} xui_message_document_anchor_t;

typedef struct xui_message_list_data_t {
	xui_message_node_data_t* arrNodes;
	uint64_t iNextAccessibleId;
	int iNodeCount;
	int iNodeCapacity;
	int iDocumentNodeCount;
	xui_font pFont;
	int bUseDefaultFont;
	xui_message_list_metrics_t tMetrics;
	xui_message_list_colors_t tColors;
	xui_message_list_event_proc onEvent;
	void* pEventUser;
	xui_message_list_node_renderer_proc onRenderNode;
	void* pRenderNodeUser;
	float fScrollY;
	float fContentHeight;
	int iHover;
	int iSelected;
	int iSelectCount;
	int iClickCount;
	int iChangeCount;
	int bAutoScroll;
	int iSelectionAnchorNode;
	int iSelectionAnchorOffset;
	int iSelectionActiveNode;
	int iSelectionActiveOffset;
	xui_doc_position_t tSelectionAnchorDocument;
	xui_doc_position_t tSelectionActiveDocument;
	xui_doc_table_selection_t tTableSelection;
	xui_doc_cell_hit_t tTableDragAnchor;
	xui_doc_cell_hit_t tTableDragFocus;
	int iTableSelectionNode;
	int bTableSelecting;
	xui_doc_node_id iPressedTaskNode;
	int iPressedTaskMessage;
	int bSelecting;
	int iDocumentSelectionNode;
	int bDocumentSelecting;
	xui_widget pContextMenu;
	int bLayoutValid;
	int iLayoutDirtyFrom;
	int iLaidOutCount;
	float fLayoutWidth;
	float fLayoutHeight;
	xui_font pLayoutFont;
	uint32_t iLayoutLanguageRevision;
	uint32_t iLayoutDpiGeneration;
	uint64_t iResourceRegistryGeneration;
	uint64_t iLayoutResourceGeneration;
	xui_message_document_anchor_t tPendingDocumentAnchor;
} xui_message_list_data_t;

static void __xuiMessageAccessibleClearText(xui_message_node_data_t* pNode)
{
	if ( pNode == NULL ) return;
	xuiDocumentFreeBuffer(pNode->sAccessibleDocumentText);
	pNode->sAccessibleDocumentText = NULL;
	pNode->iAccessibleDocumentId = 0;
	pNode->iAccessibleDocumentRevision = 0;
}

static void __xuiMessageAccessibleClearDocumentValues(xui_message_document_binding_t* pBinding)
{
	uint64_t i;
	if ( pBinding == NULL ) return;
	for ( i = 0; i < pBinding->iAccessibleNodeCount; i++ ) {
		free(pBinding->arrAccessibleNodes[i].sValue);
		pBinding->arrAccessibleNodes[i].sValue = NULL;
	}
	pBinding->iAccessibleRevision = 0;
}

static void __xuiMessageAccessibleFreeDocumentNodes(xui_message_document_binding_t* pBinding)
{
	if ( pBinding == NULL ) return;
	__xuiMessageAccessibleClearDocumentValues(pBinding);
	free(pBinding->arrAccessibleNodes);
	pBinding->arrAccessibleNodes = NULL;
	pBinding->iAccessibleNodeCount = 0;
	pBinding->iAccessibleIdentity = 0;
}

typedef struct xui_message_paint_t {
    xui_message_list_colors_t tColors;
    uint32_t iAuxiliaryColor;
    uint32_t iAuxiliaryHeaderColor;
    uint32_t iTextSelectionColor;
} xui_message_paint_t;

static void __xuiMessageStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
    xui_style_property_t tProperty;
    memset(&tProperty, 0, sizeof(tProperty));
    tProperty.iSize = sizeof(tProperty);
    if ( xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty) == XUI_OK &&
         tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR ) *pColor = tProperty.tValue.iColor;
}

static void __xuiMessageResolvePaint(xui_widget pWidget, const xui_message_list_data_t* pData, xui_message_paint_t* pPaint)
{
    pPaint->tColors = pData->tColors;
    pPaint->iAuxiliaryColor = XUI_COLOR_RGBA(255, 255, 255, 226);
    pPaint->iAuxiliaryHeaderColor = XUI_COLOR_RGBA(232, 237, 243, 220);
    pPaint->iTextSelectionColor = XUI_COLOR_RGBA(68, 130, 205, 104);
    __xuiMessageStyleColor(pWidget, "messagelist.background.color", &pPaint->tColors.iBackgroundColor);
    __xuiMessageStyleColor(pWidget, "messagelist.bubble.self_color", &pPaint->tColors.iSelfBubbleColor);
    __xuiMessageStyleColor(pWidget, "messagelist.bubble.other_color", &pPaint->tColors.iOtherBubbleColor);
    __xuiMessageStyleColor(pWidget, "messagelist.bubble.system_color", &pPaint->tColors.iSystemBubbleColor);
    __xuiMessageStyleColor(pWidget, "messagelist.text.self_color", &pPaint->tColors.iSelfTextColor);
    __xuiMessageStyleColor(pWidget, "messagelist.text.other_color", &pPaint->tColors.iOtherTextColor);
    __xuiMessageStyleColor(pWidget, "messagelist.text.system_color", &pPaint->tColors.iSystemTextColor);
    __xuiMessageStyleColor(pWidget, "messagelist.text.meta_color", &pPaint->tColors.iMetaTextColor);
    __xuiMessageStyleColor(pWidget, "messagelist.avatar.self_color", &pPaint->tColors.iAvatarSelfColor);
    __xuiMessageStyleColor(pWidget, "messagelist.avatar.other_color", &pPaint->tColors.iAvatarOtherColor);
    __xuiMessageStyleColor(pWidget, "messagelist.row.hover_color", &pPaint->tColors.iHoverColor);
    __xuiMessageStyleColor(pWidget, "messagelist.row.selected_color", &pPaint->tColors.iSelectedColor);
    __xuiMessageStyleColor(pWidget, "messagelist.border.color", &pPaint->tColors.iBorderColor);
    __xuiMessageStyleColor(pWidget, "messagelist.auxiliary.background_color", &pPaint->iAuxiliaryColor);
    __xuiMessageStyleColor(pWidget, "messagelist.auxiliary.header_color", &pPaint->iAuxiliaryHeaderColor);
    __xuiMessageStyleColor(pWidget, "messagelist.text.selection_color", &pPaint->iTextSelectionColor);
}

static void __xuiMessageRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
    static const char* const arrKeys[] = {
        "messagelist.background.color",
        "messagelist.bubble.self_color",
        "messagelist.bubble.other_color",
        "messagelist.bubble.system_color",
        "messagelist.text.self_color",
        "messagelist.text.other_color",
        "messagelist.text.system_color",
        "messagelist.text.meta_color",
        "messagelist.avatar.self_color",
        "messagelist.avatar.other_color",
        "messagelist.row.hover_color",
        "messagelist.row.selected_color",
        "messagelist.border.color",
        "messagelist.auxiliary.background_color",
        "messagelist.auxiliary.header_color",
        "messagelist.text.selection_color",
    };
    xui_style_property_info_t tInfo;
    size_t i;
    for ( i = 0; i < sizeof(arrKeys) / sizeof(arrKeys[0]); i++ ) {
        if ( xuiStyleFindProperty(pContext, arrKeys[i]) != 0 ) continue;
        memset(&tInfo, 0, sizeof(tInfo));
        tInfo.iSize = sizeof(tInfo);
        tInfo.sName = arrKeys[i];
        tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
        tInfo.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
        tInfo.pWidgetType = pType;
        (void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
    }
}

static int __xuiMessageNodeCanSelectText(const xui_message_node_data_t* pNode);
static int __xuiMessageNodeCanSelect(const xui_message_node_data_t* pNode);
static int __xuiMessageInvalidateAfterNodeUpdate(xui_widget pWidget, xui_message_list_data_t* pData, int iIndex);
static int __xuiMessageHitDocument(xui_widget pWidget, xui_message_list_data_t* pData,
	int iIndex, double fWorldX, double fWorldY, int bClamp, xui_doc_position_t* pPosition);
static int __xuiMessageHitDocumentCell(xui_widget pWidget, xui_message_list_data_t* pData,
	int iIndex, double fWorldX, double fWorldY, xui_doc_cell_hit_t* pCell);
static int __xuiMessageHitDocumentTaskMarker(xui_widget pWidget,
	xui_message_list_data_t* pData, int iIndex, double fWorldX,
	double fWorldY, xui_doc_node_id* pItem);
static void __xuiMessageCaptureDocumentAnchor(xui_widget pWidget,
	xui_message_list_data_t* pData, xui_rect_t tContent,
	xui_message_document_anchor_t* pAnchor, int bForce);

static int __xuiMessageStyleAffectsMetrics(xui_message_document_binding_t* pBinding,
	xui_document_change_set pChanges)
{
	xui_document_snapshot pAfter = NULL;
	const uint32_t iColorFlags = XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO |
		XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
		XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT;
	uint64_t i;
	int bChanged = 0;
	if ( pChanges == NULL || pBinding->pRenderer->snapshot == NULL ||
	     xuiDocumentAcquireSnapshot(pBinding->pDocument, &pAfter) != XUI_OK ) return 0;
	for ( i = 0; i < pChanges->count; i++ ) {
		const xui_doc_operation_t* pOp = &pChanges->ops[i];
		xui_doc_node_info_t tBefore = {0}, tAfter = {0};
		const xui_doc_attributes_t *a, *b;
		if ( !(pOp->iFlags & XUI_DOC_CHANGE_STYLE) ) continue;
		tBefore.iSize = tAfter.iSize = sizeof(tBefore);
		if ( xuiDocumentSnapshotGetNode(pBinding->pRenderer->snapshot,
			pOp->iNodeId, &tBefore) != XUI_OK ||
		     xuiDocumentSnapshotGetNode(pAfter, pOp->iNodeId, &tAfter) != XUI_OK )
			{ bChanged = 1; break; }
		a = &tBefore.tAttributes; b = &tAfter.tAttributes;
		if ( tBefore.iKind != tAfter.iKind || a->iMarks != b->iMarks ||
		     ((a->iFlags ^ b->iFlags) & ~iColorFlags) ||
		     a->iHeadingLevel != b->iHeadingLevel ||
		     a->iAlignment != b->iAlignment ||
		     a->iRowSpan != b->iRowSpan || a->iColumnSpan != b->iColumnSpan ||
		     a->iListStart != b->iListStart || a->fFontSize != b->fFontSize ||
		     a->fWidth != b->fWidth || a->fHeight != b->fHeight ||
		     a->fParagraphSpacing != b->fParagraphSpacing ||
		     strcmp(a->sFontFamily, b->sFontFamily) )
			{ bChanged = 1; break; }
	}
	xuiDocumentSnapshotRelease(pAfter);
	return bChanged;
}

static size_t __xuiMessageDocumentFindBlock(xui_document_renderer pRenderer,
	xui_doc_node_id iNodeId)
{
	xui_doc_node_info_t tNode = {0};
	while ( iNodeId != 0 ) {
		size_t* pIndex = xrtMapGet(&pRenderer->block_index,
			xuiXrtBytes(&iNodeId, sizeof(iNodeId)));
		if ( pIndex != NULL ) return *pIndex;
		tNode.iSize = sizeof(tNode);
		if ( xuiDocumentSnapshotGetNode(pRenderer->snapshot,
			iNodeId, &tNode) != XUI_OK ) return SIZE_MAX;
		iNodeId = tNode.iParentId;
	}
	return SIZE_MAX;
}

static int __xuiMessageInlineStyleSplits(xui_document_change_set pChanges)
{
	uint64_t i;
	if ( pChanges == NULL || !(pChanges->flags & XUI_DOC_CHANGE_STYLE) ||
	     !(pChanges->flags & XUI_DOC_CHANGE_STRUCTURE) ) return 0;
	for ( i = 0; i < pChanges->count; i++ ) {
		const xui_doc_operation_t* pOp = &pChanges->ops[i];
		if ( !(pOp->iFlags & XUI_DOC_CHANGE_STRUCTURE) ) continue;
		if ( pOp->iKind == XUI_DOC_OP_INSERT ) {
			const xui_doc_operation_t* pNext = i + 1 < pChanges->count ?
				&pChanges->ops[i + 1] : NULL;
			if ( pNext == NULL || pNext->iKind != XUI_DOC_OP_SPLIT ||
			     pNext->iParentId != 0 || pNext->iOtherNodeId != pOp->iNodeId ||
			     pOp->iParentId == 0 ) return 0;
		} else if ( pOp->iKind == XUI_DOC_OP_SPLIT ) {
			const xui_doc_operation_t* pPrevious = i ? &pChanges->ops[i - 1] : NULL;
			if ( pPrevious == NULL || pPrevious->iKind != XUI_DOC_OP_INSERT ||
			     pOp->iParentId != 0 || pPrevious->iNodeId != pOp->iOtherNodeId )
				return 0;
		} else return 0;
	}
	return 1;
}

static void __xuiMessageDocumentChanged(xui_document pDocument, xui_document_change_set pChanges, void* pUser)
{
	xui_message_document_binding_t* pBinding = (xui_message_document_binding_t*)pUser;
	xui_message_list_data_t* pData;
	xui_document_snapshot pSnapshot = NULL;
	xui_doc_position_t tMapped;
	xui_message_document_anchor_t tAnchor = {0};
	int iMapping;
	int bSelectionMapFailed = 0;
	int bSelectionAffected;
	int bTableSelectionCleared = 0;
	int iRet;
	size_t i;
	(void)pDocument;
	if ( pBinding == NULL || pBinding->pList == NULL ) return;
	pData = (xui_message_list_data_t*)xuiWidgetGetTypeData(pBinding->pList);
	if ( pData == NULL || pBinding->iIndex < 0 || pBinding->iIndex >= pData->iNodeCount ||
	     pData->arrNodes[pBinding->iIndex].pDocumentBinding != pBinding ) return;
	bSelectionAffected = pData->iSelectionAnchorNode == pBinding->iIndex ||
		pData->iSelectionActiveNode == pBinding->iIndex;
	__xuiMessageAccessibleClearText(&pData->arrNodes[pBinding->iIndex]);
	__xuiMessageAccessibleClearDocumentValues(pBinding);
	/* Capture against the old renderer snapshot. A later layout cannot recover
	 * the old visible line after an inline style split reflows that same block. */
	if ( pData->tPendingDocumentAnchor.pBinding == pBinding ) {
		tAnchor = pData->tPendingDocumentAnchor;
		memset(&pData->tPendingDocumentAnchor, 0, sizeof(pData->tPendingDocumentAnchor));
	} else if ( pData->tPendingDocumentAnchor.pBinding == NULL && pChanges != NULL &&
	           (pChanges->flags & XUI_DOC_CHANGE_STYLE) &&
	           !(pChanges->flags & (XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_RESET)) &&
	           __xuiMessageStyleAffectsMetrics(pBinding, pChanges) ) {
		__xuiMessageCaptureDocumentAnchor(pBinding->pList, pData,
			xuiWidgetGetContentRect(pBinding->pList), &tAnchor, 1);
		if ( tAnchor.pBinding != pBinding ) {
			xrtFree(tAnchor.arrMeasuredPrefix);
			memset(&tAnchor, 0, sizeof(tAnchor));
		}
	}
	if ( tAnchor.pBinding != NULL &&
	     (pChanges == NULL ||
	      xuiDocumentMapPosition(pChanges, &tAnchor.tPosition, &tMapped, &iMapping) != XUI_OK ||
	      iMapping == XUI_DOC_MAP_DELETED) ) {
		xrtFree(tAnchor.arrMeasuredPrefix);
		memset(&tAnchor, 0, sizeof(tAnchor));
	} else if ( tAnchor.pBinding != NULL ) tAnchor.tPosition = tMapped;
	if ( pBinding->tSelection.tAnchor.iSize != 0 ) {
		if ( xuiDocumentMapPosition(pChanges, &pBinding->tSelection.tAnchor, &tMapped, &iMapping) == XUI_OK )
			pBinding->tSelection.tAnchor = tMapped;
		else memset(&pBinding->tSelection, 0, sizeof(pBinding->tSelection));
		if ( pBinding->tSelection.tAnchor.iSize != 0 ) {
			if ( xuiDocumentMapPosition(pChanges, &pBinding->tSelection.tCaret, &tMapped, &iMapping) == XUI_OK )
				pBinding->tSelection.tCaret = tMapped;
			else memset(&pBinding->tSelection, 0, sizeof(pBinding->tSelection));
		}
	}
	if ( pData->iSelectionAnchorNode == pBinding->iIndex &&
	     pData->tSelectionAnchorDocument.iSize != 0 ) {
		if ( xuiDocumentMapPosition(pChanges, &pData->tSelectionAnchorDocument, &tMapped, &iMapping) == XUI_OK )
			pData->tSelectionAnchorDocument = tMapped;
		else bSelectionMapFailed = 1;
	}
	if ( pData->iSelectionActiveNode == pBinding->iIndex &&
	     pData->tSelectionActiveDocument.iSize != 0 ) {
		if ( xuiDocumentMapPosition(pChanges, &pData->tSelectionActiveDocument, &tMapped, &iMapping) == XUI_OK )
			pData->tSelectionActiveDocument = tMapped;
		else bSelectionMapFailed = 1;
	}
	if ( bSelectionMapFailed ) {
		pData->iSelectionAnchorNode = -1;
		pData->iSelectionActiveNode = -1;
		memset(&pData->tSelectionAnchorDocument, 0, sizeof(pData->tSelectionAnchorDocument));
		memset(&pData->tSelectionActiveDocument, 0, sizeof(pData->tSelectionActiveDocument));
		memset(&pBinding->tSelection, 0, sizeof(pBinding->tSelection));
		pData->iDocumentSelectionNode = -1;
		pData->bSelecting = 0;
		pData->bDocumentSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pBinding->pList)) == pBinding->pList )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pBinding->pList), pBinding->pList);
	}
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	if ( iRet == XUI_OK ) iRet = xuiDocumentRendererSetSnapshot(pBinding->pRenderer, pSnapshot, pChanges);
	if ( pData->iTableSelectionNode == pBinding->iIndex &&
	     pData->tTableSelection.iTableId != 0 ) {
		memset(&pData->tTableSelection, 0, sizeof(pData->tTableSelection));
		memset(&pData->tTableDragAnchor, 0, sizeof(pData->tTableDragAnchor));
		memset(&pData->tTableDragFocus, 0, sizeof(pData->tTableDragFocus));
		pData->iTableSelectionNode = -1;
		if ( pData->bTableSelecting &&
		     xuiGetPointerCapture(xuiWidgetGetContext(pBinding->pList)) == pBinding->pList )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pBinding->pList), pBinding->pList);
		pData->bTableSelecting = 0;
		bTableSelectionCleared = 1;
	}
	if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
	if ( iRet == XUI_OK && tAnchor.pBinding != NULL ) {
		/* Previously unmeasured styled predecessors must be settled as well as
		 * the old measured prefix before resolving the mapped visible caret. */
		if ( tAnchor.arrMeasuredPrefix != NULL && pChanges != NULL &&
		     tAnchor.iOldBlockCount == pBinding->pRenderer->count ) {
			if ( (pChanges->flags & XUI_DOC_CHANGE_STRUCTURE) &&
			     !__xuiMessageInlineStyleSplits(pChanges) )
				memset(tAnchor.arrMeasuredPrefix, 1, tAnchor.iPrefixCount);
			for ( i = 0; i < pChanges->count; i++ ) {
				size_t iBlock = __xuiMessageDocumentFindBlock(pBinding->pRenderer,
					pChanges->ops[i].iNodeId);
				if ( iBlock < tAnchor.iPrefixCount )
					tAnchor.arrMeasuredPrefix[iBlock] = 1;
			}
		}
		pData->tPendingDocumentAnchor = tAnchor;
	} else xrtFree(tAnchor.arrMeasuredPrefix);
	pBinding->bNeedsSync = iRet != XUI_OK;
	pData->arrNodes[pBinding->iIndex].bMeasureDirty = 1;
	if ( pBinding->iIndex < pData->iLayoutDirtyFrom ) pData->iLayoutDirtyFrom = pBinding->iIndex;
	pData->iChangeCount++;
	(void)xuiWidgetInvalidate(pBinding->pList, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	xuiInternalAccessibilityQueue(pBinding->pList, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pBinding->pList, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bSelectionAffected || bTableSelectionCleared )
		xuiInternalAccessibilityQueue(pBinding->pList, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
}

static void __xuiMessageDocumentRendererDesc(xui_message_document_binding_t* pBinding,
	xui_message_list_data_t* pData, xui_doc_renderer_desc_t* pDesc)
{
	xui_message_paint_t tPaint;
	*pDesc = pBinding->tDesc.tRenderer;
	if ( pDesc->iSize == 0 ) { memset(pDesc, 0, sizeof(*pDesc)); pDesc->iSize = sizeof(*pDesc); }
	if ( pDesc->tFonts.normal == NULL ) pDesc->tFonts.normal = pData->bUseDefaultFont ?
		xuiGetDefaultFont(xuiWidgetGetContext(pBinding->pList)) : pData->pFont;
	if ( pDesc->iTextColor == 0 ) {
		int iType = pData->arrNodes[pBinding->iIndex].iType;
		__xuiMessageResolvePaint(pBinding->pList, pData, &tPaint);
		pDesc->iTextColor = iType == XUI_MESSAGE_NODE_SELF ? tPaint.tColors.iSelfTextColor :
			iType == XUI_MESSAGE_NODE_SYSTEM ? tPaint.tColors.iSystemTextColor : tPaint.tColors.iOtherTextColor;
	}
}

static int __xuiMessageDocumentSync(xui_message_document_binding_t* pBinding)
{
	xui_document_snapshot pSnapshot = NULL;
	xui_message_list_data_t* pData = (xui_message_list_data_t*)xuiWidgetGetTypeData(pBinding->pList);
	xui_doc_renderer_desc_t tDesc;
	xui_document_renderer pNext = NULL;
	int iRet;
	if ( pData != NULL && pBinding->iIndex >= 0 && pBinding->iIndex < pData->iNodeCount &&
	     pData->arrNodes[pBinding->iIndex].pDocumentBinding == pBinding ) {
		__xuiMessageDocumentRendererDesc(pBinding, pData, &tDesc);
		if ( pBinding->pResolvedFont != tDesc.tFonts.normal ) {
			iRet = xuiDocumentRendererCreate(xuiWidgetGetContext(pBinding->pList), &tDesc, &pNext);
			if ( iRet == XUI_OK ) iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
			if ( iRet == XUI_OK ) iRet = xuiDocumentRendererSetSnapshot(pNext, pSnapshot, NULL);
			if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
			if ( iRet != XUI_OK ) { xuiDocumentRendererRelease(pNext); return iRet; }
			xuiDocumentRendererRelease(pBinding->pRenderer);
			pBinding->pRenderer = pNext;
			pData->arrNodes[pBinding->iIndex].bDocumentSizeExact = 0;
			pBinding->pResolvedFont = tDesc.tFonts.normal;
			pBinding->iResolvedTextColor = tDesc.iTextColor;
			pBinding->bNeedsSync = 0;
		} else if ( pBinding->iResolvedTextColor != tDesc.iTextColor ) {
			/* Default text color is read during Draw, not text shaping. */
			pBinding->pRenderer->desc.iTextColor = tDesc.iTextColor;
			pBinding->iResolvedTextColor = tDesc.iTextColor;
		}
	}
	if ( !pBinding->bNeedsSync ) return XUI_OK;
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	if ( iRet == XUI_OK ) iRet = xuiDocumentRendererSetSnapshot(pBinding->pRenderer, pSnapshot, NULL);
	if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
	if ( iRet == XUI_OK ) pBinding->bNeedsSync = 0;
	return iRet;
}

static xui_message_list_data_t* __xuiMessageListGetData(xui_widget pWidget)
{
	if ( pWidget == NULL ) return NULL;
	if ( strcmp(xuiWidgetTypeGetName(xuiWidgetGetType(pWidget)), "messagelist") != 0 ) return NULL;
	return (xui_message_list_data_t*)xuiWidgetGetTypeData(pWidget);
}

static const char* __xuiMessageText(const char* sText)
{
	return (sText != NULL) ? sText : "";
}

static char* __xuiMessageDup(const char* sText)
{
	char* sCopy;
	size_t iLen;
	sText = __xuiMessageText(sText);
	iLen = strlen(sText);
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) return NULL;
	memcpy(sCopy, sText, iLen + 1u);
	return sCopy;
}

static int __xuiMessageReplace(char** ppText, const char* sText)
{
	char* sCopy;
	if ( ppText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sCopy = __xuiMessageDup(sText);
	if ( sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( *ppText != NULL ) xrtFree(*ppText);
	*ppText = sCopy;
	return XUI_OK;
}

static float __xuiMessageMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiMessageMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static int __xuiMessageAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static int __xuiMessageDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pDraw == NULL) || (pProxy->drawRectFill == NULL) || (__xuiMessageAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiMessageDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiMessageAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	return __xuiMessageDrawFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiMessageDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiMessageAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	if ( (pProxy == NULL) || (pDraw == NULL) || (pProxy->drawRectStroke == NULL) ) {
		return XUI_OK;
	}
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static int __xuiMessageDrawText(xui_proxy pProxy, xui_draw_context pDraw, const xui_text_item_t* pTextItem, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
    xui_font pFont = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* sText = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

	if ( (pProxy == NULL) || (pDraw == NULL) || (pFont == NULL) || (pProxy->drawText == NULL) || (sText == NULL) || (sText[0] == 0) ||
	     (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiMessageAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawText(pProxy, pDraw, pTextItem, tRect, iColor, iFlags);
}

static float __xuiMessageClamp(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) fMax = fMin;
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static int __xuiMessageFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static int __xuiMessageNodeTypeValid(int iType)
{
	return iType == XUI_MESSAGE_NODE_SELF || iType == XUI_MESSAGE_NODE_OTHER || iType == XUI_MESSAGE_NODE_SYSTEM || iType == XUI_MESSAGE_NODE_AUXILIARY;
}

static int __xuiMessageNodeHasExtension(const xui_message_node_t* pNode)
{
	return pNode != NULL && pNode->iSize >= offsetof(xui_message_node_t, iAuxiliaryKind) + sizeof(pNode->iAuxiliaryKind);
}

static int __xuiMessageDescValid(const xui_message_list_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( pDesc->iNodeCount < 0 ) return 0;
	if ( (pDesc->iNodeCount > 0) && (pDesc->arrNodes == NULL) ) return 0;
	return 1;
}

static void __xuiMessageDefaultMetrics(xui_message_list_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fPaddingX = 16.0f;
	pMetrics->fPaddingY = 14.0f;
	pMetrics->fNodeGap = 14.0f;
	pMetrics->fAvatarSize = 34.0f;
	pMetrics->fAvatarGap = 8.0f;
	pMetrics->fBubblePaddingX = 12.0f;
	pMetrics->fBubblePaddingY = 8.0f;
	pMetrics->fSystemPaddingX = 8.0f;
	pMetrics->fSystemPaddingY = 4.0f;
	pMetrics->fMetaHeight = 18.0f;
	pMetrics->fMinBubbleHeight = 36.0f;
	pMetrics->fWheelStep = 48.0f;
}

static void __xuiMessageDefaultColors(xui_message_list_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iBackgroundColor = XUI_COLOR_RGBA(242, 243, 245, 255);
	pColors->iSelfBubbleColor = XUI_COLOR_RGBA(188, 232, 255, 255);
	pColors->iOtherBubbleColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pColors->iSystemBubbleColor = XUI_COLOR_RGBA(255, 255, 255, 180);
	pColors->iSelfTextColor = XUI_COLOR_RGBA(20, 28, 38, 255);
	pColors->iOtherTextColor = XUI_COLOR_RGBA(20, 28, 38, 255);
	pColors->iSystemTextColor = XUI_COLOR_RGBA(154, 158, 166, 255);
	pColors->iMetaTextColor = XUI_COLOR_RGBA(148, 153, 162, 255);
	pColors->iAvatarSelfColor = XUI_COLOR_RGBA(64, 82, 104, 255);
	pColors->iAvatarOtherColor = XUI_COLOR_RGBA(160, 198, 218, 255);
	pColors->iHoverColor = XUI_COLOR_RGBA(75, 138, 208, 40);
	pColors->iSelectedColor = XUI_COLOR_RGBA(47, 128, 237, 70);
	pColors->iBorderColor = XUI_COLOR_RGBA(214, 220, 228, 255);
}

static int __xuiMessageMetricsValid(const xui_message_list_metrics_t* pMetrics)
{
	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	return __xuiMessageFloatValid(pMetrics->fPaddingX) &&
	       __xuiMessageFloatValid(pMetrics->fPaddingY) &&
	       __xuiMessageFloatValid(pMetrics->fNodeGap) &&
	       __xuiMessageFloatValid(pMetrics->fAvatarSize) &&
	       __xuiMessageFloatValid(pMetrics->fAvatarGap) &&
	       __xuiMessageFloatValid(pMetrics->fBubblePaddingX) &&
	       __xuiMessageFloatValid(pMetrics->fBubblePaddingY) &&
	       __xuiMessageFloatValid(pMetrics->fSystemPaddingX) &&
	       __xuiMessageFloatValid(pMetrics->fSystemPaddingY) &&
	       __xuiMessageFloatValid(pMetrics->fMetaHeight) &&
	       __xuiMessageFloatValid(pMetrics->fMinBubbleHeight) &&
	       __xuiMessageFloatValid(pMetrics->fWheelStep);
}

static void __xuiMessageFreeDocumentBinding(xui_message_document_binding_t* pBinding)
{
	if ( pBinding == NULL ) return;
	if ( pBinding->iSubscription != 0 ) xuiDocumentUnsubscribe(pBinding->pDocument, pBinding->iSubscription);
	__xuiMessageAccessibleFreeDocumentNodes(pBinding);
	xuiDocumentRendererRelease(pBinding->pRenderer);
	xuiDocumentRelease(pBinding->pDocument);
	free(pBinding);
}

static void __xuiMessageFreeNode(xui_message_node_data_t* pNode)
{
	if ( pNode == NULL ) return;
	__xuiMessageAccessibleClearText(pNode);
	__xuiMessageFreeDocumentBinding(pNode->pDocumentBinding);
	if ( pNode->pTextLayout != NULL ) xuiTextLayoutDestroy(pNode->pTextLayout);
	if ( pNode->pTitleLayout != NULL ) xuiTextLayoutDestroy(pNode->pTitleLayout);
	if ( pNode->arrTextCarets != NULL ) xrtFree(pNode->arrTextCarets);
	if ( pNode->sId != NULL ) xrtFree(pNode->sId);
	if ( pNode->sSender != NULL ) xrtFree(pNode->sSender);
	if ( pNode->sTime != NULL ) xrtFree(pNode->sTime);
	if ( pNode->sText != NULL ) xrtFree(pNode->sText);
	if ( pNode->sParentId != NULL ) xrtFree(pNode->sParentId);
	if ( pNode->sTitle != NULL ) xrtFree(pNode->sTitle);
	memset(pNode, 0, sizeof(*pNode));
}

static void __xuiMessageInvalidateNodeTextLayout(xui_message_node_data_t* pNode)
{
	if ( pNode == NULL ) return;
	pNode->sTextLayoutSource = NULL;
	pNode->iTextCaretCount = 0;
}

static int __xuiMessageCopyNode(xui_message_node_data_t* pDst, const xui_message_node_t* pSrc)
{
	int iType;
	int iRet;
	if ( (pDst == NULL) || (pSrc == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pDst, 0, sizeof(*pDst));
	pDst->bMeasureDirty = 1;
	iType = __xuiMessageNodeTypeValid(pSrc->iType) ? pSrc->iType : XUI_MESSAGE_NODE_OTHER;
	pDst->iType = iType;
	pDst->iFlags = pSrc->iFlags;
	pDst->pUser = pSrc->pUser;
	pDst->iAuxiliaryKind = __xuiMessageNodeHasExtension(pSrc) ? pSrc->iAuxiliaryKind : 0;
	iRet = __xuiMessageReplace(&pDst->sId, pSrc->sId);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sSender, pSrc->sSender);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sTime, pSrc->sTime);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sText, pSrc->sText);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sParentId, __xuiMessageNodeHasExtension(pSrc) ? pSrc->sParentId : NULL);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sTitle, __xuiMessageNodeHasExtension(pSrc) ? pSrc->sTitle : NULL);
	if ( iRet != XUI_OK ) __xuiMessageFreeNode(pDst);
	return iRet;
}

static int __xuiMessageReserve(xui_message_list_data_t* pData, int iCapacity)
{
	xui_message_node_data_t* pNewNodes;
	int iNewCapacity;
	if ( pData == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pData->iNodeCapacity ) return XUI_OK;
	iNewCapacity = (pData->iNodeCapacity > 0) ? pData->iNodeCapacity * 2 : 16;
	while ( iNewCapacity < iCapacity ) iNewCapacity *= 2;
	pNewNodes = (xui_message_node_data_t*)xrtMalloc(sizeof(*pNewNodes) * (size_t)iNewCapacity);
	if ( pNewNodes == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pNewNodes, 0, sizeof(*pNewNodes) * (size_t)iNewCapacity);
	if ( pData->arrNodes != NULL ) {
		memcpy(pNewNodes, pData->arrNodes, sizeof(*pNewNodes) * (size_t)pData->iNodeCount);
		xrtFree(pData->arrNodes);
	}
	pData->arrNodes = pNewNodes;
	pData->iNodeCapacity = iNewCapacity;
	return XUI_OK;
}

static void __xuiMessageClearData(xui_message_list_data_t* pData)
{
	int i;
	if ( pData == NULL ) return;
	pData->iPressedTaskNode = 0;
	xrtFree(pData->tPendingDocumentAnchor.arrMeasuredPrefix);
	memset(&pData->tPendingDocumentAnchor, 0, sizeof(pData->tPendingDocumentAnchor));
	if ( pData->bDocumentSelecting && pData->iDocumentSelectionNode >= 0 &&
	     pData->iDocumentSelectionNode < pData->iNodeCount &&
	     pData->arrNodes[pData->iDocumentSelectionNode].pDocumentBinding != NULL ) {
		xui_widget pWidget = pData->arrNodes[pData->iDocumentSelectionNode].pDocumentBinding->pList;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		__xuiMessageFreeNode(&pData->arrNodes[i]);
	}
	pData->iNodeCount = 0;
	pData->iDocumentNodeCount = 0;
	pData->iHover = -1;
	pData->iSelected = -1;
	pData->iSelectionAnchorNode = -1;
	pData->iSelectionAnchorOffset = 0;
	pData->iSelectionActiveNode = -1;
	pData->iSelectionActiveOffset = 0;
	memset(&pData->tSelectionAnchorDocument, 0, sizeof(pData->tSelectionAnchorDocument));
	memset(&pData->tSelectionActiveDocument, 0, sizeof(pData->tSelectionActiveDocument));
	pData->bSelecting = 0;
	pData->iDocumentSelectionNode = -1;
	pData->bDocumentSelecting = 0;
	memset(&pData->tTableSelection, 0, sizeof(pData->tTableSelection));
	memset(&pData->tTableDragAnchor, 0, sizeof(pData->tTableDragAnchor));
	memset(&pData->tTableDragFocus, 0, sizeof(pData->tTableDragFocus));
	pData->iTableSelectionNode = -1;
	pData->bTableSelecting = 0;
	pData->fScrollY = 0.0f;
	pData->fContentHeight = 0.0f;
	pData->bLayoutValid = 0;
	pData->iLayoutDirtyFrom = 0;
	pData->iLaidOutCount = 0;
}

static xui_font __xuiMessageFont(xui_widget pWidget, xui_message_list_data_t* pData)
{
	if ( pData == NULL ) return NULL;
	return pData->bUseDefaultFont ? xuiGetDefaultFont(xuiWidgetGetContext(pWidget)) : pData->pFont;
}

static xui_message_node_t __xuiMessagePublicNode(const xui_message_node_data_t* pNode)
{
	xui_message_node_t tNode;
	memset(&tNode, 0, sizeof(tNode));
	if ( pNode == NULL ) return tNode;
	tNode.iSize = sizeof(tNode);
	tNode.sId = __xuiMessageText(pNode->sId);
	tNode.sSender = __xuiMessageText(pNode->sSender);
	tNode.sTime = __xuiMessageText(pNode->sTime);
	tNode.sText = __xuiMessageText(pNode->sText);
	tNode.iType = pNode->iType;
	tNode.iFlags = pNode->iFlags;
	tNode.pUser = pNode->pUser;
	tNode.sParentId = __xuiMessageText(pNode->sParentId);
	tNode.sTitle = __xuiMessageText(pNode->sTitle);
	tNode.iAuxiliaryKind = pNode->iAuxiliaryKind;
	return tNode;
}

static float __xuiMessageTextWidth(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_proxy pProxy;
	xui_vec2_t tSize;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) ) {
		tSize = (xui_vec2_t){0.0f, 0.0f};
		if ( pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=__xuiMessageText(sText), .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) == XUI_OK ) {
			return tSize.fX;
		}
	}
	return (float)strlen(__xuiMessageText(sText)) * 7.0f;
}

static float __xuiMessageLineHeight(xui_context pContext, xui_font pFont)
{
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (pFont != NULL) ) {
		memset(&tMetrics, 0, sizeof(tMetrics));
		if ( pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK && tMetrics.fLineHeight > 0.0f ) {
			return tMetrics.fLineHeight;
		}
	}
	return 18.0f;
}

static int __xuiMessageEnsureNodeTextLayout(xui_widget pWidget, xui_message_list_data_t* pData,
	xui_message_node_data_t* pNode, float fMaxWidth, xui_text_layout* ppLayout)
{
	xui_text_layout_desc_t tDesc;
	xui_font pFont;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pNode == NULL || ppLayout == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppLayout = NULL;
	pFont = __xuiMessageFont(pWidget, pData);
	fMaxWidth = __xuiMessageMax(1.0f, fMaxWidth);
	if ( pNode->pTextLayout != NULL && pNode->pTextLayoutFont == pFont &&
	     pNode->sTextLayoutSource == pNode->sText && pNode->fTextLayoutWidth == fMaxWidth &&
	     pNode->iTextDpiGeneration == pWidget->pContext->iDpiGeneration ) {
		*ppLayout = pNode->pTextLayout;
		return XUI_OK;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = __xuiMessageText(pNode->sText);
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = fMaxWidth;
	tDesc.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.fLineGap = 2.0f;
	pNode->iTextCaretCount = 0;
	if ( pNode->pTextLayout != NULL ) iRet = xuiTextLayoutReset(pNode->pTextLayout, &tDesc);
	else iRet = xuiTextLayoutCreate(xuiWidgetGetContext(pWidget), &pNode->pTextLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		if ( pNode->pTextLayout != NULL ) xuiTextLayoutDestroy(pNode->pTextLayout);
		pNode->pTextLayout = NULL;
		pNode->pTextLayoutFont = NULL;
		pNode->sTextLayoutSource = NULL;
		pNode->fTextLayoutWidth = 0.0f;
		return iRet;
	}
	pNode->pTextLayoutFont = pFont;
	pNode->sTextLayoutSource = pNode->sText;
	pNode->fTextLayoutWidth = fMaxWidth;
	pNode->iTextDpiGeneration = pWidget->pContext->iDpiGeneration;
	*ppLayout = pNode->pTextLayout;
	return XUI_OK;
}

static int __xuiMessageMeasureNodeWrapped(xui_widget pWidget, xui_message_list_data_t* pData,
	xui_message_node_data_t* pNode, float fMaxWidth, xui_vec2_t* pSize)
{
	xui_text_layout pLayout;
	xui_doc_rect_t tDocumentSize;
	int bExact;
	int iRet;
	if ( pNode->pDocumentBinding != NULL ) {
		iRet = __xuiMessageDocumentSync(pNode->pDocumentBinding);
		if ( iRet != XUI_OK ) return iRet;
		fMaxWidth = __xuiMessageMax(1.0f, fMaxWidth);
		iRet = xuiDocumentRendererLayout(pNode->pDocumentBinding->pRenderer, fMaxWidth, 0.0, 0.0);
		if ( iRet == XUI_OK ) iRet = xuiDocumentRendererGetSize(pNode->pDocumentBinding->pRenderer, &tDocumentSize, &bExact);
		if ( iRet != XUI_OK ) return iRet;
		if ( !isfinite(tDocumentSize.height) || tDocumentSize.height < 0 || tDocumentSize.height > 100000000.0 ) return XUI_DOC_ERROR_LIMIT;
		pNode->bDocumentSizeExact = bExact;
		pSize->fX = fMaxWidth;
		pSize->fY = (float)__xuiMessageMax(20.0f, (float)tDocumentSize.height);
		return XUI_OK;
	}

	iRet = __xuiMessageEnsureNodeTextLayout(pWidget, pData, pNode, fMaxWidth, &pLayout);
	if ( iRet == XUI_OK ) {
		*pSize = xuiTextLayoutGetSize(pLayout);
		return XUI_OK;
	}
	pSize->fX = __xuiMessageMin(__xuiMessageTextWidth(pWidget, __xuiMessageFont(pWidget, pData), pNode->sText), fMaxWidth);
	pSize->fY = __xuiMessageLineHeight(xuiWidgetGetContext(pWidget), __xuiMessageFont(pWidget, pData));
	return iRet;
}

static int __xuiMessageMeasureTitle(xui_widget pWidget, xui_message_list_data_t* pData,
	xui_message_node_data_t* pNode, const char* sText, float fMaxWidth, xui_vec2_t* pSize)
{
	xui_text_layout_desc_t tDesc;
	xui_font pFont;
	uint32_t iRevision;
	int iRet;
	if ( pWidget == NULL || pData == NULL || pNode == NULL || pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pFont = __xuiMessageFont(pWidget, pData);
	iRevision = xuiGetLanguageRevision(xuiWidgetGetContext(pWidget));
	fMaxWidth = __xuiMessageMax(1.0f, fMaxWidth);
	if ( pNode->pTitleLayout != NULL && pNode->pTitleLayoutFont == pFont &&
	     pNode->sTitleLayoutSource == sText && pNode->fTitleLayoutWidth == fMaxWidth &&
	     pNode->iTitleLanguageRevision == iRevision &&
	     pNode->iTitleDpiGeneration == pWidget->pContext->iDpiGeneration ) {
		*pSize = xuiTextLayoutGetSize(pNode->pTitleLayout);
		return XUI_OK;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = __xuiMessageText(sText);
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = fMaxWidth;
	tDesc.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.fLineGap = 2.0f;
	if ( pNode->pTitleLayout != NULL ) iRet = xuiTextLayoutReset(pNode->pTitleLayout, &tDesc);
	else iRet = xuiTextLayoutCreate(xuiWidgetGetContext(pWidget), &pNode->pTitleLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		if ( pNode->pTitleLayout != NULL ) xuiTextLayoutDestroy(pNode->pTitleLayout);
		pNode->pTitleLayout = NULL;
		pSize->fX = __xuiMessageMin(__xuiMessageTextWidth(pWidget, tDesc.pFont, sText), fMaxWidth);
		pSize->fY = __xuiMessageLineHeight(xuiWidgetGetContext(pWidget), tDesc.pFont);
		return iRet;
	}
	pNode->pTitleLayoutFont = pFont;
	pNode->sTitleLayoutSource = sText;
	pNode->fTitleLayoutWidth = fMaxWidth;
	pNode->iTitleLanguageRevision = iRevision;
	pNode->iTitleDpiGeneration = pWidget->pContext->iDpiGeneration;
	*pSize = xuiTextLayoutGetSize(pNode->pTitleLayout);
	return XUI_OK;
}

static const char* __xuiMessageAuxiliaryTitle(xui_widget pWidget, const xui_message_node_data_t* pNode)
{
	if ( pNode != NULL && pNode->sTitle != NULL && pNode->sTitle[0] != 0 ) return pNode->sTitle;
	return xuiTranslate(xuiWidgetGetContext(pWidget),
		(pNode != NULL && pNode->iAuxiliaryKind == XUI_MESSAGE_AUXILIARY_TOOL) ? XUI_TR_MESSAGE_TOOL : XUI_TR_MESSAGE_THINKING);
}

/* Node bottoms and selectable counts are monotone prefix indexes. */
static int __xuiMessageLowerBoundY(const xui_message_list_data_t* pData, float fY)
{
	int iLow = 0;
	int iHigh = pData->iLaidOutCount;
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		const xui_rect_t* pRect = &pData->arrNodes[iMid].tNodeRect;
		XUI_MESSAGE_LIST_AUDIT_STEP(HitNode);
		if ( pRect->fY + pRect->fH < fY ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	return iLow;
}

static int __xuiMessageSelectableByCount(const xui_message_list_data_t* pData, int iCount)
{
	int iLow = 0;
	int iHigh = pData->iLaidOutCount;
	if ( iCount <= 0 ) return -1;
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		XUI_MESSAGE_LIST_AUDIT_STEP(HitNode);
		if ( pData->arrNodes[iMid].iSelectablePrefix < iCount ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	return iLow < pData->iLaidOutCount ? iLow : -1;
}

static void __xuiMessageDirtyNode(xui_message_list_data_t* pData, int iIndex)
{
	pData->arrNodes[iIndex].bMeasureDirty = 1;
	if ( iIndex < pData->iLayoutDirtyFrom ) pData->iLayoutDirtyFrom = iIndex;
}

static int __xuiMessageLayoutNodesForContent(xui_widget pWidget,
	xui_message_list_data_t* pData, xui_rect_t tContent, int bUpdateScroll);

/* The renderer's internal height index is a Fenwick tree. Keep this small
 * lookup local so the MessageList audit can still link against the DLL. */
static double __xuiMessageDocumentHeightBefore(xui_document_renderer pRenderer, size_t iIndex)
{
	double fHeight = 0;
	for ( ; iIndex; iIndex -= iIndex & (~iIndex + 1) ) fHeight += pRenderer->heights[iIndex];
	return fHeight;
}

static size_t __xuiMessageDocumentBlockAt(xui_document_renderer pRenderer, double fY)
{
	size_t iIndex = 0, iBit = 1;
	double fTop = 0;
	while ( iBit <= pRenderer->count / 2 ) iBit <<= 1;
	for ( ; iBit; iBit >>= 1 ) {
		size_t iNext = iIndex + iBit;
		if ( iNext <= pRenderer->count && fTop + pRenderer->heights[iNext] <= fY ) {
			fTop += pRenderer->heights[iNext];
			iIndex = iNext;
		}
	}
	return iIndex < pRenderer->count ? iIndex : pRenderer->count - 1;
}

static void __xuiMessageCaptureDocumentAnchor(xui_widget pWidget,
	xui_message_list_data_t* pData, xui_rect_t tContent,
	xui_message_document_anchor_t* pAnchor, int bForce)
{
	xui_message_node_data_t* pNode;
	xui_document_renderer pRenderer;
	xui_doc_rect_t tCaret;
	float fLocalY, fTextScreenY;
	int iIndex, bAll, bFrozen;
	size_t i, iBlock;
	memset(pAnchor, 0, sizeof(*pAnchor));
	if ( pData->iDocumentNodeCount == 0 || pData->iLaidOutCount == 0 ||
	     tContent.fH <= 0 || pData->fScrollY <= 0 ) return;
	if ( pData->bAutoScroll && pData->fScrollY >=
	     __xuiMessageMax(0.0f, pData->fContentHeight - pData->fLayoutHeight) - 0.5f ) return;
	iIndex = __xuiMessageLowerBoundY(pData, pData->fScrollY);
	if ( iIndex < 0 || iIndex >= pData->iLaidOutCount ) return;
	pNode = &pData->arrNodes[iIndex];
	if ( pNode->pDocumentBinding == NULL ||
	     (pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
	      (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0) ) return;
	bAll = !pData->bLayoutValid || pData->fLayoutWidth != tContent.fW ||
		pData->pLayoutFont != __xuiMessageFont(pWidget, pData) ||
		pData->iLayoutLanguageRevision != xuiGetLanguageRevision(xuiWidgetGetContext(pWidget)) ||
		pData->iLayoutDpiGeneration != pWidget->pContext->iDpiGeneration;
	if ( !bForce && !bAll && !pNode->bMeasureDirty &&
	     pData->iLayoutResourceGeneration == xuiResourceGetRegistryGeneration(
	     xuiWidgetGetContext(pWidget)) ) return;
	fTextScreenY = pNode->tTextRect.fY - pData->fScrollY;
	pAnchor->fScreenY = __xuiMessageMin(__xuiMessageMax(0.0f, tContent.fH - 1.0f),
		__xuiMessageMax(__xuiMessageMin(20.0f, tContent.fH / 4.0f),
			fTextScreenY + 4.0f));
	fLocalY = pData->fScrollY + pAnchor->fScreenY - pNode->tTextRect.fY;
	if ( fLocalY < 0 || fLocalY >= pNode->tTextRect.fH ) return;
	pRenderer = pNode->pDocumentBinding->pRenderer;
	if ( pRenderer == NULL || pRenderer->snapshot == NULL ) return;
	bFrozen = pRenderer->freeze_dynamic_refresh;
	pRenderer->freeze_dynamic_refresh = 1;
	if ( xuiDocumentRendererHitTest(pRenderer,
		__xuiMessageMin(45.0f, pNode->tTextRect.fW / 3.0f),
		fLocalY, &pAnchor->tPosition) == XUI_OK &&
	     xuiDocumentRendererGetCaretRect(pRenderer, &pAnchor->tPosition, &tCaret) == XUI_OK ) {
		pAnchor->pBinding = pNode->pDocumentBinding;
		pAnchor->iNode = iIndex;
		pAnchor->fLineFraction = __xuiMessageClamp(
			(float)((fLocalY - tCaret.y) / __xuiMessageMax(1.0f, (float)tCaret.height)),
			0.0f, 0.99f);
		if ( pRenderer->mode != XUI_DOC_SOURCE_TEXT && pRenderer->count ) {
			iBlock = __xuiMessageDocumentBlockAt(pRenderer, fLocalY);
			pAnchor->iPrefixCount = iBlock + 1;
			pAnchor->iOldBlockCount = pRenderer->count;
			pAnchor->arrMeasuredPrefix = (uint8_t*)xrtMalloc(pAnchor->iPrefixCount);
			if ( pAnchor->arrMeasuredPrefix != NULL )
				for ( i = 0; i < pAnchor->iPrefixCount; i++ )
					pAnchor->arrMeasuredPrefix[i] = (uint8_t)(
						pRenderer->blocks[i].ever_measured || pRenderer->blocks[i].object_dependent);
		}
	}
	pRenderer->freeze_dynamic_refresh = bFrozen;
}

static int __xuiMessageApplyDocumentAnchor(xui_widget pWidget,
	xui_message_list_data_t* pData, xui_rect_t tContent,
	xui_message_document_anchor_t* pAnchor, int* pbChanged)
{
	xui_message_node_data_t* pNode;
	xui_document_renderer pRenderer;
	xui_doc_rect_t tCaret, tSize;
	float fHeight, fScroll;
	int bExact, iRet;
	size_t i;
	if ( pAnchor->pBinding == NULL ) return XUI_OK;
	if ( pAnchor->iNode < 0 || pAnchor->iNode >= pData->iNodeCount ) return XUI_OK;
	pNode = &pData->arrNodes[pAnchor->iNode];
	if ( pNode->pDocumentBinding != pAnchor->pBinding ) return XUI_OK;
	pRenderer = pNode->pDocumentBinding->pRenderer;
	if ( pRenderer == NULL || pRenderer->snapshot == NULL ||
	     pAnchor->tPosition.iDocumentId != pRenderer->snapshot->identity ||
	     pAnchor->tPosition.iRevision != pRenderer->snapshot->revision ) return XUI_OK;
	if ( !pAnchor->bPrefixApplied ) {
		pAnchor->bPrefixApplied = 1;
		if ( pAnchor->arrMeasuredPrefix != NULL &&
		     pAnchor->iOldBlockCount == pRenderer->count ) {
			for ( i = 0; i < pAnchor->iPrefixCount; i++ ) {
				if ( !pAnchor->arrMeasuredPrefix[i] ) continue;
				iRet = xuiDocumentRendererLayout(pRenderer,
					__xuiMessageMax(1.0f, pNode->tTextRect.fW),
					__xuiMessageDocumentHeightBefore(pRenderer, i), 1.0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	iRet = xuiDocumentRendererGetCaretRect(pRenderer, &pAnchor->tPosition, &tCaret);
	if ( iRet != XUI_OK ) return XUI_OK;
	iRet = xuiDocumentRendererGetSize(pRenderer, &tSize, &bExact);
	if ( iRet != XUI_OK ) return iRet;
	if ( !isfinite(tSize.height) || tSize.height < 0 || tSize.height > 100000000.0 )
		return XUI_DOC_ERROR_LIMIT;
	pNode->bDocumentSizeExact = bExact;
	fHeight = __xuiMessageMax(20.0f, (float)tSize.height);
	if ( fabsf(pNode->tMeasuredText.fY - fHeight) > 0.01f ) {
		pNode->tMeasuredText.fY = fHeight;
		if ( pAnchor->iNode < pData->iLayoutDirtyFrom )
			pData->iLayoutDirtyFrom = pAnchor->iNode;
		iRet = __xuiMessageLayoutNodesForContent(pWidget, pData, tContent, 1);
		if ( iRet != XUI_OK ) return iRet;
		*pbChanged = 1;
	}
	fScroll = pNode->tTextRect.fY + (float)tCaret.y +
		pAnchor->fLineFraction * (float)tCaret.height - pAnchor->fScreenY;
	fScroll = __xuiMessageClamp(fScroll, 0.0f,
		__xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH));
	if ( fabsf(pData->fScrollY - fScroll) > 0.25f ) {
		pData->fScrollY = fScroll;
		*pbChanged = 1;
	}
	return XUI_OK;
}

static int __xuiMessageLayoutNodesForContent(xui_widget pWidget, xui_message_list_data_t* pData,
	xui_rect_t tContent, int bUpdateScroll)
{
	xui_message_node_data_t* pNode;
	float fY;
	float fConversationLaneX;
	float fConversationLaneW;
	float fTextMax;
	float fBubbleW;
	float fBubbleH;
	float fRowH;
	float fAuxIndent;
	float fHeaderH;
	float fMetaW;
	float fLineHeight;
	float fAnchorOffset = 0.0f;
	xui_vec2_t tTextSize;
	xui_vec2_t tTitleSize;
	xui_font pFont;
	uint32_t iLanguageRevision;
	int iStart;
	int iAnchor = -1;
	int iSelectable;
	int bAll;
	int bMeasure;
	int bPinEnd = 0;
	int iRet;
	int i;
	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pFont = __xuiMessageFont(pWidget, pData);
	iLanguageRevision = xuiGetLanguageRevision(xuiWidgetGetContext(pWidget));
	bAll = !pData->bLayoutValid || pData->fLayoutWidth != tContent.fW ||
		pData->pLayoutFont != pFont || pData->iLayoutLanguageRevision != iLanguageRevision ||
		pData->iLayoutDpiGeneration != pWidget->pContext->iDpiGeneration;
	iStart = bAll ? 0 : pData->iLayoutDirtyFrom;
	if ( iStart >= pData->iNodeCount && !bAll && pData->fLayoutHeight == tContent.fH ) return XUI_OK;
	if ( bUpdateScroll && pData->iLaidOutCount > 0 ) {
		float fOldEnd = __xuiMessageMax(0.0f, pData->fContentHeight - pData->fLayoutHeight);
		bPinEnd = pData->bAutoScroll && pData->fScrollY >= fOldEnd - 0.5f;
		iAnchor = __xuiMessageLowerBoundY(pData, pData->fScrollY);
		if ( iAnchor < pData->iLaidOutCount ) fAnchorOffset = pData->fScrollY - pData->arrNodes[iAnchor].tNodeRect.fY;
		else iAnchor = -1;
	}
	fConversationLaneX = pData->tMetrics.fPaddingX + pData->tMetrics.fAvatarSize + pData->tMetrics.fAvatarGap;
	fConversationLaneW = __xuiMessageMax(0.0f, tContent.fW - fConversationLaneX * 2.0f);
	fTextMax = __xuiMessageMax(1.0f, fConversationLaneW - pData->tMetrics.fBubblePaddingX * 2.0f);
	fLineHeight = iStart < pData->iNodeCount ? __xuiMessageLineHeight(xuiWidgetGetContext(pWidget), pFont) : 0.0f;
	/* Finish fallible measurements before replacing any prefix geometry. */
	for ( i = iStart; i < pData->iNodeCount; i++ ) {
		pNode = &pData->arrNodes[i];
		bMeasure = bAll || pNode->bMeasureDirty;
		if ( !bMeasure ) continue;
		XUI_MESSAGE_LIST_AUDIT_STEP(MeasureNode);
		if ( pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) {
			iRet = __xuiMessageMeasureNodeWrapped(pWidget, pData, pNode,
				__xuiMessageMax(1.0f, fConversationLaneW - pData->tMetrics.fSystemPaddingX * 2.0f), &pNode->tMeasuredText);
		} else if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY ) {
			fAuxIndent = (pNode->sParentId != NULL && pNode->sParentId[0] != 0) ? 12.0f : 0.0f;
			fBubbleW = __xuiMessageMax(0.0f, fConversationLaneW - fAuxIndent);
			iRet = __xuiMessageMeasureTitle(pWidget, pData, pNode, __xuiMessageAuxiliaryTitle(pWidget, pNode),
				__xuiMessageMax(16.0f, fBubbleW - pData->tMetrics.fBubblePaddingX * 2.0f - 20.0f), &pNode->tMeasuredTitle);
			if ( iRet != XUI_OK && (pFont != NULL || pNode->pDocumentBinding != NULL) ) return iRet;
			if ( (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) == 0 )
				iRet = __xuiMessageMeasureNodeWrapped(pWidget, pData, pNode,
					__xuiMessageMax(16.0f, fBubbleW - pData->tMetrics.fBubblePaddingX * 2.0f), &pNode->tMeasuredText);
		} else {
			iRet = __xuiMessageMeasureNodeWrapped(pWidget, pData, pNode, fTextMax, &pNode->tMeasuredText);
			pNode->fMetaWidth = __xuiMessageTextWidth(pWidget, pFont, pNode->sSender);
		}
		if ( iRet != XUI_OK && (pFont != NULL || pNode->pDocumentBinding != NULL) ) return iRet;
	}
	fY = iStart > 0 ? pData->arrNodes[iStart - 1].fNextY : pData->tMetrics.fPaddingY;
	iSelectable = iStart > 0 ? pData->arrNodes[iStart - 1].iSelectablePrefix : 0;
	for ( i = iStart; i < pData->iNodeCount; i++ ) {
		XUI_MESSAGE_LIST_AUDIT_STEP(LayoutNode);
		pNode = &pData->arrNodes[i];
		memset(&pNode->tNodeRect, 0, sizeof(pNode->tNodeRect));
		memset(&pNode->tBubbleRect, 0, sizeof(pNode->tBubbleRect));
		memset(&pNode->tHeaderRect, 0, sizeof(pNode->tHeaderRect));
		memset(&pNode->tTextRect, 0, sizeof(pNode->tTextRect));
		if ( pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) {
			tTextSize = pNode->tMeasuredText;
			fBubbleW = __xuiMessageMin(fConversationLaneW, __xuiMessageMax(56.0f, tTextSize.fX + pData->tMetrics.fSystemPaddingX * 2.0f));
			fBubbleH = __xuiMessageMax(fLineHeight, tTextSize.fY) + pData->tMetrics.fSystemPaddingY * 2.0f;
			pNode->tNodeRect = (xui_rect_t){0.0f, fY, tContent.fW, fBubbleH};
			pNode->tBubbleRect = (xui_rect_t){(tContent.fW - fBubbleW) * 0.5f, fY, fBubbleW, fBubbleH};
			pNode->tTextRect = (xui_rect_t){pNode->tBubbleRect.fX + pData->tMetrics.fSystemPaddingX, fY + pData->tMetrics.fSystemPaddingY, __xuiMessageMax(0.0f, fBubbleW - pData->tMetrics.fSystemPaddingX * 2.0f), fBubbleH - pData->tMetrics.fSystemPaddingY * 2.0f};
			fY += fBubbleH + pData->tMetrics.fNodeGap;
			goto node_done;
		}
		if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY ) {
			fAuxIndent = (pNode->sParentId != NULL && pNode->sParentId[0] != 0) ? 12.0f : 0.0f;
			fBubbleW = __xuiMessageMax(0.0f, fConversationLaneW - fAuxIndent);
			fHeaderH = fLineHeight + 10.0f;
			tTitleSize = pNode->tMeasuredTitle;
			fHeaderH = __xuiMessageMax(fHeaderH, tTitleSize.fY + 8.0f);
			memset(&tTextSize, 0, sizeof(tTextSize));
			if ( (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) == 0 ) {
				tTextSize = pNode->tMeasuredText;
			}
			fBubbleH = fHeaderH + ((pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) ? 0.0f : (tTextSize.fY + pData->tMetrics.fBubblePaddingY * 2.0f));
			pNode->tNodeRect = (xui_rect_t){0.0f, fY, tContent.fW, fBubbleH};
			pNode->tBubbleRect = (xui_rect_t){fConversationLaneX + fAuxIndent, fY, fBubbleW, fBubbleH};
			pNode->tHeaderRect = (xui_rect_t){pNode->tBubbleRect.fX, fY, fBubbleW, fHeaderH};
			pNode->tTextRect = (xui_rect_t){pNode->tBubbleRect.fX + pData->tMetrics.fBubblePaddingX, fY + fHeaderH + pData->tMetrics.fBubblePaddingY, __xuiMessageMax(0.0f, fBubbleW - pData->tMetrics.fBubblePaddingX * 2.0f), tTextSize.fY};
			fY += fBubbleH + pData->tMetrics.fNodeGap;
			goto node_done;
		}
		tTextSize = pNode->tMeasuredText;
		fMetaW = pNode->fMetaWidth;
		fBubbleW = __xuiMessageMin(fConversationLaneW,
			__xuiMessageMax(48.0f, __xuiMessageMax(tTextSize.fX + pData->tMetrics.fBubblePaddingX * 2.0f, fMetaW)));
		fBubbleH = __xuiMessageMax(pData->tMetrics.fMinBubbleHeight, tTextSize.fY + pData->tMetrics.fBubblePaddingY * 2.0f);
		fRowH = __xuiMessageMax(pData->tMetrics.fAvatarSize, pData->tMetrics.fMetaHeight + fBubbleH);
		pNode->tNodeRect = (xui_rect_t){0.0f, fY, tContent.fW, fRowH};
		if ( pNode->iType == XUI_MESSAGE_NODE_SELF ) {
			pNode->tBubbleRect = (xui_rect_t){fConversationLaneX + fConversationLaneW - fBubbleW, fY + pData->tMetrics.fMetaHeight, fBubbleW, fBubbleH};
		} else {
			pNode->tBubbleRect = (xui_rect_t){fConversationLaneX, fY + pData->tMetrics.fMetaHeight, fBubbleW, fBubbleH};
		}
		pNode->tTextRect = (xui_rect_t){pNode->tBubbleRect.fX + pData->tMetrics.fBubblePaddingX, pNode->tBubbleRect.fY + pData->tMetrics.fBubblePaddingY, __xuiMessageMax(0.0f, fBubbleW - pData->tMetrics.fBubblePaddingX * 2.0f), fBubbleH - pData->tMetrics.fBubblePaddingY * 2.0f};
		fY += fRowH + pData->tMetrics.fNodeGap;
	node_done:
		pNode->fNextY = fY;
		iSelectable += __xuiMessageNodeCanSelect(pNode) ? 1 : 0;
		pNode->iSelectablePrefix = iSelectable;
		pNode->bMeasureDirty = 0;
	}
	pData->fContentHeight = __xuiMessageMax(0.0f, fY - pData->tMetrics.fNodeGap + pData->tMetrics.fPaddingY);
	if ( bUpdateScroll ) {
		if ( bPinEnd ) pData->fScrollY = __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH);
		else if ( iAnchor >= 0 && iAnchor < pData->iNodeCount ) pData->fScrollY = pData->arrNodes[iAnchor].tNodeRect.fY + fAnchorOffset;
		pData->fScrollY = __xuiMessageClamp(pData->fScrollY, 0.0f, __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH));
	}
	pData->bLayoutValid = 1;
	pData->iLayoutDirtyFrom = pData->iNodeCount;
	pData->iLaidOutCount = pData->iNodeCount;
	pData->fLayoutWidth = tContent.fW;
	pData->fLayoutHeight = tContent.fH;
	pData->pLayoutFont = pFont;
	pData->iLayoutLanguageRevision = iLanguageRevision;
	pData->iLayoutDpiGeneration = pWidget->pContext->iDpiGeneration;
	return XUI_OK;
}

static int __xuiMessageLayoutNodes(xui_widget pWidget, xui_message_list_data_t* pData)
{
	xui_rect_t tContent = xuiWidgetGetContentRect(pWidget);
	xui_doc_rect_t tSize;
	xui_message_document_anchor_t tAnchor;
	xui_message_node_data_t* pNode;
	float fHeight;
	int iRet, i, iFirst, iDirty, iPass, bExact, bChanged = 0, bAnchorChanged;
	if ( pData->tPendingDocumentAnchor.pBinding != NULL ) {
		tAnchor = pData->tPendingDocumentAnchor;
		memset(&pData->tPendingDocumentAnchor, 0, sizeof(pData->tPendingDocumentAnchor));
	} else __xuiMessageCaptureDocumentAnchor(pWidget, pData, tContent, &tAnchor, 0);
	iRet = __xuiMessageLayoutNodesForContent(pWidget, pData, tContent, 1);
	if ( iRet != XUI_OK ) goto cleanup;
	if ( pData->iDocumentNodeCount == 0 ) goto cleanup;
	for ( iPass = 0; iPass < 8; iPass++ ) {
		bAnchorChanged = 0;
		iRet = __xuiMessageApplyDocumentAnchor(pWidget, pData, tContent,
			&tAnchor, &bAnchorChanged);
		if ( iRet != XUI_OK ) goto cleanup;
		if ( bAnchorChanged ) bChanged = 1;
		iDirty = pData->iNodeCount;
		iFirst = __xuiMessageLowerBoundY(pData, pData->fScrollY);
		for ( i = iFirst; i < pData->iNodeCount; i++ ) {
			pNode = &pData->arrNodes[i];
			if ( pNode->tNodeRect.fY > pData->fScrollY + tContent.fH ) break;
			if ( pNode->pDocumentBinding == NULL ||
			     (pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
			      (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0) ) continue;
			iRet = __xuiMessageDocumentSync(pNode->pDocumentBinding);
			if ( iRet != XUI_OK ) goto cleanup;
			if ( pNode->bDocumentSizeExact ) {
				iRet = xuiDocumentRendererGetSize(pNode->pDocumentBinding->pRenderer,
					&tSize, &bExact);
				if ( iRet != XUI_OK ) goto cleanup;
				if ( bExact ) continue;
				pNode->bDocumentSizeExact = 0;
			}
			iRet = xuiDocumentRendererLayout(pNode->pDocumentBinding->pRenderer,
				__xuiMessageMax(1.0f, pNode->tTextRect.fW),
				__xuiMessageMax(0.0f, pData->fScrollY - pNode->tTextRect.fY),
				__xuiMessageMax(0.0f, tContent.fH));
			if ( iRet == XUI_OK ) iRet = xuiDocumentRendererGetSize(
				pNode->pDocumentBinding->pRenderer, &tSize, &bExact);
			if ( iRet != XUI_OK ) goto cleanup;
			if ( !isfinite(tSize.height) || tSize.height < 0 || tSize.height > 100000000.0 )
				{ iRet = XUI_DOC_ERROR_LIMIT; goto cleanup; }
			pNode->bDocumentSizeExact = bExact;
			fHeight = __xuiMessageMax(20.0f, (float)tSize.height);
			if ( fabsf(pNode->tMeasuredText.fY - fHeight) > 0.01f ) {
				pNode->tMeasuredText.fY = fHeight;
				if ( i < iDirty ) iDirty = i;
			}
		}
		if ( iDirty == pData->iNodeCount ) {
			if ( !bAnchorChanged ) break;
			continue;
		}
		if ( iDirty < pData->iLayoutDirtyFrom ) pData->iLayoutDirtyFrom = iDirty;
		iRet = __xuiMessageLayoutNodesForContent(pWidget, pData, tContent, 1);
		if ( iRet != XUI_OK ) goto cleanup;
		bChanged = 1;
	}
	if ( bChanged ) {
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
cleanup:
	if ( iRet == XUI_OK )
		pData->iLayoutResourceGeneration = xuiResourceGetRegistryGeneration(xuiWidgetGetContext(pWidget));
	xrtFree(tAnchor.arrMeasuredPrefix);
	return iRet;
}

static int __xuiMessageInvalidate(xui_widget pWidget, xui_message_list_data_t* pData)
{
	int iRet;
	int iPaintRet;
	if ( pData != NULL ) pData->iChangeCount++;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	iPaintRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return iRet != XUI_OK ? iRet : iPaintRet;
}

static void __xuiMessageAccessibleFoldChanged(xui_widget pWidget,
	const xui_message_node_data_t* pNode)
{
	if ( pNode->pDocumentBinding != NULL )
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
}

static int __xuiMessageDocumentSelectionRange(const xui_message_list_data_t* pData,
	int iNode, xui_doc_range_t* pRange);
static int __xuiMessageGetTextSelectionForNode(const xui_message_list_data_t* pData,
	int iNode, int* pStart, int* pEnd);
static int __xuiMessageNodeCanSelect(const xui_message_node_data_t* pNode);
static int __xuiMessageNodeCanSelectText(const xui_message_node_data_t* pNode);
static void __xuiMessageSetTextSelection(xui_message_list_data_t* pData,
	int iAnchorNode, int iAnchorOffset, int iActiveNode, int iActiveOffset);
static int __xuiMessageAccessibleSelection(xui_message_list_data_t* pData,
	int iNode, xui_accessible_node_t* pResult);

typedef struct xui_message_accessible_id_map_t {
	xui_doc_node_id iDocumentNodeId;
	uint64_t iAccessibleId;
} xui_message_accessible_id_map_t;

static int __xuiMessageAccessibleIdMapCompare(const void* pA, const void* pB)
{
	const xui_message_accessible_id_map_t* pLeft = (const xui_message_accessible_id_map_t*)pA;
	const xui_message_accessible_id_map_t* pRight = (const xui_message_accessible_id_map_t*)pB;
	return pLeft->iDocumentNodeId < pRight->iDocumentNodeId ? -1 :
		pLeft->iDocumentNodeId > pRight->iDocumentNodeId ? 1 : 0;
}

static uint64_t __xuiMessageAccessibleIdMapFind(const xui_message_accessible_id_map_t* arrMap,
	uint64_t iCount, xui_doc_node_id iDocumentNodeId)
{
	uint64_t iLow = 0, iHigh = iCount;
	while ( iLow < iHigh ) {
		uint64_t iMid = iLow + (iHigh - iLow) / 2;
		if ( arrMap[iMid].iDocumentNodeId < iDocumentNodeId ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	return iLow < iCount && arrMap[iLow].iDocumentNodeId == iDocumentNodeId ?
		arrMap[iLow].iAccessibleId : 0;
}

static int __xuiMessageAccessibleBuildDocument(xui_message_list_data_t* pData,
	xui_message_node_data_t* pNode)
{
	xui_message_document_binding_t* pBinding = pNode->pDocumentBinding;
	xui_document_snapshot pSnapshot = NULL;
	xui_doc_node_id* arrDocumentIds = NULL;
	xui_message_accessible_document_node_t* arrNodes = NULL;
	xui_message_accessible_id_map_t* arrOld = NULL;
	xui_message_accessible_id_map_t* arrNew = NULL;
	uint64_t iIdentity, iRevision, iCount = 0, iNext, i;
	int iRet;
	if ( pBinding == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	iIdentity = xuiDocumentSnapshotGetIdentity(pSnapshot);
	iRevision = xuiDocumentSnapshotGetRevision(pSnapshot);
	if ( pBinding->arrAccessibleNodes != NULL &&
	     pBinding->iAccessibleIdentity == iIdentity &&
	     pBinding->iAccessibleRevision == iRevision ) {
		xuiDocumentSnapshotRelease(pSnapshot);
		return XUI_OK;
	}
	iRet = doc_accessible_snapshot_ids(pSnapshot, &arrDocumentIds, &iCount);
	if ( iRet != XUI_OK ) goto done;
	if ( iCount > SIZE_MAX / sizeof(*arrNodes) ||
	     iCount > SIZE_MAX / sizeof(*arrNew) ||
	     pBinding->iAccessibleNodeCount > SIZE_MAX / sizeof(*arrOld) ) {
		iRet = XUI_DOC_ERROR_LIMIT; goto done;
	}
	arrNodes = calloc((size_t)iCount, sizeof(*arrNodes));
	arrNew = malloc((size_t)iCount * sizeof(*arrNew));
	if ( !arrNodes || !arrNew ) { iRet = XUI_ERROR_OUT_OF_MEMORY; goto done; }
	if ( pBinding->iAccessibleIdentity == iIdentity && pBinding->iAccessibleNodeCount ) {
		arrOld = malloc((size_t)pBinding->iAccessibleNodeCount * sizeof(*arrOld));
		if ( !arrOld ) { iRet = XUI_ERROR_OUT_OF_MEMORY; goto done; }
		for ( i = 0; i < pBinding->iAccessibleNodeCount; i++ ) {
			arrOld[i].iDocumentNodeId = pBinding->arrAccessibleNodes[i].iDocumentNodeId;
			arrOld[i].iAccessibleId = pBinding->arrAccessibleNodes[i].iAccessibleId;
		}
		qsort(arrOld, (size_t)pBinding->iAccessibleNodeCount,
			sizeof(*arrOld), __xuiMessageAccessibleIdMapCompare);
	}
	iNext = pData->iNextAccessibleId;
	for ( i = 0; i < iCount; i++ ) {
		uint64_t iAccessibleId = arrOld ? __xuiMessageAccessibleIdMapFind(arrOld,
			pBinding->iAccessibleNodeCount, arrDocumentIds[i]) : 0;
		if ( iAccessibleId == 0 ) {
			if ( iNext == UINT64_MAX ) { iRet = XUI_DOC_ERROR_LIMIT; goto done; }
			iAccessibleId = ++iNext;
		}
		arrNodes[i].iDocumentNodeId = arrDocumentIds[i];
		arrNodes[i].iAccessibleId = iAccessibleId;
		arrNew[i] = (xui_message_accessible_id_map_t){arrDocumentIds[i], iAccessibleId};
	}
	qsort(arrNew, (size_t)iCount, sizeof(*arrNew), __xuiMessageAccessibleIdMapCompare);
	for ( i = 0; i < iCount; i++ ) {
		doc_node* pDocumentNode = doc_index_get(pSnapshot->state->index, arrDocumentIds[i]);
		if ( pDocumentNode == NULL ) { iRet = XUI_DOC_ERROR_SCHEMA; goto done; }
		arrNodes[i].iParentAccessibleId = pDocumentNode->id == DOC_ROOT ?
			pNode->iAccessibleId : __xuiMessageAccessibleIdMapFind(arrNew,
				iCount, pDocumentNode->parent);
		if ( arrNodes[i].iParentAccessibleId == 0 ) { iRet = XUI_DOC_ERROR_SCHEMA; goto done; }
	}
	__xuiMessageAccessibleFreeDocumentNodes(pBinding);
	pBinding->arrAccessibleNodes = arrNodes;
	pBinding->iAccessibleNodeCount = iCount;
	pBinding->iAccessibleIdentity = iIdentity;
	pBinding->iAccessibleRevision = iRevision;
	pData->iNextAccessibleId = iNext;
	arrNodes = NULL;
done:
	free(arrDocumentIds); free(arrOld); free(arrNew); free(arrNodes);
	xuiDocumentSnapshotRelease(pSnapshot);
	return iRet;
}

static int __xuiMessageAccessibleCount(xui_widget pWidget, void* pUser)
{
	xui_message_list_data_t* pData = (xui_message_list_data_t*)pUser;
	uint64_t iCount = 1;
	int i;
	(void)pWidget;
	if ( pData == NULL ) return 0;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		xui_message_node_data_t* pNode = &pData->arrNodes[i];
		if ( ++iCount > INT_MAX ) return 0;
		if ( pNode->pDocumentBinding != NULL &&
		     !(pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
		       (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED)) ) {
			if ( __xuiMessageAccessibleBuildDocument(pData, pNode) != XUI_OK ||
			     pNode->pDocumentBinding->iAccessibleNodeCount > INT_MAX - iCount ) return 0;
			iCount += pNode->pDocumentBinding->iAccessibleNodeCount;
		}
	}
	return (int)iCount;
}

static int __xuiMessageAccessibleLocate(xui_message_list_data_t* pData,
	int iIndex, int* pMessageIndex, uint64_t* pDocumentIndex)
{
	uint64_t iCursor = 1;
	int i;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		xui_message_node_data_t* pNode = &pData->arrNodes[i];
		if ( iCursor++ == (uint64_t)iIndex ) {
			*pMessageIndex = i; *pDocumentIndex = UINT64_MAX;
			return XUI_OK;
		}
		if ( pNode->pDocumentBinding != NULL &&
		     !(pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
		       (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED)) ) {
			int iRet = __xuiMessageAccessibleBuildDocument(pData, pNode);
			uint64_t iCount;
			if ( iRet != XUI_OK ) return iRet;
			iCount = pNode->pDocumentBinding->iAccessibleNodeCount;
			if ( (uint64_t)iIndex >= iCursor && (uint64_t)iIndex - iCursor < iCount ) {
				*pMessageIndex = i; *pDocumentIndex = (uint64_t)iIndex - iCursor;
				return XUI_OK;
			}
			iCursor += iCount;
		}
	}
	return XUI_ERROR_NOT_FOUND;
}

static int __xuiMessageAccessibleBody(xui_message_node_data_t* pNode, const char** psValue)
{
	xui_document_snapshot pSnapshot = NULL;
	xui_document pDocument;
	char* sText = NULL;
	uint64_t iBytes = 0, iIdentity, iRevision;
	int iRet;
	if ( pNode->pDocumentBinding == NULL ) {
		*psValue = __xuiMessageText(pNode->sText);
		return XUI_OK;
	}
	pDocument = pNode->pDocumentBinding->pDocument;
	iIdentity = xuiDocumentGetIdentity(pDocument);
	iRevision = xuiDocumentGetRevision(pDocument);
	if ( pNode->sAccessibleDocumentText != NULL &&
	     pNode->iAccessibleDocumentId == iIdentity &&
	     pNode->iAccessibleDocumentRevision == iRevision ) {
		*psValue = pNode->sAccessibleDocumentText;
		return XUI_OK;
	}
	iRet = xuiDocumentAcquireSnapshot(pDocument, &pSnapshot);
	if ( iRet == XUI_OK ) iRet = xuiDocumentSnapshotCopyPlainText(pSnapshot, &sText, &iBytes);
	if ( pSnapshot != NULL ) {
		iIdentity = xuiDocumentSnapshotGetIdentity(pSnapshot);
		iRevision = xuiDocumentSnapshotGetRevision(pSnapshot);
		xuiDocumentSnapshotRelease(pSnapshot);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( memchr(sText, 0, (size_t)iBytes) != NULL ) {
		xuiDocumentFreeBuffer(sText);
		return XUI_DOC_ERROR_UNREPRESENTABLE;
	}
	__xuiMessageAccessibleClearText(pNode);
	pNode->sAccessibleDocumentText = sText;
	pNode->iAccessibleDocumentId = iIdentity;
	pNode->iAccessibleDocumentRevision = iRevision;
	*psValue = sText;
	return XUI_OK;
}

static int __xuiMessageAccessibleGetDocument(xui_widget pWidget,
	xui_message_list_data_t* pData, int iMessage, uint64_t iDocumentIndex,
	xui_accessible_node_t* pResult)
{
	xui_message_node_data_t* pMessage = &pData->arrNodes[iMessage];
	xui_message_document_binding_t* pBinding = pMessage->pDocumentBinding;
	xui_message_accessible_document_node_t* pEntry;
	xui_document_snapshot pSnapshot = NULL;
	doc_node* pNode;
	xui_doc_rect_t tNodeRect = {0};
	xui_rect_t tWorld, tContent;
	int bHasBounds = 0, iRet;
	if ( pBinding == NULL || iDocumentIndex >= pBinding->iAccessibleNodeCount )
		return XUI_ERROR_NOT_FOUND;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	if ( pBinding->iAccessibleIdentity != pSnapshot->identity ||
	     pBinding->iAccessibleRevision != pSnapshot->revision ) {
		xuiDocumentSnapshotRelease(pSnapshot);
		return XUI_DOC_ERROR_STALE;
	}
	pEntry = &pBinding->arrAccessibleNodes[iDocumentIndex];
	pNode = doc_index_get(pSnapshot->state->index, pEntry->iDocumentNodeId);
	if ( pNode == NULL ) { xuiDocumentSnapshotRelease(pSnapshot); return XUI_DOC_ERROR_STALE; }
	pResult->iId = pEntry->iAccessibleId;
	pResult->iParentId = pEntry->iParentAccessibleId;
	pResult->iRole = doc_accessible_role(pNode);
	pResult->iState = XUI_ACCESSIBLE_STATE_READONLY;
	pResult->sName = "";
	pResult->sDescription = doc_string(pNode->title);
	if ( pNode->id == DOC_ROOT ) {
		pResult->sName = "Document";
		iRet = __xuiMessageAccessibleBody(pMessage, &pResult->sValue);
	} else {
		if ( pEntry->sValue == NULL ) {
			uint64_t iBytes = 0;
			iRet = doc_accessible_snapshot_value(pSnapshot, pNode,
				&pEntry->sValue, &iBytes);
		}
		if ( iRet == XUI_OK ) {
			if ( pNode->kind == XUI_DOC_IMAGE ) {
				pResult->sName = pEntry->sValue;
				if ( pNode->attrs->iMarks & XUI_DOC_LINK )
					pResult->sDescription = doc_string(pNode->link_target);
			}
			else if ( pNode->kind == XUI_DOC_TEXT &&
			         (pNode->attrs->iMarks & XUI_DOC_LINK) ) {
				pResult->sName = pEntry->sValue;
				pResult->sDescription = doc_string(pNode->resource);
				pResult->sValue = pEntry->sValue;
			} else if ( pNode->kind == XUI_DOC_PARAGRAPH ||
			            pNode->kind == XUI_DOC_HEADING ||
			            pNode->kind == XUI_DOC_TEXT ||
			            pNode->kind == XUI_DOC_CODE_BLOCK ||
			            pNode->kind == XUI_DOC_FOOTNOTE_REF ||
			            pNode->kind == XUI_DOC_LIST ||
			            pNode->kind == XUI_DOC_LIST_ITEM ||
			            pNode->kind == XUI_DOC_QUOTE ||
			            pNode->kind == XUI_DOC_FOOTNOTE ||
			            pNode->kind == XUI_DOC_TABLE ||
			            pNode->kind == XUI_DOC_ROW ||
			            pNode->kind == XUI_DOC_CELL ||
			            pNode->kind == XUI_DOC_MATH ||
			            pNode->kind == XUI_DOC_DIAGRAM ||
			            pNode->kind == XUI_DOC_HTML ||
			            pNode->kind == XUI_DOC_EXTENSION )
				pResult->sValue = pEntry->sValue;
		}
	}
	if ( iRet != XUI_OK ) { xuiDocumentSnapshotRelease(pSnapshot); return iRet; }
	if ( pNode->kind == XUI_DOC_HEADING ) pResult->iLevel = (int)pNode->attrs->iHeadingLevel;
	if ( pNode->kind == XUI_DOC_LIST_ITEM && (pNode->attrs->iFlags & XUI_DOC_TASK) &&
	     (pNode->attrs->iFlags & XUI_DOC_CHECKED) )
		pResult->iState |= XUI_ACCESSIBLE_STATE_CHECKED;
	if ( pNode->kind == XUI_DOC_LIST_ITEM && (pNode->attrs->iFlags & XUI_DOC_TASK) &&
	     pBinding->tDesc.onTaskToggle != NULL ) {
		pResult->iState &= ~XUI_ACCESSIBLE_STATE_READONLY;
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE);
	}
	if ( pNode->kind == XUI_DOC_TABLE ) {
		doc_node* pFirstRow = NULL;
		uint64_t iRows = doc_seq_size(pNode->children), iColumns = 0, i;
		pResult->iRowCount = iRows > INT_MAX ? INT_MAX : (int)iRows;
		if ( iRows ) pFirstRow = doc_index_get(pSnapshot->state->index,
			doc_seq_get_id(pNode->children, 0));
		if ( pFirstRow != NULL ) for ( i = 0; i < doc_seq_size(pFirstRow->children); i++ ) {
			doc_node* pCell = doc_index_get(pSnapshot->state->index,
				doc_seq_get_id(pFirstRow->children, i));
			if ( pCell != NULL ) iColumns += pCell->attrs->iColumnSpan ? pCell->attrs->iColumnSpan : 1;
		}
		pResult->iColumnCount = iColumns > INT_MAX ? INT_MAX : (int)iColumns;
	}
	if ( pNode->kind == XUI_DOC_CELL ) {
		doc_table_cell_slot tSlot;
		iRet = doc_table_locate_cell(pSnapshot->state, pNode->id, &tSlot);
		if ( iRet != XUI_OK ) { xuiDocumentSnapshotRelease(pSnapshot); return iRet; }
		pResult->iRow = tSlot.row > INT_MAX ? INT_MAX : (int)tSlot.row;
		pResult->iColumn = tSlot.column > INT_MAX ? INT_MAX : (int)tSlot.column;
		pResult->iRowCount = tSlot.row_span > INT_MAX ? INT_MAX : (int)tSlot.row_span;
		pResult->iColumnCount = tSlot.column_span > INT_MAX ? INT_MAX : (int)tSlot.column_span;
		if ( pData->iTableSelectionNode == iMessage &&
		     pData->tTableSelection.iTableId == tSlot.table &&
		     tSlot.row >= pData->tTableSelection.iRow &&
		     tSlot.column >= pData->tTableSelection.iColumn &&
		     (uint64_t)tSlot.row + tSlot.row_span <=
			(uint64_t)pData->tTableSelection.iRow + pData->tTableSelection.iRows &&
		     (uint64_t)tSlot.column + tSlot.column_span <=
			(uint64_t)pData->tTableSelection.iColumn + pData->tTableSelection.iColumns)
			pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
	}
	if ( pNode->id == DOC_ROOT ) {
		pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
		iRet = __xuiMessageAccessibleSelection(pData, iMessage, pResult);
		if ( iRet != XUI_OK ) { xuiDocumentSnapshotRelease(pSnapshot); return iRet; }
	} else if ( __xuiMessageNodeCanSelect(pMessage) &&
	     doc_accessible_text_selectable_kind(pNode->kind) ) {
		xui_doc_range_t tRange;
		pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
		iRet = __xuiMessageDocumentSelectionRange(pData, iMessage, &tRange);
		if ( iRet == XUI_OK ) {
			int bSelected = 0;
			iRet = doc_accessible_snapshot_selection_offsets(pSnapshot, pNode,
				&tRange, &pResult->iTextStart, &pResult->iTextEnd, &bSelected);
			if ( iRet != XUI_OK ) { xuiDocumentSnapshotRelease(pSnapshot); return iRet; }
			if ( bSelected ) pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		} else if ( iRet != XUI_ERROR_NOT_FOUND ) {
			xuiDocumentSnapshotRelease(pSnapshot); return iRet;
		}
	} else if ( __xuiMessageNodeCanSelect(pMessage) &&
	     doc_selectable_object_kind(pNode->kind) ) {
		xui_doc_range_t tRange;
		int bSelected = 0;
		pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
		iRet = __xuiMessageDocumentSelectionRange(pData, iMessage, &tRange);
		if ( iRet == XUI_OK ) {
			iRet = doc_accessible_snapshot_object_selected(pSnapshot,
				pNode, &tRange, &bSelected);
			if ( iRet != XUI_OK ) { xuiDocumentSnapshotRelease(pSnapshot); return iRet; }
			if ( bSelected ) pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		} else if ( iRet != XUI_ERROR_NOT_FOUND ) {
			xuiDocumentSnapshotRelease(pSnapshot); return iRet;
		}
	}
	if ( pNode->id == DOC_ROOT ) {
		tNodeRect = (xui_doc_rect_t){0, 0,
			pMessage->tTextRect.fW, pMessage->tTextRect.fH};
		bHasBounds = 1;
	} else {
		iRet = __xuiMessageDocumentSync(pBinding);
		if ( iRet == XUI_OK ) {
			if ( pNode->kind == XUI_DOC_CELL ) {
				xui_doc_cell_hit_t tCell = {0};
				tCell.iSize = sizeof(tCell);
				iRet = xuiDocumentRendererGetCellRect(pBinding->pRenderer,
					pNode->id, &tCell);
				if ( iRet == XUI_OK ) tNodeRect = tCell.tBounds;
			} else iRet = xuiDocumentRendererGetNodeRect(pBinding->pRenderer,
				pNode->id, &tNodeRect);
		}
		if ( iRet == XUI_OK ) bHasBounds = 1;
		else if ( iRet != XUI_ERROR_NOT_FOUND ) {
			xuiDocumentSnapshotRelease(pSnapshot); return iRet;
		}
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tContent = xuiWidgetGetContentRect(pWidget);
	if ( bHasBounds ) {
		pResult->tBounds = (xui_rect_t){
			tWorld.fX + tContent.fX + pMessage->tTextRect.fX + (float)tNodeRect.x,
			tWorld.fY + tContent.fY + pMessage->tTextRect.fY +
				(float)tNodeRect.y - pData->fScrollY,
			(float)tNodeRect.width, (float)tNodeRect.height};
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW);
		if ( pResult->tBounds.fX + pResult->tBounds.fW <= tWorld.fX + tContent.fX ||
		     pResult->tBounds.fX >= tWorld.fX + tContent.fX + tContent.fW ||
		     pResult->tBounds.fY + pResult->tBounds.fH <= tWorld.fY + tContent.fY ||
		     pResult->tBounds.fY >= tWorld.fY + tContent.fY + tContent.fH )
			pResult->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	} else pResult->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	if ( pBinding->tDesc.onActivate && (pNode->kind == XUI_DOC_IMAGE ||
	     (pNode->kind == XUI_DOC_TEXT && (pNode->attrs->iMarks & XUI_DOC_LINK)) ||
	     pNode->kind == XUI_DOC_MATH || pNode->kind == XUI_DOC_DIAGRAM ||
	     pNode->kind == XUI_DOC_HTML) )
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
	xuiDocumentSnapshotRelease(pSnapshot);
	return XUI_OK;
}

static int __xuiMessageAccessibleSelection(xui_message_list_data_t* pData,
	int iNode, xui_accessible_node_t* pResult)
{
	xui_message_node_data_t* pNode = &pData->arrNodes[iNode];
	uint64_t iAnchor, iCaret;
	int iStart, iEnd, iRet;
	if ( !__xuiMessageNodeCanSelect(pNode) ) return XUI_OK;
	if ( pNode->pDocumentBinding != NULL ) {
		xui_doc_range_t tRange;
		xui_document_snapshot pSnapshot = NULL;
		doc_plain_projection tProjection = {0};
		iRet = __xuiMessageDocumentSelectionRange(pData, iNode, &tRange);
		if ( iRet == XUI_ERROR_NOT_FOUND ) return XUI_OK;
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDocumentAcquireSnapshot(pNode->pDocumentBinding->pDocument, &pSnapshot);
		if ( iRet == XUI_OK ) iRet = doc_plain_project(pSnapshot, XUI_DOC_SEMANTIC, &tProjection);
		if ( iRet == XUI_OK && tProjection.bytes > INT_MAX ) iRet = XUI_DOC_ERROR_LIMIT;
		if ( iRet == XUI_OK ) iRet = doc_plain_project_position(&tProjection, &tRange.tAnchor, &iAnchor);
		if ( iRet == XUI_OK ) iRet = doc_plain_project_position(&tProjection, &tRange.tCaret, &iCaret);
		if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
		if ( iRet == XUI_OK ) {
			if ( pData->iSelectionAnchorNode > pData->iSelectionActiveNode ) {
				uint64_t iSwap = iAnchor; iAnchor = iCaret; iCaret = iSwap;
			}
			pResult->iTextStart = (int)iAnchor;
			pResult->iTextEnd = (int)iCaret;
			if ( iAnchor != iCaret ) pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		}
		doc_plain_projection_free(&tProjection);
		return iRet;
	}
	if ( !__xuiMessageNodeCanSelectText(pNode) ) return XUI_OK;
	if ( strlen(__xuiMessageText(pNode->sText)) > INT_MAX ) return XUI_DOC_ERROR_LIMIT;
	if ( __xuiMessageGetTextSelectionForNode(pData, iNode, &iStart, &iEnd) ) {
		if ( pData->iSelectionAnchorNode > pData->iSelectionActiveNode ||
		     (pData->iSelectionAnchorNode == pData->iSelectionActiveNode &&
		      pData->iSelectionAnchorOffset > pData->iSelectionActiveOffset) ) {
			pResult->iTextStart = iEnd;
			pResult->iTextEnd = iStart;
		} else {
			pResult->iTextStart = iStart;
			pResult->iTextEnd = iEnd;
		}
		pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
	} else if ( pData->iSelectionAnchorNode == iNode &&
	            pData->iSelectionActiveNode == iNode &&
	            pData->iSelectionAnchorOffset == pData->iSelectionActiveOffset )
		pResult->iTextStart = pResult->iTextEnd = pData->iSelectionAnchorOffset;
	return XUI_OK;
}

static int __xuiMessageAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pResult, void* pUser)
{
	xui_message_list_data_t* pData = (xui_message_list_data_t*)pUser;
	xui_message_node_data_t* pNode;
	xui_rect_t tWorld, tContent, tRect;
	uint64_t iDocumentIndex;
	int iMessage, iRet;
	if ( pData == NULL || pResult == NULL || iIndex < 0 )
		return XUI_ERROR_NOT_FOUND;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	if ( iIndex == 0 ) {
		pResult->iId = 1;
		pResult->iRole = XUI_ACCESSIBLE_ROLE_LIST;
		pResult->sName = xuiWidgetGetAccessibleName(pWidget) ?
			xuiWidgetGetAccessibleName(pWidget) : "Messages";
		pResult->tBounds = tWorld;
		pResult->iState = XUI_ACCESSIBLE_STATE_READONLY;
		return XUI_OK;
	}
	iRet = __xuiMessageAccessibleLocate(pData, iIndex, &iMessage, &iDocumentIndex);
	if ( iRet != XUI_OK ) return iRet;
	if ( iDocumentIndex != UINT64_MAX )
		return __xuiMessageAccessibleGetDocument(pWidget, pData,
			iMessage, iDocumentIndex, pResult);
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pNode = &pData->arrNodes[iMessage];
	if ( pNode->iAccessibleId < 2 ) return XUI_ERROR_INVALID_STATE;
	pResult->iId = pNode->iAccessibleId;
	pResult->iParentId = 1;
	pResult->iRole = pNode->iType == XUI_MESSAGE_NODE_AUXILIARY ?
		XUI_ACCESSIBLE_ROLE_GROUP : XUI_ACCESSIBLE_ROLE_LIST_ITEM;
	pResult->sName = pNode->iType == XUI_MESSAGE_NODE_AUXILIARY ?
		__xuiMessageAuxiliaryTitle(pWidget, pNode) :
		pNode->sSender && pNode->sSender[0] ? pNode->sSender :
		pNode->iType == XUI_MESSAGE_NODE_SYSTEM ? "System message" : "Message";
	pResult->sDescription = __xuiMessageText(pNode->sTime);
	pResult->iState = XUI_ACCESSIBLE_STATE_READONLY | XUI_ACCESSIBLE_STATE_SELECTABLE;
	pResult->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW);
	if ( iMessage == pData->iSelected ) pResult->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
	if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
	     (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSIBLE) ) {
		int bCollapsed = (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0;
		pResult->iState |= bCollapsed ? XUI_ACCESSIBLE_STATE_COLLAPSED : XUI_ACCESSIBLE_STATE_EXPANDED;
		pResult->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE) |
			XUI_ACCESSIBLE_ACTION_MASK(bCollapsed ? XUI_ACCESSIBLE_ACTION_EXPAND : XUI_ACCESSIBLE_ACTION_COLLAPSE);
	}
	if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
	     (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) ) pResult->sValue = "";
	else {
		iRet = __xuiMessageAccessibleBody(pNode, &pResult->sValue);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiMessageAccessibleSelection(pData, iMessage, pResult);
	if ( iRet != XUI_OK ) return iRet;
	tContent = xuiWidgetGetContentRect(pWidget);
	tRect = pNode->tNodeRect;
	pResult->tBounds = (xui_rect_t){tWorld.fX + tContent.fX + tRect.fX,
		tWorld.fY + tContent.fY + tRect.fY - pData->fScrollY, tRect.fW, tRect.fH};
	if ( pResult->tBounds.fX + tRect.fW <= tWorld.fX + tContent.fX ||
	     pResult->tBounds.fX >= tWorld.fX + tContent.fX + tContent.fW ||
	     pResult->tBounds.fY + tRect.fH <= tWorld.fY + tContent.fY ||
	     pResult->tBounds.fY >= tWorld.fY + tContent.fY + tContent.fH )
		pResult->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	return XUI_OK;
}

static xui_doc_cell_hit_t __xuiMessageTableSlotHit(const doc_table_cell_slot* pSlot)
{
	xui_doc_cell_hit_t tHit = {0};
	tHit.iSize = sizeof(tHit);
	tHit.iTableId = pSlot->table;
	tHit.iCellId = pSlot->cell;
	tHit.iRow = pSlot->row;
	tHit.iColumn = pSlot->column;
	tHit.iRowSpan = pSlot->row_span;
	tHit.iColumnSpan = pSlot->column_span;
	return tHit;
}

static xui_doc_table_selection_t __xuiMessageTableRangeFromCells(
	const xui_doc_cell_hit_t* pAnchor, const xui_doc_cell_hit_t* pFocus)
{
	xui_doc_table_selection_t tRange = {0};
	uint32_t iEndRow, iEndColumn;
	tRange.iSize = sizeof(tRange);
	tRange.iTableId = pAnchor->iTableId;
	tRange.iRow = pAnchor->iRow < pFocus->iRow ? pAnchor->iRow : pFocus->iRow;
	tRange.iColumn = pAnchor->iColumn < pFocus->iColumn ? pAnchor->iColumn : pFocus->iColumn;
	iEndRow = pAnchor->iRow + pAnchor->iRowSpan;
	iEndColumn = pAnchor->iColumn + pAnchor->iColumnSpan;
	if ( pFocus->iRow + pFocus->iRowSpan > iEndRow )
		iEndRow = pFocus->iRow + pFocus->iRowSpan;
	if ( pFocus->iColumn + pFocus->iColumnSpan > iEndColumn )
		iEndColumn = pFocus->iColumn + pFocus->iColumnSpan;
	tRange.iRows = iEndRow - tRange.iRow;
	tRange.iColumns = iEndColumn - tRange.iColumn;
	return tRange;
}

static int __xuiMessageSetDocumentTableSelectionAt(xui_widget pWidget,
	xui_message_list_data_t* pData, int iMessage,
	const xui_doc_table_selection_t* pSelection, int bInteraction)
{
	xui_message_node_data_t* pMessage;
	xui_document_snapshot pSnapshot = NULL;
	xui_doc_table_selection_t tExpanded;
	doc_table_cell_slot tFirst, tLast;
	int iRet;
	if ( pData == NULL || iMessage < 0 || iMessage >= pData->iNodeCount )
		return XUI_ERROR_INVALID_ARGUMENT;
	pMessage = &pData->arrNodes[iMessage];
	if ( pMessage->pDocumentBinding == NULL ) return XUI_ERROR_NOT_FOUND;
	if ( pSelection == NULL ) {
		if ( pData->iTableSelectionNode != iMessage ||
		     pData->tTableSelection.iTableId == 0 ) return XUI_OK;
		memset(&pData->tTableSelection, 0, sizeof(pData->tTableSelection));
		memset(&pData->tTableDragAnchor, 0, sizeof(pData->tTableDragAnchor));
		memset(&pData->tTableDragFocus, 0, sizeof(pData->tTableDragFocus));
		pData->iTableSelectionNode = -1;
		pData->bTableSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( pSelection->iSize != sizeof(*pSelection) || !pSelection->iTableId ||
	     !pSelection->iRows || !pSelection->iColumns ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiMessageNodeCanSelect(pMessage) ) return XUI_ERROR_UNSUPPORTED;
	iRet = xuiDocumentAcquireSnapshot(pMessage->pDocumentBinding->pDocument, &pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	tExpanded = *pSelection;
	iRet = doc_table_expand_selection(pSnapshot->state, tExpanded.iTableId,
		&tExpanded.iRow, &tExpanded.iColumn,
		&tExpanded.iRows, &tExpanded.iColumns);
	if ( iRet == XUI_OK ) iRet = doc_table_cell_at(pSnapshot->state,
		tExpanded.iTableId, tExpanded.iRow, tExpanded.iColumn, &tFirst);
	if ( iRet == XUI_OK ) iRet = doc_table_cell_at(pSnapshot->state,
		tExpanded.iTableId, tExpanded.iRow + tExpanded.iRows - 1,
		tExpanded.iColumn + tExpanded.iColumns - 1, &tLast);
	xuiDocumentSnapshotRelease(pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMessageSetTextSelection(pData, -1, 0, -1, 0);
	pData->iDocumentSelectionNode = -1;
	pData->bSelecting = pData->bDocumentSelecting = 0;
	pData->tTableSelection = tExpanded;
	pData->tTableDragAnchor = __xuiMessageTableSlotHit(&tFirst);
	pData->tTableDragFocus = __xuiMessageTableSlotHit(&tLast);
	pData->iTableSelectionNode = iMessage;
	if ( !bInteraction && xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	if ( !bInteraction ) return xuiMessageListSetSelected(pWidget, iMessage);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiMessageUpdateTableDrag(xui_widget pWidget,
	xui_message_list_data_t* pData, double fWorldX, double fWorldY)
{
	xui_doc_cell_hit_t tCell = {0}, tAnchor;
	xui_doc_table_selection_t tRange;
	int iMessage, iRet;
	if ( pData == NULL || !pData->bTableSelecting ) return XUI_ERROR_INVALID_STATE;
	iMessage = pData->iTableSelectionNode;
	if ( iMessage < 0 || iMessage >= pData->iNodeCount ) return XUI_ERROR_INVALID_STATE;
	tCell.iSize = sizeof(tCell);
	iRet = __xuiMessageHitDocumentCell(pWidget, pData, iMessage,
		fWorldX, fWorldY, &tCell);
	if ( iRet == XUI_ERROR_NOT_FOUND ) return XUI_OK;
	if ( iRet != XUI_OK ) return iRet;
	if ( tCell.iTableId != pData->tTableDragAnchor.iTableId ) return XUI_OK;
	if ( tCell.iCellId == pData->tTableDragFocus.iCellId ) return XUI_OK;
	tAnchor = pData->tTableDragAnchor;
	tRange = __xuiMessageTableRangeFromCells(&tAnchor, &tCell);
	if ( tRange.iRow == pData->tTableSelection.iRow &&
	     tRange.iColumn == pData->tTableSelection.iColumn &&
	     tRange.iRows == pData->tTableSelection.iRows &&
	     tRange.iColumns == pData->tTableSelection.iColumns ) {
		pData->tTableDragFocus = tCell;
		return XUI_OK;
	}
	iRet = __xuiMessageSetDocumentTableSelectionAt(pWidget, pData,
		iMessage, &tRange, 1);
	if ( pData->iTableSelectionNode == iMessage &&
	     pData->tTableSelection.iTableId == tAnchor.iTableId ) {
		pData->tTableDragAnchor = tAnchor;
		pData->tTableDragFocus = tCell;
		pData->bTableSelecting = 1;
	}
	return iRet;
}

static int __xuiMessageExtendTableSelection(xui_widget pWidget,
	xui_message_list_data_t* pData, int iKey)
{
	xui_document_snapshot pSnapshot = NULL;
	xui_message_document_binding_t* pBinding;
	doc_table_cell_slot tAnchor, tFocus, tTarget;
	xui_doc_table_selection_t tRange;
	xui_doc_cell_hit_t tAnchorHit, tTargetHit, tTargetRect = {0};
	xui_rect_t tContent;
	doc_node* pNode;
	uint32_t iRow, iColumn;
	float fTop, fBottom, fScroll;
	int iMessage, bActive, bAtEdge = 0, iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bActive = pData->tTableSelection.iTableId != 0;
	iMessage = bActive ? pData->iTableSelectionNode : pData->iDocumentSelectionNode;
	if ( iMessage < 0 || iMessage >= pData->iNodeCount ||
	     !__xuiMessageNodeCanSelect(&pData->arrNodes[iMessage]) ) return XUI_ERROR_NOT_FOUND;
	pBinding = pData->arrNodes[iMessage].pDocumentBinding;
	if ( pBinding == NULL ) return XUI_ERROR_NOT_FOUND;
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	if ( bActive ) {
		iRet = doc_table_locate_cell(pSnapshot->state,
			pData->tTableDragAnchor.iCellId, &tAnchor);
		if ( iRet == XUI_OK ) iRet = doc_table_locate_cell(pSnapshot->state,
			pData->tTableDragFocus.iCellId, &tFocus);
		if ( iRet != XUI_OK || tAnchor.table != pData->tTableSelection.iTableId ||
		     tFocus.table != tAnchor.table ) {
			iRet = doc_table_cell_at(pSnapshot->state,
				pData->tTableSelection.iTableId, pData->tTableSelection.iRow,
				pData->tTableSelection.iColumn, &tAnchor);
			if ( iRet == XUI_OK ) iRet = doc_table_cell_at(pSnapshot->state,
				pData->tTableSelection.iTableId,
				pData->tTableSelection.iRow + pData->tTableSelection.iRows - 1,
				pData->tTableSelection.iColumn + pData->tTableSelection.iColumns - 1,
				&tFocus);
		}
	} else {
		if ( pData->tSelectionActiveDocument.iSize == 0 ||
		     pData->tSelectionActiveDocument.iKind == XUI_DOC_POSITION_SOURCE )
			iRet = XUI_ERROR_NOT_FOUND;
		else {
			pNode = doc_index_get(pSnapshot->state->index,
				pData->tSelectionActiveDocument.iNodeId);
			while ( pNode != NULL && pNode->kind != XUI_DOC_CELL )
				pNode = pNode->parent ? doc_index_get(pSnapshot->state->index,
					pNode->parent) : NULL;
			iRet = pNode != NULL ? doc_table_locate_cell(pSnapshot->state,
				pNode->id, &tFocus) : XUI_ERROR_NOT_FOUND;
			if ( iRet == XUI_OK ) tAnchor = tFocus;
		}
	}
	if ( iRet != XUI_OK ) { xuiDocumentSnapshotRelease(pSnapshot); return iRet; }
	iRow = tFocus.row; iColumn = tFocus.column;
	if ( iKey == XUI_KEY_LEFT ) {
		if ( iColumn == 0 ) bAtEdge = 1; else iColumn--;
	} else if ( iKey == XUI_KEY_RIGHT ) {
		iColumn += tFocus.column_span;
		if ( iColumn >= tFocus.columns ) bAtEdge = 1;
	} else if ( iKey == XUI_KEY_UP ) {
		if ( iRow == 0 ) bAtEdge = 1; else iRow--;
	} else if ( iKey == XUI_KEY_DOWN ) {
		iRow += tFocus.row_span;
		if ( iRow >= tFocus.rows ) bAtEdge = 1;
	} else { xuiDocumentSnapshotRelease(pSnapshot); return XUI_ERROR_INVALID_ARGUMENT; }
	if ( bAtEdge && bActive ) { xuiDocumentSnapshotRelease(pSnapshot); return XUI_OK; }
	tTarget = tFocus;
	if ( !bAtEdge ) iRet = doc_table_cell_at(pSnapshot->state,
		tFocus.table, iRow, iColumn, &tTarget);
	xuiDocumentSnapshotRelease(pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	tAnchorHit = __xuiMessageTableSlotHit(&tAnchor);
	tTargetHit = __xuiMessageTableSlotHit(&tTarget);
	tRange = __xuiMessageTableRangeFromCells(&tAnchorHit, &tTargetHit);
	iRet = __xuiMessageSetDocumentTableSelectionAt(pWidget, pData,
		iMessage, &tRange, 1);
	if ( iRet != XUI_OK ) return iRet;
	pData->tTableDragAnchor = tAnchorHit;
	pData->tTableDragFocus = tTargetHit;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMessageDocumentSync(pBinding);
	if ( iRet != XUI_OK ) return iRet;
	tTargetRect.iSize = sizeof(tTargetRect);
	if ( xuiDocumentRendererGetCellRect(pBinding->pRenderer,
	     tTarget.cell, &tTargetRect) != XUI_OK ) return XUI_OK;
	tContent = xuiWidgetGetContentRect(pWidget);
	fTop = pData->arrNodes[iMessage].tTextRect.fY + (float)tTargetRect.tBounds.y;
	fBottom = fTop + (float)tTargetRect.tBounds.height;
	fScroll = pData->fScrollY;
	if ( fTop < fScroll ) fScroll = fTop;
	else if ( fBottom > fScroll + tContent.fH ) fScroll = fBottom - tContent.fH;
	return fScroll != pData->fScrollY ? xuiMessageListSetScroll(pWidget, fScroll) : XUI_OK;
}

static int __xuiMessageAccessibleDocumentAction(xui_widget pWidget,
	xui_message_list_data_t* pData, int iMessage, uint64_t iDocumentIndex,
	int iAction, const void* pPayload)
{
	xui_message_node_data_t* pMessage = &pData->arrNodes[iMessage];
	xui_message_document_binding_t* pBinding = pMessage->pDocumentBinding;
	xui_message_accessible_document_node_t* pEntry;
	xui_document_snapshot pSnapshot = NULL;
	doc_node* pNode;
	int iRet;
	if ( pBinding == NULL || iDocumentIndex >= pBinding->iAccessibleNodeCount )
		return XUI_ERROR_NOT_FOUND;
	pEntry = &pBinding->arrAccessibleNodes[iDocumentIndex];
	if ( iAction == XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW ) {
		xui_accessible_node_t tAccessible = {0};
		xui_rect_t tWorld, tContent;
		float fTop, fScroll;
		tAccessible.iSize = sizeof(tAccessible);
		iRet = __xuiMessageAccessibleGetDocument(pWidget, pData, iMessage,
			iDocumentIndex, &tAccessible);
		if ( iRet != XUI_OK ) return iRet;
		if ( !(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(
			XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW)) ) return XUI_ERROR_UNSUPPORTED;
		tWorld = xuiWidgetGetWorldRect(pWidget);
		tContent = xuiWidgetGetContentRect(pWidget);
		fTop = tWorld.fY + tContent.fY;
		fScroll = pData->fScrollY;
		if ( tAccessible.tBounds.fY < fTop || tAccessible.tBounds.fH > tContent.fH )
			fScroll += tAccessible.tBounds.fY - fTop;
		else if ( tAccessible.tBounds.fY + tAccessible.tBounds.fH > fTop + tContent.fH )
			fScroll += tAccessible.tBounds.fY + tAccessible.tBounds.fH - fTop - tContent.fH;
		return xuiMessageListSetScroll(pWidget, fScroll);
	}
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	if ( pBinding->iAccessibleIdentity != pSnapshot->identity ||
	     pBinding->iAccessibleRevision != pSnapshot->revision ) {
		xuiDocumentSnapshotRelease(pSnapshot);
		return XUI_DOC_ERROR_STALE;
	}
	pNode = doc_index_get(pSnapshot->state->index, pEntry->iDocumentNodeId);
	if ( pNode == NULL ) { xuiDocumentSnapshotRelease(pSnapshot); return XUI_DOC_ERROR_STALE; }
	if ( iAction == XUI_ACCESSIBLE_ACTION_TOGGLE &&
	     pNode->kind == XUI_DOC_LIST_ITEM && (pNode->attrs->iFlags & XUI_DOC_TASK) &&
	     pBinding->tDesc.onTaskToggle != NULL ) {
		void (*onToggle)(xui_widget, xui_doc_node_id, int, void*) =
			pBinding->tDesc.onTaskToggle;
		void* pUser = pBinding->tDesc.pUser;
		xui_doc_node_id iTask = pNode->id;
		int bChecked = (pNode->attrs->iFlags & XUI_DOC_CHECKED) == 0;
		xuiDocumentSnapshotRelease(pSnapshot);
		onToggle(pWidget, iTask, bChecked, pUser);
		return XUI_OK;
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE && pBinding->tDesc.onActivate &&
	     (pNode->kind == XUI_DOC_IMAGE ||
	      (pNode->kind == XUI_DOC_TEXT && (pNode->attrs->iMarks & XUI_DOC_LINK)) ||
	      pNode->kind == XUI_DOC_MATH || pNode->kind == XUI_DOC_DIAGRAM ||
	      pNode->kind == XUI_DOC_HTML) ) {
		pBinding->tDesc.onActivate(pWidget, pNode->id,
			pNode->kind == XUI_DOC_IMAGE && (pNode->attrs->iMarks & XUI_DOC_LINK) ?
				doc_string(pNode->link_target) : doc_string(pNode->resource),
			pBinding->tDesc.pUser);
		xuiDocumentSnapshotRelease(pSnapshot);
		return XUI_OK;
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_SELECTION && pPayload == NULL &&
	     pNode->kind == XUI_DOC_CELL && __xuiMessageNodeCanSelect(pMessage) ) {
		doc_table_cell_slot tSlot;
		xui_doc_table_selection_t tSelected = {0};
		iRet = doc_table_locate_cell(pSnapshot->state, pNode->id, &tSlot);
		xuiDocumentSnapshotRelease(pSnapshot);
		if ( iRet != XUI_OK ) return iRet;
		tSelected.iSize = sizeof(tSelected);
		tSelected.iTableId = tSlot.table;
		tSelected.iRow = tSlot.row;
		tSelected.iColumn = tSlot.column;
		tSelected.iRows = tSlot.row_span;
		tSelected.iColumns = tSlot.column_span;
		return __xuiMessageSetDocumentTableSelectionAt(pWidget, pData,
			iMessage, &tSelected, 0);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_SELECTION &&
	     __xuiMessageNodeCanSelect(pMessage) ) {
		xui_doc_range_t tRange = {0};
		if ( doc_selectable_object_kind(pNode->kind) )
			iRet = doc_accessible_snapshot_object_range(pSnapshot, pNode, &tRange);
		else iRet = doc_accessible_snapshot_selection_range(pSnapshot, pNode,
			(const xui_accessible_selection_t*)pPayload, &tRange);
		xuiDocumentSnapshotRelease(pSnapshot);
		if ( iRet != XUI_OK ) return iRet;
		__xuiMessageSetTextSelection(pData, -1, 0, -1, 0);
		pData->tSelectionAnchorDocument = tRange.tAnchor;
		pData->tSelectionActiveDocument = tRange.tCaret;
		pData->iSelectionAnchorNode = pData->iSelectionActiveNode = iMessage;
		pData->iDocumentSelectionNode = iMessage;
		pBinding->tSelection = tRange;
		pData->bSelecting = pData->bDocumentSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		return xuiMessageListSetSelected(pWidget, iMessage);
	}
	xuiDocumentSnapshotRelease(pSnapshot);
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiMessageAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_message_list_data_t* pData = (xui_message_list_data_t*)pUser;
	xui_message_node_data_t* pNode = NULL;
	uint64_t iDocumentIndex = UINT64_MAX;
	int i;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS && (iNodeId == 0 || iNodeId == 1) )
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	for ( i = 0; i < pData->iNodeCount; i++ )
		if ( pData->arrNodes[i].iAccessibleId == iNodeId ) { pNode = &pData->arrNodes[i]; break; }
	if ( pNode == NULL ) for ( i = 0; i < pData->iNodeCount && pNode == NULL; i++ ) {
		xui_message_node_data_t* pCandidate = &pData->arrNodes[i];
		int iBuild;
		uint64_t j;
		if ( pCandidate->pDocumentBinding == NULL ||
		     (pCandidate->iType == XUI_MESSAGE_NODE_AUXILIARY &&
		      (pCandidate->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED)) ) continue;
		iBuild = __xuiMessageAccessibleBuildDocument(pData, pCandidate);
		if ( iBuild != XUI_OK ) return iBuild;
		for ( j = 0; j < pCandidate->pDocumentBinding->iAccessibleNodeCount; j++ )
			if ( pCandidate->pDocumentBinding->arrAccessibleNodes[j].iAccessibleId == iNodeId ) {
				pNode = pCandidate; iDocumentIndex = j; break;
			}
		if ( pNode != NULL ) i--;
	}
	if ( pNode == NULL ) return XUI_ERROR_NOT_FOUND;
	if ( iDocumentIndex != UINT64_MAX ) {
		if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS )
			return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		if ( pNode->pDocumentBinding->arrAccessibleNodes[iDocumentIndex].iDocumentNodeId != DOC_ROOT ||
		     iAction != XUI_ACCESSIBLE_ACTION_SET_SELECTION )
			return __xuiMessageAccessibleDocumentAction(pWidget, pData,
				i, iDocumentIndex, iAction, pPayload);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS )
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	if ( iAction == XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW )
		return xuiMessageListEnsureVisible(pWidget, i);
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_SELECTION ) {
		const xui_accessible_selection_t* pSelection = (const xui_accessible_selection_t*)pPayload;
		int iAnchor = 0, iCaret = 0, iRet;
		if ( pSelection != NULL && (pSelection->iSize < sizeof(*pSelection) ||
		     pSelection->iAnchor < 0 || pSelection->iCaret < 0)) return XUI_ERROR_INVALID_ARGUMENT;
		if ( !__xuiMessageNodeCanSelect(pNode) )
			return pSelection == NULL ? xuiMessageListSetSelected(pWidget, i) : XUI_ERROR_UNSUPPORTED;
		if ( pNode->pDocumentBinding != NULL ) {
			xui_document_snapshot pSnapshot = NULL;
			doc_plain_projection tProjection = {0};
			xui_doc_position_t tAnchor, tCaret;
			const char* sValue = NULL;
			iRet = __xuiMessageAccessibleBody(pNode, &sValue);
			if ( iRet != XUI_OK ) return iRet;
			iRet = xuiDocumentAcquireSnapshot(pNode->pDocumentBinding->pDocument, &pSnapshot);
			if ( iRet == XUI_OK ) iRet = doc_plain_project(pSnapshot, XUI_DOC_SEMANTIC, &tProjection);
			if ( iRet == XUI_OK && tProjection.bytes > INT_MAX ) iRet = XUI_DOC_ERROR_LIMIT;
			if ( iRet == XUI_OK && pSelection != NULL &&
			     ((uint64_t)pSelection->iAnchor > tProjection.bytes ||
			      (uint64_t)pSelection->iCaret > tProjection.bytes)) iRet = XUI_ERROR_INVALID_ARGUMENT;
			if ( iRet == XUI_OK ) {
				iAnchor = pSelection ? pSelection->iAnchor : 0;
				iCaret = pSelection ? pSelection->iCaret : (int)tProjection.bytes;
				if ((iAnchor < (int)tProjection.bytes &&
				     ((unsigned char)tProjection.text[iAnchor] & 0xc0u) == 0x80u) ||
				    (iCaret < (int)tProjection.bytes &&
				     ((unsigned char)tProjection.text[iCaret] & 0xc0u) == 0x80u))
					iRet = XUI_DOC_ERROR_UTF8;
			}
			if ( iRet == XUI_OK ) {
				tAnchor = doc_plain_unproject(&tProjection, (uint64_t)iAnchor, iAnchor <= iCaret);
				tCaret = iAnchor == iCaret ? tAnchor :
					doc_plain_unproject(&tProjection, (uint64_t)iCaret, iCaret < iAnchor);
			}
			doc_plain_projection_free(&tProjection);
			if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
			if ( iRet != XUI_OK ) return iRet;
			__xuiMessageSetTextSelection(pData, -1, 0, -1, 0);
			pData->tSelectionAnchorDocument = tAnchor;
			pData->tSelectionActiveDocument = tCaret;
			pData->iSelectionAnchorNode = pData->iSelectionActiveNode = i;
			pData->iDocumentSelectionNode = i;
			pNode->pDocumentBinding->tSelection.tAnchor = tAnchor;
			pNode->pDocumentBinding->tSelection.tCaret = tCaret;
		} else if ( __xuiMessageNodeCanSelectText(pNode) ) {
			const char* sValue = __xuiMessageText(pNode->sText);
			size_t iBytes = strlen(sValue);
			if ( iBytes > INT_MAX ) return XUI_DOC_ERROR_LIMIT;
			iAnchor = pSelection ? pSelection->iAnchor : 0;
			iCaret = pSelection ? pSelection->iCaret : (int)iBytes;
			if ( (size_t)iAnchor > iBytes || (size_t)iCaret > iBytes )
				return XUI_ERROR_INVALID_ARGUMENT;
			if ( (iAnchor < (int)iBytes && ((unsigned char)sValue[iAnchor] & 0xc0u) == 0x80u) ||
			     (iCaret < (int)iBytes && ((unsigned char)sValue[iCaret] & 0xc0u) == 0x80u) )
				return XUI_DOC_ERROR_UTF8;
			__xuiMessageSetTextSelection(pData, i, iAnchor, i, iCaret);
			pData->iDocumentSelectionNode = -1;
		} else if ( pSelection != NULL ) return XUI_ERROR_UNSUPPORTED;
		pData->bSelecting = pData->bDocumentSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		return xuiMessageListSetSelected(pWidget, i);
	}
	if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
	     (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSIBLE) &&
	     (iAction == XUI_ACCESSIBLE_ACTION_TOGGLE || iAction == XUI_ACCESSIBLE_ACTION_EXPAND ||
	      iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE) ) {
		int bCollapsed = (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0;
		int bNext = iAction == XUI_ACCESSIBLE_ACTION_TOGGLE ? !bCollapsed :
			iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE;
		if ( bNext == bCollapsed ) return XUI_OK;
		if ( bNext ) pNode->iFlags |= XUI_MESSAGE_NODE_FLAG_COLLAPSED;
		else pNode->iFlags &= ~XUI_MESSAGE_NODE_FLAG_COLLAPSED;
		__xuiMessageDirtyNode(pData, i);
		{
			int iRet = __xuiMessageInvalidate(pWidget, pData);
			if ( iRet == XUI_OK ) __xuiMessageAccessibleFoldChanged(pWidget, pNode);
			return iRet;
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiMessageNotify(xui_widget pWidget, xui_message_list_data_t* pData, int iEvent, int iIndex, const xui_event_t* pInput)
{
	xui_message_list_event_t tEvent;
	xui_message_node_t tNode;
	if ( (pData == NULL) || (pData->onEvent == NULL) ) return XUI_OK;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iEvent = iEvent;
	tEvent.iIndex = iIndex;
	tEvent.iNodeType = -1;
	if ( (iIndex >= 0) && (iIndex < pData->iNodeCount) ) {
		tNode = __xuiMessagePublicNode(&pData->arrNodes[iIndex]);
		tEvent.pNode = &tNode;
		tEvent.iNodeType = tNode.iType;
	}
	if ( pInput != NULL ) {
		tEvent.fX = pInput->fX;
		tEvent.fY = pInput->fY;
		tEvent.iButton = pInput->iButton;
		tEvent.iModifiers = pInput->iModifiers;
	}
	pData->onEvent(pWidget, &tEvent, pData->pEventUser);
	return XUI_OK;
}

static int __xuiMessageGetIndexAtData(xui_widget pWidget, xui_message_list_data_t* pData, float fX, float fY)
{
	xui_rect_t tContent;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int i;
	if ( (pWidget == NULL) || (pData == NULL) ) return -1;
	if ( __xuiMessageLayoutNodes(pWidget, pData) != XUI_OK ) return -1;
	tContent = xuiWidgetGetContentRect(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = fX - tWorld.fX - tContent.fX;
	fLocalY = fY - tWorld.fY - tContent.fY + pData->fScrollY;
	i = __xuiMessageLowerBoundY(pData, fLocalY);
	if ( i < pData->iNodeCount ) {
		tRect = pData->arrNodes[i].tNodeRect;
		if ( fLocalX >= tRect.fX && fLocalX <= tRect.fX + tRect.fW && fLocalY >= tRect.fY && fLocalY <= tRect.fY + tRect.fH ) {
			return i;
		}
	}
	return -1;
}

static int __xuiMessageNodeCanSelectText(const xui_message_node_data_t* pNode)
{
	if ( pNode == NULL || pNode->pDocumentBinding != NULL || pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) return 0;
	if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY && (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0 ) return 0;
	return pNode->sText != NULL && pNode->sText[0] != 0;
}

static int __xuiMessageNodeCanSelect(const xui_message_node_data_t* pNode)
{
	if ( pNode == NULL || pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) return 0;
	if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY && (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0 ) return 0;
	return pNode->pDocumentBinding != NULL || __xuiMessageNodeCanSelectText(pNode);
}

static int __xuiMessageDocumentBoundary(xui_message_document_binding_t* pBinding,
	int bEnd, xui_doc_position_t* pPosition)
{
	xui_document_snapshot pSnapshot = NULL;
	xui_doc_node_info_t tRoot = {0};
	int iRet;
	if ( pBinding == NULL || pPosition == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
	tRoot.iSize = sizeof(tRoot);
	if ( iRet == XUI_OK ) iRet = xuiDocumentSnapshotGetNode(pSnapshot, XUI_DOCUMENT_ROOT, &tRoot);
	if ( iRet == XUI_OK ) {
		memset(pPosition, 0, sizeof(*pPosition));
		pPosition->iSize = sizeof(*pPosition);
		pPosition->iKind = XUI_DOC_POSITION_GAP;
		pPosition->iDocumentId = xuiDocumentSnapshotGetIdentity(pSnapshot);
		pPosition->iRevision = xuiDocumentSnapshotGetRevision(pSnapshot);
		pPosition->iNodeId = XUI_DOCUMENT_ROOT;
		pPosition->iOffset = bEnd ? tRoot.iChildCount : 0;
		pPosition->iAffinity = bEnd ? XUI_DOC_BEFORE : XUI_DOC_AFTER;
	}
	if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
	return iRet;
}

static int __xuiMessageDocumentSelectionRange(const xui_message_list_data_t* pData,
	int iNode, xui_doc_range_t* pRange)
{
	int iStart, iEnd;
	int bForward;
	xui_message_document_binding_t* pBinding;
	int iRet;
	if ( pData == NULL || pRange == NULL || iNode < 0 || iNode >= pData->iNodeCount ||
	     pData->iSelectionAnchorNode < 0 || pData->iSelectionActiveNode < 0 ) return XUI_ERROR_NOT_FOUND;
	pBinding = pData->arrNodes[iNode].pDocumentBinding;
	if ( pBinding == NULL || !__xuiMessageNodeCanSelect(&pData->arrNodes[iNode]) ) return XUI_ERROR_NOT_FOUND;
	bForward = pData->iSelectionAnchorNode <= pData->iSelectionActiveNode;
	iStart = bForward ? pData->iSelectionAnchorNode : pData->iSelectionActiveNode;
	iEnd = bForward ? pData->iSelectionActiveNode : pData->iSelectionAnchorNode;
	if ( iNode < iStart || iNode > iEnd ) return XUI_ERROR_NOT_FOUND;
	memset(pRange, 0, sizeof(*pRange));
	if ( iNode == iStart &&
	     (bForward ? pData->tSelectionAnchorDocument.iSize : pData->tSelectionActiveDocument.iSize) != 0 )
		pRange->tAnchor = bForward ? pData->tSelectionAnchorDocument : pData->tSelectionActiveDocument;
	else {
		iRet = __xuiMessageDocumentBoundary(pBinding, 0, &pRange->tAnchor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( iNode == iEnd &&
	     (bForward ? pData->tSelectionActiveDocument.iSize : pData->tSelectionAnchorDocument.iSize) != 0 )
		pRange->tCaret = bForward ? pData->tSelectionActiveDocument : pData->tSelectionAnchorDocument;
	else {
		iRet = __xuiMessageDocumentBoundary(pBinding, 1, &pRange->tCaret);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiMessagePositionCompare(int iNodeA, int iOffsetA, int iNodeB, int iOffsetB)
{
	if ( iNodeA != iNodeB ) return (iNodeA < iNodeB) ? -1 : 1;
	if ( iOffsetA != iOffsetB ) return (iOffsetA < iOffsetB) ? -1 : 1;
	return 0;
}

static int __xuiMessageGetTextSelectionForNode(const xui_message_list_data_t* pData, int iNode, int* pStart, int* pEnd)
{
	int iStartNode;
	int iStartOffset;
	int iEndNode;
	int iEndOffset;
	int iLength;
	if ( pStart != NULL ) *pStart = 0;
	if ( pEnd != NULL ) *pEnd = 0;
	if ( (pData == NULL) || (iNode < 0) || (iNode >= pData->iNodeCount) ||
	     pData->iSelectionAnchorNode < 0 || pData->iSelectionActiveNode < 0 ||
	     __xuiMessagePositionCompare(pData->iSelectionAnchorNode, pData->iSelectionAnchorOffset, pData->iSelectionActiveNode, pData->iSelectionActiveOffset) == 0 ) return 0;
	if ( __xuiMessagePositionCompare(pData->iSelectionAnchorNode, pData->iSelectionAnchorOffset, pData->iSelectionActiveNode, pData->iSelectionActiveOffset) <= 0 ) {
		iStartNode = pData->iSelectionAnchorNode;
		iStartOffset = pData->iSelectionAnchorOffset;
		iEndNode = pData->iSelectionActiveNode;
		iEndOffset = pData->iSelectionActiveOffset;
	} else {
		iStartNode = pData->iSelectionActiveNode;
		iStartOffset = pData->iSelectionActiveOffset;
		iEndNode = pData->iSelectionAnchorNode;
		iEndOffset = pData->iSelectionAnchorOffset;
	}
	if ( iNode < iStartNode || iNode > iEndNode ) return 0;
	iLength = (int)strlen(__xuiMessageText(pData->arrNodes[iNode].sText));
	if ( pStart != NULL ) *pStart = (iNode == iStartNode) ? iStartOffset : 0;
	if ( pEnd != NULL ) *pEnd = (iNode == iEndNode) ? iEndOffset : iLength;
	if ( pStart != NULL && *pStart < 0 ) *pStart = 0;
	if ( pEnd != NULL && *pEnd > iLength ) *pEnd = iLength;
	return pStart != NULL && pEnd != NULL && *pEnd > *pStart;
}

static int __xuiMessageEnsureLineCarets(xui_widget pWidget, xui_message_list_data_t* pData,
	xui_message_node_data_t* pNode, xui_text_layout pLayout, int iLine, const xui_text_line_t* pLine)
{
	xui_text_shape_t tShape;
	xui_message_text_caret_t* pCarets;
	const char* sText;
	const char* sDisplay;
	int iLength = pLine->iTextSize;
	int iBoundary, iCapacity = 1, iCount = 1;
	int iSource = 0, iDisplay = 0, iDisplaySize, iDisplayBoundary = 0;
	int iCluster = 0, iClusterEnd = 0;
	int iRet;
	float fX = 0.0f;
	if ( pNode->iTextCaretCount > 0 && pNode->iTextCaretLine == iLine ) return XUI_OK;
	pNode->iTextCaretCount = 0;
	iRet = xuiInternalTextLayoutGetDisplayLine(pLayout, iLine, &sDisplay, &iDisplaySize);
	if ( iRet != XUI_OK ) return iRet;
	sText = xuiTextLayoutGetText(pLayout) + pLine->iTextOffset;
	memset(&tShape, 0, sizeof(tShape));
	iRet = xuiTextShape(xuiWidgetGetContext(pWidget), &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=__xuiMessageFont(pWidget, pData), .sText=sDisplay, .iTextSize=iDisplaySize, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tShape);
	if ( iRet != XUI_OK ) { xuiTextShapeFree(&tShape); return iRet; }
	/* Layout lines already end at grapheme boundaries. Work on this line only,
	 * including zero-width source boundaries that disappeared from its display. */
	for ( iBoundary = 0; iBoundary < iLength; iCapacity++ ) {
		XUI_MESSAGE_LIST_AUDIT_STEP(CaretSource);
		if ( iCapacity == INT_MAX ) { xuiTextShapeFree(&tShape); return XUI_ERROR_OUT_OF_MEMORY; }
		iBoundary = xuiInternalTextGraphemeNext(sText, iLength, iBoundary);
	}
	if ( (size_t)iCapacity > SIZE_MAX / sizeof(*pCarets) ) { xuiTextShapeFree(&tShape); return XUI_ERROR_OUT_OF_MEMORY; }
	if ( pNode->iTextCaretCapacity < iCapacity ) {
		pCarets = (xui_message_text_caret_t*)xrtRealloc(pNode->arrTextCarets,
			sizeof(*pCarets) * (size_t)iCapacity);
		if ( pCarets == NULL ) { xuiTextShapeFree(&tShape); return XUI_ERROR_OUT_OF_MEMORY; }
		pNode->arrTextCarets = pCarets;
		pNode->iTextCaretCapacity = iCapacity;
	}
	pCarets = pNode->arrTextCarets;
	pCarets[0] = (xui_message_text_caret_t){pLine->iTextOffset, 0.0f};
	for ( iBoundary = 0; iBoundary < iLength; ) {
		uint32 iScalar = 0;
		XUI_MESSAGE_LIST_AUDIT_STEP(CaretSource);
		iBoundary = xuiInternalTextGraphemeNext(sText, iLength, iBoundary);
		/* Display is an ordered source subsequence plus an optional final '-'.
		 * Match whole UTF-8 scalars: removed and retained scalars can share bytes. */
		while ( iSource < iBoundary ) {
			xstrview tView = {sText + iSource, (size_t)(iLength - iSource)};
			size_t iRead = 0;
			XUI_MESSAGE_LIST_AUDIT_STEP(CaretDecode);
			if ( xrtUtf8Decode(tView, &iScalar, &iRead) != XUTF_OK ) iRead = 1;
			if ( iRead <= (size_t)(iDisplaySize - iDisplay) &&
			     memcmp(sText + iSource, sDisplay + iDisplay, iRead) == 0 ) iDisplay += (int)iRead;
			iSource += (int)iRead;
		}
		if ( iSource == iLength && iDisplay < iDisplaySize ) {
			/* The selected hyphen belongs to the original SHY's ending offset. */
			if ( iScalar != 0xADu || pLine->iBreakType != XUI_TEXT_BREAK_WRAP ||
			     iDisplaySize - iDisplay != 1 || sDisplay[iDisplay] != '-' ) {
				xuiTextShapeFree(&tShape);
				return XUI_ERROR_INVALID_STATE;
			}
			iDisplay++;
		}
		while ( iCluster < tShape.iClusterCount && tShape.pClusters[iCluster].iTextEnd <= iDisplay ) {
			XUI_MESSAGE_LIST_AUDIT_STEP(CaretCluster);
			fX += tShape.pClusters[iCluster].fAdvance;
			iClusterEnd = tShape.pClusters[iCluster++].iTextEnd;
		}
		while ( iDisplayBoundary < iDisplay ) {
			XUI_MESSAGE_LIST_AUDIT_STEP(CaretDisplay);
			iDisplayBoundary = xuiInternalTextGraphemeNext(sDisplay, iDisplaySize, iDisplayBoundary);
		}
		if ( iClusterEnd == iDisplay && iDisplayBoundary == iDisplay )
			pCarets[iCount++] = (xui_message_text_caret_t){pLine->iTextOffset + iBoundary, fX};
	}
	xuiTextShapeFree(&tShape);
	pNode->iTextCaretLine = iLine;
	pNode->iTextCaretCount = iCount;
	return XUI_OK;
}

static float __xuiMessageLineCaretX(const xui_message_node_data_t* pNode, const xui_text_line_t* pLine, int iOffset)
{
	int iLow = 0;
	int iHigh = pNode->iTextCaretCount - 1;
	if ( iOffset <= pLine->iTextOffset ) return 0.0f;
	if ( iOffset >= pLine->iTextOffset + pLine->iTextSize ) return pNode->arrTextCarets[iHigh].fX;
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		if ( pNode->arrTextCarets[iMid].iOffset < iOffset ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	return pNode->arrTextCarets[iLow].fX;
}

static int __xuiMessageLowerBoundLineY(xui_text_layout pLayout, float fY)
{
	int iLow = 0;
	int iHigh = xuiTextLayoutGetLineCount(pLayout);
	xui_text_line_t tLine;
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		XUI_MESSAGE_LIST_AUDIT_STEP(HitLine);
		if ( xuiTextLayoutGetLine(pLayout, iMid, &tLine) != XUI_OK ) return iHigh;
		if ( tLine.fY + tLine.fH < fY ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	return iLow;
}

static int __xuiMessageHitTextOffset(xui_widget pWidget, xui_message_list_data_t* pData, float fX, float fY, int* pNodeIndex, int* pOffset)
{
	xui_message_node_data_t* pNode;
	xui_text_layout pLayout;
	xui_text_line_t tLine;
	xui_rect_t tContent;
	xui_rect_t tText;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int iIndex;
	int iLine;
	int iLow;
	int iHigh;
	if ( pNodeIndex != NULL ) *pNodeIndex = -1;
	if ( pOffset != NULL ) *pOffset = 0;
	if ( (pWidget == NULL) || (pData == NULL) ) return 0;
	iIndex = __xuiMessageGetIndexAtData(pWidget, pData, fX, fY);
	if ( iIndex < 0 ) return 0;
	pNode = &pData->arrNodes[iIndex];
	if ( !__xuiMessageNodeCanSelectText(pNode) ) return 0;
	tContent = xuiWidgetGetContentRect(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tText = pNode->tTextRect;
	fLocalX = fX - tWorld.fX - tContent.fX;
	fLocalY = fY - tWorld.fY - tContent.fY + pData->fScrollY;
	if ( fLocalX < tText.fX || fLocalX > tText.fX + tText.fW || fLocalY < tText.fY || fLocalY > tText.fY + tText.fH ) return 0;
	pLayout = pNode->pTextLayout;
	iLine = __xuiMessageLowerBoundLineY(pLayout, fLocalY - tText.fY);
	if ( xuiTextLayoutGetLine(pLayout, iLine, &tLine) != XUI_OK || fLocalY < tText.fY + tLine.fY ) return 0;
	if ( __xuiMessageEnsureLineCarets(pWidget, pData, pNode, pLayout, iLine, &tLine) != XUI_OK ) return 0;
	iLow = 0;
	iHigh = pNode->iTextCaretCount - 1;
	while ( iLow < iHigh ) {
		int iMid = iLow + (iHigh - iLow) / 2;
		float fMiddle = (pNode->arrTextCarets[iMid].fX + pNode->arrTextCarets[iMid + 1].fX) * 0.5f;
		XUI_MESSAGE_LIST_AUDIT_STEP(HitCaret);
		if ( fLocalX - tText.fX > fMiddle ) iLow = iMid + 1;
		else iHigh = iMid;
	}
	if ( pNodeIndex != NULL ) *pNodeIndex = iIndex;
	if ( pOffset != NULL ) *pOffset = pNode->arrTextCarets[iLow].iOffset;
	return 1;
}

static int __xuiMessageResolveSelectionEndpoint(xui_widget pWidget, xui_message_list_data_t* pData,
	float fX, float fY, int iAnchorNode, int* pNodeIndex, int* pOffset,
	xui_doc_position_t* pDocumentPosition)
{
	xui_rect_t tContent, tWorld, tText;
	xui_message_node_data_t* pNode;
	float fLocalX, fLocalY;
	int iCandidate, iBefore, iAfter, i, iCount, bStart;
	if ( pNodeIndex == NULL || pOffset == NULL || pDocumentPosition == NULL ||
	     pWidget == NULL || pData == NULL || iAnchorNode < 0 || iAnchorNode >= pData->iNodeCount ) return 0;
	*pNodeIndex = -1;
	*pOffset = 0;
	memset(pDocumentPosition, 0, sizeof(*pDocumentPosition));
	if ( __xuiMessageLayoutNodes(pWidget, pData) != XUI_OK ) return 0;
	tContent = xuiWidgetGetContentRect(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = fX - tWorld.fX - tContent.fX;
	fLocalY = fY - tWorld.fY - tContent.fY + pData->fScrollY;
	i = __xuiMessageLowerBoundY(pData, fLocalY);
	iCount = i > 0 ? pData->arrNodes[i - 1].iSelectablePrefix : 0;
	iBefore = __xuiMessageSelectableByCount(pData, iCount);
	iAfter = __xuiMessageSelectableByCount(pData, iCount + 1);
	iCandidate = -1;
	if ( iAfter >= 0 ) {
		pNode = &pData->arrNodes[iAfter];
		if ( fLocalY >= pNode->tNodeRect.fY &&
		     fLocalY <= pNode->tNodeRect.fY + pNode->tNodeRect.fH ) iCandidate = iAfter;
	}
	if ( iCandidate < 0 ) {
		if ( fLocalY < pData->arrNodes[iAnchorNode].tNodeRect.fY )
			iCandidate = iBefore >= 0 ? iBefore : iAfter;
		else iCandidate = iAfter >= 0 ? iAfter : iBefore;
	}
	if ( iCandidate < 0 ) return 0;
	pNode = &pData->arrNodes[iCandidate];
	tText = pNode->tTextRect;
	bStart = iCandidate < iAnchorNode ||
		(iCandidate == iAnchorNode && (fLocalY < tText.fY ||
		(fLocalY <= tText.fY + tText.fH && fLocalX < tText.fX)));
	*pNodeIndex = iCandidate;
	if ( pNode->pDocumentBinding != NULL ) {
		if ( fLocalY >= tText.fY && fLocalY <= tText.fY + tText.fH &&
		     __xuiMessageHitDocument(pWidget, pData, iCandidate, fX, fY, 1,
		         pDocumentPosition) == XUI_OK ) return 1;
		return __xuiMessageDocumentBoundary(pNode->pDocumentBinding,
			!bStart, pDocumentPosition) == XUI_OK;
	}
	if ( fLocalX >= tText.fX && fLocalX <= tText.fX + tText.fW &&
	     fLocalY >= tText.fY && fLocalY <= tText.fY + tText.fH &&
	     __xuiMessageHitTextOffset(pWidget, pData, fX, fY, pNodeIndex, pOffset) ) return 1;
	*pNodeIndex = iCandidate;
	*pOffset = bStart ? 0 : (int)strlen(__xuiMessageText(pNode->sText));
	return 1;
}

static void __xuiMessageSetTextSelection(xui_message_list_data_t* pData, int iAnchorNode, int iAnchorOffset, int iActiveNode, int iActiveOffset)
{
	if ( pData == NULL ) return;
	pData->iSelectionAnchorNode = iAnchorNode;
	pData->iSelectionAnchorOffset = iAnchorOffset;
	pData->iSelectionActiveNode = iActiveNode;
	pData->iSelectionActiveOffset = iActiveOffset;
	memset(&pData->tSelectionAnchorDocument, 0, sizeof(pData->tSelectionAnchorDocument));
	memset(&pData->tSelectionActiveDocument, 0, sizeof(pData->tSelectionActiveDocument));
	memset(&pData->tTableSelection, 0, sizeof(pData->tTableSelection));
	memset(&pData->tTableDragAnchor, 0, sizeof(pData->tTableDragAnchor));
	memset(&pData->tTableDragFocus, 0, sizeof(pData->tTableDragFocus));
	pData->iTableSelectionNode = -1;
	pData->bTableSelecting = 0;
}

static int __xuiMessagePointInRect(xui_rect_t tRect, float fX, float fY)
{
	return fX >= tRect.fX && fX <= tRect.fX + tRect.fW && fY >= tRect.fY && fY <= tRect.fY + tRect.fH;
}

static int __xuiMessageAppendSelectionBytes(char** ppText, int* pLength, int* pCapacity, const char* sText, int iLength)
{
	char* sNew;
	int iRequired;
	int iCapacity;
	if ( ppText == NULL || pLength == NULL || pCapacity == NULL || iLength < 0 ||
	     (iLength > 0 && sText == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( *pLength < 0 || *pLength > INT_MAX - iLength - 1 ) return XUI_DOC_ERROR_LIMIT;
	iRequired = *pLength + iLength + 1;
	if ( iRequired > *pCapacity ) {
		iCapacity = (*pCapacity > 0) ? *pCapacity : 128;
		while ( iCapacity < iRequired )
			iCapacity = iCapacity > INT_MAX / 2 ? iRequired : iCapacity * 2;
		sNew = (char*)xrtMalloc((size_t)iCapacity);
		if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		if ( *ppText != NULL && *pLength > 0 ) memcpy(sNew, *ppText, (size_t)*pLength);
		if ( *ppText != NULL ) xrtFree(*ppText);
		*ppText = sNew;
		*pCapacity = iCapacity;
	}
	if ( iLength > 0 && sText != NULL ) memcpy(*ppText + *pLength, sText, (size_t)iLength);
	*pLength += iLength;
	(*ppText)[*pLength] = 0;
	return XUI_OK;
}

static int __xuiMessageBuildSelectedText(xui_message_list_data_t* pData, char** ppText)
{
	char* sText;
	const char* sNodeText;
	int iStartNode;
	int iStartOffset;
	int iEndNode;
	int iEndOffset;
	int iLength;
	int iCapacity;
	int i;
	int iNodeStart;
	int iNodeEnd;
	int iRet;
	if ( ppText != NULL ) *ppText = NULL;
	if ( pData == NULL || ppText == NULL ||
	     __xuiMessagePositionCompare(pData->iSelectionAnchorNode, pData->iSelectionAnchorOffset, pData->iSelectionActiveNode, pData->iSelectionActiveOffset) == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiMessagePositionCompare(pData->iSelectionAnchorNode, pData->iSelectionAnchorOffset, pData->iSelectionActiveNode, pData->iSelectionActiveOffset) <= 0 ) {
		iStartNode = pData->iSelectionAnchorNode;
		iStartOffset = pData->iSelectionAnchorOffset;
		iEndNode = pData->iSelectionActiveNode;
		iEndOffset = pData->iSelectionActiveOffset;
	} else {
		iStartNode = pData->iSelectionActiveNode;
		iStartOffset = pData->iSelectionActiveOffset;
		iEndNode = pData->iSelectionAnchorNode;
		iEndOffset = pData->iSelectionAnchorOffset;
	}
	if ( iStartNode < 0 || iEndNode >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	sText = NULL;
	iLength = 0;
	iCapacity = 0;
	for ( i = iStartNode; i <= iEndNode; i++ ) {
		if ( !__xuiMessageNodeCanSelectText(&pData->arrNodes[i]) ) continue;
		sNodeText = __xuiMessageText(pData->arrNodes[i].sText);
		iNodeStart = (i == iStartNode) ? iStartOffset : 0;
		iNodeEnd = (i == iEndNode) ? iEndOffset : (int)strlen(sNodeText);
		if ( iNodeStart < 0 ) iNodeStart = 0;
		if ( iNodeEnd > (int)strlen(sNodeText) ) iNodeEnd = (int)strlen(sNodeText);
		if ( iNodeEnd < iNodeStart ) iNodeEnd = iNodeStart;
		if ( iLength > 0 ) {
			iRet = __xuiMessageAppendSelectionBytes(&sText, &iLength, &iCapacity, "\n", 1);
			if ( iRet != XUI_OK ) goto failed;
		}
		iRet = __xuiMessageAppendSelectionBytes(&sText, &iLength, &iCapacity, sNodeText + iNodeStart, iNodeEnd - iNodeStart);
		if ( iRet != XUI_OK ) goto failed;
	}
	if ( sText == NULL || iLength == 0 ) {
		if ( sText != NULL ) xrtFree(sText);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppText = sText;
	return XUI_OK;
failed:
	if ( sText != NULL ) xrtFree(sText);
	return iRet;
}

static int __xuiMessageBuildMixedSelectedText(xui_message_list_data_t* pData, char** ppText)
{
	char* sText = NULL;
	int iLength = 0, iCapacity = 0, iStart, iEnd, i, iRet = XUI_OK;
	if ( ppText != NULL ) *ppText = NULL;
	if ( pData == NULL || ppText == NULL || pData->iSelectionAnchorNode < 0 ||
	     pData->iSelectionActiveNode < 0 || pData->iSelectionAnchorNode >= pData->iNodeCount ||
	     pData->iSelectionActiveNode >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	iStart = pData->iSelectionAnchorNode < pData->iSelectionActiveNode ?
		pData->iSelectionAnchorNode : pData->iSelectionActiveNode;
	iEnd = pData->iSelectionAnchorNode > pData->iSelectionActiveNode ?
		pData->iSelectionAnchorNode : pData->iSelectionActiveNode;
	for ( i = iStart; i <= iEnd; i++ ) {
		xui_message_node_data_t* pNode = &pData->arrNodes[i];
		const char* sPart = NULL;
		int iPartLength = 0;
		char* sDocumentText = NULL;
		if ( !__xuiMessageNodeCanSelect(pNode) ) continue;
		if ( pNode->pDocumentBinding != NULL ) {
			xui_document_snapshot pSnapshot = NULL;
			xui_doc_range_t tRange;
			uint64_t iBytes = 0;
			iRet = __xuiMessageDocumentSelectionRange(pData, i, &tRange);
			if ( iRet == XUI_OK ) iRet = xuiDocumentAcquireSnapshot(pNode->pDocumentBinding->pDocument, &pSnapshot);
			if ( iRet == XUI_OK ) iRet = xuiDocumentSnapshotCopyRange(pSnapshot, &tRange,
				&sDocumentText, &iBytes);
			if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
			if ( iRet != XUI_OK ) { xuiDocumentFreeBuffer(sDocumentText); goto failed; }
			if ( iBytes >= INT_MAX ) {
				xuiDocumentFreeBuffer(sDocumentText);
				iRet = XUI_DOC_ERROR_LIMIT;
				goto failed;
			}
			sPart = sDocumentText;
			iPartLength = (int)iBytes;
		} else {
			int iPartStart = 0, iPartEnd;
			sPart = __xuiMessageText(pNode->sText);
			iPartEnd = (int)strlen(sPart);
			if ( i == pData->iSelectionAnchorNode ) iPartStart = pData->iSelectionAnchorOffset;
			if ( i == pData->iSelectionActiveNode ) iPartEnd = pData->iSelectionActiveOffset;
			if ( pData->iSelectionAnchorNode > pData->iSelectionActiveNode ) {
				iPartStart = i == pData->iSelectionActiveNode ? pData->iSelectionActiveOffset : 0;
				iPartEnd = i == pData->iSelectionAnchorNode ? pData->iSelectionAnchorOffset : (int)strlen(sPart);
			}
			if ( iPartStart < 0 ) iPartStart = 0;
			if ( iPartEnd > (int)strlen(sPart) ) iPartEnd = (int)strlen(sPart);
			if ( iPartEnd < iPartStart ) iPartEnd = iPartStart;
			sPart += iPartStart;
			iPartLength = iPartEnd - iPartStart;
		}
		if ( iPartLength > 0 ) {
			if ( iLength > 0 ) iRet = __xuiMessageAppendSelectionBytes(&sText,
				&iLength, &iCapacity, "\n", 1);
			if ( iRet == XUI_OK ) iRet = __xuiMessageAppendSelectionBytes(&sText,
				&iLength, &iCapacity, sPart, iPartLength);
		}
		xuiDocumentFreeBuffer(sDocumentText);
		if ( iRet != XUI_OK ) goto failed;
	}
	if ( iLength == 0 ) { iRet = XUI_ERROR_INVALID_ARGUMENT; goto failed; }
	*ppText = sText;
	return XUI_OK;
failed:
	if ( sText != NULL ) xrtFree(sText);
	return iRet;
}

static void __xuiMessageContextMenuSelect(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_widget pWidget = (xui_widget)pUser;
	(void)pMenu;
	(void)iIndex;
	if ( pWidget == NULL || iValue != 1 ) return;
	(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	(void)xuiMessageListCopySelection(pWidget);
}

static int __xuiMessageOpenContextMenu(xui_widget pWidget, xui_message_list_data_t* pData, float fX, float fY)
{
	xui_menu_item_t tItem;
	if ( pWidget == NULL || pData == NULL || pData->pContextMenu == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	memset(&tItem, 0, sizeof(tItem));
	tItem.sText = xuiTranslate(xuiWidgetGetContext(pWidget), XUI_TR_MESSAGE_COPY);
	tItem.sShortcut = "Ctrl+C";
	tItem.iType = XUI_MENU_ITEM_NORMAL;
	tItem.iState = (xuiMessageListGetSelectedText(pWidget, NULL, 0) > 1) ? XUI_MENU_ITEM_ENABLED : 0u;
	tItem.iValue = 1;
	if ( xuiMenuSetItems(pData->pContextMenu, &tItem, 1) != XUI_OK ) return XUI_ERROR;
	return xuiMenuOpenAt(pData->pContextMenu, pWidget, fX, fY);
}

static int __xuiMessageEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_message_list_data_t* pData;
	xui_message_node_data_t* pNode;
	xui_message_document_binding_t* pBinding;
	xui_doc_position_t tDocumentHit;
	xui_doc_position_t tDocumentAnchor;
	xui_rect_t tContent;
	xui_rect_t tHeader;
	xui_rect_t tWorld;
	float fMaxScroll;
	float fOldScroll;
	int iIndex;
	int iTextNode;
	int iTextOffset;
	int bChanged;
	(void)pUser;
	pData = __xuiMessageListGetData(pWidget);
	if ( (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ||
	     (pEvent->iType == XUI_EVENT_CONTEXT_MENU && pEvent->iKey == XUI_KEY_CONTEXT_MENU) ) {
		int iRet = __xuiMessageLayoutNodes(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		iIndex = __xuiMessageGetIndexAtData(pWidget, pData, pEvent->fX, pEvent->fY);
		bChanged = 0;
		if ( iIndex != pData->iHover ) {
			pData->iHover = iIndex;
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_HOVER, iIndex, pEvent);
			bChanged = 1;
		}
		if ( pData->bTableSelecting ) {
			int iRet = __xuiMessageUpdateTableDrag(pWidget, pData,
				pEvent->fX, pEvent->fY);
			if ( iRet != XUI_OK ) return iRet;
			if ( bChanged ) (void)xuiWidgetInvalidate(pWidget,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pData->bDocumentSelecting && pData->iDocumentSelectionNode >= 0 &&
		     pData->iDocumentSelectionNode < pData->iNodeCount ) {
			pBinding = pData->arrNodes[pData->iDocumentSelectionNode].pDocumentBinding;
			if ( pBinding != NULL ) {
				tContent = xuiWidgetGetContentRect(pWidget);
				tWorld = xuiWidgetGetWorldRect(pWidget);
				fMaxScroll = __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH);
				fOldScroll = pData->fScrollY;
				if ( pEvent->fY < tWorld.fY + tContent.fY + 12.0f ) pData->fScrollY -= pData->tMetrics.fWheelStep * 0.35f;
				else if ( pEvent->fY > tWorld.fY + tContent.fY + tContent.fH - 12.0f ) pData->fScrollY += pData->tMetrics.fWheelStep * 0.35f;
				pData->fScrollY = __xuiMessageClamp(pData->fScrollY, 0.0f, fMaxScroll);
				if ( pData->fScrollY != fOldScroll ) bChanged = 1;
			if ( __xuiMessageResolveSelectionEndpoint(pWidget, pData, pEvent->fX, pEvent->fY,
				     pData->iSelectionAnchorNode, &iTextNode, &iTextOffset, &tDocumentHit) &&
				     (pData->iSelectionActiveNode != iTextNode ||
				      pData->iSelectionActiveOffset != iTextOffset ||
				      pData->tSelectionActiveDocument.iNodeId != tDocumentHit.iNodeId ||
				      pData->tSelectionActiveDocument.iOffset != tDocumentHit.iOffset ||
				      pData->tSelectionActiveDocument.iAffinity != tDocumentHit.iAffinity) ) {
					pData->iSelectionActiveNode = iTextNode;
					pData->iSelectionActiveOffset = iTextOffset;
					pData->tSelectionActiveDocument = tDocumentHit;
					if ( iTextNode == pData->iDocumentSelectionNode )
						pBinding->tSelection.tCaret = tDocumentHit;
					pBinding->iPressedNode = 0;
					bChanged = 1;
					xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
				}
			}
			if ( bChanged ) return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pData->bSelecting ) {
			tContent = xuiWidgetGetContentRect(pWidget);
			tWorld = xuiWidgetGetWorldRect(pWidget);
			fMaxScroll = __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH);
			fOldScroll = pData->fScrollY;
			if ( pEvent->fY < tWorld.fY + tContent.fY + 12.0f ) pData->fScrollY -= pData->tMetrics.fWheelStep * 0.35f;
			else if ( pEvent->fY > tWorld.fY + tContent.fY + tContent.fH - 12.0f ) pData->fScrollY += pData->tMetrics.fWheelStep * 0.35f;
			pData->fScrollY = __xuiMessageClamp(pData->fScrollY, 0.0f, fMaxScroll);
			if ( pData->fScrollY != fOldScroll ) bChanged = 1;
		}
		if ( pData->bSelecting &&
		     __xuiMessageResolveSelectionEndpoint(pWidget, pData, pEvent->fX, pEvent->fY,
		         pData->iSelectionAnchorNode, &iTextNode, &iTextOffset, &tDocumentHit) ) {
			if ( iTextNode != pData->iSelectionActiveNode || iTextOffset != pData->iSelectionActiveOffset ||
			     pData->tSelectionActiveDocument.iNodeId != tDocumentHit.iNodeId ||
			     pData->tSelectionActiveDocument.iOffset != tDocumentHit.iOffset ||
			     pData->tSelectionActiveDocument.iAffinity != tDocumentHit.iAffinity ) {
				pData->iSelectionActiveNode = iTextNode;
				pData->iSelectionActiveOffset = iTextOffset;
				pData->tSelectionActiveDocument = tDocumentHit;
				bChanged = 1;
				xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
			}
		}
		if ( bChanged ) return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	} else if ( pEvent->iType == XUI_EVENT_POINTER_LEAVE ) {
		if ( pData->iHover != -1 && !pData->bSelecting ) {
			pData->iHover = -1;
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_HOVER, -1, pEvent);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	} else if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		pData->iPressedTaskNode = 0;
		iIndex = __xuiMessageGetIndexAtData(pWidget, pData, pEvent->fX, pEvent->fY);
		if ( iIndex >= 0 ) {
			pData->iSelected = iIndex;
			pData->iSelectCount++;
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_SELECT, iIndex, pEvent);
			pNode = &pData->arrNodes[iIndex];
			tContent = xuiWidgetGetContentRect(pWidget);
			tWorld = xuiWidgetGetWorldRect(pWidget);
			tHeader = pNode->tHeaderRect;
			if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY &&
			     (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSIBLE) != 0 &&
			     __xuiMessagePointInRect(tHeader,
			         pEvent->fX - tWorld.fX - tContent.fX,
			         pEvent->fY - tWorld.fY - tContent.fY + pData->fScrollY) ) {
				pNode->iFlags ^= XUI_MESSAGE_NODE_FLAG_COLLAPSED;
				__xuiMessageDirtyNode(pData, iIndex);
				(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_TOGGLE, iIndex, pEvent);
				{
					int iRet = __xuiMessageInvalidate(pWidget, pData);
				if ( iRet == XUI_OK ) __xuiMessageAccessibleFoldChanged(pWidget, pNode);
					return iRet;
				}
			}
			if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT &&
			     pNode->pDocumentBinding != NULL &&
			     pNode->pDocumentBinding->tDesc.onTaskToggle != NULL ) {
				xui_doc_node_id iTask = 0;
				if ( __xuiMessageHitDocumentTaskMarker(pWidget, pData, iIndex,
				     pEvent->fX, pEvent->fY, &iTask) == XUI_OK ) {
					pData->iPressedTaskNode = iTask;
					pData->iPressedTaskMessage = iIndex;
					(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
					(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
					return XUI_EVENT_DISPATCH_STOP;
				}
			}
			if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT &&
			     (pEvent->iModifiers & XUI_MOD_ALT) && pNode->pDocumentBinding != NULL ) {
				xui_doc_cell_hit_t tCell = {0};
				tCell.iSize = sizeof(tCell);
				if ( __xuiMessageHitDocumentCell(pWidget, pData, iIndex,
				     pEvent->fX, pEvent->fY, &tCell) == XUI_OK ) {
					xui_doc_table_selection_t tRange =
						__xuiMessageTableRangeFromCells(&tCell, &tCell);
					int iRet = __xuiMessageSetDocumentTableSelectionAt(pWidget,
						pData, iIndex, &tRange, 1);
					if ( iRet != XUI_OK ) return iRet;
					pData->tTableDragAnchor = tCell;
					pData->tTableDragFocus = tCell;
					pData->bTableSelecting = 1;
					(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
					(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
					return XUI_EVENT_DISPATCH_STOP;
				}
			}
			if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT && pNode->pDocumentBinding != NULL &&
			     __xuiMessageHitDocument(pWidget, pData, iIndex, pEvent->fX, pEvent->fY, 0, &tDocumentHit) == XUI_OK ) {
				pBinding = pNode->pDocumentBinding;
				tDocumentAnchor = (pEvent->iModifiers & XUI_MOD_SHIFT) != 0 &&
					pData->iSelectionAnchorNode == iIndex &&
					pData->tSelectionAnchorDocument.iSize != 0 ?
					pData->tSelectionAnchorDocument : tDocumentHit;
				__xuiMessageSetTextSelection(pData, -1, 0, -1, 0);
				pData->tSelectionAnchorDocument = tDocumentAnchor;
				pData->tSelectionActiveDocument = tDocumentHit;
				pData->iSelectionAnchorNode = iIndex;
				pData->iSelectionActiveNode = iIndex;
				pBinding->tSelection.tAnchor = tDocumentAnchor;
				pBinding->tSelection.tCaret = tDocumentHit;
				pBinding->iPressedNode = (pEvent->iModifiers & XUI_MOD_SHIFT) == 0 ?
					tDocumentHit.iNodeId : 0;
				pData->iDocumentSelectionNode = iIndex;
				pData->bDocumentSelecting = 1;
				(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
				(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
				(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
				return XUI_EVENT_DISPATCH_STOP;
			}
			if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT && __xuiMessageHitTextOffset(pWidget, pData, pEvent->fX, pEvent->fY, &iTextNode, &iTextOffset) ) {
				pData->iDocumentSelectionNode = -1;
				(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
				__xuiMessageSetTextSelection(pData, iTextNode, iTextOffset, iTextNode, iTextOffset);
				pData->bSelecting = 1;
				(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
				return XUI_EVENT_DISPATCH_STOP;
			}
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	} else if ( pEvent->iType == XUI_EVENT_POINTER_UP || pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		if ( pData->iPressedTaskNode != 0 ) {
			xui_doc_node_id iPressed = pData->iPressedTaskNode;
			int iMessage = pData->iPressedTaskMessage;
			pData->iPressedTaskNode = 0;
			if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
				(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			if ( pEvent->iType == XUI_EVENT_POINTER_UP &&
			     iMessage >= 0 && iMessage < pData->iNodeCount ) {
				xui_doc_node_id iReleased = 0;
				pBinding = pData->arrNodes[iMessage].pDocumentBinding;
				if ( pBinding != NULL && pBinding->tDesc.onTaskToggle != NULL &&
				     __xuiMessageHitDocumentTaskMarker(pWidget, pData, iMessage,
				         pEvent->fX, pEvent->fY, &iReleased) == XUI_OK &&
				     iReleased == iPressed ) {
					xui_document_snapshot pSnapshot = NULL;
					if ( xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot) == XUI_OK ) {
						doc_node* pTask = doc_index_get(pSnapshot->state->index, iPressed);
						if ( pTask != NULL && pTask->kind == XUI_DOC_LIST_ITEM &&
						     (pTask->attrs->iFlags & XUI_DOC_TASK) ) {
							void (*onToggle)(xui_widget, xui_doc_node_id, int, void*) =
								pBinding->tDesc.onTaskToggle;
							void* pCallbackUser = pBinding->tDesc.pUser;
							int bChecked = (pTask->attrs->iFlags & XUI_DOC_CHECKED) == 0;
							xuiDocumentSnapshotRelease(pSnapshot);
							onToggle(pWidget, iPressed, bChecked, pCallbackUser);
							return XUI_EVENT_DISPATCH_STOP;
						}
						xuiDocumentSnapshotRelease(pSnapshot);
					}
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pData->bTableSelecting ) {
			int iRet = pEvent->iType == XUI_EVENT_POINTER_UP ?
				__xuiMessageUpdateTableDrag(pWidget, pData, pEvent->fX, pEvent->fY) : XUI_OK;
			pData->bTableSelecting = 0;
			if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
				(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			return iRet == XUI_OK ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pData->bDocumentSelecting ) {
			int iDocumentIndex = pData->iDocumentSelectionNode;
			pData->bDocumentSelecting = 0;
			if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
				(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			if ( pEvent->iType == XUI_EVENT_POINTER_UP && iDocumentIndex >= 0 &&
			     iDocumentIndex < pData->iNodeCount &&
			     (pBinding = pData->arrNodes[iDocumentIndex].pDocumentBinding) != NULL &&
			     pBinding->iPressedNode != 0 && pBinding->tDesc.onActivate != NULL &&
			     __xuiMessageHitDocument(pWidget, pData, iDocumentIndex,
			         pEvent->fX, pEvent->fY, 0, &tDocumentHit) == XUI_OK &&
			     tDocumentHit.iNodeId == pBinding->iPressedNode ) {
				xui_document_snapshot pSnapshot = NULL;
				xui_doc_node_info_t tInfo = {0};
				tInfo.iSize = sizeof(tInfo);
				if ( xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot) == XUI_OK ) {
					if ( xuiDocumentSnapshotGetNode(pSnapshot, tDocumentHit.iNodeId, &tInfo) == XUI_OK )
						pBinding->tDesc.onActivate(pWidget, tDocumentHit.iNodeId,
							tInfo.iKind == XUI_DOC_IMAGE && (tInfo.tAttributes.iMarks & XUI_DOC_LINK) ?
								tInfo.sLinkTarget : tInfo.sResource, pBinding->tDesc.pUser);
					xuiDocumentSnapshotRelease(pSnapshot);
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pData->bSelecting || xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pData->bSelecting = 0;
			if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) (void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
	} else if ( pEvent->iType == XUI_EVENT_POINTER_CLICK || pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK ) {
		iIndex = __xuiMessageGetIndexAtData(pWidget, pData, pEvent->fX, pEvent->fY);
		if ( iIndex >= 0 ) {
			pData->iClickCount++;
			(void)__xuiMessageNotify(pWidget, pData, pEvent->iType == XUI_EVENT_POINTER_CLICK ? XUI_MESSAGE_EVENT_CLICK : XUI_MESSAGE_EVENT_DOUBLE_CLICK, iIndex, pEvent);
		}
	} else if ( pEvent->iType == XUI_EVENT_CONTEXT_MENU ) {
		float fMenuX = pEvent->fX;
		float fMenuY = pEvent->fY;
		if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
			tWorld = xuiWidgetGetWorldRect(pWidget);
			tContent = xuiWidgetGetContentRect(pWidget);
			iIndex = pData->iSelected;
			if ( iIndex >= 0 && iIndex < pData->iNodeCount ) {
				xui_rect_t tNode = pData->arrNodes[iIndex].tNodeRect;
				fMenuX = tWorld.fX + tContent.fX + tNode.fX;
				fMenuY = tWorld.fY + tContent.fY + tNode.fY - pData->fScrollY + tNode.fH;
			} else {
				fMenuX = tWorld.fX + tContent.fX;
				fMenuY = tWorld.fY + tContent.fY;
			}
		} else {
			iIndex = __xuiMessageGetIndexAtData(pWidget, pData, pEvent->fX, pEvent->fY);
		}
		if ( iIndex >= 0 ) {
			pNode = &pData->arrNodes[iIndex];
			pData->iSelected = iIndex;
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
			if ( __xuiMessageNodeCanSelectText(pNode) && __xuiMessagePositionCompare(pData->iSelectionAnchorNode, pData->iSelectionAnchorOffset, pData->iSelectionActiveNode, pData->iSelectionActiveOffset) == 0 ) {
				__xuiMessageSetTextSelection(pData, iIndex, 0, iIndex, (int)strlen(__xuiMessageText(pNode->sText)));
			}
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_CONTEXT_MENU, iIndex, pEvent);
			(void)__xuiMessageOpenContextMenu(pWidget, pData, fMenuX, fMenuY);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_CONTEXT_MENU, -1, pEvent);
		(void)__xuiMessageOpenContextMenu(pWidget, pData, fMenuX, fMenuY);
		return XUI_EVENT_DISPATCH_STOP;
	} else if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		tContent = xuiWidgetGetContentRect(pWidget);
		fMaxScroll = __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH);
		fOldScroll = pData->fScrollY;
		pData->fScrollY = __xuiMessageClamp(pData->fScrollY - pEvent->fWheelY * pData->tMetrics.fWheelStep, 0.0f, fMaxScroll);
		if ( pData->fScrollY != fOldScroll ) {
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_SCROLL, -1, pEvent);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	} else if ( pEvent->iType == XUI_EVENT_BOUNDS_CHANGED ) {
		return __xuiMessageInvalidate(pWidget, pData);
	} else if ( pEvent->iType == XUI_EVENT_KEY_DOWN ) {
		if ( (pEvent->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT)) ==
		     (XUI_MOD_ALT | XUI_MOD_SHIFT) &&
		     (pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT ||
		      pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN) ) {
			int iRet = __xuiMessageExtendTableSelection(pWidget, pData, pEvent->iKey);
			return iRet == XUI_ERROR_NOT_FOUND ? XUI_OK :
				iRet == XUI_OK ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_ESCAPE && pData->tTableSelection.iTableId != 0 ) {
			int iRet = __xuiMessageSetDocumentTableSelectionAt(pWidget, pData,
				pData->iTableSelectionNode, NULL, 0);
			return iRet == XUI_OK ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( (pEvent->iModifiers & XUI_MOD_CTRL) != 0 && (pEvent->iKey == 'c' || pEvent->iKey == 'C') ) {
			(void)xuiMessageListCopySelection(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
	}
	return XUI_OK;
}

static int __xuiMessageInitEvents(xui_widget pWidget)
{
	int iRet;
	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiMessageEvent, NULL);
	return iRet;
}

static int __xuiMessageContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)pUser;
	(void)tConstraint;
	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiMessageListGetData(pWidget) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pSize->fX = XUI_MESSAGE_LIST_DEFAULT_WIDTH;
	pSize->fY = XUI_MESSAGE_LIST_DEFAULT_HEIGHT;
	return XUI_OK;
}

static int __xuiMessageDrawWrappedText(xui_widget pWidget, xui_message_list_data_t* pData, xui_proxy pProxy, xui_draw_context pDraw, xui_text_layout pLayout, int iNodeIndex, xui_rect_t tRect, uint32_t iColor, int bCenter, uint32_t iSelectionColor)
{
	xui_text_line_t tLine;
	xui_font pFont;
	xui_rect_t tContent;
	xui_rect_t tLineRect;
	xui_rect_t tSelectionRect;
	const char* sDisplay;
	int iDisplaySize;
	float fStart;
	float fEnd;
	int iStart;
	int iEnd;
	int iLine;
	int iRet;
	if ( pWidget == NULL || pData == NULL || pProxy == NULL || pDraw == NULL || tRect.fW <= 0.0f || tRect.fH <= 0.0f ) return XUI_OK;
	pFont = __xuiMessageFont(pWidget, pData);
	/* Headless callers may intentionally omit a font; preserve the old no-op rendering behavior. */
	if ( pFont == NULL ) return XUI_OK;
	/* The bubble shrinks after measurement; paint and hit must retain that wrap. */
	if ( pLayout == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	tContent = xuiWidgetGetContentRect(pWidget);
	for ( iLine = __xuiMessageLowerBoundLineY(pLayout, tContent.fY - tRect.fY);
	      iLine < xuiTextLayoutGetLineCount(pLayout); iLine++ ) {
		XUI_MESSAGE_LIST_AUDIT_STEP(RenderLine);
		memset(&tLine, 0, sizeof(tLine));
		tLine.iSize = sizeof(tLine);
		if ( xuiTextLayoutGetLine(pLayout, iLine, &tLine) != XUI_OK ) return XUI_ERROR_INVALID_STATE;
		if ( tRect.fY + tLine.fY > tContent.fY + tContent.fH ) break;
		if ( tLine.iTextSize <= 0 ) continue;
		tLineRect = (xui_rect_t){tRect.fX + (bCenter ? __xuiMessageMax(0.0f, (tRect.fW - tLine.fW) * 0.5f) : 0.0f), tRect.fY + tLine.fY, bCenter ? tLine.fW : tRect.fW, tLine.fH};
		if ( iNodeIndex >= 0 && __xuiMessageGetTextSelectionForNode(pData, iNodeIndex, &iStart, &iEnd) ) {
			iStart = (iStart > tLine.iTextOffset) ? iStart : tLine.iTextOffset;
			iEnd = (iEnd < tLine.iTextOffset + tLine.iTextSize) ? iEnd : tLine.iTextOffset + tLine.iTextSize;
			if ( iEnd > iStart ) {
				xui_message_node_data_t* pNode = &pData->arrNodes[iNodeIndex];
				iRet = __xuiMessageEnsureLineCarets(pWidget, pData, pNode, pLayout, iLine, &tLine);
				if ( iRet != XUI_OK ) return iRet;
				fStart = __xuiMessageLineCaretX(pNode, &tLine, iStart);
				fEnd = __xuiMessageLineCaretX(pNode, &tLine, iEnd);
				tSelectionRect = (xui_rect_t){tLineRect.fX + fStart, tLineRect.fY, fEnd - fStart, tLineRect.fH};
				if ( fEnd > fStart ) (void)__xuiMessageDrawFill(pProxy, pDraw, tSelectionRect, iSelectionColor);
			}
		}
		iRet = xuiInternalTextLayoutGetDisplayLine(pLayout, iLine, &sDisplay, &iDisplaySize);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiMessageDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sDisplay, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLineRect, iColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiMessageHitDocument(xui_widget pWidget, xui_message_list_data_t* pData,
	int iIndex, double fWorldX, double fWorldY, int bClamp, xui_doc_position_t* pPosition)
{
	xui_message_node_data_t* pNode;
	xui_rect_t tWorld;
	xui_rect_t tContent;
	xui_rect_t tText;
	double fX, fY;
	int iRet;
	if ( pData == NULL || pPosition == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pNode = &pData->arrNodes[iIndex];
	if ( pNode->pDocumentBinding == NULL || !__xuiMessageNodeCanSelect(pNode) )
		return XUI_ERROR_NOT_FOUND;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tContent = xuiWidgetGetContentRect(pWidget);
	tText = pNode->tTextRect;
	fX = fWorldX - tWorld.fX - tContent.fX - tText.fX;
	fY = fWorldY - tWorld.fY - tContent.fY - tText.fY + pData->fScrollY;
	if ( !bClamp && (fX < 0 || fY < 0 || fX >= tText.fW || fY >= tText.fH ||
	     fWorldX < tWorld.fX + tContent.fX || fWorldY < tWorld.fY + tContent.fY ||
	     fWorldX >= tWorld.fX + tContent.fX + tContent.fW ||
	     fWorldY >= tWorld.fY + tContent.fY + tContent.fH) ) return XUI_ERROR_NOT_FOUND;
	if ( bClamp ) {
		fX = __xuiMessageClamp((float)fX, 0.0f, __xuiMessageMax(0.0f, tText.fW - 1.0f));
		fY = __xuiMessageClamp((float)fY, 0.0f, __xuiMessageMax(0.0f, tText.fH - 1.0f));
	}
	iRet = __xuiMessageDocumentSync(pNode->pDocumentBinding);
	return iRet == XUI_OK ? xuiDocumentRendererHitTest(pNode->pDocumentBinding->pRenderer,
		fX, fY, pPosition) : iRet;
}

static int __xuiMessageHitDocumentCell(xui_widget pWidget, xui_message_list_data_t* pData,
	int iIndex, double fWorldX, double fWorldY, xui_doc_cell_hit_t* pCell)
{
	xui_doc_position_t tPosition = {0};
	xui_rect_t tWorld, tContent, tText;
	int iRet;
	if ( pCell == NULL || pCell->iSize != sizeof(*pCell) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageHitDocument(pWidget, pData, iIndex, fWorldX, fWorldY, 0, &tPosition);
	if ( iRet != XUI_OK ) return iRet;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tContent = xuiWidgetGetContentRect(pWidget);
	tText = pData->arrNodes[iIndex].tTextRect;
	return xuiDocumentRendererHitTestCell(pData->arrNodes[iIndex].pDocumentBinding->pRenderer,
		fWorldX - tWorld.fX - tContent.fX - tText.fX,
		fWorldY - tWorld.fY - tContent.fY - tText.fY + pData->fScrollY, pCell);
}

static int __xuiMessageHitDocumentTaskMarker(xui_widget pWidget,
	xui_message_list_data_t* pData, int iIndex, double fWorldX,
	double fWorldY, xui_doc_node_id* pItem)
{
	xui_message_node_data_t* pNode;
	xui_rect_t tWorld, tContent, tText;
	double fX, fY;
	int iRet;
	if ( pData == NULL || pItem == NULL || iIndex < 0 ||
	     iIndex >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	*pItem = 0;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pNode = &pData->arrNodes[iIndex];
	if ( pNode->pDocumentBinding == NULL ) return XUI_ERROR_NOT_FOUND;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tContent = xuiWidgetGetContentRect(pWidget);
	tText = pNode->tTextRect;
	fX = fWorldX - tWorld.fX - tContent.fX - tText.fX;
	fY = fWorldY - tWorld.fY - tContent.fY - tText.fY + pData->fScrollY;
	if ( fX < 0 || fY < 0 || fX >= tText.fW || fY >= tText.fH ||
	     fWorldX < tWorld.fX + tContent.fX ||
	     fWorldY < tWorld.fY + tContent.fY ||
	     fWorldX >= tWorld.fX + tContent.fX + tContent.fW ||
	     fWorldY >= tWorld.fY + tContent.fY + tContent.fH )
		return XUI_ERROR_NOT_FOUND;
	iRet = __xuiMessageDocumentSync(pNode->pDocumentBinding);
	return iRet == XUI_OK ? xuiDocumentRendererHitTaskMarker(
		pNode->pDocumentBinding->pRenderer, fX, fY, pItem) : iRet;
}

static int __xuiMessageDrawDocument(xui_message_list_data_t* pData, int iIndex,
	xui_message_node_data_t* pNode, xui_draw_context pDraw, xui_rect_t tRect, xui_rect_t tViewport,
	uint32_t iDefaultSelectionColor)
{
	xui_message_document_binding_t* pBinding = pNode->pDocumentBinding;
	xui_rect_t tClip;
	xui_doc_range_t tSelection;
	const xui_doc_range_t* pSelection = NULL;
	uint32_t iSelectionColor;
	int iRet;
	if ( pBinding == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tClip.fX = __xuiMessageMax(tRect.fX, tViewport.fX);
	tClip.fY = __xuiMessageMax(tRect.fY, tViewport.fY);
	tClip.fW = __xuiMessageMax(0.0f, __xuiMessageMin(tRect.fX + tRect.fW, tViewport.fX + tViewport.fW) - tClip.fX);
	tClip.fH = __xuiMessageMax(0.0f, __xuiMessageMin(tRect.fY + tRect.fH, tViewport.fY + tViewport.fH) - tClip.fY);
	if ( tClip.fW <= 0.0f || tClip.fH <= 0.0f ) return XUI_OK;
	iRet = __xuiMessageDocumentSync(pBinding);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDocumentRendererLayout(pBinding->pRenderer, __xuiMessageMax(1.0f, tRect.fW),
		__xuiMessageMax(0.0f, tClip.fY - tRect.fY), tClip.fH);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiMessageDocumentSelectionRange(pData, iIndex, &tSelection) == XUI_OK )
		pSelection = &tSelection;
	iSelectionColor = pBinding->tDesc.iSelectionColor ? pBinding->tDesc.iSelectionColor : iDefaultSelectionColor;
	if ( pData->iTableSelectionNode == iIndex && pData->tTableSelection.iTableId ) {
		pBinding->pRenderer->table_selection = pData->tTableSelection;
		pBinding->pRenderer->table_selection_color = iSelectionColor;
		pSelection = NULL;
	}
	iRet = xuiDocumentRendererDraw(pBinding->pRenderer, pDraw, tRect.fX, tRect.fY,
		tClip, pSelection, iSelectionColor);
	memset(&pBinding->pRenderer->table_selection, 0,
		sizeof(pBinding->pRenderer->table_selection));
	return iRet;
}

static int __xuiMessageCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_message_list_data_t* pData;
	xui_message_paint_t tPaint;
	xui_message_node_data_t* pNode;
	xui_message_node_t tPublicNode;
	xui_proxy pProxy;
	xui_font pFont;
	xui_rect_t tContent;
	xui_rect_t tNode;
	xui_rect_t tBubble;
	xui_rect_t tText;
	xui_rect_t tHeader;
	xui_rect_t tAvatar;
	xui_rect_t tMeta;
	uint32_t iBubbleColor;
	uint32_t iAvatarColor;
	uint32_t iTextColor;
	int i;
	int iHandled;
	int iRet;
	(void)iStateId;
	(void)pUser;
	pData = __xuiMessageListGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiMessageResolvePaint(pWidget, pData, &tPaint);
	pFont = __xuiMessageFont(pWidget, pData);
	tContent = xuiWidgetGetContentRect(pWidget);
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiMessageDrawFill(pProxy, pDraw, tContent, tPaint.tColors.iBackgroundColor);
	for ( i = __xuiMessageLowerBoundY(pData, pData->fScrollY); i < pData->iNodeCount; i++ ) {
		XUI_MESSAGE_LIST_AUDIT_STEP(RenderNode);
		pNode = &pData->arrNodes[i];
		if ( pNode->tNodeRect.fY > pData->fScrollY + tContent.fH ) break;
		tNode = pNode->tNodeRect;
		tNode.fX += tContent.fX;
		tNode.fY += tContent.fY - pData->fScrollY;
		if ( tNode.fY + tNode.fH < tContent.fY || tNode.fY > tContent.fY + tContent.fH ) continue;
		tPublicNode = __xuiMessagePublicNode(pNode);
		if ( pData->onRenderNode != NULL ) {
			iHandled = pData->onRenderNode(pWidget, i, &tPublicNode, pDraw, tNode, (i == pData->iSelected ? XUI_WIDGET_STATE_ACTIVE : 0) | (i == pData->iHover ? XUI_WIDGET_STATE_HOVER : 0), pData->pRenderNodeUser);
			if ( iHandled < 0 ) return iHandled;
			if ( iHandled ) continue;
		}
		if ( i == pData->iHover ) (void)__xuiMessageDrawFill(pProxy, pDraw, tNode, tPaint.tColors.iHoverColor);
		if ( i == pData->iSelected ) (void)__xuiMessageDrawFill(pProxy, pDraw, tNode, tPaint.tColors.iSelectedColor);
		tBubble = pNode->tBubbleRect;
		tBubble.fX += tContent.fX;
		tBubble.fY += tContent.fY - pData->fScrollY;
		if ( pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) {
			(void)__xuiMessageDrawRectFill(pProxy, pDraw, tBubble, tPaint.tColors.iSystemBubbleColor);
			tText = pNode->tTextRect;
			tText.fX += tContent.fX;
			tText.fY += tContent.fY - pData->fScrollY;
			iRet = pNode->pDocumentBinding != NULL ?
				__xuiMessageDrawDocument(pData, i, pNode, pDraw, tText, tContent, tPaint.iTextSelectionColor) :
				__xuiMessageDrawWrappedText(pWidget, pData, pProxy, pDraw, pNode->pTextLayout, i, tText, tPaint.tColors.iSystemTextColor, 1, tPaint.iTextSelectionColor);
			if ( iRet != XUI_OK ) return iRet;
			continue;
		}
		if ( pNode->iType == XUI_MESSAGE_NODE_AUXILIARY ) {
			(void)__xuiMessageDrawRectFill(pProxy, pDraw, tBubble, tPaint.iAuxiliaryColor);
			(void)__xuiMessageDrawRectStroke(pProxy, pDraw, tBubble, 1.0f, tPaint.tColors.iBorderColor);
			tHeader = pNode->tHeaderRect;
			tHeader.fX += tContent.fX;
			tHeader.fY += tContent.fY - pData->fScrollY;
			(void)__xuiMessageDrawFill(pProxy, pDraw, tHeader, tPaint.iAuxiliaryHeaderColor);
			(void)__xuiMessageDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=(pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) ? ">" : "v", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){tHeader.fX + 7.0f, tHeader.fY, 12.0f, tHeader.fH}, tPaint.tColors.iMetaTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			tText = (xui_rect_t){tHeader.fX + 23.0f, tHeader.fY + (tHeader.fH - pNode->tMeasuredTitle.fY) * 0.5f,
				__xuiMessageMin(pNode->fTitleLayoutWidth, __xuiMessageMax(0.0f, tHeader.fW - 30.0f)), pNode->tMeasuredTitle.fY};
			iRet = __xuiMessageDrawWrappedText(pWidget, pData, pProxy, pDraw, pNode->pTitleLayout, -1, tText, tPaint.tColors.iMetaTextColor, 0, tPaint.iTextSelectionColor);
			if ( iRet != XUI_OK ) return iRet;
			if ( (pNode->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) == 0 ) {
				tText = pNode->tTextRect;
				tText.fX += tContent.fX;
				tText.fY += tContent.fY - pData->fScrollY;
				iRet = pNode->pDocumentBinding != NULL ?
					__xuiMessageDrawDocument(pData, i, pNode, pDraw, tText, tContent, tPaint.iTextSelectionColor) :
					__xuiMessageDrawWrappedText(pWidget, pData, pProxy, pDraw, pNode->pTextLayout, i, tText, tPaint.tColors.iOtherTextColor, 0, tPaint.iTextSelectionColor);
				if ( iRet != XUI_OK ) return iRet;
			}
			continue;
		}
		iBubbleColor = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? tPaint.tColors.iSelfBubbleColor : tPaint.tColors.iOtherBubbleColor;
		iTextColor = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? tPaint.tColors.iSelfTextColor : tPaint.tColors.iOtherTextColor;
		tAvatar.fY = tNode.fY + pData->tMetrics.fMetaHeight;
		tAvatar.fW = pData->tMetrics.fAvatarSize;
		tAvatar.fH = pData->tMetrics.fAvatarSize;
		tAvatar.fX = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? (tContent.fX + tContent.fW - pData->tMetrics.fPaddingX - pData->tMetrics.fAvatarSize) : (tContent.fX + pData->tMetrics.fPaddingX);
		iAvatarColor = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? tPaint.tColors.iAvatarSelfColor : tPaint.tColors.iAvatarOtherColor;
		if ( __xuiMessageAlpha(iAvatarColor) != 0 ) {
			if ( pProxy->drawCircleFill != NULL ) {
				(void)pProxy->drawCircleFill(pProxy, pDraw, tAvatar.fX + tAvatar.fW * 0.5f, tAvatar.fY + tAvatar.fH * 0.5f, pData->tMetrics.fAvatarSize * 0.5f, iAvatarColor);
			} else {
				(void)__xuiMessageDrawRectFill(pProxy, pDraw, tAvatar, iAvatarColor);
			}
		}
		tMeta.fY = tNode.fY;
		tMeta.fH = pData->tMetrics.fMetaHeight;
		tMeta.fX = tBubble.fX;
		tMeta.fW = tBubble.fW;
		if ( pNode->iType == XUI_MESSAGE_NODE_SELF ) {
			(void)__xuiMessageDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=__xuiMessageText(pNode->sSender), .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tMeta, tPaint.tColors.iMetaTextColor, XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		} else {
			(void)__xuiMessageDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=__xuiMessageText(pNode->sSender), .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tMeta, tPaint.tColors.iMetaTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
		(void)__xuiMessageDrawRectFill(pProxy, pDraw, tBubble, iBubbleColor);
		(void)__xuiMessageDrawRectStroke(pProxy, pDraw, tBubble, 1.0f, tPaint.tColors.iBorderColor);
		tText = pNode->tTextRect;
		tText.fX += tContent.fX;
		tText.fY += tContent.fY - pData->fScrollY;
		iRet = pNode->pDocumentBinding != NULL ?
			__xuiMessageDrawDocument(pData, i, pNode, pDraw, tText, tContent, tPaint.iTextSelectionColor) :
			__xuiMessageDrawWrappedText(pWidget, pData, pProxy, pDraw, pNode->pTextLayout, i, tText, iTextColor, 0, tPaint.iTextSelectionColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiMessageInitContextMenu(xui_widget pWidget, xui_message_list_data_t* pData)
{
	xui_menu_desc_t tDesc;
	int iRet;
	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pWidget;
	tDesc.pFont = __xuiMessageFont(pWidget, pData);
	iRet = xuiMenuCreate(xuiWidgetGetContext(pWidget), &pData->pContextMenu, &tDesc);
	if ( iRet != XUI_OK ) {
		pData->pContextMenu = NULL;
		return iRet;
	}
	return xuiMenuSetSelect(pData->pContextMenu, __xuiMessageContextMenuSelect, pWidget);
}

static int __xuiMessageInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_message_list_data_t* pData;
	const xui_message_list_desc_t* pDesc;
	int iRet;
	(void)pUser;
	pData = (xui_message_list_data_t*)pTypeData;
	pDesc = (const xui_message_list_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiMessageDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pData, 0, sizeof(*pData));
	pData->iNextAccessibleId = 1;
	__xuiMessageDefaultMetrics(&pData->tMetrics);
	__xuiMessageDefaultColors(&pData->tColors);
	pData->bUseDefaultFont = pDesc == NULL || pDesc->pFont == NULL;
	pData->pFont = pData->bUseDefaultFont ? xuiGetDefaultFont(xuiWidgetGetContext(pWidget)) : pDesc->pFont;
	pData->iResourceRegistryGeneration = xuiResourceGetRegistryGeneration(xuiWidgetGetContext(pWidget));
	if ( pDesc != NULL && pDesc->bHasMetrics ) {
		if ( !__xuiMessageMetricsValid(&pDesc->tMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
		pData->tMetrics = pDesc->tMetrics;
	}
	if ( pDesc != NULL && pDesc->bHasColors ) pData->tColors = pDesc->tColors;
	pData->iHover = -1;
	pData->iSelected = -1;
	pData->iSelectionAnchorNode = -1;
	pData->iSelectionActiveNode = -1;
	pData->iDocumentSelectionNode = -1;
	pData->iTableSelectionNode = -1;
	pData->bAutoScroll = (pDesc == NULL) ? 1 : (pDesc->bAutoScroll ? 1 : 0);
	if ( pDesc != NULL && pDesc->iNodeCount > 0 ) {
		iRet = xuiMessageListSetNodes(pWidget, pDesc->arrNodes, pDesc->iNodeCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	(void)xuiWidgetSetFocusable(pWidget, 1);
	(void)xuiWidgetSetTabStop(pWidget, 1);
	iRet = __xuiMessageInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	/* Clipboard remains available even if a platform cannot create a popup menu. */
	(void)__xuiMessageInitContextMenu(pWidget, pData);
	return xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiMessageAccessibleCount, __xuiMessageAccessibleGet,
		__xuiMessageAccessibleAction, pData);
}

static void __xuiMessageDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_message_list_data_t* pData;
	(void)pWidget;
	(void)pUser;
	pData = (xui_message_list_data_t*)pTypeData;
	if ( pData == NULL ) return;
	if ( pData->pContextMenu != NULL ) {
		xui_widget pPopup = xuiMenuGetPopupWidget(pData->pContextMenu);
		if ( pPopup != NULL ) xuiWidgetDestroy(pPopup);
		else xuiWidgetDestroy(pData->pContextMenu);
		pData->pContextMenu = NULL;
	}
	__xuiMessageClearData(pData);
	if ( pData->arrNodes != NULL ) xrtFree(pData->arrNodes);
	memset(pData, 0, sizeof(*pData));
}

static int __xuiMessageUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	uint64_t iGeneration;
	int iRet;
	(void)fDelta;
	(void)pUser;
	if ( pData == NULL || pData->iDocumentNodeCount == 0 ) return XUI_OK;
	iGeneration = xuiResourceGetRegistryGeneration(xuiWidgetGetContext(pWidget));
	if ( iGeneration == pData->iResourceRegistryGeneration ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) pData->iResourceRegistryGeneration = iGeneration;
	return iRet;
}

static void __xuiMessageDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_FILL;
	pLayout->iHeightMode = XUI_SIZE_FILL;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_HIDDEN;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiMessageDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

XUI_API xui_widget_type xuiMessageListGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "messagelist");
	if ( pType != NULL ) return pType;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "messagelist";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_message_list_data_t);
	tDesc.onInit = __xuiMessageInit;
	tDesc.onDestroy = __xuiMessageDestroy;
	tDesc.onUpdate = __xuiMessageUpdate;
	tDesc.onContentMeasure = __xuiMessageContentMeasure;
	tDesc.onCacheRender = __xuiMessageCacheRender;
	__xuiMessageDefaultLayout(&tDesc.tLayout);
	__xuiMessageDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiMessageRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiMessageListCreate(xui_context pContext, xui_widget* ppWidget, const xui_message_list_desc_t* pDesc)
{
	xui_widget_type pType;
	if ( (ppWidget == NULL) || !__xuiMessageDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	pType = xuiMessageListGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiMessageListSetEvent(xui_widget pWidget, xui_message_list_event_proc onEvent, void* pUser)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onEvent = onEvent;
	pData->pEventUser = pUser;
	return XUI_OK;
}

XUI_API int xuiMessageListSetNodeRenderer(xui_widget pWidget, xui_message_list_node_renderer_proc onRender, void* pUser)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onRenderNode = onRender;
	pData->pRenderNodeUser = pUser;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiMessageListSetNodes(xui_widget pWidget, const xui_message_node_t* pNodes, int iCount)
{
	xui_message_list_data_t* pData;
	int i;
	int iRet;
	if ( (iCount < 0) || (iCount > 0 && pNodes == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (uint64_t)iCount > UINT64_MAX - pData->iNextAccessibleId ) return XUI_DOC_ERROR_LIMIT;
	if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	__xuiMessageClearData(pData);
	iRet = __xuiMessageReserve(pData, iCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iCount; i++ ) {
		iRet = __xuiMessageCopyNode(&pData->arrNodes[i], &pNodes[i]);
		if ( iRet != XUI_OK ) {
			pData->iNodeCount = i;
			__xuiMessageClearData(pData);
			return iRet;
		}
		pData->arrNodes[i].iAccessibleId = ++pData->iNextAccessibleId;
	}
	pData->iNodeCount = iCount;
	if ( pData->bAutoScroll ) (void)xuiMessageListScrollToEnd(pWidget);
	iRet = __xuiMessageInvalidate(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return iRet;
}

XUI_API int xuiMessageListAddNode(xui_widget pWidget, const xui_message_node_t* pNode)
{
	xui_message_list_data_t* pData;
	int iRet;
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iNodeCount == INT_MAX ) return XUI_DOC_ERROR_LIMIT;
	if ( pData->iNextAccessibleId == UINT64_MAX ) return XUI_DOC_ERROR_LIMIT;
	iRet = __xuiMessageReserve(pData, pData->iNodeCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMessageCopyNode(&pData->arrNodes[pData->iNodeCount], pNode);
	if ( iRet != XUI_OK ) return iRet;
	pData->arrNodes[pData->iNodeCount].iAccessibleId = ++pData->iNextAccessibleId;
	pData->iNodeCount++;
	__xuiMessageDirtyNode(pData, pData->iNodeCount - 1);
	if ( pData->bAutoScroll ) (void)xuiMessageListScrollToEnd(pWidget);
	iRet = __xuiMessageInvalidate(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return iRet;
}

static int __xuiMessageFindNodeById(const xui_message_list_data_t* pData, const char* sId)
{
	int i;
	if ( pData == NULL || sId == NULL ) return -1;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		if ( strcmp(__xuiMessageText(pData->arrNodes[i].sId), sId) == 0 ) return i;
	}
	return -1;
}

XUI_API int xuiMessageListSetNodeDocument(xui_widget pWidget, const char* sId,
	const xui_message_document_desc_t* pDesc)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_message_document_binding_t* pBinding = NULL;
	xui_message_document_binding_t* pOld;
	xui_doc_renderer_desc_t tRendererDesc;
	xui_document_snapshot pSnapshot = NULL;
	float fOldScroll;
	int iOldChanges;
	int iIndex;
	int bSelectionAffected;
	int iRet;
	if ( pData == NULL || sId == NULL ||
	     (pDesc != NULL && (pDesc->iSize != sizeof(*pDesc) || pDesc->pDocument == NULL ||
	     (pDesc->tRenderer.iSize != 0 && pDesc->tRenderer.iSize != sizeof(pDesc->tRenderer)))) )
		return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 ) return XUI_ERROR_NOT_FOUND;
	if ( pDesc != NULL ) {
		pBinding = (xui_message_document_binding_t*)calloc(1, sizeof(*pBinding));
		if ( pBinding == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		pBinding->pList = pWidget;
		pBinding->iIndex = iIndex;
		pBinding->pDocument = pDesc->pDocument;
		pBinding->tDesc = *pDesc;
		xuiDocumentRetain(pBinding->pDocument);
		__xuiMessageDocumentRendererDesc(pBinding, pData, &tRendererDesc);
		pBinding->pResolvedFont = tRendererDesc.tFonts.normal;
		pBinding->iResolvedTextColor = tRendererDesc.iTextColor;
		iRet = xuiDocumentRendererCreate(xuiWidgetGetContext(pWidget), &tRendererDesc, &pBinding->pRenderer);
		if ( iRet == XUI_OK ) iRet = xuiDocumentAcquireSnapshot(pBinding->pDocument, &pSnapshot);
		if ( iRet == XUI_OK ) iRet = xuiDocumentRendererSetSnapshot(pBinding->pRenderer, pSnapshot, NULL);
		if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
		if ( iRet == XUI_OK ) iRet = xuiDocumentSubscribe(pBinding->pDocument,
			__xuiMessageDocumentChanged, pBinding, &pBinding->iSubscription);
		if ( iRet != XUI_OK ) { __xuiMessageFreeDocumentBinding(pBinding); return iRet; }
	}
	pOld = pData->arrNodes[iIndex].pDocumentBinding;
	fOldScroll = pData->fScrollY;
	iOldChanges = pData->iChangeCount;
	pData->arrNodes[iIndex].pDocumentBinding = pBinding;
	if ( pOld == NULL && pBinding != NULL ) pData->iDocumentNodeCount++;
	else if ( pOld != NULL && pBinding == NULL ) pData->iDocumentNodeCount--;
	__xuiMessageInvalidateNodeTextLayout(&pData->arrNodes[iIndex]);
	iRet = __xuiMessageInvalidateAfterNodeUpdate(pWidget, pData, iIndex);
	if ( iRet != XUI_OK ) {
		pData->arrNodes[iIndex].pDocumentBinding = pOld;
		if ( pOld == NULL && pBinding != NULL ) pData->iDocumentNodeCount--;
		else if ( pOld != NULL && pBinding == NULL ) pData->iDocumentNodeCount++;
		__xuiMessageFreeDocumentBinding(pBinding);
		__xuiMessageInvalidateNodeTextLayout(&pData->arrNodes[iIndex]);
		__xuiMessageDirtyNode(pData, iIndex);
		(void)__xuiMessageLayoutNodes(pWidget, pData);
		pData->fScrollY = fOldScroll;
		pData->iChangeCount = iOldChanges;
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return iRet;
	}
	if ( pData->iPressedTaskNode != 0 &&
	     pData->iPressedTaskMessage == iIndex ) {
		pData->iPressedTaskNode = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	bSelectionAffected = pData->iSelectionAnchorNode >= 0 && pData->iSelectionActiveNode >= 0 &&
	     ((iIndex >= pData->iSelectionAnchorNode && iIndex <= pData->iSelectionActiveNode) ||
	      (iIndex >= pData->iSelectionActiveNode && iIndex <= pData->iSelectionAnchorNode));
	if ( pData->iTableSelectionNode == iIndex ) bSelectionAffected = 1;
	if ( bSelectionAffected )
		__xuiMessageSetTextSelection(pData, -1, 0, -1, 0);
	if ( bSelectionAffected || pData->iDocumentSelectionNode == iIndex ) {
		pData->iDocumentSelectionNode = -1;
		pData->bSelecting = 0;
		pData->bDocumentSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( pOld != NULL && pData->tPendingDocumentAnchor.pBinding == pOld ) {
		xrtFree(pData->tPendingDocumentAnchor.arrMeasuredPrefix);
		memset(&pData->tPendingDocumentAnchor, 0,
			sizeof(pData->tPendingDocumentAnchor));
	}
	__xuiMessageAccessibleClearText(&pData->arrNodes[iIndex]);
	__xuiMessageFreeDocumentBinding(pOld);
	if ( bSelectionAffected ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return XUI_OK;
}

XUI_API xui_document xuiMessageListGetNodeDocument(xui_widget pWidget, const char* sId)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex = __xuiMessageFindNodeById(pData, sId);
	return iIndex < 0 || pData->arrNodes[iIndex].pDocumentBinding == NULL ? NULL :
		pData->arrNodes[iIndex].pDocumentBinding->pDocument;
}

XUI_API int xuiMessageListGetNodeDocumentRenderStats(xui_widget pWidget,
	const char* sId, xui_doc_renderer_stats_t* pStats)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	if ( pData == NULL || sId == NULL || pStats == NULL ||
	     pStats->iSize != sizeof(*pStats) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 || pData->arrNodes[iIndex].pDocumentBinding == NULL )
		return XUI_ERROR_NOT_FOUND;
	return xuiDocumentRendererGetStats(
		pData->arrNodes[iIndex].pDocumentBinding->pRenderer, pStats);
}

static int __xuiMessageInvalidateNodeDocumentLayout(xui_widget pWidget,
	const char* sId, int bFonts)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_message_document_anchor_t tAnchor;
	int iIndex;
	int iRet;
	if ( pData == NULL || sId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 || pData->arrNodes[iIndex].pDocumentBinding == NULL )
		return XUI_ERROR_NOT_FOUND;
	if ( pData->tPendingDocumentAnchor.pBinding == NULL ) {
		__xuiMessageCaptureDocumentAnchor(pWidget, pData,
			xuiWidgetGetContentRect(pWidget), &tAnchor, 1);
		if ( tAnchor.pBinding == pData->arrNodes[iIndex].pDocumentBinding )
			pData->tPendingDocumentAnchor = tAnchor;
		else xrtFree(tAnchor.arrMeasuredPrefix);
	}
	iRet = bFonts ? xuiDocumentRendererInvalidateFonts(
		pData->arrNodes[iIndex].pDocumentBinding->pRenderer) :
		xuiDocumentRendererInvalidateObjects(
			pData->arrNodes[iIndex].pDocumentBinding->pRenderer);
	if ( iRet != XUI_OK ) {
		xrtFree(pData->tPendingDocumentAnchor.arrMeasuredPrefix);
		memset(&pData->tPendingDocumentAnchor, 0, sizeof(pData->tPendingDocumentAnchor));
		return iRet;
	}
	__xuiMessageDirtyNode(pData, iIndex);
	pData->iChangeCount++;
	return xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiMessageListInvalidateNodeDocumentObjects(xui_widget pWidget,
	const char* sId)
{
	return __xuiMessageInvalidateNodeDocumentLayout(pWidget, sId, 0);
}

XUI_API int xuiMessageListInvalidateNodeDocumentFonts(xui_widget pWidget,
	const char* sId)
{
	return __xuiMessageInvalidateNodeDocumentLayout(pWidget, sId, 1);
}

XUI_API int xuiMessageListHitNodeDocument(xui_widget pWidget, const char* sId,
	double fWorldX, double fWorldY, xui_doc_position_t* pPosition)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 ) return XUI_ERROR_NOT_FOUND;
	return __xuiMessageHitDocument(pWidget, pData, iIndex, fWorldX, fWorldY, 0, pPosition);
}

XUI_API int xuiMessageListGetNodeDocumentSelection(xui_widget pWidget, const char* sId,
	xui_doc_range_t* pRange)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( pRange == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iIndex < 0 || pData->arrNodes[iIndex].pDocumentBinding == NULL ) return XUI_ERROR_NOT_FOUND;
	return __xuiMessageDocumentSelectionRange(pData, iIndex, pRange);
}

XUI_API int xuiMessageListSetNodeDocumentTableSelection(xui_widget pWidget,
	const char* sId, const xui_doc_table_selection_t* pSelection)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	if ( pData == NULL || sId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 ) return XUI_ERROR_NOT_FOUND;
	return __xuiMessageSetDocumentTableSelectionAt(pWidget, pData,
		iIndex, pSelection, 0);
}

XUI_API int xuiMessageListGetNodeDocumentTableSelection(xui_widget pWidget,
	const char* sId, xui_doc_table_selection_t* pResult)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	if ( pData == NULL || sId == NULL || pResult == NULL ||
	     pResult->iSize != sizeof(*pResult) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 || pData->arrNodes[iIndex].pDocumentBinding == NULL ||
	     pData->iTableSelectionNode != iIndex ||
	     pData->tTableSelection.iTableId == 0 ) return XUI_ERROR_NOT_FOUND;
	*pResult = pData->tTableSelection;
	return XUI_OK;
}

static int __xuiMessageAppendText(char** ppText, const char* sText)
{
	char* sNew;
	const char* sOld;
	size_t iOldLen;
	size_t iAddLen;
	if ( ppText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sOld = __xuiMessageText(*ppText);
	sText = __xuiMessageText(sText);
	iOldLen = strlen(sOld);
	iAddLen = strlen(sText);
	sNew = (char*)xrtMalloc(iOldLen + iAddLen + 1u);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memcpy(sNew, sOld, iOldLen);
	memcpy(sNew + iOldLen, sText, iAddLen + 1u);
	if ( *ppText != NULL ) xrtFree(*ppText);
	*ppText = sNew;
	return XUI_OK;
}

static int __xuiMessageInvalidateAfterNodeUpdate(xui_widget pWidget, xui_message_list_data_t* pData, int iIndex)
{
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMessageDirtyNode(pData, iIndex);
	iRet = __xuiMessageInvalidate(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->bAutoScroll ) return xuiMessageListScrollToEnd(pWidget);
	return XUI_OK;
}

static void __xuiMessageClampNodeSelection(xui_message_list_data_t* pData, int iIndex)
{
	const char* sText = __xuiMessageText(pData->arrNodes[iIndex].sText);
	int iLength = (int)strlen(sText);
	if ( pData->iSelectionAnchorNode == iIndex )
		pData->iSelectionAnchorOffset = xuiInternalTextGraphemeClamp(sText, iLength, pData->iSelectionAnchorOffset);
	if ( pData->iSelectionActiveNode == iIndex )
		pData->iSelectionActiveOffset = xuiInternalTextGraphemeClamp(sText, iLength, pData->iSelectionActiveOffset);
}

XUI_API int xuiMessageListUpdateNodeText(xui_widget pWidget, const char* sId, const char* sText)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	int iRet;
	if ( pData == NULL || sId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMessageInvalidateNodeTextLayout(&pData->arrNodes[iIndex]);
	iRet = __xuiMessageReplace(&pData->arrNodes[iIndex].sText, sText);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMessageClampNodeSelection(pData, iIndex);
	return __xuiMessageInvalidateAfterNodeUpdate(pWidget, pData, iIndex);
}

XUI_API int xuiMessageListAppendNodeText(xui_widget pWidget, const char* sId, const char* sText)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	int iRet;
	if ( pData == NULL || sId == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMessageInvalidateNodeTextLayout(&pData->arrNodes[iIndex]);
	iRet = __xuiMessageAppendText(&pData->arrNodes[iIndex].sText, sText);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMessageClampNodeSelection(pData, iIndex);
	return __xuiMessageInvalidateAfterNodeUpdate(pWidget, pData, iIndex);
}

XUI_API int xuiMessageListSetNodeTitle(xui_widget pWidget, const char* sId, const char* sTitle)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	int iRet;
	if ( pData == NULL || sId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageReplace(&pData->arrNodes[iIndex].sTitle, sTitle);
	if ( iRet != XUI_OK ) return iRet;
	pData->arrNodes[iIndex].sTitleLayoutSource = NULL;
	return __xuiMessageInvalidateAfterNodeUpdate(pWidget, pData, iIndex);
}

XUI_API int xuiMessageListSetNodeCollapsed(xui_widget pWidget, const char* sId, int bCollapsed)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex, iOld, iRet;
	if ( pData == NULL || sId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 || pData->arrNodes[iIndex].iType != XUI_MESSAGE_NODE_AUXILIARY ) return XUI_ERROR_INVALID_ARGUMENT;
	iOld = (pData->arrNodes[iIndex].iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) != 0;
	if ( bCollapsed ) pData->arrNodes[iIndex].iFlags |= XUI_MESSAGE_NODE_FLAG_COLLAPSED;
	else pData->arrNodes[iIndex].iFlags &= ~XUI_MESSAGE_NODE_FLAG_COLLAPSED;
	iRet = __xuiMessageInvalidateAfterNodeUpdate(pWidget, pData, iIndex);
	if ( iRet == XUI_OK && iOld != (bCollapsed != 0) )
		__xuiMessageAccessibleFoldChanged(pWidget, &pData->arrNodes[iIndex]);
	return iRet;
}

XUI_API int xuiMessageListGetNodeCollapsed(xui_widget pWidget, const char* sId)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iIndex;
	if ( pData == NULL || sId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiMessageFindNodeById(pData, sId);
	if ( iIndex < 0 || pData->arrNodes[iIndex].iType != XUI_MESSAGE_NODE_AUXILIARY ) return XUI_ERROR_INVALID_ARGUMENT;
	return (pData->arrNodes[iIndex].iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED) ? 1 : 0;
}

XUI_API int xuiMessageListClear(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMessageClearData(pData);
	iRet = __xuiMessageInvalidate(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return iRet;
}

XUI_API int xuiMessageListGetNodeCount(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->iNodeCount : 0;
}

XUI_API const xui_message_node_t* xuiMessageListGetNode(xui_widget pWidget, int iIndex)
{
	static xui_message_node_t tNode;
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return NULL;
	tNode = __xuiMessagePublicNode(&pData->arrNodes[iIndex]);
	return &tNode;
}

XUI_API int xuiMessageListSetSelected(xui_widget pWidget, int iIndex)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iIndex < -1 || iIndex >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iSelected = iIndex;
	pData->iSelectCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiMessageListGetSelected(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->iSelected : -1;
}

XUI_API int xuiMessageListGetHoverIndex(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->iHover : -1;
}

XUI_API int xuiMessageListGetNodeAt(xui_widget pWidget, float fX, float fY)
{
	return __xuiMessageGetIndexAtData(pWidget, __xuiMessageListGetData(pWidget), fX, fY);
}

XUI_API xui_rect_t xuiMessageListGetNodeRect(xui_widget pWidget, int iIndex)
{
	xui_rect_t tRect = {0.0f, 0.0f, 0.0f, 0.0f};
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return tRect;
	if ( __xuiMessageLayoutNodes(pWidget, pData) != XUI_OK ) return tRect;
	return pData->arrNodes[iIndex].tNodeRect;
}

XUI_API xui_rect_t xuiMessageListGetBubbleRect(xui_widget pWidget, int iIndex)
{
	xui_rect_t tRect = {0.0f, 0.0f, 0.0f, 0.0f};
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return tRect;
	if ( __xuiMessageLayoutNodes(pWidget, pData) != XUI_OK ) return tRect;
	return pData->arrNodes[iIndex].tBubbleRect;
}

XUI_API int xuiMessageListSetScroll(xui_widget pWidget, float fOffsetY)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_rect_t tContent;
	int iRet;
	if ( pData == NULL || fOffsetY != fOffsetY ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	tContent = xuiWidgetGetContentRect(pWidget);
	pData->fScrollY = __xuiMessageClamp(fOffsetY, 0.0f, __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH));
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiMessageListGetScroll(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->fScrollY : 0.0f;
}

XUI_API int xuiMessageListScrollBy(xui_widget pWidget, float fDeltaY)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiMessageListSetScroll(pWidget, pData->fScrollY + fDeltaY);
}

XUI_API int xuiMessageListEnsureVisible(xui_widget pWidget, int iIndex)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_rect_t tContent;
	xui_rect_t tRect;
	int iRet;
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	tContent = xuiWidgetGetContentRect(pWidget);
	tRect = pData->arrNodes[iIndex].tNodeRect;
	if ( tRect.fY < pData->fScrollY ) return xuiMessageListSetScroll(pWidget, tRect.fY);
	if ( tRect.fY + tRect.fH > pData->fScrollY + tContent.fH ) return xuiMessageListSetScroll(pWidget, tRect.fY + tRect.fH - tContent.fH);
	return XUI_OK;
}

XUI_API int xuiMessageListScrollToEnd(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_rect_t tContent;
	float fEnd;
	int iPass;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageLayoutNodes(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	tContent = xuiWidgetGetContentRect(pWidget);
	for ( iPass = 0; iPass < 8; iPass++ ) {
		fEnd = __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH);
		if ( fabsf(pData->fScrollY - fEnd) <= 0.01f ) break;
		pData->fScrollY = fEnd;
		iRet = __xuiMessageLayoutNodes(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	pData->fScrollY = __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiMessageListSetAutoScroll(xui_widget pWidget, int bAutoScroll)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bAutoScroll = bAutoScroll ? 1 : 0;
	return XUI_OK;
}

XUI_API int xuiMessageListGetAutoScroll(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->bAutoScroll : 0;
}

XUI_API int xuiMessageListSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->pFont = pFont;
	pData->bUseDefaultFont = pFont == NULL;
	pData->bLayoutValid = 0;
	return __xuiMessageInvalidate(pWidget, pData);
}

XUI_API xui_font xuiMessageListGetFont(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? __xuiMessageFont(pWidget, pData) : NULL;
}

XUI_API int xuiMessageListSetMetrics(xui_widget pWidget, const xui_message_list_metrics_t* pMetrics)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || !__xuiMessageMetricsValid(pMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->tMetrics = *pMetrics;
	pData->bLayoutValid = 0;
	return __xuiMessageInvalidate(pWidget, pData);
}

XUI_API int xuiMessageListGetMetrics(xui_widget pWidget, xui_message_list_metrics_t* pMetrics)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || pMetrics == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pMetrics = pData->tMetrics;
	return XUI_OK;
}

XUI_API int xuiMessageListSetColors(xui_widget pWidget, const xui_message_list_colors_t* pColors)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || pColors == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->tColors = *pColors;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiMessageListGetColors(xui_widget pWidget, xui_message_list_colors_t* pColors)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || pColors == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pColors = pData->tColors;
	return XUI_OK;
}

XUI_API int xuiMessageListClearTextSelection(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	int i;
	int bHadSelection;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bHadSelection = pData->iSelectionAnchorNode >= 0 || pData->iSelectionActiveNode >= 0 ||
		pData->iDocumentSelectionNode >= 0 || pData->iTableSelectionNode >= 0;
	__xuiMessageSetTextSelection(pData, -1, 0, -1, 0);
	pData->bSelecting = 0;
	pData->bDocumentSelecting = 0;
	pData->iDocumentSelectionNode = -1;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		if ( pData->arrNodes[i].pDocumentBinding != NULL )
			memset(&pData->arrNodes[i].pDocumentBinding->tSelection, 0,
				sizeof(pData->arrNodes[i].pDocumentBinding->tSelection));
	}
	if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	if ( bHadSelection ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiMessageBuildAnySelectedText(xui_message_list_data_t* pData, char** ppText)
{
	int i, iStart, iEnd;
	if ( pData == NULL || ppText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppText = NULL;
	if ( pData->tTableSelection.iTableId != 0 ) {
		xui_document_snapshot pSnapshot = NULL;
		char* sMatrix = NULL;
		char* sCopy;
		uint64_t iBytes = 0;
		xui_doc_table_selection_t* pSelected = &pData->tTableSelection;
		int iRet;
		if ( pData->iTableSelectionNode < 0 ||
		     pData->iTableSelectionNode >= pData->iNodeCount ||
		     pData->arrNodes[pData->iTableSelectionNode].pDocumentBinding == NULL )
			return XUI_ERROR_INVALID_STATE;
		iRet = xuiDocumentAcquireSnapshot(pData->arrNodes[
			pData->iTableSelectionNode].pDocumentBinding->pDocument, &pSnapshot);
		if ( iRet == XUI_OK ) iRet = xuiDocumentSnapshotCopyTableMatrix(pSnapshot,
			pSelected->iTableId, pSelected->iRow, pSelected->iColumn,
			pSelected->iRows, pSelected->iColumns, &sMatrix, &iBytes);
		if ( pSnapshot != NULL ) xuiDocumentSnapshotRelease(pSnapshot);
		if ( iRet != XUI_OK ) return iRet;
		if ( iBytes >= INT_MAX || memchr(sMatrix, 0, (size_t)iBytes) != NULL ) {
			xuiDocumentFreeBuffer(sMatrix);
			return iBytes >= INT_MAX ? XUI_DOC_ERROR_LIMIT : XUI_DOC_ERROR_UNREPRESENTABLE;
		}
		sCopy = (char*)xrtMalloc((size_t)iBytes + 1);
		if ( sCopy == NULL ) { xuiDocumentFreeBuffer(sMatrix); return XUI_ERROR_OUT_OF_MEMORY; }
		memcpy(sCopy, sMatrix, (size_t)iBytes);
		sCopy[iBytes] = 0;
		xuiDocumentFreeBuffer(sMatrix);
		*ppText = sCopy;
		return XUI_OK;
	}
	if ( pData->iSelectionAnchorNode >= 0 && pData->iSelectionActiveNode >= 0 &&
	     pData->iSelectionAnchorNode < pData->iNodeCount &&
	     pData->iSelectionActiveNode < pData->iNodeCount ) {
		iStart = pData->iSelectionAnchorNode < pData->iSelectionActiveNode ?
			pData->iSelectionAnchorNode : pData->iSelectionActiveNode;
		iEnd = pData->iSelectionAnchorNode > pData->iSelectionActiveNode ?
			pData->iSelectionAnchorNode : pData->iSelectionActiveNode;
		for ( i = iStart; i <= iEnd; i++ ) {
			if ( pData->arrNodes[i].pDocumentBinding != NULL )
				return __xuiMessageBuildMixedSelectedText(pData, ppText);
		}
	}
	return __xuiMessageBuildSelectedText(pData, ppText);
}

XUI_API int xuiMessageListGetSelectedText(xui_widget pWidget, char* sBuffer, int iCapacity)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	char* sText;
	int iNeed;
	if ( pData == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiMessageBuildAnySelectedText(pData, &sText) != XUI_OK ) {
		if ( sBuffer != NULL && iCapacity > 0 ) sBuffer[0] = 0;
		return 0;
	}
	iNeed = (int)strlen(sText) + 1;
	if ( sBuffer != NULL && iCapacity > 0 ) {
		strncpy(sBuffer, sText, (size_t)iCapacity - 1u);
		sBuffer[iCapacity - 1] = 0;
	}
	xrtFree(sText);
	return iNeed;
}

XUI_API int xuiMessageListCopySelection(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_proxy pProxy;
	char* sText;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageBuildAnySelectedText(pData, &sText);
	if ( iRet != XUI_OK ) return iRet;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL || pProxy->clipboardSetText == NULL ) {
		xrtFree(sText);
		return XUI_ERROR_UNSUPPORTED;
	}
	iRet = pProxy->clipboardSetText(pProxy, sText);
	xrtFree(sText);
	if ( iRet == XUI_OK ) (void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_COPY, pData->iSelected, NULL);
	return iRet;
}

static int __xuiMessageAppendEscaped(char* sBuffer, int iCapacity, int iPos, const char* sText)
{
	const unsigned char* p;
	if ( sText == NULL ) sText = "";
	for ( p = (const unsigned char*)sText; *p != 0; p++ ) {
		char ch = (char)*p;
		if ( ch == '\\' || ch == '\t' || ch == '\n' || ch == '\r' ) {
			if ( sBuffer != NULL && iPos + 2 < iCapacity ) {
				sBuffer[iPos] = '\\';
				sBuffer[iPos + 1] = (ch == '\t') ? 't' : ((ch == '\n') ? 'n' : ((ch == '\r') ? 'r' : '\\'));
			}
			iPos += 2;
		} else {
			if ( sBuffer != NULL && iPos + 1 < iCapacity ) sBuffer[iPos] = ch;
			iPos++;
		}
	}
	return iPos;
}

static int __xuiMessageAppendRaw(char* sBuffer, int iCapacity, int iPos, const char* sText)
{
	int iLen = (int)strlen(__xuiMessageText(sText));
	if ( sBuffer != NULL && iPos < iCapacity ) {
		int iCopy = iLen;
		if ( iCopy > iCapacity - iPos - 1 ) iCopy = iCapacity - iPos - 1;
		if ( iCopy > 0 ) memcpy(sBuffer + iPos, sText, (size_t)iCopy);
	}
	return iPos + iLen;
}

XUI_API int xuiMessageListExportText(xui_widget pWidget, char* sBuffer, int iCapacity)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_message_node_data_t* pNode;
	int i;
	int iPos;
	char sNum[64];
	if ( pData == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iPos = 0;
	iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "MESSAGELIST2\n");
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		pNode = &pData->arrNodes[i];
		sprintf(sNum, "N\t%d\t%d\t%d\t", pNode->iType, pNode->iFlags, pNode->iAuxiliaryKind);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, sNum);
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sId);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "\t");
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sParentId);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "\t");
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sTitle);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "\t");
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sSender);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "\t");
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sTime);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "\t");
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sText);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "\n");
	}
	if ( sBuffer != NULL && iCapacity > 0 ) {
		sBuffer[(iPos < iCapacity) ? iPos : (iCapacity - 1)] = 0;
	}
	return iPos + 1;
}

static char* __xuiMessageUnescapeField(const char* sStart, const char* sEnd)
{
	char* sOut;
	char* sWrite;
	const char* p;
	size_t iLen;
	if ( sStart == NULL || sEnd == NULL || sEnd < sStart ) return NULL;
	iLen = (size_t)(sEnd - sStart);
	sOut = (char*)xrtMalloc(iLen + 1u);
	if ( sOut == NULL ) return NULL;
	sWrite = sOut;
	for ( p = sStart; p < sEnd; p++ ) {
		if ( *p == '\\' && p + 1 < sEnd ) {
			p++;
			*sWrite++ = (*p == 't') ? '\t' : ((*p == 'n') ? '\n' : ((*p == 'r') ? '\r' : *p));
		} else {
			*sWrite++ = *p;
		}
	}
	*sWrite = 0;
	return sOut;
}

static int __xuiMessageParseIntField(const char* sStart, const char* sEnd, int* pValue)
{
	char sNumber[32];
	char* sParseEnd;
	long iValue;
	size_t iLength;
	if ( sStart == NULL || sEnd == NULL || pValue == NULL || sEnd <= sStart ) return XUI_ERROR_INVALID_ARGUMENT;
	iLength = (size_t)(sEnd - sStart);
	if ( iLength >= sizeof(sNumber) ) return XUI_ERROR_INVALID_ARGUMENT;
	memcpy(sNumber, sStart, iLength);
	sNumber[iLength] = 0;
	iValue = strtol(sNumber, &sParseEnd, 10);
	if ( sParseEnd == sNumber || *sParseEnd != 0 || iValue < INT_MIN || iValue > INT_MAX ) return XUI_ERROR_INVALID_ARGUMENT;
	*pValue = (int)iValue;
	return XUI_OK;
}

static void __xuiMessageDestroyTemporaryData(xui_message_list_data_t* pData)
{
	if ( pData == NULL ) return;
	__xuiMessageClearData(pData);
	if ( pData->arrNodes != NULL ) xrtFree(pData->arrNodes);
	pData->arrNodes = NULL;
	pData->iNodeCapacity = 0;
}

static int __xuiMessageAppendImportedNode(xui_message_list_data_t* pData, const xui_message_node_t* pNode)
{
	int iRet;
	iRet = __xuiMessageReserve(pData, pData->iNodeCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMessageCopyNode(&pData->arrNodes[pData->iNodeCount], pNode);
	if ( iRet != XUI_OK ) return iRet;
	pData->iNodeCount++;
	return XUI_OK;
}

XUI_API int xuiMessageListImportText(xui_widget pWidget, const char* sText)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_message_list_data_t tImported;
	xui_message_node_t tNode;
	xui_message_node_data_t* arrOldNodes;
	int iOldNodeCount;
	const char* p;
	const char* e;
	const char* n;
	const char* f[10];
	char* sId;
	char* sParentId;
	char* sTitle;
	char* sSender;
	char* sTime;
	char* sBody;
	int iField;
	int iRet;
	int bVersion2;
	if ( pData == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tImported, 0, sizeof(tImported));
	p = sText;
	bVersion2 = strncmp(p, "MESSAGELIST2", 12) == 0;
	if ( bVersion2 || strncmp(p, "MESSAGELIST1", 12) == 0 ) {
		e = strchr(p, '\n');
		p = (e != NULL) ? e + 1 : p + strlen(p);
	}
	while ( *p != 0 ) {
		e = strchr(p, '\n');
		if ( e == NULL ) e = p + strlen(p);
		n = (*e == '\n') ? e + 1 : e;
		if ( e > p && e[-1] == '\r' ) e--;
		if ( e > p && p[0] == 'N' && p[1] == '\t' ) {
			f[0] = p;
			iField = 1;
			for ( ; iField < (bVersion2 ? 10 : 7); iField++ ) {
				f[iField] = strchr(f[iField - 1] + 1, '\t');
				if ( f[iField] == NULL || f[iField] > e ) break;
			}
			if ( iField == (bVersion2 ? 10 : 7) ) {
				memset(&tNode, 0, sizeof(tNode));
				tNode.iSize = sizeof(tNode);
				iRet = __xuiMessageParseIntField(f[1] + 1, f[2], &tNode.iType);
				if ( iRet == XUI_OK ) iRet = __xuiMessageParseIntField(f[2] + 1, f[3], &tNode.iFlags);
				if ( iRet == XUI_OK && bVersion2 ) iRet = __xuiMessageParseIntField(f[3] + 1, f[4], &tNode.iAuxiliaryKind);
				if ( iRet != XUI_OK || !__xuiMessageNodeTypeValid(tNode.iType) ) {
					__xuiMessageDestroyTemporaryData(&tImported);
					return XUI_ERROR_INVALID_ARGUMENT;
				}
				sId = __xuiMessageUnescapeField(f[bVersion2 ? 4 : 3] + 1, f[bVersion2 ? 5 : 4]);
				sParentId = bVersion2 ? __xuiMessageUnescapeField(f[5] + 1, f[6]) : __xuiMessageDup("");
				sTitle = bVersion2 ? __xuiMessageUnescapeField(f[6] + 1, f[7]) : __xuiMessageDup("");
				sSender = __xuiMessageUnescapeField(f[bVersion2 ? 7 : 4] + 1, f[bVersion2 ? 8 : 5]);
				sTime = __xuiMessageUnescapeField(f[bVersion2 ? 8 : 5] + 1, f[bVersion2 ? 9 : 6]);
				sBody = __xuiMessageUnescapeField(f[bVersion2 ? 9 : 6] + 1, e);
				if ( sId == NULL || sParentId == NULL || sTitle == NULL || sSender == NULL || sTime == NULL || sBody == NULL ) {
					if ( sId != NULL ) xrtFree(sId);
					if ( sParentId != NULL ) xrtFree(sParentId);
					if ( sTitle != NULL ) xrtFree(sTitle);
					if ( sSender != NULL ) xrtFree(sSender);
					if ( sTime != NULL ) xrtFree(sTime);
					if ( sBody != NULL ) xrtFree(sBody);
					__xuiMessageDestroyTemporaryData(&tImported);
					return XUI_ERROR_OUT_OF_MEMORY;
				}
				tNode.sId = sId;
				tNode.sParentId = sParentId;
				tNode.sTitle = sTitle;
				tNode.sSender = sSender;
				tNode.sTime = sTime;
				tNode.sText = sBody;
				iRet = __xuiMessageAppendImportedNode(&tImported, &tNode);
				xrtFree(sId);
				xrtFree(sParentId);
				xrtFree(sTitle);
				xrtFree(sSender);
				xrtFree(sTime);
				xrtFree(sBody);
				if ( iRet != XUI_OK ) {
					__xuiMessageDestroyTemporaryData(&tImported);
					return iRet;
				}
			} else {
				__xuiMessageDestroyTemporaryData(&tImported);
				return XUI_ERROR_INVALID_ARGUMENT;
			}
		} else if ( e > p ) {
			__xuiMessageDestroyTemporaryData(&tImported);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		p = n;
	}
	if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget )
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	if ( (uint64_t)tImported.iNodeCount > UINT64_MAX - pData->iNextAccessibleId ) {
		__xuiMessageDestroyTemporaryData(&tImported);
		return XUI_DOC_ERROR_LIMIT;
	}
	arrOldNodes = pData->arrNodes;
	iOldNodeCount = pData->iNodeCount;
	xrtFree(pData->tPendingDocumentAnchor.arrMeasuredPrefix);
	memset(&pData->tPendingDocumentAnchor, 0,
		sizeof(pData->tPendingDocumentAnchor));
	pData->arrNodes = NULL;
	pData->iNodeCount = 0;
	pData->iDocumentNodeCount = 0;
	pData->iNodeCapacity = 0;
	while ( iOldNodeCount > 0 ) __xuiMessageFreeNode(&arrOldNodes[--iOldNodeCount]);
	if ( arrOldNodes != NULL ) xrtFree(arrOldNodes);
	pData->arrNodes = tImported.arrNodes;
	pData->iNodeCount = tImported.iNodeCount;
	pData->iNodeCapacity = tImported.iNodeCapacity;
	for ( iField = 0; iField < pData->iNodeCount; iField++ )
		pData->arrNodes[iField].iAccessibleId = ++pData->iNextAccessibleId;
	pData->iHover = -1;
	pData->iSelected = -1;
	pData->iSelectionAnchorNode = -1;
	pData->iSelectionAnchorOffset = 0;
	pData->iSelectionActiveNode = -1;
	pData->iSelectionActiveOffset = 0;
	memset(&pData->tSelectionAnchorDocument, 0, sizeof(pData->tSelectionAnchorDocument));
	memset(&pData->tSelectionActiveDocument, 0, sizeof(pData->tSelectionActiveDocument));
	pData->bSelecting = 0;
	pData->iDocumentSelectionNode = -1;
	pData->bDocumentSelecting = 0;
	memset(&pData->tTableSelection, 0, sizeof(pData->tTableSelection));
	memset(&pData->tTableDragAnchor, 0, sizeof(pData->tTableDragAnchor));
	memset(&pData->tTableDragFocus, 0, sizeof(pData->tTableDragFocus));
	pData->iTableSelectionNode = -1;
	pData->bTableSelecting = 0;
	pData->fScrollY = 0.0f;
	pData->fContentHeight = 0.0f;
	pData->bLayoutValid = 0;
	pData->iLayoutDirtyFrom = 0;
	pData->iLaidOutCount = 0;
	iRet = __xuiMessageInvalidate(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return iRet;
}

XUI_API int xuiMessageListSaveFile(xui_widget pWidget, const char* sPath)
{
	FILE* fp;
	char* sBuffer;
	int iNeed;
	int iRet;
	if ( sPath == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNeed = xuiMessageListExportText(pWidget, NULL, 0);
	if ( iNeed < 0 ) return iNeed;
	sBuffer = (char*)xrtMalloc((size_t)iNeed);
	if ( sBuffer == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiMessageListExportText(pWidget, sBuffer, iNeed);
	if ( iRet < 0 ) {
		xrtFree(sBuffer);
		return iRet;
	}
	fp = fopen(sPath, "wb");
	if ( fp == NULL ) {
		xrtFree(sBuffer);
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	if ( fwrite(sBuffer, 1u, strlen(sBuffer), fp) != strlen(sBuffer) || fclose(fp) != 0 ) {
		xrtFree(sBuffer);
		return XUI_ERROR;
	}
	xrtFree(sBuffer);
	return XUI_OK;
}

XUI_API int xuiMessageListLoadFile(xui_widget pWidget, const char* sPath)
{
	FILE* fp;
	char* sBuffer;
	long iSize;
	int iRet;
	if ( sPath == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	if ( fseek(fp, 0, SEEK_END) != 0 ) {
		fclose(fp);
		return XUI_ERROR;
	}
	iSize = ftell(fp);
	if ( iSize < 0 || iSize > INT_MAX || fseek(fp, 0, SEEK_SET) != 0 ) {
		fclose(fp);
		return XUI_ERROR;
	}
	sBuffer = (char*)xrtMalloc((size_t)iSize + 1u);
	if ( sBuffer == NULL ) {
		fclose(fp);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( fread(sBuffer, 1u, (size_t)iSize, fp) != (size_t)iSize ) {
		fclose(fp);
		xrtFree(sBuffer);
		return XUI_ERROR;
	}
	fclose(fp);
	sBuffer[iSize] = 0;
	iRet = xuiMessageListImportText(pWidget, sBuffer);
	xrtFree(sBuffer);
	return iRet;
}

XUI_API int xuiMessageListGetSelectCount(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->iSelectCount : 0;
}

XUI_API int xuiMessageListGetClickCount(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->iClickCount : 0;
}

XUI_API int xuiMessageListGetChangeCount(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
