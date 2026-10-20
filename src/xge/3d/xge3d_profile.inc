xge3d_renderer_options_t xge3dRendererOptionsDefault(void)
{return (xge3d_renderer_options_t){sizeof(xge3d_renderer_options_t),UINT32_MAX,UINT32_MAX,8,0};}
int xge3dRendererSetOptions(xge3d_renderer *r,const xge3d_renderer_options_t *options)
{
    if(!r)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_renderer_options_t d=options?*options:xge3dRendererOptionsDefault();
    if(d.size!=sizeof(d)||!d.max_lights||d.max_lights>8||(d.profiling!=0&&d.profiling!=1))return XGE_ERROR_INVALID_ARGUMENT;
    if(r->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    r->options=d;return XGE_OK;
}
int xge3dRendererProfile(const xge3d_renderer *r,xge3d_render_profile_t *out)
{
    if(!r||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    *out=r->profile;out->size=sizeof(*out);return XGE_OK;
}
static void d3_profile_begin(xge3d_renderer *r)
{
    ++r->sequence;r->active_query=-1;
    if(!r->options.profiling||!xge_glGenQueries||!xge_glQueryCounter||!xge_glGetQueryObjectiv||!xge_glGetQueryObjectui64v||!xge_glDeleteQueries)return;
    const char *version=(const char*)xge_glGetString(0x1F02);
    if(!version||strstr(version,"OpenGL ES"))return;
    if(!r->queries[0]){GLint major=0,minor=0;xge_glGetIntegerv(0x821B,&major);xge_glGetIntegerv(0x821C,&minor);
        if(major<3||(major==3&&minor<3)){return;}xge_glGenQueries(16,r->queries);}
    for(int i=0;i<8;++i)if(r->pending_queries[i]){
        GLint ready=0;xge_glGetQueryObjectiv(r->queries[i*2+1],0x8867,&ready);if(!ready)continue;
        uint64_t first=0,last=0;xge_glGetQueryObjectui64v(r->queries[i*2],0x8866,&first);xge_glGetQueryObjectui64v(r->queries[i*2+1],0x8866,&last);
        if(r->pending_queries[i]>r->last_gpu_sample){r->last_gpu_sample=r->pending_queries[i];r->last_gpu_ns=last>=first?last-first:0;}
        r->pending_queries[i]=0;
    }
    r->profile.gpu_available=r->last_gpu_sample!=0;r->profile.gpu_ns=r->last_gpu_ns;r->profile.gpu_sample=r->last_gpu_sample;
    for(int i=0;i<8;++i)if(!r->pending_queries[i]){r->active_query=i;xge_glQueryCounter(r->queries[i*2],0x8E28);break;}
}
static void d3_profile_end(xge3d_renderer *r)
{
    if(r->active_query<0)return;
    int i=r->active_query;xge_glQueryCounter(r->queries[i*2+1],0x8E28);r->pending_queries[i]=r->sequence;r->active_query=-1;
}
#if XGE3D_ENABLE_ASYNC
int xge3dRendererPrepare(xge3d_renderer *r,int resolution,const xge3d_upload_budget_t *budget,xge3d_upload_stats_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}memset(out,0,sizeof(*out));
    if(!r||!budget||resolution<0)return XGE_ERROR_INVALID_ARGUMENT;
    if(r->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    if(!resolution){out->completed=1;return XGE_OK;}
#if XGE3D_ENABLE_SHADOW
    if(resolution<16||resolution>8192)return XGE_ERROR_INVALID_ARGUMENT;
    if(r->shadow_texture&&r->shadow_resolution==resolution){out->completed=1;return XGE_OK;}
    if(!budget->operations||!budget->bytes)return XGE_OK;
    int result=xgeFlush();if(result)return result;d3_state state;d3_save(&state);
    result=d3_shadow_target(r,resolution);d3_restore(&state);out->operations=1;out->completed=result==XGE_OK;return result;
#else
    return XGE_ERROR_UNSUPPORTED;
#endif
}
#if XGE3D_ENABLE_ANIMATION
int xge3dNodePrepareSkin(xge3d_scene *s,xge3d_node_t handle,const xge3d_upload_budget_t *budget,xge3d_upload_stats_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}memset(out,0,sizeof(*out));
    xge3d_node_data *n=d3_node(s,handle);if(!n||!budget)return XGE_ERROR_INVALID_ARGUMENT;
    if(!n->skin){out->completed=1;return XGE_OK;}
    if(!__xge3dContext())return XGE_ERROR_NOT_INITIALIZED;
    if(n->skin->context&&n->skin->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    int result=d3_skin_update(s,n);if(result)return result;
    d3_skin *skin=n->skin;if(skin->texture&&skin->uploaded_version==skin->pose_version){out->completed=1;return XGE_OK;}
    uint64_t bytes=skin->palette.Count*sizeof(xge3d_mat4_t);
    if(!budget->operations||budget->bytes<bytes){out->min_next_bytes=bytes;return XGE_OK;}
    if(!__xge3dContext())return XGE_ERROR_NOT_INITIALIZED;
    result=xgeFlush();if(result)return result;d3_state state;d3_save(&state);
    result=d3_skin_upload(skin,&out->upload_bytes);d3_restore(&state);out->operations=1;out->completed=result==XGE_OK;return result;
}
#endif
#endif
