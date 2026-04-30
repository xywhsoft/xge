#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t app_state_t;
typedef struct login_scene_t login_scene_t;
typedef struct game_scene_t game_scene_t;

static const char* g_arrPresetItems[] = {
	"Admin preset",
	"Guest preset",
	"Clear fields"
};

struct login_scene_t {
	xge_scene_t tScene;
	app_state_t* pApp;
	xge_xui_context_t tXui;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pLoginPanelWidget;
	xge_xui_widget pBannerWidget;
	xge_xui_widget pUserLabelWidget;
	xge_xui_widget pUserInputWidget;
	xge_xui_widget pPasswordLabelWidget;
	xge_xui_widget pPasswordInputWidget;
	xge_xui_widget pRememberWidget;
	xge_xui_widget pLoginButtonWidget;
	xge_xui_widget pCloseButtonWidget;
	xge_xui_widget pMenuOwnerWidget;
	xge_xui_widget pMenuOwnerLabelWidget;
	xge_xui_widget pHintWidget;
	xge_xui_widget pDialogWidget;
	xge_xui_widget pDialogLabelWidget;
	xge_xui_panel_t tLoginPanel;
	xge_xui_image_t tBannerImage;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tUserLabel;
	xge_xui_label_t tPasswordLabel;
	xge_xui_label_t tMenuOwnerLabel;
	xge_xui_label_t tHint;
	xge_xui_label_t tDialogLabel;
	xge_xui_input_t tUserInput;
	xge_xui_input_t tPasswordInput;
	xge_xui_checkbox_t tRememberCheckBox;
	xge_xui_button_t tLoginButton;
	xge_xui_button_t tCloseButton;
	xge_xui_menu_t tPresetMenu;
	xge_xui_dialog_t tDialog;
	int iLastWidth;
	int iLastHeight;
	int iAutoStage;
};

struct game_scene_t {
	xge_scene_t tScene;
	app_state_t* pApp;
	int iEnterCount;
	int iUpdateCount;
	int iDrawCount;
};

struct app_state_t {
	xge_font_t tFont;
	xge_texture_t tBannerTexture;
	login_scene_t tLoginScene;
	game_scene_t tGameScene;
	int bFontReady;
	int bTextureReady;
	int iFrameLimit;
	int iTotalFrames;
	int iLoginAttempts;
	int iLoginSuccessCount;
	int iRememberChangeCount;
	int iLoginButtonCallbackCount;
	int iCloseButtonCallbackCount;
	int iMenuSelectCount;
	int iDialogCloseCount;
	int iLoginEnterCount;
	int iLoginLeaveCount;
	int iGameEnterCount;
	int iGameLeaveCount;
	int iLastPresetIndex;
	int bLayoutOK;
	int bInputPasswordOK;
	int bButtonCheckBoxOK;
	int bFeedbackOK;
	int bSceneSwitchOK;
	int bPendingSceneSwitch;
	int bRememberChecked;
	char sSceneMessage[128];
};

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;
	xge_rect_t tRect;

	tRect = xgeXuiWidgetGetRect(pWidget);
	tCenter.fX = tRect.fX + (tRect.fW * 0.5f);
	tCenter.fY = tRect.fY + (tRect.fH * 0.5f);
	return tCenter;
}

static int LoadSharedFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("game-login-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("game-login-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateBannerTexture(app_state_t* pApp)
{
	unsigned char arrPixels[240 * 64 * 4];
	int x;
	int y;
	int i;

	for ( y = 0; y < 64; y++ ) {
		for ( x = 0; x < 240; x++ ) {
			unsigned char iR;
			unsigned char iG;
			unsigned char iB;

			i = (y * 240 + x) * 4;
			iR = (unsigned char)(26 + (x * 34 / 239));
			iG = (unsigned char)(72 + (y * 46 / 63));
			iB = (unsigned char)(126 + ((x + y) * 64 / 302));
			if ( ((x / 12) % 2) == 0 ) {
				iR = (unsigned char)(iR + 10);
				iG = (unsigned char)(iG + 8);
			}
			if ( y < 8 ) {
				iR = (unsigned char)(iR + 30);
				iG = (unsigned char)(iG + 24);
				iB = (unsigned char)(iB + 18);
			}
			arrPixels[i + 0] = iR;
			arrPixels[i + 1] = iG;
			arrPixels[i + 2] = iB;
			arrPixels[i + 3] = 255;
		}
	}

	if ( xgeTextureCreateRGBA(&pApp->tBannerTexture, 240, 64, arrPixels) != XGE_OK ) {
		printf("game-login-lab banner texture create failed\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pApp->bTextureReady = 1;
	return XGE_OK;
}

static void RememberChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	login_scene_t* pLogin;
	app_state_t* pApp;

	(void)pWidget;
	pLogin = (login_scene_t*)pUser;
	pApp = pLogin->pApp;
	pApp->iRememberChangeCount++;
	pApp->bRememberChecked = bChecked;
}

static void DialogClose(xge_xui_widget pWidget, void* pUser)
{
	login_scene_t* pLogin;
	app_state_t* pApp;

	(void)pWidget;
	pLogin = (login_scene_t*)pUser;
	pApp = pLogin->pApp;
	pApp->iDialogCloseCount++;
}

static void PresetSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	login_scene_t* pLogin;
	app_state_t* pApp;

	(void)pWidget;
	pLogin = (login_scene_t*)pUser;
	pApp = pLogin->pApp;
	pApp->iMenuSelectCount++;
	pApp->iLastPresetIndex = iIndex;
	if ( iIndex == 0 ) {
		xgeXuiInputSetText(&pLogin->tUserInput, "admin");
		xgeXuiInputSetText(&pLogin->tPasswordInput, "admin");
	} else if ( iIndex == 1 ) {
		xgeXuiInputSetText(&pLogin->tUserInput, "guest");
		xgeXuiInputSetText(&pLogin->tPasswordInput, "guest");
	} else {
		xgeXuiInputSetText(&pLogin->tUserInput, "");
		xgeXuiInputSetText(&pLogin->tPasswordInput, "");
	}
}

static int MenuOwnerEvent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	login_scene_t* pLogin;
	xge_rect_t tRect;

	pLogin = (login_scene_t*)pUser;
	tRect = xgeXuiWidgetGetRect(pWidget);
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		xgeXuiMenuOpen(&pLogin->tPresetMenu, tRect.fX, tRect.fY + tRect.fH);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && ((pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE)) ) {
		xgeXuiMenuOpen(&pLogin->tPresetMenu, tRect.fX, tRect.fY + tRect.fH);
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void LoginClick(xge_xui_widget pWidget, void* pUser)
{
	login_scene_t* pLogin;
	app_state_t* pApp;
	const char* sUser;
	const char* sPassword;
	int bValid;

	(void)pWidget;
	pLogin = (login_scene_t*)pUser;
	pApp = pLogin->pApp;
	pApp->iLoginButtonCallbackCount++;
	pApp->iLoginAttempts++;
	sUser = xgeXuiInputGetText(&pLogin->tUserInput);
	sPassword = xgeXuiInputGetText(&pLogin->tPasswordInput);
	bValid =
		((strcmp(sUser, "admin") == 0) && (strcmp(sPassword, "admin") == 0)) ||
		((strcmp(sUser, "guest") == 0) && (strcmp(sPassword, "guest") == 0));
	if ( bValid ) {
		pApp->iLoginSuccessCount++;
		pApp->bPendingSceneSwitch = 1;
		snprintf(
			pApp->sSceneMessage,
			sizeof(pApp->sSceneMessage),
			"user=%s remember=%d preset=%d",
			sUser,
			xgeXuiCheckBoxGetChecked(&pLogin->tRememberCheckBox),
			pApp->iLastPresetIndex);
		return;
	}
	xgeXuiDialogSetTitle(&pLogin->tDialog, pApp->bFontReady ? &pApp->tFont : NULL, "Login failed");
	xgeXuiLabelSetText(&pLogin->tDialogLabel, "Credentials do not match the selected preset.\nPress ESC or click close to continue.");
	xgeXuiDialogSetOpen(&pLogin->tDialog, 1);
}

static void CloseClick(xge_xui_widget pWidget, void* pUser)
{
	login_scene_t* pLogin;
	app_state_t* pApp;

	(void)pWidget;
	pLogin = (login_scene_t*)pUser;
	pApp = pLogin->pApp;
	pApp->iCloseButtonCallbackCount++;
	xgeQuit();
}

static void LayoutLoginScene(login_scene_t* pLogin)
{
	xge_rect_t tRoot;
	float fPanelW;
	float fPanelH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pLogin->iLastWidth) && (iHeight == pLogin->iLastHeight) ) {
		return;
	}

	tRoot.fX = 18.0f;
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 36.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 700.0f ) {
		tRoot.fW = 700.0f;
	}
	if ( tRoot.fH < 420.0f ) {
		tRoot.fH = 420.0f;
	}
	fPanelW = 500.0f;
	fPanelH = 292.0f;

	xgeXuiWidgetSetRect(pLogin->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pLogin->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pLogin->pLoginPanelWidget, (xge_rect_t){ (tRoot.fW - fPanelW) * 0.5f, 68.0f, fPanelW, fPanelH });
	xgeXuiWidgetSetRect(pLogin->pBannerWidget, (xge_rect_t){ 18.0f, 34.0f, fPanelW - 36.0f, 72.0f });
	xgeXuiWidgetSetRect(pLogin->pUserLabelWidget, (xge_rect_t){ 24.0f, 124.0f, 92.0f, 24.0f });
	xgeXuiWidgetSetRect(pLogin->pUserInputWidget, (xge_rect_t){ 118.0f, 120.0f, 258.0f, 32.0f });
	xgeXuiWidgetSetRect(pLogin->pMenuOwnerWidget, (xge_rect_t){ 384.0f, 120.0f, 92.0f, 32.0f });
	xgeXuiWidgetSetRect(pLogin->pMenuOwnerLabelWidget, (xge_rect_t){ 0.0f, 0.0f, 92.0f, 18.0f });
	xgeXuiWidgetSetRect(pLogin->pPasswordLabelWidget, (xge_rect_t){ 24.0f, 166.0f, 92.0f, 24.0f });
	xgeXuiWidgetSetRect(pLogin->pPasswordInputWidget, (xge_rect_t){ 118.0f, 162.0f, 358.0f, 32.0f });
	xgeXuiWidgetSetRect(pLogin->pRememberWidget, (xge_rect_t){ 118.0f, 208.0f, 170.0f, 24.0f });
	xgeXuiWidgetSetRect(pLogin->pLoginButtonWidget, (xge_rect_t){ 118.0f, 248.0f, 128.0f, 36.0f });
	xgeXuiWidgetSetRect(pLogin->pCloseButtonWidget, (xge_rect_t){ 258.0f, 248.0f, 128.0f, 36.0f });
	xgeXuiWidgetSetRect(pLogin->pHintWidget, (xge_rect_t){ 24.0f, 252.0f, 452.0f, 20.0f });
	xgeXuiWidgetSetRect(pLogin->pDialogWidget, (xge_rect_t){ (tRoot.fW - 300.0f) * 0.5f, 110.0f, 300.0f, 148.0f });
	xgeXuiWidgetSetRect(pLogin->pDialogLabelWidget, (xge_rect_t){ 16.0f, 42.0f, 236.0f, 64.0f });

	pLogin->iLastWidth = iWidth;
	pLogin->iLastHeight = iHeight;
}

static void UpdateLoginStatus(login_scene_t* pLogin)
{
	app_state_t* pApp;
	char sText[256];

	pApp = pLogin->pApp;
	snprintf(
		sText,
		sizeof(sText),
		"layout=%d input=%d controls=%d feedback=%d scene=%d attempts=%d success=%d remember=%d menu=%d dialog=%d",
		pApp->bLayoutOK,
		pApp->bInputPasswordOK,
		pApp->bButtonCheckBoxOK,
		pApp->bFeedbackOK,
		pApp->bSceneSwitchOK,
		pApp->iLoginAttempts,
		pApp->iLoginSuccessCount,
		pApp->bRememberChecked,
		pApp->iMenuSelectCount,
		pApp->iDialogCloseCount);
	xgeXuiLabelSetText(&pLogin->tStatusLabel, sText);
}

static int CreateLoginUI(login_scene_t* pLogin)
{
	app_state_t* pApp;
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;
	xge_font pFont;

	pApp = pLogin->pApp;
	pRoot = xgeXuiRoot(&pLogin->tXui);
	pOverlayRoot = xgeXuiOverlayRoot(&pLogin->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( (pRoot == NULL) || (pOverlayRoot == NULL) ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 98, 122, 255);
	tTheme.iSelectionColor = XGE_COLOR_RGBA(96, 164, 255, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(96, 214, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(56, 124, 214, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(78, 146, 236, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(42, 96, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(86, 108, 144, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pLogin->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pLogin->tXui, pFont);

	pLogin->pRootPanel = xgeXuiWidgetCreate();
	pLogin->pStatusWidget = xgeXuiWidgetCreate();
	pLogin->pLoginPanelWidget = xgeXuiWidgetCreate();
	pLogin->pBannerWidget = xgeXuiWidgetCreate();
	pLogin->pUserLabelWidget = xgeXuiWidgetCreate();
	pLogin->pUserInputWidget = xgeXuiWidgetCreate();
	pLogin->pPasswordLabelWidget = xgeXuiWidgetCreate();
	pLogin->pPasswordInputWidget = xgeXuiWidgetCreate();
	pLogin->pRememberWidget = xgeXuiWidgetCreate();
	pLogin->pLoginButtonWidget = xgeXuiWidgetCreate();
	pLogin->pCloseButtonWidget = xgeXuiWidgetCreate();
	pLogin->pMenuOwnerWidget = xgeXuiWidgetCreate();
	pLogin->pMenuOwnerLabelWidget = xgeXuiWidgetCreate();
	pLogin->pHintWidget = xgeXuiWidgetCreate();
	pLogin->pDialogWidget = xgeXuiWidgetCreate();
	pLogin->pDialogLabelWidget = xgeXuiWidgetCreate();
	if ( (pLogin->pRootPanel == NULL) || (pLogin->pStatusWidget == NULL) || (pLogin->pLoginPanelWidget == NULL) || (pLogin->pBannerWidget == NULL) || (pLogin->pUserLabelWidget == NULL) || (pLogin->pUserInputWidget == NULL) || (pLogin->pPasswordLabelWidget == NULL) || (pLogin->pPasswordInputWidget == NULL) || (pLogin->pRememberWidget == NULL) || (pLogin->pLoginButtonWidget == NULL) || (pLogin->pCloseButtonWidget == NULL) || (pLogin->pMenuOwnerWidget == NULL) || (pLogin->pMenuOwnerLabelWidget == NULL) || (pLogin->pHintWidget == NULL) || (pLogin->pDialogWidget == NULL) || (pLogin->pDialogLabelWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pLogin->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pLogin->pRootPanel, XGE_COLOR_RGBA(18, 24, 34, 255));
	xgeXuiWidgetSetRadius(pLogin->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pLogin->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pLogin->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pLogin->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pLogin->tStatusLabel, pLogin->pStatusWidget, pFont, "game login lab");
	xgeXuiLabelSetColor(&pLogin->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pLogin->pRootPanel, pLogin->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pLogin->tLoginPanel, pLogin->pLoginPanelWidget);
	xgeXuiPanelSetTitle(&pLogin->tLoginPanel, pFont, "Game Login");
	xgeXuiPanelSetBackground(&pLogin->tLoginPanel, XGE_COLOR_RGBA(32, 40, 54, 255));
	xgeXuiPanelSetClip(&pLogin->tLoginPanel, 1);
	if ( xgeXuiWidgetAdd(pLogin->pRootPanel, pLogin->pLoginPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiImageInit(&pLogin->tBannerImage, pLogin->pBannerWidget, pApp->bTextureReady ? &pApp->tBannerTexture : NULL);
	xgeXuiImageSetMode(&pLogin->tBannerImage, XGE_XUI_IMAGE_STRETCH);
	xgeXuiImageSetColor(&pLogin->tBannerImage, XGE_COLOR_RGBA(255, 255, 255, 255));
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pBannerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pLogin->tUserLabel, pLogin->pUserLabelWidget, pFont, "User");
	xgeXuiLabelSetColor(&pLogin->tUserLabel, XGE_COLOR_RGBA(226, 232, 240, 255));
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pUserLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetPaddingPx(pLogin->pUserInputWidget, 8.0f, 6.0f, 8.0f, 6.0f);
	xgeXuiInputInit(&pLogin->tUserInput, &pLogin->tXui, pLogin->pUserInputWidget, pFont);
	xgeXuiInputSetPlaceholder(&pLogin->tUserInput, "admin / guest");
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pUserInputWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pLogin->pMenuOwnerWidget, XGE_COLOR_RGBA(54, 68, 92, 255));
	xgeXuiWidgetSetPaddingPx(pLogin->pMenuOwnerWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetFocusable(pLogin->pMenuOwnerWidget, 1);
	xgeXuiWidgetSetCaptureEventUser(pLogin->pMenuOwnerWidget, MenuOwnerEvent, pLogin);
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pMenuOwnerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pLogin->tMenuOwnerLabel, pLogin->pMenuOwnerLabelWidget, pFont, "Preset");
	xgeXuiLabelSetColor(&pLogin->tMenuOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetAlign(&pLogin->tMenuOwnerLabel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	if ( xgeXuiWidgetAdd(pLogin->pMenuOwnerWidget, pLogin->pMenuOwnerLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pLogin->tPasswordLabel, pLogin->pPasswordLabelWidget, pFont, "Password");
	xgeXuiLabelSetColor(&pLogin->tPasswordLabel, XGE_COLOR_RGBA(226, 232, 240, 255));
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pPasswordLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetPaddingPx(pLogin->pPasswordInputWidget, 8.0f, 6.0f, 8.0f, 6.0f);
	xgeXuiInputInit(&pLogin->tPasswordInput, &pLogin->tXui, pLogin->pPasswordInputWidget, pFont);
	xgeXuiInputSetPlaceholder(&pLogin->tPasswordInput, "password");
	xgeXuiInputSetPassword(&pLogin->tPasswordInput, 1);
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pPasswordInputWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiCheckBoxInit(&pLogin->tRememberCheckBox, &pLogin->tXui, pLogin->pRememberWidget);
	xgeXuiCheckBoxSetText(&pLogin->tRememberCheckBox, pFont, "Remember me");
	xgeXuiCheckBoxSetChange(&pLogin->tRememberCheckBox, RememberChange, pLogin);
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pRememberWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiButtonInit(&pLogin->tLoginButton, &pLogin->tXui, pLogin->pLoginButtonWidget);
	xgeXuiButtonSetText(&pLogin->tLoginButton, pFont, "Login");
	xgeXuiButtonSetClick(&pLogin->tLoginButton, LoginClick, pLogin);
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pLoginButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiButtonInit(&pLogin->tCloseButton, &pLogin->tXui, pLogin->pCloseButtonWidget);
	xgeXuiButtonSetText(&pLogin->tCloseButton, pFont, "Close");
	xgeXuiButtonSetClick(&pLogin->tCloseButton, CloseClick, pLogin);
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pCloseButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pLogin->tHint, pLogin->pHintWidget, pFont, "Try preset menu, wrong password dialog, then successful scene switch.");
	xgeXuiLabelSetColor(&pLogin->tHint, XGE_COLOR_RGBA(170, 188, 210, 255));
	if ( xgeXuiWidgetAdd(pLogin->pLoginPanelWidget, pLogin->pHintWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiMenuInit(&pLogin->tPresetMenu, &pLogin->tXui, pLogin->pMenuOwnerWidget);
	xgeXuiMenuSetFont(&pLogin->tPresetMenu, pFont);
	xgeXuiMenuSetItems(&pLogin->tPresetMenu, g_arrPresetItems, (int)(sizeof(g_arrPresetItems) / sizeof(g_arrPresetItems[0])));
	xgeXuiMenuSetSelect(&pLogin->tPresetMenu, PresetSelect, pLogin);
	xgeXuiMenuSetSize(&pLogin->tPresetMenu, 176.0f, 120.0f);

	xgeXuiDialogInit(&pLogin->tDialog, &pLogin->tXui, pLogin->pDialogWidget);
	xgeXuiDialogSetTitle(&pLogin->tDialog, pFont, "Login failed");
	xgeXuiDialogSetClose(&pLogin->tDialog, DialogClose, pLogin);
	xgeXuiDialogSetOpen(&pLogin->tDialog, 0);
	xgeXuiDialogSetModal(&pLogin->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pLogin->tDialog, 1);
	if ( xgeXuiWidgetAdd(pOverlayRoot, pLogin->pDialogWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pLogin->tDialogLabel, pLogin->pDialogLabelWidget, pFont, "Credentials do not match the selected preset.");
	xgeXuiLabelSetColor(&pLogin->tDialogLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pLogin->pDialogWidget, pLogin->pDialogLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	return XGE_OK;
}

static int LoginEnter(xge_scene pScene)
{
	login_scene_t* pLogin;
	app_state_t* pApp;

	pLogin = (login_scene_t*)pScene->pUser;
	pApp = pLogin->pApp;
	pApp->iLoginEnterCount++;
	if ( xgeXuiInit(&pLogin->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateLoginUI(pLogin) != XGE_OK ) {
		xgeXuiUnit(&pLogin->tXui);
		memset(&pLogin->tXui, 0, sizeof(pLogin->tXui));
		return XGE_ERROR;
	}
	LayoutLoginScene(pLogin);
	xgeXuiUpdate(&pLogin->tXui, 0.0f);
	pApp->bLayoutOK =
		(pLogin->tLoginPanel.pWidget == pLogin->pLoginPanelWidget) &&
		(pLogin->tBannerImage.pTexture == (pApp->bTextureReady ? &pApp->tBannerTexture : NULL)) &&
		(pLogin->tPresetMenu.pOwner == pLogin->pMenuOwnerWidget) &&
		(pLogin->tDialog.pWidget == pLogin->pDialogWidget);
	pApp->bInputPasswordOK =
		(pLogin->tUserInput.sPlaceholder != NULL) &&
		(pLogin->tPasswordInput.bPassword != 0) &&
		(pLogin->pUserInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pLogin->pPasswordInputWidget->procEvent == xgeXuiInputEventProc);
	UpdateLoginStatus(pLogin);
	return XGE_OK;
}

static int LoginLeave(xge_scene pScene)
{
	login_scene_t* pLogin;
	app_state_t* pApp;

	pLogin = (login_scene_t*)pScene->pUser;
	pApp = pLogin->pApp;
	pApp->iLoginLeaveCount++;
	xgeXuiLabelUnit(&pLogin->tDialogLabel);
	xgeXuiDialogUnit(&pLogin->tDialog);
	xgeXuiMenuUnit(&pLogin->tPresetMenu);
	xgeXuiLabelUnit(&pLogin->tHint);
	xgeXuiButtonUnit(&pLogin->tCloseButton);
	xgeXuiButtonUnit(&pLogin->tLoginButton);
	xgeXuiCheckBoxUnit(&pLogin->tRememberCheckBox);
	xgeXuiInputUnit(&pLogin->tPasswordInput);
	xgeXuiLabelUnit(&pLogin->tPasswordLabel);
	xgeXuiLabelUnit(&pLogin->tMenuOwnerLabel);
	xgeXuiInputUnit(&pLogin->tUserInput);
	xgeXuiLabelUnit(&pLogin->tUserLabel);
	xgeXuiImageUnit(&pLogin->tBannerImage);
	xgeXuiPanelUnit(&pLogin->tLoginPanel);
	xgeXuiLabelUnit(&pLogin->tStatusLabel);
	xgeXuiUnit(&pLogin->tXui);
	memset(&pLogin->tXui, 0, sizeof(pLogin->tXui));
	return XGE_OK;
}

static int LoginEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	login_scene_t* pLogin;

	pLogin = (login_scene_t*)pScene->pUser;
	if ( xgeXuiDispatchEvent(&pLogin->tXui, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	return XGE_OK;
}

static int RunLoginAutoFlow(login_scene_t* pLogin)
{
	app_state_t* pApp;
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	float fY;

	pApp = pLogin->pApp;
	switch ( pLogin->iAutoStage ) {
		case 0:
			tCenter = WidgetCenter(pLogin->pMenuOwnerWidget);
			xgeXuiMenuOpen(&pLogin->tPresetMenu, tCenter.fX, tCenter.fY + 16.0f);
			if ( xgeXuiMenuIsOpen(&pLogin->tPresetMenu) == 0 ) {
				break;
			}
			fY = pLogin->tPresetMenu.pListWidget->tContentRect.fY + pLogin->tPresetMenu.tList.fItemHeight * 0.5f;
			(void)fY;
			PresetSelect(NULL, 0, pLogin);
			xgeXuiMenuClose(&pLogin->tPresetMenu);
			pApp->bFeedbackOK =
				(pApp->iMenuSelectCount >= 1) &&
				(pApp->iLastPresetIndex == 0) &&
				(strcmp(xgeXuiInputGetText(&pLogin->tUserInput), "admin") == 0) &&
				(strcmp(xgeXuiInputGetText(&pLogin->tPasswordInput), "admin") == 0);
			pLogin->iAutoStage++;
			break;

		case 1:
			xgeXuiCheckBoxSetChecked(&pLogin->tRememberCheckBox, 1);
			RememberChange(NULL, 1, pLogin);
			pApp->bButtonCheckBoxOK =
				(pApp->iRememberChangeCount >= 1) &&
				(xgeXuiCheckBoxGetChecked(&pLogin->tRememberCheckBox) != 0);
			pLogin->iAutoStage++;
			break;

		case 2:
			xgeXuiInputSetText(&pLogin->tPasswordInput, "wrong");
			LoginClick(NULL, pLogin);
			pApp->bButtonCheckBoxOK =
				pApp->bButtonCheckBoxOK &&
				(pApp->iLoginButtonCallbackCount >= 1);
			pApp->bFeedbackOK =
				pApp->bFeedbackOK &&
				(pApp->iLoginAttempts == 1) &&
				(xgeXuiDialogIsOpen(&pLogin->tDialog) != 0);
			pLogin->iAutoStage++;
			break;

		case 3:
			MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
			xgeXuiDialogEvent(&pLogin->tDialog, &tEvent);
			pApp->bFeedbackOK =
				pApp->bFeedbackOK &&
				(xgeXuiDialogIsOpen(&pLogin->tDialog) == 0) &&
				(pApp->iDialogCloseCount >= 1);
			pLogin->iAutoStage++;
			break;

		case 4:
			xgeXuiInputSetText(&pLogin->tUserInput, "admin");
			xgeXuiInputSetText(&pLogin->tPasswordInput, "admin");
			LoginClick(NULL, pLogin);
			pApp->bButtonCheckBoxOK =
				pApp->bButtonCheckBoxOK &&
				(pApp->iLoginButtonCallbackCount >= 2);
			pLogin->iAutoStage++;
			if ( pApp->bPendingSceneSwitch != 0 ) {
				return xgeSceneReplace(&pApp->tGameScene.tScene);
			}
			break;
	}
	return XGE_OK;
}

static int LoginUpdate(xge_scene pScene, float fDelta)
{
	login_scene_t* pLogin;
	app_state_t* pApp;
	int iRet;

	pLogin = (login_scene_t*)pScene->pUser;
	pApp = pLogin->pApp;
	pApp->iTotalFrames++;
	LayoutLoginScene(pLogin);
	UpdateLoginStatus(pLogin);
	xgeXuiUpdate(&pLogin->tXui, fDelta);
	iRet = RunLoginAutoFlow(pLogin);
	if ( (pApp->bPendingSceneSwitch == 0) && (pApp->iTotalFrames >= ((pApp->iFrameLimit > 5 ? pApp->iFrameLimit : 5) * 4)) ) {
		xgeQuit();
	}
	return iRet;
}

static int LoginDraw(xge_scene pScene)
{
	login_scene_t* pLogin;

	pLogin = (login_scene_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(16, 20, 28, 255));
	xgeXuiPaint(&pLogin->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static int LoginFree(xge_scene pScene)
{
	(void)pScene;
	return XGE_OK;
}

static int GameEnter(xge_scene pScene)
{
	game_scene_t* pGame;
	app_state_t* pApp;

	pGame = (game_scene_t*)pScene->pUser;
	pApp = pGame->pApp;
	pApp->iGameEnterCount++;
	pApp->bSceneSwitchOK = (xgeSceneCurrent() == &pApp->tGameScene.tScene);
	return XGE_OK;
}

static int GameLeave(xge_scene pScene)
{
	game_scene_t* pGame;
	app_state_t* pApp;

	pGame = (game_scene_t*)pScene->pUser;
	pApp = pGame->pApp;
	pApp->iGameLeaveCount++;
	return XGE_OK;
}

static int GameEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	(void)pScene;
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	return XGE_OK;
}

static int GameUpdate(xge_scene pScene, float fDelta)
{
	game_scene_t* pGame;
	app_state_t* pApp;

	(void)fDelta;
	pGame = (game_scene_t*)pScene->pUser;
	pApp = pGame->pApp;
	pGame->iUpdateCount++;
	pApp->iTotalFrames++;
	if ( pApp->iTotalFrames >= pApp->iFrameLimit ) {
		xgeQuit();
	}
	return XGE_OK;
}

static int GameDraw(xge_scene pScene)
{
	game_scene_t* pGame;

	pGame = (game_scene_t*)pScene->pUser;
	pGame->iDrawCount++;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 36, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 72.0f, 72.0f, 520.0f, 196.0f }, XGE_COLOR_RGBA(34, 62, 48, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 72.0f, 72.0f, 520.0f, 196.0f }, 3.0f, XGE_COLOR_RGBA(122, 224, 152, 255));
	xgeShapeLinePx(96.0f, 132.0f, 560.0f, 132.0f, 2.0f, XGE_COLOR_RGBA(146, 232, 174, 255));
	xgeShapeCircleFillPx(162.0f, 204.0f, 22.0f, XGE_COLOR_RGBA(246, 196, 88, 255));
	xgeShapeCircleFillPx(232.0f, 204.0f, 22.0f, XGE_COLOR_RGBA(96, 184, 255, 255));
	xgeShapeCircleFillPx(302.0f, 204.0f, 22.0f, XGE_COLOR_RGBA(224, 120, 120, 255));
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static int GameFree(xge_scene pScene)
{
	(void)pScene;
	return XGE_OK;
}

static void SetupScenes(app_state_t* pApp)
{
	memset(&pApp->tLoginScene, 0, sizeof(pApp->tLoginScene));
	memset(&pApp->tGameScene, 0, sizeof(pApp->tGameScene));

	pApp->tLoginScene.pApp = pApp;
	pApp->tLoginScene.tScene.pUser = &pApp->tLoginScene;
	pApp->tLoginScene.tScene.onEnter = LoginEnter;
	pApp->tLoginScene.tScene.onLeave = LoginLeave;
	pApp->tLoginScene.tScene.onEvent = LoginEvent;
	pApp->tLoginScene.tScene.onUpdate = LoginUpdate;
	pApp->tLoginScene.tScene.onDraw = LoginDraw;
	pApp->tLoginScene.tScene.onFree = LoginFree;

	pApp->tGameScene.pApp = pApp;
	pApp->tGameScene.tScene.pUser = &pApp->tGameScene;
	pApp->tGameScene.tScene.onEnter = GameEnter;
	pApp->tGameScene.tScene.onLeave = GameLeave;
	pApp->tGameScene.tScene.onEvent = GameEvent;
	pApp->tGameScene.tScene.onUpdate = GameUpdate;
	pApp->tGameScene.tScene.onDraw = GameDraw;
	pApp->tGameScene.tScene.onFree = GameFree;
}

static void AppUnit(app_state_t* pApp)
{
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tBannerTexture);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;
	int i;
	int iRet;
	int bOK;

	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tApp.iFrameLimit = ArgInt(getenv("XGE_GAME_LOGIN_LAB_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	SetupScenes(&tApp);

	tDesc.iWidth = 760;
	tDesc.iHeight = 460;
	tDesc.sTitle = "XGE Game Login Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		return 1;
	}
	(void)LoadSharedFont(&tApp);
	(void)CreateBannerTexture(&tApp);
	iRet = xgeSceneSet(&tApp.tLoginScene.tScene);
	if ( iRet != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 2;
	}
	xgeRun(NULL, NULL);
	xgeSceneSet(NULL);
	bOK =
		tApp.bLayoutOK &&
		tApp.bInputPasswordOK &&
		tApp.bButtonCheckBoxOK &&
		tApp.bFeedbackOK &&
		tApp.bSceneSwitchOK &&
		(tApp.iLoginAttempts == 2) &&
		(tApp.iLoginSuccessCount == 1) &&
		(tApp.iGameEnterCount == 1);
	printf(
		"game-login-lab final-summary frames=%d layout=%d input=%d controls=%d feedback=%d scene=%d attempts=%d success=%d remember=%d menu=%d dialog=%d enter=%d/%d leave=%d/%d game_updates=%d draws=%d msg=%s\n",
		tApp.iTotalFrames,
		tApp.bLayoutOK,
		tApp.bInputPasswordOK,
		tApp.bButtonCheckBoxOK,
		tApp.bFeedbackOK,
		tApp.bSceneSwitchOK,
		tApp.iLoginAttempts,
		tApp.iLoginSuccessCount,
		tApp.bRememberChecked,
		tApp.iMenuSelectCount,
		tApp.iDialogCloseCount,
		tApp.iLoginEnterCount,
		tApp.iGameEnterCount,
		tApp.iLoginLeaveCount,
		tApp.iGameLeaveCount,
		tApp.tGameScene.iUpdateCount,
		tApp.tGameScene.iDrawCount,
		tApp.sSceneMessage[0] ? tApp.sSceneMessage : "none");
	AppUnit(&tApp);
	xgeUnit();
	return bOK ? 0 : 3;
}
