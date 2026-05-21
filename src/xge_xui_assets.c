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

int xgeXuiBuiltinAssetGetCount(void)
{
	return XGE_XUI_BUILTIN_ASSET_COUNT;
}

int xgeXuiBuiltinAssetGetRect(const char* sName, xge_rect_t* pRect)
{
	const xge_xui_builtin_asset_t* pAsset;
	int i;

	if ( (sName == NULL) || (pRect == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < XGE_XUI_BUILTIN_ASSET_COUNT; i++ ) {
		pAsset = &g_arrXgeXuiBuiltinAssets[i];
		if ( strcmp(pAsset->sName, sName) == 0 ) {
			*pRect = (xge_rect_t){ (float)pAsset->iX, (float)pAsset->iY, (float)pAsset->iW, (float)pAsset->iH };
			return XGE_OK;
		}
	}
	*pRect = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	return XGE_ERROR_FILE_NOT_FOUND;
}

static int __xgeXuiBuiltinAssetDrawBuild(xge_rect_t tDst, int iAsset, uint32_t iColor, xge_draw_t* pDraw)
{
	xge_texture pTexture;

	if ( (pDraw == NULL) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return 0;
	}
	pTexture = __xgeXuiBuiltinAtlasTexture();
	if ( pTexture == NULL ) {
		return 0;
	}
	memset(pDraw, 0, sizeof(*pDraw));
	pDraw->pTexture = pTexture;
	pDraw->tSrc = __xgeXuiBuiltinAssetSrc(iAsset);
	pDraw->tDst = tDst;
	pDraw->iColor = iColor;
	pDraw->iFlags = XGE_DRAW_SCREEN_SPACE;
	return 1;
}

static int __xgeXuiBuiltinAssetDrawEx(xge_rect_t tDst, int iAsset, uint32_t iColor, int bRenderCache)
{
	xge_draw_t tDraw;

	if ( __xgeXuiBuiltinAssetDrawBuild(tDst, iAsset, iColor, &tDraw) == 0 ) {
		return 0;
	}
	if ( bRenderCache != 0 ) {
		xgeDrawEx(&tDraw);
	} else {
		__xgeXuiHostDrawImage(&tDraw);
	}
	return 1;
}

static void __xgeXuiBuiltinAssetDrawClipOnly(xge_rect_t tDst, int iAsset, uint32_t iColor)
{
	xge_draw_t tDraw;

	if ( __xgeXuiBuiltinAssetDrawBuild(tDst, iAsset, iColor, &tDraw) != 0 ) {
		__xgeXuiHostDrawImageClipOnly(&tDraw);
	}
}

static int __xgeXuiBuiltinAssetDraw(xge_rect_t tDst, int iAsset, uint32_t iColor)
{
	return __xgeXuiBuiltinAssetDrawEx(tDst, iAsset, iColor, 0);
}
