#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); return 1; } } while(0)
static int loaded, released;
static int provider(const char *uri, void **bytes, int *size, void *user)
{
    (void)user;
    if (strncmp(uri,"asset://fixture/",16)) return XGE_ERROR_FILE_NOT_FOUND;
    char path[512]; snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/%s",uri+16);
    FILE *file=fopen(path,"rb"); if (!file) return XGE_ERROR_FILE_NOT_FOUND;
    fseek(file,0,SEEK_END); long length=ftell(file); rewind(file);
    *bytes=malloc((size_t)length); *size=(int)length;
    if (!*bytes) { fclose(file); return XGE_ERROR_OUT_OF_MEMORY; }
    size_t read=fread(*bytes,1,(size_t)length,file); fclose(file);
    if (read!=(size_t)length) { free(*bytes); *bytes=NULL; return XGE_ERROR_RESOURCE_FAILED; }
    ++loaded; return XGE_OK;
}
static void provider_free(void *bytes, void *user)
{ (void)user; free(bytes); ++released; }

int main(void)
{
    xge_desc_t desc={0}; desc.iRunMode=XGE_RUN_MANUAL;
    CHECK(xgeInit(&desc)==XGE_OK);
    const char *files[]={"external.gltf","embedded.gltf","embedded.glb","nonindexed.gltf","sparse.gltf"};
    for (size_t i=0;i<sizeof(files)/sizeof(files[0]);++i) {
        char path[256]; snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/%s",files[i]);
        xge3d_model *model=NULL;
        CHECK(xge3dModelLoad(path,&model)==XGE_OK);
        xge3d_model_info_t info=xge3dModelInfo(model);
        CHECK(info.node_count==2 && info.primitive_count==1 && info.material_count==1);
        CHECK(!strcmp(xge3dModelNodeName(model,1),"Triangle"));
        xge3d_material_desc_t material;
        CHECK(xge3dMaterialGetDesc(xge3dModelMaterial(model,0),&material)==XGE_OK);
        CHECK(material.maps[0].texture!=NULL && material.maps[0].texcoord==0);
        xge3d_scene *scene=NULL; xge3d_node_t a,b,na,nb;
        CHECK(xge3dSceneCreate(&scene)==XGE_OK);
        CHECK(xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&a)==XGE_OK);
        CHECK(xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&b)==XGE_OK);
        CHECK(xge3dSceneNodeCount(scene)==6);
        CHECK(xge3dModelInstanceNode(scene,a,1,&na)==XGE_OK);
        CHECK(xge3dModelInstanceNode(scene,b,1,&nb)==XGE_OK && na.slot!=nb.slot);
        xge3dModelFree(model); model=NULL;
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY; t.position.x=5;
        CHECK(xge3dNodeSetTransform(scene,a,&t)==XGE_OK);
        t.position.x=-5; CHECK(xge3dNodeSetTransform(scene,b,&t)==XGE_OK);
        xge3d_mat4_t matrix;
        CHECK(xge3dNodeGetWorldMatrix(scene,na,&matrix)==XGE_OK && fabsf(matrix.m[12]-7)<.0001f && fabsf(matrix.m[13]-1)<.0001f);
        CHECK(xge3dNodeGetWorldMatrix(scene,nb,&matrix)==XGE_OK && fabsf(matrix.m[12]+3)<.0001f);
        xge3d_ray_t ray={{7,1,5},{0,0,-1}}; xge3d_hit_t hit;
        CHECK(xge3dSceneRaycast(scene,&ray,10,&hit)==XGE_OK && hit.model_root.slot==a.slot);
        CHECK(hit.node.slot==na.slot && fabsf(hit.distance-5)<.0001f);
        CHECK(xge3dNodeDestroy(scene,a)==XGE_OK && xge3dSceneNodeCount(scene)==3);
        CHECK(xge3dModelInstanceNode(scene,b,1,&nb)==XGE_OK);
        xge3dSceneFree(scene);
    }
    xge3d_model *shared=NULL;
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/shared-materials.gltf",&shared)==XGE_OK);
    xge3d_material_desc_t m0,m1;
    CHECK(xge3dMaterialGetDesc(xge3dModelMaterial(shared,0),&m0)==XGE_OK);
    CHECK(xge3dMaterialGetDesc(xge3dModelMaterial(shared,1),&m1)==XGE_OK);
    CHECK(m0.maps[0].texture==m1.maps[0].texture && m1.maps[0].texcoord==1);
    CHECK(m1.maps[0].offset[0]==.25f && m1.maps[0].scale[1]==3 && m1.maps[0].rotation==.5f);
    xge3dModelFree(shared);
    const char *bad[]={"overflow","short","badindex","extension","cycle","missing","badbase64","missinguv"};
    for (size_t i=0;i<sizeof(bad)/sizeof(bad[0]);++i) {
        char path[256]; snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/%s.gltf",bad[i]);
        xge3d_model *model=(xge3d_model*)1;
        CHECK(xge3dModelLoad(path,&model)!=XGE_OK && model==NULL);
    }
    xge3d_model *model=(xge3d_model*)1;
    CHECK(xge3dModelLoadMemory("bad",3,NULL,&model)!=XGE_OK && model==NULL);
    xge_resource_t source={0};
    CHECK(xgeResourceLoad("artifacts/xge-3d/fixtures/embedded.glb",&source)==XGE_OK);
    CHECK(xge3dModelLoadMemory(source.pData,(size_t)source.iSize,NULL,&model)==XGE_OK);
    xgeResourceFree(&source); xge3dModelFree(model);
    xge_resource_provider_t p={0}; p.sScheme="asset"; p.load=provider; p.free=provider_free;
    CHECK(xgeResourceProviderAdd(&p)==XGE_OK);
    CHECK(xge3dModelLoad("asset://fixture/external.gltf",&model)==XGE_OK);
    CHECK(loaded==3 && released==2);
    xge3dModelFree(model);
    CHECK(released==3);
    for (int i=0;i<100;++i) {
        CHECK(xge3dModelLoad("asset://fixture/external.gltf",&model)==XGE_OK);
        xge3dModelFree(model);
    }
    CHECK(loaded==released);
    xgeResourceProviderClear();
    xgeUnit();
    puts("3D import: external/data URI/GLB/sparse/nonindexed, independent shared instances, invalid bounds/indices/cycles/extensions, provider lifetime and 100 reloads passed");
    return 0;
}
