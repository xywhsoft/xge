/* ch102 — 自定义 Shader */
#include "tut_capture.h"

static const char* VS_SRC =
	"uniform vec2 uResolution;\n"
	"attribute vec2 aPosition;\n"
	"attribute vec2 aTexCoord;\n"
	"varying vec2 vUV;\n"
	"void main() {\n"
	"  vec2 pos = aPosition / uResolution * 2.0 - 1.0;\n"
	"  gl_Position = vec4(pos.x, -pos.y, 0.0, 1.0);\n"
	"  vUV = aTexCoord;\n"
	"}\n";

static const char* FS_SRC =
	"precision mediump float;\n"
	"varying vec2 vUV;\n"
	"uniform vec4 uColor;\n"
	"void main() {\n"
	"  float d = length(vUV - 0.5) * 2.0;\n"
	"  gl_FragColor = uColor * (1.0 - d);\n"
	"}\n";

static void draw_scene(void)
{
	xge_shader_t shader;
	int ret;

	/* 创建自定义 shader */
	ret = xgeShaderCreate(&shader, VS_SRC, FS_SRC);
	if (ret != XGE_OK) return;

	/* 设置 uniform */
	xgeShaderUniform4f(&shader, "uColor", 0.3f, 0.85f, 0.76f, 1.0f);

	/* 使用 ShapeEx 绘制可视化效果 */
	{
		xge_shape_ex pShape = NULL;
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendCircle(pShape, 200.0f, 150.0f, 100.0f, 100.0f, 1);
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	/* 第二个圆形使用不同颜色 */
	xgeShaderUniform4f(&shader, "uColor", 1.0f, 0.7f, 0.3f, 1.0f);
	{
		xge_shape_ex pShape = NULL;
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendCircle(pShape, 500.0f, 150.0f, 80.0f, 80.0f, 1);
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	xgeShaderFree(&shader);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch102", argc, argv); }
