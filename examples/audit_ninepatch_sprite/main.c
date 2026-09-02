#include "../../xge.h"
#include "../audit_render_common.h"
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

/* Create a 48x48 test texture with distinct border/center regions for nine-patch testing */
static int create_test_texture(xge_texture pTex)
{
	/* 48x48 RGBA: 8px border (blue), inner fill (green), corners (red) */
	unsigned char px[48*48*4];
	int x, y;
	memset(px, 0, sizeof(px));
	for ( y = 0; y < 48; y++ ) {
		for ( x = 0; x < 48; x++ ) {
			int idx = (y*48+x)*4;
			int border = (x < 8 || x >= 40 || y < 8 || y >= 40);
			int corner = (x < 8 || x >= 40) && (y < 8 || y >= 40);
			if ( corner ) { px[idx]=220; px[idx+1]=50; px[idx+2]=50; px[idx+3]=255; }
			else if ( border ) { px[idx]=50; px[idx+1]=100; px[idx+2]=220; px[idx+3]=255; }
			else { px[idx]=50; px[idx+1]=200; px[idx+2]=80; px[idx+3]=255; }
		}
	}
	return xgeTextureCreateRGBA(pTex, 48, 48, px);
}

static void marker(float x, float y) { xgeShapeRectFill(rectf(x,y,4,4), XGE_COLOR_RGBA(255,255,255,100)); }

static int frame(void* user)
{
	audit_demo_t* d = (audit_demo_t*)user;
	xge_pass_t p; int ret;
	xge_texture_t tex;
	xge_nine_patch_t patch;

	if(!d) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin(); if(ret!=XGE_OK) return ret;
	xgePassInit(&p, &d->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&p); if(ret!=XGE_OK) return ret;

	memset(&tex, 0, sizeof(tex));
	ret = create_test_texture(&tex);
	if ( ret != XGE_OK ) { xgePassEnd(&p); xgeEnd(); xgeQuit(); return ret; }

	/* Section 1: Nine-patch STRETCH mode at various sizes */
	marker(10, 5);
	xgeNinePatchInit(&patch, &tex, rectf(0,0,48,48), 8, 8, 40, 40);
	xgeNinePatchSetMode(&patch, XGE_NINE_PATCH_STRETCH);
	xgeNinePatchSetColor(&patch, XGE_COLOR_RGBA(255,255,255,255));
	/* Small (same as source) */
	xgeNinePatchDraw(&patch, rectf(20, 10, 48, 48), 0);
	/* Medium stretched */
	xgeNinePatchDraw(&patch, rectf(90, 10, 120, 80), 0);
	/* Large stretched */
	xgeNinePatchDraw(&patch, rectf(230, 10, 200, 120), 0);
	/* Wide short */
	xgeNinePatchDraw(&patch, rectf(450, 10, 250, 40), 0);
	/* Tall narrow */
	xgeNinePatchDraw(&patch, rectf(720, 10, 50, 150), 0);

	/* Section 2: Nine-patch TILE mode */
	marker(10, 175);
	xgeNinePatchSetMode(&patch, XGE_NINE_PATCH_TILE);
	xgeNinePatchDraw(&patch, rectf(20, 180, 120, 80), 0);
	xgeNinePatchDraw(&patch, rectf(160, 180, 200, 120), 0);
	xgeNinePatchDraw(&patch, rectf(380, 180, 250, 50), 0);

	/* Section 3: Simple nine-patch (auto borders) */
	marker(10, 315);
	{
		xge_nine_patch_t simplePatch;
		xgeNinePatchInitSimple(&simplePatch, &tex, rectf(0,0,48,48));
		xgeNinePatchSetMode(&simplePatch, XGE_NINE_PATCH_STRETCH);
		xgeNinePatchSetColor(&simplePatch, XGE_COLOR_RGBA(255,255,255,255));
		xgeNinePatchDraw(&simplePatch, rectf(20, 320, 150, 100), 0);
		xgeNinePatchDraw(&simplePatch, rectf(190, 320, 100, 150), 0);
	}

	/* Section 4: Nine-patch with color tint */
	marker(10, 485);
	xgeNinePatchSetMode(&patch, XGE_NINE_PATCH_STRETCH);
	xgeNinePatchSetColor(&patch, XGE_COLOR_RGBA(255,100,100,255));
	xgeNinePatchDraw(&patch, rectf(20, 490, 130, 80), 0);
	xgeNinePatchSetColor(&patch, XGE_COLOR_RGBA(100,255,100,200));
	xgeNinePatchDraw(&patch, rectf(170, 490, 130, 80), 0);
	xgeNinePatchSetColor(&patch, XGE_COLOR_RGBA(100,100,255,128));
	xgeNinePatchDraw(&patch, rectf(320, 490, 130, 80), 0);

	/* Section 5: Sprite batch */
	marker(10, 590);
	{
		xge_sprite_batch_t batch;
		xge_draw_t draw;
		int i;
		memset(&batch, 0, sizeof(batch));
		if ( xgeSpriteBatchInit(&batch, &tex, 64, 0) == XGE_OK ) {
			for ( i = 0; i < 8; i++ ) {
				memset(&draw, 0, sizeof(draw));
				draw.pTexture = &tex;
				draw.tSrc = rectf(0, 0, 48, 48);
				draw.tDst = rectf(20.0f + i*55.0f, 600, 48, 48);
				draw.iColor = XGE_COLOR_RGBA(255,255,255,255);
				xgeSpriteBatchAdd(&batch, &draw);
			}
			/* Rotated sprites */
			for ( i = 0; i < 4; i++ ) {
				memset(&draw, 0, sizeof(draw));
				draw.pTexture = &tex;
				draw.tSrc = rectf(0, 0, 48, 48);
				draw.tDst = rectf(500.0f + i*70.0f, 600, 48, 48);
				draw.tOrigin.fX = 24; draw.tOrigin.fY = 24;
				draw.fRotation = (float)(i * 30);
				draw.iColor = XGE_COLOR_RGBA(255,200,100,255);
				xgeSpriteBatchAdd(&batch, &draw);
			}
			xgeSpriteBatchFlush(&batch);
			xgeSpriteBatchFree(&batch);
		}
	}

	xgeTextureFree(&tex);
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
	desc.iWidth=AUDIT_W; desc.iHeight=AUDIT_H; desc.sTitle="audit_ninepatch_sprite";
	desc.iFlags=XGE_INIT_WINDOW|XGE_INIT_VSYNC; desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60;
	ret=xgeInit(&desc); if(ret!=XGE_OK) return 1;
	ret=xgeRenderTargetCreate(&t.tTarget,AUDIT_W,AUDIT_H); if(ret!=XGE_OK){xgeUnit();return 1;}
	ret=xgeRun(frame,&t);
	xgeRenderTargetFree(&t.tTarget); xgeUnit();
	return (ret==XGE_OK)?0:1;
}
