#include "../xlayout.h"

#include <stdio.h>
#include <time.h>

typedef struct {
	float width;
	float height;
} benchmark_leaf_t;

static bool measure_leaf(
	xlayout_context_t* context,
	xlayout_node_t node,
	const xlayout_constraints_t* constraints,
	void* user_data,
	xlayout_measure_t* output)
{
	benchmark_leaf_t* leaf = (benchmark_leaf_t*)user_data;
	(void)context;
	(void)node;
	(void)constraints;
	output->width = leaf->width;
	output->height = leaf->height;
	output->baseline = leaf->height;
	return true;
}

int main(void)
{
	enum { NODE_COUNT = 10000, PASSES = 200 };
	xlayout_context_config_t config = { NODE_COUNT + 1u, NODE_COUNT + 1u };
	xlayout_context_t* context = xLayoutContextCreate(&config);
	benchmark_leaf_t leaf = { 12.0f, 8.0f };
	xlayout_node_t root;
	xlayout_style_t root_style;
	clock_t begin;
	double milliseconds;
	int index;
	xlayout_stats_t stats;
	if ( context == NULL ) return 1;
	root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	root_style = xLayoutStyleDefault();
	root_style.container.format = XLAYOUT_FORMAT_FLOW;
	root_style.container.column_gap = 2.0f;
	root_style.container.row_gap = 2.0f;
	xLayoutNodeSetStyle(context, root, &root_style);
	for ( index = 0; index < NODE_COUNT; ++index ) {
		xlayout_node_t node = xLayoutNodeCreate(context, XLAYOUT_ROLE_LEAF);
		if ( node == XLAYOUT_NODE_INVALID
			|| !xLayoutNodeSetMeasure(context, node, measure_leaf, &leaf)
			|| !xLayoutNodeAppend(context, root, node) ) return 2;
	}
	if ( !xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 1920, 1080 }) ) return 3;
	begin = clock();
	for ( index = 0; index < PASSES; ++index ) {
		if ( !xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 1920, 1080 }) ) return 4;
	}
	milliseconds = (double)(clock() - begin) * 1000.0 / (double)CLOCKS_PER_SEC;
	xLayoutContextGetStats(context, &stats);
	printf("xLayout benchmark: %d nodes x %d cached passes = %.2f ms (%.3f ms/pass)\n",
		NODE_COUNT, PASSES, milliseconds, milliseconds / PASSES);
	printf("measure calls=%llu cache hits=%llu workspace=%u\n",
		(unsigned long long)stats.measure_calls,
		(unsigned long long)stats.measure_cache_hits,
		stats.workspace_capacity);
	xLayoutNodeDestroy(context, root);
	xLayoutContextDestroy(context);
	return 0;
}
