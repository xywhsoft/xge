#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
xge3d_color_settings_t xge3dColorDefault(void)
{
    xge3d_color_settings_t d={.size=sizeof(d),.saturation=1,.contrast=1,.gain={1,1,1}};
#if XGE3D_ENABLE_TONEMAP
    d.tone_mapping=XGE3D_TONEMAP_ACES;
#endif
    return d;
}
int xge3dRendererSetColor(xge3d_renderer *r,const xge3d_color_settings_t *d)
{
    if(!r)return XGE_ERROR_INVALID_ARGUMENT;
    if(r->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    if(d){
        if(d->size!=sizeof(*d)||d->tone_mapping<0||d->tone_mapping>2||!isfinite(d->saturation)||d->saturation<0||d->saturation>4||
            !isfinite(d->contrast)||d->contrast<0||d->contrast>4||!d3_finite3(d->gain)||d->gain.x<0||d->gain.y<0||d->gain.z<0||
            d->gain.x>16||d->gain.y>16||d->gain.z>16)return XGE_ERROR_INVALID_ARGUMENT;
#if !XGE3D_ENABLE_TONEMAP
        if(d->tone_mapping)return XGE_ERROR_UNSUPPORTED;
#endif
#if !XGE3D_ENABLE_COLOR_GRADING
        if(d->saturation!=1||d->contrast!=1||d->gain.x!=1||d->gain.y!=1||d->gain.z!=1)return XGE_ERROR_UNSUPPORTED;
#endif
    }
    r->composite.color_settings=d?*d:xge3dColorDefault();
    r->composite.color_active=d&&(d->tone_mapping||d->saturation!=1||d->contrast!=1||d->gain.x!=1||d->gain.y!=1||d->gain.z!=1);
    return XGE_OK;
}
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
xge3d_height_fog_settings_t xge3dHeightFogDefault(void)
{return (xge3d_height_fog_settings_t){.size=sizeof(xge3d_height_fog_settings_t),.color={.57f,.73f,.85f},.density=.02f,.falloff=.2f,.max_opacity=.95f};}
int xge3dRendererSetHeightFog(xge3d_renderer *r,const xge3d_height_fog_settings_t *d)
{
    if(!r)return XGE_ERROR_INVALID_ARGUMENT;
    if(r->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    if(d&&(d->size!=sizeof(*d)||!d3_finite3(d->color)||d->color.x<0||d->color.y<0||d->color.z<0||!isfinite(d->height)||
        !isfinite(d->density)||d->density<0||d->density>1000||!isfinite(d->falloff)||d->falloff<0||d->falloff>1000||
        !isfinite(d->max_opacity)||d->max_opacity<0||d->max_opacity>1))return XGE_ERROR_INVALID_ARGUMENT;
    r->height_fog=d?*d:xge3dHeightFogDefault();r->height_active=d&&d->density>0&&d->max_opacity>0;return XGE_OK;
}
#endif
#if XGE3D_ENABLE_BLOOM
static void d3_bloom_release(d3_bloom *b)
{
    for(int i=0;i<b->count;++i){xge_glDeleteFramebuffers(1,&b->fbos[i]);xge_glDeleteTextures(1,&b->textures[i]);b->fbos[i]=b->textures[i]=0;}
    b->count=0;b->bytes=0;b->passes=0;b->source_width=b->source_height=0;
}
xge3d_bloom_settings_t xge3dBloomDefault(void)
{return (xge3d_bloom_settings_t){.size=sizeof(xge3d_bloom_settings_t),.strength=.08f,.threshold=1,.knee=.5f,.levels=4};}
int xge3dRendererSetBloom(xge3d_renderer *r,const xge3d_bloom_settings_t *d)
{
    if(!r)return XGE_ERROR_INVALID_ARGUMENT;
    if(r->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    if(d&&(d->size!=sizeof(*d)||!isfinite(d->strength)||d->strength<0||d->strength>4||!isfinite(d->threshold)||d->threshold<0||d->threshold>64||
        !isfinite(d->knee)||d->knee<0||d->knee>64||d->levels<1||d->levels>D3_BLOOM_LEVELS))return XGE_ERROR_INVALID_ARGUMENT;
    r->composite.bloom.settings=d?*d:(xge3d_bloom_settings_t){0};
    if(!d||!d->strength)d3_bloom_release(&r->composite.bloom);
    return XGE_OK;
}
int xge3dRendererBloomInfo(const xge3d_renderer *r,xge3d_bloom_info_t *out)
{
    if(!r||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    const d3_bloom *b=&r->composite.bloom;
    *out=(xge3d_bloom_info_t){.size=sizeof(*out),.levels=(uint32_t)b->count,.passes=b->passes,.storage_bytes=b->bytes};return XGE_OK;
}
static int d3_bloom_prepare(d3_bloom *b,int width,int height)
{
    int w=(width+1)/2,h=(height+1)/2,count=1;
    for(int x=w,y=h;count<b->settings.levels&&(x>1||y>1);++count){x=(x+1)/2;y=(y+1)/2;}
    if(!b->shader.iProgram){int result=d3_program_create(&b->shader,d3_sky_vertex_shader,d3_bloom_fragment_shader);if(result)return result;
        b->image=xge_glGetUniformLocation(b->shader.iProgram,"image");b->filter=xge_glGetUniformLocation(b->shader.iProgram,"filterParams");}
    if(b->source_width==width&&b->source_height==height&&b->count==count)return XGE_OK;
    d3_bloom next={0};next.source_width=width;next.source_height=height;
    GLint unpack;xge_glGetIntegerv(0x88EF,&unpack);xge_glBindBuffer(0x88EC,0);xge_glActiveTexture(GL_TEXTURE0);int result=XGE_OK;
    for(int i=0;i<count;++i){next.width[i]=w;next.height[i]=h;++next.count;
        xge_glGenTextures(1,&next.textures[i]);xge_glBindTexture(GL_TEXTURE_2D,next.textures[i]);
        xge_glTexImage2D(GL_TEXTURE_2D,0,0x881A,w,h,0,GL_RGBA,0x140B,NULL);
        xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        xge_glGenFramebuffers(1,&next.fbos[i]);xge_glBindFramebuffer(GL_FRAMEBUFFER,next.fbos[i]);
        xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,next.textures[i],0);
        if(!next.fbos[i]||!next.textures[i]||xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||xge_glGetError()!=GL_NO_ERROR){result=XGE_ERROR_GPU_FAILED;break;}
        next.bytes+=(uint64_t)w*h*8;w=(w+1)/2;h=(h+1)/2;
    }
    xge_glBindBuffer(0x88EC,unpack);
    if(result){d3_bloom_release(&next);return result;}
    d3_bloom_release(b);memcpy(b->textures,next.textures,sizeof(b->textures));memcpy(b->fbos,next.fbos,sizeof(b->fbos));
    memcpy(b->width,next.width,sizeof(b->width));memcpy(b->height,next.height,sizeof(b->height));
    b->count=next.count;b->bytes=next.bytes;b->source_width=width;b->source_height=height;return XGE_OK;
}
static int d3_bloom_render(xge3d_renderer *r)
{
    d3_compositor *c=&r->composite;d3_bloom *b=&c->bloom;if(!b->settings.strength)return XGE_OK;
    int result=d3_bloom_prepare(b,c->width,c->height);if(result)return result;
    xge_glDisable(GL_DEPTH_TEST);xge_glDisable(GL_CULL_FACE);xge_glDisable(GL_BLEND);xge_glDepthMask(GL_FALSE);
    xge_glUseProgram(b->shader.iProgram);xge_glUniform1i(b->image,0);xge_glBindVertexArray(r->sky_vao);xge_glActiveTexture(GL_TEXTURE0);
    for(int i=0;i<b->count;++i){xge_glBindFramebuffer(D3_DRAW_FRAMEBUFFER,b->fbos[i]);xge_glViewport(0,0,b->width[i],b->height[i]);
        xge_glBindTexture(GL_TEXTURE_2D,i?b->textures[i-1]:c->color);
        xge_glUniform4f(b->filter,b->settings.threshold,b->settings.knee,0,i?0:1.f/b->count);xge_glDrawArrays(GL_TRIANGLES,0,3);++b->passes;}
    xge_glEnable(GL_BLEND);xge_glBlendFuncSeparate(GL_ONE,GL_ONE,GL_ONE,GL_ONE);xge_glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
    xge_glUniform4f(b->filter,0,0,0,0);
    for(int i=b->count-2;i>=0;--i){xge_glBindFramebuffer(D3_DRAW_FRAMEBUFFER,b->fbos[i]);xge_glViewport(0,0,b->width[i],b->height[i]);
        xge_glBindTexture(GL_TEXTURE_2D,b->textures[i+1]);xge_glDrawArrays(GL_TRIANGLES,0,3);++b->passes;}
    return xge_glGetError()==GL_NO_ERROR?XGE_OK:XGE_ERROR_GPU_FAILED;
}
#endif
