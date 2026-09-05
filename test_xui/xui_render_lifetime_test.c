#include "../src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { SELF, PARENT, ROOT, CONTEXT, REPLACE_ROOT, NEXT_SIBLING, MODES };
static int failures, checks;
#define CHECK(e) do { ++checks; if (!(e)) { ++failures; printf("FAIL %d: %s\n", __LINE__, #e); } } while (0)
typedef struct fixture_t {
    xui_context context;
    xui_widget root, parent, leaf, sibling, replacement;
    int mode, inside, fired, destroyed, siblingPaints, rootPaints, deadPaints, errors;
    xui_draw_begin_proc begin;
    xui_draw_end_proc end;
    xui_surface_destroy_proc destroySurface;
    xui_draw_context draws[32];
    xui_surface surfaces[32];
    int active, begins, ends, destroyedActive;
} fixture_t;
static fixture_t* current;

static int begin_draw(xui_proxy proxy, xui_draw_context* draw, xui_surface surface)
{
    int ret = current->begin(proxy, draw, surface), i;
    if (ret != XUI_OK) return ret;
    for (i = 0; i < 32; ++i) if (current->draws[i] == NULL) break;
    CHECK(i < 32);
    if (i < 32) { current->draws[i] = *draw; current->surfaces[i] = surface; }
    ++current->begins; ++current->active;
    return ret;
}
static int end_draw(xui_proxy proxy, xui_draw_context draw)
{
    int i;
    for (i = 0; i < 32; ++i) if (current->draws[i] == draw) break;
    CHECK(i < 32);
    if (i < 32) { current->draws[i] = NULL; current->surfaces[i] = NULL; }
    ++current->ends; --current->active;
    return current->end(proxy, draw);
}
static void destroy_surface(xui_proxy proxy, xui_surface surface)
{
    int i;
    for (i = 0; i < 32; ++i) if (current->surfaces[i] == surface && current->draws[i] != NULL)
        ++current->destroyedActive;
    current->destroySurface(proxy, surface);
}
static void destroyed(xui_widget widget, void* data, void* user)
{
    fixture_t* f = user;
    CHECK(!f->inside);
    ++f->destroyed;
}
static int paint(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{
    fixture_t* f = user;
    if (widget->bDestroyPending || widget->pContext->bDestroyPending) ++f->deadPaints;
    if (widget == f->sibling) { ++f->siblingPaints; return XUI_OK; }
    if (widget == f->root) { ++f->rootPaints; return XUI_OK; }
    if (widget != f->leaf || f->fired) return XUI_OK;
    ++f->fired;
    f->inside = 1;
    CHECK(f->context->iOperationDepth > 0 && f->active > 0);
    switch (f->mode) {
    case SELF: xuiWidgetDestroy(widget); break;
    case PARENT: xuiWidgetDestroy(f->parent); break;
    case ROOT: xuiWidgetDestroy(f->root); break;
    case CONTEXT: xuiDestroy(f->context); break;
    case REPLACE_ROOT:
        xuiWidgetDestroy(f->root);
        CHECK(xuiSetRootWidget(f->context, f->replacement) == XUI_OK);
        break;
    case NEXT_SIBLING: xuiWidgetDestroy(f->sibling); break;
    }
    CHECK(f->destroyed == 0);
    f->inside = 0;
    return XUI_OK;
}
static void report(xui_context context, const xui_error_info_t* error, void* user)
{
    fixture_t* f = user;
    (void)context; (void)error;
    ++f->errors;
}
static void run_case(int mode, int overlay, int compose, int subtree)
{
    xui_test_proxy_state_t proxy;
    fixture_t f = {0};
    xui_widget_type_desc_t typeDesc = {0};
    xui_widget_type type = NULL;
    xui_cache_policy_t policy = {0};
    xui_surface target = NULL;
    xui_rect_i_t damage = {0, 0, 200, 140};
    int i;
    f.mode = mode; current = &f;
    xuiTestProxyInit(&proxy);
    f.begin = proxy.tProxy.drawBegin; proxy.tProxy.drawBegin = begin_draw;
    f.end = proxy.tProxy.drawEnd; proxy.tProxy.drawEnd = end_draw;
    f.destroySurface = proxy.tProxy.surfaceDestroy; proxy.tProxy.surfaceDestroy = destroy_surface;
    CHECK(xuiCreate(&f.context) == XUI_OK);
    CHECK(xuiSetProxy(f.context, &proxy.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(f.context, 200, 140) == XUI_OK);
    CHECK(xuiWidgetCreate(f.context, &f.root) == XUI_OK);
    CHECK(xuiSetRootWidget(f.context, f.root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(f.root, XUI_LAYOUT_MANUAL) == XUI_OK);
    if (overlay) {
        CHECK(xuiOverlayRoot(f.context) != NULL);
        f.root = xuiOverlayRoot(f.context);
    }
    CHECK(xuiWidgetCreate(f.context, &f.parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(f.root, f.parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(f.parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetRect(f.parent, (xui_rect_t){0, 0, 120, 100}) == XUI_OK);
    typeDesc.iSize = sizeof(typeDesc); typeDesc.sName = "render.lifetime";
    typeDesc.onDestroy = destroyed; typeDesc.pUser = &f;
    CHECK(xuiWidgetRegisterType(f.context, &type, &typeDesc) == XUI_OK);
    CHECK(xuiWidgetCreateTyped(f.context, type, &f.leaf, NULL) == XUI_OK);
    CHECK(xuiWidgetCreateTyped(f.context, type, &f.sibling, NULL) == XUI_OK);
    CHECK(xuiWidgetAddChild(f.parent, f.leaf) == XUI_OK);
    CHECK(xuiWidgetAddChild(f.parent, f.sibling) == XUI_OK);
    CHECK(xuiWidgetSetRect(f.leaf, (xui_rect_t){1, 1, 40, 30}) == XUI_OK);
    CHECK(xuiWidgetSetRect(f.sibling, (xui_rect_t){45, 1, 40, 30}) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(f.leaf, paint, &f) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(f.sibling, paint, &f) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(f.root, paint, &f) == XUI_OK);
    policy.iSize = sizeof(policy); policy.iPolicy = subtree ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
    policy.iFlags = XUI_CACHE_UPDATE_ALL_STATES | XUI_CACHE_CLEAR_ON_UPDATE;
    CHECK(xuiWidgetSetCachePolicy(f.leaf, &policy) == XUI_OK);
    CHECK(xuiWidgetSetCachePolicy(f.root, &policy) == XUI_OK);
    CHECK(xuiWidgetSetCacheStateCount(f.leaf, 3) == XUI_OK);
    for (i = 0; i < 3; ++i) CHECK(xuiWidgetSetCacheStateId(f.leaf, i, 10u + (unsigned)i) == XUI_OK);
    CHECK(xuiWidgetSetStateId(f.leaf, 10) == XUI_OK);
    if (mode == REPLACE_ROOT) {
        CHECK(xuiWidgetCreate(f.context, &f.replacement) == XUI_OK);
        CHECK(xuiWidgetSetRect(f.replacement, (xui_rect_t){0, 0, 200, 140}) == XUI_OK);
    }
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 200, 140, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiLayout(f.context) == XUI_OK);
    CHECK(xuiSetErrorCallback(f.context, report, &f) == XUI_OK);
    CHECK((compose ? xuiRender(f.context, target, &damage, 1) : xuiRenderPrepare(f.context)) == XUI_OK);
    CHECK(f.fired == 1 && f.deadPaints == 0 && f.destroyedActive == 0);
    CHECK(f.errors == 0);
    CHECK(f.active == 0 && f.begins == f.ends);
    CHECK(f.destroyed == (mode == SELF || mode == NEXT_SIBLING ? 1 : 2));
    if (mode != CONTEXT) {
        CHECK(f.context->iOperationDepth == 0);
        if (mode == ROOT) CHECK(overlay ? f.context->pOverlayRoot == NULL : xuiGetRootWidget(f.context) == NULL);
        if (mode == REPLACE_ROOT) CHECK(xuiGetRootWidget(f.context) == f.replacement);
        CHECK(xuiRenderPrepare(f.context) == XUI_OK);
        xuiDestroy(f.context);
    }
    CHECK(f.destroyed == 2 && f.active == 0);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
}
int main(int argc, char** argv)
{
    int mode, overlay, compose, subtree;
    for (mode = 0; mode < MODES; ++mode) {
        if (argc > 1 && mode != atoi(argv[1])) continue;
        for (overlay = 0; overlay < 2; ++overlay)
        for (compose = 0; compose < 2; ++compose)
        for (subtree = 0; subtree < 2; ++subtree) {
            if (mode == REPLACE_ROOT && overlay) continue;
            run_case(mode, overlay, compose, subtree);
        }
    }
    printf("xui_render_lifetime_test: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
