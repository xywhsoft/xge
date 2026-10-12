/*
 * af_onion.h - Onion skin rendering
 */

#ifndef AF_ONION_H
#define AF_ONION_H

#include "xui.h"
#include "../core/af_doc.h"
#include "../core/af_app.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AF_ONION_MAX_FRAMES  10

typedef struct af_onion_config_t {
	int      bEnabled;
	int      iPrevFrames;      /* number of previous frames to show */
	int      iNextFrames;      /* number of next frames to show */
	float    fPrevAlpha;       /* base alpha for previous frames */
	float    fNextAlpha;       /* base alpha for next frames */
	uint32_t iPrevColor;       /* tint color for previous frames */
	uint32_t iNextColor;       /* tint color for next frames */
	int      bColorMode;       /* 1=color tint, 0=grayscale */
} af_onion_config_t;

void afOnionInit(af_onion_config_t* pConfig);
void afOnionSetEnabled(af_onion_config_t* pConfig, int bEnabled);
void afOnionSetRange(af_onion_config_t* pConfig, int iPrev, int iNext);
void afOnionSetAlpha(af_onion_config_t* pConfig, float fPrev, float fNext);
void afOnionSetColors(af_onion_config_t* pConfig, uint32_t iPrev, uint32_t iNext);
void afOnionToggle(af_onion_config_t* pConfig);
int afOnionIsEnabled(const af_onion_config_t* pConfig);

void afOnionRender(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                   af_doc pDoc, uint32_t iCurrentFrame, const af_onion_config_t* pConfig);

#ifdef __cplusplus
}
#endif

#endif /* AF_ONION_H */
