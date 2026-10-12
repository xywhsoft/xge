#ifndef PE_DOCUMENT_H
#define PE_DOCUMENT_H
#include "xge.h"
#include <stddef.h>

#define PE_HISTORY 64
#define PE_PATH 4096
#define PE_FILE_LIMIT (1024u * 1024u)
typedef struct pe_data
{
	int count;
	xge_particle_emitter_t emitters[XGE_PARTICLE_MAX_EMITTERS];
} pe_data;
typedef struct pe_snapshot
{
	pe_data data;
	int selected;
	char label[64];
} pe_snapshot;
typedef struct pe_document
{
	pe_data data, clean;
	pe_snapshot history[PE_HISTORY + 1];
	int selected, cursor, length;
	unsigned revision;
	char path[PE_PATH], error[512];
	uint64_t disk_hash;
	int has_disk_hash;
} pe_document;

void pe_doc_init(pe_document *doc);
int pe_doc_dirty(const pe_document *doc);
int pe_data_validate(const pe_data *data, char *error, size_t size);
int pe_doc_commit(pe_document *doc, const pe_data *candidate, int selected, const char *label);
int pe_doc_undo(pe_document *doc);
int pe_doc_redo(pe_document *doc);
int pe_doc_add(pe_document *doc, int duplicate);
int pe_doc_remove(pe_document *doc);
int pe_doc_move(pe_document *doc, int to);
int pe_doc_load(pe_document *doc, const char *path, int as_template);
int pe_doc_save(pe_document *doc, const char *path, int allow_replace);
int pe_doc_text(const pe_data *data, int xson, char **text, size_t *size);
int pe_file_read(const char *path, char **text, size_t *size, uint64_t *hash);
int pe_file_write(const char *path, const char *text, size_t size);
int pe_path_absolute(const char *path, char *out, size_t size);
int pe_path_resolve(const char *document, const char *resource, char *out, size_t size);
int pe_path_is_xson(const char *path);
xge_particle_curve_t *pe_curve(pe_data *data, int emitter, int channel);
int pe_key_set(pe_document *doc, int channel, int key, float time, float value, float in, float out,
               uint32_t color);
int pe_key_add(pe_document *doc, int channel, float time, float value);
int pe_key_remove(pe_document *doc, int channel, int key);
#endif
