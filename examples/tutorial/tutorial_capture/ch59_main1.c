/* ch59 — 裁剪：形状裁剪 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex pClip = NULL;

	/* 圆形裁剪矩形 */
	xgeShapeExCreate(&pClip);
	xgeShapeExAppendCircle(pClip, 180, 140, 90, 90, 1);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 60, 30, 240, 220, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExClipShapeAdd(pShape, pClip);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
	xgeShapeExDestroy(pClip);

	/* 星形裁剪 */
	{
		xge_vec2_t star[10];
		int i;
		for (i = 0; i < 10; i++) {
			float ang = -1.5708f + i * 3.14159f / 5.0f;
			float r = (i % 2 == 0) ? 90.0f : 40.0f;
			star[i].fX = 530.0f + r * cosf(ang);
			star[i].fY = 140.0f + r * sinf(ang);
		}
		xgeShapeExCreate(&pClip);
		xgeShapeExAppendPolygon(pClip, star, 10);

		xgeShapeExCreate(&pShape);
		xgeShapeExAppendRect(pShape, 420, 30, 220, 220, 0, 0, 1);
		{
			xge_shape_ex_color_stop_t stops[2];
			stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 180, 84, 255);
			stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
			xgeShapeExFillLinearGradient(pShape, 420, 140, 640, 140,
				XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
		}
		xgeShapeExClipShapeAdd(pShape, pClip);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
		xgeShapeExDestroy(pClip);
	}

	/* 多形状裁剪（Intersect 模式）*/
	xgeShapeExCreate(&pClip);
	xgeShapeExAppendCircle(pClip, 160, 420, 80, 80, 1);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 60, 320, 250, 200, 0, 0, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExClipShapeAdd(pShape, pClip);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
	xgeShapeExDestroy(pClip);

	/* Subtract 模式裁剪 */
	xgeShapeExCreate(&pClip);
	xgeShapeExAppendCircle(pClip, 530, 420, 60, 60, 1);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 320, 220, 200, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 220));
	xgeShapeExClipShapeAddEx(pShape, pClip, XGE_SHAPE_EX_CLIP_SUBTRACT);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
	xgeShapeExDestroy(pClip);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch59", argc, argv);
}
