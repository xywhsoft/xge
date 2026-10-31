#if XGE3D_ENABLE_DEBUG_DRAW || XGE3D_ENABLE_BILLBOARD
static const char d3_stream_vs[]=
    "layout(location=0)in vec3 position;layout(location=1)in vec2 uv;layout(location=2)in vec4 color;uniform mat4 vp;\n"
    "out vec2 vUV;out vec4 vColor;out vec3 vPosition;void main(){vUV=uv;vColor=color;vPosition=position;gl_Position=vp*vec4(position,1.0);}\n";
static const char d3_stream_fs[]=
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 vUV;in vec4 vColor;in vec3 vPosition;uniform sampler2D image;uniform int textured;uniform float exposure;out vec4 frag;\n"
#if XGE3D_ENABLE_DYNAMIC
    "uniform int targetImage;\n"
#endif
#if XGE3D_ENABLE_FOG
    "uniform vec4 fogColor,fogDepth;uniform vec2 fogRange;\n"
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
    "uniform vec4 heightColor,heightParams,heightEye;uniform vec3 heightForward;\n"
#endif
    "void main(){vec4 c=vColor;if(textured!=0){vec2 uv=vUV;\n"
#if XGE3D_ENABLE_DYNAMIC
    "if(targetImage!=0)uv.y=1.0-uv.y;\n"
#endif
    "vec4 t=texture(image,uv);\n"
#if XGE3D_ENABLE_DYNAMIC
    "if(targetImage!=0){vec3 s=t.a>0.0?t.rgb/t.a:vec3(0.0);t.rgb=mix(s/12.92,pow((s+0.055)/1.055,vec3(2.4)),step(vec3(0.04045),s));}\n"
#endif
    "c*=t;}vec3 lit=c.rgb;\n"
#if XGE3D_ENABLE_FOG
    "if(fogColor.w!=0.0)lit=mix(lit,fogColor.rgb,clamp((dot(fogDepth,vec4(vPosition,1.0))-fogRange.x)/(fogRange.y-fogRange.x),0.0,1.0));\n"
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
    "if(heightColor.w!=0.0){vec3 ray=vPosition-heightEye.xyz;if(heightEye.w>0.5)ray=heightForward*dot(ray,heightForward);\n"
    "float a=clamp(-heightParams.y*(vPosition.y-ray.y-heightParams.z),-80.0,80.0),b=clamp(-heightParams.y*(vPosition.y-heightParams.z),-80.0,80.0);\n"
    "float integral=abs(b-a)<0.001?exp((a+b)*0.5):(exp(b)-exp(a))/(b-a);float amount=min(heightParams.w,1.0-exp(-min(80.0,heightParams.x*length(ray)*integral)));lit=mix(lit,heightColor.rgb,amount);}\n"
#endif
    "frag=vec4(lit*exposure*c.a,c.a);}\n";
static int d3_stream_ready(xge3d_renderer *r)
{
    d3_stream *s=&r->stream;if(s->shader.iProgram){return XGE_OK;}
    int result=d3_program_create(&s->shader,d3_stream_vs,d3_stream_fs);if(result){return result;}
    xge_glGenVertexArrays(1,&s->vao);xge_glGenBuffers(1,&s->vbo);
    if(!s->vao||!s->vbo){if(s->vao){xge_glDeleteVertexArrays(1,&s->vao);}if(s->vbo){xge_glDeleteBuffers(1,&s->vbo);}s->vao=s->vbo=0;xgeShaderFree(&s->shader);return XGE_ERROR_GPU_FAILED;}
    GLuint p=s->shader.iProgram;s->vp=xge_glGetUniformLocation(p,"vp");s->image=xge_glGetUniformLocation(p,"image");
    s->textured=xge_glGetUniformLocation(p,"textured");s->target=-1;s->exposure=xge_glGetUniformLocation(p,"exposure");
#if XGE3D_ENABLE_DYNAMIC
    s->target=xge_glGetUniformLocation(p,"targetImage");
#endif
#if XGE3D_ENABLE_FOG
    s->fog_color=xge_glGetUniformLocation(p,"fogColor");s->fog_depth=xge_glGetUniformLocation(p,"fogDepth");s->fog_range=xge_glGetUniformLocation(p,"fogRange");
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
    s->height_color=xge_glGetUniformLocation(p,"heightColor");s->height_params=xge_glGetUniformLocation(p,"heightParams");s->height_eye=xge_glGetUniformLocation(p,"heightEye");s->height_forward=xge_glGetUniformLocation(p,"heightForward");
#endif
    xge_glBindVertexArray(s->vao);xge_glBindBuffer(GL_ARRAY_BUFFER,s->vbo);
    xge_glEnableVertexAttribArray(0);xge_glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(d3_stream_vertex),(const void*)offsetof(d3_stream_vertex,position));
    xge_glEnableVertexAttribArray(1);xge_glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(d3_stream_vertex),(const void*)offsetof(d3_stream_vertex,uv));
    xge_glEnableVertexAttribArray(2);xge_glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(d3_stream_vertex),(const void*)offsetof(d3_stream_vertex,color));return XGE_OK;
}
static void d3_stream_free(xge3d_renderer *r)
{
    d3_stream *s=&r->stream;
    if(r->context==__xge3dContext()){xgeShaderFree(&s->shader);if(s->vao){xge_glDeleteVertexArrays(1,&s->vao);}if(s->vbo){xge_glDeleteBuffers(1,&s->vbo);}}xrtArrayUnit(&s->vertices);
#if XGE3D_ENABLE_DEBUG_DRAW
    xrtArrayUnit(&r->debug_lines);
#endif
#if XGE3D_ENABLE_BILLBOARD
    for(size_t i=0;i<r->billboards.Count;++i){xge3dTextureFree(((xge3d_billboard_t*)r->billboards.Data)[i].texture);}
    xrtArrayUnit(&r->billboards);xrtArrayUnit(&r->billboard_staging);xrtArrayUnit(&r->billboard_order);
#endif
}
static int d3_stream_begin(xge3d_renderer *r,const xge3d_render_desc_t *d,xge3d_mat4_t vp,int fog,xge3d_render_stats_t *stats)
{
    int result=d3_stream_ready(r);if(result){return result;}d3_stream *s=&r->stream;
    size_t bytes=s->vertices.Count*sizeof(d3_stream_vertex);xge_glBindVertexArray(s->vao);xge_glBindBuffer(GL_ARRAY_BUFFER,s->vbo);
    if(bytes>s->capacity){xge_glBufferData(GL_ARRAY_BUFFER,bytes,NULL,GL_DYNAMIC_DRAW);s->capacity=bytes;}
    xge_glBufferSubData(GL_ARRAY_BUFFER,0,bytes,s->vertices.Data);stats->upload_bytes+=bytes;
    xge_glUseProgram(s->shader.iProgram);xge_glUniformMatrix4fv(s->vp,1,GL_FALSE,vp.m);xge_glUniform1f(s->exposure,d->exposure>0?d->exposure:1);xge_glUniform1i(s->image,0);
    xge_glDisable(GL_CULL_FACE);xge_glDepthMask(GL_FALSE);xge_glEnable(GL_BLEND);xge_glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
#if XGE3D_ENABLE_FOG
    xge_glUniform4f(s->fog_color,d->fog?d->fog->color.x:0,d->fog?d->fog->color.y:0,d->fog?d->fog->color.z:0,fog&&d->fog);
    xge_glUniform2f(s->fog_range,d->fog?d->fog->start:0,d->fog?d->fog->end:1);const float *v=d->camera->view.m;xge_glUniform4f(s->fog_depth,-v[2],-v[6],-v[10],-v[14]);
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
    const xge3d_height_fog_settings_t *h=&r->height_fog;
    xge_glUniform4f(s->height_color,h->color.x,h->color.y,h->color.z,fog&&r->height_active);
    /* Render validated the global height; the normal program already holds its
       relative value. Recover origin from the per-frame value saved below. */
    xge_glUniform4f(s->height_params,h->density,h->falloff,r->stream_height,h->max_opacity);
    xge3d_mat4_t inverse;if(!d3_inverse(d->camera->view,&inverse)){return XGE_ERROR_INVALID_ARGUMENT;}
    int ortho=d->camera->projection.m[15]!=0&&d->camera->projection.m[11]==0;xge3d_vec3_t forward={-inverse.m[8],-inverse.m[9],-inverse.m[10]};d3_normalize(&forward);
    xge_glUniform4f(s->height_eye,inverse.m[12],inverse.m[13],inverse.m[14],(float)ortho);xge_glUniform3f(s->height_forward,forward.x,forward.y,forward.z);
#endif
    return XGE_OK;
}
static int d3_stream_color(const float *color)
{if(!color){return 0;}for(int i=0;i<4;++i){if(!isfinite(color[i])||color[i]<0||color[i]>(i==3?1:64)){return 0;}}return 1;}

#if XGE3D_ENABLE_DEBUG_DRAW
void xge3dDebugClear(xge3d_renderer *r){if(r){xrtArrayClear(&r->debug_lines);}}
int xge3dDebugLine(xge3d_renderer *r,xge3d_vec3_t a,xge3d_vec3_t b,const float color[4],int depth)
{
    if(!r||!d3_finite3(a)||!d3_finite3(b)||!d3_stream_color(color)||(depth!=0&&depth!=1)){return XGE_ERROR_INVALID_ARGUMENT;}
    if(r->debug_lines.Count>=1048576){return XGE_ERROR_BUFFER_TOO_SMALL;}d3_debug_line line={.a=a,.b=b,.depth=depth};memcpy(line.color,color,sizeof(line.color));
    return xrtArrayPush(&r->debug_lines,&line)?XGE_OK:XGE_ERROR_OUT_OF_MEMORY;
}
int xge3dDebugBox(xge3d_renderer *r,const xge3d_aabb_t *box,const xge3d_mat4_t *world,const float color[4],int depth)
{
    if(!r||!box||!d3_finite3(box->min)||!d3_finite3(box->max)||!d3_finite_matrix(world)||box->min.x>box->max.x||box->min.y>box->max.y||box->min.z>box->max.z){return XGE_ERROR_INVALID_ARGUMENT;}
    size_t count=r->debug_lines.Count;int result=0;xge3d_vec3_t p[8];for(int i=0;i<8;++i){p[i]=d3_point(*world,(xge3d_vec3_t){i&1?box->max.x:box->min.x,i&2?box->max.y:box->min.y,i&4?box->max.z:box->min.z});}
    for(int i=0;i<8&&!result;++i){for(int bit=1;bit<=4&&!result;bit*=2){if(!(i&bit)){result=xge3dDebugLine(r,p[i],p[i|bit],color,depth);}}}
    if(result){r->debug_lines.Count=count;}return result;
}
int xge3dDebugAxes(xge3d_renderer *r,const xge3d_mat4_t *world,float length,int depth)
{
    if(!r||!d3_finite_matrix(world)||!isfinite(length)||length<0){return XGE_ERROR_INVALID_ARGUMENT;}size_t count=r->debug_lines.Count;
    for(int i=0;i<3;++i){float color[4]={0,0,0,1};color[i]=1;xge3d_vec3_t p={0};(&p.x)[i]=length;
        int result=xge3dDebugLine(r,d3_point(*world,(xge3d_vec3_t){0}),d3_point(*world,p),color,depth);if(result){r->debug_lines.Count=count;return result;}}
    return XGE_OK;
}
#if XGE3D_ENABLE_COLLISION
int xge3dDebugShape(xge3d_renderer *r,const xge3d_shape_t *shape,const xge3d_mat4_t *world,const float color[4],int depth)
{
    if(!r||!shape||!d3_finite_matrix(world)||!d3_finite3(shape->center)||!d3_stream_color(color)||(depth!=0&&depth!=1)){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_mat4_t w=*world;xge3d_vec3_t center=d3_point(w,shape->center);w.m[12]=center.x;w.m[13]=center.y;w.m[14]=center.z;
    if(shape->type==XGE3D_SHAPE_BOX){if(!d3_finite3(shape->half_extents)||shape->half_extents.x<0||shape->half_extents.y<0||shape->half_extents.z<0){return XGE_ERROR_INVALID_ARGUMENT;}
        xge3d_aabb_t box={d3_scale(shape->half_extents,-1),shape->half_extents};return xge3dDebugBox(r,&box,&w,color,depth);}
    size_t count=r->debug_lines.Count;int result=0;
    if(shape->type==XGE3D_SHAPE_MESH){if(!shape->mesh){return XGE_ERROR_INVALID_ARGUMENT;}xge3d_mesh_desc_t mesh;result=xge3dMeshGetData(shape->mesh,&mesh);
        size_t n=mesh.index_count?mesh.index_count:mesh.vertex_count;for(size_t i=0;i<n&&!result;i+=3){xge3d_vec3_t p[3];for(int j=0;j<3;++j){size_t k=i+j;if(mesh.index_count){k=mesh.index_bits==16?((const uint16_t*)mesh.indices)[k]:((const uint32_t*)mesh.indices)[k];}p[j]=d3_point(w,mesh.vertices[k].position);}
            for(int j=0;j<3&&!result;++j){result=xge3dDebugLine(r,p[j],p[(j+1)%3],color,depth);}}
    }else if(shape->type==XGE3D_SHAPE_SPHERE||shape->type==XGE3D_SHAPE_CAPSULE){
        if(!isfinite(shape->radius)||shape->radius<0||!isfinite(shape->half_height)||shape->half_height<0){return XGE_ERROR_INVALID_ARGUMENT;}
        float half=shape->type==XGE3D_SHAPE_CAPSULE?shape->half_height:0;
        for(int plane=0;plane<3&&!result;++plane){for(int i=0;i<32&&!result;++i){xge3d_vec3_t p[2]={{0}};for(int j=0;j<2;++j){float a=(i+j)*6.2831853f/32,c=cosf(a),s=sinf(a);
                if(plane==0){p[j]=(xge3d_vec3_t){c*shape->radius,s*shape->radius+(s>=0?half:-half),0};}
                if(plane==1){p[j]=(xge3d_vec3_t){0,s*shape->radius+(s>=0?half:-half),c*shape->radius};}
                if(plane==2){p[j]=(xge3d_vec3_t){c*shape->radius,half,s*shape->radius};}}
            result=xge3dDebugLine(r,d3_point(w,p[0]),d3_point(w,p[1]),color,depth);
            if(!result&&plane==2&&half){p[0].y=p[1].y=-half;result=xge3dDebugLine(r,d3_point(w,p[0]),d3_point(w,p[1]),color,depth);}
        }}
    }else result=XGE_ERROR_INVALID_ARGUMENT;
    if(result){r->debug_lines.Count=count;}return result;
}
#endif
#if XGE3D_ENABLE_ANIMATION
int xge3dDebugSkeleton(xge3d_renderer *r,xge3d_scene *scene,xge3d_node_t node,const float color[4],int depth)
{
    xge3d_node_data *n=d3_node(scene,node);if(!r||!n||!n->skin||!d3_stream_color(color)||(depth!=0&&depth!=1)){return XGE_ERROR_INVALID_ARGUMENT;}
    size_t count=r->debug_lines.Count;d3_skin *skin=n->skin;
    for(size_t i=0;i<skin->joints.Count;++i){xge3d_node_t joint=*(xge3d_node_t*)xrtArrayGet(&skin->joints,i);xge3d_node_data *j=d3_node(scene,joint),*parent=j?j->parent_node:NULL;
        while(parent){int match=0;for(size_t k=0;k<skin->joints.Count;++k){if(((xge3d_node_t*)skin->joints.Data)[k].slot==parent->handle.slot){match=1;break;}}if(match){break;}parent=parent->parent_node;}
        if(!j){r->debug_lines.Count=count;return XGE_ERROR_INVALID_STATE;}if(!parent){continue;}xge3d_mat4_t a,b;
        int result=xge3dNodeGetWorldMatrix(scene,joint,&a);if(!result){result=xge3dNodeGetWorldMatrix(scene,parent->handle,&b);}
        if(!result){result=xge3dDebugLine(r,(xge3d_vec3_t){a.m[12],a.m[13],a.m[14]},(xge3d_vec3_t){b.m[12],b.m[13],b.m[14]},color,depth);}
        if(result){r->debug_lines.Count=count;return result;}
    }return XGE_OK;
}
#endif
static int d3_debug_render(xge3d_renderer *r,const xge3d_render_desc_t *d,xge3d_mat4_t vp,xge3d_render_stats_t *stats)
{
    if(!r->debug_lines.Count){return XGE_OK;}d3_stream *s=&r->stream;if(!xrtArrayResize(&s->vertices,r->debug_lines.Count*2)){return XGE_ERROR_OUT_OF_MEMORY;}
    size_t at=0,split=0;for(int depth=1;depth>=0;--depth){for(size_t i=0;i<r->debug_lines.Count;++i){d3_debug_line *line=xrtArrayGet(&r->debug_lines,i);if(line->depth!=depth){continue;}
            d3_stream_vertex *v=(d3_stream_vertex*)s->vertices.Data+at;v[0]=(d3_stream_vertex){.position=line->a};v[1]=(d3_stream_vertex){.position=line->b};memcpy(v[0].color,line->color,sizeof(line->color));memcpy(v[1].color,line->color,sizeof(line->color));at+=2;
        }if(depth){split=at;}}
    int result=d3_stream_begin(r,d,vp,0,stats);if(result){return result;}xge_glActiveTexture(GL_TEXTURE0);xge_glBindTexture(GL_TEXTURE_2D,0);xge_glUniform1i(s->textured,0);xge_glUniform1i(s->target,0);
    xge_glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);xge_glDepthFunc(GL_LEQUAL);
    if(split){xge_glEnable(GL_DEPTH_TEST);xge_glDrawArrays(GL_LINES,0,(GLsizei)split);++stats->draw_calls;}
    if(at>split){xge_glDisable(GL_DEPTH_TEST);xge_glDrawArrays(GL_LINES,(GLint)split,(GLsizei)(at-split));++stats->draw_calls;}return XGE_OK;
}
#endif

#if XGE3D_ENABLE_BILLBOARD
xge3d_billboard_t xge3dBillboardDefault(void)
{return (xge3d_billboard_t){.width=1,.height=1,.color={1,1,1,1},.uv={0,0,1,1},.depth_test=1};}
int xge3dBillboardsSet(xge3d_renderer *r,const xge3d_billboard_t *items,size_t count)
{
    if(!r||(count&&!items)||count>65536){return XGE_ERROR_INVALID_ARGUMENT;}
    for(size_t i=0;i<count;++i){const xge3d_billboard_t *b=items+i;
        if(!d3_finite3(b->position)||!d3_stream_color(b->color)||!isfinite(b->width)||!isfinite(b->height)||b->width<0||b->height<0||b->width>1e6f||b->height>1e6f||!isfinite(b->rotation)||
            (b->blend!=0&&b->blend!=1)||(b->depth_test!=0&&b->depth_test!=1)){return XGE_ERROR_INVALID_ARGUMENT;}
        for(int j=0;j<4;++j){if(!isfinite(b->uv[j])){return XGE_ERROR_INVALID_ARGUMENT;}}
    }
    if(!xrtArrayResize(&r->billboard_staging,count)){return XGE_ERROR_OUT_OF_MEMORY;}
    size_t i=0;for(;i<count;++i){if(items[i].texture&&!d3_texture_retain(items[i].texture)){break;}((xge3d_billboard_t*)r->billboard_staging.Data)[i]=items[i];}
    if(i<count){while(i--){xge3dTextureFree(items[i].texture);}r->billboard_staging.Count=0;return XGE_ERROR_INVALID_STATE;}
    xarray old=r->billboards;r->billboards=r->billboard_staging;r->billboard_staging=old;
    for(i=0;i<old.Count;++i){xge3dTextureFree(((xge3d_billboard_t*)old.Data)[i].texture);}xrtArrayClear(&r->billboard_staging);return XGE_OK;
}
static int d3_billboard_compare(const void *a,const void *b)
{const d3_billboard_order *x=a,*y=b;return x->depth>y->depth?-1:x->depth<y->depth?1:x->index<y->index?-1:x->index>y->index;}
static int d3_billboards_render(xge3d_renderer *r,const xge3d_render_desc_t *d,xge3d_mat4_t vp,xge3d_render_stats_t *stats)
{
    if(!r->billboards.Count){return XGE_OK;}d3_stream *s=&r->stream;
    if(!xrtArrayResize(&r->billboard_order,r->billboards.Count)||!xrtArrayResize(&s->vertices,r->billboards.Count*6)){return XGE_ERROR_OUT_OF_MEMORY;}
    d3_billboard_order *order=(d3_billboard_order*)r->billboard_order.Data;for(size_t i=0;i<r->billboards.Count;++i){xge3d_billboard_t *b=xrtArrayGet(&r->billboards,i);order[i]=(d3_billboard_order){b,-d3_point(d->camera->view,b->position).z,i};
#if XGE3D_ENABLE_DYNAMIC
        if(b->texture&&b->texture->target&&b->texture->target==d->target){return XGE_ERROR_INVALID_STATE;}
#endif
    }xrtArraySort(&r->billboard_order,d3_billboard_compare);
    xge3d_mat4_t inverse;if(!d3_inverse(d->camera->view,&inverse)){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_vec3_t right={inverse.m[0],inverse.m[1],inverse.m[2]},up={inverse.m[4],inverse.m[5],inverse.m[6]};if(!d3_normalize(&right)||!d3_normalize(&up)){return XGE_ERROR_INVALID_ARGUMENT;}
    const int corners[6][2]={{0,0},{1,0},{1,1},{0,0},{1,1},{0,1}};
    for(size_t i=0;i<r->billboards.Count;++i){const xge3d_billboard_t *b=order[i].item;float c=cosf(b->rotation),sn=sinf(b->rotation);
        for(int j=0;j<6;++j){float x=(corners[j][0]-.5f)*b->width,y=(.5f-corners[j][1])*b->height;
            d3_stream_vertex *v=(d3_stream_vertex*)s->vertices.Data+i*6+j;v->position=d3_add(b->position,d3_add(d3_scale(right,c*x-sn*y),d3_scale(up,sn*x+c*y)));
            if(!d3_finite3(v->position)){return XGE_ERROR_INVALID_ARGUMENT;}v->uv[0]=b->uv[0]+corners[j][0]*b->uv[2];v->uv[1]=b->uv[1]+corners[j][1]*b->uv[3];memcpy(v->color,b->color,sizeof(v->color));}
    }
    int result=d3_stream_begin(r,d,vp,1,stats);if(result){return result;}xge_glDepthFunc(GL_LESS);
    for(size_t i=0;i<r->billboards.Count;){const xge3d_billboard_t *b=order[i].item;size_t end=i+1;
        while(end<r->billboards.Count&&order[end].item->texture==b->texture&&order[end].item->blend==b->blend&&order[end].item->depth_test==b->depth_test){++end;}
        xge_glActiveTexture(GL_TEXTURE0);if(b->texture){result=d3_texture_upload(b->texture,&stats->upload_bytes);if(result){return result;}}else xge_glBindTexture(GL_TEXTURE_2D,0);
        xge_glUniform1i(s->textured,b->texture!=NULL);
#if XGE3D_ENABLE_DYNAMIC
        xge_glUniform1i(s->target,b->texture&&b->texture->target);
#endif
        if(b->depth_test){xge_glEnable(GL_DEPTH_TEST);}else xge_glDisable(GL_DEPTH_TEST);
        xge_glBlendFuncSeparate(GL_ONE,b->blend==XGE3D_BILLBOARD_ADD?GL_ONE:GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        xge_glDrawArrays(GL_TRIANGLES,(GLint)(i*6),(GLsizei)((end-i)*6));++stats->draw_calls;stats->triangles+=(end-i)*2;i=end;
    }return XGE_OK;
}
#endif
#endif
