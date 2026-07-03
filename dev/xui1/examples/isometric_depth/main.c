#include "../../xge.h"
#include <math.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256
#define XGE_ISO_TILE_COUNT	25

typedef struct iso_tile_t {
	xge_mesh_t tMesh;
	xge_texture_t tTexture;
} iso_tile_t;

typedef struct isometric_depth_demo_t {
	iso_tile_t arrTiles[XGE_ISO_TILE_COUNT];
	int bReady;
	float fTime;
} isometric_depth_demo_t;

static void __isoMakeTexture(unsigned char* pPixels, int iWidth, int iHeight, int iSeed)
{
	int iX;
	int iY;
	int iPos;
	int bBorder;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iPos = ((iY * iWidth) + iX) * 4;
			bBorder = (iX < 3) || (iY < 3) || (iX >= iWidth - 3) || (iY >= iHeight - 3);
			pPixels[iPos + 0] = (unsigned char)(bBorder ? 255 : 70 + (iSeed * 31) % 120);
			pPixels[iPos + 1] = (unsigned char)(bBorder ? 255 : 120 + (iSeed * 19) % 90);
			pPixels[iPos + 2] = (unsigned char)(bBorder ? 255 : 180 + (iSeed * 13) % 60);
			pPixels[iPos + 3] = 255;
		}
	}
}

static void __isoSetVertex(xge_vertex_t* pVertex, float fX, float fY, float fZ, float fU, float fV, uint32_t iColor)
{
	pVertex->fX = fX;
	pVertex->fY = fY;
	pVertex->fZ = fZ;
	pVertex->fW = 1.0f;
	pVertex->fU = fU;
	pVertex->fV = fV;
	pVertex->iColor = iColor;
}

static int __isoInit(isometric_depth_demo_t* pDemo)
{
	static const uint16_t arrIndices[6] = { 0, 1, 2, 2, 1, 3 };
	unsigned char arrPixels[64 * 64 * 4];
	xge_vertex_t arrVertices[4];
	int iX;
	int iY;
	int iIndex;
	float fCenterX;
	float fCenterY;
	float fZ;
	uint32_t iColor;

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	iIndex = 0;
	for ( iY = 0; iY < 5; iY++ ) {
		for ( iX = 0; iX < 5; iX++ ) {
			fCenterX = 320.0f + (float)(iX - iY) * 42.0f;
			fCenterY = 92.0f + (float)(iX + iY) * 24.0f;
			fZ = -0.88f + (float)(iX + iY) * 0.055f;
			iColor = XGE_COLOR_RGBA(255, 255, 255, 235);
			__isoSetVertex(&arrVertices[0], fCenterX, fCenterY - 24.0f, fZ, 0.5f, 0.0f, iColor);
			__isoSetVertex(&arrVertices[1], fCenterX + 42.0f, fCenterY, fZ, 1.0f, 0.5f, iColor);
			__isoSetVertex(&arrVertices[2], fCenterX - 42.0f, fCenterY, fZ, 0.0f, 0.5f, iColor);
			__isoSetVertex(&arrVertices[3], fCenterX, fCenterY + 24.0f, fZ, 0.5f, 1.0f, iColor);
			__isoMakeTexture(arrPixels, 64, 64, iIndex);
			if ( xgeTextureCreateRGBA(&pDemo->arrTiles[iIndex].tTexture, 64, 64, arrPixels) != XGE_OK ) {
				return XGE_ERROR_RESOURCE_FAILED;
			}
			if ( xgeMeshCreate(&pDemo->arrTiles[iIndex].tMesh, arrVertices, 4, arrIndices, 6, 0) != XGE_OK ) {
				return XGE_ERROR_GPU_FAILED;
			}
			iIndex++;
		}
	}
	pDemo->bReady = 1;
	return XGE_OK;
}

static int IsometricDepthFrame(void* pUser)
{
	isometric_depth_demo_t* pDemo;
	int i;

	pDemo = (isometric_depth_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __isoInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();

	xgeClear(XGE_COLOR_RGBA(14, 18, 25, 255));
	xgeShapeRectFillPx((xge_rect_t){ 44.0f, 42.0f, 552.0f, 360.0f }, XGE_COLOR_RGBA(28, 34, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 44.0f, 42.0f, 552.0f, 360.0f }, 2.0f, XGE_COLOR_RGBA(90, 118, 136, 255));

	xgeDepthTestSet(1);
	for ( i = 0; i < XGE_ISO_TILE_COUNT; i++ ) {
		xgeMeshDraw(&pDemo->arrTiles[i].tMesh, &pDemo->arrTiles[i].tTexture, XGE_DRAW_SCREEN_SPACE);
	}
	xgeDepthTestSet(0);
	xgeShapeCircleFillPx(320.0f + sinf(pDemo->fTime * 1.5f) * 150.0f, 340.0f, 10.0f, XGE_COLOR_RGBA(255, 220, 96, 255));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	isometric_depth_demo_t tDemo;
	int i;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Isometric Depth";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(IsometricDepthFrame, &tDemo);
	for ( i = 0; i < XGE_ISO_TILE_COUNT; i++ ) {
		xgeMeshFree(&tDemo.arrTiles[i].tMesh);
		xgeTextureFree(&tDemo.arrTiles[i].tTexture);
	}
	xgeUnit();
	return 0;
}
