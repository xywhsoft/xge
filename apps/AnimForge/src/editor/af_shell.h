/*
 * af_shell.h - DockPanel / MenuBar / Toolbar / StatusBar
 */

#ifndef AF_SHELL_H
#define AF_SHELL_H

#include "../core/af_app.h"

#ifdef __cplusplus
extern "C" {
#endif

int  afShellCreate(af_app_t* pApp);
void afShellPopulateMenuBar(af_app_t* pApp);
void afShellPopulateToolbar(af_app_t* pApp);
void afShellRefreshToolbar(af_app_t* pApp);
void afShellUpdateStatusBar(af_app_t* pApp, char* sBuffer, int iBufSize);
int  afShellHandleMenuCommand(af_app_t* pApp, int iCmdId);
int  afShellHandleShortcut(af_app_t* pApp, int iKey, uint32_t iMods);

#ifdef __cplusplus
}
#endif

#endif /* AF_SHELL_H */
