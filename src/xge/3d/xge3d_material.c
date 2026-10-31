#include "xge3d_internal.h"
#include "xge_gl.h"
#include <limits.h>

#if XGE_ENABLE_3D
int xge3dTextureResourceInfo(const xge3d_texture *texture,xge3d_resource_info_t *out)
{
    if(!texture||!out||out->size!=sizeof(*out)){return XGE_ERROR_INVALID_ARGUMENT;}
    *out=(xge3d_resource_info_t){.size=sizeof(*out),.references=texture->refs,.textures=1};
#if XGE3D_ENABLE_DYNAMIC
    if(texture->target){return XGE_OK;} /* View owns no separate pixel storage. */
#endif
    out->cpu_payload_bytes=(uint64_t)texture->width*texture->height*4;
    if(texture->id){int width=texture->width,height=texture->height;out->gpu_payload_bytes=out->cpu_payload_bytes;
        if(!texture->prepare_partial&&texture->sampler.min_filter>=0x2700){while(width>1||height>1){width=width>1?width/2:1;height=height>1?height/2:1;out->gpu_payload_bytes+=(uint64_t)width*height*4;}}}
    return XGE_OK;
}
int xge3dTextureAddRef(xge3d_texture *texture)
{return !texture?XGE_ERROR_INVALID_ARGUMENT:d3_texture_retain(texture)?XGE_OK:XGE_ERROR_INVALID_STATE;}
int xge3dMaterialAddRef(xge3d_material *material)
{return !material?XGE_ERROR_INVALID_ARGUMENT:d3_material_retain(material)?XGE_OK:XGE_ERROR_INVALID_STATE;}
static int d3_filter(int f, int min)
{ return f==GL_NEAREST || f==GL_LINEAR || (min && f>=0x2700 && f<=0x2703); }
static int d3_wrap(int w)
{ return w==GL_CLAMP_TO_EDGE || w==GL_REPEAT || w==0x8370; }

int d3_texture_retain(xge3d_texture *t)
{
    if (!t || t->refs==UINT32_MAX) {return 0;}
    ++t->refs; return 1;
}
void xge3dTextureFree(xge3d_texture *t)
{
    if (!t || --t->refs) {return;}
#if XGE3D_ENABLE_DYNAMIC
    if(t->target){xge3dTargetFree(t->target);xrtFree(t);return;}
#endif
    if (t->id && t->context==__xge3dContext()) {xge_glDeleteTextures(1,&t->id);}
    xrtArrayUnit(&t->pixels); xrtFree(t);
}
int d3_texture_detach_gpu(xge3d_texture *t)
{
    if(!t||t->refs!=1){return XGE_ERROR_INVALID_STATE;}
    if(t->id){if(!__xge3dContext()||t->context!=__xge3dContext()){return XGE_ERROR_INVALID_STATE;}xge_glDeleteTextures(1,&t->id);}
    t->id=0;t->context=0;t->prepare_offset=0;t->prepare_partial=0;return XGE_OK;
}
int xge3dTextureCreate(const xge3d_texture_desc_t *d, xge3d_texture **out)
{
    if (!out) {return XGE_ERROR_INVALID_ARGUMENT;}
    *out=NULL;
    if (!d || !d->image || (d->srgb!=0 && d->srgb!=1)) {return XGE_ERROR_INVALID_ARGUMENT;}
    const xge_image_t *image=d->image;
    if (image->iWidth<=0 || image->iHeight<=0 || image->iWidth>INT_MAX/4 ||
        image->iStride<image->iWidth*4 || image->iFormat!=XGE_PIXEL_RGBA8 || !image->pPixels ||
        (image->iFlags&XGE_IMAGE_PREMULTIPLIED) ||
        (size_t)image->iWidth>SIZE_MAX/(size_t)image->iHeight/4 ||
        (size_t)image->iStride>SIZE_MAX/(size_t)image->iHeight)
        {return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_sampler_t sampler=d->sampler;
    if (!sampler.min_filter) {sampler.min_filter=GL_LINEAR_MIPMAP_LINEAR;}
    if (!sampler.mag_filter) {sampler.mag_filter=GL_LINEAR;}
    if (!sampler.wrap_u) {sampler.wrap_u=GL_REPEAT;}
    if (!sampler.wrap_v) {sampler.wrap_v=GL_REPEAT;}
    if (!d3_filter(sampler.min_filter,1) || !d3_filter(sampler.mag_filter,0) ||
        !d3_wrap(sampler.wrap_u) || !d3_wrap(sampler.wrap_v)) {return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_texture *t=xrtMalloc(sizeof(*t));
    if (!t) {return XGE_ERROR_OUT_OF_MEMORY;}
    memset(t,0,sizeof(*t)); t->refs=1; t->width=image->iWidth; t->height=image->iHeight;
    t->srgb=d->srgb; t->sampler=sampler; xrtArrayInit(&t->pixels,1);
    if (!xrtArrayResize(&t->pixels,(size_t)t->width*t->height*4)) {
        xge3dTextureFree(t); return XGE_ERROR_OUT_OF_MEMORY;
    }
    for (int y=0;y<t->height;++y) {memcpy(t->pixels.Data+(size_t)y*t->width*4,
        (const unsigned char*)image->pPixels+(size_t)y*image->iStride,(size_t)t->width*4);}
    *out=t; return XGE_OK;
}
/* Called only inside a renderer's saved GL state, on its context thread. */
int d3_texture_upload(xge3d_texture *t, uint64_t *bytes)
{
#if XGE3D_ENABLE_DYNAMIC
    if(t->target){
        if(t->target->context!=__xge3dContext()){return XGE_ERROR_INVALID_STATE;}
        const xge_texture_t *source=&t->target->texture;t->id=source->iBackendId;t->context=t->target->context;
        t->width=source->iWidth;t->height=source->iHeight;xge_glBindTexture(GL_TEXTURE_2D,t->id);return XGE_OK;
    }
#endif
    uint64_t context=__xge3dContext();
    if (!context) {return XGE_ERROR_NOT_INITIALIZED;}
    if (t->context && t->context!=context) {return XGE_ERROR_INVALID_STATE;}
    if (t->prepare_partial) {return XGE_ERROR_INVALID_STATE;}
    if (t->id) { xge_glBindTexture(GL_TEXTURE_2D,t->id); return XGE_OK; }
    GLint maximum, unpack;
    xge_glGetIntegerv(0x0D33,&maximum);
    if (t->width>maximum || t->height>maximum) {return XGE_ERROR_UNSUPPORTED;}
    xge_glGetIntegerv(GL_UNPACK_ALIGNMENT,&unpack); xge_glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    xge_glGenTextures(1,&t->id); xge_glBindTexture(GL_TEXTURE_2D,t->id);
    xge_glTexImage2D(GL_TEXTURE_2D,0,t->srgb ? 0x8C43 : GL_RGBA8,t->width,t->height,0,GL_RGBA,GL_UNSIGNED_BYTE,t->pixels.Data);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,t->sampler.min_filter);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,t->sampler.mag_filter);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,t->sampler.wrap_u);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,t->sampler.wrap_v);
    if (t->sampler.min_filter>=0x2700 && t->sampler.min_filter<=0x2703) {xge_glGenerateMipmap(GL_TEXTURE_2D);}
    xge_glPixelStorei(GL_UNPACK_ALIGNMENT,unpack);
    GLenum error=xge_glGetError();
    if (!t->id || error!=GL_NO_ERROR) {
        if (t->id) {xge_glDeleteTextures(1,&t->id);}
        t->id=0; return XGE_ERROR_GPU_FAILED;
    }
    t->context=context;
    if (bytes) {*bytes+=t->pixels.Count;}
    return XGE_OK;
}
xge3d_material_desc_t xge3dMaterialDefault(void)
{
    xge3d_material_desc_t d={0};
    for (int i=0;i<4;++i) {d.base_color[i]=1;}
    d.metallic=d.roughness=d.normal_scale=d.occlusion_strength=1;
    d.alpha_cutoff=.5f;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {d.maps[i].scale[0]=d.maps[i].scale[1]=1;}
    return d;
}
int d3_material_retain(xge3d_material *m)
{
    if (!m || m->refs==UINT32_MAX) {return 0;}
    ++m->refs; return 1;
}
void xge3dMaterialFree(xge3d_material *m)
{
    if (!m || --m->refs) {return;}
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {xge3dTextureFree(m->desc.maps[i].texture);}
#if XGE3D_ENABLE_SHADER
    xge3dShaderFree(m->shader);
#endif
    xrtFree(m);
}
static int d3_material_validate(const xge3d_material_desc_t *d)
{
    if (!d || !isfinite(d->metallic) || d->metallic<0 || d->metallic>1 ||
        !isfinite(d->roughness) || d->roughness<0 || d->roughness>1 ||
        !isfinite(d->normal_scale) || !isfinite(d->occlusion_strength) ||
        d->occlusion_strength<0 || d->occlusion_strength>1 || !isfinite(d->alpha_cutoff) ||
        d->alpha_cutoff<0 || d->alpha_cutoff>1 || d->alpha_mode<0 || d->alpha_mode>2)
        {return XGE_ERROR_INVALID_ARGUMENT;}
    for (int i=0;i<4;++i) {if (!isfinite(d->base_color[i]) || d->base_color[i]<0 || d->base_color[i]>1)
        {return XGE_ERROR_INVALID_ARGUMENT;}}
    for (int i=0;i<3;++i) {if (!isfinite(d->emissive[i]) || d->emissive[i]<0) {return XGE_ERROR_INVALID_ARGUMENT;}}
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {
        const xge3d_texture_binding_t *b=&d->maps[i];
#if XGE3D_ENABLE_DYNAMIC
        if(b->texture&&b->texture->target&&i!=XGE3D_MAP_BASE_COLOR&&i!=XGE3D_MAP_EMISSIVE){return XGE_ERROR_UNSUPPORTED;}
#endif
        if (b->texcoord<0 || b->texcoord>1 || !isfinite(b->rotation)) {return XGE_ERROR_INVALID_ARGUMENT;}
        for (int j=0;j<2;++j) {if (!isfinite(b->offset[j]) || !isfinite(b->scale[j]))
            {return XGE_ERROR_INVALID_ARGUMENT;}}
    }
    return XGE_OK;
}
int xge3dMaterialCreate(const xge3d_material_desc_t *d, xge3d_material **out)
{
    if (!out) {return XGE_ERROR_INVALID_ARGUMENT;}
    *out=NULL;
    int result=d3_material_validate(d);if(result){return result;}
    xge3d_material *m=xrtMalloc(sizeof(*m));
    if (!m) {return XGE_ERROR_OUT_OF_MEMORY;}
    memset(m,0,sizeof(*m));m->refs=1; m->desc=*d;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {m->desc.maps[i].texture=NULL;}
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {
        xge3d_texture *t=d->maps[i].texture;
        if (t && !d3_texture_retain(t)) { xge3dMaterialFree(m); return XGE_ERROR_INVALID_STATE; }
        m->desc.maps[i].texture=t;
    }
    *out=m; return XGE_OK;
}
int xge3dMaterialGetDesc(const xge3d_material *m, xge3d_material_desc_t *out)
{
    if (!m || !out) {return XGE_ERROR_INVALID_ARGUMENT;}
    *out=m->desc; return XGE_OK;
}
#if XGE3D_ENABLE_DYNAMIC
int xge3dMaterialSetDesc(xge3d_material *m,const xge3d_material_desc_t *d)
{
    if(!m){return XGE_ERROR_INVALID_ARGUMENT;}int result=d3_material_validate(d);if(result){return result;}
    int i=0;for(;i<XGE3D_MAP_COUNT;++i){if(d->maps[i].texture&&!d3_texture_retain(d->maps[i].texture)){break;}}
    if(i!=XGE3D_MAP_COUNT){while(i--){xge3dTextureFree(d->maps[i].texture);}return XGE_ERROR_INVALID_STATE;}
    xge3d_material_desc_t old=m->desc;m->desc=*d;
    for(i=0;i<XGE3D_MAP_COUNT;++i){xge3dTextureFree(old.maps[i].texture);}return XGE_OK;
}
int xge3dTextureFromTarget(xge3d_target *target,xge3d_texture **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;
    if(!target){return XGE_ERROR_INVALID_ARGUMENT;}if(target->context!=__xge3dContext()){return XGE_ERROR_INVALID_STATE;}
    xge3d_texture *t=xrtCalloc(1,sizeof(*t));if(!t){return XGE_ERROR_OUT_OF_MEMORY;}
    if(!d3_target_retain(target)){xrtFree(t);return XGE_ERROR_INVALID_STATE;}
    t->target=target;t->refs=1;t->srgb=1;t->width=target->texture.iWidth;t->height=target->texture.iHeight;
    t->sampler=(xge3d_sampler_t){GL_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};*out=t;return XGE_OK;
}
int xge3dTextureUpdate(xge3d_texture *t,int x,int y,int width,int height,const void *pixels,int stride)
{
    if(!t||!pixels||x<0||y<0||width<=0||height<=0||x>t->width-width||y>t->height-height||
        width>INT_MAX/4||stride<width*4||(size_t)stride>SIZE_MAX/(size_t)height){return XGE_ERROR_INVALID_ARGUMENT;}
    if(t->target){return XGE_ERROR_UNSUPPORTED;}
    if(t->prepare_partial){return XGE_ERROR_INVALID_STATE;}
    if(t->id){
        if(t->context!=__xge3dContext()){return XGE_ERROR_INVALID_STATE;}
        GLint binding,alignment,row,skip_x,skip_y,pbo;
        xge_glGetIntegerv(0x8069,&binding);xge_glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);
        xge_glGetIntegerv(0x0CF2,&row);xge_glGetIntegerv(0x0CF4,&skip_x);xge_glGetIntegerv(0x0CF3,&skip_y);xge_glGetIntegerv(0x88EF,&pbo);
        xge_glBindBuffer(0x88EC,0);xge_glPixelStorei(GL_UNPACK_ALIGNMENT,1);xge_glPixelStorei(0x0CF2,0);xge_glPixelStorei(0x0CF4,0);xge_glPixelStorei(0x0CF3,0);
        xge_glBindTexture(GL_TEXTURE_2D,t->id);
        for(int r=0;r<height;++r){xge_glTexSubImage2D(GL_TEXTURE_2D,0,x,y+r,width,1,GL_RGBA,GL_UNSIGNED_BYTE,(const unsigned char*)pixels+(size_t)r*stride);}
        if(t->sampler.min_filter>=0x2700){xge_glGenerateMipmap(GL_TEXTURE_2D);}
        int result=xge_glGetError()==GL_NO_ERROR?XGE_OK:XGE_ERROR_GPU_FAILED;
        xge_glBindTexture(GL_TEXTURE_2D,binding);xge_glBindBuffer(0x88EC,pbo);xge_glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);
        xge_glPixelStorei(0x0CF2,row);xge_glPixelStorei(0x0CF4,skip_x);xge_glPixelStorei(0x0CF3,skip_y);if(result){return result;}
    }
    for(int r=0;r<height;++r){memcpy((unsigned char*)t->pixels.Data+((size_t)(y+r)*t->width+x)*4,(const unsigned char*)pixels+(size_t)r*stride,(size_t)width*4);}
    return XGE_OK;
}
#endif
#endif
