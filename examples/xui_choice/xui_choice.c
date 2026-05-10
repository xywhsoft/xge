#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tCheckOff;
	xge_texture_t tCheckOn;
	xge_texture_t tRadioOff;
	xge_texture_t tRadioOn;
	xge_texture_t tToggleOff;
	xge_texture_t tToggleOn;
	xge_xui_widget pRoot;
	xge_xui_widget pLabels[6];
	xge_xui_widget pCheckWidgets[4];
	xge_xui_widget pRadioWidgets[4];
	xge_xui_widget pToggleWidgets[6];
	xge_xui_widget pGroupWidgets[3];
	xge_xui_label_t tLabels[6];
	xge_xui_checkbox_t tChecks[4];
	xge_xui_radio_t tRadios[4];
	xge_xui_toggle_t tToggles[6];
	xge_xui_radio_group_t tGroup;
	xge_xui_radio_t tGroupRadios[3];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void PutPixelRGBA(unsigned char* pPixels, int iW, int x, int y, uint32_t iColor)
{
	int iOffset;

	iOffset = (y * iW + x) * 4;
	pPixels[iOffset + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
	pPixels[iOffset + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
	pPixels[iOffset + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
	pPixels[iOffset + 3] = (unsigned char)XGE_COLOR_GET_A(iColor);
}

static void FillChoiceTexturePixels(unsigned char* pPixels, int iW, int iH, uint32_t iFill, uint32_t iStroke, int iCircle, int bChecked)
{
	int x;
	int y;
	float fCx;
	float fCy;
	float fR;
	float fD;

	fCx = (float)(iW - 1) * 0.5f;
	fCy = (float)(iH - 1) * 0.5f;
	fR = (float)iW * 0.45f;
	memset(pPixels, 0, (size_t)iW * (size_t)iH * 4);
	for ( y = 0; y < iH; y++ ) {
		for ( x = 0; x < iW; x++ ) {
			if ( iCircle ) {
				fD = ((float)x - fCx) * ((float)x - fCx) + ((float)y - fCy) * ((float)y - fCy);
				if ( fD <= fR * fR && fD >= (fR - 2.0f) * (fR - 2.0f) ) {
					PutPixelRGBA(pPixels, iW, x, y, iStroke);
				} else if ( bChecked && fD <= (fR - 7.0f) * (fR - 7.0f) ) {
					PutPixelRGBA(pPixels, iW, x, y, iFill);
				} else if ( fD <= (fR - 2.0f) * (fR - 2.0f) ) {
					PutPixelRGBA(pPixels, iW, x, y, XGE_COLOR_RGBA(255, 255, 255, 255));
				}
			} else {
				if ( x < 2 || y < 2 || x >= iW - 2 || y >= iH - 2 ) {
					PutPixelRGBA(pPixels, iW, x, y, iStroke);
				} else {
					PutPixelRGBA(pPixels, iW, x, y, iFill);
				}
			}
		}
	}
	if ( !iCircle && bChecked ) {
		for ( y = 12; y < 19; y++ ) {
			PutPixelRGBA(pPixels, iW, 8 + (y - 12), y, XGE_COLOR_RGBA(255, 255, 255, 255));
			PutPixelRGBA(pPixels, iW, 9 + (y - 12), y, XGE_COLOR_RGBA(255, 255, 255, 255));
		}
		for ( y = 18; y > 7; y-- ) {
			PutPixelRGBA(pPixels, iW, 15 + (18 - y), y, XGE_COLOR_RGBA(255, 255, 255, 255));
			PutPixelRGBA(pPixels, iW, 16 + (18 - y), y, XGE_COLOR_RGBA(255, 255, 255, 255));
		}
	}
}

static void FillToggleTexturePixels(unsigned char* pPixels, int iW, int iH, uint32_t iTrack, uint32_t iStroke, int bChecked)
{
	int x;
	int y;
	float fRadius;
	float fCxLeft;
	float fCxRight;
	float fCy;
	float fKnobCx;
	float fKnobR;
	float fDLeft;
	float fDRight;
	float fDKnob;

	memset(pPixels, 0, (size_t)iW * (size_t)iH * 4);
	fRadius = (float)iH * 0.5f - 1.0f;
	fCxLeft = fRadius + 1.0f;
	fCxRight = (float)iW - fRadius - 2.0f;
	fCy = (float)iH * 0.5f;
	fKnobR = fRadius - 4.0f;
	fKnobCx = bChecked ? (fCxRight - 1.0f) : (fCxLeft + 1.0f);
	for ( y = 0; y < iH; y++ ) {
		for ( x = 0; x < iW; x++ ) {
			fDLeft = ((float)x - fCxLeft) * ((float)x - fCxLeft) + ((float)y - fCy) * ((float)y - fCy);
			fDRight = ((float)x - fCxRight) * ((float)x - fCxRight) + ((float)y - fCy) * ((float)y - fCy);
			if ( ((float)x >= fCxLeft && (float)x <= fCxRight && fabsf((float)y - fCy) <= fRadius) || fDLeft <= fRadius * fRadius || fDRight <= fRadius * fRadius ) {
				PutPixelRGBA(pPixels, iW, x, y, iTrack);
				if ( fDLeft >= (fRadius - 1.0f) * (fRadius - 1.0f) || fDRight >= (fRadius - 1.0f) * (fRadius - 1.0f) || y <= 1 || y >= iH - 2 ) {
					PutPixelRGBA(pPixels, iW, x, y, iStroke);
				}
			}
			fDKnob = ((float)x - fKnobCx) * ((float)x - fKnobCx) + ((float)y - fCy) * ((float)y - fCy);
			if ( fDKnob <= fKnobR * fKnobR ) {
				PutPixelRGBA(pPixels, iW, x, y, XGE_COLOR_RGBA(255, 255, 255, 255));
			}
		}
	}
}

static int MakeChoiceTexture(xge_texture pTexture, int iW, int iH, uint32_t iFill, uint32_t iStroke, int iCircle, int bChecked)
{
	unsigned char* pPixels;
	int iRet;

	pPixels = (unsigned char*)malloc((size_t)iW * (size_t)iH * 4);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	FillChoiceTexturePixels(pPixels, iW, iH, iFill, iStroke, iCircle, bChecked);
	iRet = xgeTextureCreateRGBA(pTexture, iW, iH, pPixels);
	free(pPixels);
	return iRet;
}

static int MakeToggleTexture(xge_texture pTexture, int iW, int iH, uint32_t iTrack, uint32_t iStroke, int bChecked)
{
	unsigned char* pPixels;
	int iRet;

	pPixels = (unsigned char*)malloc((size_t)iW * (size_t)iH * 4);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	FillToggleTexturePixels(pPixels, iW, iH, iTrack, iStroke, bChecked);
	iRet = xgeTextureCreateRGBA(pTexture, iW, iH, pPixels);
	free(pPixels);
	return iRet;
}

static int InitTextures(app_state_t* pApp)
{
	if ( MakeChoiceTexture(&pApp->tCheckOff, 28, 28, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(94, 110, 130, 255), 0, 0) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MakeChoiceTexture(&pApp->tCheckOn, 28, 28, XGE_COLOR_RGBA(68, 182, 122, 255), XGE_COLOR_RGBA(28, 112, 78, 255), 0, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MakeChoiceTexture(&pApp->tRadioOff, 28, 28, XGE_COLOR_RGBA(73, 144, 226, 255), XGE_COLOR_RGBA(104, 120, 136, 255), 1, 0) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MakeChoiceTexture(&pApp->tRadioOn, 28, 28, XGE_COLOR_RGBA(73, 144, 226, 255), XGE_COLOR_RGBA(40, 94, 160, 255), 1, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MakeToggleTexture(&pApp->tToggleOff, 64, 28, XGE_COLOR_RGBA(210, 218, 226, 255), XGE_COLOR_RGBA(122, 138, 154, 255), 0) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MakeToggleTexture(&pApp->tToggleOn, 64, 28, XGE_COLOR_RGBA(46, 184, 118, 255), XGE_COLOR_RGBA(20, 112, 82, 255), 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void AddTitle(app_state_t* pApp, int iIndex, const char* sText, float fY)
{
	xge_font pFont;

	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	pApp->pLabels[iIndex] = xgeXuiWidgetCreate();
	xgeXuiWidgetSetRect(pApp->pLabels[iIndex], (xge_rect_t){ 26.0f, fY, 720.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tLabels[iIndex], pApp->pLabels[iIndex], pFont, sText);
	xgeXuiLabelSetColor(&pApp->tLabels[iIndex], XGE_COLOR_RGBA(54, 82, 108, 255));
	xgeXuiWidgetAdd(pApp->pRoot, pApp->pLabels[iIndex]);
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	xge_xui_widget pXuiRoot;
	xge_font pFont;
	xge_rect_t tSrc28;
	xge_rect_t tSrc64;
	int i;

	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	if ( InitTextures(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pXuiRoot = xgeXuiRoot(&pApp->tXui);
	pApp->pRoot = xgeXuiWidgetCreate();
	xgeXuiWidgetSetRect(pApp->pRoot, (xge_rect_t){ 0.0f, 0.0f, 760.0f, 560.0f });
	xgeXuiWidgetSetBackground(pApp->pRoot, XGE_COLOR_RGBA(232, 240, 248, 255));
	xgeXuiWidgetAdd(pXuiRoot, pApp->pRoot);

	AddTitle(pApp, 0, "CheckBox", 24.0f);
	AddTitle(pApp, 1, "Radio", 120.0f);
	AddTitle(pApp, 2, "Toggle", 216.0f);
	AddTitle(pApp, 3, "Toggle no text", 304.0f);
	AddTitle(pApp, 4, "RadioGroup", 382.0f);
	AddTitle(pApp, 5, "Custom textures use larger artwork and stay vertically aligned with text", 506.0f);

	tSrc28 = (xge_rect_t){ 0.0f, 0.0f, 28.0f, 28.0f };
	tSrc64 = (xge_rect_t){ 0.0f, 0.0f, 64.0f, 28.0f };
	for ( i = 0; i < 4; i++ ) {
		pApp->pCheckWidgets[i] = xgeXuiWidgetCreate();
		xgeXuiWidgetSetRect(pApp->pCheckWidgets[i], (xge_rect_t){ 40.0f + (float)i * 170.0f, 62.0f, 150.0f, 34.0f });
		xgeXuiCheckBoxInit(&pApp->tChecks[i], &pApp->tXui, pApp->pCheckWidgets[i]);
		xgeXuiCheckBoxSetText(&pApp->tChecks[i], pFont, (i == 0) ? "Unchecked" : ((i == 1) ? "Checked" : ((i == 2) ? "Disabled" : "Texture")));
		if ( i == 1 || i == 2 || i == 3 ) {
			xgeXuiCheckBoxSetChecked(&pApp->tChecks[i], 1);
		}
		if ( i == 2 ) {
			xgeXuiWidgetSetEnabled(pApp->pCheckWidgets[i], 0);
		}
		if ( i == 3 ) {
			xgeXuiCheckBoxSetTextures(&pApp->tChecks[i], &pApp->tCheckOff, tSrc28, &pApp->tCheckOn, tSrc28);
			xgeXuiCheckBoxSetIndicatorSize(&pApp->tChecks[i], 28.0f);
		}
		xgeXuiWidgetAdd(pApp->pRoot, pApp->pCheckWidgets[i]);

		pApp->pRadioWidgets[i] = xgeXuiWidgetCreate();
		xgeXuiWidgetSetRect(pApp->pRadioWidgets[i], (xge_rect_t){ 40.0f + (float)i * 170.0f, 158.0f, 150.0f, 34.0f });
		xgeXuiRadioInit(&pApp->tRadios[i], &pApp->tXui, pApp->pRadioWidgets[i]);
		xgeXuiRadioSetText(&pApp->tRadios[i], pFont, (i == 0) ? "Unchecked" : ((i == 1) ? "Checked" : ((i == 2) ? "Disabled" : "Texture")));
		if ( i == 1 || i == 2 || i == 3 ) {
			xgeXuiRadioSetChecked(&pApp->tRadios[i], 1);
		}
		if ( i == 2 ) {
			xgeXuiWidgetSetEnabled(pApp->pRadioWidgets[i], 0);
		}
		if ( i == 3 ) {
			xgeXuiRadioSetTextures(&pApp->tRadios[i], &pApp->tRadioOff, tSrc28, &pApp->tRadioOn, tSrc28);
			xgeXuiRadioSetIndicatorSize(&pApp->tRadios[i], 28.0f);
		}
		xgeXuiWidgetAdd(pApp->pRoot, pApp->pRadioWidgets[i]);

		pApp->pToggleWidgets[i] = xgeXuiWidgetCreate();
		xgeXuiWidgetSetRect(pApp->pToggleWidgets[i], (xge_rect_t){ 46.0f + (float)i * 170.0f, 254.0f, 120.0f, 34.0f });
		xgeXuiToggleInit(&pApp->tToggles[i], &pApp->tXui, pApp->pToggleWidgets[i]);
		xgeXuiToggleSetInnerText(&pApp->tToggles[i], pFont, "OFF", "ON");
		if ( i == 1 || i == 2 || i == 3 ) {
			xgeXuiToggleSetChecked(&pApp->tToggles[i], 1);
		}
		if ( i == 2 ) {
			xgeXuiWidgetSetEnabled(pApp->pToggleWidgets[i], 0);
		}
		if ( i == 3 ) {
			xgeXuiToggleSetTextures(&pApp->tToggles[i], &pApp->tToggleOff, tSrc64, &pApp->tToggleOn, tSrc64);
			xgeXuiToggleSetMetrics(&pApp->tToggles[i], 64.0f, 28.0f, 2.0f, 7.0f, 5.0f);
		}
		xgeXuiWidgetAdd(pApp->pRoot, pApp->pToggleWidgets[i]);
	}
	for ( i = 4; i < 6; i++ ) {
		pApp->pToggleWidgets[i] = xgeXuiWidgetCreate();
		xgeXuiWidgetSetRect(pApp->pToggleWidgets[i], (xge_rect_t){ 46.0f + (float)(i - 4) * 170.0f, 342.0f, 120.0f, 34.0f });
		xgeXuiToggleInit(&pApp->tToggles[i], &pApp->tXui, pApp->pToggleWidgets[i]);
		xgeXuiToggleSetInnerText(&pApp->tToggles[i], NULL, NULL, NULL);
		if ( i == 5 ) {
			xgeXuiToggleSetChecked(&pApp->tToggles[i], 1);
		}
		xgeXuiWidgetAdd(pApp->pRoot, pApp->pToggleWidgets[i]);
	}

	xgeXuiRadioGroupInit(&pApp->tGroup);
	for ( i = 0; i < 3; i++ ) {
		pApp->pGroupWidgets[i] = xgeXuiWidgetCreate();
		xgeXuiWidgetSetRect(pApp->pGroupWidgets[i], (xge_rect_t){ 40.0f + (float)i * 170.0f, 420.0f, 150.0f, 34.0f });
		xgeXuiRadioInit(&pApp->tGroupRadios[i], &pApp->tXui, pApp->pGroupWidgets[i]);
		xgeXuiRadioSetText(&pApp->tGroupRadios[i], pFont, (i == 0) ? "Easy" : ((i == 1) ? "Normal" : "Hard"));
		xgeXuiRadioSetGroup(&pApp->tGroupRadios[i], &pApp->tGroup, i + 1);
		xgeXuiWidgetAdd(pApp->pRoot, pApp->pGroupWidgets[i]);
	}
	xgeXuiRadioGroupSetSelected(&pApp->tGroup, 2);
	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	int i;

	for ( i = 0; i < 6; i++ ) {
		xgeXuiToggleUnit(&pApp->tToggles[i]);
	}
	for ( i = 0; i < 4; i++ ) {
		xgeXuiRadioUnit(&pApp->tRadios[i]);
		xgeXuiCheckBoxUnit(&pApp->tChecks[i]);
	}
	for ( i = 0; i < 3; i++ ) {
		xgeXuiRadioUnit(&pApp->tGroupRadios[i]);
	}
	xgeXuiRadioGroupUnit(&pApp->tGroup);
	for ( i = 0; i < 6; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabels[i]);
	}
	xgeTextureFree(&pApp->tCheckOff);
	xgeTextureFree(&pApp->tCheckOn);
	xgeTextureFree(&pApp->tRadioOff);
	xgeTextureFree(&pApp->tRadioOn);
	xgeTextureFree(&pApp->tToggleOff);
	xgeTextureFree(&pApp->tToggleOn);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 240, 248, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui-choice summary frames=%d group=%d toggle=%d\n", pApp->iFrameCount, xgeXuiRadioGroupGetSelected(&pApp->tGroup), xgeXuiToggleGetChecked(&pApp->tToggles[1]));
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	iFrameLimit = ArgInt(getenv("XGE_XUI_CHOICE_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Choice";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}
