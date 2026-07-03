#include "uid_backend_events.h"
#include <string.h>

void UIDesignBackendDispatcherInit(uid_backend_dispatcher_t* pDispatcher)
{
	if ( pDispatcher != NULL ) {
		memset(pDispatcher, 0, sizeof(*pDispatcher));
	}
}

int UIDesignBackendDispatch(uid_backend_dispatcher_t* pDispatcher, const uid_backend_event_t* pEvent)
{
	if ( (pDispatcher == NULL) || (pDispatcher->procDispatch == NULL) || (pEvent == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return pDispatcher->procDispatch(pEvent, pDispatcher->pUser);
}
