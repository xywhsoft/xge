#include "../../xge.h"
#include "../audit_render_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AUDIT_W 900
#define AUDIT_H 900
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct { xge_render_target_t tTarget; char sCapturePath[260]; int bCaptureDone; int iFrame; int iMaxFrames; } audit_demo_t;
static xge_rect_t rectf(float x, float y, float w, float h) { xge_rect_t r; r.fX=x; r.fY=y; r.fW=w; r.fH=h; return r; }
static int parse_args(audit_demo_t* d, int argc, char** argv) { int i; for(i=1;i<argc;i++){if(strcmp(argv[i],"--capture")==0&&i+1<argc)snprintf(d->sCapturePath,260,"%s",argv[++i]);else if(strncmp(argv[i],"--capture=",10)==0)snprintf(d->sCapturePath,260,"%s",argv[i]+10);else if(strcmp(argv[i],"--frames")==0&&i+1<argc)d->iMaxFrames=atoi(argv[++i]);else if(strncmp(argv[i],"--frames=",9)==0)d->iMaxFrames=atoi(argv[i]+9);} return XGE_OK; }
static int capture(audit_demo_t* d) { unsigned char*px;int s,r; if(!d||!d->sCapturePath[0]||d->bCaptureDone)return XGE_OK; s=AUDIT_W*4; px=(unsigned char*)malloc((size_t)s*AUDIT_H); if(!px)return XGE_ERROR_OUT_OF_MEMORY; r=xgeRenderTargetReadPixels(&d->tTarget,px,s); if(r==XGE_OK)r=xgeImageSavePNG(d->sCapturePath,AUDIT_W,AUDIT_H,px,s); free(px); if(r==XGE_OK){d->bCaptureDone=1;printf("capture saved: %s\n",d->sCapturePath);} return r; }

/* Section 1: Rect clip */
static void draw_rect_clip(float by)
{
	xge_shape_ex s;
	/* Full rect with clip rect applied */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 30, by, 200, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(100, 200, 255, 255));
	xgeShapeExClipRectSet(s, rectf(80, by+20, 100, 40));
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	/* Circle with rect clip */
	xgeShapeExCreate(&s);
	xgeShapeExAppendCircle(s, 400, by+40, 40, 40, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 150, 80, 255));
	xgeShapeExClipRectSet(s, rectf(370, by+10, 60, 60));
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

/* Section 2: Shape clip (circle clips a rect) */
static void draw_shape_clip(float by)
{
	xge_shape_ex s, clip;
	/* Circle clip shape */
	xgeShapeExCreate(&clip);
	xgeShapeExAppendCircle(clip, 130, by+40, 35, 35, 1);
	/* Rect being clipped */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 50, by, 160, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(180, 100, 255, 255));
	xgeShapeExClipShapeAdd(s, clip);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(clip);
	/* Star clip on rect */
	xgeShapeExCreate(&clip);
	xgeShapeExMoveTo(clip, 400, by);
	xgeShapeExLineTo(clip, 420, by+30);
	xgeShapeExLineTo(clip, 455, by+35);
	xgeShapeExLineTo(clip, 430, by+55);
	xgeShapeExLineTo(clip, 435, by+80);
	xgeShapeExLineTo(clip, 400, by+65);
	xgeShapeExLineTo(clip, 365, by+80);
	xgeShapeExLineTo(clip, 370, by+55);
	xgeShapeExLineTo(clip, 345, by+35);
	xgeShapeExLineTo(clip, 380, by+30);
	xgeShapeExClose(clip);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 320, by, 160, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 220, 80, 255));
	xgeShapeExClipShapeAdd(s, clip);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(clip);
}

/* Section 3: Clip modes (intersect vs subtract) */
static void draw_clip_modes(float by)
{
	xge_shape_ex s, clip;
	/* Intersect mode */
	xgeShapeExCreate(&clip);
	xgeShapeExAppendCircle(clip, 100, by+40, 30, 30, 1);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 40, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(80, 255, 180, 255));
	xgeShapeExClipShapeAddEx(s, clip, XGE_SHAPE_EX_CLIP_INTERSECT);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(clip);
	/* Subtract mode */
	xgeShapeExCreate(&clip);
	xgeShapeExAppendCircle(clip, 320, by+40, 30, 30, 1);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 260, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(80, 255, 180, 255));
	xgeShapeExClipShapeAddEx(s, clip, XGE_SHAPE_EX_CLIP_SUBTRACT);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(clip);
}

/* Section 4: Mask alpha */
static void draw_mask_alpha(float by)
{
	xge_shape_ex s, mask;
	/* Alpha mask: gradient circle as mask */
	xgeShapeExCreate(&mask);
	xgeShapeExAppendCircle(mask, 100, by+40, 40, 40, 1);
	xgeShapeExFillColor(mask, XGE_COLOR_RGBA(255, 255, 255, 200));
	/* Source shape */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 40, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 100, 100, 255));
	xgeShapeExMaskShapeSet(s, mask, XGE_SHAPE_EX_MASK_ALPHA);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(mask);
	/* Inv-alpha mask */
	xgeShapeExCreate(&mask);
	xgeShapeExAppendCircle(mask, 320, by+40, 40, 40, 1);
	xgeShapeExFillColor(mask, XGE_COLOR_RGBA(255, 255, 255, 200));
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 260, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(100, 100, 255, 255));
	xgeShapeExMaskShapeSet(s, mask, XGE_SHAPE_EX_MASK_INV_ALPHA);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(mask);
}

/* Section 5: Mask luma */
static void draw_mask_luma(float by)
{
	xge_shape_ex s, mask;
	/* Luma mask with gradient */
	xge_shape_ex_color_stop_t stops[2];
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	xgeShapeExCreate(&mask);
	xgeShapeExAppendRect(mask, 40, by, 150, 80, 0, 0, 1);
	xgeShapeExFillLinearGradient(mask, 40, 0, 190, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 40, by, 150, 80, 0, 0, 1);
	xgeShapeExFillColor(s, XGE_COLOR_RGBA(255, 200, 50, 255));
	xgeShapeExMaskShapeSet(s, mask, XGE_SHAPE_EX_MASK_LUMA);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(mask);
}

/* Section 6: Mask add/subtract/intersect */
static void draw_mask_ops(float by)
{
	xge_shape_ex s, mask;
	uint32_t col = XGE_COLOR_RGBA(200, 80, 255, 255);
	/* Add mask */
	xgeShapeExCreate(&mask);
	xgeShapeExAppendCircle(mask, 80, by+40, 30, 30, 1);
	xgeShapeExFillColor(mask, XGE_COLOR_RGBA(255, 255, 255, 128));
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 30, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, col);
	xgeShapeExMaskShapeSet(s, mask, XGE_SHAPE_EX_MASK_ADD);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(mask);
	/* Subtract mask */
	xgeShapeExCreate(&mask);
	xgeShapeExAppendCircle(mask, 280, by+40, 30, 30, 1);
	xgeShapeExFillColor(mask, XGE_COLOR_RGBA(255, 255, 255, 200));
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 230, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, col);
	xgeShapeExMaskShapeSet(s, mask, XGE_SHAPE_EX_MASK_SUBTRACT);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(mask);
	/* Intersect mask */
	xgeShapeExCreate(&mask);
	xgeShapeExAppendCircle(mask, 480, by+40, 30, 30, 1);
	xgeShapeExFillColor(mask, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 430, by, 120, 80, 0, 0, 1);
	xgeShapeExFillColor(s, col);
	xgeShapeExMaskShapeSet(s, mask, XGE_SHAPE_EX_MASK_INTERSECT);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
	xgeShapeExDestroy(mask);
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

	marker(10,5); draw_rect_clip(10);
	marker(10,105); draw_shape_clip(110);
	marker(10,205); draw_clip_modes(210);
	marker(10,305); draw_mask_alpha(310);
	marker(10,405); draw_mask_luma(410);
	marker(10,505); draw_mask_ops(510);

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
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_shape_ex_clip_mask";
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC; desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
