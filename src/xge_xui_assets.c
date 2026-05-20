typedef struct xge_xui_builtin_asset_t {
	const char* sName;
	int iX;
	int iY;
	int iW;
	int iH;
} xge_xui_builtin_asset_t;

#include "xge_xui_builtin_assets.inc"

static xge_texture __xgeXuiBuiltinAtlasTexture(void)
{
	static xge_texture_t tTexture;
	static int iState = 0;

	if ( iState == 1 ) {
		return &tTexture;
	}
	if ( iState < 0 ) {
		return NULL;
	}
	if ( xgeTextureLoadMemoryEx(&tTexture, g_arrXgeXuiBuiltinAtlasPng, g_iXgeXuiBuiltinAtlasPngSize, XGE_IMAGE_PREMULTIPLIED) != XGE_OK ) {
		iState = -1;
		return NULL;
	}
	iState = 1;
	return &tTexture;
}

static xge_rect_t __xgeXuiBuiltinAssetSrc(int iAsset)
{
	const xge_xui_builtin_asset_t* pAsset;

	if ( (iAsset < 0) || (iAsset >= XGE_XUI_BUILTIN_ASSET_COUNT) ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	pAsset = &g_arrXgeXuiBuiltinAssets[iAsset];
	return (xge_rect_t){ (float)pAsset->iX, (float)pAsset->iY, (float)pAsset->iW, (float)pAsset->iH };
}

static void __xgeXuiBuiltinAssetDrawEx(xge_rect_t tDst, int iAsset, uint32_t iColor, int bRenderCache)
{
	xge_texture pTexture;
	xge_draw_t tDraw;

	if ( (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return;
	}
	pTexture = __xgeXuiBuiltinAtlasTexture();
	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc = __xgeXuiBuiltinAssetSrc(iAsset);
	tDraw.tDst = tDst;
	tDraw.iColor = iColor;
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	if ( bRenderCache != 0 ) {
		xgeDrawEx(&tDraw);
	} else {
		__xgeXuiHostDrawImage(&tDraw);
	}
}

static void __xgeXuiBuiltinAssetDraw(xge_rect_t tDst, int iAsset, uint32_t iColor)
{
	__xgeXuiBuiltinAssetDrawEx(tDst, iAsset, iColor, 0);
}

