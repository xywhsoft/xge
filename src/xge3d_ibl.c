#include "xge3d_internal.h"
#if XGE3D_ENABLE_IBL
int xge3dEnvironmentSetIBL(xge3d_environment *e,const xge3d_ibl_desc_t *d)
{
    if (!e) return XGE_ERROR_INVALID_ARGUMENT;
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
