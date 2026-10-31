#if XGE3D_ENABLE_MOTION
static int collider_contact(xge3d_scene *s,xge3d_node_data *n,convex query,xge3d_aabb_t bounds,xge3d_contact_t *out)
{
    xge3d_mat4_t world;int result=xge3dNodeGetWorldMatrix(s,n->handle,&world);if(result){return result;}
    cv normal,point;double depth;
    if(n->collider->type!=XGE3D_SHAPE_MESH){
        result=convex_penetration(&query,&(convex){.shape=n->collider,.world=world},&normal,&point,&depth);
        if(!result){*out=(xge3d_contact_t){.hit={.node=n->handle,.position=cvout(point),.normal=cvout(normal),.triangle=SIZE_MAX,.started_overlapping=depth>0},.depth=(float)depth};}return result;
    }
    xge3d_mesh *m=n->collider->mesh;result=prepare_triangles(m);if(result){return result;}
    cv shift=cvpoint(&world,cvfrom(n->collider->center));world.m[12]=(float)shift.x;world.m[13]=(float)shift.y;world.m[14]=(float)shift.z;
    xge3d_mat4_t inverse;if(!d3_inverse(world,&inverse)){return XGE_ERROR_UNSUPPORTED;}xge3d_aabb_t local=transform_bounds(bounds,inverse);
    int stack[64],top=0,found=0;stack[top++]=0;
    while(top){d3_spatial_branch *b=xrtArrayGet(&m->collision_bvh->tree,(size_t)stack[--top]);if(!xge3dAabbIntersects(&local,&b->box)){continue;}
        if(b->left>=0){stack[top++]=b->left;stack[top++]=b->right;continue;}
        xge3d_vec3_t p[3];mesh_triangle(m,b->node.slot-1,p);convex target={.tri=1};for(int i=0;i<3;++i){target.triangle[i]=cvpoint(&world,cvfrom(p[i]));}
        ++s->collision_tests;result=convex_penetration(&query,&target,&normal,&point,&depth);if(result==XGE_ERROR_NOT_FOUND){continue;}if(result){return result;}
        if(!found||depth>out->depth){*out=(xge3d_contact_t){.hit={.node=n->handle,.position=cvout(point),.normal=cvout(normal),.triangle=b->node.slot-1,.started_overlapping=depth>0},.depth=(float)depth};}found=1;
    }return found?XGE_OK:XGE_ERROR_NOT_FOUND;
}
static int contact_query(xge3d_scene *s,const xge3d_shape_t *shape,const xge3d_mat4_t *world,const xge3d_collision_filter_t *filter,
    xge3d_contact_t *contacts,size_t capacity,size_t *count,xge3d_contact_t *deepest)
{
    if(count){*count=0;}
    if(!s||!shape_valid(shape,0)||!affine_valid(world)||!count||(capacity&&!contacts)){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_node_data *excluded=NULL;if(filter&&(filter->exclude.slot||filter->exclude.scene)){excluded=d3_node(s,filter->exclude);if(!excluded){return XGE_ERROR_INVALID_ARGUMENT;}}
    int result=prepare_colliders(s);if(result){return result;}convex query={.shape=shape,.world=*world};xge3d_aabb_t bounds=convex_bounds(&query);
    if(!d3_finite3(bounds.min)||!d3_finite3(bounds.max)){return XGE_ERROR_INVALID_ARGUMENT;}
    for(int i=0;i<3;++i){(&bounds.min.x)[i]=nextafterf((&bounds.min.x)[i]-1e-5f,-INFINITY);(&bounds.max.x)[i]=nextafterf((&bounds.max.x)[i]+1e-5f,INFINITY);}
    int stack[64],top=0;if(s->collision_root>=0){stack[top++]=s->collision_root;}
    while(top){d3_spatial_branch *b=xrtArrayGet(&s->collision_tree,(size_t)stack[--top]);if(!xge3dAabbIntersects(&bounds,&b->box)){continue;}
        if(b->left>=0){stack[top++]=b->left;stack[top++]=b->right;continue;}
        xge3d_node_data *n=d3_node(s,b->node);if(!n||!n->collider||!(n->query_layers&(filter?filter->layers:UINT32_MAX))){continue;}
        int skip=0;for(xge3d_node_data *p=n;excluded&&p;p=p->parent_node){if(p==excluded){skip=1;break;}}if(skip){continue;}
        xge3d_contact_t c={0};result=collider_contact(s,n,query,bounds,&c);if(result==XGE_ERROR_NOT_FOUND){continue;}if(result){*count=0;return result;}
        if(deepest&&(!*count||c.depth>deepest->depth)){*deepest=c;}if(*count<capacity){contacts[*count]=c;}++*count;
    }return contacts&&*count>capacity?XGE_ERROR_BUFFER_TOO_SMALL:XGE_OK;
}
int xge3dSceneContactShape(xge3d_scene *s,const xge3d_shape_t *shape,const xge3d_mat4_t *world,
    const xge3d_collision_filter_t *filter,xge3d_contact_t *out,size_t capacity,size_t *count)
{return contact_query(s,shape,world,filter,out,capacity,count,NULL);}
xge3d_motion_settings_t xge3dMotionDefault(void)
{return (xge3d_motion_settings_t){sizeof(xge3d_motion_settings_t),8,.001f,1};}
static int motion_normal(xge3d_motion_result_t *r,xge3d_vec3_t n)
{
    if(!d3_normalize(&n)){return 0;}for(int i=0;i<r->contact_count;++i){if(d3_dot(n,r->normals[i])>.9999f){return 1;}}
    if(r->contact_count>=8){return 0;}r->normals[r->contact_count++]=n;return 1;
}
static int motion_allowed(xge3d_vec3_t v,const xge3d_motion_result_t *r)
{for(int i=0;i<r->contact_count;++i){if(d3_dot(v,r->normals[i])< -1e-6f){return 0;}}return 1;}
static xge3d_vec3_t motion_project(xge3d_vec3_t v,const xge3d_motion_result_t *r)
{
    if(motion_allowed(v,r)){return v;}xge3d_vec3_t best={0};float distance=d3_dot(v,v);
    for(int i=0;i<r->contact_count;++i){xge3d_vec3_t n=r->normals[i],candidate=d3_sub(v,d3_scale(n,d3_dot(v,n))),delta=d3_sub(v,candidate);
        float d=d3_dot(delta,delta);if(d<distance&&motion_allowed(candidate,r)){best=candidate;distance=d;}
        for(int j=0;j<i;++j){xge3d_vec3_t axis=d3_cross(n,r->normals[j]);if(!d3_normalize(&axis)){continue;}
            candidate=d3_scale(axis,d3_dot(v,axis));delta=d3_sub(v,candidate);d=d3_dot(delta,delta);
            if(d<distance&&motion_allowed(candidate,r)){best=candidate;distance=d;}
        }
    }return best;
}
int xge3dSceneMoveShape(xge3d_scene *s,const xge3d_shape_t *shape,const xge3d_mat4_t *world,xge3d_vec3_t movement,
    const xge3d_collision_filter_t *filter,const xge3d_motion_settings_t *settings,xge3d_motion_result_t *out)
{
    xge3d_motion_settings_t defaults=xge3dMotionDefault();const xge3d_motion_settings_t *d=settings?settings:&defaults;
    if(!out||out->size!=sizeof(*out)||!s||!shape_valid(shape,0)||!affine_valid(world)||!d3_finite3(movement)||
        d->size!=sizeof(*d)||d->iterations<1||d->iterations>32||!isfinite(d->margin)||d->margin<.0001f||d->margin>1||
        !isfinite(d->max_recovery)||d->max_recovery<=0||d->max_recovery>1000||!isfinite(d3_dot(movement,movement))){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_motion_result_t r={.size=sizeof(r)};xge3d_mat4_t at=*world;float recovered=0;int ready=0,result;
    for(int i=0;i<d->iterations;++i){size_t count;xge3d_contact_t c={0};result=contact_query(s,shape,&at,filter,NULL,0,&count,&c);if(result){return result;}
        if(!count||c.depth<1e-5f){ready=1;break;}
        float distance=c.depth+d->margin;if(recovered+distance>d->max_recovery){r.flags|=XGE3D_MOTION_LIMITED;break;}
        xge3d_vec3_t correction=d3_scale(c.hit.normal,distance);r.recovery=d3_add(r.recovery,correction);recovered+=distance;
        at.m[12]+=correction.x;at.m[13]+=correction.y;at.m[14]+=correction.z;r.flags|=XGE3D_MOTION_RECOVERED;++r.iterations;
        if(!motion_normal(&r,c.hit.normal)){r.flags|=XGE3D_MOTION_LIMITED;break;}
    }
    if(!ready){size_t count;xge3d_contact_t c={0};result=contact_query(s,shape,&at,filter,NULL,0,&count,&c);if(result){return result;}ready=!count||c.depth<1e-5f;if(!ready){r.flags|=XGE3D_MOTION_LIMITED;}}
    xge3d_vec3_t moved={0},remaining=movement;
    if(ready){for(int i=0;i<d->iterations;++i){double length=sqrt((double)d3_dot(remaining,remaining));if(length<1e-7){remaining=(xge3d_vec3_t){0};break;}
        xge3d_collision_hit_t hit={0};result=query_collision(s,shape,&at,remaining,filter,NULL,0,NULL,&hit,2);if(result==XGE_ERROR_NOT_FOUND){moved=d3_add(moved,remaining);remaining=(xge3d_vec3_t){0};break;}if(result){return result;}
        r.flags|=XGE3D_MOTION_BLOCKED;++r.iterations;
        float travel=fmaxf(0,hit.fraction-d->margin/(float)length);xge3d_vec3_t step=d3_scale(remaining,travel);
        moved=d3_add(moved,step);at.m[12]+=step.x;at.m[13]+=step.y;at.m[14]+=step.z;remaining=d3_scale(remaining,1-travel);
        if(!motion_normal(&r,hit.normal)){r.flags|=XGE3D_MOTION_LIMITED;break;}remaining=motion_project(remaining,&r);
        if(i+1==d->iterations&&d3_dot(remaining,remaining)>1e-12f){r.flags|=XGE3D_MOTION_LIMITED;}
    }}
    r.displacement=d3_add(r.recovery,moved);r.remainder=d3_sub(movement,moved);if(!d3_finite3(r.displacement)){return XGE_ERROR_INVALID_ARGUMENT;}*out=r;return XGE_OK;
}
#endif
