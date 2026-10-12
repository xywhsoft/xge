/* ch106 — ShapeBatch */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_batch_t batch;
	int i, ret;

	/* 初始化 ShapeBatch（预分配三角形容量） */
	ret = xgeShapeBatchInit(&batch, XGE_COLOR_RGBA(79, 216, 194, 255), 128, 0);
	if (ret != XGE_OK) return;

	/* 批量添加矩形 */
	for (i = 0; i < 8; i++) {
		xge_rect_t rect;
		rect.fX = 50.0f + (i % 4) * 180.0f;
		rect.fY = 50.0f + (i / 4) * 200.0f;
		rect.fW = 120.0f;
		rect.fH = 100.0f;
		xgeShapeBatchRectFill(&batch, rect);
	}

	/* 一次性提交 */
	xgeShapeBatchFlush(&batch);

	/* 第二批：不同颜色 */
	xgeShapeBatchInit(&batch, XGE_COLOR_RGBA(255, 180, 84, 200), 64, 0);
	for (i = 0; i < 4; i++) {
		xge_rect_t rect;
		rect.fX = 100.0f + i * 160.0f;
		rect.fY = 320.0f;
		rect.fW = 80.0f;
		rect.fH = 80.0f;
		xgeShapeBatchRectFill(&batch, rect);
	}
	xgeShapeBatchFlush(&batch);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch106", argc, argv); }
