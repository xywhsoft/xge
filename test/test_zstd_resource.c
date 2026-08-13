#include "xge.h"

#include <stdio.h>

typedef struct xui_builtin_asset_t {
	const char* sName;
	int iX;
	int iY;
	int iW;
	int iH;
} xui_builtin_asset_t;

#include "src/xui_builtin_assets.inc"

int main(void)
{
	unsigned char* pPixels;
	uint32_t iHash;
	int iDecodedSize;
	int iRet;
	int i;

	pPixels = (unsigned char*)xrtMalloc((size_t)XUI_BUILTIN_ATLAS_RGBA_SIZE);
	if ( pPixels == NULL ) {
		printf("xge_zstd_resource_test failed: allocation\n");
		return 1;
	}
	iDecodedSize = 0;
	iRet = xgeZstdDecompress(pPixels, XUI_BUILTIN_ATLAS_RGBA_SIZE,
		g_arrXuiBuiltinAtlasRgbaZstd, g_iXuiBuiltinAtlasRgbaZstdSize, &iDecodedSize);
	if ( (iRet != XGE_OK) || (iDecodedSize != XUI_BUILTIN_ATLAS_RGBA_SIZE) ) {
		xrtFree(pPixels);
		printf("xge_zstd_resource_test failed: decode ret=%d size=%d\n", iRet, iDecodedSize);
		return 1;
	}
	iHash = 2166136261u;
	for ( i = 0; i < iDecodedSize; i++ ) {
		iHash = (iHash ^ pPixels[i]) * 16777619u;
	}
	xrtFree(pPixels);
	if ( iHash != 0x78f87662u ) {
		printf("xge_zstd_resource_test failed: hash=%08x\n", (unsigned int)iHash);
		return 1;
	}
	iDecodedSize = -1;
	iRet = xgeZstdDecompress(&iHash, (int)sizeof(iHash),
		g_arrXuiBuiltinAtlasRgbaZstd, g_iXuiBuiltinAtlasRgbaZstdSize, &iDecodedSize);
	if ( (iRet != XGE_ERROR_RESOURCE_FAILED) || (iDecodedSize != 0) ) {
		printf("xge_zstd_resource_test failed: small output ret=%d size=%d\n", iRet, iDecodedSize);
		return 1;
	}
	printf("xge_zstd_resource_test passed\n");
	return 0;
}
