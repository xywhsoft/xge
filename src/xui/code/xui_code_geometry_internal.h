#ifndef XUI_CODE_GEOMETRY_INTERNAL_H
#define XUI_CODE_GEOMETRY_INTERNAL_H

#include "xui.h"

#include <limits.h>
#include <math.h>

static int xuiCodeInternalRectValid(xui_rect_t tRect)
{
	if ( tRect.fW < 0 || tRect.fH < 0 ) return 0;
	if ( tRect.fX > INT_MAX - tRect.fW ) return 0;
	if ( tRect.fY > INT_MAX - tRect.fH ) return 0;
	return 1;
}

static int xuiCodeInternalSnapPixel(double fValue, int* pPixel)
{
	double fPixel;

	if ( pPixel == NULL || !isfinite(fValue) ) return XUI_ERROR_INVALID_ARGUMENT;
	fPixel = floor(fValue + 0.5);
	if ( fPixel < (double)INT_MIN || fPixel > (double)INT_MAX ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	*pPixel = (int)fPixel;
	return XUI_OK;
}

static int xuiCodeInternalRectFromDouble(double fX, double fY,
	double fWidth, double fHeight, xui_rect_t* pRect)
{
	double fRight;
	double fBottom;
	long long iWidth;
	long long iHeight;
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;
	int iRet;

	if ( pRect == NULL || !isfinite(fX) || !isfinite(fY) ||
	     !isfinite(fWidth) || !isfinite(fHeight) ||
	     fWidth < 0.0 || fHeight < 0.0 ) return XUI_ERROR_INVALID_ARGUMENT;
	fRight = fX + fWidth;
	fBottom = fY + fHeight;
	if ( !isfinite(fRight) || !isfinite(fBottom) ) return XUI_ERROR_LIMIT_EXCEEDED;
	iRet = xuiCodeInternalSnapPixel(fX, &iLeft);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeInternalSnapPixel(fY, &iTop);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeInternalSnapPixel(fRight, &iRight);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeInternalSnapPixel(fBottom, &iBottom);
	if ( iRet != XUI_OK ) return iRet;
	iWidth = (long long)iRight - (long long)iLeft;
	iHeight = (long long)iBottom - (long long)iTop;
	if ( iWidth < 0 || iWidth > INT_MAX || iHeight < 0 || iHeight > INT_MAX ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	*pRect = (xui_rect_t){ iLeft, iTop, (int)iWidth, (int)iHeight };
	return XUI_OK;
}

#endif
