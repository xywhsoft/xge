/* ch60 — 裁剪：模板裁剪 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pClip = NULL;
	xge_shape_ex pShape = NULL;
	int bApplied = 0;

	/* 模板裁剪：圆形模板区域内绘制多个图形 */
	xgeShapeExCreate(&pClip);
	xgeShapeExAppendCircle(pClip, 200, 180, 120, 120, 1);

	xgeShapeExStencilClipBegin(pClip, 0.5f, NULL, &bApplied);

	/* 在模板区域内绘制条纹 */
	{
		int i;
		for (i = 0; i < 12; i++) {
			xgeShapeExCreate(&pShape);
			xgeShapeExAppendRect(pShape, (float)(60 + i * 25), 40, 12, 280, 0, 0, 1);
			xgeShapeExFillColor(pShape, (i % 2 == 0) ?
				XGE_COLOR_RGBA(79, 216, 194, 255) :
				XGE_COLOR_RGBA(54, 162, 235, 255));
			xgeShapeExDraw(pShape, 0.5f);
			xgeShapeExDestroy(pShape);
		}
	}
	xgeShapeExStencilClipEnd(bApplied, XGE_OK);
	xgeShapeExDestroy(pClip);

	/* 星形模板裁剪 */
	{
		xge_vec2_t star[10];
		int i;
		for (i = 0; i < 10; i++) {
			float ang = -1.5708f + i * 3.14159f / 5.0f;
			float r = (i % 2 == 0) ? 110.0f : 50.0f;
			star[i].fX = 550.0f + r * cosf(ang);
			star[i].fY = 180.0f + r * sinf(ang);
		}
		xgeShapeExCreate(&pClip);
		xgeShapeExAppendPolygon(pClip, star, 10);

		bApplied = 0;
		xgeShapeExStencilClipBegin(pClip, 0.5f, NULL, &bApplied);

		/* 径向渐变填充 */
		{
			xge_shape_ex_color_stop_t stops[3];
			stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 200, 255);
			stops[1].fOffset = 0.5f; stops[1].iColor = XGE_COLOR_RGBA(255, 180, 84, 255);
			stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
			xgeShapeExCreate(&pShape);
			xgeShapeExAppendRect(pShape, 420, 50, 260, 260, 0, 0, 1);
			xgeShapeExFillRadialGradient(pShape, 550, 180, 130, 550, 180,
				XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
			xgeShapeExDraw(pShape, 0.5f);
			xgeShapeExDestroy(pShape);
		}
		xgeShapeExStencilClipEnd(bApplied, XGE_OK);
		xgeShapeExDestroy(pClip);
	}

	/* 圆角矩形模板 + 网格 */
	xgeShapeExCreate(&pClip);
	xgeShapeExAppendRect(pClip, 100, 370, 600, 180, 20, 20, 1);
	bApplied = 0;
	xgeShapeExStencilClipBegin(pClip, 0.5f, NULL, &bApplied);
	{
		int i, j;
		for (i = 0; i < 15; i++) {
			for (j = 0; j < 5; j++) {
				xgeShapeExCreate(&pShape);
				xgeShapeExAppendCircle(pShape, (float)(120 + i * 42), (float)(390 + j * 38), 8, 8, 1);
				xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, (uint32_t)(100 + (i + j) * 10)));
				xgeShapeExDraw(pShape, 0.5f);
				xgeShapeExDestroy(pShape);
			}
		}
	}
	xgeShapeExStencilClipEnd(bApplied, XGE_OK);
	xgeShapeExDestroy(pClip);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch60", argc, argv);
}
