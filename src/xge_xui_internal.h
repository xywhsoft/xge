#ifndef XGE_XUI_INTERNAL_H
#define XGE_XUI_INTERNAL_H

/*
 * Private XUI implementation header.
 *
 * XGE currently builds as a single translation unit, so private helpers with
 * internal linkage are shared by include order. This header marks the internal
 * boundary and is the home for small shared private definitions as XUI controls
 * are split into per-control source files.
 */

#define XGE_XUI_INTERNAL 1

static int __xgeXuiRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < (tRect.fX + tRect.fW)) && (fY < (tRect.fY + tRect.fH));
}

static float __xgeXuiClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

#endif
