#include "../src/pe_fields.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
static int checks, failures;
#define CHECK(x)                                                                                             \
	do                                                                                                       \
	{                                                                                                        \
		++checks;                                                                                            \
		if (!(x))                                                                                            \
		{                                                                                                    \
			++failures;                                                                                      \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x);                                     \
		}                                                                                                    \
	} while (0)
int main(int argc, char **argv)
{
	pe_document *d = malloc(sizeof(*d)), *loaded = malloc(sizeof(*loaded));
	pe_data data;
	char text[256], path[PE_PATH], other[PE_PATH], bad[PE_PATH], directory[PE_PATH];
	wchar_t temp[PE_PATH], folder[PE_PATH];
	if (!d || !loaded)
		return 1;
	GetTempPathW(PE_PATH, temp);
	swprintf(folder, PE_PATH, L"%lsXGE-particleedit-%lu-%llu", temp, (unsigned long)GetCurrentProcessId(),
	         (unsigned long long)GetTickCount64());
	CHECK(CreateDirectoryW(folder, NULL));
	WideCharToMultiByte(CP_UTF8, 0, folder, -1, directory, sizeof(directory), NULL, NULL);
	CHECK(snprintf(path, sizeof(path), "%s/粒子 定义.xson", directory) < (int)sizeof(path));
	CHECK(snprintf(other, sizeof(other), "%s/roundtrip.json", directory) < (int)sizeof(other));
	CHECK(snprintf(bad, sizeof(bad), "%s/invalid.xson", directory) < (int)sizeof(bad));
	pe_doc_init(d);
	CHECK(!pe_doc_dirty(d));
	CHECK(pe_data_validate(&d->data, NULL, 0));
	for (int i = 0; i < pe_field_count; ++i)
	{
		xge_particle_emitter_t emitter = d->data.emitters[0];
		pe_field_get(&pe_fields[i], &emitter, text, sizeof(text));
		CHECK(pe_field_set(&pe_fields[i], &emitter, text));
		if (pe_fields[i].type != PE_FLOAT)
			CHECK(!memcmp((char *)&emitter + pe_fields[i].offset,
			              (char *)&d->data.emitters[0] + pe_fields[i].offset,
			              pe_fields[i].type == PE_STRING ? pe_fields[i].capacity : 4));
		CHECK(pe_field_find(pe_fields[i].id) == i);
	}
	int n;
	float f;
	uint32_t color;
	CHECK(!pe_parse_float("nan", -100, 100, &f));
	CHECK(!pe_parse_float("inf", -100, 100, &f));
	CHECK(!pe_parse_float("1e900", -100, 100, &f));
	CHECK(!pe_parse_float("12garbage", -100, 100, &f));
	CHECK(!pe_parse_int("1.5", 0, 10, &n));
	CHECK(!pe_parse_int("99999999999", 0, 10, &n));
	CHECK(pe_parse_color("#12345678", &color));
	CHECK(color == XGE_COLOR_RGBA(0x12, 0x34, 0x56, 0x78));
	CHECK(!pe_parse_color("#gg345678", &color));
	CHECK(!pe_parse_color("#123", &color));
	CHECK(pe_doc_save(d, path, 0));
	CHECK(!pe_doc_dirty(d));
	CHECK(pe_doc_load(loaded, path, 0));
	char *before = NULL, *after = NULL;
	size_t size_before, size_after;
	CHECK(pe_doc_text(&d->data, 0, &before, &size_before));
	CHECK(pe_doc_text(&loaded->data, 0, &after, &size_after));
	CHECK(size_before == size_after && !strcmp(before, after));
	xrtFree(before);
	xrtFree(after);
	data = d->data;
	data.emitters[0].fRate = 113;
	CHECK(pe_doc_commit(d, &data, 0, "rate"));
	CHECK(pe_doc_dirty(d));
	CHECK(pe_doc_undo(d));
	CHECK(!pe_doc_dirty(d));
	CHECK(pe_doc_redo(d));
	CHECK(pe_doc_dirty(d));
	CHECK(pe_doc_save(d, path, 0));
	CHECK(!pe_doc_dirty(d));
	CHECK(pe_doc_undo(d));
	CHECK(pe_doc_dirty(d));
	CHECK(pe_doc_redo(d));
	CHECK(!pe_doc_dirty(d));
	CHECK(pe_doc_add(d, 1));
	CHECK(pe_doc_add(d, 0));
	data = d->data;
	data.emitters[0].arrSubEmitters[1] = (xge_particle_subemitter_t){2, 10, 1};
	CHECK(pe_doc_commit(d, &data, 0, "link"));
	CHECK(pe_doc_move(d, 2));
	CHECK(d->data.emitters[2].arrSubEmitters[1].iEmitter == 1);
	d->selected = 1;
	CHECK(pe_doc_remove(d));
	CHECK(d->data.emitters[1].arrSubEmitters[1].iEmitter == -1);
	CHECK(pe_doc_undo(d));
	CHECK(d->data.emitters[2].arrSubEmitters[1].iEmitter == 1);
	data = d->data;
	data.emitters[1].arrSubEmitters[0] = (xge_particle_subemitter_t){2, 1, 0};
	CHECK(!pe_doc_commit(d, &data, 1, "cycle"));
	CHECK(d->data.emitters[1].arrSubEmitters[0].iEmitter == -1);
	d->selected = 0;
	CHECK(pe_key_add(d, 0, .25f, 2));
	CHECK(pe_key_add(d, 0, .75f, .5f));
	CHECK(!pe_key_add(d, 0, .75f, 3));
	CHECK(!pe_key_set(d, 0, 1, .1f, 1, 0, 0, 0));
	CHECK(pe_key_set(d, 0, 1, .9f, 1, 2, -3, 0));
	CHECK(pe_key_remove(d, 0, 0));
	CHECK(pe_key_add(d, 3, 0, 0));
	CHECK(pe_key_add(d, 3, 1, 0));
	CHECK(pe_key_set(d, 3, 1, 1, 0, 0, 0, XGE_COLOR_RGBA(255, 20, 10, 0)));
	CHECK(pe_key_remove(d, 3, 0));
	for (int i = 0; i < 100; ++i)
	{
		data = d->data;
		data.emitters[0].fRate = (float)(i + 1000);
		CHECK(pe_doc_commit(d, &data, 0, "bounded"));
	}
	CHECK(d->length == PE_HISTORY + 1);
	for (int i = 0; i < PE_HISTORY; ++i)
		CHECK(pe_doc_undo(d));
	CHECK(!pe_doc_undo(d));
	CHECK(pe_doc_redo(d));
	data = d->data;
	data.emitters[0].fRate = 42;
	CHECK(pe_doc_commit(d, &data, 0, "branch"));
	CHECK(!pe_doc_redo(d));
	CHECK(pe_doc_save(d, other, 0));
	CHECK(pe_doc_load(loaded, other, 0));
	CHECK(!pe_doc_save(loaded, path, 0));
	CHECK(pe_file_write(other, "external change", 15));
	data = loaded->data;
	data.emitters[0].fRate = 99;
	CHECK(pe_doc_commit(loaded, &data, 0, "local"));
	CHECK(!pe_doc_save(loaded, other, 0));
	CHECK(pe_doc_dirty(loaded));
	char *disk = NULL;
	size_t disk_size;
	CHECK(pe_file_read(other, &disk, &disk_size, NULL));
	CHECK(disk_size == 15 && !strcmp(disk, "external change"));
	free(disk);
	data = d->data;
	CHECK(pe_file_write(bad, "{version:99}", 12));
	CHECK(!pe_doc_load(d, bad, 0));
	CHECK(!memcmp(&data, &d->data, sizeof(data)));
	CHECK(pe_file_write(bad, "nul\0tail", 8));
	CHECK(!pe_file_read(bad, &disk, NULL, NULL));
	CHECK(!pe_doc_save(d, "Z:/missing-root/do-not-create/effect.xson", 0));
	if (argc > 1)
	{
		const char *names[] = {"impact", "fire_smoke", "explosion", "weather", "dust", "magic", "confetti"};
		for (int i = 0; i < 7; ++i)
		{
			CHECK(snprintf(text, sizeof(text), "%s/%s.xson", argv[1], names[i]) < (int)sizeof(text));
			CHECK(pe_doc_load(loaded, text, 1));
			CHECK(!loaded->path[0]);
			CHECK(!pe_doc_dirty(loaded));
			CHECK(pe_doc_text(&loaded->data, 0, &before, &size_before));
			xge_particle_definition def = NULL;
			CHECK(xgeParticleDefinitionParse(&def, before, size_before, 0, NULL, 0) == XGE_OK);
			CHECK(xgeParticleDefinitionCount(def) == loaded->data.count);
			xgeParticleDefinitionFree(def);
			xrtFree(before);
		}
	}
	/* Delete only the three exact paths generated by this test. */
	wchar_t w[PE_PATH];
	const char *files[] = {path, other, bad};
	for (int i = 0; i < 3; ++i)
	{
		MultiByteToWideChar(CP_UTF8, 0, files[i], -1, w, PE_PATH);
		CHECK(DeleteFileW(w));
	}
	CHECK(RemoveDirectoryW(folder));
	printf("particleedit document checks=%d failures=%d\n", checks, failures);
	free(d);
	free(loaded);
	return failures ? 1 : 0;
}
