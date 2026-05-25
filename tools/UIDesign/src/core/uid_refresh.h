#ifndef UID_REFRESH_H
#define UID_REFRESH_H

#include "xge.h"

#define UID_INVALIDATE_REASON_BOOT        0x0001
#define UID_INVALIDATE_REASON_RESIZE      0x0002
#define UID_INVALIDATE_REASON_DOCUMENT    0x0004
#define UID_INVALIDATE_REASON_SELECTION   0x0008
#define UID_INVALIDATE_REASON_INSPECTOR   0x0010
#define UID_INVALIDATE_REASON_STRUCTURE   0x0020
#define UID_INVALIDATE_REASON_CANVAS      0x0040

typedef struct uid_app_t uid_app_t;

typedef struct uid_refresh_state_t {
	uint32_t iPendingReasons;
	uint32_t iContinuousReasons;
} uid_refresh_state_t;

void UIDesignRefreshInit(uid_refresh_state_t* pRefresh);
void UIDesignInvalidate(uid_app_t* pApp, int iReason);
void UIDesignInvalidateInspector(uid_app_t* pApp);
void UIDesignInvalidateStructure(uid_app_t* pApp);
void UIDesignBeginContinuousRefresh(uid_app_t* pApp, int iReason);
void UIDesignEndContinuousRefresh(uid_app_t* pApp, int iReason);
void UIDesignRefreshAfterUpdate(uid_app_t* pApp);

#endif
