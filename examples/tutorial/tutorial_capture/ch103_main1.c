/* ch103 — Shader 变体集 */
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
	"#ifdef USE_CIRCLE\n"
	"void main() {\n"
	"  float d = length(vUV - 0.5) * 2.0;\n"
	"  if (d > 1.0) discard;\n"
	"  gl_FragColor = uColor;\n"
	"}\n"
	"#else\n"
	"void main() {\n"
	"  gl_FragColor = uColor;\n"
	"}\n"
	"#endif\n";

static void draw_scene(void)
{
	xge_shader_variant_set_t variantSet;
	xge_shader pShader = NULL;
	xge_shader_define_t defines[1];
	int ret;

	/* 初始化变体集 */
	ret = xgeShaderVariantSetInit(&variantSet, VS_SRC, FS_SRC);
	if (ret != XGE_OK) return;

	/* 获取无宏定义的变体 */
	ret = xgeShaderVariantGet(&variantSet, 0, NULL, 0, &pShader);
	if (ret == XGE_OK && pShader) {
		xgeShapeRectFill((xge_rect_t){60.0f, 60.0f, 150.0f, 150.0f}, XGE_COLOR_RGBA(79, 216, 194, 255));
	}

	/* 获取带 USE_CIRCLE 宏的变体 */
	strcpy(defines[0].sName, "USE_CIRCLE");
	defines[0].iValue = 1;
	pShader = NULL;
	ret = xgeShaderVariantGet(&variantSet, 1, defines, 1, &pShader);
	if (ret == XGE_OK && pShader) {
		xgeShapeCircleFill(450.0f, 135.0f, 75.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	}

	xgeShaderVariantSetFree(&variantSet);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch103", argc, argv); }
