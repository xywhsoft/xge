#include "xge.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const xge3d_vec3_t up={0,1,0};
    const xge3d_vec3_t eye={.25f,1.5f,3}, target={2.25f,1.5f,3};
    const xge3d_dvec3_t origin={1e12,-1e12,1e12};
    const xge3d_dvec3_t global_eye={origin.x+.25,origin.y+1.5,origin.z+3};
    const xge3d_dvec3_t global_target={origin.x+2.25,origin.y+1.5,origin.z+3};
    xge3d_camera_t camera;
    assert(xge3dCameraPerspective(&camera,1,1.5f,.1f,100)==XGE_OK);
    xge3d_camera_t original=camera;
    xge3d_mat4_t local,global;

    assert(xge3dViewLookAt(eye,target,up,&local)==XGE_OK);
    assert(xge3dViewLookAtGlobal(origin,global_eye,global_target,up,&global)==XGE_OK);
    assert(!memcmp(&local,&global,sizeof(local)));
    assert(xge3dCameraLookAt(&camera,eye,target,up)==XGE_OK);
    assert(!memcmp(&camera.view,&local,sizeof(local)));
    assert(xge3dCameraLookAtGlobal(&camera,origin,global_eye,global_target,up)==XGE_OK);
    assert(!memcmp(&camera.view,&global,sizeof(global)));
    assert(!memcmp(&camera.projection,&original.projection,sizeof(camera.projection)));
    assert(camera.near_z==original.near_z && camera.far_z==original.far_z);

    xge3d_ray_t ray;
    assert(xge3dCameraScreenRay(&camera,400,300,800,600,&ray)==XGE_OK);
    assert(fabsf(ray.direction.x-1)<1e-5f && fabsf(ray.direction.y)<1e-5f && fabsf(ray.direction.z)<1e-5f);
    assert(fabsf(ray.origin.x-.35f)<1e-5f && fabsf(ray.origin.y-1.5f)<1e-5f && fabsf(ray.origin.z-3)<1e-5f);
    const xge3d_aabb_t front={{1,1.4f,2.9f},{2,1.6f,3.1f}};
    const xge3d_aabb_t behind={{-2,1.4f,2.9f},{-1,1.6f,3.1f}};
    assert(xge3dCameraIntersectsAabb(&camera,&front));
    assert(!xge3dCameraIntersectsAabb(&camera,&behind));

    const xge3d_vec3_t parallel={1,0,0}, nan_up={NAN,1,0};
    assert(xge3dViewLookAt(eye,eye,up,&global)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dViewLookAt(eye,target,parallel,&global)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dViewLookAt(eye,target,nan_up,&global)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dViewLookAt(eye,target,up,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dViewLookAtGlobal(origin,global_eye,global_target,up,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dViewLookAtGlobal(origin,(xge3d_dvec3_t){DBL_MAX,0,0},global_target,up,&global)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dViewLookAtGlobal((xge3d_dvec3_t){-DBL_MAX,0,0},(xge3d_dvec3_t){DBL_MAX,0,0},global_target,up,&global)==XGE_ERROR_INVALID_ARGUMENT);
    assert(!memcmp(&global,&local,sizeof(global)));
    const xge3d_vec3_t large_eye={FLT_MAX,FLT_MAX,0}, large_target={FLT_MAX,FLT_MAX,-1}, diagonal_up={-1,1,0};
    assert(xge3dCameraLookAt(&camera,large_eye,large_target,diagonal_up)==XGE_ERROR_INVALID_ARGUMENT);
    assert(!memcmp(&camera.view,&local,sizeof(local)));
    assert(!memcmp(&camera.projection,&original.projection,sizeof(camera.projection)));
    puts("CAMERA_VIEW3D_NATIVE_OK");
    return 0;
}
