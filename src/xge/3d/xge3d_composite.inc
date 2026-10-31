/* Linear associated RGBA16F scene, display-associated RGBA8 public output.
 * Shared target depth avoids extra copies; window/current targets use an exact
 * depth-format copy. No geometry or readback is routed through the CPU. */
static void d3_composite_free(d3_compositor *c)
{
    if(c->fbo)xge_glDeleteFramebuffers(1,&c->fbo);
    if(c->depth)xge_glDeleteRenderbuffers(1,&c->depth);
    if(c->color)xge_glDeleteTextures(1,&c->color);
    if(c->input)xge_glDeleteTextures(1,&c->input);
    c->fbo=c->depth=c->color=c->input=0;c->width=c->height=0;
}
static int d3_composite_resize(d3_compositor *c,int width,int height,GLenum format)
{
    if(c->fbo&&c->width==width&&c->height==height&&c->depth_format==format)return XGE_OK;
    GLint limit,unpack;xge_glGetIntegerv(0x0D33,&limit);
    if(width<=0||height<=0)return XGE_ERROR_INVALID_ARGUMENT;
    if(width>limit||height>limit)return XGE_ERROR_UNSUPPORTED;
    d3_compositor next={0};next.width=width;next.height=height;next.depth_format=format;
    xge_glGetIntegerv(0x88EF,&unpack);xge_glBindBuffer(0x88EC,0);xge_glActiveTexture(GL_TEXTURE0);
    xge_glGenTextures(1,&next.color);xge_glBindTexture(GL_TEXTURE_2D,next.color);
    xge_glTexImage2D(GL_TEXTURE_2D,0,0x881A,width,height,0,GL_RGBA,0x140B,NULL);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    xge_glBindBuffer(0x88EC,unpack);
    xge_glGenRenderbuffers(1,&next.depth);xge_glBindRenderbuffer(GL_RENDERBUFFER,next.depth);
    xge_glRenderbufferStorage(GL_RENDERBUFFER,format,width,height);
    xge_glGenFramebuffers(1,&next.fbo);xge_glBindFramebuffer(GL_FRAMEBUFFER,next.fbo);
    xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,next.color,0);
    xge_glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,next.depth);
    int result=!next.color||!next.depth||!next.fbo||xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||xge_glGetError()!=GL_NO_ERROR?XGE_ERROR_GPU_FAILED:XGE_OK;
    if(result){d3_composite_free(&next);return result;}
    d3_composite_free(c);c->fbo=next.fbo;c->color=next.color;c->input=next.input;c->depth=next.depth;
    c->width=width;c->height=height;c->depth_format=format;return XGE_OK;
}
static int d3_composite_input(d3_compositor *c)
{
    if(c->input)return XGE_OK;
    GLint unpack;xge_glGetIntegerv(0x88EF,&unpack);xge_glBindBuffer(0x88EC,0);xge_glActiveTexture(GL_TEXTURE0);
    xge_glGenTextures(1,&c->input);xge_glBindTexture(GL_TEXTURE_2D,c->input);
    xge_glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,c->width,c->height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    xge_glBindBuffer(0x88EC,unpack);
    if(c->input&&xge_glGetError()==GL_NO_ERROR)return XGE_OK;
    if(c->input){xge_glDeleteTextures(1,&c->input);}c->input=0;return XGE_ERROR_GPU_FAILED;
}
static void d3_composite_draw(xge3d_renderer *r,GLuint texture,int importing)
{
    d3_compositor *c=&r->composite;xge_glDisable(GL_DEPTH_TEST);xge_glDisable(GL_CULL_FACE);xge_glDisable(GL_BLEND);
    xge_glDepthMask(GL_FALSE);xge_glUseProgram(c->shader.iProgram);xge_glBindVertexArray(r->sky_vao);
    xge_glActiveTexture(GL_TEXTURE0);xge_glBindTexture(GL_TEXTURE_2D,texture);xge_glUniform1i(c->image,0);xge_glUniform1i(c->importing,importing);
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    xge_glUniform1i(c->color_enabled,!importing&&c->color_active);
#endif
#if XGE3D_ENABLE_TONEMAP
    xge_glUniform1i(c->color_tone,c->color_settings.tone_mapping);
#endif
#if XGE3D_ENABLE_COLOR_GRADING
    xge_glUniform3f(c->color_gain,c->color_settings.gain.x,c->color_settings.gain.y,c->color_settings.gain.z);
    xge_glUniform2f(c->color_grade,c->color_settings.saturation,c->color_settings.contrast);
#endif
#if XGE3D_ENABLE_BLOOM
    xge_glActiveTexture(GL_TEXTURE0+1);xge_glBindTexture(GL_TEXTURE_2D,!importing&&c->bloom.count?c->bloom.textures[0]:texture);
    xge_glUniform1i(c->bloom_image,1);xge_glUniform1f(c->bloom_strength,importing?0:c->bloom.settings.strength);xge_glActiveTexture(GL_TEXTURE0);
#endif
#if XGE3D_ENABLE_FXAA
    xge_glUniform1i(c->antialiasing,c->mode);
#endif
    xge_glDrawArrays(GL_TRIANGLES,0,3);
}
static int d3_composite_begin(xge3d_renderer *r,const xge3d_render_desc_t *d,d3_output *out,const d3_state *state)
{
    xge_glGetIntegerv(0x8CA6,&out->fbo);xge_glGetIntegerv(0x0BA2,out->viewport);
    GLint samples=0,bits=0,type=0,stencil_type=0,stencil=0,encoding=0,draw_buffer=0,other=0;
    xge_glGetIntegerv(0x80A9,&samples);if(samples)return XGE_ERROR_UNSUPPORTED;
    GLenum attachment=out->fbo?GL_DEPTH_ATTACHMENT:0x1801;
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8216,&bits);
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8211,&type);
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8217,&stencil);
    attachment=out->fbo?0x8D20:0x1802;
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8CD0,&stencil_type);
    if(stencil_type!=GL_NONE)xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8217,&stencil);
    xge_glGetIntegerv(0x8825,&draw_buffer);xge_glGetIntegerv(0x8826,&other);
    if(draw_buffer==GL_NONE||other!=GL_NONE)return XGE_ERROR_UNSUPPORTED;
    attachment=out->fbo?(GLenum)draw_buffer:draw_buffer==0x0404?0x0400:0x0402;
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8210,&encoding);
    out->srgb=encoding==0x8C40;GLint color_type;
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8211,&color_type);
    if(color_type==GL_INT||color_type==GL_UNSIGNED_INT)return XGE_ERROR_UNSUPPORTED;
    GLenum format=type==GL_FLOAT?(stencil?0x8CAD:0x8CAC):stencil?0x88F0:bits==16?0x81A5:bits==24?D3_DEPTH_COMPONENT24:0x81A7;
    if(out->viewport[0]<0||out->viewport[1]<0||out->viewport[2]<=0||out->viewport[3]<=0||
       out->viewport[0]>INT_MAX-out->viewport[2]||out->viewport[1]>INT_MAX-out->viewport[3])return XGE_ERROR_INVALID_ARGUMENT;
    int result=d3_composite_resize(&r->composite,out->viewport[2],out->viewport[3],format);
    if(result){return result;}
    d3_compositor *c=&r->composite;out->shared_depth=d->target!=NULL;
    xge_glBindFramebuffer(GL_FRAMEBUFFER,c->fbo);
    xge_glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,d->target?d->target->depth:c->depth);
    xge_glViewport(0,0,c->width,c->height);xge_glDisable(GL_SCISSOR_TEST);xge_glDisable(GL_STENCIL_TEST);
    xge_glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);if(state->srgb>=0)xge_glDisable(0x8DB9);
    xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,out->fbo);xge_glGetIntegerv(0x0C02,&out->read_buffer);xge_glReadBuffer(draw_buffer);
    if(!(d->clear_flags&XGE3D_CLEAR_DEPTH)&&!out->shared_depth)
        xge_glBlitFramebuffer(out->viewport[0],out->viewport[1],out->viewport[0]+c->width,out->viewport[1]+c->height,0,0,c->width,c->height,GL_DEPTH_BUFFER_BIT,GL_NEAREST);
    if(!(d->clear_flags&XGE3D_CLEAR_COLOR)){
        result=d3_composite_input(c);
        if(result){xge_glReadBuffer(out->read_buffer);return result;}
        xge_glActiveTexture(GL_TEXTURE0);xge_glBindTexture(GL_TEXTURE_2D,c->input);
        xge_glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,out->viewport[0],out->viewport[1],c->width,c->height);
        d3_composite_draw(r,c->input,1);
    }
    xge_glReadBuffer(out->read_buffer);xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,c->fbo);
    return xge_glGetError()==GL_NO_ERROR?XGE_OK:XGE_ERROR_GPU_FAILED;
}
static int d3_composite_end(xge3d_renderer *r,const d3_output *out)
{
    d3_compositor *c=&r->composite;
#if XGE3D_ENABLE_BLOOM
    int result=d3_bloom_render(r);if(result)return result;
#endif
    xge_glBindFramebuffer(D3_DRAW_FRAMEBUFFER,out->fbo);xge_glViewport(out->viewport[0],out->viewport[1],c->width,c->height);
    if(out->srgb){xge_graphics_mapping_t mapping;
        if(xgeGraphicsMappingGet(XGE_GPU_BACKEND_NONE,&mapping)!=XGE_OK||!mapping.bOpenGLCore)return XGE_ERROR_UNSUPPORTED;
        xge_glDisable(0x8DB9);}
    d3_composite_draw(r,c->color,0);
    if(!out->shared_depth){xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,c->fbo);
        xge_glBlitFramebuffer(0,0,c->width,c->height,out->viewport[0],out->viewport[1],out->viewport[0]+c->width,out->viewport[1]+c->height,GL_DEPTH_BUFFER_BIT,GL_NEAREST);}
    return xge_glGetError()==GL_NO_ERROR?XGE_OK:XGE_ERROR_GPU_FAILED;
}
static void d3_composite_detach(xge3d_renderer *r,const d3_output *out)
{
    if(out->shared_depth){xge_glBindFramebuffer(D3_DRAW_FRAMEBUFFER,r->composite.fbo);
        xge_glFramebufferRenderbuffer(D3_DRAW_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,r->composite.depth);}
}
static float d3_decode_clear(float value,float alpha)
{
    if(alpha<=0){return 0;}float v=fminf(1,fmaxf(0,value/alpha));
    return (v<=.04045f?v/12.92f:powf((v+.055f)/1.055f,2.4f))*alpha;
}
#if XGE3D_ENABLE_FXAA
int xge3dRendererSetAntialiasing(xge3d_renderer *r,int mode)
{
    if(!r||(mode!=XGE3D_AA_NONE&&mode!=XGE3D_AA_FXAA))return XGE_ERROR_INVALID_ARGUMENT;
    if(r->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
    r->composite.mode=mode;return XGE_OK;
}
#endif
