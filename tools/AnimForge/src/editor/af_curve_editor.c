/*
 * af_curve_editor.c - Curve editor panel (Canvas-based bezier curve view)
 *
 * Displays and edits animation curves for selected elements.
 * Shows value-over-time graphs with draggable keyframe points.
 */

#include "af_curve_editor.h"
#include "af_draw_helper.h"
#include "../core/af_app.h"
#include "../anim/af_easing.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Constants                                                          */
/* ------------------------------------------------------------------ */

#define AF_CURVE_MARGIN_LEFT    50
#define AF_CURVE_MARGIN_RIGHT   20
#define AF_CURVE_MARGIN_TOP     30
#define AF_CURVE_MARGIN_BOTTOM  30
#define AF_CURVE_HANDLE_SIZE    6.0f

/* Channel colors */
static const uint32_t s_arrChannelColors[AF_CHANNEL_COUNT] = {
	XUI_COLOR_RGBA(255, 80, 80, 255),    /* X - red */
	XUI_COLOR_RGBA(80, 255, 80, 255),    /* Y - green */
	XUI_COLOR_RGBA(80, 80, 255, 255),    /* ScaleX - blue */
	XUI_COLOR_RGBA(255, 255, 80, 255),   /* ScaleY - yellow */
	XUI_COLOR_RGBA(255, 80, 255, 255),   /* Rotation - magenta */
	XUI_COLOR_RGBA(80, 255, 255, 255),   /* Opacity - cyan */
	XUI_COLOR_RGBA(255, 160, 80, 255),   /* SkewX - orange */
	XUI_COLOR_RGBA(160, 80, 255, 255),   /* SkewY - purple */
	XUI_COLOR_RGBA(200, 200, 200, 255),  /* PivotX - gray */
	XUI_COLOR_RGBA(150, 150, 150, 255),  /* PivotY - dark gray */
	XUI_COLOR_RGBA(255, 100, 100, 255),  /* ColorR */
	XUI_COLOR_RGBA(100, 255, 100, 255),  /* ColorG */
	XUI_COLOR_RGBA(100, 100, 255, 255),  /* ColorB */
	XUI_COLOR_RGBA(200, 200, 200, 255),  /* ColorA */
};

static const char* s_arrChannelNames[AF_CHANNEL_COUNT] = {
	"X", "Y", "ScaleX", "ScaleY", "Rotation", "Opacity",
	"SkewX", "SkewY", "PivotX", "PivotY",
	"ColorR", "ColorG", "ColorB", "ColorA"
};

/* ------------------------------------------------------------------ */
/* State                                                              */
/* ------------------------------------------------------------------ */

typedef struct af_curve_editor_state_t {
	int      bVisible;
	uint32_t iChannelMask;      /* visible channels */
	float    fFrameStart;       /* view range */
	float    fFrameEnd;
	float    fValueMin;
	float    fValueMax;
	int      iDragKey;          /* keyframe being dragged, -1 none */
	int      iDragChannel;
	int      bDragging;
	float    fLastMouseX;
	float    fLastMouseY;
} af_curve_editor_state_t;

static af_curve_editor_state_t s_tState;

/* ------------------------------------------------------------------ */
/* Init / Shutdown                                                    */
/* ------------------------------------------------------------------ */

void afCurveEditorInit(af_curve_editor_t* pEditor)
{
	memset(pEditor, 0, sizeof(*pEditor));
	memset(&s_tState, 0, sizeof(s_tState));
	s_tState.iDragKey = -1;
	s_tState.iDragChannel = -1;
	s_tState.iChannelMask = 0x003F; /* first 6 channels visible by default */
	s_tState.fValueMin = -100.0f;
	s_tState.fValueMax = 100.0f;
}

void afCurveEditorShutdown(af_curve_editor_t* pEditor)
{
	(void)pEditor;
}

/* ------------------------------------------------------------------ */
/* Coordinate transforms                                              */
/* ------------------------------------------------------------------ */

static float __frameToX(float fFrame, float fCanvasW)
{
	float plotW = fCanvasW - AF_CURVE_MARGIN_LEFT - AF_CURVE_MARGIN_RIGHT;
	if ( s_tState.fFrameEnd <= s_tState.fFrameStart ) return AF_CURVE_MARGIN_LEFT;
	return AF_CURVE_MARGIN_LEFT + (fFrame - s_tState.fFrameStart) /
	       (s_tState.fFrameEnd - s_tState.fFrameStart) * plotW;
}

static float __valueToY(float fValue, float fCanvasH)
{
	float plotH = fCanvasH - AF_CURVE_MARGIN_TOP - AF_CURVE_MARGIN_BOTTOM;
	float range = s_tState.fValueMax - s_tState.fValueMin;
	if ( range <= 0.0f ) return AF_CURVE_MARGIN_TOP;
	return AF_CURVE_MARGIN_TOP + (1.0f - (fValue - s_tState.fValueMin) / range) * plotH;
}

static float __xToFrame(float fX, float fCanvasW)
{
	float plotW = fCanvasW - AF_CURVE_MARGIN_LEFT - AF_CURVE_MARGIN_RIGHT;
	if ( plotW <= 0.0f ) return s_tState.fFrameStart;
	return s_tState.fFrameStart + (fX - AF_CURVE_MARGIN_LEFT) / plotW *
	       (s_tState.fFrameEnd - s_tState.fFrameStart);
}

static float __yToValue(float fY, float fCanvasH)
{
	float plotH = fCanvasH - AF_CURVE_MARGIN_TOP - AF_CURVE_MARGIN_BOTTOM;
	float range = s_tState.fValueMax - s_tState.fValueMin;
	if ( plotH <= 0.0f ) return 0.0f;
	return s_tState.fValueMin + (1.0f - (fY - AF_CURVE_MARGIN_TOP) / plotH) * range;
}

/* ------------------------------------------------------------------ */
/* Rendering                                                          */
/* ------------------------------------------------------------------ */

void afCurveEditorRender(af_curve_editor_t* pEditor, xui_proxy_t* pProxy, xui_draw_context pCtx,
                         float fW, float fH, af_doc pDoc, uint32_t iCurveId)
{
	af_curve_t* pCurve;
	int ch, ki;
	uint32_t bgColor = XUI_COLOR_RGBA(30, 30, 32, 255);
	uint32_t gridColor = XUI_COLOR_RGBA(60, 60, 64, 255);

	/* Background */
	afDrawFillRect(pProxy, pCtx, 0, 0, fW, fH, bgColor);

	pCurve = afDocFindCurve(pDoc, iCurveId);
	if ( pCurve == NULL ) {
		return;
	}

	/* Update frame range from curve */
	if ( pCurve->iKeyCount > 0 ) {
		s_tState.fFrameStart = (float)pCurve->arrKeys[0].iFrame;
		s_tState.fFrameEnd = (float)pCurve->arrKeys[pCurve->iKeyCount - 1].iFrame;
		if ( s_tState.fFrameEnd <= s_tState.fFrameStart )
			s_tState.fFrameEnd = s_tState.fFrameStart + 10.0f;
	}

	/* Horizontal grid lines (value) */
	{
		float step = (s_tState.fValueMax - s_tState.fValueMin) / 4.0f;
		float v;
		for ( v = s_tState.fValueMin; v <= s_tState.fValueMax; v += step ) {
			float y = __valueToY(v, fH);
			afDrawLine(pProxy, pCtx, AF_CURVE_MARGIN_LEFT, y, fW - AF_CURVE_MARGIN_RIGHT, y, 1.0f, gridColor);
		}
	}

	/* Vertical grid lines (frames) */
	{
		float frameStep = (s_tState.fFrameEnd - s_tState.fFrameStart) / 8.0f;
		float f;
		if ( frameStep < 1.0f ) frameStep = 1.0f;
		for ( f = s_tState.fFrameStart; f <= s_tState.fFrameEnd; f += frameStep ) {
			float x = __frameToX(f, fW);
			afDrawLine(pProxy, pCtx, x, AF_CURVE_MARGIN_TOP, x, fH - AF_CURVE_MARGIN_BOTTOM, 1.0f, gridColor);
		}
	}

	/* Draw curves for each visible channel */
	for ( ch = 0; ch < AF_CHANNEL_COUNT; ch++ ) {
		if ( !(s_tState.iChannelMask & (1u << ch)) )
			continue;
		if ( !(pCurve->iChannelMask & (1u << ch)) )
			continue;

		/* Draw curve segments */
		for ( ki = 0; ki < pCurve->iKeyCount - 1; ki++ ) {
			af_curve_key_t* pK0 = &pCurve->arrKeys[ki];
			af_curve_key_t* pK1 = &pCurve->arrKeys[ki + 1];
			float v0 = pK0->arrValues[ch];
			float v1 = pK1->arrValues[ch];
			float f0 = (float)pK0->iFrame;
			float f1 = (float)pK1->iFrame;
			int steps = 20;
			int si;
			float prevX = __frameToX(f0, fW);
			float prevY = __valueToY(v0, fH);

			for ( si = 1; si <= steps; si++ ) {
				float t = (float)si / (float)steps;
				float easedT = afEaseEval(t, (int)pK0->iInterpType,
				                          pK0->fEaseIn, pK0->fEaseOut, pK0->arrBezier);
				float val = v0 + (v1 - v0) * easedT;
				float frame = f0 + (f1 - f0) * t;
				float x = __frameToX(frame, fW);
				float y = __valueToY(val, fH);
				afDrawLine(pProxy, pCtx, prevX, prevY, x, y, 2.0f, s_arrChannelColors[ch]);
				prevX = x;
				prevY = y;
			}
		}

		/* Draw keyframe points */
		for ( ki = 0; ki < pCurve->iKeyCount; ki++ ) {
			float x = __frameToX((float)pCurve->arrKeys[ki].iFrame, fW);
			float y = __valueToY(pCurve->arrKeys[ki].arrValues[ch], fH);
			afDrawCircleFill(pProxy, pCtx, x, y, 4.0f, s_arrChannelColors[ch]);
		}
	}

	/* Draw channel legend */
	{
		int legendY = AF_CURVE_MARGIN_TOP;
		for ( ch = 0; ch < AF_CHANNEL_COUNT; ch++ ) {
			if ( !(s_tState.iChannelMask & (1u << ch)) )
				continue;
			if ( !(pCurve->iChannelMask & (1u << ch)) )
				continue;

			afDrawFillRect(pProxy, pCtx, AF_CURVE_MARGIN_LEFT + 5, (float)legendY, 10, 10, s_arrChannelColors[ch]);
			legendY += 14;
		}
	}
}

/* ------------------------------------------------------------------ */
/* Input handling                                                     */
/* ------------------------------------------------------------------ */

int afCurveEditorMouseDown(af_curve_editor_t* pEditor, float fX, float fY,
                           float fW, float fH, af_doc pDoc, uint32_t iCurveId)
{
	af_curve_t* pCurve = afDocFindCurve(pDoc, iCurveId);
	int ch, ki;

	if ( pCurve == NULL )
		return 0;

	/* Hit test keyframe points */
	for ( ch = 0; ch < AF_CHANNEL_COUNT; ch++ ) {
		if ( !(s_tState.iChannelMask & (1u << ch)) )
			continue;
		if ( !(pCurve->iChannelMask & (1u << ch)) )
			continue;

		for ( ki = 0; ki < pCurve->iKeyCount; ki++ ) {
			float x = __frameToX((float)pCurve->arrKeys[ki].iFrame, fW);
			float y = __valueToY(pCurve->arrKeys[ki].arrValues[ch], fH);
			float dx = fX - x;
			float dy = fY - y;
			if ( dx*dx + dy*dy < AF_CURVE_HANDLE_SIZE * AF_CURVE_HANDLE_SIZE * 4.0f ) {
				s_tState.iDragKey = ki;
				s_tState.iDragChannel = ch;
				s_tState.bDragging = 1;
				return 1;
			}
		}
	}

	return 0;
}

int afCurveEditorMouseMove(af_curve_editor_t* pEditor, float fX, float fY,
                           float fW, float fH, af_doc pDoc, uint32_t iCurveId)
{
	af_curve_t* pCurve;

	if ( !s_tState.bDragging || s_tState.iDragKey < 0 )
		return 0;

	pCurve = afDocFindCurve(pDoc, iCurveId);
	if ( pCurve == NULL )
		return 0;

	/* Update keyframe value */
	{
		float newValue = __yToValue(fY, fH);
		int ch = s_tState.iDragChannel;
		int ki = s_tState.iDragKey;

		if ( ki >= 0 && ki < pCurve->iKeyCount && ch >= 0 && ch < AF_CHANNEL_COUNT ) {
			pCurve->arrKeys[ki].arrValues[ch] = newValue;
			pDoc->bModified = 1;
		}
	}

	return 1;
}

int afCurveEditorMouseUp(af_curve_editor_t* pEditor, float fX, float fY)
{
	s_tState.bDragging = 0;
	s_tState.iDragKey = -1;
	s_tState.iDragChannel = -1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Channel visibility toggle                                          */
/* ------------------------------------------------------------------ */

void afCurveEditorToggleChannel(af_curve_editor_t* pEditor, int iChannel)
{
	if ( iChannel < 0 || iChannel >= AF_CHANNEL_COUNT )
		return;
	s_tState.iChannelMask ^= (1u << iChannel);
}

void afCurveEditorSetChannelMask(af_curve_editor_t* pEditor, uint32_t iMask)
{
	s_tState.iChannelMask = iMask;
}

void afCurveEditorSetValueRange(af_curve_editor_t* pEditor, float fMin, float fMax)
{
	s_tState.fValueMin = fMin;
	s_tState.fValueMax = fMax;
}
