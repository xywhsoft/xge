#include "mapedit_files.h"
#include <windows.h>
#include <wchar.h>
#include <wctype.h>

static int full_path(const char* path, wchar_t* full, int capacity)
{
	wchar_t wide[32768];
	DWORD count;
	if (!path || !*path || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 32768)) return 0;
	count = GetFullPathNameW(wide, (DWORD)capacity, full, NULL);
	return count > 0 && count < (DWORD)capacity;
}

uint64_t mapeditPathKey(const char* path)
{
	wchar_t full[32768];
	uint64_t hash = UINT64_C(14695981039346656037);
	size_t i;
	if (!full_path(path, full, 32768)) return 0;
	for (i = 0; full[i]; ++i) {
		uint32_t c = (uint32_t)towlower(full[i] == L'/' ? L'\\' : full[i]);
		hash = (hash ^ (c & 255u)) * UINT64_C(1099511628211);
		hash = (hash ^ (c >> 8)) * UINT64_C(1099511628211);
	}
	return hash;
}

mapedit_file_stamp_t mapeditFileStamp(const char* path)
{
	wchar_t full[32768];
	WIN32_FILE_ATTRIBUTE_DATA data;
	mapedit_file_stamp_t stamp = {0, 0, 0};
	if (!full_path(path, full, 32768) || !GetFileAttributesExW(full, GetFileExInfoStandard, &data)) return stamp;
	stamp.exists = 1;
	stamp.time = ((uint64_t)data.ftLastWriteTime.dwHighDateTime << 32) | data.ftLastWriteTime.dwLowDateTime;
	stamp.size = ((uint64_t)data.nFileSizeHigh << 32) | data.nFileSizeLow;
	return stamp;
}

int mapeditFileStampEqual(mapedit_file_stamp_t a, mapedit_file_stamp_t b)
{
	return a.exists == b.exists && a.time == b.time && a.size == b.size;
}
