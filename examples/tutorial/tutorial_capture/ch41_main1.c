/* ch41 — 填充：纯色 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 不透明纯色填充 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 160, 120, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 半透明填充 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 130, 100, 160, 120, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 150));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 圆形纯色 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 450, 120, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 多色色板 */
	{
		uint32_t colors[6] = {
			XGE_COLOR_RGBA(255, 99, 132, 255),
			XGE_COLOR_RGBA(54, 162, 235, 255),
			XGE_COLOR_RGBA(255, 206, 86, 255),
			XGE_COLOR_RGBA(75, 192, 192, 255),
			XGE_COLOR_RGBA(153, 102, 255, 255),
			XGE_COLOR_RGBA(255, 159, 64, 255)
		};
		int i;
		for (i = 0; i < 6; i++) {
			xgeShapeExCreate(&pShape);
			xgeShapeExAppendRect(pShape, (float)(50 + i * 120), 300, 100, 80, 8, 8, 1);
			xgeShapeExFillColor(pShape, colors[i]);
			xgeShapeExDraw(pShape, 0.5f);
			xgeShapeExDestroy(pShape);
		}
	}

	/* Alpha 渐变条 */
	{
		int i;
		for (i = 0; i < 10; i++) {
			xgeShapeExCreate(&pShape);
			xgeShapeExAppendRect(pShape, (float)(50 + i * 72), 440, 64, 60, 6, 6, 1);
			xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, (uint32_t)(25 + i * 25)));
			xgeShapeExDraw(pShape, 0.5f);
			xgeShapeExDestroy(pShape);
		}
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch41", argc, argv);
}
