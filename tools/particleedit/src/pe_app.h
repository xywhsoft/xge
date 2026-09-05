#ifndef PE_APP_H
#define PE_APP_H
#include "pe_fields.h"
#include "pe_preview.h"
#include "xui.h"

#define PE_BG XUI_COLOR_RGBA(23, 28, 38, 255)
#define PE_PANEL XUI_COLOR_RGBA(29, 36, 47, 255)
#define PE_LINE XUI_COLOR_RGBA(51, 63, 78, 255)
#define PE_TEXT XUI_COLOR_RGBA(221, 230, 240, 255)
#define PE_MUTED XUI_COLOR_RGBA(150, 167, 186, 255)
#define PE_ACCENT XUI_COLOR_RGBA(73, 201, 189, 255)
#define PE_SELECTED XUI_COLOR_RGBA(41, 92, 106, 255)
enum
{
	PE_WIN_PREVIEW,
	PE_WIN_EMITTERS,
	PE_WIN_PROPERTIES,
	PE_WIN_TIMELINE,
	PE_WIN_CURVES,
	PE_WIN_BURSTS,
	PE_WIN_SETTINGS,
	PE_WIN_COUNT
};
enum
{
	PE_NEW,
	PE_OPEN,
	PE_SAVE,
	PE_SAVE_AS,
	PE_UNDO,
	PE_REDO,
	PE_RESET_LAYOUT,
	PE_ADD,
	PE_DUPLICATE,
	PE_REMOVE,
	PE_UP,
	PE_DOWN,
	PE_MUTE,
	PE_SOLO,
	PE_PLAY,
	PE_RESTART,
	PE_STEP,
	PE_EMIT,
	PE_FIT,
	PE_BOUNDS,
	PE_GROUND,
	PE_MOVE,
	PE_CURVE_ADD,
	PE_CURVE_REMOVE,
	PE_CURVE_RESET,
	PE_CURVE_FADE,
	PE_BURST_ADD,
	PE_BURST_REMOVE,
	PE_TEXTURE,
	PE_RELOAD,
	PE_HELP,
	PE_TEMPLATE,
	PE_SHOW_CURVES,
	PE_SHOW_BURSTS,
	PE_COMMAND_COUNT
};
typedef struct pe_app pe_app;
typedef struct pe_action
{
	pe_app *app;
	int command;
} pe_action;
struct pe_app
{
	pe_document *doc;
	pe_preview preview;
	xui_context ui;
	xui_proxy_t proxy;
	xui_surface target;
	xui_font font;
	xui_widget root, header, transport, status, dock;
	xui_widget panels[PE_WIN_COUNT], list, properties, timeline, curve, key_grid, bursts, settings;
	xui_widget channel_combo, interpolation_combo, preset_combo;
	xui_widget buttons[PE_COMMAND_COUNT];
	pe_action actions[PE_COMMAND_COUNT];
	int windows[PE_WIN_COUNT], bottom_pane, property_pane;
	xvalue *default_layout;
	int width, height, frame, frames, exercise, tested, error, syncing, refresh, rebuild, selection_changed;
	int channel, key, burst, preset;
	int curve_drag, timeline_drag, drag_span, drag_layer, drag_burst, drag_mode, drag_start_frame;
	pe_data drag_data;
	float curve_min, curve_max, drag_min, drag_max;
	int preview_drag;
	float zoom, pan_x, pan_y, mouse_x, mouse_y, initial_pan_x, initial_pan_y;
	int span_ids[XGE_PARTICLE_MAX_EMITTERS];
	char list_text[XGE_PARTICLE_MAX_EMITTERS][100];
	const char *list_items[XGE_PARTICLE_MAX_EMITTERS];
	char status_text[512], layout_path[PE_PATH], preset_root[PE_PATH], capture[PE_PATH], input[PE_PATH];
	int layout_changes, layout_failed;
	int startup_preset, startup_curve;
	double layout_save_at;
};
void pe_status(pe_app *a, const char *text);
void pe_changed(pe_app *a);
void pe_select(pe_app *a, int selected);
int pe_finish_edit(pe_app *a);
int pe_command(pe_app *a, int command);
int pe_ui_create(pe_app *a);
void pe_ui_refresh(pe_app *a);
int pe_ui_resize(pe_app *a);
int pe_curve_paint(xui_widget widget, xui_draw_context draw, uint32_t state, void *user);
int pe_curve_event(xui_widget widget, const xui_event_t *event, void *user);
int pe_timeline_event(xui_widget widget, const xui_event_t *event, void *user);
void pe_timeline_sync(pe_app *a);
void pe_curve_range(pe_app *a);
void pe_key_grid_sync(pe_app *a);
int pe_exercise(pe_app *a);
void pe_layout_save(pe_app *a);
int pe_set_property(pe_app *a, const char *id, const char *value);
#endif
