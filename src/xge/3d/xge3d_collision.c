#include "xge3d_internal.h"
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#if XGE3D_ENABLE_COLLISION
/* Distance GJK with exhaustive simplex face reduction. No EPA/rigid-body state.
 * Narrow phase uses double; public positions and scene transforms remain float. */
typedef struct cv {double x,y,z;} cv;
static cv cvadd(cv a,cv b){return (cv){a.x+b.x,a.y+b.y,a.z+b.z};}
static cv cvsub(cv a,cv b){return (cv){a.x-b.x,a.y-b.y,a.z-b.z};}
static cv cvscale(cv a,double s){return (cv){a.x*s,a.y*s,a.z*s};}
static double cvdot(cv a,cv b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static cv cvfrom(xge3d_vec3_t a){return (cv){a.x,a.y,a.z};}
static xge3d_vec3_t cvout(cv a){return (xge3d_vec3_t){(float)a.x,(float)a.y,(float)a.z};}
static cv cvpoint(const xge3d_mat4_t *m,cv p)
{return (cv){m->m[0]*p.x+m->m[4]*p.y+m->m[8]*p.z+m->m[12],m->m[1]*p.x+m->m[5]*p.y+m->m[9]*p.z+m->m[13],m->m[2]*p.x+m->m[6]*p.y+m->m[10]*p.z+m->m[14]};}
typedef struct convex {const xge3d_shape_t *shape;xge3d_mat4_t world;cv offset,triangle[3];int tri;} convex;
static cv support(const convex *c,cv d)
{
    if(c->tri){int at=0;for(int i=1;i<3;++i)if(cvdot(c->triangle[i],d)>cvdot(c->triangle[at],d)){at=i;}return c->triangle[at];}
    const float *m=c->world.m;cv local={m[0]*d.x+m[1]*d.y+m[2]*d.z,m[4]*d.x+m[5]*d.y+m[6]*d.z,m[8]*d.x+m[9]*d.y+m[10]*d.z};
    const xge3d_shape_t *s=c->shape;cv p=cvfrom(s->center);
    if(s->type==XGE3D_SHAPE_BOX){p=cvadd(p,(cv){local.x>=0?s->half_extents.x:-s->half_extents.x,local.y>=0?s->half_extents.y:-s->half_extents.y,local.z>=0?s->half_extents.z:-s->half_extents.z});}
    else{double length=sqrt(cvdot(local,local));if(length>0){p=cvadd(p,cvscale(local,s->radius/length));}if(s->type==XGE3D_SHAPE_CAPSULE){p.y+=local.y>=0?s->half_height:-s->half_height;}}
    return cvadd(cvpoint(&c->world,p),c->offset);
}
typedef struct support_point {cv a,b,p;} support_point;
static support_point minkowski(const convex *a,const convex *b,cv direction)
{support_point p={.a=support(a,direction),.b=support(b,cvscale(direction,-1))};p.p=cvsub(p.a,p.b);return p;}
static int simplex_reduce(support_point *points,int *count,cv *a,cv *b,cv *closest)
{
    double best=DBL_MAX,chosen[4]={0};int selected=0;
    for(int mask=1;mask<(1<<*count);++mask){int ids[4],n=0;double weights[4]={1,0,0,0};
        for(int i=0;i<*count;++i)if(mask&(1<<i)){ids[n++]=i;}
        cv edges[3];double matrix[3][4]={{0}};int good=1;
        for(int i=1;i<n;++i){edges[i-1]=cvsub(points[ids[i]].p,points[ids[0]].p);}
        for(int i=0;i<n-1;++i){for(int j=0;j<n-1;++j){matrix[i][j]=cvdot(edges[i],edges[j]);}matrix[i][n-1]=-cvdot(edges[i],points[ids[0]].p);}
        for(int i=0;i<n-1;++i){int pivot=i;for(int j=i+1;j<n-1;++j)if(fabs(matrix[j][i])>fabs(matrix[pivot][i])){pivot=j;}
            if(fabs(matrix[pivot][i])<1e-26){good=0;break;}
            for(int k=i;k<n;++k){double t=matrix[pivot][k];matrix[pivot][k]=matrix[i][k];matrix[i][k]=t;}
            double scale=matrix[i][i];for(int k=i;k<n;++k){matrix[i][k]/=scale;}
            for(int j=0;j<n-1;++j)if(j!=i){double scale2=matrix[j][i];for(int k=i;k<n;++k){matrix[j][k]-=scale2*matrix[i][k];}}}
        if(!good){continue;}
        for(int i=1;i<n;++i){weights[i]=matrix[i-1][n-1];weights[0]-=weights[i];}
        cv p={0};double total=0;for(int i=0;i<n;++i){if(weights[i]<-1e-10){good=0;break;}weights[i]=fmax(0,weights[i]);total+=weights[i];}
        if(!good||total<=0){continue;}
        for(int i=0;i<n;++i){weights[i]/=total;p=cvadd(p,cvscale(points[ids[i]].p,weights[i]));}
        double distance=cvdot(p,p);if(distance<best){best=distance;selected=mask;memset(chosen,0,sizeof(chosen));for(int i=0;i<n;++i){chosen[ids[i]]=weights[i];}}
    }
    if(!selected){return 0;}
    *a=*b=*closest=(cv){0};support_point kept[4];int n=0;
    for(int i=0;i<*count;++i)if(chosen[i]>0){*a=cvadd(*a,cvscale(points[i].a,chosen[i]));*b=cvadd(*b,cvscale(points[i].b,chosen[i]));if(chosen[i]>1e-12){kept[n++]=points[i];}}
    *closest=cvsub(*a,*b);memcpy(points,kept,(size_t)n*sizeof(*points));*count=n;return n>0;
}
static int convex_distance_seed(const convex *a,const convex *b,cv *normal,cv *point,double *upper,double *lower,support_point *seed,int *seed_count)
{
    support_point points[4];int count=1;points[0]=minkowski(a,b,(cv){1,0,0});cv pa,pb,closest;*normal=(cv){1,0,0};
    for(int iteration=0;iteration<160;++iteration){if(!simplex_reduce(points,&count,&pa,&pb,&closest)){return XGE_ERROR_INVALID_STATE;}
        *upper=sqrt(cvdot(closest,closest));*point=pb;
        if(*upper<1e-9){*lower=0;if(!seed)*normal=(cv){0,1,0};if(seed){memcpy(seed,points,(size_t)count*sizeof(*seed));*seed_count=count;}return XGE_OK;}
        *normal=cvscale(closest,1/ *upper);support_point next=minkowski(a,b,cvscale(*normal,-1));
        *lower=fmax(0,cvdot(next.p,*normal));
        if(*upper-*lower<=1e-7*fmax(1,*upper)){return XGE_OK;}
        int duplicate=0;for(int i=0;i<count;++i)if(cvdot(cvsub(next.p,points[i].p),cvsub(next.p,points[i].p))<1e-24){duplicate=1;}
        if(duplicate){return XGE_OK;}
        if(count==4){return XGE_ERROR_INVALID_STATE;}points[count++]=next;
    }
    return XGE_ERROR_INVALID_STATE;
}
static int convex_distance(const convex *a,const convex *b,cv *normal,cv *point,double *upper,double *lower)
{return convex_distance_seed(a,b,normal,point,upper,lower,NULL,NULL);}
static int shape_valid(const xge3d_shape_t *s,int mesh)
{
    if(!s||!d3_finite3(s->center)){return 0;}
    if(s->type==XGE3D_SHAPE_MESH){return mesh&&s->mesh;}
    if(s->mesh){return 0;}
    if(s->type==XGE3D_SHAPE_BOX){return d3_finite3(s->half_extents)&&s->half_extents.x>=0&&s->half_extents.y>=0&&s->half_extents.z>=0;}
    return (s->type==XGE3D_SHAPE_SPHERE||s->type==XGE3D_SHAPE_CAPSULE)&&isfinite(s->radius)&&s->radius>=0&&isfinite(s->half_height)&&s->half_height>=0;
}
static int affine_valid(const xge3d_mat4_t *m)
{xge3d_mat4_t inverse;return d3_finite_matrix(m)&&!m->m[3]&&!m->m[7]&&!m->m[11]&&m->m[15]==1&&d3_inverse(*m,&inverse);}
xge3d_shape_t xge3dShapeDefault(int type)
{return (xge3d_shape_t){.type=type,.half_extents={.5f,.5f,.5f},.radius=.5f,.half_height=.5f};}
int xge3dNodeSetCollider(xge3d_scene *s,xge3d_node_t h,const xge3d_shape_t *shape)
{
    xge3d_node_data *n=d3_node(s,h);if(!n||(shape&&!shape_valid(shape,1))){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_shape_t *copy=NULL;if(shape){copy=xrtMalloc(sizeof(*copy));if(!copy){return XGE_ERROR_OUT_OF_MEMORY;}*copy=*shape;
        if(copy->mesh&&!d3_mesh_retain(copy->mesh)){xrtFree(copy);return XGE_ERROR_INVALID_STATE;}}
    if(n->collider){xge3dMeshFree(n->collider->mesh);xrtFree(n->collider);}n->collider=copy;s->collision_rebuild=1;return XGE_OK;
}
int xge3dNodeGetCollider(xge3d_scene *s,xge3d_node_t h,xge3d_shape_t *out)
{if(!out){return XGE_ERROR_INVALID_ARGUMENT;}memset(out,0,sizeof(*out));xge3d_node_data *n=d3_node(s,h);if(!n){return XGE_ERROR_INVALID_ARGUMENT;}if(!n->collider){return XGE_ERROR_NOT_FOUND;}*out=*n->collider;return XGE_OK;}
static xge3d_aabb_t convex_bounds(const convex *c)
{
    xge3d_aabb_t box;cv x=support(c,(cv){1,0,0}),y=support(c,(cv){0,1,0}),z=support(c,(cv){0,0,1});box.max=(xge3d_vec3_t){(float)x.x,(float)y.y,(float)z.z};
    x=support(c,(cv){-1,0,0});y=support(c,(cv){0,-1,0});z=support(c,(cv){0,0,-1});box.min=(xge3d_vec3_t){(float)x.x,(float)y.y,(float)z.z};return box;
}
static xge3d_aabb_t transform_bounds(xge3d_aabb_t box,xge3d_mat4_t world)
{xge3d_aabb_t out={0};for(int i=0;i<8;++i){xge3d_vec3_t p=d3_point(world,(xge3d_vec3_t){i&1?box.max.x:box.min.x,i&2?box.max.y:box.min.y,i&4?box.max.z:box.min.z});if(!i){out.min=out.max=p;}else d3_bounds_point(&out,p);}return out;}
static int collider_bounds(xge3d_scene *s,xge3d_node_data *n,xge3d_aabb_t *out)
{
    xge3d_mat4_t world;int result=xge3dNodeGetWorldMatrix(s,n->handle,&world);if(result){return result;}if(!affine_valid(&world)){return XGE_ERROR_UNSUPPORTED;}
    if(n->collider->type==XGE3D_SHAPE_MESH){cv shift=cvpoint(&world,cvfrom(n->collider->center));world.m[12]=(float)shift.x;world.m[13]=(float)shift.y;world.m[14]=(float)shift.z;*out=transform_bounds(n->collider->mesh->bounds,world);}
    else *out=convex_bounds(&(convex){.shape=n->collider,.world=world});
    return d3_finite3(out->min)&&d3_finite3(out->max)?XGE_OK:XGE_ERROR_INVALID_ARGUMENT;
}
void d3_collision_dirty(xge3d_scene *s,xge3d_node_data *n)
{if(!n->collider||n->collision_leaf<0||n->collision_queued||s->collision_rebuild){return;}if(!xrtArrayPush(&s->collision_pending,&n->handle)){s->collision_rebuild=1;}else n->collision_queued=1;}
static xge3d_aabb_t union_box(xge3d_aabb_t a,xge3d_aabb_t b){d3_bounds_point(&a,b.min);d3_bounds_point(&a,b.max);return a;}
#define COMPARE(axis) static int compare_##axis(const void *a,const void *b){const d3_spatial_item *p=a,*q=b;double x=(double)p->box.min.axis+p->box.max.axis,y=(double)q->box.min.axis+q->box.max.axis;return x<y?-1:x>y?1:0;}
COMPARE(x) COMPARE(y) COMPARE(z)
static int build_tree(xarray *tree,xarray *items,size_t first,size_t count,int parent,int *next,xge3d_scene *s)
{
    int at=(*next)++;d3_spatial_branch *b=xrtArrayGet(tree,(size_t)at);d3_spatial_item *p=(d3_spatial_item*)items->Data;b->parent=parent;b->left=b->right=-1;b->node=(xge3d_node_t){0};b->box=p[first].box;
    for(size_t i=1;i<count;++i){b->box=union_box(b->box,p[first+i].box);}
    if(count==1){b->node=p[first].node;if(s){d3_node(s,b->node)->collision_leaf=at;}return at;}
    xge3d_vec3_t e=d3_sub(b->box.max,b->box.min);qsort(p+first,count,sizeof(*p),e.x>=e.y&&e.x>=e.z?compare_x:e.y>=e.z?compare_y:compare_z);
    b->left=build_tree(tree,items,first,count/2,at,next,s);b->right=build_tree(tree,items,first+count/2,count-count/2,at,next,s);return at;
}
static int prepare_colliders(xge3d_scene *s)
{
    uint64_t version=xrtAtomic64Load(&d3_geometry_version,XMEMORY_ACQUIRE);
    if(s->collision_geometry_version!=version){s->collision_rebuild=1;}
    if(s->collision_rebuild){xrtArrayClear(&s->collision_items);
        for(size_t i=0;i<s->nodes.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);n->collision_leaf=-1;n->collision_queued=0;if(!n->collider||!d3_node(s,n->handle)){continue;}
            d3_spatial_item item={.node=n->handle};int result=collider_bounds(s,n,&item.box);if(result){return result;}if(!xrtArrayPush(&s->collision_items,&item)){return XGE_ERROR_OUT_OF_MEMORY;}}
        size_t count=s->collision_items.Count;if(count>(INT_MAX-1u)/2u){return XGE_ERROR_UNSUPPORTED;}
        if(!xrtArrayResize(&s->collision_tree,count?count*2-1:0)){return XGE_ERROR_OUT_OF_MEMORY;}
        int next=0;s->collision_root=count?build_tree(&s->collision_tree,&s->collision_items,0,count,-1,&next,s):-1;
        s->collision_geometry_version=version;s->collision_rebuild=0;xrtArrayClear(&s->collision_pending);
    }else{for(size_t i=0;i<s->collision_pending.Count;++i){xge3d_node_data *n=d3_node(s,*(xge3d_node_t*)xrtArrayGet(&s->collision_pending,i));if(!n||!n->collider){continue;}
            d3_spatial_branch *b=xrtArrayGet(&s->collision_tree,(size_t)n->collision_leaf);int result=collider_bounds(s,n,&b->box);if(result){return result;}
            int parent=b->parent;while(parent>=0){b=xrtArrayGet(&s->collision_tree,(size_t)parent);b->box=union_box(((d3_spatial_branch*)s->collision_tree.Data)[b->left].box,((d3_spatial_branch*)s->collision_tree.Data)[b->right].box);parent=b->parent;}n->collision_queued=0;}
        xrtArrayClear(&s->collision_pending);}
    return XGE_OK;
}
struct d3_triangle_bvh {xarray tree;uint64_t version;};
void d3_triangle_bvh_free(struct d3_triangle_bvh *b){if(b){xrtArrayUnit(&b->tree);xrtFree(b);}}
static void mesh_triangle(const xge3d_mesh *m,size_t triangle,xge3d_vec3_t p[3])
{for(int j=0;j<3;++j){size_t i=triangle*3+(size_t)j;if(m->indices.Count){i=m->index_bits==16?((const uint16_t*)m->indices.Data)[i]:((const uint32_t*)m->indices.Data)[i];}p[j]=((const xge3d_vertex_t*)m->vertices.Data)[i].position;}}
static int prepare_triangles(xge3d_mesh *m)
{
    if(m->collision_bvh&&m->collision_bvh->version==m->version){return XGE_OK;}
    size_t count=(m->indices.Count?m->indices.Count:m->vertices.Count)/3;if(!count||count>(INT_MAX-1u)/2u){return XGE_ERROR_UNSUPPORTED;}
    struct d3_triangle_bvh *b=xrtMalloc(sizeof(*b));if(!b){return XGE_ERROR_OUT_OF_MEMORY;}memset(b,0,sizeof(*b));xrtArrayInit(&b->tree,sizeof(d3_spatial_branch));xarray items;xrtArrayInit(&items,sizeof(d3_spatial_item));int result=XGE_ERROR_OUT_OF_MEMORY;
    if(!xrtArrayResize(&items,count)||!xrtArrayResize(&b->tree,count*2-1)){goto done;}
    for(size_t i=0;i<count;++i){xge3d_vec3_t p[3];mesh_triangle(m,i,p);d3_spatial_item *item=xrtArrayGet(&items,i);item->box=(xge3d_aabb_t){p[0],p[0]};d3_bounds_point(&item->box,p[1]);d3_bounds_point(&item->box,p[2]);item->node=(xge3d_node_t){i+1,0};}
    {int next=0;build_tree(&b->tree,&items,0,count,-1,&next,NULL);}b->version=m->version;d3_triangle_bvh_free(m->collision_bvh);m->collision_bvh=b;b=NULL;result=XGE_OK;
done:xrtArrayUnit(&items);d3_triangle_bvh_free(b);return result;
}
#include "xge3d_penetration.inc"
static int test_convex(convex query,const convex *target,cv motion,int sweep,xge3d_collision_hit_t *hit)
{
    double time=0;cv start=query.offset,normal={0},point={0};
    for(int i=0;i<128;++i){query.offset=cvadd(start,cvscale(motion,time));double upper,lower;int result=convex_distance(&query,target,&normal,&point,&upper,&lower);if(result){return result;}
        if(upper<=1e-5){hit->fraction=(float)time;hit->distance=(float)(time*sqrt(cvdot(motion,motion)));hit->position=cvout(point);
#if XGE3D_ENABLE_MOTION
            if(time==0&&sweep==2){
                if(upper<1e-8){double depth;result=convex_penetration(&query,target,&normal,&point,&depth);if(result)return result;}
                if(cvdot(motion,normal)>=-1e-10)return XGE_ERROR_NOT_FOUND;
            }else
#endif
            if(time==0&&upper<1e-9){cv backwards=cvscale(motion,-1);double length=sqrt(cvdot(backwards,backwards));if(length>0){normal=cvscale(backwards,1/length);}}
            hit->normal=cvout(normal);hit->started_overlapping=time==0;return XGE_OK;}
        if(!sweep){return XGE_ERROR_NOT_FOUND;}
        double closing=-cvdot(motion,normal);if(closing<=1e-14){return XGE_ERROR_NOT_FOUND;}
        double step=fmax(0,lower-5e-6)/closing;if(step<=1e-14){return XGE_ERROR_INVALID_STATE;}time+=step;
        if(time>1+1e-10){return XGE_ERROR_NOT_FOUND;}if(time>1){time=1;}
    }return XGE_ERROR_INVALID_STATE;
}
static int collider_test(xge3d_scene *s,xge3d_node_data *n,convex query,xge3d_aabb_t swept,cv motion,int sweep,xge3d_collision_hit_t *hit)
{
    xge3d_mat4_t world;int result=xge3dNodeGetWorldMatrix(s,n->handle,&world);if(result){return result;}hit->triangle=SIZE_MAX;
    if(n->collider->type!=XGE3D_SHAPE_MESH){return test_convex(query,&(convex){.shape=n->collider,.world=world},motion,sweep,hit);}
    xge3d_mesh *m=n->collider->mesh;result=prepare_triangles(m);if(result){return result;}
    /* Mesh center is a local translation, including under rotated/sheared nodes. */
    cv shift=cvpoint(&world,cvfrom(n->collider->center));world.m[12]=(float)shift.x;world.m[13]=(float)shift.y;world.m[14]=(float)shift.z;
    xge3d_mat4_t inverse;if(!d3_inverse(world,&inverse)){return XGE_ERROR_UNSUPPORTED;}xge3d_aabb_t local=transform_bounds(swept,inverse);
    int stack[64],top=0;stack[top++]=0;int found=0;
    while(top){d3_spatial_branch *b=xrtArrayGet(&m->collision_bvh->tree,(size_t)stack[--top]);if(!xge3dAabbIntersects(&local,&b->box)){continue;}
        if(b->left>=0){stack[top++]=b->left;stack[top++]=b->right;continue;}
        xge3d_vec3_t p[3];mesh_triangle(m,b->node.slot-1,p);convex target={.tri=1};for(int i=0;i<3;++i){target.triangle[i]=cvpoint(&world,cvfrom(p[i]));}
        xge3d_collision_hit_t next={0};++s->collision_tests;result=test_convex(query,&target,motion,sweep,&next);
        if(result==XGE_ERROR_NOT_FOUND){continue;}if(result){return result;}
        if(!found||next.fraction<hit->fraction){*hit=next;hit->triangle=b->node.slot-1;found=1;}if(!sweep){break;}
    }return found?XGE_OK:XGE_ERROR_NOT_FOUND;
}
static int query_collision(xge3d_scene *s,const xge3d_shape_t *shape,const xge3d_mat4_t *world,xge3d_vec3_t movement,const xge3d_collision_filter_t *filter,
    xge3d_node_t *nodes,size_t capacity,size_t *count,xge3d_collision_hit_t *hit,int sweep)
{
    if(count){*count=0;}if(hit){memset(hit,0,sizeof(*hit));}
    if(!s||!shape_valid(shape,0)||!affine_valid(world)||!d3_finite3(movement)||(capacity&&!nodes)){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_node_data *excluded=NULL;if(filter&&(filter->exclude.slot||filter->exclude.scene)){excluded=d3_node(s,filter->exclude);if(!excluded){return XGE_ERROR_INVALID_ARGUMENT;}}
    int result=prepare_colliders(s);if(result){return result;}
    convex query={.shape=shape,.world=*world};xge3d_aabb_t bounds=convex_bounds(&query),end=convex_bounds(&(convex){.shape=shape,.world=*world,.offset=cvfrom(movement)});bounds=union_box(bounds,end);
    float *lo=&bounds.min.x,*hi=&bounds.max.x;for(int i=0;i<3;++i){lo[i]=nextafterf(lo[i]-1e-5f,-INFINITY);hi[i]=nextafterf(hi[i]+1e-5f,INFINITY);}
    if(!d3_finite3(bounds.min)||!d3_finite3(bounds.max)){return XGE_ERROR_INVALID_ARGUMENT;}
    int stack[64],top=0,found=0;if(s->collision_root>=0){stack[top++]=s->collision_root;}
    while(top){d3_spatial_branch *b=xrtArrayGet(&s->collision_tree,(size_t)stack[--top]);if(!xge3dAabbIntersects(&bounds,&b->box)){continue;}
        if(b->left>=0){stack[top++]=b->left;stack[top++]=b->right;continue;}
        xge3d_node_data *n=d3_node(s,b->node);if(!n||!n->collider||!(n->query_layers&(filter?filter->layers:UINT32_MAX))){continue;}
        int skip=0;for(xge3d_node_data *p=n;excluded&&p;p=p->parent_node)if(p==excluded){skip=1;break;}if(skip){continue;}
        xge3d_collision_hit_t next={0};result=collider_test(s,n,query,bounds,cvfrom(movement),sweep,&next);if(result==XGE_ERROR_NOT_FOUND){continue;}if(result){if(count){*count=0;}if(hit){memset(hit,0,sizeof(*hit));}return result;}
        next.node=n->handle;if(sweep){if(!found||next.fraction<hit->fraction){*hit=next;}found=1;}
        else{if(*count<capacity){nodes[*count]=n->handle;}++*count;}
    }
    return sweep?(found?XGE_OK:XGE_ERROR_NOT_FOUND):(nodes&&*count>capacity?XGE_ERROR_BUFFER_TOO_SMALL:XGE_OK);
}
int xge3dSceneOverlapShape(xge3d_scene *s,const xge3d_shape_t *shape,const xge3d_mat4_t *world,const xge3d_collision_filter_t *filter,xge3d_node_t *nodes,size_t capacity,size_t *count)
{if(!count){return XGE_ERROR_INVALID_ARGUMENT;}return query_collision(s,shape,world,(xge3d_vec3_t){0},filter,nodes,capacity,count,NULL,0);}
int xge3dSceneSweepShape(xge3d_scene *s,const xge3d_shape_t *shape,const xge3d_mat4_t *world,xge3d_vec3_t motion,const xge3d_collision_filter_t *filter,xge3d_collision_hit_t *out)
{if(!out){return XGE_ERROR_INVALID_ARGUMENT;}return query_collision(s,shape,world,motion,filter,NULL,0,NULL,out,1);}
int xge3dSceneCollisionRaycast(xge3d_scene *s,const xge3d_ray_t *ray,float max,const xge3d_collision_filter_t *filter,xge3d_collision_hit_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}memset(out,0,sizeof(*out));if(!ray||!d3_finite3(ray->origin)||!d3_finite3(ray->direction)||!isfinite(max)||max<=0){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_vec3_t d=ray->direction;if(!d3_normalize(&d)){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_shape_t point=xge3dShapeDefault(XGE3D_SHAPE_SPHERE);point.radius=0;point.center=ray->origin;xge3d_mat4_t world=d3_identity();return xge3dSceneSweepShape(s,&point,&world,d3_scale(d,max),filter,out);
}
int xge3dSceneCollisionStats(xge3d_scene *s,xge3d_collision_stats_t *out)
{
    if(!s||!out){return XGE_ERROR_INVALID_ARGUMENT;}memset(out,0,sizeof(*out));out->triangle_tests=s->collision_tests;xmap seen;xrtMapInit(&seen,sizeof(void*));
    for(size_t i=0;i<s->nodes.Count;++i){xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i);if(!n->collider||!d3_node(s,n->handle)){continue;}++out->colliders;
        xge3d_mesh *m=n->collider->mesh;if(!m||!m->collision_bvh){continue;}xbytesview key={(const unsigned char*)&m,sizeof(m)};
        if(xrtMapGet(&seen,key)){continue;}if(!xrtMapSetPtr(&seen,key,m)){xrtMapUnit(&seen);return XGE_ERROR_OUT_OF_MEMORY;}
        out->triangle_bvh_bytes+=sizeof(*m->collision_bvh)+m->collision_bvh->tree.Count*sizeof(d3_spatial_branch);}
    xrtMapUnit(&seen);return XGE_OK;
}
#include "xge3d_motion.inc"
#endif
