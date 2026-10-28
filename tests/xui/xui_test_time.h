#ifndef XUI_TEST_TIME_H
#define XUI_TEST_TIME_H

#include "xrt.h"
#include <stdint.h>

typedef double xui_test_deadline_t;

static inline uint64_t xuiTestClockUs(void)
{
    double now = xrtTimer();
    return now > 0.0 ? (uint64_t)(now * 1000000.0) : 0;
}

static inline xui_test_deadline_t xuiTestDeadlineAfterUs(uint64_t timeout)
{
    return xrtTimer() + (double)timeout / 1000000.0;
}

static inline int xuiTestDeadlineExpired(xui_test_deadline_t deadline)
{
    return xrtTimer() >= deadline;
}

#endif
