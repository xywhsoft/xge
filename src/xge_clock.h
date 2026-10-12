#ifndef XGE_CLOCK_H
#define XGE_CLOCK_H

#include "../lib/xrt/xrt_config.h"
#include "../lib/xrt/xrt.h"

/* Internal budgets and document histories use microseconds; XRT uses seconds. */
static inline uint64_t xgeClockMicroseconds(void)
{
    return (uint64_t)(xrtTimer() * 1000000.0);
}

#endif
