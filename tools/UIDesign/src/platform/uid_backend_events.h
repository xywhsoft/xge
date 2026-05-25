#ifndef UID_BACKEND_EVENTS_H
#define UID_BACKEND_EVENTS_H

#include "xge.h"

typedef struct uid_backend_event_t {
	const char* sActionId;
	const char* sPageId;
	const char* sNodeId;
	const char* sEventName;
	xvalue pPayload;
} uid_backend_event_t;

typedef int (*uid_backend_dispatch_proc)(const uid_backend_event_t* pEvent, void* pUser);

typedef struct uid_backend_dispatcher_t {
	uid_backend_dispatch_proc procDispatch;
	void* pUser;
} uid_backend_dispatcher_t;

void UIDesignBackendDispatcherInit(uid_backend_dispatcher_t* pDispatcher);
int UIDesignBackendDispatch(uid_backend_dispatcher_t* pDispatcher, const uid_backend_event_t* pEvent);

#endif
