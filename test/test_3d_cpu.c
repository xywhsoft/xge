#include "../xge.h"
#include <math.h>
#include <stdio.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); return 1; } } while(0)
#define CLOSE(a,b) (fabsf((a)-(b)) < 0.0001f)

int main(void)
{
    xge3d_scene *a=NULL, *b=NULL;
    xge3d_node_t root, child, other, stale;
    xge3d_transform_t t = XGE3D_TRANSFORM_IDENTITY;
    xge3d_mat4_t world;
    xge3d_camera_t camera;
    CHECK(xge3dSceneCreate(&a)==XGE_OK && xge3dSceneCreate(&b)==XGE_OK);
    CHECK(xge3dNodeCreate(a,(xge3d_node_t){0},&root)==XGE_OK);
    CHECK(xge3dNodeCreate(a,root,&child)==XGE_OK);
    CHECK(xge3dNodeCreate(b,(xge3d_node_t){0},&other)==XGE_OK);
    CHECK(xge3dNodeSetParent(a,root,child)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeSetParent(a,child,other)==XGE_ERROR_INVALID_ARGUMENT);
    t.position.x=10; t.scale=(xge3d_vec3_t){2,3,-4};
    CHECK(xge3dNodeSetTransform(a,root,&t)==XGE_OK);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;
    t.position=(xge3d_vec3_t){1,2,3};
    CHECK(xge3dNodeSetTransform(a,child,&t)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(a,child,&world)==XGE_OK);
    CHECK(CLOSE(world.m[12],12) && CLOSE(world.m[13],6) && CLOSE(world.m[14],-12));
    CHECK(xge3dNodeSetParent(a,child,(xge3d_node_t){0})==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(a,child,&world)==XGE_OK && CLOSE(world.m[12],1));
    /* Keep shear exactly; do not silently decompose imported matrices. */
    world.m[4]=0.35f;
    CHECK(xge3dNodeSetMatrix(a,child,&world)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(a,child,&world)==XGE_OK && CLOSE(world.m[4],0.35f));
    t.position.x=NAN;
    CHECK(xge3dNodeSetTransform(a,child,&t)==XGE_ERROR_INVALID_ARGUMENT);
    stale=child;
    CHECK(xge3dNodeDestroy(a,child)==XGE_OK);
    CHECK(xge3dNodeCreate(a,root,&child)==XGE_OK && child.slot!=stale.slot);
    CHECK(xge3dNodeSetVisible(a,stale,1)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeSetVisible(b,root,1)==XGE_ERROR_INVALID_ARGUMENT);
    xge3d_vertex_t vertices[3]={0};
    vertices[1].position.x=1; vertices[2].position.y=1;
    uint16_t indices[3]={0,1,2};
    xge3d_mesh_desc_t md={vertices,3,indices,3,16};
    xge3d_mesh *mesh=NULL;
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK);
    CHECK(xge3dNodeSetMesh(a,root,mesh)==XGE_OK);
    CHECK(xge3dNodeSetMesh(a,child,mesh)==XGE_OK);
    /* Caller can release its reference; both nodes still own the shared mesh. */
    xge3dMeshFree(mesh); mesh=NULL;
    indices[2]=3;
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_ERROR_INVALID_ARGUMENT && mesh==NULL);
    indices[2]=2;
    for (int i=0;i<100;++i) {
        CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK);
        md.index_bits=8;
        CHECK(xge3dMeshUpdate(mesh,&md)==XGE_ERROR_INVALID_ARGUMENT);
        md.index_bits=16;
        CHECK(xge3dMeshUpdate(mesh,&md)==XGE_OK);
        xge3dMeshFree(mesh); mesh=NULL;
    }
    CHECK(xge3dCameraPerspective(&camera,1.0f,1.5f,0.1f,100)==XGE_OK);
    float near_clip=camera.projection.m[10]*-0.1f+camera.projection.m[14];
    float far_clip=camera.projection.m[10]*-100+camera.projection.m[14];
    CHECK(CLOSE(near_clip/0.1f,-1) && CLOSE(far_clip/100,1));
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(CLOSE(camera.view.m[14],-3));
    xge3d_ray_t ray;
    CHECK(xge3dCameraScreenRay(&camera,75,50,150,100,&ray)==XGE_OK);
    CHECK(CLOSE(ray.origin.z,2.9f) && CLOSE(ray.direction.z,-1) && CLOSE(ray.direction.x,0));
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dCameraPerspective(&camera,1,0,0.1f,100)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dCameraOrthographic(&camera,4,2,0,10)==XGE_OK);
    CHECK(CLOSE(camera.projection.m[0],0.5f) && CLOSE(camera.projection.m[10],-0.2f));
    CHECK(xge3dCameraScreenRay(&camera,75,25,100,100,&ray)==XGE_OK);
    CHECK(CLOSE(ray.origin.x,1) && CLOSE(ray.origin.y,.5f) && CLOSE(ray.direction.z,-1));
    /* Mirrored, nonuniformly scaled triangles and inherited visibility. */
    ray=(xge3d_ray_t){{9,1,5},{0,0,-4}};
    xge3d_hit_t hit;
    CHECK(xge3dSceneRaycast(a,&ray,10,&hit)==XGE_ERROR_NOT_FOUND);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;
    t.position.x=10; t.scale=(xge3d_vec3_t){-2,3,1};
    CHECK(xge3dNodeSetTransform(a,root,&t)==XGE_OK);
    CHECK(xge3dNodeSetVisible(a,child,0)==XGE_OK);
    CHECK(xge3dSceneRaycast(a,&ray,10,&hit)==XGE_OK && hit.node.slot==root.slot);
    CHECK(CLOSE(hit.distance,5) && CLOSE(hit.position.x,9) && CLOSE(hit.normal.z,1));
    CHECK(xge3dSceneRaycast(a,&ray,4.99f,&hit)==XGE_ERROR_NOT_FOUND && hit.node.slot==0);
    CHECK(xge3dNodeSetVisible(a,root,0)==XGE_OK);
    CHECK(xge3dSceneRaycast(a,&ray,10,&hit)==XGE_ERROR_NOT_FOUND);
    camera.projection=(xge3d_mat4_t){{0}};
    CHECK(xge3dCameraScreenRay(&camera,0,0,100,100,&ray)==XGE_ERROR_INVALID_ARGUMENT);
    /* Restore the original parent scale for the deep hierarchy check. */
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY; t.position.x=10; t.scale=(xge3d_vec3_t){2,3,-4};
    CHECK(xge3dNodeSetTransform(a,root,&t)==XGE_OK);
    /* Deep hierarchy is evaluated iteratively and deletion invalidates all nodes. */
    xge3d_node_t tail=child;
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY; t.position.y=1;
    for(int i=0;i<2048;++i) {
        xge3d_node_t n;
        CHECK(xge3dNodeCreate(a,tail,&n)==XGE_OK);
        CHECK(xge3dNodeSetTransform(a,n,&t)==XGE_OK);
        tail=n;
    }
    CHECK(xge3dNodeGetWorldMatrix(a,tail,&world)==XGE_OK && CLOSE(world.m[13],6144));
    CHECK(xge3dNodeDestroy(a,root)==XGE_OK && xge3dSceneNodeCount(a)==0);
    CHECK(xge3dNodeGetWorldMatrix(a,tail,&world)==XGE_ERROR_INVALID_ARGUMENT);
    xge3dSceneFree(a); xge3dSceneFree(b);
    puts("3D CPU: transforms, projection, hierarchy, stale/foreign handles, shared mesh lifetime and failed updates passed");
    return 0;
}
