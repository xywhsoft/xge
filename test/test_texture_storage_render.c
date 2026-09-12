#include "../xge.h"
#include "../xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct test_t { int checks, failed; } test_t;
#define VERIFY(expr) do { test->checks++; if (!(expr)) { \
	fprintf(stderr, "GPU FAIL line %d: %s\n", __LINE__, #expr); test->failed = 1; goto cleanup; } } while (0)
#define SUCCESS(expr) VERIFY((expr) == XGE_OK)

static xge_texture_storage_info_t storage(xge_texture texture)
{
	xge_texture_storage_info_t info = {0};
	info.iSize = sizeof(info);
	if ( xgeTextureGetStorageInfo(texture, &info) != XGE_OK ) info.iStorage = 0;
	return info;
}
static void fill(unsigned char* pixels, int kind)
{
	int i;
	for ( i = 0; i < 81; i++ ) {
		unsigned char v = (unsigned char)(i * 3);
		pixels[4*i] = pixels[4*i+1] = pixels[4*i+2] = v;
		pixels[4*i+3] = kind == 0 ? v : kind == 1 ? 255 : (unsigned char)(255-v);
		if ( kind == 3 ) pixels[4*i] ^= 127;
		if ( kind == 4 || kind == 5 ) pixels[4*i] = pixels[4*i+1] = pixels[4*i+2] = kind == 4 ? 255 : 0;
	}
}
static int render(xge_render_target target, xge_texture texture, xge_material material,
                  int filter, int blend, unsigned char* pixels)
{
	xge_pass_t pass;
	xge_draw_t draw = {0};
	xge_sampler_t sampler = xgeSamplerDefault();
	int result;
	sampler.iMinFilter = sampler.iMagFilter = filter;
	result = xgeTextureSetSampler(texture, &sampler);
	if ( result != XGE_OK ) return result;
	xgePassInit(&pass, target, XGE_PASS_CLEAR_COLOR, 0x293747ffu);
	result = xgePassBegin(&pass);
	if ( result != XGE_OK ) return result;
	xgeBlendSet(blend);
	draw.pTexture = texture;
	draw.tDst = (xge_rect_t){2.0f, 3.0f, 27.0f, 25.0f};
	draw.iColor = 0xffffffffu;
	draw.iFlags = XGE_DRAW_SCREEN_SPACE;
	if ( material ) xgeMaterialDraw(material, &draw);
	else xgeDrawEx(&draw);
	result = xgePassEnd(&pass);
	if ( result != XGE_OK ) return result;
	return xgeRenderTargetReadPixels(target, pixels, 32 * 4);
}
static const char* font_path(void)
{
	const char* paths[] = {"C:/Windows/Fonts/arial.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/System/Library/Fonts/Supplemental/Arial.ttf"};
	size_t i;
	for ( i = 0; i < sizeof(paths)/sizeof(paths[0]); i++ ) {
		FILE* file = fopen(paths[i], "rb");
		if ( file ) { fclose(file); return paths[i]; }
	}
	return NULL;
}
static int frame(void* user)
{
	test_t* test = (test_t*)user;
	xge_texture_t compact = {0}, original = {0};
	xge_render_target_t target = {0};
	xge_shader_t shader = {0};
	xge_material_t material;
	xge_font_t font = {0};
	xge_font_cache_stats_t font_stats;
	xge_pass_t pass;
	xui_proxy_t proxy = xuiProxyXge();
	xui_surface surface = NULL;
	xui_surface_desc_t desc = {0};
	unsigned char pixels[81*4], cpu[81*4], expected[32*32*4], actual[32*32*4];
	int k, filter, blend, custom, begun = 0, nonzero;
	int native = xgeGraphicsBackendGet().iType == XGE_GPU_BACKEND_OPENGL33 || xgeGraphicsBackendGet().iType == XGE_GPU_BACKEND_GLES30;
	const char* vertex_gl = "#version 330 core\nlayout(location=0) in vec4 aPos;uniform vec2 uResolution;out vec2 uv;void main(){vec2 p=aPos.xy/uResolution*2.-1.;gl_Position=vec4(p.x,-p.y,0.,1.);uv=aPos.zw;}";
	const char* fragment_gl = "#version 330 core\nin vec2 uv;uniform sampler2D uTexture;out vec4 color;void main(){color=texture(uTexture,uv);}";
	const char* vertex_es = "#version 300 es\nprecision highp float;layout(location=0) in vec4 aPos;uniform vec2 uResolution;out vec2 uv;void main(){vec2 p=aPos.xy/uResolution*2.-1.;gl_Position=vec4(p.x,-p.y,0.,1.);uv=aPos.zw;}";
	const char* fragment_es = "#version 300 es\nprecision highp float;in vec2 uv;uniform sampler2D uTexture;out vec4 color;void main(){color=texture(uTexture,uv);}";
	xgeMaterialInit(&material);
	SUCCESS(xgeBegin()); begun = 1;
	SUCCESS(xgeRenderTargetCreate(&target, 32, 32));
	VERIFY(storage(&target.tTexture).iStorage == XGE_TEXTURE_STORAGE_RGBA8);
	VERIFY(storage(&target.tTexture).iCompression == XGE_TEXTURE_COMPRESS_NONE);
	if ( xgeGraphicsBackendGet().iType == XGE_GPU_BACKEND_OPENGL33 ) SUCCESS(xgeShaderCreate(&shader, vertex_gl, fragment_gl));
	else SUCCESS(xgeShaderCreate(&shader, vertex_es, fragment_es));
	xgeMaterialSetShader(&material, &shader);
	/* Full framebuffer equality, including filtered alpha and arbitrary sampler2D. */
	for ( k = 0; k < 6; k++ ) {
		int format = !native || k == 3 ? XGE_TEXTURE_STORAGE_RGBA8 : k == 2 ? XGE_TEXTURE_STORAGE_RG8 : XGE_TEXTURE_STORAGE_R8;
		fill(pixels, k);
		SUCCESS(xgeTextureCreateRGBA(&compact, 9, 9, pixels));
		SUCCESS(xgeTextureCreateRGBAEx(&original, 9, 9, pixels, XGE_TEXTURE_COMPRESS_NONE));
		VERIFY(storage(&compact).iStorage == format);
		VERIFY(storage(&compact).iGpuBytes == (uint64_t)(81 * (format == XGE_TEXTURE_STORAGE_R8 ? 1 : format == XGE_TEXTURE_STORAGE_RG8 ? 2 : 4)));
		SUCCESS(xgeTextureReadPixels(&compact, cpu, 36)); VERIFY(memcmp(pixels, cpu, sizeof(cpu)) == 0);
		for ( filter = 0; filter < 2; filter++ ) for ( blend = 0; blend < 2; blend++ ) for ( custom = 0; custom < 2; custom++ ) {
			int f = filter ? XGE_FILTER_LINEAR : XGE_FILTER_NEAREST;
			int b = blend ? XGE_BLEND_ALPHA : XGE_BLEND_NONE;
			SUCCESS(render(&target, &original, custom ? &material : NULL, f, b, expected));
			SUCCESS(render(&target, &compact, custom ? &material : NULL, f, b, actual));
			VERIFY(memcmp(actual, expected, sizeof(actual)) == 0);
		}
		xgeTextureFree(&compact); xgeTextureFree(&original);
	}
	/* Live dirty updates retain exact coverage, then promote on a color write. */
	fill(pixels, 0);
	SUCCESS(xgeTextureCreateRGBA(&compact, 9, 9, pixels));
	for ( k = 0; k < 2; k++ ) {
		unsigned char patch[4] = {93, 93, 93, 93};
		if ( k ) patch[0] = 12;
		memcpy(pixels + (2*9+3)*4, patch, 4);
		SUCCESS(xgeTextureUpdateRGBA(&compact, 3, 2, 1, 1, patch, 4));
		SUCCESS(xgeTextureCreateRGBAEx(&original, 9, 9, pixels, XGE_TEXTURE_COMPRESS_NONE));
		SUCCESS(render(&target, &original, NULL, XGE_FILTER_LINEAR, XGE_BLEND_ALPHA, expected));
		SUCCESS(render(&target, &compact, NULL, XGE_FILTER_LINEAR, XGE_BLEND_ALPHA, actual));
		VERIFY(memcmp(actual, expected, sizeof(actual)) == 0);
		VERIFY(storage(&compact).iStorage == (native && !k ? XGE_TEXTURE_STORAGE_R8 : XGE_TEXTURE_STORAGE_RGBA8));
		xgeTextureFree(&original);
	}
	xgeTextureFree(&compact);
	/* XUI keeps its existing surface ABI and RGBA read/update contract. */
	fill(pixels, 0);
	for ( k = 0; k < 3; k++ ) {
		uint32_t flags = k == 1 ? XUI_SURFACE_COMPRESS_NONE : k == 2 ? XUI_SURFACE_COMPRESS_LOSSY : XUI_SURFACE_COMPRESS_LOSSLESS;
		SUCCESS(proxy.surfaceCreateRGBA(&proxy, &surface, 9, 9, pixels, 36, flags));
		SUCCESS(proxy.surfaceGetDesc(&proxy, surface, &desc));
		VERIFY((desc.iFlags & XUI_SURFACE_COMPRESS_MASK) == flags);
		SUCCESS(proxy.surfaceReadRGBA(&proxy, surface, cpu, 36)); VERIFY(memcmp(cpu, pixels, sizeof(cpu)) == 0);
		if ( k == 0 ) {
			SUCCESS(xgeTextureCreateRGBAEx(&original, 9, 9, pixels, XGE_TEXTURE_COMPRESS_NONE));
			SUCCESS(render(&target, &original, NULL, XGE_FILTER_LINEAR, XGE_BLEND_ALPHA, expected));
			xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x293747ffu);
			SUCCESS(xgePassBegin(&pass)); xgeBlendSet(XGE_BLEND_ALPHA);
			SUCCESS(proxy.surfaceDraw(&proxy, surface, (xui_rect_t){0,0,9,9}, (xui_rect_t){2,3,27,25}, 0xffffffffu, XUI_SURFACE_DRAW_SCREEN_SPACE));
			SUCCESS(xgePassEnd(&pass)); SUCCESS(xgeRenderTargetReadPixels(&target, actual, 128));
			VERIFY(memcmp(actual, expected, sizeof(actual)) == 0);
			xgeTextureFree(&original);
		}
		proxy.surfaceDestroy(&proxy, surface); surface = NULL;
	}
	/* SVGs and writable XUI targets obey creation flags too. */
	{
		const char* svg = "<svg xmlns='http://www.w3.org/2000/svg' width='9' height='9'><rect width='9' height='9' fill='white'/></svg>";
		SUCCESS(xgeSvgTextureLoadMemoryEx(&compact, svg, (int)strlen(svg), 9, 9, XGE_TEXTURE_COMPRESS_NONE));
		VERIFY(storage(&compact).iStorage == XGE_TEXTURE_STORAGE_RGBA8); xgeTextureFree(&compact);
		SUCCESS(xgeSvgTextureLoadMemory(&compact, svg, (int)strlen(svg), 9, 9));
		VERIFY(storage(&compact).iStorage == (native ? XGE_TEXTURE_STORAGE_R8 : XGE_TEXTURE_STORAGE_RGBA8)); xgeTextureFree(&compact);
		SUCCESS(proxy.surfaceLoadSvgMemory(&proxy, &surface, svg, (int)strlen(svg), 9, 9, XUI_SURFACE_COMPRESS_NONE));
		SUCCESS(proxy.surfaceGetDesc(&proxy, surface, &desc)); VERIFY((desc.iFlags & XUI_SURFACE_COMPRESS_MASK) == XUI_SURFACE_COMPRESS_NONE);
		proxy.surfaceDestroy(&proxy, surface); surface = NULL;
		memset(&desc, 0, sizeof(desc)); desc.iWidth = desc.iHeight = 9; desc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_COMPRESS_LOSSY;
		SUCCESS(proxy.surfaceCreate(&proxy, &surface, &desc));
		SUCCESS(proxy.surfaceClear(&proxy, surface, 0x1b3d5fffu));
		SUCCESS(proxy.surfaceReadRGBA(&proxy, surface, cpu, 36));
		VERIFY(cpu[0] == 27 && cpu[1] == 61 && cpu[2] == 95 && cpu[3] == 255);
		proxy.surfaceDestroy(&proxy, surface); surface = NULL;
	}
	/* Font atlas stays R8 while newly encountered glyphs update dirty regions. */
	if ( font_path() != NULL ) {
		SUCCESS(xgeFontLoad(&font, font_path(), 18));
		for ( k = 0; k < 2; k++ ) {
			xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x00000000u);
			SUCCESS(xgePassBegin(&pass)); xgeBlendSet(XGE_BLEND_ALPHA);
			xgeTextDraw(&font, k ? "Bg" : "A", 1, 1, 0xffffffffu);
			SUCCESS(xgePassEnd(&pass)); SUCCESS(xgeRenderTargetReadPixels(&target, actual, 128));
			for ( nonzero = 0, custom = 0; custom < (int)sizeof(actual); custom++ ) nonzero |= actual[custom];
			VERIFY(nonzero != 0);
			SUCCESS(xgeFontCacheGetStats(&font, &font_stats));
			VERIFY(font_stats.iAtlasPageCount == 1);
			VERIFY(font_stats.iAtlasGpuBytes == (uint64_t)font.tAtlas.iPageWidth * (uint64_t)font.tAtlas.iPageHeight * (native ? 1u : 4u));
		}
		printf("Font atlas: %llu GPU bytes, %llu atlas CPU bytes\n", (unsigned long long)font_stats.iAtlasGpuBytes, (unsigned long long)font_stats.iAtlasCpuBytes);
	} else puts("SKIP font atlas: no test font available");
cleanup:
	if ( surface ) proxy.surfaceDestroy(&proxy, surface);
	xgeFontFree(&font); xgeMaterialFree(&material); xgeShaderFree(&shader);
	xgeTextureFree(&compact); xgeTextureFree(&original); xgeRenderTargetFree(&target);
	if ( begun ) (void)xgeEnd();
	xgeQuit();
	return test->failed ? XGE_ERROR_INVALID_STATE : XGE_OK;
}
int main(void)
{
	xge_desc_t app = {0};
	test_t test = {0};
	int result;
	app.iWidth = app.iHeight = 128; app.sTitle = "XGE texture storage GPU regression";
	if ( xgeInit(&app) != XGE_OK ) return 1;
	result = xgeRun(frame, &test);
	xgeUnit();
	if ( result != XGE_OK || test.failed ) return 1;
	printf("PASS texture storage GPU: %d checks (exact filtered pixels, custom shader, updates, XUI, SVG, fonts)\n", test.checks);
	return 0;
}
