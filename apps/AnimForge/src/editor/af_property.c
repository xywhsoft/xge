/*
 * af_property.c - Property panel binding
 *
 * Displays and edits properties of the selected element:
 * position, scale, rotation, skew, opacity, color transform,
 * blend mode, and tween parameters.
 */

#include "af_property.h"
#include "af_draw_helper.h"
#include "../core/af_app.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* State                                                              */
/* ------------------------------------------------------------------ */

typedef struct af_property_state_t {
	uint32_t iTargetInstance;   /* instance being inspected */
	int      iSection;          /* expanded section */
	int      iEditField;        /* field being edited, -1 none */
	char     sEditBuffer[64];
} af_property_state_t;

static af_property_state_t s_tPropState;

/* Property sections */
#define AF_PROP_SECTION_TRANSFORM  0
#define AF_PROP_SECTION_COLOR      1
#define AF_PROP_SECTION_TWEEN      2
#define AF_PROP_SECTION_BLEND      3
#define AF_PROP_SECTION_COUNT      4

/* ------------------------------------------------------------------ */
/* Init / Shutdown                                                    */
/* ------------------------------------------------------------------ */

void afPropertyInit(af_property_t* pProp)
{
	memset(pProp, 0, sizeof(*pProp));
	memset(&s_tPropState, 0, sizeof(s_tPropState));
	s_tPropState.iEditField = -1;
}

void afPropertyShutdown(af_property_t* pProp)
{
	(void)pProp;
}

/* ------------------------------------------------------------------ */
/* Rendering                                                          */
/* ------------------------------------------------------------------ */

static void __afDrawPropRow(xui_proxy_t* pProxy, xui_draw_context pCtx, float fX, float fY, float fW,
                            const char* sLabel, float fValue, int bSelected)
{
	uint32_t labelColor = AF_COLOR_TEXT_DIM;
	uint32_t valueColor = AF_COLOR_TEXT;

	/* Label */
	afDrawFillRect(pProxy, pCtx, fX, fY + 4, (float)(strlen(sLabel) * 6), 8, labelColor);

	/* Value field */
	{
		float fieldX = fX + 80.0f;
		float fieldW = fW - 90.0f;
		uint32_t fieldBg = bSelected ? AF_COLOR_ACCENT : XUI_COLOR_RGBA(60, 60, 64, 255);
		afDrawFillRect(pProxy, pCtx, fieldX, fY, fieldW, 18, fieldBg);

		/* Value bar visualization */
		{
			float barW = fieldW * 0.5f;
			if ( barW > fieldW - 4 ) barW = fieldW - 4;
			if ( barW < 2 ) barW = 2;
			afDrawFillRect(pProxy, pCtx, fieldX + 2, fY + 6, barW, 6, valueColor);
		}
	}
}

void afPropertyRender(af_property_t* pProp, xui_proxy_t* pProxy, xui_draw_context pCtx,
                      float fW, float fH, af_doc pDoc, uint32_t iInstanceId)
{
	af_element_t* pElem;
	float y = 8.0f;
	float rowH = 22.0f;
	uint32_t bgColor = AF_COLOR_PANEL;
	uint32_t headerColor = AF_COLOR_HIGHLIGHT;

	/* Background */
	afDrawFillRect(pProxy, pCtx, 0, 0, fW, fH, bgColor);

	pElem = afDocFindElement(pDoc, iInstanceId);
	if ( pElem == NULL ) {
		afDrawFillRect(pProxy, pCtx, 10, 20, 80, 8, AF_COLOR_TEXT_DIM);
		return;
	}

	/* Transform section header */
	afDrawFillRect(pProxy, pCtx, 0, y, fW, 20, headerColor);
	y += 24;

	/* Position */
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "X", pElem->fTx, 0); y += rowH;
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "Y", pElem->fTy, 0); y += rowH;

	/* Scale */
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "ScaleX", pElem->fScaleX, 0); y += rowH;
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "ScaleY", pElem->fScaleY, 0); y += rowH;

	/* Rotation */
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "Rotation", pElem->fRotation * 180.0f / 3.14159265f, 0); y += rowH;

	/* Skew */
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "SkewX", pElem->fSkewX, 0); y += rowH;
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "SkewY", pElem->fSkewY, 0); y += rowH;

	/* Pivot */
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "PivotX", pElem->fPivotX, 0); y += rowH;
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "PivotY", pElem->fPivotY, 0); y += rowH;

	/* Opacity section */
	y += 8;
	afDrawFillRect(pProxy, pCtx, 0, y, fW, 20, headerColor);
	y += 24;

	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "Opacity", pElem->fOpacity, 0); y += rowH;

	/* Color transform */
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "ColorT", (float)pElem->iColorTransform, 0); y += rowH;

	/* Tween section */
	y += 8;
	afDrawFillRect(pProxy, pCtx, 0, y, fW, 20, headerColor);
	y += 24;

	if ( pElem->iTweenId != 0xFFFFFFFFu ) {
		af_curve_t* pCurve = afDocFindCurve(pDoc, pElem->iTweenId);
		if ( pCurve ) {
			__afDrawPropRow(pProxy, pCtx, 8, y, fW, "Keys", (float)pCurve->iKeyCount, 0); y += rowH;
			__afDrawPropRow(pProxy, pCtx, 8, y, fW, "Channels", (float)pCurve->iChannelMask, 0); y += rowH;
		}
	} else {
		afDrawFillRect(pProxy, pCtx, 8, y + 4, 60, 8, AF_COLOR_TEXT_DIM);
		y += rowH;
	}

	/* Blend mode */
	y += 8;
	afDrawFillRect(pProxy, pCtx, 0, y, fW, 20, headerColor);
	y += 24;
	__afDrawPropRow(pProxy, pCtx, 8, y, fW, "Blend", (float)pElem->iBlendMode, 0); y += rowH;

	(void)fH;
}

/* ------------------------------------------------------------------ */
/* Input                                                              */
/* ------------------------------------------------------------------ */

int afPropertyMouseDown(af_property_t* pProp, float fX, float fY,
                        float fW, float fH, af_doc pDoc, uint32_t iInstanceId)
{
	af_element_t* pElem = afDocFindElement(pDoc, iInstanceId);
	float rowH = 22.0f;
	float y = 32.0f; /* after first header */
	int row;

	if ( pElem == NULL )
		return 0;

	/* Determine which row was clicked */
	row = (int)((fY - y) / rowH);

	/* Check if in value field area */
	if ( fX > 88.0f ) {
		s_tPropState.iEditField = row;
		s_tPropState.iTargetInstance = iInstanceId;
		return 1;
	}

	s_tPropState.iEditField = -1;
	(void)fW; (void)fH;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Property setters (called from UI editing)                          */
/* ------------------------------------------------------------------ */

int afPropertySetValue(af_doc pDoc, uint32_t iInstanceId, int iField, float fValue)
{
	af_element_t* pElem = afDocFindElement(pDoc, iInstanceId);
	if ( pElem == NULL )
		return -1;

	switch ( iField ) {
	case 0: pElem->fTx = fValue; break;
	case 1: pElem->fTy = fValue; break;
	case 2: pElem->fScaleX = fValue; break;
	case 3: pElem->fScaleY = fValue; break;
	case 4: pElem->fRotation = fValue * 3.14159265f / 180.0f; break;
	case 5: pElem->fSkewX = fValue; break;
	case 6: pElem->fSkewY = fValue; break;
	case 7: pElem->fPivotX = fValue; break;
	case 8: pElem->fPivotY = fValue; break;
	case 9: pElem->fOpacity = fValue; break;
	default: return -2;
	}

	pDoc->bModified = 1;
	return 0;
}

float afPropertyGetValue(af_doc pDoc, uint32_t iInstanceId, int iField)
{
	af_element_t* pElem = afDocFindElement(pDoc, iInstanceId);
	if ( pElem == NULL )
		return 0.0f;

	switch ( iField ) {
	case 0: return pElem->fTx;
	case 1: return pElem->fTy;
	case 2: return pElem->fScaleX;
	case 3: return pElem->fScaleY;
	case 4: return pElem->fRotation * 180.0f / 3.14159265f;
	case 5: return pElem->fSkewX;
	case 6: return pElem->fSkewY;
	case 7: return pElem->fPivotX;
	case 8: return pElem->fPivotY;
	case 9: return pElem->fOpacity;
	default: return 0.0f;
	}
}
