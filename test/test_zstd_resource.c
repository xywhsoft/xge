#include "xge.h"
#include "src/xui_builtin_atlas.h"

#include <stdio.h>

int main(void)
{
	unsigned char* pPixels;
	uint32_t iHash;
	int iDecodedSize;
	int iRet;
	int i;

	pPixels = (unsigned char*)xrtMalloc((size_t)g_iXuiBuiltinAtlasRgbaSize);
	if ( pPixels == NULL ) {
		printf("xge_zstd_resource_test failed: allocation\n");
		return 1;
	}
	iDecodedSize = 0;
	iRet = xgeZstdDecompress(pPixels, g_iXuiBuiltinAtlasRgbaSize,
		g_arrXuiBuiltinAtlasRgbaZstd, g_iXuiBuiltinAtlasRgbaZstdSize, &iDecodedSize);
	if ( (iRet != XGE_OK) || (iDecodedSize != g_iXuiBuiltinAtlasRgbaSize) ) {
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
