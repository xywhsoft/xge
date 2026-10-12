/* ch108 - Draw APIs.  Texture lifetime spans all frames that reference it. */
#include "tut_capture.h"

static xge_texture_t g_texture;
static int g_texture_ready;

static void draw_scene(void)
{
	unsigned char pixels[32 * 32 * 4];
	int x, y;
	if ( !g_texture_ready ) {
		for (y = 0; y < 32; y++) for (x = 0; x < 32; x++) {
			int i = (y * 32 + x) * 4;
			pixels[i] = (unsigned char)(x * 8); pixels[i + 1] = 200; pixels[i + 2] = (unsigned char)(y * 8); pixels[i + 3] = 255;
		}
		memset(&g_texture, 0, sizeof(g_texture));
		if (xgeTextureCreateRGBA(&g_texture, 32, 32, pixels) != XGE_OK) return;
		g_texture_ready = 1;
	}
	xgeDraw(&g_texture, 60.0f, 50.0f);
	xgeDrawPx(&g_texture, 150, 50);
	{
		xge_draw_t draw; memset(&draw, 0, sizeof(draw)); draw.pTexture = &g_texture;
		draw.tSrc.fW = 32; draw.tSrc.fH = 32; draw.tDst = (xge_rect_t){280,50,96,96}; draw.tOrigin = (xge_vec2_t){48,48}; draw.fRotation = .4f; draw.iColor = XGE_COLOR_RGBA(255,255,255,255); xgeDrawEx(&draw);
		draw.tDst = (xge_rect_t){450,50,64,64}; draw.iFlags = XGE_DRAW_FLIP_X; xgeDrawEx(&draw);
		draw.tSrc = (xge_rect_t){8,8,16,16}; draw.tDst = (xge_rect_t){600,50,128,128}; draw.iFlags = 0; draw.iColor = XGE_COLOR_RGBA(255,200,150,255); xgeDrawEx(&draw);
	}
}
int main(int argc, char** argv) { return tut_run(draw_scene, "ch108-draw", argc, argv); }
