/* Balanced scene BVH. Structural edits rebuild explicitly/on first query;
 * transform/pose edits refit only queued leaves and their ancestor paths. */
xatomic64 d3_geometry_version;
static xge3d_aabb_t d3_union_box(xge3d_aabb_t a,xge3d_aabb_t b)
{d3_bounds_point(&a,b.min);d3_bounds_point(&a,b.max);return a;}
#define D3_SPATIAL_COMPARE(axis) \
static int d3_spatial_compare_##axis(const void *a,const void *b){ \
const d3_spatial_item *x=a,*y=b; \
double u=(double)x->box.min.axis+x->box.max.axis,v=(double)y->box.min.axis+y->box.max.axis; \
return u<v?-1:u>v?1:x->node.slot<y->node.slot?-1:x->node.slot>y->node.slot;}
D3_SPATIAL_COMPARE(x)
D3_SPATIAL_COMPARE(y)
D3_SPATIAL_COMPARE(z)
#undef D3_SPATIAL_COMPARE
static int d3_spatial_build(xge3d_scene *s,size_t first,size_t count,int parent,int *next)
{
    int at=(*next)++;d3_spatial_branch *b=xrtArrayGet(&s->spatial_tree,(size_t)at);
    d3_spatial_item *items=(d3_spatial_item*)s->spatial_items.Data;
    b->parent=parent;b->left=b->right=-1;b->node=(xge3d_node_t){0};b->box=items[first].box;
    for(size_t i=1;i<count;++i)b->box=d3_union_box(b->box,items[first+i].box);
    if(count==1){b->node=items[first].node;d3_node(s,b->node)->spatial_leaf=at;return at;}
    xge3d_vec3_t extent=d3_sub(b->box.max,b->box.min);
    int (*compare)(const void*,const void*)=extent.x>=extent.y&&extent.x>=extent.z?d3_spatial_compare_x:extent.y>=extent.z?d3_spatial_compare_y:d3_spatial_compare_z;
    qsort(items+first,count,sizeof(*items),compare);
    size_t left=count/2;
    b->left=d3_spatial_build(s,first,left,at,next);b->right=d3_spatial_build(s,first+left,count-left,at,next);
    return at;
}
int xge3dScenePrepareSpatial(xge3d_scene *s)
{
    if(!s)return XGE_ERROR_INVALID_ARGUMENT;
    uint64_t geometry=xrtAtomic64Load(&d3_geometry_version,XMEMORY_ACQUIRE);
    if(geometry!=s->spatial_geometry_version)s->spatial_rebuild=1;
    if(s->spatial_rebuild){
        xrtArrayClear(&s->spatial_items);
        for(size_t i=0;i<s->nodes.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);
            n->spatial_leaf=-1;n->spatial_queued=0;if(!n->mesh||!d3_node(s,n->handle))continue;
            d3_spatial_item item={.node=n->handle};int result=d3_node_bounds(s,n,&item.box);if(result)return result;
            if(!xrtArrayPush(&s->spatial_items,&item))return XGE_ERROR_OUT_OF_MEMORY;
        }
        size_t count=s->spatial_items.Count;if(count>(INT_MAX-1u)/2u)return XGE_ERROR_UNSUPPORTED;
        if(!xrtArrayResize(&s->spatial_tree,count?count*2-1:0))return XGE_ERROR_OUT_OF_MEMORY;
        int next=0;s->spatial_root=count?d3_spatial_build(s,0,count,-1,&next):-1;
        s->spatial_rebuild=0;s->spatial_geometry_version=geometry;xrtArrayClear(&s->spatial_pending);++s->stats.spatial_builds;
    }else{
        for(size_t i=0;i<s->spatial_pending.Count;++i){xge3d_node_data *n=d3_node(s,*(xge3d_node_t*)xrtArrayGet(&s->spatial_pending,i));
            if(!n){continue;}n->spatial_queued=0;if(n->spatial_leaf<0)continue;
            d3_spatial_branch *b=xrtArrayGet(&s->spatial_tree,(size_t)n->spatial_leaf);
            int result=d3_node_bounds(s,n,&b->box);if(result)return result;++s->stats.spatial_refits;
            int parent=b->parent;while(parent>=0){b=xrtArrayGet(&s->spatial_tree,(size_t)parent);
                b->box=d3_union_box(((d3_spatial_branch*)s->spatial_tree.Data)[b->left].box,((d3_spatial_branch*)s->spatial_tree.Data)[b->right].box);parent=b->parent;}
        }
        xrtArrayClear(&s->spatial_pending);
    }return XGE_OK;
}
static int d3_query_accept(xge3d_scene *s,xge3d_node_data *n,const xge3d_query_filter_t *filter)
{
    if(!n||!d3_node(s,n->handle)||!(n->query_layers&(filter?filter->layers:UINT32_MAX)))return 0;
    if(!filter||!filter->include_hidden)for(xge3d_node_data *p=n;p;p=p->parent_node)if(!p->visible)return 0;
    return 1;
}
int xge3dSceneQueryAabbEx(xge3d_scene *s,const xge3d_aabb_t *box,const xge3d_query_filter_t *filter,
    xge3d_node_t *nodes,size_t capacity,size_t *count)
{
    if(!count){return XGE_ERROR_INVALID_ARGUMENT;}*count=0;
    if(!s||!d3_box_valid(box)||(capacity&&!nodes))return XGE_ERROR_INVALID_ARGUMENT;
    int result=xge3dScenePrepareSpatial(s);if(result)return result;
    int stack[64],top=0;if(s->spatial_root>=0)stack[top++]=s->spatial_root;
    while(top){d3_spatial_branch *b=xrtArrayGet(&s->spatial_tree,(size_t)stack[--top]);++s->stats.query_tree_visits;
        if(!xge3dAabbIntersects(box,&b->box))continue;
        if(b->left>=0){stack[top++]=b->left;stack[top++]=b->right;continue;}
        xge3d_node_data *n=d3_node(s,b->node);if(!d3_query_accept(s,n,filter))continue;++s->stats.query_mesh_tests;
        if(*count<capacity){nodes[*count]=n->handle;}++*count;
    }
    return nodes&&*count>capacity?XGE_ERROR_BUFFER_TOO_SMALL:XGE_OK;
}
