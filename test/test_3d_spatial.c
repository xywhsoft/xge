#include "../xge.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}} while(0)
#define NEAR(a,b) (fabs((double)(a)-(b))<.0001)
#if XGE3D_ENABLE_ANIMATION
static int contains(xge3d_aabb_t b,xge3d_vec3_t p)
{return p.x>=b.min.x-.0001f && p.x<=b.max.x+.0001f && p.y>=b.min.y-.0001f && p.y<=b.max.y+.0001f && p.z>=b.min.z-.0001f && p.z<=b.max.z+.0001f;}
static xge3d_vec3_t point(xge3d_mat4_t m,xge3d_vec3_t v)
{return (xge3d_vec3_t){m.m[0]*v.x+m.m[4]*v.y+m.m[8]*v.z+m.m[12],m.m[1]*v.x+m.m[5]*v.y+m.m[9]*v.z+m.m[13],m.m[2]*v.x+m.m[6]*v.y+m.m[10]*v.z+m.m[14]};}
#endif
int main(void)
{
    xge3d_aabb_t box={{-1,-1,-1},{1,1,1}},touch={{1,0,0},{2,1,1}},bad={{2,0,0},{1,1,1}};
    xge3d_sphere_t sphere={{2,0,0},1};float distance=99;xge3d_hit_t hit;
    CHECK(xge3dAabbIntersects(&box,&touch) && !xge3dAabbIntersects(&box,&bad));
    CHECK(xge3dSphereIntersectsAabb(&sphere,&box));sphere.center.x=2.01f;CHECK(!xge3dSphereIntersectsAabb(&sphere,&box));
    xge3d_ray_t ray={{0,0,5},{0,0,-7}};
    CHECK(xge3dRayAabb(&ray,&box,4,&distance)==XGE_OK && NEAR(distance,4));
    CHECK(xge3dRayAabb(&ray,&box,3.99f,&distance)==XGE_ERROR_NOT_FOUND && distance==0);
    ray.origin=(xge3d_vec3_t){1,0,5};CHECK(xge3dRayAabb(&ray,&box,10,&distance)==XGE_OK && NEAR(distance,4));
    ray.origin.x=1.01f;CHECK(xge3dRayAabb(&ray,&box,10,&distance)==XGE_ERROR_NOT_FOUND);
    ray.origin=(xge3d_vec3_t){0};CHECK(xge3dRayAabb(&ray,&box,10,&distance)==XGE_OK && distance==0);
    sphere=(xge3d_sphere_t){{0},1};ray.origin.z=5;
    CHECK(xge3dRaySphere(&ray,&sphere,4,&distance)==XGE_OK && NEAR(distance,4));
    ray.origin.x=1;CHECK(xge3dRaySphere(&ray,&sphere,5,&distance)==XGE_OK && NEAR(distance,5));
    ray.origin.x=1.01f;CHECK(xge3dRaySphere(&ray,&sphere,10,&distance)==XGE_ERROR_NOT_FOUND);
    ray.origin=(xge3d_vec3_t){0};CHECK(xge3dRaySphere(&ray,&sphere,10,&distance)==XGE_OK && distance==0);
    xge3d_vec3_t tri[]={{-1,-1,0},{1,-1,0},{0,1,0}};ray.origin=(xge3d_vec3_t){0,0,5};
    CHECK(xge3dRayTriangle(&ray,tri,5,&hit)==XGE_OK && NEAR(hit.distance,5) && NEAR(hit.normal.z,1));
    CHECK(xge3dRayTriangle(&ray,tri,4.9f,&hit)==XGE_ERROR_NOT_FOUND && hit.distance==0);
    ray.direction=(xge3d_vec3_t){1,0,0};CHECK(xge3dRayTriangle(&ray,tri,10,&hit)==XGE_ERROR_NOT_FOUND);
    ray.direction=(xge3d_vec3_t){0};CHECK(xge3dRayAabb(&ray,&box,10,&distance)==XGE_ERROR_INVALID_ARGUMENT);
    sphere.radius=-1;CHECK(xge3dRaySphere(&ray,&sphere,10,&distance)==XGE_ERROR_INVALID_ARGUMENT);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,4,4,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(xge3dCameraIntersectsAabb(&camera,&box));touch=(xge3d_aabb_t){{2,-1,-1},{3,1,1}};
    CHECK(xge3dCameraIntersectsAabb(&camera,&touch));touch.min.x=2.01f;CHECK(!xge3dCameraIntersectsAabb(&camera,&touch));
    touch=(xge3d_aabb_t){{-1,-1,3},{1,1,4}};CHECK(!xge3dCameraIntersectsAabb(&camera,&touch));

    xge3d_scene *s=NULL;xge3d_mesh *mesh=NULL;xge3d_node_t root,child,other;size_t count;
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&root)==XGE_OK);
    CHECK(xge3dNodeCreate(s,root,&child)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&other)==XGE_OK);
    CHECK(xge3dSceneNodeAt(s,0).slot==root.slot && !xge3dSceneNodeAt(s,3).slot);
    xge3d_vertex_t vertices[3]={0};for (int i=0;i<3;++i) vertices[i].position=tri[i];
    xge3d_mesh_desc_t md={vertices,3,NULL,0,0};CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK);
    CHECK(xge3dNodeSetMesh(s,child,mesh)==XGE_OK && xge3dNodeSetMesh(s,other,mesh)==XGE_OK);
    const xge3d_mesh *borrow=NULL;CHECK(xge3dNodeGetMesh(s,child,&borrow)==XGE_OK && borrow==mesh);
    CHECK(xge3dMeshGetBounds(mesh,&box)==XGE_OK && NEAR(box.min.x,-1) && NEAR(box.max.y,1));
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=10;t.scale.x=-2;t.scale.y=3;
    CHECK(xge3dNodeSetTransform(s,root,&t)==XGE_OK);
    xge3d_mat4_t m;CHECK(xge3dNodeGetWorldMatrix(s,child,&m)==XGE_OK);m.m[4]=.5f;
    CHECK(xge3dNodeSetMatrix(s,other,&m)==XGE_OK);
    CHECK(xge3dNodeGetBounds(s,other,0,&box)==XGE_OK && NEAR(box.min.x,7.5) && NEAR(box.max.x,12.5));
    CHECK(xge3dNodeGetBounds(s,root,0,&box)==XGE_ERROR_NOT_FOUND);
    CHECK(xge3dNodeGetBounds(s,root,1,&box)==XGE_OK && NEAR(box.min.x,8) && NEAR(box.max.x,12));
    CHECK(xge3dSceneQueryAabb(s,&box,NULL,0,&count)==XGE_OK && count==2);
    xge3d_node_t nodes[2];CHECK(xge3dSceneQueryAabb(s,&box,nodes,1,&count)==XGE_ERROR_BUFFER_TOO_SMALL && count==2);
    CHECK(xge3dNodeSetVisible(s,root,0)==XGE_OK && xge3dSceneQueryAabb(s,&box,nodes,2,&count)==XGE_OK && count==1 && nodes[0].slot==other.slot);
    CHECK(xge3dNodeGetBounds(s,root,1,&box)==XGE_OK); /* Geometry export includes hidden nodes. */
    vertices[0].position.x=-3;CHECK(xge3dMeshUpdate(mesh,&md)==XGE_OK);
    CHECK(xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,16));

    xge3d_mesh *low=NULL;vertices[0].position.x=-7;CHECK(xge3dMeshCreate(&md,&low)==XGE_OK);
    xge3d_lod_t levels[]={{low,5},{low,10},{low,20}};
    CHECK(xge3dNodeSetLods(s,child,levels,3)==XGE_OK && xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,24));
    CHECK(xge3dNodeGetMesh(s,child,&borrow)==XGE_OK && borrow==mesh);
    levels[1].min_distance=4;CHECK(xge3dNodeSetLods(s,child,levels,3)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeSetLods(s,child,levels,4)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,24));
    vertices[0].position.x=-8;CHECK(xge3dMeshUpdate(low,&md)==XGE_OK && xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,26));
    xge3dMeshFree(low); /* All three retained levels survive caller release. */
    CHECK(xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,26));
    CHECK(xge3dNodeSetMesh(s,child,mesh)==XGE_OK && xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,16));
    vertices[0].position.x=NAN;CHECK(xge3dMeshUpdate(mesh,&md)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeGetBounds(s,child,0,&box)==XGE_OK && NEAR(box.max.x,16));

    xge3d_dvec3_t origin={1e9,2e9,-3e9},global={1e9+.25,2e9+.5,-3e9+.75};xge3d_vec3_t relative;
    CHECK(xge3dRelativePosition(origin,global,&relative)==XGE_OK && relative.x==.25f && relative.y==.5f && relative.z==.75f);
    CHECK(xge3dSceneSetOrigin(s,origin)==XGE_OK && xge3dNodeSetGlobalPosition(s,root,global)==XGE_OK);
    CHECK(xge3dNodeSetGlobalPosition(s,child,global)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeGetWorldMatrix(s,child,&m)==XGE_OK && m.m[12]==.25f);
    for (int i=0;i<32;++i) {
        CHECK(xge3dSceneSetOrigin(s,(xge3d_dvec3_t){0})==XGE_OK);
        CHECK(xge3dSceneSetOrigin(s,origin)==XGE_OK);
        CHECK(xge3dNodeGetWorldMatrix(s,root,&m)==XGE_OK && m.m[12]==.25f && m.m[13]==.5f && m.m[14]==.75f);
    }
    CHECK(xge3dSceneSetOrigin(s,(xge3d_dvec3_t){DBL_MAX,0,0})==XGE_ERROR_INVALID_ARGUMENT && xge3dSceneOrigin(s).x==origin.x);
    CHECK(xge3dNodeGetWorldMatrix(s,root,&m)==XGE_OK && m.m[12]==.25f);
    CHECK(xge3dNodeSetParent(s,child,(xge3d_node_t){0})==XGE_OK);
    CHECK(xge3dSceneSetOrigin(s,(xge3d_dvec3_t){origin.x-2,origin.y,origin.z})==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(s,child,&m)==XGE_OK && m.m[12]==2); /* Detach retains local transform. */
    CHECK(xge3dCameraLookAtGlobal(&camera,origin,(xge3d_dvec3_t){origin.x,origin.y,origin.z+3},origin,(xge3d_vec3_t){0,1,0})==XGE_OK && NEAR(camera.view.m[14],-3));
    xge3d_node_t stale=child;CHECK(xge3dNodeDestroy(s,child)==XGE_OK);
    CHECK(xge3dNodeGetBounds(s,stale,1,&box)==XGE_ERROR_INVALID_ARGUMENT && box.min.x==0);
    xge3dMeshFree(mesh);xge3dSceneFree(s);
#if XGE3D_ENABLE_ANIMATION
    xge3d_model *model=NULL;CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)==XGE_OK && xge3dSceneCreate(&s)==XGE_OK);
    CHECK(xge3dModelInstantiate(s,model,(xge3d_node_t){0},&root)==XGE_OK && xge3dModelInstanceNode(s,root,1,&child)==XGE_OK);
    CHECK(xge3dNodeGetMesh(s,child,&borrow)==XGE_OK);xge3d_lod_t skinned_lod={(xge3d_mesh*)borrow,10};
    CHECK(xge3dNodeSetLods(s,child,&skinned_lod,1)==XGE_ERROR_UNSUPPORTED);
    xge3d_node_t tip;CHECK(xge3dModelInstanceNode(s,root,3,&tip)==XGE_OK);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){2,3,0};t.rotation=(xge3d_quat_t){0,0,.70710678f,.70710678f};
    CHECK(xge3dNodeSetTransform(s,tip,&t)==XGE_OK && xge3dNodeGetBounds(s,child,0,&box)==XGE_OK);
    const xge3d_mat4_t *palette;CHECK(xge3dNodeSkinMatrices(s,child,&palette,&count)==XGE_OK && xge3dNodeGetMesh(s,child,&borrow)==XGE_OK);
    CHECK(xge3dMeshGetData(borrow,&md)==XGE_OK && xge3dNodeGetWorldMatrix(s,child,&m)==XGE_OK);
    for (size_t i=0;i<md.vertex_count;++i) {
        const xge3d_vertex_t *v=&md.vertices[i];xge3d_vec3_t p={0};
        for (int j=0;j<4;++j) if (v->weights[j]>0) {xge3d_vec3_t q=point(palette[v->joints[j]],v->position);p.x+=q.x*v->weights[j];p.y+=q.y*v->weights[j];p.z+=q.z*v->weights[j];}
        CHECK(contains(box,point(m,p)));
    }
    xge3dSceneFree(s);xge3dModelFree(model);
#endif
    puts("3D spatial CPU: geometric rays/overlaps, shared and skinned bounds, cache invalidation, query capacity/visibility and double-origin precision/rollback passed");return 0;
}
