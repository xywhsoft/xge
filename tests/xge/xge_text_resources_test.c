/* Generated valid font, bounded resource/OOM/lifetime checks; no network or
 * malformed-font fuzzing. Optional software EGL proves real queued drawing. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
int main(int argc,char** argv)
{
 assert(argc==3);int mode=atoi(argv[2]);assert(mode>=0 && mode<=6);
#if !defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 assert(xrtMemDebugEnable(true));assert(xgeInit(NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 xge_desc_t engine={0};engine.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&engine)==XGE_OK);
 xge_font font=NULL;assert(xgeFontLoadOwned(&font,argv[1],20)==XGE_OK);
 xge_text_shape_desc_t desc={0};desc.iSize=sizeof(desc);desc.pFont=font;desc.sText="ffi";desc.iTextSize=3;desc.iFlags=XGE_TEXT_SHAPE_KERNING;
 if(mode==0){
  desc.sText="\n";desc.iTextSize=1;xge_glyph_run_t run={0};assert(xgeTextShape(&desc,&run)==XGE_OK);
  assert(font->iRefCount==2 && run.iGlyphCount==1 && run.pGlyphs[0].pFont==font);
  xgeFontFree(font);font=NULL;xge_font_metrics_t metrics={0};assert(xgeFontGetMetrics(run.pGlyphs[0].pFont,&metrics)==XGE_OK);
  xgeGlyphRunFree(&run);assert(!xgeGraphicsOwnedResourceCount());
 }
#if defined(XGE_TEXT_TEST_OWNED)
 else if(mode==1 || mode==2){
  if(mode==2){desc.sText="A\0V";desc.iTextSize=3;}
  xge_glyph_run_t* run=NULL;assert(xgeTextShapeOwned(&run,&desc)==XGE_OK && run);
  assert(run->iTextSize==3 && run->iGlyphCount>0 && run->iGlyphCount<=3 && xgeGlyphRunRetainedBytes(run)>0);
  if(mode==2)assert(run->iGlyphCount==3);
  const xge_glyph_position_t* data=NULL;const xge_glyph_caret_t* carets=NULL;size_t count=0,caretCount=0;
  assert(xgeGlyphRunGetGlyphs(run,&data,&count)==XGE_OK && data==run->pGlyphs && count==(size_t)run->iGlyphCount);
  assert(xgeGlyphRunGetCarets(run,&carets,&caretCount)==XGE_OK && carets==run->pCarets && caretCount==(size_t)run->iCaretCount);
  xge_font retained=NULL;assert(xgeGlyphRunGetFont(run,0,&retained)==XGE_OK && retained && xgeFontIsOwned(retained));
  xge_font invalid=(void*)(uintptr_t)1;assert(xgeGlyphRunGetFont(run,SIZE_MAX,&invalid)==XGE_ERROR_INVALID_ARGUMENT && !invalid);
  xgeFontFree(retained);
  assert(xgeGlyphRunAddRef(run)==2);xgeGlyphRunFree(run);xgeFontFree(font);font=NULL;
  assert(xgeGraphicsOwnedResourceCount()==1 && xgeGlyphRunMeasure(run).fX>0);
  uint32_t cluster=0;int trailing=0;assert(xgeGlyphRunHitTest(run,0,0,&cluster,&trailing)==XGE_OK && cluster<=3);
  assert(xgeGlyphRunHitTest(run,NAN,0,&cluster,&trailing)==XGE_ERROR_INVALID_ARGUMENT);
#if XGE_ENABLE_SHAPE_EX
  xge_shape_ex shape=NULL;assert(xgeShapeExCreate(&shape)==XGE_OK && shape && xgeGlyphRunAppendShapeEx(run,shape,0,0)==XGE_OK);
  xge_rect_t bounds={0};assert(xgeShapeExGetBounds(shape,0.25f,&bounds)==XGE_OK && bounds.fW>0);xgeShapeExDestroy(shape);
#endif
  xgeGlyphRunFree(run);assert(!xgeGraphicsOwnedResourceCount());
 }else if(mode==3){
  const char context[]="x AV y";desc.sText=context+2;desc.iTextSize=2;desc.sContext=context;desc.iContextSize=6;desc.iContextOffset=2;desc.sLanguage="en";
  xge_glyph_run_t* run=NULL;assert(xgeTextShapeOwned(&run,&desc)==XGE_OK && run->iTextSize==2);
  xgeGlyphRunFree(run);desc.sText="";desc.iTextSize=0;desc.sContext=NULL;desc.iContextSize=0;desc.iContextOffset=0;desc.sLanguage=NULL;
  assert(xgeTextShapeOwned(&run,&desc)==XGE_OK && !run->iGlyphCount && xgeGlyphRunAddRef(run)==2);
  const xge_glyph_position_t* data=(void*)(uintptr_t)1;size_t count=99;assert(xgeGlyphRunGetGlyphs(run,&data,&count)==XGE_OK && !count && !data);
  xgeGlyphRunFree(run);xgeGlyphRunFree(run);
  desc.sText="A";desc.iTextSize=-2;run=(void*)(uintptr_t)1;assert(xgeTextShapeOwned(&run,&desc)==XGE_ERROR_INVALID_ARGUMENT && !run);
  xge_glyph_run_t raw={0};data=(void*)(uintptr_t)1;count=99;
  assert(!xgeGlyphRunAddRef(&raw) && xgeGlyphRunGetGlyphs(&raw,&data,&count)==XGE_ERROR_INVALID_ARGUMENT && !data && !count);
 }else if(mode==4){
  xge_glyph_bitmap_resource bitmap=NULL;assert(xgeFontGlyphRasterizeOwned(&bitmap,font,'A')==XGE_OK && bitmap);
  xge_glyph_bitmap_info_t info={0};info.iSize=sizeof(info);assert(xgeGlyphBitmapResourceGetInfo(bitmap,&info)==XGE_OK && info.iCodepoint=='A' && info.iWidth>0 && info.iBytes>0);
  const unsigned char* data=NULL;size_t count=0;assert(xgeGlyphBitmapResourceGetPixels(bitmap,&data,&count)==XGE_OK && data && count==info.iBytes);
  size_t visible=0;for(size_t i=0;i<count;++i)if(data[i])++visible;assert(visible);
  assert(xgeGlyphBitmapResourceAddRef(bitmap)==2);xgeGlyphBitmapResourceFree(bitmap);xgeFontFree(font);font=NULL;
  assert(!xgeGraphicsOwnedResourceCount() && xgeGlyphBitmapResourceGetInfo(bitmap,&info)==XGE_OK);xgeGlyphBitmapResourceFree(bitmap);
 }else if(mode==5){
  unsigned failed=0,succeeded=0;
  for(int64_t budget=0;budget<96;++budget){
   xge_glyph_run_t* run=(void*)(uintptr_t)1;assert(xrtMemDebugFailAfter(budget));int status=xgeTextShapeOwned(&run,&desc);
   bool triggered=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
   if(status==XGE_OK){assert(!triggered && run);++succeeded;xgeGlyphRunFree(run);}
   else{assert(status==XGE_ERROR_OUT_OF_MEMORY && triggered && !run);++failed;}
   xrtClearError();
  }
  for(int64_t budget=0;budget<32;++budget){
   xge_glyph_bitmap_resource bitmap=(void*)(uintptr_t)1;assert(xrtMemDebugFailAfter(budget));int status=xgeFontGlyphRasterizeOwned(&bitmap,font,'A');
   bool triggered=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
   if(status==XGE_OK){assert(!triggered && bitmap);++succeeded;xgeGlyphBitmapResourceFree(bitmap);}
   else{assert(status==XGE_ERROR_OUT_OF_MEMORY && triggered && !bitmap);++failed;}
   xrtClearError();
  }
  assert(failed && succeeded);printf("Owned text allocation budgets: %u failed, %u succeeded\n",failed,succeeded);
 }else if(mode==6){
  xge_graphics_context context=NULL;int status=xgeGraphicsContextCreateEglOwned(&context,64,64,true,NULL);
#if XGE_ENABLE_EGL
  assert(status==XGE_OK && context);xge_glyph_run_t* run=NULL;assert(xgeTextShapeOwned(&run,&desc)==XGE_OK);
  xgeFontFree(font);font=NULL;
  xge_render_target target=NULL;xge_pass pass=NULL;assert(xgeRenderTargetCreateOwned(&target,64,64)==XGE_OK);
  assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(0,0,0,255))==XGE_OK && xgePassBegin(pass)==XGE_OK);
  xge_text_paint_span_t span={sizeof(span),0,1,XGE_COLOR_RGBA(255,0,0,255)};
  xge_text_decoration_t decoration={0};decoration.iSize=sizeof(decoration);decoration.iColor=XGE_COLOR_RGBA(0,255,0,255);decoration.iType=XGE_TEXT_DECORATION_UNDERLINE;
  xgeGlyphRunDraw(run,4,22,XGE_COLOR_RGBA(255,255,255,255),0);
  xgeGlyphRunDrawSpans(run,4,44,XGE_COLOR_RGBA(255,255,255,255),0,&span,1);
  xgeGlyphRunDrawDecorated(run,32,44,XGE_COLOR_RGBA(255,255,255,255),0,&decoration,1);
  xgeGlyphRunFree(run);assert(xgeGraphicsOwnedResourceCount()==2 && xgePassEnd(pass)==XGE_OK);
  unsigned char pixels[64*64*4];assert(xgeRenderTargetReadPixelsSized(target,pixels,sizeof(pixels),64*4)==XGE_OK);
  unsigned painted=0;for(unsigned i=0;i<64*64;++i)if(pixels[i*4] || pixels[i*4+1] || pixels[i*4+2])++painted;
  assert(painted>20 && painted<64*64);xgePassFree(pass);xgeRenderTargetFree(target);
  bool consumed=false;assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed);
  printf("Actual queued GlyphRun drawing: %u painted pixels after Run/Font free\n",painted);
#else
  assert(status==XGE_ERROR_UNSUPPORTED && !context);
#endif
 }
#endif
 xgeFontFree(font);assert(!xgeGraphicsOwnedResourceCount());assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
 xrtMemDebugSnapshot(&after);fprintf(stderr,"Heap before=%zu/%zu after=%zu/%zu invalid=%llu/%llu double=%llu/%llu\n",before.LiveCount,before.LiveBytes,after.LiveCount,after.LiveBytes,(unsigned long long)before.InvalidFreeCount,(unsigned long long)after.InvalidFreeCount,(unsigned long long)before.DoubleFreeCount,(unsigned long long)after.DoubleFreeCount);
 assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes && after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
 printf("Text resource mode %d passed\n",mode);return 0;
}
