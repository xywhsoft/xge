#ifndef XGE3D_MOTION_INTERNAL_H
#define XGE3D_MOTION_INTERNAL_H
/* Offline-only XRT modules, independent of xge.dll's runtime profile. */
#define XRT_MODULE_ARRAY
#define XRT_MODULE_CODEC_BASE64
#define XRT_MODULE_FILE_WHOLE
#define XRT_MODULE_JSON
#include "../lib/xrt/xrt.h"
#include "../lib/ufbx/ufbx.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MOTION_MAX_NODES 2048
#define MOTION_MAX_BYTES (256u*1024u*1024u)
typedef struct motion_node { char *name;size_t parent;ufbx_transform rest;int channels[6],channel_count; } motion_node;
typedef struct motion_asset { xarray nodes,times,poses;char *name; } motion_asset;
typedef struct motion_options { const char *clip;double fps,start,end,unit;int z_up,list,reference; } motion_options;

void motion_init(motion_asset *a);
void motion_free(motion_asset *a);
char *motion_string(const char *s);
int motion_load(const char *path,const motion_options *options,motion_asset *a);
int motion_write(const char *path,const motion_asset *a);
int motion_retarget(motion_asset *a,const char *target,const char *mapping);
#endif
