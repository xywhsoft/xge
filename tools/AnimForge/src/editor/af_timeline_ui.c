/*
 * af_timeline_ui.c - AnimForge timeline panel implementation
 *
 * Custom-drawn timeline: layer list, frame ruler, keyframe dots,
 * tween spans, playhead. Handles pointer input for playhead scrubbing
 * and layer selection.
 */

#include "af_timeline_ui.h"
#include "af_shell.h"
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Colors                                                             */
/* ------------------------------------------------------------------ */

#define AF_TL_COLOR_BG         XUI_COLOR_RGBA(30, 30, 30, 255)
#define AF_TL_COLOR_LAYER_BG   XUI_COLOR_RGBA(37, 37, 38, 255)
#define AF_TL_COLOR_LAYER_SEL  XUI_COLOR_RGBA(0, 122, 204, 60)
#define AF_TL_COLOR_GRID       XUI_COLOR_RGBA(60, 60, 60, 255)
#define AF_TL_COLOR_GRID_5     XUI_COLOR_RGBA(80, 80, 80, 255)
#define AF_TL_COLOR_RULER_BG   XUI_COLOR_RGBA(45, 45, 48, 255)
#define AF_TL_COLOR_KEYFRAME   XUI_COLOR_RGBA(232, 168, 56, 255)
#define AF_TL_COLOR_TWEEN      XUI_COLOR_RGBA(0, 122, 204, 120)
#define AF_TL_COLOR_PLAYHEAD   XUI_COLOR_RGBA(255, 60, 60, 255)
#define AF_TL_COLOR_TEXT        XUI_COLOR_RGBA(200, 200, 200, 255)
#define AF_TL_COLOR_TEXT_DIM    XUI_COLOR_RGBA(120, 120, 120, 255)
#define AF_TL_COLOR_FRAME_SEL  XUI_COLOR_RGBA(0, 122, 204, 40)

/* ------------------------------------------------------------------ */
/* Render callback                                                    */
/* ------------------------------------------------------------------ */

static int __tlRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	af_timeline_ui_t* pTl = (af_timeline_ui_t*)pUser;
	af_app_t* pApp;
	xui_proxy_t* pProxy;
	xui_rect_t rc;
	int iLayerCount, iFrameCount;
	int i, f;
	float fContentX, fContentY;

	(void)iStateId;
	if ( pTl == NULL || pWidget == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp = pTl->pApp;
	if ( pApp == NULL ) return XUI_OK;
	pProxy = &pApp->tProxy;

	rc = xuiWidgetGetContentRect(pWidget);
	if ( rc.fW <= 0.0f || rc.fH <= 0.0f ) return XUI_OK;

	iLayerCount = pApp->tDoc.tTimeline.iLayerCount;
	iFrameCount = (int)pApp->tDoc.iFrameCount;

	/* Background */
	if ( pProxy->drawRectFill != NULL ) {
		pProxy->drawRectFill(pProxy, pDraw, rc, AF_TL_COLOR_BG);
	}

	fContentX = rc.fX + AF_TL_LAYER_WIDTH;
	fContentY = rc.fY + AF_TL_RULER_HEIGHT;

	/* ---- Frame ruler ---- */
	{
		xui_rect_t rulerRc = { rc.fX, rc.fY, rc.fW, AF_TL_RULER_HEIGHT };
		if ( pProxy->drawRectFill != NULL ) {
			pProxy->drawRectFill(pProxy, pDraw, rulerRc, AF_TL_COLOR_RULER_BG);
		}
		/* Frame numbers every 5 frames */
		if ( pProxy->drawText != NULL && pApp->pFontMono != NULL ) {
			for ( f = 0; f < iFrameCount; f += 5 ) {
				float fX = fContentX + (float)(f - pTl->iScrollX) * AF_TL_FRAME_WIDTH;
				if ( fX >= fContentX && fX < rc.fX + rc.fW ) {
					char sNum[16];
					xui_rect_t tTextRc;
					snprintf(sNum, sizeof(sNum), "%d", f);
					tTextRc.fX = fX + 2.0f;
					tTextRc.fY = rc.fY + 4.0f;
					tTextRc.fW = 40.0f;
					tTextRc.fH = 14.0f;
					pProxy->drawText(pProxy, pDraw, pApp->pFontMono, sNum, tTextRc, AF_TL_COLOR_TEXT_DIM, 0);
				}
			}
		}
		/* Ruler bottom border */
		if ( pProxy->drawLine != NULL ) {
			pProxy->drawLine(pProxy, pDraw, rc.fX, fContentY, rc.fX + rc.fW, fContentY, 1.0f, AF_TL_COLOR_GRID);
		}
	}

	/* ---- Layer list (left column) ---- */
	{
		xui_rect_t layerColRc = { rc.fX, fContentY, AF_TL_LAYER_WIDTH, rc.fH - AF_TL_RULER_HEIGHT };
		if ( pProxy->drawRectFill != NULL ) {
			pProxy->drawRectFill(pProxy, pDraw, layerColRc, AF_TL_COLOR_LAYER_BG);
		}
		for ( i = 0; i < iLayerCount; i++ ) {
			af_layer_t* pLayer = &pApp->tDoc.tTimeline.arrLayers[i];
			float fRowY = fContentY + (float)(i - pTl->iScrollY) * AF_TL_ROW_HEIGHT;
			if ( fRowY < fContentY - AF_TL_ROW_HEIGHT || fRowY > rc.fY + rc.fH ) continue;

			/* Selected layer highlight */
			if ( i == pTl->iSelectLayer && pProxy->drawRectFill != NULL ) {
				xui_rect_t selRc = { rc.fX, fRowY, AF_TL_LAYER_WIDTH, AF_TL_ROW_HEIGHT };
				pProxy->drawRectFill(pProxy, pDraw, selRc, AF_TL_COLOR_LAYER_SEL);
			}

			/* Layer name */
			if ( pProxy->drawText != NULL && pApp->pFontMono != NULL ) {
				xui_rect_t tNameRc;
				tNameRc.fX = rc.fX + 6.0f;
				tNameRc.fY = fRowY + 3.0f;
				tNameRc.fW = AF_TL_LAYER_WIDTH - AF_TL_ICON_WIDTH * 2 - 12.0f;
				tNameRc.fH = 16.0f;
				pProxy->drawText(pProxy, pDraw, pApp->pFontMono, pLayer->sName, tNameRc, AF_TL_COLOR_TEXT, 0);
			}

			/* Visibility icon (eye) */
			{
				float fIconX = rc.fX + AF_TL_LAYER_WIDTH - AF_TL_ICON_WIDTH * 2;
				xui_rect_t iconRc;
				uint32_t iIconColor = (pLayer->iFlags & AF_LAYER_FLAG_VISIBLE) ? AF_TL_COLOR_TEXT : AF_TL_COLOR_TEXT_DIM;
				iconRc.fX = fIconX + 3.0f;
				iconRc.fY = fRowY + 5.0f;
				iconRc.fW = 12.0f;
				iconRc.fH = 12.0f;
				if ( pProxy->drawRectFill != NULL ) {
					pProxy->drawRectFill(pProxy, pDraw, iconRc, iIconColor);
				}
			}
			/* Lock icon */
			{
				float fIconX = rc.fX + AF_TL_LAYER_WIDTH - AF_TL_ICON_WIDTH;
				xui_rect_t iconRc;
				uint32_t iIconColor = (pLayer->iFlags & AF_LAYER_FLAG_LOCKED) ? AF_TL_COLOR_KEYFRAME : AF_TL_COLOR_TEXT_DIM;
				iconRc.fX = fIconX + 4.0f;
				iconRc.fY = fRowY + 5.0f;
				iconRc.fW = 10.0f;
				iconRc.fH = 12.0f;
				if ( pProxy->drawRectFill != NULL ) {
					pProxy->drawRectFill(pProxy, pDraw, iconRc, iIconColor);
				}
			}

			/* Row separator */
			if ( pProxy->drawLine != NULL ) {
				pProxy->drawLine(pProxy, pDraw, rc.fX, fRowY + AF_TL_ROW_HEIGHT,
					rc.fX + rc.fW, fRowY + AF_TL_ROW_HEIGHT, 1.0f, AF_TL_COLOR_GRID);
			}
		}
		/* Column separator */
		if ( pProxy->drawLine != NULL ) {
			pProxy->drawLine(pProxy, pDraw, fContentX, rc.fY, fContentX, rc.fY + rc.fH, 1.0f, AF_TL_COLOR_GRID);
		}
	}

	/* ---- Frame grid + keyframes ---- */
	for ( i = 0; i < iLayerCount; i++ ) {
		af_layer_t* pLayer = &pApp->tDoc.tTimeline.arrLayers[i];
		float fRowY = fContentY + (float)(i - pTl->iScrollY) * AF_TL_ROW_HEIGHT;
		int k;

		if ( fRowY < fContentY - AF_TL_ROW_HEIGHT || fRowY > rc.fY + rc.fH ) continue;

		/* Vertical grid lines */
		if ( pProxy->drawLine != NULL ) {
			for ( f = 0; f < iFrameCount; f += 5 ) {
				float fX = fContentX + (float)(f - pTl->iScrollX) * AF_TL_FRAME_WIDTH;
				if ( fX >= fContentX && fX < rc.fX + rc.fW ) {
					uint32_t iCol = (f % 10 == 0) ? AF_TL_COLOR_GRID_5 : AF_TL_COLOR_GRID;
					pProxy->drawLine(pProxy, pDraw, fX, fRowY, fX, fRowY + AF_TL_ROW_HEIGHT, 1.0f, iCol);
				}
			}
		}

		/* Tween spans (blue bars between keyframes with tweens) */
		for ( k = 0; k < pLayer->iKeyframeCount - 1; k++ ) {
			af_keyframe_t* pKf = &pLayer->arrKeyframes[k];
			if ( pKf->iElementCount > 0 && pKf->arrElements[0].iTweenId != 0xFFFFFFFFu ) {
				af_keyframe_t* pKfNext = &pLayer->arrKeyframes[k + 1];
				float fX1 = fContentX + (float)((int)pKf->iFrame - pTl->iScrollX) * AF_TL_FRAME_WIDTH;
				float fX2 = fContentX + (float)((int)pKfNext->iFrame - pTl->iScrollX) * AF_TL_FRAME_WIDTH;
				if ( fX2 > fContentX && fX1 < rc.fX + rc.fW && pProxy->drawRectFill != NULL ) {
					xui_rect_t spanRc;
					spanRc.fX = fX1;
					spanRc.fY = fRowY + 6.0f;
					spanRc.fW = fX2 - fX1;
					spanRc.fH = AF_TL_ROW_HEIGHT - 12.0f;
					pProxy->drawRectFill(pProxy, pDraw, spanRc, AF_TL_COLOR_TWEEN);
				}
			}
		}

		/* Keyframe dots */
		for ( k = 0; k < pLayer->iKeyframeCount; k++ ) {
			af_keyframe_t* pKf = &pLayer->arrKeyframes[k];
			float fX = fContentX + (float)((int)pKf->iFrame - pTl->iScrollX) * AF_TL_FRAME_WIDTH + AF_TL_FRAME_WIDTH * 0.5f;
			float fY = fRowY + AF_TL_ROW_HEIGHT * 0.5f;
			if ( fX >= fContentX && fX < rc.fX + rc.fW ) {
				/* Draw diamond for keyframe */
				if ( pProxy->drawRectFill != NULL ) {
					xui_rect_t dotRc;
					dotRc.fX = fX - 3.0f;
					dotRc.fY = fY - 3.0f;
					dotRc.fW = 6.0f;
					dotRc.fH = 6.0f;
					pProxy->drawRectFill(pProxy, pDraw, dotRc, AF_TL_COLOR_KEYFRAME);
				}
			}
		}
	}

	/* ---- Frame selection highlight ---- */
	if ( pTl->iSelectFrameA >= 0 && pTl->iSelectFrameB >= 0 && pTl->iSelectFrameA != pTl->iSelectFrameB ) {
		int fA = (pTl->iSelectFrameA < pTl->iSelectFrameB) ? pTl->iSelectFrameA : pTl->iSelectFrameB;
		int fB = (pTl->iSelectFrameA < pTl->iSelectFrameB) ? pTl->iSelectFrameB : pTl->iSelectFrameA;
		float fX1 = fContentX + (float)(fA - pTl->iScrollX) * AF_TL_FRAME_WIDTH;
		float fX2 = fContentX + (float)(fB - pTl->iScrollX + 1) * AF_TL_FRAME_WIDTH;
		if ( pProxy->drawRectFill != NULL ) {
			xui_rect_t selRc;
			selRc.fX = fX1;
			selRc.fY = fContentY;
			selRc.fW = fX2 - fX1;
			selRc.fH = rc.fH - AF_TL_RULER_HEIGHT;
			pProxy->drawRectFill(pProxy, pDraw, selRc, AF_TL_COLOR_FRAME_SEL);
		}
	}

	/* ---- Playhead ---- */
	{
		float fPX = fContentX + (float)(pTl->iPlayheadFrame - pTl->iScrollX) * AF_TL_FRAME_WIDTH + AF_TL_FRAME_WIDTH * 0.5f;
		if ( fPX >= fContentX && fPX <= rc.fX + rc.fW && pProxy->drawLine != NULL ) {
			pProxy->drawLine(pProxy, pDraw, fPX, rc.fY, fPX, rc.fY + rc.fH, 2.0f, AF_TL_COLOR_PLAYHEAD);
		}
		/* Playhead handle (triangle at top) */
		if ( pProxy->drawRectFill != NULL ) {
			xui_rect_t handleRc;
			handleRc.fX = fPX - 4.0f;
			handleRc.fY = rc.fY;
			handleRc.fW = 8.0f;
			handleRc.fH = 8.0f;
			pProxy->drawRectFill(pProxy, pDraw, handleRc, AF_TL_COLOR_PLAYHEAD);
		}
	}

	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Context menu callbacks                                             */
/* ------------------------------------------------------------------ */

static void __tlCtxFrameSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	af_timeline_ui_t* pTl = (af_timeline_ui_t*)pUser;
	af_app_t* pApp;
	af_doc_t* pDoc;
	uint32_t iLayerId;

	(void)pMenu; (void)iIndex;
	if ( pTl == NULL || pTl->pApp == NULL ) return;
	pApp = pTl->pApp;
	pDoc = &pApp->tDoc;
	if ( pTl->iCtxLayer < 0 || pTl->iCtxLayer >= pDoc->tTimeline.iLayerCount ) return;
	iLayerId = pDoc->tTimeline.arrLayers[pTl->iCtxLayer].iId;

	switch ( iValue ) {
	case AF_TL_CTX_INSERT_KF:
		afDocSetKeyframe(pDoc, iLayerId, (uint32_t)pTl->iCtxFrame, 0);
		pDoc->bModified = 1;
		break;
	case AF_TL_CTX_INSERT_BLANK_KF:
		afDocSetKeyframe(pDoc, iLayerId, (uint32_t)pTl->iCtxFrame, AF_KF_BLANK);
		pDoc->bModified = 1;
		break;
	case AF_TL_CTX_CLEAR_KF:
		afDocClearKeyframe(pDoc, iLayerId, (uint32_t)pTl->iCtxFrame);
		pDoc->bModified = 1;
		break;
	case AF_TL_CTX_CREATE_TWEEN:
		/* TODO: create motion tween between keyframes */
		break;
	default:
		break;
	}
	xgeRenderRequest();
}

static void __tlCtxLayerSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	af_timeline_ui_t* pTl = (af_timeline_ui_t*)pUser;
	af_app_t* pApp;
	af_doc_t* pDoc;
	uint32_t iLayerId;
	char sName[64];

	(void)pMenu; (void)iIndex;
	if ( pTl == NULL || pTl->pApp == NULL ) return;
	pApp = pTl->pApp;
	pDoc = &pApp->tDoc;

	switch ( iValue ) {
	case AF_TL_CTX_NEW_LAYER:
		snprintf(sName, sizeof(sName), "Layer %d", pDoc->tTimeline.iLayerCount + 1);
		afDocAddLayer(pDoc, sName, AF_LAYER_NORMAL, NULL);
		pDoc->bModified = 1;
		break;
	case AF_TL_CTX_DELETE_LAYER:
		if ( pTl->iCtxLayer >= 0 && pTl->iCtxLayer < pDoc->tTimeline.iLayerCount ) {
			iLayerId = pDoc->tTimeline.arrLayers[pTl->iCtxLayer].iId;
			if ( pDoc->tTimeline.iLayerCount > 1 ) {
				afDocRemoveLayer(pDoc, iLayerId);
				pDoc->bModified = 1;
				if ( pTl->iSelectLayer >= pDoc->tTimeline.iLayerCount )
					pTl->iSelectLayer = pDoc->tTimeline.iLayerCount - 1;
			}
		}
		break;
	case AF_TL_CTX_RENAME_LAYER:
		if ( pTl->iCtxLayer >= 0 && pTl->iCtxLayer < pDoc->tTimeline.iLayerCount ) {
			af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[pTl->iCtxLayer];
			/* Simple rename: append _renamed suffix (full inline editing requires text input) */
			char sNewName[AF_DOC_MAX_NAME];
			snprintf(sNewName, sizeof(sNewName), "%s_r", pLayer->sName);
			strncpy(pLayer->sName, sNewName, AF_DOC_MAX_NAME - 1);
			pLayer->sName[AF_DOC_MAX_NAME - 1] = '\0';
			pDoc->bModified = 1;
		}
		break;
	case AF_TL_CTX_DUP_LAYER:
		if ( pTl->iCtxLayer >= 0 && pTl->iCtxLayer < pDoc->tTimeline.iLayerCount ) {
			af_layer_t* pSrcLayer = &pDoc->tTimeline.arrLayers[pTl->iCtxLayer];
			char sNewName[AF_DOC_MAX_NAME];
			uint32_t iNewId = 0;
			snprintf(sNewName, sizeof(sNewName), "%s_copy", pSrcLayer->sName);
			if ( afDocAddLayer(pDoc, sNewName, pSrcLayer->iType, &iNewId) == 0 ) {
				/* Copy keyframes from source layer */
				af_layer_t* pNewLayer = afDocFindLayer(pDoc, iNewId);
				if ( pNewLayer != NULL ) {
					int k;
					pNewLayer->iFlags = pSrcLayer->iFlags;
					pNewLayer->iColor = pSrcLayer->iColor;
					for ( k = 0; k < pSrcLayer->iKeyframeCount && k < pNewLayer->iKeyframeCount; k++ ) {
						pNewLayer->arrKeyframes[k] = pSrcLayer->arrKeyframes[k];
					}
					pNewLayer->iKeyframeCount = pSrcLayer->iKeyframeCount;
				}
				pDoc->bModified = 1;
			}
		}
		break;
	default:
		break;
	}
	xgeRenderRequest();
}

/* ------------------------------------------------------------------ */
/* Pointer event handler                                              */
/* ------------------------------------------------------------------ */

static int __tlPointerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	af_timeline_ui_t* pTl = (af_timeline_ui_t*)pUser;
	af_app_t* pApp;
	xui_rect_t rc;
	float fX, fY;
	int iFrame, iLayer;

	if ( pTl == NULL || pEvent == NULL ) return XUI_OK;
	pApp = pTl->pApp;
	if ( pApp == NULL ) return XUI_OK;

	rc = xuiWidgetGetContentRect(pWidget);
	fX = pEvent->fX;
	fY = pEvent->fY;

	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		/* Right-click: show context menu */
		if ( pEvent->iButton == XUI_POINTER_BUTTON_RIGHT ) {
			if ( fX < rc.fX + AF_TL_LAYER_WIDTH ) {
				/* Layer area context menu */
				iLayer = (int)((fY - rc.fY - AF_TL_RULER_HEIGHT) / AF_TL_ROW_HEIGHT) + pTl->iScrollY;
				pTl->iCtxLayer = iLayer;
				if ( pTl->pCtxLayer != NULL ) {
					xuiMenuOpenAt(pTl->pCtxLayer, pWidget, fX, fY);
				}
			} else {
				/* Frame area context menu */
				iLayer = (int)((fY - rc.fY - AF_TL_RULER_HEIGHT) / AF_TL_ROW_HEIGHT) + pTl->iScrollY;
				iFrame = (int)((fX - rc.fX - AF_TL_LAYER_WIDTH) / AF_TL_FRAME_WIDTH) + pTl->iScrollX;
				if ( iFrame < 0 ) iFrame = 0;
				pTl->iCtxLayer = iLayer;
				pTl->iCtxFrame = iFrame;
				if ( pTl->pCtxFrame != NULL ) {
					xuiMenuOpenAt(pTl->pCtxFrame, pWidget, fX, fY);
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}

		/* Left-click */
		if ( fX < rc.fX + AF_TL_LAYER_WIDTH ) {
			/* Check visibility/lock icon columns */
			float fIconX = rc.fX + AF_TL_LAYER_WIDTH - AF_TL_ICON_WIDTH * 2;
			iLayer = (int)((fY - rc.fY - AF_TL_RULER_HEIGHT) / AF_TL_ROW_HEIGHT) + pTl->iScrollY;
			if ( iLayer >= 0 && iLayer < pApp->tDoc.tTimeline.iLayerCount ) {
				af_layer_t* pLayer = &pApp->tDoc.tTimeline.arrLayers[iLayer];
				if ( fX >= fIconX && fX < fIconX + AF_TL_ICON_WIDTH ) {
					/* Toggle visibility */
					pLayer->iFlags ^= AF_LAYER_FLAG_VISIBLE;
					pApp->tDoc.bModified = 1;
					xgeRenderRequest();
					return XUI_EVENT_DISPATCH_STOP;
				} else if ( fX >= fIconX + AF_TL_ICON_WIDTH && fX < fIconX + AF_TL_ICON_WIDTH * 2 ) {
					/* Toggle lock */
					pLayer->iFlags ^= AF_LAYER_FLAG_LOCKED;
					pApp->tDoc.bModified = 1;
					xgeRenderRequest();
					return XUI_EVENT_DISPATCH_STOP;
				}
			}
			/* Layer selection */
			if ( iLayer >= 0 && iLayer < pApp->tDoc.tTimeline.iLayerCount ) {
				pTl->iSelectLayer = iLayer;
				pApp->iActiveLayerId = pApp->tDoc.tTimeline.arrLayers[iLayer].iId;
				xgeRenderRequest();
			}
		} else {
			/* Playhead scrubbing */
			iFrame = (int)((fX - rc.fX - AF_TL_LAYER_WIDTH) / AF_TL_FRAME_WIDTH) + pTl->iScrollX;
			if ( iFrame < 0 ) iFrame = 0;
			if ( iFrame >= (int)pApp->tDoc.iFrameCount ) iFrame = (int)pApp->tDoc.iFrameCount - 1;
			pTl->iPlayheadFrame = iFrame;
			pTl->bDraggingPlayhead = 1;
			pApp->iCurrentFrame = iFrame;
			xgeRenderRequest();
		}
		return XUI_EVENT_DISPATCH_STOP;

	case XUI_EVENT_POINTER_DOUBLE_CLICK:
		/* Double-click on frame area = insert keyframe */
		if ( fX >= rc.fX + AF_TL_LAYER_WIDTH ) {
			iLayer = (int)((fY - rc.fY - AF_TL_RULER_HEIGHT) / AF_TL_ROW_HEIGHT) + pTl->iScrollY;
			iFrame = (int)((fX - rc.fX - AF_TL_LAYER_WIDTH) / AF_TL_FRAME_WIDTH) + pTl->iScrollX;
			if ( iFrame < 0 ) iFrame = 0;
			if ( iLayer >= 0 && iLayer < pApp->tDoc.tTimeline.iLayerCount ) {
				uint32_t iLayerId = pApp->tDoc.tTimeline.arrLayers[iLayer].iId;
				afDocSetKeyframe(&pApp->tDoc, iLayerId, (uint32_t)iFrame, 0);
				pApp->tDoc.bModified = 1;
				xgeRenderRequest();
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;

	case XUI_EVENT_POINTER_MOVE:
		if ( pTl->bDraggingPlayhead ) {
			iFrame = (int)((fX - rc.fX - AF_TL_LAYER_WIDTH) / AF_TL_FRAME_WIDTH) + pTl->iScrollX;
			if ( iFrame < 0 ) iFrame = 0;
			if ( iFrame >= (int)pApp->tDoc.iFrameCount ) iFrame = (int)pApp->tDoc.iFrameCount - 1;
			if ( iFrame != pTl->iPlayheadFrame ) {
				pTl->iPlayheadFrame = iFrame;
				pApp->iCurrentFrame = iFrame;
				xgeRenderRequest();
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;

	case XUI_EVENT_POINTER_UP:
		if ( pTl->bDraggingPlayhead ) {
			pTl->bDraggingPlayhead = 0;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;

	case XUI_EVENT_POINTER_WHEEL:
		/* Scroll frames horizontally with wheel */
		pTl->iScrollX -= (int)(pEvent->fWheelY * 3.0f);
		if ( pTl->iScrollX < 0 ) pTl->iScrollX = 0;
		xgeRenderRequest();
		return XUI_EVENT_DISPATCH_STOP;

	default:
		break;
	}

	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

int afTimelineUiCreate(af_timeline_ui_t* pTl, af_app_t* pApp, xui_widget pParent)
{
	xui_canvas_desc_t tDesc;
	xui_widget pCanvas = NULL;
	xui_menu_desc_t menuDesc;
	xui_menu_item_t frameItems[4];
	xui_menu_item_t layerItems[4];
	int ret;

	if ( pTl == NULL || pApp == NULL ) return -1;
	memset(pTl, 0, sizeof(af_timeline_ui_t));
	pTl->pApp = pApp;
	pTl->iSelectLayer = -1;
	pTl->iSelectFrameA = -1;
	pTl->iSelectFrameB = -1;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fCanvasWidth = 2000.0f;
	tDesc.fCanvasHeight = 300.0f;
	tDesc.iPolicyX = 0;
	tDesc.iPolicyY = 0;
	tDesc.iClearColor = AF_TL_COLOR_BG;
	tDesc.iBackgroundColor = AF_TL_COLOR_BG;

	ret = xuiCanvasCreate(pApp->pContext, &pCanvas, &tDesc);
	if ( ret != XUI_OK || pCanvas == NULL ) return -2;

	pTl->pCanvas = pCanvas;

	/* Set render callback on the canvas frame widget */
	{
		xui_widget pFrame = xuiCanvasGetFrameWidget(pCanvas);
		if ( pFrame != NULL ) {
			xuiWidgetSetCacheRenderCallback(pFrame, __tlRender, pTl);
		}
	}

	/* Set pointer event handler */
	xuiWidgetSetEventCallback(pCanvas, __tlPointerEvent, pTl);

	/* Create frame context menu */
	memset(&menuDesc, 0, sizeof(menuDesc));
	menuDesc.iSize = sizeof(menuDesc);
	menuDesc.pOwner = pCanvas;
	menuDesc.pFont = pApp->pFont;
	ret = xuiMenuCreate(pApp->pContext, &pTl->pCtxFrame, &menuDesc);
	if ( ret == XUI_OK && pTl->pCtxFrame != NULL ) {
		memset(frameItems, 0, sizeof(frameItems));
		frameItems[0] = (xui_menu_item_t){"Insert Keyframe", "F6", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_INSERT_KF, 0, NULL, NULL};
		frameItems[1] = (xui_menu_item_t){"Insert Blank Keyframe", "F7", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_INSERT_BLANK_KF, 0, NULL, NULL};
		frameItems[2] = (xui_menu_item_t){"Clear Keyframe", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_CLEAR_KF, 0, NULL, NULL};
		frameItems[3] = (xui_menu_item_t){"Create Motion Tween", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_CREATE_TWEEN, 0, NULL, NULL};
		(void)xuiMenuSetItems(pTl->pCtxFrame, frameItems, 4);
		(void)xuiMenuSetSelect(pTl->pCtxFrame, __tlCtxFrameSelected, pTl);
	}

	/* Create layer context menu */
	ret = xuiMenuCreate(pApp->pContext, &pTl->pCtxLayer, &menuDesc);
	if ( ret == XUI_OK && pTl->pCtxLayer != NULL ) {
		memset(layerItems, 0, sizeof(layerItems));
		layerItems[0] = (xui_menu_item_t){"New Layer", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_NEW_LAYER, 0, NULL, NULL};
		layerItems[1] = (xui_menu_item_t){"Delete Layer", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_DELETE_LAYER, 0, NULL, NULL};
		layerItems[2] = (xui_menu_item_t){"Rename Layer", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_RENAME_LAYER, 0, NULL, NULL};
		layerItems[3] = (xui_menu_item_t){"Duplicate Layer", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_TL_CTX_DUP_LAYER, 0, NULL, NULL};
		(void)xuiMenuSetItems(pTl->pCtxLayer, layerItems, 4);
		(void)xuiMenuSetSelect(pTl->pCtxLayer, __tlCtxLayerSelected, pTl);
	}

	/* Add to parent */
	if ( pParent != NULL ) {
		xuiWidgetAddChild(pParent, pCanvas);
	}

	return 0;
}

void afTimelineUiSetFrame(af_timeline_ui_t* pTl, int iFrame)
{
	if ( pTl == NULL ) return;
	if ( pTl->iPlayheadFrame != iFrame ) {
		pTl->iPlayheadFrame = iFrame;
		/* Auto-scroll if playhead goes off screen */
		if ( iFrame < pTl->iScrollX ) {
			pTl->iScrollX = iFrame;
		}
		xgeRenderRequest();
	}
}

uint32_t afTimelineUiGetSelectedLayer(af_timeline_ui_t* pTl)
{
	if ( pTl == NULL || pTl->pApp == NULL ) return 0;
	if ( pTl->iSelectLayer < 0 ) return 0;
	if ( pTl->iSelectLayer >= pTl->pApp->tDoc.tTimeline.iLayerCount ) return 0;
	return pTl->pApp->tDoc.tTimeline.arrLayers[pTl->iSelectLayer].iId;
}
