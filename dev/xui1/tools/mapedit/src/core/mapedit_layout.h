#ifndef MAPEDIT_LAYOUT_H
#define MAPEDIT_LAYOUT_H

#include "mapedit_app.h"

int MapEditLayoutBuild(mapedit_app_t* pApp);
int MapEditLayoutLoadUser(mapedit_app_t* pApp);
int MapEditLayoutLoadDefault(mapedit_app_t* pApp);
int MapEditLayoutSaveUser(mapedit_app_t* pApp);
int MapEditLayoutReset(mapedit_app_t* pApp);

#endif
