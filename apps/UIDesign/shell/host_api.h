#ifndef UID_SHELL_API_H
#define UID_SHELL_API_H
#include "uidesign_xui.h"
/* This is the example host's business-code contract, NOT a component API. */
typedef struct shell_services {
    size_t size;
    void *user;
    void (*log)(const char *, void *user);
    void *view;
    xui_widget (*find)(void *view, const char *control_name);
} shell_services;
typedef void (*shell_invoke_proc)(const uid_runtime_event *, const shell_services *);
#endif
