#include "xge3d_internal.h"
#if XGE3D_ENABLE_LIGHTING
xge3d_light_desc_t xge3dLightDefault(int type)
{
    return (xge3d_light_desc_t){.type=type,.color={1,1,1},.direction={0,0,-1},
        .intensity=1,.range=10,.inner_angle=.35f,.outer_angle=.65f};
}
int xge3dNodeSetLight(xge3d_scene *scene, xge3d_node_t node, const xge3d_light_desc_t *d)
{
    xge3d_node_data *n=d3_node(scene,node);
    if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    if (!d) { n->has_light=0; return XGE_OK; }
    xge3d_vec3_t direction=d->direction;
#if XGE3D_ENABLE_SHADOW
    if (d->casts_shadow && d->type==XGE3D_LIGHT_POINT) return XGE_ERROR_UNSUPPORTED;
#endif
    if (d->type<0 || d->type>2 || !d3_finite3(d->color) || d->color.x<0 || d->color.y<0 || d->color.z<0 ||
        !isfinite(d->intensity) || d->intensity<0 || !d3_finite3(direction) ||
        (d->type!=XGE3D_LIGHT_POINT && !d3_normalize(&direction)) ||
        !isfinite(d->range) || d->range<0 ||
        !isfinite(d->inner_angle) || !isfinite(d->outer_angle) ||
        (d->type==XGE3D_LIGHT_SPOT && (d->inner_angle<0 || d->inner_angle>=d->outer_angle || d->outer_angle>=1.57079632f)))
        return XGE_ERROR_INVALID_ARGUMENT;
    n->light=*d; n->light.direction=direction; n->has_light=1; return XGE_OK;
}
#endif
