#include "../../xge.h"
#include <math.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct shader_distortion_demo_t {
	xge_texture_t tTexture;
	xge_shader_t tShader;
	xge_material_t tMaterial;
	int bReady;
	float fTime;
} shader_distortion_demo_t;

static void __shaderDistortionMakePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iPos;
	int bLine;
	int iRing;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iPos = ((iY * iWidth) + iX) * 4;
			iRing = ((iX - iWidth / 2) * (iX - iWidth / 2) + (iY - iHeight / 2) * (iY - iHeight / 2)) / 360;
			bLine = ((iX % 28) < 3) || ((iY % 28) < 3) || ((iRing & 1) == 0);
			pPixels[iPos + 0] = (unsigned char)(bLine ? 245 : 38 + iX * 170 / iWidth);
			pPixels[iPos + 1] = (unsigned char)(bLine ? 245 : 80 + iY * 150 / iHeight);
			pPixels[iPos + 2] = (unsigned char)(bLine ? 245 : 230);
			pPixels[iPos + 3] = 255;
		}
	}
}

static int __shaderDistortionInit(shader_distortion_demo_t* pDemo)
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
		"uniform float uTime;\n"
		"uniform float uStrength;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	vec2 center = vUV - vec2(0.5, 0.5);\n"
		"	float dist = length(center);\n"
		"	float wave = sin(dist * 42.0 - uTime * 5.0) * uStrength;\n"
		"	vec2 uv = vUV + normalize(center + vec2(0.0001, 0.0)) * wave;\n"
		"	uv = clamp(uv, vec2(0.0), vec2(1.0));\n"
		"	FragColor = texture(uTexture, uv) * vColor;\n"
		"}\n";
	unsigned char arrPixels[256 * 256 * 4];

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	__shaderDistortionMakePixels(arrPixels, 256, 256);
	if ( xgeTextureCreateRGBA(&pDemo->tTexture, 256, 256, arrPixels) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeShaderCreate(&pDemo->tShader, sVS, sFS) != XGE_OK ) {
		xgeTextureFree(&pDemo->tTexture);
		return XGE_ERROR_GPU_FAILED;
	}
	xgeMaterialInit(&pDemo->tMaterial);
	xgeMaterialSetShader(&pDemo->tMaterial, &pDemo->tShader);
	xgeMaterialSetTexture(&pDemo->tMaterial, &pDemo->tTexture);
	xgeMaterialSetColor(&pDemo->tMaterial, XGE_COLOR_RGBA(255, 255, 255, 245));
	xgeMaterialSetBlend(&pDemo->tMaterial, XGE_BLEND_ALPHA);
	pDemo->bReady = 1;
	return XGE_OK;
}

static int ShaderDistortionFrame(void* pUser)
{
	shader_distortion_demo_t* pDemo;
	xge_draw_t tDraw;
	float fStrength;

	pDemo = (shader_distortion_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __shaderDistortionInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();
	fStrength = 0.010f + 0.012f * (0.5f + 0.5f * sinf(pDemo->fTime * 1.6f));

	xgeClear(XGE_COLOR_RGBA(14, 18, 26, 255));
	xgeShapeRectFillPx((xge_rect_t){ 42.0f, 42.0f, 556.0f, 364.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 42.0f, 42.0f, 556.0f, 364.0f }, 2.0f, XGE_COLOR_RGBA(90, 118, 136, 255));

	xgeDraw(&pDemo->tTexture, 82.0f, 96.0f);

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tTexture;
	tDraw.tDst.fX = 440.0f;
	tDraw.tDst.fY = 224.0f;
	tDraw.tDst.fW = 280.0f;
	tDraw.tDst.fH = 280.0f;
	tDraw.tOrigin.fX = 140.0f;
	tDraw.tOrigin.fY = 140.0f;
	tDraw.fRotation = sinf(pDemo->fTime * 0.7f) * 0.18f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	xgeShaderUniform1f(&pDemo->tShader, "uTime", pDemo->fTime);
	xgeShaderUniform1f(&pDemo->tShader, "uStrength", fStrength);
	xgeMaterialDraw(&pDemo->tMaterial, &tDraw);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	shader_distortion_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Shader Distortion";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(ShaderDistortionFrame, &tDemo);
	xgeMaterialFree(&tDemo.tMaterial);
	xgeShaderFree(&tDemo.tShader);
	xgeTextureFree(&tDemo.tTexture);
	xgeUnit();
	return 0;
}
