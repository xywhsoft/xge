#ifndef MAPEDIT_FILES_H
#define MAPEDIT_FILES_H
#include <stdint.h>
typedef struct mapedit_file_stamp_t {
	uint64_t time, size;
	int exists;
} mapedit_file_stamp_t;
uint64_t mapeditPathKey(const char* path);
mapedit_file_stamp_t mapeditFileStamp(const char* path);
int mapeditFileStampEqual(mapedit_file_stamp_t a, mapedit_file_stamp_t b);
#endif
