#ifndef XUI_TABLE_VIEW_PAINT_H
#define XUI_TABLE_VIEW_PAINT_H

#include "../xui.h"

/* Private, paint-only composition. Nonzero callback results replace only the
 * corresponding chrome pass; ordinary cell content/renderers keep their order. */
typedef struct xui_table_view_paint_adapter_t {
	int bHasBackground;
	uint32_t iBackgroundColor;
	int bHasGrid;
	uint32_t iGridColor;
	/* Opt in only when bHasStyle supplies all three cell colors, including zero. */
	int bCellColorsPresent;
	xui_table_view_cell_renderer_proc onCellBackground;
	xui_table_view_cell_renderer_proc onCellDecorations;
	void* pUser;
} xui_table_view_paint_adapter_t;

int xuiInternalTableViewSetPaintAdapter(xui_widget pWidget, const xui_table_view_paint_adapter_t* pAdapter);

#endif
