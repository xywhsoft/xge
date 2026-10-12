#ifndef XGE_IME_OWNER_INTERNAL_H
#define XGE_IME_OWNER_INTERNAL_H
#include <stdint.h>

/* Private provenance for the XGE/XUI bridge; public event ABI stays unchanged. */
void xgeInternalImeSetFocusOwner(uint64_t owner);
void xgeInternalImeSetSnapshotOwner(uint64_t owner);
uint64_t xgeInternalInputImeOwner(void);

#endif
