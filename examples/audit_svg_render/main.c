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

static const char* SVG_SIMPLE =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
	"<rect x=\"10\" y=\"10\" width=\"80\" height=\"80\" fill=\"#64c8ff\" rx=\"8\"/>"
	"<circle cx=\"50\" cy=\"50\" r=\"20\" fill=\"#ff6464\"/>"
	"</svg>";

static const char* SVG_GRADIENT =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
	"<defs><linearGradient id=\"g1\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"1\">"
	"<stop offset=\"0\" stop-color=\"#ff0000\"/><stop offset=\"1\" stop-color=\"#0000ff\"/>"
	"</linearGradient></defs>"
	"<rect width=\"100\" height=\"100\" fill=\"url(#g1)\"/>"
	"</svg>";

static const char* SVG_PATH =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
	"<path d=\"M50 5 L61 35 L95 35 L68 57 L79 91 L50 70 L21 91 L32 57 L5 35 L39 35 Z\" fill=\"#ffd700\" stroke=\"#ff8c00\" stroke-width=\"2\"/>"
	"</svg>";

static const char* SVG_TRANSFORM =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
	"<g transform=\"rotate(45 50 50)\">"
	"<rect x=\"25\" y=\"25\" width=\"50\" height=\"50\" fill=\"#80ff80\"/>"
	"</g>"
	"</svg>";

static const char* SVG_CLIP =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
	"<defs><clipPath id=\"c1\"><circle cx=\"50\" cy=\"50\" r=\"40\"/></clipPath></defs>"
	"<rect width=\"100\" height=\"100\" fill=\"#ff80ff\" clip-path=\"url(#c1)\"/>"
	"</svg>";

static const char* SVG_OPACITY =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
	"<rect width=\"100\" height=\"100\" fill=\"#4488ff\"/>"
	"<rect x=\"20\" y=\"20\" width=\"60\" height=\"60\" fill=\"#ff4444\" opacity=\"0.5\"/>"
	"</svg>";

static void draw_svg_at(const char* svgData, float x, float y, float w, float h)
{
	xge_svg svg;
	if ( xgeSvgCreate(&svg) != XGE_OK ) return;
	if ( xgeSvgLoadMemory(svg, svgData, (int)strlen(svgData)) != XGE_OK ) { xgeSvgDestroy(svg); return; }
	xgeSvgDraw(svg, rectf(x, y, w, h), 0.5f);
	xgeSvgDestroy(svg);
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

	/* Row 1: Basic SVG shapes */
	marker(10, 5);
	draw_svg_at(SVG_SIMPLE, 20, 10, 120, 120);
	draw_svg_at(SVG_GRADIENT, 160, 10, 120, 120);
	draw_svg_at(SVG_PATH, 300, 10, 120, 120);

	/* Row 2: Transform, clip, opacity */
	marker(10, 145);
	draw_svg_at(SVG_TRANSFORM, 20, 150, 120, 120);
	draw_svg_at(SVG_CLIP, 160, 150, 120, 120);
	draw_svg_at(SVG_OPACITY, 300, 150, 120, 120);

	/* Row 3: Scaling tests */
	marker(10, 285);
	draw_svg_at(SVG_SIMPLE, 20, 290, 60, 60);
	draw_svg_at(SVG_SIMPLE, 100, 290, 180, 180);
	draw_svg_at(SVG_PATH, 320, 290, 200, 200);

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
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_svg_render";
	desc.iFlags=XGE_INIT_OFFSCREEN; desc.iRunMode=XGE_RUN_GAME_LOOP;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
