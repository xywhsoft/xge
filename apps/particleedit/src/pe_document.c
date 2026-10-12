#include "pe_document.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

static int fail(pe_document *d, const char *message)
{
	snprintf(d->error, sizeof(d->error), "%s", message);
	return 0;
}
static int wide(const char *s, wchar_t *out, int capacity)
{
	return s && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, out, capacity) > 0;
}
int pe_path_absolute(const char *path, char *out, size_t size)
{
	wchar_t w[PE_PATH], absolute[PE_PATH];
	DWORD n;
	if (!wide(path, w, PE_PATH))
		return 0;
	n = GetFullPathNameW(w, PE_PATH, absolute, NULL);
	return n > 0 && n < PE_PATH &&
	       WideCharToMultiByte(CP_UTF8, 0, absolute, -1, out, (int)size, NULL, NULL) > 0;
}
int pe_path_resolve(const char *document, const char *resource, char *out, size_t size)
{
	char joined[PE_PATH];
	const char *slash, *back;
	size_t n;
	if (!resource || !resource[0])
		return 0;
	if (resource[0] == '/' || resource[0] == '\\' || (resource[1] == ':'))
		return pe_path_absolute(resource, out, size);
	slash = strrchr(document, '/');
	back = strrchr(document, '\\');
	if (!slash || (back && back > slash))
		slash = back;
	n = slash ? (size_t)(slash + 1 - document) : 0;
	if (n + strlen(resource) >= sizeof(joined))
		return 0;
	memcpy(joined, document, n);
	strcpy(joined + n, resource);
	return pe_path_absolute(joined, out, size);
}
int pe_path_is_xson(const char *path)
{
	const char *extension = strrchr(path, '.');
	return extension && !_stricmp(extension, ".xson");
}
int pe_file_read(const char *path, char **text, size_t *size, uint64_t *hash)
{
	wchar_t w[PE_PATH];
	HANDLE file;
	LARGE_INTEGER length;
	DWORD read;
	char *buffer;
	size_t i;
	uint64_t h = UINT64_C(14695981039346656037);
	*text = NULL;
	if (!wide(path, w, PE_PATH))
		return 0;
	file = CreateFileW(w, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return 0;
	if (!GetFileSizeEx(file, &length) || length.QuadPart < 0 || length.QuadPart > PE_FILE_LIMIT)
	{
		CloseHandle(file);
		return 0;
	}
	buffer = malloc((size_t)length.QuadPart + 1);
	if (!buffer)
	{
		CloseHandle(file);
		return 0;
	}
	if (!ReadFile(file, buffer, (DWORD)length.QuadPart, &read, NULL) || read != length.QuadPart)
	{
		free(buffer);
		CloseHandle(file);
		return 0;
	}
	CloseHandle(file);
	buffer[read] = 0;
	if (memchr(buffer, 0, read))
	{
		free(buffer);
		return 0;
	}
	for (i = 0; i < read; ++i)
	{
		h ^= (unsigned char)buffer[i];
		h *= UINT64_C(1099511628211);
	}
	*text = buffer;
	if (size)
		*size = read;
	if (hash)
		*hash = h;
	return 1;
}
int pe_file_write(const char *path, const char *text, size_t size)
{
	wchar_t w[PE_PATH], temp[PE_PATH];
	HANDLE file = INVALID_HANDLE_VALUE;
	DWORD written;
	static unsigned sequence;
	int i, ok;
	if (size > PE_FILE_LIMIT || !wide(path, w, PE_PATH) || wcslen(w) + 48 >= PE_PATH)
		return 0;
	for (i = 0; i < 32 && file == INVALID_HANDLE_VALUE; ++i)
	{
		swprintf(temp, PE_PATH, L"%ls.pe-%lu-%u.tmp", w, (unsigned long)GetCurrentProcessId(), ++sequence);
		file = CreateFileW(temp, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS)
			return 0;
	}
	if (file == INVALID_HANDLE_VALUE)
		return 0;
	ok = WriteFile(file, text, (DWORD)size, &written, NULL) && written == size && FlushFileBuffers(file);
	if (!CloseHandle(file))
		ok = 0;
	if (ok)
		ok = MoveFileExW(temp, w, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
	if (!ok)
		DeleteFileW(temp); /* Only the uniquely-created temporary file, never the document. */
	return ok;
}
int pe_data_validate(const pe_data *data, char *error, size_t size)
{
	xge_particle_definition definition = NULL;
	int result = xgeParticleDefinitionCreate(&definition, data->emitters, data->count);
	xgeParticleDefinitionFree(definition);
	if (result != XGE_OK && error && size)
		snprintf(error, size, "参数无效：检查范围、Burst < 持续时间、关键点顺序或子发射器循环引用（%d）",
		         result);
	return result == XGE_OK;
}
static void baseline(pe_document *d)
{
	d->clean = d->data;
	d->cursor = 0;
	d->length = 1;
	d->history[0].data = d->data;
	d->history[0].selected = d->selected;
	strcpy(d->history[0].label, "初始状态");
	++d->revision;
	d->error[0] = 0;
}
void pe_doc_init(pe_document *d)
{
	memset(d, 0, sizeof(*d));
	d->data.count = 1;
	xgeParticleEmitterInit(&d->data.emitters[0]);
	strcpy(d->data.emitters[0].sName, "Emitter 1");
	strcpy(d->data.emitters[0].sTexture, "soft");
	d->data.emitters[0].fRate = 90;
	d->data.emitters[0].tSpeed = (xge_particle_range_t){40, 120};
	d->data.emitters[0].tSize = (xge_particle_range_t){12, 24};
	d->data.emitters[0].fSpread = 6.2831853f;
	d->data.emitters[0].tAlphaOverLife.iCount = 2;
	d->data.emitters[0].tAlphaOverLife.arrKeys[0] = (xge_particle_key_t){0, 1, 0, 0};
	d->data.emitters[0].tAlphaOverLife.arrKeys[1] = (xge_particle_key_t){1, 0, 0, 0};
	baseline(d);
}
int pe_doc_dirty(const pe_document *d)
{
	return memcmp(&d->data, &d->clean, sizeof(d->data)) != 0;
}
int pe_doc_commit(pe_document *d, const pe_data *candidate, int selected, const char *label)
{
	pe_data copy = *candidate;
	if (!pe_data_validate(&copy, d->error, sizeof(d->error)))
		return 0;
	memset(copy.emitters + copy.count, 0,
	       (XGE_PARTICLE_MAX_EMITTERS - copy.count) * sizeof(copy.emitters[0]));
	if (!memcmp(&d->data, &copy, sizeof(copy)))
		return 1;
	if (d->cursor == PE_HISTORY)
	{
		memmove(d->history, d->history + 1, PE_HISTORY * sizeof(d->history[0]));
		--d->cursor;
	}
	d->data = copy;
	d->selected = selected < 0 ? 0 : selected >= copy.count ? copy.count - 1 : selected;
	++d->cursor;
	d->length = d->cursor + 1;
	d->history[d->cursor].data = copy;
	d->history[d->cursor].selected = d->selected;
	snprintf(d->history[d->cursor].label, sizeof(d->history[d->cursor].label), "%s", label);
	++d->revision;
	d->error[0] = 0;
	return 1;
}
int pe_doc_undo(pe_document *d)
{
	if (!d->cursor)
		return 0;
	--d->cursor;
	d->data = d->history[d->cursor].data;
	d->selected = d->history[d->cursor].selected;
	++d->revision;
	return 1;
}
int pe_doc_redo(pe_document *d)
{
	if (d->cursor + 1 >= d->length)
		return 0;
	++d->cursor;
	d->data = d->history[d->cursor].data;
	d->selected = d->history[d->cursor].selected;
	++d->revision;
	return 1;
}
int pe_doc_add(pe_document *d, int duplicate)
{
	pe_data data = d->data;
	int n = data.count;
	if (n == XGE_PARTICLE_MAX_EMITTERS)
		return fail(d, "最多 16 个发射器");
	if (duplicate)
		data.emitters[n] = data.emitters[d->selected];
	else
		xgeParticleEmitterInit(&data.emitters[n]);
	snprintf(data.emitters[n].sName, sizeof(data.emitters[n].sName), "Emitter %d", n + 1);
	data.count++;
	return pe_doc_commit(d, &data, n, duplicate ? "复制发射器" : "添加发射器");
}
int pe_doc_remove(pe_document *d)
{
	pe_data data = d->data;
	int i, j, n = d->selected;
	if (data.count <= 1)
		return fail(d, "至少保留一个发射器");
	memmove(data.emitters + n, data.emitters + n + 1, (data.count - n - 1) * sizeof(data.emitters[0]));
	--data.count;
	for (i = 0; i < data.count; ++i)
		for (j = 0; j < 3; ++j)
		{
			int *target = &data.emitters[i].arrSubEmitters[j].iEmitter;
			if (*target == n)
				*target = -1;
			else if (*target > n)
				--*target;
		}
	return pe_doc_commit(d, &data, n, "删除发射器并修正引用");
}
int pe_doc_move(pe_document *d, int to)
{
	pe_data data = d->data;
	int from = d->selected, i, j, map[XGE_PARTICLE_MAX_EMITTERS];
	if (to < 0 || to >= data.count || to == from)
		return 0;
	for (i = 0; i < data.count; ++i)
	{
		map[i] = i == from                          ? to
		         : from < to && i > from && i <= to ? i - 1
		         : to < from && i >= to && i < from ? i + 1
		                                            : i;
		data.emitters[map[i]] = d->data.emitters[i];
	}
	for (i = 0; i < data.count; ++i)
		for (j = 0; j < 3; ++j)
		{
			int *target = &data.emitters[i].arrSubEmitters[j].iEmitter;
			if (*target >= 0)
				*target = map[*target];
		}
	return pe_doc_commit(d, &data, to, "重排发射器并修正引用");
}
int pe_doc_load(pe_document *d, const char *path, int as_template)
{
	xge_particle_definition definition = NULL;
	char absolute[PE_PATH], *text = NULL;
	pe_data data = {0};
	size_t size;
	uint64_t hash;
	int i, result;
	if (!pe_path_absolute(path, absolute, sizeof(absolute)) || !pe_file_read(absolute, &text, &size, &hash))
		return fail(d, "读取失败：路径、UTF-8、权限或文件大小（上限 1 MiB）");
	result = xgeParticleDefinitionParse(&definition, text, size, pe_path_is_xson(path), d->error,
	                                    sizeof(d->error));
	free(text);
	if (result != XGE_OK)
		return 0;
	data.count = xgeParticleDefinitionCount(definition);
	for (i = 0; i < data.count; ++i)
	{
		xgeParticleDefinitionGet(definition, i, &data.emitters[i]);
		if (as_template && !data.emitters[i].sName[0])
			snprintf(data.emitters[i].sName, sizeof(data.emitters[i].sName), "Emitter %d", i + 1);
	}
	xgeParticleDefinitionFree(definition);
	d->data = data;
	d->selected = 0;
	snprintf(d->path, sizeof(d->path), "%s", as_template ? "" : absolute);
	d->disk_hash = hash;
	d->has_disk_hash = !as_template;
	baseline(d);
	return 1;
}
int pe_doc_text(const pe_data *data, int xson, char **text, size_t *size)
{
	xge_particle_definition definition = NULL;
	int result = xgeParticleDefinitionCreate(&definition, data->emitters, data->count);
	if (result == XGE_OK)
		result = xgeParticleDefinitionStringify(definition, xson, text, size);
	xgeParticleDefinitionFree(definition);
	return result == XGE_OK;
}
int pe_doc_save(pe_document *d, const char *path, int allow_replace)
{
	char absolute[PE_PATH], *text = NULL, *existing = NULL;
	wchar_t w[PE_PATH];
	uint64_t hash;
	size_t size;
	int same;
	if (!pe_path_absolute(path, absolute, sizeof(absolute)))
		return fail(d, "无效的保存路径");
	same = d->has_disk_hash && !_stricmp(absolute, d->path);
	if (same)
	{
		if (!pe_file_read(absolute, &existing, NULL, &hash))
			return fail(d, "磁盘文件已删除或无法读取；请另存为");
		free(existing);
		if (hash != d->disk_hash)
			return fail(d, "磁盘文件被其他程序修改；请重新打开或另存为，未覆盖原文件");
	}
	else if (!allow_replace)
	{
		if (!wide(absolute, w, PE_PATH))
			return fail(d, "保存路径转换失败");
		if (GetFileAttributesW(w) != INVALID_FILE_ATTRIBUTES)
			return fail(d, "目标已存在，需要明确确认覆盖");
		if (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND)
			return fail(d, "无法确认目标文件状态");
	}
	if (!pe_doc_text(&d->data, pe_path_is_xson(path), &text, &size))
		return fail(d, "粒子定义验证或序列化失败");
	if (!pe_file_write(absolute, text, size))
	{
		xrtFree(text);
		return fail(d, "原子保存失败；原文件未主动删除");
	}
	hash = UINT64_C(14695981039346656037);
	for (size_t i = 0; i < size; ++i)
	{
		hash ^= (unsigned char)text[i];
		hash *= UINT64_C(1099511628211);
	}
	xrtFree(text);
	strcpy(d->path, absolute);
	d->disk_hash = hash;
	d->has_disk_hash = 1;
	d->clean = d->data;
	d->error[0] = 0;
	return 1;
}
xge_particle_curve_t *pe_curve(pe_data *data, int emitter, int channel)
{
	xge_particle_emitter_t *e = &data->emitters[emitter];
	return channel == 0 ? &e->tSizeOverLife : channel == 1 ? &e->tSpeedOverLife : &e->tAlphaOverLife;
}
int pe_key_set(pe_document *d, int channel, int key, float time, float value, float in, float out,
               uint32_t color)
{
	pe_data data = d->data;
	if (channel == 3)
	{
		xge_particle_gradient_t *g = &data.emitters[d->selected].tColorOverLife;
		if (key < 0 || key >= g->iCount)
			return 0;
		g->arrKeys[key] = (xge_particle_color_key_t){time, color};
	}
	else
	{
		xge_particle_curve_t *c = pe_curve(&data, d->selected, channel);
		if (key < 0 || key >= c->iCount)
			return 0;
		c->arrKeys[key] = (xge_particle_key_t){time, value, in, out};
	}
	return pe_doc_commit(d, &data, d->selected, "编辑生命周期关键点");
}
int pe_key_add(pe_document *d, int channel, float time, float value)
{
	pe_data data = d->data;
	int i;
	if (channel == 3)
	{
		xge_particle_gradient_t *g = &data.emitters[d->selected].tColorOverLife;
		uint32_t color = xgeParticleGradientEval(g, time, 0xffffffffu);
		if (g->iCount == XGE_PARTICLE_MAX_KEYS)
			return fail(d, "渐变最多 8 个关键点");
		for (i = g->iCount; i > 0 && g->arrKeys[i - 1].fTime > time; --i)
			g->arrKeys[i] = g->arrKeys[i - 1];
		g->arrKeys[i] = (xge_particle_color_key_t){time, color};
		++g->iCount;
	}
	else
	{
		xge_particle_curve_t *c = pe_curve(&data, d->selected, channel);
		if (c->iCount == XGE_PARTICLE_MAX_KEYS)
			return fail(d, "曲线最多 8 个关键点");
		for (i = c->iCount; i > 0 && c->arrKeys[i - 1].fTime > time; --i)
			c->arrKeys[i] = c->arrKeys[i - 1];
		c->arrKeys[i] = (xge_particle_key_t){time, value, 0, 0};
		++c->iCount;
	}
	return pe_doc_commit(d, &data, d->selected, "添加生命周期关键点");
}
int pe_key_remove(pe_document *d, int channel, int key)
{
	pe_data data = d->data;
	if (channel == 3)
	{
		xge_particle_gradient_t *g = &data.emitters[d->selected].tColorOverLife;
		if (key < 0 || key >= g->iCount)
			return 0;
		memmove(g->arrKeys + key, g->arrKeys + key + 1, (g->iCount - key - 1) * sizeof(g->arrKeys[0]));
		g->arrKeys[--g->iCount] = (xge_particle_color_key_t){0};
	}
	else
	{
		xge_particle_curve_t *c = pe_curve(&data, d->selected, channel);
		if (key < 0 || key >= c->iCount)
			return 0;
		memmove(c->arrKeys + key, c->arrKeys + key + 1, (c->iCount - key - 1) * sizeof(c->arrKeys[0]));
		c->arrKeys[--c->iCount] = (xge_particle_key_t){0};
	}
	return pe_doc_commit(d, &data, d->selected, "删除生命周期关键点");
}
