#include "host_files.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static int wide(const char *s, wchar_t *w, int cap) {
    return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, w, cap) != 0;
}
void host_append(host_buffer *b, const char *fmt, ...) {
    va_list ap, copy;
    int n;
    size_t cap;
    char *next;
    if (b->failed)
        return;
    va_start(ap, fmt);
    va_copy(copy, ap);
    n = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (n < 0 || b->size + (size_t)n + 1 > HOST_CODE_LIMIT * 12u) {
        b->failed = 1;
        va_end(ap);
        return;
    }
    if (b->size + (size_t)n + 1 > b->capacity) {
        cap = (b->size + (size_t)n + 1) * 2;
        next = realloc(b->data, cap);
        if (!next) {
            b->failed = 1;
            va_end(ap);
            return;
        }
        b->data = next;
        b->capacity = cap;
    }
    vsnprintf(b->data + b->size, b->capacity - b->size, fmt, ap);
    b->size += (size_t)n;
    va_end(ap);
}
int host_path(char *out, size_t cap, const char *base, const char *leaf) {
    int n = snprintf(out, cap, "%s/%s", base, leaf);
    return n >= 0 && (size_t)n < cap;
}
int host_mkdir(const char *path) {
    wchar_t w[HOST_PATH];
    DWORD attrs;
    if (!wide(path, w, HOST_PATH))
        return 0;
    if (CreateDirectoryW(w, NULL))
        return 1;
    attrs = GetFileAttributesW(w);
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}
char *host_read(const char *path, size_t *size, size_t limit) {
    wchar_t w[HOST_PATH];
    FILE *f;
    long length;
    char *data;
    if (!wide(path, w, HOST_PATH) || !(f = _wfopen(w, L"rb")))
        return NULL;
    if (fseek(f, 0, SEEK_END) || (length = ftell(f)) < 0 || (size_t)length > limit ||
        fseek(f, 0, SEEK_SET)) {
        fclose(f);
        return NULL;
    }
    data = malloc((size_t)length + 1);
    if (!data || fread(data, 1, (size_t)length, f) != (size_t)length) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    data[length] = 0;
    if (size)
        *size = (size_t)length;
    return data;
}
int host_write(const char *path, const void *data, size_t size) {
    wchar_t w[HOST_PATH];
    HANDLE f;
    DWORD written;
    int ok;
    if (size > MAXDWORD || !wide(path, w, HOST_PATH))
        return 0;
    f = CreateFileW(w, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE)
        return 0;
    ok = WriteFile(f, data, (DWORD)size, &written, NULL) && written == size && FlushFileBuffers(f);
    CloseHandle(f);
    return ok;
}
int host_atomic_write(const char *path, const void *data, size_t size) {
    char temp[HOST_PATH];
    wchar_t a[HOST_PATH], b[HOST_PATH];
    int n;
    n = snprintf(temp, sizeof(temp), "%s.tmp.%lu.%llu", path, GetCurrentProcessId(),
                 (unsigned long long)GetTickCount64());
    if (n < 0 || n >= (int)sizeof(temp) || !wide(temp, a, HOST_PATH) || !wide(path, b, HOST_PATH))
        return 0;
    if (!host_write(temp, data, size))
        return 0;
    if (MoveFileExW(a, b, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return 1;
    /* Keep failed save data for recovery; never mark the document saved. */
    return 0;
}
int host_choose_file(int save, char *path, size_t capacity) {
    wchar_t w[HOST_PATH] = L"", result[HOST_PATH];
    OPENFILENAMEW ofn = {0};
    if (path && *path && !wide(path, w, HOST_PATH))
        return 0;
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = w;
    ofn.nMaxFile = HOST_PATH;
    ofn.lpstrFilter = L"UIDesign shell project (*.uidproj)\0*.uidproj\0\0";
    ofn.lpstrDefExt = L"uidproj";
    ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST |
                (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    if (!(save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn)))
        return 0;
    if (!GetFullPathNameW(w, HOST_PATH, result, NULL))
        return 0;
    return WideCharToMultiByte(CP_UTF8, 0, result, -1, path, (int)capacity, NULL, NULL) != 0;
}
int host_executable_directory(char *path, size_t capacity) {
    wchar_t w[HOST_PATH], *slash;
    DWORD n = GetModuleFileNameW(NULL, w, HOST_PATH);
    if (!n || n >= HOST_PATH)
        return 0;
    slash = wcsrchr(w, L'\\');
    if (!slash)
        return 0;
    *slash = 0;
    return WideCharToMultiByte(CP_UTF8, 0, w, -1, path, (int)capacity, NULL, NULL) != 0;
}
HMODULE host_load_library(const char *path) {
    wchar_t w[HOST_PATH];
    return wide(path, w, HOST_PATH) ? LoadLibraryW(w) : NULL;
}
int host_start_compiler(const char *compiler, const char *sdk, const char *work,
                        PROCESS_INFORMATION *process, HANDLE *log, HANDLE *job) {
    char command[HOST_PATH * 6], path[HOST_PATH];
    wchar_t cmd[HOST_PATH * 6], cwd[HOST_PATH], exe[HOST_PATH], logpath[HOST_PATH];
    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, TRUE};
    STARTUPINFOW start = {0};
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {0};
    int n, ok;
    /* No command shell. User text is only in source files, never command arguments. */
    if (strchr(compiler, '"') || strchr(sdk, '"') || strchr(work, '"'))
        return 0;
    n = snprintf(
        command, sizeof(command),
        "\"%s\" -std=c11 -shared -O0 -g -Wall -Wextra -Werror=implicit-function-declaration "
        "-I\"%s\" -I\"%s/tools/UIDesign/component/include\" form.generated.c business.c "
        "\"%s/build/xge.lib\" -o form.dll",
        compiler, sdk, sdk, sdk);
    if (n < 0 || n >= (int)sizeof(command) || !wide(command, cmd, HOST_PATH * 6) ||
        !wide(work, cwd, HOST_PATH) || !wide(compiler, exe, HOST_PATH) ||
        !host_path(path, sizeof(path), work, "build.log") || !wide(path, logpath, HOST_PATH))
        return 0;
    *log = CreateFileW(logpath, GENERIC_WRITE, FILE_SHARE_READ, &security, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (*log == INVALID_HANDLE_VALUE)
        return 0;
    start.cb = sizeof(start);
    start.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    start.wShowWindow = SW_HIDE;
    start.hStdOutput = *log;
    start.hStdError = *log;
    start.hStdInput = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  &security, OPEN_EXISTING, 0, NULL);
    *job = CreateJobObjectW(NULL, NULL);
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    ok = *job &&
         SetInformationJobObject(*job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    if (ok)
        ok = CreateProcessW(exe, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL,
                            cwd, &start, process);
    if (ok && !AssignProcessToJobObject(*job, process->hProcess)) {
        TerminateProcess(process->hProcess, 1);
        CloseHandle(process->hProcess);
        CloseHandle(process->hThread);
        ok = 0;
    }
    if (ok)
        ResumeThread(process->hThread);
    if (start.hStdInput != INVALID_HANDLE_VALUE)
        CloseHandle(start.hStdInput);
    if (!ok) {
        if (*job)
            CloseHandle(*job);
        *job = NULL;
        CloseHandle(*log);
        *log = NULL;
    }
    return ok;
}
