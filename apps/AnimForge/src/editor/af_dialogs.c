/*
 * af_dialogs.c - Dialog windows
 *
 * New Document, Project Settings, Export Options, and About dialogs.
 */

#include "af_dialogs.h"
#include "af_draw_helper.h"
#include "../core/af_app.h"
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Dialog state                                                       */
/* ------------------------------------------------------------------ */

typedef struct af_dialog_state_t {
	int   iActiveDialog;      /* AF_DIALOG_* or 0 for none */
	int   bModal;

	/* New Document dialog */
	float fNewWidth;
	float fNewHeight;
	float fNewFrameRate;
	int   iNewFrameCount;
	uint32_t iNewBgColor;

	/* Project Settings dialog */
	float fSetWidth;
	float fSetHeight;
	float fSetFrameRate;
	uint32_t iSetBgColor;

	/* Export dialog */
	int   iExportType;        /* AF_EXPORT_XANIM / PNG_SEQ / GIF / SPRITE_SHEET */
	char  sExportPath[512];
	int   iExportStartFrame;
	int   iExportEndFrame;
	float fExportScale;

	/* Button hover states */
	int   iHoverButton;
} af_dialog_state_t;

static af_dialog_state_t s_tDlg;

/* ------------------------------------------------------------------ */
/* Dialog open/close                                                  */
/* ------------------------------------------------------------------ */

void afDialogOpenNewDocument(af_app_t* pApp)
{
	memset(&s_tDlg, 0, sizeof(s_tDlg));
	s_tDlg.iActiveDialog = AF_DIALOG_NEW_DOC;
	s_tDlg.bModal = 1;
	s_tDlg.fNewWidth = 640.0f;
	s_tDlg.fNewHeight = 480.0f;
	s_tDlg.fNewFrameRate = 24.0f;
	s_tDlg.iNewFrameCount = 100;
	s_tDlg.iNewBgColor = 0xFFFFFFFF;
}

void afDialogOpenProjectSettings(af_app_t* pApp)
{
	memset(&s_tDlg, 0, sizeof(s_tDlg));
	s_tDlg.iActiveDialog = AF_DIALOG_PROJECT_SETTINGS;
	s_tDlg.bModal = 1;
	s_tDlg.fSetWidth = pApp->tDoc.fStageWidth;
	s_tDlg.fSetHeight = pApp->tDoc.fStageHeight;
	s_tDlg.fSetFrameRate = pApp->tDoc.fFrameRate;
	s_tDlg.iSetBgColor = pApp->tDoc.iBackgroundColor;
}

void afDialogOpenExport(af_app_t* pApp)
{
	memset(&s_tDlg, 0, sizeof(s_tDlg));
	s_tDlg.iActiveDialog = AF_DIALOG_EXPORT;
	s_tDlg.bModal = 1;
	s_tDlg.iExportType = AF_EXPORT_XANIM;
	s_tDlg.iExportStartFrame = 0;
	s_tDlg.iExportEndFrame = (int)pApp->tDoc.iFrameCount - 1;
	s_tDlg.fExportScale = 1.0f;
	strcpy(s_tDlg.sExportPath, pApp->tDoc.sFilePath);
}

void afDialogOpenAbout(af_app_t* pApp)
{
	memset(&s_tDlg, 0, sizeof(s_tDlg));
	s_tDlg.iActiveDialog = AF_DIALOG_ABOUT;
	s_tDlg.bModal = 1;
}

void afDialogClose(void)
{
	s_tDlg.iActiveDialog = 0;
	s_tDlg.bModal = 0;
}

int afDialogIsActive(void)
{
	return s_tDlg.iActiveDialog != 0;
}

/* ------------------------------------------------------------------ */
/* Rendering                                                          */
/* ------------------------------------------------------------------ */

static void __afDrawDialogFrame(xui_proxy_t* pProxy, xui_draw_context pCtx, float fX, float fY,
                                float fW, float fH, const char* sTitle)
{
	/* Dim background */
	afDrawFillRect(pProxy, pCtx, 0, 0, 2000, 2000, XUI_COLOR_RGBA(0, 0, 0, 128));

	/* Dialog background */
	afDrawFillRect(pProxy, pCtx, fX, fY, fW, fH, AF_COLOR_PANEL);

	/* Title bar */
	afDrawFillRect(pProxy, pCtx, fX, fY, fW, 28, AF_COLOR_HIGHLIGHT);

	/* Title text placeholder */
	afDrawFillRect(pProxy, pCtx, fX + 10, fY + 9, (float)(strlen(sTitle) * 7), 10, AF_COLOR_TEXT);

	/* Border */
	afDrawStrokeRect(pProxy, pCtx, fX, fY, fW, fH, 1.0f, AF_COLOR_ACCENT);
}

static void __afDrawButton(xui_proxy_t* pProxy, xui_draw_context pCtx, float fX, float fY,
                           float fW, float fH, const char* sLabel, int bPrimary)
{
	uint32_t bgColor = bPrimary ? AF_COLOR_ACCENT : AF_COLOR_HIGHLIGHT;
	afDrawFillRect(pProxy, pCtx, fX, fY, fW, fH, bgColor);
	afDrawFillRect(pProxy, pCtx, fX + 8, fY + 6, (float)(strlen(sLabel) * 6), 8, AF_COLOR_TEXT);
}

static void __afDrawField(xui_proxy_t* pProxy, xui_draw_context pCtx, float fX, float fY,
                          float fW, const char* sLabel, float fValue)
{
	/* Label */
	afDrawFillRect(pProxy, pCtx, fX, fY + 4, (float)(strlen(sLabel) * 6), 8, AF_COLOR_TEXT_DIM);

	/* Input field */
	afDrawFillRect(pProxy, pCtx, fX + 100, fY, fW - 100, 20, XUI_COLOR_RGBA(60, 60, 64, 255));

	/* Value indicator */
	afDrawFillRect(pProxy, pCtx, fX + 104, fY + 6, 40, 8, AF_COLOR_TEXT);

	(void)fValue;
}

void afDialogRender(af_app_t* pApp, xui_proxy_t* pProxy, xui_draw_context pCtx, float fScreenW, float fScreenH)
{
	float dlgW, dlgH, dlgX, dlgY;

	if ( s_tDlg.iActiveDialog == 0 )
		return;

	switch ( s_tDlg.iActiveDialog ) {
	case AF_DIALOG_NEW_DOC:
		dlgW = 360; dlgH = 260;
		dlgX = (fScreenW - dlgW) * 0.5f;
		dlgY = (fScreenH - dlgH) * 0.5f;
		__afDrawDialogFrame(pProxy, pCtx, dlgX, dlgY, dlgW, dlgH, "New Document");
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 45, dlgW - 40, "Width", s_tDlg.fNewWidth);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 75, dlgW - 40, "Height", s_tDlg.fNewHeight);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 105, dlgW - 40, "Frame Rate", s_tDlg.fNewFrameRate);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 135, dlgW - 40, "Frames", (float)s_tDlg.iNewFrameCount);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 165, dlgW - 40, "Background", 0);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 180, dlgY + dlgH - 45, 80, 28, "Create", 1);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 90, dlgY + dlgH - 45, 70, 28, "Cancel", 0);
		break;

	case AF_DIALOG_PROJECT_SETTINGS:
		dlgW = 360; dlgH = 220;
		dlgX = (fScreenW - dlgW) * 0.5f;
		dlgY = (fScreenH - dlgH) * 0.5f;
		__afDrawDialogFrame(pProxy, pCtx, dlgX, dlgY, dlgW, dlgH, "Project Settings");
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 45, dlgW - 40, "Width", s_tDlg.fSetWidth);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 75, dlgW - 40, "Height", s_tDlg.fSetHeight);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 105, dlgW - 40, "Frame Rate", s_tDlg.fSetFrameRate);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 180, dlgY + dlgH - 45, 80, 28, "Apply", 1);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 90, dlgY + dlgH - 45, 70, 28, "Cancel", 0);
		break;

	case AF_DIALOG_EXPORT:
		dlgW = 400; dlgH = 240;
		dlgX = (fScreenW - dlgW) * 0.5f;
		dlgY = (fScreenH - dlgH) * 0.5f;
		__afDrawDialogFrame(pProxy, pCtx, dlgX, dlgY, dlgW, dlgH, "Export");
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 45, dlgW - 40, "Format", (float)s_tDlg.iExportType);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 75, dlgW - 40, "Start Frame", (float)s_tDlg.iExportStartFrame);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 105, dlgW - 40, "End Frame", (float)s_tDlg.iExportEndFrame);
		__afDrawField(pProxy, pCtx, dlgX + 20, dlgY + 135, dlgW - 40, "Scale", s_tDlg.fExportScale);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 180, dlgY + dlgH - 45, 80, 28, "Export", 1);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 90, dlgY + dlgH - 45, 70, 28, "Cancel", 0);
		break;

	case AF_DIALOG_ABOUT:
		dlgW = 300; dlgH = 160;
		dlgX = (fScreenW - dlgW) * 0.5f;
		dlgY = (fScreenH - dlgH) * 0.5f;
		__afDrawDialogFrame(pProxy, pCtx, dlgX, dlgY, dlgW, dlgH, "About AnimForge");
		afDrawFillRect(pProxy, pCtx, dlgX + 20, dlgY + 50, 180, 10, AF_COLOR_TEXT);
		afDrawFillRect(pProxy, pCtx, dlgX + 20, dlgY + 70, 220, 8, AF_COLOR_TEXT_DIM);
		afDrawFillRect(pProxy, pCtx, dlgX + 20, dlgY + 85, 160, 8, AF_COLOR_TEXT_DIM);
		__afDrawButton(pProxy, pCtx, dlgX + dlgW - 90, dlgY + dlgH - 45, 70, 28, "OK", 1);
		break;

	default:
		break;
	}
}

/* ------------------------------------------------------------------ */
/* Input handling                                                     */
/* ------------------------------------------------------------------ */

int afDialogMouseDown(af_app_t* pApp, float fX, float fY, float fScreenW, float fScreenH)
{
	float dlgW, dlgH, dlgX, dlgY;

	if ( s_tDlg.iActiveDialog == 0 )
		return 0;

	/* Calculate dialog bounds based on type */
	switch ( s_tDlg.iActiveDialog ) {
	case AF_DIALOG_NEW_DOC:       dlgW = 360; dlgH = 260; break;
	case AF_DIALOG_PROJECT_SETTINGS: dlgW = 360; dlgH = 220; break;
	case AF_DIALOG_EXPORT:        dlgW = 400; dlgH = 240; break;
	case AF_DIALOG_ABOUT:         dlgW = 300; dlgH = 160; break;
	default: return 0;
	}

	dlgX = (fScreenW - dlgW) * 0.5f;
	dlgY = (fScreenH - dlgH) * 0.5f;

	/* Check if click is inside dialog */
	if ( fX < dlgX || fX > dlgX + dlgW || fY < dlgY || fY > dlgY + dlgH ) {
		/* Click outside modal dialog - ignore or close */
		return 1; /* consume event */
	}

	/* Check Cancel/OK button area */
	if ( fY >= dlgY + dlgH - 45 && fY <= dlgY + dlgH - 17 ) {
		/* Cancel button */
		if ( fX >= dlgX + dlgW - 90 && fX <= dlgX + dlgW - 20 ) {
			afDialogClose();
			return 1;
		}
		/* Primary button (Create/Apply/Export/OK) */
		if ( fX >= dlgX + dlgW - 180 && fX <= dlgX + dlgW - 100 ) {
			switch ( s_tDlg.iActiveDialog ) {
			case AF_DIALOG_NEW_DOC:
				afAppNewDocument(pApp, s_tDlg.fNewWidth, s_tDlg.fNewHeight,
				                 s_tDlg.fNewFrameRate, (uint32_t)s_tDlg.iNewFrameCount);
				break;
			case AF_DIALOG_PROJECT_SETTINGS:
				afDocSetStage(&pApp->tDoc, s_tDlg.fSetWidth, s_tDlg.fSetHeight,
				              s_tDlg.fSetFrameRate, pApp->tDoc.iFrameCount);
				pApp->tDoc.iBackgroundColor = s_tDlg.iSetBgColor;
				break;
			case AF_DIALOG_EXPORT:
				/* Export handled by af_export module */
				break;
			default:
				break;
			}
			afDialogClose();
			return 1;
		}
	}

	return 1; /* consume all events while modal */
}

int afDialogKeyDown(af_app_t* pApp, int iKey)
{
	if ( s_tDlg.iActiveDialog == 0 )
		return 0;

	/* Escape closes dialog */
	if ( iKey == 256 ) {
		afDialogClose();
		return 1;
	}

	/* Enter activates primary button */
	if ( iKey == 257 ) {
		switch ( s_tDlg.iActiveDialog ) {
		case AF_DIALOG_NEW_DOC:
			afAppNewDocument(pApp, s_tDlg.fNewWidth, s_tDlg.fNewHeight,
			                 s_tDlg.fNewFrameRate, (uint32_t)s_tDlg.iNewFrameCount);
			break;
		case AF_DIALOG_ABOUT:
			break;
		default:
			break;
		}
		afDialogClose();
		return 1;
	}

	return 1; /* consume all keys while modal */
}
