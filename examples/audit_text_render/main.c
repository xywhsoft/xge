#include "../../xge.h"
#include "../audit_render_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUDIT_W 900
#define AUDIT_H 600
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct { xge_render_target_t tTarget; char sCapturePath[260]; int bCaptureDone; int iFrame; int iMaxFrames; } audit_demo_t;
static xge_rect_t rectf(float x, float y, float w, float h) { xge_rect_t r; r.fX=x; r.fY=y; r.fW=w; r.fH=h; return r; }
static int parse_args(audit_demo_t* d, int argc, char** argv) { int i; for(i=1;i<argc;i++){if(strcmp(argv[i],"--capture")==0&&i+1<argc)snprintf(d->sCapturePath,260,"%s",argv[++i]);else if(strncmp(argv[i],"--capture=",10)==0)snprintf(d->sCapturePath,260,"%s",argv[i]+10);else if(strcmp(argv[i],"--frames")==0&&i+1<argc)d->iMaxFrames=atoi(argv[++i]);else if(strncmp(argv[i],"--frames=",9)==0)d->iMaxFrames=atoi(argv[i]+9);} return XGE_OK; }
static int capture(audit_demo_t* d) { unsigned char*px;int s,r; if(!d||!d->sCapturePath[0]||d->bCaptureDone)return XGE_OK; s=AUDIT_W*4; px=(unsigned char*)malloc((size_t)s*AUDIT_H); if(!px)return XGE_ERROR_OUT_OF_MEMORY; r=xgeRenderTargetReadPixels(&d->tTarget,px,s); if(r==XGE_OK)r=xgeImageSavePNG(d->sCapturePath,AUDIT_W,AUDIT_H,px,s); free(px); if(r==XGE_OK){d->bCaptureDone=1;printf("capture saved: %s\n",d->sCapturePath);} return r; }

static const char* find_ttf(void)
{
	static const char* paths[] = {"C:\\Windows\\Fonts\\arial.ttf","C:\\Windows\\Fonts\\segoeui.ttf","C:\\Windows\\Fonts\\calibri.ttf"};
	int i;
	for(i=0;i<3;i++){FILE*f=fopen(paths[i],"rb");if(f){fclose(f);return paths[i];}}
	return NULL;
}

static void marker(float x, float y) { xgeShapeRectFill(rectf(x,y,4,4), XGE_COLOR_RGBA(255,255,255,100)); }

static int frame(void* user)
{
	audit_demo_t* d = (audit_demo_t*)user;
	xge_pass_t p; int ret;
	xge_font_t font;
	const char* ttf;

	if(!d) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin(); if(ret!=XGE_OK) return ret;
	xgePassInit(&p, &d->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&p); if(ret!=XGE_OK) return ret;

	ttf = find_ttf();
	if ( ttf && xgeFontLoad(&font, ttf, 16.0f) == XGE_OK ) {
		/* Basic text draw */
		marker(10, 5);
		xgeTextDraw(&font, "Hello World - Basic Text", 30, 30, XGE_COLOR_RGBA(255,255,255,255));

		/* Text in rect with alignment flags */
		marker(10, 65);
		xgeShapeRectStroke(rectf(30, 70, 200, 40), 1.0f, XGE_COLOR_RGBA(100,100,100,255));
		xgeTextDrawRect(&font, "Left-Top", rectf(30, 70, 200, 40), XGE_COLOR_RGBA(100,255,100,255), 0);
		xgeShapeRectStroke(rectf(250, 70, 200, 40), 1.0f, XGE_COLOR_RGBA(100,100,100,255));
		xgeTextDrawRect(&font, "Center", rectf(250, 70, 200, 40), XGE_COLOR_RGBA(100,200,255,255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
		xgeShapeRectStroke(rectf(470, 70, 200, 40), 1.0f, XGE_COLOR_RGBA(100,100,100,255));
		xgeTextDrawRect(&font, "Right-Bottom", rectf(470, 70, 200, 40), XGE_COLOR_RGBA(255,150,100,255), XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_BOTTOM);

		/* Different sizes */
		marker(10, 125);
		xgeFontFree(&font);
		xgeFontLoad(&font, ttf, 24.0f);
		xgeTextDraw(&font, "24px Large Text", 30, 140, XGE_COLOR_RGBA(255,200,50,255));
		xgeFontFree(&font);
		xgeFontLoad(&font, ttf, 10.0f);
		xgeTextDraw(&font, "10px Small Text for density testing", 30, 175, XGE_COLOR_RGBA(200,200,200,255));
		xgeFontFree(&font);
		xgeFontLoad(&font, ttf, 14.0f);

		/* Text measurement */
		marker(10, 200);
		{
			xge_vec2_t sz = xgeTextMeasure(&font, "Measured");
			xgeTextDraw(&font, "Measured", 30, 220, XGE_COLOR_RGBA(255,100,255,255));
			xgeShapeRectStroke(rectf(30, 210, sz.fX, sz.fY), 1.0f, XGE_COLOR_RGBA(255,100,255,128));
		}

		/* Decorated text via glyph run */
		marker(10, 260);
		{
			xge_text_shape_desc_t desc;
			xge_glyph_run_t run;
			memset(&desc, 0, sizeof(desc));
			desc.pFont = &font;
			desc.sText = "Underlined Text";
			desc.iTextSize = -1;
			if ( xgeTextShape(&desc, &run) == XGE_OK ) {
				xge_text_decoration_t dec;
				dec.iType = XGE_TEXT_DECORATION_UNDERLINE;
				dec.fOffset = 0;
				dec.fThickness = 1.5f;
				dec.iColor = XGE_COLOR_RGBA(255,80,80,255);
				xgeGlyphRunDrawDecorated(&run, 30, 280, XGE_COLOR_RGBA(255,255,255,255), 0, &dec, 1);
				xgeGlyphRunFree(&run);
			}
		}

		xgeFontFree(&font);
	} else {
		/* No font available - draw placeholder */
		xgeShapeRectFill(rectf(30,30,200,30), XGE_COLOR_RGBA(255,0,0,255));
	}

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
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_text_render";
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC; desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
