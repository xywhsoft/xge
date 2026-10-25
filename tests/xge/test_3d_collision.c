#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"collision %d: %s\n",__LINE__,#x);return 1;}}while(0)
static xge3d_mat4_t matrix(float x,float y,float z){xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){x,y,z};xge3d_mat4_t m;xge3dTransformMatrix(&t,&m);return m;}
static double random_real(xrng *rng){return xrtRngReal(rng)*20-10;}
int main(void)
{
    xge3d_scene *s=NULL;CHECK(!xge3dSceneCreate(&s));xge3d_node_t root,wall;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&root)&&!xge3dNodeCreate(s,root,&wall));
    xge3d_shape_t box=xge3dShapeDefault(XGE3D_SHAPE_BOX);box.half_extents=(xge3d_vec3_t){1,2,2};CHECK(!xge3dNodeSetCollider(s,wall,&box)&&!xge3dNodeSetVisible(s,root,0));
    xge3d_shape_t sphere=xge3dShapeDefault(XGE3D_SHAPE_SPHERE);xge3d_mat4_t at=matrix(-4,0,0);xge3d_collision_hit_t hit;
    CHECK(!xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){8,0,0},NULL,&hit)&&fabsf(hit.distance-2.5f)<1e-4f&&hit.normal.x<-.99f&&hit.node.slot==wall.slot&&!hit.started_overlapping);
    xge3d_collision_filter_t filter={UINT32_MAX,root};CHECK(xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){8,0,0},&filter,&hit)==XGE_ERROR_NOT_FOUND&&!hit.node.slot);
    CHECK(!xge3dNodeSetLayers(s,wall,0,4,0));filter=(xge3d_collision_filter_t){2,{0}};size_t count=99;at=matrix(0,0,0);
    CHECK(!xge3dSceneOverlapShape(s,&sphere,&at,&filter,NULL,0,&count)&&!count);filter.layers=4;
    CHECK(!xge3dSceneOverlapShape(s,&sphere,&at,&filter,NULL,0,&count)&&count==1);
    CHECK(!xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){0,0,0},NULL,&hit)&&hit.started_overlapping&&hit.fraction==0);
    at=matrix(8,0,0);CHECK(!xge3dNodeSetMatrix(s,root,&at));at=matrix(-4,0,0);
    CHECK(!xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){20,0,0},NULL,&hit)&&fabsf(hit.distance-10.5f)<1e-4f);
    CHECK(!xge3dNodeSetGlobalPosition(s,root,(xge3d_dvec3_t){1e9+8,0,-1e9})&&!xge3dSceneSetOrigin(s,(xge3d_dvec3_t){1e9,0,-1e9}));
    CHECK(!xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){20,0,0},NULL,&hit)&&fabsf(hit.distance-10.5f)<1e-4f);
    CHECK(!xge3dNodeDestroy(s,root));
    /* Convex support mappings cover nonuniform scale, reflection and shear. */
    CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&wall));at=matrix(0,0,0);at.m[0]=-2;at.m[4]=.7f;CHECK(!xge3dNodeSetMatrix(s,wall,&at)&&!xge3dNodeSetCollider(s,wall,&sphere));
    xge3d_shape_t point=xge3dShapeDefault(XGE3D_SHAPE_SPHERE);point.radius=0;at=matrix(.7f,.1f,0);
    CHECK(!xge3dSceneOverlapShape(s,&point,&at,NULL,NULL,0,&count)&&count==1);at=matrix(2,2,0);CHECK(!xge3dSceneOverlapShape(s,&point,&at,NULL,NULL,0,&count)&&!count);
    CHECK(!xge3dNodeDestroy(s,wall));
    for(int target=0;target<3;++target){
        CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&wall));xge3d_shape_t target_shape=xge3dShapeDefault(target);CHECK(!xge3dNodeSetCollider(s,wall,&target_shape));
        for(int query_type=0;query_type<3;++query_type){xge3d_shape_t query=xge3dShapeDefault(query_type);at=matrix(0,0,0);
            CHECK(!xge3dSceneOverlapShape(s,&query,&at,NULL,NULL,0,&count)&&count==1);at=matrix(-4,0,0);
            CHECK(!xge3dSceneOverlapShape(s,&query,&at,NULL,NULL,0,&count)&&!count);
            CHECK(!xge3dSceneSweepShape(s,&query,&at,(xge3d_vec3_t){8,0,0},NULL,&hit)&&fabsf(hit.distance-3)<1e-4f&&hit.normal.x<-.98f);}
        CHECK(!xge3dNodeDestroy(s,wall));
    }
    CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&wall));xge3d_shape_t tangent=sphere;tangent.radius=1;CHECK(!xge3dNodeSetCollider(s,wall,&tangent));at=matrix(-4,2,0);
    CHECK(!xge3dSceneSweepShape(s,&tangent,&at,(xge3d_vec3_t){8,0,0},NULL,&hit)&&fabsf(hit.fraction-.5f)<.002f);
    at.m[0]=0;CHECK(xge3dSceneSweepShape(s,&tangent,&at,(xge3d_vec3_t){8,0,0},NULL,&hit)==XGE_ERROR_INVALID_ARGUMENT&&!hit.node.slot);
    CHECK(!xge3dNodeDestroy(s,wall));
    /* Random sphere-sphere queries compared against independent analytic math. */
    enum{N=80};xge3d_vec3_t centers[N];float radii[N];xge3d_node_t nodes[N];xrng rng;xrtRngSeed(&rng,345,1);
    for(int i=0;i<N;++i){centers[i]=(xge3d_vec3_t){(float)random_real(&rng),(float)random_real(&rng),(float)random_real(&rng)};radii[i]=.2f+(float)xrtRngReal(&rng);
        xge3d_shape_t target=sphere;target.radius=radii[i];CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&nodes[i]));at=matrix(centers[i].x,centers[i].y,centers[i].z);
        CHECK(!xge3dNodeSetMatrix(s,nodes[i],&at)&&!xge3dNodeSetCollider(s,nodes[i],&target));}
    for(int q=0;q<200;++q){xge3d_vec3_t p={(float)random_real(&rng),(float)random_real(&rng),(float)random_real(&rng)},motion={(float)random_real(&rng)*2,(float)random_real(&rng)*2,(float)random_real(&rng)*2};at=matrix(p.x,p.y,p.z);
        size_t expected=0;double best=2;int chosen=-1;
        for(int i=0;i<N;++i){double x=p.x-centers[i].x,y=p.y-centers[i].y,z=p.z-centers[i].z,r=sphere.radius+radii[i],c=x*x+y*y+z*z-r*r;
            if(c<=0){++expected;if(best>0){best=0;chosen=i;}continue;}double a=motion.x*motion.x+motion.y*motion.y+motion.z*motion.z,b=x*motion.x+y*motion.y+z*motion.z,disc=b*b-a*c;
            if(disc>=0&&a>0){double t=(-b-sqrt(disc))/a;if(t>=0&&t<=1&&t<best){best=t;chosen=i;}}}
        CHECK(!xge3dSceneOverlapShape(s,&sphere,&at,NULL,NULL,0,&count)&&count==expected);
        int result=xge3dSceneSweepShape(s,&sphere,&at,motion,NULL,&hit);
        if(chosen<0){CHECK(result==XGE_ERROR_NOT_FOUND);}else{if(result)fprintf(stderr,"random query %d result %d expected %.8f\n",q,result,best);CHECK(!result&&fabs(hit.fraction-best)<.0002);}
    }
    for(int i=0;i<N;++i)CHECK(!xge3dNodeDestroy(s,nodes[i]));
    /* Static mesh triangle BVH and geometry update, with no visual mesh. */
    xarray vertices,indices;xrtArrayInit(&vertices,sizeof(xge3d_vertex_t));xrtArrayInit(&indices,sizeof(uint32_t));CHECK(xrtArrayResize(&vertices,33*33)&&xrtArrayResize(&indices,32*32*6));memset(vertices.Data,0,vertices.Count*vertices.ItemSize);
    for(int z=0;z<33;++z)for(int x=0;x<33;++x)((xge3d_vertex_t*)vertices.Data)[z*33+x].position=(xge3d_vec3_t){(float)x,0,(float)z};
    for(int z=0;z<32;++z)for(int x=0;x<32;++x){uint32_t a=(uint32_t)(z*33+x),ix[]={a,a+33,a+34,a,a+34,a+1};memcpy(indices.Data+(z*32+x)*6*sizeof(uint32_t),ix,sizeof(ix));}
    xge3d_mesh_desc_t md={(const xge3d_vertex_t*)vertices.Data,vertices.Count,indices.Data,indices.Count,32};xge3d_mesh *mesh=NULL;CHECK(!xge3dMeshCreate(&md,&mesh)&&!xge3dNodeCreate(s,(xge3d_node_t){0},&wall));
    xge3d_shape_t shape=xge3dShapeDefault(XGE3D_SHAPE_MESH);shape.mesh=mesh;CHECK(!xge3dNodeSetCollider(s,wall,&shape));at=matrix(14.3f,4,15.7f);
    CHECK(!xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){0,-10,0},NULL,&hit)&&fabsf(hit.distance-3.5f)<1e-4f&&hit.normal.y>.99f&&hit.triangle!=SIZE_MAX);
    xge3d_collision_stats_t stats;CHECK(!xge3dSceneCollisionStats(s,&stats)&&stats.triangle_bvh_bytes>0&&stats.triangle_tests<30);
    for(size_t i=0;i<vertices.Count;++i)((xge3d_vertex_t*)vertices.Data)[i].position.y=2;
    CHECK(!xge3dMeshUpdate(mesh,&md)&&!xge3dSceneSweepShape(s,&sphere,&at,(xge3d_vec3_t){0,-10,0},NULL,&hit)&&fabsf(hit.distance-1.5f)<1e-4f);
    xge3d_shape_t capsule=xge3dShapeDefault(XGE3D_SHAPE_CAPSULE);CHECK(!xge3dSceneSweepShape(s,&capsule,&at,(xge3d_vec3_t){0,-10,0},NULL,&hit)&&fabsf(hit.distance-1)<1e-4f);
    xge3d_ray_t ray={{14.3f,4,15.7f},{0,-2,0}};CHECK(!xge3dSceneCollisionRaycast(s,&ray,10,NULL,&hit)&&fabsf(hit.distance-2)<1e-4f);
    xge3dNodeDestroy(s,wall);xge3dMeshFree(mesh);xrtArrayUnit(&vertices);xrtArrayUnit(&indices);xge3dSceneFree(s);
    puts("Collision: 200 analytic random sweeps/overlaps, affine primitives, hidden/layer/self filters, parent movement/rebase, mesh BVH/update, capsules and rays passed");return 0;
}
