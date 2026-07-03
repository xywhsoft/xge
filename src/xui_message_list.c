#include "xui_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct xui_message_node_data_t {
	char* sId;
	char* sSender;
	char* sTime;
	char* sText;
	int iType;
	int iFlags;
	void* pUser;
	xui_rect_t tNodeRect;
	xui_rect_t tBubbleRect;
} xui_message_node_data_t;

typedef struct xui_message_list_data_t {
	xui_message_node_data_t* arrNodes;
	int iNodeCount;
	int iNodeCapacity;
	xui_font pFont;
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
} xui_message_list_data_t;

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

static int __xuiMessageDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( (pProxy == NULL) || (pDraw == NULL) || (pFont == NULL) || (pProxy->drawText == NULL) || (sText == NULL) || (sText[0] == 0) ||
	     (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiMessageAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawText(pProxy, pDraw, pFont, sText, tRect, iColor, iFlags);
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
	return iType == XUI_MESSAGE_NODE_SELF || iType == XUI_MESSAGE_NODE_OTHER || iType == XUI_MESSAGE_NODE_SYSTEM;
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
	pMetrics->fBubbleMaxWidth = 360.0f;
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
	       __xuiMessageFloatValid(pMetrics->fBubbleMaxWidth) &&
	       __xuiMessageFloatValid(pMetrics->fBubblePaddingX) &&
	       __xuiMessageFloatValid(pMetrics->fBubblePaddingY) &&
	       __xuiMessageFloatValid(pMetrics->fSystemPaddingX) &&
	       __xuiMessageFloatValid(pMetrics->fSystemPaddingY) &&
	       __xuiMessageFloatValid(pMetrics->fMetaHeight) &&
	       __xuiMessageFloatValid(pMetrics->fMinBubbleHeight) &&
	       __xuiMessageFloatValid(pMetrics->fWheelStep);
}

static void __xuiMessageFreeNode(xui_message_node_data_t* pNode)
{
	if ( pNode == NULL ) return;
	if ( pNode->sId != NULL ) xrtFree(pNode->sId);
	if ( pNode->sSender != NULL ) xrtFree(pNode->sSender);
	if ( pNode->sTime != NULL ) xrtFree(pNode->sTime);
	if ( pNode->sText != NULL ) xrtFree(pNode->sText);
	memset(pNode, 0, sizeof(*pNode));
}

static int __xuiMessageCopyNode(xui_message_node_data_t* pDst, const xui_message_node_t* pSrc)
{
	int iType;
	int iRet;
	if ( (pDst == NULL) || (pSrc == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pDst, 0, sizeof(*pDst));
	iType = __xuiMessageNodeTypeValid(pSrc->iType) ? pSrc->iType : XUI_MESSAGE_NODE_OTHER;
	pDst->iType = iType;
	pDst->iFlags = pSrc->iFlags;
	pDst->pUser = pSrc->pUser;
	iRet = __xuiMessageReplace(&pDst->sId, pSrc->sId);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sSender, pSrc->sSender);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sTime, pSrc->sTime);
	if ( iRet == XUI_OK ) iRet = __xuiMessageReplace(&pDst->sText, pSrc->sText);
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
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		__xuiMessageFreeNode(&pData->arrNodes[i]);
	}
	pData->iNodeCount = 0;
	pData->iHover = -1;
	pData->iSelected = -1;
	pData->fScrollY = 0.0f;
	pData->fContentHeight = 0.0f;
}

static xui_font __xuiMessageFont(xui_widget pWidget, xui_message_list_data_t* pData)
{
	if ( pData == NULL ) return NULL;
	return (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
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
	return tNode;
}

static float __xuiMessageTextWidth(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_proxy pProxy;
	xui_vec2_t tSize;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) ) {
		tSize = (xui_vec2_t){0.0f, 0.0f};
		if ( pProxy->textMeasure(pProxy, pFont, __xuiMessageText(sText), &tSize) == XUI_OK ) {
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

static float __xuiMessageEstimateBubbleHeight(xui_widget pWidget, xui_message_list_data_t* pData, const char* sText, float fMaxTextWidth)
{
	xui_font pFont;
	float fLineHeight;
	float fWidth;
	int iLines;
	if ( pData == NULL ) return 0.0f;
	pFont = __xuiMessageFont(pWidget, pData);
	fLineHeight = __xuiMessageLineHeight(xuiWidgetGetContext(pWidget), pFont);
	fWidth = __xuiMessageTextWidth(pWidget, pFont, sText);
	iLines = 1;
	if ( fMaxTextWidth > 1.0f && fWidth > fMaxTextWidth ) {
		iLines = (int)((fWidth + fMaxTextWidth - 1.0f) / fMaxTextWidth);
	}
	return __xuiMessageMax(pData->tMetrics.fMinBubbleHeight, (float)iLines * fLineHeight + pData->tMetrics.fBubblePaddingY * 2.0f);
}

static int __xuiMessageLayoutNodes(xui_widget pWidget, xui_message_list_data_t* pData)
{
	xui_rect_t tContent;
	xui_message_node_data_t* pNode;
	float fY;
	float fAvailable;
	float fMaxBubble;
	float fTextMax;
	float fBubbleW;
	float fBubbleH;
	float fTextW;
	float fRowH;
	int i;
	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tContent = xuiWidgetGetContentRect(pWidget);
	fAvailable = __xuiMessageMax(0.0f, tContent.fW - pData->tMetrics.fPaddingX * 2.0f);
	fMaxBubble = __xuiMessageMin(pData->tMetrics.fBubbleMaxWidth, fAvailable - pData->tMetrics.fAvatarSize - pData->tMetrics.fAvatarGap);
	if ( fMaxBubble < 80.0f ) fMaxBubble = __xuiMessageMax(40.0f, fAvailable);
	fTextMax = __xuiMessageMax(16.0f, fMaxBubble - pData->tMetrics.fBubblePaddingX * 2.0f);
	fY = pData->tMetrics.fPaddingY;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		pNode = &pData->arrNodes[i];
		memset(&pNode->tNodeRect, 0, sizeof(pNode->tNodeRect));
		memset(&pNode->tBubbleRect, 0, sizeof(pNode->tBubbleRect));
		if ( pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) {
			fTextW = __xuiMessageTextWidth(pWidget, __xuiMessageFont(pWidget, pData), pNode->sText);
			fBubbleW = __xuiMessageMin(fAvailable, fTextW + pData->tMetrics.fSystemPaddingX * 2.0f);
			fBubbleH = __xuiMessageLineHeight(xuiWidgetGetContext(pWidget), __xuiMessageFont(pWidget, pData)) + pData->tMetrics.fSystemPaddingY * 2.0f;
			pNode->tNodeRect = (xui_rect_t){0.0f, fY, tContent.fW, fBubbleH};
			pNode->tBubbleRect = (xui_rect_t){(tContent.fW - fBubbleW) * 0.5f, fY, fBubbleW, fBubbleH};
			fY += fBubbleH + pData->tMetrics.fNodeGap;
			continue;
		}
		fTextW = __xuiMessageTextWidth(pWidget, __xuiMessageFont(pWidget, pData), pNode->sText);
		fBubbleW = __xuiMessageMin(fMaxBubble, __xuiMessageMax(48.0f, fTextW + pData->tMetrics.fBubblePaddingX * 2.0f));
		fBubbleH = __xuiMessageEstimateBubbleHeight(pWidget, pData, pNode->sText, fTextMax);
		fRowH = __xuiMessageMax(pData->tMetrics.fAvatarSize, pData->tMetrics.fMetaHeight + fBubbleH);
		pNode->tNodeRect = (xui_rect_t){0.0f, fY, tContent.fW, fRowH};
		if ( pNode->iType == XUI_MESSAGE_NODE_SELF ) {
			pNode->tBubbleRect = (xui_rect_t){tContent.fW - pData->tMetrics.fPaddingX - pData->tMetrics.fAvatarSize - pData->tMetrics.fAvatarGap - fBubbleW, fY + pData->tMetrics.fMetaHeight, fBubbleW, fBubbleH};
		} else {
			pNode->tBubbleRect = (xui_rect_t){pData->tMetrics.fPaddingX + pData->tMetrics.fAvatarSize + pData->tMetrics.fAvatarGap, fY + pData->tMetrics.fMetaHeight, fBubbleW, fBubbleH};
		}
		fY += fRowH + pData->tMetrics.fNodeGap;
	}
	pData->fContentHeight = __xuiMessageMax(0.0f, fY - pData->tMetrics.fNodeGap + pData->tMetrics.fPaddingY);
	pData->fScrollY = __xuiMessageClamp(pData->fScrollY, 0.0f, __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH));
	return XUI_OK;
}

static int __xuiMessageInvalidate(xui_widget pWidget, xui_message_list_data_t* pData)
{
	if ( pData != NULL ) pData->iChangeCount++;
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
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
	float fLocalX;
	float fLocalY;
	int i;
	if ( (pWidget == NULL) || (pData == NULL) ) return -1;
	tContent = xuiWidgetGetContentRect(pWidget);
	fLocalX = fX - tContent.fX;
	fLocalY = fY - tContent.fY + pData->fScrollY;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		tRect = pData->arrNodes[i].tNodeRect;
		if ( fLocalX >= tRect.fX && fLocalX <= tRect.fX + tRect.fW && fLocalY >= tRect.fY && fLocalY <= tRect.fY + tRect.fH ) {
			return i;
		}
	}
	return -1;
}

static int __xuiMessageEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_message_list_data_t* pData;
	xui_rect_t tContent;
	float fMaxScroll;
	float fOldScroll;
	int iIndex;
	(void)pUser;
	pData = __xuiMessageListGetData(pWidget);
	if ( (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		iIndex = __xuiMessageGetIndexAtData(pWidget, pData, pEvent->fX, pEvent->fY);
		if ( iIndex != pData->iHover ) {
			pData->iHover = iIndex;
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_HOVER, iIndex, pEvent);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	} else if ( pEvent->iType == XUI_EVENT_POINTER_LEAVE ) {
		if ( pData->iHover != -1 ) {
			pData->iHover = -1;
			(void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_HOVER, -1, pEvent);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	} else if ( pEvent->iType == XUI_EVENT_POINTER_DOWN || pEvent->iType == XUI_EVENT_POINTER_CLICK || pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK || pEvent->iType == XUI_EVENT_CONTEXT_MENU ) {
		iIndex = __xuiMessageGetIndexAtData(pWidget, pData, pEvent->fX, pEvent->fY);
		if ( iIndex >= 0 ) {
			pData->iSelected = iIndex;
			pData->iSelectCount++;
			if ( pEvent->iType == XUI_EVENT_POINTER_CLICK ) pData->iClickCount++;
			if ( pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK ) pData->iClickCount++;
			if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) (void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_SELECT, iIndex, pEvent);
			if ( pEvent->iType == XUI_EVENT_POINTER_CLICK ) (void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_CLICK, iIndex, pEvent);
			if ( pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK ) (void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_DOUBLE_CLICK, iIndex, pEvent);
			if ( pEvent->iType == XUI_EVENT_CONTEXT_MENU ) (void)__xuiMessageNotify(pWidget, pData, XUI_MESSAGE_EVENT_CONTEXT_MENU, iIndex, pEvent);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
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
	}
	return XUI_OK;
}

static int __xuiMessageInitEvents(xui_widget pWidget)
{
	int iRet;
	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiMessageEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiMessageEvent, NULL);
	return iRet;
}

static int __xuiMessageContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_message_list_data_t* pData;
	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	pSize->fX = (tConstraint.fX > 0.0f && tConstraint.fX < XUI_LAYOUT_UNBOUNDED) ? tConstraint.fX : 360.0f;
	pSize->fY = __xuiMessageMin(__xuiMessageMax(160.0f, pData->fContentHeight), (tConstraint.fY > 0.0f) ? tConstraint.fY : XUI_LAYOUT_UNBOUNDED);
	return XUI_OK;
}

static int __xuiMessageCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_message_list_data_t* pData;
	xui_message_node_data_t* pNode;
	xui_message_node_t tPublicNode;
	xui_proxy pProxy;
	xui_font pFont;
	xui_rect_t tContent;
	xui_rect_t tNode;
	xui_rect_t tBubble;
	xui_rect_t tText;
	xui_rect_t tAvatar;
	xui_rect_t tMeta;
	uint32_t iBubbleColor;
	uint32_t iAvatarColor;
	uint32_t iTextColor;
	float fLineHeight;
	int i;
	int iHandled;
	int iRet;
	(void)iStateId;
	(void)pUser;
	pData = __xuiMessageListGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	pFont = __xuiMessageFont(pWidget, pData);
	tContent = xuiWidgetGetContentRect(pWidget);
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	(void)__xuiMessageDrawFill(pProxy, pDraw, tContent, pData->tColors.iBackgroundColor);
	fLineHeight = __xuiMessageLineHeight(xuiWidgetGetContext(pWidget), pFont);
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		pNode = &pData->arrNodes[i];
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
		if ( i == pData->iHover ) (void)__xuiMessageDrawFill(pProxy, pDraw, tNode, pData->tColors.iHoverColor);
		if ( i == pData->iSelected ) (void)__xuiMessageDrawFill(pProxy, pDraw, tNode, pData->tColors.iSelectedColor);
		tBubble = pNode->tBubbleRect;
		tBubble.fX += tContent.fX;
		tBubble.fY += tContent.fY - pData->fScrollY;
		if ( pNode->iType == XUI_MESSAGE_NODE_SYSTEM ) {
			(void)__xuiMessageDrawRectFill(pProxy, pDraw, tBubble, pData->tColors.iSystemBubbleColor);
			tText = tBubble;
			tText.fX += pData->tMetrics.fSystemPaddingX;
			tText.fW -= pData->tMetrics.fSystemPaddingX * 2.0f;
			(void)__xuiMessageDrawText(pProxy, pDraw, pFont, __xuiMessageText(pNode->sText), tText, pData->tColors.iSystemTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			continue;
		}
		iBubbleColor = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? pData->tColors.iSelfBubbleColor : pData->tColors.iOtherBubbleColor;
		iTextColor = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? pData->tColors.iSelfTextColor : pData->tColors.iOtherTextColor;
		tAvatar.fY = tNode.fY + pData->tMetrics.fMetaHeight;
		tAvatar.fW = pData->tMetrics.fAvatarSize;
		tAvatar.fH = pData->tMetrics.fAvatarSize;
		tAvatar.fX = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? (tContent.fX + tContent.fW - pData->tMetrics.fPaddingX - pData->tMetrics.fAvatarSize) : (tContent.fX + pData->tMetrics.fPaddingX);
		iAvatarColor = (pNode->iType == XUI_MESSAGE_NODE_SELF) ? pData->tColors.iAvatarSelfColor : pData->tColors.iAvatarOtherColor;
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
			(void)__xuiMessageDrawText(pProxy, pDraw, pFont, __xuiMessageText(pNode->sSender), tMeta, pData->tColors.iMetaTextColor, XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		} else {
			(void)__xuiMessageDrawText(pProxy, pDraw, pFont, __xuiMessageText(pNode->sSender), tMeta, pData->tColors.iMetaTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
		(void)__xuiMessageDrawRectFill(pProxy, pDraw, tBubble, iBubbleColor);
		(void)__xuiMessageDrawRectStroke(pProxy, pDraw, tBubble, 1.0f, pData->tColors.iBorderColor);
		tText = tBubble;
		tText.fX += pData->tMetrics.fBubblePaddingX;
		tText.fY += pData->tMetrics.fBubblePaddingY;
		tText.fW -= pData->tMetrics.fBubblePaddingX * 2.0f;
		tText.fH -= pData->tMetrics.fBubblePaddingY * 2.0f;
		iRet = __xuiMessageDrawText(pProxy, pDraw, pFont, __xuiMessageText(pNode->sText), tText, iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		(void)fLineHeight;
	}
	return XUI_OK;
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
	__xuiMessageDefaultMetrics(&pData->tMetrics);
	__xuiMessageDefaultColors(&pData->tColors);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( pDesc != NULL && pDesc->bHasMetrics ) {
		if ( !__xuiMessageMetricsValid(&pDesc->tMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
		pData->tMetrics = pDesc->tMetrics;
	}
	if ( pDesc != NULL && pDesc->bHasColors ) pData->tColors = pDesc->tColors;
	pData->iHover = -1;
	pData->iSelected = -1;
	pData->bAutoScroll = (pDesc == NULL) ? 1 : (pDesc->bAutoScroll ? 1 : 0);
	if ( pDesc != NULL && pDesc->iNodeCount > 0 ) {
		iRet = xuiMessageListSetNodes(pWidget, pDesc->arrNodes, pDesc->iNodeCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	(void)xuiWidgetSetFocusable(pWidget, 1);
	(void)xuiWidgetSetTabStop(pWidget, 1);
	return __xuiMessageInitEvents(pWidget);
}

static void __xuiMessageDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_message_list_data_t* pData;
	(void)pWidget;
	(void)pUser;
	pData = (xui_message_list_data_t*)pTypeData;
	if ( pData == NULL ) return;
	__xuiMessageClearData(pData);
	if ( pData->arrNodes != NULL ) xrtFree(pData->arrNodes);
	memset(pData, 0, sizeof(*pData));
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
	tDesc.onContentMeasure = __xuiMessageContentMeasure;
	tDesc.onCacheRender = __xuiMessageCacheRender;
	__xuiMessageDefaultLayout(&tDesc.tLayout);
	__xuiMessageDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
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
	}
	pData->iNodeCount = iCount;
	if ( pData->bAutoScroll ) (void)xuiMessageListScrollToEnd(pWidget);
	return __xuiMessageInvalidate(pWidget, pData);
}

XUI_API int xuiMessageListAddNode(xui_widget pWidget, const xui_message_node_t* pNode)
{
	xui_message_list_data_t* pData;
	int iRet;
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMessageReserve(pData, pData->iNodeCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMessageCopyNode(&pData->arrNodes[pData->iNodeCount], pNode);
	if ( iRet != XUI_OK ) return iRet;
	pData->iNodeCount++;
	if ( pData->bAutoScroll ) (void)xuiMessageListScrollToEnd(pWidget);
	return __xuiMessageInvalidate(pWidget, pData);
}

XUI_API int xuiMessageListClear(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMessageClearData(pData);
	return __xuiMessageInvalidate(pWidget, pData);
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
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	return pData->arrNodes[iIndex].tNodeRect;
}

XUI_API xui_rect_t xuiMessageListGetBubbleRect(xui_widget pWidget, int iIndex)
{
	xui_rect_t tRect = {0.0f, 0.0f, 0.0f, 0.0f};
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return tRect;
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	return pData->arrNodes[iIndex].tBubbleRect;
}

XUI_API int xuiMessageListSetScroll(xui_widget pWidget, float fOffsetY)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_rect_t tContent;
	if ( pData == NULL || fOffsetY != fOffsetY ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	tContent = xuiWidgetGetContentRect(pWidget);
	pData->fScrollY = __xuiMessageClamp(fOffsetY, 0.0f, __xuiMessageMax(0.0f, pData->fContentHeight - tContent.fH));
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
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iNodeCount ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)__xuiMessageLayoutNodes(pWidget, pData);
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
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)__xuiMessageLayoutNodes(pWidget, pData);
	tContent = xuiWidgetGetContentRect(pWidget);
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
	return __xuiMessageInvalidate(pWidget, pData);
}

XUI_API xui_font xuiMessageListGetFont(xui_widget pWidget)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiMessageListSetMetrics(xui_widget pWidget, const xui_message_list_metrics_t* pMetrics)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	if ( pData == NULL || !__xuiMessageMetricsValid(pMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->tMetrics = *pMetrics;
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
	iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, "MESSAGELIST1\n");
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		pNode = &pData->arrNodes[i];
		sprintf(sNum, "N\t%d\t%d\t", pNode->iType, pNode->iFlags);
		iPos = __xuiMessageAppendRaw(sBuffer, iCapacity, iPos, sNum);
		iPos = __xuiMessageAppendEscaped(sBuffer, iCapacity, iPos, pNode->sId);
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

XUI_API int xuiMessageListImportText(xui_widget pWidget, const char* sText)
{
	xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
	xui_message_node_t tNode;
	const char* p;
	const char* e;
	const char* f[7];
	char* sId;
	char* sSender;
	char* sTime;
	char* sBody;
	int iField;
	int iRet;
	if ( pData == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMessageClearData(pData);
	p = sText;
	if ( strncmp(p, "MESSAGELIST1", 12) == 0 ) {
		e = strchr(p, '\n');
		p = (e != NULL) ? e + 1 : p + strlen(p);
	}
	while ( *p != 0 ) {
		e = strchr(p, '\n');
		if ( e == NULL ) e = p + strlen(p);
		if ( e > p && p[0] == 'N' && p[1] == '\t' ) {
			f[0] = p;
			iField = 1;
			for ( ; iField < 7; iField++ ) {
				f[iField] = strchr(f[iField - 1] + 1, '\t');
				if ( f[iField] == NULL || f[iField] > e ) break;
			}
			if ( iField == 7 ) {
				sId = __xuiMessageUnescapeField(f[3] + 1, f[4]);
				sSender = __xuiMessageUnescapeField(f[4] + 1, f[5]);
				sTime = __xuiMessageUnescapeField(f[5] + 1, f[6]);
				sBody = __xuiMessageUnescapeField(f[6] + 1, e);
				if ( sId == NULL || sSender == NULL || sTime == NULL || sBody == NULL ) {
					if ( sId != NULL ) xrtFree(sId);
					if ( sSender != NULL ) xrtFree(sSender);
					if ( sTime != NULL ) xrtFree(sTime);
					if ( sBody != NULL ) xrtFree(sBody);
					return XUI_ERROR_OUT_OF_MEMORY;
				}
				memset(&tNode, 0, sizeof(tNode));
				tNode.iSize = sizeof(tNode);
				tNode.iType = atoi(f[1] + 1);
				tNode.iFlags = atoi(f[2] + 1);
				tNode.sId = sId;
				tNode.sSender = sSender;
				tNode.sTime = sTime;
				tNode.sText = sBody;
				iRet = xuiMessageListAddNode(pWidget, &tNode);
				xrtFree(sId);
				xrtFree(sSender);
				xrtFree(sTime);
				xrtFree(sBody);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		p = (*e == '\n') ? e + 1 : e;
	}
	return __xuiMessageInvalidate(pWidget, pData);
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
	fwrite(sBuffer, 1u, strlen(sBuffer), fp);
	fclose(fp);
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
	fseek(fp, 0, SEEK_END);
	iSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if ( iSize < 0 ) {
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
