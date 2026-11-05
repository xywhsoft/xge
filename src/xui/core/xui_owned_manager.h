#ifndef XUI_OWNED_MANAGER_INTERNAL_H
#define XUI_OWNED_MANAGER_INTERNAL_H
#include "xui.h"

typedef struct xui_owned_manager_t xui_owned_manager_t;
typedef struct xui_owned_manager_ops_t {
    bool (*Trace)(const void*,xrtownershipvisitor,void*);
    int (*Ready)(const void*);
    /* Outside Freeze. complete is independent from the checked result;
     * DEFERRED keeps every unconsumed cache owner in its actual slot. */
    int (*Prepare)(void*,bool* complete);
    /* Close detaches/request-destroys widget parts without freeing storage.
     * Finalize runs only after all queued native widget tails have returned;
     * every result consumes manager storage and its remaining semantic duties. */
    void (*Close)(void*);
    int (*Finalize)(void*);
} xui_owned_manager_ops_t;
struct xui_owned_manager_t {
    xui_owned_manager_t* previous;
    xui_owned_manager_t* next;
    xui_owned_manager_t* pendingNext;
    xui_context context;
    const xui_owned_manager_ops_t* ops;
    void* data;
    xui_lifetime_link_t* watches;
    uint64_t creatorThread;
    unsigned active;
    bool registered,pending,queued,closing;
};
typedef struct xui_manager_scope_t {
    const void* self;
    xui_owned_manager_t* manager;
    xui_call_scope_t contextScope;
} xui_manager_scope_t;
typedef struct xui_manager_callback_update_t {
    xui_manager_scope_t scope;
    xrtownershipscope mutation;
    xui_callback_owner_t previous;
    bool published;
} xui_manager_callback_update_t;
struct xui_callback_snapshot_t;

int xuiInternalManagerRegister(xui_context,xui_owned_manager_t*,const xui_owned_manager_ops_t*,void*);
int xuiInternalManagerScopeBegin(xui_owned_manager_t*,xui_manager_scope_t*);
int xuiInternalManagerScopeEnd(xui_manager_scope_t*,int);
/* Requires an existing protecting Context operation. Logical invalidation is
 * synchronous; physical cleanup waits for the outermost native tail. */
int xuiInternalManagerRequestClose(xui_owned_manager_t*);
int xuiInternalManagerWatch(xui_owned_manager_t*,xui_lifetime_link_t*,void(*)(void*),void*);
bool xuiInternalManagerAlive(const xui_owned_manager_t*);
/* Pure creator-thread read admission: no operation tail, retirement drain,
 * allocation or callback. Borrowed results remain valid until the next Native
 * operation that can modify/destroy their owning manager. */
int xuiInternalManagerReadStatus(const xui_owned_manager_t*);
/* Native typed function/user slots stay in their original manager fields.
 * Begin holds the candidate before publication; caller writes its exact typed
 * function/user while the gate is held, then Publish and End drop the old edge. */
int xuiInternalManagerCallbackUpdateBegin(xui_owned_manager_t*,xui_callback_owner_t*,bool,const xui_callback_owner_t*,xui_manager_callback_update_t*);
void xuiInternalManagerCallbackUpdatePublish(xui_manager_callback_update_t*);
int xuiInternalManagerCallbackUpdateEnd(xui_manager_callback_update_t*,int);
/* Caller already owns a whole-manager scope, and snapshots typed fn/user
 * before Hold can reenter. Report admission failures only for consumers that
 * cannot return a checked status; End uses the existing CallbackSnapshotEnd. */
int xuiInternalManagerCallbackSnapshotBegin(xui_owned_manager_t*,const xui_callback_owner_t*,struct xui_callback_snapshot_t*,bool);
bool xuiInternalContextTraceManagers(xui_context,xrtownershipvisitor,void*);
bool xuiInternalContextManagersReady(xui_context);
int xuiInternalContextPrepareManagers(xui_context,bool* complete);
/* Context calls Drain only after pending Widget destruction has drained, at
 * its operation tail. A nonzero operation depth protects Context during it.
 * DestroyAll runs while Context.bDestroying, before roots/all widgets. */
bool xuiInternalContextDrainManagers(xui_context);
void xuiInternalContextDestroyManagers(xui_context);
#endif
