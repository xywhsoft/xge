#include "xge.h"
#include "xui.h"
#include "xrt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A real window/GPU is mandatory. No headless success or unsupported fallback. */
static int checks, failed = 1;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"orientation FAIL %d: %s\n",__LINE__,#x); goto cleanup; } } while (0)
#define OK(x) CHECK((x) == XGE_OK)
static unsigned char pattern[8*8*4], pixels[8*8*4];

static int same_crop(const unsigned char *actual, int stride, int sx, int sy, int w, int h, int flags)
{
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        int px=sx+((flags&XGE_DRAW_FLIP_X) ? w-1-x : x);
        int py=sy+((flags&XGE_DRAW_FLIP_Y) ? h-1-y : y);
        if (memcmp(actual+y*stride+x*4,pattern+(py*8+px)*4,4)) {
            fprintf(stderr,"pixel mismatch at %d,%d source %d,%d actual=%u,%u,%u expected=%u,%u,%u\n",
                x,y,px,py,actual[y*stride+x*4],actual[y*stride+x*4+1],actual[y*stride+x*4+2],
                pattern[(py*8+px)*4],pattern[(py*8+px)*4+1],pattern[(py*8+px)*4+2]);
            return 0;
        }
    }
    return 1;
}

static int render(xge_render_target dst, xge_texture src, xge_shader shader, int mode,
                  xge_rect_t crop, uint32_t flags)
{
    xge_pass_t pass={0}; xge_material_t material={0}; xge_sprite_batch_t batch={0};
    xge_mesh_t mesh={0}; xge_vertex_t quad[4]={0}; xge_nine_patch_t patch;
    int result=0;
    xge_draw_t d={0}; d.pTexture=src; d.tSrc=crop;
    d.tDst=(xge_rect_t){0,0,crop.fW,crop.fH}; d.iColor=0xffffffffu;
    d.iFlags=flags|XGE_DRAW_SCREEN_SPACE;
    xgePassInit(&pass,dst,XGE_PASS_CLEAR_COLOR,0x010203ffu);
    if (xgePassBegin(&pass)!=XGE_OK) return 0;
    xgeBlendSet(XGE_BLEND_NONE);
    if (mode==0) xgeDrawEx(&d);
    if (mode==1 || mode==3) {
        xgeMaterialInit(&material); xgeMaterialSetShader(&material,shader);
        xgeMaterialSetTexture(&material,src); xgeMaterialSetBlend(&material,XGE_BLEND_NONE);
        if (mode==1) xgeMaterialDraw(&material,&d);
    }
    if (mode==2 || mode==3) {
        if (xgeSpriteBatchInit(&batch,src,1,0)!=XGE_OK || xgeSpriteBatchAdd(&batch,&d)!=XGE_OK) goto cleanup;
        if (xgeSpriteBatchFlushMaterial(&batch,mode==3?&material:NULL)!=XGE_OK) goto cleanup;
    }
    if (mode==4 || mode==5) {
        /* Vertex UVs describe this cropped image, including a requested local mirror. */
        float u0=crop.fX/8, v0=crop.fY/8, u1=(crop.fX+crop.fW)/8, v1=(crop.fY+crop.fH)/8;
        if (flags&XGE_DRAW_FLIP_X) { float t=u0;u0=u1;u1=t; }
        if (flags&XGE_DRAW_FLIP_Y) { float t=v0;v0=v1;v1=t; }
        for (int i=0;i<4;++i) {
            quad[i].fX=(i&1)?crop.fW:0; quad[i].fY=(i&2)?crop.fH:0;
            quad[i].fW=1; quad[i].fU=(i&1)?u1:u0; quad[i].fV=(i&2)?v1:v0;
            quad[i].iColor=0xffffffffu;
        }
        uint32_t vf=XGE_DRAW_SCREEN_SPACE|(flags&XGE_DRAW_RAW_UV);
        if (mode==4) xgeDrawQuad3D(src,quad,vf);
        else {
            const uint16_t indices[]={0,1,2,2,1,3};
            if (xgeMeshCreate(&mesh,quad,4,indices,6,0)!=XGE_OK) goto cleanup;
            xgeMeshDraw(&mesh,src,vf);
        }
    }
    if (mode==6) {
        xgeNinePatchInit(&patch,src,crop,0,0,crop.fW,crop.fH);
        xgeNinePatchDraw(&patch,d.tDst,d.iFlags);
    }
    result=xgePassEnd(&pass)==XGE_OK && xgeRenderTargetReadPixels(dst,pixels,32)==XGE_OK;
cleanup:
    if (pass.bActive) xgePassEnd(&pass);
    xgeSpriteBatchFree(&batch); xgeMaterialFree(&material); xgeMeshFree(&mesh);
    return result;
}

static int frame(void *unused)
{
    (void)unused;
    xge_texture_t image={0}, decoded={0};
    xge_render_target_t source={0}, target={0}, nested={0}, window={0}, captured={0};
    xge_pass_t pass={0}; xge_shader_t shader={0}, mixed={0}; xge_material_t material={0};
    xui_proxy_t proxy=xuiProxyXge(); xui_surface surface=NULL, copy=NULL;
    xge3d_renderer *renderer=NULL; xge3d_target *target3d=NULL;
    xge3d_scene *scene=NULL; xge3d_mesh *mesh3d=NULL;
    void *png=NULL; size_t png_size=0;
    unsigned char *screen=NULL;
    int begun=0;
    const char *vertex="#version 330 core\nlayout(location=0) in vec4 aPos;layout(location=1) in vec2 aUV;"
        "uniform vec2 uResolution;out vec2 vUV;void main(){vUV=aUV;gl_Position=vec4(aPos.x/uResolution.x*2.-1.,1.-aPos.y/uResolution.y*2.,0,1);}";
    const char *fragment="#version 330 core\nin vec2 vUV;out vec4 FragColor;uniform sampler2D uTexture;void main(){FragColor=texture(uTexture,vUV);}";
    const char *mixed_fragment="#version 330 core\nin vec2 vUV;out vec4 FragColor;"
        "uniform sampler2D uTexture;uniform sampler2D uTexture2;uniform sampler2D uTexture3;"
        XGE_GLSL_MATERIAL_UV
        "void main(){FragColor=(texture(uTexture,vUV)+texture(uTexture2,xgeMaterialUV2(vUV))+texture(uTexture3,xgeMaterialUV3(vUV)))/3.;}";
    for (int y=0;y<8;++y) for (int x=0;x<8;++x) {
        unsigned char *p=pattern+(y*8+x)*4;
        p[0]=(unsigned char)(20+x*27); p[1]=(unsigned char)(15+y*29);
        p[2]=(unsigned char)(30+(x*3+y*11)%200); p[3]=255;
    }
    CHECK(xgeTextureCoordinateContract()==XGE_TEXTURE_COORDINATE_CONTRACT);
    OK(xgeBegin()); begun=1;
    OK(xgeTextureCreateRGBAEx(&image,8,8,pattern,XGE_TEXTURE_COMPRESS_NONE));
    OK(xgeRenderTargetCreate(&source,8,8)); OK(xgeRenderTargetCreate(&target,8,8));
    OK(xgeRenderTargetCreate(&nested,8,8));
    xge_sampler_t sampler=xgeSamplerDefault(); sampler.iMinFilter=sampler.iMagFilter=XGE_FILTER_NEAREST;
    OK(xgeTextureSetSampler(&image,&sampler)); OK(xgeTextureSetSampler(&source.tTexture,&sampler));
    OK(xgeTextureSetSampler(&nested.tTexture,&sampler));
    OK(xgeShaderCreate(&shader,vertex,fragment)); OK(xgeShaderCreate(&mixed,vertex,mixed_fragment));
    CHECK(render(&source,&image,&shader,0,(xge_rect_t){0,0,8,8},0));
    CHECK(same_crop(pixels,32,0,0,8,8,0));
    CHECK(xgeTextureReadPixels(&source.tTexture,pixels,32)==XGE_ERROR_INVALID_STATE);
    xge_vec2_t uv=xgeTextureMapUV(&source.tTexture,(xge_vec2_t){.25f,.125f});
    CHECK(uv.fX==.25f && uv.fY==.875f);
#ifdef XGE_ORIENTATION_NEGATIVE_CONTROL
    /* Simulate a missing producer-origin tag; the rendered pixel checks must fail. */
    source.tTexture.iFlags &= ~XGE_TEXTURE_ORIGIN_BOTTOM_LEFT;
#endif
    const xge_rect_t crops[]={{0,0,8,8},{1,1,4,3},{0,0,8,2},{0,6,8,2},{2,0,3,8}};
    for (int kind=0;kind<2;++kind) for (int mode=0;mode<7;++mode)
        for (int c=0;c<5;++c) for (int flip=0;flip<4;++flip) {
            CHECK(render(&target,kind?&source.tTexture:&image,&shader,mode,crops[c],(uint32_t)flip));
            if (!same_crop(pixels,32,(int)crops[c].fX,(int)crops[c].fY,(int)crops[c].fW,(int)crops[c].fH,flip)) {
                fprintf(stderr,"kind=%d mode=%d crop=%d flip=%d\n",kind,mode,c,flip); CHECK(0);
            }
            ++checks;
        }
    /* Legacy/raw opt-in reproduces the previous native coordinates exactly. */
    CHECK(render(&target,&source.tTexture,&shader,0,(xge_rect_t){0,0,8,8},XGE_DRAW_RAW_UV));
    CHECK(same_crop(pixels,32,0,0,8,8,XGE_DRAW_FLIP_Y));
    CHECK(render(&target,&source.tTexture,&shader,0,(xge_rect_t){0,0,8,8},XGE_DRAW_RAW_UV|XGE_DRAW_FLIP_Y));
    CHECK(same_crop(pixels,32,0,0,8,8,0));
    /* Independent origins for all three material slots, plus batch material. */
    for (int mask=0;mask<8;++mask) {
        xge_texture a=(mask&1)?&source.tTexture:&image;
        xgeMaterialInit(&material); xgeMaterialSetShader(&material,&mixed);
        xgeMaterialSetTexture(&material,a);
        xgeMaterialSetTexture2(&material,(mask&2)?&source.tTexture:&image);
        xgeMaterialSetTexture3(&material,(mask&4)?&source.tTexture:&image);
        xgeMaterialSetBlend(&material,XGE_BLEND_NONE);
        xge_draw_t d={0}; d.pTexture=a; d.tDst=(xge_rect_t){0,0,8,8}; d.iColor=0xffffffffu; d.iFlags=XGE_DRAW_SCREEN_SPACE;
        xgePassInit(&pass,&target,XGE_PASS_CLEAR_COLOR,0); OK(xgePassBegin(&pass));
        xgeMaterialDraw(&material,&d); OK(xgePassEnd(&pass));
        OK(xgeRenderTargetReadPixels(&target,pixels,32)); CHECK(same_crop(pixels,32,0,0,8,8,0));
        xge_sprite_batch_t batch={0};
        OK(xgeSpriteBatchInit(&batch,a,1,0)); OK(xgeSpriteBatchAdd(&batch,&d));
        xgePassInit(&pass,&target,XGE_PASS_CLEAR_COLOR,0); OK(xgePassBegin(&pass));
        int batch_result=xgeSpriteBatchFlushMaterial(&batch,&material);
        xgeSpriteBatchFree(&batch); OK(batch_result); OK(xgePassEnd(&pass));
        OK(xgeRenderTargetReadPixels(&target,pixels,32)); CHECK(same_crop(pixels,32,0,0,8,8,0));
        xgeMaterialFree(&material);
    }
    CHECK(render(&nested,&source.tTexture,&shader,0,(xge_rect_t){0,0,8,8},0));
    CHECK(render(&target,&nested.tTexture,&shader,0,(xge_rect_t){0,0,8,8},0));
    CHECK(same_crop(pixels,32,0,0,8,8,0));
    /* A top-left upload into an FBO must not modify the mirrored/bottom region. */
    unsigned char patch[8]={255,0,255,255,0,255,255,255};
    OK(xgeTextureUpdateRGBA(&source.tTexture,1,2,1,2,patch,4));
    OK(xgeRenderTargetReadPixels(&source,pixels,32));
    CHECK(!memcmp(pixels+(2*8+1)*4,patch,4) && !memcmp(pixels+(3*8+1)*4,patch+4,4));
    CHECK(!memcmp(pixels+(5*8+1)*4,pattern+(5*8+1)*4,4));
    CHECK(render(&source,&image,&shader,0,(xge_rect_t){0,0,8,8},0));
    /* Resize recreates direction metadata. */
    OK(xgeRenderTargetResize(&source,12,12)); OK(xgeRenderTargetResize(&source,8,8));
    OK(xgeTextureSetSampler(&source.tTexture,&sampler));
    CHECK(render(&source,&image,&shader,0,(xge_rect_t){0,0,8,8},0));
    CHECK(render(&target,&source.tTexture,&shader,0,(xge_rect_t){0,0,8,8},0)); CHECK(same_crop(pixels,32,0,0,8,8,0));
    /* XUI target-to-target cropping, mirroring and arbitrary quads share XGE rules. */
    xui_surface_desc_t sd={0}; sd.iKind=XUI_SURFACE_KIND_TEXTURE; sd.iFormat=XUI_SURFACE_FORMAT_RGBA8;
    sd.iWidth=sd.iHeight=8; sd.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    OK(proxy.surfaceCreate(&proxy,&surface,&sd)); OK(proxy.surfaceCreate(&proxy,&copy,&sd));
    OK(proxy.surfaceUpdateRGBA(&proxy,surface,(xui_rect_i_t){0,0,8,8},pattern,32));
    for (int flip=0;flip<4;++flip) {
        uint32_t f=(flip&1?XUI_SURFACE_DRAW_FLIP_X:0)|(flip&2?XUI_SURFACE_DRAW_FLIP_Y:0);
        OK(proxy.surfaceClear(&proxy,copy,0));
        OK(proxy.surfaceDrawTo(&proxy,copy,surface,(xui_rect_t){1,1,4,3},(xui_rect_t){0,0,4,3},0xffffffffu,f));
        OK(proxy.surfaceReadRGBA(&proxy,copy,pixels,32)); CHECK(same_crop(pixels,32,1,1,4,3,flip));
        xui_surface_vertex_t quad[4]={0};
        for (int i=0;i<4;++i) {
            quad[i].fX=(i&1)?8:0; quad[i].fY=(i&2)?8:0;
            quad[i].fW=1; quad[i].fU=(i&1)?1:0; quad[i].fV=(i&2)?1:0;
            quad[i].iColor=0xffffffffu;
        }
        OK(proxy.surfaceClear(&proxy,copy,0));
        OK(proxy.surfaceDrawQuadTo(&proxy,copy,surface,quad,f|XUI_SURFACE_DRAW_SCREEN_SPACE));
        OK(proxy.surfaceReadRGBA(&proxy,copy,pixels,32)); CHECK(same_crop(pixels,32,0,0,8,8,flip));
    }
    /* The 3D target's displayed image must match its own normalized readback. */
    OK(xge3dRendererCreate(&renderer)); OK(xge3dTargetCreate(8,8,&target3d)); OK(xge3dSceneCreate(&scene));
    xge3d_vertex_t tri[3]={0}; tri[0].position=(xge3d_vec3_t){-.8f,-.7f,0};
    tri[1].position=(xge3d_vec3_t){.8f,-.7f,0}; tri[2].position=(xge3d_vec3_t){-.8f,.7f,0};
    const uint16_t indices[]={0,1,2}; xge3d_mesh_desc_t md={tri,3,indices,3,16};
    OK(xge3dMeshCreate(&md,&mesh3d)); xge3d_node_t node;
    OK(xge3dNodeCreate(scene,(xge3d_node_t){0},&node)); OK(xge3dNodeSetMesh(scene,node,mesh3d));
    const float red[]={1,0,0,1}; OK(xge3dNodeSetColor(scene,node,red));
    xge3d_camera_t camera; OK(xge3dCameraPerspective(&camera,1,1,.1f,20));
    OK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,2},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t rd={&camera,target3d,{0,0,1,1},3}; OK(xge3dRender(renderer,scene,&rd,NULL));
    unsigned char expected3d[sizeof(pixels)]; OK(xge3dTargetReadPixels(target3d,expected3d,sizeof(expected3d),32));
    int asymmetric=0;
    for (int y=0;y<8;++y) asymmetric |= memcmp(expected3d+y*32,expected3d+(7-y)*32,32)!=0;
    CHECK(asymmetric);
    OK(xgeTextureSetSampler((xge_texture)xge3dTargetTexture(target3d),&sampler));
    CHECK(render(&target,(xge_texture)xge3dTargetTexture(target3d),&shader,0,(xge_rect_t){0,0,8,8},0));
    CHECK(!memcmp(pixels,expected3d,sizeof(pixels)));
    OK(xge3dTargetResize(target3d,10,10)); CHECK(xgeTextureUVTransform(xge3dTargetTexture(target3d)).fScaleV==-1);
    /* Final window, capture, PNG encode/decode and a second upload agree. */
    xgeBlendSet(XGE_BLEND_NONE);
    xge_draw_t d={0}; d.pTexture=&nested.tTexture; d.tDst=(xge_rect_t){0,0,8,8};
    d.iFlags=XGE_DRAW_SCREEN_SPACE; d.iColor=0xffffffffu; xgeDrawEx(&d); OK(xgeFlush());
    OK(xgeRenderTargetWindow(&window)); screen=malloc((size_t)window.iWidth*window.iHeight*4); CHECK(screen);
    OK(xgeRenderTargetReadPixels(&window,screen,window.iWidth*4)); CHECK(same_crop(screen,window.iWidth*4,0,0,8,8,0));
    OK(xgeImageSavePNG("build/full-dev/orientation-window.png",window.iWidth,window.iHeight,screen,window.iWidth*4));
    OK(xgeRenderTargetCreate(&captured,8,8)); OK(xgeRenderTargetCaptureCurrent(&captured,0,0));
    CHECK(render(&target,&captured.tTexture,&shader,0,(xge_rect_t){0,0,8,8},0)); CHECK(same_crop(pixels,32,0,0,8,8,0));
    OK(xgeImageEncodePNGEx(8,8,pixels,32,0,&png,&png_size));
    OK(xgeTextureLoadMemoryEx(&decoded,png,(int)png_size,XGE_TEXTURE_COMPRESS_NONE));
    CHECK(render(&target,&decoded,&shader,0,(xge_rect_t){0,0,8,8},0)); CHECK(same_crop(pixels,32,0,0,8,8,0));
    failed=0;
cleanup:
    if (pass.bActive) xgePassEnd(&pass);
    if (copy) proxy.surfaceDestroy(&proxy,copy); if (surface) proxy.surfaceDestroy(&proxy,surface);
    xgeMaterialFree(&material); xgeShaderFree(&shader); xgeShaderFree(&mixed);
    xge3dMeshFree(mesh3d); xge3dSceneFree(scene); xge3dTargetFree(target3d); xge3dRendererFree(renderer);
    xgeTextureFree(&image); xgeTextureFree(&decoded);
    xgeRenderTargetFree(&source); xgeRenderTargetFree(&target); xgeRenderTargetFree(&nested); xgeRenderTargetFree(&captured);
    if (png) xrtFree(png); free(screen);
    if (begun) (void)xgeEnd(); xgeQuit(); return failed?XGE_ERROR_INVALID_STATE:XGE_OK;
}

int main(void)
{
    xge_desc_t d={0}; d.iWidth=128; d.iHeight=96; d.sTitle="XGE 3 image coordinate regression";
    if (xgeInit(&d)!=XGE_OK) return 1;
    int r=xgeRun(frame,NULL); xgeUnit();
    printf("%s texture orientation GPU: %d checks\n",r==XGE_OK&&!failed?"PASS":"FAIL",checks);
    return r==XGE_OK&&!failed?0:1;
}
