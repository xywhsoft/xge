/* Native owned allocation, not a language bridge. Admission covers creation
 * itself, including allocation/error callbacks, and transfers to the record
 * on success. Ordinary updates/draws keep their original native entry points. */
static void* __xgeOwnedGraphicsAllocate(size_t bytes,int* result)
{
    *result=__xgeGraphicsOwnerAcquire(&g_xgeGraphicsLifetime);
    if(*result!=XGE_OK)return NULL;
    void* record=xrtCalloc(1,bytes);
    if(!record){(void)__xgeGraphicsOwnerRelease(&g_xgeGraphicsLifetime);*result=XGE_ERROR_OUT_OF_MEMORY;}
    return record;
}
#define XGE_OWNED_GRAPHICS_FINISH(Output,Record,Result,Release,Mark) do { \
    if((Result)!=XGE_OK){Release(Record);xrtFree(Record); \
        (void)__xgeGraphicsOwnerRelease(&g_xgeGraphicsLifetime);return (Result);} \
    Mark;*(Output)=(Record);return XGE_OK; \
} while(0)

int xgeTextureCreateRGBAOwned(xge_texture* out,int width,int height,const void* pixels,size_t bytes)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(width<=0 || height<=0 || width>INT32_MAX/4 || height>INT32_MAX/(width*4) ||
        (pixels ? bytes<(size_t)width*(size_t)height*4:bytes!=0))return XGE_ERROR_INVALID_ARGUMENT;
    int result;xge_texture texture=__xgeOwnedGraphicsAllocate(sizeof(*texture),&result);
    if(!texture)return result;
    result=xgeTextureCreateRGBA(texture,width,height,pixels);
    XGE_OWNED_GRAPHICS_FINISH(out,texture,result,xgeTextureFree,*__xgeTextureRecordOwner(texture)=1);
}
int xgeTextureLoadMemoryOwned(xge_texture* out,const void* data,int bytes)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_texture texture=__xgeOwnedGraphicsAllocate(sizeof(*texture),&result);
    if(!texture)return result;
    result=xgeTextureLoadMemory(texture,data,bytes);
    XGE_OWNED_GRAPHICS_FINISH(out,texture,result,xgeTextureFree,*__xgeTextureRecordOwner(texture)=1);
}
int xgeTextureLoadOwned(xge_texture* out,const char* path)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_texture texture=__xgeOwnedGraphicsAllocate(sizeof(*texture),&result);
    if(!texture)return result;
    result=xgeTextureLoad(texture,path);
    XGE_OWNED_GRAPHICS_FINISH(out,texture,result,xgeTextureFree,*__xgeTextureRecordOwner(texture)=1);
}
int xgeTextureCreateYUV420POwned(xge_texture* out,int width,int height)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_texture texture=__xgeOwnedGraphicsAllocate(sizeof(*texture),&result);
    if(!texture)return result;
    result=xgeTextureCreateYUV420P(texture,width,height);
    XGE_OWNED_GRAPHICS_FINISH(out,texture,result,xgeTextureFree,*__xgeTextureRecordOwner(texture)=1);
}
int xgeBufferCreateOwned(xge_buffer* out,int kind,int usage,const void* data,int bytes)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_buffer buffer=__xgeOwnedGraphicsAllocate(sizeof(*buffer),&result);
    if(!buffer)return result;
    result=xgeBufferCreate(buffer,kind,usage,data,bytes);
    XGE_OWNED_GRAPHICS_FINISH(out,buffer,result,xgeBufferFree,buffer->_bOwnsRecord=true);
}
#if XGE_ENABLE_2D
int xgeMeshCreateOwned(xge_mesh* out,const xge_vertex_t* vertices,int vertexCount,const uint16_t* indices,int indexCount,uint32_t flags)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_mesh mesh=__xgeOwnedGraphicsAllocate(sizeof(*mesh),&result);
    if(!mesh)return result;
    result=xgeMeshCreate(mesh,vertices,vertexCount,indices,indexCount,flags);
    XGE_OWNED_GRAPHICS_FINISH(out,mesh,result,xgeMeshFree,mesh->_bOwnsRecord=true);
}
#endif
int xgeShaderCreateOwned(xge_shader* out,const char* vertex,const char* fragment)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_shader shader=__xgeOwnedGraphicsAllocate(sizeof(*shader),&result);
    if(!shader)return result;
    result=xgeShaderCreate(shader,vertex,fragment);
    XGE_OWNED_GRAPHICS_FINISH(out,shader,result,xgeShaderFree,shader->_pStorageOwner=shader);
}
int xgeRenderTargetCreateOwned(xge_render_target* out,int width,int height)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_render_target target=__xgeOwnedGraphicsAllocate(sizeof(*target),&result);
    if(!target)return result;
    result=xgeRenderTargetCreate(target,width,height);
    XGE_OWNED_GRAPHICS_FINISH(out,target,result,xgeRenderTargetFree,
        target->_iOwnerReferences=1;((xge_texture_shadow_t*)target->tTexture.pBackend)->pTargetOwner=target);
}
int xgeRenderTargetWindowOwned(xge_render_target* out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!g_xge.bInitialized)return XGE_ERROR_NOT_INITIALIZED;
    int result;xge_render_target target=__xgeOwnedGraphicsAllocate(sizeof(*target),&result);
    if(!target)return result;
    result=xgeRenderTargetWindow(target);
    XGE_OWNED_GRAPHICS_FINISH(out,target,result,xgeRenderTargetFree,target->_iOwnerReferences=1);
}
#if XGE_ENABLE_TEXT
int xgeFontLoadOwned(xge_font* out,const char* path,float size)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_font font=__xgeOwnedGraphicsAllocate(sizeof(*font),&result);
    if(!font)return result;
    result=xgeFontLoad(font,path,size);
    XGE_OWNED_GRAPHICS_FINISH(out,font,result,xgeFontFree,((xge_font_instance_backend_t*)font->pBackend)->bOwnsRecord=1);
}
int xgeFontLoadMemoryOwned(xge_font* out,const void* data,int bytes,float size)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_font font=__xgeOwnedGraphicsAllocate(sizeof(*font),&result);
    if(!font)return result;
    result=xgeFontLoadMemory(font,data,bytes,size);
    XGE_OWNED_GRAPHICS_FINISH(out,font,result,xgeFontFree,((xge_font_instance_backend_t*)font->pBackend)->bOwnsRecord=1);
}
int xgeFontCreateSizedOwned(xge_font* out,xge_font source,float size)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int result;xge_font font=__xgeOwnedGraphicsAllocate(sizeof(*font),&result);
    if(!font)return result;
    result=xgeFontCreateSized(font,source,size);
    XGE_OWNED_GRAPHICS_FINISH(out,font,result,xgeFontFree,((xge_font_instance_backend_t*)font->pBackend)->bOwnsRecord=1);
}
#endif
#undef XGE_OWNED_GRAPHICS_FINISH
