#include "pe_app.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

static const char *preset_files[] = {"impact", "fire_smoke", "explosion", "weather",
                                     "dust",   "magic",      "confetti"};
static int init(pe_app *a);
void pe_status(pe_app *a, const char *text)
{
	snprintf(a->status_text, sizeof(a->status_text), "%s", text ? text : "");
	if (a->status)
		xuiLabelSetText(a->status, a->status_text);
	xgeRenderRequest();
}
void pe_changed(pe_app *a)
{
	a->refresh = a->rebuild = 1;
	xgeRenderRequest();
}
void pe_select(pe_app *a, int selected)
{
	if (selected < 0 || selected >= a->doc->data.count || selected == a->doc->selected)
		return;
	if (!pe_finish_edit(a))
		return;
	a->doc->selected = selected;
	a->key = 0;
	a->burst = 0;
	a->selection_changed = a->refresh = 1;
}
int pe_finish_edit(pe_app *a)
{
	xui_widget grids[] = {a->properties, a->bursts, a->key_grid, a->settings};
	for (int i = 0; i < 4; ++i)
		if (grids[i] && xuiPropertyGridIsEditing(grids[i]))
		{
			if (!xuiPropertyGridEndEdit(grids[i], 1) || xuiPropertyGridIsEditing(grids[i]))
			{
				pe_status(a, "请修正当前编辑值，或按 Esc 取消后再执行命令");
				return 0;
			}
		}
	return 1;
}
static int file_dialog(pe_app *a, int save, int texture, char *path)
{
	OPENFILENAMEW dialog = {0};
	wchar_t w[PE_PATH] = {0};
	if (save && a->doc->path[0])
		MultiByteToWideChar(CP_UTF8, 0, a->doc->path, -1, w, PE_PATH);
	dialog.lStructSize = sizeof(dialog);
	dialog.hwndOwner = (HWND)xgePlatformNativeHandle();
	dialog.lpstrFile = w;
	dialog.nMaxFile = PE_PATH;
	dialog.lpstrTitle = texture ? L"选择预览纹理" : save ? L"保存 XGE 粒子定义" : L"打开 XGE 粒子定义";
	dialog.lpstrFilter = texture ? L"图片 (*.png;*.jpg;*.bmp)\0*.png;*.jpg;*.bmp\0所有文件\0*.*\0\0"
	                             : L"XSON 粒子 (*.xson)\0*.xson\0JSON 粒子 (*.json)\0*.json\0\0";
	dialog.lpstrDefExt =
	    save ? (pe_path_is_xson(a->doc->path) || !a->doc->path[0] ? L"xson" : L"json") : NULL;
	dialog.nFilterIndex = save && a->doc->path[0] && !pe_path_is_xson(a->doc->path) ? 2 : 1;
	dialog.Flags =
	    OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
	int ok = save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
	if (!ok)
	{
		if (CommDlgExtendedError())
			pe_status(a, "文件对话框失败");
		return 0;
	}
	return WideCharToMultiByte(CP_UTF8, 0, w, -1, path, PE_PATH, NULL, NULL) > 0;
}
static int save_document(pe_app *a, int as)
{
	char path[PE_PATH];
	if (!pe_finish_edit(a))
		return 0;
	if (as || !a->doc->path[0])
	{
		if (!file_dialog(a, 1, 0, path))
			return 0;
	}
	else
		strcpy(path, a->doc->path);
	if (!pe_doc_save(a->doc, path, as || !a->doc->path[0]))
	{
		pe_status(a, a->doc->error);
		return 0;
	}
	pe_status(a, "保存成功：运行时可直接加载此 JSON / XSON，资源逻辑名原样保留");
	a->refresh = a->rebuild = 1;
	return 1;
}
static int guard(pe_app *a)
{
	if (!pe_finish_edit(a))
		return 0;
	if (!pe_doc_dirty(a->doc))
		return 1;
	int result = MessageBoxW((HWND)xgePlatformNativeHandle(),
	                         L"当前粒子定义尚未保存。\n\n是：保存并继续\n否：放弃修改\n取消：留在当前文档",
	                         L"XGE Particle Editor", MB_YESNOCANCEL | MB_ICONWARNING);
	return result == IDNO || (result == IDYES && save_document(a, 0));
}
static int quit_requested(void *user)
{
	return guard(user);
}
static void reset_preview_flags(pe_app *a)
{
	memset(a->preview.muted, 0, sizeof(a->preview.muted));
	a->preview.solo = -1;
	a->preview.target_tick = 0;
	a->selection_changed = 1;
	a->key = a->burst = 0;
}
static void show_panel(pe_app *a, int which)
{
	xui_dock_window_info_t info = {0};
	info.iSize = sizeof(info);
	if (xuiDockPanelGetWindowInfo(a->dock, a->windows[which], &info) == XUI_OK)
	{
		if (info.iPane >= 0)
			xuiDockPanelSetPaneActiveWindow(a->dock, info.iPane, a->windows[which]);
		if (info.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE)
			xuiDockPanelExpandAutoHideWindow(a->dock, a->windows[which]);
	}
}
int pe_command(pe_app *a, int command)
{
	pe_data data = a->doc->data;
	xge_particle_emitter_t *e = &data.emitters[a->doc->selected];
	int ok = 1;
	char path[PE_PATH];
	if (a->curve_drag || a->timeline_drag)
	{
		pe_status(a, "请先结束拖拽或按 Esc 取消");
		return 0;
	}
	if (!pe_finish_edit(a))
		return 0;
	/* EndEdit can commit a field; take the candidate only after that commit. */
	data = a->doc->data;
	e = &data.emitters[a->doc->selected];
	switch (command)
	{
	case PE_NEW:
		if (!guard(a))
			return 0;
		pe_doc_init(a->doc);
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_OPEN:
		if (!guard(a) || !file_dialog(a, 0, 0, path))
			return 0;
		if (!pe_doc_load(a->doc, path, 0))
		{
			pe_status(a, a->doc->error);
			return 0;
		}
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_SAVE:
		return save_document(a, 0);
	case PE_SAVE_AS:
		return save_document(a, 1);
	case PE_UNDO:
		ok = pe_doc_undo(a->doc);
		memset(a->preview.muted, 0, sizeof(a->preview.muted));
		a->preview.solo = -1;
		a->selection_changed = 1;
		pe_changed(a);
		break;
	case PE_REDO:
		ok = pe_doc_redo(a->doc);
		memset(a->preview.muted, 0, sizeof(a->preview.muted));
		a->preview.solo = -1;
		a->selection_changed = 1;
		pe_changed(a);
		break;
	case PE_RESET_LAYOUT:
		xuiDockPanelLoadState(a->dock, a->default_layout);
		a->layout_failed = 0;
		pe_layout_save(a);
		pe_status(a, "已恢复默认 Dock 布局");
		break;
	case PE_ADD:
		ok = pe_doc_add(a->doc, 0);
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_DUPLICATE:
		ok = pe_doc_add(a->doc, 1);
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_REMOVE:
		ok = pe_doc_remove(a->doc);
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_UP:
		ok = pe_doc_move(a->doc, a->doc->selected - 1);
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_DOWN:
		ok = pe_doc_move(a->doc, a->doc->selected + 1);
		reset_preview_flags(a);
		pe_changed(a);
		break;
	case PE_MUTE:
		a->preview.muted[a->doc->selected] = !a->preview.muted[a->doc->selected];
		pe_changed(a);
		break;
	case PE_SOLO:
		a->preview.solo = a->preview.solo == a->doc->selected ? -1 : a->doc->selected;
		pe_changed(a);
		break;
	case PE_PLAY:
		a->preview.playing = !a->preview.playing;
		if (a->preview.playing && a->preview.tick >= (int)(a->preview.range_seconds * PE_HZ))
			pe_preview_seek(&a->preview, 0);
		a->refresh = 1;
		break;
	case PE_RESTART:
		pe_preview_seek(&a->preview, 0);
		a->preview.target_tick = 0;
		pe_changed(a);
		break;
	case PE_STEP:
		a->preview.playing = 0;
		ok = pe_preview_seek(&a->preview, (int)fminf(a->preview.range_seconds * PE_HZ,
		                                             a->preview.target_tick + 2)) == XGE_OK;
		a->refresh = 1;
		break;
	case PE_EMIT: {
		uint32_t spawned = 0;
		int result = xgeParticleEmit(a->preview.world, a->preview.effect, a->doc->selected, 32, &spawned);
		if (result != XGE_OK)
		{
			pe_status(a, "手动发射失败：请先重播，或检查预览预算");
			return 0;
		}
		pe_status(a, "手动测试发射：此临时操作不写入资产，重播 / 回退 / 参数修改时清除");
		break;
	}
	case PE_FIT:
		a->zoom = 1;
		a->pan_x = a->pan_y = 0;
		break;
	case PE_BOUNDS:
		a->preview.bounds = !a->preview.bounds;
		a->refresh = 1;
		break;
	case PE_GROUND:
		a->preview.ground = !a->preview.ground;
		pe_changed(a);
		break;
	case PE_MOVE:
		a->preview.moving = !a->preview.moving;
		pe_changed(a);
		break;
	case PE_CURVE_ADD: {
		xge_particle_curve_t *c = pe_curve(&data, a->doc->selected, a->channel);
		int count = a->channel == 3 ? e->tColorOverLife.iCount : c->iCount;
		float best = -1, time = .5f, previous = 0;
		for (int i = 0; i <= count; ++i)
		{
			float next = i == count        ? 1
			             : a->channel == 3 ? e->tColorOverLife.arrKeys[i].fTime
			                               : c->arrKeys[i].fTime;
			if (next - previous > best)
			{
				best = next - previous;
				time = (next + previous) * .5f;
			}
			previous = next;
		}
		ok = pe_key_add(a->doc, a->channel, time, xgeParticleCurveEval(c, time, 1));
		pe_changed(a);
		break;
	}
	case PE_CURVE_REMOVE:
		ok = pe_key_remove(a->doc, a->channel, a->key);
		pe_changed(a);
		break;
	case PE_CURVE_RESET:
		if (a->channel == 3)
			memset(&e->tColorOverLife, 0, sizeof(e->tColorOverLife));
		else
			memset(pe_curve(&data, a->doc->selected, a->channel), 0, sizeof(xge_particle_curve_t));
		ok = pe_doc_commit(a->doc, &data, a->doc->selected, "清空生命周期轨道");
		pe_changed(a);
		break;
	case PE_CURVE_FADE:
		if (a->channel == 3)
		{
			e->tColorOverLife = (xge_particle_gradient_t){0};
			e->tColorOverLife.iCount = 2;
			e->tColorOverLife.arrKeys[0] = (xge_particle_color_key_t){0, 0xffffffffu};
			e->tColorOverLife.arrKeys[1] = (xge_particle_color_key_t){1, XGE_COLOR_RGBA(255, 255, 255, 0)};
		}
		else
		{
			xge_particle_curve_t *c = pe_curve(&data, a->doc->selected, a->channel);
			memset(c, 0, sizeof(*c));
			c->iCount = 2;
			c->arrKeys[0] = (xge_particle_key_t){0, 1, 0, 0};
			c->arrKeys[1] = (xge_particle_key_t){1, 0, 0, 0};
		}
		ok = pe_doc_commit(a->doc, &data, a->doc->selected, "应用淡出轨道");
		a->key = 0;
		pe_changed(a);
		break;
	case PE_BURST_ADD:
		if (e->iBurstCount == XGE_PARTICLE_MAX_BURSTS)
		{
			pe_status(a, "最多 16 个 Burst");
			return 0;
		}
		float time = fmaxf(0, a->preview.target_tick / (float)PE_HZ - e->fDelay);
		if (e->fDuration > 0 && time >= e->fDuration)
			time = fmaxf(0, e->fDuration - .001f);
		a->burst = e->iBurstCount;
		e->arrBursts[e->iBurstCount++] = (xge_particle_burst_t){time, 32};
		ok = pe_doc_commit(a->doc, &data, a->doc->selected, "添加 Burst");
		pe_changed(a);
		break;
	case PE_BURST_REMOVE:
		if (a->burst < 0 || a->burst >= e->iBurstCount)
			return 0;
		memmove(e->arrBursts + a->burst, e->arrBursts + a->burst + 1,
		        (e->iBurstCount - a->burst - 1) * sizeof(e->arrBursts[0]));
		e->arrBursts[--e->iBurstCount] = (xge_particle_burst_t){0};
		ok = pe_doc_commit(a->doc, &data, a->doc->selected, "删除 Burst");
		pe_changed(a);
		break;
	case PE_TEXTURE: {
		if (!file_dialog(a, 0, 1, path))
			return 0;
		/* Runtime binding strings are capped at 127 UTF-8 bytes. Never truncate. */
		const char *chosen = path;
		char folder[PE_PATH];
		strcpy(folder, a->doc->path);
		char *slash = strrchr(folder, '\\');
		if (slash)
			slash[1] = 0;
		if (slash && !_strnicmp(folder, path, strlen(folder)))
			chosen = path + strlen(folder);
		if (strlen(chosen) >= sizeof(e->sTexture))
		{
			pe_status(a, "路径超过 127 字节；请将图片放入粒子文件目录后使用相对路径");
			return 0;
		}
		strcpy(e->sTexture, chosen);
		ok = pe_doc_commit(a->doc, &data, a->doc->selected, "绑定纹理");
		pe_changed(a);
		break;
	}
	case PE_RELOAD:
		pe_preview_reload_textures(&a->preview);
		pe_changed(a);
		break;
	case PE_HELP:
		MessageBoxW(
		    (HWND)xgePlatformNativeHandle(),
		    L"XGE 原生粒子编辑器\n\nCtrl+N/O/S：新建 / 打开 / 保存\nCtrl+Shift+S：另存为；Ctrl+Z/Y：撤销 / "
		    L"重做\n空格：播放 / "
		    L"暂停；F：适合窗口\n\nDock：拖动标题调整停靠、分组、浮动，布局自动保存。\n时间轴：拖动色条移动启"
		    L"动时间，右边缘改持续时间；菱形为 Burst。\n曲线：双击添加关键点，拖动编辑，Delete "
		    L"删除；右侧精确数值和切线。\n\n角度界面用度，文件用弧度。预览设置不写入资产。\n手动发射仅为临时"
		    L"测试；未解析材质保持原名并警告。",
		    L"操作说明", MB_OK | MB_ICONINFORMATION);
		break;
	case PE_TEMPLATE:
		if (!guard(a))
			return 0;
		if (snprintf(path, sizeof(path), "%s/%s.xson", a->preset_root, preset_files[a->preset]) >=
		    (int)sizeof(path))
			return 0;
		if (!pe_doc_load(a->doc, path, 1))
		{
			pe_status(a, a->doc->error);
			return 0;
		}
		reset_preview_flags(a);
		static const float origins[][2] = {{532, 405}, {500, 549}, {500, 395}, {500, 135},
		                                   {500, 430}, {500, 375}, {500, 535}};
		static const float durations[] = {1.5f, 10, 3, 10, 10, 10, 5};
		a->preview.origin_x = origins[a->preset][0];
		a->preview.origin_y = origins[a->preset][1];
		a->preview.range_seconds = durations[a->preset];
		a->preview.moving = a->preset == 4;
		a->preview.ground = a->preset == 0 || a->preset == 3;
		a->preview.playing = 1;
		pe_changed(a);
		break;
	case PE_SHOW_CURVES:
		show_panel(a, PE_WIN_CURVES);
		break;
	case PE_SHOW_BURSTS:
		show_panel(a, PE_WIN_BURSTS);
		break;
	default:
		return 0;
	}
	if (!ok && a->doc->error[0])
		pe_status(a, a->doc->error);
	xgeRenderRequest();
	return ok;
}
static int hotkey(xui_widget widget, const xui_event_t *e, void *user)
{
	pe_app *a = user;
	(void)widget;
	xui_widget focus = xuiGetFocusWidget(a->ui);
	int editing =
	    focus && (xuiWidgetGetType(focus) == xuiInputGetType(a->ui) ||
	              xuiPropertyGridIsEditing(a->properties) || xuiPropertyGridIsEditing(a->key_grid) ||
	              xuiPropertyGridIsEditing(a->bursts) || xuiPropertyGridIsEditing(a->settings));
	if (editing && e->iKey != 'S' && e->iKey != 's')
		return XUI_OK;
	int command = -1, key = e->iKey;
	if (e->iModifiers & XUI_MOD_CTRL)
	{
		if (key == 'N' || key == 'n')
			command = PE_NEW;
		else if (key == 'O' || key == 'o')
			command = PE_OPEN;
		else if (key == 'S' || key == 's')
			command = (e->iModifiers & XUI_MOD_SHIFT) ? PE_SAVE_AS : PE_SAVE;
		else if (key == 'Z' || key == 'z')
			command = (e->iModifiers & XUI_MOD_SHIFT) ? PE_REDO : PE_UNDO;
		else if (key == 'Y' || key == 'y')
			command = PE_REDO;
	}
	else if (key == XUI_KEY_SPACE)
		command = PE_PLAY;
	else if (key == 'F' || key == 'f')
		command = PE_FIT;
	if (command >= 0)
	{
		pe_command(a, command);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}
static void layout_init(pe_app *a)
{
	wchar_t base[PE_PATH], folder[PE_PATH];
	char *text = NULL;
	size_t size;
	if (a->frames || a->exercise)
		return; /* Scripted runs never overwrite the user's workspace. */
	DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", base, PE_PATH);
	if (!n || n + 40 >= PE_PATH)
		return;
	swprintf(folder, PE_PATH, L"%ls\\XGE", base);
	CreateDirectoryW(folder, NULL);
	swprintf(folder, PE_PATH, L"%ls\\XGE\\ParticleEdit", base);
	CreateDirectoryW(folder, NULL);
	swprintf(folder, PE_PATH, L"%ls\\XGE\\ParticleEdit\\layout.xson", base);
	WideCharToMultiByte(CP_UTF8, 0, folder, -1, a->layout_path, PE_PATH, NULL, NULL);
	if (pe_file_read(a->layout_path, &text, &size, NULL))
	{
		xvalue *state = xrtXsonParse((xstrview){text, size});
		if (!state || xuiDockPanelLoadState(a->dock, state) != XUI_OK)
		{
			xuiDockPanelLoadState(a->dock, a->default_layout);
			a->layout_failed = 1;
			pe_status(a, "布局文件无效，已使用默认布局；原文件保留，点重置布局后才覆盖");
		}
		if (state)
			xrtValueRelease(state);
		free(text);
	}
	else if (GetFileAttributesW(folder) != INVALID_FILE_ATTRIBUTES)
	{
		a->layout_failed = 1;
		pe_status(a, "布局文件不可读取，保留原文件");
	}
	a->layout_changes = xuiDockPanelGetLayoutChangeCount(a->dock);
}
void pe_layout_save(pe_app *a)
{
	xvalue *state = NULL;
	char *text;
	size_t size;
	if (!a->layout_path[0] || a->layout_failed)
		return;
	if (xuiDockPanelSaveState(a->dock, &state) != XUI_OK)
		return;
	text = xrtXsonStringify(state, 1, &size);
	xuiDockPanelStateFree(state);
	if (text)
	{
		if (!pe_file_write(a->layout_path, text, size))
			pe_status(a, "布局保存失败");
		xrtFree(text);
	}
}
static int capture(pe_app *a)
{
	size_t bytes = (size_t)a->width * a->height * 4;
	unsigned char *pixels = malloc(bytes);
	int result;
	if (!pixels)
		return XGE_ERROR_OUT_OF_MEMORY;
	result = a->proxy.surfaceReadRGBA(&a->proxy, a->target, pixels, a->width * 4);
	if (result == XGE_OK)
		result = xgeImageSavePNG(a->capture, a->width, a->height, pixels, a->width * 4);
	free(pixels);
	return result;
}
static int frame(void *user)
{
	pe_app *a = user;
	int result;
	if ((result = xgeBegin()) != XGE_OK)
		return result;
	if (!a->ui)
	{
		/* Sokol creates its native window and GL context only when xgeRun starts. */
		if ((result = init(a)) != XGE_OK)
		{
			fprintf(stderr, "particleedit initialize: %d\n", result);
			return result;
		}
		if (a->input[0])
		{
			if (!pe_doc_load(a->doc, a->input, 0))
			{
				fprintf(stderr, "%s\n", a->doc->error);
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		}
		else if (a->startup_preset >= 0)
		{
			a->preset = a->startup_preset;
			if (!pe_command(a, PE_TEMPLATE))
				return XGE_ERROR_INVALID_ARGUMENT;
		}
		a->refresh = a->rebuild = 1;
		if (a->startup_curve)
			show_panel(a, PE_WIN_CURVES);
	}
	if ((result = pe_ui_resize(a)) != XUI_OK)
		return result;
	if ((result = xuiProxyXgePumpInput(a->ui)) != XUI_OK)
		return result;
	if ((result = xuiDispatchPendingEvents(a->ui)) != XUI_OK)
		return result;
	if (a->refresh && !a->curve_drag && !a->timeline_drag)
		pe_ui_refresh(a);
	if ((result = xuiLayout(a->ui)) != XUI_OK)
		return result;
	if ((result = xuiUpdate(a->ui, xgeGetDelta())) != XUI_OK)
		return result;
	if (a->exercise && !a->tested && a->frame == 3)
	{
		a->tested = 1;
		if (!pe_exercise(a))
		{
			a->error = 1;
			fprintf(stderr, "particleedit UI exercise failed\n");
			xgeQuit();
			return XGE_ERROR_INVALID_STATE;
		}
		if (a->refresh)
			pe_ui_refresh(a);
		xuiLayout(a->ui);
	}
	if (a->rebuild)
	{
		pe_preview_rebuild(&a->preview, a->doc, 1);
		a->rebuild = 0;
	}
	if ((result = pe_preview_update(&a->preview, a->frames ? 1.0 / 60 : xgeGetDelta())) != XGE_OK)
		return result;
	a->syncing = 1;
	xuiTimeLineViewSetCurrentFrame(a->timeline, a->preview.target_tick / 2);
	a->syncing = 0;
	if ((result = pe_preview_draw(&a->preview, a->doc)) != XGE_OK)
		return result;
	xuiWidgetInvalidate(a->panels[PE_WIN_PREVIEW], XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	char title[PE_PATH + 100], status[1024];
	wchar_t wtitle[PE_PATH + 100];
	snprintf(title, sizeof(title), "%s%s — XGE Particle Editor", pe_doc_dirty(a->doc) ? "* " : "",
	         a->doc->path[0] ? a->doc->path : "未命名粒子");
	if (MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, PE_PATH + 100) > 0 && xgePlatformNativeHandle())
		SetWindowTextW((HWND)xgePlatformNativeHandle(), wtitle);
	xge_particle_stats_t stats = xgeParticleStats(a->preview.world);
	snprintf(status, sizeof(status), "%s%s  |  %d 发射器 · 峰值 %u · 丢弃 %llu · 碰撞 %llu  |  %s",
	         pe_doc_dirty(a->doc) ? "● " : "", a->doc->path[0] ? "已关联文件" : "新文档", a->doc->data.count,
	         stats.iPeakParticles, (unsigned long long)stats.iDroppedParticles,
	         (unsigned long long)stats.iCollisions, a->status_text);
	xuiLabelSetText(a->status, status);
	a->proxy.surfaceClear(&a->proxy, a->target, PE_BG);
	xui_rect_i_t full = {0, 0, a->width, a->height};
	if ((result = xuiRender(a->ui, a->target, &full, 1)) != XUI_OK)
		return result;
	xgeClear(PE_BG);
	xui_rect_t rect = {0, 0, (float)a->width, (float)a->height};
	if ((result = a->proxy.surfaceDraw(&a->proxy, a->target, rect, rect, 0xffffffffu,
	                                   XUI_SURFACE_DRAW_SCREEN_SPACE)) != XUI_OK)
		return result;
	if (a->frames && a->frame + 1 >= a->frames && a->capture[0])
	{
		if ((result = capture(a)) != XGE_OK)
			return result;
	}
	if ((result = xgeEnd()) != XGE_OK)
		return result;
	++a->frame;
	if (a->frames && a->frame >= a->frames)
	{
		printf("particleedit frames=%d emitters=%d live=%u peak=%u steps=%llu unresolved=%d tested=%d\n",
		       a->frame, a->doc->data.count, stats.iLiveParticles, stats.iPeakParticles,
		       (unsigned long long)stats.iSteps, a->preview.unresolved, a->tested);
		xgeQuit();
	}
	int changes = xuiDockPanelGetLayoutChangeCount(a->dock);
	if (changes != a->layout_changes)
	{
		a->layout_changes = changes;
		a->layout_save_at = xgeTimer() + .5;
	}
	if (a->layout_save_at > 0 && xgeTimer() >= a->layout_save_at)
	{
		pe_layout_save(a);
		a->layout_save_at = 0;
	}
	if (a->preview.playing || a->preview.tick != a->preview.target_tick || a->layout_save_at > 0 || a->frames)
		xgeRenderRequestAfter(1.0f / 60);
	return XGE_OK;
}
static int init(pe_app *a)
{
	int result;
	a->proxy = xuiProxyXge();
	if ((result = xuiCreate(&a->ui)) != XUI_OK)
		return result;
	if ((result = xuiSetProxy(a->ui, &a->proxy)) != XUI_OK)
		return result;
	wchar_t folder[PE_PATH];
	char font_path[PE_PATH];
	GetWindowsDirectoryW(folder, PE_PATH);
	size_t n = wcslen(folder);
	if (n + 30 < PE_PATH)
		wcscat(folder, L"\\Fonts\\msyh.ttc");
	WideCharToMultiByte(CP_UTF8, 0, folder, -1, font_path, PE_PATH, NULL, NULL);
	result = a->proxy.fontLoadFile(&a->proxy, &a->font, font_path, 17, 0);
	if (result != XUI_OK)
		return result;
	xuiSetDefaultFont(a->ui, a->font);
	if ((result = pe_preview_init(&a->preview)) != XGE_OK)
		return result;
	if ((result = pe_ui_create(a)) != XUI_OK)
		return result;
	if ((result = pe_ui_resize(a)) != XUI_OK)
		return result;
	/* Creation can precede the first native resize; arrange a real root immediately. */
	xuiWidgetSetRect(a->root, (xui_rect_t){0, 0, (float)a->width, (float)a->height});
	pe_ui_refresh(a);
	xuiLayout(a->ui);
	layout_init(a);
	const char *keys = "NOSZY";
	for (int i = 0; keys[i]; ++i)
		xuiHotKeyRegister(a->ui, a->root, keys[i], XUI_MOD_CTRL, hotkey, a);
	xuiHotKeyRegister(a->ui, a->root, 'S', XUI_MOD_CTRL | XUI_MOD_SHIFT, hotkey, a);
	xuiHotKeyRegister(a->ui, a->root, 'Z', XUI_MOD_CTRL | XUI_MOD_SHIFT, hotkey, a);
	xuiHotKeyRegister(a->ui, a->root, XUI_KEY_SPACE, 0, hotkey, a);
	xuiHotKeyRegister(a->ui, a->root, 'F', 0, hotkey, a);
	xgeSetQuitRequestCallback(quit_requested, a);
	return XGE_OK;
}
static void cleanup(pe_app *a)
{
	xgeSetQuitRequestCallback(NULL, NULL);
	pe_layout_save(a);
	if (a->default_layout)
		xuiDockPanelStateFree(a->default_layout);
	if (a->ui)
		xuiDestroy(a->ui);
	if (a->font)
		a->proxy.fontDestroy(&a->proxy, a->font);
	if (a->target)
		a->proxy.surfaceDestroy(&a->proxy, a->target);
	pe_preview_free(&a->preview);
}
int main(void)
{
	int argc = 0, result = 1;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pe_app *a = calloc(1, sizeof(*a));
	if (!a || !argv)
	{
		free(a);
		if (argv)
			LocalFree(argv);
		return 1;
	}
	a->doc = malloc(sizeof(*a->doc));
	if (!a->doc)
	{
		free(a);
		LocalFree(argv);
		return 1;
	}
	pe_doc_init(a->doc);
	a->zoom = 1;
	a->key = 0;
	a->burst = 0;
	char exe[PE_PATH];
	wchar_t wexe[PE_PATH];
	GetModuleFileNameW(NULL, wexe, PE_PATH);
	WideCharToMultiByte(CP_UTF8, 0, wexe, -1, exe, PE_PATH, NULL, NULL);
	pe_path_resolve(exe, "../presets", a->preset_root, sizeof(a->preset_root));
	int preset = -1, initial_curve = 0;
	for (int i = 1; i < argc; ++i)
	{
		char argument[PE_PATH];
		WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, argument, PE_PATH, NULL, NULL);
		if (!strcmp(argument, "--help"))
		{
			puts("XGE Particle Editor\nUsage: particleedit.exe [file.json|file.xson] [--preset 0..6] "
			     "[--frames N] [--capture file.png] [--exercise] [--curves]");
			result = 0;
			goto done;
		}
		else if (!strcmp(argument, "--exercise"))
			a->exercise = 1;
		else if (!strcmp(argument, "--curves"))
			initial_curve = 1;
		else if ((!strcmp(argument, "--frames") || !strcmp(argument, "--preset") ||
		          !strcmp(argument, "--capture")) &&
		         i + 1 < argc)
		{
			char value[PE_PATH];
			WideCharToMultiByte(CP_UTF8, 0, argv[++i], -1, value, PE_PATH, NULL, NULL);
			if (!strcmp(argument, "--frames"))
			{
				if (!pe_parse_int(value, 1, 100000, &a->frames))
					goto done;
			}
			else if (!strcmp(argument, "--preset"))
			{
				if (!pe_parse_int(value, 0, 6, &preset))
					goto done;
			}
			else if (!pe_path_absolute(value, a->capture, sizeof(a->capture)))
				goto done;
		}
		else if (argument[0] == '-')
		{
			fprintf(stderr, "unknown option: %s\n", argument);
			goto done;
		}
		else
		{
			if (a->input[0] || !pe_path_absolute(argument, a->input, sizeof(a->input)))
				goto done;
		}
	}
	if (a->capture[0] && !a->frames)
		a->frames = 120;
	if (a->exercise && !a->frames)
		a->frames = 120;
	xge_desc_t desc = {0};
	desc.iWidth = 1500;
	desc.iHeight = 960;
	desc.sTitle = "XGE Particle Editor";
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC | (a->frames ? 0 : XGE_INIT_ON_DEMAND);
	desc.iTargetFPS = 60;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	result = xgeInit(&desc);
	if (result != XGE_OK)
	{
		fprintf(stderr, "xgeInit: %d\n", result);
		result = 1;
		goto done;
	}
	a->startup_preset = preset;
	a->startup_curve = initial_curve;
	result = xgeRun(frame, a);
	if (result != XGE_OK)
		fprintf(stderr, "particleedit failed: %d\n", result);
	cleanup(a);
	xgeUnit();
	result = result == XGE_OK && !a->error ? 0 : 1;
done:
	free(a->doc);
	free(a);
	LocalFree(argv);
	return result;
}
