#include "xge3d_internal.h"
#include "xge_clock.h"

#if XGE_ENABLE_3D

/* Scene mutation is serialized by the application, like the GPU context. */
static uint64_t d3_next_scene;

void d3_detach(xge3d_scene *s,xge3d_node_data *n)
{
    (void)s;
    if(n->previous_sibling)n->previous_sibling->next_sibling=n->next_sibling;
    else if(n->parent_node)n->parent_node->first_child=n->next_sibling;
    if(n->next_sibling)n->next_sibling->previous_sibling=n->previous_sibling;
    n->parent_node=n->next_sibling=n->previous_sibling=NULL;n->parent=(xge3d_node_t){0};
}
void d3_attach(xge3d_scene *s,xge3d_node_data *n,xge3d_node_data *parent)
{
    (void)s;n->parent_node=parent;n->parent=parent?parent->handle:(xge3d_node_t){0};
    if(parent){n->next_sibling=parent->first_child;if(n->next_sibling)n->next_sibling->previous_sibling=n;parent->first_child=n;}
}
static xge3d_node_data *d3_subtree_after(xge3d_node_data *n,xge3d_node_data *root)
{
    while(n!=root&&!n->next_sibling)n=n->parent_node;
    return n==root?NULL:n->next_sibling;
}
xge3d_node_data *d3_subtree_next(xge3d_node_data *n,xge3d_node_data *root)
{return n->first_child?n->first_child:d3_subtree_after(n,root);}
void d3_spatial_dirty(xge3d_scene *s,xge3d_node_data *n)
{
#if XGE3D_ENABLE_COLLISION
    d3_collision_dirty(s,n);
#endif
    if(n->spatial_leaf<0||n->spatial_queued||s->spatial_rebuild)return;
    if(!xrtArrayPush(&s->spatial_pending,&n->handle)){s->spatial_rebuild=1;return;}
    n->spatial_queued=1;
}
void d3_node_dirty(xge3d_scene *s,xge3d_node_data *root)
{
    for(xge3d_node_data *n=root;n;){
        if(n->world_dirty){n=d3_subtree_after(n,root);continue;}
        n->world_dirty=1;n->bounds_version=0;d3_spatial_dirty(s,n);
#if XGE3D_ENABLE_ANIMATION
        if(n->skin)n->skin->dirty=1;
        for(size_t i=0;i<n->skin_users.Count;++i){d3_skin *skin=*(d3_skin**)xrtArrayGet(&n->skin_users,i);skin->dirty=1;
            xge3d_node_data *owner=xrtSlotMapGet(&s->slots,skin->owner.slot);if(owner){owner->bounds_version=0;d3_spatial_dirty(s,owner);}}
#endif
        n=d3_subtree_next(n,root);
    }
}
static void d3_destroy_one(xge3d_scene *s,xge3d_node_data *n)
{
#if XGE3D_ENABLE_SCENE
    xrtFree(n->name);
#endif
#if XGE3D_ENABLE_COLLISION
    s->collision_rebuild=1;
    if(n->collider){xge3dMeshFree(n->collider->mesh);xrtFree(n->collider);}
#endif
    d3_detach(s,n);s->spatial_rebuild=1;
#if XGE3D_ENABLE_ANIMATION
    for(size_t i=0;i<n->skin_users.Count;++i){d3_skin *skin=*(d3_skin**)xrtArrayGet(&n->skin_users,i);skin->dirty=1;
        xge3d_node_data *owner=xrtSlotMapGet(&s->slots,skin->owner.slot);if(owner){owner->bounds_version=0;d3_spatial_dirty(s,owner);}}
    d3_skin_free(n->skin);xrtArrayUnit(&n->skin_users);
#endif
    if(n->retiring)--s->retiring_count;
    xrtSlotMapRemove(&s->slots,n->handle.slot,NULL);
    size_t at=n->dense_index,last=s->nodes.Count-1;
    if(at!=last){xge3d_node_data *moved=*(xge3d_node_data**)xrtArrayGet(&s->nodes,last);moved->dense_index=at;}
    xrtArrayRemoveSwap(&s->nodes,at);
    xge3dMeshFree(n->mesh);d3_lods_free(n->lods);xge3dMaterialFree(n->material);
#if XGE3D_ENABLE_MODEL
    xrtArrayUnit(&n->instance_nodes);xge3dModelFree(n->model);
#endif
    ++s->stats.destroyed_nodes;xrtFree(n);++s->version;
}

#if XGE3D_ENABLE_SHADOW
int xge3dNodeSetShadowFlags(xge3d_scene *s,xge3d_node_t h,uint32_t flags)
{
    xge3d_node_data *n=d3_node(s,h);
    if(!n||(flags&~(XGE3D_SHADOW_CAST|XGE3D_SHADOW_RECEIVE)))return XGE_ERROR_INVALID_ARGUMENT;
    n->shadow_flags=flags;return XGE_OK;
}
int xge3dNodeGetShadowFlags(xge3d_scene *s,xge3d_node_t h,uint32_t *out)
{
    xge3d_node_data *n=d3_node(s,h);if(!n||!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=n->shadow_flags;return XGE_OK;
}
#endif
int xge3dSceneCreate(xge3d_scene **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (d3_next_scene == UINT64_MAX) return XGE_ERROR_INVALID_STATE;
    xge3d_scene *s = xrtMalloc(sizeof(*s));
    if (!s) return XGE_ERROR_OUT_OF_MEMORY;
    memset(s, 0, sizeof(*s));
#if XGE3D_ENABLE_COLLISION
    xrtArrayInit(&s->collision_tree,sizeof(d3_spatial_branch));xrtArrayInit(&s->collision_items,sizeof(d3_spatial_item));
    xrtArrayInit(&s->collision_pending,sizeof(xge3d_node_t));s->collision_rebuild=1;s->collision_root=-1;
#endif
    xrtSlotMapInit(&s->slots);
    xrtArrayInit(&s->nodes, sizeof(xge3d_node_data*));
    xrtArrayInit(&s->scratch, sizeof(xge3d_node_data*));
    xrtArrayInit(&s->spatial_tree,sizeof(d3_spatial_branch));xrtArrayInit(&s->spatial_items,sizeof(d3_spatial_item));
    xrtArrayInit(&s->spatial_pending,sizeof(xge3d_node_t));xrtArrayInit(&s->query_stack,sizeof(int));
    s->spatial_root=-1;s->spatial_rebuild=1;
    s->id = ++d3_next_scene;
    s->version = 1;
    *out = s;
    return XGE_OK;
}

void xge3dSceneFree(xge3d_scene *s)
{
    if (!s) return;
    while(s->nodes.Count){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,s->nodes.Count-1);
        while(n->first_child){n=n->first_child;}d3_destroy_one(s,n);}
    xrtArrayUnit(&s->nodes);
    xrtArrayUnit(&s->scratch);
    xrtArrayUnit(&s->spatial_tree);xrtArrayUnit(&s->spatial_items);xrtArrayUnit(&s->spatial_pending);xrtArrayUnit(&s->query_stack);
    xrtSlotMapUnit(&s->slots);
#if XGE3D_ENABLE_COLLISION
    xrtArrayUnit(&s->collision_tree);xrtArrayUnit(&s->collision_items);xrtArrayUnit(&s->collision_pending);
#endif
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
    /* A public root-local assignment reanchors its double translation even
     * when the float matrix is unchanged. SetGlobalPosition overrides it. */
    if (!n->parent.slot) n->global_position=(xge3d_dvec3_t){s->origin.x+matrix.m[12],s->origin.y+matrix.m[13],s->origin.z+matrix.m[14]};
    if(!memcmp(&n->local,&matrix,sizeof(matrix)))return;
    d3_node_dirty(s,n);
    n->local=matrix;
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
        if (!n->parent.slot) {d3_node_dirty(s,n);xge3dRelativePosition(origin,n->global_position,&p);n->local.m[12]=p.x;n->local.m[13]=p.y;n->local.m[14]=p.z;}
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
#if XGE3D_ENABLE_COLLISION
    n->collision_leaf=-1;
#endif
    n->local = n->world = d3_identity();
#if XGE3D_ENABLE_SHADOW
    n->shadow_flags=XGE3D_SHADOW_CAST|XGE3D_SHADOW_RECEIVE;
#endif
    n->parent = parent;
    n->global_position=s->origin;
    n->visible = 1;
    n->world_dirty=1;n->spatial_leaf=-1;n->render_layers=n->query_layers=n->shadow_layers=UINT32_MAX;
#if XGE3D_ENABLE_ANIMATION
    xrtArrayInit(&n->skin_users,sizeof(d3_skin*));
#endif
#if XGE3D_ENABLE_MODEL
    xrtArrayInit(&n->instance_nodes,sizeof(xge3d_node_t));
#endif
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
    n->dense_index=s->nodes.Count-1;d3_attach(s,n,d3_node(s,parent));
    ++s->version;s->spatial_rebuild=1;
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
    if(n->parent.slot==parent.slot&&n->parent.scene==parent.scene)return XGE_OK;
    d3_node_dirty(s,n);d3_detach(s,n);d3_attach(s,n,d3_node(s,parent));
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

int xge3dNodeGetLocalMatrix(xge3d_scene *s,xge3d_node_t node,xge3d_mat4_t *out)
{
    xge3d_node_data *n=d3_node(s,node);
    if(!n||!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=n->local;return XGE_OK;
}
int xge3dNodeGetWorldMatrix(xge3d_scene *s, xge3d_node_t node, xge3d_mat4_t *out)
{
    xge3d_node_data *n = d3_node(s, node), *p=n;
    if (!n || !out) return XGE_ERROR_INVALID_ARGUMENT;
    xrtArrayClear(&s->scratch);
    /* An explicit walk handles deep hierarchies without recursive C calls. */
    while (p && p->world_dirty) {
        if (!xrtArrayPush(&s->scratch, &p)) return XGE_ERROR_OUT_OF_MEMORY;
        p = d3_node(s, p->parent);
    }
    for (size_t i=s->scratch.Count; i>0; --i) {
        xge3d_node_data *child=*(xge3d_node_data**)xrtArrayGet(&s->scratch, i-1);
        child->world = p ? d3_mul(p->world, child->local) : child->local;
        child->version = ++s->version;child->world_dirty=0;++s->stats.world_updates;
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
    n->bounds_version=0;s->spatial_rebuild=1;
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
    d3_lods_free(n->lods);n->lods=lods;n->bounds_version=0;d3_spatial_dirty(s,n);++s->version;return XGE_OK;
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

int xge3dNodeSetMaterialTree(xge3d_scene *s,xge3d_node_t handle,xge3d_material *material)
{
    xge3d_node_data *root=d3_node(s,handle);if(!root)return XGE_ERROR_INVALID_ARGUMENT;
    xrtArrayClear(&s->scratch);
    for(xge3d_node_data *n=root;n;n=d3_subtree_next(n,root)){
        if(n->mesh&&n->material!=material&&!xrtArrayPush(&s->scratch,&n))return XGE_ERROR_OUT_OF_MEMORY;
    }
    if(material&&(uint64_t)s->scratch.Count>UINT32_MAX-material->refs)return XGE_ERROR_INVALID_STATE;
    for(size_t i=0;i<s->scratch.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->scratch,i);
        if(material){++material->refs;}xge3dMaterialFree(n->material);n->material=material;
    }return XGE_OK;
}

int xge3dNodeDestroy(xge3d_scene *s,xge3d_node_t node)
{
    xge3d_node_data *root=d3_node(s,node);if(!root)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_node_data *n=root;
    while(n){while(n->first_child)n=n->first_child;
        xge3d_node_data *parent=n==root?NULL:n->parent_node;d3_destroy_one(s,n);n=parent;}
    return XGE_OK;
}
int xge3dNodeDestroyStep(xge3d_scene *s,xge3d_node_t handle,const xge3d_work_budget_t *budget,int *completed)
{
    if(!completed){return XGE_ERROR_INVALID_ARGUMENT;}*completed=0;
    xge3d_node_data *root=s&&handle.scene==s->id?xrtSlotMapGet(&s->slots,handle.slot):NULL;
    if(!root||!budget)return XGE_ERROR_INVALID_ARGUMENT;
    if(!root->retiring){if(!d3_node(s,handle))return XGE_ERROR_INVALID_STATE;
        root->retiring=1;root->visible=0;root->retire_cursor=root;++s->retiring_count;}
    uint64_t start=xgeClockMicroseconds();size_t count=0;
    while(count<budget->nodes){
        if(count&&budget->microseconds&&xgeClockMicroseconds()-start>=budget->microseconds)break;
        xge3d_node_data *n=root->retire_cursor;while(n->first_child)n=n->first_child;
        int last=n==root; xge3d_node_data *parent=n->parent_node;
        if(!last)root->retire_cursor=parent;
        d3_destroy_one(s,n);++count;if(last){*completed=1;break;}
    }return XGE_OK;
}
int xge3dSceneStats(xge3d_scene *s,xge3d_scene_stats_t *out,int reset)
{
    if(!s||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    *out=s->stats;out->size=sizeof(*out);out->nodes=s->nodes.Count;
    out->spatial_bytes=s->spatial_tree.Count*sizeof(d3_spatial_branch)+s->spatial_items.Count*sizeof(d3_spatial_item);
    if(reset){memset(&s->stats,0,sizeof(s->stats));}return XGE_OK;
}
int xge3dNodeSetLayers(xge3d_scene *s,xge3d_node_t handle,uint32_t render,uint32_t query,uint32_t shadow)
{
    xge3d_node_data *n=d3_node(s,handle);if(!n)return XGE_ERROR_INVALID_ARGUMENT;
    n->render_layers=render;n->query_layers=query;n->shadow_layers=shadow;return XGE_OK;
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
