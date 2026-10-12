/* ch100 — GPU Buffer */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_buffer_t vbo, ibo;
	float vertices[] = {
		/* x, y */
		100.0f, 50.0f,
		250.0f, 200.0f,
		50.0f, 200.0f
	};
	uint16_t indices[] = { 0, 1, 2 };
	int ret;

	/* 创建顶点缓冲 */
	ret = xgeBufferCreate(&vbo, XGE_BUFFER_VERTEX, XGE_BUFFER_STATIC, vertices, sizeof(vertices));
	if (ret != XGE_OK) return;

	/* 创建索引缓冲 */
	ret = xgeBufferCreate(&ibo, XGE_BUFFER_INDEX, XGE_BUFFER_STATIC, indices, sizeof(indices));
	if (ret != XGE_OK) { xgeBufferFree(&vbo); return; }

	/* 上传到 GPU */
	xgeBufferUpload(&vbo);
	xgeBufferUpload(&ibo);

	/* 使用 ShapeEx 绘制三角形作为可视化 */
	{
		xge_shape_ex pShape = NULL;
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendTriangle(pShape, 100.0f, 50.0f, 250.0f, 200.0f, 50.0f, 200.0f, 1);
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	/* 动态缓冲更新演示 */
	{
		float updated[] = { 150.0f, 80.0f, 300.0f, 250.0f, 80.0f, 250.0f };
		xgeBufferUpdate(&vbo, 0, updated, sizeof(updated));
		xgeBufferUpload(&vbo);
	}

	/* 绘制更新后的三角形 */
	{
		xge_shape_ex pShape = NULL;
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendTriangle(pShape, 400.0f, 80.0f, 600.0f, 250.0f, 350.0f, 250.0f, 1);
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	xgeBufferFree(&vbo);
	xgeBufferFree(&ibo);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch100", argc, argv); }
