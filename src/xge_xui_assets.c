typedef struct xge_xui_builtin_asset_t {
	const char* sName;
	int iX;
	int iY;
	int iW;
	int iH;
} xge_xui_builtin_asset_t;

#define XGE_XUI_BUILTIN_ATLAS_CACHE_CAPACITY 8

typedef struct xge_xui_builtin_atlas_cache_t {
	const xge_xui_host_v2_t* pHostV2;
	xui_texture pTexture;
	int iUnsupported;
} xge_xui_builtin_atlas_cache_t;

#include "xge_xui_builtin_assets.inc"

static xui_texture __xgeXuiBuiltinAtlasTexture(void)
{
	static xge_xui_builtin_atlas_cache_t arrCache[XGE_XUI_BUILTIN_ATLAS_CACHE_CAPACITY];
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_builtin_atlas_cache_t* pFree;
	int i;

	pHostV2 = __xgeXuiHostV2ForContext(g_xgeXuiActiveContext);
	if ( pHostV2 == NULL ) {
		return NULL;
	}
	pFree = NULL;
	for ( i = 0; i < XGE_XUI_BUILTIN_ATLAS_CACHE_CAPACITY; i++ ) {
		if ( arrCache[i].pHostV2 == pHostV2 ) {
			if ( arrCache[i].iUnsupported != 0 ) {
				return NULL;
			}
			if ( arrCache[i].pTexture != NULL ) {
				return arrCache[i].pTexture;
			}
			pFree = &arrCache[i];
			break;
		}
		if ( (pFree == NULL) && (arrCache[i].pHostV2 == NULL) ) {
			pFree = &arrCache[i];
		}
	}
	if ( pFree == NULL ) {
		return NULL;
	}
	pFree->pHostV2 = pHostV2;
	if ( pHostV2->texture_create_memory == NULL ) {
		pFree->iUnsupported = 1;
		return NULL;
	}
	if ( pHostV2->texture_create_memory(&pFree->pTexture, g_arrXgeXuiBuiltinAtlasPng, g_iXgeXuiBuiltinAtlasPngSize, XGE_IMAGE_PREMULTIPLIED, pHostV2->pUser) != XGE_OK ) {
		pFree->pTexture = NULL;
		return NULL;
	}
	return pFree->pTexture;
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
	xui_texture pTexture;

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

static int __xgeXuiBuiltinAssetDraw(xge_rect_t tDst, int iAsset, uint32_t iColor)
{
	xge_draw_t tDraw;

	if ( __xgeXuiBuiltinAssetDrawBuild(tDst, iAsset, iColor, &tDraw) == 0 ) {
		return 0;
	}
	__xgeXuiHostDrawImage(&tDraw);
	return 1;
}

static void __xgeXuiBuiltinAssetDrawClipOnly(xge_rect_t tDst, int iAsset, uint32_t iColor)
{
	xge_draw_t tDraw;

	if ( __xgeXuiBuiltinAssetDrawBuild(tDst, iAsset, iColor, &tDraw) != 0 ) {
		__xgeXuiHostDrawImageClipOnly(&tDraw);
	}
}
