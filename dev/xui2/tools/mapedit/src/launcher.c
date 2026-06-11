#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <shellapi.h>

#if defined(MAPEDIT_FORCE_DISCRETE_GPU)
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

#define MAPEDIT_LAUNCHER_PATH_MAX 1024
#define MAPEDIT_LAUNCHER_CMD_MAX 32768

static int mapedit_launcher_append(wchar_t* sOut, size_t iCap, size_t* pLen, const wchar_t* sText)
{
	size_t n;
	if ( sOut == NULL || pLen == NULL || sText == NULL ) return 0;
	n = wcslen(sText);
	if ( *pLen + n + 1 >= iCap ) return 0;
	memcpy(sOut + *pLen, sText, n * sizeof(wchar_t));
	*pLen += n;
	sOut[*pLen] = 0;
	return 1;
}

static int mapedit_launcher_append_char(wchar_t* sOut, size_t iCap, size_t* pLen, wchar_t ch)
{
	if ( sOut == NULL || pLen == NULL ) return 0;
	if ( *pLen + 2 >= iCap ) return 0;
	sOut[*pLen] = ch;
	*pLen += 1;
	sOut[*pLen] = 0;
	return 1;
}

static int mapedit_launcher_append_quoted_arg(wchar_t* sOut, size_t iCap, size_t* pLen, const wchar_t* sArg)
{
	size_t i;
	size_t slashes;
	if ( sArg == NULL ) sArg = L"";
	if ( !mapedit_launcher_append_char(sOut, iCap, pLen, L'"') ) return 0;
	slashes = 0;
	for ( i = 0; sArg[i] != 0; i++ ) {
		if ( sArg[i] == L'\\' ) {
			slashes++;
			continue;
		}
		if ( sArg[i] == L'"' ) {
			while ( slashes-- > 0 ) {
				if ( !mapedit_launcher_append(sOut, iCap, pLen, L"\\\\") ) return 0;
			}
			if ( !mapedit_launcher_append(sOut, iCap, pLen, L"\\\"") ) return 0;
			slashes = 0;
			continue;
		}
		while ( slashes-- > 0 ) {
			if ( !mapedit_launcher_append_char(sOut, iCap, pLen, L'\\') ) return 0;
		}
		if ( !mapedit_launcher_append_char(sOut, iCap, pLen, sArg[i]) ) return 0;
		slashes = 0;
	}
	while ( slashes-- > 0 ) {
		if ( !mapedit_launcher_append(sOut, iCap, pLen, L"\\\\") ) return 0;
	}
	if ( !mapedit_launcher_append_char(sOut, iCap, pLen, L'"') ) return 0;
	return 1;
}

static int mapedit_launcher_get_dir(wchar_t* sExe, wchar_t* sDir, size_t iCap)
{
	DWORD n;
	wchar_t* p;
	if ( sExe == NULL || sDir == NULL || iCap == 0 ) return 0;
	n = GetModuleFileNameW(NULL, sExe, (DWORD)iCap);
	if ( n == 0 || n >= (DWORD)iCap ) return 0;
	wcsncpy(sDir, sExe, iCap);
	sDir[iCap - 1] = 0;
	p = wcsrchr(sDir, L'\\');
	if ( p == NULL ) p = wcsrchr(sDir, L'/');
	if ( p == NULL ) return 0;
	*p = 0;
	return 1;
}

static int mapedit_launcher_build_cmdline(wchar_t* sCmd, size_t iCap, const wchar_t* sCorePath)
{
	LPWSTR* argv;
	int argc;
	int i;
	size_t len;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if ( argv == NULL ) return 0;
	len = 0;
	sCmd[0] = 0;
	if ( !mapedit_launcher_append_quoted_arg(sCmd, iCap, &len, sCorePath) ) {
		LocalFree(argv);
		return 0;
	}
	if ( !mapedit_launcher_append(sCmd, iCap, &len, L" --mapedit-wrapper-child") ) {
		LocalFree(argv);
		return 0;
	}
	for ( i = 1; i < argc; i++ ) {
		if ( !mapedit_launcher_append_char(sCmd, iCap, &len, L' ') ||
			 !mapedit_launcher_append_quoted_arg(sCmd, iCap, &len, argv[i]) ) {
			LocalFree(argv);
			return 0;
		}
	}
	LocalFree(argv);
	return 1;
}

static void mapedit_launcher_foreground_existing(void)
{
	HWND hWnd;
	hWnd = FindWindowA(NULL, "MapEdit");
	if ( hWnd != NULL ) {
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
	}
}

int main(void)
{
	wchar_t exePath[MAPEDIT_LAUNCHER_PATH_MAX];
	wchar_t appDir[MAPEDIT_LAUNCHER_PATH_MAX];
	wchar_t corePath[MAPEDIT_LAUNCHER_PATH_MAX];
	wchar_t cmdLine[MAPEDIT_LAUNCHER_CMD_MAX];
	HANDLE hMutex;
	DWORD err;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	DWORD exitCode;

	SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
	SetEnvironmentVariableA("SHIM_MCCOMPAT", "0x800000001");
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	if ( !mapedit_launcher_get_dir(exePath, appDir, MAPEDIT_LAUNCHER_PATH_MAX) ) {
		MessageBoxW(NULL, L"Failed to locate MapEdit directory.", L"MapEdit", MB_OK | MB_ICONERROR);
		return 1;
	}
	SetCurrentDirectoryW(appDir);
	SetDllDirectoryW(appDir);

	hMutex = CreateMutexW(NULL, TRUE, L"Local\\xge_xui2_mapedit_single_instance");
	if ( hMutex == NULL ) {
		MessageBoxW(NULL, L"Failed to create MapEdit single-instance mutex.", L"MapEdit", MB_OK | MB_ICONERROR);
		return 1;
	}
	err = GetLastError();
	if ( err == ERROR_ALREADY_EXISTS ) {
		mapedit_launcher_foreground_existing();
		CloseHandle(hMutex);
		return 0;
	}

	if ( _snwprintf(corePath, MAPEDIT_LAUNCHER_PATH_MAX, L"%ls\\xge_mapedit_core.exe", appDir) < 0 ||
		 !mapedit_launcher_build_cmdline(cmdLine, MAPEDIT_LAUNCHER_CMD_MAX, corePath) ) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		MessageBoxW(NULL, L"Failed to build MapEdit launch command.", L"MapEdit", MB_OK | MB_ICONERROR);
		return 1;
	}

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	GetStartupInfoW(&si);
	if ( !CreateProcessW(corePath, cmdLine, NULL, NULL, TRUE, 0, NULL, appDir, &si, &pi) ) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		MessageBoxW(NULL, L"Failed to start xge_mapedit_core.exe.", L"MapEdit", MB_OK | MB_ICONERROR);
		return 1;
	}

	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);
	exitCode = 1;
	(void)GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hProcess);
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return (int)exitCode;
}
