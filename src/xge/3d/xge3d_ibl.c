#include "xge3d_internal.h"
#if XGE_ENABLE_3D
#if XGE3D_ENABLE_IBL
int xge3dEnvironmentSetIBL(xge3d_environment *e,const xge3d_ibl_desc_t *d)
{
    if (!e) return XGE_ERROR_INVALID_ARGUMENT;
    if(!d3_environment_thread(e))return XGE_ERROR_INVALID_STATE;
    uint64_t context=__xge3dContext();
    if ((e->irradiance.id && e->irradiance.context!=context) || (e->prefiltered.id && e->prefiltered.context!=context) ||
        (e->brdf && e->brdf->id && e->brdf->context!=context)) return XGE_ERROR_INVALID_STATE;
    d3_cube irradiance={0},prefiltered={0};xge3d_texture *brdf=NULL;int result=XGE_OK;
    if (d) {
        if (!d->prefiltered || !d->level_count || !d->prefiltered[0].faces[0] || !d->brdf)
            return XGE_ERROR_INVALID_ARGUMENT;
        int size=d->prefiltered[0].faces[0]->iWidth;size_t count=1;
        if (size<=0 || (size&(size-1))) return XGE_ERROR_INVALID_ARGUMENT;
        for (int mip=size;mip>1;mip/=2) ++count;
        if (count!=d->level_count) return XGE_ERROR_INVALID_ARGUMENT;
        result=d3_cube_copy(&irradiance,&d->irradiance,1,0);
        if (result==XGE_OK) result=d3_cube_copy(&prefiltered,d->prefiltered,d->level_count,0);
        xge3d_texture_desc_t desc={d->brdf,0,{9729,9729,33071,33071}};
        if (result==XGE_OK) result=xge3dTextureCreate(&desc,&brdf);
    }
    if (result==XGE_OK) {
        d3_cube_free(&e->irradiance);d3_cube_free(&e->prefiltered);xge3dTextureFree(e->brdf);
        e->irradiance=irradiance;e->prefiltered=prefiltered;e->brdf=brdf;
    } else { d3_cube_free(&irradiance);d3_cube_free(&prefiltered);xge3dTextureFree(brdf); }
    return result;
}
#endif

int xge3dEnvironmentSetIBLImages(xge3d_environment *e,
    xge_image_t *const *irradiance,size_t irradiance_count,
    xge_image_t *const *prefiltered,size_t prefiltered_count,const xge_image_t *brdf)
{
#if XGE3D_ENABLE_IBL
    if(!irradiance || irradiance_count!=6 || !prefiltered || !prefiltered_count ||
        prefiltered_count%6 || prefiltered_count/6>16 || !brdf)
        return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_cube_level_t levels[16];
    xge3d_ibl_desc_t desc={.prefiltered=levels,.level_count=prefiltered_count/6,.brdf=brdf};
    for(int face=0;face<6;++face)desc.irradiance.faces[face]=irradiance[face];
    for(size_t level=0;level<desc.level_count;++level)
        for(int face=0;face<6;++face)levels[level].faces[face]=prefiltered[level*6+face];
    return xge3dEnvironmentSetIBL(e,&desc);
#else
    (void)e;(void)irradiance;(void)irradiance_count;(void)prefiltered;(void)prefiltered_count;(void)brdf;
    return XGE_ERROR_UNSUPPORTED;
#endif
}

int xge3dEnvironmentSetIBLRGBA(xge3d_environment *e,
    int irradiance_edge,const void *irradiance,size_t irradiance_bytes,
    int prefiltered_edge,const void *prefiltered,size_t prefiltered_bytes,
    int brdf_width,int brdf_height,const void *brdf,size_t brdf_bytes)
{
#if XGE3D_ENABLE_IBL
    if(prefiltered_edge<=0 || (prefiltered_edge&(prefiltered_edge-1)))
        return XGE_ERROR_INVALID_ARGUMENT;
    size_t count=1;
    for(int edge=prefiltered_edge;edge>1;edge/=2)++count;
    if(count>16)return XGE_ERROR_INVALID_ARGUMENT;
    xge_image_t irradiance_faces[1][6],prefiltered_faces[16][6],lut;
    xge3d_cube_level_t levels[16];
    xge3d_ibl_desc_t desc={.prefiltered=levels,.level_count=count,.brdf=&lut};
    int status=d3_packed_cube(&desc.irradiance,irradiance_faces,1,irradiance_edge,irradiance,irradiance_bytes);
    if(status!=XGE_OK)return status;
    status=d3_packed_cube(levels,prefiltered_faces,count,prefiltered_edge,prefiltered,prefiltered_bytes);
    if(status!=XGE_OK)return status;
    status=d3_packed_image(&lut,brdf_width,brdf_height,brdf,brdf_bytes);
    if(status!=XGE_OK)return status;
    return xge3dEnvironmentSetIBL(e,&desc);
#else
    (void)e;(void)irradiance_edge;(void)irradiance;(void)irradiance_bytes;
    (void)prefiltered_edge;(void)prefiltered;(void)prefiltered_bytes;
    (void)brdf_width;(void)brdf_height;(void)brdf;(void)brdf_bytes;
    return XGE_ERROR_UNSUPPORTED;
#endif
}
int xge3dEnvironmentClearIBL(xge3d_environment *e)
{
#if XGE3D_ENABLE_IBL
    return xge3dEnvironmentSetIBL(e,NULL);
#else
    (void)e;
    return XGE_ERROR_UNSUPPORTED;
#endif
}
#endif
