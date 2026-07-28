#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUDIT_W 900
#define AUDIT_H 900
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct { xge_render_target_t tTarget; char sCapturePath[260]; int bCaptureDone; int iFrame; int iMaxFrames; } audit_demo_t;
static xge_rect_t rectf(float x, float y, float w, float h) { xge_rect_t r; r.fX=x; r.fY=y; r.fW=w; r.fH=h; return r; }
static int parse_args(audit_demo_t* d, int argc, char** argv) { int i; for(i=1;i<argc;i++){if(strcmp(argv[i],"--capture")==0&&i+1<argc)snprintf(d->sCapturePath,260,"%s",argv[++i]);else if(strncmp(argv[i],"--capture=",10)==0)snprintf(d->sCapturePath,260,"%s",argv[i]+10);else if(strcmp(argv[i],"--frames")==0&&i+1<argc)d->iMaxFrames=atoi(argv[++i]);else if(strncmp(argv[i],"--frames=",9)==0)d->iMaxFrames=atoi(argv[i]+9);} return XGE_OK; }
static int capture(audit_demo_t* d) { unsigned char*px;int s,r; if(!d||!d->sCapturePath[0]||d->bCaptureDone)return XGE_OK; s=AUDIT_W*4; px=(unsigned char*)malloc((size_t)s*AUDIT_H); if(!px)return XGE_ERROR_OUT_OF_MEMORY; r=xgeRenderTargetReadPixels(&d->tTarget,px,s); if(r==XGE_OK)r=xgeImageSavePNG(d->sCapturePath,AUDIT_W,AUDIT_H,px,s); free(px); if(r==XGE_OK){d->bCaptureDone=1;printf("capture saved: %s\n",d->sCapturePath);} return r; }

/* Draw a blend mode test: background rect + foreground shape with blend */
static void draw_blend_test(float x, float y, int blend, uint32_t bgCol, uint32_t fgCol)
{
	xge_shape_ex s;
	/* Background */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, x, y, 80, 60, 0, 0, 1);
	xgeShapeExFillColor(s, bgCol);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	/* Foreground with blend */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, x+20, y+10, 80, 60, 0, 0, 1);
	xgeShapeExFillColor(s, fgCol);
	xgeShapeExBlend(s, blend);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

/* Draw all 18 blend modes in a grid */
static void draw_all_blends(float by)
{
	uint32_t bg = XGE_COLOR_RGBA(200, 100, 50, 255);
	uint32_t fg = XGE_COLOR_RGBA(50, 150, 200, 200);
	int blends[] = {
		XGE_BLEND_NONE, XGE_BLEND_ALPHA, XGE_BLEND_ADD, XGE_BLEND_MULTIPLY,
		XGE_BLEND_SCREEN, XGE_BLEND_DARKEN, XGE_BLEND_LIGHTEN, XGE_BLEND_OVERLAY,
		XGE_BLEND_COLOR_DODGE, XGE_BLEND_COLOR_BURN, XGE_BLEND_HARD_LIGHT, XGE_BLEND_SOFT_LIGHT,
		XGE_BLEND_DIFFERENCE, XGE_BLEND_EXCLUSION, XGE_BLEND_HUE, XGE_BLEND_SATURATION,
		XGE_BLEND_COLOR, XGE_BLEND_LUMINOSITY
	};
	int i, col, row;
	for ( i = 0; i < 18; i++ ) {
		col = i % 6;
		row = i / 6;
		draw_blend_test(20.0f + (float)col * 145.0f, by + (float)row * 80.0f, blends[i], bg, fg);
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

	marker(10, 5);
	draw_all_blends(10);

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
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_shape_ex_blend";
	desc.iFlags=XGE_INIT_OFFSCREEN; desc.iRunMode=XGE_RUN_GAME_LOOP;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
