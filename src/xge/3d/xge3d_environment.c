#include "xge3d_internal.h"
#include "xge_gl.h"
#include "xge_graphics_internal.h"
#include <limits.h>

#if XGE_ENABLE_3D
int d3_packed_image(xge_image_t *image,int width,int height,const void *data,size_t bytes)
{
    if(!data || width<=0 || width>INT_MAX/4 || height<=0 ||
        (size_t)height>SIZE_MAX/(size_t)(width*4) ||
        bytes!=(size_t)height*(size_t)(width*4))
        return XGE_ERROR_INVALID_ARGUMENT;
    *image=(xge_image_t){.iWidth=width,.iHeight=height,.iStride=width*4,
        .iFormat=XGE_PIXEL_RGBA8,.pPixels=(void*)data};
    return XGE_OK;
}

int d3_packed_cube(xge3d_cube_level_t *levels,xge_image_t (*images)[6],size_t count,
    int edge,const void *data,size_t bytes)
{
    if(!data || !count || count>16 || edge<=0 || edge>INT_MAX/4)
        return XGE_ERROR_INVALID_ARGUMENT;
    const unsigned char *cursor=data;
    for(size_t level=0;level<count;++level){
        if((size_t)edge>bytes/(size_t)edge/24 || (edge==1 && level+1<count))
            return XGE_ERROR_INVALID_ARGUMENT;
        size_t face_bytes=(size_t)edge*(size_t)edge*4;
        for(int face=0;face<6;++face){
            xge_image_t *image=&images[level][face];
            *image=(xge_image_t){.iWidth=edge,.iHeight=edge,.iStride=edge*4,
                .iFormat=XGE_PIXEL_RGBA8,.pPixels=(void*)cursor};
            levels[level].faces[face]=image;
            cursor+=face_bytes;
        }
        bytes-=face_bytes*6;
        edge=edge>1?edge/2:1;
    }
    return bytes?XGE_ERROR_INVALID_ARGUMENT:XGE_OK;
}

static const GLenum d3_unpack_keys[4]={GL_UNPACK_ALIGNMENT,0x0CF2,0x0CF3,0x0CF4};
void d3_unpack_begin(int state[5])
{
    for(int i=0;i<4;++i)xge_glGetIntegerv(d3_unpack_keys[i],&state[i]);
    xge_glGetIntegerv(0x88EF,&state[4]); /* PIXEL_UNPACK_BUFFER_BINDING */
    xge_glBindBuffer(0x88EC,0);
    for(int i=0;i<4;++i)xge_glPixelStorei(d3_unpack_keys[i],i==0?1:0);
}
void d3_unpack_end(const int state[5])
{
    for(int i=0;i<4;++i)xge_glPixelStorei(d3_unpack_keys[i],state[i]);
    xge_glBindBuffer(0x88EC,state[4]);
}

void d3_cube_free(d3_cube *c)
{
    if (c->id && c->context==__xge3dContext()) xge_glDeleteTextures(1,&c->id);
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
static bool d3_environment_context_matches(const xge3d_environment *e,uint64_t context)
{
    if(e->sky.id && e->sky.context!=context)return false;
#if XGE3D_ENABLE_IBL
    if((e->irradiance.id && e->irradiance.context!=context) ||
        (e->prefiltered.id && e->prefiltered.context!=context) ||
        (e->brdf && e->brdf->id && e->brdf->context!=context))return false;
#endif
    return true;
}
static int d3_environment_release(xge3d_environment *e,bool *consumed)
{
    if(!e)return XGE_OK;
    if(!d3_environment_thread(e))return XGE_ERROR_INVALID_STATE;
    bool managed=e->owner_thread!=0;
    uint64_t current=managed && e->refs==1?__xge3dContext():0;
    if(current && !d3_environment_context_matches(e,current))return XGE_ERROR_INVALID_STATE;
    if(consumed)*consumed=true;
    if(--e->refs)return XGE_OK;
    d3_cube_free(&e->sky);
#if XGE3D_ENABLE_IBL
    d3_cube_free(&e->irradiance);d3_cube_free(&e->prefiltered);xge3dTextureFree(e->brdf);
#endif
    xrtFree(e);
    return managed?xgeInternalGraphicsOwnerRelease():XGE_OK;
}
void xge3dEnvironmentFree(xge3d_environment *e){(void)d3_environment_release(e,NULL);}

int xge3dEnvironmentReleaseOwnerChecked(xge3d_environment *e,bool *consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    return e&&e->owner_thread?d3_environment_release(e,consumed):XGE_ERROR_INVALID_ARGUMENT;
}
int xge3dEnvironmentReleaseChecked(xge3d_environment *e)
{
    bool consumed;
    return xge3dEnvironmentReleaseOwnerChecked(e,&consumed);
}
int xge3dEnvironmentShareOwned(xge3d_environment *e,xge3d_environment **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!e)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_environment_thread(e) || e->refs==UINT32_MAX)return XGE_ERROR_INVALID_STATE;
    if(!e->owner_thread){
        if(!d3_environment_context_matches(e,__xge3dContext()))return XGE_ERROR_INVALID_STATE;
        int status=xgeInternalGraphicsOwnerAcquire();
        if(status!=XGE_OK)return status;
        e->owner_thread=xrtThreadCurrentId();
    }
    ++e->refs;
    *out=e;
    return XGE_OK;
}
int xge3dEnvironmentCreate(const xge3d_environment_desc_t *d,xge3d_environment **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if (!d) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_environment *e=xrtMalloc(sizeof(*e));if (!e) return XGE_ERROR_OUT_OF_MEMORY;
    memset(e,0,sizeof(*e));e->refs=1;
    xge3d_cube_level_t level;memcpy(level.faces,d->faces,sizeof(level.faces));
    int result=d3_cube_copy(&e->sky,&level,1,d->srgb);
    if (result!=XGE_OK) { xge3dEnvironmentFree(e);return result; }
    *out=e;return XGE_OK;
}

static int d3_environment_create_owned(const xge3d_environment_desc_t *desc,xge3d_environment **out)
{
    int status=xgeInternalGraphicsOwnerAcquire();
    if(status!=XGE_OK)return status;
    status=xge3dEnvironmentCreate(desc,out);
    if(status!=XGE_OK){
        (void)xgeInternalGraphicsOwnerRelease();
        return status;
    }
    (*out)->owner_thread=xrtThreadCurrentId();
    return XGE_OK;
}
int xge3dEnvironmentCreateImagesOwned(xge_image_t *const *faces,size_t count,int srgb,xge3d_environment **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!faces || count!=6)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_environment_desc_t desc={.srgb=srgb};
    for(int i=0;i<6;++i)desc.faces[i]=faces[i];
    return d3_environment_create_owned(&desc,out);
}
int xge3dEnvironmentCreateRGBAOwned(int edge,const void *pixels,size_t bytes,int srgb,xge3d_environment **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    xge3d_cube_level_t level;
    xge_image_t faces[1][6];
    int status=d3_packed_cube(&level,faces,1,edge,pixels,bytes);
    if(status!=XGE_OK)return status;
    xge3d_environment_desc_t desc={.srgb=srgb};
    memcpy(desc.faces,level.faces,sizeof(desc.faces));
    return d3_environment_create_owned(&desc,out);
}
int d3_cube_upload(d3_cube *e, uint64_t *bytes)
{
    uint64_t context=__xge3dContext();
    if (!context) return XGE_ERROR_NOT_INITIALIZED;
    if (e->context && e->context!=context) return XGE_ERROR_INVALID_STATE;
    if (e->id) { xge_glBindTexture(0x8513,e->id); return XGE_OK; }
    GLint maximum;
    xge_glGetIntegerv(0x851C,&maximum);
    if (e->size>maximum) return XGE_ERROR_UNSUPPORTED;
    int unpack[5];
    d3_unpack_begin(unpack);
    xge_glGenTextures(1,&e->id); xge_glBindTexture(0x8513,e->id);
    int size=e->size;size_t offset=0;
    for (int l=0;l<e->levels;++l) {
        for (int f=0;f<6;++f) xge_glTexImage2D(0x8515+f,l,e->srgb ? 0x8C43 : GL_RGBA8,
            size,size,0,GL_RGBA,GL_UNSIGNED_BYTE,e->pixels.Data+offset+(size_t)f*size*size*4);
        offset+=(size_t)size*size*24;size=size>1 ? size/2 : 1;
    }
    xge_glTexParameteri(0x8513,0x813D,e->levels-1);
    xge_glTexParameteri(0x8513,GL_TEXTURE_MIN_FILTER,e->levels>1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    xge_glTexParameteri(0x8513,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    xge_glTexParameteri(0x8513,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    xge_glTexParameteri(0x8513,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    xge_glTexParameteri(0x8513,0x8072,GL_CLAMP_TO_EDGE);
    d3_unpack_end(unpack);
    if (!e->id || xge_glGetError()!=GL_NO_ERROR) {
        if (e->id) xge_glDeleteTextures(1,&e->id);
        e->id=0; return XGE_ERROR_GPU_FAILED;
    }
    e->context=context;
    if (bytes) *bytes+=e->pixels.Count;
    return XGE_OK;
}
int d3_environment_upload(xge3d_environment *e,uint64_t *bytes)
{ return d3_cube_upload(&e->sky,bytes); }
#endif
