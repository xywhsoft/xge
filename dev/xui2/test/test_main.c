#include "xge.h"
#include "xui.h"

#include <stdio.h>
#include <string.h>

static float __xgeSmokeTriSign(float fX, float fY, const xge_shape_vertex_t* pA, const xge_shape_vertex_t* pB)
{
	return ((fX - pB->fX) * (pA->fY - pB->fY)) - ((pA->fX - pB->fX) * (fY - pB->fY));
}

static int __xgeSmokeMeshContainsPoint(const xge_shape_vertex_t* pVertices, const uint32_t* pIndices, int iIndexCount, float fX, float fY)
{
	const xge_shape_vertex_t* pA;
	const xge_shape_vertex_t* pB;
	const xge_shape_vertex_t* pC;
	float fD1;
	float fD2;
	float fD3;
	int i;

	for ( i = 0; i + 2 < iIndexCount; i += 3 ) {
		pA = &pVertices[pIndices[i]];
		pB = &pVertices[pIndices[i + 1]];
		pC = &pVertices[pIndices[i + 2]];
		fD1 = __xgeSmokeTriSign(fX, fY, pA, pB);
		fD2 = __xgeSmokeTriSign(fX, fY, pB, pC);
		fD3 = __xgeSmokeTriSign(fX, fY, pC, pA);
		if ( !(((fD1 < -0.001f) || (fD2 < -0.001f) || (fD3 < -0.001f)) &&
		       ((fD1 > 0.001f) || (fD2 > 0.001f) || (fD3 > 0.001f))) ) {
			return 1;
		}
	}
	return 0;
}

int main(void)
{
	uint32_t iColor;
	xge_color_t tColor;
	xge_frame_stats_t tStats;
	xge_path pPath;
	xge_svg pSvg;
	xge_svg pCachedA;
	xge_svg pCachedB;
	xge_svg pCachedC;
	xge_texture_t tSvgTexture;
	xge_path_command_t tCommand;
	xge_svg_path_info_t tSvgInfo;
	xge_rect_t tViewBox;
	xge_rect_t tViewport;
	xge_shape_vertex_t arrVertices[256];
	uint32_t arrIndices[512];
	xge_vec2_t arrPoints[128];
	unsigned char arrSvgRasterPixels[64 * 64 * 4];
	unsigned char arrSvgTexturePixels[96 * 64 * 4];
	float arrDash[2];
	FILE* pFile;
	const char sSvgText[] =
		"<svg viewBox=\"0 0 32 32\" preserveAspectRatio=\"xMinYMax meet\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" fill=\"none\" stroke=\"#112233\" stroke-width=\"2\">"
		"<style>.cssFill{fill:#13579b;stroke:none}#idFill{fill:#2468ac;stroke:none}polygon{fill:#579b13}</style>"
		"<defs><linearGradient id=\"accentGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.1 0) scale(0.8 1)\">"
		"<stop offset=\"0%\" stop-color=\"#00aaff\"/>"
		"<stop offset=\"100%\" style=\"stop-color:#ff5522;stop-opacity:0.75\"/>"
		"</linearGradient></defs>"
		"<defs><radialGradient id=\"radialGlow\" cx=\"50%\" cy=\"50%\" r=\"50%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.9 0.9) translate(0.05 0.05)\">"
		"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
		"<stop offset=\"100%\" stop-color=\"#2d7dd2\" stop-opacity=\"0.8\"/>"
		"</radialGradient></defs>"
		"<defs><linearGradient id=\"sharedStops\"><stop offset=\"0%\" stop-color=\"#111111\"/><stop offset=\"100%\" stop-color=\"#eeeeee\"/></linearGradient>"
		"<linearGradient id=\"hrefGrad\" href=\"#sharedStops\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\"/>"
		"<radialGradient id=\"hrefRadial\" xlink:href=\"#sharedStops\" cx=\"50%\" cy=\"50%\" r=\"50%\"/></defs>"
		"<defs><clipPath id=\"cropClip\" transform=\"translate(1 0)\"><g transform=\"translate(-1 0)\"><rect x=\"24\" y=\"14\" width=\"5\" height=\"5\"/></g></clipPath></defs>"
		"<defs><clipPath id=\"objectCrop\" clipPathUnits=\"objectBoundingBox\"><g transform=\"translate(0.05 0)\"><rect x=\"0\" y=\"0\" width=\"0.55\" height=\"1\"/><rect x=\"0.35\" y=\"0\" width=\"0.55\" height=\"1\"/></g></clipPath></defs>"
		"<defs><mask id=\"softCrop\" maskContentUnits=\"objectBoundingBox\"><g transform=\"translate(0.05 0)\"><rect x=\"0\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#808080\"/><rect x=\"0.35\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#b0b0b0\"/></g></mask></defs>"
		"<defs><path id=\"reuseMark\" d=\"M0 0 L3 0 L1.5 3 Z\" fill=\"#123456\" stroke=\"none\"/></defs>"
		"<defs><g id=\"badgePair\"><rect x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#abcdef\" stroke=\"none\"/><circle cx=\"4\" cy=\"1\" r=\"1\" fill=\"#fedcba\" stroke=\"none\"/></g></defs>"
		"<g transform=\"translate(4 3)\" stroke-linecap=\"round\">"
		"<path d=\"M1 1 L5 1 L5 5 Z\" fill=\"#336699\" style=\"stroke-width:1.5;stroke-dasharray:2 1\"/>"
		"<path d=\"M10 10 Q12 14 14 10 T18 10 A3 3 0 0 1 21 13\" stroke=\"url(#accentGrad)\"/>"
		"<rect x=\"2\" y=\"20\" width=\"6\" height=\"4\" rx=\"1\" fill=\"#00ff00\"/>"
		"<circle cx=\"16\" cy=\"22\" r=\"3\" fill=\"#0000ff\"/>"
		"<ellipse cx=\"24\" cy=\"22\" rx=\"4\" ry=\"2\" fill=\"#ff00ff\"/>"
		"<line x1=\"2\" y1=\"29\" x2=\"8\" y2=\"29\" stroke=\"#000000\"/>"
		"<polyline points=\"10,29 14,25 18,29\" stroke=\"#000000\"/>"
		"<polygon points=\"20,29 24,25 28,29\" fill=\"#ffaa00\"/>"
		"<path d=\"M4 8 L12 8 L12 16 L4 16 Z M6 10 L10 10 L10 14 L6 14 Z\" fill=\"#8844ff\" fill-rule=\"evenodd\" stroke=\"none\"/>"
		"<rect x=\"22\" y=\"6\" width=\"6\" height=\"6\" fill=\"url(#accentGrad)\" stroke=\"none\" style=\"clip-path:url(#objectCrop);mask:url(#softCrop)\"/>"
		"<circle cx=\"26\" cy=\"16\" r=\"3\" fill=\"url(#radialGlow)\" stroke=\"none\" clip-path=\"url(#cropClip)\"/>"
		"<path d=\"M20 4 L25 1 L29 4\" stroke=\"url(#radialGlow)\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>"
		"<use href=\"#reuseMark\" x=\"24\" y=\"0\"/>"
		"<use href=\"#badgePair\" x=\"28\" y=\"8\"/>"
		"<use href=\"#latePair\" x=\"16\" y=\"0\"/>"
		"<use href=\"#lateSymbol\" x=\"0\" y=\"24\" width=\"4\" height=\"2\"/>"
		"<path class=\"cssFill\" d=\"M30 24 L32 24 L32 26 Z\"/>"
		"<rect x=\"28\" y=\"20\" width=\"2\" height=\"2\" fill=\"url(#hrefGrad)\" stroke=\"none\"/>"
		"<path d=\"M30 28 L32 28\" fill=\"none\" stroke=\"url(#hrefRadial)\" stroke-width=\"1\"/>"
		"<path id=\"idFill\" d=\"M28 14 L30 14 L30 16 Z\"/>"
		"<polygon points=\"22,14 24,14 24,16\"/>"
		"</g>"
		"<defs><g id=\"latePair\"><rect x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#0badf0\" stroke=\"none\"/><circle cx=\"4\" cy=\"1\" r=\"1\" fill=\"#0fdbad\" stroke=\"none\"/></g></defs>"
		"<defs><symbol id=\"lateSymbol\" viewBox=\"-1 -1 2 2\" preserveAspectRatio=\"none\"><rect x=\"-1\" y=\"-1\" width=\"2\" height=\"2\" fill=\"#cafe44\" stroke=\"none\"/></symbol></defs>"
		"</svg>";
	int iVertexCount;
	int iIndexCount;
	int iPointCount;
	int iRet;
	int iAlphaPixels;
	int i;

	pCachedA = NULL;
	pCachedB = NULL;
	pCachedC = NULL;
	memset(&tSvgTexture, 0, sizeof(tSvgTexture));

	if ( XUI_VERSION_MAJOR != 2 ) {
		printf("xge smoke failed: xui version got %d\n", XUI_VERSION_MAJOR);
		return 1;
	}

	iColor = xgeColorRGBA(1, 2, 3, 4);
	if ( iColor != 0x01020304u ) {
		printf("xge smoke failed: color pack got 0x%08x\n", iColor);
		return 1;
	}

	tColor = xgeColorUnpack(iColor);
	if ( (tColor.fR < 0.003f) || (tColor.fR > 0.005f) ||
	     (tColor.fG < 0.007f) || (tColor.fG > 0.009f) ||
	     (tColor.fB < 0.011f) || (tColor.fB > 0.013f) ||
	     (tColor.fA < 0.015f) || (tColor.fA > 0.017f) ) {
		printf("xge smoke failed: color unpack got %.4f,%.4f,%.4f,%.4f\n", tColor.fR, tColor.fG, tColor.fB, tColor.fA);
		return 1;
	}

	tStats = xgeFrameStatsGet();
	if ( tStats.iFrameCount != 0 ) {
		printf("xge smoke failed: uninitialized frame count got %d\n", tStats.iFrameCount);
		return 1;
	}

	pPath = NULL;
	iRet = xgePathCreate(&pPath);
	if ( (iRet != XGE_OK) || (pPath == NULL) ) {
		printf("xge smoke failed: path create got %d\n", iRet);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 L10 0 L10 10 L0 10 Z");
	if ( iRet != XGE_OK ) {
		printf("xge smoke failed: path parse square got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}
	if ( xgePathGetCommandCount(pPath) != 5 ) {
		printf("xge smoke failed: command count got %d\n", xgePathGetCommandCount(pPath));
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathGetCommand(pPath, 1, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_LINE) ||
	     (tCommand.arrPoints[0].fX != 10.0f) || (tCommand.arrPoints[0].fY != 0.0f) ) {
		printf("xge smoke failed: command read got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}
	iPointCount = xgePathFlatten(pPath, arrPoints, (int)(sizeof(arrPoints) / sizeof(arrPoints[0])), 1.0f);
	if ( iPointCount != 5 ) {
		printf("xge smoke failed: square flatten count got %d\n", iPointCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iVertexCount = 0;
	iIndexCount = 0;
	iRet = xgePathBuildFillMesh(pPath, NULL, 0, NULL, 0, 0xff0000ffu, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 4) || (iIndexCount != 6) ) {
		printf("xge smoke failed: fill query got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildFillMesh(pPath, arrVertices, 1, arrIndices, 1, 0xff0000ffu, 1.0f, &iVertexCount, &iIndexCount);
	if ( iRet != XGE_ERROR_BUFFER_TOO_SMALL ) {
		printf("xge smoke failed: fill small buffer got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildFillMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 0xff0000ffu, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 4) || (iIndexCount != 6) ||
	     (arrVertices[0].iColor != 0xff0000ffu) || (arrIndices[5] != 3u) ) {
		printf("xge smoke failed: fill mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildFillAAMesh(pPath, NULL, 0, NULL, 0, 0xff0000ffu, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 16) || (iIndexCount != 24) ) {
		printf("xge smoke failed: fill aa query got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildFillAAMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 0xff0000ffu, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 16) || (iIndexCount != 24) ||
	     (arrVertices[0].iColor != 0xff0000ffu) || (arrVertices[2].iColor != 0x00000000u) ||
	     (arrVertices[3].fY >= 0.0f) ) {
		printf("xge smoke failed: fill aa mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 L10 0 L10 10 L5 5 L0 10 Z");
	if ( iRet == XGE_OK ) {
		iRet = xgePathBuildFillMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 0xff00ffffu, 1.0f, &iVertexCount, &iIndexCount);
	}
	if ( (iRet != XGE_OK) || (iVertexCount < 8) || (iIndexCount < 12) ||
	     __xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 5.0f, 7.5f) ) {
		printf("xge smoke failed: concave fill mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 L10 0 L10 10 L0 10 Z M3 3 L7 3 L7 7 L3 7 Z");
	if ( iRet == XGE_OK ) {
		iRet = xgePathBuildFillMeshEx(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 0xabcdef12u, XGE_PATH_FILL_EVEN_ODD, 1.0f, &iVertexCount, &iIndexCount);
	}
	if ( (iRet != XGE_OK) || (iVertexCount < 12) || (iIndexCount < 18) ||
	     __xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 5.0f, 5.0f) ||
	     !__xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 1.5f, 5.0f) ) {
		printf("xge smoke failed: evenodd compound fill mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 L10 0 L10 10 L0 10 Z M3 3 L3 7 L7 7 L7 3 Z");
	if ( iRet == XGE_OK ) {
		iRet = xgePathBuildFillMeshEx(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 0x44556677u, XGE_PATH_FILL_NON_ZERO, 1.0f, &iVertexCount, &iIndexCount);
	}
	if ( (iRet != XGE_OK) || (iVertexCount < 12) || (iIndexCount < 18) ||
	     __xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 5.0f, 5.0f) ||
	     !__xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 8.5f, 5.0f) ) {
		printf("xge smoke failed: nonzero compound fill mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathBuildStrokeMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 2.0f, 0x00ff00ffu, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount <= 0) || (iIndexCount <= 0) ) {
		printf("xge smoke failed: stroke mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathParseSvg(pPath, "M0 0 L10 0");
	if ( iRet != XGE_OK ) {
		printf("xge smoke failed: stroke aa path parse got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildStrokeAAMesh(pPath, NULL, 0, NULL, 0, 2.0f, 0x00ff00ffu, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 16) || (iIndexCount != 24) ) {
		printf("xge smoke failed: stroke aa query got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildStrokeAAMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 2.0f, 0x00ff00ffu, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 16) || (iIndexCount != 24) ||
	     (arrVertices[0].iColor != 0x00ff00ffu) || (arrVertices[2].iColor != 0x00000000u) ||
	     (arrVertices[3].fY <= arrVertices[0].fY) ) {
		printf("xge smoke failed: stroke aa mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	arrDash[0] = 4.0f;
	arrDash[1] = 2.0f;
	iRet = xgePathBuildDashedStrokeAAMesh(pPath, NULL, 0, NULL, 0, 2.0f, 0x0000ffffu, arrDash, 2, 0.0f, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 32) || (iIndexCount != 48) ) {
		printf("xge smoke failed: dashed stroke aa query got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildDashedStrokeMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 2.0f, 0x0000ffffu, arrDash, 2, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount <= 0) || (iIndexCount <= 0) ) {
		printf("xge smoke failed: dashed stroke mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathParseSvg(pPath, "M0 0 L10 0 L10 10");
	if ( iRet != XGE_OK ) {
		printf("xge smoke failed: stroke aa join path parse got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildStrokeAAMesh(pPath, NULL, 0, NULL, 0, 2.0f, 0x00ff00ffu, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 32) || (iIndexCount != 48) ) {
		printf("xge smoke failed: stroke aa join query got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathBuildStrokeAAMesh(pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), 2.0f, 0x00ff00ffu, 1.0f, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount != 32) || (iIndexCount != 48) ||
	     (arrVertices[24].iColor != 0x00ff00ffu) || (arrVertices[26].iColor != 0x00000000u) ) {
		printf("xge smoke failed: stroke aa join mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 C10 0 10 10 20 10 Q25 15 30 10");
	iPointCount = xgePathFlatten(pPath, arrPoints, (int)(sizeof(arrPoints) / sizeof(arrPoints[0])), 1.0f);
	if ( (iRet != XGE_OK) || (iPointCount <= 8) ) {
		printf("xge smoke failed: curve parse/flatten got %d points=%d\n", iRet, iPointCount);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 C10 0 20 0 30 0 S50 0 60 0");
	if ( (iRet != XGE_OK) || (xgePathGetCommandCount(pPath) != 3) ) {
		printf("xge smoke failed: smooth cubic parse got %d count=%d\n", iRet, xgePathGetCommandCount(pPath));
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathGetCommand(pPath, 2, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_CUBIC) ||
	     (tCommand.arrPoints[0].fX != 40.0f) || (tCommand.arrPoints[0].fY != 0.0f) ||
	     (tCommand.arrPoints[1].fX != 50.0f) || (tCommand.arrPoints[2].fX != 60.0f) ) {
		printf("xge smoke failed: smooth cubic command got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 Q10 20 20 0 T40 0");
	if ( (iRet != XGE_OK) || (xgePathGetCommandCount(pPath) != 3) ) {
		printf("xge smoke failed: smooth quad parse got %d count=%d\n", iRet, xgePathGetCommandCount(pPath));
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathGetCommand(pPath, 2, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_QUAD) ||
	     (tCommand.arrPoints[0].fX != 30.0f) || (tCommand.arrPoints[0].fY != -20.0f) ||
	     (tCommand.arrPoints[1].fX != 40.0f) || (tCommand.arrPoints[1].fY != 0.0f) ) {
		printf("xge smoke failed: smooth quad command got %d\n", iRet);
		xgePathDestroy(pPath);
		return 1;
	}

	iRet = xgePathParseSvg(pPath, "M0 0 A10 10 0 0 1 10 10");
	iPointCount = xgePathFlatten(pPath, arrPoints, (int)(sizeof(arrPoints) / sizeof(arrPoints[0])), 1.0f);
	if ( (iRet != XGE_OK) || (xgePathGetCommandCount(pPath) < 2) || (iPointCount <= 8) ) {
		printf("xge smoke failed: arc parse/flatten got %d count=%d points=%d\n", iRet, xgePathGetCommandCount(pPath), iPointCount);
		xgePathDestroy(pPath);
		return 1;
	}
	iRet = xgePathGetCommand(pPath, xgePathGetCommandCount(pPath) - 1, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_CUBIC) ||
	     (tCommand.arrPoints[2].fX < 9.99f) || (tCommand.arrPoints[2].fX > 10.01f) ||
	     (tCommand.arrPoints[2].fY < 9.99f) || (tCommand.arrPoints[2].fY > 10.01f) ) {
		printf("xge smoke failed: arc command got %d end=%.3f,%.3f\n", iRet, tCommand.arrPoints[2].fX, tCommand.arrPoints[2].fY);
		xgePathDestroy(pPath);
		return 1;
	}
	xgePathDestroy(pPath);

	pSvg = NULL;
	iRet = xgeSvgCreate(&pSvg);
	if ( (iRet != XGE_OK) || (pSvg == NULL) ) {
		printf("xge smoke failed: svg create got %d\n", iRet);
		return 1;
	}
	iRet = xgeSvgLoadMemory(pSvg, sSvgText, (int)sizeof(sSvgText) - 1);
	if ( iRet != XGE_OK ) {
		printf("xge smoke failed: svg memory load got %d\n", iRet);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetViewBox(pSvg, &tViewBox);
	if ( (iRet != XGE_OK) || (tViewBox.fX != 0.0f) || (tViewBox.fY != 0.0f) || (tViewBox.fW != 32.0f) || (tViewBox.fH != 32.0f) ) {
		printf("xge smoke failed: svg viewbox got %d %.1f %.1f %.1f %.1f\n", iRet, tViewBox.fX, tViewBox.fY, tViewBox.fW, tViewBox.fH);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetDrawViewport(pSvg, (xge_rect_t){0.0f, 0.0f, 64.0f, 128.0f}, &tViewport);
	if ( (iRet != XGE_OK) || (tViewport.fX != 0.0f) || (tViewport.fY != 64.0f) || (tViewport.fW != 64.0f) || (tViewport.fH != 64.0f) ) {
		printf("xge smoke failed: svg aspect meet viewport got %d %.1f %.1f %.1f %.1f\n", iRet, tViewport.fX, tViewport.fY, tViewport.fW, tViewport.fH);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgSetPreserveAspectRatio(pSvg, "none");
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgGetDrawViewport(pSvg, (xge_rect_t){1.0f, 2.0f, 64.0f, 128.0f}, &tViewport);
	}
	if ( (iRet != XGE_OK) || (tViewport.fX != 1.0f) || (tViewport.fY != 2.0f) || (tViewport.fW != 64.0f) || (tViewport.fH != 128.0f) ) {
		printf("xge smoke failed: svg aspect none viewport got %d %.1f %.1f %.1f %.1f\n", iRet, tViewport.fX, tViewport.fY, tViewport.fW, tViewport.fH);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgSetPreserveAspectRatio(pSvg, "xMaxYMin slice");
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgGetDrawViewport(pSvg, (xge_rect_t){0.0f, 0.0f, 64.0f, 128.0f}, &tViewport);
	}
	if ( (iRet != XGE_OK) || (tViewport.fX != -64.0f) || (tViewport.fY != 0.0f) || (tViewport.fW != 128.0f) || (tViewport.fH != 128.0f) ) {
		printf("xge smoke failed: svg aspect slice viewport got %d %.1f %.1f %.1f %.1f\n", iRet, tViewport.fX, tViewport.fY, tViewport.fW, tViewport.fH);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	if ( xgeSvgGetPathCount(pSvg) != 23 ) {
		printf("xge smoke failed: svg path count got %d\n", xgeSvgGetPathCount(pSvg));
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 0, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x336699ffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x112233ffu) ||
	     (tSvgInfo.tStyle.fStrokeWidth < 1.49f) || (tSvgInfo.tStyle.fStrokeWidth > 1.51f) ||
	     (tSvgInfo.tStyle.iLineCap != XGE_PATH_CAP_ROUND) ||
	     (tSvgInfo.tStyle.iDashCount != 2) || (tSvgInfo.tStyle.pDashPattern == NULL) ||
	     (tSvgInfo.tStyle.pDashPattern[0] != 2.0f) || (tSvgInfo.tStyle.pDashPattern[1] != 1.0f) ) {
		printf("xge smoke failed: svg path info got %d\n", iRet);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 1, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ||
	     (tSvgInfo.sStrokeGradientId == NULL) ||
	     (strcmp(tSvgInfo.sStrokeGradientId, "accentGrad") != 0) ) {
		printf("xge smoke failed: svg stroke gradient info got %d id=%s\n", iRet, (tSvgInfo.sStrokeGradientId != NULL) ? tSvgInfo.sStrokeGradientId : "(null)");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 14.0f) || (tCommand.arrPoints[0].fY != 13.0f) ) {
		printf("xge smoke failed: svg transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 0, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ) {
		printf("xge smoke failed: svg fill path reload got %d\n", iRet);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathBuildFillMesh(tSvgInfo.pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), tSvgInfo.tStyle.iFillColor, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount < 3) || (iIndexCount < 3) ) {
		printf("xge smoke failed: svg fill mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 2, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x00ff00ffu) ||
	     (xgePathGetCommandCount(tSvgInfo.pPath) <= 5) ) {
		printf("xge smoke failed: svg rect info got %d count=%d\n", iRet, (tSvgInfo.pPath != NULL) ? xgePathGetCommandCount(tSvgInfo.pPath) : -1);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 7.0f) || (tCommand.arrPoints[0].fY != 23.0f) ) {
		printf("xge smoke failed: svg rect transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 5, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x000000ffu) ||
	     (xgePathGetCommandCount(tSvgInfo.pPath) != 2) ) {
		printf("xge smoke failed: svg line info got %d count=%d\n", iRet, (tSvgInfo.pPath != NULL) ? xgePathGetCommandCount(tSvgInfo.pPath) : -1);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 1, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_LINE) ||
	     (tCommand.arrPoints[0].fX != 12.0f) || (tCommand.arrPoints[0].fY != 32.0f) ) {
		printf("xge smoke failed: svg line transformed end got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 7, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0xffaa00ffu) ||
	     (xgePathGetCommandCount(tSvgInfo.pPath) != 4) ) {
		printf("xge smoke failed: svg polygon info got %d count=%d\n", iRet, (tSvgInfo.pPath != NULL) ? xgePathGetCommandCount(tSvgInfo.pPath) : -1);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 8, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x8844ffffu) ||
	     (tSvgInfo.tStyle.iFillRule != XGE_PATH_FILL_EVEN_ODD) ) {
		printf("xge smoke failed: svg evenodd info got %d\n", iRet);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathBuildFillMeshEx(tSvgInfo.pPath, arrVertices, (int)(sizeof(arrVertices) / sizeof(arrVertices[0])), arrIndices, (int)(sizeof(arrIndices) / sizeof(arrIndices[0])), tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iFillRule, 1.0f, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount < 12) || (iIndexCount < 18) ||
	     __xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 12.0f, 15.0f) ||
	     !__xgeSmokeMeshContainsPoint(arrVertices, arrIndices, iIndexCount, 9.0f, 15.0f) ) {
		printf("xge smoke failed: svg evenodd mesh got %d vc=%d ic=%d\n", iRet, iVertexCount, iIndexCount);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 9, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x00000000u) ||
	     (tSvgInfo.sFillGradientId == NULL) ||
	     (strcmp(tSvgInfo.sFillGradientId, "accentGrad") != 0) ||
	     (tSvgInfo.sClipPathId == NULL) ||
	     (strcmp(tSvgInfo.sClipPathId, "objectCrop") != 0) ||
	     (tSvgInfo.sMaskId == NULL) ||
	     (strcmp(tSvgInfo.sMaskId, "softCrop") != 0) ) {
		printf("xge smoke failed: svg gradient fill info got %d id=%s\n", iRet, (tSvgInfo.sFillGradientId != NULL) ? tSvgInfo.sFillGradientId : "(null)");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 10, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x00000000u) ||
	     (tSvgInfo.sFillGradientId == NULL) ||
	     (strcmp(tSvgInfo.sFillGradientId, "radialGlow") != 0) ||
	     (tSvgInfo.sClipPathId == NULL) ||
	     (strcmp(tSvgInfo.sClipPathId, "cropClip") != 0) ) {
		printf("xge smoke failed: svg radial gradient fill info got %d id=%s\n", iRet, (tSvgInfo.sFillGradientId != NULL) ? tSvgInfo.sFillGradientId : "(null)");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 11, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ||
	     (tSvgInfo.sStrokeGradientId == NULL) ||
	     (strcmp(tSvgInfo.sStrokeGradientId, "radialGlow") != 0) ) {
		printf("xge smoke failed: svg radial stroke gradient info got %d id=%s\n", iRet, (tSvgInfo.sStrokeGradientId != NULL) ? tSvgInfo.sStrokeGradientId : "(null)");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 12, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x123456ffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg use info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 28.0f) || (tCommand.arrPoints[0].fY != 3.0f) ) {
		printf("xge smoke failed: svg use transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 13, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0xabcdefffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg group use first info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 32.0f) || (tCommand.arrPoints[0].fY != 11.0f) ) {
		printf("xge smoke failed: svg group use transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 14, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0xfedcbaffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg group use second info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 15, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x0badf0ffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg forward use first info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 20.0f) || (tCommand.arrPoints[0].fY != 3.0f) ) {
		printf("xge smoke failed: svg forward use transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 16, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x0fdbadffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg forward use second info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 17, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0xcafe44ffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg forward symbol use info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 4.0f) || (tCommand.arrPoints[0].fY != 27.0f) ) {
		printf("xge smoke failed: svg forward symbol use transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 18, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x13579bffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg css class style info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 34.0f) || (tCommand.arrPoints[0].fY != 27.0f) ) {
		printf("xge smoke failed: svg css class transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 19, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x00000000u) ||
	     (tSvgInfo.sFillGradientId == NULL) ||
	     (strcmp(tSvgInfo.sFillGradientId, "hrefGrad") != 0) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg href linear gradient info got %d id=%s\n", iRet, (tSvgInfo.sFillGradientId != NULL) ? tSvgInfo.sFillGradientId : "(null)");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 32.0f) || (tCommand.arrPoints[0].fY != 23.0f) ) {
		printf("xge smoke failed: svg href linear transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 20, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ||
	     (tSvgInfo.sStrokeGradientId == NULL) ||
	     (strcmp(tSvgInfo.sStrokeGradientId, "hrefRadial") != 0) ||
	     (tSvgInfo.tStyle.fStrokeWidth < 0.99f) || (tSvgInfo.tStyle.fStrokeWidth > 1.01f) ) {
		printf("xge smoke failed: svg href radial gradient info got %d id=%s\n", iRet, (tSvgInfo.sStrokeGradientId != NULL) ? tSvgInfo.sStrokeGradientId : "(null)");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 21, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x2468acffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x00000000u) ) {
		printf("xge smoke failed: svg css id style info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 32.0f) || (tCommand.arrPoints[0].fY != 17.0f) ) {
		printf("xge smoke failed: svg css id transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgGetPathInfo(pSvg, 22, &tSvgInfo);
	if ( (iRet != XGE_OK) || (tSvgInfo.pPath == NULL) ||
	     (tSvgInfo.tStyle.iFillColor != 0x579b13ffu) ||
	     (tSvgInfo.tStyle.iStrokeColor != 0x112233ffu) ) {
		printf("xge smoke failed: svg css tag style info got %d fill=%08x stroke=%08x\n", iRet, tSvgInfo.tStyle.iFillColor, tSvgInfo.tStyle.iStrokeColor);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgePathGetCommand(tSvgInfo.pPath, 0, &tCommand);
	if ( (iRet != XGE_OK) || (tCommand.iCommand != XGE_PATH_CMD_MOVE) ||
	     (tCommand.arrPoints[0].fX != 26.0f) || (tCommand.arrPoints[0].fY != 17.0f) ) {
		printf("xge smoke failed: svg css tag transformed move got %d %.1f %.1f\n", iRet, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	pFile = fopen("build\\xge_svg_smoke.svg", "wb");
	if ( pFile == NULL ) {
		printf("xge smoke failed: svg file open\n");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	if ( fwrite(sSvgText, 1, sizeof(sSvgText) - 1, pFile) != sizeof(sSvgText) - 1 ) {
		fclose(pFile);
		printf("xge smoke failed: svg file write\n");
		xgeSvgDestroy(pSvg);
		return 1;
	}
	fclose(pFile);
	memset(arrSvgRasterPixels, 0xcc, sizeof(arrSvgRasterPixels));
	iRet = xgeSvgRasterizeMemory(sSvgText, (int)sizeof(sSvgText) - 1, 64, 64, arrSvgRasterPixels, 0);
	iAlphaPixels = 0;
	for ( i = 0; i < 64 * 64; i++ ) {
		if ( arrSvgRasterPixels[(i * 4) + 3] != 0 ) {
			iAlphaPixels++;
			if ( (arrSvgRasterPixels[(i * 4) + 0] > arrSvgRasterPixels[(i * 4) + 3]) ||
			     (arrSvgRasterPixels[(i * 4) + 1] > arrSvgRasterPixels[(i * 4) + 3]) ||
			     (arrSvgRasterPixels[(i * 4) + 2] > arrSvgRasterPixels[(i * 4) + 3]) ) {
				printf("xge smoke failed: svg raster alpha premultiply at %d\n", i);
				xgeSvgDestroy(pSvg);
				return 1;
			}
		}
	}
	if ( (iRet != XGE_OK) || (iAlphaPixels <= 0) ) {
		printf("xge smoke failed: svg memory raster got %d alpha=%d\n", iRet, iAlphaPixels);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgTextureLoadMemory(&tSvgTexture, sSvgText, (int)sizeof(sSvgText) - 1, 96, 64);
	if ( (iRet != XGE_OK) || (tSvgTexture.iWidth != 96) || (tSvgTexture.iHeight != 64) ) {
		printf("xge smoke failed: svg memory texture got %d size=%dx%d\n", iRet, tSvgTexture.iWidth, tSvgTexture.iHeight);
		xgeTextureFree(&tSvgTexture);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	memset(arrSvgTexturePixels, 0, sizeof(arrSvgTexturePixels));
	iRet = xgeTextureReadPixels(&tSvgTexture, arrSvgTexturePixels, 96 * 4);
	iAlphaPixels = 0;
	for ( i = 0; i < 96 * 64; i++ ) {
		if ( arrSvgTexturePixels[(i * 4) + 3] != 0 ) {
			iAlphaPixels++;
		}
	}
	if ( (iRet != XGE_OK) || (iAlphaPixels <= 0) ) {
		printf("xge smoke failed: svg texture read got %d alpha=%d\n", iRet, iAlphaPixels);
		xgeTextureFree(&tSvgTexture);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	xgeTextureFree(&tSvgTexture);
	memset(&tSvgTexture, 0, sizeof(tSvgTexture));
	iRet = xgeSvgTextureLoad(&tSvgTexture, "build\\xge_svg_smoke.svg", 0, 48);
	if ( (iRet != XGE_OK) || (tSvgTexture.iWidth != 48) || (tSvgTexture.iHeight != 48) ) {
		printf("xge smoke failed: svg file texture got %d size=%dx%d\n", iRet, tSvgTexture.iWidth, tSvgTexture.iHeight);
		xgeTextureFree(&tSvgTexture);
		xgeSvgDestroy(pSvg);
		return 1;
	}
	xgeTextureFree(&tSvgTexture);
	memset(&tSvgTexture, 0, sizeof(tSvgTexture));
	iRet = xgeSvgLoad(pSvg, "build\\xge_svg_smoke.svg");
	if ( (iRet != XGE_OK) || (xgeSvgGetPathCount(pSvg) != 23) ) {
		printf("xge smoke failed: svg file load got %d count=%d\n", iRet, xgeSvgGetPathCount(pSvg));
		xgeSvgDestroy(pSvg);
		return 1;
	}
	iRet = xgeSvgLoadCached("build\\xge_svg_smoke.svg", &pCachedA);
	if ( (iRet != XGE_OK) || (pCachedA == NULL) || (xgeSvgGetPathCount(pCachedA) != 23) ) {
		printf("xge smoke failed: svg cache first load got %d count=%d\n", iRet, xgeSvgGetPathCount(pCachedA));
		xgeSvgDestroy(pSvg);
		xgeSvgDestroy(pCachedA);
		return 1;
	}
	iRet = xgeSvgLoadCached("build\\xge_svg_smoke.svg", &pCachedB);
	if ( (iRet != XGE_OK) || (pCachedB == NULL) || (pCachedA != pCachedB) ) {
		printf("xge smoke failed: svg cache hit got %d same=%d\n", iRet, pCachedA == pCachedB);
		xgeSvgDestroy(pSvg);
		xgeSvgDestroy(pCachedA);
		xgeSvgDestroy(pCachedB);
		xgeSvgCacheClear();
		return 1;
	}
	xgeSvgDestroy(pCachedA);
	pCachedA = NULL;
	xgeSvgDestroy(pCachedB);
	pCachedB = NULL;
	iRet = xgeSvgCacheInvalidate("build\\xge_svg_smoke.svg");
	if ( iRet != XGE_OK ) {
		printf("xge smoke failed: svg cache invalidate got %d\n", iRet);
		xgeSvgDestroy(pSvg);
		xgeSvgCacheClear();
		return 1;
	}
	iRet = xgeSvgLoadCached("build\\xge_svg_smoke.svg", &pCachedC);
	if ( (iRet != XGE_OK) || (pCachedC == NULL) || (xgeSvgGetPathCount(pCachedC) != 23) ) {
		printf("xge smoke failed: svg cache reload got %d count=%d\n", iRet, xgeSvgGetPathCount(pCachedC));
		xgeSvgDestroy(pSvg);
		xgeSvgDestroy(pCachedC);
		xgeSvgCacheClear();
		return 1;
	}
	xgeSvgDestroy(pCachedC);
	pCachedC = NULL;
	xgeSvgCacheClear();
	xgeSvgDestroy(pSvg);

	printf("xge smoke passed\n");
	return 0;
}
