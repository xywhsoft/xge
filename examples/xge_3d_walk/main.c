#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tutorial policy stays here: island generation, movement, gravity and camera.
 * XGE supplies copied heightfields, triangle sampling, skins, sky and rendering. */
#define GRID 257
#define CELL 1.5f
#define HALF ((GRID-1)*CELL*.5f)
#define TRY(call) do {int code=(call);if (code!=XGE_OK) return code;} while(0)
typedef struct controls {float forward,side,turn;int run,jump;} controls;
typedef struct demo {
    xge3d_scene *scene;xge3d_renderer *renderer;xge3d_target *target;
    xge3d_terrain *terrain;xge3d_environment *sky;xge3d_animator *animator;xge3d_clip *clips[4];
    xge3d_node_t land,actor;xge3d_vec3_t position,eye;
    float yaw,pitch,distance,facing,velocity,travel,min_height,max_height;double previous;
    unsigned seed;int frame,frames,failed,autopilot,action,grounded,jumps,landings,blocked,regenerated,first_person;
    const char *capture;
} demo;
static float clamp(float v,float a,float b) {return fminf(b,fmaxf(a,v));}
static float smooth(float v) {return v*v*(3-2*v);}
static float noise(const float *grid,float x,float z)
{
    int ix=(int)x,iz=(int)z;float a=smooth(x-ix),b=smooth(z-iz);
    float p=grid[iz*66+ix],q=grid[iz*66+ix+1],r=grid[(iz+1)*66+ix],s=grid[(iz+1)*66+ix+1];
    return (p+(q-p)*a)*(1-b)+(r+(s-r)*a)*b;
}
static void release(demo *d)
{
    xge3dAnimatorFree(d->animator);d->animator=NULL;
    for (int i=0;i<4;++i) {xge3dClipFree(d->clips[i]);d->clips[i]=NULL;}
    xge3dSceneFree(d->scene);d->scene=NULL;xge3dTerrainFree(d->terrain);d->terrain=NULL;
    xge3dEnvironmentFree(d->sky);d->sky=NULL;xge3dTargetFree(d->target);d->target=NULL;
    xge3dRendererFree(d->renderer);d->renderer=NULL;
}
static int ground(demo *d,float x,float z,float *height,xge3d_vec3_t *normal)
{return xge3dTerrainSample(d->terrain,0,x+HALF,z+HALF,height,normal);}
static int reset_player(demo *d)
{
    d->position=(xge3d_vec3_t){40,0,110};xge3d_vec3_t normal;
    TRY(ground(d,d->position.x,d->position.z,&d->position.y,&normal));
    d->velocity=0;d->grounded=1;d->yaw=3.14159265f;d->pitch=.20f;d->distance=7;d->facing=0;return XGE_OK;
}
static int make_terrain(demo *d)
{
    xarray heights,pixels;xrtArrayInit(&heights,sizeof(float));xrtArrayInit(&pixels,4);
    xge3d_texture *texture=NULL;xge3d_material *material=NULL;xge3d_terrain *terrain=NULL;xge3d_node_t root={0};
    int result=XGE_ERROR_OUT_OF_MEMORY;
    if (!xrtArrayResize(&heights,GRID*GRID) || !xrtArrayResize(&pixels,GRID*GRID)) goto done;
    float *h=(float*)heights.Data;memset(h,0,GRID*GRID*sizeof(float));float samples[66*66];
    xrng rng;xrtRngSeed(&rng,d->seed,42);float amplitude=6;
    for (int step=64;step>=4;step/=2,amplitude*=.5f) {
        for (int i=0;i<66*66;++i) samples[i]=(float)(xrtRngReal(&rng)*2-1);
        for (int z=0;z<GRID;++z) for (int x=0;x<GRID;++x) h[z*GRID+x]+=amplitude*noise(samples,x/(float)step,z/(float)step);
    }
    d->min_height=1e6f;d->max_height=-1e6f;
    for (int z=0;z<GRID;++z) for (int x=0;x<GRID;++x) {
        float wx=(x-128)*CELL,wz=(z-128)*CELL,radius=sqrtf(wx*wx+wz*wz);
        float value=h[z*GRID+x]+12+2*sinf(wx*.025f)-32*smooth(clamp((radius-115)/65,0,1));h[z*GRID+x]=value;
        d->min_height=fminf(d->min_height,value);d->max_height=fmaxf(d->max_height,value);
        float beach=clamp((value-.4f)/3.5f,0,1),rock=clamp((value-14)/5,0,1),grain=.90f+(float)xrtRngReal(&rng)*.18f;
        const float sand[]={190,172,113},grass[]={72,112,43},stone[]={127,128,112};unsigned char *p=pixels.Data+(z*GRID+x)*4;
        for (int k=0;k<3;++k) p[k]=(unsigned char)clamp(((sand[k]*(1-beach)+grass[k]*beach)*(1-rock)+stone[k]*rock)*grain,0,255);
        p[3]=255;
    }
    xge_image_t image={GRID,GRID,XGE_PIXEL_RGBA8,GRID*4,pixels.Data,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_texture_desc_t td={&image,1,{0}};result=xge3dTextureCreate(&td,&texture);
    xge3d_material_desc_t md=xge3dMaterialDefault();md.metallic=0;md.roughness=1;
    md.emissive[0]=.018f;md.emissive[1]=.022f;md.emissive[2]=.012f;md.maps[0].texture=texture;
    if (result==XGE_OK) result=xge3dMaterialCreate(&md,&material);
    xge3d_terrain_desc_t desc=xge3dTerrainDefault();desc.heights=h;desc.width=desc.depth=GRID;desc.cell_size=CELL;
    desc.chunk_cells=32;desc.lod_count=4;desc.lod_distances[0]=90;desc.lod_distances[1]=180;desc.lod_distances[2]=300;desc.material=material;
    if (result==XGE_OK) result=xge3dTerrainCreate(&desc,&terrain);
    if (result==XGE_OK) result=xge3dTerrainInstantiate(d->scene,terrain,(xge3d_node_t){0},&root);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){-HALF,0,-HALF};
    if (result==XGE_OK) result=xge3dNodeSetTransform(d->scene,root,&t);
    if (result==XGE_OK && d->land.slot) result=xge3dNodeDestroy(d->scene,d->land);
    if (result==XGE_OK) {xge3dTerrainFree(d->terrain);d->terrain=terrain;d->land=root;terrain=NULL;result=reset_player(d);}
    else if (root.slot) xge3dNodeDestroy(d->scene,root);
done:
    xge3dTerrainFree(terrain);xge3dMaterialFree(material);xge3dTextureFree(texture);xrtArrayUnit(&heights);xrtArrayUnit(&pixels);return result;
}
static int make_sky(demo *d)
{
    enum {SIDE=128};xarray pixels;xrtArrayInit(&pixels,1);
    if (!xrtArrayResize(&pixels,6*SIDE*SIDE*4)) return XGE_ERROR_OUT_OF_MEMORY;
    const xge3d_vec3_t normals[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    const xge3d_vec3_t us[]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
    const xge3d_vec3_t vs[]={{0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0}};
    xge_image_t images[6];xge3d_environment_desc_t desc={0};desc.srgb=1;
    for (int f=0;f<6;++f) {
        unsigned char *face=pixels.Data+f*SIDE*SIDE*4;
        for (int y=0;y<SIDE;++y) for (int x=0;x<SIDE;++x) {
            float a=(x+.5f)*2/SIDE-1,b=(y+.5f)*2/SIDE-1;
            xge3d_vec3_t v={normals[f].x+us[f].x*a+vs[f].x*b,normals[f].y+us[f].y*a+vs[f].y*b,normals[f].z+us[f].z*a+vs[f].z*b};
            float length=sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);v.x/=length;v.y/=length;v.z/=length;
            float top=powf(clamp(v.y,0,1),.45f),cloud=0;
            if (v.y>.12f && v.y<.65f) cloud=powf(clamp(.5f+.25f*sinf(v.x*19+v.z*13)+.25f*sinf(v.z*31-v.x*9),0,1),9)*.25f;
            const float horizon[]={199,222,237},zenith[]={50,116,184};unsigned char *p=face+(y*SIDE+x)*4;
            float sun=powf(fmaxf(0,v.x*.36f+v.y*.72f+v.z*.594f),512);
            for (int k=0;k<3;++k) p[k]=(unsigned char)clamp((horizon[k]*(1-top)+zenith[k]*top)*(1-cloud)+255*cloud+sun*100,0,255);
            p[3]=255;
        }
        images[f]=(xge_image_t){SIDE,SIDE,XGE_PIXEL_RGBA8,SIDE*4,face,XGE_IMAGE_STRAIGHT_ALPHA};desc.faces[f]=&images[f];
    }
    int result=xge3dEnvironmentCreate(&desc,&d->sky);xrtArrayUnit(&pixels);return result;
}
static int make_sea(demo *d)
{
    xge3d_vertex_t vertices[6]={0};const float corners[6][2]={{-1,-1},{-1,1},{1,1},{-1,-1},{1,1},{1,-1}};
    for (int i=0;i<6;++i) {vertices[i].position=(xge3d_vec3_t){corners[i][0]*3000,0,corners[i][1]*3000};vertices[i].normal.y=1;}
    xge3d_mesh_desc_t desc={vertices,6};xge3d_mesh *mesh=NULL;xge3d_material *material=NULL;xge3d_node_t node;
    int result=xge3dMeshCreate(&desc,&mesh);xge3d_material_desc_t md=xge3dMaterialDefault();
    md.metallic=0;md.roughness=.25f;md.unlit=1;md.base_color[0]=.026f;md.base_color[1]=.19f;md.base_color[2]=.26f;
    if (result==XGE_OK) result=xge3dMaterialCreate(&md,&material);
    if (result==XGE_OK) result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&node);
    if (result==XGE_OK) result=xge3dNodeSetMesh(d->scene,node,mesh);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(d->scene,node,material);
    xge3dMeshFree(mesh);xge3dMaterialFree(material);return result;
}
static int make_actor(demo *d)
{
    char *directory=xrtPathAppDir(),*path=directory ? xrtPathJoin(directory,"assets/explorer.gltf") : NULL;xrtFree(directory);
    if (!path) return XGE_ERROR_OUT_OF_MEMORY;
    xge3d_model *model=NULL;int result=xge3dModelLoad(path,&model);xrtFree(path);
    if (result==XGE_OK) result=xge3dModelInstantiate(d->scene,model,(xge3d_node_t){0},&d->actor);
    if (result==XGE_OK) result=xge3dAnimatorCreate(d->scene,d->actor,&d->animator);
    for (int i=0;i<4 && result==XGE_OK;++i) result=xge3dClipFromModel(model,i,&d->clips[i]);
    xge3dModelFree(model);d->action=-1;return result;
}
static int update_player(demo *d,controls c,float dt)
{
    d->yaw+=c.turn*dt;float length=sqrtf(c.forward*c.forward+c.side*c.side);
    float vx=0,vz=0,speed=c.run ? 6.4f : 3.2f;
    if (length>0) {vx=(-sinf(d->yaw)*c.forward+cosf(d->yaw)*c.side)*speed/length;vz=(-cosf(d->yaw)*c.forward-sinf(d->yaw)*c.side)*speed/length;}
    if (c.jump && d->grounded) {d->velocity=6.5f;d->grounded=0;++d->jumps;}
    int steps=(int)ceilf(dt*120);if (steps<1) steps=1;float step=dt/steps,moved=0;
    for (int i=0;i<steps;++i) {
        float nx=d->position.x+vx*step,nz=d->position.z+vz*step,height;xge3d_vec3_t normal;
        int valid=ground(d,nx,nz,&height,&normal);
        if (valid!=XGE_OK || height<.25f || normal.y<.68f) {
            ++d->blocked;TRY(ground(d,d->position.x,d->position.z,&height,&normal));
        } else {float distance=sqrtf(vx*vx+vz*vz)*step;d->travel+=distance;moved+=distance;d->position.x=nx;d->position.z=nz;}
        if (d->grounded) d->position.y=height;
        else {
            d->velocity-=18*step;d->position.y+=d->velocity*step;
            if (d->position.y<=height) {d->position.y=height;d->velocity=0;d->grounded=1;++d->landings;}
        }
    }
    if (moved>0) d->facing=atan2f(vx,vz);
    int action=!d->grounded ? 3 : moved>0 ? c.run ? 2 : 1 : 0;
    if (action!=d->action) {
        xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=action!=3;
        TRY(xge3dAnimatorSetLayer(d->animator,0,d->clips[action],&layer));d->action=action;
    }
    TRY(xge3dAnimatorUpdate(d->animator,dt));
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=d->position;t.rotation=(xge3d_quat_t){0,sinf(d->facing*.5f),0,cosf(d->facing*.5f)};
    return xge3dNodeSetTransform(d->scene,d->actor,&t);
}
static int camera(demo *d,xge3d_camera_t *out)
{
    TRY(xge3dCameraPerspective(out,.95f,(float)xgeGetWidth()/xgeGetHeight(),.08f,2500));
    xge3d_vec3_t target=d->position;target.y+=1.2f;
    xge3d_vec3_t offset={sinf(d->yaw)*cosf(d->pitch),sinf(d->pitch),cosf(d->yaw)*cosf(d->pitch)};
    if (d->first_person) {
        d->eye=d->position;d->eye.y+=1.67f;target=(xge3d_vec3_t){d->eye.x-offset.x,d->eye.y-offset.y,d->eye.z-offset.z};
    } else {
        float distance=d->distance;
        /* Stop the camera boom before the terrain, then keep the eye above it. */
        for (int i=1;i<=24;++i) {
            float s=d->distance*i/24,height;xge3d_vec3_t normal;
            if (ground(d,target.x+offset.x*s,target.z+offset.z*s,&height,&normal)==XGE_OK && target.y+offset.y*s<height+.45f) {distance=fmaxf(.6f,d->distance*(i-1)/24);break;}
        }
        d->eye=(xge3d_vec3_t){target.x+offset.x*distance,target.y+offset.y*distance,target.z+offset.z*distance};
        float h;xge3d_vec3_t normal;if (ground(d,d->eye.x,d->eye.z,&h,&normal)==XGE_OK) d->eye.y=fmaxf(d->eye.y,h+.45f);
    }
    TRY(xge3dNodeSetVisible(d->scene,d->actor,!d->first_person));
    return xge3dCameraLookAt(out,d->eye,target,(xge3d_vec3_t){0,1,0});
}
#if XGE_ENABLE_2D
/* Small application bitmap labels keep this demo independent of font/XUI libraries. */
static void label(float x,float y,const char *text,uint32_t color)
{
    static const unsigned char glyphs[][5]={
        {62,81,73,69,62},{0,66,127,64,0},{66,97,81,73,70},{33,65,69,75,49},{24,20,18,127,16},
        {39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30},
        {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},
        {127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},
        {127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},
        {127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},
        {63,64,64,64,63},{31,32,64,32,31},{127,32,24,32,127},{99,20,8,20,99},{3,4,120,4,3},{97,81,73,69,67}};
    for (;*text;++text,x+=12) {
        int c=*text,index=c>='0' && c<='9' ? c-'0' : c>='A' && c<='Z' ? c-'A'+10 : -1;
        for (int col=0;col<5;++col) for (int row=0;row<7;++row)
            if ((index>=0 && (glyphs[index][col]&(1<<row))) || (c=='-' && row==3) || (c=='.' && col==2 && row==6))
                xgeShapeRectFill((xge_rect_t){x+col*2,y+row*2,2,2},color);
    }
}
static void hud(demo *d)
{
    xgeShapeRectFill((xge_rect_t){14,14,570,122},XGE_COLOR_RGBA(13,27,33,205));
    label(28,26,"XGE 3D - ISLAND WALK",XGE_COLOR_RGBA(255,207,116,255));
    label(28,49,"WASD MOVE  SHIFT RUN  SPACE JUMP",XGE_COLOR_RGBA(230,241,245,255));
    label(28,70,"RMB DRAG  QE ORBIT  WHEEL ZOOM",XGE_COLOR_RGBA(230,241,245,255));
    label(28,91,"F VIEW  R RESET  G NEW ISLAND  ESC EXIT",XGE_COLOR_RGBA(230,241,245,255));
    char line[100];snprintf(line,sizeof(line),"SEED %u  HEIGHT %.1f  %s",d->seed,d->position.y,(const char*[]){"IDLE","WALK","RUN","JUMP"}[d->action]);
    label(28,112,line,XGE_COLOR_RGBA(138,214,182,255));
    float x=xgeGetWidth()*.5f,y=xgeGetHeight()*.5f;
    if (d->first_person) {xgeShapeRectFill((xge_rect_t){x-5,y,11,1},0xffffffff);xgeShapeRectFill((xge_rect_t){x,y-5,1,11},0xffffffff);}
}
#endif
static int capture(demo *d,xge3d_render_desc_t *draw)
{
    int width=xgeGetWidth(),height=xgeGetHeight();xarray pixels;xrtArrayInit(&pixels,1);
    if (!xrtArrayResize(&pixels,(size_t)width*height*4)) return XGE_ERROR_OUT_OF_MEMORY;
    int result=xge3dTargetCreate(width,height,&d->target);draw->target=d->target;
    if (result==XGE_OK) result=xge3dRender(d->renderer,d->scene,draw,NULL);
    if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,width*4);
    if (result==XGE_OK) result=xgeImageSavePNG(d->capture,width,height,pixels.Data,width*4);
    xrtArrayUnit(&pixels);return result;
}
static int frame(void *user)
{
    demo *d=user;int result=XGE_OK;
    if (xgeGetWidth()<=0 || xgeGetHeight()<=0) {d->previous=xgeTimer();return 0;}
    if (!d->renderer && (result=xge3dRendererCreate(&d->renderer))!=XGE_OK) goto fail;
    double now=xgeTimer();float dt=d->autopilot ? 1.f/60 : (float)clamp((float)(now-d->previous),.001f,.05f);d->previous=now;
    controls c={0};
    c.forward=xgeKeyDown('W')-xgeKeyDown('S')+xgeKeyDown(XGE_KEY_UP)-xgeKeyDown(XGE_KEY_DOWN);
    c.side=xgeKeyDown('D')-xgeKeyDown('A');c.turn=(xgeKeyDown('E')-xgeKeyDown('Q'))*1.5f;
    c.run=xgeKeyDown(XGE_KEY_LEFT_SHIFT)||xgeKeyDown(XGE_KEY_RIGHT_SHIFT);c.jump=xgeKeyPressed(XGE_KEY_SPACE);
    if (xgeMouseDown(XGE_MOUSE_RIGHT)) {float dx,dy;xgeMouseGetDelta(&dx,&dy);d->yaw-=dx*.005f;d->pitch=clamp(d->pitch+dy*.004f,.04f,1.2f);}
    float wx,wy;xgeMouseGetWheel(&wx,&wy);(void)wx;d->distance=clamp(d->distance-wy,2.5f,18);
    if (xgeKeyPressed('F')) d->first_person=!d->first_person;
    if (xgeKeyPressed('R') && (result=reset_player(d))!=XGE_OK) goto fail;
    if (xgeKeyPressed('G') || (d->autopilot && d->frame==360)) {++d->seed;if ((result=make_terrain(d))!=XGE_OK) goto fail;++d->regenerated;}
    if (d->autopilot) {
        c.forward=d->frame%300<240 ? 1 : 0;c.side=d->frame%300>=180 && d->frame%300<240 ? .65f : 0;
        c.run=d->frame%300>=90 && d->frame%300<180;c.jump=d->frame%180==70;c.turn=.14f;
    }
    if ((result=update_player(d,c,dt))!=XGE_OK) goto fail;
    xge3d_camera_t view;if ((result=camera(d,&view))!=XGE_OK) goto fail;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.cascades=3;shadows.resolution=1024;shadows.distance=130;shadows.depth_padding=25;
    shadows.bias=.0015f;shadows.normal_bias=.06f;
    xge3d_fog_settings_t fog=xge3dFogDefault();fog.start=90;fog.end=600;
    xge3d_render_desc_t draw={&view,NULL,{0,0,0,1},3,d->sky};draw.shadows=&shadows;draw.fog=&fog;
    xge3d_render_stats_t stats;if ((result=xge3dRender(d->renderer,d->scene,&draw,&stats))!=XGE_OK) goto fail;
#if XGE_ENABLE_2D
    hud(d);
#endif
    if (d->frame%30==0) {char title[160];snprintf(title,sizeof(title),"XGE 3D Island Walk | %.1f FPS | seed %u | y %.2f | WASD / SHIFT / SPACE / RMB / G",dt>0 ? 1/dt : 0,d->seed,d->position.y);xgeSetTitle(title);}
    ++d->frame;
    if ((d->frames && d->frame>=d->frames) || xgeKeyPressed(XGE_KEY_ESCAPE)) {
        if (d->capture && (result=capture(d,&draw))!=XGE_OK) goto fail;
        if (d->autopilot && d->frames>=600 && (d->travel<20 || d->jumps<3 || d->landings<3 || d->regenerated!=1)) {result=XGE_ERROR_INVALID_STATE;goto fail;}
        printf("island walk: %d frames, seed %u, heights %.2f..%.2f, %.2f m travel, %d jumps/%d landings, %d regenerated, %u draws/%u shadow maps\n",d->frame,d->seed,d->min_height,d->max_height,d->travel,d->jumps,d->landings,d->regenerated,stats.draw_calls,stats.shadow_maps);
        release(d);return 1;
    }
    return 0;
fail:
    fprintf(stderr,"island walk failed: %d at frame %d\n",result,d->frame);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={.seed=20261003};
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i],"--frames") && i+1<argc) {d.frames=atoi(argv[++i]);if (d.frames<=0) return 2;}
        else if (!strcmp(argv[i],"--seed") && i+1<argc) d.seed=(unsigned)strtoul(argv[++i],NULL,10);
        else if (!strcmp(argv[i],"--capture") && i+1<argc) d.capture=argv[++i];
        else if (!strcmp(argv[i],"--autopilot")) d.autopilot=1;
        else if (!strcmp(argv[i],"--first-person")) d.first_person=1;
        else {fprintf(stderr,"Usage: %s [--seed N] [--frames N] [--autopilot] [--first-person] [--capture PNG]\n",argv[0]);return 2;}
    }
    xge_desc_t window={0};window.iWidth=1100;window.iHeight=700;window.sTitle="XGE 3D Island Walk";window.iTargetFPS=60;
    if (xgeInit(&window)!=XGE_OK) return 1;
    int result=xge3dSceneCreate(&d.scene);
    if (result==XGE_OK) result=make_terrain(&d);
    if (result==XGE_OK) result=make_sky(&d);
    if (result==XGE_OK) result=make_sea(&d);
    if (result==XGE_OK) result=make_actor(&d);
    xge3d_node_t sun;xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);
    light.direction=(xge3d_vec3_t){-.36f,-.72f,-.594f};light.intensity=3.5f;light.casts_shadow=1;
    if (result==XGE_OK) result=xge3dNodeCreate(d.scene,(xge3d_node_t){0},&sun);
    if (result==XGE_OK) result=xge3dNodeSetLight(d.scene,sun,&light);
    light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);light.direction=(xge3d_vec3_t){.3f,-.7f,.65f};
    light.color=(xge3d_vec3_t){.72f,.83f,1};light.intensity=.8f;
    if (result==XGE_OK) result=xge3dNodeCreate(d.scene,(xge3d_node_t){0},&sun);
    if (result==XGE_OK) result=xge3dNodeSetLight(d.scene,sun,&light);
    d.previous=xgeTimer();
    if (result==XGE_OK) result=xgeRun(frame,&d);
    if (result!=XGE_OK) fprintf(stderr,"island walk startup/run failed: %d\n",result);
    release(&d);xgeUnit();return result==XGE_OK && !d.failed ? 0 : 1;
}
