/* A single moving outgoing clip per layer; interrupted blends use a bounded
 * channel snapshot. Scratch storage is shared and reused by all eight layers. */
static double d3_transition_alpha(const d3_transition *t,double delta)
{return t->duration>0?fmin(1,(t->time+delta)/t->duration):1;}
static float d3_layer_gain(const d3_layer *l,double delta)
{return l->fade_duration>0?(float)(l->fade_from+(l->fade_to-l->fade_from)*fmin(1,(l->fade_time+delta)/l->fade_duration)):l->weight;}
static void d3_playback_mean(const d3_layer *l,const d3_transition *t,double delta,float means[2])
{
    if(delta<=0){double alpha=d3_transition_alpha(t,0);means[0]=(float)(l->weight*alpha);means[1]=(float)(l->weight*(1-alpha));return;}
    double cuts[]={delta,l->fade_duration-l->fade_time,t->duration-t->time},begin=0,sum[2]={0};
    for(int part=0;part<3&&begin<delta;++part){double end=delta;
        for(int i=1;i<3;++i)if(cuts[i]>begin&&cuts[i]<end)end=cuts[i];
        double at[]={begin,(begin+end)*.5,end},value[3];
        for(int i=0;i<3;++i)value[i]=d3_layer_gain(l,at[i])*d3_transition_alpha(t,at[i]);
        sum[0]+=(end-begin)*(value[0]+4*value[1]+value[2])/6;
        for(int i=0;i<3;++i)value[i]=d3_layer_gain(l,at[i])*(1-d3_transition_alpha(t,at[i]));
        sum[1]+=(end-begin)*(value[0]+4*value[1]+value[2])/6;begin=end;
    }
    means[0]=(float)(sum[0]/delta);means[1]=(float)(sum[1]/delta);
}
static void d3_channel_merge(d3_channel_pose *p,int path,const float *value,float weight)
{
    if(weight<=0)return;
    int channel=path-1;float total=p->weights[channel]+weight;
    float t=weight/total;p->weights[channel]=total;
    if(path==cgltf_animation_path_type_rotation){xge3d_quat_t q;memcpy(&q,value,sizeof(q));p->value.rotation=d3_slerp(p->value.rotation,q,t);}
    else{xge3d_vec3_t *out=path==cgltf_animation_path_type_translation?&p->value.position:&p->value.scale;
        *out=d3_add(d3_scale(*out,1-t),d3_scale((xge3d_vec3_t){value[0],value[1],value[2]},t));}
}
static void d3_channels_add(xge3d_animator *a,const d3_layer *l,double time,float factor)
{
    if(!l->clip||factor<=0)return;
    for(size_t i=0;i<l->clip->tracks.Count;++i){const d3_track *track=xrtArrayConstGet(&l->clip->tracks,i);size_t node=((size_t*)l->targets.Data)[i];
        float weight=factor*((float*)l->mask.Data)[node],value[4]={0};if(weight<=0)continue;
        d3_sample(track,(float)time,value);d3_channel_merge(xrtArrayGet(&a->blend,node),track->path,value,weight);}
}
static void d3_channels_blend(xge3d_animator *a,int index,double incoming,double previous,double alpha)
{
    const d3_transition *t=&a->transitions[index];
    if(t->snapshot.Count)memcpy(a->blend.Data,t->snapshot.Data,t->snapshot.Count*t->snapshot.ItemSize);
    else{for(size_t i=0;i<a->nodes.Count;++i){d3_channel_pose *p=xrtArrayGet(&a->blend,i);*p=(d3_channel_pose){.value=*(xge3d_transform_t*)xrtArrayGet(&a->rest,i)};}
        d3_channels_add(a,&t->previous,previous,1);}
    for(size_t i=0;i<a->nodes.Count;++i){d3_channel_pose *p=xrtArrayGet(&a->blend,i);for(int k=0;k<3;++k)p->weights[k]*=(float)(1-alpha);}
    d3_channels_add(a,&a->layers[index],incoming,(float)alpha);
}
static int d3_apply_channel(xge3d_animator *a,size_t node,int path,const float *value,float weight,int mode)
{
    if(weight<=0)return XGE_OK;
    weight=fminf(1,weight);
    xge3d_transform_t *pose=xrtArrayGet(&a->pose,node),*rest=xrtArrayGet(&a->rest,node);
    if(mode==XGE3D_ANIMATION_ADDITIVE){
        if(path==cgltf_animation_path_type_rotation){xge3d_quat_t q;memcpy(&q,value,sizeof(q));
            xge3d_quat_t inverse={-rest->rotation.x,-rest->rotation.y,-rest->rotation.z,rest->rotation.w};
            pose->rotation=d3_quat_mul(pose->rotation,d3_slerp((xge3d_quat_t){0,0,0,1},d3_quat_mul(inverse,q),weight));
        }else if(path==cgltf_animation_path_type_translation)pose->position=d3_add(pose->position,d3_scale(d3_sub((xge3d_vec3_t){value[0],value[1],value[2]},rest->position),weight));
        else for(int k=0;k<3;++k){float reference=(&rest->scale.x)[k];
            if(fabsf(reference)<1e-12f){if(fabsf(value[k])>1e-12f)return XGE_ERROR_INVALID_ARGUMENT;}
            else(&pose->scale.x)[k]*=1+weight*(value[k]/reference-1);}
    }else if(path==cgltf_animation_path_type_rotation){xge3d_quat_t q;memcpy(&q,value,sizeof(q));pose->rotation=d3_slerp(pose->rotation,q,weight);}
    else{xge3d_vec3_t *out=path==cgltf_animation_path_type_translation?&pose->position:&pose->scale;
        *out=d3_add(d3_scale(*out,1-weight),d3_scale((xge3d_vec3_t){value[0],value[1],value[2]},weight));}
    return XGE_OK;
}
static int d3_playback_pose(xge3d_animator *a,int index,double time,double previous,float weight,double alpha)
{
    const d3_layer *l=&a->layers[index];int result=XGE_OK;
    if(a->transitions[index].duration>0){d3_channels_blend(a,index,time,previous,alpha);
        for(size_t i=0;i<a->nodes.Count;++i){const d3_channel_pose *p=xrtArrayConstGet(&a->blend,i);
            const float *values[]={&p->value.position.x,&p->value.rotation.x,&p->value.scale.x};
            for(int k=0;k<3;++k){result=d3_apply_channel(a,i,k+1,values[k],weight*p->weights[k],a->modes[index]);if(result)return result;}}
    }else for(size_t i=0;i<l->clip->tracks.Count;++i){const d3_track *track=xrtArrayConstGet(&l->clip->tracks,i);size_t node=((size_t*)l->targets.Data)[i];float value[4]={0};
        float gain=weight*((float*)l->mask.Data)[node];if(gain<=0)continue;d3_sample(track,(float)time,value);
        result=d3_apply_channel(a,node,track->path,value,gain,a->modes[index]);if(result)return result;}
    return result;
}
int xge3dAnimatorFadeLayer(xge3d_animator *a,int index,float weight,float seconds)
{
    if(!a||index<0||index>=XGE3D_ANIMATION_MAX_LAYERS||!isfinite(weight)||weight<0||weight>1||!isfinite(seconds)||seconds<0)return XGE_ERROR_INVALID_ARGUMENT;
    d3_layer *l=&a->layers[index];if(!l->clip)return XGE_ERROR_NOT_FOUND;
    l->fade_from=l->weight;l->fade_to=weight;l->fade_duration=seconds;l->fade_time=0;l->stopping=0;
    if(!seconds)l->weight=weight;
    return XGE_OK;
}
int xge3dAnimatorPlay(xge3d_animator *a,int index,xge3d_clip *clip,const xge3d_animation_layer_t *desc,float seconds)
{
    if(!a||index<0||index>=XGE3D_ANIMATION_MAX_LAYERS||!isfinite(seconds)||seconds<0)return XGE_ERROR_INVALID_ARGUMENT;
    if(!seconds)return xge3dAnimatorSetLayer(a,index,clip,desc);
    d3_layer *l=&a->layers[index];d3_transition *t=&a->transitions[index];
    if(!clip){if(!l->clip)return XGE_OK;int result=xge3dAnimatorFadeLayer(a,index,0,seconds);if(!result)l->stopping=1;return result;}
    d3_layer incoming={0};int result=d3_layer_bind(a,clip,desc,&incoming);if(result)return result;
    xarray snapshot;xrtArrayInit(&snapshot,sizeof(d3_channel_pose));
    if(l->clip&&!xrtArrayResize(&a->blend,a->nodes.Count)){result=XGE_ERROR_OUT_OF_MEMORY;goto done;}
    if(t->duration>0){
        if(!xrtArrayResize(&snapshot,a->nodes.Count)){result=XGE_ERROR_OUT_OF_MEMORY;goto done;}
        d3_channels_blend(a,index,l->time,t->previous.time,d3_transition_alpha(t,0));
        for(size_t i=0;i<a->blend.Count;++i){xge3d_mat4_t matrix;result=xge3dTransformMatrix(&((d3_channel_pose*)a->blend.Data)[i].value,&matrix);if(result)goto done;}
        memcpy(snapshot.Data,a->blend.Data,a->blend.Count*a->blend.ItemSize);
    }
    float old_weight=l->clip?l->weight:0;int paused=l->paused;
    if(t->duration>0){d3_layer_free(l);d3_transition_free(t);t->snapshot=snapshot;memset(&snapshot,0,sizeof(snapshot));}
    else{t->previous=*l;memset(l,0,sizeof(*l));xrtArrayUnit(&t->previous.events);}
    if(t->previous.clip||t->snapshot.Count){t->duration=seconds;t->time=0;}
    incoming.fade_from=old_weight;incoming.fade_to=incoming.weight;incoming.weight=old_weight;incoming.fade_duration=seconds;incoming.paused=paused;
    *l=incoming;return XGE_OK;
done:d3_layer_free(&incoming);xrtArrayUnit(&snapshot);return result;
}
int xge3dAnimatorSetPaused(xge3d_animator *a,int paused)
{if(!a||(paused!=0&&paused!=1))return XGE_ERROR_INVALID_ARGUMENT;a->paused=paused;return XGE_OK;}
int xge3dAnimatorSetLayerPaused(xge3d_animator *a,int index,int paused)
{if(!a||index<0||index>=XGE3D_ANIMATION_MAX_LAYERS||(paused!=0&&paused!=1))return XGE_ERROR_INVALID_ARGUMENT;a->layers[index].paused=paused;return XGE_OK;}
int xge3dAnimatorGetPlayback(const xge3d_animator *a,int index,xge3d_playback_info_t *out)
{
    if(!a||index<0||index>=XGE3D_ANIMATION_MAX_LAYERS||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    const d3_layer *l=&a->layers[index];const d3_transition *t=&a->transitions[index];
    xge3d_playback_info_t info={.size=sizeof(info),.paused=a->paused||l->paused,.mode=a->modes[index]};
    if(l->clip){info.active=1;info.time=(float)l->time;info.duration=l->clip->duration;info.speed=l->speed;info.weight=l->weight;info.loop=l->loop;
        info.target_weight=l->fade_duration>0?l->fade_to:l->weight;info.finished=!l->loop&&l->time>=l->clip->duration;
        info.fade_remaining=(float)fmax(l->fade_duration-l->fade_time,t->duration-t->time);info.transitioning=t->duration>0;info.stopping=l->stopping;}
    *out=info;return XGE_OK;
}
static void d3_playback_advance(const d3_layer *l,double delta,double *time,double *cycles)
{
    *cycles=0;if(!l->clip){*time=0;return;}*time=l->time+delta*l->speed;
    if(l->loop&&l->clip->duration>0){*cycles=floor(*time/l->clip->duration);*time=fmod(*time,l->clip->duration);}else *time=fmin(*time,l->clip->duration);
}
