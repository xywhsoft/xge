#ifndef XGE_TRANSFER_INTERNAL_API_H
#define XGE_TRANSFER_INTERNAL_API_H
#include <stdint.h>
/* Pure metadata. Zero means no native Sokol UI callback thread is active. */
uint64_t xgeInternalTransferThreadId(void);
#endif
