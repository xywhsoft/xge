#include "ui_design_toolbox.h"
#include "ui_design_canvas.h"
#include "ui_design_registry.h"

#include <string.h>

#define UI_DESIGN_TOOLBOX_HIT_NONE 0
#define UI_DESIGN_TOOLBOX_HIT_POINTER 1
#define UI_DESIGN_TOOLBOX_HIT_CATEGORY 2
#define UI_DESIGN_TOOLBOX_HIT_TOOL 3

#define UI_DESIGN_TOOLBOX_MARGIN 8.0f
#define UI_DESIGN_TOOLBOX_TITLE_H 28.0f
#define UI_DESIGN_TOOLBOX_HEADER_H 24.0f
#define UI_DESIGN_TOOLBOX_ITEM_H 26.0f

typedef struct ui_design_toolbox_hit_t {
	int iKind;
	int iCategory;
	ui_design_node_type_t iType;
} ui_design_toolbox_hit_t;

static int __uiDesignToolboxRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX <= tRect.fX + tRect.fW) && (fY <= tRect.fY + tRect.fH);
}

static int __uiDesignToolboxCategoryCollapsed(ui_design_app_t* pApp, int iCategory)
{
	if ( (pApp == NULL) || (iCategory < 0) || (iCategory >= 32) ) return 0;
	return ((pApp->iToolboxCollapsedMask & (1u << (uint32_t)iCategory)) != 0u);
}

static void __uiDesignToolboxToggleCategory(ui_design_app_t* pApp, int iCategory)
{
	if ( (pApp == NULL) || (iCategory < 0) || (iCategory >= 32) ) return;
	pApp->iToolboxCollapsedMask ^= (1u << (uint32_t)iCategory);
}

static uint32_t __uiDesignToolboxIconColor(ui_design_node_type_t iType)
{
	switch ( iType ) {
	case UI_DESIGN_NODE_WIDGET: return XUI_COLOR_RGBA(87, 132, 177, 255);
	case UI_DESIGN_NODE_LABEL: return XUI_COLOR_RGBA(94, 109, 130, 255);
	case UI_DESIGN_NODE_BUTTON: return XUI_COLOR_RGBA(53, 131, 205, 255);
	case UI_DESIGN_NODE_CHECKBOX: return XUI_COLOR_RGBA(61, 150, 104, 255);
	case UI_DESIGN_NODE_RADIO: return XUI_COLOR_RGBA(186, 113, 45, 255);
	default: return XUI_COLOR_RGBA(104, 124, 148, 255);
	}
}

static int __uiDesignToolboxDrawText(ui_design_app_t* pApp, xui_draw_context pDraw, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText != NULL ? sText : "", tRect, iColor, iFlags | XUI_TEXT_CLIP);
}

static int __uiDesignToolboxHit(ui_design_app_t* pApp, float fWorldX, float fWorldY, ui_design_toolbox_hit_t* pHit)
{
	const ui_design_control_desc_t* pDesc;
	const char* sCategory;
	xui_rect_t tWorld;
	xui_rect_t tContent;
	xui_rect_t tRow;
	float fX;
	float fY;
	float y;
	int i;
	int c;

	if ( pHit != NULL ) memset(pHit, 0, sizeof(*pHit));
	if ( (pApp == NULL) || (pApp->pToolbox == NULL) || (pHit == NULL) ) return 0;
	tWorld = xuiWidgetGetWorldRect(pApp->pToolbox);
	tContent = xuiWidgetGetContentRect(pApp->pToolbox);
	fX = fWorldX - tWorld.fX;
	fY = fWorldY - tWorld.fY;
	y = UI_DESIGN_TOOLBOX_TITLE_H + 4.0f;
	tRow = (xui_rect_t){UI_DESIGN_TOOLBOX_MARGIN, y, tContent.fW - UI_DESIGN_TOOLBOX_MARGIN * 2.0f, UI_DESIGN_TOOLBOX_ITEM_H};
	if ( __uiDesignToolboxRectContains(tRow, fX, fY) ) {
		pHit->iKind = UI_DESIGN_TOOLBOX_HIT_POINTER;
		return 1;
	}
	y += UI_DESIGN_TOOLBOX_ITEM_H + 8.0f;
	for ( c = 0; c < uiDesignRegistryGetCategoryCount(); c++ ) {
		sCategory = uiDesignRegistryGetCategory(c);
		tRow = (xui_rect_t){0.0f, y, tContent.fW, UI_DESIGN_TOOLBOX_HEADER_H};
		if ( __uiDesignToolboxRectContains(tRow, fX, fY) ) {
			pHit->iKind = UI_DESIGN_TOOLBOX_HIT_CATEGORY;
			pHit->iCategory = c;
			return 1;
		}
		y += UI_DESIGN_TOOLBOX_HEADER_H;
		if ( __uiDesignToolboxCategoryCollapsed(pApp, c) ) continue;
		for ( i = 0; i < uiDesignRegistryGetCount(); i++ ) {
			pDesc = uiDesignRegistryGetAt(i);
			if ( (pDesc == NULL) || (strcmp(pDesc->sCategory, sCategory) != 0) ) continue;
			tRow = (xui_rect_t){UI_DESIGN_TOOLBOX_MARGIN, y, tContent.fW - UI_DESIGN_TOOLBOX_MARGIN * 2.0f, UI_DESIGN_TOOLBOX_ITEM_H};
			if ( __uiDesignToolboxRectContains(tRow, fX, fY) ) {
				pHit->iKind = UI_DESIGN_TOOLBOX_HIT_TOOL;
				pHit->iCategory = c;
				pHit->iType = pDesc->iType;
				return 1;
			}
			y += UI_DESIGN_TOOLBOX_ITEM_H;
		}
		y += 5.0f;
	}
	return 0;
}

static int __uiDesignToolboxRenderItem(ui_design_app_t* pApp, xui_draw_context pDraw, xui_rect_t tRow, const ui_design_control_desc_t* pDesc)
{
	xui_rect_t tIcon;
	xui_rect_t tText;
	uint32_t iFill;
	uint32_t iText;
	int iRet;

	iFill = XUI_COLOR_RGBA(255, 255, 255, 255);
	iText = XUI_COLOR_RGBA(37, 52, 72, 255);
	if ( (pDesc != NULL) && (pApp->iActiveTool == pDesc->iType) ) {
		iFill = XUI_COLOR_RGBA(221, 236, 252, 255);
	}
	iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRow, iFill);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pDesc != NULL) && (pApp->iActiveTool == pDesc->iType) ) {
		iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tRow, 1.0f, XUI_COLOR_RGBA(52, 124, 204, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	tIcon = (xui_rect_t){tRow.fX + 7.0f, tRow.fY + 6.0f, 14.0f, 14.0f};
	tText = (xui_rect_t){tRow.fX + 28.0f, tRow.fY, tRow.fW - 34.0f, tRow.fH};
	iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tIcon, (pDesc != NULL) ? __uiDesignToolboxIconColor(pDesc->iType) : XUI_COLOR_RGBA(90, 106, 128, 255));
	if ( iRet != XUI_OK ) return iRet;
	return __uiDesignToolboxDrawText(pApp, pDraw, (pDesc != NULL) ? pDesc->sTitle : "Pointer", tText, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
}

static int __uiDesignToolboxRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	const ui_design_control_desc_t* pDesc;
	const char* sCategory;
	xui_rect_t tContent;
	xui_rect_t tRow;
	xui_rect_t tGlyph;
	xui_rect_t tText;
	float y;
	int i;
	int c;
	int iRet;

	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pApp == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tContent = xuiWidgetGetContentRect(pWidget);
	iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tContent, XUI_COLOR_RGBA(246, 249, 253, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignToolboxDrawText(pApp, pDraw, "Toolbox", (xui_rect_t){UI_DESIGN_TOOLBOX_MARGIN, 3.0f, tContent.fW - UI_DESIGN_TOOLBOX_MARGIN * 2.0f, UI_DESIGN_TOOLBOX_TITLE_H - 4.0f}, XUI_COLOR_RGBA(35, 50, 70, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;
	y = UI_DESIGN_TOOLBOX_TITLE_H + 4.0f;
	tRow = (xui_rect_t){UI_DESIGN_TOOLBOX_MARGIN, y, tContent.fW - UI_DESIGN_TOOLBOX_MARGIN * 2.0f, UI_DESIGN_TOOLBOX_ITEM_H};
	iRet = __uiDesignToolboxRenderItem(pApp, pDraw, tRow, NULL);
	if ( iRet != XUI_OK ) return iRet;
	y += UI_DESIGN_TOOLBOX_ITEM_H + 8.0f;
	for ( c = 0; c < uiDesignRegistryGetCategoryCount(); c++ ) {
		sCategory = uiDesignRegistryGetCategory(c);
		tRow = (xui_rect_t){0.0f, y, tContent.fW, UI_DESIGN_TOOLBOX_HEADER_H};
		iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRow, XUI_COLOR_RGBA(229, 237, 247, 255));
		if ( iRet != XUI_OK ) return iRet;
		tGlyph = (xui_rect_t){9.0f, y, 16.0f, UI_DESIGN_TOOLBOX_HEADER_H};
		tText = (xui_rect_t){28.0f, y, tContent.fW - 36.0f, UI_DESIGN_TOOLBOX_HEADER_H};
		iRet = __uiDesignToolboxDrawText(pApp, pDraw, __uiDesignToolboxCategoryCollapsed(pApp, c) ? "+" : "-", tGlyph, XUI_COLOR_RGBA(60, 78, 99, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignToolboxDrawText(pApp, pDraw, sCategory, tText, XUI_COLOR_RGBA(52, 70, 91, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
		if ( iRet != XUI_OK ) return iRet;
		y += UI_DESIGN_TOOLBOX_HEADER_H;
		if ( __uiDesignToolboxCategoryCollapsed(pApp, c) ) continue;
		for ( i = 0; i < uiDesignRegistryGetCount(); i++ ) {
			pDesc = uiDesignRegistryGetAt(i);
			if ( (pDesc == NULL) || (strcmp(pDesc->sCategory, sCategory) != 0) ) continue;
			tRow = (xui_rect_t){UI_DESIGN_TOOLBOX_MARGIN, y, tContent.fW - UI_DESIGN_TOOLBOX_MARGIN * 2.0f, UI_DESIGN_TOOLBOX_ITEM_H};
			iRet = __uiDesignToolboxRenderItem(pApp, pDraw, tRow, pDesc);
			if ( iRet != XUI_OK ) return iRet;
			y += UI_DESIGN_TOOLBOX_ITEM_H;
		}
		y += 5.0f;
	}
	return XUI_OK;
}

static int __uiDesignToolboxEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	ui_design_app_t* pApp;
	ui_design_toolbox_hit_t tHit;

	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) || (pApp == NULL) ) return XUI_OK;
	if ( pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) return XUI_OK;
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		if ( !__uiDesignToolboxHit(pApp, pEvent->fX, pEvent->fY, &tHit) ) return XUI_OK;
		if ( tHit.iKind == UI_DESIGN_TOOLBOX_HIT_POINTER ) {
			pApp->iActiveTool = UI_DESIGN_NODE_NONE;
			pApp->bDraggingTool = 0;
			pApp->iDraggingTool = UI_DESIGN_NODE_NONE;
			uiDesignAppInvalidate(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( tHit.iKind == UI_DESIGN_TOOLBOX_HIT_CATEGORY ) {
			__uiDesignToolboxToggleCategory(pApp, tHit.iCategory);
			uiDesignAppInvalidate(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( tHit.iKind == UI_DESIGN_TOOLBOX_HIT_TOOL ) {
			pApp->iActiveTool = tHit.iType;
			pApp->iDraggingTool = tHit.iType;
			pApp->bDraggingTool = 1;
			pApp->fDragX = pEvent->fX;
			pApp->fDragY = pEvent->fY;
			(void)xuiSetPointerCapture(pApp->pContext, pWidget);
			uiDesignAppInvalidate(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_MOVE:
		if ( pApp->bDraggingTool ) {
			pApp->fDragX = pEvent->fX;
			pApp->fDragY = pEvent->fY;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT && pApp->bDraggingTool ) {
			(void)xuiReleasePointerCapture(pApp->pContext, pWidget);
			(void)uiDesignCanvasDropTool(pApp, pApp->iDraggingTool, pEvent->fX, pEvent->fY);
			pApp->bDraggingTool = 0;
			pApp->iDraggingTool = UI_DESIGN_NODE_NONE;
			uiDesignAppInvalidate(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		pApp->bDraggingTool = 0;
		pApp->iDraggingTool = UI_DESIGN_NODE_NONE;
		uiDesignAppInvalidate(pApp);
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

int uiDesignToolboxCreate(ui_design_app_t* pApp)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pToolbox);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(pApp->pToolbox, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(pApp->pToolbox, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pToolbox, __uiDesignToolboxRender, pApp);
	(void)xuiWidgetSetEventCallback(pApp->pToolbox, __uiDesignToolboxEvent, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pToolbox, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE, 1);
	return XUI_OK;
}
