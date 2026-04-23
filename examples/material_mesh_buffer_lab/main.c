#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

static const char* g_materialVS =
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

static const char* g_materialFS =
	"#version 330 core\n"
	"in vec2 vUV;\n"
	"in vec4 vColor;\n"
	"uniform sampler2D uTexture;\n"
	"uniform vec4 uColor;\n"
	"out vec4 FragColor;\n"
	"void main() {\n"
	"	vec4 c = texture(uTexture, vUV) * vColor * uColor;\n"
	"	float stripe = step(0.5, fract((vUV.x + vUV.y) * 8.0));\n"
	"	c.rgb = mix(c.rgb, c.rgb * vec3(1.10, 0.82, 0.55), stripe * 0.28);\n"
	"	FragColor = c;\n"
	"}\n";

typedef struct material_mesh_buffer_lab_t {
	xge_texture_t tTexture;
	xge_shader_t tShader;
	xge_material_t tMaterial;
	xge_mesh_t tMesh;
	xge_buffer_t tVertexBuffer;
	xge_buffer_t tIndexBuffer;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bTextureOK;
	int bShaderOK;
	int bBufferCreateOK;
	int bBufferUpdateOK;
	int bBufferUploadOK;
	int bMeshCreateOK;
	int bMeshUpdateOK;
	int bMeshDrawn;
	int bMaterialInitOK;
	int bMaterialShaderOK;
	int bMaterialTextureOK;
	int bMaterialColorOK;
	int bMaterialBlendOK;
	int bMaterialDrawn;
	int bFreed;
} material_mesh_buffer_lab_t;

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
			int bCell;
			int bLine;

			iPos = ((iY * iWidth) + iX) * 4;
			bCell = (((iX / 16) + (iY / 16)) & 1) != 0;
			bLine = ((iX % 16) == 0) || ((iY % 16) == 0);
			pPixels[iPos + 0] = (unsigned char)(bLine ? 255 : (bCell ? 76 : 28));
			pPixels[iPos + 1] = (unsigned char)(bLine ? 248 : (bCell ? 168 : 84));
			pPixels[iPos + 2] = (unsigned char)(bLine ? 228 : (bCell ? 220 : 188));
			pPixels[iPos + 3] = 255;
		}
	}
}

static void FillQuad(xge_vertex_t* pVertices, float fX, float fY, float fW, float fH, uint32_t iColor)
{
	pVertices[0] = (xge_vertex_t){ fX, fY, 0.0f, 1.0f, 0.0f, 0.0f, iColor };
	pVertices[1] = (xge_vertex_t){ fX + fW, fY, 0.0f, 1.0f, 1.0f, 0.0f, iColor };
	pVertices[2] = (xge_vertex_t){ fX + fW, fY + fH, 0.0f, 1.0f, 1.0f, 1.0f, iColor };
	pVertices[3] = (xge_vertex_t){ fX, fY + fH, 0.0f, 1.0f, 0.0f, 1.0f, iColor };
}

static int InitBuffers(material_mesh_buffer_lab_t* pLab)
{
	xge_vertex_t arrVertices[4];
	xge_vertex_t arrUpdatedVertices[4];
	uint16_t arrIndices[6];
	uint16_t arrUpdatedIndices[6];
	int iRet;
	int bOK;

	arrIndices[0] = 0;
	arrIndices[1] = 1;
	arrIndices[2] = 2;
	arrIndices[3] = 0;
	arrIndices[4] = 2;
	arrIndices[5] = 3;
	memcpy(arrUpdatedIndices, arrIndices, sizeof(arrIndices));
	FillQuad(arrVertices, 48.0f, 76.0f, 144.0f, 144.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	FillQuad(arrUpdatedVertices, 56.0f, 86.0f, 136.0f, 136.0f, XGE_COLOR_RGBA(255, 246, 214, 255));

	bOK = 1;
	iRet = xgeBufferCreate(&pLab->tVertexBuffer, XGE_BUFFER_VERTEX, XGE_BUFFER_DYNAMIC, arrVertices, (int)sizeof(arrVertices));
	bOK = bOK && (iRet == XGE_OK);
	iRet = xgeBufferCreate(&pLab->tIndexBuffer, XGE_BUFFER_INDEX, XGE_BUFFER_DYNAMIC, arrIndices, (int)sizeof(arrIndices));
	bOK = bOK && (iRet == XGE_OK);
	pLab->bBufferCreateOK = bOK;
	if ( !pLab->bBufferCreateOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: buffer create\n");
		return XGE_ERROR_GPU_FAILED;
	}

	bOK = 1;
	iRet = xgeBufferUpdate(&pLab->tVertexBuffer, 0, arrUpdatedVertices, (int)sizeof(arrUpdatedVertices));
	bOK = bOK && (iRet == XGE_OK);
	iRet = xgeBufferUpdate(&pLab->tIndexBuffer, 0, arrUpdatedIndices, (int)sizeof(arrUpdatedIndices));
	bOK = bOK && (iRet == XGE_OK);
	pLab->bBufferUpdateOK = bOK;
	if ( !pLab->bBufferUpdateOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: buffer update\n");
		return XGE_ERROR_GPU_FAILED;
	}

	bOK = 1;
	iRet = xgeBufferUpload(&pLab->tVertexBuffer);
	bOK = bOK && (iRet == XGE_OK);
	iRet = xgeBufferUpload(&pLab->tIndexBuffer);
	bOK = bOK && (iRet == XGE_OK);
	pLab->bBufferUploadOK = bOK;
	if ( !pLab->bBufferUploadOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: buffer upload\n");
		return XGE_ERROR_GPU_FAILED;
	}
	return XGE_OK;
}

static int InitMesh(material_mesh_buffer_lab_t* pLab)
{
	xge_vertex_t arrVertices[4];
	xge_vertex_t arrUpdatedVertices[4];
	uint16_t arrIndices[6];
	int iRet;

	arrIndices[0] = 0;
	arrIndices[1] = 1;
	arrIndices[2] = 2;
	arrIndices[3] = 0;
	arrIndices[4] = 2;
	arrIndices[5] = 3;
	FillQuad(arrVertices, 396.0f, 94.0f, 154.0f, 142.0f, XGE_COLOR_RGBA(210, 240, 255, 235));
	FillQuad(arrUpdatedVertices, 388.0f, 86.0f, 172.0f, 156.0f, XGE_COLOR_RGBA(224, 248, 255, 245));

	iRet = xgeMeshCreate(&pLab->tMesh, arrVertices, 4, arrIndices, 6, XGE_MESH_DYNAMIC);
	pLab->bMeshCreateOK = (iRet == XGE_OK);
	if ( !pLab->bMeshCreateOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: mesh create\n");
		return XGE_ERROR_GPU_FAILED;
	}
	iRet = xgeMeshUpdate(&pLab->tMesh, arrUpdatedVertices, 4, arrIndices, 6);
	pLab->bMeshUpdateOK = (iRet == XGE_OK);
	if ( !pLab->bMeshUpdateOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: mesh update\n");
		return XGE_ERROR_GPU_FAILED;
	}
	return XGE_OK;
}

static int InitMaterial(material_mesh_buffer_lab_t* pLab)
{
	xgeMaterialInit(&pLab->tMaterial);
	pLab->bMaterialInitOK = 1;
	xgeMaterialSetShader(&pLab->tMaterial, &pLab->tShader);
	pLab->bMaterialShaderOK = (pLab->tMaterial.pShader == &pLab->tShader);
	xgeMaterialSetTexture(&pLab->tMaterial, &pLab->tTexture);
	pLab->bMaterialTextureOK = (pLab->tMaterial.pTexture == &pLab->tTexture);
	xgeMaterialSetColor(&pLab->tMaterial, XGE_COLOR_RGBA(255, 242, 216, 248));
	pLab->bMaterialColorOK = (pLab->tMaterial.iColor == XGE_COLOR_RGBA(255, 242, 216, 248));
	xgeMaterialSetBlend(&pLab->tMaterial, XGE_BLEND_ALPHA);
	pLab->bMaterialBlendOK = (pLab->tMaterial.tPipeline.iBlend == XGE_BLEND_ALPHA);
	if ( !(pLab->bMaterialShaderOK && pLab->bMaterialTextureOK && pLab->bMaterialColorOK && pLab->bMaterialBlendOK) ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: material setup shader=%d texture=%d color=%d blend=%d\n",
			pLab->bMaterialShaderOK,
			pLab->bMaterialTextureOK,
			pLab->bMaterialColorOK,
			pLab->bMaterialBlendOK);
		return XGE_ERROR_GPU_FAILED;
	}
	return XGE_OK;
}

static int InitLab(material_mesh_buffer_lab_t* pLab)
{
	unsigned char arrPixels[128 * 128 * 4];

	if ( pLab->bReady ) {
		return XGE_OK;
	}
	MakePixels(arrPixels, 128, 128);
	pLab->bTextureOK = (xgeTextureCreateRGBA(&pLab->tTexture, 128, 128, arrPixels) == XGE_OK);
	if ( !pLab->bTextureOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: texture\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bShaderOK = (xgeShaderCreate(&pLab->tShader, g_materialVS, g_materialFS) == XGE_OK);
	if ( !pLab->bShaderOK ) {
		fprintf(stderr, "material-mesh-buffer-lab stage failed: shader\n");
		return XGE_ERROR_GPU_FAILED;
	}
	if ( InitBuffers(pLab) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	if ( InitMesh(pLab) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	if ( InitMaterial(pLab) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	pLab->bReady = 1;
	printf("material-mesh-buffer-lab init texture=%d shader=%d buffer=%d/%d/%d mesh=%d/%d material=%d/%d/%d/%d/%d\n",
		pLab->bTextureOK,
		pLab->bShaderOK,
		pLab->bBufferCreateOK,
		pLab->bBufferUpdateOK,
		pLab->bBufferUploadOK,
		pLab->bMeshCreateOK,
		pLab->bMeshUpdateOK,
		pLab->bMaterialInitOK,
		pLab->bMaterialShaderOK,
		pLab->bMaterialTextureOK,
		pLab->bMaterialColorOK,
		pLab->bMaterialBlendOK);
	return XGE_OK;
}

static void DrawMaterial(material_mesh_buffer_lab_t* pLab)
{
	xge_draw_t tDraw;

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pLab->tTexture;
	tDraw.tSrc.fX = 0.0f;
	tDraw.tSrc.fY = 0.0f;
	tDraw.tSrc.fW = (float)pLab->tTexture.iWidth;
	tDraw.tSrc.fH = (float)pLab->tTexture.iHeight;
	tDraw.tDst.fX = 82.0f;
	tDraw.tDst.fY = 96.0f;
	tDraw.tDst.fW = 184.0f;
	tDraw.tDst.fH = 184.0f;
	tDraw.tOrigin.fX = 92.0f;
	tDraw.tOrigin.fY = 92.0f;
	tDraw.fRotation = 0.14f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 250);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeMaterialDraw(&pLab->tMaterial, &tDraw);
	pLab->bMaterialDrawn = 1;
}

static void DrawMesh(material_mesh_buffer_lab_t* pLab)
{
	xgeMeshDraw(&pLab->tMesh, &pLab->tTexture, XGE_DRAW_SCREEN_SPACE);
	pLab->bMeshDrawn = 1;
}

static void CleanupLab(material_mesh_buffer_lab_t* pLab)
{
	if ( pLab->bFreed ) {
		return;
	}
	xgeMaterialFree(&pLab->tMaterial);
	xgeMeshFree(&pLab->tMesh);
	xgeBufferFree(&pLab->tIndexBuffer);
	xgeBufferFree(&pLab->tVertexBuffer);
	xgeShaderFree(&pLab->tShader);
	xgeTextureFree(&pLab->tTexture);
	pLab->bFreed = 1;
	pLab->bReady = 0;
}

static void PrintFinalSummary(material_mesh_buffer_lab_t* pLab)
{
	printf("material-mesh-buffer-lab final-summary frames=%d buffer=%d/%d/%d mesh=%d/%d/%d material=%d/%d/%d/%d/%d/%d\n",
		pLab->iFrameCount,
		pLab->bBufferCreateOK,
		pLab->bBufferUpdateOK,
		pLab->bBufferUploadOK,
		pLab->bMeshCreateOK,
		pLab->bMeshUpdateOK,
		pLab->bMeshDrawn,
		pLab->bMaterialInitOK,
		pLab->bMaterialShaderOK,
		pLab->bMaterialTextureOK,
		pLab->bMaterialColorOK,
		pLab->bMaterialBlendOK,
		pLab->bMaterialDrawn);
}

static int MaterialMeshBufferFrame(void* pUser)
{
	material_mesh_buffer_lab_t* pLab;
	int iRet;

	pLab = (material_mesh_buffer_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 22, 26, 255));
	xgeShapeRectFillPx((xge_rect_t){ 30.0f, 40.0f, 580.0f, 316.0f }, XGE_COLOR_RGBA(31, 38, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 30.0f, 40.0f, 580.0f, 316.0f }, 2.0f, XGE_COLOR_RGBA(118, 136, 132, 255));
	xgeShapeRectFillPx((xge_rect_t){ 56.0f, 292.0f, 210.0f, 18.0f }, XGE_COLOR_RGBA(220, 146, 82, 255));
	xgeShapeRectFillPx((xge_rect_t){ 388.0f, 292.0f, 172.0f, 18.0f }, XGE_COLOR_RGBA(70, 164, 208, 255));
	DrawMaterial(pLab);
	DrawMesh(pLab);
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	material_mesh_buffer_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_MATERIAL_MESH_BUFFER_FRAMES"), 180);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_MATERIAL_MESH_BUFFER_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 396;
	tDesc.sTitle = "XGE Material Mesh Buffer Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MaterialMeshBufferFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("material-mesh-buffer-lab summary frames=%d freed=%d\n", tLab.iFrameCount, tLab.bFreed);
	return 0;
}
