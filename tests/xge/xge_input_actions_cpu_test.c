/* Production actions and XRT, deterministic CPU input; no window/device. */
#define XRT_MODULE_ARRAY
#define XRT_MODULE_MEMORY_DEBUG
#define XRT_IMPLEMENTATION
#include "extlibs/xrt/xrt.h"
#include "xge.h"
int xgeInputKeyConsumed(int key){(void)key;return 0;}
int xgeKeyDown(int key){return key==1;}
int xgeMouseDown(int button){(void)button;return 0;}
int xgeGamepadConnected(int device){(void)device;return 0;}
int xgeGamepadButtonDown(int device,uint32_t button){(void)device;(void)button;return 0;}
float xgeGamepadAxis(int device,int axis){(void)device;(void)axis;return 0;}
#include "src/xge/core/xge_input_actions.c"
#include "tests/xge/test_input_actions.c"
