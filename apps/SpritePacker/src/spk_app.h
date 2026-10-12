#ifndef SPK_APP_H
#define SPK_APP_H

#include "xge.h"
#include "xui.h"

#define SPK_W 1400
#define SPK_H 900
#define SPK_MENU_H 26.0f
#define SPK_TOOLBAR_H 32.0f
#define SPK_STATUS_H 28.0f
#define SPK_MAX_IMAGES 256
#define SPK_MAX_SIZE 4096

/* Commands */
enum {
	SPK_CMD_PACK = 1,
	SPK_CMD_EXPORT,
	SPK_CMD_EXIT,
	SPK_CMD_ABOUT
};

/* Packed image entry */
typedef struct {
	char sName[128];
	char sPath[512];
	int iW, iH;
	int iX, iY; /* packed position */
	int bPacked;
} spk_image_t;

/* App state */
typedef struct {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	int iTargetW, iTargetH;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pFileMenu;
	xui_widget pHelpMenu;
	xui_widget pToolbar;
	xui_widget pStatus;
	xui_widget pDock;
	xui_widget pPreviewWidget;

	spk_image_t arrImages[SPK_MAX_IMAGES];
	int iImageCount;
	int iAtlasW, iAtlasH;
	int iPadding;
	int bPacked;

	int bMouse;
	float fMouseX, fMouseY;
	uint32_t iButtons;
	int bCreateOK, bRenderOK;
	int iFrame, iFrameLimit;
} spk_app_t;

int SpritePackerCreateUI(spk_app_t* pApp);
void SpritePackerDestroy(spk_app_t* pApp);
int SpritePackerFrame(void* pUser);
int SpritePackerHandleInput(spk_app_t* pApp);
int SpritePackerSyncViewport(spk_app_t* pApp);
int SpritePackerPreviewRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
void SpritePackerPack(spk_app_t* pApp);
int SpritePackerExport(spk_app_t* pApp, const char* sPath);

#endif
