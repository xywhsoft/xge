#if XGE3D_ENABLE_SHADER
static char *d3_hook_source(const char *body,const char *hook,const char *identity,const char *define)
{
    const char *source=hook?hook:identity;size_t a=strlen(body),b=strlen(source),c=strlen(define);
    if(b>65536){return NULL;}char *s=xrtMalloc(a+b+c+160);if(!s){return NULL;}
    snprintf(s,a+b+c+160,"#ifdef GL_ES\nprecision highp float;\n#endif\n#define %s\nuniform vec4 xge3dParams[4];\n%s\n%s",define,source,body);return s;
}
int xge3dShaderCreate(const xge3d_shader_desc_t *d,xge3d_shader **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;
    if(!d||d->size!=sizeof(*d)||!isfinite(d->bounds_margin)||d->bounds_margin<0||d->bounds_margin>1e6f||
        (d->vertex&&strlen(d->vertex)>65536)||(d->surface&&strlen(d->surface)>65536)){return XGE_ERROR_INVALID_ARGUMENT;}
    int result=d3_ready();if(result){return result;}xge3d_shader *s=xrtCalloc(1,sizeof(*s));if(!s){return XGE_ERROR_OUT_OF_MEMORY;}
    s->refs=1;s->context=__xge3dContext();s->margin=d->bounds_margin;
    char *vs=d3_hook_source(d3_vertex_shader,d->vertex,"void xge3dVertex(inout vec3 p,inout vec3 n,vec2 uv){}","D3_CUSTOM_VERTEX");
    const char *identity="void xge3dSurface(inout vec4 base,inout vec3 emission,vec2 uv){}";
    char *fs=d3_hook_source(d3_fragment_shader,d->surface,identity,"D3_CUSTOM_SURFACE");
    result=vs&&fs?d3_program_create(&s->program.shader,vs,fs):XGE_ERROR_OUT_OF_MEMORY;
#if XGE3D_ENABLE_SHADOW
    char *depth=d3_hook_source(d3_depth_fragment_shader,d->surface,identity,"D3_CUSTOM_SURFACE");
    if(!depth&&!result){result=XGE_ERROR_OUT_OF_MEMORY;}if(!result){result=d3_program_create(&s->program.depth_shader,vs,depth);}xrtFree(depth);
#endif
    xrtFree(vs);xrtFree(fs);if(result){xge3dShaderFree(s);return result;}d3_program_locations(&s->program);*out=s;return XGE_OK;
}
void xge3dShaderFree(xge3d_shader *s)
{
    if(!s||--s->refs){return;}if(s->context==__xge3dContext()){xgeShaderFree(&s->program.shader);
#if XGE3D_ENABLE_SHADOW
        xgeShaderFree(&s->program.depth_shader);
#endif
    }xrtFree(s);
}
int xge3dMaterialSetShader(xge3d_material *m,xge3d_shader *s,const float parameters[16])
{
    if(!m){return XGE_ERROR_INVALID_ARGUMENT;}if(parameters){for(int i=0;i<16;++i){if(!isfinite(parameters[i])){return XGE_ERROR_INVALID_ARGUMENT;}}}
    if(s){if(s->refs==UINT32_MAX){return XGE_ERROR_INVALID_STATE;}++s->refs;}
    xge3d_shader *old=m->shader;m->shader=s;if(parameters){memcpy(m->parameters,parameters,sizeof(m->parameters));}else memset(m->parameters,0,sizeof(m->parameters));xge3dShaderFree(old);return XGE_OK;
}
static int d3_select_program(xge3d_renderer *r,const xge3d_node_data *n)
{
    xge3d_shader *s=n->material?n->material->shader:NULL;
    if(s&&s->context!=r->context){return XGE_ERROR_INVALID_STATE;}r->program=s?&s->program:&r->base;return XGE_OK;
}
static void d3_custom_bounds(const xge3d_node_data *n,xge3d_mat4_t world,xge3d_aabb_t *box)
{
    if(!n->material||!n->material->shader||!n->material->shader->margin){return;}
    double scale=1;
#if XGE3D_ENABLE_ANIMATION
    if(n->skin){scale=0;for(size_t i=0;i<n->skin->palette.Count;++i){const xge3d_mat4_t *m=xrtArrayGet(&n->skin->palette,i);
        for(int row=0;row<3;++row){double s=fabs(m->m[row])+fabs(m->m[4+row])+fabs(m->m[8+row]);if(s>scale){scale=s;}}}}
#endif
    for(int i=0;i<3;++i){double margin=n->material->shader->margin*scale*(fabs(world.m[i])+fabs(world.m[4+i])+fabs(world.m[8+i]));
        (&box->min.x)[i]-=(float)margin;(&box->max.x)[i]+=(float)margin;}
}
#endif
