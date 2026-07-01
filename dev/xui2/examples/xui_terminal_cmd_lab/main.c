#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "xui.h"
#include "xge.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	900
#define DEMO_TARGET_H	560
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define DEMO_LINE_MAX	512
#define DEMO_PATH_MAX	1024
#define DEMO_HISTORY_MAX	32

#define XUI_DEMO_KEY_LEFT_SHIFT		340
#define XUI_DEMO_KEY_LEFT_CTRL		341
#define XUI_DEMO_KEY_LEFT_ALT		342
#define XUI_DEMO_KEY_LEFT_SUPER		343
#define XUI_DEMO_KEY_RIGHT_SHIFT	344
#define XUI_DEMO_KEY_RIGHT_CTRL		345
#define XUI_DEMO_KEY_RIGHT_ALT		346
#define XUI_DEMO_KEY_RIGHT_SUPER	347

typedef enum xui_cmd_lab_mode_t {
	XUI_CMD_LAB_SHELL = 0,
	XUI_CMD_LAB_PROCESS = 1
} xui_cmd_lab_mode_t;

typedef struct xui_terminal_cmd_lab_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTerminal;
	xui_widget pStatus;
	xui_terminal_session_t* pSession;
	xui_cmd_lab_mode_t iMode;
	char sCwd[DEMO_PATH_MAX];
	char sLine[DEMO_LINE_MAX];
	int iLineLen;
	char arrHistory[DEMO_HISTORY_MAX][DEMO_LINE_MAX];
	int iHistoryCount;
	int iHistoryCursor;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int bHasMouse;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bProcessSmoke;
	int bAutoDone;
	int iAutoStep;
	int iAutoProcessFrame;
	int iInputBytes;
	int iCommandCount;
	int iProcessCount;
	int iResizeCount;
	int iLastCols;
	int iLastRows;
	int bCreateOK;
	int bLayoutOK;
	int bDynamicOK;
} xui_terminal_cmd_lab_t;

static void __xuiCmdLabUsage(void)
{
	printf("usage: xui_terminal_cmd_lab [--frames N] [--seconds N] [--process-smoke]\n");
}

static int __xuiCmdLabParseArgs(xui_terminal_cmd_lab_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--process-smoke") == 0 ) {
			pDemo->bProcessSmoke = 1;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiCmdLabUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xuiCmdLabAsciiLower(int c)
{
	return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

static int __xuiCmdLabStrIcmp(const char* a, const char* b)
{
	unsigned char ca;
	unsigned char cb;

	if ( a == NULL ) a = "";
	if ( b == NULL ) b = "";
	while ( *a != '\0' || *b != '\0' ) {
		ca = (unsigned char)__xuiCmdLabAsciiLower((unsigned char)*a);
		cb = (unsigned char)__xuiCmdLabAsciiLower((unsigned char)*b);
		if ( ca != cb ) return (int)ca - (int)cb;
		if ( *a != '\0' ) a++;
		if ( *b != '\0' ) b++;
	}
	return 0;
}

static int __xuiCmdLabStartsWithI(const char* sText, const char* sPrefix)
{
	while ( *sPrefix != '\0' ) {
		if ( __xuiCmdLabAsciiLower((unsigned char)*sText) != __xuiCmdLabAsciiLower((unsigned char)*sPrefix) ) return 0;
		sText++;
		sPrefix++;
	}
	return 1;
}

static char* __xuiCmdLabTrim(char* sText)
{
	char* sEnd;

	if ( sText == NULL ) return NULL;
	while ( *sText != '\0' && isspace((unsigned char)*sText) ) sText++;
	sEnd = sText + strlen(sText);
	while ( sEnd > sText && isspace((unsigned char)sEnd[-1]) ) {
		sEnd--;
		*sEnd = '\0';
	}
	return sText;
}

static void __xuiCmdLabCopy(char* sDst, int iCapacity, const char* sSrc)
{
	if ( sDst == NULL || iCapacity <= 0 ) return;
	if ( sSrc == NULL ) sSrc = "";
	snprintf(sDst, (size_t)iCapacity, "%s", sSrc);
}

static const char* __xuiCmdLabFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\NotoSansMonoCJKsc-Regular.otf",
		"C:\\Windows\\Fonts\\SarasaMonoSC-Regular.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf",
		"C:\\Windows\\Fonts\\NotoSansSC-VF.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\Deng.ttf",
		"C:\\Windows\\Fonts\\CascadiaMono.ttf",
		"C:\\Windows\\Fonts\\consola.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static void __xuiCmdLabWrite(xui_terminal_cmd_lab_t* pDemo, const char* sText)
{
	if ( pDemo == NULL || pDemo->pTerminal == NULL || sText == NULL ) return;
	(void)xuiTerminalWriteText(pDemo->pTerminal, sText);
}

static void __xuiCmdLabWriteBytes(xui_terminal_cmd_lab_t* pDemo, const void* pBytes, int iSize)
{
	if ( pDemo == NULL || pDemo->pTerminal == NULL || pBytes == NULL || iSize <= 0 ) return;
	(void)xuiTerminalWrite(pDemo->pTerminal, pBytes, iSize);
}

static int __xuiCmdLabEncodeUtf8(uint32_t iCodepoint, char* sBuffer)
{
	if ( sBuffer == NULL ) return 0;
	if ( iCodepoint <= 0x7fu ) {
		sBuffer[0] = (char)iCodepoint;
		sBuffer[1] = '\0';
		return 1;
	}
	if ( iCodepoint <= 0x7ffu ) {
		sBuffer[0] = (char)(0xc0u | (iCodepoint >> 6));
		sBuffer[1] = (char)(0x80u | (iCodepoint & 0x3fu));
		sBuffer[2] = '\0';
		return 2;
	}
	if ( iCodepoint <= 0xffffu ) {
		sBuffer[0] = (char)(0xe0u | (iCodepoint >> 12));
		sBuffer[1] = (char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		sBuffer[2] = (char)(0x80u | (iCodepoint & 0x3fu));
		sBuffer[3] = '\0';
		return 3;
	}
	if ( iCodepoint <= 0x10ffffu ) {
		sBuffer[0] = (char)(0xf0u | (iCodepoint >> 18));
		sBuffer[1] = (char)(0x80u | ((iCodepoint >> 12) & 0x3fu));
		sBuffer[2] = (char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		sBuffer[3] = (char)(0x80u | (iCodepoint & 0x3fu));
		sBuffer[4] = '\0';
		return 4;
	}
	return 0;
}

static int __xuiCmdLabTerminalInputText(xui_terminal_cmd_lab_t* pDemo, const char* sText)
{
	if ( pDemo == NULL || pDemo->pTerminal == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiTerminalInputText(pDemo->pTerminal, sText);
}

static int __xuiCmdLabTerminalInputCodepoint(xui_terminal_cmd_lab_t* pDemo, uint32_t iCodepoint)
{
	char sText[8];
	int iSize;

	iSize = __xuiCmdLabEncodeUtf8(iCodepoint, sText);
	if ( iSize <= 0 ) return XUI_OK;
	return __xuiCmdLabTerminalInputText(pDemo, sText);
}

static void __xuiCmdLabPrintf(xui_terminal_cmd_lab_t* pDemo, const char* sFmt, ...)
{
	char sBuffer[2048];
	va_list args;

	if ( pDemo == NULL || sFmt == NULL ) return;
	va_start(args, sFmt);
	vsnprintf(sBuffer, sizeof(sBuffer), sFmt, args);
	va_end(args);
	__xuiCmdLabWrite(pDemo, sBuffer);
}

static void __xuiCmdLabPrompt(xui_terminal_cmd_lab_t* pDemo)
{
	__xuiCmdLabWrite(pDemo, "\x1b[1;32mxui\x1b[0m:\x1b[36m");
	__xuiCmdLabWrite(pDemo, pDemo->sCwd);
	__xuiCmdLabWrite(pDemo, "\x1b[0m$ ");
	(void)xuiTerminalFlush(pDemo->pTerminal);
}

static void __xuiCmdLabWinError(xui_terminal_cmd_lab_t* pDemo, const char* sAction)
{
	DWORD iError;
	char sMessage[512];
	char* sTrimmed;

	iError = GetLastError();
	sMessage[0] = '\0';
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, iError, 0, sMessage, (DWORD)sizeof(sMessage), NULL);
	sTrimmed = __xuiCmdLabTrim(sMessage);
	__xuiCmdLabPrintf(pDemo, "%s failed: %lu", (sAction != NULL) ? sAction : "operation", (unsigned long)iError);
	if ( sTrimmed != NULL && sTrimmed[0] != '\0' ) {
		__xuiCmdLabPrintf(pDemo, " (%s)", sTrimmed);
	}
	__xuiCmdLabWrite(pDemo, "\r\n");
}

static void __xuiCmdLabPathFromArg(xui_terminal_cmd_lab_t* pDemo, const char* sArg, char* sOut, int iCapacity)
{
	int iLen;

	if ( sOut == NULL || iCapacity <= 0 ) return;
	if ( sArg == NULL || sArg[0] == '\0' ) {
		__xuiCmdLabCopy(sOut, iCapacity, pDemo->sCwd);
		return;
	}
	if ( (strlen(sArg) >= 2 && sArg[1] == ':') || sArg[0] == '\\' || sArg[0] == '/' ) {
		__xuiCmdLabCopy(sOut, iCapacity, sArg);
		return;
	}
	__xuiCmdLabCopy(sOut, iCapacity, pDemo->sCwd);
	iLen = (int)strlen(sOut);
	if ( iLen < iCapacity - 1 ) {
		sOut[iLen++] = '\\';
		sOut[iLen] = '\0';
	}
	if ( iLen < iCapacity - 1 ) {
		strncat(sOut, sArg, (size_t)(iCapacity - iLen - 1));
	}
}

static int __xuiCmdLabRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_terminal_cmd_lab_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_terminal_cmd_lab_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 241, 250, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiCmdLabAddLabel(xui_terminal_cmd_lab_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget pParent, uint32_t iColor, uint32_t iFlags, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = iFlags | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pLabel, tRect);
	(void)xuiWidgetSetHitTestVisible(pLabel, 0);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static void __xuiCmdLabHistoryPush(xui_terminal_cmd_lab_t* pDemo, const char* sLine)
{
	if ( pDemo == NULL || sLine == NULL || sLine[0] == '\0' ) return;
	if ( pDemo->iHistoryCount > 0 && strcmp(pDemo->arrHistory[pDemo->iHistoryCount - 1], sLine) == 0 ) {
		pDemo->iHistoryCursor = -1;
		return;
	}
	if ( pDemo->iHistoryCount >= DEMO_HISTORY_MAX ) {
		memmove(pDemo->arrHistory[0], pDemo->arrHistory[1], sizeof(pDemo->arrHistory[0]) * (DEMO_HISTORY_MAX - 1));
		pDemo->iHistoryCount = DEMO_HISTORY_MAX - 1;
	}
	__xuiCmdLabCopy(pDemo->arrHistory[pDemo->iHistoryCount], DEMO_LINE_MAX, sLine);
	pDemo->iHistoryCount++;
	pDemo->iHistoryCursor = -1;
}

static void __xuiCmdLabReplaceLine(xui_terminal_cmd_lab_t* pDemo, const char* sLine)
{
	if ( pDemo == NULL ) return;
	if ( sLine == NULL ) sLine = "";
	__xuiCmdLabCopy(pDemo->sLine, DEMO_LINE_MAX, sLine);
	pDemo->iLineLen = (int)strlen(pDemo->sLine);
	__xuiCmdLabWrite(pDemo, "\r\x1b[2K");
	__xuiCmdLabPrompt(pDemo);
	__xuiCmdLabWrite(pDemo, pDemo->sLine);
	(void)xuiTerminalFlush(pDemo->pTerminal);
}

static void __xuiCmdLabHistoryPrev(xui_terminal_cmd_lab_t* pDemo)
{
	if ( pDemo == NULL || pDemo->iHistoryCount <= 0 ) return;
	if ( pDemo->iHistoryCursor < 0 ) {
		pDemo->iHistoryCursor = pDemo->iHistoryCount - 1;
	} else if ( pDemo->iHistoryCursor > 0 ) {
		pDemo->iHistoryCursor--;
	}
	__xuiCmdLabReplaceLine(pDemo, pDemo->arrHistory[pDemo->iHistoryCursor]);
}

static void __xuiCmdLabHistoryNext(xui_terminal_cmd_lab_t* pDemo)
{
	if ( pDemo == NULL || pDemo->iHistoryCount <= 0 || pDemo->iHistoryCursor < 0 ) return;
	pDemo->iHistoryCursor++;
	if ( pDemo->iHistoryCursor >= pDemo->iHistoryCount ) {
		pDemo->iHistoryCursor = -1;
		__xuiCmdLabReplaceLine(pDemo, "");
	} else {
		__xuiCmdLabReplaceLine(pDemo, pDemo->arrHistory[pDemo->iHistoryCursor]);
	}
}

static void __xuiCmdLabCompleteCommand(xui_terminal_cmd_lab_t* pDemo)
{
	static const char* arrCommands[] = {
		"help", "cls", "clear", "pwd", "cd", "dir", "type", "echo",
		"history", "ver", "ansi-test", "run", "cmd", "powershell", "exit"
	};
	const char* sMatch;
	int iMatchCount;
	int i;

	if ( pDemo == NULL || pDemo->iLineLen <= 0 || strchr(pDemo->sLine, ' ') != NULL ) return;
	sMatch = NULL;
	iMatchCount = 0;
	for ( i = 0; i < (int)(sizeof(arrCommands) / sizeof(arrCommands[0])); i++ ) {
		if ( __xuiCmdLabStartsWithI(arrCommands[i], pDemo->sLine) ) {
			sMatch = arrCommands[i];
			iMatchCount++;
		}
	}
	if ( iMatchCount == 1 && sMatch != NULL ) {
		__xuiCmdLabReplaceLine(pDemo, sMatch);
	}
}

static void __xuiCmdLabPrintHelp(xui_terminal_cmd_lab_t* pDemo)
{
	__xuiCmdLabWrite(pDemo,
		"Built-in commands:\r\n"
		"  help                  show this text\r\n"
		"  cls | clear           clear screen and scrollback\r\n"
		"  pwd                   show current directory\r\n"
		"  cd [path]             change current directory\r\n"
		"  dir [path]            list a directory\r\n"
		"  type <file>           print a text file\r\n"
		"  echo <text>           print text\r\n"
		"  history               print command history\r\n"
		"  ansi-test             print color and cursor rewrite samples\r\n"
		"  run <command line>    start a ConPTY-backed external process\r\n"
		"  cmd                   start interactive cmd.exe\r\n"
		"  powershell            start interactive PowerShell\r\n"
		"  exit                  close this example while in shell mode\r\n"
		"\r\n"
		"Unknown commands are treated as external command lines. Use exit inside\r\n"
		"cmd or PowerShell to return to the XUI shell.\r\n");
}

static void __xuiCmdLabCommandDir(xui_terminal_cmd_lab_t* pDemo, const char* sArg)
{
	WIN32_FIND_DATAA tData;
	HANDLE hFind;
	char sPath[DEMO_PATH_MAX];
	char sPattern[DEMO_PATH_MAX + 4];
	unsigned long long iSize;
	int iCount;

	__xuiCmdLabPathFromArg(pDemo, sArg, sPath, sizeof(sPath));
	snprintf(sPattern, sizeof(sPattern), "%s\\*", sPath);
	hFind = FindFirstFileA(sPattern, &tData);
	if ( hFind == INVALID_HANDLE_VALUE ) {
		__xuiCmdLabWinError(pDemo, "dir");
		return;
	}
	__xuiCmdLabPrintf(pDemo, "Directory of %s\r\n\r\n", sPath);
	iCount = 0;
	do {
		if ( strcmp(tData.cFileName, ".") == 0 || strcmp(tData.cFileName, "..") == 0 ) continue;
		if ( (tData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ) {
			__xuiCmdLabPrintf(pDemo, "  <DIR>          %s\r\n", tData.cFileName);
		} else {
			iSize = (((unsigned long long)tData.nFileSizeHigh) << 32) | (unsigned long long)tData.nFileSizeLow;
			__xuiCmdLabPrintf(pDemo, "  %12llu  %s\r\n", iSize, tData.cFileName);
		}
		iCount++;
	} while ( FindNextFileA(hFind, &tData) );
	FindClose(hFind);
	__xuiCmdLabPrintf(pDemo, "\r\n%d item(s)\r\n", iCount);
}

static void __xuiCmdLabCommandType(xui_terminal_cmd_lab_t* pDemo, const char* sArg)
{
	FILE* pFile;
	char sPath[DEMO_PATH_MAX];
	char sLine[1024];
	int iLines;

	if ( sArg == NULL || sArg[0] == '\0' ) {
		__xuiCmdLabWrite(pDemo, "type requires a file path.\r\n");
		return;
	}
	__xuiCmdLabPathFromArg(pDemo, sArg, sPath, sizeof(sPath));
	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) {
		__xuiCmdLabPrintf(pDemo, "type failed: cannot open %s\r\n", sPath);
		return;
	}
	iLines = 0;
	while ( fgets(sLine, sizeof(sLine), pFile) != NULL ) {
		char* sEnd = sLine + strlen(sLine);
		while ( sEnd > sLine && (sEnd[-1] == '\n' || sEnd[-1] == '\r') ) {
			sEnd--;
			*sEnd = '\0';
		}
		__xuiCmdLabWrite(pDemo, sLine);
		__xuiCmdLabWrite(pDemo, "\r\n");
		iLines++;
		if ( iLines >= 200 ) {
			__xuiCmdLabWrite(pDemo, "... output truncated after 200 lines\r\n");
			break;
		}
	}
	fclose(pFile);
}

static void __xuiCmdLabAnsiTest(xui_terminal_cmd_lab_t* pDemo)
{
	__xuiCmdLabWrite(pDemo, "\x1b[1;36mANSI rendering test\x1b[0m\r\n");
	__xuiCmdLabWrite(pDemo, "  \x1b[31mred\x1b[0m  \x1b[32mgreen\x1b[0m  \x1b[33myellow\x1b[0m  \x1b[34mblue\x1b[0m  \x1b[35mmagenta\x1b[0m  \x1b[36mcyan\x1b[0m\r\n");
	__xuiCmdLabWrite(pDemo, "  \x1b[38;5;196m256-color red\x1b[0m  \x1b[38;2;120;180;255mtrue-color sky\x1b[0m  \x1b[4munderline\x1b[0m  \x1b[7minverse\x1b[0m\r\n");
	__xuiCmdLabWrite(pDemo, "  rewrite: 0%");
	__xuiCmdLabWrite(pDemo, "\r  rewrite: 100%\r\n");
}

static void __xuiCmdLabUpdateCwd(xui_terminal_cmd_lab_t* pDemo)
{
	DWORD iLen;

	iLen = GetCurrentDirectoryA((DWORD)sizeof(pDemo->sCwd), pDemo->sCwd);
	if ( iLen == 0 || iLen >= sizeof(pDemo->sCwd) ) {
		__xuiCmdLabCopy(pDemo->sCwd, sizeof(pDemo->sCwd), ".");
	}
}

static void __xuiCmdLabProcessResize(xui_terminal_session_t* pSession, int iColumns, int iRows, void* pUser)
{
	xui_terminal_cmd_lab_t* pDemo = (xui_terminal_cmd_lab_t*)pUser;

	(void)pSession;
	if ( pDemo != NULL ) {
		pDemo->iResizeCount++;
		pDemo->iLastCols = iColumns;
		pDemo->iLastRows = iRows;
	}
}

static int __xuiCmdLabStartProcess(xui_terminal_cmd_lab_t* pDemo, const char* sCommandLine)
{
	xui_terminal_process_desc_t tDesc;
	xui_terminal_session_t* pSession;
	int iCols;
	int iRows;

	if ( pDemo == NULL || sCommandLine == NULL || sCommandLine[0] == '\0' ) return 0;
	if ( pDemo->pSession != NULL ) {
		__xuiCmdLabWrite(pDemo, "A process is already attached.\r\n");
		return 0;
	}
	iCols = xuiTerminalGetColumns(pDemo->pTerminal);
	iRows = xuiTerminalGetRows(pDemo->pTerminal);
	if ( iCols <= 0 ) iCols = 100;
	if ( iRows <= 0 ) iRows = 28;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sCommandLine = sCommandLine;
	tDesc.sWorkingDirectory = pDemo->sCwd;
	tDesc.iFlags = XUI_TERMINAL_PROCESS_CONPTY;
	tDesc.iColumns = iCols;
	tDesc.iRows = iRows;
	tDesc.onResize = __xuiCmdLabProcessResize;
	tDesc.pResizeUser = pDemo;
	pSession = xuiTerminalCreateProcessSession(&tDesc);
	if ( pSession == NULL ) {
		__xuiCmdLabPrintf(pDemo, "Cannot start external process: %s\r\n", sCommandLine);
		return 0;
	}
	__xuiCmdLabPrintf(pDemo, "\x1b[90m[start process: %s]\x1b[0m\r\n", sCommandLine);
	if ( xuiTerminalAttachSession(pDemo->pTerminal, pSession) != XUI_OK ) {
		xuiTerminalSessionDestroy(pSession);
		__xuiCmdLabWrite(pDemo, "Cannot attach process session.\r\n");
		return 0;
	}
	pDemo->pSession = pSession;
	pDemo->iMode = XUI_CMD_LAB_PROCESS;
	pDemo->iProcessCount++;
	(void)xuiTerminalFlush(pDemo->pTerminal);
	return 1;
}

static void __xuiCmdLabFinishProcessIfDone(xui_terminal_cmd_lab_t* pDemo)
{
	if ( pDemo == NULL || pDemo->iMode != XUI_CMD_LAB_PROCESS || pDemo->pSession == NULL ) return;
	(void)xuiTerminalSessionPoll(pDemo->pSession);
	if ( xuiTerminalSessionIsRunning(pDemo->pSession) ) return;
	(void)xuiTerminalSessionPoll(pDemo->pSession);
	(void)xuiTerminalDetachSession(pDemo->pTerminal);
	xuiTerminalSessionDestroy(pDemo->pSession);
	pDemo->pSession = NULL;
	pDemo->iMode = XUI_CMD_LAB_SHELL;
	__xuiCmdLabWrite(pDemo, "\r\n\x1b[90m[process exited]\x1b[0m\r\n");
	__xuiCmdLabPrompt(pDemo);
}

static void __xuiCmdLabExecuteLine(xui_terminal_cmd_lab_t* pDemo, const char* sLine)
{
	char sWork[DEMO_LINE_MAX];
	char* sCommand;
	char* sArg;

	if ( pDemo == NULL || sLine == NULL ) return;
	__xuiCmdLabCopy(sWork, sizeof(sWork), sLine);
	sCommand = __xuiCmdLabTrim(sWork);
	if ( sCommand == NULL || sCommand[0] == '\0' ) {
		__xuiCmdLabPrompt(pDemo);
		return;
	}
	__xuiCmdLabHistoryPush(pDemo, sCommand);
	pDemo->iCommandCount++;
	sArg = sCommand;
	while ( *sArg != '\0' && !isspace((unsigned char)*sArg) ) sArg++;
	if ( *sArg != '\0' ) {
		*sArg = '\0';
		sArg++;
	}
	sArg = __xuiCmdLabTrim(sArg);

	if ( __xuiCmdLabStrIcmp(sCommand, "help") == 0 ) {
		__xuiCmdLabPrintHelp(pDemo);
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "cls") == 0 || __xuiCmdLabStrIcmp(sCommand, "clear") == 0 ) {
		(void)xuiTerminalClearScrollback(pDemo->pTerminal);
		__xuiCmdLabWrite(pDemo, "\x1b[2J\x1b[H");
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "pwd") == 0 ) {
		__xuiCmdLabPrintf(pDemo, "%s\r\n", pDemo->sCwd);
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "cd") == 0 ) {
		char sPath[DEMO_PATH_MAX];
		char sFull[DEMO_PATH_MAX];
		if ( sArg == NULL || sArg[0] == '\0' ) {
			__xuiCmdLabPrintf(pDemo, "%s\r\n", pDemo->sCwd);
		} else {
			__xuiCmdLabPathFromArg(pDemo, sArg, sPath, sizeof(sPath));
			if ( GetFullPathNameA(sPath, (DWORD)sizeof(sFull), sFull, NULL) == 0 || !SetCurrentDirectoryA(sFull) ) {
				__xuiCmdLabWinError(pDemo, "cd");
			} else {
				__xuiCmdLabUpdateCwd(pDemo);
			}
		}
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "dir") == 0 ) {
		__xuiCmdLabCommandDir(pDemo, sArg);
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "type") == 0 ) {
		__xuiCmdLabCommandType(pDemo, sArg);
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "echo") == 0 ) {
		__xuiCmdLabWrite(pDemo, (sArg != NULL) ? sArg : "");
		__xuiCmdLabWrite(pDemo, "\r\n");
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "history") == 0 ) {
		int i;
		for ( i = 0; i < pDemo->iHistoryCount; i++ ) {
			__xuiCmdLabPrintf(pDemo, "%2d  %s\r\n", i + 1, pDemo->arrHistory[i]);
		}
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "ver") == 0 ) {
		__xuiCmdLabWrite(pDemo, "XUI Terminal Command Lab\r\n");
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "ansi-test") == 0 ) {
		__xuiCmdLabAnsiTest(pDemo);
		__xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "run") == 0 ) {
		if ( sArg == NULL || sArg[0] == '\0' ) {
			__xuiCmdLabWrite(pDemo, "run requires a command line.\r\n");
			__xuiCmdLabPrompt(pDemo);
		} else if ( !__xuiCmdLabStartProcess(pDemo, sArg) ) {
			__xuiCmdLabPrompt(pDemo);
		}
	} else if ( __xuiCmdLabStrIcmp(sCommand, "cmd") == 0 ) {
		if ( !__xuiCmdLabStartProcess(pDemo, "cmd.exe /d /q") ) __xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "powershell") == 0 || __xuiCmdLabStrIcmp(sCommand, "pwsh") == 0 ) {
		if ( !__xuiCmdLabStartProcess(pDemo, "powershell.exe -NoLogo") ) __xuiCmdLabPrompt(pDemo);
	} else if ( __xuiCmdLabStrIcmp(sCommand, "exit") == 0 ) {
		xgeQuit();
	} else {
		if ( !__xuiCmdLabStartProcess(pDemo, sLine) ) {
			__xuiCmdLabPrompt(pDemo);
		}
	}
	(void)xuiTerminalFlush(pDemo->pTerminal);
}

static int __xuiCmdLabPrevUtf8Bytes(const char* sText, int iLen)
{
	int i;

	if ( sText == NULL || iLen <= 0 ) return 0;
	i = iLen - 1;
	while ( i > 0 && (((unsigned char)sText[i] & 0xc0u) == 0x80u) ) i--;
	return iLen - i;
}

static void __xuiCmdLabBackspace(xui_terminal_cmd_lab_t* pDemo)
{
	int iRemove;

	if ( pDemo == NULL || pDemo->iLineLen <= 0 ) return;
	iRemove = __xuiCmdLabPrevUtf8Bytes(pDemo->sLine, pDemo->iLineLen);
	if ( iRemove <= 0 || iRemove > pDemo->iLineLen ) iRemove = 1;
	pDemo->iLineLen -= iRemove;
	pDemo->sLine[pDemo->iLineLen] = '\0';
	__xuiCmdLabWrite(pDemo, "\b \b");
}

static void __xuiCmdLabHandleShellInput(xui_terminal_cmd_lab_t* pDemo, const uint8_t* pData, int iSize)
{
	int i;

	if ( pDemo == NULL || pData == NULL || iSize <= 0 ) return;
	for ( i = 0; i < iSize; i++ ) {
		uint8_t c = pData[i];
		if ( c == 0x1bu && i + 2 < iSize && pData[i + 1] == '[' ) {
			if ( pData[i + 2] == 'A' ) {
				__xuiCmdLabHistoryPrev(pDemo);
			} else if ( pData[i + 2] == 'B' ) {
				__xuiCmdLabHistoryNext(pDemo);
			}
			i += 2;
		} else if ( c == 0x03u ) {
			pDemo->sLine[0] = '\0';
			pDemo->iLineLen = 0;
			__xuiCmdLabWrite(pDemo, "^C\r\n");
			__xuiCmdLabPrompt(pDemo);
		} else if ( c == 0x0cu ) {
			(void)xuiTerminalClearScrollback(pDemo->pTerminal);
			__xuiCmdLabWrite(pDemo, "\x1b[2J\x1b[H");
			__xuiCmdLabPrompt(pDemo);
		} else if ( c == '\r' || c == '\n' ) {
			char sExecute[DEMO_LINE_MAX];
			__xuiCmdLabWrite(pDemo, "\r\n");
			__xuiCmdLabCopy(sExecute, sizeof(sExecute), pDemo->sLine);
			pDemo->sLine[0] = '\0';
			pDemo->iLineLen = 0;
			pDemo->iHistoryCursor = -1;
			if ( c == '\r' && i + 1 < iSize && pData[i + 1] == '\n' ) i++;
			__xuiCmdLabExecuteLine(pDemo, sExecute);
		} else if ( c == '\t' ) {
			__xuiCmdLabCompleteCommand(pDemo);
		} else if ( c == 0x7fu || c == 0x08u ) {
			__xuiCmdLabBackspace(pDemo);
		} else if ( c >= 0x20u ) {
			if ( pDemo->iLineLen < DEMO_LINE_MAX - 5 ) {
				pDemo->sLine[pDemo->iLineLen++] = (char)c;
				pDemo->sLine[pDemo->iLineLen] = '\0';
				__xuiCmdLabWriteBytes(pDemo, &c, 1);
			}
		}
	}
	(void)xuiTerminalFlush(pDemo->pTerminal);
}

static void __xuiCmdLabInput(xui_widget pWidget, const uint8_t* pData, int iSize, void* pUser)
{
	xui_terminal_cmd_lab_t* pDemo = (xui_terminal_cmd_lab_t*)pUser;

	(void)pWidget;
	if ( pDemo == NULL || pData == NULL || iSize <= 0 ) return;
	pDemo->iInputBytes += iSize;
	if ( pDemo->iMode == XUI_CMD_LAB_SHELL ) {
		__xuiCmdLabHandleShellInput(pDemo, pData, iSize);
	}
}

static void __xuiCmdLabResize(xui_widget pWidget, int iColumns, int iRows, void* pUser)
{
	xui_terminal_cmd_lab_t* pDemo = (xui_terminal_cmd_lab_t*)pUser;

	(void)pWidget;
	if ( pDemo != NULL ) {
		pDemo->iResizeCount++;
		pDemo->iLastCols = iColumns;
		pDemo->iLastRows = iRows;
		if ( pDemo->pSession != NULL ) {
			(void)xuiTerminalSessionResize(pDemo->pSession, iColumns, iRows);
		}
	}
}

static int __xuiCmdLabCreateUi(xui_terminal_cmd_lab_t* pDemo)
{
	xui_terminal_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCmdLabRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiCmdLabAddLabel(pDemo, "XUI Terminal Command Lab", (xui_rect_t){48.0f, 38.0f, 260.0f, 24.0f}, pDemo->pRoot, XUI_COLOR_RGBA(45, 58, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.iColumns = 102;
	tDesc.iRows = 28;
	tDesc.iScrollbackLimit = 1024;
	tDesc.fCellWidth = 0.0f;
	tDesc.fCellHeight = 16.0f;
	tDesc.fPadding = 8.0f;
	tDesc.iBackgroundColor = XUI_COLOR_RGBA(14, 20, 29, 255);
	tDesc.iForegroundColor = XUI_COLOR_RGBA(222, 234, 246, 255);
	tDesc.iCursorColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	iRet = xuiTerminalCreate(pDemo->pContext, &pDemo->pTerminal, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pTerminal, (xui_rect_t){48.0f, 72.0f, 804.0f, 420.0f});
	(void)xuiTerminalSetInputCallback(pDemo->pTerminal, __xuiCmdLabInput, pDemo);
	(void)xuiTerminalSetResizeCallback(pDemo->pTerminal, __xuiCmdLabResize, pDemo);
	(void)xuiTerminalSetParseBudget(pDemo->pTerminal, 65536);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTerminal);
	if ( iRet != XUI_OK ) return iRet;

	__xuiCmdLabWrite(pDemo, "\x1b[1;36mXUI Terminal Command Lab\x1b[0m\r\n");
	__xuiCmdLabWrite(pDemo, "This example validates terminal input, built-in shell commands, and ConPTY process sessions.\r\n");
	__xuiCmdLabWrite(pDemo, "Try: help, dir, type <file>, ansi-test, cmd, powershell, or run <command line>.\r\n\r\n");
	__xuiCmdLabPrompt(pDemo);

	iRet = __xuiCmdLabAddLabel(pDemo, "mode=shell input=0 commands=0 processes=0 cols=0 rows=0 changes=0", (xui_rect_t){48.0f, 506.0f, 804.0f, 22.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, &pDemo->pStatus);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pTerminal);
	return XUI_OK;
}

static int __xuiCmdLabCreateAssets(xui_terminal_cmd_lab_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiCmdLabFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	__xuiCmdLabUpdateCwd(pDemo);
	pDemo->iHistoryCursor = -1;
	pDemo->iMode = XUI_CMD_LAB_SHELL;
	return __xuiCmdLabCreateUi(pDemo);
}

static void __xuiCmdLabDestroyAssets(xui_terminal_cmd_lab_t* pDemo)
{
	if ( pDemo->pSession != NULL ) {
		(void)xuiTerminalDetachSession(pDemo->pTerminal);
		xuiTerminalSessionDestroy(pDemo->pSession);
		pDemo->pSession = NULL;
	}
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static uint32_t __xuiCmdLabReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static uint32_t __xuiCmdLabReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiCmdLabPointInRect(float fX, float fY, xui_rect_t tRect)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static void __xuiCmdLabFocusTerminalAt(xui_terminal_cmd_lab_t* pDemo, float fX, float fY)
{
	xui_rect_t tRect;

	if ( pDemo == NULL || pDemo->pTerminal == NULL ) return;
	tRect = xuiWidgetGetWorldRect(pDemo->pTerminal);
	if ( __xuiCmdLabPointInRect(fX, fY, tRect) ) {
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pTerminal);
	}
}

static int __xuiCmdLabMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_BACKSPACE: return XUI_KEY_BACKSPACE;
	case XGE_KEY_DELETE: return XUI_KEY_DELETE;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_PAGE_UP: return XUI_KEY_PAGE_UP;
	case XGE_KEY_PAGE_DOWN: return XUI_KEY_PAGE_DOWN;
	case XGE_KEY_ESCAPE: return XUI_KEY_ESCAPE;
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	case XGE_KEY_F10: return XUI_KEY_F10;
	default: return 0;
	}
}

static uint32_t __xuiCmdLabShiftedDigit(int iKey)
{
	static const char sShifted[] = ")!@#$%^&*(";
	if ( iKey >= '0' && iKey <= '9' ) return (uint32_t)sShifted[iKey - '0'];
	return 0u;
}

static uint32_t __xuiCmdLabAsciiFromKey(int iKey, uint32_t iModifiers)
{
	int bShift;

	bShift = ((iModifiers & XUI_MOD_SHIFT) != 0u);
	if ( iKey >= 'A' && iKey <= 'Z' ) {
		return (uint32_t)(bShift ? iKey : (iKey - 'A' + 'a'));
	}
	if ( iKey >= '0' && iKey <= '9' ) {
		return bShift ? __xuiCmdLabShiftedDigit(iKey) : (uint32_t)iKey;
	}
	switch ( iKey ) {
	case XGE_KEY_SPACE: return ' ';
	case '-': return bShift ? '_' : '-';
	case '=': return bShift ? '+' : '=';
	case '[': return bShift ? '{' : '[';
	case ']': return bShift ? '}' : ']';
	case '\\': return bShift ? '|' : '\\';
	case ';': return bShift ? ':' : ';';
	case '\'': return bShift ? '"' : '\'';
	case ',': return bShift ? '<' : ',';
	case '.': return bShift ? '>' : '.';
	case '/': return bShift ? '?' : '/';
	case '`': return bShift ? '~' : '`';
	default: return 0u;
	}
}

static int __xuiCmdLabSendTerminalKey(xui_terminal_cmd_lab_t* pDemo, int iKey, uint32_t iModifiers)
{
	char sSeq[8];
	uint8_t b;

	if ( pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( ((iModifiers & XUI_MOD_CTRL) != 0u) && iKey >= 'A' && iKey <= 'Z' ) {
		b = (uint8_t)(iKey - 'A' + 1);
		sSeq[0] = (char)b;
		sSeq[1] = '\0';
		return __xuiCmdLabTerminalInputText(pDemo, sSeq);
	}
	switch ( iKey ) {
	case XGE_KEY_ENTER:
		return __xuiCmdLabTerminalInputText(pDemo, "\r");
	case XGE_KEY_TAB:
		return __xuiCmdLabTerminalInputText(pDemo, "\t");
	case XGE_KEY_BACKSPACE:
		return __xuiCmdLabTerminalInputText(pDemo, ((iModifiers & XUI_MOD_CTRL) != 0u) ? "\x08" : "\x7f");
	case XGE_KEY_LEFT:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[D");
	case XGE_KEY_RIGHT:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[C");
	case XGE_KEY_UP:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[A");
	case XGE_KEY_DOWN:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[B");
	case XGE_KEY_HOME:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[H");
	case XGE_KEY_END:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[F");
	case XGE_KEY_PAGE_UP:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[5~");
	case XGE_KEY_PAGE_DOWN:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[6~");
	case XGE_KEY_DELETE:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b[3~");
	case XGE_KEY_ESCAPE:
		return __xuiCmdLabTerminalInputText(pDemo, "\x1b");
	default:
		return XUI_OK;
	}
}

static int __xuiCmdLabSendPrintableFallback(xui_terminal_cmd_lab_t* pDemo, uint32_t iModifiers)
{
	static const int arrKeys[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		XGE_KEY_SPACE, '-', '=', '[', ']', '\\', ';', '\'', ',', '.', '/', '`'
	};
	uint32_t iText;
	int iRet;
	int i;

	if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) != 0u ) return XUI_OK;
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		if ( !xgeKeyPressed(arrKeys[i]) ) continue;
		iText = __xuiCmdLabAsciiFromKey(arrKeys[i], iModifiers);
		if ( iText == 0u ) continue;
		iRet = __xuiCmdLabTerminalInputCodepoint(pDemo, iText);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCmdLabSendKeys(xui_terminal_cmd_lab_t* pDemo)
{
	static const int arrControlKeys[] = {
		'A', 'C', 'D', 'L', 'Q', 'V'
	};
	static const int arrSpecialKeys[] = {
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_PAGE_UP,
		XGE_KEY_PAGE_DOWN,
		XGE_KEY_ESCAPE,
		XGE_KEY_MENU,
		XGE_KEY_F10
	};
	uint32_t iModifiers;
	uint32_t iText;
	int bTextDelivered;
	int iKey;
	int iRet;
	int i;

	iModifiers = __xuiCmdLabReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) && pDemo->iMode == XUI_CMD_LAB_SHELL && pDemo->iLineLen == 0 ) {
		xgeQuit();
		return XUI_OK;
	}
	for ( i = 0; i < (int)(sizeof(arrSpecialKeys) / sizeof(arrSpecialKeys[0])); i++ ) {
		iKey = __xuiCmdLabMapKey(arrSpecialKeys[i]);
		if ( iKey == 0 ) iKey = arrSpecialKeys[i];
		if ( xgeKeyPressed(arrSpecialKeys[i]) ) {
			if ( arrSpecialKeys[i] == XGE_KEY_MENU || arrSpecialKeys[i] == XGE_KEY_F10 ) {
				iRet = xuiInputKeyDown(pDemo->pContext, iKey, iModifiers);
				if ( iRet != XUI_OK ) return iRet;
			} else {
				iRet = __xuiCmdLabSendTerminalKey(pDemo, arrSpecialKeys[i], iModifiers);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		if ( xgeKeyReleased(arrSpecialKeys[i]) ) {
			iRet = xuiInputKeyUp(pDemo->pContext, iKey, iModifiers);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	bTextDelivered = 0;
	while ( (iText = xgeTextGet()) != 0 ) {
		if ( ((iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u) || (iText >= 0x20u) ) {
			iRet = __xuiCmdLabTerminalInputCodepoint(pDemo, iText);
			if ( iRet != XUI_OK ) return iRet;
			bTextDelivered = 1;
		}
	}
	if ( !bTextDelivered && (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) != 0u ) {
		for ( i = 0; i < (int)(sizeof(arrControlKeys) / sizeof(arrControlKeys[0])); i++ ) {
			if ( xgeKeyPressed(arrControlKeys[i]) ) {
				iRet = __xuiCmdLabSendTerminalKey(pDemo, arrControlKeys[i], iModifiers);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( xgeKeyReleased(arrControlKeys[i]) ) {
				iRet = xuiInputKeyUp(pDemo->pContext, arrControlKeys[i], iModifiers);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	if ( !bTextDelivered ) {
		iRet = __xuiCmdLabSendPrintableFallback(pDemo, iModifiers);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCmdLabHandleInput(xui_terminal_cmd_lab_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	float fUiX;
	float fUiY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	fUiX = fX - DEMO_OFFSET_X;
	fUiY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiCmdLabReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fUiX, fUiY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, fUiX, fUiY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0u ) {
		__xuiCmdLabFocusTerminalAt(pDemo, fUiX, fUiY);
		iRet = xuiInputPointerDown(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0u ) {
		__xuiCmdLabFocusTerminalAt(pDemo, fUiX, fUiY);
		iRet = xuiInputPointerDown(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0u ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0u ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0u ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0u ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiCmdLabSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiCmdLabRunChecks(xui_terminal_cmd_lab_t* pDemo, int bAutoRun)
{
	if ( pDemo == NULL || !bAutoRun || pDemo->bAutoDone ) return;
	if ( pDemo->iAutoStep == 0 ) {
		(void)xuiTerminalInputText(pDemo->pTerminal, "help\r");
		pDemo->iAutoStep++;
	} else if ( pDemo->iAutoStep == 1 ) {
		(void)xuiTerminalInputText(pDemo->pTerminal, "echo scripted input\r");
		pDemo->iAutoStep++;
	} else if ( pDemo->iAutoStep == 2 ) {
		(void)xuiTerminalInputText(pDemo->pTerminal, "ansi-test\r");
		pDemo->iAutoStep++;
	} else if ( pDemo->iAutoStep == 3 ) {
		if ( pDemo->bProcessSmoke ) {
			(void)xuiTerminalInputText(pDemo->pTerminal, "run cmd.exe /d /c echo xui-terminal-cmd-lab\r");
			pDemo->iAutoProcessFrame = pDemo->iFrame;
			pDemo->iAutoStep++;
		} else {
			pDemo->bDynamicOK = (pDemo->iCommandCount >= 3 && pDemo->iInputBytes > 0 && xuiTerminalGetChangeCount(pDemo->pTerminal) > 0);
			pDemo->bAutoDone = 1;
		}
	} else if ( pDemo->iAutoStep == 4 ) {
		if ( pDemo->iMode == XUI_CMD_LAB_SHELL && pDemo->iProcessCount > 0 ) {
			pDemo->bDynamicOK = (pDemo->iCommandCount >= 4 && pDemo->iInputBytes > 0 && pDemo->iProcessCount > 0 && xuiTerminalGetChangeCount(pDemo->pTerminal) > 0);
			pDemo->bAutoDone = 1;
		} else if ( pDemo->pSession != NULL && pDemo->iFrame - pDemo->iAutoProcessFrame > 180 ) {
			(void)xuiTerminalSessionTerminate(pDemo->pSession);
		}
	}
}

static void __xuiCmdLabUpdateStatus(xui_terminal_cmd_lab_t* pDemo)
{
	char sText[256];
	const char* sMode;

	if ( pDemo == NULL || pDemo->pStatus == NULL ) return;
	sMode = (pDemo->iMode == XUI_CMD_LAB_PROCESS) ? "process" : "shell";
	snprintf(sText, sizeof(sText), "mode=%s input=%d commands=%d processes=%d resize=%d cols=%d rows=%d changes=%d",
		sMode,
		pDemo->iInputBytes,
		pDemo->iCommandCount,
		pDemo->iProcessCount,
		pDemo->iResizeCount,
		xuiTerminalGetColumns(pDemo->pTerminal),
		xuiTerminalGetRows(pDemo->pTerminal),
		xuiTerminalGetChangeCount(pDemo->pTerminal));
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiCmdLabFrame(void* pUser)
{
	xui_terminal_cmd_lab_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_terminal_cmd_lab_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiCmdLabHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCmdLabRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCmdLabFinishProcessIfDone(pDemo);
	__xuiCmdLabUpdateStatus(pDemo);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bLayoutOK = (xuiTerminalGetColumns(pDemo->pTerminal) > 0 && xuiTerminalGetRows(pDemo->pTerminal) > 0);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_terminal_cmd_lab final-summary frames=%d create=%d layout=%d dynamic=%d mode=%d commands=%d processes=%d cols=%d rows=%d input=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDynamicOK, (int)pDemo->iMode,
			pDemo->iCommandCount, pDemo->iProcessCount,
			xuiTerminalGetColumns(pDemo->pTerminal), xuiTerminalGetRows(pDemo->pTerminal),
			pDemo->iInputBytes, xuiTerminalGetChangeCount(pDemo->pTerminal),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_terminal_cmd_lab_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCmdLabParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCmdLabUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Terminal Command Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_terminal_cmd_lab: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCmdLabCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_terminal_cmd_lab: create assets failed: %d\n", iRet);
		__xuiCmdLabDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiCmdLabFrame, &tDemo);
	__xuiCmdLabDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bDynamicOK)) ? 0 : 1;
}
