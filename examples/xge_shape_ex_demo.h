#ifndef XGE_SHAPE_EX_DEMO_H
#define XGE_SHAPE_EX_DEMO_H

#include "../xge.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int xgeShapeExDemoPositiveInt(const char* sText, int* pValue)
{
	char* sEnd;
	long iValue;

	if ( sText == NULL || pValue == NULL || sText[0] == 0 ) return 0;
	sEnd = NULL;
	iValue = strtol(sText, &sEnd, 10);
	if ( sEnd == sText || *sEnd != 0 || iValue <= 0 || iValue > INT_MAX ) return 0;
	*pValue = (int)iValue;
	return 1;
}

static void xgeShapeExDemoUsage(const char* sName, int bRepeat)
{
	printf("usage: %s [--frames N] [--capture PATH]%s\n", sName,
		bRepeat ? " [--repeat N]" : "");
}

static int xgeShapeExDemoParseArgs(const char* sName, int argc, char** argv,
	char* sCapturePath, size_t iCaptureCapacity, int* pMaxFrames, int* pRepeat)
{
	int i;

	if ( sName == NULL || argv == NULL || sCapturePath == NULL || iCaptureCapacity == 0 || pMaxFrames == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( ++i >= argc || !xgeShapeExDemoPositiveInt(argv[i], pMaxFrames) ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			if ( !xgeShapeExDemoPositiveInt(argv[i] + 9, pMaxFrames) ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( ++i >= argc || argv[i][0] == 0 ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(sCapturePath, iCaptureCapacity, "%s", argv[i]);
			sCapturePath[iCaptureCapacity - 1] = 0;
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			if ( argv[i][10] == 0 ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(sCapturePath, iCaptureCapacity, "%s", argv[i] + 10);
			sCapturePath[iCaptureCapacity - 1] = 0;
		} else if ( strcmp(argv[i], "--repeat") == 0 ) {
			if ( pRepeat == NULL || ++i >= argc || !xgeShapeExDemoPositiveInt(argv[i], pRepeat) ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--repeat=", 9) == 0 ) {
			if ( pRepeat == NULL || !xgeShapeExDemoPositiveInt(argv[i] + 9, pRepeat) ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			xgeShapeExDemoUsage(sName, pRepeat != NULL);
			return 1;
		} else if ( argv[i][0] != '-' && sCapturePath[0] == 0 ) {
			/* Preserve the original one-positional-argument capture invocation. */
			snprintf(sCapturePath, iCaptureCapacity, "%s", argv[i]);
			sCapturePath[iCaptureCapacity - 1] = 0;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static void xgeShapeExDemoPresent(xge_render_target pTarget, int iWidth, int iHeight,
	uint32_t iBackgroundColor)
{
	xge_draw_t tDraw;

	xgeClear(iBackgroundColor);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = xgeRenderTargetTexture(pTarget);
	tDraw.tSrc = (xge_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight};
	tDraw.tDst = (xge_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight};
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

static int xgeShapeExDemoShouldQuit(int bCaptureDone, int iFrame, int iMaxFrames)
{
	return bCaptureDone || (iMaxFrames > 0 && iFrame >= iMaxFrames);
}

#endif
