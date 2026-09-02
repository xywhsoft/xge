#include "../../xge.h"
#include "../audit_render_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AUDIT_W 900
#define AUDIT_H 700
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct { xge_render_target_t tTarget; char sCapturePath[260]; int bCaptureDone; int iFrame; int iMaxFrames; } audit_demo_t;
static xge_rect_t rectf(float x, float y, float w, float h) { xge_rect_t r; r.fX=x; r.fY=y; r.fW=w; r.fH=h; return r; }
static int parse_args(audit_demo_t* d, int argc, char** argv) { int i; for(i=1;i<argc;i++){if(strcmp(argv[i],"--capture")==0&&i+1<argc)snprintf(d->sCapturePath,260,"%s",argv[++i]);else if(strncmp(argv[i],"--capture=",10)==0)snprintf(d->sCapturePath,260,"%s",argv[i]+10);else if(strcmp(argv[i],"--frames")==0&&i+1<argc)d->iMaxFrames=atoi(argv[++i]);else if(strncmp(argv[i],"--frames=",9)==0)d->iMaxFrames=atoi(argv[i]+9);} return XGE_OK; }
static int capture(audit_demo_t* d) { unsigned char*px;int s,r; if(!d||!d->sCapturePath[0]||d->bCaptureDone)return XGE_OK; s=AUDIT_W*4; px=(unsigned char*)malloc((size_t)s*AUDIT_H); if(!px)return XGE_ERROR_OUT_OF_MEMORY; r=xgeRenderTargetReadPixels(&d->tTarget,px,s); if(r==XGE_OK)r=xgeImageSavePNG(d->sCapturePath,AUDIT_W,AUDIT_H,px,s); free(px); if(r==XGE_OK){d->bCaptureDone=1;printf("capture saved: %s\n",d->sCapturePath);} return r; }

static void marker(float x, float y) { xgeShapeRectFill(rectf(x,y,4,4), XGE_COLOR_RGBA(255,255,255,100)); }

/* Draw a grid of shapes in world space for camera testing */
static void draw_world_content(void)
{
	int i;
	/* Grid lines */
	for ( i = 0; i <= 10; i++ ) {
		float pos = (float)(i * 40);
		xgeShapeLine(pos, 0, pos, 400, 1.0f, XGE_COLOR_RGBA(60,70,80,255));
		xgeShapeLine(0, pos, 400, pos, 1.0f, XGE_COLOR_RGBA(60,70,80,255));
	}
	/* Colored shapes at known positions */
	xgeShapeRectFill(rectf(0, 0, 40, 40), XGE_COLOR_RGBA(255,0,0,255));
	xgeShapeRectFill(rectf(160, 0, 40, 40), XGE_COLOR_RGBA(0,255,0,255));
	xgeShapeRectFill(rectf(360, 0, 40, 40), XGE_COLOR_RGBA(0,0,255,255));
	xgeShapeCircleFill(200, 200, 30, XGE_COLOR_RGBA(255,255,0,255));
	xgeShapeRectFill(rectf(0, 360, 40, 40), XGE_COLOR_RGBA(255,0,255,255));
	xgeShapeRectFill(rectf(360, 360, 40, 40), XGE_COLOR_RGBA(0,255,255,255));
}

static int frame(void* user)
{
	audit_demo_t* d = (audit_demo_t*)user;
	xge_pass_t p; int ret;
	xge_camera_t cam;

	if(!d) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin(); if(ret!=XGE_OK) return ret;
	xgePassInit(&p, &d->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&p); if(ret!=XGE_OK) return ret;

	/* Section 1: Default orthographic camera (identity) */
	marker(10, 5);
	cam = xgeCameraDefault(400, 400);
	xgeCameraSet(&cam);
	xgeViewportSet(rectf(0, 0, 400, 300));
	draw_world_content();
	xgeViewportClear();

	/* Section 2: Camera with translation (pan right/down) */
	marker(420, 5);
	cam = xgeCameraDefault(400, 400);
	cam.tPosition.fX = -100;
	cam.tPosition.fY = -100;
	xgeCameraSet(&cam);
	xgeViewportSet(rectf(420, 0, 400, 300));
	draw_world_content();
	xgeViewportClear();

	/* Section 3: Camera with zoom (scale 2x) */
	marker(10, 320);
	cam = xgeCameraDefault(400, 400);
	cam.tScale.fX = 2.0f;
	cam.tScale.fY = 2.0f;
	xgeCameraSet(&cam);
	xgeViewportSet(rectf(0, 320, 400, 300));
	draw_world_content();
	xgeViewportClear();

	/* Section 4: Camera with rotation (30 degrees) */
	marker(420, 320);
	cam = xgeCameraDefault(400, 400);
	cam.fRotation = 30.0f;
	cam.tPosition.fX = -200;
	cam.tPosition.fY = -200;
	xgeCameraSet(&cam);
	xgeViewportSet(rectf(420, 320, 400, 300));
	draw_world_content();
	xgeViewportClear();

	/* Section 5: Perspective camera */
	marker(10, 640);
	cam = xgeCameraPerspective(400, 300, 60.0f, 0.1f, 1000.0f);
	cam.tPosition.fX = -200;
	cam.tPosition.fY = -200;
	xgeCameraSet(&cam);
	xgeViewportSet(rectf(0, 630, 440, 60));
	/* Simple content for perspective */
	xgeShapeRectFill(rectf(100, 100, 200, 200), XGE_COLOR_RGBA(255,128,0,255));
	xgeShapeCircleFill(200, 200, 50, XGE_COLOR_RGBA(0,200,255,255));
	xgeViewportClear();

	/* Section 6: WorldToScreen / ScreenToWorld verification */
	marker(460, 640);
	{
		xge_vec2_t worldPt, screenPt, backPt;
		cam = xgeCameraDefault(900, 700);
		xgeCameraSet(&cam);
		worldPt.fX = 450; worldPt.fY = 350;
		screenPt = xgeWorldToScreen(worldPt);
		backPt = xgeScreenToWorld(screenPt);
		/* Draw at screen position */
		xgeShapeCircleFill(screenPt.fX, screenPt.fY, 10, XGE_COLOR_RGBA(255,255,0,255));
		/* Draw crosshair at back-projected position */
		xgeShapeLine(backPt.fX-15, backPt.fY, backPt.fX+15, backPt.fY, 2.0f, XGE_COLOR_RGBA(0,255,0,255));
		xgeShapeLine(backPt.fX, backPt.fY-15, backPt.fX, backPt.fY+15, 2.0f, XGE_COLOR_RGBA(0,255,0,255));
	}

	/* Reset camera */
	cam = xgeCameraDefault((float)AUDIT_W, (float)AUDIT_H);
	xgeCameraSet(&cam);

	ret = xgePassEnd(&p); if(ret!=XGE_OK) return ret;
	ret = capture(d); if(ret!=XGE_OK) return ret;
	ret = auditPresentRenderTarget(&d->tTarget, AUDIT_W, AUDIT_H, BG_COLOR); if(ret!=XGE_OK) return ret;
	ret = xgeEnd(); if(ret!=XGE_OK) return ret;
	d->iFrame++;
	if(d->bCaptureDone||((d->iMaxFrames>0)&&(d->iFrame>=d->iMaxFrames))) xgeQuit();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	audit_demo_t t; xge_desc_t desc; int ret;
	memset(&t,0,sizeof(t)); parse_args(&t,argc,argv);
	memset(&desc,0,sizeof(desc));
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_camera_viewport";
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC; desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
