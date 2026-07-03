#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

static const char* g_shaderVS =
	"#version 330 core\n"
	"layout (location = 0) in vec4 aPos;\n"
	"layout (location = 1) in vec2 aUV;\n"
	"layout (location = 2) in vec4 aColor;\n"
	"uniform vec2 uResolution;\n"
	"uniform vec2 uOffset;\n"
	"out vec2 vUV;\n"
	"out vec4 vColor;\n"
	"void main() {\n"
	"	vec2 pos = ((aPos.xy + uOffset) / uResolution) * 2.0 - 1.0;\n"
	"	pos.y = -pos.y;\n"
	"	gl_Position = vec4(pos * aPos.w, aPos.z * aPos.w, aPos.w);\n"
	"	vUV = aUV;\n"
	"	vColor = aColor;\n"
	"}\n";

static const char* g_shaderFS =
	"#version 330 core\n"
	"in vec2 vUV;\n"
	"in vec4 vColor;\n"
	"uniform sampler2D uTexture;\n"
	"uniform vec4 uColor;\n"
	"uniform float uAmount;\n"
	"uniform vec3 uTint;\n"
	"uniform vec4 uRect;\n"
	"out vec4 FragColor;\n"
	"void main() {\n"
	"	vec4 c = texture(uTexture, vUV) * vColor * uColor;\n"
	"	float mask = smoothstep(uRect.x, uRect.y, vUV.x) * smoothstep(uRect.z, uRect.w, vUV.y);\n"
	"#if XGE_TINT_MODE == 1\n"
	"	c.rgb = mix(c.rgb, c.rgb * uTint, uAmount);\n"
	"#elif XGE_TINT_MODE == 2\n"
	"	float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));\n"
	"	c.rgb = mix(vec3(g), uTint, uAmount);\n"
	"#else\n"
	"	c.rgb = mix(c.rgb, uTint, uAmount * 0.35);\n"
	"#endif\n"
	"	c.rgb += mask * vec3(0.04, 0.02, 0.00);\n"
	"	FragColor = c;\n"
	"}\n";

typedef struct shader_variant_lab_t {
	xge_texture_t tTexture;
	xge_shader_t tBasicShader;
	xge_shader_variant_set_t tVariantSet;
	xge_shader pVariantWarm;
	xge_shader pVariantCool;
	xge_shader pVariantWarmCached;
	xge_material_t tBasicMaterial;
	xge_material_t tWarmMaterial;
	xge_material_t tCoolMaterial;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bTextureOK;
	int bShaderCreateOK;
	int bAddRefOK;
	int bUniform1OK;
	int bUniform2OK;
	int bUniform3OK;
	int bUniform4OK;
	int bVariantSetOK;
	int bVariantWarmOK;
	int bVariantCoolOK;
	int bVariantCacheOK;
	int bDrawn;
} shader_variant_lab_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static double ArgDouble(const char* sText, double fDefault)
{
	double fValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return fDefault;
	}
	fValue = atof(sText);
	return (fValue > 0.0) ? fValue : fDefault;
}

static void MakePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			int iPos;
			int bGrid;

			iPos = ((iY * iWidth) + iX) * 4;
			bGrid = ((iX % 16) < 2) || ((iY % 16) < 2);
			pPixels[iPos + 0] = (unsigned char)(bGrid ? 240 : 40 + (iX * 160 / iWidth));
			pPixels[iPos + 1] = (unsigned char)(bGrid ? 240 : 70 + (iY * 140 / iHeight));
			pPixels[iPos + 2] = (unsigned char)(bGrid ? 240 : 220);
			pPixels[iPos + 3] = 255;
		}
	}
}

static void InitMaterial(xge_material pMaterial, xge_shader pShader, xge_texture pTexture)
{
	xgeMaterialInit(pMaterial);
	xgeMaterialSetShader(pMaterial, pShader);
	xgeMaterialSetTexture(pMaterial, pTexture);
	xgeMaterialSetColor(pMaterial, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeMaterialSetBlend(pMaterial, XGE_BLEND_ALPHA);
}

static int SetShaderUniforms(xge_shader pShader, float fAmount, float fOffsetX, float fOffsetY, float fR, float fG, float fB)
{
	int bOK;

	bOK = 1;
	if ( xgeShaderUniform1f(pShader, "uAmount", fAmount) != XGE_OK ) {
		bOK = 0;
	}
	if ( xgeShaderUniform2f(pShader, "uOffset", fOffsetX, fOffsetY) != XGE_OK ) {
		bOK = 0;
	}
	if ( xgeShaderUniform3f(pShader, "uTint", fR, fG, fB) != XGE_OK ) {
		bOK = 0;
	}
	if ( xgeShaderUniform4f(pShader, "uRect", 0.15f, 0.92f, 0.10f, 0.88f) != XGE_OK ) {
		bOK = 0;
	}
	return bOK;
}

static int InitLab(shader_variant_lab_t* pLab)
{
	unsigned char arrPixels[128 * 128 * 4];
	xge_shader_define_t tWarmDefine;
	xge_shader_define_t tCoolDefine;
	int iRet;

	if ( pLab->bReady ) {
		return XGE_OK;
	}
	MakePixels(arrPixels, 128, 128);
	pLab->bTextureOK = (xgeTextureCreateRGBA(&pLab->tTexture, 128, 128, arrPixels) == XGE_OK);
	if ( !pLab->bTextureOK ) {
		fprintf(stderr, "shader-variant-lab stage failed: texture\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bShaderCreateOK = (xgeShaderCreate(&pLab->tBasicShader, g_shaderVS, g_shaderFS) == XGE_OK);
	if ( !pLab->bShaderCreateOK ) {
		fprintf(stderr, "shader-variant-lab stage failed: shader create\n");
		return XGE_ERROR_GPU_FAILED;
	}
	pLab->bAddRefOK = (xgeShaderAddRef(&pLab->tBasicShader) > 1);
	pLab->bUniform1OK = (xgeShaderUniform1f(&pLab->tBasicShader, "uAmount", 0.22f) == XGE_OK);
	pLab->bUniform2OK = (xgeShaderUniform2f(&pLab->tBasicShader, "uOffset", 0.0f, 0.0f) == XGE_OK);
	pLab->bUniform3OK = (xgeShaderUniform3f(&pLab->tBasicShader, "uTint", 0.85f, 0.90f, 1.00f) == XGE_OK);
	pLab->bUniform4OK = (xgeShaderUniform4f(&pLab->tBasicShader, "uRect", 0.10f, 0.90f, 0.10f, 0.90f) == XGE_OK);
	pLab->bVariantSetOK = (xgeShaderVariantSetInit(&pLab->tVariantSet, g_shaderVS, g_shaderFS) == XGE_OK);
	if ( !pLab->bVariantSetOK ) {
		fprintf(stderr, "shader-variant-lab stage failed: variant set init\n");
		return XGE_ERROR_GPU_FAILED;
	}
	memset(&tWarmDefine, 0, sizeof(tWarmDefine));
	snprintf(tWarmDefine.sName, sizeof(tWarmDefine.sName), "%s", "XGE_TINT_MODE");
	tWarmDefine.iValue = 1;
	memset(&tCoolDefine, 0, sizeof(tCoolDefine));
	snprintf(tCoolDefine.sName, sizeof(tCoolDefine.sName), "%s", "XGE_TINT_MODE");
	tCoolDefine.iValue = 2;
	iRet = xgeShaderVariantGet(&pLab->tVariantSet, 101u, &tWarmDefine, 1, &pLab->pVariantWarm);
	pLab->bVariantWarmOK = (iRet == XGE_OK) && (pLab->pVariantWarm != NULL);
	iRet = xgeShaderVariantGet(&pLab->tVariantSet, 202u, &tCoolDefine, 1, &pLab->pVariantCool);
	pLab->bVariantCoolOK = (iRet == XGE_OK) && (pLab->pVariantCool != NULL);
	iRet = xgeShaderVariantGet(&pLab->tVariantSet, 101u, &tWarmDefine, 1, &pLab->pVariantWarmCached);
	pLab->bVariantCacheOK = (iRet == XGE_OK) && (pLab->pVariantWarmCached == pLab->pVariantWarm) && (pLab->tVariantSet.iCount == 2);
	if ( !(pLab->bAddRefOK && pLab->bUniform1OK && pLab->bUniform2OK && pLab->bUniform3OK && pLab->bUniform4OK && pLab->bVariantWarmOK && pLab->bVariantCoolOK && pLab->bVariantCacheOK) ) {
		fprintf(stderr, "shader-variant-lab stage failed: addref=%d uniforms=%d/%d/%d/%d variants=%d/%d cache=%d count=%d\n",
			pLab->bAddRefOK, pLab->bUniform1OK, pLab->bUniform2OK, pLab->bUniform3OK, pLab->bUniform4OK,
			pLab->bVariantWarmOK, pLab->bVariantCoolOK, pLab->bVariantCacheOK, pLab->tVariantSet.iCount);
		return XGE_ERROR_GPU_FAILED;
	}
	InitMaterial(&pLab->tBasicMaterial, &pLab->tBasicShader, &pLab->tTexture);
	InitMaterial(&pLab->tWarmMaterial, pLab->pVariantWarm, &pLab->tTexture);
	InitMaterial(&pLab->tCoolMaterial, pLab->pVariantCool, &pLab->tTexture);
	pLab->bReady = 1;
	printf("shader-variant-lab init texture=%d shader=%d addref=%d uniforms=%d/%d/%d/%d variant_set=%d warm=%d cool=%d cache=%d count=%d\n",
		pLab->bTextureOK,
		pLab->bShaderCreateOK,
		pLab->bAddRefOK,
		pLab->bUniform1OK,
		pLab->bUniform2OK,
		pLab->bUniform3OK,
		pLab->bUniform4OK,
		pLab->bVariantSetOK,
		pLab->bVariantWarmOK,
		pLab->bVariantCoolOK,
		pLab->bVariantCacheOK,
		pLab->tVariantSet.iCount);
	return XGE_OK;
}

static void DrawMaterialQuad(xge_material pMaterial, xge_texture pTexture, float fX, float fY, float fW, float fH)
{
	xge_draw_t tDraw;

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc.fX = 0.0f;
	tDraw.tSrc.fY = 0.0f;
	tDraw.tSrc.fW = (float)pTexture->iWidth;
	tDraw.tSrc.fH = (float)pTexture->iHeight;
	tDraw.tDst.fX = fX;
	tDraw.tDst.fY = fY;
	tDraw.tDst.fW = fW;
	tDraw.tDst.fH = fH;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeMaterialDraw(pMaterial, &tDraw);
}

static void CleanupLab(shader_variant_lab_t* pLab)
{
	xgeMaterialFree(&pLab->tCoolMaterial);
	xgeMaterialFree(&pLab->tWarmMaterial);
	xgeMaterialFree(&pLab->tBasicMaterial);
	xgeShaderVariantSetFree(&pLab->tVariantSet);
	if ( pLab->bAddRefOK ) {
		xgeShaderFree(&pLab->tBasicShader);
		pLab->bAddRefOK = 0;
	}
	xgeShaderFree(&pLab->tBasicShader);
	xgeTextureFree(&pLab->tTexture);
	pLab->bReady = 0;
}

static int ShaderVariantFrame(void* pUser)
{
	shader_variant_lab_t* pLab;
	int iRet;

	pLab = (shader_variant_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		printf("shader-variant-lab esc-summary frames=%d variants=%d\n", pLab->iFrameCount, pLab->tVariantSet.iCount);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(14, 18, 26, 255));
	xgeShapeRectFillPx((xge_rect_t){ 34.0f, 42.0f, 572.0f, 334.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 34.0f, 42.0f, 572.0f, 334.0f }, 2.0f, XGE_COLOR_RGBA(110, 136, 160, 255));
	(void)SetShaderUniforms(&pLab->tBasicShader, 0.18f, 0.0f, 0.0f, 0.80f, 0.95f, 1.00f);
	DrawMaterialQuad(&pLab->tBasicMaterial, &pLab->tTexture, 64.0f, 100.0f, 128.0f, 128.0f);
	(void)SetShaderUniforms(pLab->pVariantWarm, 0.72f, 0.0f, 0.0f, 1.18f, 0.78f, 0.46f);
	DrawMaterialQuad(&pLab->tWarmMaterial, &pLab->tTexture, 256.0f, 100.0f, 128.0f, 128.0f);
	(void)SetShaderUniforms(pLab->pVariantCool, 0.68f, 0.0f, 0.0f, 0.25f, 0.76f, 1.00f);
	DrawMaterialQuad(&pLab->tCoolMaterial, &pLab->tTexture, 448.0f, 100.0f, 128.0f, 128.0f);
	pLab->bDrawn = 1;
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		printf("shader-variant-lab final-summary frames=%d texture=%d shader=%d uniforms=%d/%d/%d/%d variant_set=%d warm=%d cool=%d cache=%d count=%d drawn=%d\n",
			pLab->iFrameCount,
			pLab->bTextureOK,
			pLab->bShaderCreateOK,
			pLab->bUniform1OK,
			pLab->bUniform2OK,
			pLab->bUniform3OK,
			pLab->bUniform4OK,
			pLab->bVariantSetOK,
			pLab->bVariantWarmOK,
			pLab->bVariantCoolOK,
			pLab->bVariantCacheOK,
			pLab->tVariantSet.iCount,
			pLab->bDrawn);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	shader_variant_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_SHADER_VARIANT_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_SHADER_VARIANT_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE Shader Variant Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(ShaderVariantFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("shader-variant-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
