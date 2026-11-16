#include "xge.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "xge_data_lifetime_internal.h"

static bool __xgeBackendOwnerValid(const xge_callback_owner_t* owner)
{
    return (!owner->tReference.Data && !owner->tReference.Ops && !owner->onHold && !owner->onDrop) ||
        (owner->tReference.Data && owner->tReference.Ops && owner->onHold && owner->onDrop);
}

#define BR_BODY xge_platform_backend_ref_t
#define BR_HANDLE xge_platform_backend_ref
#define BR_DESCRIPTOR xge_platform_backend_t
#define BR_FN(name) __xgePlatformBackend##name
#define BR_PUBLIC(name) xgePlatformBackend##name
#include "xge_backend_owned.inl"
#undef BR_BODY
#undef BR_HANDLE
#undef BR_DESCRIPTOR
#undef BR_FN
#undef BR_PUBLIC

#define BR_BODY xge_graphics_backend_ref_t
#define BR_HANDLE xge_graphics_backend_ref
#define BR_DESCRIPTOR xge_graphics_backend_t
#define BR_FN(name) __xgeGraphicsBackend##name
#define BR_PUBLIC(name) xgeGraphicsBackend##name
#include "xge_backend_owned.inl"
#undef BR_BODY
#undef BR_HANDLE
#undef BR_DESCRIPTOR
#undef BR_FN
#undef BR_PUBLIC
