#include "xge3d_motion_internal.h"
#include <ctype.h>
#define CGLTF_IMPLEMENTATION
#include "../lib/cgltf/cgltf.h"

static int samples(motion_asset *a,const motion_options *o,double duration,double *begin)
{
    double end=o->end==-1 ? duration : o->end;*begin=o->start;
    if (!isfinite(duration) || duration<0 || end>duration+1e-6 || end<*begin || end-*begin>3600) return 0;
    end=fmin(end,duration);double span=end-*begin;size_t count=(size_t)ceil(fmax(span*o->fps-1e-8,0))+1;
    /* A glTF float endpoint may round onto the preceding regular sample. */
    while (count>1 && (float)((count-2)/o->fps)>=(float)span) --count;
    if (!a->nodes.Count || a->nodes.Count>MOTION_MAX_NODES || count>MOTION_MAX_BYTES/sizeof(ufbx_transform)/a->nodes.Count) return 0;
    if (!xrtArrayResize(&a->times,count) || !xrtArrayResize(&a->poses,count*a->nodes.Count)) return 0;
    for (size_t i=0;i<count;++i) ((float*)a->times.Data)[i]=(float)(i+1==count ? span : i/o->fps);
    return 1;
}
static int transform_valid(ufbx_transform *t)
{
    for (int i=0;i<3;++i) if (!isfinite(t->translation.v[i]) || !isfinite(t->scale.v[i])) return 0;
    double length=0;for (int i=0;i<4;++i) {if (!isfinite(t->rotation.v[i])) return 0;length+=t->rotation.v[i]*t->rotation.v[i];}
    if (length<1e-20 || !isfinite(length)) return 0;
    t->rotation=ufbx_quat_normalize(t->rotation);return 1;
}
static int clip_index(const char *choice,const char *name,size_t index)
{
    if (!choice) return index==0;
    char *end;unsigned long number=strtoul(choice,&end,10);
    return *choice && !*end ? number==index : !strcmp(choice,name);
}
static int fbx_load(const char *path,const motion_options *o,motion_asset *a)
{
    ufbx_load_opts opts={0};opts.ignore_geometry=1;opts.ignore_embedded=1;
    opts.target_axes=ufbx_axes_right_handed_y_up;opts.target_unit_meters=1;
    opts.space_conversion=UFBX_SPACE_CONVERSION_ADJUST_TRANSFORMS;
    opts.temp_allocator.memory_limit=512u*1024u*1024u;opts.result_allocator.memory_limit=512u*1024u*1024u;
    ufbx_error error;ufbx_scene *scene=ufbx_load_file(path,&opts,&error);
    if (!scene) {fprintf(stderr,"FBX load: %s\n",error.description.data);return 0;}
    int ok=0;ufbx_anim_stack *clip=NULL;
    for (size_t i=0;i<scene->anim_stacks.count;++i) {
        ufbx_anim_stack *s=scene->anim_stacks.data[i];
        if (o->list) printf("clip %zu: %s [%.9g, %.9g]\n",i,s->name.data,s->time_begin,s->time_end);
        if (clip_index(o->clip,s->name.data,i)) {if (clip) goto done;clip=s;}
    }
    if (o->list) {for (size_t i=0;i<scene->nodes.count;++i) printf("node %zu: %s\n",i,scene->nodes.data[i]->name.data);ok=1;goto done;}
    if (!clip || scene->nodes.count>MOTION_MAX_NODES) {fprintf(stderr,"FBX clip missing, ambiguous or too many nodes\n");goto done;}
    a->name=motion_string(clip->name.data);if (!a->name) goto done;
    for (size_t i=0;i<scene->nodes.count;++i) {
        ufbx_node *node=scene->nodes.data[i];char fallback[64];snprintf(fallback,sizeof(fallback),"__fbx_node_%zu",i);
        motion_node n={.parent=node->parent ? node->parent->typed_id : SIZE_MAX,.rest=node->local_transform};
        n.name=motion_string(node->name.length ? node->name.data : fallback);
        if (!n.name || !transform_valid(&n.rest) || !xrtArrayPush(&a->nodes,&n)) {xrtFree(n.name);goto done;}
    }
    double begin;if (!samples(a,o,clip->time_end-clip->time_begin,&begin)) goto done;
    for (size_t f=0;f<a->times.Count;++f) for (size_t n=0;n<a->nodes.Count;++n) {
        ufbx_transform t=ufbx_evaluate_transform(clip->anim,scene->nodes.data[n],clip->time_begin+begin+((float*)a->times.Data)[f]);
        if (!transform_valid(&t)) goto done;
        ((ufbx_transform*)a->poses.Data)[f*a->nodes.Count+n]=t;
    }
    ok=1;
done:
    ufbx_free_scene(scene);return ok;
}

typedef struct bvh_parser { const char *cursor;char token[256];motion_asset *asset; } bvh_parser;
static int token(bvh_parser *p)
{
    while (isspace((unsigned char)*p->cursor)) ++p->cursor;
    size_t length=0;
    if (*p->cursor=='{' || *p->cursor=='}') p->token[length++]=*p->cursor++;
    else while (*p->cursor && !isspace((unsigned char)*p->cursor) && *p->cursor!='{' && *p->cursor!='}') {
        if (length+1==sizeof(p->token)) return 0;
        p->token[length++]=*p->cursor++;
    }
    p->token[length]=0;return length!=0;
}
static int expect(bvh_parser *p,const char *word) {return token(p) && !strcmp(p->token,word);}
static int real(bvh_parser *p,double *out)
{
    if (!token(p)) return 0;
    char *end;*out=strtod(p->token,&end);return !*end && isfinite(*out);
}
static int bvh_node(bvh_parser *p,size_t parent,int end_site,size_t depth)
{
    if (depth>128 || p->asset->nodes.Count>=MOTION_MAX_NODES) return 0;
    motion_node node={.parent=parent,.rest={.rotation={.w=1},.scale={.x=1,.y=1,.z=1}}};char label[256];
    if (end_site) snprintf(label,sizeof(label),"__end_%zu",p->asset->nodes.Count);
    else {if (!token(p)) return 0;memcpy(label,p->token,sizeof(label));}
    node.name=motion_string(label);if (!node.name) return 0;
    size_t index=p->asset->nodes.Count;
    if (!xrtArrayPush(&p->asset->nodes,&node)) {xrtFree(node.name);return 0;}
    if (!expect(p,"{") || !expect(p,"OFFSET")) return 0;
    for (int i=0;i<3;++i) if (!real(p,&node.rest.translation.v[i])) return 0;
    if (!end_site) {
        double count;if (!expect(p,"CHANNELS") || !real(p,&count) || count<0 || count>6 || count!=floor(count)) return 0;
        node.channel_count=(int)count;unsigned seen=0;
        for (int i=0;i<node.channel_count;++i) {
            if (!token(p)) return 0;
            int axis=p->token[0]=='X' ? 0 : p->token[0]=='Y' ? 1 : p->token[0]=='Z' ? 2 : -1;
            int type=!strcmp(p->token+1,"position") ? 0 : !strcmp(p->token+1,"rotation") ? 3 : -1;
            if (axis<0 || type<0 || (seen&(1u<<(axis+type)))) return 0;
            node.channels[i]=axis+type;seen|=1u<<(axis+type);
        }
    }
    *(motion_node*)xrtArrayGet(&p->asset->nodes,index)=node;
    while (token(p)) {
        if (!strcmp(p->token,"}")) return 1;
        if (!strcmp(p->token,"JOINT")) {if (!bvh_node(p,index,0,depth+1)) return 0;}
        else if (!strcmp(p->token,"End")) {if (!expect(p,"Site") || !bvh_node(p,index,1,depth+1)) return 0;}
        else return 0;
    }
    return 0;
}
static ufbx_transform bvh_axes(ufbx_transform t,const motion_options *o)
{
    for (int k=0;k<3;++k) t.translation.v[k]*=o->unit;
    if (o->z_up) {
        t.translation=(ufbx_vec3){.x=t.translation.x,.y=t.translation.z,.z=-t.translation.y};
        const double s=.7071067811865475244;
        t.rotation=ufbx_quat_mul((ufbx_quat){.x=-s,.w=s},ufbx_quat_mul(t.rotation,(ufbx_quat){.x=s,.w=s}));
    }
    return t;
}
static int bvh_load(const char *path,const motion_options *o,motion_asset *a)
{
    size_t length;unsigned char *raw=xrtFileReadAllLimit(path,MOTION_MAX_BYTES,&length);if (!raw) return 0;
    char *text=xrtMalloc(length+1);if (!text) {xrtFree(raw);return 0;}memcpy(text,raw,length);text[length]=0;xrtFree(raw);
    int ok=0;xarray poses;xrtArrayInit(&poses,sizeof(ufbx_transform));bvh_parser p={.cursor=text,.asset=a};
    if (memchr(text,0,length) || !expect(&p,"HIERARCHY") || !expect(&p,"ROOT") || !bvh_node(&p,SIZE_MAX,0,0) || !expect(&p,"MOTION")) goto done;
    double frames,step;
    if (!expect(&p,"Frames:") || !real(&p,&frames) || frames<1 || frames!=floor(frames) ||
        !expect(&p,"Frame") || !expect(&p,"Time:") || !real(&p,&step) || step<=0 || frames>MOTION_MAX_BYTES/sizeof(ufbx_transform)/a->nodes.Count) goto done;
    if (o->list) {printf("clip 0: BVH [0, %.9g] %g frames\n",(frames-1)*step,frames);
        for (size_t i=0;i<a->nodes.Count;++i) printf("node %zu: %s\n",i,((motion_node*)a->nodes.Data)[i].name);ok=1;goto done;}
    if (!clip_index(o->clip,"BVH",0) || !xrtArrayResize(&poses,(size_t)frames*a->nodes.Count)) goto done;
    for (size_t f=0;f<(size_t)frames;++f) for (size_t n=0;n<a->nodes.Count;++n) {
        motion_node *node=xrtArrayGet(&a->nodes,n);ufbx_transform t=node->rest;
        for (int c=0;c<node->channel_count;++c) {
            double value;if (!real(&p,&value)) goto done;int channel=node->channels[c];
            if (channel<3) t.translation.v[channel]+=value;
            else {double angle=value*.00872664625997164788;ufbx_quat q={.w=cos(angle)};q.v[channel-3]=sin(angle);t.rotation=ufbx_quat_mul(t.rotation,q);}
        }
        t=bvh_axes(t,o);if (!transform_valid(&t)) goto done;((ufbx_transform*)poses.Data)[f*a->nodes.Count+n]=t;
    }
    if (token(&p)) goto done;
    for (size_t i=0;i<a->nodes.Count;++i) {motion_node *node=xrtArrayGet(&a->nodes,i);node->rest=bvh_axes(node->rest,o);}
    double begin;if (!samples(a,o,(frames-1)*step,&begin)) goto done;
    for (size_t f=0;f<a->times.Count;++f) {
        double source_time=begin+((float*)a->times.Data)[f],position=source_time/step;
        size_t lo=(size_t)fmin(floor(position),frames-1),hi=(size_t)fmin(lo+1,frames-1);double t=fmin(fmax(position-lo,0),1);
        for (size_t n=0;n<a->nodes.Count;++n) {
            ufbx_transform x=((ufbx_transform*)poses.Data)[lo*a->nodes.Count+n],y=((ufbx_transform*)poses.Data)[hi*a->nodes.Count+n];
            for (int k=0;k<3;++k) x.translation.v[k]=x.translation.v[k]*(1-t)+y.translation.v[k]*t;
            x.rotation=ufbx_quat_slerp(x.rotation,y.rotation,t);((ufbx_transform*)a->poses.Data)[f*a->nodes.Count+n]=x;
        }
    }
    a->name=motion_string("BVH");ok=a->name!=NULL;
done:
    if (!ok) fprintf(stderr,"Invalid or unsupported BVH near: %.80s\n",p.cursor);
    xrtArrayUnit(&poses);xrtFree(text);return ok;
}
typedef struct gltf_track { size_t node;int path,cubic,step; xarray times,values; } gltf_track;
static int gltf_sample(const gltf_track *track,float time,ufbx_transform *pose)
{
    const float *times=(const float*)track->times.Data,*v=(const float*)track->values.Data;size_t lo=0,hi=track->times.Count-1;
    while (lo<hi) {size_t middle=(lo+hi+1)/2;if (times[middle]<=time) lo=middle;else hi=middle-1;}
    size_t next=lo+1<track->times.Count ? lo+1 : lo,n=track->path==cgltf_animation_path_type_rotation ? 4 : 3;
    size_t stride=n*(track->cubic ? 3 : 1);const float *x=v+lo*stride+(track->cubic ? n : 0),*y=v+next*stride+(track->cubic ? n : 0);
    double span=times[next]-times[lo],t=span>0 ? fmin(fmax((time-times[lo])/span,0),1) : 0;
    if (track->step) t=0;
    double *dest=track->path==cgltf_animation_path_type_rotation ? pose->rotation.v : track->path==cgltf_animation_path_type_translation ? pose->translation.v : pose->scale.v;
    if (n==4 && !track->cubic) {
        pose->rotation=ufbx_quat_slerp((ufbx_quat){.x=x[0],.y=x[1],.z=x[2],.w=x[3]},(ufbx_quat){.x=y[0],.y=y[1],.z=y[2],.w=y[3]},t);
    } else for (size_t i=0;i<n;++i) {
        if (track->cubic && span>0) {double t2=t*t,t3=t2*t;dest[i]=(2*t3-3*t2+1)*x[i]+(t3-2*t2+t)*span*x[n+i]+(-2*t3+3*t2)*y[i]+(t3-t2)*span*y[(ptrdiff_t)i-(ptrdiff_t)n];}
        else dest[i]=x[i]*(1-t)+y[i]*t;
    }
    return transform_valid(pose);
}
static int gltf_load(const char *path,const motion_options *o,motion_asset *a)
{
    cgltf_options options={0};cgltf_data *data=NULL;int ok=0;xarray tracks;xrtArrayInit(&tracks,sizeof(gltf_track));
    if (cgltf_parse_file(&options,path,&data)!=cgltf_result_success || data->nodes_count>MOTION_MAX_NODES) goto done;
    size_t total=0;
    for (size_t i=0;i<data->buffers_count;++i) {if (data->buffers[i].size>MOTION_MAX_BYTES-total) goto done;total+=data->buffers[i].size;}
    if (cgltf_load_buffers(&options,data,path)!=cgltf_result_success || cgltf_validate(data)!=cgltf_result_success) goto done;
    cgltf_animation *clip=NULL;
    for (size_t i=0;i<data->animations_count;++i) {
        const char *label=data->animations[i].name ? data->animations[i].name : "";
        if (o->list) printf("clip %zu: %s\n",i,label);
        if (clip_index(o->clip,label,i)) {if (clip) goto done;clip=&data->animations[i];}
    }
    if (o->list) {for (size_t i=0;i<data->nodes_count;++i) printf("node %zu: %s\n",i,data->nodes[i].name ? data->nodes[i].name : "");ok=1;goto done;}
    for (size_t i=0;i<data->nodes_count;++i) {
        const cgltf_node *n=&data->nodes[i];char fallback[64];snprintf(fallback,sizeof(fallback),"__gltf_node_%zu",i);
        motion_node node={.parent=n->parent ? cgltf_node_index(data,n->parent) : SIZE_MAX};node.name=motion_string(n->name ? n->name : fallback);
        float matrix[16];cgltf_node_transform_local(n,matrix);ufbx_matrix m={0};
        for (int c=0;c<4;++c) for (int r=0;r<3;++r) m.cols[c].v[r]=matrix[c*4+r];
        node.rest=ufbx_matrix_to_transform(&m);ufbx_matrix check=ufbx_transform_to_matrix(&node.rest);int exact=1;
        for (int c=0;c<4;++c) for (int r=0;r<3;++r) if (fabs(check.cols[c].v[r]-m.cols[c].v[r])>1e-5*(1+fabs(m.cols[c].v[r]))) exact=0;
        if (!node.name || !exact || !transform_valid(&node.rest) || !xrtArrayPush(&a->nodes,&node)) {xrtFree(node.name);goto done;}
    }
    if (o->reference) {a->name=motion_string(path);ok=a->name!=NULL;goto done;}
    if (!clip) {fprintf(stderr,"glTF animation missing or ambiguous\n");goto done;}
    a->name=motion_string(clip->name ? clip->name : "Animation");if (!a->name) goto done;
    double duration=0;size_t track_bytes=0;
    for (size_t i=0;i<clip->channels_count;++i) {
        const cgltf_animation_channel *channel=&clip->channels[i];const cgltf_animation_sampler *s=channel->sampler;
        gltf_track t={0};xrtArrayInit(&t.times,sizeof(float));xrtArrayInit(&t.values,sizeof(float));int valid=0;
        if (!channel->target_node || channel->target_node->has_matrix || !s || !s->input || !s->output) goto failed_track;
        t.node=cgltf_node_index(data,channel->target_node);t.path=(int)channel->target_path;
        t.cubic=s->interpolation==cgltf_interpolation_type_cubic_spline;t.step=s->interpolation==cgltf_interpolation_type_step;
        size_t count=s->input->count,n=t.path==cgltf_animation_path_type_rotation ? 4 : 3,multiplier=t.cubic ? 3 : 1;
        if ((t.path!=cgltf_animation_path_type_translation && t.path!=cgltf_animation_path_type_rotation && t.path!=cgltf_animation_path_type_scale) ||
            (!t.cubic && !t.step && s->interpolation!=cgltf_interpolation_type_linear) || !count || count>MOTION_MAX_BYTES/(sizeof(float)*(1+n*multiplier)) ||
            s->input->type!=cgltf_type_scalar || s->input->component_type!=cgltf_component_type_r_32f ||
            cgltf_num_components(s->output->type)!=n || s->output->component_type!=cgltf_component_type_r_32f || s->output->count!=count*multiplier) goto failed_track;
        size_t bytes=count*sizeof(float)*(1+n*multiplier);if (bytes>MOTION_MAX_BYTES-track_bytes) goto failed_track;track_bytes+=bytes;
        if (!xrtArrayResize(&t.times,count) || !xrtArrayResize(&t.values,count*n*multiplier)) goto failed_track;
        if (cgltf_accessor_unpack_floats(s->input,(float*)t.times.Data,count)!=count ||
            cgltf_accessor_unpack_floats(s->output,(float*)t.values.Data,t.values.Count)!=t.values.Count) goto failed_track;
        for (size_t k=0;k<count;++k) {float value=((float*)t.times.Data)[k];
            if (!isfinite(value) || value<0 || (k && value<=((float*)t.times.Data)[k-1])) goto failed_track;duration=fmax(duration,value);}
        for (size_t k=0;k<t.values.Count;++k) if (!isfinite(((float*)t.values.Data)[k])) goto failed_track;
        if (n==4) for (size_t k=0;k<count;++k) {
            const float *q=(const float*)t.values.Data+(k*multiplier+(t.cubic ? 1 : 0))*4;double length=0;
            for (int c=0;c<4;++c) length+=(double)q[c]*q[c];if (length<1e-20) goto failed_track;
        }
        for (size_t k=0;k<tracks.Count;++k) {gltf_track *previous=xrtArrayGet(&tracks,k);if (previous->node==t.node && previous->path==t.path) goto failed_track;}
        valid=xrtArrayPush(&tracks,&t);
failed_track:
        if (!valid) {xrtArrayUnit(&t.times);xrtArrayUnit(&t.values);goto done;}
    }
    double begin;if (!samples(a,o,duration,&begin)) goto done;
    for (size_t f=0;f<a->times.Count;++f) {
        ufbx_transform *poses=(ufbx_transform*)a->poses.Data+f*a->nodes.Count;
        for (size_t n=0;n<a->nodes.Count;++n) poses[n]=((motion_node*)a->nodes.Data)[n].rest;
        for (size_t t=0;t<tracks.Count;++t) {gltf_track *track=xrtArrayGet(&tracks,t);if (!gltf_sample(track,(float)(begin+((float*)a->times.Data)[f]),&poses[track->node])) goto done;}
    }
    ok=1;
done:
    if (!ok) fprintf(stderr,"Invalid or unsupported glTF motion/reference: %s\n",path);
    for (size_t i=0;i<tracks.Count;++i) {gltf_track *t=xrtArrayGet(&tracks,i);xrtArrayUnit(&t->times);xrtArrayUnit(&t->values);}
    xrtArrayUnit(&tracks);cgltf_free(data);return ok;
}
int motion_load(const char *path,const motion_options *o,motion_asset *a)
{
    const char *extension=strrchr(path,'.');char kind[16]={0};if (!extension || strlen(extension)>=sizeof(kind)) return 0;
    for (size_t i=0;extension[i];++i) kind[i]=(char)tolower((unsigned char)extension[i]);
    if (!strcmp(kind,".fbx")) return fbx_load(path,o,a);
    if (!strcmp(kind,".bvh")) return bvh_load(path,o,a);
    if (!strcmp(kind,".gltf") || !strcmp(kind,".glb")) return gltf_load(path,o,a);
    fprintf(stderr,"Unsupported motion format: %s\n",path);return 0;
}
