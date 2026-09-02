#include "../../xge.h"
#include "../audit_render_common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUDIT_W 900
#define AUDIT_H 1200
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct audit_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
} audit_demo_t;

static xge_rect_t rectf(float x, float y, float w, float h)
{
	xge_rect_t r;
	r.fX = x; r.fY = y; r.fW = w; r.fH = h;
	return r;
}

static void usage(void)
{
	printf("usage: audit_shape_basic [--frames N] [--capture PATH]\n");
}

static int parse_args(audit_demo_t* demo, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			demo->iMaxFrames = atoi(argv[++i]);
			if ( demo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			demo->iMaxFrames = atoi(argv[i] + 9);
			if ( demo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(demo->sCapturePath, sizeof(demo->sCapturePath), "%s", argv[++i]);
			demo->sCapturePath[sizeof(demo->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(demo->sCapturePath, sizeof(demo->sCapturePath), "%s", argv[i] + 10);
			demo->sCapturePath[sizeof(demo->sCapturePath) - 1] = '\0';
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			usage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int capture(audit_demo_t* demo)
{
	unsigned char* pixels;
	int stride;
	int ret;

	if ( (demo == NULL) || (demo->sCapturePath[0] == '\0') || demo->bCaptureDone ) {
		return XGE_OK;
	}
	stride = AUDIT_W * 4;
	pixels = (unsigned char*)malloc((size_t)stride * AUDIT_H);
	if ( pixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = xgeRenderTargetReadPixels(&demo->tTarget, pixels, stride);
	if ( ret == XGE_OK ) {
		ret = xgeImageSavePNG(demo->sCapturePath, AUDIT_W, AUDIT_H, pixels, stride);
	}
	free(pixels);
	if ( ret == XGE_OK ) {
		demo->bCaptureDone = 1;
		printf("audit_shape_basic capture saved: %s\n", demo->sCapturePath);
	}
	return ret;
}

/* Section 1: Line width precision (1px to 8px horizontal lines) */
static void draw_line_widths(float fBaseY)
{
	float fX = 30.0f;
	float fWidths[] = {1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f};
	int i;
	uint32_t iColor = XGE_COLOR_RGBA(100, 200, 255, 255);

	for ( i = 0; i < 8; i++ ) {
		float fY = fBaseY + (float)i * 22.0f;
		xgeShapeLine(fX, fY, fX + 100.0f, fY, fWidths[i], iColor);
	}
}

/* Section 2: Diagonal line AA quality */
static void draw_diagonal_aa(float fBaseX, float fBaseY)
{
	uint32_t iColor = XGE_COLOR_RGBA(255, 180, 80, 255);
	float fAngles[] = {0.0f, 15.0f, 30.0f, 45.0f, 60.0f, 75.0f, 90.0f};
	int i;

	for ( i = 0; i < 7; i++ ) {
		float fRad = fAngles[i] * 3.14159265f / 180.0f;
		float fLen = 80.0f;
		float fX1 = fBaseX + (float)i * 120.0f;
		float fY1 = fBaseY + 60.0f;
		float fX2 = fX1 + cosf(fRad) * fLen;
		float fY2 = fY1 - sinf(fRad) * fLen;
		xgeShapeLine(fX1, fY1, fX2, fY2, 2.0f, iColor);
	}
}

/* Section 3: Circle fill/stroke at various radii */
static void draw_circles(float fBaseX, float fBaseY)
{
	float fRadii[] = {4.0f, 8.0f, 12.0f, 20.0f, 30.0f, 40.0f};
	uint32_t iFill = XGE_COLOR_RGBA(80, 220, 160, 255);
	uint32_t iStroke = XGE_COLOR_RGBA(255, 100, 140, 255);
	int i;

	for ( i = 0; i < 6; i++ ) {
		float fCX = fBaseX + (float)i * 140.0f + 50.0f;
		float fCY = fBaseY + 50.0f;
		xgeShapeCircleFill(fCX, fCY, fRadii[i], iFill);
		xgeShapeCircleStroke(fCX, fCY, fRadii[i] + 8.0f, 2.0f, iStroke);
	}
}

/* Section 4: Rounded rectangle corner radius variations */
static void draw_round_rects(float fBaseX, float fBaseY)
{
	float fRadii[] = {0.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f};
	uint32_t iFill = XGE_COLOR_RGBA(180, 130, 255, 255);
	uint32_t iStroke = XGE_COLOR_RGBA(255, 220, 100, 255);
	int i;

	for ( i = 0; i < 6; i++ ) {
		float fX = fBaseX + (float)(i % 3) * 280.0f;
		float fY = fBaseY + (float)(i / 3) * 100.0f;
		xgeShapeRoundRectFill(rectf(fX, fY, 120.0f, 70.0f), fRadii[i], iFill);
		xgeShapeRoundRectStroke(rectf(fX + 140.0f, fY, 120.0f, 70.0f), fRadii[i], 2.5f, iStroke);
	}
}

/* Section 5: Arc precision at various start/end angles */
static void draw_arcs(float fBaseX, float fBaseY)
{
	uint32_t iColor = XGE_COLOR_RGBA(255, 140, 200, 255);
	float fStarts[] = {0.0f, 0.785f, 1.571f, 3.14159f, 4.712f};
	float fEnds[] = {1.571f, 3.14159f, 4.712f, 6.283f, 6.283f};
	int i;

	for ( i = 0; i < 5; i++ ) {
		float fCX = fBaseX + (float)i * 170.0f + 50.0f;
		float fCY = fBaseY + 50.0f;
		xgeShapeArc(fCX, fCY, 35.0f, fStarts[i], fEnds[i], 3.0f, iColor);
	}
}

/* Section 6: Ellipse fill/stroke */
static void draw_ellipses(float fBaseX, float fBaseY)
{
	uint32_t iFill = XGE_COLOR_RGBA(100, 255, 200, 200);
	uint32_t iStroke = XGE_COLOR_RGBA(255, 255, 100, 255);
	float fRX[] = {40.0f, 60.0f, 30.0f, 50.0f};
	float fRY[] = {20.0f, 30.0f, 40.0f, 15.0f};
	int i;

	for ( i = 0; i < 4; i++ ) {
		float fCX = fBaseX + (float)i * 210.0f + 60.0f;
		float fCY = fBaseY + 40.0f;
		xgeShapeEllipseFill(fCX, fCY, fRX[i], fRY[i], iFill);
		xgeShapeEllipseStroke(fCX, fCY, fRX[i] + 10.0f, fRY[i] + 10.0f, 2.0f, iStroke);
	}
}

/* Section 7: Polygon fills (triangle, pentagon, star) */
static void draw_polygons(float fBaseX, float fBaseY)
{
	uint32_t iColor1 = XGE_COLOR_RGBA(255, 120, 80, 255);
	uint32_t iColor2 = XGE_COLOR_RGBA(80, 180, 255, 255);
	uint32_t iColor3 = XGE_COLOR_RGBA(200, 255, 80, 255);
	xge_vec2_t tri[3] = {{50, 10}, {10, 80}, {90, 80}};
	xge_vec2_t pent[5];
	xge_vec2_t star[10];
	int i;
	float fAngle;

	/* Triangle */
	for ( i = 0; i < 3; i++ ) {
		tri[i].fX += fBaseX;
		tri[i].fY += fBaseY;
	}
	xgeShapePolygonFill(tri, 3, iColor1);

	/* Pentagon */
	for ( i = 0; i < 5; i++ ) {
		fAngle = (float)i * 6.28318f / 5.0f - 1.5708f;
		pent[i].fX = fBaseX + 200.0f + cosf(fAngle) * 40.0f;
		pent[i].fY = fBaseY + 50.0f + sinf(fAngle) * 40.0f;
	}
	xgeShapePolygonFill(pent, 5, iColor2);

	/* Star (5-point) */
	for ( i = 0; i < 10; i++ ) {
		float fR = (i % 2 == 0) ? 40.0f : 18.0f;
		fAngle = (float)i * 3.14159f / 5.0f - 1.5708f;
		star[i].fX = fBaseX + 400.0f + cosf(fAngle) * fR;
		star[i].fY = fBaseY + 50.0f + sinf(fAngle) * fR;
	}
	xgeShapePolygonFill(star, 10, iColor3);
}

/* Section 8: Capsule shapes */
static void draw_capsules(float fBaseX, float fBaseY)
{
	uint32_t iFill = XGE_COLOR_RGBA(160, 220, 255, 255);
	uint32_t iStroke = XGE_COLOR_RGBA(255, 160, 100, 255);

	xgeShapeCapsuleFill(rectf(fBaseX, fBaseY, 100.0f, 30.0f), iFill);
	xgeShapeCapsuleStroke(rectf(fBaseX + 130.0f, fBaseY, 100.0f, 30.0f), 2.5f, iStroke);
	xgeShapeCapsuleFill(rectf(fBaseX + 260.0f, fBaseY, 60.0f, 60.0f), iFill);
	xgeShapeCapsuleStroke(rectf(fBaseX + 350.0f, fBaseY, 150.0f, 20.0f), 3.0f, iStroke);
}

/* Section 9: Pixel-perfect mode comparison (Px suffix) */
static void draw_pixel_perfect(float fBaseX, float fBaseY)
{
	uint32_t iColor = XGE_COLOR_RGBA(255, 255, 255, 255);

	/* Sub-pixel positioned shapes - normal mode */
	xgeShapeRectFill(rectf(fBaseX + 0.5f, fBaseY, 40.0f, 40.0f), iColor);
	xgeShapeRectStroke(rectf(fBaseX + 50.5f, fBaseY + 0.5f, 40.0f, 40.0f), 1.0f, iColor);

	/* Pixel-snapped mode */
	xgeShapeRectFillPx(rectf(fBaseX + 120.5f, fBaseY, 40.0f, 40.0f), iColor);
	xgeShapeRectStrokePx(rectf(fBaseX + 170.5f, fBaseY + 0.5f, 40.0f, 40.0f), 1.0f, iColor);

	/* Circle comparison */
	xgeShapeCircleFill(fBaseX + 250.0f, fBaseY + 20.0f, 15.0f, iColor);
	xgeShapeCircleFillPx(fBaseX + 310.0f, fBaseY + 20.0f, 15.0f, iColor);
}

/* Section 10: Overlapping semi-transparent shapes (blend test) */
static void draw_alpha_overlap(float fBaseX, float fBaseY)
{
	uint32_t iRed = XGE_COLOR_RGBA(255, 0, 0, 128);
	uint32_t iGreen = XGE_COLOR_RGBA(0, 255, 0, 128);
	uint32_t iBlue = XGE_COLOR_RGBA(0, 0, 255, 128);

	xgeShapeCircleFill(fBaseX + 40.0f, fBaseY + 30.0f, 35.0f, iRed);
	xgeShapeCircleFill(fBaseX + 70.0f, fBaseY + 30.0f, 35.0f, iGreen);
	xgeShapeCircleFill(fBaseX + 55.0f, fBaseY + 60.0f, 35.0f, iBlue);
}

static void draw_section_label(const char* sLabel, float fX, float fY)
{
	/* We can't draw text without a font in offscreen mode easily,
	 * so we use a small colored marker instead */
	(void)sLabel;
	xgeShapeRectFill(rectf(fX, fY, 4.0f, 4.0f), XGE_COLOR_RGBA(255, 255, 255, 100));
}

static int frame(void* user)
{
	audit_demo_t* demo = (audit_demo_t*)user;
	xge_pass_t tPass;
	int ret;

	if ( demo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;

	xgePassInit(&tPass, &demo->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&tPass);
	if ( ret != XGE_OK ) return ret;

	/* Section 1: Line widths (y=20..196) */
	draw_section_label("line-widths", 10.0f, 10.0f);
	draw_line_widths(20.0f);

	/* Section 2: Diagonal AA (y=200..280) */
	draw_section_label("diagonal-aa", 10.0f, 195.0f);
	draw_diagonal_aa(30.0f, 200.0f);

	/* Section 3: Circles (y=300..420) */
	draw_section_label("circles", 10.0f, 295.0f);
	draw_circles(20.0f, 300.0f);

	/* Section 4: Round rects (y=430..640) */
	draw_section_label("round-rects", 10.0f, 425.0f);
	draw_round_rects(20.0f, 430.0f);

	/* Section 5: Arcs (y=650..760) */
	draw_section_label("arcs", 10.0f, 645.0f);
	draw_arcs(20.0f, 650.0f);

	/* Section 6: Ellipses (y=770..860) */
	draw_section_label("ellipses", 10.0f, 765.0f);
	draw_ellipses(20.0f, 770.0f);

	/* Section 7: Polygons (y=870..970) */
	draw_section_label("polygons", 10.0f, 865.0f);
	draw_polygons(30.0f, 870.0f);

	/* Section 8: Capsules (y=980..1050) */
	draw_section_label("capsules", 10.0f, 975.0f);
	draw_capsules(30.0f, 985.0f);

	/* Section 9: Pixel-perfect (y=1060..1110) */
	draw_section_label("pixel-perfect", 10.0f, 1055.0f);
	draw_pixel_perfect(30.0f, 1060.0f);

	/* Section 10: Alpha overlap (y=1120..1190) */
	draw_section_label("alpha-overlap", 10.0f, 1115.0f);
	draw_alpha_overlap(30.0f, 1120.0f);

	ret = xgePassEnd(&tPass);
	if ( ret != XGE_OK ) return ret;

	ret = capture(demo);
	if ( ret != XGE_OK ) return ret;
	ret = auditPresentRenderTarget(&demo->tTarget, AUDIT_W, AUDIT_H, BG_COLOR);
	if ( ret != XGE_OK ) return ret;
	ret = xgeEnd();
	if ( ret != XGE_OK ) return ret;
	demo->iFrame++;
	if ( demo->bCaptureDone || ((demo->iMaxFrames > 0) && (demo->iFrame >= demo->iMaxFrames)) ) {
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	audit_demo_t tDemo;
	xge_desc_t tDesc;
	int ret;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iMaxFrames = 0;

	ret = parse_args(&tDemo, argc, argv);
	if ( ret != XGE_OK ) {
		if ( ret == 1 ) return 0;
		fprintf(stderr, "invalid arguments\n");
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = AUDIT_W;
	tDesc.iHeight = AUDIT_H;
	tDesc.sTitle = "audit_shape_basic";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;

	ret = xgeInit(&tDesc);
	if ( ret != XGE_OK ) {
		fprintf(stderr, "xgeInit failed: %d\n", ret);
		return 1;
	}

	ret = xgeRenderTargetCreate(&tDemo.tTarget, AUDIT_W, AUDIT_H);
	if ( ret != XGE_OK ) {
		fprintf(stderr, "render target create failed: %d\n", ret);
		xgeUnit();
		return 1;
	}

	ret = xgeRun(frame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
