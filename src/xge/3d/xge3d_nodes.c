#include "xge3d_internal.h"
#if XGE3D_ENABLE_SCENE
int xge3dNodeGetParent(xge3d_scene *s,xge3d_node_t h,xge3d_node_t *out)
{if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};xge3d_node_data *n=d3_node(s,h);if(!n){return XGE_ERROR_INVALID_ARGUMENT;}*out=n->parent;return XGE_OK;}
size_t xge3dNodeChildCount(xge3d_scene *s,xge3d_node_t h)
{
    if(!s){return 0;}size_t count=0;
    if(!h.slot&&!h.scene){for(size_t i=0;i<s->nodes.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);if(!n->parent_node&&d3_node(s,n->handle)){++count;}}}
    else{ xge3d_node_data *p=d3_node(s,h);for(xge3d_node_data *n=p?p->first_child:NULL;n;n=n->next_sibling)if(d3_node(s,n->handle)){++count;}}
    return count;
}
xge3d_node_t xge3dNodeChildAt(xge3d_scene *s,xge3d_node_t h,size_t at)
{
    if(!s){return (xge3d_node_t){0};}
    if(!h.slot&&!h.scene){for(size_t i=0;i<s->nodes.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);if(!n->parent_node&&d3_node(s,n->handle)&&!at--){return n->handle;}}}
    else{xge3d_node_data *p=d3_node(s,h);for(xge3d_node_data *n=p?p->first_child:NULL;n;n=n->next_sibling)if(d3_node(s,n->handle)&&!at--){return n->handle;}}
    return (xge3d_node_t){0};
}
int xge3dNodeSetName(xge3d_scene *s,xge3d_node_t h,const char *name)
{
    xge3d_node_data *n=d3_node(s,h);if(!n){return XGE_ERROR_INVALID_ARGUMENT;}char *copy=NULL;
    if(name&&*name){size_t size=strlen(name)+1;if(size>1048576){return XGE_ERROR_INVALID_ARGUMENT;}copy=xrtMalloc(size);if(!copy){return XGE_ERROR_OUT_OF_MEMORY;}memcpy(copy,name,size);}
    xrtFree(n->name);n->name=copy;return XGE_OK;
}
const char *xge3dNodeName(xge3d_scene *s,xge3d_node_t h){xge3d_node_data *n=d3_node(s,h);return n?n->name:NULL;}
int xge3dNodeSetUserData(xge3d_scene *s,xge3d_node_t h,void *data)
{xge3d_node_data *n=d3_node(s,h);if(!n){return XGE_ERROR_INVALID_ARGUMENT;}n->user_data=data;return XGE_OK;}
void *xge3dNodeUserData(xge3d_scene *s,xge3d_node_t h){xge3d_node_data *n=d3_node(s,h);return n?n->user_data:NULL;}
int xge3dNodeFind(xge3d_scene *s,xge3d_node_t root,const char *name,xge3d_node_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};
    if(!s||!name||!*name){return XGE_ERROR_INVALID_ARGUMENT;}
    if(root.slot||root.scene){xge3d_node_data *r=d3_node(s,root);if(!r){return XGE_ERROR_INVALID_ARGUMENT;}
        for(xge3d_node_data *n=r;n;n=d3_subtree_next(n,r))if(d3_node(s,n->handle)&&n->name&&!strcmp(n->name,name)){*out=n->handle;return XGE_OK;}}
    else for(size_t i=0;i<s->nodes.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);if(d3_node(s,n->handle)&&n->name&&!strcmp(n->name,name)){*out=n->handle;return XGE_OK;}}
    return XGE_ERROR_NOT_FOUND;
}
int xge3dNodeGetTransform(xge3d_scene *s,xge3d_node_t h,xge3d_transform_t *out)
{
    xge3d_node_data *n=d3_node(s,h);if(!n||!out){return XGE_ERROR_INVALID_ARGUMENT;}
    float r[3][3],scale[3];
    for(int c=0;c<3;++c){double length=0;for(int k=0;k<3;++k){length+=(double)n->local.m[c*4+k]*n->local.m[c*4+k];}
        scale[c]=(float)sqrt(length);if(!isfinite(scale[c])||scale[c]<1e-12f){return XGE_ERROR_UNSUPPORTED;}
        for(int k=0;k<3;++k){r[k][c]=n->local.m[c*4+k]/scale[c];}}
    for(int a=0;a<3;++a)for(int b=a+1;b<3;++b){double dot=0;for(int k=0;k<3;++k){dot+=r[k][a]*r[k][b];}if(fabs(dot)>1e-5){return XGE_ERROR_UNSUPPORTED;}}
    double det=r[0][0]*(r[1][1]*r[2][2]-r[1][2]*r[2][1])-r[0][1]*(r[1][0]*r[2][2]-r[1][2]*r[2][0])+r[0][2]*(r[1][0]*r[2][1]-r[1][1]*r[2][0]);
    if(det<0){scale[0]=-scale[0];for(int k=0;k<3;++k){r[k][0]=-r[k][0];}}
    xge3d_quat_t q;float trace=r[0][0]+r[1][1]+r[2][2];
    if(trace>0){float v=sqrtf(trace+1)*2;q=(xge3d_quat_t){(r[2][1]-r[1][2])/v,(r[0][2]-r[2][0])/v,(r[1][0]-r[0][1])/v,v*.25f};}
    else{int a=r[0][0]>r[1][1]?0:1;if(r[2][2]>r[a][a]){a=2;}int b=(a+1)%3,c=(a+2)%3;
        float v=sqrtf(fmaxf(0,1+r[a][a]-r[b][b]-r[c][c]))*2,values[4]={0};if(v<1e-12f){return XGE_ERROR_UNSUPPORTED;}
        values[a]=v*.25f;values[b]=(r[b][a]+r[a][b])/v;values[c]=(r[c][a]+r[a][c])/v;values[3]=(r[c][b]-r[b][c])/v;
        q=(xge3d_quat_t){values[0],values[1],values[2],values[3]};}
    *out=(xge3d_transform_t){{n->local.m[12],n->local.m[13],n->local.m[14]},q,{scale[0],scale[1],scale[2]}};return XGE_OK;
}
static xge3d_node_t d3_clone_lookup(xmap *map,xge3d_node_t h)
{xge3d_node_t *v=xrtMapGet(map,(xbytesview){(const unsigned char*)&h.slot,sizeof(h.slot)});return v?*v:(xge3d_node_t){0};}
int xge3dNodeClone(xge3d_scene *s,xge3d_node_t source,xge3d_node_t parent,uint32_t flags,xge3d_node_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};xge3d_node_data *root=d3_node(s,source);
    if(!root||(flags&~XGE3D_CLONE_USER_DATA)||((parent.slot||parent.scene)&&!d3_node(s,parent))){return XGE_ERROR_INVALID_ARGUMENT;}
    xarray originals;xrtArrayInit(&originals,sizeof(xge3d_node_data*));xmap map;xrtMapInit(&map,sizeof(xge3d_node_t));
    int result=XGE_ERROR_OUT_OF_MEMORY;xge3d_node_t clone={0};
    for(xge3d_node_data *n=root;n;n=d3_subtree_next(n,root))if(!d3_node(s,n->handle)||!xrtArrayPush(&originals,&n)){goto done;}
    for(size_t i=0;i<originals.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&originals,i);xge3d_node_t h;
        result=xge3dNodeCreate(s,i?d3_clone_lookup(&map,n->parent):parent,&h);if(result){goto done;}if(!i){clone=h;d3_node(s,h)->visible=0;}
        if(!xrtMapSet(&map,(xbytesview){(const unsigned char*)&n->handle.slot,sizeof(n->handle.slot)},&h)){result=XGE_ERROR_OUT_OF_MEMORY;goto done;}
        if((result=xge3dNodeSetMatrix(s,h,&n->local))||(result=xge3dNodeSetMesh(s,h,n->mesh))||
           (result=xge3dNodeSetMaterial(s,h,n->material))||(result=xge3dNodeSetName(s,h,n->name))){goto done;}
        xge3d_node_data *next=d3_node(s,h);memcpy(next->color,n->color,sizeof(n->color));
        next->render_layers=n->render_layers;next->query_layers=n->query_layers;next->shadow_layers=n->shadow_layers;
#if XGE3D_ENABLE_SHADOW
        next->shadow_flags=n->shadow_flags;
#endif
        if(i){next->visible=n->visible;}if(flags&XGE3D_CLONE_USER_DATA){next->user_data=n->user_data;}
        if(n->lods&&(result=xge3dNodeSetLods(s,h,n->lods->levels,n->lods->count))){goto done;}
#if XGE3D_ENABLE_COLLISION
        if(n->collider&&(result=xge3dNodeSetCollider(s,h,n->collider))){goto done;}
#endif
#if XGE3D_ENABLE_LIGHTING
        next->has_light=n->has_light;next->light=n->light;
#endif
#if XGE3D_ENABLE_MODEL
        if(n->model){if(!d3_model_retain(n->model)){result=XGE_ERROR_INVALID_STATE;goto done;}next->model=n->model;}
#endif
    }
    for(size_t i=0;i<originals.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&originals,i);xge3d_node_t h=d3_clone_lookup(&map,n->handle);
#if XGE3D_ENABLE_MODEL
        xge3d_node_data *next=d3_node(s,h);next->instance_root=d3_clone_lookup(&map,n->instance_root);next->source_node=n->source_node;
        if(!xrtArrayResize(&next->instance_nodes,n->instance_nodes.Count)){result=XGE_ERROR_OUT_OF_MEMORY;goto done;}
        for(size_t j=0;j<n->instance_nodes.Count;++j){*(xge3d_node_t*)xrtArrayGet(&next->instance_nodes,j)=d3_clone_lookup(&map,*(xge3d_node_t*)xrtArrayGet(&n->instance_nodes,j));}
#endif
#if XGE3D_ENABLE_ANIMATION
        if(n->skin){xge3d_node_t joints[D3_MAX_JOINTS];for(size_t j=0;j<n->skin->joints.Count;++j){joints[j]=d3_clone_lookup(&map,*(xge3d_node_t*)xrtArrayGet(&n->skin->joints,j));
                if(!joints[j].slot){result=XGE_ERROR_UNSUPPORTED;goto done;}}
            xge3d_skin_binding_t b={joints,(const xge3d_mat4_t*)n->skin->inverse_bind.Data,n->skin->joints.Count};if((result=xge3dNodeBindSkin(s,h,&b))){goto done;}}
#endif
        (void)h;
    }
    if(!root->parent.slot&&!parent.slot){result=xge3dNodeSetGlobalPosition(s,clone,root->global_position);if(result){goto done;}}
    d3_node(s,clone)->visible=root->visible;*out=clone;clone=(xge3d_node_t){0};result=XGE_OK;
done:
    if(clone.slot){xge3dNodeDestroy(s,clone);}xrtMapUnit(&map);xrtArrayUnit(&originals);return result;
}
xge3d_scene_node_desc_t xge3dSceneNodeDefault(void)
{xge3d_scene_node_desc_t d={0};d.parent=SIZE_MAX;d.transform=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;return d;}
int xge3dSceneLoad(xge3d_scene *s,xge3d_node_t parent,const xge3d_scene_desc_t *desc,xge3d_node_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};
    if(!s||!desc||desc->count>65536||(desc->count&&!desc->nodes)||((parent.slot||parent.scene)&&!d3_node(s,parent))){return XGE_ERROR_INVALID_ARGUMENT;}
    for(size_t i=0;i<desc->count;++i){const xge3d_scene_node_desc_t *n=desc->nodes+i;xge3d_mat4_t m;
        if((n->parent!=SIZE_MAX&&n->parent>=i)||(n->flags&~XGE3D_SCENE_NODE_HIDDEN)||xge3dTransformMatrix(&n->transform,&m)){return XGE_ERROR_INVALID_ARGUMENT;}}
    xarray nodes;xrtArrayInit(&nodes,sizeof(xge3d_node_t));int result=XGE_ERROR_OUT_OF_MEMORY;xge3d_node_t root={0};
    if(!xrtArrayResize(&nodes,desc->count)){goto done;}
#if XGE3D_ENABLE_MODEL
    xarray models;xrtArrayInit(&models,sizeof(xge3d_model*));
    if(!xrtArrayResize(&models,desc->count)){goto models_done;}
    if(models.Count){memset(models.Data,0,models.Count*models.ItemSize);}
    for(size_t i=0;i<desc->count;++i)if(desc->nodes[i].model_uri){
        xge3d_model **m=xrtArrayGet(&models,i);result=desc->load_model?desc->load_model(desc->nodes[i].model_uri,m,desc->user):xge3dModelLoad(desc->nodes[i].model_uri,m);
        if(result||!*m){if(!result){result=XGE_ERROR_RESOURCE_FAILED;}goto models_done;}}
#endif
    result=xge3dNodeCreate(s,parent,&root);if(result){goto staged_done;}d3_node(s,root)->visible=0;
    for(size_t i=0;i<desc->count;++i){const xge3d_scene_node_desc_t *n=desc->nodes+i;xge3d_node_t *h=xrtArrayGet(&nodes,i);
        xge3d_node_t p=n->parent==SIZE_MAX?root:*(xge3d_node_t*)xrtArrayGet(&nodes,n->parent);
#if XGE3D_ENABLE_MODEL
        xge3d_model *m=*(xge3d_model**)xrtArrayGet(&models,i);result=m?xge3dModelInstantiate(s,m,p,h):xge3dNodeCreate(s,p,h);
#else
        result=xge3dNodeCreate(s,p,h);
#endif
        if(result){break;}
        if((result=xge3dNodeSetTransform(s,*h,&n->transform))||(result=xge3dNodeSetName(s,*h,n->name))){break;}
        if(n->mesh&&(result=xge3dNodeSetMesh(s,*h,n->mesh))){break;}
        if(n->material&&(result=xge3dNodeSetMaterial(s,*h,n->material))){break;}
        d3_node(s,*h)->visible=!(n->flags&XGE3D_SCENE_NODE_HIDDEN);
    }
    if(!result){d3_node(s,root)->visible=1;*out=root;root=(xge3d_node_t){0};}
staged_done:
    if(root.slot){xge3dNodeDestroy(s,root);}
#if XGE3D_ENABLE_MODEL
models_done:
    for(size_t i=0;i<models.Count;++i){xge3dModelFree(*(xge3d_model**)xrtArrayGet(&models,i));}xrtArrayUnit(&models);
#endif
done:
    xrtArrayUnit(&nodes);return result;
}
#endif
