#include "xge3d_internal.h"
#include "xge_clock.h"
#if XGE3D_ENABLE_MODEL
#define CGLTF_IMPLEMENTATION
#include "extlibs/cgltf/cgltf.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct d3_primitive {
    xge3d_mesh *mesh;
    xge3d_material *material;
} d3_primitive;
typedef struct d3_mesh_range { size_t first, count; } d3_mesh_range;
struct xge3d_model {
    uint32_t refs;
    int retiring;
    size_t truncated_weight_vertices;
    cgltf_data *data;
    xge_resource_t source;
    xarray primitives, ranges, images, materials, textures;
    xmap texture_cache;
    uint64_t parser_bytes, buffer_bytes;
};
typedef union d3_gltf_allocation { max_align_t alignment; size_t bytes; } d3_gltf_allocation;
static void *d3_gltf_alloc(void *user, cgltf_size size)
{
    if(size>INT_MAX-sizeof(d3_gltf_allocation))return NULL;
    d3_gltf_allocation *p=xrtMalloc(sizeof(*p)+size);if(!p)return NULL;
    p->bytes=sizeof(*p)+size;((xge3d_model*)user)->parser_bytes+=p->bytes;return p+1;
}
static void d3_gltf_free(void *user, void *data)
{if(data){d3_gltf_allocation *p=(d3_gltf_allocation*)data-1;((xge3d_model*)user)->parser_bytes-=p->bytes;xrtFree(p);}}
static int d3_gltf_result(cgltf_result result)
{
    if (result==cgltf_result_success) return XGE_OK;
    if (result==cgltf_result_out_of_memory) return XGE_ERROR_OUT_OF_MEMORY;
    if (result==cgltf_result_unknown_format || result==cgltf_result_legacy_gltf)
        return XGE_ERROR_UNSUPPORTED;
    return XGE_ERROR_RESOURCE_FAILED;
}
static char *d3_uri(const char *base, const char *relative)
{
    const char *slash=base ? strrchr(base,'/') : NULL;
    const char *backslash=base ? strrchr(base,'\\') : NULL;
    if (backslash && (!slash || backslash>slash)) slash=backslash;
    size_t prefix=slash ? (size_t)(slash-base)+1 : 0, len=strlen(relative);
    if (strstr(relative,"://") || relative[0]=='/' || relative[0]=='\\' ||
        (len>1 && relative[1]==':')) prefix=0;
    if (prefix>SIZE_MAX-len-1) return NULL;
    char *uri=xrtMalloc(prefix+len+1);
    if (!uri) return NULL;
    if (prefix) memcpy(uri,base,prefix);
    memcpy(uri+prefix,relative,len+1);
    cgltf_size decoded=cgltf_decode_uri(uri+prefix);
    if (strlen(uri+prefix)!=decoded) { xrtFree(uri); return NULL; }
    return uri;
}
static int d3_cancelled(const d3_model_io *io)
{return io && io->cancelled && io->cancelled(io->user);}
static int d3_read(const char *uri,xge_resource_t *out,const d3_model_io *io)
{return d3_cancelled(io) ? XGE_ERROR : io ? io->load(uri,out,io->user) : xgeResourceLoad(uri,out);}
static int d3_blob(const char *base, const char *uri, void **out, size_t *size,const d3_model_io *io)
{
    *out=NULL; *size=0;
    if (d3_cancelled(io)) return XGE_ERROR;
    if (!strncmp(uri,"data:",5)) {
        const char *comma=strchr(uri,',');
        if (!comma || comma-uri<7 || memcmp(comma-7,";base64",7)) return XGE_ERROR_UNSUPPORTED;
        *out=xrtBase64DecodeNew(comma+1,strlen(comma+1),size,NULL);
        return *out ? XGE_OK : XGE_ERROR_RESOURCE_FAILED;
    }
    char *path=d3_uri(base,uri);
    if (!path) return XGE_ERROR_RESOURCE_FAILED;
    xge_resource_t resource={0};
    int result=d3_read(path,&resource,io); xrtFree(path);
    if (result!=XGE_OK) return result;
    if (resource.iSize<=0 || !resource.pData) result=XGE_ERROR_RESOURCE_FAILED;
    else {
        *out=xrtMalloc((size_t)resource.iSize);
        if (!*out) result=XGE_ERROR_OUT_OF_MEMORY;
        else { *size=(size_t)resource.iSize; memcpy(*out,resource.pData,*size); }
    }
    xgeResourceFree(&resource);
    return result;
}
static int d3_buffers(xge3d_model *m, const char *base,const d3_model_io *io)
{
    cgltf_data *data=m->data;
    for (size_t i=0;i<data->buffers_count;++i) {
        if (d3_cancelled(io)) return XGE_ERROR;
        cgltf_buffer *b=&data->buffers[i];
        if (!b->size || b->size>INT_MAX) return XGE_ERROR_RESOURCE_FAILED;
        if (!b->uri) {
            if (i || !data->bin || b->size>data->bin_size) return XGE_ERROR_RESOURCE_FAILED;
            b->data=(void*)data->bin;
        } else {
            size_t size=0;
            int result=d3_blob(base,b->uri,&b->data,&size,io);
            /* d3_blob uses the ordinary allocator, not the parser header.
             * ModelFree owns these buffers, including partially loaded ones. */
            b->data_free_method=cgltf_data_free_method_none;m->buffer_bytes+=size;
            if (result!=XGE_OK) return result;
            if (size<b->size) return XGE_ERROR_RESOURCE_FAILED;
        }
    }
    return XGE_OK;
}
/* Check subtraction bounds before cgltf's validation does address arithmetic. */
static int d3_span(const cgltf_buffer_view *v, size_t offset, size_t count, size_t stride, size_t element)
{
    return v && count && stride>=element && offset<=v->size && element<=v->size-offset &&
        count-1<=(v->size-offset-element)/stride;
}
static int d3_validate(cgltf_data *data)
{
    for (size_t i=0;i<data->extensions_required_count;++i) {
        const char *name=data->extensions_required[i];
#if XGE3D_ENABLE_LIGHTING
        if (!strcmp(name,"KHR_lights_punctual")) continue;
#endif
        if (strcmp(name,"KHR_materials_unlit") && strcmp(name,"KHR_texture_transform"))
            return XGE_ERROR_UNSUPPORTED;
    }
    for (size_t i=0;i<data->buffer_views_count;++i) {
        cgltf_buffer_view *v=&data->buffer_views[i];
        if (v->has_meshopt_compression) return XGE_ERROR_UNSUPPORTED;
        if (!v->buffer || !v->buffer->data || v->offset>v->buffer->size || v->size>v->buffer->size-v->offset)
            return XGE_ERROR_RESOURCE_FAILED;
    }
    for (size_t i=0;i<data->accessors_count;++i) {
        cgltf_accessor *a=&data->accessors[i];
        size_t element=cgltf_calc_size(a->type,a->component_type);
        size_t component=cgltf_component_size(a->component_type);
        if (!element || !component || !a->count || a->count>INT_MAX || !a->stride ||
            a->offset%component || a->stride%component) return XGE_ERROR_RESOURCE_FAILED;
        if (a->buffer_view && a->buffer_view->offset%component) return XGE_ERROR_RESOURCE_FAILED;
        if (a->buffer_view && !d3_span(a->buffer_view,a->offset,a->count,a->stride,element))
            return XGE_ERROR_RESOURCE_FAILED;
        if (a->is_sparse) {
            cgltf_accessor_sparse *s=&a->sparse;
            size_t unit=cgltf_component_size(s->indices_component_type);
            if (!unit || s->count>a->count || !s->indices_buffer_view || !s->values_buffer_view ||
                s->indices_byte_offset%unit || s->indices_buffer_view->offset%unit ||
                s->values_byte_offset%component || s->values_buffer_view->offset%component ||
                !d3_span(s->indices_buffer_view,s->indices_byte_offset,s->count,unit,unit) ||
                !d3_span(s->values_buffer_view,s->values_byte_offset,s->count,element,element))
                return XGE_ERROR_RESOURCE_FAILED;
        }
    }
    xarray parents; xrtArrayInit(&parents,sizeof(unsigned char));
    if (!xrtArrayResize(&parents,data->nodes_count)) { xrtArrayUnit(&parents); return XGE_ERROR_OUT_OF_MEMORY; }
    if (parents.Count) memset(parents.Data,0,parents.Count);
    int result=XGE_OK;
    for (size_t i=0;i<data->nodes_count;++i) {
        cgltf_node *n=&data->nodes[i];
        xge3d_mat4_t local; cgltf_node_transform_local(n,local.m);
        if (!d3_finite_matrix(&local) || local.m[3] || local.m[7] || local.m[11] || local.m[15]!=1)
            result=XGE_ERROR_RESOURCE_FAILED;
        for (size_t j=0;j<n->children_count;++j) {
            cgltf_node *child=n->children[j];
            size_t index=cgltf_node_index(data,child);
            if (child->parent!=n || ((unsigned char*)parents.Data)[index]) result=XGE_ERROR_RESOURCE_FAILED;
            ((unsigned char*)parents.Data)[index]=1;
        }
    }
    xrtArrayUnit(&parents);
    if (result!=XGE_OK) return result;
#if !XGE3D_ENABLE_ANIMATION
    if (data->skins_count) return XGE_ERROR_UNSUPPORTED;
#else
    for (size_t i=0;i<data->skins_count;++i) {
        const cgltf_skin *skin=&data->skins[i];
        if (!skin->joints_count) return XGE_ERROR_RESOURCE_FAILED;
        if (skin->joints_count>D3_MAX_JOINTS) return XGE_ERROR_UNSUPPORTED;
        if (skin->inverse_bind_matrices && (skin->inverse_bind_matrices->type!=cgltf_type_mat4 ||
            skin->inverse_bind_matrices->component_type!=cgltf_component_type_r_32f ||
            skin->inverse_bind_matrices->count!=skin->joints_count)) return XGE_ERROR_RESOURCE_FAILED;
        for (size_t j=0;j<skin->joints_count;++j) {
            if (!skin->joints[j]) return XGE_ERROR_RESOURCE_FAILED;
            for (size_t k=0;k<j;++k) if (skin->joints[k]==skin->joints[j]) return XGE_ERROR_RESOURCE_FAILED;
        }
    }
#endif
    return d3_gltf_result(cgltf_validate(data));
}
static int d3_attributes(const cgltf_primitive *p, xarray *vertices)
{
    const cgltf_accessor *position=cgltf_find_accessor(p,cgltf_attribute_type_position,0);
    if (!position || position->type!=cgltf_type_vec3 || position->count>INT_MAX/sizeof(xge3d_vertex_t))
        return XGE_ERROR_RESOURCE_FAILED;
    if (!xrtArrayResize(vertices,position->count)) return XGE_ERROR_OUT_OF_MEMORY;
    memset(vertices->Data,0,vertices->Count*vertices->ItemSize);
    const cgltf_attribute_type attrs[]={cgltf_attribute_type_position,cgltf_attribute_type_normal,
        cgltf_attribute_type_texcoord,cgltf_attribute_type_tangent,cgltf_attribute_type_texcoord};
    const size_t offsets[]={offsetof(xge3d_vertex_t,position),offsetof(xge3d_vertex_t,normal),
        offsetof(xge3d_vertex_t,uv),offsetof(xge3d_vertex_t,tangent),offsetof(xge3d_vertex_t,uv1)};
    const size_t components[]={3,3,2,4,2};
    xarray values; xrtArrayInit(&values,sizeof(float));
    int result=XGE_OK;
    for (int i=0;i<5;++i) {
        const cgltf_accessor *a=cgltf_find_accessor(p,attrs[i],i==4 ? 1 : 0);
        if (!a) continue;
        size_t count=position->count*components[i];
        if (a->count!=position->count || cgltf_num_components(a->type)!=components[i]) {
            result=XGE_ERROR_RESOURCE_FAILED; break;
        }
        if (!xrtArrayResize(&values,count)) { result=XGE_ERROR_OUT_OF_MEMORY; break; }
        if (cgltf_accessor_unpack_floats(a,(float*)values.Data,count)!=count) {
            result=XGE_ERROR_RESOURCE_FAILED; break;
        }
        for (size_t v=0;v<position->count;++v)
            memcpy((char*)vertices->Data+v*vertices->ItemSize+offsets[i],
                (float*)values.Data+v*components[i],components[i]*sizeof(float));
    }
    xrtArrayUnit(&values);
    return result;
}
#if XGE3D_ENABLE_ANIMATION
static int d3_weights(const cgltf_primitive *p,xarray *vertices,size_t *truncated)
{
    if (!cgltf_find_accessor(p,cgltf_attribute_type_joints,0) && !cgltf_find_accessor(p,cgltf_attribute_type_weights,0))
        return cgltf_find_accessor(p,cgltf_attribute_type_joints,1) || cgltf_find_accessor(p,cgltf_attribute_type_weights,1) ? XGE_ERROR_RESOURCE_FAILED : XGE_OK;
    if (cgltf_find_accessor(p,cgltf_attribute_type_joints,2) || cgltf_find_accessor(p,cgltf_attribute_type_weights,2))
        return XGE_ERROR_UNSUPPORTED;
    xarray values;xrtArrayInit(&values,sizeof(float));int result=XGE_OK;
    if (!xrtArrayResize(&values,vertices->Count*16)) return XGE_ERROR_OUT_OF_MEMORY;
    memset(values.Data,0,values.Count*sizeof(float));float *v=(float*)values.Data;
    for (int set=0;set<2;++set) for (int kind=0;kind<2;++kind) {
        const cgltf_accessor *a=cgltf_find_accessor(p,kind ? cgltf_attribute_type_weights : cgltf_attribute_type_joints,set);
        if (!a) {
            if (cgltf_find_accessor(p,kind ? cgltf_attribute_type_joints : cgltf_attribute_type_weights,set)) {result=XGE_ERROR_RESOURCE_FAILED;goto done;}
            continue;
        }
        if (a->type!=cgltf_type_vec4 || a->count!=vertices->Count ||
            (!kind && (a->normalized || (a->component_type!=cgltf_component_type_r_8u && a->component_type!=cgltf_component_type_r_16u))) ||
            (kind && a->component_type!=cgltf_component_type_r_32f &&
                (!a->normalized || (a->component_type!=cgltf_component_type_r_8u && a->component_type!=cgltf_component_type_r_16u)))) {
            result=XGE_ERROR_RESOURCE_FAILED;goto done;
        }
        size_t offset=(size_t)(set*2+kind)*vertices->Count*4;
        if (cgltf_accessor_unpack_floats(a,v+offset,vertices->Count*4)!=vertices->Count*4) {result=XGE_ERROR_RESOURCE_FAILED;goto done;}
    }
    for (size_t i=0;i<vertices->Count;++i) {
        xge3d_vertex_t *vertex=xrtArrayGet(vertices,i);float weights[8],sum=0;uint16_t joints[8];int active=0;
        for (int k=0;k<8;++k) {
            size_t offset=(size_t)(k/4)*vertices->Count*8+i*4+(size_t)k%4;
            float joint=v[offset],weight=v[offset+vertices->Count*4];
            if (!isfinite(joint) || joint<0 || joint>65535 || floorf(joint)!=joint || !isfinite(weight) || weight<0) {result=XGE_ERROR_RESOURCE_FAILED;goto done;}
            joints[k]=(uint16_t)joint;weights[k]=weight;if (weight>0) ++active;
        }
        if (active>4) ++*truncated;
        for (int k=0;k<4;++k) {
            int best=0;for (int b=1;b<8;++b) if (weights[b]>weights[best]) best=b;
            vertex->weights[k]=weights[best];vertex->joints[k]=weights[best]>0 ? joints[best] : 0;
            sum+=weights[best];weights[best]=0;
        }
        if (sum>0 && isfinite(sum)) for (int k=0;k<4;++k) vertex->weights[k]/=sum;
        else if (!isfinite(sum)) {result=XGE_ERROR_RESOURCE_FAILED;goto done;}
    }
done:
    xrtArrayUnit(&values);return result;
}
#endif
static int d3_primitive_load(const cgltf_primitive *p, d3_primitive *out,size_t *truncated)
{
    memset(out,0,sizeof(*out));
    if (p->type!=cgltf_primitive_type_triangles || p->has_draco_mesh_compression || p->targets_count)
        return XGE_ERROR_UNSUPPORTED;
    xarray vertices, indices;
    xrtArrayInit(&vertices,sizeof(xge3d_vertex_t)); xrtArrayInit(&indices,sizeof(uint32_t));
    int result=d3_attributes(p,&vertices);
    if (result!=XGE_OK) goto done;
#if XGE3D_ENABLE_ANIMATION
    result=d3_weights(p,&vertices,truncated);if (result!=XGE_OK) goto done;
#else
    (void)truncated;
#endif
    size_t count=p->indices ? p->indices->count : vertices.Count;
    if (!count || count%3 || count>INT_MAX) { result=XGE_ERROR_RESOURCE_FAILED; goto done; }
    if (p->indices) {
        if (p->indices->type!=cgltf_type_scalar || p->indices->is_sparse ||
            (p->indices->component_type!=cgltf_component_type_r_8u &&
             p->indices->component_type!=cgltf_component_type_r_16u &&
             p->indices->component_type!=cgltf_component_type_r_32u)) {
            result=XGE_ERROR_UNSUPPORTED; goto done;
        }
        if (!xrtArrayResize(&indices,count)) { result=XGE_ERROR_OUT_OF_MEMORY; goto done; }
        if (cgltf_accessor_unpack_indices(p->indices,indices.Data,4,count)!=count) {
            result=XGE_ERROR_RESOURCE_FAILED; goto done;
        }
        for (size_t i=0;i<count;++i) if (((uint32_t*)indices.Data)[i]>=vertices.Count) {
            result=XGE_ERROR_RESOURCE_FAILED; goto done;
        }
    }
    /* glTF without normals requires flat faces; preserve UVs when expanding. */
    if (!cgltf_find_accessor(p,cgltf_attribute_type_normal,0)) {
        xarray flat; xrtArrayInit(&flat,sizeof(xge3d_vertex_t));
        if (!xrtArrayResize(&flat,count)) { xrtArrayUnit(&flat); result=XGE_ERROR_OUT_OF_MEMORY; goto done; }
        for (size_t i=0;i<count;++i) {
            size_t index=indices.Count ? ((uint32_t*)indices.Data)[i] : i;
            ((xge3d_vertex_t*)flat.Data)[i]=((xge3d_vertex_t*)vertices.Data)[index];
        }
        for (size_t i=0;i<count;i+=3) {
            xge3d_vertex_t *v=(xge3d_vertex_t*)flat.Data+i;
            xge3d_vec3_t normal=d3_cross(d3_sub(v[1].position,v[0].position),d3_sub(v[2].position,v[0].position));
            d3_normalize(&normal);
            v[0].normal=v[1].normal=v[2].normal=normal;
        }
        xrtArrayUnit(&vertices); vertices=flat; xrtArrayClear(&indices);
    }
    xge3d_mesh_desc_t desc={(xge3d_vertex_t*)vertices.Data,vertices.Count,indices.Count ? indices.Data : NULL,
        indices.Count,indices.Count ? 32 : 0};
    result=xge3dMeshCreate(&desc,&out->mesh);
done:
    xrtArrayUnit(&vertices); xrtArrayUnit(&indices);
    return result;
}
static int d3_images(xge3d_model *m, const char *base,const d3_model_io *io)
{
    if (!xrtArrayResize(&m->images,m->data->images_count)) return XGE_ERROR_OUT_OF_MEMORY;
    if (m->images.Count) memset(m->images.Data,0,m->images.Count*m->images.ItemSize);
    for (size_t i=0;i<m->images.Count;++i) {
        if (d3_cancelled(io)) return XGE_ERROR;
        cgltf_image *source=&m->data->images[i];
        void *owned=NULL; const void *bytes=NULL; size_t size=0;
        int result=XGE_OK;
        if (source->buffer_view) {
            bytes=cgltf_buffer_view_data(source->buffer_view); size=source->buffer_view->size;
        } else if (source->uri) {
            result=d3_blob(base,source->uri,&owned,&size,io); bytes=owned;
        }
        if (result==XGE_OK) result=size && size<=INT_MAX && bytes ?
            xgeImageLoadMemoryEx(xrtArrayGet(&m->images,i),bytes,(int)size,XGE_IMAGE_STRAIGHT_ALPHA) : XGE_ERROR_RESOURCE_FAILED;
        xrtFree(owned);
        if (result!=XGE_OK) return result;
    }
    return XGE_OK;
}

static int d3_texture_binding(xge3d_model *m, const cgltf_texture_view *view,
    int role, xge3d_texture_binding_t *out)
{
    if (!view->texture) return XGE_OK;
    cgltf_texture *source=view->texture;
    if (!source->image) return XGE_ERROR_UNSUPPORTED;
    out->texcoord=view->has_transform && view->transform.has_texcoord ? view->transform.texcoord : view->texcoord;
    if (out->texcoord<0 || out->texcoord>1) return XGE_ERROR_UNSUPPORTED;
    if (view->has_transform) {
        memcpy(out->offset,view->transform.offset,sizeof(out->offset));
        memcpy(out->scale,view->transform.scale,sizeof(out->scale));
        out->rotation=view->transform.rotation;
    }
    uint64_t key=((uint64_t)cgltf_texture_index(m->data,source)<<3)|(unsigned)role;
    xbytesview bytes={(const unsigned char*)&key,sizeof(key)};
    out->texture=xrtMapGetPtr(&m->texture_cache,bytes);
    if (out->texture) return XGE_OK;
    xge3d_texture_desc_t desc={0};
    desc.image=xrtArrayGet(&m->images,cgltf_image_index(m->data,source->image));
    desc.srgb=role==XGE3D_MAP_BASE_COLOR || role==XGE3D_MAP_EMISSIVE;
    if (source->sampler) {
        cgltf_sampler *s=source->sampler;
        desc.sampler=(xge3d_sampler_t){s->min_filter,s->mag_filter,s->wrap_s,s->wrap_t};
    }
    int result=xge3dTextureCreate(&desc,&out->texture);
    if (result!=XGE_OK) return result;
    if (!xrtArrayPush(&m->textures,&out->texture)) {
        xge3dTextureFree(out->texture); out->texture=NULL; return XGE_ERROR_OUT_OF_MEMORY;
    }
    if (!xrtMapSetPtr(&m->texture_cache,bytes,out->texture)) return XGE_ERROR_OUT_OF_MEMORY;
    return XGE_OK;
}
static int d3_materials(xge3d_model *m)
{
    /* Last entry is the standard glTF material for primitives without a material. */
    for (size_t i=0;i<=m->data->materials_count;++i) {
        xge3d_material_desc_t desc=xge3dMaterialDefault();
        if (i<m->data->materials_count) {
            cgltf_material *s=&m->data->materials[i];
            memcpy(desc.base_color,s->pbr_metallic_roughness.base_color_factor,sizeof(desc.base_color));
            memcpy(desc.emissive,s->emissive_factor,sizeof(desc.emissive));
            desc.metallic=s->pbr_metallic_roughness.metallic_factor;
            desc.roughness=s->pbr_metallic_roughness.roughness_factor;
            desc.alpha_mode=(int)s->alpha_mode; desc.alpha_cutoff=s->alpha_cutoff;
            desc.double_sided=s->double_sided; desc.unlit=s->unlit;
            if (s->normal_texture.texture) desc.normal_scale=s->normal_texture.scale;
            if (s->occlusion_texture.texture) desc.occlusion_strength=s->occlusion_texture.scale;
            const cgltf_texture_view *views[]={&s->pbr_metallic_roughness.base_color_texture,
                &s->pbr_metallic_roughness.metallic_roughness_texture,&s->normal_texture,
                &s->occlusion_texture,&s->emissive_texture};
            for (int role=0;role<XGE3D_MAP_COUNT;++role) {
                int result=d3_texture_binding(m,views[role],role,&desc.maps[role]);
                if (result!=XGE_OK) return result;
            }
        }
        xge3d_material *material=NULL;
        int result=xge3dMaterialCreate(&desc,&material);
        if (result!=XGE_OK) return result;
        if (!xrtArrayPush(&m->materials,&material)) {
            xge3dMaterialFree(material); return XGE_ERROR_OUT_OF_MEMORY;
        }
    }
    /* Textures own copies; retain no second decoded copy of each source image. */
    for (size_t i=0;i<m->images.Count;++i) xgeImageFree(xrtArrayGet(&m->images,i));
    xrtArrayClear(&m->images);
    return XGE_OK;
}
void xge3dModelFree(xge3d_model *m)
{
    if (!m || --m->refs) return;
    for (size_t i=0;i<m->primitives.Count;++i)
        xge3dMeshFree(((d3_primitive*)xrtArrayGet(&m->primitives,i))->mesh);
    for (size_t i=0;i<m->images.Count;++i) xgeImageFree(xrtArrayGet(&m->images,i));
    for (size_t i=0;i<m->materials.Count;++i)
        xge3dMaterialFree(*(xge3d_material**)xrtArrayGet(&m->materials,i));
    for (size_t i=0;i<m->textures.Count;++i)
        xge3dTextureFree(*(xge3d_texture**)xrtArrayGet(&m->textures,i));
    xrtArrayUnit(&m->materials); xrtArrayUnit(&m->textures); xrtMapUnit(&m->texture_cache);
    xrtArrayUnit(&m->primitives); xrtArrayUnit(&m->ranges); xrtArrayUnit(&m->images);
    if(m->data)for(size_t i=0;i<m->data->buffers_count;++i)
        if(m->data->buffers[i].uri)xrtFree(m->data->buffers[i].data);
    cgltf_free(m->data); xgeResourceFree(&m->source); xrtFree(m);
}
struct xge3d_model_retirement {xge3d_model *model;uint64_t owner;size_t phase,index,operations,total;int completed;};
int xge3dModelRetireBegin(xge3d_model **owned,xge3d_model_retirement **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;
    if(!owned){return XGE_ERROR_INVALID_ARGUMENT;}if(!*owned)return XGE_OK;
    if(!__xge3dContext())return XGE_ERROR_INVALID_STATE;
    xge3d_model *m=*owned;if(m->retiring)return XGE_ERROR_INVALID_STATE;
    if(m->refs>1){xge3dModelFree(m);*owned=NULL;return XGE_OK;}
    xge3d_model_retirement *r=xrtMalloc(sizeof(*r));if(!r)return XGE_ERROR_OUT_OF_MEMORY;
    memset(r,0,sizeof(*r));r->model=m;r->owner=xrtThreadCurrentId();r->total=m->materials.Count+m->primitives.Count+m->textures.Count;
    m->retiring=1;*owned=NULL;*out=r;return XGE_OK;
}
int xge3dModelRetireStep(xge3d_model_retirement *r,const xge3d_release_budget_t *budget,int *completed)
{
    if(!completed){return XGE_ERROR_INVALID_ARGUMENT;}*completed=0;
    if(!r||!budget)return XGE_ERROR_INVALID_ARGUMENT;
    if(r->owner!=xrtThreadCurrentId()||!__xge3dContext())return XGE_ERROR_INVALID_STATE;
    if(r->completed){*completed=1;return XGE_OK;}
    uint64_t begin=xgeClockMicroseconds();size_t work=0;xge3d_model *m=r->model;
    while(r->phase<3){size_t count=r->phase==0?m->materials.Count:r->phase==1?m->primitives.Count:m->textures.Count;
        if(r->index==count){++r->phase;r->index=0;continue;}
        if(work==budget->operations||(work&&budget->microseconds&&xgeClockMicroseconds()-begin>=budget->microseconds))break;
        int result=XGE_OK;
        if(r->phase==0){xge3d_material **p=xrtArrayGet(&m->materials,r->index);xge3dMaterialFree(*p);*p=NULL;}
        else if(r->phase==1){d3_primitive *p=xrtArrayGet(&m->primitives,r->index);
            if(p->mesh&&p->mesh->refs>1){xge3dMeshFree(p->mesh);p->mesh=NULL;}else if(p->mesh)result=d3_mesh_detach_gpu(p->mesh);
        }else{xge3d_texture **p=xrtArrayGet(&m->textures,r->index);
            if(*p&&(*p)->refs>1){xge3dTextureFree(*p);*p=NULL;}else if(*p)result=d3_texture_detach_gpu(*p);
        }
        if(result){return result;}++r->index;++work;++r->operations;
    }
    r->completed=r->phase==3;*completed=r->completed;return XGE_OK;
}
int xge3dModelRetireInfo(const xge3d_model_retirement *r,xge3d_retirement_info_t *out)
{
    if(!r||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_retirement_info_t){.size=sizeof(*out),.operations=r->operations,.total_operations=r->total,.completed=r->completed};return XGE_OK;
}
int xge3dModelRetireFree(xge3d_model_retirement *r)
{
    if(!r){return XGE_OK;}if(!r->completed&&r->owner!=xrtThreadCurrentId())return XGE_ERROR_INVALID_STATE;
    xge3dModelFree(r->model);xrtFree(r);return XGE_OK;
}
static int d3_model_load(xge_resource_t *source, const char *uri, xge3d_model **out,const d3_model_io *io)
{
    xge3d_model *m=xrtMalloc(sizeof(*m));
    if (!m) return XGE_ERROR_OUT_OF_MEMORY;
    memset(m,0,sizeof(*m)); m->refs=1; m->source=*source; memset(source,0,sizeof(*source));
    xrtArrayInit(&m->primitives,sizeof(d3_primitive)); xrtArrayInit(&m->ranges,sizeof(d3_mesh_range));
    xrtArrayInit(&m->images,sizeof(xge_image_t));
    xrtArrayInit(&m->materials,sizeof(xge3d_material*));
    xrtArrayInit(&m->textures,sizeof(xge3d_texture*)); xrtMapInit(&m->texture_cache,sizeof(void*));
    cgltf_options options={0}; options.memory.alloc_func=d3_gltf_alloc; options.memory.free_func=d3_gltf_free;options.memory.user_data=m;
    const char *stage="parse";int result=d3_cancelled(io) ? XGE_ERROR : d3_gltf_result(cgltf_parse(&options,m->source.pData,(size_t)m->source.iSize,&m->data));
    if (result!=XGE_OK) goto done;
    stage="buffers";result=d3_buffers(m,uri,io); if (result!=XGE_OK) goto done;
    stage="validate";result=d3_validate(m->data); if (result!=XGE_OK) goto done;
    stage="images";result=d3_images(m,uri,io); if (result!=XGE_OK) goto done;
    stage="materials";result=d3_materials(m); if (result!=XGE_OK) goto done;stage="geometry";
    for (size_t i=0;i<m->data->meshes_count;++i) {
        cgltf_mesh *mesh=&m->data->meshes[i];
        d3_mesh_range range={m->primitives.Count,mesh->primitives_count};
        if (!xrtArrayPush(&m->ranges,&range)) { result=XGE_ERROR_OUT_OF_MEMORY; goto done; }
        for (size_t j=0;j<mesh->primitives_count;++j) {
            if (d3_cancelled(io)) {result=XGE_ERROR;goto done;}
            d3_primitive primitive;
            cgltf_material *material=mesh->primitives[j].material;
            size_t index=material ? cgltf_material_index(m->data,material) : m->data->materials_count;
            xge3d_material *shared=*(xge3d_material**)xrtArrayGet(&m->materials,index);
            for (int role=0;role<XGE3D_MAP_COUNT;++role) {
                xge3d_texture_binding_t *binding=&shared->desc.maps[role];
                if (binding->texture && !cgltf_find_accessor(&mesh->primitives[j],cgltf_attribute_type_texcoord,binding->texcoord)) {
                    result=XGE_ERROR_RESOURCE_FAILED; goto done;
                }
            }
            result=d3_primitive_load(&mesh->primitives[j],&primitive,&m->truncated_weight_vertices);
            if (result!=XGE_OK) goto done;
            primitive.material=shared;
            if (!xrtArrayPush(&m->primitives,&primitive)) {
                xge3dMeshFree(primitive.mesh); result=XGE_ERROR_OUT_OF_MEMORY; goto done;
            }
        }
    }
#if XGE3D_ENABLE_ANIMATION
    for (size_t i=0;i<m->data->nodes_count;++i) {
        const cgltf_node *n=&m->data->nodes[i];if (!n->skin || !n->mesh) continue;
        d3_mesh_range *range=xrtArrayGet(&m->ranges,cgltf_mesh_index(m->data,n->mesh));
        for (size_t j=0;j<range->count;++j) {
            xge3d_mesh_desc_t mesh;xge3dMeshGetData(((d3_primitive*)xrtArrayGet(&m->primitives,range->first+j))->mesh,&mesh);
            for (size_t v=0;v<mesh.vertex_count;++v) {
                float sum=0;
                for (int k=0;k<4;++k) {
                    sum+=mesh.vertices[v].weights[k];
                    if (mesh.vertices[v].joints[k]>=n->skin->joints_count) {result=XGE_ERROR_RESOURCE_FAILED;goto done;}
                }
                if (fabsf(sum-1)>.001f) {result=XGE_ERROR_RESOURCE_FAILED;goto done;}
            }
        }
    }
#endif
done:
    if (result==XGE_OK) *out=m; else {if(getenv("XGE3D_MODEL_TRACE"))fprintf(stderr,"MODEL_LOAD stage=%s result=%d uri=%s\n",stage,result,uri?uri:"");xge3dModelFree(m);}
    return result;
}
int xge3dModelLoad(const char *uri, xge3d_model **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (!uri) return XGE_ERROR_INVALID_ARGUMENT;
    xge_resource_t source={0}; int result=xgeResourceLoad(uri,&source);
    if (result==XGE_OK) result=d3_model_load(&source,uri,out,NULL);
    xgeResourceFree(&source);
    return result;
}
int xge3dModelLoadMemory(const void *bytes, size_t size, const char *base, xge3d_model **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (!bytes || !size || size>INT_MAX) return XGE_ERROR_INVALID_ARGUMENT;
    xge_resource_t source={0}; int result=xgeResourceLoadMemory(bytes,(int)size,&source);
    if (result==XGE_OK) result=d3_model_load(&source,base,out,NULL);
    xgeResourceFree(&source);
    return result;
}
#if XGE3D_ENABLE_ASYNC
int d3_model_load_io(const char *uri,const d3_model_io *io,xge3d_model **out)
{
    *out=NULL;xge_resource_t source={0};int result=d3_read(uri,&source,io);
    if (result==XGE_OK) result=d3_model_load(&source,uri,out,io);
    xgeResourceFree(&source);return result;
}
size_t d3_model_resource_count(const xge3d_model *m,int stage)
{return stage==0 ? m->textures.Count : stage==1 ? m->primitives.Count : 0;}
void *d3_model_resource(const xge3d_model *m,int stage,size_t i)
{
    if (i>=d3_model_resource_count(m,stage)) return NULL;
    if (stage==0) return *(xge3d_texture**)xrtArrayConstGet(&m->textures,i);
    return ((const d3_primitive*)xrtArrayConstGet(&m->primitives,i))->mesh;
}
uint64_t d3_model_upload_bytes(const xge3d_model *m)
{
    uint64_t bytes=0;
    for (size_t i=0;i<m->textures.Count;++i) bytes+=(*(xge3d_texture**)xrtArrayConstGet(&m->textures,i))->pixels.Count;
    for (size_t i=0;i<m->primitives.Count;++i) {
        const xge3d_mesh *mesh=((const d3_primitive*)xrtArrayConstGet(&m->primitives,i))->mesh;
        bytes+=mesh->vertices.Count*mesh->vertices.ItemSize+mesh->indices.Count*mesh->indices.ItemSize;
    }
    return bytes;
}
#endif
xge3d_model_info_t xge3dModelInfo(const xge3d_model *m)
{
    return m ? (xge3d_model_info_t){m->data->nodes_count,m->primitives.Count,m->data->materials_count,
        m->data->animations_count,m->data->skins_count,m->truncated_weight_vertices} : (xge3d_model_info_t){0};
}
struct cgltf_data *d3_model_data(const xge3d_model *m) { return m&&!m->retiring ? m->data : NULL; }
int d3_model_retain(xge3d_model *m) { if (!m || m->retiring || m->refs==UINT32_MAX) return 0;++m->refs;return 1; }
#if XGE3D_ENABLE_ANIMATION
size_t xge3dModelSkinJointNode(const xge3d_model *m,size_t skin,size_t joint)
{
    if (!m || m->retiring || skin>=m->data->skins_count || joint>=m->data->skins[skin].joints_count) return SIZE_MAX;
    return cgltf_node_index(m->data,m->data->skins[skin].joints[joint]);
}
#endif
const char *xge3dModelNodeName(const xge3d_model *m, size_t index)
{ return m && !m->retiring && index<m->data->nodes_count ? m->data->nodes[index].name : NULL; }
const xge3d_material *xge3dModelMaterial(const xge3d_model *m, size_t index)
{ return m && !m->retiring && index<m->data->materials_count ? *(xge3d_material**)xrtArrayConstGet(&m->materials,index) : NULL; }
static int d3_selected(const cgltf_data *data, const cgltf_node *node)
{
    while (node->parent) node=node->parent;
    const cgltf_scene *scene=data->scene ? data->scene : data->scenes_count ? &data->scenes[0] : NULL;
    if (!scene) return 1;
    for (size_t i=0;i<scene->nodes_count;++i) if (scene->nodes[i]==node) return 1;
    return 0;
}
#if XGE3D_ENABLE_CACHE
int d3_model_share_textures(xge3d_model *m,int (*intern)(xge3d_texture *,xge3d_texture **,void *),void *user)
{
    for(size_t i=0;i<m->textures.Count;++i){xge3d_texture **slot=xrtArrayGet(&m->textures,i),*old=*slot,*shared=NULL;int result=intern(old,&shared,user);if(result)return result;if(shared==old)continue;
        size_t references=1;for(size_t j=0;j<m->materials.Count;++j){xge3d_material *material=*(xge3d_material**)xrtArrayGet(&m->materials,j);for(int k=0;k<5;++k)references+=material->desc.maps[k].texture==old;}
        size_t retained=0;for(;retained<references;++retained)if(!d3_texture_retain(shared))break;
        if(retained!=references){while(retained--)xge3dTextureFree(shared);return XGE_ERROR_INVALID_STATE;}
        for(size_t j=0;j<m->materials.Count;++j){xge3d_material *material=*(xge3d_material**)xrtArrayGet(&m->materials,j);for(int k=0;k<5;++k)if(material->desc.maps[k].texture==old){material->desc.maps[k].texture=shared;xge3dTextureFree(old);}}
        xmapiter it;xrtMapIterBegin(&m->texture_cache,&it);void **value;while((value=xrtMapIterNext(&it,NULL)))if(*value==old)*value=shared;xrtMapIterEnd(&it);
        *slot=shared;xge3dTextureFree(old);
    }return XGE_OK;
}
int d3_model_cache_bytes(const xge3d_model *m,uint64_t *cpu,uint64_t *gpu)
{
    xge3d_asset_memory_t info={.size=sizeof(info)};int result=xge3dModelMemoryInfo(m,&info);if(result)return result;
    *cpu=info.cpu_storage_bytes;*gpu=info.gpu_required_bytes;
    for(size_t i=0;i<m->textures.Count;++i){xge3d_texture *t=*(xge3d_texture**)xrtArrayConstGet(&m->textures,i);*cpu-=sizeof(*t)+(uint64_t)t->pixels.Capacity*t->pixels.ItemSize;
        int w=t->width,h=t->height;*gpu-=(uint64_t)w*h*4;if(t->sampler.min_filter>=0x2700)while(w>1||h>1){w=w>1?w/2:1;h=h>1?h/2:1;*gpu-=(uint64_t)w*h*4;}}
    return XGE_OK;
}
#endif
#include "xge3d_instance.inc"
#endif
