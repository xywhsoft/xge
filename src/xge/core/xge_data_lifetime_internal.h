#ifndef XGE_DATA_LIFETIME_INTERNAL_H
#define XGE_DATA_LIFETIME_INTERNAL_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Private immutable type metadata. Each callback sees its actual concrete
 * Native payload; language callbacks are never erased into this table. */
typedef struct xge_data_lifetime_type_t {
    size_t iBodyBytes;
    size_t iRefsOffset;
    bool (*onReadable)(const void*);
    bool (*onTrace)(const void*,xrtownershipvisitor,void*);
    int (*onFinalize)(void*,bool*);
    int (*onFinish)(void*);
} xge_data_lifetime_type_t;
typedef struct xge_data_lifetime_t {
    const xge_data_lifetime_type_t* pType;
    uint64_t iCreator;
    const void* pClaim;
    atomic_uint iState;
    atomic_uint iActivity;
} xge_data_lifetime_t;
enum {
    XGE_DATA_FINALIZING=1u,XGE_DATA_FINALIZED=2u,XGE_DATA_CLEARED=4u,
    XGE_DATA_FINISHING=8u,XGE_DATA_FINISHED=16u,XGE_DATA_CONSUMING=32u
};
#define XGE_DATA_LOCK UINT32_C(0x80000000)
typedef struct xge_data_outcome_t {int iStatus;xerror* pError;} xge_data_outcome_t;
#if UINTPTR_MAX==UINT64_MAX
_Static_assert(sizeof(xge_data_lifetime_t)==32,"DataObject x64 lifetime accounting changed");
#endif
#endif
