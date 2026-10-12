/*
 * af_property.h - Property panel binding
 */

#ifndef AF_PROPERTY_H
#define AF_PROPERTY_H

#include "xui.h"
#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct af_property_t {
	xui_widget pPanel;
	uint32_t   iTargetInstance;
} af_property_t;

void afPropertyInit(af_property_t* pProp);
void afPropertyShutdown(af_property_t* pProp);

void afPropertyRender(af_property_t* pProp, xui_proxy_t* pProxy, xui_draw_context pCtx,
                      float fW, float fH, af_doc pDoc, uint32_t iInstanceId);
int afPropertyMouseDown(af_property_t* pProp, float fX, float fY,
                        float fW, float fH, af_doc pDoc, uint32_t iInstanceId);

int afPropertySetValue(af_doc pDoc, uint32_t iInstanceId, int iField, float fValue);
float afPropertyGetValue(af_doc pDoc, uint32_t iInstanceId, int iField);

#ifdef __cplusplus
}
#endif

#endif /* AF_PROPERTY_H */
