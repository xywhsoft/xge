#include "../../xge.h"

static int MainFrame(void* pUser)
{
	static int bTextureReady = 0;
	static xge_texture_t objTexture;
	static unsigned char arrPixels[128 * 128 * 4];
	(void)pUser;
	if ( bTextureReady == 0 ) {
		int iX;
		int iY;
		for ( iY = 0; iY < 128; iY++ ) {
			for ( iX = 0; iX < 128; iX++ ) {
				int iPos = ((iY * 128) + iX) * 4;
				int bAlt = (((iX / 16) + (iY / 16)) & 1);
				arrPixels[iPos + 0] = bAlt ? 255 : 64;
				arrPixels[iPos + 1] = bAlt ? 210 : 120;
				arrPixels[iPos + 2] = bAlt ? 64 : 255;
				arrPixels[iPos + 3] = 255;
			}
		}
		if ( xgeTextureCreateRGBA(&objTexture, 128, 128, arrPixels) == XGE_OK ) {
			bTextureReady = 1;
		}
	}
	if ( xgeKeyDown(256) ) {
		if ( bTextureReady ) {
			xgeTextureFree(&objTexture);
			bTextureReady = 0;
		}
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(24, 32, 48, 255));
	if ( bTextureReady ) {
		xgeDraw(&objTexture, 360.0f, 260.0f);
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t objDesc;
	(void)argc;
	(void)argv;

	objDesc.iWidth = 800;
	objDesc.iHeight = 600;
	objDesc.sTitle = "XGE V2 MVP";
	objDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	objDesc.iRunMode = XGE_RUN_GAME_LOOP;
	objDesc.iTargetFPS = 60;
	objDesc.pNativeWindow = 0;
	objDesc.pUser = 0;

	if ( xgeInit(&objDesc) != XGE_OK ) {
		return 1;
	}

	xgeRun(MainFrame, 0);
	xgeUnit();
	return 0;
}
