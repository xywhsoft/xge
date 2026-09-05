#include "pe_app.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
#define CHECK(x)                                                                                             \
	do                                                                                                       \
	{                                                                                                        \
		++checks;                                                                                            \
		if (!(x))                                                                                            \
		{                                                                                                    \
			fprintf(stderr, "UI check %d failed at %s:%d: %s\n", checks, __FILE__, __LINE__, #x);            \
			goto failed;                                                                                     \
		}                                                                                                    \
	} while (0)
static int input_click(pe_app *a, float x, float y)
{
	int r = xuiInputPointerDown(a->ui, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if (r == XUI_OK)
		r = xuiDispatchPendingEvents(a->ui);
	if (r == XUI_OK)
		r = xuiInputPointerUp(a->ui, x, y, XUI_POINTER_BUTTON_LEFT, 0);
	if (r == XUI_OK)
		r = xuiDispatchPendingEvents(a->ui);
	return r;
}
static int button_click(pe_app *a, int command)
{
	xui_rect_t r = xuiWidgetGetWorldRect(a->buttons[command]);
	return input_click(a, r.fX + r.fW * .5f, r.fY + r.fH * .5f);
}
static int drag(pe_app *a, float x, float y, float dx, float dy)
{
	int r = xuiInputPointerDown(a->ui, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if (r == XUI_OK)
		r = xuiDispatchPendingEvents(a->ui);
	if (r == XUI_OK)
		r = xuiInputPointerMove(a->ui, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT);
	if (r == XUI_OK)
		r = xuiDispatchPendingEvents(a->ui);
	if (r == XUI_OK)
		r = xuiInputPointerUp(a->ui, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT, 0);
	if (r == XUI_OK)
		r = xuiDispatchPendingEvents(a->ui);
	return r;
}
static void sync(pe_app *a)
{
	pe_ui_refresh(a);
	xuiLayout(a->ui);
}
static xui_widget find_type(xui_widget root, xui_widget_type type)
{
	if (xuiWidgetGetType(root) == type)
		return root;
	for (xui_widget child = xuiWidgetGetFirstChild(root); child; child = xuiWidgetGetNextSibling(child))
	{
		xui_widget match = find_type(child, type);
		if (match)
			return match;
	}
	return NULL;
}
static uint64_t particle_hash;
static int hash_particle(const xge_particle_snapshot_t *particle, void *user)
{
	(void)user;
	const unsigned char *bytes = (const unsigned char *)&particle->tPosition;
	for (size_t i = 0; i < sizeof(particle->tPosition); ++i)
	{
		particle_hash ^= bytes[i];
		particle_hash *= UINT64_C(1099511628211);
	}
	return 1;
}
int pe_exercise(pe_app *a)
{
	pe_document *saved = malloc(sizeof(*saved));
	int cursor, property;
	xui_rect_t r;
	pe_data data;
	uint64_t first, second;
	if (!saved)
		return 0;
	*saved = *a->doc;
	pe_doc_init(a->doc);
	a->preview.playing = 0;
	a->preview.target_tick = 0;
	a->preview.solo = -1;
	memset(a->preview.muted, 0, sizeof(a->preview.muted));
	sync(a);
	CHECK(xuiDockPanelGetWindowCount(a->dock) == PE_WIN_COUNT);
	CHECK(xuiPropertyGridGetPropertyCount(a->properties) == pe_field_count);
	CHECK(xuiTimeLineViewGetLayerCount(a->timeline) == 1);
	CHECK(button_click(a, PE_DUPLICATE) == XUI_OK);
	sync(a);
	CHECK(a->doc->data.count == 2 && a->doc->selected == 1);
	CHECK(button_click(a, PE_UNDO) == XUI_OK);
	sync(a);
	CHECK(a->doc->data.count == 1);
	CHECK(button_click(a, PE_REDO) == XUI_OK);
	sync(a);
	CHECK(a->doc->data.count == 2);
	property = xuiPropertyGridFindProperty(a->properties, "fRate");
	CHECK(property >= 0);
	CHECK(xuiPropertyGridBeginEdit(a->properties, property) == 1);
	xui_widget editor = xuiGetFocusWidget(a->ui);
	CHECK(editor != NULL);
	CHECK(xuiInputSetText(editor, "137") == XUI_OK);
	CHECK(xuiPropertyGridEndEdit(a->properties, 1) == 1);
	CHECK(a->doc->data.emitters[1].fRate == 137);
	sync(a);
	/* A pending edit is committed to its original emitter before selection changes. */
	CHECK(xuiPropertyGridBeginEdit(a->properties, property) == 1);
	editor = xuiGetFocusWidget(a->ui);
	CHECK(xuiInputSetText(editor, "151") == XUI_OK);
	cursor = a->doc->cursor;
	CHECK(xuiInputKeyDown(a->ui, 'Z', XUI_MOD_CTRL) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(a->ui) == XUI_OK);
	CHECK(xuiInputKeyUp(a->ui, 'Z', 0) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(a->ui) == XUI_OK);
	CHECK(a->doc->cursor == cursor);
	CHECK(xuiInputSetText(editor, "151") == XUI_OK);
	pe_select(a, 0);
	CHECK(a->doc->data.emitters[1].fRate == 151);
	CHECK(a->doc->data.emitters[0].fRate == 90);
	pe_select(a, 1);
	sync(a);
	/* Native enum editing must commit numeric values, not translated labels. */
	xuiPropertyGridSetCategoryExpanded(a->properties, 1, 1);
	xuiLayout(a->ui);
	property = xuiPropertyGridFindProperty(a->properties, "iShape");
	CHECK(xuiPropertyGridBeginEdit(a->properties, property) == 1);
	xui_widget combo = find_type(xuiPropertyGridGetTableGrid(a->properties), xuiComboBoxGetType(a->ui));
	CHECK(combo != NULL);
	CHECK(xuiComboBoxSetSelectedValue(combo, 2) == XUI_OK);
	CHECK(xuiPropertyGridEndEdit(a->properties, 1) == 1);
	CHECK(a->doc->data.emitters[1].iShape == 2);
	sync(a);
	CHECK(!pe_set_property(a, "tLife.fMin", "99999"));
	CHECK(a->doc->data.emitters[1].tLife.fMin == 1);
	CHECK(pe_set_property(a, "fDirection", "90"));
	CHECK(fabsf(a->doc->data.emitters[1].fDirection - 1.570796327f) < .00001f);
	sync(a);
	CHECK(button_click(a, PE_MUTE) == XUI_OK);
	CHECK(a->preview.muted[1]);
	CHECK(button_click(a, PE_MUTE) == XUI_OK);
	CHECK(!a->preview.muted[1]);
	sync(a);
	CHECK(pe_command(a, PE_SHOW_CURVES));
	a->channel = 2;
	a->key = 0;
	sync(a);
	r = xuiWidgetGetWorldRect(a->curve);
	CHECK(r.fW > 100 && r.fH > 80);
	float gx = r.fX + 55, gy = r.fY + 20, gw = r.fW - 78, gh = r.fH - 54;
	float y = gy + gh * (1 - (1 - a->curve_min) / (a->curve_max - a->curve_min));
	cursor = a->doc->cursor;
	CHECK(drag(a, gx, y, gw * .16f, gh * .12f) == XUI_OK);
	CHECK(a->doc->cursor == cursor + 1);
	CHECK(a->doc->data.emitters[1].tAlphaOverLife.arrKeys[0].fTime > .1f);
	CHECK(!a->curve_drag);
	CHECK(pe_command(a, PE_UNDO));
	sync(a);
	CHECK(a->doc->data.emitters[1].tAlphaOverLife.arrKeys[0].fTime == 0);
	a->channel = 3;
	CHECK(pe_command(a, PE_CURVE_FADE));
	sync(a);
	CHECK(a->doc->data.emitters[1].tColorOverLife.iCount == 2);
	CHECK(xuiPropertyGridFindProperty(a->key_grid, "color") >= 0);
	data = a->doc->data;
	data.emitters[1].fDelay = .5f;
	data.emitters[1].fDuration = 2;
	data.emitters[1].iBurstCount = 1;
	data.emitters[1].arrBursts[0] = (xge_particle_burst_t){.25f, 20};
	CHECK(pe_doc_commit(a->doc, &data, 1, "test timeline"));
	xuiTimeLineViewSetFrameWidth(a->timeline, 2);
	xuiDockPanelSetPaneActiveWindow(a->dock, a->bottom_pane, a->windows[PE_WIN_TIMELINE]);
	sync(a);
	r = xuiWidgetGetWorldRect(a->timeline);
	float header, fw, row, ruler, ox, oy;
	xuiTimeLineViewGetMetrics(a->timeline, &header, &fw, &row, &ruler);
	xuiTimeLineViewGetOffset(a->timeline, &ox, &oy);
	cursor = a->doc->cursor;
	CHECK(drag(a, r.fX + header + 60 * fw - ox, r.fY + ruler + row * 1.5f - oy, 30 * fw, 0) == XUI_OK);
	CHECK(a->doc->cursor == cursor + 1);
	CHECK(fabsf(a->doc->data.emitters[1].fDelay - 1) < .025f);
	sync(a);
	/* Burst glyph and resize edge each form one document transaction. */
	cursor = a->doc->cursor;
	CHECK(drag(a, r.fX + header + 75 * fw - ox, r.fY + ruler + row * 1.5f - oy, 15 * fw, 0) == XUI_OK);
	CHECK(a->doc->cursor == cursor + 1);
	CHECK(fabsf(a->doc->data.emitters[1].arrBursts[0].fTime - .5f) < .03f);
	sync(a);
	cursor = a->doc->cursor;
	CHECK(drag(a, r.fX + header + 180 * fw - ox - 2, r.fY + ruler + row * 1.5f - oy, 30 * fw, 0) == XUI_OK);
	CHECK(a->doc->cursor == cursor + 1);
	CHECK(a->doc->data.emitters[1].fDuration > 2.4f);
	sync(a);
	/* Two real pointer clicks add at the clicked frame, never silently clamped. */
	float tx = r.fX + header + 132 * fw - ox, ty = r.fY + ruler + row * 1.5f - oy;
	cursor = a->doc->cursor;
	CHECK(input_click(a, tx, ty) == XUI_OK);
	CHECK(input_click(a, tx, ty) == XUI_OK);
	CHECK(!a->timeline_drag && a->doc->cursor == cursor + 1);
	CHECK(a->doc->data.emitters[1].iBurstCount == 2);
	CHECK(fabsf(a->doc->data.emitters[1].fDelay + a->doc->data.emitters[1].arrBursts[1].fTime - 2.2f) < .02f);
	sync(a);
	tx = r.fX + header + 252 * fw - ox;
	cursor = a->doc->cursor;
	CHECK(input_click(a, tx, ty) == XUI_OK);
	CHECK(input_click(a, tx, ty) == XUI_OK);
	CHECK(a->doc->cursor == cursor && a->doc->data.emitters[1].iBurstCount == 2);
	CHECK(strstr(a->status_text, "区间外") != NULL);
	/* An existing glyph opens its row instead of adding a duplicate. */
	sync(a);
	tx = r.fX + header + 90 * fw - ox;
	CHECK(input_click(a, tx, ty) == XUI_OK);
	CHECK(input_click(a, tx, ty) == XUI_OK);
	CHECK(!a->timeline_drag && a->burst == 0 && a->doc->cursor == cursor);
	sync(a);
	CHECK(xuiWidgetGetVisible(a->panels[PE_WIN_BURSTS]));
	CHECK(xuiPropertyGridGetSelected(a->bursts) == 0);
	CHECK(pe_command(a, PE_SHOW_BURSTS));
	sync(a);
	property = xuiPropertyGridFindProperty(a->bursts, "burst0n");
	CHECK(xuiPropertyGridBeginEdit(a->bursts, property) == 1);
	editor = xuiGetFocusWidget(a->ui);
	CHECK(xuiInputSetText(editor, "37") == XUI_OK);
	CHECK(xuiPropertyGridEndEdit(a->bursts, 1) == 1);
	CHECK(a->doc->data.emitters[1].arrBursts[0].iCount == 37);
	xuiDockPanelSetPaneActiveWindow(a->dock, a->bottom_pane, a->windows[PE_WIN_TIMELINE]);
	sync(a);
	/* Scrubbing the ruler uses the native XUI callback and pauses playback. */
	CHECK(input_click(a, r.fX + header + 42 * fw - ox, r.fY + 8) == XUI_OK);
	CHECK(!a->preview.playing && abs(a->preview.target_tick - 84) <= 2);
	a->preview.target_tick = 0;
	CHECK(pe_preview_rebuild(&a->preview, a->doc, 0) == XGE_OK);
	CHECK(pe_preview_seek(&a->preview, 120) == XGE_OK);
	for (int i = 0; i < 100 && a->preview.tick != 120; ++i)
		CHECK(pe_preview_update(&a->preview, 0) == XGE_OK);
	CHECK(a->preview.tick == 120);
	particle_hash = UINT64_C(14695981039346656037);
	xgeParticleVisit(a->preview.world, hash_particle, NULL);
	first = particle_hash;
	CHECK(pe_preview_seek(&a->preview, 0) == XGE_OK);
	CHECK(pe_preview_seek(&a->preview, 120) == XGE_OK);
	for (int i = 0; i < 100 && a->preview.tick != 120; ++i)
		CHECK(pe_preview_update(&a->preview, 0) == XGE_OK);
	particle_hash = UINT64_C(14695981039346656037);
	xgeParticleVisit(a->preview.world, hash_particle, NULL);
	second = particle_hash;
	CHECK(first == second);
	CHECK(xgeParticleStats(a->preview.world).iLiveParticles > 0);
	for (int i = 0; i < 24; ++i)
		CHECK(pe_preview_rebuild(&a->preview, a->doc, 0) == XGE_OK);
	CHECK(xgeParticleStats(a->preview.world).iCachedEffects == 0);
	data = a->doc->data;
	strcpy(a->doc->data.emitters[0].sTexture, "missing-preview-image.png");
	strcpy(a->doc->data.emitters[0].sMaterial, "project-specific-material");
	CHECK(pe_preview_rebuild(&a->preview, a->doc, 0) == XGE_OK);
	CHECK(a->preview.unresolved == 2);
	CHECK(!strcmp(a->doc->data.emitters[0].sMaterial, "project-specific-material"));
	a->doc->data = data;
	a->doc->data.emitters[0].iMaxParticles = PE_PREVIEW_CAPACITY + 1;
	CHECK(pe_preview_rebuild(&a->preview, a->doc, 0) == XGE_ERROR_BUFFER_TOO_SMALL);
	CHECK(a->preview.world == NULL);
	CHECK(a->doc->data.emitters[0].iMaxParticles == PE_PREVIEW_CAPACITY + 1);
	a->doc->data = data;
	xvalue *layout = NULL;
	CHECK(xuiDockPanelSaveState(a->dock, &layout) == XUI_OK);
	CHECK(xuiDockPanelFloatWindow(a->dock, a->windows[PE_WIN_CURVES], (xui_rect_t){150, 160, 750, 370}) ==
	      XUI_OK);
	CHECK(xuiDockPanelLoadState(a->dock, layout) == XUI_OK);
	xuiDockPanelStateFree(layout);
	*a->doc = *saved;
	free(saved);
	a->channel = 0;
	a->key = 0;
	a->preview.target_tick = 0;
	a->preview.playing = 1;
	pe_changed(a);
	pe_status(a, "交互回归通过：属性、Dock、时间轴、曲线、撤销、确定性回放");
	printf("particleedit UI checks=%d passed\n", checks);
	return 1;
failed:
	*a->doc = *saved;
	free(saved);
	return 0;
}
