#include "xge_clock.h"
#include "xge3d_internal.h"
#include "xge_gl.h"
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#if XGE_ENABLE_3D
#include "xge3d_shaders.h"

/* GL 3.3 / GLES 3.0 constants absent from the historical small loader. */
#define D3_DEPTH_COMPONENT24 0x81A6
#define D3_DRAW_FRAMEBUFFER 0x8CA9
#define D3_READ_FRAMEBUFFER 0x8CA8

typedef struct d3_draw {
    xge3d_node_data *node;
    xge3d_mesh *mesh;
    xge3d_mat4_t world;
    float depth;
    int transparent;
    xge3d_aabb_t bounds;
    int in_frustum, main_visible, shadow_visible;
    float determinant;
} d3_draw;
typedef struct d3_batch { d3_draw *draw;size_t count,offset; } d3_batch;
typedef struct d3_instance_palette { xarray matrices;GLuint texture;int columns,rows; } d3_instance_palette;
typedef struct d3_instance_uniforms { GLint sampler,enabled,offset,columns,vp; } d3_instance_uniforms;
typedef struct d3_compositor {
    xge_shader_t shader;GLuint fbo,color,input,depth;
    int width,height;GLenum depth_format;GLint image,importing;
#if XGE3D_ENABLE_FXAA
    GLint antialiasing;int mode;
#endif
} d3_compositor;
typedef struct d3_output {GLint fbo,viewport[4],read_buffer;int shared_depth,srgb;} d3_output;
#if XGE3D_ENABLE_SHADOW
#define D3_INSTANCE_PASSES 7
#else
#define D3_INSTANCE_PASSES 1
#endif
struct xge3d_renderer {
    xge3d_renderer_options_t options;xge3d_render_profile_t profile;
    d3_compositor composite;
    GLuint queries[16];uint64_t pending_queries[8],sequence,last_gpu_ns,last_gpu_sample;
    int active_query;
    xge_shader_t shader, sky_shader;
    GLuint sky_vao;
    GLint sky_inverse, sky_map, sky_exposure;
    GLint mvp, world, color, emissive, alpha_mode, alpha_cutoff, map_mask;
    GLint maps[5], texcoord[5], uv_transform[5], uv_rotation[5];
    GLint exposure;
#if XGE3D_ENABLE_FOG
    GLint fog_color, fog_range, fog_depth;
#endif
#if XGE3D_ENABLE_ANIMATION
    GLint skin_sampler, skin_enabled;
    size_t max_joints;
#if XGE3D_ENABLE_SHADOW
    GLint depth_skin_sampler, depth_skin_enabled;
#endif
#endif
#if XGE3D_ENABLE_LIGHTING
    GLint camera_position, light_count, unlit, material_params;
    GLint light_position[8], light_direction[8], light_color[8], light_cone[8];
#endif
#if XGE3D_ENABLE_SHADOW
    xge_shader_t depth_shader;
    GLuint shadow_texture, shadow_fbo;
    int shadow_resolution;
    GLint depth_mvp, depth_color, depth_alpha, depth_cutoff, depth_map, depth_has_map, depth_uv, depth_rotation, depth_texcoord;
    GLint shadow_atlas, shadow_matrices[6], shadow_light_map[8], shadow_sun, shadow_cascades;
    GLint shadow_splits, shadow_params, shadow_texel, shadow_view_depth, shadow_near;
    GLint receive_shadows;
#endif
#if XGE3D_ENABLE_IBL
    GLint ibl_irradiance, ibl_prefiltered, ibl_brdf, ibl_intensity, ibl_max_lod;
#endif
    xarray draws;
    xarray pass_draws, batches, matrices;
    d3_instance_palette instances[D3_INSTANCE_PASSES];
    d3_instance_uniforms instance_uniforms;
#if XGE3D_ENABLE_SHADOW
    d3_instance_uniforms depth_instance_uniforms;
#endif
    int texture_limit;
    uint64_t context;
};

#define D3_TEXTURE_UNITS 11
struct xge3d_target {
    xge_texture_t texture;
    GLuint fbo, depth;
    uint64_t context;
};

typedef struct d3_state {
    GLint viewport[4], program, vao, buffer, draw_fbo, read_fbo, renderbuffer;
    GLint active_texture, texture[D3_TEXTURE_UNITS], cube[D3_TEXTURE_UNITS], depth_func, depth_mask, cull_face, front_face;
    GLint blend[6], srgb, seamless;
    GLint enabled[5], color_mask[4];
    GLfloat clear_color[4], clear_depth;
} d3_state;
static const GLenum d3_caps[] = {GL_DEPTH_TEST,GL_BLEND,GL_CULL_FACE,GL_SCISSOR_TEST,GL_STENCIL_TEST};

static void d3_save(d3_state *s)
{
    xge_glGetIntegerv(0x0BA2,s->viewport);
    xge_glGetIntegerv(0x8B8D,&s->program);
    xge_glGetIntegerv(0x85B5,&s->vao);
    xge_glGetIntegerv(0x8894,&s->buffer);
    xge_glGetIntegerv(0x8CA6,&s->draw_fbo);
    xge_glGetIntegerv(0x8CAA,&s->read_fbo);
    xge_glGetIntegerv(0x8CA7,&s->renderbuffer);
    xge_glGetIntegerv(0x84E0,&s->active_texture);
    for (int i=0;i<D3_TEXTURE_UNITS;++i) {
        xge_glActiveTexture(GL_TEXTURE0+i);
        xge_glGetIntegerv(GL_TEXTURE_BINDING_2D,&s->texture[i]);
        xge_glGetIntegerv(0x8514,&s->cube[i]);
    }
    xge_glActiveTexture(GL_TEXTURE0);
    const GLenum blend_parameters[]={0x80C9,0x80C8,0x80CB,0x80CA,0x8009,0x883D};
    for (int i=0;i<6;++i) xge_glGetIntegerv(blend_parameters[i],&s->blend[i]);
    xge_graphics_mapping_t mapping;
    s->srgb=s->seamless=-1;
    if (xgeGraphicsMappingGet(XGE_GPU_BACKEND_NONE,&mapping)==XGE_OK && mapping.bOpenGLCore) {
        xge_glGetIntegerv(0x8DB9,&s->srgb);
        xge_glGetIntegerv(0x884F,&s->seamless);
    }
    xge_glGetIntegerv(0x0B74,&s->depth_func);
    xge_glGetIntegerv(0x0B72,&s->depth_mask);
    xge_glGetIntegerv(0x0B45,&s->cull_face);
    xge_glGetIntegerv(0x0B46,&s->front_face);
    xge_glGetIntegerv(0x0C23,s->color_mask);
    xge_glGetFloatv(0x0C22,s->clear_color);
    xge_glGetFloatv(0x0B73,&s->clear_depth);
    for (int i=0;i<5;++i) xge_glGetIntegerv(d3_caps[i],&s->enabled[i]);
}
static void d3_clear_depth(float depth)
{
    if (xge_glClearDepthf) xge_glClearDepthf(depth);
    else xge_glClearDepth(depth);
}
static void d3_restore(const d3_state *s)
{
    xge_glBindFramebuffer(D3_DRAW_FRAMEBUFFER,s->draw_fbo);
    xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,s->read_fbo);
    xge_glBindRenderbuffer(GL_RENDERBUFFER,s->renderbuffer);
    xge_glViewport(s->viewport[0],s->viewport[1],s->viewport[2],s->viewport[3]);
    xge_glUseProgram(s->program);
    xge_glBindVertexArray(s->vao);
    xge_glBindBuffer(GL_ARRAY_BUFFER,s->buffer);
    for (int i=0;i<D3_TEXTURE_UNITS;++i) {
        xge_glActiveTexture(GL_TEXTURE0+i);
        xge_glBindTexture(GL_TEXTURE_2D,s->texture[i]);
        xge_glBindTexture(0x8513,s->cube[i]);
    }
    xge_glActiveTexture(s->active_texture);
    xge_glBlendFuncSeparate(s->blend[0],s->blend[1],s->blend[2],s->blend[3]);
    xge_glBlendEquationSeparate(s->blend[4],s->blend[5]);
    if (s->srgb>=0) { if (s->srgb) xge_glEnable(0x8DB9); else xge_glDisable(0x8DB9); }
    if (s->seamless>=0) { if (s->seamless) xge_glEnable(0x884F); else xge_glDisable(0x884F); }
    xge_glDepthFunc(s->depth_func);
    xge_glDepthMask((GLboolean)s->depth_mask);
    xge_glCullFace(s->cull_face);
    xge_glFrontFace(s->front_face);
    xge_glColorMask((GLboolean)s->color_mask[0],(GLboolean)s->color_mask[1],
        (GLboolean)s->color_mask[2],(GLboolean)s->color_mask[3]);
    xge_glClearColor(s->clear_color[0],s->clear_color[1],s->clear_color[2],s->clear_color[3]);
    d3_clear_depth(s->clear_depth);
    for (int i=0;i<5;++i) {
        if (s->enabled[i]) xge_glEnable(d3_caps[i]); else xge_glDisable(d3_caps[i]);
    }
}
static int d3_ready(void)
{
    if (!__xge3dContext()) return XGE_ERROR_NOT_INITIALIZED;
    return xge_glDepthMask && xge_glGetFloatv && xge_glBlendFuncSeparate && xge_glBlendEquationSeparate && xge_glDrawArraysInstanced && xge_glDrawElementsInstanced &&
        xge_glGetFramebufferAttachmentParameteriv && xge_glBlitFramebuffer && xge_glReadBuffer && (xge_glClearDepth || xge_glClearDepthf)
        ? XGE_OK : XGE_ERROR_UNSUPPORTED;
}
static void d3_mesh_attributes(void)
{
    const GLint sizes[]={3,3,2,4,2};
    const size_t offsets[]={offsetof(xge3d_vertex_t,position),offsetof(xge3d_vertex_t,normal),
        offsetof(xge3d_vertex_t,uv),offsetof(xge3d_vertex_t,tangent),offsetof(xge3d_vertex_t,uv1)};
    for (GLuint i=0;i<5;++i) {
        xge_glEnableVertexAttribArray(i);
        xge_glVertexAttribPointer(i,sizes[i],GL_FLOAT,GL_FALSE,sizeof(xge3d_vertex_t),(const void*)offsets[i]);
    }
#if XGE3D_ENABLE_ANIMATION
    xge_glEnableVertexAttribArray(5);xge_glVertexAttribPointer(5,4,GL_UNSIGNED_SHORT,GL_FALSE,sizeof(xge3d_vertex_t),(const void*)offsetof(xge3d_vertex_t,joints));
    xge_glEnableVertexAttribArray(6);xge_glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(xge3d_vertex_t),(const void*)offsetof(xge3d_vertex_t,weights));
#endif
}
static int d3_mesh_upload(xge3d_mesh *m, uint64_t *bytes)
{
    int result=d3_ready();
    if (result!=XGE_OK) return result;
    if (m->context && m->context!=__xge3dContext()) return XGE_ERROR_INVALID_STATE;
    if (m->prepare_partial) return XGE_ERROR_INVALID_STATE;
    if (m->vao) return XGE_OK;
    d3_state state; d3_save(&state);
    xge_glGenVertexArrays(1,&m->vao);xge_glGenBuffers(1,&m->vbo);
    xge_glBindVertexArray(m->vao);xge_glBindBuffer(GL_ARRAY_BUFFER,m->vbo);
    size_t vb=m->vertices.Count*sizeof(xge3d_vertex_t),ib=m->indices.Count*m->indices.ItemSize;
    xge_glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)vb,m->vertices.Data,GL_STATIC_DRAW);d3_mesh_attributes();
    if (ib) {
        xge_glGenBuffers(1,&m->ibo);
        xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m->ibo);
        xge_glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)ib,m->indices.Data,GL_STATIC_DRAW);
    }
    GLenum error=xge_glGetError();
    m->context=__xge3dContext();
    if (!m->vao || !m->vbo || (ib && !m->ibo) || error!=GL_NO_ERROR) {
        if (m->vao) xge_glDeleteVertexArrays(1,&m->vao);
        if (m->vbo) xge_glDeleteBuffers(1,&m->vbo);
        if (m->ibo) xge_glDeleteBuffers(1,&m->ibo);
        m->vao=m->vbo=m->ibo=0;
        result=XGE_ERROR_GPU_FAILED;
    } else if (bytes) *bytes+=vb+ib;
    d3_restore(&state);
    return result;
}

#if XGE3D_ENABLE_ASYNC
int xge3dMeshPrepare(xge3d_mesh *m,const xge3d_upload_budget_t *budget,xge3d_upload_stats_t *out)
{
    xge3d_upload_stats_t stats={0};if(out)*out=stats;
    if(!m||!budget)return XGE_ERROR_INVALID_ARGUMENT;
    uint64_t context=__xge3dContext();if(m->context&&m->context!=context)return XGE_ERROR_INVALID_STATE;
    if(m->vao&&!m->prepare_partial){stats.completed=1;if(out)*out=stats;return XGE_OK;}
    if(!budget->bytes||!budget->operations)return XGE_OK;
    int result=d3_ready();if(result!=XGE_OK)return result;
    if(!xge_glBufferSubData)return XGE_ERROR_UNSUPPORTED;
    result=xgeFlush();if(result!=XGE_OK)return result;
    uint64_t now=xgeClockMicroseconds(),deadline=budget->microseconds?(UINT64_MAX-now<budget->microseconds?UINT64_MAX:now+budget->microseconds):0;
    size_t vb=m->vertices.Count*m->vertices.ItemSize,ib=m->indices.Count*m->indices.ItemSize,total=vb+ib;
    d3_state state;d3_save(&state);
    while(stats.operations<budget->operations&&(!deadline||xgeClockMicroseconds()<deadline)){
        size_t transferred=0;
        if(!m->vao){
            m->context=context;m->prepare_partial=1;m->prepare_offset=0;
            xge_glGenVertexArrays(1,&m->vao);xge_glGenBuffers(1,&m->vbo);
            xge_glBindVertexArray(m->vao);xge_glBindBuffer(GL_ARRAY_BUFFER,m->vbo);
            xge_glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)vb,NULL,GL_STATIC_DRAW);d3_mesh_attributes();
            if(ib){xge_glGenBuffers(1,&m->ibo);xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m->ibo);xge_glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)ib,NULL,GL_STATIC_DRAW);}
            if(!m->vao||!m->vbo||(ib&&!m->ibo))result=XGE_ERROR_GPU_FAILED;
        }else if(m->prepare_offset<total){
            uint64_t remaining=budget->bytes-stats.upload_bytes;if(!remaining){stats.min_next_bytes=1;break;}
            int vertex=m->prepare_offset<vb;size_t base=vertex?0:vb,end=vertex?vb:total;
            transferred=end-m->prepare_offset;if(transferred>remaining)transferred=(size_t)remaining;
            xge_glBindVertexArray(m->vao);GLenum target=vertex?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER;
            xge_glBindBuffer(target,vertex?m->vbo:m->ibo);
            const unsigned char *data=vertex?m->vertices.Data:m->indices.Data;
            xge_glBufferSubData(target,(GLintptr)(m->prepare_offset-base),(GLsizeiptr)transferred,data+m->prepare_offset-base);
        }else break;
        ++stats.operations;if(xge_glGetError()!=GL_NO_ERROR)result=XGE_ERROR_GPU_FAILED;
        if(result!=XGE_OK){break;}m->prepare_offset+=transferred;stats.upload_bytes+=transferred;
    }
    if(result!=XGE_OK){if(m->vao)xge_glDeleteVertexArrays(1,&m->vao);if(m->vbo)xge_glDeleteBuffers(1,&m->vbo);if(m->ibo)xge_glDeleteBuffers(1,&m->ibo);m->vao=m->vbo=m->ibo=0;m->context=0;m->prepare_offset=0;m->prepare_partial=0;stats.failed=1;}
    else if(m->vao&&m->prepare_offset==total){m->prepare_partial=0;stats.completed=1;}
    d3_restore(&state);if(out)*out=stats;return result;
}
int xge3dTexturePrepare(xge3d_texture *texture,const xge3d_upload_budget_t *budget,
    xge3d_upload_stats_t *out)
{
    xge3d_upload_stats_t stats={0};if(out)*out=stats;
    if(!texture||!budget)return XGE_ERROR_INVALID_ARGUMENT;
    uint64_t context=__xge3dContext();if(texture->context&&texture->context!=context)return XGE_ERROR_INVALID_STATE;
    if(texture->id&&!texture->prepare_partial){stats.completed=1;if(out)*out=stats;return XGE_OK;}
    if(!budget->bytes||!budget->operations)return XGE_OK;
    if(budget->bytes<4){stats.min_next_bytes=4;if(out)*out=stats;return XGE_OK;}
    int result=d3_ready();if(result!=XGE_OK)return result;
    if(!xge_glTexSubImage2D)return XGE_ERROR_UNSUPPORTED;
    result=xgeFlush();if(result!=XGE_OK)return result;
    uint64_t now=xgeClockMicroseconds(),deadline=budget->microseconds?(UINT64_MAX-now<budget->microseconds?UINT64_MAX:now+budget->microseconds):0;
    d3_state state;d3_save(&state);
    const GLenum unpack_keys[]={GL_UNPACK_ALIGNMENT,0x0CF2,0x0CF3,0x0CF4,0x88EF};GLint unpack[5];
    for(int i=0;i<5;++i)xge_glGetIntegerv(unpack_keys[i],&unpack[i]);
    xge_glPixelStorei(GL_UNPACK_ALIGNMENT,1);xge_glPixelStorei(0x0CF2,0);xge_glPixelStorei(0x0CF3,0);xge_glPixelStorei(0x0CF4,0);xge_glBindBuffer(0x88EC,0);xge_glActiveTexture(GL_TEXTURE0);
    size_t total=texture->pixels.Count;
    while(stats.operations<budget->operations&&(!deadline||xgeClockMicroseconds()<deadline)){
        size_t transferred=0;
        if(!texture->id){
            GLint maximum;xge_glGetIntegerv(0x0D33,&maximum);
            if(texture->width>maximum||texture->height>maximum){result=XGE_ERROR_UNSUPPORTED;break;}
            texture->context=context;texture->prepare_partial=1;texture->prepare_offset=0;
            xge_glGenTextures(1,&texture->id);xge_glBindTexture(GL_TEXTURE_2D,texture->id);
            xge_glTexImage2D(GL_TEXTURE_2D,0,texture->srgb?0x8C43:GL_RGBA8,texture->width,texture->height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
            xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,texture->sampler.min_filter);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,texture->sampler.mag_filter);
            xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,texture->sampler.wrap_u);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,texture->sampler.wrap_v);
            if(!texture->id)result=XGE_ERROR_GPU_FAILED;
        }else if(texture->prepare_offset<total){
            uint64_t remaining=budget->bytes-stats.upload_bytes;if(remaining<4){stats.min_next_bytes=4;break;}
            size_t row=(size_t)texture->width*4,y=texture->prepare_offset/row,x=(texture->prepare_offset%row)/4,width=texture->width-x,rows=1;
            if(!x&&remaining>=row){rows=(size_t)(remaining/row);if(rows>(size_t)texture->height-y)rows=texture->height-y;}
            else if(width>remaining/4)width=(size_t)(remaining/4);
            transferred=width*rows*4;xge_glBindTexture(GL_TEXTURE_2D,texture->id);
            xge_glTexSubImage2D(GL_TEXTURE_2D,0,(GLint)x,(GLint)y,(GLsizei)width,(GLsizei)rows,GL_RGBA,GL_UNSIGNED_BYTE,texture->pixels.Data+texture->prepare_offset);
        }else{
            if(texture->sampler.min_filter>=0x2700&&texture->sampler.min_filter<=0x2703){xge_glBindTexture(GL_TEXTURE_2D,texture->id);xge_glGenerateMipmap(GL_TEXTURE_2D);++stats.operations;}
            if(xge_glGetError()!=GL_NO_ERROR){result=XGE_ERROR_GPU_FAILED;break;}
            texture->prepare_partial=0;stats.completed=1;break;
        }
        ++stats.operations;if(xge_glGetError()!=GL_NO_ERROR)result=XGE_ERROR_GPU_FAILED;
        if(result!=XGE_OK){break;}texture->prepare_offset+=transferred;stats.upload_bytes+=transferred;
    }
    if(result!=XGE_OK){if(texture->id)xge_glDeleteTextures(1,&texture->id);texture->id=0;texture->context=0;texture->prepare_offset=0;texture->prepare_partial=0;stats.failed=1;}
    for(int i=0;i<4;++i){xge_glPixelStorei(unpack_keys[i],unpack[i]);}xge_glBindBuffer(0x88EC,unpack[4]);d3_restore(&state);
    if(out){*out=stats;}return result;
}
/* Private resources are not exposed until this cursor completes. Bytes count
 * transferred CPU payload, operations count storage, transfer or mip work. */
int d3_model_upload_step(xge3d_model *model,d3_upload_cursor *c,const xge3d_upload_budget_t *budget,
    xge3d_upload_stats_t *stats,uint64_t deadline,int *complete)
{
    *complete=0;
    if (!c->total) {*complete=1;return XGE_OK;}
    if (!budget->bytes || !budget->operations) return XGE_OK;
    int result=d3_ready();if (result!=XGE_OK) return result;
    if (!xge_glBufferSubData || !xge_glTexSubImage2D) return XGE_ERROR_UNSUPPORTED;
    uint64_t context=__xge3dContext();if (c->context && c->context!=context) return XGE_ERROR_INVALID_STATE;
    result=xgeFlush();if (result!=XGE_OK) return result;
    c->context=context;d3_state state;d3_save(&state);
    const GLenum unpack_keys[]={GL_UNPACK_ALIGNMENT,0x0CF2,0x0CF3,0x0CF4,0x88EF};GLint unpack[5];
    for (int i=0;i<5;++i) xge_glGetIntegerv(unpack_keys[i],&unpack[i]);
    xge_glPixelStorei(GL_UNPACK_ALIGNMENT,1);xge_glPixelStorei(0x0CF2,0);xge_glPixelStorei(0x0CF3,0);xge_glPixelStorei(0x0CF4,0);xge_glBindBuffer(0x88EC,0);
    xge_glActiveTexture(GL_TEXTURE0);
    while (c->stage<2) {
        if (c->resource>=d3_model_resource_count(model,c->stage)) {++c->stage;c->resource=c->offset=0;continue;}
        xge3d_texture *texture=c->stage==0 ? d3_model_resource(model,0,c->resource) : NULL;
        xge3d_mesh *mesh=c->stage==1 ? d3_model_resource(model,1,c->resource) : NULL;
        size_t vb=mesh ? mesh->vertices.Count*mesh->vertices.ItemSize : 0;
        size_t total=texture ? texture->pixels.Count : vb+mesh->indices.Count*mesh->indices.ItemSize;
        int mip=texture && texture->sampler.min_filter>=0x2700 && texture->sampler.min_filter<=0x2703;
        if (c->offset==total && !mip) {++c->resource;c->offset=0;continue;}
        if (stats->operations>=budget->operations || (deadline && xgeClockMicroseconds()>=deadline)) break;
        uint64_t remaining=budget->bytes-stats->upload_bytes,min=texture ? 4 : 1;
        if (c->offset<total && remaining<min) {stats->min_next_bytes=min;break;}
        size_t transferred=0;
        if (texture) {
            xge_glBindTexture(GL_TEXTURE_2D,texture->id);
            if (!texture->id) {
                if (texture->width>0 && (texture->width>INT_MAX/4 || texture->height<=0)) {result=XGE_ERROR_INVALID_ARGUMENT;break;}
                GLint maximum;xge_glGetIntegerv(0x0D33,&maximum);
                if (texture->width>maximum || texture->height>maximum) {result=XGE_ERROR_UNSUPPORTED;break;}
                texture->context=context;xge_glGenTextures(1,&texture->id);xge_glBindTexture(GL_TEXTURE_2D,texture->id);
                xge_glTexImage2D(GL_TEXTURE_2D,0,texture->srgb ? 0x8C43 : GL_RGBA8,texture->width,texture->height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
                xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,texture->sampler.min_filter);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,texture->sampler.mag_filter);
                xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,texture->sampler.wrap_u);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,texture->sampler.wrap_v);
                if (!texture->id) result=XGE_ERROR_GPU_FAILED;
            } else if (c->offset<total) {
                size_t row=(size_t)texture->width*4,y=c->offset/row,x=(c->offset%row)/4,width=texture->width-x,rows=1;
                if (!x && remaining>=row) {rows=remaining/row;if (rows>(size_t)texture->height-y) rows=texture->height-y;}
                else if (width>remaining/4) width=(size_t)(remaining/4);
                transferred=width*rows*4;
                xge_glTexSubImage2D(GL_TEXTURE_2D,0,(GLint)x,(GLint)y,(GLsizei)width,(GLsizei)rows,GL_RGBA,GL_UNSIGNED_BYTE,texture->pixels.Data+c->offset);
            } else {xge_glGenerateMipmap(GL_TEXTURE_2D);++c->resource;c->offset=0;}
        } else {
            if (!mesh->vao) {
                mesh->context=context;xge_glGenVertexArrays(1,&mesh->vao);xge_glGenBuffers(1,&mesh->vbo);
                xge_glBindVertexArray(mesh->vao);xge_glBindBuffer(GL_ARRAY_BUFFER,mesh->vbo);xge_glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)vb,NULL,GL_STATIC_DRAW);d3_mesh_attributes();
                if (mesh->indices.Count) {xge_glGenBuffers(1,&mesh->ibo);xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh->ibo);xge_glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)(total-vb),NULL,GL_STATIC_DRAW);}
                if (!mesh->vao || !mesh->vbo || (mesh->indices.Count && !mesh->ibo)) result=XGE_ERROR_GPU_FAILED;
            } else {
                xge_glBindVertexArray(mesh->vao);GLenum target=c->offset<vb ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
                xge_glBindBuffer(target,c->offset<vb ? mesh->vbo : mesh->ibo);
                size_t end=c->offset<vb ? vb : total;transferred=end-c->offset;if (transferred>remaining) transferred=(size_t)remaining;
                size_t offset=c->offset<vb ? c->offset : c->offset-vb;
                const unsigned char *data=c->offset<vb ? mesh->vertices.Data : mesh->indices.Data;
                xge_glBufferSubData(target,(GLintptr)offset,(GLsizeiptr)transferred,data+offset);
            }
        }
        ++stats->operations;
        if (xge_glGetError()!=GL_NO_ERROR) result=XGE_ERROR_GPU_FAILED;
        if (result!=XGE_OK) break;
        c->offset+=transferred;c->uploaded+=transferred;stats->upload_bytes+=transferred;
    }
    if (result==XGE_OK && c->stage==2) *complete=1;
    for (int i=0;i<4;++i) xge_glPixelStorei(unpack_keys[i],unpack[i]);
    xge_glBindBuffer(0x88EC,unpack[4]);
    d3_restore(&state);return result;
}
#endif

int d3_mesh_retain(xge3d_mesh *m)
{
    if (!m || m->refs==UINT32_MAX) return 0;
    ++m->refs;
    return 1;
}
void xge3dMeshFree(xge3d_mesh *m)
{
    if (!m || --m->refs) return;
#if XGE3D_ENABLE_COLLISION
    d3_triangle_bvh_free(m->collision_bvh);
#endif
    if (m->context && m->context==__xge3dContext()) {
        if (m->vao) xge_glDeleteVertexArrays(1,&m->vao);
        if (m->vbo) xge_glDeleteBuffers(1,&m->vbo);
        if (m->ibo) xge_glDeleteBuffers(1,&m->ibo);
    }
    xrtArrayUnit(&m->vertices); xrtArrayUnit(&m->indices);
#if XGE3D_ENABLE_ANIMATION
    xrtArrayUnit(&m->joint_bounds);
#endif
    xrtFree(m);
}
int d3_mesh_detach_gpu(xge3d_mesh *m)
{
    if(!m||m->refs!=1)return XGE_ERROR_INVALID_STATE;
    if(m->vao||m->vbo||m->ibo){
        if(!__xge3dContext()||m->context!=__xge3dContext())return XGE_ERROR_INVALID_STATE;
        if(m->vao)xge_glDeleteVertexArrays(1,&m->vao);
        if(m->vbo)xge_glDeleteBuffers(1,&m->vbo);
        if(m->ibo)xge_glDeleteBuffers(1,&m->ibo);
    }
    m->vao=m->vbo=m->ibo=0;m->context=0;m->prepare_offset=0;m->prepare_partial=0;return XGE_OK;
}
int xge3dMeshCreate(const xge3d_mesh_desc_t *d, xge3d_mesh **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (!d || !d->vertices || !d->vertex_count || d->vertex_count>INT_MAX ||
        d->vertex_count>SIZE_MAX/sizeof(xge3d_vertex_t) ||
        d->index_count>INT_MAX || (d->index_count && (!d->indices ||
        (d->index_bits!=16 && d->index_bits!=32))) ||
        (!d->index_count && (d->indices || d->index_bits)) ||
        (d->index_count ? d->index_count : d->vertex_count)%3)
        return XGE_ERROR_INVALID_ARGUMENT;
    for (size_t i=0;i<d->vertex_count;++i) {
        const xge3d_vertex_t *v=&d->vertices[i];
        if (!d3_finite3(v->position) || !d3_finite3(v->normal)) return XGE_ERROR_INVALID_ARGUMENT;
        for (int j=0;j<2;++j) if (!isfinite(v->uv[j]) || !isfinite(v->uv1[j])) return XGE_ERROR_INVALID_ARGUMENT;
        for (int j=0;j<4;++j) if (!isfinite(v->tangent[j])) return XGE_ERROR_INVALID_ARGUMENT;
#if XGE3D_ENABLE_ANIMATION
        for (int j=0;j<4;++j) if (!isfinite(v->weights[j]) || v->weights[j]<0) return XGE_ERROR_INVALID_ARGUMENT;
#endif
    }
    for (size_t i=0;i<d->index_count;++i) {
        uint32_t index=d->index_bits==16 ? ((const uint16_t*)d->indices)[i] : ((const uint32_t*)d->indices)[i];
        if (index>=d->vertex_count) return XGE_ERROR_INVALID_ARGUMENT;
    }
    xge3d_mesh *m=xrtMalloc(sizeof(*m));
    if (!m) return XGE_ERROR_OUT_OF_MEMORY;
    memset(m,0,sizeof(*m)); m->refs=1; m->index_bits=d->index_bits;
    m->version=1;m->bounds=(xge3d_aabb_t){d->vertices[0].position,d->vertices[0].position};
    xrtArrayInit(&m->vertices,sizeof(xge3d_vertex_t));
    xrtArrayInit(&m->indices,d->index_bits==16 ? 2 : 4);
#if XGE3D_ENABLE_ANIMATION
    xrtArrayInit(&m->joint_bounds,sizeof(d3_joint_bounds));size_t joints=0;
    for (size_t i=0;i<d->vertex_count;++i) for (int k=0;k<4;++k)
        if (d->vertices[i].weights[k]>0 && d->vertices[i].joints[k]<D3_MAX_JOINTS) joints=(size_t)fmax(joints,d->vertices[i].joints[k]+1);
    if (!xrtArrayResize(&m->joint_bounds,joints)) {xge3dMeshFree(m);return XGE_ERROR_OUT_OF_MEMORY;}
    if (joints) memset(m->joint_bounds.Data,0,joints*sizeof(d3_joint_bounds));
#endif
    if (!xrtArrayResize(&m->vertices,d->vertex_count) || !xrtArrayResize(&m->indices,d->index_count)) {
        xge3dMeshFree(m); return XGE_ERROR_OUT_OF_MEMORY;
    }
    memcpy(m->vertices.Data,d->vertices,d->vertex_count*sizeof(xge3d_vertex_t));
    for (size_t i=0;i<d->vertex_count;++i) {
        const xge3d_vertex_t *v=&d->vertices[i];d3_bounds_point(&m->bounds,v->position);
#if XGE3D_ENABLE_ANIMATION
        for (int k=0;k<4;++k) if (v->weights[k]>0 && v->joints[k]<m->joint_bounds.Count) {
            d3_joint_bounds *b=xrtArrayGet(&m->joint_bounds,v->joints[k]);
            if (!b->valid) {b->box=(xge3d_aabb_t){v->position,v->position};b->valid=1;}else d3_bounds_point(&b->box,v->position);
        }
#endif
    }
    if (d->index_count) memcpy(m->indices.Data,d->indices,d->index_count*m->indices.ItemSize);
    if (__xge3dContext()) {
        int result=d3_mesh_upload(m,NULL);
        if (result!=XGE_OK) { xge3dMeshFree(m); return result; }
    }
    *out=m;
    return XGE_OK;
}
int xge3dMeshResourceInfo(const xge3d_mesh *mesh,xge3d_resource_info_t *out)
{
    if(!mesh||!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_resource_info_t){.size=sizeof(*out),.vertices=mesh->vertices.Count,.indices=mesh->indices.Count,.references=mesh->refs};
    out->cpu_payload_bytes=mesh->vertices.Count*sizeof(xge3d_vertex_t)+mesh->indices.Count*(mesh->index_bits/8);
    if(mesh->vbo)out->gpu_payload_bytes+=mesh->vertices.Count*sizeof(xge3d_vertex_t);
    if(mesh->ibo){out->gpu_payload_bytes+=mesh->indices.Count*(mesh->index_bits/8);}return XGE_OK;
}
int xge3dMeshAddRef(xge3d_mesh *mesh)
{return !mesh?XGE_ERROR_INVALID_ARGUMENT:d3_mesh_retain(mesh)?XGE_OK:XGE_ERROR_INVALID_STATE;}
int xge3dMeshUpdate(xge3d_mesh *m, const xge3d_mesh_desc_t *desc)
{
    if (!m) return XGE_ERROR_INVALID_ARGUMENT;
    if (m->context && m->context!=__xge3dContext()) return XGE_ERROR_INVALID_STATE;
    xge3d_mesh *next=NULL;
    int result=xge3dMeshCreate(desc,&next);
    if (result!=XGE_OK) return result;
    xge3d_mesh previous=*m;
    uint32_t refs=m->refs;
    *m=*next; m->refs=refs;m->version=previous.version+1;
    xrtAtomic64FetchAdd(&d3_geometry_version,1,XMEMORY_RELEASE);
    *next=previous; next->refs=1;
    xge3dMeshFree(next);
    return XGE_OK;
}

int xge3dMeshGetData(const xge3d_mesh *m, xge3d_mesh_desc_t *out)
{
    if (!m || !out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_mesh_desc_t){(const xge3d_vertex_t*)m->vertices.Data,m->vertices.Count,
        m->indices.Count ? m->indices.Data : NULL,m->indices.Count,m->index_bits};
    return XGE_OK;
}

static int d3_program_create(xge_shader_t *shader, const char *vertex, const char *fragment)
{
    char header[128];
    int result=xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE,header,sizeof(header));
    if (result<0) return result;
    char *vs=xrtMalloc(strlen(header)+strlen(vertex)+1);
    char *fs=xrtMalloc(strlen(header)+strlen(fragment)+1);
    if (!vs || !fs) { xrtFree(vs); xrtFree(fs); return XGE_ERROR_OUT_OF_MEMORY; }
    strcpy(vs,header); strcat(vs,vertex); strcpy(fs,header); strcat(fs,fragment);
    result=xgeShaderCreate(shader,vs,fs);
    xrtFree(vs); xrtFree(fs); return result;
}

static d3_instance_uniforms d3_instance_locations(GLuint program)
{
    return (d3_instance_uniforms){xge_glGetUniformLocation(program,"instancePalette"),xge_glGetUniformLocation(program,"instanceEnabled"),
        xge_glGetUniformLocation(program,"instanceOffset"),xge_glGetUniformLocation(program,"instanceColumns"),xge_glGetUniformLocation(program,"viewProjection")};
}
#include "xge3d_composite.inc"
int xge3dRendererCreate(xge3d_renderer **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    int result=d3_ready();
    if (result!=XGE_OK) return result;
    xge3d_renderer *r=xrtMalloc(sizeof(*r));
    if (!r) return XGE_ERROR_OUT_OF_MEMORY;
    memset(r,0,sizeof(*r));r->options=xge3dRendererOptionsDefault();r->active_query=-1;
    xrtArrayInit(&r->draws,sizeof(d3_draw));
    xrtArrayInit(&r->pass_draws,sizeof(d3_draw*));xrtArrayInit(&r->batches,sizeof(d3_batch));xrtArrayInit(&r->matrices,sizeof(xge3d_mat4_t));
    for (int i=0;i<D3_INSTANCE_PASSES;++i) xrtArrayInit(&r->instances[i].matrices,sizeof(xge3d_mat4_t));
    xge_glGetIntegerv(0x0D33,&r->texture_limit);
    r->context=__xge3dContext();
#if XGE3D_ENABLE_ANIMATION
    GLint size;xge_glGetIntegerv(0x0D33,&size);r->max_joints=size<D3_MAX_JOINTS ? (size_t)size : D3_MAX_JOINTS;
#endif
    result=d3_program_create(&r->shader,d3_vertex_shader,d3_fragment_shader);
    if (result==XGE_OK) result=d3_program_create(&r->sky_shader,d3_sky_vertex_shader,d3_sky_fragment_shader);
    if (result==XGE_OK) result=d3_program_create(&r->composite.shader,d3_sky_vertex_shader,d3_composite_fragment_shader);
#if XGE3D_ENABLE_SHADOW
    if (result==XGE_OK && (!xge_glDrawBuffers || !xge_glReadBuffer)) result=XGE_ERROR_UNSUPPORTED;
    if (result==XGE_OK) result=d3_program_create(&r->depth_shader,d3_vertex_shader,d3_depth_fragment_shader);
#endif
    if (result==XGE_OK) {
        xge_glGenVertexArrays(1,&r->sky_vao);
        if (!r->sky_vao) result=XGE_ERROR_GPU_FAILED;
    }
    if (result!=XGE_OK) { xge3dRendererFree(r); return result; }
    r->instance_uniforms=d3_instance_locations(r->shader.iProgram);
#if XGE3D_ENABLE_SHADOW
    r->depth_instance_uniforms=d3_instance_locations(r->depth_shader.iProgram);
#endif
    r->sky_inverse=xge_glGetUniformLocation(r->sky_shader.iProgram,"skyInverse");
    r->sky_map=xge_glGetUniformLocation(r->sky_shader.iProgram,"sky");
    r->sky_exposure=xge_glGetUniformLocation(r->sky_shader.iProgram,"exposure");
    r->composite.image=xge_glGetUniformLocation(r->composite.shader.iProgram,"image");
    r->composite.importing=xge_glGetUniformLocation(r->composite.shader.iProgram,"importing");
#if XGE3D_ENABLE_FXAA
    r->composite.antialiasing=xge_glGetUniformLocation(r->composite.shader.iProgram,"antialiasing");
#endif
    r->mvp=xge_glGetUniformLocation(r->shader.iProgram,"mvp");
    r->color=xge_glGetUniformLocation(r->shader.iProgram,"color");
    r->world=xge_glGetUniformLocation(r->shader.iProgram,"world");
    r->emissive=xge_glGetUniformLocation(r->shader.iProgram,"emissive");
    r->alpha_mode=xge_glGetUniformLocation(r->shader.iProgram,"alphaMode");
    r->alpha_cutoff=xge_glGetUniformLocation(r->shader.iProgram,"alphaCutoff");
    r->map_mask=xge_glGetUniformLocation(r->shader.iProgram,"mapMask");
    r->exposure=xge_glGetUniformLocation(r->shader.iProgram,"exposure");
#if XGE3D_ENABLE_FOG
    r->fog_color=xge_glGetUniformLocation(r->shader.iProgram,"fogColor");
    r->fog_range=xge_glGetUniformLocation(r->shader.iProgram,"fogRange");
    r->fog_depth=xge_glGetUniformLocation(r->shader.iProgram,"fogDepth");
#endif
#if XGE3D_ENABLE_ANIMATION
    r->skin_sampler=xge_glGetUniformLocation(r->shader.iProgram,"jointPalette");r->skin_enabled=xge_glGetUniformLocation(r->shader.iProgram,"skinEnabled");
#if XGE3D_ENABLE_SHADOW
    r->depth_skin_sampler=xge_glGetUniformLocation(r->depth_shader.iProgram,"jointPalette");r->depth_skin_enabled=xge_glGetUniformLocation(r->depth_shader.iProgram,"skinEnabled");
#endif
#endif
#if XGE3D_ENABLE_IBL
    r->ibl_irradiance=xge_glGetUniformLocation(r->shader.iProgram,"irradianceMap");
    r->ibl_prefiltered=xge_glGetUniformLocation(r->shader.iProgram,"prefilteredMap");
    r->ibl_brdf=xge_glGetUniformLocation(r->shader.iProgram,"brdfMap");
    r->ibl_intensity=xge_glGetUniformLocation(r->shader.iProgram,"iblIntensity");
    r->ibl_max_lod=xge_glGetUniformLocation(r->shader.iProgram,"iblMaxLod");
#endif
#if XGE3D_ENABLE_LIGHTING
    r->camera_position=xge_glGetUniformLocation(r->shader.iProgram,"cameraPosition");
    r->light_count=xge_glGetUniformLocation(r->shader.iProgram,"lightCount");
    r->unlit=xge_glGetUniformLocation(r->shader.iProgram,"unlit");
    r->material_params=xge_glGetUniformLocation(r->shader.iProgram,"materialParams");
    for (int i=0;i<8;++i) {
        char name[32];
        snprintf(name,sizeof(name),"lightPosition[%d]",i); r->light_position[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
        snprintf(name,sizeof(name),"lightDirection[%d]",i); r->light_direction[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
        snprintf(name,sizeof(name),"lightColor[%d]",i); r->light_color[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
        snprintf(name,sizeof(name),"lightCone[%d]",i); r->light_cone[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
    }
#endif
#if XGE3D_ENABLE_SHADOW
    GLuint dp=r->depth_shader.iProgram,sp=r->shader.iProgram;
    r->depth_mvp=xge_glGetUniformLocation(dp,"mvp"); r->depth_color=xge_glGetUniformLocation(dp,"color");
    r->depth_alpha=xge_glGetUniformLocation(dp,"alphaMode"); r->depth_cutoff=xge_glGetUniformLocation(dp,"alphaCutoff");
    r->depth_map=xge_glGetUniformLocation(dp,"baseMap"); r->depth_has_map=xge_glGetUniformLocation(dp,"hasMap");
    r->depth_uv=xge_glGetUniformLocation(dp,"uvTransform"); r->depth_rotation=xge_glGetUniformLocation(dp,"uvRotation");
    r->depth_texcoord=xge_glGetUniformLocation(dp,"texcoord");
    r->shadow_atlas=xge_glGetUniformLocation(sp,"shadowAtlas"); r->shadow_sun=xge_glGetUniformLocation(sp,"sunIndex");
    r->receive_shadows=xge_glGetUniformLocation(sp,"receiveShadows");
    r->shadow_cascades=xge_glGetUniformLocation(sp,"cascadeCount"); r->shadow_splits=xge_glGetUniformLocation(sp,"cascadeSplits");
    r->shadow_params=xge_glGetUniformLocation(sp,"shadowParams"); r->shadow_texel=xge_glGetUniformLocation(sp,"atlasTexel");
    r->shadow_view_depth=xge_glGetUniformLocation(sp,"viewDepth"); r->shadow_near=xge_glGetUniformLocation(sp,"cameraNear");
    for (int i=0;i<6;++i) {
        char name[32]; snprintf(name,sizeof(name),"shadowMatrix[%d]",i);
        r->shadow_matrices[i]=xge_glGetUniformLocation(sp,name);
    }
    for (int i=0;i<8;++i) {
        char name[32]; snprintf(name,sizeof(name),"lightMap[%d]",i);
        r->shadow_light_map[i]=xge_glGetUniformLocation(sp,name);
    }
#endif
    for (int i=0;i<5;++i) {
        char name[32];
        snprintf(name,sizeof(name),"maps[%d]",i); r->maps[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
        snprintf(name,sizeof(name),"texcoord[%d]",i); r->texcoord[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
        snprintf(name,sizeof(name),"uvTransform[%d]",i); r->uv_transform[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
        snprintf(name,sizeof(name),"uvRotation[%d]",i); r->uv_rotation[i]=xge_glGetUniformLocation(r->shader.iProgram,name);
    }
    r->context=__xge3dContext();
    *out=r;
    return XGE_OK;
}
#if XGE3D_ENABLE_ANIMATION
size_t xge3dRendererMaxJoints(const xge3d_renderer *r) {return r ? r->max_joints : 0;}
#endif
void xge3dRendererFree(xge3d_renderer *r)
{
    if (!r) return;
    if (r->context==__xge3dContext()) {
        if(r->queries[0]&&xge_glDeleteQueries)xge_glDeleteQueries(16,r->queries);
        xgeShaderFree(&r->shader); xgeShaderFree(&r->sky_shader);
        d3_composite_free(&r->composite);xgeShaderFree(&r->composite.shader);
        if (r->sky_vao) xge_glDeleteVertexArrays(1,&r->sky_vao);
        for (int i=0;i<D3_INSTANCE_PASSES;++i) if (r->instances[i].texture) xge_glDeleteTextures(1,&r->instances[i].texture);
#if XGE3D_ENABLE_SHADOW
        xgeShaderFree(&r->depth_shader);
        if (r->shadow_texture) xge_glDeleteTextures(1,&r->shadow_texture);
        if (r->shadow_fbo) xge_glDeleteFramebuffers(1,&r->shadow_fbo);
#endif
    }
    xrtArrayUnit(&r->draws);
    xrtArrayUnit(&r->pass_draws);xrtArrayUnit(&r->batches);xrtArrayUnit(&r->matrices);
    for (int i=0;i<D3_INSTANCE_PASSES;++i) xrtArrayUnit(&r->instances[i].matrices);
    xrtFree(r);
}

int xge3dTargetCreate(int width, int height, xge3d_target **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (width<=0 || height<=0 || width>INT_MAX/4 || (size_t)width>SIZE_MAX/(size_t)height/4)
        return XGE_ERROR_INVALID_ARGUMENT;
    int result=d3_ready();
    if (result!=XGE_OK) return result;
    GLint limit=0;
    xge_glGetIntegerv(0x0D33,&limit);
    if (width>limit || height>limit) return XGE_ERROR_UNSUPPORTED;
    xge_glGetIntegerv(0x84E8,&limit);
    if (width>limit || height>limit) return XGE_ERROR_UNSUPPORTED;
    xge3d_target *t=xrtMalloc(sizeof(*t));
    if (!t) return XGE_ERROR_OUT_OF_MEMORY;
    memset(t,0,sizeof(*t)); t->context=__xge3dContext();
    d3_state state; d3_save(&state);
    result=xgeTextureCreateRGBAEx(&t->texture,width,height,NULL,XGE_TEXTURE_COMPRESS_NONE);
    if (result==XGE_OK) {
        t->texture.iFlags |= XGE_TEXTURE_ORIGIN_BOTTOM_LEFT | XGE_TEXTURE_RENDER_TARGET;
        xge_glGenFramebuffers(1,&t->fbo); xge_glBindFramebuffer(GL_FRAMEBUFFER,t->fbo);
        xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,t->texture.iBackendId,0);
        xge_glGenRenderbuffers(1,&t->depth); xge_glBindRenderbuffer(GL_RENDERBUFFER,t->depth);
        xge_glRenderbufferStorage(GL_RENDERBUFFER,D3_DEPTH_COMPONENT24,width,height);
        xge_glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,t->depth);
        if (!t->fbo || !t->depth || xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE ||
            xge_glGetError()!=GL_NO_ERROR) result=XGE_ERROR_GPU_FAILED;
    }
    if (result!=XGE_OK) xge3dTargetFree(t);
    d3_restore(&state);
    if (result==XGE_OK) *out=t;
    return result;
}
void xge3dTargetFree(xge3d_target *t)
{
    if (!t) return;
    if (t->context==__xge3dContext()) {
        if (t->fbo) xge_glDeleteFramebuffers(1,&t->fbo);
        if (t->depth) xge_glDeleteRenderbuffers(1,&t->depth);
    } else t->texture.iBackendId=0; /* Never delete a reused name in a later context. */
    xgeTextureFree(&t->texture);
    xrtFree(t);
}
int xge3dTargetResize(xge3d_target *t, int width, int height)
{
    if (!t) return XGE_ERROR_INVALID_ARGUMENT;
    if (t->context!=__xge3dContext()) return XGE_ERROR_INVALID_STATE;
    xge3d_target *next=NULL;
    int result=xge3dTargetCreate(width,height,&next);
    if (result!=XGE_OK) return result;
    xge3d_target previous=*t;
    *t=*next; *next=previous;
    xge3dTargetFree(next);
    return XGE_OK;
}
const xge_texture_t *xge3dTargetTexture(const xge3d_target *t)
{ return t ? &t->texture : NULL; }

int xge3dTargetReadPixels(xge3d_target *t, void *rgba, size_t size, int stride)
{
    if (!t || !rgba || stride<t->texture.iWidth*4 ||
        (size_t)stride>SIZE_MAX/(size_t)t->texture.iHeight || size<(size_t)stride*t->texture.iHeight)
        return XGE_ERROR_INVALID_ARGUMENT;
    if (t->context!=__xge3dContext()) return XGE_ERROR_INVALID_STATE;
    int width=t->texture.iWidth, height=t->texture.iHeight;
    GLint framebuffer, pack;
    xge_glGetIntegerv(0x8CAA,&framebuffer); xge_glGetIntegerv(0x0D05,&pack);
    xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,t->fbo); xge_glPixelStorei(0x0D05,1);
    for (int y=0;y<height;++y)
        xge_glReadPixels(0,height-1-y,width,1,GL_RGBA,GL_UNSIGNED_BYTE,(unsigned char*)rgba+(size_t)y*stride);
    int result=xge_glGetError()==GL_NO_ERROR ? XGE_OK : XGE_ERROR_GPU_FAILED;
    xge_glPixelStorei(0x0D05,pack); xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,framebuffer);
    return result;
}

static int d3_draw_compare(const void *a, const void *b)
{
    const d3_draw *x=a, *y=b;
    if (x->transparent!=y->transparent) return x->transparent-y->transparent;
    if (x->transparent) return x->depth<y->depth ? 1 : x->depth>y->depth ? -1 : 0;
    const xge3d_node_data *xn=x->node,*yn=y->node;
    if (x->mesh!=y->mesh) return (uintptr_t)x->mesh<(uintptr_t)y->mesh ? -1 : 1;
    if (xn->material!=yn->material) return (uintptr_t)xn->material<(uintptr_t)yn->material ? -1 : 1;
#if XGE3D_ENABLE_SHADOW
    if ((xn->shadow_flags&XGE3D_SHADOW_RECEIVE)!=(yn->shadow_flags&XGE3D_SHADOW_RECEIVE))
        return (xn->shadow_flags&XGE3D_SHADOW_RECEIVE)?1:-1;
#endif
    int color=memcmp(xn->color,yn->color,sizeof(xn->color));if (color) return color;
    return (x->determinant<0)-(y->determinant<0);
}

/* Group filtered draws in their existing material order. Skin and transparent
 * geometry keep individual submissions; no mesh VAO is modified for batches. */
static int d3_batch_compatible(const d3_draw *a,const d3_draw *b)
{
    if (a->transparent || b->transparent) return 0;
#if XGE3D_ENABLE_ANIMATION
    if (a->node->skin || b->node->skin) return 0;
#endif
    return d3_draw_compare(a,b)==0;
}
static int d3_batches_prepare(xge3d_renderer *r,xge3d_mat4_t vp,int pass,const xge3d_render_desc_t *desc,uint64_t *bytes)
{
    xrtArrayClear(&r->pass_draws);xrtArrayClear(&r->batches);xrtArrayClear(&r->matrices);
    for (size_t i=0;i<r->draws.Count;++i) {
        d3_draw *d=xrtArrayGet(&r->draws,i);
        if (fabsf(d->determinant)<1e-12f || (pass && d->transparent)) continue;
        if(pass?!d->shadow_visible:!d->main_visible)continue;
        if (!desc->disable_culling && (pass ? !d3_frustum_box(vp,d->bounds) : !d->in_frustum)) continue;
        if (!xrtArrayPush(&r->pass_draws,&d)) return XGE_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i=0;i<r->pass_draws.Count;) {
        d3_draw *d=*(d3_draw**)xrtArrayGet(&r->pass_draws,i);size_t end=i+1;
        if (!desc->disable_instancing) while (end<r->pass_draws.Count && d3_batch_compatible(d,*(d3_draw**)xrtArrayGet(&r->pass_draws,end))) ++end;
        d3_batch batch={d,end-i,r->matrices.Count};
        if (batch.count>1) for (size_t k=i;k<end;++k) {
            d3_draw *item=*(d3_draw**)xrtArrayGet(&r->pass_draws,k);
            if (!xrtArrayPush(&r->matrices,&item->world)) return XGE_ERROR_OUT_OF_MEMORY;
        }
        if (!xrtArrayPush(&r->batches,&batch)) return XGE_ERROR_OUT_OF_MEMORY;
        i=end;
    }
    if (!r->matrices.Count) return XGE_OK;
    d3_instance_palette *p=&r->instances[pass];
    int columns=r->matrices.Count<64 ? (int)r->matrices.Count : 64;
    size_t rows=(r->matrices.Count+columns-1)/columns;
    if (rows>(size_t)r->texture_limit || r->matrices.Count>INT_MAX) return XGE_ERROR_UNSUPPORTED;
    /* Padding is zeroed so identical batches compare and upload only once. */
    size_t count=r->matrices.Count,padded=rows*columns;
    if (!xrtArrayResize(&r->matrices,padded)) return XGE_ERROR_OUT_OF_MEMORY;
    if (padded>count) memset((xge3d_mat4_t*)r->matrices.Data+count,0,(padded-count)*sizeof(xge3d_mat4_t));
    size_t size=padded*sizeof(xge3d_mat4_t);
    xge_glActiveTexture(GL_TEXTURE0+10);
    if (p->texture && p->columns==columns && p->rows==(int)rows && p->matrices.Count==padded && !memcmp(p->matrices.Data,r->matrices.Data,size)) {
        xge_glBindTexture(GL_TEXTURE_2D,p->texture);return XGE_OK;
    }
    if (!xrtArrayResize(&p->matrices,padded)) return XGE_ERROR_OUT_OF_MEMORY;
    int resized=!p->texture || p->columns!=columns || p->rows!=(int)rows;
    GLuint texture=p->texture;
    if (resized) xge_glGenTextures(1,&texture);
    xge_glBindTexture(GL_TEXTURE_2D,texture);
    if (resized) {
        xge_glTexImage2D(GL_TEXTURE_2D,0,0x8814,columns*4,(GLsizei)rows,0,GL_RGBA,GL_FLOAT,r->matrices.Data);
        xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    } else xge_glTexSubImage2D(GL_TEXTURE_2D,0,0,0,columns*4,(GLsizei)rows,GL_RGBA,GL_FLOAT,r->matrices.Data);
    if (!texture || xge_glGetError()!=GL_NO_ERROR) {if (resized && texture) xge_glDeleteTextures(1,&texture);p->matrices.Count=0;return XGE_ERROR_GPU_FAILED;}
    if (resized && p->texture) xge_glDeleteTextures(1,&p->texture);
    p->texture=texture;p->columns=columns;p->rows=(int)rows;memcpy(p->matrices.Data,r->matrices.Data,size);*bytes+=size;return XGE_OK;
}
static void d3_instance_bind(xge3d_renderer *r,d3_instance_uniforms u,xge3d_mat4_t vp,int pass,const d3_batch *batch)
{
    xge_glUniform1i(u.sampler,10);xge_glUniform1i(u.enabled,batch->count>1);xge_glUniform1i(u.offset,(GLint)batch->offset);
    xge_glUniform1i(u.columns,r->instances[pass].columns ? r->instances[pass].columns : 1);xge_glUniformMatrix4fv(u.vp,1,GL_FALSE,vp.m);
}

static int d3_material_bind(xge3d_renderer *r, const xge3d_node_data *n, uint64_t *bytes)
{
    xge3d_material_desc_t fallback=xge3dMaterialDefault();
    fallback.unlit=1; /* A plain NodeSetColor mesh retains its flat-color path. */
    const xge3d_material_desc_t *m=n->material ? &n->material->desc : &fallback;
    xge_glUniform4f(r->color,m->base_color[0]*n->color[0],m->base_color[1]*n->color[1],
        m->base_color[2]*n->color[2],m->base_color[3]*n->color[3]);
    xge_glUniform3f(r->emissive,m->emissive[0],m->emissive[1],m->emissive[2]);
    xge_glUniform1i(r->alpha_mode,m->alpha_mode); xge_glUniform1f(r->alpha_cutoff,m->alpha_cutoff);
#if XGE3D_ENABLE_SHADOW
    xge_glUniform1i(r->receive_shadows,(n->shadow_flags&XGE3D_SHADOW_RECEIVE)!=0);
#endif
#if XGE3D_ENABLE_LIGHTING
    xge_glUniform1i(r->unlit,m->unlit);
    xge_glUniform4f(r->material_params,m->metallic,m->roughness,m->normal_scale,m->occlusion_strength);
#endif
    int mask=0;
    for (int i=0;i<5;++i) {
        const xge3d_texture_binding_t *b=&m->maps[i];
        xge_glActiveTexture(GL_TEXTURE0+i);
        if (b->texture) {
            int result=d3_texture_upload(b->texture,bytes);
            if (result!=XGE_OK) return result;
            mask|=1<<i;
        } else xge_glBindTexture(GL_TEXTURE_2D,0);
        xge_glUniform1i(r->maps[i],i); xge_glUniform1i(r->texcoord[i],b->texcoord);
        xge_glUniform4f(r->uv_transform[i],b->offset[0],b->offset[1],b->scale[0],b->scale[1]);
        xge_glUniform1f(r->uv_rotation[i],b->rotation);
    }
    xge_glUniform1i(r->map_mask,mask);
    if (m->double_sided) xge_glDisable(GL_CULL_FACE); else xge_glEnable(GL_CULL_FACE);
    if (m->alpha_mode==XGE3D_ALPHA_BLEND) {
        xge_glEnable(GL_BLEND); xge_glDepthMask(GL_FALSE);
        xge_glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        xge_glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
    } else { xge_glDisable(GL_BLEND); xge_glDepthMask(GL_TRUE); }
    return XGE_OK;
}

#if XGE3D_ENABLE_ANIMATION
static int d3_skin_bind(xge3d_scene *s,xge3d_node_data *node,GLint sampler,GLint enabled,uint64_t *bytes)
{
    int result=d3_skin_update(s,node);if (result!=XGE_OK) return result;
    xge_glActiveTexture(GL_TEXTURE0+9);
    if (node->skin) { result=d3_skin_upload(node->skin,bytes);if (result!=XGE_OK) return result; }
    else xge_glBindTexture(GL_TEXTURE_2D,0);
    xge_glUniform1i(sampler,9);xge_glUniform1i(enabled,node->skin!=NULL);return XGE_OK;
}
#endif

static void d3_mesh_draw(xge3d_mesh *m,size_t count,xge3d_render_stats_t *stats)
{
    xge_glBindVertexArray(m->vao);
    if (count>1) {
        if (m->indices.Count) xge_glDrawElementsInstanced(GL_TRIANGLES,(GLsizei)m->indices.Count,m->index_bits==16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,NULL,(GLsizei)count);
        else xge_glDrawArraysInstanced(GL_TRIANGLES,0,(GLsizei)m->vertices.Count,(GLsizei)count);
        ++stats->instanced_draw_calls;stats->instances+=count;
    } else if (m->indices.Count) xge_glDrawElements(GL_TRIANGLES,(GLsizei)m->indices.Count,
        m->index_bits==16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,NULL);
    else xge_glDrawArrays(GL_TRIANGLES,0,(GLsizei)m->vertices.Count);
    ++stats->draw_calls;stats->triangles+=count*(m->indices.Count ? m->indices.Count : m->vertices.Count)/3;
}

#if XGE3D_ENABLE_SHADOW
static int d3_shadow_target(xge3d_renderer *r, int resolution)
{
    if (r->shadow_resolution==resolution) return XGE_OK;
    GLint limit; xge_glGetIntegerv(0x0D33,&limit);
    if (resolution>limit/3) return XGE_ERROR_UNSUPPORTED;
    GLuint texture=0,fbo=0;
    xge_glActiveTexture(GL_TEXTURE0+5); xge_glGenTextures(1,&texture); xge_glBindTexture(GL_TEXTURE_2D,texture);
    xge_glTexImage2D(GL_TEXTURE_2D,0,D3_DEPTH_COMPONENT24,3*resolution,2*resolution,0,0x1902,GL_UNSIGNED_INT,NULL);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    xge_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    xge_glGenFramebuffers(1,&fbo); xge_glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,texture,0);
    GLenum none=GL_NONE; xge_glDrawBuffers(1,&none); xge_glReadBuffer(GL_NONE);
    if (!texture || !fbo || xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE || xge_glGetError()!=GL_NO_ERROR) {
        if (texture) xge_glDeleteTextures(1,&texture);
        if (fbo) xge_glDeleteFramebuffers(1,&fbo);
        return XGE_ERROR_GPU_FAILED;
    }
    if (r->shadow_texture) xge_glDeleteTextures(1,&r->shadow_texture);
    if (r->shadow_fbo) xge_glDeleteFramebuffers(1,&r->shadow_fbo);
    r->shadow_texture=texture; r->shadow_fbo=fbo; r->shadow_resolution=resolution; return XGE_OK;
}

static int d3_shadow_render(xge3d_renderer *r,xge3d_scene *scene,const xge3d_render_desc_t *d,const d3_light *lights,
    int light_count,xge3d_render_stats_t *counts)
{
    d3_shadow_frame frame;
    int result=d3_shadow_build(d->camera,d->shadows,lights,light_count,scene->origin,&frame);
    if (result!=XGE_OK) return result;
    if (frame.count) {
        GLint draw_fbo,read_fbo,viewport[4];
        xge_glGetIntegerv(0x8CA6,&draw_fbo);xge_glGetIntegerv(0x8CAA,&read_fbo);xge_glGetIntegerv(0x0BA2,viewport);
        result=d3_shadow_target(r,d->shadows->resolution); if (result!=XGE_OK) return result;
        xge_glBindFramebuffer(GL_FRAMEBUFFER,r->shadow_fbo); xge_glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
        xge_glDisable(GL_BLEND);xge_glDepthMask(GL_TRUE);xge_glDepthFunc(GL_LESS);d3_clear_depth(1);xge_glClear(GL_DEPTH_BUFFER_BIT);
        xge_glUseProgram(r->depth_shader.iProgram);xge_glUniform1i(r->depth_map,0);
        for (int map=0;map<frame.count;++map) {
            int size=r->shadow_resolution;xge_glViewport(map%3*size,map/3*size,size,size);
            result=d3_batches_prepare(r,frame.matrices[map],map+1,d,&counts->upload_bytes);if (result!=XGE_OK) return result;
            for (size_t i=0;i<r->batches.Count;++i) {
                d3_batch *batch=xrtArrayGet(&r->batches,i);d3_draw *draw=batch->draw;xge3d_node_data *n=draw->node;
                xge3d_material_desc_t fallback=xge3dMaterialDefault();
                const xge3d_material_desc_t *m=n->material ? &n->material->desc : &fallback;
                xge3d_mat4_t w=draw->world;
#if XGE3D_ENABLE_ANIMATION
                result=d3_skin_bind(scene,n,r->depth_skin_sampler,r->depth_skin_enabled,&counts->upload_bytes);if (result!=XGE_OK) return result;
#else
                (void)scene;
#endif
                result=d3_mesh_upload(draw->mesh,&counts->upload_bytes);if (result!=XGE_OK) return result;
                xge_glFrontFace(draw->determinant<0 ? GL_CW : GL_CCW);
                if (m->double_sided) xge_glDisable(GL_CULL_FACE);else xge_glEnable(GL_CULL_FACE);
                d3_instance_bind(r,r->depth_instance_uniforms,frame.matrices[map],map+1,batch);
                xge3d_mat4_t mvp=d3_mul(frame.matrices[map],w);xge_glUniformMatrix4fv(r->depth_mvp,1,GL_FALSE,mvp.m);
                xge_glUniform4f(r->depth_color,m->base_color[0],m->base_color[1],m->base_color[2],m->base_color[3]*n->color[3]);
                xge_glUniform1i(r->depth_alpha,m->alpha_mode);xge_glUniform1f(r->depth_cutoff,m->alpha_cutoff);
                const xge3d_texture_binding_t *b=&m->maps[0];xge_glActiveTexture(GL_TEXTURE0);
                if (b->texture) { result=d3_texture_upload(b->texture,&counts->upload_bytes);if (result!=XGE_OK) return result; }
                else xge_glBindTexture(GL_TEXTURE_2D,0);
                xge_glUniform1i(r->depth_has_map,b->texture!=NULL);xge_glUniform1i(r->depth_texcoord,b->texcoord);
                xge_glUniform4f(r->depth_uv,b->offset[0],b->offset[1],b->scale[0],b->scale[1]);xge_glUniform1f(r->depth_rotation,b->rotation);
                d3_mesh_draw(draw->mesh,batch->count,counts);++counts->shadow_draw_calls;
            }
        }
        xge_glBindFramebuffer(D3_DRAW_FRAMEBUFFER,draw_fbo);xge_glBindFramebuffer(D3_READ_FRAMEBUFFER,read_fbo);
        xge_glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);xge_glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
        counts->shadow_maps=frame.count;
    }
    xge_glUseProgram(r->shader.iProgram);xge_glActiveTexture(GL_TEXTURE0+5);
    xge_glBindTexture(GL_TEXTURE_2D,frame.count ? r->shadow_texture : 0);xge_glUniform1i(r->shadow_atlas,5);
    for (int i=0;i<8;++i) xge_glUniform1i(r->shadow_light_map[i],frame.light_map[i]);
    xge_glUniform1i(r->shadow_sun,frame.sun);xge_glUniform1i(r->shadow_cascades,frame.cascades);
    for (int i=0;i<frame.count;++i) xge_glUniformMatrix4fv(r->shadow_matrices[i],1,GL_FALSE,frame.matrices[i].m);
    xge_glUniform4f(r->shadow_splits,frame.splits[0],frame.splits[1],frame.splits[2],frame.splits[3]);
    if (d->shadows) xge_glUniform4f(r->shadow_params,d->shadows->bias,d->shadows->normal_bias,d->shadows->filter_radius,d->shadows->blend);
    if (frame.count) xge_glUniform2f(r->shadow_texel,1.0f/(r->shadow_resolution*3),1.0f/(r->shadow_resolution*2));
    const float *v=d->camera->view.m;xge_glUniform4f(r->shadow_view_depth,-v[2],-v[6],-v[10],-v[14]);
    xge_glUniform1f(r->shadow_near,d->camera->near_z);return XGE_OK;
}
#endif

#if XGE3D_ENABLE_FOG
xge3d_fog_settings_t xge3dFogDefault(void)
{ return (xge3d_fog_settings_t){{.57f,.73f,.85f},60,300}; }
#endif
#include "xge3d_profile.inc"

int xge3dRender(xge3d_renderer *r, xge3d_scene *s, const xge3d_render_desc_t *d, xge3d_render_stats_t *stats)
{
    if (stats) memset(stats,0,sizeof(*stats));
    if (!r || !s || !d || !d->camera || !d3_finite_matrix(&d->camera->view) ||
        !d3_finite_matrix(&d->camera->projection) || (d->clear_flags&~3u))
        return XGE_ERROR_INVALID_ARGUMENT;
    for (int i=0;i<4;++i) if (!isfinite(d->clear_color[i])) return XGE_ERROR_INVALID_ARGUMENT;
    if (!isfinite(d->exposure) || d->exposure<0) return XGE_ERROR_INVALID_ARGUMENT;
#if XGE3D_ENABLE_FOG
    if (d->fog && (!d3_finite3(d->fog->color) || d->fog->color.x<0 || d->fog->color.y<0 || d->fog->color.z<0 ||
        !isfinite(d->fog->start) || !isfinite(d->fog->end) || d->fog->start<0 || d->fog->end<=d->fog->start))
        return XGE_ERROR_INVALID_ARGUMENT;
#endif
#if XGE3D_ENABLE_IBL
    if (!isfinite(d->ibl_intensity) || d->ibl_intensity<0 ||
        (d->ibl_intensity>0 && (!d->environment || !d->environment->brdf))) return XGE_ERROR_INVALID_ARGUMENT;
#endif
    if (r->context!=__xge3dContext() || (d->target && d->target->context!=r->context))
        return XGE_ERROR_INVALID_STATE;
    int result=xgeFlush();
    if (result!=XGE_OK) return result;
    uint64_t profile_start=xgeClockMicroseconds(),collect_start=0,submit_start=0;
    memset(&r->profile,0,sizeof(r->profile));r->profile.size=sizeof(r->profile);d3_profile_begin(r);
    d3_state state; d3_save(&state);
    xge3d_render_stats_t counts={0};
    d3_output output={0};int composite_started=0;
    if (d->target) {
        xge_glBindFramebuffer(GL_FRAMEBUFFER,d->target->fbo);
        xge_glViewport(0,0,d->target->texture.iWidth,d->target->texture.iHeight);
    }
    GLint depth_bits=0, depth_type=GL_NONE;
    GLenum attachment=d->target || state.draw_fbo ? GL_DEPTH_ATTACHMENT : 0x1801;
    xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8CD0,&depth_type);
    if (depth_type!=GL_NONE)
        xge_glGetFramebufferAttachmentParameteriv(D3_DRAW_FRAMEBUFFER,attachment,0x8216,&depth_bits);
    if (!depth_bits) { result=XGE_ERROR_UNSUPPORTED; goto done; }
    result=d3_composite_begin(r,d,&output,&state);if(result!=XGE_OK)goto done;composite_started=1;
    xge_glEnable(GL_DEPTH_TEST); xge_glDepthFunc(GL_LESS); xge_glDepthMask(GL_TRUE);
    xge_glEnable(GL_CULL_FACE); xge_glCullFace(GL_BACK);
    if (state.srgb>=0) xge_glDisable(0x8DB9); /* Shader writes final sRGB bytes. */
    if (state.seamless>=0) xge_glEnable(0x884F);
    xge_glDisable(GL_BLEND); xge_glDisable(GL_SCISSOR_TEST); xge_glDisable(GL_STENCIL_TEST);
    xge_glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    float clear_alpha=fminf(1,fmaxf(0,d->clear_color[3]));
    xge_glClearColor(d3_decode_clear(d->clear_color[0],clear_alpha),d3_decode_clear(d->clear_color[1],clear_alpha),
        d3_decode_clear(d->clear_color[2],clear_alpha),clear_alpha);
    d3_clear_depth(1);
    GLbitfield clear=0;
    if (d->clear_flags&XGE3D_CLEAR_COLOR) clear|=GL_COLOR_BUFFER_BIT;
    if (d->clear_flags&XGE3D_CLEAR_DEPTH) clear|=GL_DEPTH_BUFFER_BIT;
    if (clear) xge_glClear(clear);
    if (d->environment) {
        xge3d_mat4_t rotation=d->camera->view,inverse;
        rotation.m[12]=rotation.m[13]=rotation.m[14]=0;
        if (!d3_inverse(d3_mul(d->camera->projection,rotation),&inverse)) {
            result=XGE_ERROR_INVALID_ARGUMENT; goto done;
        }
        xge_glActiveTexture(GL_TEXTURE0);
        result=d3_environment_upload(d->environment,&counts.upload_bytes);
        if (result!=XGE_OK) goto done;
        xge_glUseProgram(r->sky_shader.iProgram); xge_glBindVertexArray(r->sky_vao);
        xge_glUniformMatrix4fv(r->sky_inverse,1,GL_FALSE,inverse.m); xge_glUniform1i(r->sky_map,0);
        xge_glUniform1f(r->sky_exposure,d->exposure>0 ? d->exposure : 1);
        xge_glDisable(GL_CULL_FACE); xge_glDepthMask(GL_FALSE); xge_glDepthFunc(GL_LEQUAL);
        xge_glDrawArrays(GL_TRIANGLES,0,3);
        xge_glEnable(GL_CULL_FACE); xge_glDepthMask(GL_TRUE); xge_glDepthFunc(GL_LESS);
        ++counts.draw_calls;
    }
    xge3d_mat4_t vp=d3_mul(d->camera->projection,d->camera->view);
    xge3d_mat4_t inverse_view;
    if (!d3_inverse(d->camera->view,&inverse_view)) { result=XGE_ERROR_INVALID_ARGUMENT; goto done; }
    xge3d_vec3_t eye={inverse_view.m[12],inverse_view.m[13],inverse_view.m[14]};
    xge_glUseProgram(r->shader.iProgram);
    xge_glUniform1f(r->exposure,d->exposure>0 ? d->exposure : 1);
#if XGE3D_ENABLE_FOG
    xge_glUniform4f(r->fog_color,d->fog ? d->fog->color.x : 0,d->fog ? d->fog->color.y : 0,d->fog ? d->fog->color.z : 0,d->fog!=NULL);
    xge_glUniform2f(r->fog_range,d->fog ? d->fog->start : 0,d->fog ? d->fog->end : 1);
    const float *fog_view=d->camera->view.m;
    xge_glUniform4f(r->fog_depth,-fog_view[2],-fog_view[6],-fog_view[10],-fog_view[14]);
#endif
#if XGE3D_ENABLE_LIGHTING
    if (d->camera->projection.m[15]!=0 && d->camera->projection.m[11]==0)
        xge_glUniform4f(r->camera_position,inverse_view.m[8],inverse_view.m[9],inverse_view.m[10],1);
    else xge_glUniform4f(r->camera_position,inverse_view.m[12],inverse_view.m[13],inverse_view.m[14],0);
    collect_start=xgeClockMicroseconds();
    int light_count=0;d3_light lights[8];double scores[8]={0};
    for(size_t i=0;i<s->nodes.Count;++i){
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i),*p=n;
        if(!n->has_light||!n->light.intensity||!(n->render_layers&r->options.render_layers))continue;
        for(;p&&p->visible;p=p->parent_node){}if(p)continue;
        ++r->profile.candidate_lights;xge3d_mat4_t world;
        result=xge3dNodeGetWorldMatrix(s,n->handle,&world);if(result)goto done;
        const xge3d_light_desc_t *light=&n->light;
        xge3d_vec3_t position={world.m[12],world.m[13],world.m[14]};
        if(light->type!=XGE3D_LIGHT_DIRECTIONAL&&light->range>0){
            xge3d_vec3_t radius={light->range,light->range,light->range};
            if(!d3_frustum_box(vp,(xge3d_aabb_t){d3_sub(position,radius),d3_add(position,radius)}))continue;}
        double dx=(double)position.x-eye.x,dy=(double)position.y-eye.y,dz=(double)position.z-eye.z;
        double score=light->type==XGE3D_LIGHT_DIRECTIONAL?light->intensity:light->intensity/(1+dx*dx+dy*dy+dz*dz);
        int slot=light_count;
        if(light_count==(int)r->options.max_lights){slot=0;for(int j=1;j<light_count;++j){
                int a=lights[j].desc.type==XGE3D_LIGHT_DIRECTIONAL,b=lights[slot].desc.type==XGE3D_LIGHT_DIRECTIONAL;
                if(a<b||(a==b&&scores[j]<scores[slot]))slot=j;}
            int a=light->type==XGE3D_LIGHT_DIRECTIONAL,b=lights[slot].desc.type==XGE3D_LIGHT_DIRECTIONAL;
            if(a<b||(a==b&&score<=scores[slot]))continue;}
        else ++light_count;
        xge3d_vec3_t q=light->direction,direction={world.m[0]*q.x+world.m[4]*q.y+world.m[8]*q.z,
            world.m[1]*q.x+world.m[5]*q.y+world.m[9]*q.z,world.m[2]*q.x+world.m[6]*q.y+world.m[10]*q.z};
        if(light->type!=XGE3D_LIGHT_POINT&&!d3_normalize(&direction)){result=XGE_ERROR_INVALID_ARGUMENT;goto done;}
        lights[slot]=(d3_light){*light,position,direction};scores[slot]=score;
    }
    r->profile.selected_lights=(uint32_t)light_count;r->profile.skipped_lights=r->profile.candidate_lights-(uint32_t)light_count;
#if XGE3D_ENABLE_SHADOW
    int shadow_suns=0,shadow_spots=0;
#endif
    for(int i=0;i<light_count;++i){d3_light *l=&lights[i];
#if XGE3D_ENABLE_SHADOW
        if(l->desc.casts_shadow){if(l->desc.type==XGE3D_LIGHT_DIRECTIONAL){if(shadow_suns++)l->desc.casts_shadow=0;}
            else if(l->desc.type==XGE3D_LIGHT_SPOT){if(shadow_spots++>=2)l->desc.casts_shadow=0;}}
#endif
        xge_glUniform4f(r->light_position[i],l->position.x,l->position.y,l->position.z,(float)l->desc.type);
        xge_glUniform4f(r->light_direction[i],l->direction.x,l->direction.y,l->direction.z,l->desc.range);
        xge_glUniform4f(r->light_color[i],l->desc.color.x*l->desc.intensity,l->desc.color.y*l->desc.intensity,l->desc.color.z*l->desc.intensity,0);
        xge_glUniform2f(r->light_cone[i],cosf(l->desc.inner_angle),cosf(l->desc.outer_angle));
    }
    xge_glUniform1i(r->light_count,light_count);
#endif
    xrtArrayClear(&r->draws);
    for (size_t i=0;i<s->nodes.Count;++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i), *p=n;
        if (!n->mesh) continue;
        for (;p && p->visible;p=p->parent_node) {}
        if (p) continue;
        d3_draw draw={0}; draw.node=n;
        draw.main_visible=(n->render_layers&r->options.render_layers)!=0;
        draw.shadow_visible=(n->shadow_layers&r->options.shadow_layers)!=0;
#if XGE3D_ENABLE_SHADOW
        draw.shadow_visible=draw.shadow_visible&&(n->shadow_flags&XGE3D_SHADOW_CAST)!=0;
#endif
        if(!draw.main_visible&&!draw.shadow_visible)continue;
        result=xge3dNodeGetWorldMatrix(s,n->handle,&draw.world);
        if (result!=XGE_OK) goto done;
        draw.mesh=n->mesh;
        if (n->lods) {
            const float *m=draw.world.m;
            double x=(double)m[12]-eye.x,y=(double)m[13]-eye.y,z=(double)m[14]-eye.z,distance=sqrt(x*x+y*y+z*z);
            for (size_t k=0;k<n->lods->count && distance>=n->lods->levels[k].min_distance;++k) draw.mesh=n->lods->levels[k].mesh;
        }
        const float *w=draw.world.m;
        draw.determinant=d3_dot((xge3d_vec3_t){w[0],w[1],w[2]},d3_cross((xge3d_vec3_t){w[4],w[5],w[6]},(xge3d_vec3_t){w[8],w[9],w[10]}));
        if (!isfinite(draw.determinant)) {result=XGE_ERROR_INVALID_ARGUMENT;goto done;}
        result=d3_node_bounds(s,n,&draw.bounds);if (result!=XGE_OK) goto done;
        draw.in_frustum=draw.main_visible&&(d->disable_culling || d3_frustum_box(vp,draw.bounds));
        if (draw.in_frustum) ++counts.visible_meshes;else ++counts.culled_meshes;
        draw.transparent=n->material && n->material->desc.alpha_mode==XGE3D_ALPHA_BLEND;
        xge3d_mat4_t view_world=d3_mul(d->camera->view,draw.world);
        draw.depth=-view_world.m[14];
        if (!xrtArrayPush(&r->draws,&draw)) { result=XGE_ERROR_OUT_OF_MEMORY; goto done; }
    }
    xrtArraySort(&r->draws,d3_draw_compare);
    if(!collect_start)collect_start=profile_start;
    r->profile.collect_cpu_us=xgeClockMicroseconds()-collect_start;
    uint64_t shadow_start=xgeClockMicroseconds();
#if XGE3D_ENABLE_SHADOW
    result=d3_shadow_render(r,s,d,lights,light_count,&counts);if (result!=XGE_OK) goto done;
#endif
    r->profile.shadow_cpu_us=xgeClockMicroseconds()-shadow_start;submit_start=xgeClockMicroseconds();
#if XGE3D_ENABLE_IBL
    xge_glActiveTexture(GL_TEXTURE0+6);
    if (d->ibl_intensity>0) { result=d3_cube_upload(&d->environment->irradiance,&counts.upload_bytes);if (result!=XGE_OK) goto done; }
    else xge_glBindTexture(0x8513,0);
    xge_glActiveTexture(GL_TEXTURE0+7);
    if (d->ibl_intensity>0) { result=d3_cube_upload(&d->environment->prefiltered,&counts.upload_bytes);if (result!=XGE_OK) goto done; }
    else xge_glBindTexture(0x8513,0);
    xge_glActiveTexture(GL_TEXTURE0+8);
    if (d->ibl_intensity>0) { result=d3_texture_upload(d->environment->brdf,&counts.upload_bytes);if (result!=XGE_OK) goto done; }
    else xge_glBindTexture(GL_TEXTURE_2D,0);
    xge_glUseProgram(r->shader.iProgram);
    xge_glUniform1i(r->ibl_irradiance,6);xge_glUniform1i(r->ibl_prefiltered,7);xge_glUniform1i(r->ibl_brdf,8);
    xge_glUniform1f(r->ibl_intensity,d->ibl_intensity);
    xge_glUniform1f(r->ibl_max_lod,d->ibl_intensity>0 ? (float)d->environment->prefiltered.levels-1 : 0);
#endif
    result=d3_batches_prepare(r,vp,0,d,&counts.upload_bytes);if (result!=XGE_OK) goto done;
    for (size_t i=0;i<r->batches.Count;++i) {
        d3_batch *batch=xrtArrayGet(&r->batches,i);d3_draw *draw=batch->draw;
        xge3d_node_data *n=draw->node;
        xge3d_mat4_t world=draw->world;
        result=d3_mesh_upload(draw->mesh,&counts.upload_bytes);
        if (result!=XGE_OK) goto done;
        xge3d_mat4_t mvp=d3_mul(vp,world);
        if (!d3_finite_matrix(&mvp)) { result=XGE_ERROR_INVALID_ARGUMENT; goto done; }
        xge_glFrontFace(draw->determinant<0 ? GL_CW : GL_CCW);
        xge_glUseProgram(r->shader.iProgram);
        d3_instance_bind(r,r->instance_uniforms,vp,0,batch);
        xge_glUniformMatrix4fv(r->mvp,1,GL_FALSE,mvp.m);
        xge_glUniformMatrix4fv(r->world,1,GL_FALSE,world.m);
#if XGE3D_ENABLE_ANIMATION
        result=d3_skin_bind(s,n,r->skin_sampler,r->skin_enabled,&counts.upload_bytes);if (result!=XGE_OK) goto done;
#endif
        result=d3_material_bind(r,n,&counts.upload_bytes);
        if (result!=XGE_OK) goto done;
        d3_mesh_draw(draw->mesh,batch->count,&counts);
    }
    if (xge_glGetError()!=GL_NO_ERROR) result=XGE_ERROR_GPU_FAILED;
    if(result==XGE_OK)result=d3_composite_end(r,&output);
done:
    if(composite_started||output.shared_depth)d3_composite_detach(r,&output);
    d3_profile_end(r);d3_restore(&state);
    if(submit_start)r->profile.submit_cpu_us=xgeClockMicroseconds()-submit_start;
    r->profile.total_cpu_us=xgeClockMicroseconds()-profile_start;
    if (stats) *stats=counts;
    return result;
}

#endif
