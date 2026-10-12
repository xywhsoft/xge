/*
 * af_dialogs.h - Dialog windows
 */

#ifndef AF_DIALOGS_H
#define AF_DIALOGS_H

#include "../core/af_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dialog types */
#define AF_DIALOG_NONE              0
#define AF_DIALOG_NEW_DOC           1
#define AF_DIALOG_PROJECT_SETTINGS  2
#define AF_DIALOG_EXPORT            3
#define AF_DIALOG_ABOUT             4

/* Export types */
#define AF_EXPORT_XANIM         0
#define AF_EXPORT_PNG_SEQUENCE  1
#define AF_EXPORT_GIF           2
#define AF_EXPORT_SPRITE_SHEET  3

void afDialogOpenNewDocument(af_app_t* pApp);
void afDialogOpenProjectSettings(af_app_t* pApp);
void afDialogOpenExport(af_app_t* pApp);
void afDialogOpenAbout(af_app_t* pApp);
void afDialogClose(void);
int  afDialogIsActive(void);

void afDialogRender(af_app_t* pApp, xui_proxy_t* pProxy, xui_draw_context pCtx, float fScreenW, float fScreenH);
int  afDialogMouseDown(af_app_t* pApp, float fX, float fY, float fScreenW, float fScreenH);
int  afDialogKeyDown(af_app_t* pApp, int iKey);

#ifdef __cplusplus
}
#endif

#endif /* AF_DIALOGS_H */
