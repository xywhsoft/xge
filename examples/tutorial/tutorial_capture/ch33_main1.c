/* ch33 — ShapeEx 对象生命周期 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xgeShapeExCreate(&pShape);
	if ( !pShape ) return;

	/* 构建一个简单路径 */
	xgeShapeExAppendCircle(pShape, 300, 250, 100, 100, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 第二个对象 */
	pShape = NULL;
	xgeShapeExCreate(&pShape);
	if ( !pShape ) return;
	xgeShapeExAppendRect(pShape, 450, 300, 200, 150, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch33", argc, argv);
}
