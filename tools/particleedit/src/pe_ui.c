#include "pe_app.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *preset_names[] = {"命中火花", "火焰与烟雾", "爆炸",   "雨雪",
                                     "移动尾尘", "魔法光环",   "UI 彩纸"};
static const char *channel_names[] = {"尺寸 × 生命周期", "速度 × 生命周期", "Alpha × 生命周期", "颜色渐变"};
static const char *interpolation_names[] = {"线性 Linear", "阶梯 Step", "平滑 Hermite"};

static xui_widget panel(pe_app *a, xui_widget parent, int layout)
{
	xui_widget w = NULL;
	if (xuiWidgetCreate(a->ui, &w) != XUI_OK)
		return NULL;
	xuiWidgetSetLayoutType(w, layout);
	xuiWidgetSetSizeMode(w, XUI_SIZE_FILL, XUI_SIZE_FILL);
	xuiWidgetSetOverflow(w, XUI_OVERFLOW_CLIP);
	if (parent)
		xuiWidgetAddChild(parent, w);
	return w;
}
static void dock_child(xui_widget w, int side, float width, float height)
{
	xuiWidgetSetDock(w, side);
	xuiWidgetSetSizeMode(w, width > 0 ? XUI_SIZE_FIXED : XUI_SIZE_FILL,
	                     height > 0 ? XUI_SIZE_FIXED : XUI_SIZE_FILL);
	xuiWidgetSetPreferredSize(w, (xui_vec2_t){width, height});
}
static void click(xui_widget w, void *user)
{
	pe_action *action = user;
	(void)w;
	pe_command(action->app, action->command);
}
static xui_widget button(pe_app *a, xui_widget parent, int command, const char *text, float width)
{
	xui_widget w = NULL;
	xui_button_desc_t d = {0};
	d.iSize = sizeof(d);
	d.sText = text;
	d.pFont = a->font;
	d.iTextColor = PE_TEXT;
	d.iDisabledTextColor = PE_MUTED;
	d.iDisabledColor = PE_BG;
	d.iCheckedColor = PE_SELECTED;
	d.iNormalColor = PE_PANEL;
	d.iHoverColor = XUI_COLOR_RGBA(47, 65, 81, 255);
	d.iActiveColor = PE_SELECTED;
	d.iBorderColor = PE_LINE;
	d.fBorderWidth = 1;
	d.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE;
	if (xuiButtonCreate(a->ui, &w, &d) != XUI_OK)
		return NULL;
	xuiWidgetSetSizeMode(w, XUI_SIZE_FIXED, XUI_SIZE_FILL);
	xuiWidgetSetPreferredSize(w, (xui_vec2_t){width, 30});
	xuiWidgetSetMinSize(w, (xui_vec2_t){width, 28});
	xuiWidgetAddChild(parent, w);
	a->actions[command] = (pe_action){a, command};
	xuiButtonSetClick(w, click, &a->actions[command]);
	a->buttons[command] = w;
	return w;
}
static xui_widget row(pe_app *a, xui_widget parent, float height)
{
	xui_widget w = panel(a, parent, XUI_LAYOUT_ROW);
	dock_child(w, XUI_DOCK_TOP, 0, height);
	xuiWidgetSetGap(w, 5);
	xuiWidgetSetPadding(w, (xui_thickness_t){5, 4, 5, 4});
	return w;
}
static xui_widget label(pe_app *a, xui_widget parent, const char *text, int side, float height)
{
	xui_widget w = NULL;
	xui_label_desc_t d = {0};
	d.iSize = sizeof(d);
	d.pFont = a->font;
	d.sText = text;
	d.iTextColor = PE_MUTED;
	d.iTextFlags = XUI_TEXT_ALIGN_MIDDLE;
	xuiLabelCreate(a->ui, &w, &d);
	xuiWidgetAddChild(parent, w);
	dock_child(w, side, 0, height);
	xuiWidgetSetPadding(w, (xui_thickness_t){10, 0, 6, 0});
	return w;
}
static xui_widget combo(pe_app *a, xui_widget parent, const char **items, int count, float width)
{
	xui_widget w = NULL;
	xui_combobox_desc_t d = {0};
	d.iSize = sizeof(d);
	d.pFont = a->font;
	d.arrItems = items;
	d.iItemCount = count;
	d.iSelected = 0;
	d.fItemHeight = 30;
	d.fPopupMaxHeight = 320;
	d.iTextColor = d.iPopupTextColor = d.iPopupHoverTextColor = PE_TEXT;
	d.iBackgroundColor = d.iPopupPanelColor = PE_PANEL;
	d.iBorderColor = d.iPopupBorderColor = PE_LINE;
	d.iArrowColor = PE_ACCENT;
	d.iHoverBackgroundColor = d.iOpenBackgroundColor = d.iPopupHoverColor = PE_SELECTED;
	xuiComboBoxCreate(a->ui, &w, &d);
	xuiWidgetAddChild(parent, w);
	xuiWidgetSetSizeMode(w, XUI_SIZE_FIXED, XUI_SIZE_FILL);
	xuiWidgetSetPreferredSize(w, (xui_vec2_t){width, 30});
	xuiWidgetSetMinSize(w, (xui_vec2_t){width, 28});
	return w;
}
static int background(xui_widget w, xui_draw_context draw, uint32_t state, void *user)
{
	pe_app *a = user;
	xui_rect_t r = xuiWidgetGetRect(w);
	(void)state;
	return a->proxy.drawRectFill(&a->proxy, draw, (xui_rect_t){0, 0, r.fW, r.fH}, PE_BG);
}
static void canvas(pe_app *a, xui_widget w, xui_widget_cache_render_proc paint)
{
	xui_cache_policy_t policy = {0};
	policy.iSize = sizeof(policy);
	policy.iPolicy = XUI_CACHE_POLICY_SELF;
	policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	xuiWidgetSetCachePolicy(w, &policy);
	xuiWidgetSetCacheRenderCallback(w, paint, a);
}
static xui_rect_t stage_rect(pe_app *a, xui_widget w)
{
	xui_rect_t r = xuiWidgetGetRect(w);
	float fit = fminf(r.fW / (float)PE_STAGE_W, (r.fH - 58) / (float)PE_STAGE_H) * a->zoom;
	fit = fmaxf(.01f, fit);
	return (xui_rect_t){(r.fW - PE_STAGE_W * fit) * .5f + a->pan_x,
	                    30 + (r.fH - 58 - PE_STAGE_H * fit) * .5f + a->pan_y, PE_STAGE_W * fit,
	                    PE_STAGE_H * fit};
}
static int preview_paint(xui_widget w, xui_draw_context draw, uint32_t state, void *user)
{
	pe_app *a = user;
	xui_rect_t r = xuiWidgetGetRect(w), dst = stage_rect(a, w);
	xge_draw_t image = {0};
	xge_particle_stats_t stats = xgeParticleStats(a->preview.world);
	char text[256];
	(void)state;
	a->proxy.drawRectFill(&a->proxy, draw, (xui_rect_t){0, 0, r.fW, r.fH}, XUI_COLOR_RGBA(12, 17, 26, 255));
	/* The XGE proxy's cache-render callback owns an active XGE pass. Draw the
	 * native render target here, without readback, opaque-handle casts or a
	 * second particle simulation. XUI still owns clipping and composition. */
	image.pTexture = xgeRenderTargetTexture(&a->preview.target);
	image.tSrc = (xge_rect_t){0, 0, PE_STAGE_W, PE_STAGE_H};
	image.tDst = (xge_rect_t){dst.fX, dst.fY, dst.fW, dst.fH};
	image.iColor = 0xffffffffu;
	image.iFlags = XGE_DRAW_SCREEN_SPACE | XGE_DRAW_FLIP_Y;
	xgeDrawEx(&image);
	a->proxy.drawRectStroke(&a->proxy, draw, dst, 1, PE_LINE);
	snprintf(text, sizeof(text), "%s  %.3f s   |   %u 粒子   |   seed %llu",
	         a->preview.tick != a->preview.target_tick ? "回放中"
	         : a->preview.playing                      ? "播放"
	                                                   : "暂停",
	         a->preview.tick / (float)PE_HZ, stats.iLiveParticles, (unsigned long long)a->preview.seed);
	a->proxy.drawRectFill(&a->proxy, draw, (xui_rect_t){0, 0, r.fW, 30}, PE_BG);
	a->proxy.drawText(&a->proxy, draw, a->font, text, (xui_rect_t){12, 0, r.fW - 20, 30}, PE_TEXT,
	                  XUI_TEXT_ALIGN_MIDDLE);
	a->proxy.drawRectFill(&a->proxy, draw, (xui_rect_t){0, r.fH - 28, r.fW, 28}, PE_BG);
	a->proxy.drawText(&a->proxy, draw, a->font, "1000 × 680 舞台  ·  滚轮缩放 / 中键平移  ·  F 适合窗口",
	                  (xui_rect_t){12, r.fH - 28, r.fW - 20, 28}, PE_MUTED, XUI_TEXT_ALIGN_MIDDLE);
	if (a->preview.message[0])
	{
		a->proxy.drawRectFill(&a->proxy, draw, (xui_rect_t){8, 36, r.fW - 16, 55},
		                      XUI_COLOR_RGBA(55, 40, 25, 245));
		a->proxy.drawText(&a->proxy, draw, a->font, a->preview.message, (xui_rect_t){16, 38, r.fW - 32, 50},
		                  XUI_COLOR_RGBA(244, 193, 111, 255), 0);
	}
	return XUI_OK;
}
static int preview_event(xui_widget w, const xui_event_t *e, void *user)
{
	pe_app *a = user;
	if (e->iPhase == XUI_EVENT_PHASE_CAPTURE)
		return XUI_OK;
	if (e->iType == XUI_EVENT_POINTER_WHEEL)
	{
		a->zoom = fminf(8, fmaxf(.15f, a->zoom * powf(1.15f, e->fWheelY)));
	}
	else if (e->iType == XUI_EVENT_POINTER_DOWN)
	{
		xuiSetFocusWidget(a->ui, w);
		if (e->iButton != XUI_POINTER_BUTTON_MIDDLE)
			return XUI_OK;
		a->preview_drag = 1;
		a->mouse_x = e->fX;
		a->mouse_y = e->fY;
		a->initial_pan_x = a->pan_x;
		a->initial_pan_y = a->pan_y;
		xuiSetPointerCapture(a->ui, w);
	}
	else if (e->iType == XUI_EVENT_POINTER_MOVE && a->preview_drag)
	{
		a->pan_x = a->initial_pan_x + e->fX - a->mouse_x;
		a->pan_y = a->initial_pan_y + e->fY - a->mouse_y;
	}
	else if (e->iType == XUI_EVENT_POINTER_UP && a->preview_drag)
	{
		a->preview_drag = 0;
		xuiReleasePointerCapture(a->ui, w);
	}
	else if (e->iType == XUI_EVENT_POINTER_CAPTURE_LOST)
		a->preview_drag = 0;
	else
		return XUI_OK;
	xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_EVENT_DISPATCH_STOP;
}
static void style_editors(pe_app *a, xui_widget widget)
{
	xui_widget_type type = xuiWidgetGetType(widget);
	if (type == xuiInputGetType(a->ui))
		xuiInputSetColors(widget, PE_PANEL, PE_TEXT, PE_LINE, PE_ACCENT);
	if (type == xuiNumericInputGetType(a->ui))
		xuiNumericInputSetColors(widget, PE_PANEL, PE_TEXT, PE_LINE, PE_ACCENT);
	if (type == xuiComboBoxGetType(a->ui))
		xuiComboBoxSetColors(widget, PE_TEXT, PE_MUTED, PE_PANEL, PE_SELECTED, PE_SELECTED, PE_BG);
	for (xui_widget child = xuiWidgetGetFirstChild(widget); child; child = xuiWidgetGetNextSibling(child))
		style_editors(a, child);
}
static xui_widget property_grid(pe_app *a, xui_widget parent, int description)
{
	xui_widget w = NULL;
	xui_property_grid_desc_t d = {0};
	d.iSize = sizeof(d);
	d.pFont = a->font;
	d.fNameWidth = 193;
	d.fRowHeight = 31;
	d.fCategoryHeight = 33;
	d.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	d.iDescriptionMode =
	    description ? XUI_PROPERTY_GRID_DESCRIPTION_BOTH : XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP;
	d.fDescriptionPanelHeight = description ? 72 : 0;
	d.tStyle = (xui_property_grid_style_t){PE_PANEL,    PE_LINE,
	                                       PE_BG,       PE_SELECTED,
	                                       PE_TEXT,     PE_ACCENT,
	                                       PE_PANEL,    PE_TEXT,
	                                       PE_SELECTED, PE_PANEL,
	                                       PE_TEXT,     PE_SELECTED,
	                                       PE_MUTED,    XUI_COLOR_RGBA(239, 107, 99, 255),
	                                       PE_ACCENT};
	xuiPropertyGridCreate(a->ui, &w, &d);
	style_editors(a, w);
	xuiWidgetAddChild(parent, w);
	dock_child(w, XUI_DOCK_FILL, 0, 0);
	return w;
}
static int add_property(xui_widget grid, int category, const char *id, const char *name, const char *value,
                        int type, const char *help)
{
	xui_property_desc_t d = {id, name, help, type, value, value, 0};
	return xuiPropertyGridAddProperty(grid, category, &d);
}
int pe_set_property(pe_app *a, const char *id, const char *value)
{
	int index = pe_field_find(id);
	pe_data data = a->doc->data;
	if (index < 0 || !pe_field_set(&pe_fields[index], &data.emitters[a->doc->selected], value))
	{
		pe_status(a, "输入无效：检查数字范围、颜色格式或文本长度");
		return 0;
	}
	if (!pe_doc_commit(a->doc, &data, a->doc->selected, pe_fields[index].name))
	{
		pe_status(a, a->doc->error);
		return 0;
	}
	pe_changed(a);
	return 1;
}
static int property_validate(xui_widget w, int property, const char *id, const char *value, int type,
                             void *user)
{
	pe_app *a = user;
	pe_data data = a->doc->data;
	int index = pe_field_find(id);
	(void)w;
	(void)property;
	(void)type;
	if (a->syncing)
		return 1;
	if (index < 0 || !pe_field_set(&pe_fields[index], &data.emitters[a->doc->selected], value))
	{
		pe_status(a, "输入超出范围、文本过长或格式无效");
		return 0;
	}
	if (!pe_data_validate(&data, a->doc->error, sizeof(a->doc->error)))
	{
		pe_status(a, a->doc->error);
		return 0;
	}
	return 1;
}
static void property_change(xui_widget w, int property, const char *id, const char *value, int type,
                            void *user)
{
	pe_app *a = user;
	(void)w;
	(void)property;
	(void)type;
	if (!a->syncing)
		pe_set_property(a, id, value);
}
static void properties_sync(pe_app *a)
{
	char value[256], category_id[24];
	const char *group = NULL;
	int category = -1, ordinal = -1, expanded[20], old = xuiPropertyGridGetCategoryCount(a->properties);
	float scroll = xuiPropertyGridGetScroll(a->properties);
	for (int i = 0; i < old && i < 20; ++i)
		expanded[i] = xuiPropertyGridGetCategoryExpanded(a->properties, i);
	xuiPropertyGridClear(a->properties);
	for (int i = 0; i < pe_field_count; ++i)
	{
		const pe_field *f = &pe_fields[i];
		int sub_target = strstr(f->id, "arrSubEmitters[") && strstr(f->id, ".iEmitter");
		int type = f->type == PE_FLOAT    ? XUI_TABLE_CELL_TYPE_FLOAT
		           : f->type == PE_STRING ? XUI_TABLE_CELL_TYPE_TEXT
		           : f->type == PE_BOOL   ? XUI_TABLE_CELL_TYPE_BOOL
		           : f->type == PE_COLOR  ? XUI_TABLE_CELL_TYPE_COLOR
		           : f->type == PE_ENUM   ? XUI_TABLE_CELL_TYPE_ENUM
		                                  : XUI_TABLE_CELL_TYPE_INT;
		xui_table_grid_editor_config_t config = {0};
		xui_combobox_item_t options[XGE_PARTICLE_MAX_EMITTERS + 1] = {0};
		if (sub_target)
			type = XUI_TABLE_CELL_TYPE_ENUM;
		if (!group || strcmp(group, f->group))
		{
			group = f->group;
			++ordinal;
			snprintf(category_id, sizeof(category_id), "group%d", ordinal);
			category = xuiPropertyGridAddCategory(a->properties, category_id, group,
			                                      ordinal < old ? expanded[ordinal] : ordinal == 0);
		}
		pe_field_get(f, &a->doc->data.emitters[a->doc->selected], value, sizeof(value));
		int property = add_property(a->properties, category, f->id, f->name, value, type, f->help);
		config.fMin = (float)f->min;
		config.fMax = (float)f->max;
		config.fStep = .01f;
		config.iPrecision = 4;
		config.bAlphaEnabled = 1;
		if (sub_target)
		{
			options[0] = (xui_combobox_item_t){"关闭 (-1)", -1, 1, 0, 0, NULL};
			for (int j = 0; j < a->doc->data.count; ++j)
			{
				pe_data candidate = a->doc->data;
				*(int *)((char *)&candidate.emitters[a->doc->selected] + f->offset) = j;
				options[j + 1].sText = a->list_text[j];
				options[j + 1].iValue = j;
				options[j + 1].bEnabled = pe_data_validate(&candidate, NULL, 0);
			}
			config.arrEnumItemData = options;
			config.iEnumItemCount = a->doc->data.count + 1;
			config.iEnumSelectedValue = atoi(value);
			config.bEnumUseValue = 1;
		}
		else if (f->type == PE_ENUM)
		{
			for (int j = 0; j < f->option_count; ++j)
			{
				options[j].sText = f->options[j];
				options[j].iValue = j;
				options[j].bEnabled = 1;
			}
			config.arrEnumItemData = options;
			config.iEnumItemCount = f->option_count;
			config.iEnumSelected = atoi(value);
			config.iEnumSelectedValue = atoi(value);
			config.bEnumUseValue = 1;
		}
		xuiPropertyGridSetEditorConfig(a->properties, property, &config);
	}
	xuiPropertyGridSetScroll(a->properties, a->selection_changed ? 0 : scroll);
}
static int key_candidate(pe_app *a, const char *id, const char *value, pe_data *data)
{
	float number;
	int count = a->channel == 3 ? data->emitters[a->doc->selected].tColorOverLife.iCount
	                            : pe_curve(data, a->doc->selected, a->channel)->iCount;
	if (a->key < 0 || a->key >= count)
		return 0;
	if (a->channel == 3)
	{
		xge_particle_color_key_t *k = &data->emitters[a->doc->selected].tColorOverLife.arrKeys[a->key];
		if (!strcmp(id, "color"))
			return pe_parse_color(value, &k->iColor);
		if (!pe_parse_float(value, 0, 1, &number))
			return 0;
		k->fTime = number;
	}
	else
	{
		xge_particle_key_t *k = &pe_curve(data, a->doc->selected, a->channel)->arrKeys[a->key];
		if (!pe_parse_float(value, !strcmp(id, "time") ? 0 : -1e6f, !strcmp(id, "time") ? 1 : 1e6f, &number))
			return 0;
		if (!strcmp(id, "time"))
			k->fTime = number;
		else if (!strcmp(id, "value"))
			k->fValue = number;
		else if (!strcmp(id, "in"))
			k->fInTangent = number;
		else if (!strcmp(id, "out"))
			k->fOutTangent = number;
		else
			return 0;
	}
	return 1;
}
static int key_validate(xui_widget w, int index, const char *id, const char *value, int type, void *user)
{
	pe_app *a = user;
	pe_data data = a->doc->data;
	(void)w;
	(void)index;
	(void)type;
	if (a->syncing)
		return 1;
	if (!key_candidate(a, id, value, &data) || !pe_data_validate(&data, a->doc->error, sizeof(a->doc->error)))
	{
		pe_status(a, "关键点无效：时间须在 0～1 内且严格递增");
		return 0;
	}
	return 1;
}
static void key_change(xui_widget w, int index, const char *id, const char *value, int type, void *user)
{
	pe_app *a = user;
	pe_data data = a->doc->data;
	(void)w;
	(void)index;
	(void)type;
	if (!a->syncing && key_candidate(a, id, value, &data) &&
	    pe_doc_commit(a->doc, &data, a->doc->selected, "修改关键点"))
		pe_changed(a);
}
void pe_key_grid_sync(pe_app *a)
{
	const char *ids[] = {"time", "value", "in", "out"};
	const char *names[] = {"归一化时间", "系数", "入切线斜率", "出切线斜率"};
	int category, count;
	char value[128];
	xge_particle_curve_t *c = pe_curve(&a->doc->data, a->doc->selected, a->channel);
	xge_particle_gradient_t *g = &a->doc->data.emitters[a->doc->selected].tColorOverLife;
	count = a->channel == 3 ? g->iCount : c->iCount;
	if (a->key >= count)
		a->key = count - 1;
	if (a->key < 0 && count)
		a->key = 0;
	xuiPropertyGridClear(a->key_grid);
	snprintf(value, sizeof(value), "关键点 %d / %d", a->key + 1, count);
	category = xuiPropertyGridAddCategory(a->key_grid, "key", value, 1);
	if (a->key < 0)
	{
		add_property(a->key_grid, category, "hint", "空曲线", "默认系数 1", XUI_TABLE_CELL_TYPE_TEXT,
		             "双击画布添加关键点；空渐变为白色");
		xuiPropertyGridSetPropertyFlags(a->key_grid, 0, XUI_PROPERTY_FLAG_READONLY);
		return;
	}
	for (int i = 0; i < (a->channel == 3 ? 1 : 4); ++i)
	{
		xge_particle_key_t k = c->arrKeys[a->key];
		float number = a->channel == 3 ? g->arrKeys[a->key].fTime
		               : i == 0        ? k.fTime
		               : i == 1        ? k.fValue
		               : i == 2        ? k.fInTangent
		                               : k.fOutTangent;
		snprintf(value, sizeof(value), "%.9g", number);
		int p =
		    add_property(a->key_grid, category, ids[i], names[i], value, XUI_TABLE_CELL_TYPE_FLOAT,
		                 i > 1 ? "Hermite 切线斜率：值变化 / 归一化时间" : "出生随机值乘以该生命周期系数");
		xui_table_grid_editor_config_t config = {0};
		config.fMin = i == 0 ? 0 : -1e6f;
		config.fMax = i == 0 ? 1 : 1e6f;
		config.fStep = .01f;
		config.iPrecision = 4;
		xuiPropertyGridSetEditorConfig(a->key_grid, p, &config);
		if (i > 1 && c->iInterpolation != XGE_PARTICLE_CURVE_HERMITE)
			xuiPropertyGridSetPropertyFlags(a->key_grid, p, XUI_PROPERTY_FLAG_DISABLED);
	}
	if (a->channel == 3)
	{
		uint32_t color = g->arrKeys[a->key].iColor;
		snprintf(value, sizeof(value), "#%02X%02X%02X%02X", XGE_COLOR_GET_R(color), XGE_COLOR_GET_G(color),
		         XGE_COLOR_GET_B(color), XGE_COLOR_GET_A(color));
		int p = add_property(a->key_grid, category, "color", "RGBA", value, XUI_TABLE_CELL_TYPE_COLOR,
		                     "点击色块编辑，包括 Alpha");
		xui_table_grid_editor_config_t config = {0};
		config.bAlphaEnabled = 1;
		xuiPropertyGridSetEditorConfig(a->key_grid, p, &config);
	}
	xuiPropertyGridSetMetrics(a->key_grid, 120, 30, 32);
}
static int burst_candidate(pe_app *a, const char *id, const char *value, pe_data *data)
{
	int index, count;
	float time;
	char field;
	xge_particle_emitter_t *e = &data->emitters[a->doc->selected];
	if (sscanf(id, "burst%d%c", &index, &field) != 2 || index < 0 || index >= e->iBurstCount)
		return 0;
	a->burst = index;
	if (field == 't')
	{
		if (!pe_parse_float(value, 0, 1e6, &time))
			return 0;
		e->arrBursts[index].fTime = time;
	}
	else
	{
		if (!pe_parse_int(value, 0, 1000000, &count))
			return 0;
		e->arrBursts[index].iCount = (uint32_t)count;
	}
	return 1;
}
static int burst_validate(xui_widget w, int index, const char *id, const char *value, int type, void *user)
{
	pe_app *a = user;
	pe_data data = a->doc->data;
	(void)w;
	(void)index;
	(void)type;
	if (a->syncing)
		return 1;
	if (!burst_candidate(a, id, value, &data) ||
	    !pe_data_validate(&data, a->doc->error, sizeof(a->doc->error)))
	{
		pe_status(a, "Burst 时间必须 >= 0 且小于非零持续时间，数量 0～100 万");
		return 0;
	}
	return 1;
}
static void burst_change(xui_widget w, int index, const char *id, const char *value, int type, void *user)
{
	pe_app *a = user;
	pe_data data = a->doc->data;
	(void)w;
	(void)index;
	(void)type;
	if (!a->syncing && burst_candidate(a, id, value, &data) &&
	    pe_doc_commit(a->doc, &data, a->doc->selected, "编辑 Burst"))
		pe_changed(a);
}
static void burst_select(xui_widget w, int index, const char *id, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)index;
	if (id)
		sscanf(id, "burst%d", &a->burst);
}
static void bursts_sync(pe_app *a)
{
	xge_particle_emitter_t *e = &a->doc->data.emitters[a->doc->selected];
	char id[32], name[80], value[64];
	float scroll = xuiPropertyGridGetScroll(a->bursts);
	xuiPropertyGridClear(a->bursts);
	for (int i = 0; i < e->iBurstCount; ++i)
	{
		snprintf(id, sizeof(id), "burst%d", i);
		snprintf(name, sizeof(name), "Burst %d · 相对发射器启动", i + 1);
		int category = xuiPropertyGridAddCategory(a->bursts, id, name, 1);
		snprintf(id, sizeof(id), "burst%dt", i);
		snprintf(value, sizeof(value), "%.9g", e->arrBursts[i].fTime);
		int property = add_property(a->bursts, category, id, "时间 / s", value, XUI_TABLE_CELL_TYPE_FLOAT,
		                            "相对 delay，不是特效绝对时间；时间轴上的菱形可以拖动");
		xui_table_grid_editor_config_t config = {0};
		config.fMax = 1000000;
		config.fStep = .01f;
		config.iPrecision = 4;
		xuiPropertyGridSetEditorConfig(a->bursts, property, &config);
		snprintf(id, sizeof(id), "burst%dn", i);
		snprintf(value, sizeof(value), "%u", e->arrBursts[i].iCount);
		property = add_property(a->bursts, category, id, "发射数量", value, XUI_TABLE_CELL_TYPE_INT,
		                        "与连续发射率相互独立");
		config.fStep = 1;
		config.iPrecision = 0;
		xuiPropertyGridSetEditorConfig(a->bursts, property, &config);
	}
	if (a->burst >= e->iBurstCount)
		a->burst = e->iBurstCount - 1;
	xuiPropertyGridSetSelected(a->bursts, a->burst < 0 ? -1 : a->burst * 2);
	xuiPropertyGridSetScroll(a->bursts, scroll);
}
static void list_select(xui_widget w, int index, void *user)
{
	(void)w;
	pe_app *a = user;
	if (!a->syncing)
		pe_select(a, index);
}
static void channel_select(xui_widget w, int index, int value, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)value;
	if (a->syncing)
		return;
	if (!pe_finish_edit(a))
	{
		a->syncing = 1;
		xuiComboBoxSetSelected(w, a->channel);
		a->syncing = 0;
		return;
	}
	a->channel = index;
	a->key = 0;
	a->refresh = 1;
}
static void interpolation_select(xui_widget w, int index, int value, void *user)
{
	pe_app *a = user;
	pe_data data = a->doc->data;
	(void)w;
	(void)value;
	if (a->syncing || a->channel == 3)
		return;
	pe_curve(&data, a->doc->selected, a->channel)->iInterpolation = index;
	if (pe_doc_commit(a->doc, &data, a->doc->selected, "修改曲线插值"))
		pe_changed(a);
}
static void preset_select(xui_widget w, int index, int value, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)value;
	a->preset = index;
}
static int settings_validate(xui_widget w, int index, const char *id, const char *value, int type, void *user)
{
	float v;
	int n;
	(void)w;
	(void)index;
	(void)type;
	(void)user;
	if (!strcmp(id, "seed"))
		return pe_parse_int(value, 0, 2147483647, &n);
	if (!strcmp(id, "range"))
		return pe_parse_float(value, 1, PE_PREVIEW_SECONDS, &v);
	if (!strcmp(id, "speed"))
		return pe_parse_float(value, .05f, 4, &v);
	if (!strcmp(id, "originX") || !strcmp(id, "originY"))
		return pe_parse_float(value, -1e6f, 1e6f, &v);
	return !strcmp(value, "true") || !strcmp(value, "false") || !strcmp(value, "0") || !strcmp(value, "1");
}
static void settings_change(xui_widget w, int index, const char *id, const char *value, int type, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)index;
	(void)type;
	if (a->syncing)
		return;
	int flag = !strcmp(value, "true") || !strcmp(value, "1");
	if (!strcmp(id, "seed"))
		a->preview.seed = (uint64_t)strtoul(value, NULL, 10);
	else if (!strcmp(id, "range"))
		a->preview.range_seconds = (float)atof(value);
	else if (!strcmp(id, "speed"))
		a->preview.speed = (float)atof(value);
	else if (!strcmp(id, "originX"))
		a->preview.origin_x = (float)atof(value);
	else if (!strcmp(id, "originY"))
		a->preview.origin_y = (float)atof(value);
	else if (!strcmp(id, "repeat"))
		a->preview.repeat = flag;
	else if (!strcmp(id, "grid"))
		a->preview.grid = flag;
	else if (!strcmp(id, "bounds"))
		a->preview.bounds = flag;
	else if (!strcmp(id, "ground"))
		a->preview.ground = flag;
	else if (!strcmp(id, "moving"))
		a->preview.moving = flag;
	a->refresh = a->rebuild = 1;
	a->preview.target_tick = 0;
}
static void settings_sync(pe_app *a)
{
	const char *ids[] = {"seed",   "range", "speed",  "originX", "originY",
	                     "repeat", "grid",  "bounds", "ground",  "moving"};
	const char *names[] = {"随机种子", "预览长度 / s", "播放速度",        "根位置 X",       "根位置 Y",
	                       "预览循环", "网格",         "边界 / 发射形状", "地面碰撞 Y=570", "移动路径"};
	float values[] = {(float)a->preview.seed,  a->preview.range_seconds, a->preview.speed,
	                  a->preview.origin_x,     a->preview.origin_y,      (float)a->preview.repeat,
	                  (float)a->preview.grid,  (float)a->preview.bounds, (float)a->preview.ground,
	                  (float)a->preview.moving};
	xuiPropertyGridClear(a->settings);
	int category = xuiPropertyGridAddCategory(a->settings, "preview", "仅影响预览，不写入粒子资产", 1);
	for (int i = 0; i < 10; ++i)
	{
		char text[64];
		if (i >= 5)
			strcpy(text, values[i] ? "true" : "false");
		else if (i == 0)
			snprintf(text, sizeof(text), "%llu", (unsigned long long)a->preview.seed);
		else
			snprintf(text, sizeof(text), "%.9g", values[i]);
		int p = add_property(a->settings, category, ids[i], names[i], text,
		                     i >= 5   ? XUI_TABLE_CELL_TYPE_BOOL
		                     : i == 0 ? XUI_TABLE_CELL_TYPE_INT
		                              : XUI_TABLE_CELL_TYPE_FLOAT,
		                     "模拟固定步长 1/120 秒，时间轴 60 fps；修改预览设置会从零重新播放，资产不变脏");
		if (i < 5)
		{
			xui_table_grid_editor_config_t c = {0};
			c.fMin = i == 1 ? 1 : i == 2 ? .05f : i == 0 ? 0 : -1e6f;
			c.fMax = i == 1 ? 60 : i == 2 ? 4 : i == 0 ? 2147483647.f : 1e6f;
			c.fStep = i == 0 ? 1 : .1f;
			c.iPrecision = 3;
			xuiPropertyGridSetEditorConfig(a->settings, p, &c);
		}
	}
}
static void timeline_frame(xui_widget w, int old, int now, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)old;
	if (!a->syncing)
	{
		a->preview.playing = 0;
		pe_preview_seek(&a->preview, now * 2);
		a->refresh = 1;
	}
}
static void timeline_layer(xui_widget w, int layer, void *user)
{
	pe_app *a = user;
	(void)w;
	if (!a->syncing)
		pe_select(a, layer);
}
static int timeline_deny_menu(xui_widget w, const xui_timeline_hit_t *hit, void *user)
{
	(void)w;
	(void)hit;
	(void)user;
	return 0;
}
static int timeline_deny_frame(xui_widget w, int l, int f, int old, int now, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)l;
	(void)f;
	(void)old;
	(void)now;
	return a->syncing;
}
static int timeline_layer_changing(xui_widget w, int l, int change, void *user)
{
	pe_app *a = user;
	(void)w;
	(void)l;
	return a->syncing || change == XUI_TIMELINE_LAYER_CHANGE_VISIBLE ||
	       change == XUI_TIMELINE_LAYER_CHANGE_SELECT;
}
static void timeline_layer_changed(xui_widget w, int l, int change, void *user)
{
	pe_app *a = user;
	if (!a->syncing && change == XUI_TIMELINE_LAYER_CHANGE_VISIBLE && l >= 0 && l < a->doc->data.count)
	{
		a->preview.muted[l] = !xuiTimeLineViewGetLayerVisible(w, l);
		a->rebuild = a->refresh = 1;
	}
}
static int timeline_ruler(xui_widget w, int frame, xui_draw_context draw, xui_rect_t rect, int state,
                          void *user)
{
	pe_app *a = user;
	float width = xuiTimeLineViewGetFrameWidth(w);
	int step = (int)ceilf(64 / (60 * width)) * 60;
	char text[32];
	(void)state;
	if (step < 60)
		step = 60;
	if (frame % step == 0)
	{
		snprintf(text, sizeof(text), "%g s", frame / 60.f);
		a->proxy.drawText(&a->proxy, draw, a->font, text, (xui_rect_t){rect.fX + 3, rect.fY, 64, rect.fH - 6},
		                  PE_MUTED, XUI_TEXT_ALIGN_MIDDLE);
		a->proxy.drawLine(&a->proxy, draw, rect.fX, rect.fY + rect.fH - 8, rect.fX, rect.fY + rect.fH, 1,
		                  PE_MUTED);
	}
	return 1;
}
static int timeline_marker(xui_widget w, int layer, int frame, const xui_timeline_frame_t *marker,
                           xui_draw_context draw, xui_rect_t rect, int state, void *user)
{
	pe_app *a = user;
	float x = rect.fX, y = rect.fY + rect.fH * .5f;
	uint32_t color = XUI_COLOR_RGBA(244, 181, 96, 255);
	(void)w;
	(void)layer;
	(void)frame;
	(void)marker;
	(void)state;
	a->proxy.drawTriangleFill(&a->proxy, draw, (xui_vec2_t){x, y - 6}, (xui_vec2_t){x - 5, y},
	                          (xui_vec2_t){x + 5, y}, color);
	a->proxy.drawTriangleFill(&a->proxy, draw, (xui_vec2_t){x, y + 6}, (xui_vec2_t){x - 5, y},
	                          (xui_vec2_t){x + 5, y}, color);
	return 1;
}

int pe_ui_create(pe_app *a)
{
	xui_theme_t theme;
	xui_dock_panel_desc_t dock = {0};
	xui_list_view_desc_t list = {0};
	xui_timeline_view_desc_t timeline = {0};
	xui_widget r;
	const char *titles[] = {
	    "实时预览", "发射器 / 模板", "发射器属性", "发射时间轴 · 60 fps", "生命周期曲线 / 渐变",
	    "Burst 表", "预览设置"};
	xuiThemeDefault(&theme);
	theme.pFont = a->font;
	theme.iTextColor = PE_TEXT;
	theme.iBackgroundColor = PE_BG;
	theme.iPanelColor = PE_PANEL;
	theme.iBorderColor = PE_LINE;
	theme.iAccentColor = PE_ACCENT;
	theme.iSelectionColor = PE_SELECTED;
	theme.iStateNormalColor = PE_PANEL;
	theme.iStateHoverColor = PE_SELECTED;
	theme.iStateActiveColor = PE_SELECTED;
	xuiSetTheme(a->ui, &theme);
	a->root = panel(a, NULL, XUI_LAYOUT_DOCK);
	xuiSetRootWidget(a->ui, a->root);
	canvas(a, a->root, background);
	a->header = row(a, a->root, 42);
	button(a, a->header, PE_NEW, "新建", 60);
	button(a, a->header, PE_OPEN, "打开", 60);
	button(a, a->header, PE_SAVE, "保存", 60);
	button(a, a->header, PE_SAVE_AS, "另存为", 78);
	button(a, a->header, PE_UNDO, "撤销", 60);
	button(a, a->header, PE_REDO, "重做", 60);
	button(a, a->header, PE_RESET_LAYOUT, "重置布局", 100);
	button(a, a->header, PE_SHOW_CURVES, "曲线 / 渐变", 115);
	button(a, a->header, PE_SHOW_BURSTS, "Burst", 70);
	button(a, a->header, PE_TEXTURE, "绑定纹理", 100);
	button(a, a->header, PE_RELOAD, "重载资源", 100);
	button(a, a->header, PE_HELP, "操作说明", 100);
	a->transport = row(a, a->root, 42);
	button(a, a->transport, PE_PLAY, "暂停", 64);
	button(a, a->transport, PE_RESTART, "重播", 64);
	button(a, a->transport, PE_STEP, "单步", 64);
	button(a, a->transport, PE_EMIT, "发射 32", 88);
	button(a, a->transport, PE_FIT, "适合窗口", 100);
	button(a, a->transport, PE_BOUNDS, "边界", 64);
	button(a, a->transport, PE_GROUND, "地面", 64);
	button(a, a->transport, PE_MOVE, "移动", 64);
	a->status = label(a, a->root, "XGE Particle Editor", XUI_DOCK_BOTTOM, 32);
	dock.iSize = sizeof(dock);
	dock.pFont = a->font;
	if (xuiDockPanelCreate(a->ui, &a->dock, &dock) != XUI_OK)
		return XUI_ERROR_OUT_OF_MEMORY;
	xuiWidgetAddChild(a->root, a->dock);
	dock_child(a->dock, XUI_DOCK_FILL, 0, 0);
	xui_dock_panel_colors_t colors;
	xuiDockPanelGetColors(a->dock, &colors);
	colors.iBackgroundColor = PE_BG;
	colors.iPaneColor = colors.iClientColor = PE_PANEL;
	colors.iCaptionColor = PE_BG;
	colors.iCaptionTextColor = PE_MUTED;
	colors.iTabColor = PE_BG;
	colors.iActiveTabColor = PE_SELECTED;
	colors.iTabTextColor = PE_MUTED;
	colors.iActiveTabTextColor = PE_TEXT;
	colors.iBorderColor = PE_LINE;
	colors.iSplitterColor = PE_BG;
	colors.iFloatTitleColor = PE_SELECTED;
	colors.iFloatBorderColor = PE_ACCENT;
	xuiDockPanelSetColors(a->dock, &colors);
	for (int i = 0; i < PE_WIN_COUNT; ++i)
		a->panels[i] = panel(a, NULL, XUI_LAYOUT_DOCK);
	canvas(a, a->panels[PE_WIN_PREVIEW], preview_paint);
	xuiWidgetSetFocusable(a->panels[PE_WIN_PREVIEW], 1);
	xuiWidgetSetEventCallback(a->panels[PE_WIN_PREVIEW], preview_event, a);
	xuiWidgetSetEventInterest(a->panels[PE_WIN_PREVIEW], XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE, 1);
	r = row(a, a->panels[PE_WIN_EMITTERS], 40);
	a->preset_combo = combo(a, r, preset_names, 7, 142);
	xuiComboBoxSetSelect(a->preset_combo, preset_select, a);
	button(a, r, PE_TEMPLATE, "载入", 55);
	r = row(a, a->panels[PE_WIN_EMITTERS], 38);
	button(a, r, PE_ADD, "添加", 58);
	button(a, r, PE_DUPLICATE, "复制", 58);
	button(a, r, PE_REMOVE, "删除", 58);
	r = row(a, a->panels[PE_WIN_EMITTERS], 38);
	button(a, r, PE_UP, "↑", 34);
	button(a, r, PE_DOWN, "↓", 34);
	button(a, r, PE_MUTE, "静音", 52);
	button(a, r, PE_SOLO, "独奏", 52);
	label(a, a->panels[PE_WIN_EMITTERS], "索引从 0 开始\n静音 / 独奏仅影响预览", XUI_DOCK_BOTTOM, 64);
	list.iSize = sizeof(list);
	list.pFont = a->font;
	list.fItemHeight = 40;
	list.iSelected = 0;
	list.iBackgroundColor = list.iRowColor = PE_PANEL;
	list.iTextColor = PE_TEXT;
	list.iSelectedColor = PE_SELECTED;
	list.iHoverColor = PE_BG;
	list.iBorderColor = PE_LINE;
	xuiListViewCreate(a->ui, &a->list, &list);
	xuiWidgetAddChild(a->panels[PE_WIN_EMITTERS], a->list);
	dock_child(a->list, XUI_DOCK_FILL, 0, 0);
	xuiListViewSetSelect(a->list, list_select, a);
	a->properties = property_grid(a, a->panels[PE_WIN_PROPERTIES], 1);
	xuiPropertyGridSetValidate(a->properties, property_validate, a);
	xuiPropertyGridSetChange(a->properties, property_change, a);
	a->settings = property_grid(a, a->panels[PE_WIN_SETTINGS], 1);
	xuiPropertyGridSetValidate(a->settings, settings_validate, a);
	xuiPropertyGridSetChange(a->settings, settings_change, a);
	label(a, a->panels[PE_WIN_TIMELINE],
	      "色条：启动 / 右缘：持续时间 · 双击区间：加 Burst / 菱形：打开表 · Ctrl+滚轮：缩放",
	      XUI_DOCK_BOTTOM, 30);
	timeline.iSize = sizeof(timeline);
	timeline.pFont = a->font;
	timeline.iFrameCount = 601;
	timeline.fFrameRate = 60;
	timeline.fFrameWidth = 1;
	timeline.fMinFrameWidth = .25f;
	timeline.fMaxFrameWidth = 30;
	timeline.fRowHeight = 34;
	timeline.fRulerHeight = 30;
	timeline.fLayerHeaderWidth = 172;
	timeline.bShowVisibilityFeature = 1;
	xuiTimeLineViewCreate(a->ui, &a->timeline, &timeline);
	xuiTimeLineViewSetFeatureFlags(a->timeline, 1, 0);
	xuiTimeLineViewSetRenderers(a->timeline, NULL, timeline_ruler, timeline_marker, NULL, a);
	xuiWidgetAddChild(a->panels[PE_WIN_TIMELINE], a->timeline);
	dock_child(a->timeline, XUI_DOCK_FILL, 0, 0);
	xui_timeline_view_colors_t tc;
	xuiTimeLineViewGetColors(a->timeline, &tc);
	tc.iBackgroundColor = PE_PANEL;
	tc.iCornerColor = tc.iRulerColor = PE_BG;
	tc.iLayerColor = PE_PANEL;
	tc.iLayerAltColor = PE_BG;
	tc.iGridColor = PE_LINE;
	tc.iGridStrongColor = PE_LINE;
	tc.iTextColor = PE_TEXT;
	tc.iMutedTextColor = PE_MUTED;
	tc.iSelectedColor = PE_SELECTED;
	tc.iCurrentColor = PE_ACCENT;
	tc.iSpanColor = PE_SELECTED;
	tc.iSpanTextColor = PE_TEXT;
	tc.iBorderColor = PE_LINE;
	xuiTimeLineViewSetColors(a->timeline, &tc);
	xuiTimeLineViewSetCurrentFrameCallbacks(a->timeline, NULL, timeline_frame, a);
	xuiTimeLineViewSetLayerSelected(a->timeline, timeline_layer, a);
	xuiTimeLineViewSetLayerCallbacks(a->timeline, timeline_layer_changing, timeline_layer_changed, a);
	xuiTimeLineViewSetContextMenu(a->timeline, timeline_deny_menu, NULL, a);
	xuiTimeLineViewSetFrameCallbacks(a->timeline, timeline_deny_frame, NULL, a);
	xuiWidgetSetEventCallback(a->panels[PE_WIN_TIMELINE], pe_timeline_event, a);
	xuiWidgetSetEventInterest(a->panels[PE_WIN_TIMELINE],
	                          XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE | XUI_EVENT_MASK_KEYBOARD, 1);
	r = row(a, a->panels[PE_WIN_CURVES], 40);
	a->channel_combo = combo(a, r, channel_names, 4, 205);
	a->interpolation_combo = combo(a, r, interpolation_names, 3, 155);
	xuiComboBoxSetSelect(a->channel_combo, channel_select, a);
	xuiComboBoxSetSelect(a->interpolation_combo, interpolation_select, a);
	button(a, r, PE_CURVE_ADD, "加点", 58);
	button(a, r, PE_CURVE_REMOVE, "删点", 58);
	button(a, r, PE_CURVE_RESET, "清空", 58);
	button(a, r, PE_CURVE_FADE, "淡出", 58);
	label(a, a->panels[PE_WIN_CURVES],
	      "X = 归一化寿命 0～1 · 双击加点 / 拖动关键点 / Delete 删除 · Hermite 可编辑切线斜率",
	      XUI_DOCK_BOTTOM, 28);
	a->key_grid = property_grid(a, a->panels[PE_WIN_CURVES], 0);
	dock_child(a->key_grid, XUI_DOCK_RIGHT, 258, 0);
	a->curve = panel(a, a->panels[PE_WIN_CURVES], XUI_LAYOUT_MANUAL);
	dock_child(a->curve, XUI_DOCK_FILL, 0, 0);
	canvas(a, a->curve, pe_curve_paint);
	xuiWidgetSetFocusable(a->curve, 1);
	xuiWidgetSetEventCallback(a->curve, pe_curve_event, a);
	xuiWidgetSetEventInterest(a->curve,
	                          XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE | XUI_EVENT_MASK_KEYBOARD, 1);
	xuiPropertyGridSetValidate(a->key_grid, key_validate, a);
	xuiPropertyGridSetChange(a->key_grid, key_change, a);
	r = row(a, a->panels[PE_WIN_BURSTS], 40);
	button(a, r, PE_BURST_ADD, "添加 Burst", 115);
	button(a, r, PE_BURST_REMOVE, "删除选中 Burst", 150);
	a->bursts = property_grid(a, a->panels[PE_WIN_BURSTS], 0);
	xuiPropertyGridSetValidate(a->bursts, burst_validate, a);
	xuiPropertyGridSetChange(a->bursts, burst_change, a);
	xuiPropertyGridSetSelect(a->bursts, burst_select, a);
	for (int i = 0; i < PE_WIN_COUNT; ++i)
	{
		xuiDockPanelAddWindow(a->dock, titles[i], a->panels[i], &a->windows[i]);
		xuiDockPanelSetWindowFlags(a->dock, a->windows[i], 0, 1);
		xuiDockPanelSetWindowMinSize(a->dock, a->windows[i], i == PE_WIN_EMITTERS ? 218 : 280, 120);
	}
	xuiDockPanelDockWindow(a->dock, a->windows[PE_WIN_PREVIEW], XUI_DOCK_PANEL_REGION_DOCUMENT,
	                       XUI_DOCK_PANEL_SIDE_FILL, 0, NULL);
	xuiDockPanelDockWindow(a->dock, a->windows[PE_WIN_TIMELINE], XUI_DOCK_PANEL_REGION_DOCUMENT,
	                       XUI_DOCK_PANEL_SIDE_BOTTOM, .30f, &a->bottom_pane);
	xuiDockPanelDockWindowToPane(a->dock, a->windows[PE_WIN_CURVES], a->bottom_pane);
	xuiDockPanelDockWindowToPane(a->dock, a->windows[PE_WIN_BURSTS], a->bottom_pane);
	xuiDockPanelDockWindow(a->dock, a->windows[PE_WIN_PROPERTIES], XUI_DOCK_PANEL_REGION_DOCUMENT,
	                       XUI_DOCK_PANEL_SIDE_RIGHT, .29f, &a->property_pane);
	xuiDockPanelDockWindowToPane(a->dock, a->windows[PE_WIN_SETTINGS], a->property_pane);
	xuiDockPanelDockWindow(a->dock, a->windows[PE_WIN_EMITTERS], XUI_DOCK_PANEL_REGION_DOCUMENT,
	                       XUI_DOCK_PANEL_SIDE_LEFT, .165f, NULL);
	xuiDockPanelSetPaneActiveWindow(a->dock, a->bottom_pane, a->windows[PE_WIN_TIMELINE]);
	xuiDockPanelSetPaneActiveWindow(a->dock, a->property_pane, a->windows[PE_WIN_PROPERTIES]);
	xuiDockPanelSaveState(a->dock, &a->default_layout);
	a->refresh = 1;
	return XUI_OK;
}
void pe_ui_refresh(pe_app *a)
{
	a->syncing = 1;
	for (int i = 0; i < a->doc->data.count; ++i)
	{
		snprintf(a->list_text[i], sizeof(a->list_text[i]), "%d  %s%s%s", i, a->doc->data.emitters[i].sName,
		         a->preview.muted[i] ? " [M]" : "", a->preview.solo == i ? " [S]" : "");
		a->list_items[i] = a->list_text[i];
	}
	xuiListViewSetItems(a->list, a->list_items, a->doc->data.count);
	xuiListViewSetSelected(a->list, a->doc->selected);
	properties_sync(a);
	pe_key_grid_sync(a);
	bursts_sync(a);
	settings_sync(a);
	pe_timeline_sync(a);
	pe_curve_range(a);
	xuiComboBoxSetSelected(a->channel_combo, a->channel);
	xuiComboBoxSetSelected(a->preset_combo, a->preset);
	xuiComboBoxSetSelected(a->interpolation_combo,
	                       pe_curve(&a->doc->data, a->doc->selected, a->channel)->iInterpolation);
	xuiWidgetSetEnabled(a->interpolation_combo, a->channel != 3);
	xuiWidgetSetEnabled(a->buttons[PE_UNDO], a->doc->cursor > 0);
	xuiWidgetSetEnabled(a->buttons[PE_REDO], a->doc->cursor + 1 < a->doc->length);
	xuiButtonSetText(a->buttons[PE_PLAY], a->preview.playing ? "暂停" : "播放");
	xuiButtonSetSelected(a->buttons[PE_MUTE], a->preview.muted[a->doc->selected]);
	xuiButtonSetSelected(a->buttons[PE_SOLO], a->preview.solo == a->doc->selected);
	xuiButtonSetSelected(a->buttons[PE_BOUNDS], a->preview.bounds);
	xuiButtonSetSelected(a->buttons[PE_GROUND], a->preview.ground);
	xuiButtonSetSelected(a->buttons[PE_MOVE], a->preview.moving);
	xuiWidgetInvalidate(a->curve, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	a->selection_changed = 0;
	a->refresh = 0;
	a->syncing = 0;
}
int pe_ui_resize(pe_app *a)
{
	int width = xgeGetWidth(), height = xgeGetHeight();
	if (width <= 0 || height <= 0)
		return XUI_OK;
	if (width == a->width && height == a->height && a->target)
		return XUI_OK;
	xui_surface target = NULL;
	xui_surface_desc_t d = {XUI_SURFACE_KIND_TEXTURE, XUI_SURFACE_FORMAT_RGBA8, width, height,
	                        XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET};
	int result = a->proxy.surfaceCreate(&a->proxy, &target, &d);
	if (result != XUI_OK)
		return result;
	if (a->target)
		a->proxy.surfaceDestroy(&a->proxy, a->target);
	a->target = target;
	a->width = width;
	a->height = height;
	xuiSetViewportSize(a->ui, width, height);
	xuiInputViewport(a->ui, width, height);
	if (a->root)
		xuiWidgetSetRect(a->root, (xui_rect_t){0, 0, (float)width, (float)height});
	return XUI_OK;
}
