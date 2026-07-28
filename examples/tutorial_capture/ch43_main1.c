/* ch43 — 填充：径向渐变 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_color_stop_t stops[3];

	/* 标准径向渐变：中心扩散 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 200, 255);
	stops[1].fOffset = 0.6f; stops[1].iColor = XGE_COLOR_RGBA(255, 180, 84, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 180, 160, 110, 110, 1);
	xgeShapeExFillRadialGradient(pShape, 180, 160, 110, 180, 160,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 焦点偏移的径向渐变（3D 球体效果）*/
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(200, 230, 255, 255);
	stops[1].fOffset = 0.5f; stops[1].iColor = XGE_COLOR_RGBA(54, 162, 235, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(20, 60, 120, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 500, 160, 100, 100, 1);
	xgeShapeExFillRadialGradient(pShape, 500, 160, 100, 470, 130,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 矩形上的径向渐变 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(13, 17, 22, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 320, 300, 200, 16, 16, 1);
	xgeShapeExFillRadialGradient(pShape, 200, 420, 150, 200, 420,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* RadialGradientEx：带 focal radius */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	stops[1].fOffset = 0.4f; stops[1].iColor = XGE_COLOR_RGBA(255, 206, 86, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(255, 99, 132, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 550, 420, 100, 100, 1);
	xgeShapeExFillRadialGradientEx(pShape, 550, 420, 100, 530, 400, 30,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch43", argc, argv);
}
