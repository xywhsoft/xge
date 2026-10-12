#include "xge3d_internal.h"
#if XGE3D_ENABLE_SHADOW
xge3d_shadow_settings_t xge3dShadowDefault(void)
{ return (xge3d_shadow_settings_t){512,3,100,.5f,.1f,.001f,.01f,1,50}; }

static xge3d_vec3_t d3_shadow_up(xge3d_vec3_t direction)
{ return fabsf(direction.y)>.95f ? (xge3d_vec3_t){0,0,1} : (xge3d_vec3_t){0,1,0}; }

int d3_shadow_build(const xge3d_camera_t *camera, const xge3d_shadow_settings_t *s,
    const d3_light *lights, int count, xge3d_dvec3_t origin, d3_shadow_frame *out)
{
    memset(out,0,sizeof(*out)); out->sun=-1;
    for (int i=0;i<8;++i) out->light_map[i]=-1;
    if (!s) return XGE_OK;
    if (s->resolution<16 || s->resolution>8192 || s->cascades<1 || s->cascades>4 ||
        !isfinite(s->distance) || s->distance<=0 || !isfinite(s->split_lambda) || s->split_lambda<0 || s->split_lambda>1 ||
        !isfinite(s->blend) || s->blend<0 || s->blend>.5f || !isfinite(s->bias) || s->bias<0 ||
        !isfinite(s->normal_bias) || s->normal_bias<0 || !isfinite(s->filter_radius) || s->filter_radius<0 || s->filter_radius>2 ||
        !isfinite(s->depth_padding) || s->depth_padding<0 || !isfinite(camera->near_z) ||
        !isfinite(camera->far_z) || camera->near_z<0 || camera->far_z<=camera->near_z || s->distance<=camera->near_z)
        return XGE_ERROR_INVALID_ARGUMENT;
    for (int i=0;i<count;++i) if (lights[i].desc.casts_shadow && lights[i].desc.type==XGE3D_LIGHT_DIRECTIONAL) {
        if (out->sun>=0) return XGE_ERROR_UNSUPPORTED;
        out->sun=i;
    }
    if (out->sun>=0) {
        xge3d_mat4_t inverse;
        if (!d3_inverse(d3_mul(camera->projection,camera->view),&inverse)) return XGE_ERROR_INVALID_ARGUMENT;
        xge3d_vec3_t corners[8];
        for (int i=0;i<8;++i) {
            xge3d_vec3_t q={i&1 ? 1 : -1,i&2 ? 1 : -1,i&4 ? 1 : -1};
            float w=inverse.m[3]*q.x+inverse.m[7]*q.y+inverse.m[11]*q.z+inverse.m[15];
            if (fabsf(w)<1e-30f) return XGE_ERROR_INVALID_ARGUMENT;
            corners[i]=d3_scale(d3_point(inverse,q),1/w);
        }
        float near=fmaxf(camera->near_z,.001f),far=fminf(camera->far_z,s->distance),previous=camera->near_z;
        for (int c=0;c<s->cascades;++c) {
            float fraction=(c+1)/(float)s->cascades;
            float split=near+(far-near)*fraction;
            if (camera->projection.m[15]==0) split=(1-s->split_lambda)*split+s->split_lambda*near*powf(far/near,fraction);
            out->splits[c]=split;
            /* Overlap receiver slices by the blend interval. */
            float start=c ? previous-(previous-(c>1 ? out->splits[c-2] : near))*s->blend : previous;
            float a=(start-camera->near_z)/(camera->far_z-camera->near_z);
            float b=(split-camera->near_z)/(camera->far_z-camera->near_z);
            xge3d_vec3_t points[8],center={0};
            for (int k=0;k<4;++k) {
                xge3d_vec3_t delta=d3_sub(corners[k+4],corners[k]);
                points[k]=d3_add(corners[k],d3_scale(delta,a)); points[k+4]=d3_add(corners[k],d3_scale(delta,b));
                center=d3_add(center,d3_add(points[k],points[k+4]));
            }
            center=d3_scale(center,.125f); float radius=0;
            for (int k=0;k<8;++k) radius=fmaxf(radius,sqrtf(d3_dot(d3_sub(points[k],center),d3_sub(points[k],center))));
            radius=ceilf(radius*16)/16; radius=fmaxf(radius,.0625f);
            /* Enclosing sphere and snapped light-space translation resist shimmering. */
            radius*=s->resolution/(float)(s->resolution-2);
            xge3d_vec3_t dir=lights[out->sun].direction;
            xge3d_vec3_t eye=d3_sub(center,d3_scale(dir,radius+s->depth_padding));
            xge3d_camera_t light_camera;
            int result=xge3dCameraOrthographic(&light_camera,2*radius,2*radius,0,2*(radius+s->depth_padding));
            if (result==XGE_OK) result=xge3dCameraLookAt(&light_camera,(xge3d_vec3_t){0},dir,d3_shadow_up(dir));
            if (result!=XGE_OK) return result;
            float texel=2*radius/s->resolution;
            for (int axis=0;axis<3;++axis) {
                const float *v=light_camera.view.m;
                double translation=-((double)v[axis]*eye.x+(double)v[4+axis]*eye.y+(double)v[8+axis]*eye.z);
                if (axis<2) {
                    /* Keep the texel grid anchored to global coordinates after
                     * rebasing; reduce the large offset before float math. */
                    double phase=fmod((double)v[axis]*origin.x+(double)v[4+axis]*origin.y+(double)v[8+axis]*origin.z,texel);
                    translation=round((translation-phase)/texel)*texel+phase;
                }
                light_camera.view.m[12+axis]=(float)translation;
            }
            out->matrices[c]=d3_mul(light_camera.projection,light_camera.view);
            previous=split;
        }
        out->cascades=s->cascades; out->count=s->cascades; out->light_map[out->sun]=0;
    }
    int spots=0;
    for (int i=0;i<count;++i) if (lights[i].desc.casts_shadow && lights[i].desc.type==XGE3D_LIGHT_SPOT) {
        if (++spots>XGE3D_MAX_SHADOW_SPOTS) return XGE_ERROR_UNSUPPORTED;
        float far=lights[i].desc.range>0 ? lights[i].desc.range : s->distance;
        if (far<=.05f) return XGE_ERROR_INVALID_ARGUMENT;
        xge3d_camera_t light_camera;
        int result=xge3dCameraPerspective(&light_camera,2*lights[i].desc.outer_angle,1,.05f,far);
        if (result==XGE_OK) result=xge3dCameraLookAt(&light_camera,lights[i].position,
            d3_add(lights[i].position,lights[i].direction),d3_shadow_up(lights[i].direction));
        if (result!=XGE_OK) return result;
        out->light_map[i]=out->count;
        out->matrices[out->count++]=d3_mul(light_camera.projection,light_camera.view);
    }
    return XGE_OK;
}
#endif
