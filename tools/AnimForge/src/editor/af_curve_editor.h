/*
 * af_curve_editor.h - Curve editor panel
 */

#ifndef AF_CURVE_EDITOR_H
#define AF_CURVE_EDITOR_H

#include "xui.h"
#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct af_curve_editor_t {
	xui_widget pCanvas;
	int        bVisible;
	uint32_t   iActiveCurveId;
} af_curve_editor_t;

void afCurveEditorInit(af_curve_editor_t* pEditor);
void afCurveEditorShutdown(af_curve_editor_t* pEditor);

void afCurveEditorRender(af_curve_editor_t* pEditor, xui_proxy_t* pProxy, xui_draw_context pCtx,
                         float fW, float fH, af_doc pDoc, uint32_t iCurveId);

int afCurveEditorMouseDown(af_curve_editor_t* pEditor, float fX, float fY,
                           float fW, float fH, af_doc pDoc, uint32_t iCurveId);
int afCurveEditorMouseMove(af_curve_editor_t* pEditor, float fX, float fY,
                           float fW, float fH, af_doc pDoc, uint32_t iCurveId);
int afCurveEditorMouseUp(af_curve_editor_t* pEditor, float fX, float fY);

void afCurveEditorToggleChannel(af_curve_editor_t* pEditor, int iChannel);
void afCurveEditorSetChannelMask(af_curve_editor_t* pEditor, uint32_t iMask);
void afCurveEditorSetValueRange(af_curve_editor_t* pEditor, float fMin, float fMax);

#ifdef __cplusplus
}
#endif

#endif /* AF_CURVE_EDITOR_H */
