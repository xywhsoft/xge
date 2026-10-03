#include "xge3d_internal.h"
#include "xge_gl.h"
#include <limits.h>

#if XGE_ENABLE_3D
static int d3_filter(int f, int min)
{ return f==GL_NEAREST || f==GL_LINEAR || (min && f>=0x2700 && f<=0x2703); }
static int d3_wrap(int w)
{ return w==GL_CLAMP_TO_EDGE || w==GL_REPEAT || w==0x8370; }

int d3_texture_retain(xge3d_texture *t)
{
    if (!t || t->refs==UINT32_MAX) return 0;
    ++t->refs; return 1;
}
void xge3dTextureFree(xge3d_texture *t)
{
    if (!t || --t->refs) return;
    if (t->id && t->context==__xge3dContext()) glDeleteTextures(1,&t->id);
    xrtArrayUnit(&t->pixels); xrtFree(t);
}
int xge3dTextureCreate(const xge3d_texture_desc_t *d, xge3d_texture **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (!d || !d->image || (d->srgb!=0 && d->srgb!=1)) return XGE_ERROR_INVALID_ARGUMENT;
    const xge_image_t *image=d->image;
    if (image->iWidth<=0 || image->iHeight<=0 || image->iWidth>INT_MAX/4 ||
        image->iStride<image->iWidth*4 || image->iFormat!=XGE_PIXEL_RGBA8 || !image->pPixels ||
        (image->iFlags&XGE_IMAGE_PREMULTIPLIED) ||
        (size_t)image->iWidth>SIZE_MAX/(size_t)image->iHeight/4 ||
        (size_t)image->iStride>SIZE_MAX/(size_t)image->iHeight)
        return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_sampler_t sampler=d->sampler;
    if (!sampler.min_filter) sampler.min_filter=GL_LINEAR_MIPMAP_LINEAR;
    if (!sampler.mag_filter) sampler.mag_filter=GL_LINEAR;
    if (!sampler.wrap_u) sampler.wrap_u=GL_REPEAT;
    if (!sampler.wrap_v) sampler.wrap_v=GL_REPEAT;
    if (!d3_filter(sampler.min_filter,1) || !d3_filter(sampler.mag_filter,0) ||
        !d3_wrap(sampler.wrap_u) || !d3_wrap(sampler.wrap_v)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_texture *t=xrtMalloc(sizeof(*t));
    if (!t) return XGE_ERROR_OUT_OF_MEMORY;
    memset(t,0,sizeof(*t)); t->refs=1; t->width=image->iWidth; t->height=image->iHeight;
    t->srgb=d->srgb; t->sampler=sampler; xrtArrayInit(&t->pixels,1);
    if (!xrtArrayResize(&t->pixels,(size_t)t->width*t->height*4)) {
        xge3dTextureFree(t); return XGE_ERROR_OUT_OF_MEMORY;
    }
    for (int y=0;y<t->height;++y) memcpy(t->pixels.Data+(size_t)y*t->width*4,
        (const unsigned char*)image->pPixels+(size_t)y*image->iStride,(size_t)t->width*4);
    *out=t; return XGE_OK;
}
/* Called only inside a renderer's saved GL state, on its context thread. */
int d3_texture_upload(xge3d_texture *t, uint64_t *bytes)
{
    uint64_t context=__xge3dContext();
    if (!context) return XGE_ERROR_NOT_INITIALIZED;
    if (t->context && t->context!=context) return XGE_ERROR_INVALID_STATE;
    if (t->id) { glBindTexture(GL_TEXTURE_2D,t->id); return XGE_OK; }
    GLint maximum, unpack;
    glGetIntegerv(0x0D33,&maximum);
    if (t->width>maximum || t->height>maximum) return XGE_ERROR_UNSUPPORTED;
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&unpack); glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glGenTextures(1,&t->id); glBindTexture(GL_TEXTURE_2D,t->id);
    glTexImage2D(GL_TEXTURE_2D,0,t->srgb ? 0x8C43 : GL_RGBA8,t->width,t->height,0,GL_RGBA,GL_UNSIGNED_BYTE,t->pixels.Data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,t->sampler.min_filter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,t->sampler.mag_filter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,t->sampler.wrap_u);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,t->sampler.wrap_v);
    if (t->sampler.min_filter>=0x2700 && t->sampler.min_filter<=0x2703) glGenerateMipmap(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT,unpack);
    GLenum error=glGetError();
    if (!t->id || error!=GL_NO_ERROR) {
        if (t->id) glDeleteTextures(1,&t->id);
        t->id=0; return XGE_ERROR_GPU_FAILED;
    }
    t->context=context;
    if (bytes) *bytes+=t->pixels.Count;
    return XGE_OK;
}
xge3d_material_desc_t xge3dMaterialDefault(void)
{
    xge3d_material_desc_t d={0};
    for (int i=0;i<4;++i) d.base_color[i]=1;
    d.metallic=d.roughness=d.normal_scale=d.occlusion_strength=1;
    d.alpha_cutoff=.5f;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) d.maps[i].scale[0]=d.maps[i].scale[1]=1;
    return d;
}
int d3_material_retain(xge3d_material *m)
{
    if (!m || m->refs==UINT32_MAX) return 0;
    ++m->refs; return 1;
}
void xge3dMaterialFree(xge3d_material *m)
{
    if (!m || --m->refs) return;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) xge3dTextureFree(m->desc.maps[i].texture);
    xrtFree(m);
}
int xge3dMaterialCreate(const xge3d_material_desc_t *d, xge3d_material **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (!d || !isfinite(d->metallic) || d->metallic<0 || d->metallic>1 ||
        !isfinite(d->roughness) || d->roughness<0 || d->roughness>1 ||
        !isfinite(d->normal_scale) || !isfinite(d->occlusion_strength) ||
        d->occlusion_strength<0 || d->occlusion_strength>1 || !isfinite(d->alpha_cutoff) ||
        d->alpha_cutoff<0 || d->alpha_cutoff>1 || d->alpha_mode<0 || d->alpha_mode>2)
        return XGE_ERROR_INVALID_ARGUMENT;
    for (int i=0;i<4;++i) if (!isfinite(d->base_color[i]) || d->base_color[i]<0 || d->base_color[i]>1)
        return XGE_ERROR_INVALID_ARGUMENT;
    for (int i=0;i<3;++i) if (!isfinite(d->emissive[i]) || d->emissive[i]<0) return XGE_ERROR_INVALID_ARGUMENT;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {
        const xge3d_texture_binding_t *b=&d->maps[i];
        if (b->texcoord<0 || b->texcoord>1 || !isfinite(b->rotation)) return XGE_ERROR_INVALID_ARGUMENT;
        for (int j=0;j<2;++j) if (!isfinite(b->offset[j]) || !isfinite(b->scale[j]))
            return XGE_ERROR_INVALID_ARGUMENT;
    }
    xge3d_material *m=xrtMalloc(sizeof(*m));
    if (!m) return XGE_ERROR_OUT_OF_MEMORY;
    m->refs=1; m->desc=*d;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) m->desc.maps[i].texture=NULL;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {
        xge3d_texture *t=d->maps[i].texture;
        if (t && !d3_texture_retain(t)) { xge3dMaterialFree(m); return XGE_ERROR_INVALID_STATE; }
        m->desc.maps[i].texture=t;
    }
    *out=m; return XGE_OK;
}
int xge3dMaterialGetDesc(const xge3d_material *m, xge3d_material_desc_t *out)
{
    if (!m || !out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=m->desc; return XGE_OK;
}
#endif
