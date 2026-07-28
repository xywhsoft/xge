/* ch153 — Breadcrumb 面包屑 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_breadcrumb_desc_t desc;
	xui_widget pBreadcrumb;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiBreadcrumbCreate(ctx->pContext, &pBreadcrumb, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBreadcrumb, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pBreadcrumb);

	/* 添加面包屑项 */
	xuiBreadcrumbAddItem(pBreadcrumb, "Home", 1, 0);
	xuiBreadcrumbAddItem(pBreadcrumb, "Products", 1, 1);
	xuiBreadcrumbAddItem(pBreadcrumb, "Category", 1, 2);
	xuiBreadcrumbAddItem(pBreadcrumb, "Item", 0, 3);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch153", argc, argv); }
