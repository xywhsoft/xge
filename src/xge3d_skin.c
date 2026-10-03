#include "xge3d_internal.h"
#include "xge_gl.h"
#include "../lib/cgltf/cgltf.h"

#if XGE3D_ENABLE_ANIMATION
void d3_skin_free(d3_skin *skin)
{
    if (!skin) return;
    if (skin->texture && skin->context==__xge3dContext()) glDeleteTextures(1,&skin->texture);
    xge3dModelFree(skin->model);xrtArrayUnit(&skin->joints);xrtArrayUnit(&skin->inverse_bind);xrtArrayUnit(&skin->palette);xrtFree(skin);
}
int d3_skin_create(xge3d_model *model,size_t index,const xarray *nodes,d3_skin **out)
{
    *out=NULL;cgltf_data *data=d3_model_data(model);
    if (!data || index>=data->skins_count) return XGE_ERROR_INVALID_ARGUMENT;
    const cgltf_skin *source=&data->skins[index];
    d3_skin *s=xrtMalloc(sizeof(*s));if (!s) return XGE_ERROR_OUT_OF_MEMORY;
    memset(s,0,sizeof(*s));
    if (!d3_model_retain(model)) {xrtFree(s);return XGE_ERROR_INVALID_STATE;}
    s->model=model;xrtArrayInit(&s->joints,sizeof(xge3d_node_t));xrtArrayInit(&s->inverse_bind,sizeof(xge3d_mat4_t));xrtArrayInit(&s->palette,sizeof(xge3d_mat4_t));
    if (!xrtArrayResize(&s->joints,source->joints_count) || !xrtArrayResize(&s->inverse_bind,source->joints_count) ||
        !xrtArrayResize(&s->palette,source->joints_count)) {d3_skin_free(s);return XGE_ERROR_OUT_OF_MEMORY;}
    memset(s->palette.Data,0,s->palette.Count*s->palette.ItemSize);
    if (source->inverse_bind_matrices && cgltf_accessor_unpack_floats(source->inverse_bind_matrices,
        (float*)s->inverse_bind.Data,source->joints_count*16)!=source->joints_count*16) {d3_skin_free(s);return XGE_ERROR_RESOURCE_FAILED;}
    for (size_t i=0;i<source->joints_count;++i) {
        size_t node=cgltf_node_index(data,source->joints[i]);
        const xge3d_node_t *handle=xrtArrayConstGet(nodes,node);
        if (!handle || !handle->slot) {d3_skin_free(s);return XGE_ERROR_RESOURCE_FAILED;}
        *(xge3d_node_t*)xrtArrayGet(&s->joints,i)=*handle;
        xge3d_mat4_t bind=source->inverse_bind_matrices ? *(xge3d_mat4_t*)xrtArrayGet(&s->inverse_bind,i) : d3_identity();
        if (!d3_finite_matrix(&bind) || bind.m[3]!=0 || bind.m[7]!=0 || bind.m[11]!=0 || bind.m[15]!=1) {d3_skin_free(s);return XGE_ERROR_RESOURCE_FAILED;}
        *(xge3d_mat4_t*)xrtArrayGet(&s->inverse_bind,i)=bind;
    }
    *out=s;return XGE_OK;
}
int d3_skin_update(xge3d_scene *scene,xge3d_node_data *node)
{
    d3_skin *s=node->skin;if (!s || s->version==scene->version) return XGE_OK;
    xge3d_mat4_t world,inverse;
    int result=xge3dNodeGetWorldMatrix(scene,node->handle,&world);
    if (result!=XGE_OK) return result;
    if (!d3_inverse(world,&inverse)) return XGE_ERROR_INVALID_ARGUMENT;
    int changed=0;
    for (size_t i=0;i<s->joints.Count;++i) {
        const xge3d_node_t *joint=xrtArrayConstGet(&s->joints,i);
        result=xge3dNodeGetWorldMatrix(scene,*joint,&world);
        if (result!=XGE_OK) return XGE_ERROR_INVALID_STATE;
        xge3d_mat4_t palette=d3_mul(d3_mul(inverse,world),*(xge3d_mat4_t*)xrtArrayGet(&s->inverse_bind,i));
        xge3d_mat4_t *previous=xrtArrayGet(&s->palette,i);
        if (memcmp(previous,&palette,sizeof(palette))) changed=1;
        *previous=palette;
    }
    if (changed) ++s->pose_version;
    s->version=scene->version;return XGE_OK;
}
int d3_skin_upload(d3_skin *s,uint64_t *bytes)
{
    uint64_t context=__xge3dContext();
    if (!context) return XGE_ERROR_NOT_INITIALIZED;
    if (s->context && s->context!=context) return XGE_ERROR_INVALID_STATE;
    glActiveTexture(GL_TEXTURE0+9);
    if (!s->texture) {
        glGenTextures(1,&s->texture);glBindTexture(GL_TEXTURE_2D,s->texture);
        glTexImage2D(GL_TEXTURE_2D,0,0x8814,4,(GLsizei)s->palette.Count,0,GL_RGBA,GL_FLOAT,s->palette.Data);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    } else {
        glBindTexture(GL_TEXTURE_2D,s->texture);
        if (s->uploaded_version==s->pose_version) return XGE_OK;
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,4,(GLsizei)s->palette.Count,GL_RGBA,GL_FLOAT,s->palette.Data);
    }
    if (!s->texture || glGetError()!=GL_NO_ERROR) {
        if (s->texture) glDeleteTextures(1,&s->texture);
        s->texture=0;return XGE_ERROR_GPU_FAILED;
    }
    s->context=context;s->uploaded_version=s->pose_version;if (bytes) *bytes+=s->palette.Count*sizeof(xge3d_mat4_t);return XGE_OK;
}
xge3d_vec3_t d3_skin_point(const d3_skin *s,const xge3d_vertex_t *v)
{
    xge3d_vec3_t result={0};
    for (int k=0;k<4;++k) if (v->weights[k]>0) {
        const xge3d_mat4_t *m=xrtArrayConstGet(&s->palette,v->joints[k]);
        if (m) result=d3_add(result,d3_scale(d3_point(*m,v->position),v->weights[k]));
    }
    return result;
}
int xge3dNodeSkinMatrices(xge3d_scene *scene,xge3d_node_t node,const xge3d_mat4_t **matrices,size_t *count)
{
    if (!matrices || !count) return XGE_ERROR_INVALID_ARGUMENT;
    *matrices=NULL;*count=0;xge3d_node_data *n=d3_node(scene,node);
    if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    if (!n->skin) return XGE_ERROR_NOT_FOUND;
    int result=d3_skin_update(scene,n);if (result!=XGE_OK) return result;
    *matrices=(const xge3d_mat4_t*)n->skin->palette.Data;*count=n->skin->palette.Count;return XGE_OK;
}
#endif
