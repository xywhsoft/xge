#include "ptl_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_LEFT_ALT 342
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#define XGE_KEY_RIGHT_ALT 346
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int ptl_arg_int(const char* s, int def){ int v; if(!s||!s[0]) return def; v=atoi(s); return v>0?v:def; }
static void ptl_set_wd(void){
#ifdef _WIN32
	char p[MAX_PATH]; char* sl;
	if(!GetModuleFileNameA(NULL,p,sizeof(p))) return;
	p[sizeof(p)-1]=0; sl=strrchr(p,'\\'); if(!sl) sl=strrchr(p,'/');
	if(sl){*sl=0; SetCurrentDirectoryA(p);}
#endif
}

static uint32_t ptl_buttons(void){
	uint32_t b=0;
	if(xgeMouseDown(XGE_MOUSE_LEFT)) b|=XUI_POINTER_BUTTON_LEFT;
	if(xgeMouseDown(XGE_MOUSE_RIGHT)) b|=XUI_POINTER_BUTTON_RIGHT;
	return b;
}
static uint32_t ptl_mods(void){
	uint32_t m=0;
	if(xgeKeyDown(XGE_KEY_LEFT_SHIFT)||xgeKeyDown(XGE_KEY_RIGHT_SHIFT)) m|=XUI_MOD_SHIFT;
	if(xgeKeyDown(XGE_KEY_LEFT_CONTROL)||xgeKeyDown(XGE_KEY_RIGHT_CONTROL)) m|=XUI_MOD_CTRL;
	if(xgeKeyDown(XGE_KEY_LEFT_ALT)||xgeKeyDown(XGE_KEY_RIGHT_ALT)) m|=XUI_MOD_ALT;
	return m;
}

static int ptl_resize_target(ptl_app_t* a, int w, int h){
	xui_surface_desc_t sd;
	if(w<=0) w=PTL_W;
	if(h<=0) h=PTL_H;
	if(a->pTarget && a->iTargetW==w && a->iTargetH==h) return XUI_OK;
	if(a->pTarget){a->tProxy.surfaceDestroy(&a->tProxy,a->pTarget); a->pTarget=NULL;}
	memset(&sd,0,sizeof(sd));
	sd.iKind=XUI_SURFACE_KIND_TEXTURE; sd.iWidth=w; sd.iHeight=h;
	sd.iFormat=XUI_SURFACE_FORMAT_RGBA8;
	sd.iFlags=XUI_SURFACE_ALPHA_PREMULTIPLIED|XUI_SURFACE_USAGE_TARGET;
	if(a->tProxy.surfaceCreate(&a->tProxy,&a->pTarget,&sd)!=XUI_OK) return XUI_ERROR;
	a->iTargetW=w; a->iTargetH=h;
	return XUI_OK;
}

int ParticleLabSyncViewport(ptl_app_t* a){
	int w=xgeGetWidth(), h=xgeGetHeight();
	float fw,fh,cy,ch;
	if(w<=0) w=PTL_W;
	if(h<=0) h=PTL_H;
	fw=(float)w; fh=(float)h;
	if(ptl_resize_target(a,w,h)!=XUI_OK) return XUI_ERROR;
	(void)xuiInputViewport(a->pContext,fw,fh);
	(void)xuiSetViewportSize(a->pContext,fw,fh);
	if(a->pRoot)(void)xuiWidgetSetRect(a->pRoot,(xui_rect_t){0,0,fw,fh});
	if(a->pMenuBar)(void)xuiWidgetSetRect(a->pMenuBar,(xui_rect_t){0,0,fw,PTL_MENU_H});
	cy=PTL_MENU_H+PTL_TOOLBAR_H; ch=fh-cy-PTL_STATUS_H;
	if(ch<1.0f) ch=1.0f;
	if(a->pToolbar)(void)xuiWidgetSetRect(a->pToolbar,(xui_rect_t){0,PTL_MENU_H,fw,PTL_TOOLBAR_H});
	if(a->pDock)(void)xuiWidgetSetRect(a->pDock,(xui_rect_t){0,cy,fw,ch});
	if(a->pStatus)(void)xuiWidgetSetRect(a->pStatus,(xui_rect_t){0,fh-PTL_STATUS_H,fw,PTL_STATUS_H});
	return XUI_OK;
}

int ParticleLabHandleInput(ptl_app_t* a){
	static const int keys[]={XGE_KEY_ENTER,XGE_KEY_TAB,XGE_KEY_SPACE,XGE_KEY_BACKSPACE,XGE_KEY_DELETE,XGE_KEY_LEFT,XGE_KEY_RIGHT,XGE_KEY_UP,XGE_KEY_DOWN,XGE_KEY_ESCAPE};
	float x,y,wx,wy; uint32_t buttons,pressed,released,mods; int i;
	mods=ptl_mods();
	(void)xuiInputSetModifiers(a->pContext,mods);
	if(xgeKeyPressed(XGE_KEY_ESCAPE)) xgeQuit();
	for(i=0;i<(int)(sizeof(keys)/sizeof(keys[0]));i++){
		if(xgeKeyPressed(keys[i]))(void)xuiInputKeyDown(a->pContext,keys[i],mods);
		if(xgeKeyReleased(keys[i]))(void)xuiInputKeyUp(a->pContext,keys[i],mods);
	}
	while((mods=xgeTextGet())!=0)(void)xuiInputText(a->pContext,mods);
	xgeMouseGet(&x,&y); xgeMouseGetWheel(&wx,&wy);
	buttons=ptl_buttons();
	if(!a->bMouse||x!=a->fMouseX||y!=a->fMouseY||buttons!=a->iButtons)
		(void)xuiInputPointerMove(a->pContext,x,y,buttons);
	if(wx!=0||wy!=0)(void)xuiInputPointerWheel(a->pContext,x,y,wx,wy,buttons);
	pressed=buttons&~a->iButtons; released=a->iButtons&~buttons;
	if(pressed&XUI_POINTER_BUTTON_LEFT)(void)xuiInputPointerDown(a->pContext,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
	if(released&XUI_POINTER_BUTTON_LEFT)(void)xuiInputPointerUp(a->pContext,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
	a->bMouse=1; a->fMouseX=x; a->fMouseY=y; a->iButtons=buttons;
	return XUI_OK;
}

/* ─── Particle simulation ─── */
void ParticleLabSimReset(ptl_app_t* a){
	memset(a->arrP,0,sizeof(a->arrP));
	a->iPCount=0; a->fEmitAcc=0.0f;
}

static void ptl_emit(ptl_app_t* a, float cx, float cy){
	int i;
	for(i=0;i<PTL_MAX_PARTICLES;i++){
		ptl_particle_t* p=&a->arrP[i];
		float ang, spd, rad;
		if(p->bAlive) continue;
		ang=(a->tParams.fAngle + ((float)(rand()%1000)/500.0f-1.0f)*a->tParams.fAngleVar)*(float)M_PI/180.0f;
		spd=a->tParams.fSpeedMin + ((float)(rand()%1000)/1000.0f)*(a->tParams.fSpeedMax-a->tParams.fSpeedMin);
		rad=(float)(rand()%20)-10.0f;
		p->fX=cx+rad; p->fY=cy+rad;
		p->fVX=cosf(ang)*spd; p->fVY=sinf(ang)*spd;
		p->fMaxLife=a->tParams.fLifeMin + ((float)(rand()%1000)/1000.0f)*(a->tParams.fLifeMax-a->tParams.fLifeMin);
		p->fLife=p->fMaxLife;
		p->fSize=a->tParams.fSizeStart;
		p->bAlive=1;
		if(i>=a->iPCount) a->iPCount=i+1;
		return;
	}
}

void ParticleLabSimUpdate(ptl_app_t* a, float dt){
	int i;
	float cx, cy;
	if(!a->bPlaying) return;
	if(dt<=0.0f) dt=1.0f/60.0f;
	if(dt>0.1f) dt=0.1f;
	/* Emit */
	cx=(float)a->iTargetW*0.5f; cy=(float)a->iTargetH*0.5f;
	a->fEmitAcc += a->tParams.fEmitRate*dt;
	while(a->fEmitAcc>=1.0f){ ptl_emit(a,cx,cy); a->fEmitAcc-=1.0f; }
	/* Update */
	for(i=0;i<a->iPCount;i++){
		ptl_particle_t* p=&a->arrP[i];
		float t;
		if(!p->bAlive) continue;
		p->fLife-=dt;
		if(p->fLife<=0.0f){ p->bAlive=0; continue; }
		p->fVY+=a->tParams.fGravity*dt;
		p->fX+=p->fVX*dt; p->fY+=p->fVY*dt;
		t=1.0f-p->fLife/p->fMaxLife;
		p->fSize=a->tParams.fSizeStart+(a->tParams.fSizeEnd-a->tParams.fSizeStart)*t;
	}
	/* Compact iPCount */
	while(a->iPCount>0 && !a->arrP[a->iPCount-1].bAlive) a->iPCount--;
}

/* ─── Presets ─── */
void ParticleLabPresetApply(ptl_app_t* a, int iPreset){
	switch(iPreset){
	case PTL_CMD_PRESET_FIRE:
		a->tParams.fEmitRate=80.0f; a->tParams.fLifeMin=0.6f; a->tParams.fLifeMax=1.2f;
		a->tParams.fSpeedMin=60.0f; a->tParams.fSpeedMax=140.0f;
		a->tParams.fAngle=-90.0f; a->tParams.fAngleVar=20.0f;
		a->tParams.fGravity=-30.0f; a->tParams.fSizeStart=6.0f; a->tParams.fSizeEnd=1.0f;
		a->tParams.iColorStart=XUI_COLOR_RGBA(255,200,50,255);
		a->tParams.iColorEnd=XUI_COLOR_RGBA(255,60,20,0);
		break;
	case PTL_CMD_PRESET_SNOW:
		a->tParams.fEmitRate=30.0f; a->tParams.fLifeMin=2.0f; a->tParams.fLifeMax=4.0f;
		a->tParams.fSpeedMin=20.0f; a->tParams.fSpeedMax=50.0f;
		a->tParams.fAngle=90.0f; a->tParams.fAngleVar=30.0f;
		a->tParams.fGravity=15.0f; a->tParams.fSizeStart=4.0f; a->tParams.fSizeEnd=3.0f;
		a->tParams.iColorStart=XUI_COLOR_RGBA(240,248,255,255);
		a->tParams.iColorEnd=XUI_COLOR_RGBA(200,220,255,0);
		break;
	default: break;
	}
	ParticleLabSimReset(a);
	a->bPlaying=1;
}

/* ─── Canvas render ─── */
int ParticleLabCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser){
	ptl_app_t* a=(ptl_app_t*)pUser;
	xui_proxy_t proxy; xui_rect_t r;
	int i;
	float cx, cy;
	(void)iStateId;
	if(!a||!pDraw) return XUI_ERROR_INVALID_ARGUMENT;
	proxy=xuiProxyXge();
	r=xuiWidgetGetContentRect(pWidget);
	/* Dark background */
	(void)proxy.drawRectFill(&proxy,pDraw,r,XUI_COLOR_RGBA(16,16,24,255));
	/* Emitter indicator */
	cx=r.fX+r.fW*0.5f; cy=r.fY+r.fH*0.5f;
	(void)proxy.drawCircleFill(&proxy,pDraw,cx,cy,5.0f,XUI_COLOR_RGBA(100,200,255,180));
	/* Particles */
	for(i=0;i<a->iPCount;i++){
		ptl_particle_t* p=&a->arrP[i];
		float t, alpha, sz, px, py;
		uint32_t col;
		int cr0,cg0,cb0,cr1,cg1,cb1;
		if(!p->bAlive) continue;
		t=1.0f-p->fLife/p->fMaxLife;
		alpha=1.0f-t;
		sz=p->fSize; if(sz<0.5f) sz=0.5f;
		/* Map particle pos (screen space) into widget rect */
		px=r.fX+p->fX; py=r.fY+p->fY;
		/* Lerp color */
		cr0=(int)((a->tParams.iColorStart>>16)&0xFF); cg0=(int)((a->tParams.iColorStart>>8)&0xFF); cb0=(int)(a->tParams.iColorStart&0xFF);
		cr1=(int)((a->tParams.iColorEnd>>16)&0xFF); cg1=(int)((a->tParams.iColorEnd>>8)&0xFF); cb1=(int)(a->tParams.iColorEnd&0xFF);
		col=XUI_COLOR_RGBA(
			(int)(cr0+(cr1-cr0)*t),
			(int)(cg0+(cg1-cg0)*t),
			(int)(cb0+(cb1-cb0)*t),
			(int)(alpha*255.0f));
		(void)proxy.drawCircleFill(&proxy,pDraw,px,py,sz,col);
	}
	return XUI_OK;
}

/* ─── Menu/toolbar callbacks ─── */
static void ptl_menu_select(xui_widget_t* w, int idx, int val, void* user){
	ptl_app_t* a=(ptl_app_t*)user; (void)w;(void)idx;
	if(!a) return;
	switch(val){
	case PTL_CMD_EXIT: xgeQuit(); break;
	case PTL_CMD_PLAY:
		a->bPlaying=1;
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Playing");
		break;
	case PTL_CMD_STOP:
		a->bPlaying=0;
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Stopped");
		break;
	case PTL_CMD_RESET:
		ParticleLabSimReset(a);
		if(a->pCanvasWidget)(void)xuiWidgetInvalidate(a->pCanvasWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Reset");
		break;
	case PTL_CMD_PRESET_FIRE:
		ParticleLabPresetApply(a,PTL_CMD_PRESET_FIRE);
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Preset: Fire");
		break;
	case PTL_CMD_PRESET_SNOW:
		ParticleLabPresetApply(a,PTL_CMD_PRESET_SNOW);
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Preset: Snow");
		break;
	case PTL_CMD_EXPORT:
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Exported params");
		break;
	case PTL_CMD_ABOUT:
		(void)xuiStatusBarSetItemText(a->pStatus,0,"ParticleLab v1.0 - XUI2");
		break;
	}
}
static void ptl_toolbar_select(xui_widget_t* w, int idx, int val, void* user){
	(void)w;(void)idx;
	ptl_menu_select(NULL,0,val,user);
}

/* ─── Create UI ─── */
int ParticleLabCreateUI(ptl_app_t* a){
	xui_cache_policy_t policy;
	xui_menubar_desc_t mbd; xui_menu_desc_t md;
	xui_menubar_item_t mbI[3]; xui_menu_item_t fileI[4], presetI[2], helpI[1];
	xui_toolbar_desc_t tbd; xui_toolbar_item_t tbI[4];
	xui_statusbar_desc_t sbd; xui_dock_panel_desc_t dd;
	const char* font="C:\\Windows\\Fonts\\msyh.ttc";
	int ret, win;

	a->tProxy=xuiProxyXge();
	ret=xuiCreate(&a->pContext); if(ret!=XUI_OK) return ret;
	ret=xuiSetProxy(a->pContext,&a->tProxy); if(ret!=XUI_OK) return ret;
	(void)xuiInputViewport(a->pContext,(float)PTL_W,(float)PTL_H);
	if(ptl_resize_target(a,PTL_W,PTL_H)!=XUI_OK) return XUI_ERROR;
	if(a->tProxy.fontLoadFile(&a->tProxy,&a->pFont,font,13.0f,XUI_FONT_FORMAT_TTF)!=XUI_OK){
		font="C:\\Windows\\Fonts\\arial.ttf";
		if(a->tProxy.fontLoadFile(&a->tProxy,&a->pFont,font,13.0f,XUI_FONT_FORMAT_TTF)!=XUI_OK) return XUI_ERROR;
	}
	(void)xuiSetDefaultFont(a->pContext,a->pFont);

	ret=xuiWidgetCreate(a->pContext,&a->pRoot); if(ret!=XUI_OK) return ret;
	memset(&policy,0,sizeof(policy)); policy.iSize=sizeof(policy);
	policy.iPolicy=XUI_CACHE_POLICY_SELF; policy.iFlags=XUI_CACHE_CLEAR_ON_UPDATE;
	(void)xuiWidgetSetRect(a->pRoot,(xui_rect_t){0,0,(float)PTL_W,(float)PTL_H});
	(void)xuiWidgetSetLayoutType(a->pRoot,XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(a->pRoot,&policy);
	(void)xuiSetRootWidget(a->pContext,a->pRoot);

	/* Menu bar */
	memset(&mbd,0,sizeof(mbd)); mbd.iSize=sizeof(mbd); mbd.pFont=a->pFont;
	(void)xuiMenuBarCreate(a->pContext,&a->pMenuBar,&mbd);
	(void)xuiWidgetSetRect(a->pMenuBar,(xui_rect_t){0,0,(float)PTL_W,PTL_MENU_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pMenuBar);
	memset(&md,0,sizeof(md)); md.iSize=sizeof(md); md.pOwner=a->pMenuBar; md.pFont=a->pFont;
	(void)xuiMenuCreate(a->pContext,&a->pFileMenu,&md);
	(void)xuiMenuCreate(a->pContext,&a->pPresetMenu,&md);
	(void)xuiMenuCreate(a->pContext,&a->pHelpMenu,&md);
	memset(fileI,0,sizeof(fileI));
	fileI[0]=(xui_menu_item_t){"播放","F5",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,PTL_CMD_PLAY,0,NULL,NULL};
	fileI[1]=(xui_menu_item_t){"停止","F6",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,PTL_CMD_STOP,0,NULL,NULL};
	fileI[2]=(xui_menu_item_t){NULL,NULL,XUI_MENU_ITEM_SEPARATOR,0,0,0,NULL,NULL};
	fileI[3]=(xui_menu_item_t){"退出","Alt+F4",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,PTL_CMD_EXIT,0,NULL,NULL};
	memset(presetI,0,sizeof(presetI));
	presetI[0]=(xui_menu_item_t){"火焰",NULL,XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,PTL_CMD_PRESET_FIRE,0,NULL,NULL};
	presetI[1]=(xui_menu_item_t){"雪花",NULL,XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,PTL_CMD_PRESET_SNOW,0,NULL,NULL};
	helpI[0]=(xui_menu_item_t){"关于",NULL,XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,PTL_CMD_ABOUT,0,NULL,NULL};
	(void)xuiMenuSetItems(a->pFileMenu,fileI,4);
	(void)xuiMenuSetItems(a->pPresetMenu,presetI,2);
	(void)xuiMenuSetItems(a->pHelpMenu,helpI,1);
	(void)xuiMenuSetSelect(a->pFileMenu,ptl_menu_select,a);
	(void)xuiMenuSetSelect(a->pPresetMenu,ptl_menu_select,a);
	(void)xuiMenuSetSelect(a->pHelpMenu,ptl_menu_select,a);
	mbI[0]=(xui_menubar_item_t){"文件",XUI_MENUBAR_ITEM_ENABLED,0,'F',a->pFileMenu,{0,0,0,0},NULL};
	mbI[1]=(xui_menubar_item_t){"预设",XUI_MENUBAR_ITEM_ENABLED,0,'P',a->pPresetMenu,{0,0,0,0},NULL};
	mbI[2]=(xui_menubar_item_t){"帮助",XUI_MENUBAR_ITEM_ENABLED,0,'H',a->pHelpMenu,{0,0,0,0},NULL};
	(void)xuiMenuBarSetItems(a->pMenuBar,mbI,3);
	(void)xuiMenuBarSetSelect(a->pMenuBar,ptl_menu_select,a);

	/* Toolbar */
	memset(&tbd,0,sizeof(tbd)); tbd.iSize=sizeof(tbd); tbd.pFont=a->pFont;
	memset(tbI,0,sizeof(tbI));
	tbI[0]=(xui_toolbar_item_t){"播放","Play",XUI_TOOLBAR_ITEM_BUTTON,0,PTL_CMD_PLAY,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[1]=(xui_toolbar_item_t){"停止","Stop",XUI_TOOLBAR_ITEM_BUTTON,0,PTL_CMD_STOP,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[2]=(xui_toolbar_item_t){"重置","Reset",XUI_TOOLBAR_ITEM_BUTTON,0,PTL_CMD_RESET,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[3]=(xui_toolbar_item_t){"火焰","Fire preset",XUI_TOOLBAR_ITEM_BUTTON,0,PTL_CMD_PRESET_FIRE,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbd.pItems=tbI; tbd.iItemCount=4;
	(void)xuiToolbarCreate(a->pContext,&a->pToolbar,&tbd);
	(void)xuiWidgetSetRect(a->pToolbar,(xui_rect_t){0,PTL_MENU_H,(float)PTL_W,PTL_TOOLBAR_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pToolbar);
	(void)xuiToolbarSetSelect(a->pToolbar,ptl_toolbar_select,a);

	/* Status bar */
	memset(&sbd,0,sizeof(sbd)); sbd.iSize=sizeof(sbd); sbd.pFont=a->pFont;
	(void)xuiStatusBarCreate(a->pContext,&a->pStatus,&sbd);
	(void)xuiWidgetSetRect(a->pStatus,(xui_rect_t){0,(float)PTL_H-PTL_STATUS_H,(float)PTL_W,PTL_STATUS_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pStatus);
	(void)xuiStatusBarAddText(a->pStatus,XUI_STATUSBAR_SECTION_LEFT,"就绪",200.0f,0,0);
	(void)xuiStatusBarAddFlexibleSpacer(a->pStatus,XUI_STATUSBAR_SECTION_LEFT,1.0f);

	/* Dock + Canvas */
	memset(&dd,0,sizeof(dd)); dd.iSize=sizeof(dd); dd.pFont=a->pFont;
	(void)xuiDockPanelCreate(a->pContext,&a->pDock,&dd);
	(void)xuiWidgetSetRect(a->pDock,(xui_rect_t){0,PTL_MENU_H+PTL_TOOLBAR_H,(float)PTL_W,(float)PTL_H-PTL_MENU_H-PTL_TOOLBAR_H-PTL_STATUS_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pDock);
	(void)xuiWidgetCreate(a->pContext,&a->pCanvasWidget);
	(void)xuiWidgetSetCacheRenderCallback(a->pCanvasWidget,ParticleLabCanvasRender,a);
	(void)xuiDockPanelAddWindow(a->pDock,"粒子画布",a->pCanvasWidget,&win);
	(void)xuiDockPanelDockWindow(a->pDock,win,XUI_DOCK_PANEL_REGION_DOCUMENT,XUI_DOCK_PANEL_SIDE_FILL,0.0f,NULL);

	a->bCreateOK=1;
	return XUI_OK;
}

void ParticleLabDestroy(ptl_app_t* a){
	if(!a) return;
	if(a->pTarget){a->tProxy.surfaceDestroy(&a->tProxy,a->pTarget); a->pTarget=NULL;}
	if(a->pFont){a->tProxy.fontDestroy(&a->tProxy,a->pFont); a->pFont=NULL;}
	if(a->pContext){xuiDestroy(a->pContext); a->pContext=NULL;}
}

int ParticleLabFrame(void* pUser){
	ptl_app_t* a=(ptl_app_t*)pUser;
	xui_rect_i_t full; xui_rect_t src,dst; int ret;
	float dt;
	if(!a) return XGE_ERROR;
	ret=xgeBegin(); if(ret!=XGE_OK) return ret;
	(void)ParticleLabSyncViewport(a);
	(void)ParticleLabHandleInput(a);
	(void)xuiDispatchPendingEvents(a->pContext);
	(void)xuiLayout(a->pContext);
	dt=xgeGetDelta();
	ParticleLabSimUpdate(a,dt);
	(void)xuiUpdate(a->pContext,dt);
	if(a->pCanvasWidget)(void)xuiWidgetInvalidate(a->pCanvasWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
	(void)a->tProxy.surfaceClear(&a->tProxy,a->pTarget,XUI_COLOR_RGBA(16,16,24,255));
	full=(xui_rect_i_t){0,0,a->iTargetW,a->iTargetH};
	(void)xuiRender(a->pContext,a->pTarget,&full,1);
	a->bRenderOK=1;
	xgeClear(XUI_COLOR_RGBA(10,10,18,255));
	src=(xui_rect_t){0,0,(float)a->iTargetW,(float)a->iTargetH}; dst=src;
	ret=a->tProxy.surfaceDraw(&a->tProxy,a->pTarget,src,dst,XUI_COLOR_WHITE,XUI_SURFACE_DRAW_SCREEN_SPACE);
	if(ret==XUI_OK) ret=xgeEnd();
	if(ret!=XGE_OK) return ret;
	a->iFrame++;
	if(a->iFrameLimit>0 && a->iFrame>=a->iFrameLimit){
		printf("particlelab_xui2 final-summary frames=%d create=%d render=%d particles=%d playing=%d\n",
			a->iFrame,a->bCreateOK,a->bRenderOK,a->iPCount,a->bPlaying);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv){
	static ptl_app_t app;
	xge_desc_t desc;
	int i, ret;
	ptl_set_wd();
	srand(12345);
	memset(&app,0,sizeof(app));
	/* Default params: fire-like */
	app.tParams.fEmitRate=60.0f; app.tParams.fLifeMin=0.8f; app.tParams.fLifeMax=1.5f;
	app.tParams.fSpeedMin=50.0f; app.tParams.fSpeedMax=120.0f;
	app.tParams.fAngle=-90.0f; app.tParams.fAngleVar=15.0f;
	app.tParams.fGravity=-20.0f; app.tParams.fSizeStart=5.0f; app.tParams.fSizeEnd=1.0f;
	app.tParams.iColorStart=XUI_COLOR_RGBA(255,180,50,255);
	app.tParams.iColorEnd=XUI_COLOR_RGBA(255,60,20,0);
	app.bPlaying=1;
	app.iFrameLimit=ptl_arg_int(getenv("XGE_PARTICLELAB_FRAMES"),0);
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"--frames")==0&&i+1<argc) app.iFrameLimit=ptl_arg_int(argv[++i],app.iFrameLimit);
		else if(strncmp(argv[i],"--frames=",9)==0) app.iFrameLimit=ptl_arg_int(argv[i]+9,app.iFrameLimit);
	}
	memset(&desc,0,sizeof(desc));
	desc.sTitle="ParticleLab"; desc.iWidth=PTL_W; desc.iHeight=PTL_H;
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC;
	desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc);
	if(ret!=XGE_OK){printf("particlelab_xui2: xgeInit failed: %d\n",ret); return 1;}
	ret=ParticleLabCreateUI(&app);
	if(ret!=XUI_OK){printf("particlelab_xui2: create ui failed: %d\n",ret); ParticleLabDestroy(&app); xgeUnit(); return 1;}
	ret=xgeRun(ParticleLabFrame,&app);
	ParticleLabDestroy(&app); xgeUnit();
	return (ret==XGE_OK&&app.bCreateOK&&app.bRenderOK)?0:1;
}
