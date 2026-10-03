#include "xge3d_internal.h"
#include "../lib/cgltf/cgltf.h"
#include <limits.h>

#if XGE3D_ENABLE_ANIMATION
typedef struct d3_track { size_t node;int path,interpolation; xarray times,values; } d3_track;
struct xge3d_clip { uint32_t refs; xge3d_model *model; const char *name; float duration; xarray tracks; };
typedef struct d3_layer { xge3d_clip *clip; double time;float speed,weight;int loop,started; xarray targets,mask,events; } d3_layer;
struct xge3d_animator {
    xge3d_scene *scene; xge3d_node_t root; xge3d_model *model;
    xarray nodes,rest,pose,matrices; d3_layer layers[2];
    xarray events,pending_events;size_t event_read,root_node;uint32_t root_flags;xge3d_root_motion_t root_motion;
};
static xge3d_quat_t d3_quat_normalize(xge3d_quat_t q)
{
    float length=sqrtf(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w);
    if (length<1e-12f) return (xge3d_quat_t){0,0,0,1};
    return (xge3d_quat_t){q.x/length,q.y/length,q.z/length,q.w/length};
}
static xge3d_quat_t d3_slerp(xge3d_quat_t a,xge3d_quat_t b,float t)
{
    float dot=a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
    if (dot<0) {b=(xge3d_quat_t){-b.x,-b.y,-b.z,-b.w};dot=-dot;}
    float x=1-t,y=t;
    if (dot<.9995f) {float angle=acosf(fminf(dot,1)),s=sinf(angle);x=sinf((1-t)*angle)/s;y=sinf(t*angle)/s;}
    return d3_quat_normalize((xge3d_quat_t){a.x*x+b.x*y,a.y*x+b.y*y,a.z*x+b.z*y,a.w*x+b.w*y});
}
void xge3dClipFree(xge3d_clip *clip)
{
    if (!clip || --clip->refs) return;
    for (size_t i=0;i<clip->tracks.Count;++i) {d3_track *t=xrtArrayGet(&clip->tracks,i);xrtArrayUnit(&t->times);xrtArrayUnit(&t->values);}
    xrtArrayUnit(&clip->tracks);xge3dModelFree(clip->model);xrtFree(clip);
}
int xge3dClipFromModel(xge3d_model *model,size_t animation,xge3d_clip **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;cgltf_data *data=d3_model_data(model);
    if (!data || animation>=data->animations_count) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_clip *clip=xrtMalloc(sizeof(*clip));if (!clip) return XGE_ERROR_OUT_OF_MEMORY;
    memset(clip,0,sizeof(*clip));clip->refs=1;
    if (!d3_model_retain(model)) {xrtFree(clip);return XGE_ERROR_INVALID_STATE;}
    clip->model=model;xrtArrayInit(&clip->tracks,sizeof(d3_track));
    cgltf_animation *source=&data->animations[animation];clip->name=source->name;
    int result=XGE_OK;
    for (size_t i=0;i<source->channels_count;++i) {
        const cgltf_animation_channel *channel=&source->channels[i];const cgltf_animation_sampler *s=channel->sampler;
        d3_track t={0};xrtArrayInit(&t.times,sizeof(float));xrtArrayInit(&t.values,sizeof(float));
        size_t components=channel->target_path==cgltf_animation_path_type_rotation ? 4 : 3;
        size_t multiplier=s && s->interpolation==cgltf_interpolation_type_cubic_spline ? 3 : 1;
        if (!channel->target_node || !s || !s->input || !s->output || channel->target_node->has_matrix) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}
        if (channel->target_path!=cgltf_animation_path_type_translation && channel->target_path!=cgltf_animation_path_type_rotation &&
            channel->target_path!=cgltf_animation_path_type_scale) {result=XGE_ERROR_UNSUPPORTED;goto failed_track;}
        if (!s->input->count || s->input->count>INT_MAX/(components*multiplier*sizeof(float)) ||
            s->input->type!=cgltf_type_scalar || s->input->component_type!=cgltf_component_type_r_32f ||
            s->output->component_type!=cgltf_component_type_r_32f || cgltf_num_components(s->output->type)!=components ||
            s->output->count!=s->input->count*multiplier) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}
        t.node=cgltf_node_index(data,channel->target_node);t.path=(int)channel->target_path;t.interpolation=(int)s->interpolation;
        if (s->interpolation!=cgltf_interpolation_type_step && s->interpolation!=cgltf_interpolation_type_linear &&
            s->interpolation!=cgltf_interpolation_type_cubic_spline) {result=XGE_ERROR_UNSUPPORTED;goto failed_track;}
        for (size_t j=0;j<clip->tracks.Count;++j) {const d3_track *previous=xrtArrayConstGet(&clip->tracks,j);
            if (previous->node==t.node && previous->path==t.path) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}}
        if (!xrtArrayResize(&t.times,s->input->count) || !xrtArrayResize(&t.values,s->output->count*components)) {result=XGE_ERROR_OUT_OF_MEMORY;goto failed_track;}
        if (cgltf_accessor_unpack_floats(s->input,(float*)t.times.Data,t.times.Count)!=t.times.Count ||
            cgltf_accessor_unpack_floats(s->output,(float*)t.values.Data,t.values.Count)!=t.values.Count) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}
        for (size_t k=0;k<t.times.Count;++k) {const float *times=(const float*)t.times.Data;
            if (!isfinite(times[k]) || times[k]<0 || (k && times[k]<=times[k-1])) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}}
        for (size_t k=0;k<t.values.Count;++k) if (!isfinite(((float*)t.values.Data)[k])) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}
        if (components==4) for (size_t k=0;k<t.times.Count;++k) {
            float *q=(float*)t.values.Data+(k*multiplier+(multiplier==3 ? 1 : 0))*4;
            float length=q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
            if (!isfinite(length) || length<1e-12f) {result=XGE_ERROR_RESOURCE_FAILED;goto failed_track;}
        }
        clip->duration=fmaxf(clip->duration,((float*)t.times.Data)[t.times.Count-1]);
        if (!xrtArrayPush(&clip->tracks,&t)) {result=XGE_ERROR_OUT_OF_MEMORY;goto failed_track;}
        continue;
failed_track:
        xrtArrayUnit(&t.times);xrtArrayUnit(&t.values);break;
    }
    if (result==XGE_OK) *out=clip;else xge3dClipFree(clip);return result;
}
int xge3dClipLoad(const char *uri,size_t animation,xge3d_clip **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;xge3d_model *model=NULL;int result=xge3dModelLoad(uri,&model);
    if (result==XGE_OK) result=xge3dClipFromModel(model,animation,out);
    xge3dModelFree(model);return result;
}
xge3d_clip_info_t xge3dClipInfo(const xge3d_clip *c)
{ return c ? (xge3d_clip_info_t){c->name,c->duration,c->tracks.Count,d3_model_data(c->model)->nodes_count} : (xge3d_clip_info_t){0}; }
const char *xge3dClipNodeName(const xge3d_clip *c,size_t node) {return c ? xge3dModelNodeName(c->model,node) : NULL;}

static void d3_layer_free(d3_layer *layer)
{xge3dClipFree(layer->clip);xrtArrayUnit(&layer->targets);xrtArrayUnit(&layer->mask);xrtArrayUnit(&layer->events);memset(layer,0,sizeof(*layer));}
void xge3dAnimatorFree(xge3d_animator *a)
{
    if (!a) return;
    for (int i=0;i<2;++i) d3_layer_free(&a->layers[i]);
    xrtArrayUnit(&a->nodes);xrtArrayUnit(&a->rest);xrtArrayUnit(&a->pose);xrtArrayUnit(&a->matrices);
    xrtArrayUnit(&a->events);xrtArrayUnit(&a->pending_events);xge3dModelFree(a->model);xrtFree(a);
}
int xge3dAnimatorCreate(xge3d_scene *scene,xge3d_node_t root,xge3d_animator **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;xge3d_node_data *n=d3_node(scene,root);if (!n || !n->model) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_animator *a=xrtMalloc(sizeof(*a));if (!a) return XGE_ERROR_OUT_OF_MEMORY;
    memset(a,0,sizeof(*a));a->scene=scene;a->root=root;
    a->root_node=SIZE_MAX;a->root_motion.rotation.w=1;
    if (!d3_model_retain(n->model)) {xrtFree(a);return XGE_ERROR_INVALID_STATE;}
    a->model=n->model;xrtArrayInit(&a->nodes,sizeof(xge3d_node_t));xrtArrayInit(&a->rest,sizeof(xge3d_transform_t));xrtArrayInit(&a->pose,sizeof(xge3d_transform_t));
    xrtArrayInit(&a->matrices,sizeof(xge3d_mat4_t));
    xrtArrayInit(&a->events,sizeof(xge3d_animation_event_t));xrtArrayInit(&a->pending_events,sizeof(xge3d_animation_event_t));
    cgltf_data *data=d3_model_data(a->model);
    if (!xrtArrayResize(&a->nodes,data->nodes_count) || !xrtArrayResize(&a->rest,data->nodes_count) ||
        !xrtArrayResize(&a->pose,data->nodes_count) || !xrtArrayResize(&a->matrices,data->nodes_count)) {xge3dAnimatorFree(a);return XGE_ERROR_OUT_OF_MEMORY;}
    for (size_t i=0;i<data->nodes_count;++i) {
        xge3d_node_t node={0};xge3dModelInstanceNode(scene,root,i,&node);*(xge3d_node_t*)xrtArrayGet(&a->nodes,i)=node;
        const cgltf_node *source=&data->nodes[i];xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;
        memcpy(&t.position,source->translation,sizeof(t.position));memcpy(&t.rotation,source->rotation,sizeof(t.rotation));memcpy(&t.scale,source->scale,sizeof(t.scale));
        *(xge3d_transform_t*)xrtArrayGet(&a->rest,i)=t;
    }
    *out=a;return XGE_OK;
}
xge3d_animation_layer_t xge3dAnimationLayerDefault(void) {return (xge3d_animation_layer_t){.speed=1,.weight=1,.loop=1};}
static size_t d3_target_name(const cgltf_data *data,const char *name)
{
    size_t found=SIZE_MAX;if (!name) return found;
    for (size_t i=0;i<data->nodes_count;++i) if (data->nodes[i].name && !strcmp(data->nodes[i].name,name)) {
        if (found!=SIZE_MAX) return SIZE_MAX;
        found=i;
    }
    return found;
}
int xge3dAnimatorSetLayer(xge3d_animator *a,int index,xge3d_clip *clip,const xge3d_animation_layer_t *d)
{
    if (!a || index<0 || index>1) return XGE_ERROR_INVALID_ARGUMENT;
    if (!clip) {d3_layer_free(&a->layers[index]);return XGE_OK;}
    xge3d_animation_layer_t fallback=xge3dAnimationLayerDefault();if (!d) d=&fallback;
    cgltf_data *source=d3_model_data(clip->model),*target=d3_model_data(a->model);
    if (!isfinite(d->time) || d->time<0 || !isfinite(d->speed) || d->speed<0 || !isfinite(d->weight) || d->weight<0 || d->weight>1 ||
        (d->mask && d->mask_count!=a->nodes.Count) || (d->binding_count && !d->bindings) || clip->refs==UINT32_MAX) return XGE_ERROR_INVALID_ARGUMENT;
    for (size_t i=0;i<d->binding_count;++i) {
        if (d->bindings[i].source>=source->nodes_count || d->bindings[i].target>=target->nodes_count) return XGE_ERROR_INVALID_ARGUMENT;
        for (size_t j=0;j<i;++j) if (d->bindings[i].source==d->bindings[j].source || d->bindings[i].target==d->bindings[j].target) return XGE_ERROR_INVALID_ARGUMENT;
    }
    d3_layer layer={0};xrtArrayInit(&layer.targets,sizeof(size_t));xrtArrayInit(&layer.mask,sizeof(float));int result=XGE_OK;
    if (!xrtArrayResize(&layer.targets,clip->tracks.Count) || !xrtArrayResize(&layer.mask,a->nodes.Count)) {result=XGE_ERROR_OUT_OF_MEMORY;goto done;}
    for (size_t i=0;i<a->nodes.Count;++i) {
        float weight=d->mask ? d->mask[i] : 1;
        if (!isfinite(weight) || weight<0 || weight>1) {result=XGE_ERROR_INVALID_ARGUMENT;goto done;}
        ((float*)layer.mask.Data)[i]=weight;
    }
    for (size_t i=0;i<clip->tracks.Count;++i) {
        const d3_track *t=xrtArrayConstGet(&clip->tracks,i);size_t mapped=SIZE_MAX;
        if (d->binding_count) {for (size_t j=0;j<d->binding_count;++j) if (d->bindings[j].source==t->node) mapped=d->bindings[j].target;}
        else if (clip->model==a->model) mapped=t->node;
        else if (d3_target_name(source,source->nodes[t->node].name)!=SIZE_MAX) mapped=d3_target_name(target,source->nodes[t->node].name);
        if (mapped==SIZE_MAX || !((xge3d_node_t*)a->nodes.Data)[mapped].slot) {result=XGE_ERROR_NOT_FOUND;goto done;}
        if (target->nodes[mapped].has_matrix) {result=XGE_ERROR_UNSUPPORTED;goto done;}
        ((size_t*)layer.targets.Data)[i]=mapped;
    }
    layer.clip=clip;++clip->refs;layer.time=d->time;layer.speed=d->speed;layer.weight=d->weight;layer.loop=!!d->loop;
    layer.time=layer.loop && clip->duration>0 ? fmod(layer.time,clip->duration) : fmin(layer.time,clip->duration);layer.started=1;
    xrtArrayInit(&layer.events,sizeof(xge3d_animation_event_t));
    d3_layer_free(&a->layers[index]);a->layers[index]=layer;return XGE_OK;
done:
    d3_layer_free(&layer);return result;
}
int xge3dAnimatorSetLayerWeight(xge3d_animator *a,int layer,float weight)
{
    if (!a || layer<0 || layer>1 || !isfinite(weight) || weight<0 || weight>1) return XGE_ERROR_INVALID_ARGUMENT;
    a->layers[layer].weight=weight;return XGE_OK;
}
float xge3dAnimatorLayerTime(const xge3d_animator *a,int layer)
{return a && layer>=0 && layer<2 ? (float)a->layers[layer].time : 0;}

int xge3dAnimatorSetEvents(xge3d_animator *a,int layer,const xge3d_animation_event_t *events,size_t count)
{
    if (!a || layer<0 || layer>1 || !a->layers[layer].clip || count>1024 || (count && !events)) return XGE_ERROR_INVALID_ARGUMENT;
    for (size_t i=0;i<count;++i) if (!isfinite(events[i].time) || events[i].time<0 || events[i].time>a->layers[layer].clip->duration ||
        (i && events[i].time<events[i-1].time)) return XGE_ERROR_INVALID_ARGUMENT;
    xarray copy;xrtArrayInit(&copy,sizeof(*events));
    if (!xrtArrayResize(&copy,count)) {xrtArrayUnit(&copy);return XGE_ERROR_OUT_OF_MEMORY;}
    if (count) memcpy(copy.Data,events,count*sizeof(*events));
    xrtArrayUnit(&a->layers[layer].events);a->layers[layer].events=copy;return XGE_OK;
}
int xge3dAnimatorNextEvent(xge3d_animator *a,xge3d_animation_event_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_animation_event_t){0};if (!a) return XGE_ERROR_INVALID_ARGUMENT;
    if (a->event_read==a->pending_events.Count) return XGE_ERROR_NOT_FOUND;
    *out=((xge3d_animation_event_t*)a->pending_events.Data)[a->event_read++];
    if (a->event_read==a->pending_events.Count) {a->event_read=0;xrtArrayResize(&a->pending_events,0);}
    return XGE_OK;
}
int xge3dAnimatorSetRootMotion(xge3d_animator *a,size_t node,uint32_t flags)
{
    if (!a || (flags&~15u)) return XGE_ERROR_INVALID_ARGUMENT;
    if (flags && (node>=a->nodes.Count || !d3_node(a->scene,((xge3d_node_t*)a->nodes.Data)[node]))) return XGE_ERROR_NOT_FOUND;
    if (flags && d3_model_data(a->model)->nodes[node].has_matrix) return XGE_ERROR_UNSUPPORTED;
    a->root_node=flags ? node : SIZE_MAX;a->root_flags=flags;
    a->root_motion=(xge3d_root_motion_t){.rotation={0,0,0,1}};return XGE_OK;
}
int xge3dAnimatorTakeRootMotion(xge3d_animator *a,xge3d_root_motion_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_root_motion_t){.rotation={0,0,0,1}};if (!a) return XGE_ERROR_INVALID_ARGUMENT;
    *out=a->root_motion;a->root_motion=(xge3d_root_motion_t){.rotation={0,0,0,1}};return XGE_OK;
}

static void d3_sample(const d3_track *track,float time,float out[4])
{
    const float *times=(const float*)track->times.Data,*v=(const float*)track->values.Data;size_t lo=0,hi=track->times.Count-1;
    while (lo<hi) {size_t middle=(lo+hi+1)/2;if (times[middle]<=time) lo=middle;else hi=middle-1;}
    size_t next=lo+1<track->times.Count ? lo+1 : lo;
    size_t n=track->path==cgltf_animation_path_type_rotation ? 4 : 3;
    int cubic=track->interpolation==cgltf_interpolation_type_cubic_spline;
    size_t stride=n*(cubic ? 3 : 1);const float *a=v+lo*stride+(cubic ? n : 0),*b=v+next*stride+(cubic ? n : 0);
    float span=times[next]-times[lo],t=span>0 ? fminf(fmaxf((time-times[lo])/span,0),1) : 0;
    if (track->interpolation==cgltf_interpolation_type_step) t=0;
    if (n==4 && !cubic) {
        xge3d_quat_t qa,qb,q;memcpy(&qa,a,sizeof(qa));memcpy(&qb,b,sizeof(qb));q=d3_slerp(d3_quat_normalize(qa),d3_quat_normalize(qb),t);memcpy(out,&q,sizeof(q));return;
    }
    for (size_t i=0;i<n;++i) {
        if (cubic && span>0) {float t2=t*t,t3=t2*t;out[i]=(2*t3-3*t2+1)*a[i]+(t3-2*t2+t)*span*a[n+i]+(-2*t3+3*t2)*b[i]+(t3-t2)*span*b[(ptrdiff_t)i-(ptrdiff_t)n];}
        else out[i]=a[i]*(1-t)+b[i]*t;
    }
    if (n==4) {xge3d_quat_t q;memcpy(&q,out,sizeof(q));q=d3_quat_normalize(q);memcpy(out,&q,sizeof(q));}
}
static int d3_event_interval(xge3d_animator *a,int layer,double begin,double end,int include_begin)
{
    const d3_layer *l=&a->layers[layer];
    for (size_t i=0;i<l->events.Count;++i) {
        xge3d_animation_event_t event=((xge3d_animation_event_t*)l->events.Data)[i];
        if ((event.time>begin || (include_begin && event.time==begin)) && event.time<=end) {
            if (a->events.Count+a->pending_events.Count-a->event_read>=4096) return XGE_ERROR_UNSUPPORTED;
            event.layer=layer;if (!xrtArrayPush(&a->events,&event)) return XGE_ERROR_OUT_OF_MEMORY;
        }
    }
    return XGE_OK;
}
static int d3_events(xge3d_animator *a,int layer,double next,double cycles)
{
    d3_layer *l=&a->layers[layer];if (!l->events.Count) return XGE_OK;
    if (!cycles) return d3_event_interval(a,layer,l->time,next,l->started);
    if ((cycles-1)*l->events.Count>4096) return XGE_ERROR_UNSUPPORTED;
    int result=d3_event_interval(a,layer,l->time,l->clip->duration,l->started);
    for (size_t i=1;result==XGE_OK && i<(size_t)cycles;++i) result=d3_event_interval(a,layer,0,l->clip->duration,1);
    if (result==XGE_OK) result=d3_event_interval(a,layer,0,next,1);
    return result;
}
static xge3d_quat_t d3_quat_mul(xge3d_quat_t a,xge3d_quat_t b)
{
    return d3_quat_normalize((xge3d_quat_t){a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
        a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,
        a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z});
}
static xge3d_quat_t d3_quat_delta(const float from[4],const float to[4])
{
    xge3d_quat_t a,b;memcpy(&a,from,sizeof(a));memcpy(&b,to,sizeof(b));
    a=(xge3d_quat_t){-a.x,-a.y,-a.z,a.w};return d3_quat_mul(b,a);
}
static xge3d_quat_t d3_quat_power(xge3d_quat_t q,uint64_t count)
{
    xge3d_quat_t out={0,0,0,1};
    while (count) {if (count&1) out=d3_quat_mul(out,q);q=d3_quat_mul(q,q);count>>=1;}
    return out;
}
static const d3_track *d3_root_track(const d3_layer *l,size_t node,int path)
{
    if (l->clip) for (size_t i=0;i<l->clip->tracks.Count;++i) {
        const d3_track *t=xrtArrayConstGet(&l->clip->tracks,i);
        if (((size_t*)l->targets.Data)[i]==node && t->path==path) return t;
    }
    return NULL;
}
static int d3_root_delta(xge3d_animator *a,const double next[2],const double cycles[2],xge3d_root_motion_t *out)
{
    *out=a->root_motion;if (!a->root_flags) return XGE_OK;
    for (int path=cgltf_animation_path_type_translation;path<=cgltf_animation_path_type_rotation;++path) {
        const d3_track *tracks[2];float weights[2]={0};
        for (int i=0;i<2;++i) {
            tracks[i]=d3_root_track(&a->layers[i],a->root_node,path);
            if (tracks[i]) weights[i]=a->layers[i].weight*((float*)a->layers[i].mask.Data)[a->root_node];
        }
        weights[0]*=1-weights[1];
        for (int i=0;i<2;++i) if (tracks[i] && weights[i]>0) {
            float old[4]={0},value[4]={0},first[4]={0},last[4]={0};const d3_layer *l=&a->layers[i];
            d3_sample(tracks[i],(float)l->time,old);d3_sample(tracks[i],(float)next[i],value);
            d3_sample(tracks[i],0,first);d3_sample(tracks[i],l->clip->duration,last);
            if (path==cgltf_animation_path_type_translation) {
                float *position=&out->translation.x;
                for (int k=0;k<3;++k) if (a->root_flags&(1u<<k))
                    position[k]=(float)(position[k]+weights[i]*((double)value[k]-old[k]+cycles[i]*((double)last[k]-first[k])));
            } else if (a->root_flags&XGE3D_ROOT_ROTATION) {
                if (cycles[i]>1e18) return XGE_ERROR_UNSUPPORTED;
                xge3d_quat_t rotation;
                if (!cycles[i]) rotation=d3_quat_delta(old,value);
                else rotation=d3_quat_mul(d3_quat_delta(first,value),d3_quat_mul(
                    d3_quat_power(d3_quat_delta(first,last),(uint64_t)cycles[i]-1),d3_quat_delta(old,last)));
                rotation=d3_slerp((xge3d_quat_t){0,0,0,1},rotation,weights[i]);
                out->rotation=d3_quat_mul(rotation,out->rotation);
            }
        }
    }
    if (!d3_finite3(out->translation)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_transform_t *pose=xrtArrayGet(&a->pose,a->root_node),*rest=xrtArrayGet(&a->rest,a->root_node);
    for (int k=0;k<3;++k) if (a->root_flags&(1u<<k)) (&pose->position.x)[k]=(&rest->position.x)[k];
    if (a->root_flags&XGE3D_ROOT_ROTATION) pose->rotation=rest->rotation;
    return XGE_OK;
}
int xge3dAnimatorUpdate(xge3d_animator *a,float delta)
{
    if (!a || !isfinite(delta) || delta<0) return XGE_ERROR_INVALID_ARGUMENT;
    if (!d3_node(a->scene,a->root)) return XGE_ERROR_INVALID_STATE;
    for (size_t i=0;i<a->nodes.Count;++i) {xge3d_node_t n=((xge3d_node_t*)a->nodes.Data)[i];if (n.slot && !d3_node(a->scene,n)) return XGE_ERROR_INVALID_STATE;}
    memcpy(a->pose.Data,a->rest.Data,a->rest.Count*a->rest.ItemSize);
    double next_time[2]={0},cycles[2]={0};xrtArrayResize(&a->events,0);
    for (int layer=0;layer<2;++layer) {
        d3_layer *l=&a->layers[layer];if (!l->clip) continue;
        double time=l->time+(double)delta*l->speed;float duration=l->clip->duration;
        if (l->loop && duration>0) {cycles[layer]=floor(time/duration);time=fmod(time,duration);}else time=fmin(time,duration);
        next_time[layer]=time;
        int result=d3_events(a,layer,time,cycles[layer]);if (result!=XGE_OK) return result;
        for (size_t i=0;i<l->clip->tracks.Count;++i) {
            const d3_track *track=xrtArrayConstGet(&l->clip->tracks,i);size_t node=((size_t*)l->targets.Data)[i];
            xge3d_transform_t *pose=xrtArrayGet(&a->pose,node);float weight=l->weight*((float*)l->mask.Data)[node],value[4]={0};
            if (weight<=0) continue;
            d3_sample(track,(float)time,value);
            if (track->path==cgltf_animation_path_type_rotation) {xge3d_quat_t q;memcpy(&q,value,sizeof(q));pose->rotation=d3_slerp(pose->rotation,q,weight);}
            else {xge3d_vec3_t v={value[0],value[1],value[2]};xge3d_vec3_t *dest=track->path==cgltf_animation_path_type_translation ? &pose->position : &pose->scale;*dest=d3_add(d3_scale(*dest,1-weight),d3_scale(v,weight));}
        }
    }
    xge3d_root_motion_t motion;int result=d3_root_delta(a,next_time,cycles,&motion);if (result!=XGE_OK) return result;
    cgltf_data *data=d3_model_data(a->model);
    for (size_t i=0;i<a->nodes.Count;++i) {
        xge3d_node_t handle=((xge3d_node_t*)a->nodes.Data)[i];if (!handle.slot || data->nodes[i].has_matrix) continue;
        int result=xge3dTransformMatrix(xrtArrayGet(&a->pose,i),xrtArrayGet(&a->matrices,i));if (result!=XGE_OK) return result;
    }
    size_t pending=a->pending_events.Count-a->event_read;
    if (!xrtArrayReserve(&a->pending_events,pending+a->events.Count)) return XGE_ERROR_OUT_OF_MEMORY;
    if (pending && a->event_read) memmove(a->pending_events.Data,(xge3d_animation_event_t*)a->pending_events.Data+a->event_read,pending*sizeof(xge3d_animation_event_t));
    xrtArrayResize(&a->pending_events,pending+a->events.Count);a->event_read=0;
    if (a->events.Count) memcpy((xge3d_animation_event_t*)a->pending_events.Data+pending,a->events.Data,a->events.Count*sizeof(xge3d_animation_event_t));
    a->root_motion=motion;
    for (int i=0;i<2;++i) if (a->layers[i].clip) {a->layers[i].time=next_time[i];a->layers[i].started=0;}
    for (size_t i=0;i<a->nodes.Count;++i) {
        xge3d_node_t handle=((xge3d_node_t*)a->nodes.Data)[i];if (!handle.slot || data->nodes[i].has_matrix) continue;
        xge3d_mat4_t matrix=*(xge3d_mat4_t*)xrtArrayGet(&a->matrices,i);
        xge3d_node_data *node=d3_node(a->scene,handle);
        if (memcmp(&node->local,&matrix,sizeof(matrix))) d3_set_local(a->scene,node,matrix);
    }
    return XGE_OK;
}
int xge3dAnimatorBoneNode(xge3d_animator *a,const char *name,xge3d_node_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_node_t){0};if (!a || !name) return XGE_ERROR_INVALID_ARGUMENT;
    size_t index=d3_target_name(d3_model_data(a->model),name);
    if (index==SIZE_MAX) return XGE_ERROR_NOT_FOUND;
    xge3d_node_t node=((xge3d_node_t*)a->nodes.Data)[index];if (!d3_node(a->scene,node)) return XGE_ERROR_NOT_FOUND;
    *out=node;return XGE_OK;
}
#endif
