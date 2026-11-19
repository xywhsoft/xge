#ifndef XUI_CODE_COMMAND_INTERNAL_H
#define XUI_CODE_COMMAND_INTERNAL_H

#include "xui.h"

int xuiInternalCodeCommandMapClaimOwner(xui_code_command_map map, xui_widget editor);
void xuiInternalCodeCommandMapDestroyOwned(xui_code_command_map map, xui_widget editor);

#endif
