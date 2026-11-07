#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define SAMPLE_COUNT 9

typedef struct perf_sample_t {
	double show_us;
	double update_us;
	double prepare_us;
	double tree_us;
	double close_us;
	xui_render_stats_t stats;
} perf_sample_t;

static double now_seconds(void)
{
	static LARGE_INTEGER frequency;
	LARGE_INTEGER value;
	if ( frequency.QuadPart == 0 ) QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&value);
	return (double)value.QuadPart / (double)frequency.QuadPart;
}

static int compare_double(const void* a, const void* b)
{
	double lhs = *(const double*)a;
	double rhs = *(const double*)b;
	return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
}

static void print_summary(const char* shape, int widgets, const char* metric,
	const double* values, int count)
{
	double sorted[SAMPLE_COUNT];
	int p95;
	memcpy(sorted, values, sizeof(double) * (size_t)count);
	qsort(sorted, (size_t)count, sizeof(double), compare_double);
	p95 = (count * 95 + 99) / 100 - 1;
	if ( p95 < 0 ) p95 = 0;
	if ( p95 >= count ) p95 = count - 1;
	printf("summary,%s,%d,warm,%s,%.3f,%.3f,%.3f\n", shape, widgets, metric,
		sorted[count / 2], sorted[p95], sorted[count - 1]);
}

static int create_widget_tree(xui_context context, int count, int balanced, xui_widget* root_out)
{
	xui_cache_policy_t no_cache;
	xui_widget* widgets;
	int i;
	int result = XUI_OK;

	widgets = (xui_widget*)calloc((size_t)count, sizeof(*widgets));
	if ( widgets == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(&no_cache, 0, sizeof(no_cache));
	no_cache.iSize = sizeof(no_cache);
	no_cache.iPolicy = XUI_CACHE_POLICY_NONE;
	for ( i = 0; i < count; ++i ) {
		result = xuiWidgetCreate(context, &widgets[i]);
		if ( result != XUI_OK ) break;
		result = xuiWidgetSetCachePolicy(widgets[i], &no_cache);
		if ( result != XUI_OK ) break;
		result = xuiWidgetSetLayoutType(widgets[i], XUI_LAYOUT_MANUAL);
		if ( result != XUI_OK ) break;
		result = xuiWidgetSetRect(widgets[i], i == 0 ?
			(xui_rect_t){0, 0, 1280, 720} :
			(xui_rect_t){(float)(i % 1270), (float)(i % 710), 1, 1});
		if ( result != XUI_OK ) break;
		if ( i > 0 ) {
			int parent = balanced ? (i - 1) / 2 : 0;
			result = xuiWidgetAddChild(widgets[parent], widgets[i]);
			if ( result != XUI_OK ) break;
		}
	}
	if ( result == XUI_OK ) {
		*root_out = widgets[0];
		result = xuiSetRootWidget(context, widgets[0]);
	}
	if ( result != XUI_OK && widgets[0] != NULL ) xuiWidgetDestroy(widgets[0]);
	free(widgets);
	return result;
}

static int sample_open(xui_context context, xui_msgbox* box, const xui_msgbox_desc_t* desc,
	perf_sample_t* sample)
{
	double start;
	int result;
	memset(sample, 0, sizeof(*sample));
	start = now_seconds();
	result = xuiMsgBoxShow(context, box, desc);
	sample->show_us = (now_seconds() - start) * 1000000.0;
	if ( result != XUI_OK ) return result;
	start = now_seconds();
	result = xuiUpdate(context, 0.016f);
	sample->update_us = (now_seconds() - start) * 1000000.0;
	if ( result != XUI_OK ) return result;
	start = now_seconds();
	result = xuiRenderPrepare(context);
	sample->prepare_us = (now_seconds() - start) * 1000000.0;
	if ( result != XUI_OK ) return result;
	start = now_seconds();
	result = xuiBuildRenderTree(context);
	sample->tree_us = (now_seconds() - start) * 1000000.0;
	if ( result != XUI_OK ) return result;
	return xuiGetRenderStats(context, &sample->stats);
}

static int sample_close(xui_context context, xui_msgbox box, perf_sample_t* sample)
{
	double start = now_seconds();
	int result = xuiMsgBoxSetOpen(box, 0);
	sample->close_us = (now_seconds() - start) * 1000000.0;
	if ( result != XUI_OK ) return result;
	return xuiUpdate(context, 0.016f);
}

static void print_sample(const char* shape, int widgets, const char* mode, int iteration,
	const perf_sample_t* sample)
{
	printf("sample,%s,%d,%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%d,%d,%d,%llu,%llu,%u,%u\n",
		shape, widgets, mode, iteration, sample->show_us, sample->update_us,
		sample->prepare_us, sample->tree_us, sample->close_us,
		sample->stats.iPrepareVisitedWidgets, sample->stats.iPrepareSkippedSubtrees,
		sample->stats.iRenderNodesReused, sample->stats.iRenderNodesUpdated,
		sample->stats.iRenderNodesInserted, sample->stats.iRenderNodesRemoved,
		(unsigned long long)sample->stats.iDirtyPropagationSteps,
		(unsigned long long)sample->stats.iEventMaskPropagationSteps,
		sample->stats.iRootStructureRevision, sample->stats.iOverlayStructureRevision);
}

static int run_case(int widget_count, int balanced)
{
	xui_test_proxy_state_t proxy_state;
	xui_context context = NULL;
	xui_widget root = NULL;
	xui_surface target = NULL;
	xui_font font = NULL;
	xui_msgbox box = NULL;
	xui_msgbox_desc_t desc;
	perf_sample_t cold;
	perf_sample_t samples[SAMPLE_COUNT];
	double values[SAMPLE_COUNT];
	const char* shape = balanced ? "balanced" : "wide";
	int i;
	int result;

	xuiTestProxyInit(&proxy_state);
	result = xuiCreate(&context);
	if ( result != XUI_OK ) goto cleanup;
	result = xuiSetProxy(context, &proxy_state.tProxy);
	if ( result != XUI_OK ) goto cleanup;
	result = proxy_state.tProxy.fontLoadMemory(&proxy_state.tProxy, &font,
		"body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( result != XUI_OK ) goto cleanup;
	result = xuiSetDefaultFont(context, font);
	if ( result != XUI_OK ) goto cleanup;
	result = xuiSetViewportSize(context, 1280, 720);
	if ( result != XUI_OK ) goto cleanup;
	result = xuiTestSurfaceCreate(&proxy_state, &target, 1280, 720, XUI_SURFACE_USAGE_TARGET);
	if ( result != XUI_OK ) goto cleanup;
	result = create_widget_tree(context, widget_count, balanced, &root);
	if ( result != XUI_OK ) goto cleanup;
	result = xuiRender(context, target, NULL, 0);
	if ( result != XUI_OK ) goto cleanup;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sTitle = "Performance";
	desc.sMessage = "Retained message box";
	desc.pFont = font;
	desc.iType = XUI_MSGBOX_ICON_INFO;
	desc.iButtons = XUI_MSGBOX_BUTTON_OK;
	desc.bHasModal = 1;
	desc.bModal = 1;
	result = sample_open(context, &box, &desc, &cold);
	if ( result != XUI_OK ) goto cleanup;
	result = sample_close(context, box, &cold);
	if ( result != XUI_OK ) goto cleanup;
	print_sample(shape, widget_count, "cold", 0, &cold);
	result = xuiRender(context, target, NULL, 0);
	if ( result != XUI_OK ) goto cleanup;

	for ( i = 0; i < SAMPLE_COUNT; ++i ) {
		result = sample_open(context, &box, &desc, &samples[i]);
		if ( result != XUI_OK ) goto cleanup;
		result = sample_close(context, box, &samples[i]);
		if ( result != XUI_OK ) goto cleanup;
		print_sample(shape, widget_count, "warm", i, &samples[i]);
		result = xuiRender(context, target, NULL, 0);
		if ( result != XUI_OK ) goto cleanup;
	}
	for ( i = 0; i < SAMPLE_COUNT; ++i ) values[i] = samples[i].show_us;
	print_summary(shape, widget_count, "show_us", values, SAMPLE_COUNT);
	for ( i = 0; i < SAMPLE_COUNT; ++i ) values[i] = samples[i].prepare_us;
	print_summary(shape, widget_count, "prepare_us", values, SAMPLE_COUNT);
	for ( i = 0; i < SAMPLE_COUNT; ++i ) values[i] = samples[i].tree_us;
	print_summary(shape, widget_count, "tree_us", values, SAMPLE_COUNT);
	result = XUI_OK;

cleanup:
	if ( box != NULL ) xuiMsgBoxDestroy(box);
	if ( target != NULL ) proxy_state.tProxy.surfaceDestroy(&proxy_state.tProxy, target);
	if ( context != NULL ) xuiDestroy(context);
	if ( font != NULL ) proxy_state.tProxy.fontDestroy(&proxy_state.tProxy, font);
	return result;
}

int main(int argc, char** argv)
{
	static const int sizes[] = {1000, 10000, 50000};
	int size_count = (argc > 1 && strcmp(argv[1], "--quick") == 0) ? 1 : 3;
	int i;
	int shape;
	int result;
	printf("kind,shape,widgets,mode,iteration_or_metric,show_us_or_median,update_us_or_p95,prepare_us_or_max,tree_us,close_us,prepare_visited,prepare_skipped,reused,updated,inserted,removed,dirty_steps,event_steps,root_revision,overlay_revision\n");
	for ( i = 0; i < size_count; ++i ) {
		for ( shape = 0; shape < 2; ++shape ) {
			result = run_case(sizes[i], shape);
			if ( result != XUI_OK ) {
				fprintf(stderr, "benchmark failed: widgets=%d shape=%d status=%d\n", sizes[i], shape, result);
				return 1;
			}
		}
	}
	return 0;
}
