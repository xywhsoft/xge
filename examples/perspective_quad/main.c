#include "../../xge.h"
#include <math.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct perspective_quad_demo_t {
	xge_texture_t tTexture;
	int bReady;
	float fTime;
} perspective_quad_demo_t;

static void __perspectiveQuadMakePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iPos;
	int bGrid;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iPos = ((iY * iWidth) + iX) * 4;
			bGrid = ((iX % 32) < 3) || ((iY % 32) < 3);
			pPixels[iPos + 0] = (unsigned char)(bGrid ? 245 : (40 + (iX * 180 / iWidth)));
			pPixels[iPos + 1] = (unsigned char)(bGrid ? 245 : (80 + (iY * 120 / iHeight)));
			pPixels[iPos + 2] = (unsigned char)(bGrid ? 245 : 210);
			pPixels[iPos + 3] = 255;
		}
	}
}

static int __perspectiveQuadInit(perspective_quad_demo_t* pDemo)
{
	unsigned char arrPixels[256 * 256 * 4];

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	__perspectiveQuadMakePixels(arrPixels, 256, 256);
	if ( xgeTextureCreateRGBA(&pDemo->tTexture, 256, 256, arrPixels) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pDemo->bReady = 1;
	return XGE_OK;
}

static void __perspectiveQuadSetVertex(xge_vertex_t* pVertex, float fX, float fY, float fZ, float fW, float fU, float fV, uint32_t iColor)
{
	pVertex->fX = fX;
	pVertex->fY = fY;
	pVertex->fZ = fZ;
	pVertex->fW = fW;
	pVertex->fU = fU;
	pVertex->fV = fV;
	pVertex->iColor = iColor;
}

static int PerspectiveQuadFrame(void* pUser)
{
	perspective_quad_demo_t* pDemo;
	xge_vertex_t arrVertices[4];
	float fSwing;

	pDemo = (perspective_quad_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __perspectiveQuadInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();
	fSwing = sinf(pDemo->fTime * 1.2f);

	xgeClear(XGE_COLOR_RGBA(16, 18, 24, 255));
	xgeShapeRectFillPx((xge_rect_t){ 42.0f, 42.0f, 556.0f, 364.0f }, XGE_COLOR_RGBA(28, 34, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 42.0f, 42.0f, 556.0f, 364.0f }, 2.0f, XGE_COLOR_RGBA(90, 118, 136, 255));

	__perspectiveQuadSetVertex(&arrVertices[0], 154.0f + fSwing * 18.0f, 102.0f, 0.0f, 0.72f, 0.0f, 0.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	__perspectiveQuadSetVertex(&arrVertices[1], 492.0f, 86.0f + fSwing * 12.0f, 0.0f, 1.35f, 1.0f, 0.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	__perspectiveQuadSetVertex(&arrVertices[2], 124.0f, 344.0f - fSwing * 10.0f, 0.0f, 0.92f, 0.0f, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	__perspectiveQuadSetVertex(&arrVertices[3], 518.0f + fSwing * 16.0f, 326.0f, 0.0f, 1.75f, 1.0f, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeDrawQuad3D(&pDemo->tTexture, arrVertices, XGE_DRAW_SCREEN_SPACE);

	xgeShapeLinePx(arrVertices[0].fX, arrVertices[0].fY, arrVertices[1].fX, arrVertices[1].fY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 170));
	xgeShapeLinePx(arrVertices[1].fX, arrVertices[1].fY, arrVertices[3].fX, arrVertices[3].fY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 170));
	xgeShapeLinePx(arrVertices[3].fX, arrVertices[3].fY, arrVertices[2].fX, arrVertices[2].fY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 170));
	xgeShapeLinePx(arrVertices[2].fX, arrVertices[2].fY, arrVertices[0].fX, arrVertices[0].fY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 170));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	perspective_quad_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Perspective Quad";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(PerspectiveQuadFrame, &tDemo);
	xgeTextureFree(&tDemo.tTexture);
	xgeUnit();
	return 0;
}
