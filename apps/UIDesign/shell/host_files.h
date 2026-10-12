#ifndef UID_HOST_FILES_H
#define UID_HOST_FILES_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#define HOST_PATH 2048
#define HOST_CODE_LIMIT (1024u * 1024u)
typedef struct host_buffer {
    char *data;
    size_t size, capacity;
    int failed;
} host_buffer;
void host_append(host_buffer *, const char *format, ...);
int host_path(char *out, size_t capacity, const char *base, const char *leaf);
int host_mkdir(const char *);
char *host_read(const char *, size_t *size, size_t limit);
int host_write(const char *, const void *, size_t);
int host_atomic_write(const char *, const void *, size_t);
int host_choose_file(int save, char *path, size_t capacity);
int host_start_compiler(const char *compiler, const char *sdk, const char *work,
                        PROCESS_INFORMATION *, HANDLE *log, HANDLE *job);
HMODULE host_load_library(const char *);
int host_executable_directory(char *, size_t);
#endif
