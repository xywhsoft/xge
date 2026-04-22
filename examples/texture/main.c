#include "../../xge.h"
#include <math.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct texture_demo_t {
	xge_texture_t tTexture;
	int bReady;
	float fTime;
} texture_demo_t;

static void __textureMakePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iPos;
	int bTile;
	int iAlpha;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iPos = ((iY * iWidth) + iX) * 4;
			bTile = (((iX / 16) + (iY / 16)) & 1);
			iAlpha = 255;
			if ( (iX < 8) || (iY < 8) || (iX >= iWidth - 8) || (iY >= iHeight - 8) ) {
				iAlpha = 150;
			}
			pPixels[iPos + 0] = (unsigned char)(bTile ? 255 : 40);
			pPixels[iPos + 1] = (unsigned char)(bTile ? 210 : 120);
			pPixels[iPos + 2] = (unsigned char)(bTile ? 72 : 230);
			pPixels[iPos + 3] = (unsigned char)iAlpha;
		}
	}
}

static int __textureDemoInit(texture_demo_t* pDemo)
{
	unsigned char arrPixels[128 * 128 * 4];

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	__textureMakePixels(arrPixels, 128, 128);
	if ( xgeTextureCreateRGBA(&pDemo->tTexture, 128, 128, arrPixels) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pDemo->bReady = 1;
	return XGE_OK;
}

static int TextureFrame(void* pUser)
{
	texture_demo_t* pDemo;
	xge_draw_t tDraw;
	float fSpin;

	pDemo = (texture_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __textureDemoInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();
	fSpin = pDemo->fTime * 1.4f;

	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeShapeRectFillPx((xge_rect_t){ 32.0f, 32.0f, 576.0f, 384.0f }, XGE_COLOR_RGBA(32, 40, 52, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 32.0f, 32.0f, 576.0f, 384.0f }, 2.0f, XGE_COLOR_RGBA(100, 126, 150, 255));

	xgeDraw(&pDemo->tTexture, 72.0f, 72.0f);
	xgeDrawPx(&pDemo->tTexture, 252, 72);

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tTexture;
	tDraw.tSrc.fX = 16.0f;
	tDraw.tSrc.fY = 16.0f;
	tDraw.tSrc.fW = 96.0f;
	tDraw.tSrc.fH = 96.0f;
	tDraw.tDst.fX = 430.0f;
	tDraw.tDst.fY = 72.0f;
	tDraw.tDst.fW = 128.0f;
	tDraw.tDst.fH = 128.0f;
	tDraw.iColor = XGE_COLOR_RGBA(120, 220, 255, 210);
	xgeDrawEx(&tDraw);

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tTexture;
	tDraw.tDst.fX = 150.0f;
	tDraw.tDst.fY = 290.0f;
	tDraw.tDst.fW = 160.0f;
	tDraw.tDst.fH = 160.0f;
	tDraw.tOrigin.fX = 80.0f;
	tDraw.tOrigin.fY = 80.0f;
	tDraw.fRotation = fSpin;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 235);
	xgeDrawEx(&tDraw);

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tTexture;
	tDraw.tDst.fX = 380.0f;
	tDraw.tDst.fY = 250.0f;
	tDraw.tDst.fW = 180.0f + sinf(pDemo->fTime * 2.0f) * 36.0f;
	tDraw.tDst.fH = 96.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 170, 170, 220);
	tDraw.iFlags = XGE_DRAW_FLIP_X;
	xgeDrawEx(&tDraw);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	texture_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Texture";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(TextureFrame, &tDemo);
	xgeTextureFree(&tDemo.tTexture);
	xgeUnit();
	return 0;
}
