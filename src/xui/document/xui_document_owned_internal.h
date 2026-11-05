#ifndef XUI_DOCUMENT_OWNED_INTERNAL_H
#define XUI_DOCUMENT_OWNED_INTERNAL_H
#include "xui_document.h"
#include <stdatomic.h>
#include <stddef.h>

struct doc_allocator;
typedef struct doc_model_type doc_model_type;
/* Embedded beside the existing real reference counter. ops is immutable and
 * NULL for every legacy/raw origin. No cold promotion of an allocator family.
 * claim is serialized by the XRT Mutation/Freeze protocol; state/activity are
 * atomic so pure readiness can observe an outside-Freeze semantic tail. */
typedef struct doc_model_lifetime {
    const doc_model_type* ops;
    uint64_t creator;
    const void* claim;
    atomic_uint state;
    atomic_uint activity;
} doc_model_lifetime;
typedef struct doc_model_prefix {
    atomic_uint refs;
    doc_model_lifetime owned;
} doc_model_prefix;
#define DOC_MODEL_ASSERT_LAYOUT(Type) \
    _Static_assert(offsetof(Type,refs)==0,"Document model ref must be the actual first member"); \
    _Static_assert(offsetof(Type,owned)==offsetof(doc_model_prefix,owned),"Document model lifetime offset mismatch")

enum {
    DOC_MODEL_ALLOCATED=1u,       /* doc_alloc control; physical free uses its existing header */
    DOC_MODEL_ALLOCATOR_OWNER=2u, /* Document/Prepare already own their allocator until physical free */
    DOC_MODEL_CREATOR=4u,
    DOC_MODEL_UNIQUE=8u
};
enum {
    DOC_MODEL_FINALIZING=1u,DOC_MODEL_FINALIZED=2u,DOC_MODEL_CLEARED=4u,
    DOC_MODEL_FINISHING=8u,DOC_MODEL_FINISHED=16u,DOC_MODEL_OWNER_CONSUMED=32u
};
#define DOC_MODEL_LOCK UINT32_C(0x80000000)
struct doc_model_type {
    unsigned flags;
    bool (*trace)(const void*,xrtownershipvisitor,void*);
    int (*ready)(const void*);
    int (*finalize)(void*,bool* complete);
    int (*finish)(void*);
};
typedef struct doc_model_call {
    const void* self;
    doc_model_lifetime* root;
    xerror* ambient;
} doc_model_call;
typedef struct doc_model_outcome {int status;xerror* error;} doc_model_outcome;

extern const doc_model_type doc_model_document_type,doc_model_snapshot_type;
extern const doc_model_type doc_model_change_type,doc_model_transaction_type;
extern const doc_model_type doc_model_prepare_type,doc_model_fragment_type,doc_model_index_type;
void doc_model_init(doc_model_lifetime*,const doc_model_type*,struct doc_allocator*);
bool doc_model_owned(const doc_model_lifetime*);
static inline bool doc_model_typed(const doc_model_lifetime* root,const doc_model_type* type)
{return root && root->ops==type;}
int doc_model_share_acquire(doc_model_lifetime*,const doc_model_type*);
#define DOC_MODEL_SHARE_FUNCTION(Name,Handle,Kind) \
    XUI_API int Name(Handle source,Handle* output) { \
        if(!output)return XUI_ERROR_INVALID_ARGUMENT; \
        *output=NULL; \
        int status=source ? doc_model_share_acquire(&source->owned,&Kind):XUI_ERROR_INVALID_ARGUMENT; \
        if(status==XUI_OK)*output=source; \
        return status; \
    }
int doc_model_admit(const doc_model_lifetime*);
bool doc_model_readable(const doc_model_lifetime*);
int doc_model_add_ref(doc_model_lifetime*);
int doc_model_release(doc_model_lifetime*,bool* consumed);
xrtownershipref doc_model_reference(const doc_model_lifetime*);
int doc_model_call_begin(doc_model_lifetime*,doc_model_call*);
int doc_model_call_end(doc_model_call*,int);
bool doc_model_mutation_begin(const doc_model_lifetime*,xrtownershipscope*);
bool doc_allocator_mutation_begin(const struct doc_allocator*,xrtownershipscope*);
void doc_model_mutation_end(xrtownershipscope*);
void doc_model_outcome_begin(doc_model_outcome*,int);
void doc_model_outcome_add(doc_model_outcome*,int);
int doc_model_outcome_end(doc_model_outcome*);
int doc_model_owner_drop(const xui_callback_owner_t*);
/* Native UI's real model slot cleanup must preserve a negative/NULL outcome
 * from an unobserved notification latch. Raw C resources keep their legacy
 * lifetime path; this helper never promotes their provenance. */
static inline int doc_document_release(xui_document document)
{
    if(!document)return XUI_OK;
    if(xuiDocumentIsOwned(document))return xuiDocumentReleaseChecked(document);
    xuiDocumentRelease(document);return XUI_OK;
}
static inline int doc_document_hold(xui_document document)
{
    if(!document)return XUI_ERROR_INVALID_ARGUMENT;
    if(xuiDocumentIsOwned(document))return xuiDocumentAddRef(document);
    xuiDocumentRetain(document);return XUI_OK;
}

/* Built-in Native observers are non-owning backlinks. Only these explicit
 * private call sites may install them into a managed Document. */
int doc_subscribe_native(xui_document,xui_doc_change_owned_proc,void*,uint64_t*);
void doc_unsubscribe_native(xui_document,uint64_t);
#endif
