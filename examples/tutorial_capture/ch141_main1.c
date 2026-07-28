/* ch141 — Image 图像 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_image_desc_t desc;
	xui_surface_desc_t surfDesc;
	xui_surface pSurface;
	xui_widget pImage;
	unsigned char pixels[64 * 64 * 4];
	int x, y, ret;

	/* 生成测试图像 */
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 64; x++) {
			int idx = (y * 64 + x) * 4;
			pixels[idx + 0] = (unsigned char)(x * 4);
			pixels[idx + 1] = (unsigned char)(y * 4);
			pixels[idx + 2] = 180;
			pixels[idx + 3] = 255;
		}
	}

	/* 创建 surface */
	memset(&surfDesc, 0, sizeof(surfDesc));
	surfDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	surfDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	surfDesc.iWidth = 64;
	surfDesc.iHeight = 64;
	ret = ctx->tProxy.surfaceCreate(&ctx->tProxy, &pSurface, &surfDesc);
	if (ret != XUI_OK) return ret;
	ctx->tProxy.surfaceUpdateRGBA(&ctx->tProxy, pSurface, (xui_rect_i_t){0, 0, 64, 64}, pixels, 64 * 4);

	/* 创建图像控件 */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pSurface = pSurface;
	desc.tSrc.fW = 64.0f; desc.tSrc.fH = 64.0f;
	ret = xuiImageCreate(ctx->pContext, &pImage, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pImage, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pImage, (xui_vec2_t){128.0f, 128.0f});
	return xuiWidgetAddChild(ctx->pRoot, pImage);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch141", argc, argv); }
