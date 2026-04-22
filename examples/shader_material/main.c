#include "../../xge.h"
#include <math.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct shader_material_demo_t {
	xge_texture_t tTexture;
	xge_shader_t tGrayShader;
	xge_material_t tMaterial;
	int bReady;
	float fTime;
} shader_material_demo_t;

static void __shaderMaterialMakePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iPos;
	int bGrid;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iPos = ((iY * iWidth) + iX) * 4;
			bGrid = ((iX % 24) < 3) || ((iY % 24) < 3);
			pPixels[iPos + 0] = (unsigned char)(bGrid ? 255 : 40 + (iX * 180 / iWidth));
			pPixels[iPos + 1] = (unsigned char)(bGrid ? 255 : 120 + (iY * 90 / iHeight));
			pPixels[iPos + 2] = (unsigned char)(bGrid ? 255 : 230);
			pPixels[iPos + 3] = 255;
		}
	}
}

static int __shaderMaterialInit(shader_material_demo_t* pDemo)
{
	static const char* sVS =
		"#version 330 core\n"
		"layout (location = 0) in vec4 aPos;\n"
		"layout (location = 1) in vec2 aUV;\n"
		"layout (location = 2) in vec4 aColor;\n"
		"uniform vec2 uResolution;\n"
		"out vec2 vUV;\n"
		"out vec4 vColor;\n"
		"void main() {\n"
		"	vec2 pos = (aPos.xy / uResolution) * 2.0 - 1.0;\n"
		"	pos.y = -pos.y;\n"
		"	gl_Position = vec4(pos * aPos.w, aPos.z * aPos.w, aPos.w);\n"
		"	vUV = aUV;\n"
		"	vColor = aColor;\n"
		"}\n";
	static const char* sFS =
		"#version 330 core\n"
		"in vec2 vUV;\n"
		"in vec4 vColor;\n"
		"uniform sampler2D uTexture;\n"
		"uniform float uAmount;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	vec4 c = texture(uTexture, vUV) * vColor;\n"
		"	float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));\n"
		"	FragColor = vec4(mix(c.rgb, vec3(g), uAmount), c.a);\n"
		"}\n";
	unsigned char arrPixels[192 * 192 * 4];

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	__shaderMaterialMakePixels(arrPixels, 192, 192);
	if ( xgeTextureCreateRGBA(&pDemo->tTexture, 192, 192, arrPixels) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeShaderCreate(&pDemo->tGrayShader, sVS, sFS) != XGE_OK ) {
		xgeTextureFree(&pDemo->tTexture);
		return XGE_ERROR_GPU_FAILED;
	}
	xgeMaterialInit(&pDemo->tMaterial);
	xgeMaterialSetShader(&pDemo->tMaterial, &pDemo->tGrayShader);
	xgeMaterialSetTexture(&pDemo->tMaterial, &pDemo->tTexture);
	xgeMaterialSetColor(&pDemo->tMaterial, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeMaterialSetBlend(&pDemo->tMaterial, XGE_BLEND_ALPHA);
	pDemo->bReady = 1;
	return XGE_OK;
}

static int ShaderMaterialFrame(void* pUser)
{
	shader_material_demo_t* pDemo;
	xge_draw_t tDraw;
	float fAmount;

	pDemo = (shader_material_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __shaderMaterialInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();
	fAmount = 0.5f + 0.5f * sinf(pDemo->fTime * 1.5f);

	xgeClear(XGE_COLOR_RGBA(16, 20, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 42.0f, 42.0f, 556.0f, 364.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 42.0f, 42.0f, 556.0f, 364.0f }, 2.0f, XGE_COLOR_RGBA(90, 118, 136, 255));

	xgeShaderUniform1f(&pDemo->tGrayShader, "uAmount", 0.0f);
	xgeDraw(&pDemo->tTexture, 92.0f, 118.0f);

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tTexture;
	tDraw.tDst.fX = 380.0f;
	tDraw.tDst.fY = 214.0f;
	tDraw.tDst.fW = 220.0f;
	tDraw.tDst.fH = 220.0f;
	tDraw.tOrigin.fX = 110.0f;
	tDraw.tOrigin.fY = 110.0f;
	tDraw.fRotation = sinf(pDemo->fTime) * 0.35f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 240);
	xgeShaderUniform1f(&pDemo->tGrayShader, "uAmount", fAmount);
	xgeMaterialDraw(&pDemo->tMaterial, &tDraw);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	shader_material_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Shader Material";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(ShaderMaterialFrame, &tDemo);
	xgeMaterialFree(&tDemo.tMaterial);
	xgeShaderFree(&tDemo.tGrayShader);
	xgeTextureFree(&tDemo.tTexture);
	xgeUnit();
	return 0;
}
