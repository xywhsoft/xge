/*
 * af_draw_helper.h - Convenience drawing wrappers for AnimForge panels
 *
 * Wraps xui_proxy_t draw calls into simple immediate-mode style helpers.
 * All AnimForge editor panels use these instead of calling proxy directly.
 */

#ifndef AF_DRAW_HELPER_H
#define AF_DRAW_HELPER_H

#include "xui.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void afDrawFillRect(xui_proxy_t* pProxy, xui_draw_context pDraw,
                                  float fX, float fY, float fW, float fH, uint32_t iColor)
{
	xui_rect_t rc;
	rc.fX = fX; rc.fY = fY; rc.fW = fW; rc.fH = fH;
	pProxy->drawRectFill(pProxy, pDraw, rc, iColor);
}

static inline void afDrawStrokeRect(xui_proxy_t* pProxy, xui_draw_context pDraw,
                                    float fX, float fY, float fW, float fH,
                                    float fWidth, uint32_t iColor)
{
	xui_rect_t rc;
	rc.fX = fX; rc.fY = fY; rc.fW = fW; rc.fH = fH;
	pProxy->drawRectStroke(pProxy, pDraw, rc, fWidth, iColor);
}

static inline void afDrawLine(xui_proxy_t* pProxy, xui_draw_context pDraw,
                              float fX0, float fY0, float fX1, float fY1,
                              float fWidth, uint32_t iColor)
{
	pProxy->drawLine(pProxy, pDraw, fX0, fY0, fX1, fY1, fWidth, iColor);
}

static inline void afDrawCircleFill(xui_proxy_t* pProxy, xui_draw_context pDraw,
                                    float fX, float fY, float fRadius, uint32_t iColor)
{
	pProxy->drawCircleFill(pProxy, pDraw, fX, fY, fRadius, iColor);
}

static inline void afDrawCircleStroke(xui_proxy_t* pProxy, xui_draw_context pDraw,
                                      float fX, float fY, float fRadius,
                                      float fWidth, uint32_t iColor)
{
	pProxy->drawCircleStroke(pProxy, pDraw, fX, fY, fRadius, fWidth, iColor);
}

#ifdef __cplusplus
}
#endif

#endif /* AF_DRAW_HELPER_H */
