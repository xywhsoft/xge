/* ch42 — 填充：线性渐变 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_color_stop_t stops[3];

	/* 水平双色渐变 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(54, 162, 235, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 300, 100, 12, 12, 1);
	xgeShapeExFillLinearGradient(pShape, 50, 100, 350, 100,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 垂直三色渐变 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
	stops[1].fOffset = 0.5f; stops[1].iColor = XGE_COLOR_RGBA(255, 180, 84, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 50, 300, 100, 12, 12, 1);
	xgeShapeExFillLinearGradient(pShape, 570, 50, 570, 150,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 对角线渐变圆形 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(160, 200, 255, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 200, 320, 90, 90, 1);
	xgeShapeExFillLinearGradient(pShape, 110, 230, 290, 410,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Object Bounding Box 模式 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(153, 102, 255, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 206, 86, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 250, 280, 140, 16, 16, 1);
	xgeShapeExFillLinearGradient(pShape, 0, 0, 1, 1,
		XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 多色停靠点渐变条 */
	{
		xge_shape_ex_color_stop_t rainbow[5];
		rainbow[0].fOffset = 0.0f;  rainbow[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
		rainbow[1].fOffset = 0.25f; rainbow[1].iColor = XGE_COLOR_RGBA(255, 255, 0, 255);
		rainbow[2].fOffset = 0.5f;  rainbow[2].iColor = XGE_COLOR_RGBA(0, 255, 0, 255);
		rainbow[3].fOffset = 0.75f; rainbow[3].iColor = XGE_COLOR_RGBA(0, 255, 255, 255);
		rainbow[4].fOffset = 1.0f;  rainbow[4].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendRect(pShape, 50, 470, 700, 50, 8, 8, 1);
		xgeShapeExFillLinearGradient(pShape, 50, 495, 750, 495,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, rainbow, 5);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch42", argc, argv);
}
