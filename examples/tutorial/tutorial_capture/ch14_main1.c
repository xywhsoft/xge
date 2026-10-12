/* ch14 — 正交投影相机：相机跟随与缩放 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	xge_camera_t cam;

	/* 设置相机：平移 + 缩放 */
	cam = xgeCameraDefault(800.0f, 600.0f);
	cam.tPosition.fX = 500.0f;
	cam.tPosition.fY = 300.0f;
	cam.tScale.fX = 1.5f;
	cam.tScale.fY = 1.5f;
	xgeCameraSet(&cam);

	/* 世界空间中的物体 */
	/* 地面 */
	rc.fX = 0; rc.fY = 350; rc.fW = 2000; rc.fH = 50;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(42, 74, 63, 255));

	/* 玩家（琥珀色圆形） */
	xgeShapeCircleFill(500, 300, 20, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 周围建筑物 */
	rc.fX = 300; rc.fY = 250; rc.fW = 60; rc.fH = 100;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, 200));
	rc.fX = 600; rc.fY = 220; rc.fW = 80; rc.fH = 130;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(109, 179, 242, 200));
	rc.fX = 750; rc.fY = 270; rc.fW = 50; rc.fH = 80;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(255, 107, 94, 200));

	/* 恢复默认相机 */
	cam = xgeCameraDefault(800.0f, 600.0f);
	xgeCameraSet(&cam);

	/* 视口边框 */
	rc.fX = 5; rc.fY = 5; rc.fW = 790; rc.fH = 590;
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 80));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch14", argc, argv);
}
