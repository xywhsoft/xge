#ifndef PTL_APP_H
#define PTL_APP_H

#include "xge.h"
#include "xui.h"

#define PTL_W 1400
#define PTL_H 900
#define PTL_MENU_H 26.0f
#define PTL_TOOLBAR_H 32.0f
#define PTL_STATUS_H 28.0f
#define PTL_MAX_PARTICLES 512

enum { PTL_CMD_PLAY=1, PTL_CMD_STOP, PTL_CMD_RESET, PTL_CMD_PRESET_FIRE, PTL_CMD_PRESET_SNOW, PTL_CMD_EXPORT, PTL_CMD_EXIT, PTL_CMD_ABOUT };

typedef struct { float fX,fY,fVX,fVY,fLife,fMaxLife,fSize; int bAlive; } ptl_particle_t;

typedef struct {
	float fEmitRate, fLifeMin, fLifeMax, fSpeedMin, fSpeedMax;
	float fAngle, fAngleVar, fGravity, fSizeStart, fSizeEnd;
	uint32_t iColorStart, iColorEnd;
} ptl_params_t;

typedef struct {
	xui_context pContext; xui_proxy_t tProxy; xui_surface pTarget;
	int iTargetW,iTargetH; xui_font pFont; xui_widget pRoot;
	xui_widget pMenuBar,pFileMenu,pPresetMenu,pHelpMenu,pToolbar,pStatus,pDock,pCanvasWidget;
	ptl_params_t tParams;
	ptl_particle_t arrP[PTL_MAX_PARTICLES]; int iPCount;
	float fEmitAcc; int bPlaying;
	int bMouse; float fMouseX,fMouseY; uint32_t iButtons;
	int bCreateOK,bRenderOK,iFrame,iFrameLimit;
} ptl_app_t;

int ParticleLabCreateUI(ptl_app_t* a);
void ParticleLabDestroy(ptl_app_t* a);
int ParticleLabFrame(void* pUser);
int ParticleLabHandleInput(ptl_app_t* a);
int ParticleLabSyncViewport(ptl_app_t* a);
int ParticleLabCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
void ParticleLabSimUpdate(ptl_app_t* a, float dt);
void ParticleLabSimReset(ptl_app_t* a);
void ParticleLabPresetApply(ptl_app_t* a, int iPreset);

#endif
