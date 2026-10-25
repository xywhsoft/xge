#include "xge3d_internal.h"
#include "extlibs/cgltf/cgltf.h"
#include <limits.h>

#if XGE3D_ENABLE_ANIMATION
typedef struct d3_track { size_t node;int path,interpolation; xarray times,values; } d3_track;
struct xge3d_clip { uint32_t refs; xge3d_model *model; const char *name; float duration; xarray tracks; };
typedef struct d3_layer {
    xge3d_clip *clip;double time;float speed,weight;int loop,started;xarray targets,mask,events;
    float fade_from,fade_to;double fade_time,fade_duration;int paused,stopping;
} d3_layer;
typedef struct d3_channel_pose {xge3d_transform_t value;float weights[3];} d3_channel_pose;
typedef struct d3_transition {d3_layer previous;xarray snapshot;double time,duration;} d3_transition;
struct xge3d_animator {
    xge3d_scene *scene; xge3d_node_t root; xge3d_model *model;
    xarray nodes,rest,pose,published,matrices; d3_layer layers[XGE3D_ANIMATION_MAX_LAYERS];int modes[XGE3D_ANIMATION_MAX_LAYERS];
    xarray events,pending_events;size_t event_read,root_node;uint32_t root_flags;xge3d_root_motion_t root_motion;
    d3_transition transitions[XGE3D_ANIMATION_MAX_LAYERS];xarray blend;int paused;
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
int xge3dClipLoadBudget(const char *uri,size_t animation,xge3d_decode_budget *budget,xge3d_clip **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    xallocationscope scope={0};if(!xrtAllocationBudgetEnter(budget,&scope))return XGE_ERROR_INVALID_ARGUMENT;
    int result=xge3dClipLoad(uri,animation,out);int rejected=scope.Rejections!=0;
    xrtAllocationBudgetLeave(&scope);return result&&rejected?XGE_ERROR_OUT_OF_MEMORY:result;
}
xge3d_clip_info_t xge3dClipInfo(const xge3d_clip *c)
{ return c ? (xge3d_clip_info_t){c->name,c->duration,c->tracks.Count,d3_model_data(c->model)->nodes_count} : (xge3d_clip_info_t){0}; }
int xge3dClipMemoryInfo(const xge3d_clip *c,xge3d_asset_memory_t *out)
{
    if(!c||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_asset_memory_t info={.size=sizeof(info)};int result=xge3dModelMemoryInfo(c->model,&info);if(result)return result;
    info.cpu_storage_bytes+=sizeof(*c)+(uint64_t)c->tracks.Capacity*c->tracks.ItemSize;
    for(size_t i=0;i<c->tracks.Count;++i){const d3_track *t=xrtArrayConstGet(&c->tracks,i);
        info.cpu_storage_bytes+=(uint64_t)t->times.Capacity*t->times.ItemSize+(uint64_t)t->values.Capacity*t->values.ItemSize;}
    *out=info;return XGE_OK;
}
const char *xge3dClipNodeName(const xge3d_clip *c,size_t node) {return c ? xge3dModelNodeName(c->model,node) : NULL;}

static void d3_layer_free(d3_layer *layer)
{xge3dClipFree(layer->clip);xrtArrayUnit(&layer->targets);xrtArrayUnit(&layer->mask);xrtArrayUnit(&layer->events);memset(layer,0,sizeof(*layer));}
static void d3_transition_free(d3_transition *t)
{d3_layer_free(&t->previous);xrtArrayUnit(&t->snapshot);memset(t,0,sizeof(*t));}
void xge3dAnimatorFree(xge3d_animator *a)
{
    if (!a) return;
    for (int i=0;i<XGE3D_ANIMATION_MAX_LAYERS;++i){d3_layer_free(&a->layers[i]);d3_transition_free(&a->transitions[i]);}
    xrtArrayUnit(&a->blend);
    xrtArrayUnit(&a->nodes);xrtArrayUnit(&a->rest);xrtArrayUnit(&a->pose);xrtArrayUnit(&a->published);xrtArrayUnit(&a->matrices);
    xrtArrayUnit(&a->events);xrtArrayUnit(&a->pending_events);xge3dModelFree(a->model);xrtFree(a);
}
int xge3dAnimatorCreate(xge3d_scene *scene,xge3d_node_t root,xge3d_animator **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;xge3d_node_data *n=d3_node(scene,root);if (!n || !n->model) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_animator *a=xrtMalloc(sizeof(*a));if (!a) return XGE_ERROR_OUT_OF_MEMORY;
    memset(a,0,sizeof(*a));a->scene=scene;a->root=root;
    a->root_node=SIZE_MAX;a->root_motion.rotation.w=1;
    xrtArrayInit(&a->blend,sizeof(d3_channel_pose));
    if (!d3_model_retain(n->model)) {xrtFree(a);return XGE_ERROR_INVALID_STATE;}
    a->model=n->model;xrtArrayInit(&a->nodes,sizeof(xge3d_node_t));xrtArrayInit(&a->rest,sizeof(xge3d_transform_t));xrtArrayInit(&a->pose,sizeof(xge3d_transform_t));xrtArrayInit(&a->published,sizeof(xge3d_transform_t));
    xrtArrayInit(&a->matrices,sizeof(xge3d_mat4_t));
    xrtArrayInit(&a->events,sizeof(xge3d_animation_event_t));xrtArrayInit(&a->pending_events,sizeof(xge3d_animation_event_t));
    cgltf_data *data=d3_model_data(a->model);
    if (!xrtArrayResize(&a->published,data->nodes_count) || !xrtArrayResize(&a->nodes,data->nodes_count) || !xrtArrayResize(&a->rest,data->nodes_count) ||
        !xrtArrayResize(&a->pose,data->nodes_count) || !xrtArrayResize(&a->matrices,data->nodes_count)) {xge3dAnimatorFree(a);return XGE_ERROR_OUT_OF_MEMORY;}
    for (size_t i=0;i<data->nodes_count;++i) {
        xge3d_node_t node={0};xge3dModelInstanceNode(scene,root,i,&node);*(xge3d_node_t*)xrtArrayGet(&a->nodes,i)=node;
        const cgltf_node *source=&data->nodes[i];xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;
        memcpy(&t.position,source->translation,sizeof(t.position));memcpy(&t.rotation,source->rotation,sizeof(t.rotation));memcpy(&t.scale,source->scale,sizeof(t.scale));
        *(xge3d_transform_t*)xrtArrayGet(&a->rest,i)=t;
        *(xge3d_transform_t*)xrtArrayGet(&a->pose,i)=t;
        *(xge3d_transform_t*)xrtArrayGet(&a->published,i)=t;
        cgltf_node_transform_local(source,((xge3d_mat4_t*)xrtArrayGet(&a->matrices,i))->m);
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
static int d3_layer_bind(xge3d_animator *a,xge3d_clip *clip,const xge3d_animation_layer_t *d,d3_layer *out)
{
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
    *out=layer;return XGE_OK;
done:
    d3_layer_free(&layer);return result;
}
int xge3dAnimatorSetLayer(xge3d_animator *a,int index,xge3d_clip *clip,const xge3d_animation_layer_t *d)
{
    if(!a||index<0||index>=XGE3D_ANIMATION_MAX_LAYERS)return XGE_ERROR_INVALID_ARGUMENT;
    d3_layer layer={0};int result=clip?d3_layer_bind(a,clip,d,&layer):XGE_OK;if(result)return result;
    d3_layer_free(&a->layers[index]);d3_transition_free(&a->transitions[index]);a->layers[index]=layer;return XGE_OK;
}
int xge3dAnimatorSetLayerWeight(xge3d_animator *a,int layer,float weight)
{
    if (!a || layer<0 || layer>=XGE3D_ANIMATION_MAX_LAYERS || !isfinite(weight) || weight<0 || weight>1) return XGE_ERROR_INVALID_ARGUMENT;
    a->layers[layer].weight=weight;a->layers[layer].fade_duration=0;a->layers[layer].stopping=0;return XGE_OK;
}
float xge3dAnimatorLayerTime(const xge3d_animator *a,int layer)
{return a && layer>=0 && layer<XGE3D_ANIMATION_MAX_LAYERS ? (float)a->layers[layer].time : 0;}

int xge3dAnimatorSetEvents(xge3d_animator *a,int layer,const xge3d_animation_event_t *events,size_t count)
{
    if (!a || layer<0 || layer>=XGE3D_ANIMATION_MAX_LAYERS || !a->layers[layer].clip || count>1024 || (count && !events)) return XGE_ERROR_INVALID_ARGUMENT;
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
#include "xge3d_playback.inc"
static int d3_root_delta(xge3d_animator *a,const double next[2][XGE3D_ANIMATION_MAX_LAYERS],const double cycles[2][XGE3D_ANIMATION_MAX_LAYERS],
    const float means[2][XGE3D_ANIMATION_MAX_LAYERS],xge3d_root_motion_t *out)
{
    *out=a->root_motion;if (!a->root_flags) return XGE_OK;
    for (int path=cgltf_animation_path_type_translation;path<=cgltf_animation_path_type_rotation;++path) {
        const d3_track *tracks[2][XGE3D_ANIMATION_MAX_LAYERS];float weights[2][XGE3D_ANIMATION_MAX_LAYERS]={{0}};
        for (int i=0;i<XGE3D_ANIMATION_MAX_LAYERS;++i) {
            const d3_layer *sources[]={&a->layers[i],&a->transitions[i].previous};
            for(int source=0;source<2;++source){tracks[source][i]=d3_root_track(sources[source],a->root_node,path);
                if(tracks[source][i])weights[source][i]=means[source][i]*((float*)sources[source]->mask.Data)[a->root_node];
                else if(source&&a->transitions[i].snapshot.Count)weights[source][i]=means[source][i]*((d3_channel_pose*)a->transitions[i].snapshot.Data)[a->root_node].weights[path-1];}
        }
        float remaining=1;
        for(int i=XGE3D_ANIMATION_MAX_LAYERS-1;i>=0;--i){float weight=fminf(1,weights[0][i]+weights[1][i]);weights[0][i]*=remaining;weights[1][i]*=remaining;
            if(a->modes[i]==XGE3D_ANIMATION_OVERRIDE)remaining*=1-weight;}
        for(int i=0;i<XGE3D_ANIMATION_MAX_LAYERS;++i)for(int source=1;source>=0;--source)if(tracks[source][i]&&weights[source][i]>0){
            float old[4]={0},value[4]={0},first[4]={0},last[4]={0};const d3_layer *l=source?&a->transitions[i].previous:&a->layers[i];
            const d3_track *track=tracks[source][i];float weight=weights[source][i];double loops=cycles[source][i];
            d3_sample(track,(float)l->time,old);d3_sample(track,(float)next[source][i],value);
            d3_sample(track,0,first);d3_sample(track,l->clip->duration,last);
            if (path==cgltf_animation_path_type_translation) {
                float *position=&out->translation.x;
                for (int k=0;k<3;++k) if (a->root_flags&(1u<<k))
                    position[k]=(float)(position[k]+weight*((double)value[k]-old[k]+loops*((double)last[k]-first[k])));
            } else if (a->root_flags&XGE3D_ROOT_ROTATION) {
                if (loops>1e18) return XGE_ERROR_UNSUPPORTED;
                xge3d_quat_t rotation;
                if (!loops) rotation=d3_quat_delta(old,value);
                else rotation=d3_quat_mul(d3_quat_delta(first,value),d3_quat_mul(
                    d3_quat_power(d3_quat_delta(first,last),(uint64_t)loops-1),d3_quat_delta(old,last)));
                rotation=d3_slerp((xge3d_quat_t){0,0,0,1},rotation,weight);
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
    if(a->paused)return XGE_OK;
    memcpy(a->pose.Data,a->rest.Data,a->rest.Count*a->rest.ItemSize);
    double next_time[2][XGE3D_ANIMATION_MAX_LAYERS]={{0}},cycles[2][XGE3D_ANIMATION_MAX_LAYERS]={{0}};
    float means[2][XGE3D_ANIMATION_MAX_LAYERS]={{0}},weights[XGE3D_ANIMATION_MAX_LAYERS]={0};xrtArrayResize(&a->events,0);
    for (int layer=0;layer<XGE3D_ANIMATION_MAX_LAYERS;++layer) {
        d3_layer *l=&a->layers[layer];if (!l->clip) continue;
        double step=l->paused?0:delta;const d3_transition *t=&a->transitions[layer];
        d3_playback_advance(l,step,&next_time[0][layer],&cycles[0][layer]);
        d3_playback_advance(&t->previous,step,&next_time[1][layer],&cycles[1][layer]);
        float average[2];d3_playback_mean(l,t,step,average);means[0][layer]=average[0];means[1][layer]=average[1];weights[layer]=d3_layer_gain(l,step);
        int result=(!l->paused&&!l->stopping)?d3_events(a,layer,next_time[0][layer],cycles[0][layer]):XGE_OK;if(result)return result;
        result=d3_playback_pose(a,layer,next_time[0][layer],next_time[1][layer],weights[layer],d3_transition_alpha(t,step));if(result)return result;
    }
    xge3d_root_motion_t motion;int result=d3_root_delta(a,next_time,cycles,means,&motion);if (result!=XGE_OK) return result;
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
    memcpy(a->published.Data,a->pose.Data,a->pose.Count*a->pose.ItemSize);
    for(int i=0;i<XGE3D_ANIMATION_MAX_LAYERS;++i){d3_layer *l=&a->layers[i];d3_transition *t=&a->transitions[i];if(!l->clip||l->paused)continue;
        l->time=next_time[0][i];l->started=0;l->weight=weights[i];
        if(l->fade_duration>0){l->fade_time=fmin(l->fade_duration,l->fade_time+delta);if(l->fade_time>=l->fade_duration)l->fade_duration=0;}
        if(t->duration>0){t->time=fmin(t->duration,t->time+delta);t->previous.time=next_time[1][i];if(t->time>=t->duration)d3_transition_free(t);}
        if(l->stopping&&!l->fade_duration){d3_layer_free(l);d3_transition_free(t);}}
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
int xge3dAnimatorSetLayerMode(xge3d_animator *a,int layer,int mode)
{
    if(!a||layer<0||layer>=XGE3D_ANIMATION_MAX_LAYERS||(mode!=XGE3D_ANIMATION_OVERRIDE&&mode!=XGE3D_ANIMATION_ADDITIVE))return XGE_ERROR_INVALID_ARGUMENT;
    a->modes[layer]=mode;return XGE_OK;
}
int xge3dAnimatorGetPose(xge3d_animator *a,size_t node,xge3d_transform_t *out)
{
    if(!a||!out||node>=a->nodes.Count)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_node(a->scene,*(xge3d_node_t*)xrtArrayGet(&a->nodes,node)))return XGE_ERROR_NOT_FOUND;
    *out=*(xge3d_transform_t*)xrtArrayGet(&a->published,node);return XGE_OK;
}
int xge3dAnimatorSetPose(xge3d_animator *a,size_t node,const xge3d_transform_t *pose)
{
    if(!a||!pose||node>=a->nodes.Count)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_node_data *n=d3_node(a->scene,*(xge3d_node_t*)xrtArrayGet(&a->nodes,node));
    if(!n){return XGE_ERROR_NOT_FOUND;}if(d3_model_data(a->model)->nodes[node].has_matrix)return XGE_ERROR_UNSUPPORTED;
    xge3d_mat4_t matrix;int result=xge3dTransformMatrix(pose,&matrix);if(result)return result;
    *(xge3d_transform_t*)xrtArrayGet(&a->pose,node)=*pose;*(xge3d_transform_t*)xrtArrayGet(&a->published,node)=*pose;*(xge3d_mat4_t*)xrtArrayGet(&a->matrices,node)=matrix;
    d3_set_local(a->scene,n,matrix);return XGE_OK;
}
#endif
