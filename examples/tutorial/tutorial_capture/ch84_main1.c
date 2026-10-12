/* ch84 - Font family resolution and weight selection.
 * Font objects are retained for the application's lifetime because XGE submits
 * text draws at pass end; releasing a font immediately after TextDraw is unsafe.
 */
#include "tut_capture.h"

#define FONT_REGULAR "C:/Windows/Fonts/segoeui.ttf"
#define FONT_BOLD    "C:/Windows/Fonts/segoeuib.ttf"

static xge_font_face g_regular, g_bold, g_resolved;
static xge_font_family g_family;
static xge_font_t g_font_regular, g_font_bold;
static int g_ready;

static void draw_scene(void)
{
	xge_font_face_desc_t face_desc;
	xge_font_instance_desc_t instance_desc;
	memset(&face_desc, 0, sizeof(face_desc)); face_desc.iSize = sizeof(face_desc);
	memset(&instance_desc, 0, sizeof(instance_desc)); instance_desc.iSize = sizeof(instance_desc); instance_desc.fPixelSize = 30.0f;
	if ( !g_ready ) {
		if (xgeFontFaceLoad(&g_regular, FONT_REGULAR, &face_desc) != XGE_OK ||
			xgeFontFaceLoad(&g_bold, FONT_BOLD, &face_desc) != XGE_OK ||
			xgeFontFamilyCreate(&g_family) != XGE_OK) return;
		(void)xgeFontFamilyAddFace(g_family, g_regular);
		(void)xgeFontFamilyAddFace(g_family, g_bold);
		if (xgeFontFamilyResolve(g_family, 400, 0, &g_resolved) != XGE_OK ||
			xgeFontCreate(&g_font_regular, g_resolved, &instance_desc) != XGE_OK) return;
		g_resolved = NULL;
		if (xgeFontFamilyResolve(g_family, 700, 0, &g_resolved) != XGE_OK ||
			xgeFontCreate(&g_font_bold, g_resolved, &instance_desc) != XGE_OK) return;
		g_ready = 1;
	}
	xgeTextDraw(&g_font_regular, "Regular Weight (400)", 60.0f, 90.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeTextDraw(&g_font_bold, "Bold Weight (700)", 60.0f, 160.0f, XGE_COLOR_RGBA(255, 200, 80, 255));
}
int main(int argc, char** argv) { return tut_run(draw_scene, "ch84-font-family", argc, argv); }
