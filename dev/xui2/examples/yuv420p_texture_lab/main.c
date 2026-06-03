#include "../../xge.h"

static xge_texture_t g_tVideo;
static unsigned char g_arrY[320 * 180];
static unsigned char g_arrU[(320 / 2) * (180 / 2)];
static unsigned char g_arrV[(320 / 2) * (180 / 2)];
static int g_bReady;

static void BuildYUVFrame(int iFrame)
{
	int x;
	int y;

	for ( y = 0; y < 180; y++ ) {
		for ( x = 0; x < 320; x++ ) {
			g_arrY[(y * 320) + x] = (unsigned char)((x + iFrame) & 255);
		}
	}
	for ( y = 0; y < 90; y++ ) {
		for ( x = 0; x < 160; x++ ) {
			g_arrU[(y * 160) + x] = (unsigned char)(96 + ((y + iFrame / 2) & 63));
			g_arrV[(y * 160) + x] = (unsigned char)(160 - ((x + iFrame / 3) & 63));
		}
	}
}

static int MainFrame(void* pUser)
{
	static int iFrame;
	xge_draw_t tDraw;
	(void)pUser;

	if ( !g_bReady ) {
		if ( xgeTextureCreateYUV420P(&g_tVideo, 320, 180) != XGE_OK ) {
			return 1;
		}
		g_bReady = 1;
	}

	BuildYUVFrame(iFrame++);
	(void)xgeTextureUpdateYUV420P(&g_tVideo, g_arrY, 320, g_arrU, 160, g_arrV, 160);

	xgeClear(XGE_COLOR_RGBA(18, 20, 24, 255));
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &g_tVideo;
	tDraw.tDst.fX = 80.0f;
	tDraw.tDst.fY = 70.0f;
	tDraw.tDst.fW = 640.0f;
	tDraw.tDst.fH = 360.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	xgeDrawEx(&tDraw);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	(void)argc;
	(void)argv;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sTitle = "XGE YUV420P Texture Lab";
	tDesc.iWidth = 800;
	tDesc.iHeight = 500;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MainFrame, NULL);
	xgeTextureFree(&g_tVideo);
	xgeUnit();
	return 0;
}
