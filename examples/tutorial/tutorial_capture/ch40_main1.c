/* ch40 — SVG path data 互转 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	char sBuffer[512];
	int iRequired = 0;

	/* 从 SVG path data 导入：星形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendSvgPath(pShape,
		"M 200 50 L 230 130 L 320 130 L 250 180 L 270 260 "
		"L 200 210 L 130 260 L 150 180 L 80 130 L 170 130 Z");
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 220));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 从 SVG path data 导入：带曲线的心形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendSvgPath(pShape,
		"M 550 150 C 550 120 510 100 510 140 C 510 170 550 200 550 220 "
		"C 550 200 590 170 590 140 C 590 100 550 120 550 150 Z");
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 220));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 程序化构建路径 → 导出 SVG path data → 再导入绘制 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 100, 350);
	xgeShapeExCubicTo(pShape, 150, 300, 250, 300, 300, 350);
	xgeShapeExCubicTo(pShape, 350, 400, 250, 450, 200, 400);
	xgeShapeExClose(pShape);

	/* 导出为 SVG path data */
	xgeShapeExGetSvgPathData(pShape, sBuffer, sizeof(sBuffer), &iRequired);
	xgeShapeExDestroy(pShape);

	/* 用导出的 data 重新创建并偏移绘制 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendSvgPath(pShape, sBuffer);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 180));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 350, 100);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* SVG arc 命令 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendSvgPath(pShape,
		"M 450 400 A 80 80 0 1 1 610 400 A 80 80 0 1 1 450 400 Z");
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 120));
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 2.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch40", argc, argv);
}
