#include "xge3d_motion_internal.h"

typedef struct bone_map { size_t source,target;const char *semantic;int translation; } bone_map;
static xvalue *field(const xvalue *v,const char *key) {return xrtValueObjectGet(v,xrtStrView(key));}
static const char *text(const xvalue *v,const char *key)
{
    xstrview value;if (!xrtValueGetString(field(v,key),&value) || !value.Size || memchr(value.Data,0,value.Size)) return NULL;
    return value.Data;
}
static size_t node_named(const motion_asset *a,const char *name)
{
    size_t found=SIZE_MAX;if (!name) return found;
    for (size_t i=0;i<a->nodes.Count;++i) if (!strcmp(((motion_node*)a->nodes.Data)[i].name,name)) {
        if (found!=SIZE_MAX) return SIZE_MAX;found=i;
    }
    return found;
}
static int reference_value(const xvalue *v,const char *key,double *out,size_t count)
{
    xvalue *array=field(v,key);if (!array) return 1;
    if (xrtValueCount(array)!=count) return 0;
    for (size_t i=0;i<count;++i) if (!xrtValueGetFloat(xrtValueArrayGet(array,i),&out[i]) || !isfinite(out[i])) return 0;
    if (count==4) {double length=0;for (size_t i=0;i<count;++i) length+=out[i]*out[i];if (length<1e-20 || !isfinite(length)) return 0;
        length=sqrt(length);for (size_t i=0;i<count;++i) out[i]/=length;}
    return 1;
}
static int order_nodes(const motion_asset *a,xarray *order)
{
    xrtArrayInit(order,sizeof(size_t));
    if (!xrtArrayReserve(order,a->nodes.Count)) return 0;
    while (order->Count<a->nodes.Count) {
        size_t old=order->Count;
        for (size_t i=0;i<a->nodes.Count;++i) {
            size_t parent=((motion_node*)a->nodes.Data)[i].parent;int present=0,ready=parent==SIZE_MAX;
            for (size_t j=0;j<order->Count;++j) {size_t n=((size_t*)order->Data)[j];if (n==i) present=1;if (n==parent) ready=1;}
            if (!present && ready && !xrtArrayPush(order,&i)) return 0;
        }
        if (order->Count==old) return 0;
    }
    return 1;
}
static void world_matrices(const motion_asset *a,const xarray *order,const ufbx_transform *poses,ufbx_matrix *world)
{
    for (size_t i=0;i<order->Count;++i) {
        size_t n=((size_t*)order->Data)[i],parent=((motion_node*)a->nodes.Data)[n].parent;
        world[n]=ufbx_transform_to_matrix(&poses[n]);if (parent!=SIZE_MAX) world[n]=ufbx_matrix_mul(&world[parent],&world[n]);
    }
}
static ufbx_quat inverse_rotation(ufbx_quat q) {return (ufbx_quat){.x=-q.x,.y=-q.y,.z=-q.z,.w=q.w};}
static int rigid_chain(const motion_asset *a,size_t node,const ufbx_transform *poses)
{
    for (size_t depth=0;node!=SIZE_MAX && depth<a->nodes.Count;++depth) {
        ufbx_vec3 scale=poses[node].scale;
        if (scale.x<=0 || fabs(scale.x-scale.y)>1e-5*scale.x || fabs(scale.x-scale.z)>1e-5*scale.x) return 0;
        node=((motion_node*)a->nodes.Data)[node].parent;
    }
    return node==SIZE_MAX;
}
int motion_retarget(motion_asset *a,const char *path,const char *map_path)
{
    int ok=0;motion_asset target;motion_init(&target);motion_options options={.reference=1};
    xarray map,source_order={0},target_order={0},source_rest,target_rest,source_world,target_world,source_frame,target_frame;
    xrtArrayInit(&map,sizeof(bone_map));xrtArrayInit(&source_rest,sizeof(ufbx_transform));xrtArrayInit(&target_rest,sizeof(ufbx_transform));
    xrtArrayInit(&source_world,sizeof(ufbx_matrix));xrtArrayInit(&target_world,sizeof(ufbx_matrix));xrtArrayInit(&source_frame,sizeof(ufbx_matrix));xrtArrayInit(&target_frame,sizeof(ufbx_matrix));
    xvalue *config=NULL;unsigned char *raw=NULL;size_t size=0;
    if (!motion_load(path,&options,&target)) goto done;
    raw=xrtFileReadAllLimit(map_path,1024u*1024u,&size);if (!raw) goto done;
    config=xrtJsonParse(xrtStrViewN((const char*)raw,size));if (!config) goto done;
    xvalue *bones=field(config,"bones");size_t count=xrtValueCount(bones);
    if (!count || count>256 || !order_nodes(a,&source_order) || !order_nodes(&target,&target_order)) goto done;
    if (!xrtArrayResize(&source_rest,a->nodes.Count) || !xrtArrayResize(&target_rest,target.nodes.Count) ||
        !xrtArrayResize(&source_world,a->nodes.Count) || !xrtArrayResize(&target_world,target.nodes.Count) ||
        !xrtArrayResize(&source_frame,a->nodes.Count) || !xrtArrayResize(&target_frame,target.nodes.Count)) goto done;
    for (size_t i=0;i<a->nodes.Count;++i) ((ufbx_transform*)source_rest.Data)[i]=((motion_node*)a->nodes.Data)[i].rest;
    for (size_t i=0;i<target.nodes.Count;++i) ((ufbx_transform*)target_rest.Data)[i]=((motion_node*)target.nodes.Data)[i].rest;
    xvalue *reference_time=field(config,"source_reference_time");
    if (reference_time) {
        double time;if (!xrtValueGetFloat(reference_time,&time) || !isfinite(time) || time<0 || !a->times.Count || time>((float*)a->times.Data)[a->times.Count-1]) goto done;
        size_t lo=0;while (lo+1<a->times.Count && ((float*)a->times.Data)[lo+1]<=time) ++lo;
        size_t hi=lo+1<a->times.Count ? lo+1 : lo;double span=((float*)a->times.Data)[hi]-((float*)a->times.Data)[lo];
        double weight=span>0 ? (time-((float*)a->times.Data)[lo])/span : 0;
        for (size_t i=0;i<a->nodes.Count;++i) {
            ufbx_transform x=((ufbx_transform*)a->poses.Data)[lo*a->nodes.Count+i],y=((ufbx_transform*)a->poses.Data)[hi*a->nodes.Count+i];
            for (int k=0;k<3;++k) {x.translation.v[k]=x.translation.v[k]*(1-weight)+y.translation.v[k]*weight;x.scale.v[k]=x.scale.v[k]*(1-weight)+y.scale.v[k]*weight;}
            x.rotation=ufbx_quat_slerp(x.rotation,y.rotation,weight);((ufbx_transform*)source_rest.Data)[i]=x;
        }
    }
    for (size_t i=0;i<count;++i) {
        xvalue *entry=xrtValueArrayGet(bones,i);const char *semantic=text(entry,"semantic"),*source=text(entry,"source"),*dest=text(entry,"target");
        bone_map b={.source=node_named(a,source),.target=node_named(&target,dest),.semantic=semantic};
        if (!semantic || b.source==SIZE_MAX || b.target==SIZE_MAX) {fprintf(stderr,"Missing/ambiguous mapping: %s / %s -> %s\n",semantic ? semantic : "?",source ? source : "?",dest ? dest : "?");goto done;}
        for (size_t j=0;j<map.Count;++j) {bone_map *old=xrtArrayGet(&map,j);
            if (old->source==b.source || old->target==b.target || !strcmp(old->semantic,semantic)) {fprintf(stderr,"Duplicate mapping: %s\n",semantic);goto done;}}
        b.translation=!strcmp(semantic,"root") || !strcmp(semantic,"hips");
        bool translate;xvalue *translation=field(entry,"translation");if (translation) {if (!xrtValueGetBool(translation,&translate)) goto done;b.translation=translate;}
        if (b.translation && strcmp(semantic,"root") && strcmp(semantic,"hips")) {fprintf(stderr,"Translation is limited to root/hips to preserve bone lengths\n");goto done;}
        if (field(entry,"target_reference_translation") && strcmp(semantic,"root") && strcmp(semantic,"hips")) goto done;
        ufbx_transform *s=xrtArrayGet(&source_rest,b.source),*t=xrtArrayGet(&target_rest,b.target);
        if (!reference_value(entry,"source_reference_rotation",s->rotation.v,4) || !reference_value(entry,"target_reference_rotation",t->rotation.v,4) ||
            !reference_value(entry,"source_reference_translation",s->translation.v,3) || !reference_value(entry,"target_reference_translation",t->translation.v,3) || !xrtArrayPush(&map,&b)) goto done;
    }
    const char *required[]={"hips","head","left_foot","right_foot","left_upper_arm","right_upper_arm","left_lower_arm","right_lower_arm","left_upper_leg","right_upper_leg","left_lower_leg","right_lower_leg"};
    size_t indices[12];
    for (int i=0;i<12;++i) {
        indices[i]=SIZE_MAX;for (size_t j=0;j<map.Count;++j) if (!strcmp(((bone_map*)map.Data)[j].semantic,required[i])) indices[i]=j;
        if (indices[i]==SIZE_MAX) {fprintf(stderr,"Missing humanoid semantic: %s\n",required[i]);goto done;}
    }
    world_matrices(a,&source_order,(ufbx_transform*)source_rest.Data,(ufbx_matrix*)source_world.Data);
    world_matrices(&target,&target_order,(ufbx_transform*)target_rest.Data,(ufbx_matrix*)target_world.Data);
    bone_map *m=(bone_map*)map.Data;ufbx_matrix *sw=(ufbx_matrix*)source_world.Data,*tw=(ufbx_matrix*)target_world.Data;
    double source_height=sw[m[indices[1]].source].cols[3].y-(sw[m[indices[2]].source].cols[3].y+sw[m[indices[3]].source].cols[3].y)*.5;
    double target_height=tw[m[indices[1]].target].cols[3].y-(tw[m[indices[2]].target].cols[3].y+tw[m[indices[3]].target].cols[3].y)*.5;
    double ratio=target_height/source_height;xvalue *scale=field(config,"translation_scale");
    if (scale && !xrtValueGetFloat(scale,&ratio)) goto done;
    if (!isfinite(ratio) || ratio<=0 || source_height<=1e-6 || target_height<=1e-6) {fprintf(stderr,"Invalid Y-up reference height or translation scale\n");goto done;}
    for (size_t i=0;i<map.Count;++i) if (!rigid_chain(a,m[i].source,(ufbx_transform*)source_rest.Data) || !rigid_chain(&target,m[i].target,(ufbx_transform*)target_rest.Data)) {
        fprintf(stderr,"Nonuniform/reflected skeleton scale: %s\n",m[i].semantic);goto done;}
    if (!a->times.Count || a->times.Count>MOTION_MAX_BYTES/sizeof(ufbx_transform)/target.nodes.Count ||
        !xrtArrayResize(&target.times,a->times.Count) || !xrtArrayResize(&target.poses,a->times.Count*target.nodes.Count)) goto done;
    memcpy(target.times.Data,a->times.Data,a->times.Count*sizeof(float));xrtFree(target.name);target.name=motion_string(a->name);if (!target.name) goto done;
    for (size_t f=0;f<a->times.Count;++f) {
        const ufbx_transform *source=(ufbx_transform*)a->poses.Data+f*a->nodes.Count;ufbx_transform *poses=(ufbx_transform*)target.poses.Data+f*target.nodes.Count;
        ufbx_matrix *sf=(ufbx_matrix*)source_frame.Data,*tf=(ufbx_matrix*)target_frame.Data;
        world_matrices(a,&source_order,source,sf);
        for (size_t i=0;i<target_order.Count;++i) {
            size_t n=((size_t*)target_order.Data)[i],parent=((motion_node*)target.nodes.Data)[n].parent;poses[n]=((motion_node*)target.nodes.Data)[n].rest;
            for (size_t j=0;j<map.Count;++j) if (m[j].target==n) {
                size_t sn=m[j].source,sp=((motion_node*)a->nodes.Data)[sn].parent;
                if (!rigid_chain(a,sn,source)) {fprintf(stderr,"Animated skeleton scale unsupported: %s\n",m[j].semantic);goto done;}
                ufbx_quat sr=ufbx_matrix_to_transform(&sw[sn]).rotation,sa=ufbx_matrix_to_transform(&sf[sn]).rotation,tr=ufbx_matrix_to_transform(&tw[n]).rotation;
                ufbx_quat desired=ufbx_quat_mul(sa,ufbx_quat_mul(inverse_rotation(sr),tr));
                poses[n].rotation=parent==SIZE_MAX ? desired : ufbx_quat_mul(inverse_rotation(ufbx_matrix_to_transform(&tf[parent]).rotation),desired);
                if (m[j].translation) {
                    ufbx_vec3 delta=source[sn].translation,reference=((ufbx_transform*)source_rest.Data)[sn].translation;
                    for (int k=0;k<3;++k) delta.v[k]-=reference.v[k];
                    if (sp!=SIZE_MAX) delta=ufbx_transform_direction(&sw[sp],delta);
                    for (int k=0;k<3;++k) delta.v[k]*=ratio;
                    if (parent!=SIZE_MAX) {ufbx_matrix inverse=ufbx_matrix_invert(&tf[parent]);delta=ufbx_transform_direction(&inverse,delta);}
                    for (int k=0;k<3;++k) poses[n].translation.v[k]=((ufbx_transform*)target_rest.Data)[n].translation.v[k]+delta.v[k];
                }
                break;
            }
            tf[n]=ufbx_transform_to_matrix(&poses[n]);if (parent!=SIZE_MAX) tf[n]=ufbx_matrix_mul(&tf[parent],&tf[n]);
        }
    }
    printf("retarget: %zu semantic bones, translation scale %.9g, %zu -> %zu nodes\n",map.Count,ratio,a->nodes.Count,target.nodes.Count);
    motion_free(a);*a=target;memset(&target,0,sizeof(target));ok=1;
done:
    if (!ok) fprintf(stderr,"Cannot retarget using %s\n",map_path);
    xrtFree(raw);xrtValueRelease(config);motion_free(&target);xrtArrayUnit(&map);xrtArrayUnit(&source_order);xrtArrayUnit(&target_order);
    xrtArrayUnit(&source_rest);xrtArrayUnit(&target_rest);xrtArrayUnit(&source_world);xrtArrayUnit(&target_world);xrtArrayUnit(&source_frame);xrtArrayUnit(&target_frame);return ok;
}
