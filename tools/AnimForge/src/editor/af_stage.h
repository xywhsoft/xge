/*
 * af_stage.h - Stage rendering and view transform
 */

#ifndef AF_STAGE_H
#define AF_STAGE_H

#include "xui.h"
#include "../core/af_doc.h"
#include "../core/af_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Guides                                                             */
/* ------------------------------------------------------------------ */

#define AF_STAGE_MAX_GUIDES  64

typedef struct af_stage_guide_t {
	int   bVertical;
	float fPosition;
} af_stage_guide_t;

typedef struct af_stage_guides_t {
	af_stage_guide_t arrGuides[AF_STAGE_MAX_GUIDES];
	int iCount;
} af_stage_guides_t;

/* ------------------------------------------------------------------ */
/* View transform                                                     */
/* ------------------------------------------------------------------ */

void afStageScreenToWorld(const af_stage_view_t* pView, float fSX, float fSY,
                          float* pWX, float* pWY);
void afStageWorldToScreen(const af_stage_view_t* pView, float fWX, float fWY,
                          float* pSX, float* pSY);
void afStageZoomAt(af_stage_view_t* pView, float fFactor, float fCenterX, float fCenterY);
void afStageFitToView(af_stage_view_t* pView, float fStageW, float fStageH,
                      float fCanvasW, float fCanvasH);
void afStagePan(af_stage_view_t* pView, float fDX, float fDY);

/* ------------------------------------------------------------------ */
/* Grid snapping                                                      */
/* ------------------------------------------------------------------ */

float afStageSnapToGrid(const af_stage_view_t* pView, float fValue);
void afStageSnapPoint(const af_stage_view_t* pView, float* pX, float* pY);

/* ------------------------------------------------------------------ */
/* Guide management                                                   */
/* ------------------------------------------------------------------ */

void afStageInitGuides(af_stage_guides_t* pGuides);
int afStageAddGuide(af_stage_guides_t* pGuides, int bVertical, float fPosition);
int afStageRemoveGuide(af_stage_guides_t* pGuides, int iIndex);
void afStageClearGuides(af_stage_guides_t* pGuides);
float afStageSnapToGuides(const af_stage_guides_t* pGuides, float fValue, int bVertical, float fThreshold);

/* ------------------------------------------------------------------ */
/* Rendering                                                          */
/* ------------------------------------------------------------------ */

void afStageRenderBackground(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                             float fStageW, float fStageH, float fCanvasW, float fCanvasH);
void afStageRenderGrid(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                       float fStageW, float fStageH);
void afStageRenderRulers(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                         float fCanvasW, float fCanvasH, float fStageW, float fStageH);
void afStageRenderGuides(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                         const af_stage_guides_t* pGuides, float fCanvasW, float fCanvasH);
void afStageRenderElement(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                          af_doc pDoc, const af_element_t* pElem);
void afStageRenderSelection(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                            af_doc pDoc, uint32_t iInstanceId);

#ifdef __cplusplus
}
#endif

#endif /* AF_STAGE_H */
