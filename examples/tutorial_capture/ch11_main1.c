/* ch11 — 清屏与颜色模型：颜色色板展示 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	/* 教程中列出的常用颜色 */
	uint32_t colors[] = {
		XGE_COLOR_RGBA(8, 17, 20, 255),     /* 深石油蓝 */
		XGE_COLOR_RGBA(255, 180, 84, 255),   /* 琥珀色 */
		XGE_COLOR_RGBA(79, 216, 194, 255),   /* 青绿色 */
		XGE_COLOR_RGBA(255, 107, 94, 255),   /* 珊瑚红 */
		XGE_COLOR_RGBA(109, 179, 242, 255),  /* 天蓝色 */
		XGE_COLOR_RGBA(160, 120, 255, 255),  /* 紫色 */
		XGE_COLOR_RGBA(120, 220, 120, 255),  /* 草绿色 */
		XGE_COLOR_RGBA(255, 220, 100, 255),  /* 明黄色 */
	};
	int i;
	float sw = 160.0f, sh = 100.0f;
	float gap = 20.0f;
	float startX = 60.0f, startY = 80.0f;

	for ( i = 0; i < 8; i++ ) {
		int col = i % 4;
		int row = i / 4;
		rc.fX = startX + col * (sw + gap);
		rc.fY = startY + row * (sh + gap + 40);
		rc.fW = sw;
		rc.fH = sh;
		xgeShapeRectFill(rc, colors[i]);
		xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(200, 210, 220, 100));
	}

	/* Alpha 渐变条 */
	for ( i = 0; i < 16; i++ ) {
		uint32_t a = (uint32_t)(255 - i * 16);
		rc.fX = 60.0f + i * 43.0f;
		rc.fY = 420.0f;
		rc.fW = 40;
		rc.fH = 60;
		xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, a));
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch11", argc, argv);
}
