/* ch53 — 透明度与可见性 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 不同透明度 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 120, 120, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExOpacity(pShape, 1.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 280, 120, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExOpacity(pShape, 0.6f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 440, 120, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExOpacity(pShape, 0.3f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 600, 120, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExOpacity(pShape, 0.1f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 重叠半透明图形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 180, 320, 80, 80, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExOpacity(pShape, 0.7f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 250, 320, 80, 80, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(54, 162, 235, 255));
	xgeShapeExOpacity(pShape, 0.7f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 215, 380, 80, 80, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 206, 86, 255));
	xgeShapeExOpacity(pShape, 0.7f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Visible = 0：不可见（只画第二个）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 450, 280, 120, 120, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExVisible(pShape, 0);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 600, 280, 120, 120, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExVisible(pShape, 1);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch53", argc, argv);
}
