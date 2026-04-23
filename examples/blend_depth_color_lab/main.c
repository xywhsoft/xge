#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../xge.h"

typedef struct depth_quad_t {
	xge_mesh_t tMesh;
	uint32_t iColor;
} depth_quad_t;

typedef struct blend_depth_color_lab_t {
	xge_texture_t tTexture;
	depth_quad_t arrDepth[4];
	uint32_t iPackedColor;
	xge_color_t tUnpackedColor;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bTextureOK;
	int bColorPackOK;
	int bBlendNoneOK;
	int bBlendAlphaOK;
	int bBlendAddOK;
	int bDepthOffOK;
	int bDepthOnOK;
	int bBlendDrawn;
	int bDepthDrawn;
} blend_depth_color_lab_t;

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

static void MakeSoftDisc(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	float fCX;
	float fCY;
	float fRadius;

	fCX = ((float)iWidth - 1.0f) * 0.5f;
	fCY = ((float)iHeight - 1.0f) * 0.5f;
	fRadius = (float)((iWidth < iHeight) ? iWidth : iHeight) * 0.43f;
	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			int iPos;
			float fDX;
			float fDY;
			float fDist;
			float fAlpha;
			unsigned char iValue;

			iPos = ((iY * iWidth) + iX) * 4;
			fDX = (float)iX - fCX;
			fDY = (float)iY - fCY;
			fDist = sqrtf((fDX * fDX) + (fDY * fDY));
			fAlpha = 1.0f - (fDist / fRadius);
			if ( fAlpha < 0.0f ) {
				fAlpha = 0.0f;
			}
			fAlpha = fAlpha * fAlpha;
			iValue = (unsigned char)(fAlpha * 255.0f);
			pPixels[iPos + 0] = iValue;
			pPixels[iPos + 1] = iValue;
			pPixels[iPos + 2] = iValue;
			pPixels[iPos + 3] = iValue;
		}
	}
}

static void FillQuad(xge_vertex_t* pVertices, float fX, float fY, float fW, float fH, float fZ, uint32_t iColor)
{
	pVertices[0] = (xge_vertex_t){ fX, fY, fZ, 1.0f, 0.0f, 0.0f, iColor };
	pVertices[1] = (xge_vertex_t){ fX + fW, fY, fZ, 1.0f, 1.0f, 0.0f, iColor };
	pVertices[2] = (xge_vertex_t){ fX + fW, fY + fH, fZ, 1.0f, 1.0f, 1.0f, iColor };
	pVertices[3] = (xge_vertex_t){ fX, fY + fH, fZ, 1.0f, 0.0f, 1.0f, iColor };
}

static int InitDepthMesh(depth_quad_t* pQuad, float fX, float fY, float fW, float fH, float fZ, uint32_t iColor)
{
	static const uint16_t arrIndices[6] = { 0, 1, 2, 0, 2, 3 };
	xge_vertex_t arrVertices[4];

	pQuad->iColor = iColor;
	FillQuad(arrVertices, fX, fY, fW, fH, fZ, iColor);
	return xgeMeshCreate(&pQuad->tMesh, arrVertices, 4, arrIndices, 6, 0);
}

static int InitLab(blend_depth_color_lab_t* pLab)
{
	unsigned char arrPixels[96 * 96 * 4];
	int iR;
	int iG;
	int iB;
	int iA;

	if ( pLab->bReady ) {
		return XGE_OK;
	}
	MakeSoftDisc(arrPixels, 96, 96);
	pLab->bTextureOK = (xgeTextureCreateRGBA(&pLab->tTexture, 96, 96, arrPixels) == XGE_OK);
	if ( !pLab->bTextureOK ) {
		fprintf(stderr, "blend-depth-color-lab stage failed: texture\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->iPackedColor = xgeColorRGBA(84, 172, 255, 208);
	pLab->tUnpackedColor = xgeColorUnpack(pLab->iPackedColor);
	iR = (int)(pLab->tUnpackedColor.fR * 255.0f + 0.5f);
	iG = (int)(pLab->tUnpackedColor.fG * 255.0f + 0.5f);
	iB = (int)(pLab->tUnpackedColor.fB * 255.0f + 0.5f);
	iA = (int)(pLab->tUnpackedColor.fA * 255.0f + 0.5f);
	pLab->bColorPackOK = (iR == 84) && (iG == 172) && (iB == 255) && (iA == 208);
	if ( !pLab->bColorPackOK ) {
		fprintf(stderr, "blend-depth-color-lab stage failed: color pack/unpack rgba=%d,%d,%d,%d\n", iR, iG, iB, iA);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( InitDepthMesh(&pLab->arrDepth[0], 64.0f, 232.0f, 118.0f, 118.0f, -0.55f, xgeColorRGBA(80, 180, 255, 236)) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	if ( InitDepthMesh(&pLab->arrDepth[1], 132.0f, 254.0f, 126.0f, 126.0f, 0.25f, xgeColorRGBA(255, 156, 88, 228)) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	if ( InitDepthMesh(&pLab->arrDepth[2], 356.0f, 232.0f, 118.0f, 118.0f, -0.55f, xgeColorRGBA(80, 180, 255, 236)) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	if ( InitDepthMesh(&pLab->arrDepth[3], 424.0f, 254.0f, 126.0f, 126.0f, 0.25f, xgeColorRGBA(255, 156, 88, 228)) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	pLab->bReady = 1;
	printf("blend-depth-color-lab init texture=%d color=%d rgba=84,172,255,208\n",
		pLab->bTextureOK,
		pLab->bColorPackOK);
	return XGE_OK;
}

static void DrawDisc(xge_texture pTexture, float fX, float fY, uint32_t iColor)
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
	tDraw.tDst.fW = 92.0f;
	tDraw.tDst.fH = 92.0f;
	tDraw.iColor = iColor;
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

static void DrawBlendPanels(blend_depth_color_lab_t* pLab)
{
	xgeShapeRectFillPx((xge_rect_t){ 34.0f, 34.0f, 174.0f, 140.0f }, XGE_COLOR_RGBA(36, 42, 48, 255));
	xgeShapeRectFillPx((xge_rect_t){ 232.0f, 34.0f, 174.0f, 140.0f }, XGE_COLOR_RGBA(36, 42, 48, 255));
	xgeShapeRectFillPx((xge_rect_t){ 430.0f, 34.0f, 174.0f, 140.0f }, XGE_COLOR_RGBA(36, 42, 48, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 34.0f, 34.0f, 174.0f, 140.0f }, 2.0f, pLab->iPackedColor);
	xgeShapeRectStrokePx((xge_rect_t){ 232.0f, 34.0f, 174.0f, 140.0f }, 2.0f, pLab->iPackedColor);
	xgeShapeRectStrokePx((xge_rect_t){ 430.0f, 34.0f, 174.0f, 140.0f }, 2.0f, pLab->iPackedColor);

	xgeBlendSet(XGE_BLEND_NONE);
	pLab->bBlendNoneOK = (xgeBlendGet() == XGE_BLEND_NONE);
	DrawDisc(&pLab->tTexture, 54.0f, 58.0f, xgeColorRGBA(255, 148, 96, 196));
	DrawDisc(&pLab->tTexture, 104.0f, 82.0f, xgeColorRGBA(90, 176, 255, 188));

	xgeBlendSet(XGE_BLEND_ALPHA);
	pLab->bBlendAlphaOK = (xgeBlendGet() == XGE_BLEND_ALPHA);
	DrawDisc(&pLab->tTexture, 252.0f, 58.0f, xgeColorRGBA(255, 148, 96, 196));
	DrawDisc(&pLab->tTexture, 302.0f, 82.0f, xgeColorRGBA(90, 176, 255, 188));

	xgeBlendSet(XGE_BLEND_ADD);
	pLab->bBlendAddOK = (xgeBlendGet() == XGE_BLEND_ADD);
	DrawDisc(&pLab->tTexture, 450.0f, 58.0f, xgeColorRGBA(255, 148, 96, 196));
	DrawDisc(&pLab->tTexture, 500.0f, 82.0f, xgeColorRGBA(90, 176, 255, 188));

	xgeBlendSet(XGE_BLEND_ALPHA);
	pLab->bBlendDrawn = 1;
}

static void DrawDepthPanels(blend_depth_color_lab_t* pLab)
{
	xgeShapeRectFillPx((xge_rect_t){ 34.0f, 198.0f, 262.0f, 160.0f }, XGE_COLOR_RGBA(28, 34, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 344.0f, 198.0f, 262.0f, 160.0f }, XGE_COLOR_RGBA(28, 34, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 34.0f, 198.0f, 262.0f, 160.0f }, 2.0f, XGE_COLOR_RGBA(112, 126, 132, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 344.0f, 198.0f, 262.0f, 160.0f }, 2.0f, XGE_COLOR_RGBA(112, 126, 132, 255));

	xgeDepthTestSet(0);
	pLab->bDepthOffOK = (xgeDepthTestGet() == 0);
	xgeMeshDraw(&pLab->arrDepth[0].tMesh, &pLab->tTexture, XGE_DRAW_SCREEN_SPACE);
	xgeMeshDraw(&pLab->arrDepth[1].tMesh, &pLab->tTexture, XGE_DRAW_SCREEN_SPACE);

	xgeDepthTestSet(1);
	pLab->bDepthOnOK = (xgeDepthTestGet() == 1);
	xgeMeshDraw(&pLab->arrDepth[2].tMesh, &pLab->tTexture, XGE_DRAW_SCREEN_SPACE);
	xgeMeshDraw(&pLab->arrDepth[3].tMesh, &pLab->tTexture, XGE_DRAW_SCREEN_SPACE);

	xgeDepthTestSet(0);
	pLab->bDepthDrawn = 1;
}

static void CleanupLab(blend_depth_color_lab_t* pLab)
{
	int i;

	for ( i = 0; i < 4; i++ ) {
		xgeMeshFree(&pLab->arrDepth[i].tMesh);
	}
	xgeTextureFree(&pLab->tTexture);
	pLab->bReady = 0;
}

static void PrintFinalSummary(blend_depth_color_lab_t* pLab)
{
	printf("blend-depth-color-lab final-summary frames=%d color=%d blend=%d/%d/%d depth=%d/%d drawn=%d/%d\n",
		pLab->iFrameCount,
		pLab->bColorPackOK,
		pLab->bBlendNoneOK,
		pLab->bBlendAlphaOK,
		pLab->bBlendAddOK,
		pLab->bDepthOffOK,
		pLab->bDepthOnOK,
		pLab->bBlendDrawn,
		pLab->bDepthDrawn);
}

static int BlendDepthColorFrame(void* pUser)
{
	blend_depth_color_lab_t* pLab;
	int iRet;

	pLab = (blend_depth_color_lab_t*)pUser;
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
	DrawBlendPanels(pLab);
	DrawDepthPanels(pLab);
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
	blend_depth_color_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_BLEND_DEPTH_COLOR_FRAMES"), 180);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_BLEND_DEPTH_COLOR_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 392;
	tDesc.sTitle = "XGE Blend Depth Color Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(BlendDepthColorFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("blend-depth-color-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
