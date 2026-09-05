#include "pe_app.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static float clamp(float v, float min, float max)
{
	return fminf(max, fmaxf(min, v));
}
static xui_rect_t graph_rect(xui_widget w)
{
	xui_rect_t r = xuiWidgetGetRect(w);
	return (xui_rect_t){55, 20, fmaxf(1, r.fW - 78), fmaxf(1, r.fH - 54)};
}
static int key_count(pe_app *a)
{
	return a->channel == 3 ? a->doc->data.emitters[a->doc->selected].tColorOverLife.iCount
	                       : pe_curve(&a->doc->data, a->doc->selected, a->channel)->iCount;
}
static float key_time(pe_app *a, int i)
{
	return a->channel == 3 ? a->doc->data.emitters[a->doc->selected].tColorOverLife.arrKeys[i].fTime
	                       : pe_curve(&a->doc->data, a->doc->selected, a->channel)->arrKeys[i].fTime;
}
void pe_curve_range(pe_app *a)
{
	xge_particle_curve_t *c = pe_curve(&a->doc->data, a->doc->selected, a->channel);
	float min = 0, max = 1;
	if (a->curve_drag)
		return;
	for (int i = 0; i <= 128; ++i)
	{
		float v = xgeParticleCurveEval(c, i / 128.f, 1);
		min = fminf(min, v);
		max = fmaxf(max, v);
	}
	for (int i = 0; i < c->iCount; ++i)
	{
		min = fminf(min, c->arrKeys[i].fValue);
		max = fmaxf(max, c->arrKeys[i].fValue);
	}
	float pad = (max - min) * .1f;
	a->curve_min = min - pad;
	a->curve_max = max + pad;
}
static float graph_y(pe_app *a, xui_rect_t r, float value)
{
	return r.fY + r.fH * (1 - (value - a->curve_min) / (a->curve_max - a->curve_min));
}
int pe_curve_paint(xui_widget w, xui_draw_context draw, uint32_t state, void *user)
{
	pe_app *a = user;
	xui_proxy p = &a->proxy;
	xui_rect_t size = xuiWidgetGetRect(w), r = graph_rect(w);
	xge_particle_curve_t *c = pe_curve(&a->doc->data, a->doc->selected, a->channel);
	xge_particle_gradient_t *g = &a->doc->data.emitters[a->doc->selected].tColorOverLife;
	char text[64];
	(void)state;
	p->drawRectFill(p, draw, (xui_rect_t){0, 0, size.fW, size.fH}, PE_BG);
	p->drawRectFill(p, draw, r, XUI_COLOR_RGBA(16, 22, 33, 255));
	for (int i = 0; i <= 4; ++i)
	{
		float x = r.fX + r.fW * i / 4.f, y = r.fY + r.fH * i / 4.f;
		p->drawLine(p, draw, x, r.fY, x, r.fY + r.fH, 1, PE_LINE);
		p->drawLine(p, draw, r.fX, y, r.fX + r.fW, y, 1, PE_LINE);
		snprintf(text, sizeof(text), "%.2g", i / 4.f);
		p->drawText(p, draw, a->font, text, (xui_rect_t){x - 16, r.fY + r.fH + 6, 46, 24}, PE_MUTED, 0);
		if (a->channel != 3)
		{
			snprintf(text, sizeof(text), "%.2g", a->curve_max - (a->curve_max - a->curve_min) * i / 4.f);
			p->drawText(p, draw, a->font, text, (xui_rect_t){0, y - 10, 50, 24}, PE_MUTED,
			            XUI_TEXT_ALIGN_RIGHT);
		}
	}
	if (a->channel == 3)
	{
		xui_rect_t bar = {r.fX, r.fY + r.fH * .2f, r.fW, r.fH * .6f};
		for (int y = 0; y < (int)bar.fH; y += 12)
			for (int x = 0; x < (int)bar.fW; x += 12)
				p->drawRectFill(
				    p, draw,
				    (xui_rect_t){bar.fX + x, bar.fY + y, fminf(12, bar.fW - x), fminf(12, bar.fH - y)},
				    ((x / 12 + y / 12) & 1) ? PE_LINE : PE_PANEL);
		for (int i = 0; i < bar.fW; ++i)
			p->drawRectFill(p, draw, (xui_rect_t){bar.fX + i, bar.fY, 1, bar.fH},
			                xgeParticleGradientEval(g, i / fmaxf(1, bar.fW - 1), 0xffffffffu));
		for (int i = 0; i < g->iCount; ++i)
		{
			float x = r.fX + g->arrKeys[i].fTime * r.fW, y = bar.fY + bar.fH;
			p->drawTriangleFill(p, draw, (xui_vec2_t){x, y}, (xui_vec2_t){x - 7, y + 12},
			                    (xui_vec2_t){x + 7, y + 12}, i == a->key ? PE_ACCENT : PE_TEXT);
			p->drawLine(p, draw, x, bar.fY, x, y, 1, i == a->key ? PE_ACCENT : PE_MUTED);
		}
	}
	else
	{
		float last = xgeParticleCurveEval(c, 0, 1);
		for (int i = 1; i <= 256; ++i)
		{
			float next = xgeParticleCurveEval(c, i / 256.f, 1);
			p->drawLine(p, draw, r.fX + r.fW * (i - 1) / 256.f, graph_y(a, r, last), r.fX + r.fW * i / 256.f,
			            graph_y(a, r, next), 2, PE_ACCENT);
			last = next;
		}
		for (int i = 0; i < c->iCount; ++i)
		{
			float x = r.fX + c->arrKeys[i].fTime * r.fW, y = graph_y(a, r, c->arrKeys[i].fValue);
			p->drawCircleFill(p, draw, x, y, i == a->key ? 6 : 4, i == a->key ? PE_TEXT : PE_ACCENT);
			if (i == a->key && c->iInterpolation == XGE_PARTICLE_CURVE_HERMITE)
			{
				float dx = .08f * r.fW;
				float dy_in = c->arrKeys[i].fInTangent * .08f * r.fH / (a->curve_max - a->curve_min);
				float dy_out = c->arrKeys[i].fOutTangent * .08f * r.fH / (a->curve_max - a->curve_min);
				if (i > 0)
					p->drawLine(p, draw, x, y, clamp(x - dx, r.fX, r.fX + r.fW),
					            clamp(y + dy_in, r.fY, r.fY + r.fH), 1, PE_MUTED);
				if (i + 1 < c->iCount)
					p->drawLine(p, draw, x, y, clamp(x + dx, r.fX, r.fX + r.fW),
					            clamp(y - dy_out, r.fY, r.fY + r.fH), 1, PE_MUTED);
			}
		}
	}
	if (!key_count(a))
		p->drawText(p, draw, a->font, "空轨道：默认值 1 / 白色；双击添加关键点",
		            (xui_rect_t){r.fX + 8, r.fY + 8, r.fW - 16, 30}, PE_TEXT, 0);
	return XUI_OK;
}
static void curve_finish(pe_app *a, int commit)
{
	pe_data edited = a->doc->data;
	a->doc->data = a->drag_data;
	a->curve_drag = 0;
	if (commit && !pe_doc_commit(a->doc, &edited, a->doc->selected, "拖动生命周期关键点"))
		pe_status(a, a->doc->error);
	pe_changed(a);
}
int pe_curve_event(xui_widget w, const xui_event_t *e, void *user)
{
	pe_app *a = user;
	xui_rect_t world = xuiWidgetGetWorldRect(w), r = graph_rect(w);
	float x = e->fX - world.fX, y = e->fY - world.fY, time = clamp((x - r.fX) / r.fW, 0, 1);
	float value = a->curve_max - (y - r.fY) / r.fH * (a->curve_max - a->curve_min);
	int count = key_count(a);
	if (e->iPhase == XUI_EVENT_PHASE_CAPTURE)
		return XUI_OK;
	if (e->iType == XUI_EVENT_KEY_DOWN)
	{
		if (e->iKey == XUI_KEY_ESCAPE && a->curve_drag)
		{
			curve_finish(a, 0);
			xuiReleasePointerCapture(a->ui, w);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if (e->iKey == XUI_KEY_DELETE && !a->curve_drag)
		{
			pe_command(a, PE_CURVE_REMOVE);
			return XUI_EVENT_DISPATCH_STOP;
		}
		return XUI_OK;
	}
	if (e->iType == XUI_EVENT_POINTER_DOUBLE_CLICK && e->iButton == XUI_POINTER_BUTTON_LEFT)
	{
		if (x < r.fX || x > r.fX + r.fW || y < r.fY || y > r.fY + r.fH)
			return XUI_OK;
		for (int i = 0; i < count; ++i)
			if (fabsf(key_time(a, i) - time) * r.fW < 9)
				return XUI_EVENT_DISPATCH_STOP;
		if (pe_key_add(a->doc, a->channel, time, value))
		{
			a->key = 0;
			for (int i = 0; i < key_count(a); ++i)
				if (fabsf(key_time(a, i) - time) < .00001f)
					a->key = i;
			pe_changed(a);
		}
		else
			pe_status(a, a->doc->error);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_DOWN && e->iButton == XUI_POINTER_BUTTON_LEFT)
	{
		if (!pe_finish_edit(a))
			return XUI_EVENT_DISPATCH_STOP;
		xuiSetFocusWidget(a->ui, w);
		float best = 13;
		int selected = -1;
		for (int i = 0; i < count; ++i)
		{
			float px = r.fX + key_time(a, i) * r.fW,
			      py =
			          a->channel == 3
			              ? r.fY + r.fH * .8f + 7
			              : graph_y(a, r,
			                        pe_curve(&a->doc->data, a->doc->selected, a->channel)->arrKeys[i].fValue);
			float distance = a->channel == 3 ? fabsf(x - px) : hypotf(x - px, y - py);
			if (distance < best)
			{
				best = distance;
				selected = i;
			}
		}
		if (selected < 0)
			return XUI_OK;
		a->key = selected;
		a->drag_data = a->doc->data;
		a->curve_drag = 1;
		a->drag_min = a->curve_min;
		a->drag_max = a->curve_max;
		a->syncing = 1;
		pe_key_grid_sync(a);
		a->syncing = 0;
		xuiSetPointerCapture(a->ui, w);
		xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_MOVE && a->curve_drag)
	{
		pe_data data = a->doc->data;
		float low = a->key > 0 ? key_time(a, a->key - 1) + .0001f : 0;
		float high = a->key + 1 < count ? key_time(a, a->key + 1) - .0001f : 1;
		if (low > high)
			return XUI_EVENT_DISPATCH_STOP;
		time = clamp(time, low, high);
		if (a->channel == 3)
			data.emitters[a->doc->selected].tColorOverLife.arrKeys[a->key].fTime = time;
		else
		{
			xge_particle_key_t *key = &pe_curve(&data, a->doc->selected, a->channel)->arrKeys[a->key];
			key->fTime = time;
			key->fValue = clamp(value, a->drag_min, a->drag_max);
		}
		if (pe_data_validate(&data, NULL, 0))
		{
			a->doc->data = data;
			a->rebuild = 1;
		}
		xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_UP && a->curve_drag)
	{
		curve_finish(a, 1);
		xuiReleasePointerCapture(a->ui, w);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_CAPTURE_LOST && a->curve_drag)
	{
		curve_finish(a, 0);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}
static int frame_of(float seconds, int limit)
{
	return (int)clamp(roundf(seconds * 60), 0, (float)limit);
}
void pe_timeline_sync(pe_app *a)
{
	int frames = (int)(a->preview.range_seconds * 60) + 1;
	float ox, oy;
	xuiTimeLineViewGetOffset(a->timeline, &ox, &oy);
	xuiTimeLineViewClear(a->timeline);
	xuiTimeLineViewSetFrameCount(a->timeline, frames);
	for (int i = 0; i < a->doc->data.count; ++i)
	{
		xge_particle_emitter_t *e = &a->doc->data.emitters[i];
		char label[100];
		int layer, span, start = frame_of(e->fDelay, frames - 1);
		int end = e->fDuration > 0 ? frame_of(e->fDelay + e->fDuration, frames) : frames;
		snprintf(label, sizeof(label), "%d  %s", i, e->sName[0] ? e->sName : "(未命名)");
		xuiTimeLineViewAddLayer(a->timeline, label, &layer);
		xuiTimeLineViewSetLayerVisible(a->timeline, layer, !a->preview.muted[i]);
		xuiTimeLineViewSetLayerColor(a->timeline, layer, i == a->doc->selected ? PE_ACCENT : PE_MUTED);
		a->span_ids[i] = -1;
		if (e->bAutomatic && e->fDelay > a->preview.range_seconds)
			continue;
		if (!e->bAutomatic)
		{
			start = 0;
			end = frames - 1;
		}
		snprintf(label, sizeof(label), "%s%s",
		         !e->bAutomatic      ? "事件触发（无固定时段）"
		         : e->fDuration == 0 ? "持续发射"
		         : e->bLoop          ? "循环 · 第一轮"
		                             : "单次",
		         a->preview.solo == i ? " [S]" : "");
		if (end <= start)
			end = start + 1;
		xuiTimeLineViewAddSpan(a->timeline, layer, start, end, XUI_TIMELINE_SPAN_CUSTOM, label, &span);
		a->span_ids[i] = span;
		xuiTimeLineViewSetSpanColor(a->timeline, span,
		                            i == a->doc->selected ? PE_SELECTED : XUI_COLOR_RGBA(57, 69, 93, 255));
		xuiTimeLineViewSetSpanUserData(a->timeline, span, (void *)(intptr_t)(i + 1));
		for (int j = 0; e->bAutomatic && j < e->iBurstCount; ++j)
		{
			float time = e->fDelay + e->arrBursts[j].fTime;
			if (time <= a->preview.range_seconds)
				xuiTimeLineViewSetFrame(a->timeline, layer, frame_of(time, frames - 1),
				                        XUI_TIMELINE_FRAME_KEY, (void *)(intptr_t)(j + 1));
		}
	}
	xuiTimeLineViewSetCurrentFrame(a->timeline, a->preview.target_tick / 2);
	xuiTimeLineViewSetOffset(a->timeline, ox, oy);
}
static void timeline_finish(pe_app *a, int commit)
{
	pe_data edited = a->doc->data;
	a->doc->data = a->drag_data;
	a->timeline_drag = 0;
	if (commit && !pe_doc_commit(a->doc, &edited, a->doc->selected,
	                             a->drag_burst >= 0  ? "拖动 Burst"
	                             : a->drag_mode == 2 ? "调整持续时间"
	                                                 : "移动发射起点"))
		pe_status(a, a->doc->error);
	pe_changed(a);
}
int pe_timeline_event(xui_widget w, const xui_event_t *e, void *user)
{
	pe_app *a = user;
	xui_rect_t world = xuiWidgetGetWorldRect(a->timeline);
	float header, fw, row, ruler, ox, oy;
	xuiTimeLineViewGetMetrics(a->timeline, &header, &fw, &row, &ruler);
	xuiTimeLineViewGetOffset(a->timeline, &ox, &oy);
	int frame = (int)floorf((e->fX - world.fX - header + ox) / fw);
	if (e->iType == XUI_EVENT_KEY_DOWN && e->iKey == XUI_KEY_ESCAPE && a->timeline_drag)
	{
		timeline_finish(a, 0);
		xuiReleasePointerCapture(a->ui, w);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_CAPTURE_LOST && a->timeline_drag)
	{
		timeline_finish(a, 0);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iPhase != XUI_EVENT_PHASE_CAPTURE && e->pTarget != w)
		return XUI_OK;
	if (e->iType == XUI_EVENT_POINTER_DOUBLE_CLICK && e->iButton == XUI_POINTER_BUTTON_LEFT)
	{
		xui_timeline_hit_t hit = {0};
		xuiTimeLineViewHitTest(a->timeline, e->fX - world.fX, e->fY - world.fY, &hit);
		if (hit.iLayer < 0 || hit.iLayer >= a->doc->data.count || hit.iFrame < 0)
			return XUI_OK;
		/* XUI emits double-click after the second down, which may have begun a bar drag. */
		if (a->timeline_drag)
		{
			timeline_finish(a, 0);
			xuiReleasePointerCapture(a->ui, w);
		}
		pe_select(a, hit.iLayer);
		if (a->doc->selected != hit.iLayer || !pe_finish_edit(a))
			return XUI_EVENT_DISPATCH_STOP;
		xge_particle_emitter_t *emitter = &a->doc->data.emitters[hit.iLayer];
		if (!emitter->bAutomatic)
		{
			pe_command(a, PE_SHOW_BURSTS);
			pe_status(a, "事件发射器没有固定时段，请在 Burst 表中编辑相对触发时间");
			return XUI_EVENT_DISPATCH_STOP;
		}
		for (int i = 0; i < emitter->iBurstCount; ++i)
			if (fabsf((emitter->fDelay + emitter->arrBursts[i].fTime) * 60 - frame) * fw <= 7)
			{
				a->burst = i;
				a->refresh = 1;
				pe_command(a, PE_SHOW_BURSTS);
				return XUI_EVENT_DISPATCH_STOP;
			}
		float time = frame / 60.f - emitter->fDelay;
		if (time < 0 || (emitter->fDuration > 0 && time >= emitter->fDuration))
		{
			pe_status(a, "此帧位于发射区间外；请先调整启动延迟或持续时间，再添加 Burst");
			return XUI_EVENT_DISPATCH_STOP;
		}
		a->preview.playing = 0;
		a->preview.target_tick = frame * (PE_HZ / 60);
		pe_command(a, PE_BURST_ADD);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_DOWN && e->iButton == XUI_POINTER_BUTTON_LEFT)
	{
		xui_timeline_hit_t hit = {0};
		if (e->fX - world.fX < header || e->fY - world.fY < ruler)
			return XUI_OK;
		xuiTimeLineViewHitTest(a->timeline, e->fX - world.fX, e->fY - world.fY, &hit);
		if (hit.iLayer < 0 || hit.iLayer >= a->doc->data.count)
			return XUI_OK;
		xge_particle_emitter_t *emitter = &a->doc->data.emitters[hit.iLayer];
		if (!emitter->bAutomatic)
		{
			pe_select(a, hit.iLayer);
			return XUI_OK;
		}
		a->drag_burst = -1;
		for (int i = 0; i < emitter->iBurstCount; ++i)
			if (fabsf((emitter->fDelay + emitter->arrBursts[i].fTime) * 60 - frame) * fw <= 7)
			{
				a->drag_burst = i;
				break;
			}
		if (a->drag_burst < 0 && hit.iSpanId < 0)
			return XUI_OK;
		pe_select(a, hit.iLayer);
		if (a->doc->selected != hit.iLayer || !pe_finish_edit(a))
			return XUI_EVENT_DISPATCH_STOP;
		a->refresh = 0;
		a->drag_data = a->doc->data;
		a->drag_start_frame = frame;
		a->drag_layer = hit.iLayer;
		a->drag_span = hit.iSpanId;
		a->drag_mode =
		    emitter->fDuration > 0 && fabsf((emitter->fDelay + emitter->fDuration) * 60 - frame) * fw < 8 ? 2
		                                                                                                  : 1;
		a->timeline_drag = 1;
		a->preview.playing = 0;
		a->burst = a->drag_burst;
		xuiSetFocusWidget(a->ui, w);
		xuiSetPointerCapture(a->ui, w);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_MOVE && a->timeline_drag)
	{
		pe_data data = a->drag_data;
		xge_particle_emitter_t *emitter = &data.emitters[a->drag_layer];
		float delta = (frame - a->drag_start_frame) / 60.f;
		if (a->drag_burst >= 0)
			emitter->arrBursts[a->drag_burst].fTime =
			    fmaxf(0, emitter->arrBursts[a->drag_burst].fTime + delta);
		else if (a->drag_mode == 2)
			emitter->fDuration = fmaxf(1 / 60.f, emitter->fDuration + delta);
		else
			emitter->fDelay = fmaxf(0, emitter->fDelay + delta);
		if (pe_data_validate(&data, NULL, 0))
		{
			a->doc->data = data;
			a->rebuild = 1;
			a->syncing = 1;
			pe_timeline_sync(a);
			a->syncing = 0;
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (e->iType == XUI_EVENT_POINTER_UP && a->timeline_drag)
	{
		timeline_finish(a, 1);
		xuiReleasePointerCapture(a->ui, w);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}
