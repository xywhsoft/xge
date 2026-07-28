/*
 * af_app.c - AnimForge application framework implementation
 *
 * Sets up XUI context, DockPanel workspace with stage/timeline/library/
 * property panels, and the main frame loop.
 */

#include "af_app.h"
#include "af_file.h"
#include "af_import.h"
#include "af_export.h"
#include "../anim/af_symbol.h"
#include "../editor/af_tools.h"
#include "../editor/af_timeline_ui.h"
#include "../editor/af_shell.h"
#include "../editor/af_dialogs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Font discovery                                                     */
/* ------------------------------------------------------------------ */

static const char* __afFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;
	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static const char* __afFindMonoTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\consola.ttf",
		"C:\\Windows\\Fonts\\cour.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf"
	};
	FILE* pFile;
	int i;
	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Stage canvas render callback                                       */
/* ------------------------------------------------------------------ */

static int __afStageRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	xui_rect_t stage;
	float fSW, fSH;
	(void)iStateId;
	if ( pWidget == NULL || pDraw == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	rc = xuiWidgetGetContentRect(pWidget);
	if ( rc.fW <= 0.0f || rc.fH <= 0.0f ) return XUI_OK;

	/* Fill workspace background (dark gray) */
	if ( pApp->tProxy.drawRectFill != NULL ) {
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, rc, AF_COLOR_STAGE_BG);
	}

	/* Compute stage rectangle in view */
	fSW = pApp->tDoc.fStageWidth * pApp->tView.fZoom;
	fSH = pApp->tDoc.fStageHeight * pApp->tView.fZoom;
	stage.fX = rc.fX + (rc.fW - fSW) * 0.5f + pApp->tView.fOffsetX;
	stage.fY = rc.fY + (rc.fH - fSH) * 0.5f + pApp->tView.fOffsetY;
	stage.fW = fSW;
	stage.fH = fSH;
	pApp->tStageRect = stage;  /* store for input conversion */

	/* Draw white stage */
	if ( pApp->tProxy.drawRectFill != NULL ) {
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, stage, AF_COLOR_STAGE);
	}

	/* Draw stage border */
	if ( pApp->tProxy.drawRectStroke != NULL ) {
		pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, stage, 1.0f, XUI_COLOR_RGBA(0, 0, 0, 255));
	}

	/* Draw grid if enabled */
	if ( pApp->tView.bShowGrid && pApp->tView.fGridSize > 0.0f ) {
		float fGrid = pApp->tView.fGridSize * pApp->tView.fZoom;
		float fX, fY;
		uint32_t iGridColor = XUI_COLOR_RGBA(200, 200, 200, 80);
		if ( fGrid >= 4.0f && pApp->tProxy.drawLine != NULL ) {
			for ( fX = stage.fX; fX <= stage.fX + stage.fW; fX += fGrid ) {
				pApp->tProxy.drawLine(&pApp->tProxy, pDraw, fX, stage.fY, fX, stage.fY + stage.fH, 1.0f, iGridColor);
			}
			for ( fY = stage.fY; fY <= stage.fY + stage.fH; fY += fGrid ) {
				pApp->tProxy.drawLine(&pApp->tProxy, pDraw, stage.fX, fY, stage.fX + stage.fW, fY, 1.0f, iGridColor);
			}
		}
	}

	/* Render document symbols using ShapeEx */
	{
		int iSym, iShape;
		xge_shape_ex_matrix_t tViewMatrix;
		
		/* Build view transform: translate to stage origin + scale by zoom */
		xgeShapeExMatrixIdentity(&tViewMatrix);
		xgeShapeExMatrixTranslate(&tViewMatrix, stage.fX, stage.fY);
		xgeShapeExMatrixScale(&tViewMatrix, pApp->tView.fZoom, pApp->tView.fZoom);
		
		for ( iSym = 0; iSym < pApp->tDoc.iSymbolCount; iSym++ ) {
			af_symbol_t* pSym = &pApp->tDoc.arrSymbols[iSym];
			
			for ( iShape = 0; iShape < pSym->iShapeCount; iShape++ ) {
				af_shape_record_t* pShapeRec = &pSym->arrShapes[iShape];
				af_path_t* pPath = &pShapeRec->tPath;
				xge_shape_ex pShape;
				int iCmd, iPt = 0;
				
				if ( pPath->iCommandCount == 0 ) continue;
				
				if ( xgeShapeExCreate(&pShape) != XGE_OK ) continue;
				
				/* Build path from commands */
				for ( iCmd = 0; iCmd < pPath->iCommandCount; iCmd++ ) {
					uint8_t cmd = pPath->arrCommands[iCmd];
					switch ( cmd ) {
					case AF_PATH_CMD_MOVE_TO:
						xgeShapeExMoveTo(pShape, pPath->arrPoints[iPt*2], pPath->arrPoints[iPt*2+1]);
						iPt++;
						break;
					case AF_PATH_CMD_LINE_TO:
						xgeShapeExLineTo(pShape, pPath->arrPoints[iPt*2], pPath->arrPoints[iPt*2+1]);
						iPt++;
						break;
					case AF_PATH_CMD_QUAD_TO:
						xgeShapeExQuadTo(pShape, 
							pPath->arrPoints[iPt*2], pPath->arrPoints[iPt*2+1],
							pPath->arrPoints[(iPt+1)*2], pPath->arrPoints[(iPt+1)*2+1]);
						iPt += 2;
						break;
					case AF_PATH_CMD_CUBIC_TO:
						xgeShapeExCubicTo(pShape,
							pPath->arrPoints[iPt*2], pPath->arrPoints[iPt*2+1],
							pPath->arrPoints[(iPt+1)*2], pPath->arrPoints[(iPt+1)*2+1],
							pPath->arrPoints[(iPt+2)*2], pPath->arrPoints[(iPt+2)*2+1]);
						iPt += 3;
						break;
					case AF_PATH_CMD_CLOSE:
						xgeShapeExClose(pShape);
						break;
					}
				}
				
				/* Apply fill */
				if ( pShapeRec->tFill.iType == AF_PAINT_SOLID ) {
					xgeShapeExFillColor(pShape, pShapeRec->tFill.iColor);
				} else if ( pShapeRec->tFill.iType == AF_PAINT_LINEAR ) {
					xge_shape_ex_color_stop_t arrStops[AF_DOC_MAX_GRADIENT_STOPS];
					int iStop;
					for ( iStop = 0; iStop < pShapeRec->tFill.iStopCount; iStop++ ) {
						arrStops[iStop].fOffset = pShapeRec->tFill.arrStops[iStop].fOffset;
						arrStops[iStop].iColor = pShapeRec->tFill.arrStops[iStop].iColor;
					}
					xgeShapeExFillLinearGradient(pShape, 
						pShapeRec->tFill.fX1, pShapeRec->tFill.fY1,
						pShapeRec->tFill.fX2, pShapeRec->tFill.fY2,
						0, arrStops, pShapeRec->tFill.iStopCount);
				} else if ( pShapeRec->tFill.iType == AF_PAINT_RADIAL ) {
					xge_shape_ex_color_stop_t arrStops[AF_DOC_MAX_GRADIENT_STOPS];
					int iStop;
					for ( iStop = 0; iStop < pShapeRec->tFill.iStopCount; iStop++ ) {
						arrStops[iStop].fOffset = pShapeRec->tFill.arrStops[iStop].fOffset;
						arrStops[iStop].iColor = pShapeRec->tFill.arrStops[iStop].iColor;
					}
					xgeShapeExFillRadialGradient(pShape,
						pShapeRec->tFill.fX1, pShapeRec->tFill.fY1,
						pShapeRec->tFill.fRadius,
						pShapeRec->tFill.fX2, pShapeRec->tFill.fY2,
						0, arrStops, pShapeRec->tFill.iStopCount);
				}
				
				/* Apply stroke */
				if ( pShapeRec->tStroke.iType == AF_PAINT_SOLID ) {
					xgeShapeExStrokeColor(pShape, pShapeRec->tStroke.iColor);
					xgeShapeExStrokeWidth(pShape, pShapeRec->fStrokeWidth);
					xgeShapeExStrokeCap(pShape, pShapeRec->iLineCap);
					xgeShapeExStrokeJoin(pShape, pShapeRec->iLineJoin);
					xgeShapeExStrokeMiterLimit(pShape, pShapeRec->fMiterLimit);
				}
				
				/* Apply view transform and draw */
				xgeShapeExTransformSet(pShape, &tViewMatrix);
				xgeShapeExDraw(pShape, 0.5f / pApp->tView.fZoom);
				xgeShapeExDestroy(pShape);
			}
		}
		
		/* Render tool overlay */
		if ( pApp->pToolMgr != NULL ) {
			/* Update tool context view transform */
			pApp->pToolMgr->tContext.fZoom = pApp->tView.fZoom;
			pApp->pToolMgr->tContext.fOffsetX = stage.fX;
			pApp->pToolMgr->tContext.fOffsetY = stage.fY;
			afToolManagerRender(pApp->pToolMgr);
		}
	}

	/* Draw placeholder text */
	if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
		char sInfo[128];
		snprintf(sInfo, sizeof(sInfo), "Stage %.0fx%.0f  Frame %d/%u  Zoom %.0f%%",
			pApp->tDoc.fStageWidth, pApp->tDoc.fStageHeight,
			pApp->iCurrentFrame, pApp->tDoc.iFrameCount,
			pApp->tView.fZoom * 100.0f);
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sInfo,
			(xui_rect_t){rc.fX + 8.0f, rc.fY + rc.fH - 24.0f, rc.fW - 16.0f, 20.0f},
			AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Library panel render                                               */
/* ------------------------------------------------------------------ */

/* Library context menu IDs */
#define AF_LIB_CTX_INSTANTIATE  2001
#define AF_LIB_CTX_DELETE       2002
#define AF_LIB_CTX_DUPLICATE    2003
#define AF_LIB_CTX_RENAME       2004

static xui_widget s_pLibCtxMenu = NULL;
static int s_iLibCtxSymbol = -1;
static int s_iLibSelected = -1;

static void __afLibCtxSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	(void)pMenu; (void)iIndex;
	if ( pApp == NULL || s_iLibCtxSymbol < 0 ) return;

	switch ( iValue ) {
	case AF_LIB_CTX_INSTANTIATE:
		{
			af_symbol_t* pSym = &pApp->tDoc.arrSymbols[s_iLibCtxSymbol];
			if ( pApp->iActiveLayerId != 0 ) {
				afSymbolInstantiate(&pApp->tDoc, pSym->iId, pApp->iActiveLayerId,
				                    (uint32_t)pApp->iCurrentFrame,
				                    pApp->tDoc.fStageWidth * 0.5f,
				                    pApp->tDoc.fStageHeight * 0.5f, NULL);
				pApp->tDoc.bModified = 1;
			}
		}
		break;
	case AF_LIB_CTX_DELETE:
		if ( s_iLibCtxSymbol < pApp->tDoc.iSymbolCount ) {
			uint32_t iId = pApp->tDoc.arrSymbols[s_iLibCtxSymbol].iId;
			if ( afSymbolGetUsageCount(&pApp->tDoc, iId) == 0 ) {
				afDocRemoveSymbol(&pApp->tDoc, iId);
				pApp->tDoc.bModified = 1;
			}
		}
		break;
	case AF_LIB_CTX_DUPLICATE:
		if ( s_iLibCtxSymbol < pApp->tDoc.iSymbolCount ) {
			uint32_t iId = pApp->tDoc.arrSymbols[s_iLibCtxSymbol].iId;
			char sName[AF_DOC_MAX_NAME];
			snprintf(sName, sizeof(sName), "%s_copy", pApp->tDoc.arrSymbols[s_iLibCtxSymbol].sName);
			afSymbolDuplicate(&pApp->tDoc, iId, sName, NULL);
			pApp->tDoc.bModified = 1;
		}
		break;
	case AF_LIB_CTX_RENAME:
		/* TODO: inline rename */
		break;
	default:
		break;
	}
	xgeRenderRequest();
}

static int __afLibraryPointerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	float fX, fY;

	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	rc = xuiWidgetGetContentRect(pWidget);
	fX = pEvent->fX;
	fY = pEvent->fY;

	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		/* Button area: Import (top-left) / New Symbol (top-right) */
		if ( fY >= rc.fY + 2.0f && fY <= rc.fY + 20.0f ) {
			float fBtnW = 60.0f;
			if ( fX >= rc.fX + 4.0f && fX <= rc.fX + 4.0f + fBtnW ) {
				/* Import button - trigger file dialog */
				afShellHandleMenuCommand(pApp, 105); /* AF_MENU_FILE_IMPORT */
				return XUI_EVENT_DISPATCH_STOP;
			}
			if ( fX >= rc.fX + 68.0f && fX <= rc.fX + 68.0f + fBtnW ) {
				/* New Symbol button */
				uint32_t iNewId = 0;
				char sName[64];
				snprintf(sName, sizeof(sName), "Symbol %d", pApp->tDoc.iSymbolCount + 1);
				afDocAddSymbol(&pApp->tDoc, AF_SYMBOL_GRAPHIC, sName, &iNewId);
				pApp->tDoc.bModified = 1;
				xgeRenderRequest();
				return XUI_EVENT_DISPATCH_STOP;
			}
		}

		/* Right-click on symbol list */
		if ( pEvent->iButton == XUI_POINTER_BUTTON_RIGHT ) {
			float fItemY = rc.fY + 24.0f;
			int i;
			for ( i = 0; i < pApp->tDoc.iSymbolCount; i++ ) {
				if ( fY >= fItemY && fY < fItemY + 20.0f ) {
					s_iLibCtxSymbol = i;
					if ( s_pLibCtxMenu != NULL ) {
						xuiMenuOpenAt(s_pLibCtxMenu, pWidget, fX, fY);
					}
					return XUI_EVENT_DISPATCH_STOP;
				}
				fItemY += 20.0f;
			}
			return XUI_OK;
		}

		/* Left-click: select symbol in list */
		{
			float fItemY = rc.fY + 24.0f;
			int i;
			s_iLibSelected = -1;
			for ( i = 0; i < pApp->tDoc.iSymbolCount; i++ ) {
				if ( fY >= fItemY && fY < fItemY + 20.0f ) {
					s_iLibSelected = i;
					xgeRenderRequest();
					return XUI_EVENT_DISPATCH_STOP;
				}
				fItemY += 20.0f;
			}
		}
	}

	return XUI_OK;
}

static int __afLibraryRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	char sText[128];
	float fY;
	int i;
	(void)iStateId;
	if ( pWidget == NULL || pDraw == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	rc = xuiWidgetGetContentRect(pWidget);
	if ( rc.fW <= 0.0f || rc.fH <= 0.0f ) return XUI_OK;
	if ( pApp->tProxy.drawRectFill != NULL ) {
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, rc, AF_COLOR_PANEL);
	}

	/* Toolbar buttons: [Import] [New Symbol] */
	if ( pApp->tProxy.drawRectFill != NULL ) {
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw,
			(xui_rect_t){rc.fX + 4.0f, rc.fY + 2.0f, 60.0f, 18.0f}, AF_COLOR_HIGHLIGHT);
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw,
			(xui_rect_t){rc.fX + 68.0f, rc.fY + 2.0f, 60.0f, 18.0f}, AF_COLOR_HIGHLIGHT);
	}
	if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "Import",
			(xui_rect_t){rc.fX + 4.0f, rc.fY + 2.0f, 60.0f, 18.0f},
			AF_COLOR_TEXT, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "+ New",
			(xui_rect_t){rc.fX + 68.0f, rc.fY + 2.0f, 60.0f, 18.0f},
			AF_COLOR_TEXT, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}

	/* Header info */
	if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
		snprintf(sText, sizeof(sText), "Symbols: %d  Res: %d", pApp->tDoc.iSymbolCount, pApp->tDoc.iResourceCount);
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
			(xui_rect_t){rc.fX + 132.0f, rc.fY + 2.0f, rc.fW - 136.0f, 18.0f},
			AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	/* Symbol list */
	fY = rc.fY + 24.0f;
	for ( i = 0; i < pApp->tDoc.iSymbolCount && fY < rc.fY + rc.fH - 20.0f; i++ ) {
		af_symbol_t* pSym = &pApp->tDoc.arrSymbols[i];
		uint32_t iIconColor;
		const char* sType = "?";
		switch ( pSym->iType ) {
		case AF_SYMBOL_GRAPHIC:    iIconColor = XUI_COLOR_RGBA(100, 200, 100, 255); sType = "G"; break;
		case AF_SYMBOL_SPRITE:     iIconColor = XUI_COLOR_RGBA(100, 150, 255, 255); sType = "S"; break;
		case AF_SYMBOL_GROUP:      iIconColor = XUI_COLOR_RGBA(255, 200, 100, 255); sType = "Gr"; break;
		case AF_SYMBOL_MOVIE_CLIP: iIconColor = XUI_COLOR_RGBA(255, 100, 150, 255); sType = "MC"; break;
		default: iIconColor = AF_COLOR_TEXT_DIM; break;
		}
		/* Selection highlight */
		if ( i == s_iLibSelected && pApp->tProxy.drawRectFill != NULL ) {
			pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw,
				(xui_rect_t){rc.fX + 2.0f, fY, rc.fW - 4.0f, 20.0f}, XUI_COLOR_RGBA(0, 122, 204, 50));
		}
		/* Icon */
		if ( pApp->tProxy.drawRectFill != NULL ) {
			pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw,
				(xui_rect_t){rc.fX + 8.0f, fY + 2.0f, 12.0f, 12.0f}, iIconColor);
		}
		/* Name */
		if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
			snprintf(sText, sizeof(sText), "%s [%s]", pSym->sName, sType);
			pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
				(xui_rect_t){rc.fX + 24.0f, fY, rc.fW - 32.0f, 16.0f},
				AF_COLOR_TEXT, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		}
		fY += 20.0f;
	}
	/* Resources */
	if ( pApp->tDoc.iResourceCount > 0 && fY < rc.fY + rc.fH - 20.0f ) {
		fY += 8.0f;
		if ( pApp->tProxy.drawLine != NULL ) {
			pApp->tProxy.drawLine(&pApp->tProxy, pDraw, rc.fX + 4.0f, fY, rc.fX + rc.fW - 4.0f, fY, 1.0f, AF_COLOR_TEXT_DIM);
		}
		fY += 6.0f;
		for ( i = 0; i < pApp->tDoc.iResourceCount && fY < rc.fY + rc.fH - 20.0f; i++ ) {
			af_resource_t* pRes = &pApp->tDoc.arrResources[i];
			if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
				snprintf(sText, sizeof(sText), "%s (%dx%d)", pRes->sName, pRes->iWidth, pRes->iHeight);
				pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
					(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 16.0f},
					AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
			}
			fY += 18.0f;
		}
	}
	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Property panel render                                              */
/* ------------------------------------------------------------------ */

static int __afPropertyPointerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	float fY;

	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if ( pApp->iSelectedInstance == 0 ) return XUI_OK;

	rc = xuiWidgetGetContentRect(pWidget);
	fY = pEvent->fY;

	/* Mouse wheel on property rows adjusts values */
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		af_element_t* pElem = afDocFindElement(&pApp->tDoc, pApp->iSelectedInstance);
		if ( pElem == NULL ) return XUI_OK;

		float fDelta = pEvent->fWheelY * 2.0f;
		float fBaseY = rc.fY + 24.0f; /* first property row */

		/* Row 2: X/Y position (offset 32 from base) */
		if ( fY >= fBaseY + 32.0f && fY < fBaseY + 48.0f ) {
			if ( pEvent->fX < rc.fX + rc.fW * 0.5f )
				pElem->fTx += fDelta;
			else
				pElem->fTy += fDelta;
			pApp->tDoc.bModified = 1;
			xgeRenderRequest();
			return XUI_EVENT_DISPATCH_STOP;
		}
		/* Row 3: Scale (offset 48) */
		if ( fY >= fBaseY + 48.0f && fY < fBaseY + 64.0f ) {
			float fScaleDelta = pEvent->fWheelY * 0.05f;
			if ( pEvent->fX < rc.fX + rc.fW * 0.5f )
				pElem->fScaleX += fScaleDelta;
			else
				pElem->fScaleY += fScaleDelta;
			if ( pElem->fScaleX < 0.01f ) pElem->fScaleX = 0.01f;
			if ( pElem->fScaleY < 0.01f ) pElem->fScaleY = 0.01f;
			pApp->tDoc.bModified = 1;
			xgeRenderRequest();
			return XUI_EVENT_DISPATCH_STOP;
		}
		/* Row 4: Rotation/Alpha (offset 64) */
		if ( fY >= fBaseY + 64.0f && fY < fBaseY + 80.0f ) {
			if ( pEvent->fX < rc.fX + rc.fW * 0.5f )
				pElem->fRotation += pEvent->fWheelY * 5.0f;
			else {
				pElem->fOpacity += pEvent->fWheelY * 0.05f;
				if ( pElem->fOpacity < 0.0f ) pElem->fOpacity = 0.0f;
				if ( pElem->fOpacity > 1.0f ) pElem->fOpacity = 1.0f;
			}
			pApp->tDoc.bModified = 1;
			xgeRenderRequest();
			return XUI_EVENT_DISPATCH_STOP;
		}
	}

	return XUI_OK;
}

static int __afPropertyRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	char sText[128];
	float fY;
	(void)iStateId;
	if ( pWidget == NULL || pDraw == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	rc = xuiWidgetGetContentRect(pWidget);
	if ( rc.fW <= 0.0f || rc.fH <= 0.0f ) return XUI_OK;
	if ( pApp->tProxy.drawRectFill != NULL ) {
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, rc, AF_COLOR_PANEL);
	}
	fY = rc.fY + 4.0f;
	if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "Properties",
			(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 16.0f},
			AF_COLOR_TEXT, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		fY += 20.0f;
	}
	/* Show selected symbol properties */
	if ( pApp->iSelectedInstance != 0 ) {
		af_element_t* pElem = afDocFindElement(&pApp->tDoc, pApp->iSelectedInstance);
		if ( pElem != NULL && pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
			af_symbol_t* pSym = afDocFindSymbol(&pApp->tDoc, pElem->iSymbolId);
			snprintf(sText, sizeof(sText), "Instance: %u", pElem->iInstanceId);
			pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
				(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
				AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
			fY += 16.0f;
			if ( pSym != NULL ) {
				snprintf(sText, sizeof(sText), "Symbol: %s", pSym->sName);
				pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
					(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
					AF_COLOR_TEXT, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
				fY += 16.0f;
			}
			snprintf(sText, sizeof(sText), "X: %.1f  Y: %.1f", pElem->fTx, pElem->fTy);
			pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
				(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
				AF_COLOR_TEXT, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
			fY += 16.0f;
			snprintf(sText, sizeof(sText), "W: %.1f  H: %.1f", pElem->fScaleX * 100.0f, pElem->fScaleY * 100.0f);
			pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
				(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
				AF_COLOR_TEXT, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
			fY += 16.0f;
			snprintf(sText, sizeof(sText), "Rot: %.1f  Alpha: %.0f%%", pElem->fRotation, pElem->fOpacity * 100.0f);
			pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
				(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
				AF_COLOR_TEXT, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		}
	} else if ( pApp->tDoc.iSymbolCount > 0 && pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
		/* Show document properties when nothing selected */
		snprintf(sText, sizeof(sText), "Stage: %.0f x %.0f", pApp->tDoc.fStageWidth, pApp->tDoc.fStageHeight);
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
			(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
			AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		fY += 16.0f;
		snprintf(sText, sizeof(sText), "FPS: %.0f  Frames: %u", pApp->tDoc.fFrameRate, pApp->tDoc.iFrameCount);
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
			(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
			AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		fY += 16.0f;
		snprintf(sText, sizeof(sText), "Layers: %d  Symbols: %d", pApp->tDoc.tTimeline.iLayerCount, pApp->tDoc.iSymbolCount);
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText,
			(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
			AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
	} else if ( pApp->tProxy.drawText != NULL && pApp->pFont != NULL ) {
		pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "No selection",
			(xui_rect_t){rc.fX + 8.0f, fY, rc.fW - 16.0f, 14.0f},
			AF_COLOR_TEXT_DIM, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Timeline panel render                                              */
/* ------------------------------------------------------------------ */

static int __afTimelineRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	int iLayerCount, iFrameCount;
	int i, f;
	float fContentX, fContentY;
	(void)iStateId;
	if ( pWidget == NULL || pDraw == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	rc = xuiWidgetGetContentRect(pWidget);
	if ( rc.fW <= 0.0f || rc.fH <= 0.0f ) return XUI_OK;

	iLayerCount = pApp->tDoc.tTimeline.iLayerCount;
	iFrameCount = (int)pApp->tDoc.iFrameCount;

	/* Background */
	if ( pApp->tProxy.drawRectFill != NULL ) {
		pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, rc, AF_COLOR_TIMELINE_BG);
	}

	fContentX = rc.fX + AF_TL_LAYER_WIDTH;
	fContentY = rc.fY + AF_TL_RULER_HEIGHT;

	/* Frame ruler */
	{
		xui_rect_t rulerRc = { rc.fX, rc.fY, rc.fW, AF_TL_RULER_HEIGHT };
		if ( pApp->tProxy.drawRectFill != NULL ) {
			pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, rulerRc, AF_COLOR_PANEL);
		}
		if ( pApp->tProxy.drawText != NULL && pApp->pFontMono != NULL ) {
			for ( f = 0; f < iFrameCount; f += 5 ) {
				float fX = fContentX + (float)f * AF_TL_FRAME_WIDTH;
				if ( fX >= fContentX && fX < rc.fX + rc.fW - 20.0f ) {
					char sNum[16];
					xui_rect_t tRc;
					snprintf(sNum, sizeof(sNum), "%d", f);
					tRc.fX = fX + 2.0f; tRc.fY = rc.fY + 4.0f;
					tRc.fW = 36.0f; tRc.fH = 14.0f;
					pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFontMono, sNum, tRc, AF_COLOR_TEXT_DIM, 0);
				}
			}
		}
		if ( pApp->tProxy.drawLine != NULL ) {
			pApp->tProxy.drawLine(&pApp->tProxy, pDraw, rc.fX, fContentY, rc.fX + rc.fW, fContentY, 1.0f, AF_COLOR_HIGHLIGHT);
		}
	}

	/* Layer list */
	for ( i = 0; i < iLayerCount; i++ ) {
		af_layer_t* pLayer = &pApp->tDoc.tTimeline.arrLayers[i];
		float fRowY = fContentY + (float)i * AF_TL_ROW_HEIGHT;
		if ( fRowY > rc.fY + rc.fH ) break;

		/* Selected layer highlight */
		if ( pLayer->iId == pApp->iActiveLayerId && pApp->tProxy.drawRectFill != NULL ) {
			xui_rect_t selRc = { rc.fX, fRowY, AF_TL_LAYER_WIDTH, AF_TL_ROW_HEIGHT };
			pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, selRc, XUI_COLOR_RGBA(0, 122, 204, 50));
		}
		/* Layer name */
		if ( pApp->tProxy.drawText != NULL && pApp->pFontMono != NULL ) {
			xui_rect_t tRc = { rc.fX + 6.0f, fRowY + 3.0f, AF_TL_LAYER_WIDTH - 12.0f, 16.0f };
			pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFontMono, pLayer->sName, tRc, AF_COLOR_TEXT, 0);
		}
		/* Row separator */
		if ( pApp->tProxy.drawLine != NULL ) {
			pApp->tProxy.drawLine(&pApp->tProxy, pDraw, rc.fX, fRowY + AF_TL_ROW_HEIGHT, rc.fX + rc.fW, fRowY + AF_TL_ROW_HEIGHT, 1.0f, AF_COLOR_HIGHLIGHT);
		}

		/* Keyframe dots + tween spans */
		{
			int k;
			for ( k = 0; k < pLayer->iKeyframeCount - 1; k++ ) {
				af_keyframe_t* pKf = &pLayer->arrKeyframes[k];
				if ( pKf->iElementCount > 0 && pKf->arrElements[0].iTweenId != 0xFFFFFFFFu ) {
					af_keyframe_t* pKfN = &pLayer->arrKeyframes[k + 1];
					float fX1 = fContentX + (float)pKf->iFrame * AF_TL_FRAME_WIDTH;
					float fX2 = fContentX + (float)pKfN->iFrame * AF_TL_FRAME_WIDTH;
					if ( pApp->tProxy.drawRectFill != NULL ) {
						xui_rect_t spanRc = { fX1, fRowY + 7.0f, fX2 - fX1, AF_TL_ROW_HEIGHT - 14.0f };
						pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, spanRc, XUI_COLOR_RGBA(0, 122, 204, 100));
					}
				}
			}
			for ( k = 0; k < pLayer->iKeyframeCount; k++ ) {
				af_keyframe_t* pKf = &pLayer->arrKeyframes[k];
				float fX = fContentX + (float)pKf->iFrame * AF_TL_FRAME_WIDTH + AF_TL_FRAME_WIDTH * 0.5f;
				float fY = fRowY + AF_TL_ROW_HEIGHT * 0.5f;
				if ( fX >= fContentX && fX < rc.fX + rc.fW && pApp->tProxy.drawRectFill != NULL ) {
					xui_rect_t dotRc = { fX - 3.0f, fY - 3.0f, 6.0f, 6.0f };
					pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, dotRc, AF_COLOR_KEYFRAME);
				}
			}
		}
	}

	/* Column separator */
	if ( pApp->tProxy.drawLine != NULL ) {
		pApp->tProxy.drawLine(&pApp->tProxy, pDraw, fContentX, rc.fY, fContentX, rc.fY + rc.fH, 1.0f, AF_COLOR_HIGHLIGHT);
	}

	/* Playhead */
	{
		float fPX = fContentX + (float)pApp->iCurrentFrame * AF_TL_FRAME_WIDTH + AF_TL_FRAME_WIDTH * 0.5f;
		if ( fPX >= fContentX && fPX <= rc.fX + rc.fW && pApp->tProxy.drawLine != NULL ) {
			pApp->tProxy.drawLine(&pApp->tProxy, pDraw, fPX, rc.fY, fPX, rc.fY + rc.fH, 2.0f, XUI_COLOR_RGBA(255, 60, 60, 255));
		}
	}

	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Root render                                                        */
/* ------------------------------------------------------------------ */

static int __afRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	xui_rect_t rc;
	(void)iStateId;
	if ( pWidget == NULL || pDraw == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	rc = xuiWidgetGetContentRect(pWidget);
	if ( pApp->tProxy.drawRectFill != NULL ) {
		return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, rc, AF_COLOR_BG);
	}
	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Helper: create a panel widget with cache render                    */
/* ------------------------------------------------------------------ */

static int __afCreatePanel(af_app_t* pApp, xui_widget* pOut, xui_widget_cache_render_proc pfnRender)
{
	xui_cache_policy_t policy;
	xui_widget widget;
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &widget);
	if ( ret != XUI_OK ) return ret;
	memset(&policy, 0, sizeof(policy));
	policy.iSize = sizeof(policy);
	policy.iPolicy = XUI_CACHE_POLICY_SELF;
	policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	policy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(widget, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(widget, &policy);
	(void)xuiWidgetSetCacheRenderCallback(widget, pfnRender, pApp);
	*pOut = widget;
	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Application init                                                   */
/* ------------------------------------------------------------------ */

int afAppInit(af_app_t* pApp)
{
	if ( pApp == NULL ) return -1;
	memset(pApp, 0, sizeof(af_app_t));
	afDocInit(&pApp->tDoc);
	afUndoInit(&pApp->tUndo);
	pApp->tView.fZoom = 1.0f;
	pApp->tView.fGridSize = 20.0f;
	pApp->tView.bShowGrid = 0;
	pApp->tView.bShowRulers = 1;
	pApp->iActiveTool = AF_TOOL_SELECT;
	pApp->iFillColorRGBA = XUI_COLOR_RGBA(0, 0, 0, 255);
	pApp->iStrokeColorRGBA = XUI_COLOR_RGBA(0, 0, 0, 255);
	pApp->fStrokeWidth = 1.0f;
	pApp->fPlaySpeed = 1.0f;
	pApp->bLoop = 1;
	return 0;
}

void afAppShutdown(af_app_t* pApp)
{
	if ( pApp == NULL ) return;
	
	/* Shutdown tool manager */
	if ( pApp->pToolMgr != NULL ) {
		afToolManagerShutdown(pApp->pToolMgr);
		free(pApp->pToolMgr);
		pApp->pToolMgr = NULL;
	}
	
	if ( pApp->pContext != NULL ) {
		xuiDestroy(pApp->pContext);
		pApp->pContext = NULL;
	}
	if ( pApp->pFont != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFont);
		pApp->pFont = NULL;
	}
	if ( pApp->pFontMono != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFontMono);
		pApp->pFontMono = NULL;
	}
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
		pApp->pTarget = NULL;
	}
	afDocClear(&pApp->tDoc);
}

/* ------------------------------------------------------------------ */
/* UI creation                                                        */
/* ------------------------------------------------------------------ */

int afAppCreateUi(af_app_t* pApp)
{
	xui_surface_desc_t surfaceDesc;
	xui_dock_panel_desc_t dockDesc;
	xui_widget wStage, wTimeline, wLibrary, wProperty;
	const char* sFontPath;
	const char* sMonoPath;
	int ret;

	/* Proxy and context */
	pApp->tProxy = xuiProxyXge();
	ret = xuiCreate(&pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if ( ret != XUI_OK ) return ret;
	ret = xuiInputViewport(pApp->pContext, (float)AF_APP_WIDTH, (float)AF_APP_HEIGHT);
	if ( ret != XUI_OK ) return ret;

	/* Render target surface */
	memset(&surfaceDesc, 0, sizeof(surfaceDesc));
	surfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	surfaceDesc.iWidth = AF_APP_WIDTH;
	surfaceDesc.iHeight = AF_APP_HEIGHT;
	surfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	surfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	ret = pApp->tProxy.surfaceCreate(&pApp->tProxy, &pApp->pTarget, &surfaceDesc);
	if ( ret != XUI_OK ) return ret;

	/* Fonts */
	sFontPath = __afFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	ret = pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF);
	if ( ret != XUI_OK ) return ret;
	(void)xuiSetDefaultFont(pApp->pContext, pApp->pFont);

	sMonoPath = __afFindMonoTtf();
	if ( sMonoPath != NULL ) {
		pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFontMono, sMonoPath, 12.0f, XUI_FONT_FORMAT_TTF);
	}

	/* Root widget */
	ret = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( ret != XUI_OK ) return ret;
	xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)AF_APP_WIDTH, (float)AF_APP_HEIGHT});
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, __afRootRender, pApp);
	ret = xuiSetRootWidget(pApp->pContext, pApp->pRoot);
	if ( ret != XUI_OK ) return ret;

	/* DockPanel - leave room for menubar(24) + toolbar(32) + statusbar(22) */
	memset(&dockDesc, 0, sizeof(dockDesc));
	dockDesc.iSize = sizeof(dockDesc);
	dockDesc.pFont = pApp->pFont;
	ret = xuiDockPanelCreate(pApp->pContext, &pApp->pDock, &dockDesc);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){0.0f, 56.0f, (float)AF_APP_WIDTH, (float)AF_APP_HEIGHT - 78.0f});
	ret = xuiWidgetAddChild(pApp->pRoot, pApp->pDock);
	if ( ret != XUI_OK ) return ret;

	/* Create panel content widgets */
	ret = __afCreatePanel(pApp, &wStage, __afStageRender);
	if ( ret != XUI_OK ) return ret;
	pApp->pStageCanvas = wStage;

	ret = __afCreatePanel(pApp, &wTimeline, __afTimelineRender);
	if ( ret != XUI_OK ) return ret;

	ret = __afCreatePanel(pApp, &wLibrary, __afLibraryRender);
	if ( ret != XUI_OK ) return ret;

	/* Library panel: event handler + context menu */
	xuiWidgetSetEventCallback(wLibrary, __afLibraryPointerEvent, pApp);
	{
		xui_menu_desc_t libMenuDesc;
		xui_menu_item_t libItems[4];
		memset(&libMenuDesc, 0, sizeof(libMenuDesc));
		libMenuDesc.iSize = sizeof(libMenuDesc);
		libMenuDesc.pOwner = wLibrary;
		libMenuDesc.pFont = pApp->pFont;
		if ( xuiMenuCreate(pApp->pContext, &s_pLibCtxMenu, &libMenuDesc) == XUI_OK && s_pLibCtxMenu != NULL ) {
			memset(libItems, 0, sizeof(libItems));
			libItems[0] = (xui_menu_item_t){"Instantiate on Stage", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_LIB_CTX_INSTANTIATE, 0, NULL, NULL};
			libItems[1] = (xui_menu_item_t){"Duplicate", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_LIB_CTX_DUPLICATE, 0, NULL, NULL};
			libItems[2] = (xui_menu_item_t){"Rename", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_LIB_CTX_RENAME, 0, NULL, NULL};
			libItems[3] = (xui_menu_item_t){"Delete", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_LIB_CTX_DELETE, 0, NULL, NULL};
			(void)xuiMenuSetItems(s_pLibCtxMenu, libItems, 4);
			(void)xuiMenuSetSelect(s_pLibCtxMenu, __afLibCtxSelected, pApp);
		}
	}

	ret = __afCreatePanel(pApp, &wProperty, __afPropertyRender);
	if ( ret != XUI_OK ) return ret;
	xuiWidgetSetEventCallback(wProperty, __afPropertyPointerEvent, pApp);

	/* Add windows to DockPanel */
	ret = xuiDockPanelAddWindow(pApp->pDock, "Stage", wStage, &pApp->iWinStage);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelAddWindow(pApp->pDock, "Timeline", wTimeline, &pApp->iWinTimeline);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelAddWindow(pApp->pDock, "Library", wLibrary, &pApp->iWinLibrary);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelAddWindow(pApp->pDock, "Properties", wProperty, &pApp->iWinProperty);
	if ( ret != XUI_OK ) return ret;

	/* Dock layout: stage center, library left, properties right, timeline bottom */
	ret = xuiDockPanelDockWindow(pApp->pDock, pApp->iWinStage, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pApp->iPaneDoc);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelDockWindow(pApp->pDock, pApp->iWinLibrary, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.16f, NULL);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelDockWindow(pApp->pDock, pApp->iWinProperty, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.18f, NULL);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelDockWindow(pApp->pDock, pApp->iWinTimeline, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.28f, &pApp->iPaneTimeline);
	if ( ret != XUI_OK ) return ret;

	/* Create menu bar, toolbar, status bar */
	ret = afShellCreate(pApp);
	if ( ret != 0 ) return ret;

	/* Create a default layer */
	afDocAddLayer(&pApp->tDoc, "Layer 1", AF_LAYER_NORMAL, &pApp->iActiveLayerId);

	/* Initialize tool manager */
	pApp->pToolMgr = (af_tool_manager_t*)calloc(1, sizeof(af_tool_manager_t));
	if ( pApp->pToolMgr != NULL ) {
		af_tool pTool;
		afToolManagerInit(pApp->pToolMgr, pApp);
		
		/* Register all tools */
		pTool = afToolSelectCreate();
		if ( pTool != NULL ) afToolManagerRegister(pApp->pToolMgr, pTool);
		pTool = afToolPenCreate();
		if ( pTool != NULL ) afToolManagerRegister(pApp->pToolMgr, pTool);
		pTool = afToolLineCreate();
		if ( pTool != NULL ) afToolManagerRegister(pApp->pToolMgr, pTool);
		pTool = afToolRectCreate();
		if ( pTool != NULL ) afToolManagerRegister(pApp->pToolMgr, pTool);
		pTool = afToolEllipseCreate();
		if ( pTool != NULL ) afToolManagerRegister(pApp->pToolMgr, pTool);
		pTool = afToolPencilCreate();
		if ( pTool != NULL ) afToolManagerRegister(pApp->pToolMgr, pTool);
		
		/* Set default tool */
		afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_SELECT);
	}

	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Input handling                                                     */
/* ------------------------------------------------------------------ */

static uint32_t __afReadButtons(void)
{
	uint32_t buttons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) buttons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) buttons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) buttons |= XUI_POINTER_BUTTON_MIDDLE;
	return buttons;
}

int afAppHandleInput(af_app_t* pApp)
{
	float x, y, wheelX, wheelY;
	uint32_t buttons, pressed, released;
	int ret;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();

	xgeMouseGet(&x, &y);
	xgeMouseGetWheel(&wheelX, &wheelY);
	buttons = __afReadButtons();

	if ( !pApp->bHasMouse || x != pApp->fLastMouseX || y != pApp->fLastMouseY || buttons != pApp->iLastButtons ) {
		ret = xuiInputPointerMove(pApp->pContext, x, y, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( wheelX != 0.0f || wheelY != 0.0f ) {
		/* Ctrl+wheel = zoom stage */
		if ( xgeKeyDown(XGE_KEY_MENU) ) {
			pApp->tView.fZoom += wheelY * 0.1f;
			if ( pApp->tView.fZoom < 0.1f ) pApp->tView.fZoom = 0.1f;
			if ( pApp->tView.fZoom > 8.0f ) pApp->tView.fZoom = 8.0f;
			xgeRenderRequest();
		} else {
			ret = xuiInputPointerWheel(pApp->pContext, x, y, wheelX, wheelY, buttons);
			if ( ret != XUI_OK ) return ret;
		}
	}

	pressed = buttons & ~pApp->iLastButtons;
	released = pApp->iLastButtons & ~buttons;

	if ( (pressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		ret = xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (released & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		ret = xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (pressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		ret = xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_RIGHT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (released & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		ret = xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_RIGHT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (pressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		ret = xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_MIDDLE, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (released & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		ret = xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_MIDDLE, buttons);
		if ( ret != XUI_OK ) return ret;
	}

	/* Dispatch to tool manager (convert to stage coordinates) */
	if ( pApp->pToolMgr != NULL ) {
		float fStageX, fStageY;
		/* Convert screen to stage coordinates using stored stage rect */
		fStageX = (x - pApp->tStageRect.fX) / pApp->tView.fZoom;
		fStageY = (y - pApp->tStageRect.fY) / pApp->tView.fZoom;
		
		if ( (pressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
			afToolManagerMouseDown(pApp->pToolMgr, fStageX, fStageY, XGE_MOUSE_LEFT);
		}
		if ( (released & XUI_POINTER_BUTTON_LEFT) != 0 ) {
			afToolManagerMouseUp(pApp->pToolMgr, fStageX, fStageY, XGE_MOUSE_LEFT);
		}
		if ( !pApp->bHasMouse || x != pApp->fLastMouseX || y != pApp->fLastMouseY ) {
			afToolManagerMouseMove(pApp->pToolMgr, fStageX, fStageY);
		}
		
		/* Handle tool keyboard shortcuts */
		if ( xgeKeyPressed('V') ) afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_SELECT);
		if ( xgeKeyPressed('P') ) afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_PEN);
		if ( xgeKeyPressed('N') ) afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_LINE);
		if ( xgeKeyPressed('R') ) afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_RECT);
		if ( xgeKeyPressed('O') ) afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_ELLIPSE);
		if ( xgeKeyPressed('Y') ) afToolManagerSetActive(pApp->pToolMgr, AF_TOOL_PENCIL);
		if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) afToolManagerCancel(pApp->pToolMgr);
	}

	/* Playback keyboard shortcuts */
	if ( xgeKeyPressed(XGE_KEY_ENTER) ) {
		if ( pApp->tPlayback.iState == AF_PLAY_PLAYING ) afAppStop(pApp);
		else afAppPlay(pApp);
	}
	if ( xgeKeyPressed(XGE_KEY_RIGHT) ) afAppStepFrame(pApp, 1);
	if ( xgeKeyPressed(XGE_KEY_LEFT) ) afAppStepFrame(pApp, -1);
	if ( xgeKeyPressed(XGE_KEY_HOME) ) {
		afPlaybackGotoFrame(&pApp->tPlayback, 0);
		pApp->iCurrentFrame = 0;
		xgeRenderRequest();
	}
	if ( xgeKeyPressed(XGE_KEY_END) ) {
		afPlaybackGotoFrame(&pApp->tPlayback, pApp->tDoc.iFrameCount - 1);
		pApp->iCurrentFrame = (int)pApp->tDoc.iFrameCount - 1;
		xgeRenderRequest();
	}
	/* F6 = insert keyframe at current frame on active layer */
	if ( xgeKeyPressed('K') ) {
		if ( pApp->iActiveLayerId != 0 ) {
			afDocSetKeyframe(&pApp->tDoc, pApp->iActiveLayerId, (uint32_t)pApp->iCurrentFrame, 0);
			xgeRenderRequest();
		}
	}

	pApp->bHasMouse = 1;
	pApp->fLastMouseX = x;
	pApp->fLastMouseY = y;
	pApp->iLastButtons = buttons;
	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Frame update                                                       */
/* ------------------------------------------------------------------ */

int afAppFrame(af_app_t* pApp)
{
	int ret;
	ret = afAppHandleInput(pApp);
	if ( ret != XUI_OK ) return ret;

	ret = xuiDispatchPendingEvents(pApp->pContext);
	if ( ret != XUI_OK ) return ret;

	ret = xuiLayout(pApp->pContext);
	if ( ret != XUI_OK ) return ret;

	ret = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( ret != XUI_OK ) return ret;

	/* Playback advance */
	afPlaybackUpdate(&pApp->tPlayback, xgeGetDelta());
	if ( pApp->tPlayback.iState == AF_PLAY_PLAYING ) {
		pApp->iCurrentFrame = (int)pApp->tPlayback.iCurrentFrame;
		xgeRenderRequest();
	}

	/* Update status bar */
	afShellUpdateStatusBar(pApp, NULL, 0);

	return XUI_OK;
}

/* ------------------------------------------------------------------ */
/* Render                                                             */
/* ------------------------------------------------------------------ */

void afAppRender(af_app_t* pApp)
{
	xui_rect_i_t fullRect;
	xui_rect_t src, dst;

	xgeBegin();

	if ( pApp->pContext != NULL && pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, AF_COLOR_BG);
		fullRect = (xui_rect_i_t){0, 0, AF_APP_WIDTH, AF_APP_HEIGHT};
		xuiRender(pApp->pContext, pApp->pTarget, &fullRect, 1);
	}

	xgeClear(XUI_COLOR_RGBA(18, 18, 18, 255));
	src = (xui_rect_t){0.0f, 0.0f, (float)AF_APP_WIDTH, (float)AF_APP_HEIGHT};
	dst = (xui_rect_t){0.0f, 0.0f, (float)AF_APP_WIDTH, (float)AF_APP_HEIGHT};
	pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, src, dst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);

	xgeEnd();
}

/* ------------------------------------------------------------------ */
/* Document operations                                                */
/* ------------------------------------------------------------------ */

int afAppNewDocument(af_app_t* pApp, float fW, float fH, float fFps, uint32_t iFrames)
{
	afDocClear(&pApp->tDoc);
	afDocInit(&pApp->tDoc);
	afDocSetStage(&pApp->tDoc, fW, fH, fFps, iFrames);
	afDocAddLayer(&pApp->tDoc, "Layer 1", AF_LAYER_NORMAL, &pApp->iActiveLayerId);
	pApp->iCurrentFrame = 0;
	pApp->bPlaying = 0;
	afPlaybackInit(&pApp->tPlayback, &pApp->tDoc);
	afUndoClear(&pApp->tUndo);
	xgeRenderRequest();
	return 0;
}

int afAppOpenDocument(af_app_t* pApp, const char* sPath)
{
	int ret;
	if ( pApp == NULL || sPath == NULL ) return -1;
	ret = afFileLoad(&pApp->tDoc, sPath);
	if ( ret != 0 ) return ret;
	pApp->iCurrentFrame = 0;
	pApp->iActiveLayerId = (pApp->tDoc.tTimeline.iLayerCount > 0) ? pApp->tDoc.tTimeline.arrLayers[0].iId : 0;
	xgeRenderRequest();
	return 0;
}

int afAppSaveDocument(af_app_t* pApp, const char* sPath)
{
	int ret;
	if ( pApp == NULL || sPath == NULL ) return -1;
	ret = afFileSave(&pApp->tDoc, sPath);
	if ( ret == 0 ) {
		pApp->tDoc.bModified = 0;
		snprintf(pApp->tDoc.sFilePath, sizeof(pApp->tDoc.sFilePath), "%s", sPath);
	}
	xgeRenderRequest();
	return ret;
}

/* ------------------------------------------------------------------ */
/* Tool / Playback                                                    */
/* ------------------------------------------------------------------ */

void afAppSetTool(af_app_t* pApp, int iTool)
{
	if ( pApp == NULL ) return;
	if ( iTool < 0 || iTool >= AF_TOOL_COUNT ) return;
	pApp->iActiveTool = iTool;
	xgeRenderRequest();
}

void afAppPlay(af_app_t* pApp)
{
	if ( pApp == NULL ) return;
	afPlaybackSyncDoc(&pApp->tPlayback, &pApp->tDoc);
	afPlaybackGotoFrame(&pApp->tPlayback, (uint32_t)pApp->iCurrentFrame);
	afPlaybackPlay(&pApp->tPlayback);
	pApp->bPlaying = 1;
}

void afAppStop(af_app_t* pApp)
{
	if ( pApp == NULL ) return;
	afPlaybackStop(&pApp->tPlayback);
	pApp->bPlaying = 0;
	pApp->iCurrentFrame = 0;
	xgeRenderRequest();
}

void afAppStepFrame(af_app_t* pApp, int iDelta)
{
	if ( pApp == NULL ) return;
	afPlaybackStop(&pApp->tPlayback);
	pApp->bPlaying = 0;
	if ( iDelta > 0 ) afPlaybackStepForward(&pApp->tPlayback);
	else afPlaybackStepBack(&pApp->tPlayback);
	pApp->iCurrentFrame = (int)pApp->tPlayback.iCurrentFrame;
	xgeRenderRequest();
}
