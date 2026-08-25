#include "../xlayout.h"

#include <stdio.h>
#include <string.h>

typedef struct {
	float width;
	float height;
	uint32_t calls;
} test_leaf_t;

typedef struct {
	float widths[4];
	float height;
	float available[4];
} test_fragments_t;

typedef struct {
	xlayout_node_t first;
	xlayout_node_t second;
	xlayout_node_t foreign;
	uint32_t calls;
	bool foreign_rejected;
} test_custom_arrange_t;

static int failures;

static float absolute(float value)
{
	return value < 0.0f ? -value : value;
}

static void expect_true(bool value, const char* message)
{
	if ( !value ) {
		printf("FAIL: %s\n", message);
		failures++;
	}
}

static void expect_near(float actual, float expected, const char* message)
{
	if ( absolute(actual - expected) > 0.01f ) {
		printf("FAIL: %s (actual %.3f, expected %.3f)\n", message, actual, expected);
		failures++;
	}
}

static bool measure_leaf(
	xlayout_context_t* context,
	xlayout_node_t node,
	const xlayout_constraints_t* constraints,
	void* user_data,
	xlayout_measure_t* output)
{
	test_leaf_t* leaf = (test_leaf_t*)user_data;
	(void)context;
	(void)node;
	leaf->calls++;
	output->width = leaf->width < constraints->max_width ? leaf->width : constraints->max_width;
	output->height = leaf->height < constraints->max_height ? leaf->height : constraints->max_height;
	output->baseline = output->height * 0.75f;
	return true;
}

static uint32_t fragment_count(
	xlayout_context_t* context,
	xlayout_node_t node,
	float max_inline_size,
	void* user_data)
{
	(void)context;
	(void)node;
	(void)max_inline_size;
	(void)user_data;
	return 4u;
}

static bool measure_fragment(
	xlayout_context_t* context,
	xlayout_node_t node,
	uint32_t index,
	float available_inline_size,
	void* user_data,
	xlayout_fragment_spec_t* output)
{
	test_fragments_t* data = (test_fragments_t*)user_data;
	(void)context;
	(void)node;
	if ( index >= 4u ) return false;
	data->available[index] = available_inline_size;
	output->width = data->widths[index];
	output->height = data->height;
	output->baseline = data->height * 0.8f;
	output->tag = 100u + index;
	return true;
}

static xlayout_node_t make_leaf(xlayout_context_t* context, test_leaf_t* data)
{
	xlayout_node_t node = xLayoutNodeCreate(context, XLAYOUT_ROLE_LEAF);
	expect_true(node != XLAYOUT_NODE_INVALID, "leaf created");
	expect_true(xLayoutNodeSetMeasure(context, node, measure_leaf, data), "leaf callback set");
	return node;
}

static bool arrange_children(
	xlayout_context_t* context,
	xlayout_node_t node,
	xlayout_rect_t content_rect,
	void* user_data)
{
	test_custom_arrange_t* data = (test_custom_arrange_t*)user_data;
	float half = content_rect.width * 0.5f;
	data->calls++;
	data->foreign_rejected = !xLayoutArrangeChild(context, node, data->foreign, content_rect);
	return xLayoutArrangeChild(context, node, data->first,
		(xlayout_rect_t){ content_rect.x, content_rect.y, half, content_rect.height })
		&& xLayoutArrangeChild(context, node, data->second,
			(xlayout_rect_t){ content_rect.x + half, content_rect.y, content_rect.width - half, content_rect.height });
}

static void test_custom_arrange_children(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t leaf = { 10.0f, 10.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t other = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	test_custom_arrange_t data;
	xlayout_result_t first_result;
	xlayout_result_t second_result;
	memset(&data, 0, sizeof(data));
	data.first = make_leaf(context, &leaf);
	data.second = make_leaf(context, &leaf);
	data.foreign = make_leaf(context, &leaf);
	expect_true(xLayoutNodeAppend(context, root, data.first), "custom first appended");
	expect_true(xLayoutNodeAppend(context, root, data.second), "custom second appended");
	expect_true(xLayoutNodeAppend(context, other, data.foreign), "foreign child appended");
	expect_true(xLayoutNodeSetArrangeChildren(context, root, arrange_children, &data), "custom arrange callback set");
	expect_true(!xLayoutArrangeChild(context, root, data.first, (xlayout_rect_t){ 0, 0, 1, 1 }),
		"direct child arrange is rejected outside callback");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 5, 7, 100, 30 }), "custom arrange succeeds");
	expect_true(data.calls == 1u, "custom arrange called once");
	expect_true(data.foreign_rejected, "custom arrange rejects child from another parent");
	xLayoutNodeGetResult(context, data.first, &first_result);
	xLayoutNodeGetResult(context, data.second, &second_result);
	expect_near(first_result.rect.x, 5.0f, "custom first x");
	expect_near(first_result.rect.width, 50.0f, "custom first width");
	expect_near(second_result.rect.x, 55.0f, "custom second x");
	expect_near(second_result.rect.width, 50.0f, "custom second width");
	expect_true(xLayoutNodeSetArrangeChildren(context, root, NULL, NULL), "custom arrange callback cleared");
	xLayoutContextDestroy(context);
}

static void test_handles_and_cache(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t leaf_data = { 20.0f, 10.0f, 0u };
	xlayout_node_t old_node = make_leaf(context, &leaf_data);
	xlayout_constraints_t constraints = xLayoutConstraints(100.0f, 100.0f);
	xlayout_measure_t measured;
	xlayout_stats_t stats;
	expect_true(xLayoutMeasure(context, old_node, &constraints, &measured), "first measure succeeds");
	expect_true(xLayoutMeasure(context, old_node, &constraints, &measured), "cached measure succeeds");
	expect_true(leaf_data.calls == 1u, "leaf callback is cached");
	xLayoutContextGetStats(context, &stats);
	expect_true(stats.measure_cache_hits >= 1u, "cache hit is reported");
	xLayoutNodeDestroy(context, old_node);
	expect_true(!xLayoutNodeIsValid(context, old_node), "destroyed handle is stale");
	expect_true(xLayoutNodeCreate(context, XLAYOUT_ROLE_LEAF) != old_node, "slot reuse changes generation");
	xLayoutContextDestroy(context);
}

static void test_stack(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t a = { 40.0f, 20.0f, 0u };
	test_leaf_t b = { 20.0f, 10.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t left = make_leaf(context, &a);
	xlayout_node_t right = make_leaf(context, &b);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t right_style = xLayoutStyleDefault();
	xlayout_result_t result;
	root_style.container.format = XLAYOUT_FORMAT_STACK;
	root_style.container.axis = XLAYOUT_HORIZONTAL;
	root_style.container.column_gap = 10.0f;
	root_style.container.align_items = XLAYOUT_ALIGN_CENTER;
	right_style.item.grow = 1.0f;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetStyle(context, right, &right_style);
	xLayoutNodeAppend(context, root, left);
	xLayoutNodeAppend(context, root, right);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 40 }), "stack arrange succeeds");
	xLayoutNodeGetResult(context, left, &result);
	expect_near(result.rect.x, 0.0f, "stack first x");
	expect_near(result.rect.y, 10.0f, "stack first centered y");
	xLayoutNodeGetResult(context, right, &result);
	expect_near(result.rect.x, 50.0f, "stack second x includes gap");
	expect_near(result.rect.width, 50.0f, "stack grow consumes free space");
	expect_near(result.rect.y, 15.0f, "stack second centered y");
	xLayoutContextDestroy(context);
}

static void test_stack_column_shrink_respects_minimum(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t content_data = { 120.0f, 240.0f, 0u };
	test_leaf_t footer_data = { 120.0f, 64.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t content = make_leaf(context, &content_data);
	xlayout_node_t footer = make_leaf(context, &footer_data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t footer_style = xLayoutStyleDefault();
	xlayout_result_t result;
	root_style.container.axis = XLAYOUT_VERTICAL;
	footer_style.size.height = (xlayout_length_t){ XLAYOUT_LENGTH_FIXED, 64.0f };
	footer_style.size.min_height = 64.0f;
	expect_true(xLayoutNodeSetStyle(context, root, &root_style), "minimum column root style set");
	expect_true(xLayoutNodeSetStyle(context, footer, &footer_style), "minimum column footer style set");
	expect_true(xLayoutNodeAppend(context, root, content), "minimum column content appended");
	expect_true(xLayoutNodeAppend(context, root, footer), "minimum column footer appended");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 120, 100 }), "minimum column arrange succeeds");
	xLayoutNodeGetResult(context, footer, &result);
	expect_near(result.rect.height, 64.0f, "column footer remains at minimum height");
	xLayoutNodeGetResult(context, content, &result);
	expect_near(result.rect.height, 36.0f, "column content consumes shrink deficit");
	xLayoutContextDestroy(context);
}

static void test_stack_minimum_overflow(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t first_data = { 120.0f, 30.0f, 0u };
	test_leaf_t second_data = { 120.0f, 36.0f, 0u };
	test_leaf_t third_data = { 120.0f, 64.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t first = make_leaf(context, &first_data);
	xlayout_node_t second = make_leaf(context, &second_data);
	xlayout_node_t third = make_leaf(context, &third_data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t first_style = xLayoutStyleDefault();
	xlayout_style_t second_style = xLayoutStyleDefault();
	xlayout_style_t third_style = xLayoutStyleDefault();
	xlayout_result_t first_result;
	xlayout_result_t second_result;
	xlayout_result_t third_result;
	root_style.container.axis = XLAYOUT_VERTICAL;
	first_style.size.min_height = 30.0f;
	second_style.size.min_height = 36.0f;
	third_style.size.min_height = 64.0f;
	expect_true(xLayoutNodeSetStyle(context, root, &root_style), "overflow root style set");
	expect_true(xLayoutNodeSetStyle(context, first, &first_style), "overflow first style set");
	expect_true(xLayoutNodeSetStyle(context, second, &second_style), "overflow second style set");
	expect_true(xLayoutNodeSetStyle(context, third, &third_style), "overflow third style set");
	expect_true(xLayoutNodeAppend(context, root, first), "overflow first appended");
	expect_true(xLayoutNodeAppend(context, root, second), "overflow second appended");
	expect_true(xLayoutNodeAppend(context, root, third), "overflow third appended");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 120, 80 }), "minimum overflow arrange succeeds");
	xLayoutNodeGetResult(context, first, &first_result);
	xLayoutNodeGetResult(context, second, &second_result);
	xLayoutNodeGetResult(context, third, &third_result);
	expect_near(first_result.rect.height, 30.0f, "overflow first minimum preserved");
	expect_near(second_result.rect.height, 36.0f, "overflow second minimum preserved");
	expect_near(third_result.rect.height, 64.0f, "overflow third minimum preserved");
	expect_true(first_result.rect.height == first_result.rect.height
		&& second_result.rect.height == second_result.rect.height
		&& third_result.rect.height == third_result.rect.height, "overflow dimensions remain finite");
	expect_true(third_result.rect.y + third_result.rect.height > 80.0f, "minimum items overflow the parent instead of shrinking");
	xLayoutContextDestroy(context);
}

static void test_stack_row_shrink_respects_minimum(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t content_data = { 200.0f, 24.0f, 0u };
	test_leaf_t input_data = { 64.0f, 24.0f, 0u };
	test_leaf_t button_data = { 36.0f, 24.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t content = make_leaf(context, &content_data);
	xlayout_node_t input = make_leaf(context, &input_data);
	xlayout_node_t button = make_leaf(context, &button_data);
	xlayout_style_t input_style = xLayoutStyleDefault();
	xlayout_style_t button_style = xLayoutStyleDefault();
	xlayout_result_t result;
	input_style.size.min_width = 64.0f;
	button_style.size.min_width = 36.0f;
	expect_true(xLayoutNodeSetStyle(context, input, &input_style), "row input style set");
	expect_true(xLayoutNodeSetStyle(context, button, &button_style), "row button style set");
	expect_true(xLayoutNodeAppend(context, root, content), "row content appended");
	expect_true(xLayoutNodeAppend(context, root, input), "row input appended");
	expect_true(xLayoutNodeAppend(context, root, button), "row button appended");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 24 }), "minimum row arrange succeeds");
	xLayoutNodeGetResult(context, input, &result);
	expect_near(result.rect.width, 64.0f, "row input remains at minimum width");
	xLayoutNodeGetResult(context, button, &result);
	expect_near(result.rect.width, 36.0f, "row button remains at minimum width");
	xLayoutContextDestroy(context);
}

static void test_stack_shrink_still_distributes(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t first_data = { 80.0f, 20.0f, 0u };
	test_leaf_t second_data = { 80.0f, 20.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t first = make_leaf(context, &first_data);
	xlayout_node_t second = make_leaf(context, &second_data);
	xlayout_style_t item_style = xLayoutStyleDefault();
	xlayout_result_t result;
	item_style.size.min_width = 20.0f;
	expect_true(xLayoutNodeSetStyle(context, first, &item_style), "ordinary shrink first style set");
	expect_true(xLayoutNodeSetStyle(context, second, &item_style), "ordinary shrink second style set");
	expect_true(xLayoutNodeAppend(context, root, first), "ordinary shrink first appended");
	expect_true(xLayoutNodeAppend(context, root, second), "ordinary shrink second appended");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 20 }), "ordinary shrink arrange succeeds");
	xLayoutNodeGetResult(context, first, &result);
	expect_near(result.rect.width, 50.0f, "ordinary shrink first width");
	xLayoutNodeGetResult(context, second, &result);
	expect_near(result.rect.width, 50.0f, "ordinary shrink second width");
	xLayoutContextDestroy(context);
}

static void test_stack_shrink_redistributes_after_minimum(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t first_data = { 100.0f, 20.0f, 0u };
	test_leaf_t second_data = { 100.0f, 20.0f, 0u };
	test_leaf_t third_data = { 100.0f, 20.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t first = make_leaf(context, &first_data);
	xlayout_node_t second = make_leaf(context, &second_data);
	xlayout_node_t third = make_leaf(context, &third_data);
	xlayout_style_t first_style = xLayoutStyleDefault();
	xlayout_result_t result;
	first_style.size.min_width = 80.0f;
	expect_true(xLayoutNodeSetStyle(context, first, &first_style), "redistribution first style set");
	expect_true(xLayoutNodeAppend(context, root, first), "redistribution first appended");
	expect_true(xLayoutNodeAppend(context, root, second), "redistribution second appended");
	expect_true(xLayoutNodeAppend(context, root, third), "redistribution third appended");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 150, 20 }), "redistribution arrange succeeds");
	xLayoutNodeGetResult(context, first, &result);
	expect_near(result.rect.width, 80.0f, "redistribution first stops at minimum");
	xLayoutNodeGetResult(context, second, &result);
	expect_near(result.rect.width, 35.0f, "redistribution second receives remaining deficit");
	xLayoutNodeGetResult(context, third, &result);
	expect_near(result.rect.width, 35.0f, "redistribution third receives remaining deficit");
	xLayoutContextDestroy(context);
}

static void test_hidden_child_becomes_visible(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t leaf = { 20.0f, 10.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t child = make_leaf(context, &leaf);
	xlayout_style_t child_style = xLayoutStyleDefault();
	xlayout_result_t result;
	child_style.size.width = (xlayout_length_t){ XLAYOUT_LENGTH_FIXED, 20.0f };
	child_style.size.height = (xlayout_length_t){ XLAYOUT_LENGTH_FIXED, 10.0f };
	child_style.item.align_self = XLAYOUT_ALIGN_CENTER;
	child_style.visible = false;
	expect_true(xLayoutNodeSetStyle(context, child, &child_style), "hidden child style set");
	expect_true(xLayoutNodeAppend(context, root, child), "hidden child appended");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 40 }),
		"hidden child arrange succeeds");
	expect_true(leaf.calls == 0u, "hidden child is not measured");
	child_style.visible = true;
	expect_true(xLayoutNodeSetStyle(context, child, &child_style), "visible child style set");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 40 }),
		"visible child arrange succeeds");
	xLayoutNodeGetResult(context, child, &result);
	expect_true(leaf.calls == 1u, "visible child is measured after cached parent layout");
	expect_near(result.rect.width, 20.0f, "visible child width is restored");
	expect_near(result.rect.height, 10.0f, "visible child height is restored");
	xLayoutContextDestroy(context);
}

static void test_layer(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t data = { 30.0f, 12.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t child = make_leaf(context, &data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t child_style = xLayoutStyleDefault();
	xlayout_result_t result;
	root_style.container.format = XLAYOUT_FORMAT_LAYER;
	root_style.container.align_items = XLAYOUT_ALIGN_START;
	root_style.container.padding = (xlayout_edges_t){ 5, 7, 0, 0 };
	child_style.item.x = 11.0f;
	child_style.item.y = 13.0f;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetStyle(context, child, &child_style);
	xLayoutNodeAppend(context, root, child);
	xLayoutArrange(context, root, (xlayout_rect_t){ 10, 20, 100, 80 });
	xLayoutNodeGetResult(context, child, &result);
	expect_near(result.rect.x, 26.0f, "layer x includes content origin");
	expect_near(result.rect.y, 40.0f, "layer y includes content origin");
	xLayoutContextDestroy(context);
}

static void test_dock(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t left_data = { 40.0f, 10.0f, 0u };
	test_leaf_t top_data = { 10.0f, 20.0f, 0u };
	test_leaf_t fill_data = { 10.0f, 10.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t left = make_leaf(context, &left_data);
	xlayout_node_t top = make_leaf(context, &top_data);
	xlayout_node_t fill = make_leaf(context, &fill_data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t item_style = xLayoutStyleDefault();
	xlayout_result_t result;
	root_style.container.format = XLAYOUT_FORMAT_DOCK;
	xLayoutNodeSetStyle(context, root, &root_style);
	item_style.item.dock = XLAYOUT_DOCK_LEFT;
	xLayoutNodeSetStyle(context, left, &item_style);
	item_style.item.dock = XLAYOUT_DOCK_TOP;
	xLayoutNodeSetStyle(context, top, &item_style);
	item_style.item.dock = XLAYOUT_DOCK_FILL;
	xLayoutNodeSetStyle(context, fill, &item_style);
	xLayoutNodeAppend(context, root, left);
	xLayoutNodeAppend(context, root, top);
	xLayoutNodeAppend(context, root, fill);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 300, 100 }), "dock arrange succeeds");
	xLayoutNodeGetResult(context, left, &result);
	expect_near(result.rect.width, 40.0f, "dock left width");
	expect_near(result.rect.height, 100.0f, "dock left fills cross axis");
	xLayoutNodeGetResult(context, top, &result);
	expect_near(result.rect.x, 40.0f, "dock top follows left");
	expect_near(result.rect.width, 260.0f, "dock top fills remaining width");
	expect_near(result.rect.height, 20.0f, "dock top height");
	xLayoutNodeGetResult(context, fill, &result);
	expect_near(result.rect.x, 40.0f, "dock fill x");
	expect_near(result.rect.y, 20.0f, "dock fill y");
	expect_near(result.rect.width, 260.0f, "dock fill width");
	expect_near(result.rect.height, 80.0f, "dock fill height");
	xLayoutContextDestroy(context);
}

static void test_track(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t a = { 30.0f, 15.0f, 0u };
	test_leaf_t b = { 20.0f, 15.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t left = make_leaf(context, &a);
	xlayout_node_t right = make_leaf(context, &b);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t item_style = xLayoutStyleDefault();
	xlayout_track_t columns[2] = { xLayoutTrackFixed(40.0f), xLayoutTrackFraction(1.0f) };
	xlayout_track_t rows[1] = { xLayoutTrackFraction(1.0f) };
	xlayout_result_t result;
	root_style.container.format = XLAYOUT_FORMAT_TRACK;
	root_style.container.column_gap = 5.0f;
	root_style.container.align_items = XLAYOUT_ALIGN_STRETCH;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetColumns(context, root, columns, 2u);
	xLayoutNodeSetRows(context, root, rows, 1u);
	xLayoutNodeSetStyle(context, left, &item_style);
	item_style.item.column = 1u;
	xLayoutNodeSetStyle(context, right, &item_style);
	xLayoutNodeAppend(context, root, left);
	xLayoutNodeAppend(context, root, right);
	xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 120, 30 });
	xLayoutNodeGetResult(context, left, &result);
	expect_near(result.rect.width, 40.0f, "fixed track width");
	xLayoutNodeGetResult(context, right, &result);
	expect_near(result.rect.x, 45.0f, "fraction track offset");
	expect_near(result.rect.width, 75.0f, "fraction track fills remainder");
	expect_near(result.rect.height, 30.0f, "row track fills height");
	xLayoutContextDestroy(context);
}

static void test_track_max_redistribution(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t data = { 1.0f, 1.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t left = make_leaf(context, &data);
	xlayout_node_t right = make_leaf(context, &data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t item_style = xLayoutStyleDefault();
	xlayout_track_t columns[2] = {
		xLayoutTrackMinMax(0.0f, 30.0f, 1.0f),
		xLayoutTrackFraction(1.0f)
	};
	xlayout_track_t rows[1] = { xLayoutTrackFraction(1.0f) };
	xlayout_result_t result;
	root_style.container.format = XLAYOUT_FORMAT_TRACK;
	root_style.container.align_items = XLAYOUT_ALIGN_STRETCH;
	root_style.container.justify_items = XLAYOUT_ALIGN_STRETCH;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetColumns(context, root, columns, 2u);
	xLayoutNodeSetRows(context, root, rows, 1u);
	xLayoutNodeSetStyle(context, left, &item_style);
	item_style.item.column = 1u;
	xLayoutNodeSetStyle(context, right, &item_style);
	xLayoutNodeAppend(context, root, left);
	xLayoutNodeAppend(context, root, right);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 20 }), "track redistribution arrange succeeds");
	xLayoutNodeGetResult(context, left, &result);
	expect_near(result.rect.width, 30.0f, "track respects maximum");
	xLayoutNodeGetResult(context, right, &result);
	expect_near(result.rect.x, 30.0f, "track redistribution offset");
	expect_near(result.rect.width, 70.0f, "track redistributes capped remainder");
	xLayoutContextDestroy(context);
}

static void test_percent_track(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t data = { 1.0f, 1.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t first = make_leaf(context, &data);
	xlayout_node_t divider = make_leaf(context, &data);
	xlayout_node_t second = make_leaf(context, &data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t item_style = xLayoutStyleDefault();
	xlayout_track_t columns[3] = {
		xLayoutTrackPercent(0.3f),
		xLayoutTrackFixed(10.0f),
		xLayoutTrackFraction(1.0f)
	};
	xlayout_track_t rows[1] = { xLayoutTrackFraction(1.0f) };
	xlayout_result_t result;
	root_style.container.format = XLAYOUT_FORMAT_TRACK;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetColumns(context, root, columns, 3u);
	xLayoutNodeSetRows(context, root, rows, 1u);
	xLayoutNodeSetStyle(context, first, &item_style);
	item_style.item.column = 1u;
	xLayoutNodeSetStyle(context, divider, &item_style);
	item_style.item.column = 2u;
	xLayoutNodeSetStyle(context, second, &item_style);
	xLayoutNodeAppend(context, root, first);
	xLayoutNodeAppend(context, root, divider);
	xLayoutNodeAppend(context, root, second);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 20 }), "percent track arrange succeeds");
	xLayoutNodeGetResult(context, first, &result);
	expect_near(result.rect.width, 27.0f, "percent track excludes fixed tracks from basis");
	xLayoutNodeGetResult(context, divider, &result);
	expect_near(result.rect.x, 27.0f, "percent track divider offset");
	expect_near(result.rect.width, 10.0f, "percent track fixed divider");
	xLayoutNodeGetResult(context, second, &result);
	expect_near(result.rect.x, 37.0f, "percent track remainder offset");
	expect_near(result.rect.width, 63.0f, "percent track leaves remainder to fraction");
	xLayoutContextDestroy(context);
}

static void test_cross_track(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t data = { 1.0f, 1.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t content = make_leaf(context, &data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_track_t columns[2] = {
		xLayoutTrackFraction(1.0f),
		xLayoutTrackCross(1.0f, 24.0f, 36.0f)
	};
	xlayout_track_t rows[1] = { xLayoutTrackFraction(1.0f) };
	xlayout_result_t result;
	columns[1].shrink = 1.0f;
	root_style.container.format = XLAYOUT_FORMAT_TRACK;
	root_style.container.align_items = XLAYOUT_ALIGN_STRETCH;
	root_style.container.justify_items = XLAYOUT_ALIGN_STRETCH;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetColumns(context, root, columns, 2u);
	xLayoutNodeSetRows(context, root, rows, 1u);
	xLayoutNodeSetStyle(context, content, &root_style);
	xLayoutNodeAppend(context, root, content);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 30 }), "cross track arrange succeeds");
	xLayoutNodeGetResult(context, content, &result);
	expect_near(result.rect.width, 70.0f, "cross column follows height");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 10 }), "cross track minimum arrange succeeds");
	xLayoutNodeGetResult(context, content, &result);
	expect_near(result.rect.width, 76.0f, "cross column respects minimum");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 50 }), "cross track maximum arrange succeeds");
	xLayoutNodeGetResult(context, content, &result);
	expect_near(result.rect.width, 64.0f, "cross column respects maximum");
	xLayoutContextDestroy(context);
}

static void test_track_shrink(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t data = { 1.0f, 1.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t left = make_leaf(context, &data);
	xlayout_node_t right = make_leaf(context, &data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t item_style = xLayoutStyleDefault();
	xlayout_track_t columns[2] = { xLayoutTrackFixed(60.0f), xLayoutTrackFixed(60.0f) };
	xlayout_track_t rows[1] = { xLayoutTrackFraction(1.0f) };
	xlayout_result_t result;
	columns[0].min_size = columns[1].min_size = 20.0f;
	columns[0].shrink = columns[1].shrink = 1.0f;
	root_style.container.format = XLAYOUT_FORMAT_TRACK;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetColumns(context, root, columns, 2u);
	xLayoutNodeSetRows(context, root, rows, 1u);
	xLayoutNodeSetStyle(context, left, &item_style);
	item_style.item.column = 1u;
	xLayoutNodeSetStyle(context, right, &item_style);
	xLayoutNodeAppend(context, root, left);
	xLayoutNodeAppend(context, root, right);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 20 }), "track shrink arrange succeeds");
	xLayoutNodeGetResult(context, left, &result);
	expect_near(result.rect.width, 50.0f, "track shrink first width");
	xLayoutNodeGetResult(context, right, &result);
	expect_near(result.rect.x, 50.0f, "track shrink second offset");
	expect_near(result.rect.width, 50.0f, "track shrink second width");
	xLayoutContextDestroy(context);
}

static void test_track_dimension_overflow_rejected(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t data = { 1.0f, 1.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t child = make_leaf(context, &data);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t child_style = xLayoutStyleDefault();

	root_style.container.format = XLAYOUT_FORMAT_TRACK;
	child_style.item.column = UINT32_MAX - 1u;
	child_style.item.column_span = 1u;
	expect_true(xLayoutNodeSetStyle(context, root, &root_style), "overflow track root style set");
	expect_true(xLayoutNodeSetStyle(context, child, &child_style), "overflow track child style set");
	expect_true(xLayoutNodeAppend(context, root, child), "overflow track child appended");
	expect_true(!xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 20 }),
		"overflow track dimensions are rejected before allocation");
	xLayoutContextDestroy(context);
}

static void test_flow_fragments(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_fragments_t data = { { 30.0f, 25.0f, 40.0f, 10.0f }, 12.0f };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t source = xLayoutNodeCreate(context, XLAYOUT_ROLE_LEAF);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_fragment_t fragment;
	root_style.container.format = XLAYOUT_FORMAT_FLOW;
	root_style.container.axis = XLAYOUT_HORIZONTAL;
	root_style.container.column_gap = 5.0f;
	root_style.container.row_gap = 3.0f;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetUserData(context, source, &data);
	xLayoutNodeSetFragmentSource(context, source, fragment_count, measure_fragment);
	xLayoutNodeAppend(context, root, source);
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 70, 100 }), "flow arrange succeeds");
	expect_true(xLayoutNodeFragmentCount(context, source) == 4u, "all source fragments retained");
	xLayoutNodeGetFragment(context, source, 0u, &fragment);
	expect_near(fragment.rect.x, 0.0f, "flow first fragment x");
	xLayoutNodeGetFragment(context, source, 1u, &fragment);
	expect_near(fragment.rect.x, 35.0f, "flow second fragment x");
	xLayoutNodeGetFragment(context, source, 2u, &fragment);
	expect_near(fragment.rect.x, 0.0f, "flow wraps third fragment");
	expect_near(fragment.rect.y, 15.0f, "flow line gap applied");
	expect_true(fragment.tag == 102u, "flow fragment tag retained");
	xLayoutContextDestroy(context);
}

static void test_flow_fragment_margins(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_fragments_t data = { { 30.0f, 25.0f, 40.0f, 10.0f }, 12.0f };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t source = xLayoutNodeCreate(context, XLAYOUT_ROLE_LEAF);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t source_style = xLayoutStyleDefault();
	xlayout_constraints_t constraints = xLayoutConstraints(70.0f, 100.0f);
	xlayout_measure_t measured;
	xlayout_fragment_t fragment;
	root_style.container.format = XLAYOUT_FORMAT_FLOW;
	root_style.container.axis = XLAYOUT_HORIZONTAL;
	root_style.container.column_gap = 5.0f;
	root_style.container.row_gap = 3.0f;
	source_style.item.margin = (xlayout_edges_t){ 4.0f, 2.0f, 6.0f, 3.0f };
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetStyle(context, source, &source_style);
	xLayoutNodeSetUserData(context, source, &data);
	xLayoutNodeSetFragmentSource(context, source, fragment_count, measure_fragment);
	xLayoutNodeAppend(context, root, source);
	expect_true(xLayoutMeasure(context, root, &constraints, &measured), "flow margin measure succeeds");
	expect_near(measured.width, 64.0f, "flow margin measured width");
	expect_near(measured.height, 37.0f, "flow margin measured height");
	expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 70, 100 }), "flow margin arrange succeeds");
	xLayoutNodeGetFragment(context, source, 0u, &fragment);
	expect_near(fragment.rect.x, 4.0f, "flow leading margin applies to first fragment");
	expect_near(fragment.rect.y, 2.0f, "flow cross margin applies to first line");
	xLayoutNodeGetFragment(context, source, 1u, &fragment);
	expect_near(fragment.rect.x, 39.0f, "flow leading margin is not repeated");
	xLayoutNodeGetFragment(context, source, 2u, &fragment);
	expect_near(fragment.rect.x, 0.0f, "flow wrapped fragment starts at line origin");
	expect_near(fragment.rect.y, 22.0f, "flow margins contribute to line height");
	xLayoutNodeGetFragment(context, source, 3u, &fragment);
	expect_near(fragment.rect.x, 45.0f, "flow trailing fragment follows gap");
	expect_near(data.available[0], 66.0f, "flow callback excludes leading margin");
	expect_near(data.available[2], 70.0f, "flow callback is remeasured after wrap");
	expect_near(data.available[3], 19.0f, "flow callback excludes trailing margin");
	xLayoutContextDestroy(context);
}

static void test_absolute_and_baseline(void)
{
	xlayout_context_t* context = xLayoutContextCreate(NULL);
	test_leaf_t short_leaf = { 20.0f, 10.0f, 0u };
	test_leaf_t tall_leaf = { 20.0f, 20.0f, 0u };
	test_leaf_t absolute_leaf = { 30.0f, 14.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t short_node = make_leaf(context, &short_leaf);
	xlayout_node_t tall_node = make_leaf(context, &tall_leaf);
	xlayout_node_t absolute_node = make_leaf(context, &absolute_leaf);
	xlayout_style_t root_style = xLayoutStyleDefault();
	xlayout_style_t child_style = xLayoutStyleDefault();
	xlayout_result_t short_result;
	xlayout_result_t tall_result;
	xlayout_result_t absolute_result;
	root_style.container.axis = XLAYOUT_HORIZONTAL;
	root_style.container.align_items = XLAYOUT_ALIGN_BASELINE;
	xLayoutNodeSetStyle(context, root, &root_style);
	xLayoutNodeSetStyle(context, short_node, &child_style);
	xLayoutNodeSetStyle(context, tall_node, &child_style);
	child_style.item.position = XLAYOUT_POSITION_ABSOLUTE;
	child_style.item.x = 50.0f;
	child_style.item.y = 7.0f;
	xLayoutNodeSetStyle(context, absolute_node, &child_style);
	xLayoutNodeAppend(context, root, short_node);
	xLayoutNodeAppend(context, root, tall_node);
	xLayoutNodeAppend(context, root, absolute_node);
	xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 100, 40 });
	xLayoutNodeGetResult(context, short_node, &short_result);
	xLayoutNodeGetResult(context, tall_node, &tall_result);
	expect_near(short_result.rect.y + short_leaf.height * 0.75f,
		tall_result.rect.y + tall_leaf.height * 0.75f, "stack baselines match");
	xLayoutNodeGetResult(context, absolute_node, &absolute_result);
	expect_near(absolute_result.rect.x, 50.0f, "absolute child x");
	expect_near(absolute_result.rect.y, 7.0f, "absolute child y");
	expect_near(absolute_result.rect.width, 30.0f, "absolute child measured before arrange");
	xLayoutContextDestroy(context);
}

static uint32_t next_random(uint32_t* state)
{
	*state = *state * 1664525u + 1013904223u;
	return *state;
}

static void test_large_tree(void)
{
	enum { NODE_COUNT = 4096 };
	xlayout_context_config_t config = { NODE_COUNT + 1u, NODE_COUNT + 1u };
	xlayout_context_t* context = xLayoutContextCreate(&config);
	test_leaf_t leaf_data = { 8.0f, 6.0f, 0u };
	xlayout_node_t root = xLayoutNodeCreate(context, XLAYOUT_ROLE_CONTAINER);
	xlayout_node_t nodes[NODE_COUNT];
	xlayout_style_t root_style = xLayoutStyleDefault();
	uint32_t random_state = 1u;
	uint32_t index;
	xlayout_stats_t stats;
	root_style.container.format = XLAYOUT_FORMAT_FLOW;
	root_style.container.column_gap = 1.0f;
	root_style.container.row_gap = 1.0f;
	xLayoutNodeSetStyle(context, root, &root_style);
	for ( index = 0; index < NODE_COUNT; ++index ) {
		xlayout_style_t style = xLayoutStyleDefault();
		nodes[index] = make_leaf(context, &leaf_data);
		style.item.margin.left = (float)(next_random(&random_state) & 1u);
		style.item.margin.top = (float)(next_random(&random_state) & 1u);
		xLayoutNodeSetStyle(context, nodes[index], &style);
		expect_true(xLayoutNodeAppend(context, root, nodes[index]), "large tree append");
	}
	for ( index = 0; index < 20u; ++index ) {
		xlayout_result_t result;
		uint32_t sample = next_random(&random_state) % NODE_COUNT;
		expect_true(xLayoutArrange(context, root, (xlayout_rect_t){ 0, 0, 640.0f + (float)(index & 1u), 480.0f }), "large tree arrange");
		xLayoutNodeGetResult(context, nodes[sample], &result);
		expect_true(result.rect.x == result.rect.x && result.rect.y == result.rect.y
			&& result.rect.width >= 0.0f && result.rect.height >= 0.0f, "large tree geometry valid");
	}
	xLayoutContextGetStats(context, &stats);
	expect_true(stats.live_nodes == NODE_COUNT + 1u, "pool tracks all live nodes");
	xLayoutNodeDestroy(context, root);
	xLayoutContextGetStats(context, &stats);
	expect_true(stats.live_nodes == 0u, "subtree destruction returns every pool node");
	xLayoutContextDestroy(context);
}

int main(void)
{
	test_handles_and_cache();
	test_stack();
	test_stack_column_shrink_respects_minimum();
	test_stack_minimum_overflow();
	test_stack_row_shrink_respects_minimum();
	test_stack_shrink_still_distributes();
	test_stack_shrink_redistributes_after_minimum();
	test_hidden_child_becomes_visible();
	test_layer();
	test_dock();
	test_track();
	test_track_max_redistribution();
	test_percent_track();
	test_cross_track();
	test_track_shrink();
	test_track_dimension_overflow_rejected();
	test_flow_fragments();
	test_flow_fragment_margins();
	test_absolute_and_baseline();
	test_custom_arrange_children();
	test_large_tree();
	if ( failures != 0 ) {
		printf("xLayout: %d failure(s)\n", failures);
		return 1;
	}
	printf("xLayout: all tests passed\n");
	return 0;
}
