#include "xge3d_internal.h"
#include "xge_graphics_internal.h"
#include "xge_gl.h"
#include <limits.h>

#if XGE_ENABLE_3D
int xge3dTextureResourceInfo(const xge3d_texture *texture,xge3d_resource_info_t *out)
{
    if(!texture||!out||out->size!=sizeof(*out)){return XGE_ERROR_INVALID_ARGUMENT;}
    if(!d3_texture_thread(texture))return XGE_ERROR_INVALID_STATE;
    *out=(xge3d_resource_info_t){.size=sizeof(*out),.references=texture->refs,.textures=1};
#if XGE3D_ENABLE_DYNAMIC
    if(texture->target){return XGE_OK;} /* View owns no separate pixel storage. */
#endif
    out->cpu_payload_bytes=(uint64_t)texture->width*texture->height*4;
    if(texture->id && (!texture->owner_thread || texture->context==__xge3dContext())){
        int width=texture->width,height=texture->height;out->gpu_payload_bytes=out->cpu_payload_bytes;
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
    if (!t || !d3_texture_thread(t) ||
        t->refs==UINT32_MAX) {return 0;}
    ++t->refs; return 1;
}
static int d3_texture_release(xge3d_texture *t,bool *consumed)
{
    if(!t)return XGE_OK;
    bool managed=t->owner_thread!=0;
    if(managed){
        if(t->owner_thread!=xrtThreadCurrentId())return XGE_ERROR_INVALID_STATE;
        uint64_t current=__xge3dContext();
        if(t->refs==1 && t->id && current && t->context!=current)
            return XGE_ERROR_INVALID_STATE;
        /* With no active context on the creator thread, window teardown
         * has already reclaimed the GL object. Release the CPU owner. */
    }
    if(consumed)*consumed=true;
    if(--t->refs)return XGE_OK;
#if XGE3D_ENABLE_DYNAMIC
    if(t->target){xge3dTargetFree(t->target);xrtFree(t);return XGE_OK;}
#endif
    if (t->id && t->context==__xge3dContext()) {xge_glDeleteTextures(1,&t->id);}
    xrtArrayUnit(&t->pixels); xrtFree(t);
    return managed?xgeInternalGraphicsOwnerRelease():XGE_OK;
}
void xge3dTextureFree(xge3d_texture *t){(void)d3_texture_release(t,NULL);}
int xge3dTextureReleaseOwnerChecked(xge3d_texture *t,bool *consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    return t&&t->owner_thread?d3_texture_release(t,consumed):XGE_ERROR_INVALID_ARGUMENT;
}
int xge3dTextureReleaseChecked(xge3d_texture *t)
{
    bool consumed;
    return xge3dTextureReleaseOwnerChecked(t,&consumed);
}
int xge3dTextureShareOwned(xge3d_texture *t,xge3d_texture **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!t)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_texture_retain(t))return XGE_ERROR_INVALID_STATE;
    if(!t->owner_thread){
        int status=XGE_OK;
        if(t->context && t->context!=__xge3dContext())status=XGE_ERROR_INVALID_STATE;
        else {
#if XGE3D_ENABLE_DYNAMIC
            if(t->target){
                uint64_t thread=xrtThreadCurrentId();
                if(t->target->owner_thread && t->target->owner_thread!=thread)
                    status=XGE_ERROR_INVALID_STATE;
                else if(!t->target->owner_thread){
                    status=xgeInternalGraphicsOwnerAcquire();
                    if(status==XGE_OK)t->target->owner_thread=thread;
                }
            } else
#endif
                status=xgeInternalGraphicsOwnerAcquire();
        }
        if(status!=XGE_OK){
            --t->refs;
            return status;
        }
        t->owner_thread=xrtThreadCurrentId();
    }
    *out=t;
    return XGE_OK;
}
int d3_texture_detach_gpu(xge3d_texture *t)
{
    if(!t||!d3_texture_thread(t)||t->refs!=1){return XGE_ERROR_INVALID_STATE;}
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

int xge3dTextureCreateRGBAOwned(int width,int height,const void *pixels,size_t bytes,
    int srgb,const xge3d_sampler_t *sampler,xge3d_texture **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!pixels||width<=0||height<=0||width>INT_MAX/4 ||
        (size_t)height>SIZE_MAX/(size_t)(width*4) ||
        bytes<(size_t)(width*4)*(size_t)height)
        return XGE_ERROR_INVALID_ARGUMENT;
    int status=xgeInternalGraphicsOwnerAcquire();
    if(status!=XGE_OK)return status;
    xge_image_t image={.iWidth=width,.iHeight=height,.iStride=width*4,
        .iFormat=XGE_PIXEL_RGBA8,.pPixels=(void*)pixels};
    xge3d_texture_desc_t desc={.image=&image,.srgb=srgb};
    if(sampler)desc.sampler=*sampler;
    status=xge3dTextureCreate(&desc,out);
    if(status!=XGE_OK){
        (void)xgeInternalGraphicsOwnerRelease();
        return status;
    }
    (*out)->owner_thread=xrtThreadCurrentId();
    return XGE_OK;
}
/* Called only inside a renderer's saved GL state, on its context thread. */
int d3_texture_upload(xge3d_texture *t, uint64_t *bytes)
{
    if(!d3_texture_thread(t))return XGE_ERROR_INVALID_STATE;
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
    GLint maximum;
    xge_glGetIntegerv(0x0D33,&maximum);
    if (t->width>maximum || t->height>maximum) {return XGE_ERROR_UNSUPPORTED;}
    int unpack[5];
    d3_unpack_begin(unpack);
    xge_glGenTextures(1,&t->id); xge_glBindTexture(GL_TEXTURE_2D,t->id);
    xge_glTexImage2D(GL_TEXTURE_2D,0,t->srgb ? 0x8C43 : GL_RGBA8,t->width,t->height,0,GL_RGBA,GL_UNSIGNED_BYTE,t->pixels.Data);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,t->sampler.min_filter);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,t->sampler.mag_filter);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,t->sampler.wrap_u);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,t->sampler.wrap_v);
    if (t->sampler.min_filter>=0x2700 && t->sampler.min_filter<=0x2703) {xge_glGenerateMipmap(GL_TEXTURE_2D);}
    d3_unpack_end(unpack);
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
    if (!m || !d3_material_thread(m) || m->refs==UINT32_MAX) {return 0;}
    ++m->refs; return 1;
}
static int d3_material_release(xge3d_material *m,bool *consumed)
{
    if(!m)return XGE_OK;
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    if(consumed)*consumed=true;
    if(--m->refs)return XGE_OK;
    bool managed=m->owner_thread!=0;
    for (int i=0;i<XGE3D_MAP_COUNT;++i) {xge3dTextureFree(m->desc.maps[i].texture);}
#if XGE3D_ENABLE_SHADER
    xge3dShaderFree(m->shader);
#endif
    xrtFree(m);
    return managed?xgeInternalGraphicsOwnerRelease():XGE_OK;
}
void xge3dMaterialFree(xge3d_material *m){(void)d3_material_release(m,NULL);}
int xge3dMaterialReleaseOwnerChecked(xge3d_material *m,bool *consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    return m&&m->owner_thread?d3_material_release(m,consumed):XGE_ERROR_INVALID_ARGUMENT;
}
int xge3dMaterialReleaseChecked(xge3d_material *m)
{
    bool consumed;
    return xge3dMaterialReleaseOwnerChecked(m,&consumed);
}
int xge3dMaterialShareOwned(xge3d_material *m,xge3d_material **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!m)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_material_retain(m))return XGE_ERROR_INVALID_STATE;
    if(!m->owner_thread){
        int status=xgeInternalGraphicsOwnerAcquire();
        if(status!=XGE_OK){
            --m->refs;
            return status;
        }
        m->owner_thread=xrtThreadCurrentId();
    }
    *out=m;
    return XGE_OK;
}

/* The public value record is the numeric prefix, before pointer alignment. */
_Static_assert(offsetof(xge3d_material_desc_t,unlit)==offsetof(xge3d_material_params_t,unlit) &&
    sizeof(xge3d_material_params_t)<=offsetof(xge3d_material_desc_t,maps),
    "material parameters must match the native numeric prefix");
static xge3d_material_params_t d3_material_params(const xge3d_material_desc_t *desc)
{
    xge3d_material_params_t params;
    memcpy(&params,desc,sizeof(params));
    return params;
}

static int d3_material_validate_params(const xge3d_material_params_t *d)
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
    return XGE_OK;
}
static int d3_material_validate_map(int slot,const xge3d_texture_binding_t *map)
{
    if(slot<0 || slot>=XGE3D_MAP_COUNT)return XGE_ERROR_INVALID_ARGUMENT;
#if XGE3D_ENABLE_DYNAMIC
    if(map->texture && map->texture->target && slot!=XGE3D_MAP_BASE_COLOR && slot!=XGE3D_MAP_EMISSIVE)
        return XGE_ERROR_UNSUPPORTED;
#endif
    if(map->texcoord<0 || map->texcoord>1 || !isfinite(map->rotation))return XGE_ERROR_INVALID_ARGUMENT;
    for(int axis=0;axis<2;++axis)if(!isfinite(map->offset[axis]) || !isfinite(map->scale[axis]))
        return XGE_ERROR_INVALID_ARGUMENT;
    return XGE_OK;
}
static int d3_material_validate(const xge3d_material_desc_t *d)
{
    if(!d)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_material_params_t params=d3_material_params(d);
    int status=d3_material_validate_params(&params);
    for(int slot=0;status==XGE_OK && slot<XGE3D_MAP_COUNT;++slot)
        status=d3_material_validate_map(slot,&d->maps[slot]);
    return status;
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
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    *out=m->desc; return XGE_OK;
}

xge3d_material_params_t xge3dMaterialParamsDefault(void)
{
    xge3d_material_desc_t desc=xge3dMaterialDefault();
    return d3_material_params(&desc);
}
xge3d_texture_mapping_t xge3dTextureMappingDefault(void)
{
    return (xge3d_texture_mapping_t){.scale={1,1}};
}
int xge3dMaterialCreateOwned(const xge3d_material_params_t *params,xge3d_material **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    xge3d_material_desc_t desc=xge3dMaterialDefault();
    if(params)memcpy(&desc,params,sizeof(*params));
    int status=xgeInternalGraphicsOwnerAcquire();
    if(status!=XGE_OK)return status;
    status=xge3dMaterialCreate(&desc,out);
    if(status!=XGE_OK){
        (void)xgeInternalGraphicsOwnerRelease();
        return status;
    }
    (*out)->owner_thread=xrtThreadCurrentId();
    return XGE_OK;
}
int xge3dMaterialGetParams(const xge3d_material *m,xge3d_material_params_t *out)
{
    if(!m||!out)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    *out=d3_material_params(&m->desc);
    return XGE_OK;
}
int xge3dMaterialSetParams(xge3d_material *m,const xge3d_material_params_t *params)
{
    if(!m)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    int status=d3_material_validate_params(params);
    if(status==XGE_OK)memcpy(&m->desc,params,sizeof(*params));
    return status;
}
int xge3dMaterialGetMapping(const xge3d_material *m,int slot,xge3d_texture_mapping_t *out)
{
    if(!m||!out||slot<0||slot>=XGE3D_MAP_COUNT)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    const xge3d_texture_binding_t *map=&m->desc.maps[slot];
    *out=(xge3d_texture_mapping_t){map->texcoord,{map->offset[0],map->offset[1]},
        {map->scale[0],map->scale[1]},map->rotation};
    return XGE_OK;
}
int xge3dMaterialSetMap(xge3d_material *m,int slot,xge3d_texture *texture,
    const xge3d_texture_mapping_t *mapping)
{
    if(!m)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    xge3d_texture_mapping_t v=mapping?*mapping:xge3dTextureMappingDefault();
    xge3d_texture_binding_t next={texture,v.texcoord,{v.offset[0],v.offset[1]},
        {v.scale[0],v.scale[1]},v.rotation};
    int status=d3_material_validate_map(slot,&next);
    if(status!=XGE_OK)return status;
    if(texture && !d3_texture_retain(texture))return XGE_ERROR_INVALID_STATE;
    xge3d_texture *old=m->desc.maps[slot].texture;
    m->desc.maps[slot]=next;
    xge3dTextureFree(old);
    return XGE_OK;
}
int xge3dMaterialMapTextureOwned(const xge3d_material *m,int slot,xge3d_texture **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!m||slot<0||slot>=XGE3D_MAP_COUNT)return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    xge3d_texture *texture=m->desc.maps[slot].texture;
    return texture?xge3dTextureShareOwned(texture,out):XGE_OK;
}
#if XGE3D_ENABLE_DYNAMIC
int xge3dMaterialSetDesc(xge3d_material *m,const xge3d_material_desc_t *d)
{
    if(!m){return XGE_ERROR_INVALID_ARGUMENT;}
    if(!d3_material_thread(m))return XGE_ERROR_INVALID_STATE;
    int result=d3_material_validate(d);if(result){return result;}
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
    t->owner_thread=target->owner_thread;
    t->context=target->context;
    t->sampler=(xge3d_sampler_t){GL_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};*out=t;return XGE_OK;
}
int xge3dTextureUpdate(xge3d_texture *t,int x,int y,int width,int height,const void *pixels,int stride)
{
    if(!t||!pixels||x<0||y<0||width<=0||height<=0||x>t->width-width||y>t->height-height||
        width>INT_MAX/4||stride<width*4||(size_t)stride>SIZE_MAX/(size_t)height){return XGE_ERROR_INVALID_ARGUMENT;}
    if(!d3_texture_thread(t))return XGE_ERROR_INVALID_STATE;
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

int xge3dTextureUpdateRGBA(xge3d_texture *texture,int x,int y,int width,int height,
    const void *pixels,size_t bytes,int stride)
{
#if XGE3D_ENABLE_DYNAMIC
    if(!texture || width<=0 || width>INT_MAX/4 || height<=0 || !pixels)
        return XGE_ERROR_INVALID_ARGUMENT;
    int row=width*4;
    if(!stride)stride=row;
    if(stride<row || bytes<(size_t)row || (size_t)(height-1)>(bytes-row)/(size_t)stride)
        return XGE_ERROR_INVALID_ARGUMENT;
    return xge3dTextureUpdate(texture,x,y,width,height,pixels,stride);
#else
    (void)texture;(void)x;(void)y;(void)width;(void)height;(void)pixels;(void)bytes;(void)stride;
    return XGE_ERROR_UNSUPPORTED;
#endif
}

int xge3dTextureFromTargetOwned(xge3d_target *target,xge3d_texture **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
#if XGE3D_ENABLE_DYNAMIC
    if(!target||!target->owner_thread)return XGE_ERROR_INVALID_ARGUMENT;
    return xge3dTextureFromTarget(target,out);
#else
    (void)target;
    return XGE_ERROR_UNSUPPORTED;
#endif
}
#endif
