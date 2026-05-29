#include "uid_app.h"
#include <string.h>

void UIDesignRefreshInit(uid_refresh_state_t* pRefresh)
{
	if ( pRefresh != NULL ) {
		memset(pRefresh, 0, sizeof(*pRefresh));
	}
}

void UIDesignInvalidate(uid_app_t* pApp, int iReason)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->tRefresh.iPendingReasons |= (uint32_t)iReason;
	xgeXuiRefreshRequest(&pApp->tXui);
	xgeRenderRequest();
}

void UIDesignInvalidateInspector(uid_app_t* pApp)
{
	UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_INSPECTOR);
}

void UIDesignInvalidateStructure(uid_app_t* pApp)
{
	UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_STRUCTURE);
}

void UIDesignBeginContinuousRefresh(uid_app_t* pApp, int iReason)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->tRefresh.iContinuousReasons |= (uint32_t)iReason;
	UIDesignInvalidate(pApp, iReason);
}

void UIDesignEndContinuousRefresh(uid_app_t* pApp, int iReason)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->tRefresh.iContinuousReasons &= ~((uint32_t)iReason);
}

void UIDesignRefreshAfterUpdate(uid_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->tRefresh.iPendingReasons = 0;
	if ( pApp->tRefresh.iContinuousReasons != 0 ) {
		xgeRenderRequest();
	}
}
