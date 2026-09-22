#include "pe_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* One semantic palette, shared by XUI tokens and our custom canvas painters.
 * Particle textures/materials and the simulation stage are deliberately not theme colors. */
static const uint32_t palettes[PE_THEME_COUNT][PE_COLOR_COUNT] = {
    [PE_THEME_DARK] =
        {[PE_BG] = 0x1e1e1eff,        [PE_PANEL] = 0x252526ff,      [PE_FIELD] = 0x333337ff,
         [PE_LINE] = 0x3f3f46ff,      [PE_TEXT] = 0xdcdcdcff,       [PE_MUTED] = 0xa8a8a8ff,
         [PE_DISABLED] = 0x777777ff,  [PE_ACCENT] = 0x4daafcff,     [PE_SELECTED] = 0x264f78ff,
         [PE_HOVER] = 0x3e3e42ff,     [PE_ACTIVE] = 0x007accff,     [PE_ON_ACCENT] = 0xffffffff,
         [PE_BUTTON] = 0x333337ff,    [PE_SCROLL] = 0x686868ff,     [PE_SCROLL_HOVER] = 0x909090ff,
         [PE_ERROR] = 0xf48771ff,     [PE_WARNING_BG] = 0x46371dff, [PE_WARNING_TEXT] = 0xffdc83ff,
         [PE_CANVAS] = 0x1e1e1eff,    [PE_GRID] = 0x3a3a40ff,       [PE_CURVE] = 0x4fc1ffff,
         [PE_BURST] = 0xffd36eff,     [PE_SPAN] = 0x3f454fff,       [PE_CHECKER_A] = 0x4a4a4aff,
         [PE_CHECKER_B] = 0x333333ff, [PE_SHADOW] = 0x00000088,     [PE_TRANSPARENT] = 0},
    [PE_THEME_LIGHT] = {
        [PE_BG] = 0xf3f3f3ff,        [PE_PANEL] = 0xffffffff,      [PE_FIELD] = 0xffffffff,
        [PE_LINE] = 0xd2d2d2ff,      [PE_TEXT] = 0x1f1f1fff,       [PE_MUTED] = 0x616161ff,
        [PE_DISABLED] = 0x8a8a8aff,  [PE_ACCENT] = 0x006cbeff,     [PE_SELECTED] = 0xcce8ffff,
        [PE_HOVER] = 0xe5f1fbff,     [PE_ACTIVE] = 0x007accff,     [PE_ON_ACCENT] = 0xffffffff,
        [PE_BUTTON] = 0xe5e5e5ff,    [PE_SCROLL] = 0xa6a6a6ff,     [PE_SCROLL_HOVER] = 0x7c7c7cff,
        [PE_ERROR] = 0xc42b1cff,     [PE_WARNING_BG] = 0xfff4ceff, [PE_WARNING_TEXT] = 0x795e00ff,
        [PE_CANVAS] = 0xffffffff,    [PE_GRID] = 0xdedee2ff,       [PE_CURVE] = 0x0078d4ff,
        [PE_BURST] = 0x976a00ff,     [PE_SPAN] = 0xe4e7ebff,       [PE_CHECKER_A] = 0xffffffff,
        [PE_CHECKER_B] = 0xd8d8d8ff, [PE_SHADOW] = 0x00000030,     [PE_TRANSPARENT] = 0}};
static const char *tokens[PE_COLOR_COUNT] = {
    "pe.background", "pe.panel",  "pe.field",        "pe.border", "pe.text",       "pe.muted",
    "pe.disabled",   "pe.accent", "pe.selection",    "pe.hover",  "pe.active",     "pe.on_accent",
    "pe.button",     "pe.scroll", "pe.scroll_hover", "pe.error",  "pe.warning_bg", "pe.warning_text",
    "pe.canvas",     "pe.grid",   "pe.curve",        "pe.burst",  "pe.span",       "pe.checker_a",
    "pe.checker_b",  "pe.shadow", "pe.transparent"};
typedef struct
{
	const char *key;
	int role;
} pe_color_rule;
/* The complete default sheet is installed once. Runtime theme changes only update tokens.
 * No recursive SetColors calls, private widget access, or per-editor color overrides. */
static const pe_color_rule rules[] = {{"text.color", PE_TEXT},
                                      {"text.disabled_color", PE_DISABLED},
                                      {"button.normal_color", PE_BUTTON},
                                      {"button.hover_color", PE_HOVER},
                                      {"button.active_color", PE_SELECTED},
                                      {"button.disabled_color", PE_BG},
                                      {"button.checked_color", PE_SELECTED},
                                      {"button.border_color", PE_LINE},
                                      {"button.hover_border_color", PE_ACCENT},
                                      {"button.active_border_color", PE_ACCENT},
                                      {"button.disabled_border_color", PE_LINE},
                                      {"button.checked_border_color", PE_ACCENT},
                                      {"button.focus_color", PE_ACCENT},
                                      {"button.icon_color", PE_TEXT},
                                      {"input.text.color", PE_TEXT},
                                      {"input.text.disabled_color", PE_DISABLED},
                                      {"input.placeholder.color", PE_MUTED},
                                      {"input.background.color", PE_FIELD},
                                      {"input.background.hover_color", PE_FIELD},
                                      {"input.background.disabled_color", PE_BG},
                                      {"input.border.color", PE_LINE},
                                      {"input.border.hover_color", PE_ACCENT},
                                      {"input.border.focus_color", PE_ACCENT},
                                      {"input.border.disabled_color", PE_LINE},
                                      {"input.error.background_color", PE_WARNING_BG},
                                      {"input.error.border_color", PE_ERROR},
                                      {"input.selection.color", PE_SELECTED},
                                      {"input.cursor.color", PE_TEXT},
                                      {"input.decoration.color", PE_MUTED},
                                      {"input.decoration.hover_color", PE_TEXT},
                                      {"input.decoration.active_color", PE_ACCENT},
                                      {"input.decoration.disabled_color", PE_DISABLED},
                                      {"numeric_input.text.color", PE_TEXT},
                                      {"numeric_input.text.disabled_color", PE_DISABLED},
                                      {"numeric_input.placeholder.color", PE_MUTED},
                                      {"numeric_input.selection.color", PE_SELECTED},
                                      {"numeric_input.cursor.color", PE_TEXT},
                                      {"numeric_input.background.color", PE_FIELD},
                                      {"numeric_input.background.hover_color", PE_FIELD},
                                      {"numeric_input.background.disabled_color", PE_BG},
                                      {"numeric_input.border.color", PE_LINE},
                                      {"numeric_input.border.hover_color", PE_ACCENT},
                                      {"numeric_input.border.focus_color", PE_ACCENT},
                                      {"numeric_input.border.disabled_color", PE_LINE},
                                      {"numeric_input.error.background_color", PE_WARNING_BG},
                                      {"numeric_input.error.border_color", PE_ERROR},
                                      {"numeric_input.spinner.color", PE_TRANSPARENT},
                                      {"numeric_input.spinner.hover_color", PE_HOVER},
                                      {"numeric_input.spinner.active_color", PE_SELECTED},
                                      {"numeric_input.spinner.border_color", PE_LINE},
                                      {"numeric_input.spinner.icon_color", PE_TEXT},
                                      {"numeric_input.spinner.icon_disabled_color", PE_DISABLED},
                                      {"combobox.popup.panel_color", PE_PANEL},
                                      {"combobox.popup.border_color", PE_LINE},
                                      {"combobox.popup.shadow_color", PE_SHADOW},
                                      {"combobox.popup.hover_color", PE_SELECTED},
                                      {"combobox.popup.text_color", PE_TEXT},
                                      {"combobox.popup.hover_text_color", PE_TEXT},
                                      {"combobox.popup.disabled_text_color", PE_DISABLED},
                                      {"combobox.popup.separator_color", PE_LINE},
                                      {"colorpicker.popup.panel_color", PE_PANEL},
                                      {"colorpicker.popup.border_color", PE_LINE},
                                      {"colorpicker.popup.shadow_color", PE_SHADOW},
                                      {"colorpicker.popup.text_color", PE_TEXT},
                                      {"colorpicker.popup.muted_text_color", PE_MUTED},
                                      {"colorpicker.accent.color", PE_ACCENT},
                                      {"colorpicker.field.color", PE_FIELD},
                                      {"colorpicker.field.border_color", PE_LINE},
                                      {"colorpicker.field.error_border_color", PE_ERROR},
                                      {"colorpicker.separator.color", PE_LINE},
                                      {"colorpicker.selection.color", PE_SELECTED},
                                      {"colorpicker.selection.text_color", PE_TEXT},
                                      {"colorpicker.track.color", PE_LINE},
                                      {"colorpicker.knob.color", PE_TEXT},
                                      {"colorpicker.marker.color", PE_ON_ACCENT},
                                      {"colorpicker.marker.border_color", PE_BG},
                                      {"colorpicker.checker.light_color", PE_CHECKER_A},
                                      {"colorpicker.checker.dark_color", PE_CHECKER_B},
                                      {"menu.panel.color", PE_PANEL},
                                      {"menu.border.color", PE_LINE},
                                      {"menu.shadow.color", PE_SHADOW},
                                      {"menu.item.hover_color", PE_SELECTED},
                                      {"menu.text.color", PE_TEXT},
                                      {"menu.text.hover_color", PE_TEXT},
                                      {"menu.text.disabled_color", PE_DISABLED},
                                      {"menu.text.danger_color", PE_ERROR},
                                      {"menu.shortcut.color", PE_MUTED},
                                      {"menu.mark.color", PE_ACCENT},
                                      {"menu.separator.color", PE_LINE},
                                      {"menu.focus.color", PE_ACCENT},
                                      {"popup.panel.color", PE_PANEL},
                                      {"popup.border.color", PE_LINE},
                                      {"popup.shadow.color", PE_SHADOW},
                                      {"tooltip.background.color", PE_PANEL},
                                      {"tooltip.border.color", PE_LINE},
                                      {"tooltip.text.color", PE_TEXT},
                                      {"scrollframe.background.color", PE_PANEL},
                                      {"scrollframe.corner.color", PE_BG},
                                      {"scrollframe.grip.color", PE_MUTED},
                                      {"scrollview.background.color", PE_PANEL},
                                      {"scrollview.corner.color", PE_BG},
                                      {"scrollview.grip.color", PE_MUTED},
                                      {"scrollbar.track.color", PE_BG},
                                      {"scrollbar.thumb.color", PE_SCROLL},
                                      {"scrollbar.thumb.hover_color", PE_SCROLL_HOVER},
                                      {"scrollbar.thumb.active_color", PE_ACCENT},
                                      {"scrollbar.focus.color", PE_ACCENT},
                                      {"scrollbar.disabled.color", PE_LINE},
                                      {"scrollbar.button.color", PE_BG},
                                      {"scrollbar.button.icon_color", PE_MUTED},
                                      {"listview.background.color", PE_PANEL},
                                      {"listview.border.color", PE_LINE},
                                      {"listview.focus.color", PE_ACCENT},
                                      {"listview.row.color", PE_PANEL},
                                      {"listview.row.hover_color", PE_HOVER},
                                      {"listview.row.selected_color", PE_SELECTED},
                                      {"listview.text.color", PE_TEXT},
                                      {"listview.text.disabled_color", PE_DISABLED},
                                      {"listview.text.selected_color", PE_TEXT},
                                      {"tableview.background.color", PE_PANEL},
                                      {"tableview.header.color", PE_BG},
                                      {"tableview.header.text_color", PE_TEXT},
                                      {"tableview.row.color", PE_PANEL},
                                      {"tableview.row.alt_color", PE_PANEL},
                                      {"tableview.row.hover_color", PE_HOVER},
                                      {"tableview.row.selected_color", PE_SELECTED},
                                      {"tableview.row.disabled_color", PE_BG},
                                      {"tableview.grid.color", PE_LINE},
                                      {"tableview.text.color", PE_TEXT},
                                      {"tableview.text.disabled_color", PE_DISABLED},
                                      {"tableview.text.selected_color", PE_TEXT},
                                      {"tableview.focus.color", PE_ACCENT},
                                      {"tableview.check.background_color", PE_FIELD},
                                      {"tableview.check.mark_color", PE_TEXT},
                                      {"tableview.checker.primary_color", PE_CHECKER_A},
                                      {"tableview.checker.secondary_color", PE_CHECKER_B},
                                      {"tableview.color.fallback_color", PE_MUTED},
                                      {"tableview.cell.invalid_color", PE_ERROR},
                                      {"tableview.cell.dirty_color", PE_ACCENT},
                                      {"tableview.cell.editing_color", PE_ACCENT},
                                      {"tableview.picker.background_color", PE_BUTTON},
                                      {"tableview.picker.border_color", PE_LINE},
                                      {"propertygrid.background.color", PE_PANEL},
                                      {"propertygrid.grid.color", PE_LINE},
                                      {"propertygrid.category.background_color", PE_BG},
                                      {"propertygrid.category.hover_color", PE_HOVER},
                                      {"propertygrid.category.text_color", PE_TEXT},
                                      {"propertygrid.category.icon_color", PE_MUTED},
                                      {"propertygrid.name.background_color", PE_PANEL},
                                      {"propertygrid.name.text_color", PE_TEXT},
                                      {"propertygrid.name.hover_color", PE_HOVER},
                                      {"propertygrid.value.background_color", PE_PANEL},
                                      {"propertygrid.value.text_color", PE_TEXT},
                                      {"propertygrid.selected.color", PE_SELECTED},
                                      {"propertygrid.readonly.text_color", PE_MUTED},
                                      {"propertygrid.invalid.color", PE_ERROR},
                                      {"propertygrid.dirty.color", PE_ACCENT},
                                      {"dockpanel.background.color", PE_BG},
                                      {"dockpanel.pane.color", PE_PANEL},
                                      {"dockpanel.client.color", PE_PANEL},
                                      {"dockpanel.caption.color", PE_BG},
                                      {"dockpanel.caption.active_color", PE_ACTIVE},
                                      {"dockpanel.caption.text_color", PE_MUTED},
                                      {"dockpanel.caption.active_text_color", PE_ON_ACCENT},
                                      {"dockpanel.tab.color", PE_BG},
                                      {"dockpanel.tab.hover_color", PE_HOVER},
                                      {"dockpanel.tab.active_color", PE_ACTIVE},
                                      {"dockpanel.tab.text_color", PE_MUTED},
                                      {"dockpanel.tab.active_text_color", PE_ON_ACCENT},
                                      {"dockpanel.border.color", PE_LINE},
                                      {"dockpanel.focus.color", PE_ACCENT},
                                      {"dockpanel.splitter.color", PE_BG},
                                      {"dockpanel.splitter.hover_color", PE_LINE},
                                      {"dockpanel.splitter.active_color", PE_ACCENT},
                                      {"dockpanel.button.color", PE_TRANSPARENT},
                                      {"dockpanel.button.hover_color", PE_HOVER},
                                      {"dockpanel.button.active_color", PE_SELECTED},
                                      {"dockpanel.auto_hide.color", PE_BG},
                                      {"dockpanel.auto_hide.hover_color", PE_HOVER},
                                      {"dockpanel.float.title_color", PE_SELECTED},
                                      {"dockpanel.float.border_color", PE_ACCENT},
                                      {"dockpanel.button.icon_color", PE_MUTED},
                                      {"dockpanel.button.disabled_color", PE_DISABLED},
                                      {"dockpanel.button.close_icon_color", PE_ERROR},
                                      {"dockpanel.auto_hide.border_color", PE_LINE},
                                      {"dockpanel.tab.disabled_border_color", PE_LINE},
                                      {"dockpanel.tab.disabled_text_color", PE_DISABLED},
                                      {"dockpanel.tab.indicator_color", PE_ACCENT},
                                      {"dockpanel.drag.indicator_color", PE_ACCENT},
                                      {"dockpanel.drag.insert_border_color", PE_ACCENT},
                                      {"dockpanel.drag.preview_color", PE_SELECTED},
                                      {"dockpanel.drag.preview_border_color", PE_ACCENT},
                                      {"dockpanel.drag.preview_inner_border_color", PE_PANEL},
                                      {"timelineview.background.color", PE_PANEL},
                                      {"timelineview.corner.color", PE_BG},
                                      {"timelineview.ruler.color", PE_BG},
                                      {"timelineview.layer.color", PE_PANEL},
                                      {"timelineview.layer.alt_color", PE_BG},
                                      {"timelineview.layer.accent_color", PE_ACCENT},
                                      {"timelineview.grid.color", PE_LINE},
                                      {"timelineview.grid.strong_color", PE_LINE},
                                      {"timelineview.text.color", PE_TEXT},
                                      {"timelineview.text.muted_color", PE_MUTED},
                                      {"timelineview.selection.color", PE_SELECTED},
                                      {"timelineview.hover.color", PE_HOVER},
                                      {"timelineview.current.color", PE_ACCENT},
                                      {"timelineview.border.color", PE_LINE},
                                      {"timelineview.focus.color", PE_ACCENT},
                                      {"timelineview.disabled.color", PE_DISABLED},
                                      {"timelineview.frame.color", PE_MUTED},
                                      {"timelineview.frame.key_color", PE_BURST},
                                      {"timelineview.frame.blank_key_color", PE_PANEL},
                                      {"timelineview.span.color", PE_SPAN},
                                      {"timelineview.span.text_color", PE_TEXT},
                                      {"timelineview.icon.color", PE_MUTED},
                                      {"timelineview.icon.hidden_color", PE_DISABLED},
                                      {"timelineview.icon.locked_color", PE_BURST}};
static const pe_color_rule picker_rules[] = {{"text.color", PE_TEXT},
                                             {"text.disabled_color", PE_DISABLED},
                                             {"background.color", PE_FIELD},
                                             {"background.hover_color", PE_FIELD},
                                             {"background.open_color", PE_FIELD},
                                             {"background.disabled_color", PE_BG},
                                             {"border.color", PE_LINE},
                                             {"border.hover_color", PE_ACCENT},
                                             {"border.focus_color", PE_ACCENT},
                                             {"arrow.color", PE_TEXT},
                                             {"arrow.disabled_color", PE_DISABLED},
                                             {"button.color", PE_TRANSPARENT},
                                             {"button.hover_color", PE_HOVER},
                                             {"button.open_color", PE_SELECTED}};
static const pe_color_rule scroll_rules[] = {{"track_color", PE_BG},           {"thumb_color", PE_SCROLL},
                                             {"hover_color", PE_SCROLL_HOVER}, {"active_color", PE_ACCENT},
                                             {"focus_color", PE_ACCENT},       {"disabled_color", PE_LINE}};
const uint32_t *pe_theme_colors(int theme)
{
	return palettes[theme == PE_THEME_LIGHT ? PE_THEME_LIGHT : PE_THEME_DARK];
}
static xui_style_property_t property(const char *key, int role)
{
	xui_style_property_t p = {0};
	p.iSize = sizeof(p);
	p.sName = key;
	p.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	p.tValue.iSize = sizeof(p.tValue);
	p.tValue.iType = XUI_STYLE_VALUE_TOKEN;
	p.tValue.sText = tokens[role];
	return p;
}
int pe_theme_init(pe_app *a)
{
	/* Register lazy editor/popup types before resolving the complete global sheet. */
	if (!xuiInputGetType(a->ui) || !xuiNumericInputGetType(a->ui) || !xuiColorPickerGetType(a->ui) ||
	    !xuiMenuGetType(a->ui) || !xuiPopupGetType(a->ui) || !xuiScrollViewGetType(a->ui))
		return XUI_ERROR_OUT_OF_MEMORY;
	enum
	{
		generated_count = 2 * (sizeof(picker_rules) / sizeof(picker_rules[0])) +
		                  3 * (sizeof(scroll_rules) / sizeof(scroll_rules[0])),
		property_count = sizeof(rules) / sizeof(rules[0]) + generated_count
	};
	xui_style_property_t props[property_count];
	char keys[generated_count][96];
	int count = 0, generated = 0;
	for (size_t i = 0; i < sizeof(rules) / sizeof(rules[0]); ++i)
		props[count++] = property(rules[i].key, rules[i].role);
	const char *pickers[] = {"combobox", "colorpicker"};
	for (int p = 0; p < 2; ++p)
		for (size_t i = 0; i < sizeof(picker_rules) / sizeof(picker_rules[0]); ++i)
		{
			snprintf(keys[generated], sizeof(keys[0]), "%s.%s", pickers[p], picker_rules[i].key);
			props[count++] = property(keys[generated++], picker_rules[i].role);
		}
	const char *collections[] = {"listview", "tableview", "timelineview"};
	for (int c = 0; c < 3; ++c)
		for (size_t i = 0; i < sizeof(scroll_rules) / sizeof(scroll_rules[0]); ++i)
		{
			snprintf(keys[generated], sizeof(keys[0]), "%s.scrollbar.%s", collections[c],
			         scroll_rules[i].key);
			props[count++] = property(keys[generated++], scroll_rules[i].role);
		}
	for (int i = 0; i < count; ++i)
		/* The shared Tooltip has no public GetType; its documented keys are registered
		 * only on first display. XUI retains these named rules until that registration. */
		if (!xuiStyleFindProperty(a->ui, props[i].sName) && strncmp(props[i].sName, "tooltip.", 8))
		{
			fprintf(stderr, "particleedit theme: unregistered XUI color: %s\n", props[i].sName);
			return XUI_ERROR_INVALID_STATE;
		}
	int result = xuiStyleSetDefault(a->ui, props, count);
	/* Hint labels use a type rule; other text keeps the normal foreground. */
	xui_style_property_t hint = property("text.color", PE_MUTED);
	xui_style_desc_t label = {sizeof(label), NULL, NULL, &hint, 1};
	if (result == XUI_OK)
		result = xuiStyleSetType(a->ui, xuiLabelGetType(a->ui), &label);
	return result == XUI_OK ? pe_theme_apply(a, a->theme) : result;
}
int pe_theme_apply(pe_app *a, int theme)
{
	if (theme < 0 || theme >= PE_THEME_COUNT)
		return XUI_ERROR_INVALID_ARGUMENT;
	const uint32_t *colors = pe_theme_colors(theme);
	int result = xuiStyleBeginUpdate(a->ui);
	if (result != XUI_OK)
		return result;
	for (int i = 0; i < PE_COLOR_COUNT && result == XUI_OK; ++i)
	{
		xui_style_value_t value = {0};
		value.iSize = sizeof(value);
		value.iType = XUI_STYLE_VALUE_COLOR;
		value.iColor = colors[i];
		result = xuiStyleSetToken(a->ui, tokens[i], &value);
	}
	int end = xuiStyleEndUpdate(a->ui);
	if (result != XUI_OK || end != XUI_OK)
		return result != XUI_OK ? result : end;
	a->theme = theme;
	a->colors = colors;
	int syncing = a->syncing;
	a->syncing = 1;
	xuiComboBoxSetSelected(a->theme_combo, theme);
	/* Timeline item colors and custom canvases are content, not XUI chrome.
	 * Recolor existing items only: don't rebuild rows, editors, playback or document history. */
	for (int i = 0; i < a->doc->data.count && i < xuiTimeLineViewGetLayerCount(a->timeline); ++i)
	{
		xuiTimeLineViewSetLayerColor(a->timeline, i, colors[i == a->doc->selected ? PE_ACCENT : PE_MUTED]);
		if (a->span_ids[i] >= 0)
			xuiTimeLineViewSetSpanColor(a->timeline, a->span_ids[i],
			                            colors[i == a->doc->selected ? PE_SELECTED : PE_SPAN]);
	}
	a->syncing = syncing;
	xuiWidgetInvalidate(a->root, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	xuiWidgetInvalidate(a->curve, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	xuiWidgetInvalidate(a->timeline, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	xuiWidgetInvalidate(a->panels[PE_WIN_PREVIEW], XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	xgeRenderRequest();
	return XUI_OK;
}
void pe_theme_preferences_load(pe_app *a)
{
	char *text = NULL;
	size_t size = 0;
	if (a->frames || a->exercise || !a->layout_path[0] ||
	    !pe_path_resolve(a->layout_path, "theme.txt", a->theme_path, sizeof(a->theme_path)))
		return;
	if (!a->theme_override && pe_file_read(a->theme_path, &text, &size, NULL))
	{
		int theme = size == 5 && !memcmp(text, "dark\n", 5)    ? PE_THEME_DARK
		            : size == 6 && !memcmp(text, "light\n", 6) ? PE_THEME_LIGHT
		                                                       : -1;
		if (theme < 0 || pe_theme_apply(a, theme) != XUI_OK)
			pe_status(a, "无法载入主题偏好，保留当前主题；可通过主题下拉框重新选择");
		free(text);
	}
}
void pe_theme_preferences_save(pe_app *a)
{
	const char *name = a->theme == PE_THEME_LIGHT ? "light\n" : "dark\n";
	if (!a->frames && !a->exercise && a->theme_path[0] && !pe_file_write(a->theme_path, name, strlen(name)))
		pe_status(a, "主题已切换，但偏好保存失败");
}
