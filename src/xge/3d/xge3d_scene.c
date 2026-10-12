#include "xge3d_internal.h"

#if XGE_ENABLE_3D

/* Scene mutation is serialized by the application, like the GPU context. */
static uint64_t d3_next_scene;

int xge3dSceneCreate(xge3d_scene **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (d3_next_scene == UINT64_MAX) return XGE_ERROR_INVALID_STATE;
    xge3d_scene *s = xrtMalloc(sizeof(*s));
    if (!s) return XGE_ERROR_OUT_OF_MEMORY;
    memset(s, 0, sizeof(*s));
    xrtSlotMapInit(&s->slots);
    xrtArrayInit(&s->nodes, sizeof(xge3d_node_data*));
    xrtArrayInit(&s->scratch, sizeof(xge3d_node_data*));
    s->id = ++d3_next_scene;
    s->version = 1;
    *out = s;
    return XGE_OK;
}

void xge3dSceneFree(xge3d_scene *s)
{
    if (!s) return;
    for (size_t i=0; i<s->nodes.Count; ++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes, i);
        xge3dMeshFree(n->mesh);
        d3_lods_free(n->lods);
        xge3dMaterialFree(n->material);
#if XGE3D_ENABLE_ANIMATION
        d3_skin_free(n->skin);
#endif
#if XGE3D_ENABLE_MODEL
        xge3dModelFree(n->model);
#endif
        xrtFree(n);
    }
    xrtArrayUnit(&s->nodes);
    xrtArrayUnit(&s->scratch);
    xrtSlotMapUnit(&s->slots);
    xrtFree(s);
}

size_t xge3dSceneNodeCount(const xge3d_scene *s)
{ return s ? s->nodes.Count : 0; }
xge3d_node_t xge3dSceneNodeAt(const xge3d_scene *s,size_t index)
{ return s && index<s->nodes.Count ? (*(xge3d_node_data**)xrtArrayConstGet(&s->nodes,index))->handle : (xge3d_node_t){0}; }
xge3d_dvec3_t xge3dSceneOrigin(const xge3d_scene *s) {return s ? s->origin : (xge3d_dvec3_t){0};}
int xge3dRelativePosition(xge3d_dvec3_t origin,xge3d_dvec3_t global,xge3d_vec3_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_vec3_t){0};
    if (!isfinite(origin.x) || !isfinite(origin.y) || !isfinite(origin.z) || !isfinite(global.x) || !isfinite(global.y) || !isfinite(global.z)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t position={(float)(global.x-origin.x),(float)(global.y-origin.y),(float)(global.z-origin.z)};
    if (!d3_finite3(position)) return XGE_ERROR_INVALID_ARGUMENT;
    *out=position;return XGE_OK;
}
void d3_set_local(xge3d_scene *s,xge3d_node_data *n,xge3d_mat4_t matrix)
{
    n->local=matrix;
    if (!n->parent.slot) n->global_position=(xge3d_dvec3_t){s->origin.x+matrix.m[12],s->origin.y+matrix.m[13],s->origin.z+matrix.m[14]};
    ++s->version;
}
int xge3dSceneSetOrigin(xge3d_scene *s,xge3d_dvec3_t origin)
{
    if (!s || !isfinite(origin.x) || !isfinite(origin.y) || !isfinite(origin.z)) return XGE_ERROR_INVALID_ARGUMENT;
    for (size_t i=0;i<s->nodes.Count;++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);xge3d_vec3_t p;
        if (!n->parent.slot && xge3dRelativePosition(origin,n->global_position,&p)!=XGE_OK) return XGE_ERROR_INVALID_ARGUMENT;
    }
    s->origin=origin;
    for (size_t i=0;i<s->nodes.Count;++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);xge3d_vec3_t p;
        if (!n->parent.slot) {xge3dRelativePosition(origin,n->global_position,&p);n->local.m[12]=p.x;n->local.m[13]=p.y;n->local.m[14]=p.z;}
    }
    ++s->version;return XGE_OK;
}
int xge3dNodeSetGlobalPosition(xge3d_scene *s,xge3d_node_t root,xge3d_dvec3_t position)
{
    xge3d_node_data *n=d3_node(s,root);if (!n || n->parent.slot) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t local;int result=xge3dRelativePosition(s->origin,position,&local);if (result!=XGE_OK) return result;
    xge3d_mat4_t matrix=n->local;matrix.m[12]=local.x;matrix.m[13]=local.y;matrix.m[14]=local.z;
    d3_set_local(s,n,matrix);n->global_position=position;return XGE_OK;
}
int xge3dCameraLookAtGlobal(xge3d_camera_t *camera,xge3d_dvec3_t origin,xge3d_dvec3_t eye,xge3d_dvec3_t target,xge3d_vec3_t up)
{
    xge3d_vec3_t e,t;
    if (xge3dRelativePosition(origin,eye,&e)!=XGE_OK || xge3dRelativePosition(origin,target,&t)!=XGE_OK) return XGE_ERROR_INVALID_ARGUMENT;
    return xge3dCameraLookAt(camera,e,t,up);
}

int xge3dNodeCreate(xge3d_scene *s, xge3d_node_t parent, xge3d_node_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out = (xge3d_node_t){0};
    if (!s || ((parent.slot || parent.scene) && !d3_node(s, parent)))
        return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_node_data *n = xrtMalloc(sizeof(*n));
    if (!n) return XGE_ERROR_OUT_OF_MEMORY;
    memset(n, 0, sizeof(*n));
    n->local = n->world = d3_identity();
    n->parent = parent;
    n->global_position=s->origin;
    n->visible = 1;
    for (int i=0; i<4; ++i) n->color[i]=1;
    n->handle = (xge3d_node_t){xrtSlotMapInsert(&s->slots, n), s->id};
    if (!n->handle.slot || !xrtArrayPush(&s->nodes, &n)) {
        if (n->handle.slot) xrtSlotMapRemove(&s->slots, n->handle.slot, NULL);
        xge3dMeshFree(n->mesh);
        xge3dMaterialFree(n->material);
#if XGE3D_ENABLE_ANIMATION
        d3_skin_free(n->skin);
#endif
#if XGE3D_ENABLE_MODEL
        xge3dModelFree(n->model);
#endif
        xrtFree(n);
        return XGE_ERROR_OUT_OF_MEMORY;
    }
    ++s->version;
    *out = n->handle;
    return XGE_OK;
}

int xge3dNodeSetParent(xge3d_scene *s, xge3d_node_t node, xge3d_node_t parent)
{
    xge3d_node_data *n = d3_node(s, node), *p = NULL;
    if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    if (parent.slot || parent.scene) {
        p = d3_node(s, parent);
        if (!p) return XGE_ERROR_INVALID_ARGUMENT;
    }
    for (; p; p = d3_node(s, p->parent))
        if (p == n) return XGE_ERROR_INVALID_ARGUMENT;
    n->parent = parent;
    if (!parent.slot) n->global_position=(xge3d_dvec3_t){s->origin.x+n->local.m[12],s->origin.y+n->local.m[13],s->origin.z+n->local.m[14]};
    ++s->version;
    return XGE_OK;
}

int xge3dNodeSetMatrix(xge3d_scene *s, xge3d_node_t node, const xge3d_mat4_t *matrix)
{
    xge3d_node_data *n = d3_node(s, node);
    if (!n || !d3_finite_matrix(matrix) || matrix->m[3] != 0 ||
        matrix->m[7] != 0 || matrix->m[11] != 0 || matrix->m[15] != 1)
        return XGE_ERROR_INVALID_ARGUMENT;
    d3_set_local(s,n,*matrix);
    return XGE_OK;
}

int xge3dTransformMatrix(const xge3d_transform_t *t, xge3d_mat4_t *out)
{
    if (!t || !out || !d3_finite3(t->position) || !d3_finite3(t->scale))
        return XGE_ERROR_INVALID_ARGUMENT;
    float x=t->rotation.x, y=t->rotation.y, z=t->rotation.z, w=t->rotation.w;
    float length=sqrtf(x*x+y*y+z*z+w*w);
    if (!isfinite(length) || length<1e-12f) return XGE_ERROR_INVALID_ARGUMENT;
    x/=length; y/=length; z/=length; w/=length;
    *out = (xge3d_mat4_t){{
        (1-2*(y*y+z*z))*t->scale.x, 2*(x*y+z*w)*t->scale.x, 2*(x*z-y*w)*t->scale.x, 0,
        2*(x*y-z*w)*t->scale.y, (1-2*(x*x+z*z))*t->scale.y, 2*(y*z+x*w)*t->scale.y, 0,
        2*(x*z+y*w)*t->scale.z, 2*(y*z-x*w)*t->scale.z, (1-2*(x*x+y*y))*t->scale.z, 0,
        t->position.x, t->position.y, t->position.z, 1}};
    return d3_finite_matrix(out) ? XGE_OK : XGE_ERROR_INVALID_ARGUMENT;
}

int xge3dNodeSetTransform(xge3d_scene *s, xge3d_node_t node, const xge3d_transform_t *t)
{
    xge3d_mat4_t m;
    int result = xge3dTransformMatrix(t, &m);
    return result == XGE_OK ? xge3dNodeSetMatrix(s, node, &m) : result;
}

int xge3dNodeGetWorldMatrix(xge3d_scene *s, xge3d_node_t node, xge3d_mat4_t *out)
{
    xge3d_node_data *n = d3_node(s, node), *p=n;
    if (!n || !out) return XGE_ERROR_INVALID_ARGUMENT;
    xrtArrayClear(&s->scratch);
    /* An explicit walk handles deep hierarchies without recursive C calls. */
    while (p && p->version != s->version) {
        if (!xrtArrayPush(&s->scratch, &p)) return XGE_ERROR_OUT_OF_MEMORY;
        p = d3_node(s, p->parent);
    }
    for (size_t i=s->scratch.Count; i>0; --i) {
        xge3d_node_data *child=*(xge3d_node_data**)xrtArrayGet(&s->scratch, i-1);
        child->world = p ? d3_mul(p->world, child->local) : child->local;
        child->version = s->version;
        p = child;
    }
    *out = n->world;
    return XGE_OK;
}

int xge3dNodeSetVisible(xge3d_scene *s, xge3d_node_t node, int visible)
{
    xge3d_node_data *n = d3_node(s, node);
    if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    n->visible = !!visible;
    return XGE_OK;
}

int xge3dNodeSetMesh(xge3d_scene *s, xge3d_node_t node, xge3d_mesh *mesh)
{
    xge3d_node_data *n=d3_node(s,node);
    if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    if (mesh && !d3_mesh_retain(mesh)) return XGE_ERROR_INVALID_STATE;
#if XGE3D_ENABLE_ANIMATION
    d3_skin_free(n->skin);n->skin=NULL;
#endif
    xge3dMeshFree(n->mesh);
    d3_lods_free(n->lods);n->lods=NULL;
    n->mesh=mesh;
    ++s->version;
    return XGE_OK;
}

void d3_lods_free(d3_lods *lods)
{
    if (!lods) return;
    for (size_t i=0;i<lods->count;++i) xge3dMeshFree(lods->levels[i].mesh);
    xrtFree(lods);
}
int xge3dNodeSetLods(xge3d_scene *s,xge3d_node_t node,const xge3d_lod_t *levels,size_t count)
{
    xge3d_node_data *n=d3_node(s,node);
    if (!n || count>3 || (count && (!levels || !n->mesh))) return XGE_ERROR_INVALID_ARGUMENT;
#if XGE3D_ENABLE_ANIMATION
    if (count && n->skin) return XGE_ERROR_UNSUPPORTED;
#endif
    float previous=0;
    for (size_t i=0;i<count;++i) {
        if (!levels[i].mesh || !isfinite(levels[i].min_distance) || levels[i].min_distance<=previous) return XGE_ERROR_INVALID_ARGUMENT;
        previous=levels[i].min_distance;
    }
    d3_lods *lods=NULL;
    if (count) {
        lods=xrtMalloc(sizeof(*lods));if (!lods) return XGE_ERROR_OUT_OF_MEMORY;
        memset(lods,0,sizeof(*lods));
        for (size_t i=0;i<count;++i) {
            if (!d3_mesh_retain(levels[i].mesh)) {d3_lods_free(lods);return XGE_ERROR_INVALID_STATE;}
            lods->levels[lods->count++]=levels[i];
        }
    }
    d3_lods_free(n->lods);n->lods=lods;++s->version;return XGE_OK;
}

int xge3dNodeSetColor(xge3d_scene *s, xge3d_node_t node, const float rgba[4])
{
    xge3d_node_data *n=d3_node(s,node);
    if (!n || !rgba) return XGE_ERROR_INVALID_ARGUMENT;
    for (int i=0; i<4; ++i) if (!isfinite(rgba[i]) || rgba[i]<0 || rgba[i]>1)
        return XGE_ERROR_INVALID_ARGUMENT;
    memcpy(n->color,rgba,sizeof(n->color));
    return XGE_OK;
}

int xge3dNodeSetMaterial(xge3d_scene *s, xge3d_node_t node, xge3d_material *material)
{
    xge3d_node_data *n=d3_node(s,node);
    if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    if (material && !d3_material_retain(material)) return XGE_ERROR_INVALID_STATE;
    xge3dMaterialFree(n->material); n->material=material;
    return XGE_OK;
}

int xge3dNodeDestroy(xge3d_scene *s, xge3d_node_t node)
{
    xge3d_node_data *root = d3_node(s, node);
    if (!root) return XGE_ERROR_INVALID_ARGUMENT;
    xrtArrayClear(&s->scratch);
    for (size_t i=0; i<s->nodes.Count; ++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes, i), *p=n;
        for (; p && p!=root; p=d3_node(s, p->parent)) {}
        if (p && !xrtArrayPush(&s->scratch, &n)) return XGE_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i=0; i<s->scratch.Count; ++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->scratch, i);
        xrtSlotMapRemove(&s->slots, n->handle.slot, NULL);
        /* Remove by pointer: ancestors can be released before their descendants. */
        for (size_t j=0; j<s->nodes.Count; ++j)
            if (*(xge3d_node_data**)xrtArrayGet(&s->nodes, j)==n) {
                xrtArrayRemoveSwap(&s->nodes, j);
                break;
            }
        xge3dMeshFree(n->mesh);
        d3_lods_free(n->lods);
        xge3dMaterialFree(n->material);
#if XGE3D_ENABLE_ANIMATION
        d3_skin_free(n->skin);
#endif
#if XGE3D_ENABLE_MODEL
        xge3dModelFree(n->model);
#endif
        xrtFree(n);
    }
    ++s->version;
    return XGE_OK;
}

int xge3dCameraPerspective(xge3d_camera_t *out, float fov, float aspect, float near_z, float far_z)
{
    if (!out || !isfinite(fov) || fov<=0 || fov>=3.14159265f ||
        !isfinite(aspect) || aspect<=0 || !isfinite(near_z) || near_z<=0 ||
        !isfinite(far_z) || far_z<=near_z) return XGE_ERROR_INVALID_ARGUMENT;
    float f=1/tanf(fov*0.5f);
    *out = (xge3d_camera_t){d3_identity(), {{f/aspect,0,0,0, 0,f,0,0,
        0,0,(far_z+near_z)/(near_z-far_z),-1,
        0,0,2*far_z*near_z/(near_z-far_z),0}}, near_z, far_z};
    return d3_finite_matrix(&out->projection) ? XGE_OK : XGE_ERROR_INVALID_ARGUMENT;
}

int xge3dCameraOrthographic(xge3d_camera_t *out, float width, float height, float near_z, float far_z)
{
    if (!out || !isfinite(width) || width<=0 || !isfinite(height) || height<=0 ||
        !isfinite(near_z) || near_z<0 || !isfinite(far_z) || far_z<=near_z)
        return XGE_ERROR_INVALID_ARGUMENT;
    *out = (xge3d_camera_t){d3_identity(), {{2/width,0,0,0, 0,2/height,0,0,
        0,0,-2/(far_z-near_z),0, 0,0,-(far_z+near_z)/(far_z-near_z),1}}, near_z, far_z};
    return d3_finite_matrix(&out->projection) ? XGE_OK : XGE_ERROR_INVALID_ARGUMENT;
}

int xge3dCameraLookAt(xge3d_camera_t *camera, xge3d_vec3_t eye, xge3d_vec3_t target, xge3d_vec3_t up)
{
    if (!camera || !d3_finite3(eye) || !d3_finite3(target) || !d3_finite3(up))
        return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t z=d3_sub(eye,target);
    if (!d3_normalize(&z)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t x=d3_cross(up,z);
    if (!d3_normalize(&x)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t y=d3_cross(z,x);
    camera->view = (xge3d_mat4_t){{x.x,y.x,z.x,0, x.y,y.y,z.y,0, x.z,y.z,z.z,0,
        -d3_dot(x,eye),-d3_dot(y,eye),-d3_dot(z,eye),1}};
    return XGE_OK;
}

#endif
