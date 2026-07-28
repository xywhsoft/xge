/* ch84 — 字体族与字重 */
#include "tut_capture.h"

#define FONT_REGULAR "C:/Windows/Fonts/segoeui.ttf"
#define FONT_BOLD    "C:/Windows/Fonts/segoeuib.ttf"

static void draw_scene(void)
{
	xge_font_face pRegular = NULL, pBold = NULL, pResolved = NULL;
	xge_font_family pFamily = NULL;
	xge_font_t font;
	xge_font_face_desc_t faceDesc;
	xge_font_instance_desc_t instDesc;
	int ret;

	memset(&faceDesc, 0, sizeof(faceDesc));
	faceDesc.iSize = sizeof(faceDesc);
	memset(&instDesc, 0, sizeof(instDesc));
	instDesc.iSize = sizeof(instDesc);
	instDesc.fPixelSize = 30.0f;

	ret = xgeFontFaceLoad(&pRegular, FONT_REGULAR, &faceDesc);
	if (ret != XGE_OK) return;
	ret = xgeFontFaceLoad(&pBold, FONT_BOLD, &faceDesc);
	if (ret != XGE_OK) { xgeFontFaceFree(pRegular); return; }

	ret = xgeFontFamilyCreate(&pFamily);
	if (ret != XGE_OK) { xgeFontFaceFree(pRegular); xgeFontFaceFree(pBold); return; }
	xgeFontFamilyAddFace(pFamily, pRegular);
	xgeFontFamilyAddFace(pFamily, pBold);

	/* 解析 Regular (weight=400) */
	xgeFontFamilyResolve(pFamily, 400, 0, &pResolved);
	if (pResolved) {
		xgeFontCreate(&font, pResolved, &instDesc);
		xgeTextDraw(&font, "Regular Weight (400)", 60.0f, 80.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
		xgeFontFree(&font);
	}

	/* 解析 Bold (weight=700) */
	pResolved = NULL;
	xgeFontFamilyResolve(pFamily, 700, 0, &pResolved);
	if (pResolved) {
		xgeFontCreate(&font, pResolved, &instDesc);
		xgeTextDraw(&font, "Bold Weight (700)", 60.0f, 140.0f, XGE_COLOR_RGBA(255, 200, 80, 255));
		xgeFontFree(&font);
	}

	xgeFontFamilyFree(pFamily);
	xgeFontFaceFree(pRegular);
	xgeFontFaceFree(pBold);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch84", argc, argv); }
