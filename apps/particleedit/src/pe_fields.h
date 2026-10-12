#ifndef PE_FIELDS_H
#define PE_FIELDS_H
#include "pe_document.h"
enum
{
	PE_FLOAT,
	PE_INT,
	PE_UINT,
	PE_BOOL,
	PE_STRING,
	PE_COLOR,
	PE_ENUM
};
typedef struct pe_field
{
	const char *id, *name, *group, *help;
	size_t offset, capacity;
	int type;
	double min, max;
	float scale;
	const char *const *options;
	int option_count;
} pe_field;
extern const pe_field pe_fields[];
extern const int pe_field_count;
int pe_field_find(const char *id);
void pe_field_get(const pe_field *field, const xge_particle_emitter_t *e, char *out, size_t size);
int pe_field_set(const pe_field *field, xge_particle_emitter_t *e, const char *text);
int pe_parse_float(const char *text, float min, float max, float *value);
int pe_parse_int(const char *text, int min, int max, int *value);
int pe_parse_color(const char *text, uint32_t *value);
#endif
