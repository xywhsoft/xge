/* CPU-only native record lifetime test. No graphics context is created. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
typedef struct Entry {const char* name;xge_host_xrt_proc address;} Entry;
static const Entry entries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;size_t first=0,last=sizeof(entries)/sizeof(*entries);
    while(first<last){size_t middle=first+(last-first)/2;int order=strcmp(name,entries[middle].name);
        if(order<0)last=middle;else if(order>0)first=middle+1;else return entries[middle].address;}
    return NULL;
}
static void textures(int yuv)
{
    xge_texture texture=xrtCalloc(1,sizeof(*texture));assert(texture);
    int result=yuv ? xgeTextureCreateYUV420P(texture,2,2) : xgeTextureCreateRGBA(texture,2,2,NULL);
    assert(result==XGE_OK && texture->iRefCount==1);
    assert(xgeTextureAddRef(texture)==2);
    assert(xgeTextureAdoptStorage(texture)==XGE_ERROR_INVALID_STATE);
    xgeTextureFree(texture);assert(texture->iRefCount==1);
    assert(xrtMemDebugFailAfter(0));assert(xgeTextureAdoptStorage(texture)==XGE_OK);assert(xrtMemDebugFailAfter(-1));
    assert(xgeTextureAdoptStorage(texture)==XGE_ERROR_INVALID_STATE);
    assert(xgeTextureAddRef(texture)==2);xgeTextureFree(texture);
    assert(texture->iRefCount==1 && texture->iWidth==2);
    xge_material_t material; xgeMaterialInit(&material);
    assert(xgeMaterialSetTextureChecked(&material,texture)==XGE_OK);
    assert(xgeMaterialSetTexture2Checked(&material,texture)==XGE_OK);
    assert(xgeMaterialSetTexture3Checked(&material,texture)==XGE_OK && texture->iRefCount==4);
    assert(xgeMaterialSetTextureChecked(&material,texture)==XGE_OK && texture->iRefCount==4);
    xge_texture_t overflow={0};overflow.iRefCount=INT_MAX;
    assert(xgeMaterialSetTextureChecked(&material,&overflow)==XGE_ERROR_INVALID_STATE && material.pTexture==texture);
    assert(xgeMaterialSetTexture2Checked(&material,&overflow)==XGE_ERROR_INVALID_STATE && material.pTexture2==texture);
    assert(xgeMaterialSetTexture3Checked(&material,&overflow)==XGE_ERROR_INVALID_STATE && material.pTexture3==texture);
    xge_sprite_batch_t batch={0};
    assert(xgeSpriteBatchInit(&batch,&overflow,1,0)==XGE_ERROR_INVALID_STATE && !batch.pVertices && !batch.pTexture);
    xgeTextureFree(texture);assert(texture->iRefCount==3 && texture->iWidth==2);
    xgeMaterialFree(&material); /* Last native owner frees payload AND record. */
    xge_texture_t automatic={0};
    assert(xgeTextureCreateRGBA(&automatic,2,2,NULL)==XGE_OK);
    xgeTextureFree(&automatic);assert(!automatic.iRefCount && !automatic.pBackend);
}
static void fonts(const char* path)
{
    xge_font_face face=NULL;assert(xgeFontFaceLoad(&face,path,NULL)==XGE_OK && face);
    xge_font font=xrtCalloc(1,sizeof(*font));assert(font);
    xge_font_instance_desc_t desc={0};desc.iSize=sizeof(desc);desc.fPixelSize=16;
    assert(xgeFontCreate(font,face,&desc)==XGE_OK);
    assert(xgeFontAddRef(font)==2 && xgeFontAdoptStorage(font)==XGE_ERROR_INVALID_STATE);
    xgeFontFree(font);assert(font->iRefCount==1);
    assert(xrtMemDebugFailAfter(0));assert(xgeFontAdoptStorage(font)==XGE_OK);assert(xrtMemDebugFailAfter(-1));
    assert(xgeFontAdoptStorage(font)==XGE_ERROR_INVALID_STATE);
    assert(xgeFontAddRef(font)==2);xgeFontFree(font);assert(font->iRefCount==1 && font->fSize==16);
    xge_font fallback=xrtCalloc(1,sizeof(*fallback));assert(fallback);
    assert(xgeFontCreate(fallback,face,&desc)==XGE_OK && xgeFontAdoptStorage(fallback)==XGE_OK);
    assert(xgeFontSetFallbackChecked(font,fallback)==XGE_OK && fallback->iRefCount==2);
    xge_font_t overflow={0};overflow.iRefCount=INT_MAX;
    assert(xgeFontSetFallbackChecked(font,&overflow)==XGE_ERROR_INVALID_STATE && font->pFallback==fallback);
    assert(xgeFontSetFallbackChecked(fallback,font)==XGE_ERROR_INVALID_ARGUMENT && !fallback->pFallback);
    xgeFontFree(fallback);assert(fallback->iRefCount==1);
    xgeFontFree(font); /* Releases the only remaining reference to fallback. */
    xge_font_t automatic={0};assert(xgeFontCreate(&automatic,face,&desc)==XGE_OK);
    xgeFontFree(&automatic);assert(!automatic.iRefCount && !automatic.pBackend);
    xgeFontFaceFree(face);
}
static void allocation_failures(void)
{
    for(int budget=0;budget<6;++budget){
        xge_texture texture=xrtCalloc(1,sizeof(*texture));assert(texture);
        assert(xrtMemDebugFailAfter(budget));int result=xgeTextureCreateRGBA(texture,2,2,NULL);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(result==XGE_OK){assert(xgeTextureAdoptStorage(texture)==XGE_OK);xgeTextureFree(texture);}
        else{xgeTextureFree(texture);xrtFree(texture);}
    }
}
int main(int argc,char** argv)
{
    assert(argc==2);const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xgeHostXrtRuntimeIdentity()==(xge_host_xrt_proc)xrtVersion);
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xgeTextureAdoptStorage(NULL)==XGE_ERROR_INVALID_STATE);
    assert(xgeFontAdoptStorage(NULL)==XGE_ERROR_INVALID_STATE);
    xge_texture_t texture={0};xge_font_t font={0};
    assert(xgeTextureAdoptStorage(&texture)==XGE_ERROR_INVALID_STATE && xgeFontAdoptStorage(&font)==XGE_ERROR_INVALID_STATE);
    assert(xgeTextureAddRef(&texture)==XGE_ERROR_INVALID_STATE && xgeFontAddRef(&font)==0);
    texture.iRefCount=font.iRefCount=INT_MAX;
    assert(xgeTextureAddRef(&texture)==XGE_ERROR_INVALID_STATE && texture.iRefCount==INT_MAX);
    assert(xgeFontAddRef(&font)==0 && font.iRefCount==INT_MAX);
    textures(0);textures(1);fonts(argv[1]);allocation_failures();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    printf("native adopted record storage: delayed last-reference free, zero-copy adoption, automatic records, OOM; public sizes font=%zu texture=%zu\n",sizeof(xge_font_t),sizeof(xge_texture_t));
    return 0;
}
