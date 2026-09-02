#include "../../xge.h"
#include "../audit_render_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AUDIT_W 900
#define AUDIT_H 800
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct { xge_render_target_t tTarget; char sCapturePath[260]; int bCaptureDone; int iFrame; int iMaxFrames; } audit_demo_t;
static xge_rect_t rectf(float x, float y, float w, float h) { xge_rect_t r; r.fX=x; r.fY=y; r.fW=w; r.fH=h; return r; }
static int parse_args(audit_demo_t* d, int argc, char** argv) { int i; for(i=1;i<argc;i++){if(strcmp(argv[i],"--capture")==0&&i+1<argc)snprintf(d->sCapturePath,260,"%s",argv[++i]);else if(strncmp(argv[i],"--capture=",10)==0)snprintf(d->sCapturePath,260,"%s",argv[i]+10);else if(strcmp(argv[i],"--frames")==0&&i+1<argc)d->iMaxFrames=atoi(argv[++i]);else if(strncmp(argv[i],"--frames=",9)==0)d->iMaxFrames=atoi(argv[i]+9);} return XGE_OK; }
static int capture(audit_demo_t* d) { unsigned char*px;int s,r; if(!d||!d->sCapturePath[0]||d->bCaptureDone)return XGE_OK; s=AUDIT_W*4; px=(unsigned char*)malloc((size_t)s*AUDIT_H); if(!px)return XGE_ERROR_OUT_OF_MEMORY; r=xgeRenderTargetReadPixels(&d->tTarget,px,s); if(r==XGE_OK)r=xgeImageSavePNG(d->sCapturePath,AUDIT_W,AUDIT_H,px,s); free(px); if(r==XGE_OK){d->bCaptureDone=1;printf("capture saved: %s\n",d->sCapturePath);} return r; }

/* Section 1: Basic transforms */
static void draw_basic_transforms(float by)
{
	xge_shape_ex s;
	uint32_t col = XGE_COLOR_RGBA(100, 200, 255, 255);
	/* Identity (original) */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 30, by, 60, 40, 0, 0, 1);
	xgeShapeExFillColor(s, col);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	/* Translate */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 60, 40, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 150, 80, 255));
	xgeShapeExTransformTranslate(s, 150, by);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	/* Scale */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 60, 40, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(80, 255, 150, 255));
	xgeShapeExTransformTranslate(s, 300, by);
	xgeShapeExTransformScale(s, 1.5f, 0.7f);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	/* Rotate 45 degrees */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, -30, -20, 60, 40, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 80, 200, 255));
	xgeShapeExTransformTranslate(s, 500, by + 20);
	xgeShapeExTransformRotate(s, 0.7854f);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	/* Skew */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 60, 40, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(200, 255, 80, 255));
	xgeShapeExTransformTranslate(s, 620, by);
	xgeShapeExTransformSkew(s, 0.3f, 0.0f);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

/* Section 2: Nested scene transforms */
static void draw_nested_transforms(float by)
{
	xge_shape_ex_scene parent, child;
	xge_shape_ex s;

	xgeShapeExSceneCreate(&parent);
	xgeShapeExSceneCreate(&child);

	/* Child shape */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, -20, -15, 40, 30, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 200, 50, 255));
	xgeShapeExSceneAdd(child, s);
	xgeShapeExDestroy(s);

	/* Child rotated 30 degrees */
	xgeShapeExSceneTransformTranslate(child, 60, 30);
	xgeShapeExSceneTransformRotate(child, 0.5236f);

	/* Parent translated and scaled */
	xgeShapeExSceneTransformTranslate(parent, 50, by);
	xgeShapeExSceneTransformScale(parent, 2.0f, 2.0f);
	xgeShapeExSceneAddScene(parent, child);

	xgeShapeExSceneDraw(parent, 0.5f);
	xgeShapeExSceneDestroy(parent);

	/* Second nested: 3 levels */
	xgeShapeExSceneCreate(&parent);
	xgeShapeExSceneCreate(&child);
	xge_shape_ex_scene grandchild;
	xgeShapeExSceneCreate(&grandchild);

	xgeShapeExCreate(&s);
	xgeShapeExAppendCircle(s, 0, 0, 10, 10, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 80, 80, 255));
	xgeShapeExSceneAdd(grandchild, s);
	xgeShapeExDestroy(s);

	xgeShapeExSceneTransformTranslate(grandchild, 20, 0);
	xgeShapeExSceneTransformRotate(child, 1.0472f); /* 60 deg */
	xgeShapeExSceneTransformTranslate(child, 40, 0);
	xgeShapeExSceneAddScene(child, grandchild);
	xgeShapeExSceneTransformTranslate(parent, 400, by + 40);
	xgeShapeExSceneAddScene(parent, child);

	/* Add reference shapes at each level */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, -5, -5, 10, 10, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(80, 255, 80, 255));
	xgeShapeExSceneAdd(parent, s);
	xgeShapeExDestroy(s);

	xgeShapeExSceneDraw(parent, 0.5f);
	xgeShapeExSceneDestroy(parent);
}

/* Section 3: Opacity */
static void draw_opacity(float by)
{
	xge_shape_ex s;
	float opacities[] = {1.0f, 0.75f, 0.5f, 0.25f, 0.1f};
	int i;
	uint32_t col = XGE_COLOR_RGBA(255, 100, 150, 255);
	for ( i = 0; i < 5; i++ ) {
		xgeShapeExCreate(&s);
		xgeShapeExAppendRect(s, 30.0f + (float)i * 170.0f, by, 100, 60, 8, 8, 1);
		xgeShapeExFillColor(s, col);
		xgeShapeExOpacity(s, opacities[i]);
		xgeShapeExDraw(s, 0.5f);
		xgeShapeExDestroy(s);
	}
}

/* Section 4: Scene opacity inheritance */
static void draw_scene_opacity(float by)
{
	xge_shape_ex_scene scene;
	xge_shape_ex s;
	xge_shape_ex_matrix_t mat;

	xgeShapeExSceneCreate(&scene);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 80, 60, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(100, 200, 255, 255));
	xgeShapeExOpacity(s, 0.8f);
	xgeShapeExSceneAdd(scene, s);
	xgeShapeExDestroy(s);

	xgeShapeExCreate(&s);
	xgeShapeExAppendCircle(s, 60, 40, 25, 25, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 150, 50, 255));
	xgeShapeExOpacity(s, 0.6f);
	xgeShapeExSceneAdd(scene, s);
	xgeShapeExDestroy(s);

	/* Scene-level opacity 0.5 */
	xgeShapeExSceneOpacity(scene, 0.5f);
	memset(&mat, 0, sizeof(mat));
	mat.fA = 1; mat.fD = 1; mat.fE = 50; mat.fF = by;
	xgeShapeExSceneTransformSet(scene, &mat);
	xgeShapeExSceneDraw(scene, 0.5f);
	xgeShapeExSceneDestroy(scene);

	/* Same without scene opacity for comparison */
	xgeShapeExSceneCreate(&scene);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 80, 60, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(100, 200, 255, 255));
	xgeShapeExOpacity(s, 0.8f);
	xgeShapeExSceneAdd(scene, s);
	xgeShapeExDestroy(s);
	xgeShapeExCreate(&s);
	xgeShapeExAppendCircle(s, 60, 40, 25, 25, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 150, 50, 255));
	xgeShapeExOpacity(s, 0.6f);
	xgeShapeExSceneAdd(scene, s);
	xgeShapeExDestroy(s);
	memset(&mat, 0, sizeof(mat));
	mat.fA = 1; mat.fD = 1; mat.fE = 350; mat.fF = by;
	xgeShapeExSceneTransformSet(scene, &mat);
	xgeShapeExSceneDraw(scene, 0.5f);
	xgeShapeExSceneDestroy(scene);
}

static void marker(float x, float y) { xgeShapeRectFill(rectf(x,y,4,4), XGE_COLOR_RGBA(255,255,255,100)); }

static int frame(void* user)
{
	audit_demo_t* d = (audit_demo_t*)user;
	xge_pass_t p; int ret;
	if(!d) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin(); if(ret!=XGE_OK) return ret;
	xgePassInit(&p, &d->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&p); if(ret!=XGE_OK) return ret;

	marker(10, 5); draw_basic_transforms(10);
	marker(10, 80); draw_nested_transforms(90);
	marker(10, 200); draw_opacity(210);
	marker(10, 290); draw_scene_opacity(300);

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
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_shape_ex_transform";
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC; desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
