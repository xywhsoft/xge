#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUDIT_W 900
#define AUDIT_H 700
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct { xge_render_target_t tTarget; char sCapturePath[260]; int bCaptureDone; int iFrame; int iMaxFrames; } audit_demo_t;
static xge_rect_t rectf(float x, float y, float w, float h) { xge_rect_t r; r.fX=x; r.fY=y; r.fW=w; r.fH=h; return r; }
static int parse_args(audit_demo_t* d, int argc, char** argv) { int i; for(i=1;i<argc;i++){if(strcmp(argv[i],"--capture")==0&&i+1<argc)snprintf(d->sCapturePath,260,"%s",argv[++i]);else if(strncmp(argv[i],"--capture=",10)==0)snprintf(d->sCapturePath,260,"%s",argv[i]+10);else if(strcmp(argv[i],"--frames")==0&&i+1<argc)d->iMaxFrames=atoi(argv[++i]);else if(strncmp(argv[i],"--frames=",9)==0)d->iMaxFrames=atoi(argv[i]+9);} return XGE_OK; }
static int capture(audit_demo_t* d) { unsigned char*px;int s,r; if(!d||!d->sCapturePath[0]||d->bCaptureDone)return XGE_OK; s=AUDIT_W*4; px=(unsigned char*)malloc((size_t)s*AUDIT_H); if(!px)return XGE_ERROR_OUT_OF_MEMORY; r=xgeRenderTargetReadPixels(&d->tTarget,px,s); if(r==XGE_OK)r=xgeImageSavePNG(d->sCapturePath,AUDIT_W,AUDIT_H,px,s); free(px); if(r==XGE_OK){d->bCaptureDone=1;printf("capture saved: %s\n",d->sCapturePath);} return r; }

/* Helper: create a scene with a colored rect and apply effect */
static void draw_scene_with_effect(float x, float y, int effectType)
{
	xge_shape_ex_scene scene;
	xge_shape_ex s;
	xge_shape_ex_matrix_t mat;

	xgeShapeExSceneCreate(&scene);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 80, 60, 8, 8, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(100, 200, 255, 255));
	xgeShapeExSceneAdd(scene, s);
	xgeShapeExDestroy(s);

	switch (effectType) {
	case 0: /* Gaussian blur */
		xgeShapeExSceneEffectGaussianBlur(scene, 4.0f, 0, 0, 1);
		break;
	case 1: /* Drop shadow */
		xgeShapeExSceneEffectDropShadow(scene, XGE_COLOR_RGBA(0, 0, 0, 180), 135.0f, 5.0f, 3.0f, 1);
		break;
	case 2: /* Fill */
		xgeShapeExSceneEffectFill(scene, XGE_COLOR_RGBA(255, 80, 120, 255));
		break;
	case 3: /* Tint */
		xgeShapeExSceneEffectTint(scene, XGE_COLOR_RGBA(0, 0, 80, 255), XGE_COLOR_RGBA(255, 200, 100, 255), 0.8f);
		break;
	case 4: /* Tritone */
		xgeShapeExSceneEffectTritone(scene, XGE_COLOR_RGBA(80, 0, 120, 255), XGE_COLOR_RGBA(120, 80, 60, 255), XGE_COLOR_RGBA(255, 220, 180, 255), 0);
		break;
	}

	memset(&mat, 0, sizeof(mat));
	mat.fA = 1; mat.fD = 1; mat.fE = x; mat.fF = y;
	xgeShapeExSceneTransformSet(scene, &mat);
	xgeShapeExSceneDraw(scene, 0.5f);
	xgeShapeExSceneDestroy(scene);
}

/* Section: All 5 effects in a row */
static void draw_effects(float by)
{
	int i;
	float spacing = 170.0f;
	for ( i = 0; i < 5; i++ ) {
		draw_scene_with_effect(20.0f + (float)i * spacing, by, i);
	}
}

/* Section: Blur quality comparison */
static void draw_blur_quality(float by)
{
	xge_shape_ex_scene scene;
	xge_shape_ex s;
	xge_shape_ex_matrix_t mat;
	float sigmas[] = {1.0f, 2.0f, 4.0f, 8.0f};
	int i;
	for ( i = 0; i < 4; i++ ) {
		xgeShapeExSceneCreate(&scene);
		xgeShapeExCreate(&s);
		xgeShapeExAppendRect(s, 0, 0, 60, 60, 0, 0, 1);
		xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 180, 50, 255));
		xgeShapeExSceneAdd(scene, s);
		xgeShapeExDestroy(s);
		xgeShapeExSceneEffectGaussianBlur(scene, sigmas[i], 0, 0, 1);
		memset(&mat, 0, sizeof(mat));
		mat.fA = 1; mat.fD = 1; mat.fE = 30.0f + (float)i * 210.0f; mat.fF = by;
		xgeShapeExSceneTransformSet(scene, &mat);
		xgeShapeExSceneDraw(scene, 0.5f);
		xgeShapeExSceneDestroy(scene);
	}
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

	marker(10, 5); draw_effects(20);
	marker(10, 110); draw_blur_quality(120);

	ret = xgePassEnd(&p); if(ret!=XGE_OK) return ret;
	ret = capture(d); if(ret!=XGE_OK) return ret;
	xgeEnd(); d->iFrame++;
	if(d->bCaptureDone||((d->iMaxFrames>0)&&(d->iFrame>=d->iMaxFrames))) xgeQuit();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	audit_demo_t t; xge_desc_t desc; int ret;
	memset(&t,0,sizeof(t)); parse_args(&t,argc,argv);
	memset(&desc,0,sizeof(desc));
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_shape_ex_effect";
	desc.iFlags=XGE_INIT_OFFSCREEN; desc.iRunMode=XGE_RUN_GAME_LOOP;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
