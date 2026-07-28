/* ch39 — 路径构建：几何快捷方式 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_vec2_t pts[5];
	int i;

	/* AppendRect 圆角矩形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 150, 100, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 180));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendCircle */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 350, 100, 60, 60, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 180));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendEllipse */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendEllipse(pShape, 550, 100, 90, 50, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 180));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendCapsule */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCapsule(pShape, 50, 220, 200, 50, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendArc */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendArc(pShape, 400, 270, 60, 60, 0, 4.5f);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendPie */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendPie(pShape, 600, 270, 60, 60, 0.5f, 4.0f);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendTriangle */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendTriangle(pShape, 100, 450, 200, 350, 200, 450, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendPolygon（五角星）*/
	for (i = 0; i < 5; i++) {
		float ang = -1.5708f + i * 2.0f * 3.14159f / 5.0f;
		pts[i].fX = 400.0f + 70.0f * cosf(ang);
		pts[i].fY = 430.0f + 70.0f * sinf(ang);
	}
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendPolygon(pShape, pts, 5);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* AppendPolyline（开放折线）*/
	{
		xge_vec2_t line[4] = {{550,400},{600,350},{650,420},{720,380}};
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendPolyline(pShape, line, 4);
		xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
		xgeShapeExStrokeWidth(pShape, 2.5f);
		xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
		xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_ROUND);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch39", argc, argv);
}
