#include "../../xge.h"
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256
#define XGE_EXAMPLE_KEY_SPACE	32

typedef struct scene_demo_t {
	xge_scene_t tMainScene;
	xge_scene_t tMenuScene;
	int bMenuOpen;
	float fTime;
} scene_demo_t;

static int MainUpdate(xge_scene pScene, float fDelta)
{
	scene_demo_t* pDemo;

	pDemo = (scene_demo_t*)pScene->pUser;
	pDemo->fTime += fDelta;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( (pDemo->bMenuOpen == 0) && xgeKeyPressed(XGE_EXAMPLE_KEY_SPACE) ) {
		pDemo->bMenuOpen = 1;
		return xgeScenePush(&pDemo->tMenuScene);
	}
	return 0;
}

static int MainDraw(xge_scene pScene)
{
	scene_demo_t* pDemo;
	xge_rect_t tRect;
	float fX;

	pDemo = (scene_demo_t*)pScene->pUser;
	xgeClear(XGE_COLOR_RGBA(18, 24, 30, 255));
	fX = 80.0f + (pDemo->fTime * 32.0f);
	while ( fX > 420.0f ) {
		fX -= 340.0f;
	}
	xgeShapeCircleFillPx(fX, 120.0f, 28.0f, XGE_COLOR_RGBA(255, 210, 96, 255));
	xgeShapeLinePx(48.0f, 190.0f, 420.0f, 190.0f, 4.0f, XGE_COLOR_RGBA(120, 200, 255, 255));
	tRect.fX = 120.0f;
	tRect.fY = 220.0f;
	tRect.fW = 220.0f;
	tRect.fH = 76.0f;
	xgeShapeRectStrokePx(tRect, 3.0f, XGE_COLOR_RGBA(200, 230, 255, 255));
	return 0;
}

static int MenuUpdate(xge_scene pScene, float fDelta)
{
	scene_demo_t* pDemo;

	(void)fDelta;
	pDemo = (scene_demo_t*)pScene->pUser;
	if ( xgeKeyPressed(XGE_EXAMPLE_KEY_SPACE) || xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) || xgeMouseDown(XGE_MOUSE_LEFT) ) {
		pDemo->bMenuOpen = 0;
		return xgeScenePop();
	}
	return 0;
}

static int MenuDraw(xge_scene pScene)
{
	xge_rect_t tRect;

	(void)pScene;
	xgeShapeRectFillPx((xge_rect_t){ 0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight() }, XGE_COLOR_RGBA(0, 0, 0, 130));
	tRect.fX = 110.0f;
	tRect.fY = 86.0f;
	tRect.fW = 260.0f;
	tRect.fH = 148.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(42, 96, 160, 240));
	xgeShapeRectStrokePx(tRect, 4.0f, XGE_COLOR_RGBA(210, 235, 255, 255));
	xgeShapeCircleFillPx(160.0f, 160.0f, 22.0f, XGE_COLOR_RGBA(255, 220, 96, 255));
	xgeShapeCircleFillPx(240.0f, 160.0f, 22.0f, XGE_COLOR_RGBA(96, 220, 150, 255));
	xgeShapeCircleFillPx(320.0f, 160.0f, 22.0f, XGE_COLOR_RGBA(255, 120, 150, 255));
	return 0;
}

static void DemoSceneInit(scene_demo_t* pDemo)
{
	memset(pDemo, 0, sizeof(*pDemo));
	pDemo->tMainScene.pUser = pDemo;
	pDemo->tMainScene.onUpdate = MainUpdate;
	pDemo->tMainScene.onDraw = MainDraw;
	pDemo->tMenuScene.pUser = pDemo;
	pDemo->tMenuScene.onUpdate = MenuUpdate;
	pDemo->tMenuScene.onDraw = MenuDraw;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	scene_demo_t tDemo;

	(void)argc;
	(void)argv;
	DemoSceneInit(&tDemo);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 480;
	tDesc.iHeight = 320;
	tDesc.sTitle = "XGE Scene";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 60.0f, 4);
	xgeSceneSet(&tDemo.tMainScene);
	xgeRun(NULL, NULL);
	xgeUnit();
	return 0;
}
