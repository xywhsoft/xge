#include "spk_app.h"
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

static int spk_arg_int(const char* s, int def) { int v; if (!s||!s[0]) return def; v=atoi(s); return v>0?v:def; }
static void spk_set_wd(void) {
#ifdef _WIN32
	char p[MAX_PATH]; char* sl;
	if (!GetModuleFileNameA(NULL,p,sizeof(p))) return;
	p[sizeof(p)-1]=0; sl=strrchr(p,'\\'); if(!sl) sl=strrchr(p,'/');
	if(sl){*sl=0; SetCurrentDirectoryA(p);}
#endif
}

static uint32_t spk_buttons(void) {
	uint32_t b=0;
	if(xgeMouseDown(XGE_MOUSE_LEFT)) b|=XUI_POINTER_BUTTON_LEFT;
	if(xgeMouseDown(XGE_MOUSE_RIGHT)) b|=XUI_POINTER_BUTTON_RIGHT;
	return b;
}
static uint32_t spk_mods(void) {
	uint32_t m=0;
	if(xgeKeyDown(XGE_KEY_LEFT_SHIFT)||xgeKeyDown(XGE_KEY_RIGHT_SHIFT)) m|=XUI_MOD_SHIFT;
	if(xgeKeyDown(XGE_KEY_LEFT_CONTROL)||xgeKeyDown(XGE_KEY_RIGHT_CONTROL)) m|=XUI_MOD_CTRL;
	if(xgeKeyDown(XGE_KEY_LEFT_ALT)||xgeKeyDown(XGE_KEY_RIGHT_ALT)) m|=XUI_MOD_ALT;
	return m;
}

static int spk_resize_target(spk_app_t* a, int w, int h) {
	xui_surface_desc_t sd;
	if(w<=0) w=SPK_W; if(h<=0) h=SPK_H;
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

int SpritePackerSyncViewport(spk_app_t* a) {
	int w=xgeGetWidth(), h=xgeGetHeight();
	float fw,fh,cy,ch;
	if(w<=0) w=SPK_W; if(h<=0) h=SPK_H;
	fw=(float)w; fh=(float)h;
	if(spk_resize_target(a,w,h)!=XUI_OK) return XUI_ERROR;
	(void)xuiInputViewport(a->pContext,fw,fh);
	(void)xuiSetViewportSize(a->pContext,fw,fh);
	if(a->pRoot)(void)xuiWidgetSetRect(a->pRoot,(xui_rect_t){0,0,fw,fh});
	if(a->pMenuBar)(void)xuiWidgetSetRect(a->pMenuBar,(xui_rect_t){0,0,fw,SPK_MENU_H});
	cy=SPK_MENU_H+SPK_TOOLBAR_H; ch=fh-cy-SPK_STATUS_H;
	if(ch<1.0f) ch=1.0f;
	if(a->pToolbar)(void)xuiWidgetSetRect(a->pToolbar,(xui_rect_t){0,SPK_MENU_H,fw,SPK_TOOLBAR_H});
	if(a->pDock)(void)xuiWidgetSetRect(a->pDock,(xui_rect_t){0,cy,fw,ch});
	if(a->pStatus)(void)xuiWidgetSetRect(a->pStatus,(xui_rect_t){0,fh-SPK_STATUS_H,fw,SPK_STATUS_H});
	return XUI_OK;
}

int SpritePackerHandleInput(spk_app_t* a) {
	static const int keys[]={XGE_KEY_ENTER,XGE_KEY_TAB,XGE_KEY_SPACE,XGE_KEY_BACKSPACE,XGE_KEY_DELETE,XGE_KEY_LEFT,XGE_KEY_RIGHT,XGE_KEY_UP,XGE_KEY_DOWN,XGE_KEY_ESCAPE};
	float x,y,wx,wy; uint32_t buttons,pressed,released,mods; int i;
	mods=spk_mods();
	(void)xuiInputSetModifiers(a->pContext,mods);
	if(xgeKeyPressed(XGE_KEY_ESCAPE)) xgeQuit();
	for(i=0;i<(int)(sizeof(keys)/sizeof(keys[0]));i++){
		if(xgeKeyPressed(keys[i]))(void)xuiInputKeyDown(a->pContext,keys[i],mods);
		if(xgeKeyReleased(keys[i]))(void)xuiInputKeyUp(a->pContext,keys[i],mods);
	}
	while((mods=xgeTextGet())!=0)(void)xuiInputText(a->pContext,mods);
	xgeMouseGet(&x,&y); xgeMouseGetWheel(&wx,&wy);
	buttons=spk_buttons();
	if(!a->bMouse||x!=a->fMouseX||y!=a->fMouseY||buttons!=a->iButtons)
		(void)xuiInputPointerMove(a->pContext,x,y,buttons);
	if(wx!=0||wy!=0)(void)xuiInputPointerWheel(a->pContext,x,y,wx,wy,buttons);
	pressed=buttons&~a->iButtons; released=a->iButtons&~buttons;
	if(pressed&XUI_POINTER_BUTTON_LEFT)(void)xuiInputPointerDown(a->pContext,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
	if(released&XUI_POINTER_BUTTON_LEFT)(void)xuiInputPointerUp(a->pContext,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
	a->bMouse=1; a->fMouseX=x; a->fMouseY=y; a->iButtons=buttons;
	return XUI_OK;
}

/* ─── Shelf packing algorithm ─── */
void SpritePackerPack(spk_app_t* a) {
	int i, shelfX, shelfY, shelfH, atlasW;
	/* Sort by height descending (simple insertion sort) */
	for(i=0;i<a->iImageCount-1;i++){
		int j=i+1;
		while(j>0 && a->arrImages[j].iH > a->arrImages[j-1].iH){
			spk_image_t tmp=a->arrImages[j]; a->arrImages[j]=a->arrImages[j-1]; a->arrImages[j-1]=tmp; j--;
		}
	}
	atlasW = a->iAtlasW > 0 ? a->iAtlasW : 1024;
	shelfX=0; shelfY=0; shelfH=0;
	for(i=0;i<a->iImageCount;i++){
		spk_image_t* img=&a->arrImages[i];
		if(shelfX + img->iW + a->iPadding > atlasW){
			shelfX=0; shelfY += shelfH + a->iPadding; shelfH=0;
		}
		img->iX=shelfX; img->iY=shelfY; img->bPacked=1;
		shelfX += img->iW + a->iPadding;
		if(img->iH > shelfH) shelfH=img->iH;
	}
	a->iAtlasH = shelfY + shelfH;
	/* Round up to power of 2 */
	{int p=1; while(p<a->iAtlasH) p<<=1; a->iAtlasH=p;}
	a->bPacked=1;
}

/* ─── Export ─── */
int SpritePackerExport(spk_app_t* a, const char* sPath) {
	FILE* fp; int i;
	if(!a||!sPath) return XUI_ERROR;
	fp=fopen(sPath,"w");
	if(!fp) return XUI_ERROR;
	fprintf(fp,"{\n  \"atlas\": { \"width\": %d, \"height\": %d },\n", a->iAtlasW, a->iAtlasH);
	fprintf(fp,"  \"frames\": [\n");
	for(i=0;i<a->iImageCount;i++){
		spk_image_t* img=&a->arrImages[i];
		fprintf(fp,"    { \"name\": \"%s\", \"x\": %d, \"y\": %d, \"w\": %d, \"h\": %d }%s\n",
			img->sName, img->iX, img->iY, img->iW, img->iH, (i<a->iImageCount-1)?",":"");
	}
	fprintf(fp,"  ]\n}\n");
	fclose(fp);
	return XUI_OK;
}

/* ─── Preview render ─── */
int SpritePackerPreviewRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser) {
	spk_app_t* a=(spk_app_t*)pUser;
	xui_proxy_t proxy; xui_rect_t r;
	float scale, offX, offY;
	int i;
	(void)iStateId;
	if(!a||!pDraw) return XUI_ERROR_INVALID_ARGUMENT;
	proxy=xuiProxyXge();
	r=xuiWidgetGetContentRect(pWidget);
	(void)proxy.drawRectFill(&proxy,pDraw,r,XUI_COLOR_RGBA(30,30,40,255));
	if(!a->bPacked||a->iImageCount==0) return XUI_OK;
	/* Fit atlas in view */
	scale=(r.fW-40.0f)/(float)a->iAtlasW;
	if((r.fH-40.0f)/(float)a->iAtlasH < scale) scale=(r.fH-40.0f)/(float)a->iAtlasH;
	if(scale>1.0f) scale=1.0f;
	offX=r.fX+(r.fW-(float)a->iAtlasW*scale)*0.5f;
	offY=r.fY+(r.fH-(float)a->iAtlasH*scale)*0.5f;
	/* Atlas background */
	(void)proxy.drawRectFill(&proxy,pDraw,(xui_rect_t){offX,offY,(float)a->iAtlasW*scale,(float)a->iAtlasH*scale},XUI_COLOR_RGBA(20,20,28,255));
	/* Draw packed rects */
	for(i=0;i<a->iImageCount;i++){
		spk_image_t* img=&a->arrImages[i];
		uint32_t col=XUI_COLOR_RGBA(60+i*7%180, 100+i*13%140, 200-i*5%100, 200);
		if(!img->bPacked) continue;
		(void)proxy.drawRectFill(&proxy,pDraw,(xui_rect_t){offX+img->iX*scale, offY+img->iY*scale, img->iW*scale, img->iH*scale},col);
		(void)proxy.drawRectStroke(&proxy,pDraw,(xui_rect_t){offX+img->iX*scale, offY+img->iY*scale, img->iW*scale, img->iH*scale},1.0f,XUI_COLOR_RGBA(200,220,255,180));
	}
	return XUI_OK;
}

/* ─── Menu/toolbar callbacks ─── */
static void spk_menu_select(xui_widget_t* w, int idx, int val, void* user) {
	spk_app_t* a=(spk_app_t*)user; (void)w;(void)idx;
	if(!a) return;
	switch(val){
	case SPK_CMD_EXIT: xgeQuit(); break;
	case SPK_CMD_PACK:
		SpritePackerPack(a);
		if(a->pPreviewWidget)(void)xuiWidgetInvalidate(a->pPreviewWidget,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER);
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Packed");
		break;
	case SPK_CMD_EXPORT:
		SpritePackerExport(a,"output/atlas.xson");
		(void)xuiStatusBarSetItemText(a->pStatus,0,"Exported");
		break;
	case SPK_CMD_ABOUT:
		(void)xuiStatusBarSetItemText(a->pStatus,0,"SpritePacker v1.0 - XUI2");
		break;
	}
}
static void spk_toolbar_select(xui_widget_t* w, int idx, int val, void* user) {
	(void)w;(void)idx;
	spk_menu_select(NULL,0,val,user);
}

/* ─── Create UI ─── */
int SpritePackerCreateUI(spk_app_t* a) {
	xui_cache_policy_t policy;
	xui_menubar_desc_t mbd; xui_menu_desc_t md;
	xui_menubar_item_t mbI[2]; xui_menu_item_t fileI[4], helpI[1];
	xui_toolbar_desc_t tbd; xui_toolbar_item_t tbI[2];
	xui_statusbar_desc_t sbd; xui_dock_panel_desc_t dd;
	const char* font="C:\\Windows\\Fonts\\msyh.ttc";
	int ret, win;

	a->tProxy=xuiProxyXge();
	ret=xuiCreate(&a->pContext); if(ret!=XUI_OK) return ret;
	ret=xuiSetProxy(a->pContext,&a->tProxy); if(ret!=XUI_OK) return ret;
	(void)xuiInputViewport(a->pContext,(float)SPK_W,(float)SPK_H);
	if(spk_resize_target(a,SPK_W,SPK_H)!=XUI_OK) return XUI_ERROR;
	if(a->tProxy.fontLoadFile(&a->tProxy,&a->pFont,font,13.0f,XUI_FONT_FORMAT_TTF)!=XUI_OK){
		font="C:\\Windows\\Fonts\\arial.ttf";
		if(a->tProxy.fontLoadFile(&a->tProxy,&a->pFont,font,13.0f,XUI_FONT_FORMAT_TTF)!=XUI_OK) return XUI_ERROR;
	}
	(void)xuiSetDefaultFont(a->pContext,a->pFont);

	ret=xuiWidgetCreate(a->pContext,&a->pRoot); if(ret!=XUI_OK) return ret;
	memset(&policy,0,sizeof(policy)); policy.iSize=sizeof(policy);
	policy.iPolicy=XUI_CACHE_POLICY_SELF; policy.iFlags=XUI_CACHE_CLEAR_ON_UPDATE;
	(void)xuiWidgetSetRect(a->pRoot,(xui_rect_t){0,0,(float)SPK_W,(float)SPK_H});
	(void)xuiWidgetSetLayoutType(a->pRoot,XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(a->pRoot,&policy);
	(void)xuiSetRootWidget(a->pContext,a->pRoot);

	/* Menu */
	memset(&mbd,0,sizeof(mbd)); mbd.iSize=sizeof(mbd); mbd.pFont=a->pFont;
	(void)xuiMenuBarCreate(a->pContext,&a->pMenuBar,&mbd);
	(void)xuiWidgetSetRect(a->pMenuBar,(xui_rect_t){0,0,(float)SPK_W,SPK_MENU_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pMenuBar);
	memset(&md,0,sizeof(md)); md.iSize=sizeof(md); md.pOwner=a->pMenuBar; md.pFont=a->pFont;
	(void)xuiMenuCreate(a->pContext,&a->pFileMenu,&md);
	(void)xuiMenuCreate(a->pContext,&a->pHelpMenu,&md);
	memset(fileI,0,sizeof(fileI));
	fileI[0]=(xui_menu_item_t){"打包","Ctrl+P",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,SPK_CMD_PACK,0,NULL,NULL};
	fileI[1]=(xui_menu_item_t){"导出","Ctrl+E",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,SPK_CMD_EXPORT,0,NULL,NULL};
	fileI[2]=(xui_menu_item_t){NULL,NULL,XUI_MENU_ITEM_SEPARATOR,0,0,0,NULL,NULL};
	fileI[3]=(xui_menu_item_t){"退出","Alt+F4",XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,SPK_CMD_EXIT,0,NULL,NULL};
	helpI[0]=(xui_menu_item_t){"关于",NULL,XUI_MENU_ITEM_NORMAL,XUI_MENU_ITEM_ENABLED,SPK_CMD_ABOUT,0,NULL,NULL};
	(void)xuiMenuSetItems(a->pFileMenu,fileI,4);
	(void)xuiMenuSetItems(a->pHelpMenu,helpI,1);
	(void)xuiMenuSetSelect(a->pFileMenu,spk_menu_select,a);
	(void)xuiMenuSetSelect(a->pHelpMenu,spk_menu_select,a);
	mbI[0]=(xui_menubar_item_t){"文件",XUI_MENUBAR_ITEM_ENABLED,0,'F',a->pFileMenu,{0,0,0,0},NULL};
	mbI[1]=(xui_menubar_item_t){"帮助",XUI_MENUBAR_ITEM_ENABLED,0,'H',a->pHelpMenu,{0,0,0,0},NULL};
	(void)xuiMenuBarSetItems(a->pMenuBar,mbI,2);
	(void)xuiMenuBarSetSelect(a->pMenuBar,spk_menu_select,a);

	/* Toolbar */
	memset(&tbd,0,sizeof(tbd)); tbd.iSize=sizeof(tbd); tbd.pFont=a->pFont;
	memset(tbI,0,sizeof(tbI));
	tbI[0]=(xui_toolbar_item_t){"打包","Pack all",XUI_TOOLBAR_ITEM_BUTTON,0,SPK_CMD_PACK,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbI[1]=(xui_toolbar_item_t){"导出","Export atlas",XUI_TOOLBAR_ITEM_BUTTON,0,SPK_CMD_EXPORT,0,NULL,{0,0,0,0},{0,0,0,0},NULL};
	tbd.pItems=tbI; tbd.iItemCount=2;
	(void)xuiToolbarCreate(a->pContext,&a->pToolbar,&tbd);
	(void)xuiWidgetSetRect(a->pToolbar,(xui_rect_t){0,SPK_MENU_H,(float)SPK_W,SPK_TOOLBAR_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pToolbar);
	(void)xuiToolbarSetSelect(a->pToolbar,spk_toolbar_select,a);

	/* Status */
	memset(&sbd,0,sizeof(sbd)); sbd.iSize=sizeof(sbd); sbd.pFont=a->pFont;
	(void)xuiStatusBarCreate(a->pContext,&a->pStatus,&sbd);
	(void)xuiWidgetSetRect(a->pStatus,(xui_rect_t){0,(float)SPK_H-SPK_STATUS_H,(float)SPK_W,SPK_STATUS_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pStatus);
	(void)xuiStatusBarAddText(a->pStatus,XUI_STATUSBAR_SECTION_LEFT,"就绪",200.0f,0,0);
	(void)xuiStatusBarAddFlexibleSpacer(a->pStatus,XUI_STATUSBAR_SECTION_LEFT,1.0f);

	/* Dock + Preview */
	memset(&dd,0,sizeof(dd)); dd.iSize=sizeof(dd); dd.pFont=a->pFont;
	(void)xuiDockPanelCreate(a->pContext,&a->pDock,&dd);
	(void)xuiWidgetSetRect(a->pDock,(xui_rect_t){0,SPK_MENU_H+SPK_TOOLBAR_H,(float)SPK_W,(float)SPK_H-SPK_MENU_H-SPK_TOOLBAR_H-SPK_STATUS_H});
	(void)xuiWidgetAddChild(a->pRoot,a->pDock);
	(void)xuiWidgetCreate(a->pContext,&a->pPreviewWidget);
	(void)xuiWidgetSetCacheRenderCallback(a->pPreviewWidget,SpritePackerPreviewRender,a);
	(void)xuiDockPanelAddWindow(a->pDock,"图集预览",a->pPreviewWidget,&win);
	(void)xuiDockPanelDockWindow(a->pDock,win,XUI_DOCK_PANEL_REGION_DOCUMENT,XUI_DOCK_PANEL_SIDE_FILL,0.0f,NULL);

	a->bCreateOK=1;
	return XUI_OK;
}

void SpritePackerDestroy(spk_app_t* a) {
	if(!a) return;
	if(a->pTarget){a->tProxy.surfaceDestroy(&a->tProxy,a->pTarget); a->pTarget=NULL;}
	if(a->pFont){a->tProxy.fontDestroy(&a->tProxy,a->pFont); a->pFont=NULL;}
	if(a->pContext){xuiDestroy(a->pContext); a->pContext=NULL;}
}

int SpritePackerFrame(void* pUser) {
	spk_app_t* a=(spk_app_t*)pUser;
	xui_rect_i_t full; xui_rect_t src,dst; int ret;
	if(!a) return XGE_ERROR;
	ret=xgeBegin(); if(ret!=XGE_OK) return ret;
	(void)SpritePackerSyncViewport(a);
	(void)SpritePackerHandleInput(a);
	(void)xuiDispatchPendingEvents(a->pContext);
	(void)xuiLayout(a->pContext);
	(void)xuiUpdate(a->pContext,xgeGetDelta());
	(void)a->tProxy.surfaceClear(&a->tProxy,a->pTarget,XUI_COLOR_RGBA(240,244,248,255));
	full=(xui_rect_i_t){0,0,a->iTargetW,a->iTargetH};
	(void)xuiRender(a->pContext,a->pTarget,&full,1);
	a->bRenderOK=1;
	xgeClear(XUI_COLOR_RGBA(18,23,32,255));
	src=(xui_rect_t){0,0,(float)a->iTargetW,(float)a->iTargetH}; dst=src;
	ret=a->tProxy.surfaceDraw(&a->tProxy,a->pTarget,src,dst,XUI_COLOR_WHITE,XUI_SURFACE_DRAW_SCREEN_SPACE);
	if(ret==XUI_OK) ret=xgeEnd();
	if(ret!=XGE_OK) return ret;
	a->iFrame++;
	if(a->iFrameLimit>0 && a->iFrame>=a->iFrameLimit){
		printf("spritepacker_xui2 final-summary frames=%d create=%d render=%d images=%d atlas=%dx%d packed=%d\n",
			a->iFrame,a->bCreateOK,a->bRenderOK,a->iImageCount,a->iAtlasW,a->iAtlasH,a->bPacked);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv) {
	static spk_app_t app;
	xge_desc_t desc;
	int i, ret;
	spk_set_wd();
	memset(&app,0,sizeof(app));
	app.iAtlasW=1024; app.iAtlasH=1024; app.iPadding=2;
	/* Demo images */
	app.iImageCount=5;
	strcpy(app.arrImages[0].sName,"hero_idle"); app.arrImages[0].iW=64; app.arrImages[0].iH=64;
	strcpy(app.arrImages[1].sName,"hero_run"); app.arrImages[1].iW=64; app.arrImages[1].iH=64;
	strcpy(app.arrImages[2].sName,"sword"); app.arrImages[2].iW=32; app.arrImages[2].iH=32;
	strcpy(app.arrImages[3].sName,"shield"); app.arrImages[3].iW=48; app.arrImages[3].iH=48;
	strcpy(app.arrImages[4].sName,"potion"); app.arrImages[4].iW=16; app.arrImages[4].iH=24;
	app.iFrameLimit=spk_arg_int(getenv("XGE_SPRITEPACKER_FRAMES"),0);
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"--frames")==0&&i+1<argc) app.iFrameLimit=spk_arg_int(argv[++i],app.iFrameLimit);
		else if(strncmp(argv[i],"--frames=",9)==0) app.iFrameLimit=spk_arg_int(argv[i]+9,app.iFrameLimit);
	}
	memset(&desc,0,sizeof(desc));
	desc.sTitle="SpritePacker"; desc.iWidth=SPK_W; desc.iHeight=SPK_H;
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC;
	desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc);
	if(ret!=XGE_OK){printf("spritepacker_xui2: xgeInit failed: %d\n",ret); return 1;}
	SpritePackerPack(&app); /* auto-pack demo data */
	ret=SpritePackerCreateUI(&app);
	if(ret!=XUI_OK){printf("spritepacker_xui2: create ui failed: %d\n",ret); SpritePackerDestroy(&app); xgeUnit(); return 1;}
	ret=xgeRun(SpritePackerFrame,&app);
	SpritePackerDestroy(&app); xgeUnit();
	return (ret==XGE_OK&&app.bCreateOK&&app.bRenderOK)?0:1;
}
