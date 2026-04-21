#include "../../xge.h"

static int MainFrame(void* pUser)
{
	xge_rect_t tRect;
	xge_vec2_t arrPoly[5];
	xge_camera_t tCamera;

	(void)pUser;
	if ( xgeKeyDown(256) ) {
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 24, 30, 255));

	xgeShapeLinePx(40.0f, 56.0f, 260.0f, 56.0f, 3.0f, XGE_COLOR_RGBA(120, 200, 255, 255));
	xgeShapeLinePx(40.0f, 80.0f, 260.0f, 140.0f, 8.0f, XGE_COLOR_RGBA(255, 220, 96, 255));
	xgeShapePointPx(44.0f, 56.0f, 8.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapePointPx(260.0f, 140.0f, 10.0f, XGE_COLOR_RGBA(255, 128, 128, 255));

	tRect.fX = 320.0f;
	tRect.fY = 48.0f;
	tRect.fW = 160.0f;
	tRect.fH = 92.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(42, 96, 160, 255));
	xgeShapeRectStrokePx(tRect, 4.0f, XGE_COLOR_RGBA(180, 225, 255, 255));

	xgeShapeCircleFillPx(130.0f, 260.0f, 54.0f, XGE_COLOR_RGBA(62, 172, 110, 255));
	xgeShapeCircleStrokePx(130.0f, 260.0f, 64.0f, 5.0f, XGE_COLOR_RGBA(210, 255, 225, 255));
	xgeShapeArcPx(130.0f, 260.0f, 82.0f, 0.2f, 4.8f, 6.0f, XGE_COLOR_RGBA(255, 180, 96, 255));

	arrPoly[0].fX = 330.0f; arrPoly[0].fY = 250.0f;
	arrPoly[1].fX = 390.0f; arrPoly[1].fY = 190.0f;
	arrPoly[2].fX = 470.0f; arrPoly[2].fY = 225.0f;
	arrPoly[3].fX = 455.0f; arrPoly[3].fY = 315.0f;
	arrPoly[4].fX = 350.0f; arrPoly[4].fY = 325.0f;
	xgeShapePolygonFillPx(arrPoly, 5, XGE_COLOR_RGBA(128, 96, 220, 255));
	xgeShapeTriangleFillPx(arrPoly[1], arrPoly[2], arrPoly[3], XGE_COLOR_RGBA(255, 120, 150, 190));
	xgeShapeRectStrokePx((xge_rect_t){ 300.0f, 170.0f, 200.0f, 180.0f }, 2.0f, XGE_COLOR_RGBA(230, 230, 240, 180));

	tCamera = xgeCameraDefault((float)xgeGetWidth(), (float)xgeGetHeight());
	tCamera.iCoordinateMode = XGE_COORD_CENTER;
	tCamera.tScale.fX = 1.2f;
	tCamera.tScale.fY = 1.2f;
	xgeCameraSet(&tCamera);
	xgeShapeLine(-88.0f, -120.0f, -36.0f, -88.0f, 3.0f, XGE_COLOR_RGBA(255, 255, 255, 210));
	xgeShapeRectStroke((xge_rect_t){ -96.0f, -136.0f, 78.0f, 58.0f }, 2.0f, XGE_COLOR_RGBA(180, 225, 255, 220));
	xgeShapeCircleFill(-56.0f, -108.0f, 12.0f, XGE_COLOR_RGBA(255, 220, 96, 220));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	tDesc.iWidth = 540;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE Shape";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	tDesc.pNativeWindow = 0;
	tDesc.pUser = 0;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MainFrame, 0);
	xgeUnit();
	return 0;
}
