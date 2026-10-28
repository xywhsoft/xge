#ifndef XUI_CHOICE_INTERNAL_H
#define XUI_CHOICE_INTERNAL_H

#include "xui.h"

#if XUI_ENABLE_RADIO_GROUP
int xuiInternalRadioGroupMoveSelection(xui_widget pWidget, int iDirection);
int xuiInternalRadioGroupRemoveOption(xui_widget pGroup, xui_widget pOption, int bNotify);
int xuiInternalRadioGroupSyncTabStops(xui_widget pGroup);
void xuiInternalRadioGroupOptionParentChanged(xui_widget pWidget,
	xui_widget pOldParent, xui_widget pNewParent);
int xuiInternalCheckCardNotifyFromGroup(xui_widget pWidget);
#endif

#endif
