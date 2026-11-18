#define CGLTF_IMPLEMENTATION
#include "extlibs/cgltf/cgltf.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct allocation_probe {
    size_t allowance,live;
    bool denied;
} allocation_probe;

static void *allocate(void *user,cgltf_size bytes)
{
    allocation_probe *probe=user;
    if(!probe->allowance){probe->denied=true;return NULL;}
    --probe->allowance;
    void *block=malloc(bytes?bytes:1);
    assert(block);
    ++probe->live;
    return block;
}
static void release(void *user,void *block)
{
    if(!block)return;
    allocation_probe *probe=user;
    assert(probe->live);
    --probe->live;
    free(block);
}

int main(void)
{
    /* A local glTF document with an ordinary POSITION array and one material
     * variant exercises both parser-owned count/pointer publication paths. */
    static const char document[]=
        "{\"asset\":{\"version\":\"2.0\"},"
        "\"extensionsUsed\":[\"KHR_materials_variants\"],"
        "\"extensions\":{\"KHR_materials_variants\":{\"variants\":[{\"name\":\"Red\"}]}},"
        "\"scenes\":[{\"nodes\":[0]}],\"nodes\":[{\"mesh\":0}],"
        "\"materials\":[{}],\"meshes\":[{\"primitives\":[{"
        "\"attributes\":{\"POSITION\":0},\"material\":0,"
        "\"extensions\":{\"KHR_materials_variants\":{\"mappings\":[{\"material\":0,\"variants\":[0]}]}}"
        "}]}],\"accessors\":[{\"componentType\":5126,\"count\":3,\"type\":\"VEC3\"}]}";
    for(size_t allowance=0;allowance<100;++allowance){
        allocation_probe probe={.allowance=allowance};
        cgltf_options options={.memory={allocate,release,&probe}};
        cgltf_data *data=NULL;
        cgltf_result result=cgltf_parse(&options,document,sizeof(document)-1,&data);
        if(result==cgltf_result_success){
            assert(!probe.denied && data && data->meshes[0].primitives[0].mappings_count==1);
            cgltf_free(data);
            assert(!probe.live && allowance>5);
            printf("CGLTF_OOM_OK %zu\n",allowance);
            return 0;
        }
        assert(probe.denied && !data && !probe.live);
    }
    return 1;
}
