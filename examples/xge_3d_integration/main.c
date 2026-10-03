#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
#include "xui.h"
#endif

/* Geometry copies and movement rules are application data, outside XGE. */
typedef struct triangle {xge3d_vec3_t p[3];int walkable;} triangle;
typedef struct demo {
    xge3d_scene *scene;xge3d_renderer *renderer;xge3d_target *target;
    xge3d_terrain *terrain;xge3d_environment *sky;xge3d_loader *loader;
    xge3d_animator *animator[2];xge3d_clip *clips[2][6];
    xge3d_request_t requests[15];int loaded[15];
    xge3d_node_t actors[2],terrain_root,obstacle;
    xge3d_dvec3_t origin,positions[2];xarray triangles;
    int frame,frames,failed,action,last_action,pressed,ready,rebases,walkable,queries,moves,deletes;unsigned actions;
    uint64_t start,uploads;const char *capture;
#if XGE_ENABLE_2D
    xge_render_target_t composite;
#endif
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
    xui_context ui;xui_surface ui_target;xui_widget button;xui_font font;xui_proxy_t proxy;
#endif
} demo;
static const char *actions[]={"idle","walk","run","jump","slash","gather"};
static xge3d_vec3_t sub(xge3d_vec3_t a,xge3d_vec3_t b)
{return (xge3d_vec3_t){a.x-b.x,a.y-b.y,a.z-b.z};}
static xge3d_vec3_t transform(const xge3d_mat4_t *m,xge3d_vec3_t v)
{return (xge3d_vec3_t){m->m[0]*v.x+m->m[4]*v.y+m->m[8]*v.z+m->m[12],m->m[1]*v.x+m->m[5]*v.y+m->m[9]*v.z+m->m[13],m->m[2]*v.x+m->m[6]*v.y+m->m[10]*v.z+m->m[14]};}
static void release(demo *d)
{
    xge3dLoaderFree(d->loader);d->loader=NULL;
    for (int i=0;i<2;++i) {
        xge3dAnimatorFree(d->animator[i]);d->animator[i]=NULL;
        for (int a=0;a<6;++a) {xge3dClipFree(d->clips[i][a]);d->clips[i][a]=NULL;}
    }
    xge3dSceneFree(d->scene);d->scene=NULL;xge3dTerrainFree(d->terrain);d->terrain=NULL;
    xge3dEnvironmentFree(d->sky);d->sky=NULL;xge3dTargetFree(d->target);d->target=NULL;
    xge3dRendererFree(d->renderer);d->renderer=NULL;xrtArrayUnit(&d->triangles);
#if XGE_ENABLE_2D
    xgeRenderTargetFree(&d->composite);
#endif
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
    if (d->ui) xuiDestroy(d->ui);
    d->ui=NULL;
    if (d->ui_target) d->proxy.surfaceDestroy(&d->proxy,d->ui_target);
    d->ui_target=NULL;
    if (d->font) d->proxy.fontDestroy(&d->proxy,d->font);
    d->font=NULL;
#endif
}
static int export_geometry(demo *d)
{
    xrtArrayClear(&d->triangles);d->walkable=0;
    for (size_t i=0;i<xge3dSceneNodeCount(d->scene);++i) {
        xge3d_node_t node;xge3d_mesh_desc_t mesh;xge3d_mat4_t matrix;
        node=xge3dSceneNodeAt(d->scene,i);int result=XGE_OK;
        const xge3d_mesh *source=NULL;result=xge3dNodeGetMesh(d->scene,node,&source);
        if (result==XGE_ERROR_NOT_FOUND) continue;
        if (result!=XGE_OK) return result;
        if ((result=xge3dMeshGetData(source,&mesh))!=XGE_OK) return result;
        const xge3d_mat4_t *palette;size_t joints;result=xge3dNodeSkinMatrices(d->scene,node,&palette,&joints);
        if (result==XGE_OK) continue; /* Animated bodies use their own application collision shape. */
        if (result!=XGE_ERROR_NOT_FOUND) return result;
        if ((result=xge3dNodeGetWorldMatrix(d->scene,node,&matrix))!=XGE_OK) return result;
        size_t count=mesh.index_count ? mesh.index_count : mesh.vertex_count;
        for (size_t k=0;k<count;k+=3) {
            triangle tri={0};
            for (int j=0;j<3;++j) {
                size_t index=mesh.index_count ? mesh.index_bits==16 ? ((const uint16_t*)mesh.indices)[k+j] : ((const uint32_t*)mesh.indices)[k+j] : k+j;
                tri.p[j]=transform(&matrix,mesh.vertices[index].position);
            }
            xge3d_vec3_t a=sub(tri.p[1],tri.p[0]),b=sub(tri.p[2],tri.p[0]);
            xge3d_vec3_t n={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
            tri.walkable=n.y>.65f*sqrtf(n.x*n.x+n.y*n.y+n.z*n.z);
            d->walkable+=tri.walkable;if (!xrtArrayPush(&d->triangles,&tri)) return XGE_ERROR_OUT_OF_MEMORY;
        }
    }
    return d->walkable>0 ? XGE_OK : XGE_ERROR_NOT_FOUND;
}
static int ground(demo *d,xge3d_dvec3_t position,float *height)
{
    xge3d_vec3_t local;int result=xge3dRelativePosition(d->origin,position,&local);if (result!=XGE_OK) return result;
    xge3d_ray_t ray={{local.x,20,local.z},{0,-1,0}};float distance=40;int found=0;++d->queries;
    for (size_t i=0;i<d->triangles.Count;++i) {
        const triangle *tri=xrtArrayConstGet(&d->triangles,i);xge3d_hit_t hit;
        if (tri->walkable && xge3dRayTriangle(&ray,tri->p,distance,&hit)==XGE_OK) {distance=hit.distance;found=1;}
    }
    if (found) *height=20-distance;
    return found ? XGE_OK : XGE_ERROR_NOT_FOUND;
}
static int make_box(xge3d_mesh **out)
{
    const xge3d_vec3_t n[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    const xge3d_vec3_t u[]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
    const xge3d_vec3_t v[]={{0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0}};
    const int corners[]={0,1,2,0,2,3},xs[]={-1,1,1,-1},ys[]={-1,-1,1,1};xge3d_vertex_t vertices[36]={0};
    for (int f=0;f<6;++f) for (int k=0;k<6;++k) {
        int c=corners[k];xge3d_vertex_t *p=&vertices[f*6+k];
        p->position=(xge3d_vec3_t){.5f*(n[f].x+u[f].x*xs[c]+v[f].x*ys[c]),.5f+.5f*(n[f].y+u[f].y*xs[c]+v[f].y*ys[c]),.5f*(n[f].z+u[f].z*xs[c]+v[f].z*ys[c])};
        p->normal=n[f];p->uv[0]=(xs[c]+1)*.5f;p->uv[1]=(ys[c]+1)*.5f;
        p->tangent[0]=u[f].x;p->tangent[1]=u[f].y;p->tangent[2]=u[f].z;p->tangent[3]=1;
    }
    xge3d_mesh_desc_t desc={vertices,36};return xge3dMeshCreate(&desc,out);
}
static int build_scene(demo *d)
{
    int result=xge3dSceneCreate(&d->scene);d->origin=(xge3d_dvec3_t){1000000000,0,-1000000000};
    if (result==XGE_OK) result=xge3dSceneSetOrigin(d->scene,d->origin);
    float heights[33*33];for (int z=0;z<33;++z) for (int x=0;x<33;++x) heights[z*33+x]=.3f*sinf(x*.3f)+.2f*cosf(z*.25f);
    xge3d_material_desc_t md=xge3dMaterialDefault();md.metallic=0;md.roughness=.9f;md.base_color[0]=.15f;md.base_color[1]=.3f;md.base_color[2]=.08f;
    xge3d_material *material=NULL;if (result==XGE_OK) result=xge3dMaterialCreate(&md,&material);
    xge3d_terrain_desc_t td=xge3dTerrainDefault();td.heights=heights;td.width=td.depth=33;td.chunk_cells=8;td.material=material;
    td.lod_distances[0]=18;td.lod_distances[1]=35;
    if (result==XGE_OK) result=xge3dTerrainCreate(&td,&d->terrain);
    xge3dMaterialFree(material);material=NULL;
    if (result==XGE_OK) result=xge3dTerrainInstantiate(d->scene,d->terrain,(xge3d_node_t){0},&d->terrain_root);
    if (result==XGE_OK) result=xge3dNodeSetGlobalPosition(d->scene,d->terrain_root,(xge3d_dvec3_t){d->origin.x-16,0,d->origin.z-16});
    md.base_color[0]=.52f;md.base_color[1]=.34f;md.base_color[2]=.19f;
    if (result==XGE_OK) result=xge3dMaterialCreate(&md,&material);
    xge3d_mesh *box=NULL;if (result==XGE_OK) result=make_box(&box);
    for (int i=0;i<80 && result==XGE_OK;++i) {
        float x=-13+(i%10)*2.8f,z=-13+(i/10)*3.4f,h;
        xge3d_node_t node;result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&node);if (!i) d->obstacle=node;
        if (result==XGE_OK) result=xge3dNodeSetMesh(d->scene,node,box);
        if (result==XGE_OK) result=xge3dNodeSetMaterial(d->scene,node,material);
        xge3d_vec3_t normal;if (result==XGE_OK) result=xge3dTerrainSample(d->terrain,0,x+16,z+16,&h,&normal);
        if (result==XGE_OK) result=xge3dNodeSetGlobalPosition(d->scene,node,(xge3d_dvec3_t){d->origin.x+x,h,d->origin.z+z});
    }
    xge3dMeshFree(box);xge3dMaterialFree(material);
    xge3d_node_t sun,spot;xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);
    light.direction=(xge3d_vec3_t){-.5f,-1,-.7f};light.intensity=2;light.casts_shadow=1;
    if (result==XGE_OK) result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&sun);
    if (result==XGE_OK) result=xge3dNodeSetLight(d->scene,sun,&light);
    light=xge3dLightDefault(XGE3D_LIGHT_SPOT);light.direction=(xge3d_vec3_t){0,-1,-1};light.intensity=50;light.range=18;light.casts_shadow=1;
    if (result==XGE_OK) result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&spot);
    if (result==XGE_OK) result=xge3dNodeSetLight(d->scene,spot,&light);
    if (result==XGE_OK) result=xge3dNodeSetGlobalPosition(d->scene,spot,(xge3d_dvec3_t){d->origin.x,5,d->origin.z+4});
    unsigned char colors[6][4]={{90,129,161,255},{90,129,161,255},{74,117,165,255},{20,30,40,255},{90,129,161,255},{90,129,161,255}};
    xge_image_t images[6];xge3d_environment_desc_t sky={0};sky.srgb=1;
    for (int i=0;i<6;++i) {images[i]=(xge_image_t){1,1,XGE_PIXEL_RGBA8,4,colors[i],XGE_IMAGE_STRAIGHT_ALPHA};sky.faces[i]=&images[i];}
    if (result==XGE_OK) result=xge3dEnvironmentCreate(&sky,&d->sky);
    return result;
}
static int take_model(demo *d,int index)
{
    xge3d_model *model=NULL;int result=xge3dLoaderTake(d->loader,d->requests[index],&model);
    if (index>=3) {
        if (result==XGE_OK) result=xge3dClipFromModel(model,0,&d->clips[(index-3)/6][(index-3)%6]);
    } else {
        xge3d_node_t root={0},mesh={0};if (result==XGE_OK) result=xge3dModelInstantiate(d->scene,model,(xge3d_node_t){0},&root);
        if (index<2) {
            d->actors[index]=root;d->positions[index]=(xge3d_dvec3_t){1000000000+(index ? 2.0 : -2.0),0,-999999999};
            if (result==XGE_OK) result=xge3dAnimatorCreate(d->scene,root,&d->animator[index]);
            if (result==XGE_OK) result=xge3dAnimatorSetRootMotion(d->animator[index],0,XGE3D_ROOT_X|XGE3D_ROOT_Z);
            if (result==XGE_OK) result=xge3dModelInstanceNode(d->scene,root,17,&mesh);
            xge3d_material_desc_t md=xge3dMaterialDefault();md.metallic=0;md.roughness=.8f;md.double_sided=1;
            md.base_color[0]=index ? .8f : .08f;md.base_color[1]=index ? .18f : .5f;md.base_color[2]=index ? .08f : .8f;
            for (int j=0;j<3;++j) md.emissive[j]=md.base_color[j]*.05f;
            xge3d_material *material=NULL;if (result==XGE_OK) result=xge3dMaterialCreate(&md,&material);
            if (result==XGE_OK) result=xge3dNodeSetMaterial(d->scene,mesh,material);
            xge3dMaterialFree(material);
        } else if (result==XGE_OK) result=xge3dNodeSetGlobalPosition(d->scene,root,(xge3d_dvec3_t){999999993,1,-1000000003});
    }
    xge3dModelFree(model);return result;
}
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
static void next_action(xui_widget widget,void *user)
{(void)widget;demo *d=user;d->action=(d->action+1)%6;}
static int ui_prepare(demo *d)
{
    d->proxy=xuiProxyXge();int result=xuiCreate(&d->ui);
    if (result==XUI_OK) result=xuiSetProxy(d->ui,&d->proxy);
    if (result==XUI_OK) result=xuiInputViewport(d->ui,280,44);
    if (result==XUI_OK) result=d->proxy.fontLoadFile(&d->proxy,&d->font,"C:/Windows/Fonts/segoeui.ttf",16,0);
    if (result==XUI_OK) result=xuiSetDefaultFont(d->ui,d->font);
    xui_surface_desc_t sd={0};sd.iKind=XUI_SURFACE_KIND_TEXTURE;sd.iFormat=XUI_SURFACE_FORMAT_RGBA8;sd.iWidth=280;sd.iHeight=44;
    sd.iFlags=XUI_SURFACE_ALPHA_PREMULTIPLIED|XUI_SURFACE_USAGE_TARGET;
    if (result==XUI_OK) result=d->proxy.surfaceCreate(&d->proxy,&d->ui_target,&sd);
    xui_button_desc_t bd={0};bd.iSize=sizeof(bd);bd.sText="Loading models...";
    if (result==XUI_OK) result=xuiButtonCreate(d->ui,&d->button,&bd);
    if (result==XUI_OK) result=xuiSetRootWidget(d->ui,d->button);
    if (result==XUI_OK) result=xuiWidgetArrange(d->button,(xui_rect_t){0,0,280,44});
    if (result==XUI_OK) result=xuiButtonSetClick(d->button,next_action,d);
    return result==XUI_OK ? XGE_OK : XGE_ERROR;
}
static int ui_draw(demo *d)
{
    float x,y;xgeMouseGet(&x,&y);int pressed=xgeMouseDown(0);
    int result=xuiInputPointerMove(d->ui,(int)x-8,(int)y-8,pressed ? 1 : 0);
    if (result==XUI_OK && pressed!=d->pressed) result=pressed ? xuiInputPointerDown(d->ui,(int)x-8,(int)y-8,0,1) : xuiInputPointerUp(d->ui,(int)x-8,(int)y-8,0,0);
    d->pressed=pressed;char text[100];snprintf(text,sizeof(text),"Action: %s | click to change",d->ready==15 ? actions[d->action] : "loading");
    if (result==XUI_OK) result=xuiButtonSetText(d->button,text);
    xui_rect_i_t rect={0,0,280,44};if (result==XUI_OK) result=xuiRender(d->ui,d->ui_target,&rect,1);
    return result==XUI_OK ? XGE_OK : XGE_ERROR;
}
#endif
static int frame(void *user)
{
    demo *d=user;int result=XGE_OK;
    if (!d->renderer) {
        result=xge3dRendererCreate(&d->renderer);if (result==XGE_OK) result=xge3dTargetCreate(640,480,&d->target);
#if XGE_ENABLE_2D
        if (result==XGE_OK) result=xgeRenderTargetCreate(&d->composite,640,480);
#endif
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
        if (result==XGE_OK) result=ui_prepare(d);
#endif
        if (result!=XGE_OK) goto fail;
    }
    xge3d_upload_budget_t budget={512,1000,4};xge3d_upload_stats_t uploads;
    if ((result=xge3dLoaderPump(d->loader,&budget,&uploads))!=XGE_OK) goto fail;
    if (uploads.upload_bytes>512 || uploads.operations>4) {result=XGE_ERROR_INVALID_STATE;goto fail;}
    d->uploads+=uploads.upload_bytes;
    for (int i=0;i<15;++i) if (!d->loaded[i]) {
        xge3d_request_info_t info;if ((result=xge3dLoaderStatus(d->loader,d->requests[i],&info))!=XGE_OK) goto fail;
        if (info.state==XGE3D_REQUEST_FAILED) {result=info.result;goto fail;}
        if (info.state==XGE3D_REQUEST_READY) {if ((result=take_model(d,i))!=XGE_OK) goto fail;d->loaded[i]=1;++d->ready;}
    }
    if (d->ready<15 && xrtClock()-d->start>10000000) {result=XGE_ERROR_INVALID_STATE;goto fail;}
    if (d->ready==15) {
        if (!d->triangles.Count && (result=export_geometry(d))!=XGE_OK) goto fail;
        if (d->frame==30) {
            if ((result=xge3dNodeSetGlobalPosition(d->scene,d->obstacle,(xge3d_dvec3_t){1000000000,.2,-1000000004}))!=XGE_OK) goto fail;
            ++d->moves;if ((result=export_geometry(d))!=XGE_OK) goto fail;
        }
        if (d->frame==60) {
            d->origin.x+=32;d->origin.z-=16;
            if ((result=xge3dSceneSetOrigin(d->scene,d->origin))!=XGE_OK) goto fail;
            ++d->rebases;if ((result=export_geometry(d))!=XGE_OK) goto fail;
        }
        if (d->frame==90) {
            if ((result=xge3dNodeDestroy(d->scene,d->obstacle))!=XGE_OK) goto fail;
            ++d->deletes;if ((result=export_geometry(d))!=XGE_OK) goto fail;
        }
        if (d->frames) d->action=(d->frame/60)%6;
        for (int i=0;i<2;++i) {
            if (d->action!=d->last_action) {
                xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();
                if ((result=xge3dAnimatorSetLayer(d->animator[i],0,d->clips[i][d->action],&layer))!=XGE_OK) goto fail;
            }
            if ((result=xge3dAnimatorUpdate(d->animator[i],1.f/60))!=XGE_OK) goto fail;
            xge3d_root_motion_t motion;if ((result=xge3dAnimatorTakeRootMotion(d->animator[i],&motion))!=XGE_OK) goto fail;
            xge3d_dvec3_t proposed=d->positions[i];proposed.x+=motion.translation.x;proposed.z+=motion.translation.z;
            if (d->action==1 || d->action==2) proposed.z-=.01*(d->action==2 ? 2 : 1);
            float h;if (ground(d,proposed,&h)==XGE_OK && fabs(h-proposed.y)<.5) {proposed.y=h;d->positions[i]=proposed;}
            if ((result=xge3dNodeSetGlobalPosition(d->scene,d->actors[i],d->positions[i]))!=XGE_OK) goto fail;
        }
        d->last_action=d->action;d->actions|=1u<<d->action;
    }
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
    if ((result=ui_draw(d))!=XGE_OK) goto fail;
#endif
    xge3d_camera_t camera;if ((result=xge3dCameraPerspective(&camera,.9f,640.f/480,.1f,100))!=XGE_OK) goto fail;
    double angle=d->frame*.001;
    if ((result=xge3dCameraLookAtGlobal(&camera,d->origin,(xge3d_dvec3_t){1000000000+10*cos(angle),8,-1000000000+13+5*sin(angle)},(xge3d_dvec3_t){1000000000,1,-1000000000},(xge3d_vec3_t){0,1,0}))!=XGE_OK) goto fail;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.cascades=2;shadows.resolution=1024;shadows.distance=40;shadows.depth_padding=12;
    shadows.bias=.005f;shadows.normal_bias=.08f;
    xge3d_render_desc_t draw={&camera,d->target,{0,0,0,1},3,d->sky};draw.shadows=&shadows;draw.exposure=1;
#if !XGE_ENABLE_2D
    draw.target=NULL;
#endif
    xge3d_render_stats_t stats;if ((result=xge3dRender(d->renderer,d->scene,&draw,&stats))!=XGE_OK) goto fail;
#if XGE_ENABLE_2D
    xge_pass_t pass;xgePassInit(&pass,&d->composite,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(0,0,0,255));
    if ((result=xgePassBegin(&pass))!=XGE_OK) goto fail;
    xge_draw_t blit={0};blit.pTexture=(xge_texture)xge3dTargetTexture(d->target);blit.tSrc=(xge_rect_t){0,0,640,480};blit.tDst=blit.tSrc;
    blit.iFlags=XGE_DRAW_FLIP_Y;blit.iColor=XGE_COLOR_RGBA(255,255,255,255);xgeDrawEx(&blit);
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
    result=d->proxy.surfaceDraw(&d->proxy,d->ui_target,(xui_rect_t){0,0,280,44},(xui_rect_t){8,8,280,44},XUI_COLOR_RGBA(255,255,255,255),0);
#endif
    int end=xgePassEnd(&pass);if (result==XGE_OK) result=end;if (result!=XGE_OK) goto fail;
    blit.pTexture=xgeRenderTargetTexture(&d->composite);blit.tDst=(xge_rect_t){0,0,(float)xgeGetWidth(),(float)xgeGetHeight()};xgeDrawEx(&blit);
#endif
    if (d->ready==15) ++d->frame;
    if ((d->frames && d->frame==d->frames) || xgeKeyPressed(XGE_KEY_ESCAPE)) {
        if (d->capture) {
            xarray pixels;xrtArrayInit(&pixels,1);if (!xrtArrayResize(&pixels,640*480*4)) {result=XGE_ERROR_OUT_OF_MEMORY;goto fail;}
#if XGE_ENABLE_2D
            result=xgeRenderTargetReadPixels(&d->composite,pixels.Data,640*4);
#else
            draw.target=d->target;result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,640*4);
#endif
            if (result==XGE_OK) result=xgeImageSavePNG(d->capture,640,480,pixels.Data,640*4);
            xrtArrayUnit(&pixels);if (result!=XGE_OK) goto fail;
        }
        printf("integration: %d frames, %d/15 loaded, %llu budgeted bytes, actions 0x%x, %u draws/%u shadow maps, %zu exported triangles/%d walkable, %d ground queries, %d move/%d delete/%d rebase\n",d->frame,d->ready,(unsigned long long)d->uploads,d->actions,stats.draw_calls,stats.shadow_maps,d->triangles.Count,d->walkable,d->queries,d->moves,d->deletes,d->rebases);
        if (d->frames>=360 && (d->actions!=63 || d->ready!=15 || !d->moves || !d->deletes || !d->rebases || d->queries!=d->frames*2 || !stats.instanced_draw_calls || stats.shadow_maps!=3)) {result=XGE_ERROR_INVALID_STATE;goto fail;}
        release(d);return 1;
    }
    return 0;
fail:
    fprintf(stderr,"integration failed: %d, frame %d, ready %d\n",result,d->frame,d->ready);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={.last_action=-1};xrtArrayInit(&d.triangles,sizeof(triangle));
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i],"--frames") && i+1<argc) {d.frames=atoi(argv[++i]);if (d.frames<=0) return 2;}
        else if (!strcmp(argv[i],"--capture") && i+1<argc) d.capture=argv[++i];else return 2;
    }
    int result=build_scene(&d);if (result==XGE_OK) result=xge3dLoaderCreate(NULL,&d.loader);
    for (int i=0;i<15 && result==XGE_OK;++i) {
        char path[256];if (i<2) snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/rig-%c.gltf",'a'+i);
        else if (i==2) snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/async.gltf");
        else snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/retarget-%c-%s.gltf",'a'+(i-3)/6,actions[(i-3)%6]);
        result=xge3dLoaderRequest(d.loader,path,&d.requests[i]);
    }
    xge_desc_t desc={0};desc.iWidth=640;desc.iHeight=480;desc.sTitle="XGE 3D integration - click action, Escape exits";
    if (result==XGE_OK) result=xgeInit(&desc);
    d.start=xrtClock();
    if (result==XGE_OK) result=xgeRun(frame,&d);
    if (result!=XGE_OK) fprintf(stderr,"integration startup/run failed: %d\n",result);
    release(&d);xgeUnit();return result==XGE_OK && !d.failed ? 0 : 1;
}
