#include "xge3d_internal.h"
#include "xge_gl.h"
#include <limits.h>

#if XGE_ENABLE_3D
void d3_cube_free(d3_cube *c)
{
    if (c->id && c->context==__xge3dContext()) glDeleteTextures(1,&c->id);
    xrtArrayUnit(&c->pixels); memset(c,0,sizeof(*c));
}
int d3_cube_copy(d3_cube *c,const xge3d_cube_level_t *levels,size_t count,int srgb)
{
    if (!levels || !count || count>16 || !levels[0].faces[0] || (srgb!=0 && srgb!=1)) return XGE_ERROR_INVALID_ARGUMENT;
    int size=levels[0].faces[0]->iWidth;
    if (size<=0 || size>INT_MAX/4) return XGE_ERROR_INVALID_ARGUMENT;
    size_t bytes=0; int mip=size;
    for (size_t l=0;l<count;++l) {
        if ((size_t)mip>SIZE_MAX/(size_t)mip/24 || (size_t)mip*mip*24>SIZE_MAX-bytes) return XGE_ERROR_INVALID_ARGUMENT;
        bytes+=(size_t)mip*mip*24;
        for (int f=0;f<6;++f) {
            const xge_image_t *i=levels[l].faces[f];
            if (!i || i->iWidth!=mip || i->iHeight!=mip || i->iStride<mip*4 || i->iFormat!=XGE_PIXEL_RGBA8 ||
                !i->pPixels || (i->iFlags&XGE_IMAGE_PREMULTIPLIED) || (size_t)i->iStride>SIZE_MAX/(size_t)mip)
                return XGE_ERROR_INVALID_ARGUMENT;
        }
        if (l+1<count && mip==1) return XGE_ERROR_INVALID_ARGUMENT;
        mip=mip>1 ? mip/2 : 1;
    }
    xrtArrayInit(&c->pixels,1);
    if (!xrtArrayResize(&c->pixels,bytes)) { d3_cube_free(c);return XGE_ERROR_OUT_OF_MEMORY; }
    c->size=size;c->levels=(int)count;c->srgb=srgb;mip=size;size_t offset=0;
    for (size_t l=0;l<count;++l) {
        for (int f=0;f<6;++f) for (int y=0;y<mip;++y)
            memcpy(c->pixels.Data+offset+((size_t)f*mip+y)*mip*4,
                (const unsigned char*)levels[l].faces[f]->pPixels+(size_t)y*levels[l].faces[f]->iStride,(size_t)mip*4);
        offset+=(size_t)mip*mip*24;mip=mip>1 ? mip/2 : 1;
    }
    return XGE_OK;
}
void xge3dEnvironmentFree(xge3d_environment *e)
{
    if (!e) return;
    d3_cube_free(&e->sky);
#if XGE3D_ENABLE_IBL
    d3_cube_free(&e->irradiance);d3_cube_free(&e->prefiltered);xge3dTextureFree(e->brdf);
#endif
    xrtFree(e);
}
int xge3dEnvironmentCreate(const xge3d_environment_desc_t *d,xge3d_environment **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if (!d) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_environment *e=xrtMalloc(sizeof(*e));if (!e) return XGE_ERROR_OUT_OF_MEMORY;
    memset(e,0,sizeof(*e));xge3d_cube_level_t level;memcpy(level.faces,d->faces,sizeof(level.faces));
    int result=d3_cube_copy(&e->sky,&level,1,d->srgb);
    if (result!=XGE_OK) { xge3dEnvironmentFree(e);return result; }
    *out=e;return XGE_OK;
}
int d3_cube_upload(d3_cube *e, uint64_t *bytes)
{
    uint64_t context=__xge3dContext();
    if (!context) return XGE_ERROR_NOT_INITIALIZED;
    if (e->context && e->context!=context) return XGE_ERROR_INVALID_STATE;
    if (e->id) { glBindTexture(0x8513,e->id); return XGE_OK; }
    GLint maximum,unpack;
    glGetIntegerv(0x851C,&maximum);
    if (e->size>maximum) return XGE_ERROR_UNSUPPORTED;
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&unpack); glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glGenTextures(1,&e->id); glBindTexture(0x8513,e->id);
    int size=e->size;size_t offset=0;
    for (int l=0;l<e->levels;++l) {
        for (int f=0;f<6;++f) glTexImage2D(0x8515+f,l,e->srgb ? 0x8C43 : GL_RGBA8,
            size,size,0,GL_RGBA,GL_UNSIGNED_BYTE,e->pixels.Data+offset+(size_t)f*size*size*4);
        offset+=(size_t)size*size*24;size=size>1 ? size/2 : 1;
    }
    glTexParameteri(0x8513,0x813D,e->levels-1);
    glTexParameteri(0x8513,GL_TEXTURE_MIN_FILTER,e->levels>1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(0x8513,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(0x8513,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(0x8513,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(0x8513,0x8072,GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT,unpack);
    if (!e->id || glGetError()!=GL_NO_ERROR) {
        if (e->id) glDeleteTextures(1,&e->id);
        e->id=0; return XGE_ERROR_GPU_FAILED;
    }
    e->context=context;
    if (bytes) *bytes+=e->pixels.Count;
    return XGE_OK;
}
int d3_environment_upload(xge3d_environment *e,uint64_t *bytes)
{ return d3_cube_upload(&e->sky,bytes); }
#endif
