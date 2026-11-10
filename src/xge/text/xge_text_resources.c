/* Native cold ownership for immutable shaping/raster outputs. Public draw,
 * hit-test, measure and outline operations still use their original symbols. */
struct xge_glyph_bitmap_resource_t {
    xge_glyph_bitmap_t bitmap;
    int references;
};
static int __xgeTextResourceStatus(int status)
{
    if(xrtErrorKind(xrtGetError())==XERR_MEMORY)return XGE_ERROR_OUT_OF_MEMORY;
    return status==XGE_OK && xrtGetError() ? XGE_ERROR_CALLBACK_FAILED:status;
}
static int __xgeTextResourceResult(int status,xerror* ambient)
{
    xerror* error=xrtTakeError();
    if(error)xrtErrorFree(ambient);else error=ambient;
    xrtSetErrorTake(error);return status;
}
int xgeTextShapeOwned(xge_glyph_run_t** out,const xge_text_shape_desc_t* desc)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!desc || desc->iSize<sizeof(*desc) || !desc->pFont || desc->pFont->iRefCount<=0)return XGE_ERROR_INVALID_ARGUMENT;
    xerror* ambient=xrtTakeError();xge_glyph_run_t* run=xrtCalloc(1,sizeof(*run));
    int status=run ? xgeTextShape(desc,run):XGE_ERROR_OUT_OF_MEMORY;
    if(status==XGE_OK && !run->pBackend){
        run->pBackend=xrtCalloc(1,sizeof(xge_glyph_run_backend_t));
        if(!run->pBackend)status=XGE_ERROR_OUT_OF_MEMORY;
    }
    status=__xgeTextResourceStatus(status);
    if(status==XGE_OK){
        xge_glyph_run_backend_t* backend=run->pBackend;
        /* A native reference cannot keep caller-owned stack storage alive. */
        for(int i=0;i<backend->iFontCount;++i)
            if(!xgeFontIsOwned(backend->ppFonts[i])){status=XGE_ERROR_INVALID_STATE;break;}
        if(status==XGE_OK){backend->iOwnedReferences=1;*out=run;}
    }
    if(status!=XGE_OK){
        xerror* error=xrtTakeError();xgeGlyphRunFree(run);xrtFree(run);
        if(error){xrtClearError();xrtSetErrorTake(error);}
    }
    return __xgeTextResourceResult(status,ambient);
}
int xgeGlyphRunAddRef(const xge_glyph_run_t* run)
{
    xge_glyph_run_backend_t* backend=run ? run->pBackend:NULL;
    if(!backend || backend->iOwnedReferences<=0 || backend->iOwnedReferences==INT_MAX)return 0;
    return ++backend->iOwnedReferences;
}
int xgeGlyphRunGetGlyphs(const xge_glyph_run_t* run,const xge_glyph_position_t** data,size_t* count)
{
    if(data)*data=NULL;
    if(count)*count=0;
    if(!data || !count || !run || !run->pBackend ||
       ((xge_glyph_run_backend_t*)run->pBackend)->iOwnedReferences<=0 || run->iGlyphCount<0)return XGE_ERROR_INVALID_ARGUMENT;
    *data=run->pGlyphs;*count=(size_t)run->iGlyphCount;return XGE_OK;
}
int xgeGlyphRunGetCarets(const xge_glyph_run_t* run,const xge_glyph_caret_t** data,size_t* count)
{
    if(data)*data=NULL;
    if(count)*count=0;
    if(!data || !count || !run || !run->pBackend ||
       ((xge_glyph_run_backend_t*)run->pBackend)->iOwnedReferences<=0 || run->iCaretCount<0)return XGE_ERROR_INVALID_ARGUMENT;
    *data=run->pCarets;*count=(size_t)run->iCaretCount;return XGE_OK;
}
int xgeGlyphRunGetFont(const xge_glyph_run_t* run,size_t index,xge_font* out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;const xge_glyph_position_t* data=NULL;size_t count=0;
    int status=xgeGlyphRunGetGlyphs(run,&data,&count);
    if(status!=XGE_OK)return status;
    if(index>=count)return XGE_ERROR_INVALID_ARGUMENT;
    xge_font font=data[index].pFont;
    if(font && (!xgeFontIsOwned(font) || xgeFontAddRef(font)<=0))return XGE_ERROR_INVALID_STATE;
    *out=font;return XGE_OK;
}
static int __xgeGlyphBitmapResourceCreate(xge_glyph_bitmap_resource* out,xge_font font,uint32_t codepoint,int index,bool byIndex)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!font || font->iRefCount<=0)return XGE_ERROR_INVALID_ARGUMENT;
    xerror* ambient=xrtTakeError();xge_glyph_bitmap_resource bitmap=xrtCalloc(1,sizeof(*bitmap));
    int status=bitmap ? (byIndex ? xgeFontGlyphRasterizeByIndex(font,index,&bitmap->bitmap):
        xgeFontGlyphRasterize(font,codepoint,&bitmap->bitmap)):XGE_ERROR_OUT_OF_MEMORY;
    status=__xgeTextResourceStatus(status);
    if(status==XGE_OK){bitmap->references=1;*out=bitmap;}
    else{
        xerror* error=xrtTakeError();
        if(bitmap){xgeGlyphBitmapFree(&bitmap->bitmap);xrtFree(bitmap);}
        if(error){xrtClearError();xrtSetErrorTake(error);}
    }
    return __xgeTextResourceResult(status,ambient);
}
int xgeFontGlyphRasterizeOwned(xge_glyph_bitmap_resource* out,xge_font font,uint32_t codepoint)
{return __xgeGlyphBitmapResourceCreate(out,font,codepoint,0,false);}
int xgeFontGlyphRasterizeByIndexOwned(xge_glyph_bitmap_resource* out,xge_font font,int index)
{return __xgeGlyphBitmapResourceCreate(out,font,0,index,true);}
int xgeGlyphBitmapResourceAddRef(xge_glyph_bitmap_resource bitmap)
{
    if(!bitmap || bitmap->references<=0 || bitmap->references==INT_MAX)return 0;
    return ++bitmap->references;
}
void xgeGlyphBitmapResourceFree(xge_glyph_bitmap_resource bitmap)
{
    if(!bitmap || bitmap->references<=0)return;
    if(--bitmap->references)return;
    xgeGlyphBitmapFree(&bitmap->bitmap);xrtFree(bitmap);
}
int xgeGlyphBitmapResourceGetPixels(xge_glyph_bitmap_resource bitmap,const unsigned char** data,size_t* count)
{
    if(data)*data=NULL;
    if(count)*count=0;
    if(!bitmap || bitmap->references<=0 || !data || !count)return XGE_ERROR_INVALID_ARGUMENT;
    const xge_glyph_bitmap_t* raw=&bitmap->bitmap;
    if(raw->iHeight<0 || raw->iStride<0 || (raw->iHeight && (size_t)raw->iStride>SIZE_MAX/(size_t)raw->iHeight))return XGE_ERROR_INVALID_STATE;
    size_t bytes=(size_t)raw->iStride*(size_t)raw->iHeight;
    if(bytes && !raw->pPixels)return XGE_ERROR_INVALID_STATE;
    *data=raw->pPixels;*count=bytes;return XGE_OK;
}
int xgeGlyphBitmapResourceGetInfo(xge_glyph_bitmap_resource bitmap,xge_glyph_bitmap_info_t* info)
{
    if(!info || info->iSize!=sizeof(*info))return XGE_ERROR_INVALID_ARGUMENT;
    const unsigned char* data=NULL;size_t count=0;
    int status=xgeGlyphBitmapResourceGetPixels(bitmap,&data,&count);if(status!=XGE_OK)return status;
    const xge_glyph_bitmap_t* raw=&bitmap->bitmap;
    *info=(xge_glyph_bitmap_info_t){sizeof(*info),raw->iCodepoint,raw->iWidth,raw->iHeight,raw->iStride,raw->iOffsetX,raw->iOffsetY,raw->iFormat,count};return XGE_OK;
}
