#include "ant_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static int ant_arg_int(const char* s, int def){ int v; if(!s||!s[0]) return def; v=atoi(s); return v>0?v:def; }
static void ant_set_wd(void){
#ifdef _WIN32
	char p[MAX_PATH]; char* sl;
	if(!GetModuleFileNameA(NULL,p,sizeof(p))) return;
	p[sizeof(p)-1]=0; sl=strrchr(p,'\\'); if(!sl) sl=strrchr(p,'/');
	if(sl){*sl=0; SetCurrentDirectoryA(p);}
#endif
}

static uint32_t ant_buttons(void){
	uint32_t b=0;
	if(xgeMouseDown(XGE_MOUSE_LEFT)) b|=XUI_POINTER_BUTTON_LEFT;
	if(xgeMouseDown(XGE_MOUSE_RIGHT)) b|=XUI_POINTER_BUTTON_RIGHT;
	return b;
}
static uint32_t ant_mods(void){
	uint32_t m=0;
	if(xgeKeyDown(XGE_KEY_LEFT_SHIFT)||xgeKeyDown(XGE_KEY_RIGHT_SHIFT)) m|=XUI_MOD_SHIFT;
	if(xgeKeyDown(XGE_KEY_LEFT_CONTROL)||xgeKeyDown(XGE_KEY_RIGHT_CONTROL)) m|=XUI_MOD_CTRL;
	if(xgeKeyDown(XGE_KEY_LEFT_ALT)||xgeKeyDown(XGE_KEY_RIGHT_ALT)) m|=XUI_MOD_ALT;
	return m;
}

static int ant_resize_target(ant_app_t* a, int w, int h){
	xui_surface_desc_t sd;
	if(w<=0) w=ANT_W;
	if(h<=0) h=ANT_H;
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

int AnimTimelineSyncViewport(ant_app_t* a){
	int w=xgeGetWidth(), h=xgeGetHeight();
	float fw,fh,cy,ch;
	if(w<=0) w=ANT_W;
	if(h<=0) h=ANT_H;
	fw=(float)w; fh=(float)h;
	if(ant_resize_target(a,w,h)!=XUI_OK) return XUI_ERROR;
	(void)xuiInputViewport(a->pContext,fw,fh);
	(void)xuiSetViewportSize(a->pContext,fw,fh);
	if(a->pRoot)(void)xuiWidgetSetRect(a->pRoot,(xui_rect_t){0,0,fw,fh});
	if(a->pMenuBar)(void)xuiWidgetSetRect(a->pMenuBar,(xui_rect_t){0,0,fw,ANT_MENU_H});
	cy=ANT_MENU_H+ANT_TOOLBAR_H; ch=fh-cy-ANT_STATUS_H;
	if(ch<1.0f) ch=1.0f;
	if(a->pToolbar)(void)xuiWidgetSetRect(a->pToolbar,(xui_rect_t){0,ANT_MENU_H,fw,ANT_TOOLBAR_H});
	if(a->pDock)(void)xuiWidgetSetRect(a->pDock,(xui_rect_t){0,cy,fw,ch});
	if(a->pStatus)(void)xuiWidgetSetRect(a->pStatus,(xui_rect_t){0,fh-ANT_STATUS_H,fw,ANT_STATUS_H});
	return XUI_OK;
}

int AnimTimelineHandleInput(ant_app_t* a){
	static const int keys[]={XGE_KEY_ENTER,XGE_KEY_TAB,XGE_KEY_SPACE,XGE_KEY_BACKSPACE,XGE_KEY_DELETE,XGE_KEY_LEFT,XGE_KEY_RIGHT,XGE_KEY_UP,XGE_KEY_DOWN,XGE_KEY_ESCAPE};
	float x,y,wx,wy; uint32_t buttons,pressed,released,mods; int i;
	mods=ant_mods();
	(void)xuiInputSetModifiers(a->pContext,mods);
	if(xgeKeyPressed(XGE_KEY_ESCAPE)) xgeQuit();
	/* Space = play/stop toggle */
	if(xgeKeyPressed(XGE_KEY_SPACE)) a->bPlaying=!a->bPlaying;
	for(i=0;i<(int)(sizeof(keys)/sizeof(keys[0]));i++){
		if(xgeKeyPressed(keys[i]))(void)xuiInputKeyDown(a->pContext,keys[i],mods);
		if(xgeKeyReleased(keys[i]))(void)xuiInputKeyUp(a->pContext,keys[i],mods);
	}
	while((mods=xgeTextGet())!=0)(void)xuiInputText(a->pContext,mods);
	xgeMouseGet(&x,&y); xgeMouseGetWheel(&wx,&wy);
	buttons=ant_buttons();
	if(!a->bMouse||x!=a->fMouseX||y!=a->fMouseY||buttons!=a->iButtons)
		(void)xuiInputPointerMove(a->pContext,x,y,buttons);
	if(wx!=0||wy!=0)(void)xuiInputPointerWheel(a->pContext,x,y,wx,wy,buttons);
	pressed=buttons&~a->iButtons; released=a->iButtons&~buttons;
	if(pressed&XUI_POINTER_BUTTON_LEFT)(void)xuiInputPointerDown(a->pContext,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
	if(released&XUI_POINTER_BUTTON_LEFT)(void)xuiInputPointerUp(a->pContext,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
	a->bMouse=1; a->fMouseX=x; a->fMouseY=y; a->iButtons=buttons;
	return XUI_OK;
}

/* ─── Duration recalc ─── */
void AnimTimelineRecalcDuration(ant_app_t* a){
	int i,j,max=0;
	for(i=0;i<a->iLayerCount;i++){
		int sum=0;
		for(j=0;j<a->arrLayers[i].iFrameCount;j++) sum+=a->arrLayers[i].arrFrames[j].iDuration;
		if(sum>max) max=sum;
	}
	a->iTotalTicks=max;
}

/* ─── Playback ─── */
void AnimTimelinePlayUpdate(ant_app_t* a, float dt){
	if(!a->bPlaying) return;
	if(a->iTotalTicks<=0) return;
	a->fPlayTime += dt*(float)a->iFPS;
	if(a->fPlayTime >= (float)a->iTotalTicks) a->fPlayTime -= (float)a->iTotalTicks;
	/* Determine current frame on active layer */
	if(a->iCurLayer>=0 && a->iCurLayer<a->iLayerCount){
		ant_layer_t* L=&a->arrLayers[a->iCurLayer];
		int tick=(int)a->fPlayTime, acc=0, fi;
		for(fi=0;fi<L->iFrameCount;fi++){
			acc+=L->arrFrames[fi].iDuration;
			if(tick<acc){ a->iCurFrame=fi; break; }
		}
	}
}

/* ─── Canvas render (preview current frame) ─── */
int AnimTimelineCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser){
	ant_app_t* a=(ant_app_t*)pUser;
	xui_proxy_t proxy; xui_rect_t r;
	int i;
	(void)iStateId;
	if(!a||!pDraw) return XUI_ERROR_INVALID_ARGUMENT;
	proxy=xuiProxyXge();
	r=xuiWidgetGetContentRect(pWidget);
	(void)proxy.drawRectFill(&proxy,pDraw,r,XUI_COLOR_RGBA(24,24,32,255));
	/* Draw each visible layer's current frame as a colored rect */
	for(i=0;i<a->iLayerCount;i++){
		ant_layer_t* L=&a->arrLayers[i];
		float cx, cy, sz;
		if(!L->bVisible||L->iFrameCount==0) continue;
		{
			int fi=a->iCurFrame;
			if(fi>=L->iFrameCount) fi=L->iFrameCount-1;
			cx=r.fX+r.fW*0.5f; cy=r.fY+r.fH*0.5f;
			sz=40.0f+(float)(i*20);
			(void)proxy.drawRectFill(&proxy,pDraw,
				(xui_rect_t){cx-sz*0.5f+(float)(i*10-20), cy-sz*0.5f, sz, sz},
				L->arrFrames[fi].iColor);
		}
	}
	/* Frame indicator bar at bottom */
	if(a->iTotalTicks>0){
		float pct=a->fPlayTime/(float)a->iTotalTicks;
		float bw=r.fW-40.0f;
		(void)proxy.drawRectFill(&proxy,pDraw,(xui_rect_t){r.fX+20.0f,r.fY+r.fH-16.0f,bw,6.0f},XUI_COLOR_RGBA(60,60,80,255));
		(void)proxy.drawRectFill(&proxy,pDraw,(xui_rect_t){r.fX+20.0f,r.fY+r.fH-16.0f,bw*pct,6.0f},XUI_COLOR_RGBA(80,200,255,255));
	}
	return XUI_OK;
}

/* ─── Timeline render ─── */
int AnimTimelineTimelineRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser){
	ant_app_t* a=(ant_app_t*)pUser;
	xui_proxy_t proxy; xui_rect_t r;
	int i,j;
	float rowH, x0, tickW;
	(void)iStateId;
	if(!a||!pDraw) return XUI_ERROR_INVALID_ARGUMENT;
	proxy=xuiProxyXge();
	r=xuiWidgetGetContentRect(pWidget);
	(void)proxy.drawRectFill(&proxy,pDraw,r,XUI_COLOR_RGBA(32,32,42,255));
	if(a->iLayerCount==0) return XUI_OK;
	rowH=(r.fH-20.0f)/(float)a->iLayerCount;
	if(rowH>30.0f) rowH=30.0f;
	x0=r.fX+80.0f; /* label area */
	tickW=(r.fW-90.0f)/(float)(a->iTotalTicks>0?a->iTotalTicks:60);
	if(tickW>20.0f) tickW=20.0f;
	for(i=0;i<a->iLayerCount;i++){
		ant_layer_t* L=&a->arrLayers[i];
		float ry=r.fY+10.0f+(float)i*rowH;
		float fx=x0;
		/* Layer label bg */
		(void)proxy.drawRectFill(&proxy,pDraw,(xui_rect_t){r.fX,ry,78.0f,rowH-2.0f},
			(i==a->iCurLayer)?XUI_COLOR_RGBA(60,80,120,255):XUI_COLOR_RGBA(44,44,56,255));
		/* Frame blocks */
		for(j=0;j<L->iFrameCount;j++){
			float fw=(float)L->arrFrames[j].iDuration*tickW;
			uint32_t col=L->arrFrames[j].iColor;
			(void)proxy.drawRectFill(&proxy,pDraw,(xui_rect_t){fx,ry+1.0f,fw-1.0f,rowH-4.0f},col);
			(void)proxy.drawRectStroke(&proxy,pDraw,(xui_rect_t){fx,ry+1.0f,fw-1.0f,rowH-4.0f},1.0f,XUI_COLOR_RGBA(180,180,200,120));
			if(L->arrFrames[j].bKeyframe)
				(void)proxy.drawCircleFill(&proxy,pDraw,fx+4.0f,ry+rowH*0.5f,3.0f,XUI_COLOR_RGBA(255,220,80,255));
			fx+=fw;
		}
	}
	/* Playhead */
	if(a->iTotalTicks>0){
		float px=x0+a->fPlayTime*tickW;
		(void)proxy.drawLine(&proxy,pDraw,px,r.fY,px,r.fY+r.fH,2.0f,XUI_COLOR_RGBA(255,80,80,220));
	}
	return XUI_OK;
}

/* ─── Menu/toolbar callbacks ─── */
static void ant_menu_select(xui_widget_t* w, int idx, int val, void* user){
	ant_app_t* a=(ant_app_t*)user; (void)w;(void)idx;
	if(!a) return;
	switch(val){
	case ANT_CMD_EXIT: xgeQuit(); break;
	case ANT_CMD_PLAY: a->bPlaying=1; (void)xuiStatusBarSetItemText(a->pStatus,0,"Playing"); break;
	case ANT_CMD_STOP: a->bPlaying=0; a->fPlayTime=0; (void)xuiStatusBarSetItemText(a->pStatus,0,"Stopped"); break;
	case ANT_CMD_STEP_FWD:
		a->bPlaying=0;
		if(a->iCurLayer>=0&&a->iCurLayer<a->iLayerCount){
			ant_layer_t* L=&a->arrLayers[a->iCurLayer];
			if(a->iCurFrame<L->iFrameCount-1) a->iCurFrame++;
		}
		break;
	case ANT_CMD_STEP_BACK:
		a->bPlaying=0;
		if(a->iCurFrame>0) a->iCurFrame--;
		break;
	case ANT_CMD_ADD_FRAME:
		if(a->iCurLayer>=0&&a->iCurLayer<a->iLayerCount){
			ant_layer_t* L=&a->arrLayers[a->iCurLayer];
			if(L->iFrameCount<ANT_MAX_FRAMES){
				ant_frame_t* f=&L->arrFrames[L->iFrameCount];
				f->iDuration=10; f->iColor=XUI_COLOR_RGBA(100+L->iFrameCount*20%155,150,220,200); f->bKeyframe=1;
				L->iFrameCount++;
				AnimTimelineRecalcDuration(a);
			}
		}
		if(a->pTimelineWidget)(void)xuiWidgetInvalidate(a->pTimelineWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
		break;
	case ANT_CMD_DEL_FRAME:
		if(a->iCurLayer>=0&&a->iCurLayer<a->iLayerCount){
			ant_layer_t* L=&a->arrLayers[a->iCurLayer];
			if(L->iFrameCount>0&&a->iCurFrame<L->iFrameCount){
				memmove(&L->arrFrames[a->iCurFrame],&L->arrFrames[a->iCurFrame+1],sizeof(ant_frame_t)*(L->iFrameCount-a->iCurFrame-1));
				L->iFrameCount--;
				if(a->iCurFrame>=L->iFrameCount) a->iCurFrame=L->iFrameCount-1;
				AnimTimelineRecalcDuration(a);
			}
		}
		if(a->pTimelineWidget)(void)xuiWidgetInvalidate(a->pTimelineWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
		break;
	case ANT_CMD_EXPORT:
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Exported timeline");
		break;
	case ANT_CMD_ABOUT:
		(void)xuiStatusBarSetItemText(a->pStatus,0,"AnimTimeline v1.0 - XUI2");
		break;
	}
}
static void ant_toolbar_select(xui_widget_t* w, int idx, int val, void* user){
	(void)w;(void)idx;
	ant_menu_select(NULL,0,val,user);
}

/* ─── Create UI ─── */
int AnimTimelineCreateUI(ant_app_t* a){
	xui_cache_policy_t policy;
	xui_menubar_desc_t mbd; xui_menu_desc_t md;
	xui_menubar_item_t mbI[2]; xui_menu_item_t fileI[6], helpI[1];
	xui_toolbar_desc_t tbd; xui_toolbar_item_t tbI[5];
	xui_statusbar_desc_t sbd; xui_dock_panel_desc_t dd;
	const char* font="C:\\Windows\\Fonts\\msyh.ttc";
	int ret, win;

	a->tProxy=xuiProxyXge();
	ret=xuiCreate(&a->pContext); if(ret!=XUI_OK) return ret;
	ret=xuiSetProxy(a->pContext,&a->tProxy); if(ret!=XUI_OK) return ret;
	(void)xuiInputViewport(a->pContext,(float)ANT_W,(float)ANT_H);
	if(ant_resize_target(a,ANT_W,ANT_H)!=XUI_OK) return XUI_ERROR;
	if(a->tProxy.fontLoadFile(&a->tProxy,&a->pFont,font,13.0f,XUI_FONT_FORMAT_TTF)!=XUI_OK){
		font="C:\\Windows\\Fonts\\arial.ttf";
		if(a->tProxy.fontLoadFile(&a->tProxy,&a->pFont,font,13.0f,XUI_FONT_FORMAT_TTF)!=XUI_OK) return XUI_ERROR;
	}
	(void)xuiSetDefaultFont(a->pContext,a->pFont);

	ret=xuiWidgetCreate(a->pContext,&a->pRoot); if(ret!=XUI_OK) return ret;
	memset(&policy,0,sizeof(policy)); policy.iSize=sizeof(policy);
	policy.iPolicy=XUI_CACHE_POLICY_SELF; policy.iFlags=XUI_CACHE_CLEAR_ON_UPDATE;
	(void)xuiWidgetSetRect(a->pRoot,(xui_rect_t){0,0,(float)ANT_W,(float)ANT_H});
	(void)xuiWidgetSetLayoutType(a->pRoot,XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(a->pRoot,&policy);
	(void)xuiSetRootWidget(a->pContext,a->pRoot);

	/* Menu bar */
	memset(&mbd,0,sizeof(mbd)); mbd.iSize=sizeof(mbd); mbd.pFont=a->pFont;
	(void)xuiMenuBarCreate(a->pContext,&a->pMenuBar,&mbd);
	(void)xuiWidgetSetRect(a->pMenuBar,(xui_rect_t){0,0,(float)ANT_W,ANT_MENU_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pMenuBar);
	memset(&md,0,sizeof(md)); md.iSize=sizeof(md); md.pOwner=a->pMenuBar; md.pFont=a->pFont;
	(void)xuiMenuCreate(a->pContext,&a->pFileMenu,&md);
	(void)xuiMenuCreate(a->pContext,&a->pHelpMenu,&md);
	memset(fileI,0,sizeof(fileI));
	fileI[0]=(xui_menu_item_t){"播放","Space",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,ANT_CMD_PLAY,0,NULL,NULL};
	fileI[1]=(xui_menu_item_t){"停止",NULL,XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,ANT_CMD_STOP,0,NULL,NULL};
	fileI[2]=(xui_menu_item_t){"添加帧","Ctrl+A",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,ANT_CMD_ADD_FRAME,0,NULL,NULL};
	fileI[3]=(xui_menu_item_t){"删除帧","Del",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,ANT_CMD_DEL_FRAME,0,NULL,NULL};
	fileI[4]=(xui_menu_item_t){NULL,NULL,XUI_MENU_ITEM_SEPARATOR,0,0,0,NULL,NULL};
	fileI[5]=(xui_menu_item_t){"退出","Alt+F4",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,ANT_CMD_EXIT,0,NULL,NULL};
	helpI[0]=(xui_menu_item_t){"关于",NULL,XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,ANT_CMD_ABOUT,0,NULL,NULL};
	(void)xuiMenuSetItems(a->pFileMenu,fileI,6);
	(void)xuiMenuSetItems(a->pHelpMenu,helpI,1);
	(void)xuiMenuSetSelect(a->pFileMenu,ant_menu_select,a);
	(void)xuiMenuSetSelect(a->pHelpMenu,ant_menu_select,a);
	mbI[0]=(xui_menubar_item_t){"编辑",XUI_MENUBAR_ITEM_ENABLED,0,'E',a->pFileMenu,{0,0,0,0},NULL};
	mbI[1]=(xui_menubar_item_t){"帮助",XUI_MENUBAR_ITEM_ENABLED,0,'H',a->pHelpMenu,{0,0,0,0},NULL};
	(void)xuiMenuBarSetItems(a->pMenuBar,mbI,2);
	(void)xuiMenuBarSetSelect(a->pMenuBar,ant_menu_select,a);

	/* Toolbar */
	memset(&tbd,0,sizeof(tbd)); tbd.iSize=sizeof(tbd); tbd.pFont=a->pFont;
	memset(tbI,0,sizeof(tbI));
	tbI[0]=(xui_toolbar_item_t){"播放","Play",XUI_TOOLBAR_ITEM_BUTTON,0,ANT_CMD_PLAY,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[1]=(xui_toolbar_item_t){"停止","Stop",XUI_TOOLBAR_ITEM_BUTTON,0,ANT_CMD_STOP,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[2]=(xui_toolbar_item_t){"+帧","Add frame",XUI_TOOLBAR_ITEM_BUTTON,0,ANT_CMD_ADD_FRAME,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[3]=(xui_toolbar_item_t){"-帧","Del frame",XUI_TOOLBAR_ITEM_BUTTON,0,ANT_CMD_DEL_FRAME,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[4]=(xui_toolbar_item_t){"导出","Export",XUI_TOOLBAR_ITEM_BUTTON,0,ANT_CMD_EXPORT,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbd.pItems=tbI; tbd.iItemCount=5;
	(void)xuiToolbarCreate(a->pContext,&a->pToolbar,&tbd);
	(void)xuiWidgetSetRect(a->pToolbar,(xui_rect_t){0,ANT_MENU_H,(float)ANT_W,ANT_TOOLBAR_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pToolbar);
	(void)xuiToolbarSetSelect(a->pToolbar,ant_toolbar_select,a);

	/* Status bar */
	memset(&sbd,0,sizeof(sbd)); sbd.iSize=sizeof(sbd); sbd.pFont=a->pFont;
	(void)xuiStatusBarCreate(a->pContext,&a->pStatus,&sbd);
	(void)xuiWidgetSetRect(a->pStatus,(xui_rect_t){0,(float)ANT_H-ANT_STATUS_H,(float)ANT_W,ANT_STATUS_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pStatus);
	(void)xuiStatusBarAddText(a->pStatus,XUI_STATUSBAR_SECTION_LEFT,"就绪",200.0f,0,0);
	(void)xuiStatusBarAddFlexibleSpacer(a->pStatus,XUI_STATUSBAR_SECTION_LEFT,1.0f);

	/* Dock: canvas (top) + timeline (bottom) */
	memset(&dd,0,sizeof(dd)); dd.iSize=sizeof(dd); dd.pFont=a->pFont;
	(void)xuiDockPanelCreate(a->pContext,&a->pDock,&dd);
	(void)xuiWidgetSetRect(a->pDock,(xui_rect_t){0,ANT_MENU_H+ANT_TOOLBAR_H,(float)ANT_W,(float)ANT_H-ANT_MENU_H-ANT_TOOLBAR_H-ANT_STATUS_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pDock);
	/* Canvas */
	(void)xuiWidgetCreate(a->pContext,&a->pCanvasWidget);
	(void)xuiWidgetSetCacheRenderCallback(a->pCanvasWidget,AnimTimelineCanvasRender,a);
	(void)xuiDockPanelAddWindow(a->pDock,"预览",a->pCanvasWidget,&win);
	(void)xuiDockPanelDockWindow(a->pDock,win,XUI_DOCK_PANEL_REGION_DOCUMENT,XUI_DOCK_PANEL_SIDE_FILL,0.0f,NULL);
	/* Timeline */
	(void)xuiWidgetCreate(a->pContext,&a->pTimelineWidget);
	(void)xuiWidgetSetCacheRenderCallback(a->pTimelineWidget,AnimTimelineTimelineRender,a);
	(void)xuiDockPanelAddWindow(a->pDock,"时间轴",a->pTimelineWidget,&win);
	(void)xuiDockPanelDockWindow(a->pDock,win,XUI_DOCK_PANEL_REGION_DOCUMENT,XUI_DOCK_PANEL_SIDE_BOTTOM,0.3f,NULL);

	a->bCreateOK=1;
	return XUI_OK;
}

void AnimTimelineDestroy(ant_app_t* a){
	if(!a) return;
	if(a->pTarget){a->tProxy.surfaceDestroy(&a->tProxy,a->pTarget); a->pTarget=NULL;}
	if(a->pFont){a->tProxy.fontDestroy(&a->tProxy,a->pFont); a->pFont=NULL;}
	if(a->pContext){xuiDestroy(a->pContext); a->pContext=NULL;}
}

int AnimTimelineFrame(void* pUser){
	ant_app_t* a=(ant_app_t*)pUser;
	xui_rect_i_t full; xui_rect_t src,dst; int ret;
	float dt;
	if(!a) return XGE_ERROR;
	ret=xgeBegin(); if(ret!=XGE_OK) return ret;
	(void)AnimTimelineSyncViewport(a);
	(void)AnimTimelineHandleInput(a);
	(void)xuiDispatchPendingEvents(a->pContext);
	(void)xuiLayout(a->pContext);
	dt=xgeGetDelta();
	AnimTimelinePlayUpdate(a,dt);
	(void)xuiUpdate(a->pContext,dt);
	if(a->pCanvasWidget)(void)xuiWidgetInvalidate(a->pCanvasWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
	if(a->pTimelineWidget)(void)xuiWidgetInvalidate(a->pTimelineWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
	(void)a->tProxy.surfaceClear(&a->tProxy,a->pTarget,XUI_COLOR_RGBA(28,28,38,255));
	full=(xui_rect_i_t){0,0,a->iTargetW,a->iTargetH};
	(void)xuiRender(a->pContext,a->pTarget,&full,1);
	a->bRenderOK=1;
	xgeClear(XUI_COLOR_RGBA(12,12,20,255));
	src=(xui_rect_t){0,0,(float)a->iTargetW,(float)a->iTargetH}; dst=src;
	ret=a->tProxy.surfaceDraw(&a->tProxy,a->pTarget,src,dst,XUI_COLOR_WHITE,XUI_SURFACE_DRAW_SCREEN_SPACE);
	if(ret==XUI_OK) ret=xgeEnd();
	if(ret!=XGE_OK) return ret;
	a->iFrame++;
	if(a->iFrameLimit>0 && a->iFrame>=a->iFrameLimit){
		printf("animtimeline_xui2 final-summary frames=%d create=%d render=%d layers=%d totalTicks=%d playing=%d\n",
			a->iFrame,a->bCreateOK,a->bRenderOK,a->iLayerCount,a->iTotalTicks,a->bPlaying);
		xgeQuit();
	}
	return XGE_OK;
}

/* ─── Demo data ─── */
static void ant_init_demo(ant_app_t* a){
	/* Layer 0: body */
	strcpy(a->arrLayers[0].sName,"Body");
	a->arrLayers[0].bVisible=1;
	a->arrLayers[0].iFrameCount=4;
	a->arrLayers[0].arrFrames[0]=(ant_frame_t){10,XUI_COLOR_RGBA(80,160,255,200),1};
	a->arrLayers[0].arrFrames[1]=(ant_frame_t){8,XUI_COLOR_RGBA(100,180,255,200),0};
	a->arrLayers[0].arrFrames[2]=(ant_frame_t){10,XUI_COLOR_RGBA(80,160,255,200),1};
	a->arrLayers[0].arrFrames[3]=(ant_frame_t){8,XUI_COLOR_RGBA(60,140,235,200),0};
	/* Layer 1: arms */
	strcpy(a->arrLayers[1].sName,"Arms");
	a->arrLayers[1].bVisible=1;
	a->arrLayers[1].iFrameCount=3;
	a->arrLayers[1].arrFrames[0]=(ant_frame_t){12,XUI_COLOR_RGBA(255,180,60,200),1};
	a->arrLayers[1].arrFrames[1]=(ant_frame_t){12,XUI_COLOR_RGBA(255,200,80,200),1};
	a->arrLayers[1].arrFrames[2]=(ant_frame_t){12,XUI_COLOR_RGBA(255,160,40,200),1};
	/* Layer 2: effects */
	strcpy(a->arrLayers[2].sName,"FX");
	a->arrLayers[2].bVisible=1;
	a->arrLayers[2].iFrameCount=2;
	a->arrLayers[2].arrFrames[0]=(ant_frame_t){18,XUI_COLOR_RGBA(255,100,100,150),1};
	a->arrLayers[2].arrFrames[1]=(ant_frame_t){18,XUI_COLOR_RGBA(255,60,60,100),0};
	a->iLayerCount=3;
	a->iCurLayer=0; a->iCurFrame=0;
	a->iFPS=12; a->bPlaying=1; a->fPlayTime=0;
	AnimTimelineRecalcDuration(a);
}

int main(int argc, char** argv){
	static ant_app_t app;
	xge_desc_t desc;
	int i, ret;
	ant_set_wd();
	memset(&app,0,sizeof(app));
	ant_init_demo(&app);
	app.iFrameLimit=ant_arg_int(getenv("XGE_ANIMTIMELINE_FRAMES"),0);
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"--frames")==0&&i+1<argc) app.iFrameLimit=ant_arg_int(argv[++i],app.iFrameLimit);
		else if(strncmp(argv[i],"--frames=",9)==0) app.iFrameLimit=ant_arg_int(argv[i]+9,app.iFrameLimit);
	}
	memset(&desc,0,sizeof(desc));
	desc.sTitle="AnimTimeline"; desc.iWidth=ANT_W; desc.iHeight=ANT_H;
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC;
	desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc);
	if(ret!=XGE_OK){printf("animtimeline_xui2: xgeInit failed: %d\n",ret); return 1;}
	ret=AnimTimelineCreateUI(&app);
	if(ret!=XUI_OK){printf("animtimeline_xui2: create ui failed: %d\n",ret); AnimTimelineDestroy(&app); xgeUnit(); return 1;}
	ret=xgeRun(AnimTimelineFrame,&app);
	AnimTimelineDestroy(&app); xgeUnit();
	return (ret==XGE_OK&&app.bCreateOK&&app.bRenderOK)?0:1;
}
