#ifndef XGE3D_INTERNAL_H
#define XGE3D_INTERNAL_H

#include "../xge.h"
#include "../lib/xrt/xrt.h"
#include <math.h>
#include <string.h>

typedef struct xge3d_node_data {
    xge3d_node_t handle, parent;
    xge3d_mat4_t local, world;
    uint64_t version;
    xge3d_dvec3_t global_position;
    xge3d_aabb_t bounds;
    uint64_t bounds_version, mesh_version;
    int visible;
    xge3d_mesh *mesh;
    struct d3_lods *lods;
    xge3d_material *material;
    float color[4];
#if XGE3D_ENABLE_ANIMATION
    struct d3_skin *skin;
#endif
#if XGE3D_ENABLE_LIGHTING
    int has_light;
    xge3d_light_desc_t light;
#endif
#if XGE3D_ENABLE_MODEL
    xge3d_model *model;
    xge3d_node_t instance_root;
    size_t source_node;
#endif
} xge3d_node_data;
typedef struct d3_lods { size_t count;xge3d_lod_t levels[3];uint64_t versions[3]; } d3_lods;
void d3_lods_free(d3_lods *lods);

struct xge3d_scene {
    uint64_t id, version;
    xge3d_dvec3_t origin;
    xslotmap slots;
    xarray nodes, scratch;
};

typedef struct d3_joint_bounds { xge3d_aabb_t box;int valid; } d3_joint_bounds;
struct xge3d_mesh {
    xarray vertices, indices;
#if XGE3D_ENABLE_ANIMATION
    xarray joint_bounds;
#endif
    xge3d_aabb_t bounds;
    uint32_t refs, vao, vbo, ibo;
    int index_bits;
    uint64_t context, version;
};
int d3_node_bounds(xge3d_scene *scene,xge3d_node_data *node,xge3d_aabb_t *out);
int d3_frustum_box(xge3d_mat4_t matrix,xge3d_aabb_t box);
void d3_set_local(xge3d_scene *scene,xge3d_node_data *node,xge3d_mat4_t matrix);
static inline void d3_bounds_point(xge3d_aabb_t *box,xge3d_vec3_t p)
{
    box->min=(xge3d_vec3_t){fminf(box->min.x,p.x),fminf(box->min.y,p.y),fminf(box->min.z,p.z)};
    box->max=(xge3d_vec3_t){fmaxf(box->max.x,p.x),fmaxf(box->max.y,p.y),fmaxf(box->max.z,p.z)};
}

static inline xge3d_vec3_t d3_add(xge3d_vec3_t a, xge3d_vec3_t b)
{ return (xge3d_vec3_t){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline xge3d_vec3_t d3_sub(xge3d_vec3_t a, xge3d_vec3_t b)
{ return (xge3d_vec3_t){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline xge3d_vec3_t d3_scale(xge3d_vec3_t a, float s)
{ return (xge3d_vec3_t){a.x*s, a.y*s, a.z*s}; }
static inline float d3_dot(xge3d_vec3_t a, xge3d_vec3_t b)
{ return a.x*b.x+a.y*b.y+a.z*b.z; }
static inline xge3d_vec3_t d3_cross(xge3d_vec3_t a, xge3d_vec3_t b)
{ return (xge3d_vec3_t){a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
static inline int d3_finite3(xge3d_vec3_t a)
{ return isfinite(a.x) && isfinite(a.y) && isfinite(a.z); }
static inline int d3_normalize(xge3d_vec3_t *a)
{
    float n = sqrtf(d3_dot(*a, *a));
    if (!isfinite(n) || n < 1e-12f) return 0;
    *a = d3_scale(*a, 1.0f/n);
    return 1;
}
static inline xge3d_mat4_t d3_identity(void)
{ return (xge3d_mat4_t){{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}}; }
static inline int d3_finite_matrix(const xge3d_mat4_t *m)
{
    if (!m) return 0;
    for (int i=0; i<16; ++i) if (!isfinite(m->m[i])) return 0;
    return 1;
}
static inline xge3d_mat4_t d3_mul(xge3d_mat4_t a, xge3d_mat4_t b)
{
    xge3d_mat4_t r = {{0}};
    for (int c=0; c<4; ++c) for (int row=0; row<4; ++row)
        for (int k=0; k<4; ++k) r.m[c*4+row] += a.m[k*4+row]*b.m[c*4+k];
    return r;
}
static inline xge3d_node_data *d3_node(xge3d_scene *s, xge3d_node_t h)
{ return s && h.scene == s->id ? xrtSlotMapGet(&s->slots, h.slot) : NULL; }

static inline xge3d_vec3_t d3_point(xge3d_mat4_t m, xge3d_vec3_t p)
{ return (xge3d_vec3_t){m.m[0]*p.x+m.m[4]*p.y+m.m[8]*p.z+m.m[12],
    m.m[1]*p.x+m.m[5]*p.y+m.m[9]*p.z+m.m[13],
    m.m[2]*p.x+m.m[6]*p.y+m.m[10]*p.z+m.m[14]}; }

/* Fixed-size matrix arithmetic needs no general container or allocation. */
static inline int d3_inverse(xge3d_mat4_t m, xge3d_mat4_t *out)
{
    double a[4][8];
    for (int r=0;r<4;++r) for (int c=0;c<8;++c)
        a[r][c]=c<4 ? m.m[c*4+r] : c-4==r;
    for (int c=0;c<4;++c) {
        int pivot=c;
        for (int r=c+1;r<4;++r) if (fabs(a[r][c])>fabs(a[pivot][c])) pivot=r;
        if (!isfinite(a[pivot][c]) || fabs(a[pivot][c])<1e-30) return 0;
        if (pivot!=c) for (int k=0;k<8;++k) {
            double t=a[c][k]; a[c][k]=a[pivot][k]; a[pivot][k]=t;
        }
        double scale=a[c][c];
        for (int k=0;k<8;++k) a[c][k]/=scale;
        for (int r=0;r<4;++r) if (r!=c) {
            double f=a[r][c];
            for (int k=0;k<8;++k) a[r][k]-=f*a[c][k];
        }
    }
    for (int r=0;r<4;++r) for (int c=0;c<4;++c) out->m[c*4+r]=(float)a[r][c+4];
    return d3_finite_matrix(out);
}

uint64_t __xge3dContext(void);
int d3_mesh_retain(xge3d_mesh *mesh);

struct xge3d_texture {
    xarray pixels;
    int width, height, srgb;
    xge3d_sampler_t sampler;
    uint32_t refs, id;
    uint64_t context;
};
struct xge3d_material {
    uint32_t refs;
    xge3d_material_desc_t desc;
};
int d3_texture_retain(xge3d_texture *texture);
int d3_texture_upload(xge3d_texture *texture, uint64_t *bytes);
int d3_material_retain(xge3d_material *material);
#if XGE3D_ENABLE_MODEL
struct cgltf_data *d3_model_data(const xge3d_model *model);
int d3_model_retain(xge3d_model *model);
typedef struct d3_model_io {
    int (*load)(const char *uri,xge_resource_t *out,void *user);
    int (*cancelled)(void *user);void *user;
} d3_model_io;
#if XGE3D_ENABLE_ASYNC
typedef struct d3_upload_cursor { size_t resource,offset;int stage;uint64_t uploaded,total,context; } d3_upload_cursor;
int d3_model_load_io(const char *uri,const d3_model_io *io,xge3d_model **out);
size_t d3_model_resource_count(const xge3d_model *model,int stage);
void *d3_model_resource(const xge3d_model *model,int stage,size_t index);
uint64_t d3_model_upload_bytes(const xge3d_model *model);
int d3_model_upload_step(xge3d_model *model,d3_upload_cursor *cursor,const xge3d_upload_budget_t *budget,
    xge3d_upload_stats_t *stats,uint64_t deadline,int *complete);
#endif
#endif
#if XGE3D_ENABLE_ANIMATION
#define D3_MAX_JOINTS 256
typedef struct d3_skin {
    xge3d_model *model;
    xarray joints, inverse_bind, palette;
    uint64_t version, pose_version, uploaded_version, context;
    uint32_t texture;
} d3_skin;
int d3_skin_create(xge3d_model *model,size_t index,const xarray *nodes,d3_skin **out);
void d3_skin_free(d3_skin *skin);
int d3_skin_update(xge3d_scene *scene,xge3d_node_data *node);
int d3_skin_upload(d3_skin *skin,uint64_t *bytes);
xge3d_vec3_t d3_skin_point(const d3_skin *skin,const xge3d_vertex_t *vertex);
#endif
typedef struct d3_cube {
    xarray pixels;
    int size, levels, srgb;
    uint32_t id;
    uint64_t context;
} d3_cube;
struct xge3d_environment {
    d3_cube sky;
#if XGE3D_ENABLE_IBL
    d3_cube irradiance, prefiltered;
    xge3d_texture *brdf;
#endif
};
int d3_cube_copy(d3_cube *cube,const xge3d_cube_level_t *levels,size_t count,int srgb);
void d3_cube_free(d3_cube *cube);
int d3_cube_upload(d3_cube *cube,uint64_t *bytes);
int d3_environment_upload(xge3d_environment *environment, uint64_t *bytes);
#if XGE3D_ENABLE_LIGHTING
typedef struct d3_light { xge3d_light_desc_t desc; xge3d_vec3_t position, direction; } d3_light;
#endif
#if XGE3D_ENABLE_SHADOW
typedef struct d3_shadow_frame {
    xge3d_mat4_t matrices[6];
    float splits[4];
    int light_map[8], count, sun, cascades;
} d3_shadow_frame;
int d3_shadow_build(const xge3d_camera_t *camera, const xge3d_shadow_settings_t *settings,
    const d3_light *lights, int count, xge3d_dvec3_t origin, d3_shadow_frame *out);
#endif

#endif
