typedef struct d3_instance_skin { xge3d_node_t node;size_t source; } d3_instance_skin;
struct xge3d_instance_builder {
    xge3d_scene *scene;xge3d_model *model;xge3d_node_t root;
    xarray nodes,skins;size_t source,primitive,parent_index,skin_index;
    int phase,error,taken;
};
int xge3dModelAddRef(xge3d_model *m)
{return !m?XGE_ERROR_INVALID_ARGUMENT:d3_model_retain(m)?XGE_OK:XGE_ERROR_INVALID_STATE;}
static uint64_t d3_array_storage(const xarray *a)
{return (uint64_t)a->Capacity*a->ItemSize;}
int xge3dDecodeBudgetCreate(uint64_t bytes,xge3d_decode_budget **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!bytes||bytes>SIZE_MAX)return XGE_ERROR_INVALID_ARGUMENT;
    return xrtAllocationBudgetCreate((size_t)bytes,out)?XGE_OK:XGE_ERROR_OUT_OF_MEMORY;
}
void xge3dDecodeBudgetFree(xge3d_decode_budget *budget){xrtAllocationBudgetFree(budget);}
int xge3dDecodeBudgetInfo(xge3d_decode_budget *budget,xge3d_decode_budget_info_t *out)
{
    if(!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    xallocationbudgetinfo info={.Size=sizeof(info)};
    if(!xrtAllocationBudgetInfo(budget,&info))return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_decode_budget_info_t){sizeof(*out),info.Limit,info.LiveBytes,info.PeakBytes,info.LiveCount,info.Rejections};return XGE_OK;
}
int xge3dDecodeBudgetSetLimit(xge3d_decode_budget *budget,uint64_t bytes)
{return bytes&&bytes<=SIZE_MAX&&xrtAllocationBudgetSetLimit(budget,(size_t)bytes)?XGE_OK:XGE_ERROR_INVALID_ARGUMENT;}
int xge3dModelLoadBudget(const char *uri,xge3d_decode_budget *budget,xge3d_model **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    xallocationscope scope={0};if(!xrtAllocationBudgetEnter(budget,&scope))return XGE_ERROR_INVALID_ARGUMENT;
    int result=xge3dModelLoad(uri,out);int rejected=scope.Rejections!=0;
    xrtAllocationBudgetLeave(&scope);return result&&rejected?XGE_ERROR_OUT_OF_MEMORY:result;
}
int xge3dModelLoadMemoryBudget(const void *data,size_t size,const char *base,xge3d_decode_budget *budget,xge3d_model **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    xallocationscope scope={0};if(!xrtAllocationBudgetEnter(budget,&scope))return XGE_ERROR_INVALID_ARGUMENT;
    int result=xge3dModelLoadMemory(data,size,base,out);int rejected=scope.Rejections!=0;
    xrtAllocationBudgetLeave(&scope);return result&&rejected?XGE_ERROR_OUT_OF_MEMORY:result;
}
int xge3dModelMemoryInfo(const xge3d_model *m,xge3d_asset_memory_t *out)
{
    if(!m||m->retiring||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_asset_memory_t info={.size=sizeof(info),.source_bytes=(uint64_t)m->source.iSize,.parser_bytes=m->parser_bytes+m->buffer_bytes};
    info.cpu_storage_bytes=sizeof(*m)+info.source_bytes+info.parser_bytes+
        d3_array_storage(&m->primitives)+d3_array_storage(&m->ranges)+d3_array_storage(&m->images)+
        d3_array_storage(&m->materials)+d3_array_storage(&m->textures);
    /* KeyOffset includes the entry header and value storage. Every key is a
     * uint64_t texture/role pair. Heap alignment padding remains excluded. */
    info.cpu_storage_bytes+=(uint64_t)m->texture_cache.BucketCount*sizeof(void*)+
        (uint64_t)m->texture_cache.Count*(m->texture_cache.KeyOffset+sizeof(uint64_t));
    for(size_t i=0;i<m->primitives.Count;++i){const xge3d_mesh *mesh=((const d3_primitive*)xrtArrayConstGet(&m->primitives,i))->mesh;
        info.cpu_storage_bytes+=sizeof(*mesh)+d3_array_storage(&mesh->vertices)+d3_array_storage(&mesh->indices);
#if XGE3D_ENABLE_ANIMATION
        info.cpu_storage_bytes+=d3_array_storage(&mesh->joint_bounds);
#endif
        info.gpu_required_bytes+=(uint64_t)mesh->vertices.Count*sizeof(xge3d_vertex_t)+(uint64_t)mesh->indices.Count*(mesh->index_bits/8);
    }
    info.cpu_storage_bytes+=(uint64_t)m->materials.Count*sizeof(xge3d_material);
    for(size_t i=0;i<m->textures.Count;++i){const xge3d_texture *t=*(xge3d_texture*const*)xrtArrayConstGet(&m->textures,i);
        info.cpu_storage_bytes+=sizeof(*t)+d3_array_storage(&t->pixels);
        int w=t->width,h=t->height;info.gpu_required_bytes+=(uint64_t)w*h*4;
        if(t->sampler.min_filter>=0x2700)while(w>1||h>1){w=w>1?w/2:1;h=h>1?h/2:1;info.gpu_required_bytes+=(uint64_t)w*h*4;}
    }
    *out=info;return XGE_OK;
}
int xge3dModelResourceInfo(const xge3d_model *model,xge3d_resource_info_t *out)
{
    if(!model||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_resource_info_t){.size=sizeof(*out),.references=model->refs};
    for(size_t i=0;i<model->primitives.Count;++i){const d3_primitive *p=xrtArrayConstGet(&model->primitives,i);
        xge3d_resource_info_t part={.size=sizeof(part)};xge3dMeshResourceInfo(p->mesh,&part);
        out->cpu_payload_bytes+=part.cpu_payload_bytes;out->gpu_payload_bytes+=part.gpu_payload_bytes;out->vertices+=part.vertices;out->indices+=part.indices;}
    for(size_t i=0;i<model->textures.Count;++i){const xge3d_texture *texture=*(xge3d_texture*const*)xrtArrayConstGet(&model->textures,i);
        xge3d_resource_info_t part={.size=sizeof(part)};xge3dTextureResourceInfo(texture,&part);
        out->cpu_payload_bytes+=part.cpu_payload_bytes;out->gpu_payload_bytes+=part.gpu_payload_bytes;++out->textures;}
    return XGE_OK;
}
void xge3dModelInstanceFree(xge3d_instance_builder *b)
{
    if(!b)return;
    if(b->root.slot&&d3_node(b->scene,b->root))xge3dNodeDestroy(b->scene,b->root);
    xge3dModelFree(b->model);xrtArrayUnit(&b->nodes);xrtArrayUnit(&b->skins);xrtFree(b);
}
int xge3dModelInstanceBegin(xge3d_scene *s,xge3d_model *m,xge3d_node_t parent,xge3d_instance_builder **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;
    if(!s||!m)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_instance_builder *b=xrtMalloc(sizeof(*b));if(!b)return XGE_ERROR_OUT_OF_MEMORY;memset(b,0,sizeof(*b));
    b->scene=s;xrtArrayInit(&b->nodes,sizeof(xge3d_node_t));xrtArrayInit(&b->skins,sizeof(d3_instance_skin));
    if(!d3_model_retain(m)){xge3dModelInstanceFree(b);return XGE_ERROR_INVALID_STATE;}b->model=m;
    int result=xge3dNodeCreate(s,parent,&b->root);
    if(result==XGE_OK&&!xrtArrayResize(&b->nodes,m->data->nodes_count))result=XGE_ERROR_OUT_OF_MEMORY;
    if(result){xge3dModelInstanceFree(b);return result;}
    if(b->nodes.Count)memset(b->nodes.Data,0,b->nodes.Count*b->nodes.ItemSize);
    d3_node(s,b->root)->visible=0;*out=b;return XGE_OK;
}
static int d3_instance_primitive(xge3d_instance_builder *b,size_t source,size_t primitive,xge3d_node_t node)
{
    d3_mesh_range *range=xrtArrayGet(&b->model->ranges,cgltf_mesh_index(b->model->data,b->model->data->nodes[source].mesh));
    d3_primitive *p=xrtArrayGet(&b->model->primitives,range->first+primitive);
    int result=xge3dNodeSetMesh(b->scene,node,p->mesh);if(result)return result;
    result=xge3dNodeSetMaterial(b->scene,node,p->material);if(result)return result;
    d3_instance_skin skin={node,source};
    return xrtArrayPush(&b->skins,&skin)?XGE_OK:XGE_ERROR_OUT_OF_MEMORY;
}
int xge3dModelInstanceStep(xge3d_instance_builder *b,const xge3d_work_budget_t *budget,int *completed)
{
    if(!completed){return XGE_ERROR_INVALID_ARGUMENT;}*completed=0;
    if(!b||!budget||b->taken)return XGE_ERROR_INVALID_ARGUMENT;
    if(b->error){return b->error;}if(!d3_node(b->scene,b->root))return b->error=XGE_ERROR_INVALID_STATE;
    uint64_t start=xgeClockMicroseconds();size_t work=0;int result=0;
    while(b->phase<4&&work<budget->nodes){
        if(work&&budget->microseconds&&xgeClockMicroseconds()-start>=budget->microseconds)break;
        if(b->phase==0){
            if(b->source==b->nodes.Count){b->phase=2;continue;}++work;
            cgltf_node *source=&b->model->data->nodes[b->source];
            if(!d3_selected(b->model->data,source)){++b->source;continue;}
            xge3d_node_t *node=xrtArrayGet(&b->nodes,b->source);result=xge3dNodeCreate(b->scene,b->root,node);if(result)break;
            xge3d_node_data *n=d3_node(b->scene,*node);n->instance_root=b->root;n->source_node=b->source;
            xge3d_mat4_t matrix;cgltf_node_transform_local(source,matrix.m);result=xge3dNodeSetMatrix(b->scene,*node,&matrix);if(result)break;
#if XGE3D_ENABLE_LIGHTING
            if(source->light){const cgltf_light *l=source->light;xge3d_light_desc_t d=xge3dLightDefault((int)l->type-1);
                d.color=(xge3d_vec3_t){l->color[0],l->color[1],l->color[2]};d.intensity=l->intensity;d.range=l->range;
                d.inner_angle=l->spot_inner_cone_angle;d.outer_angle=l->spot_outer_cone_angle;result=xge3dNodeSetLight(b->scene,*node,&d);if(result)break;}
#endif
            if(source->mesh){b->phase=1;b->primitive=0;}else ++b->source;
        }else if(b->phase==1){
            const cgltf_node *source=&b->model->data->nodes[b->source];
            d3_mesh_range *range=xrtArrayGet(&b->model->ranges,cgltf_mesh_index(b->model->data,source->mesh));
            if(b->primitive==range->count){++b->source;b->phase=0;continue;}++work;
            xge3d_node_t node=*(xge3d_node_t*)xrtArrayGet(&b->nodes,b->source);
            if(range->count>1){result=xge3dNodeCreate(b->scene,node,&node);if(result)break;
                xge3d_node_data *n=d3_node(b->scene,node);n->instance_root=b->root;n->source_node=SIZE_MAX;}
            result=d3_instance_primitive(b,b->source,b->primitive++,node);if(result)break;
        }else if(b->phase==2){
            if(b->parent_index==b->nodes.Count){b->phase=3;continue;}size_t i=b->parent_index++;++work;
            const cgltf_node *source=&b->model->data->nodes[i];xge3d_node_t node=*(xge3d_node_t*)xrtArrayGet(&b->nodes,i);
            if(node.slot&&source->parent){xge3d_node_t parent=*(xge3d_node_t*)xrtArrayGet(&b->nodes,cgltf_node_index(b->model->data,source->parent));
                result=xge3dNodeSetParent(b->scene,node,parent);if(result)break;}
        }else{
            if(b->skin_index==b->skins.Count){b->phase=4;continue;}++work;
#if XGE3D_ENABLE_ANIMATION
            d3_instance_skin *skin=xrtArrayGet(&b->skins,b->skin_index++);const cgltf_node *source=&b->model->data->nodes[skin->source];
            if(source->skin){xge3d_node_data *n=d3_node(b->scene,skin->node);
                result=d3_skin_create(b->scene,n,b->model,cgltf_skin_index(b->model->data,source->skin),&b->nodes,&n->skin);if(result)break;}
#else
            ++b->skin_index;
#endif
        }
    }
    if(result){b->error=result;return result;}*completed=b->phase==4;return XGE_OK;
}
int xge3dModelInstanceTake(xge3d_instance_builder *b,xge3d_node_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};
    if(!b||b->taken||b->phase!=4||b->error)return XGE_ERROR_INVALID_STATE;
    xge3d_node_data *n=d3_node(b->scene,b->root);if(!n||!d3_model_retain(b->model))return XGE_ERROR_INVALID_STATE;
    n->model=b->model;n->instance_nodes=b->nodes;memset(&b->nodes,0,sizeof(b->nodes));n->visible=1;
    *out=b->root;b->root=(xge3d_node_t){0};b->taken=1;return XGE_OK;
}
int xge3dModelInstantiate(xge3d_scene *s,xge3d_model *m,xge3d_node_t parent,xge3d_node_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};xge3d_instance_builder *b=NULL;
    int result=xge3dModelInstanceBegin(s,m,parent,&b),completed=0;
    xge3d_work_budget_t budget={SIZE_MAX,0};if(!result)result=xge3dModelInstanceStep(b,&budget,&completed);
    if(!result){result=xge3dModelInstanceTake(b,out);}xge3dModelInstanceFree(b);return result;
}
int xge3dModelInstanceNode(xge3d_scene *s,xge3d_node_t root,size_t index,xge3d_node_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_node_t){0};xge3d_node_data *r=d3_node(s,root);
    if(!r||!r->model||index>=r->instance_nodes.Count)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_node_t h=*(xge3d_node_t*)xrtArrayGet(&r->instance_nodes,index);
    if(!d3_node(s,h)){return XGE_ERROR_NOT_FOUND;}*out=h;return XGE_OK;
}
