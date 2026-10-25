#include "xge3d_internal.h"
#if XGE3D_ENABLE_CACHE
typedef struct cache_texture {xge3d_texture *texture;uint64_t hash;struct cache_texture *next;} cache_texture;
typedef struct cache_entry {
    char *key;xge3d_model *model;uint64_t stamp,cpu,gpu;size_t views,active;int error,mapped;
#if XGE3D_ENABLE_ASYNC
    xge3d_request_t job;
#endif
} cache_entry;
#if XGE3D_ENABLE_ASYNC
typedef struct cache_view {cache_entry *entry;xge3d_cache_request_t handle;size_t index;int cancelled;} cache_view;
#endif
struct xge3d_cache {
    uint64_t id,owner,context,clock;xge3d_cache_desc_t desc;
    xarray entries,textures;xmap keys,texture_keys;xge3d_cache_stats_t counters;
#if XGE3D_ENABLE_ASYNC
    xge3d_loader *loader;xslotmap slots;xarray views;
#endif
};
static xatomic64 cache_ids;
static int cache_owner(xge3d_cache *c)
{if(!c||c->owner!=xrtThreadCurrentId()||(c->context&&c->context!=__xge3dContext())){return 0;}if(__xge3dContext()){c->context=__xge3dContext();}return 1;}
static xbytesview string_key(const char *s){return (xbytesview){(const unsigned char*)s,strlen(s)};}
static char *cache_key(xge3d_cache *c,const char *uri)
{
    if(!uri||!*uri||strlen(uri)>1048576){return NULL;}
    if(strstr(uri,"://")&&strncmp(uri,"file://",7)){if(!(c->desc.flags&XGE3D_CACHE_RESOURCE_PROVIDERS)){return NULL;}size_t size=strlen(uri)+1;char *s=xrtMalloc(size);if(s){memcpy(s,uri,size);}return s;}
    const char *path=!strncmp(uri,"file://",7)?uri+7:uri;char *absolute=xrtPathAbs(path);if(!absolute){return NULL;}
    char *clean=xrtPathClean((xstrview){absolute,strlen(absolute)},XPATH_NATIVE);xrtFree(absolute);return clean;
}
static void unmap_entry(xge3d_cache *c,cache_entry *e)
{if(e->mapped){xrtMapRemove(&c->keys,string_key(e->key));e->mapped=0;}}
static void drop_entry(xge3d_cache *c,cache_entry *e)
{
    unmap_entry(c,e);
#if XGE3D_ENABLE_ASYNC
    if(e->job.slot){xge3dLoaderRelease(c->loader,e->job);}
#endif
    for(size_t i=0;i<c->entries.Count;++i)if(*(cache_entry**)xrtArrayGet(&c->entries,i)==e){xrtArrayRemoveSwap(&c->entries,i);break;}
    xge3dModelFree(e->model);xrtFree(e->key);xrtFree(e);
}
static cache_entry *get_entry(xge3d_cache *c,const char *uri,int *error)
{
    if(!uri||!*uri||strlen(uri)>1048576){*error=XGE_ERROR_INVALID_ARGUMENT;return NULL;}
    if(strstr(uri,"://")&&strncmp(uri,"file://",7)&&!(c->desc.flags&XGE3D_CACHE_RESOURCE_PROVIDERS)){*error=XGE_ERROR_UNSUPPORTED;return NULL;}
    char *key=cache_key(c,uri);if(!key){*error=!uri||!*uri?XGE_ERROR_INVALID_ARGUMENT:XGE_ERROR_RESOURCE_FAILED;return NULL;}
    cache_entry *e=xrtMapGetPtr(&c->keys,string_key(key));if(e){xrtFree(key);return e;}
    e=xrtMalloc(sizeof(*e));if(!e){xrtFree(key);*error=XGE_ERROR_OUT_OF_MEMORY;return NULL;}memset(e,0,sizeof(*e));e->key=key;
    if(!xrtArrayPush(&c->entries,&e)){xrtFree(e->key);xrtFree(e);*error=XGE_ERROR_OUT_OF_MEMORY;return NULL;}
    if(!xrtMapSetPtr(&c->keys,string_key(key),e)){drop_entry(c,e);*error=XGE_ERROR_OUT_OF_MEMORY;return NULL;}
    e->mapped=1;++c->counters.misses;return e;
}
static int intern_texture(xge3d_texture *t,xge3d_texture **out,void *user)
{
    xge3d_cache *c=user;int desc[]={t->width,t->height,t->srgb,t->sampler.min_filter,t->sampler.mag_filter,t->sampler.wrap_u,t->sampler.wrap_v};
    uint64_t hash=xrtHash64Seed(t->pixels.Data,t->pixels.Count,xrtHash64(desc,sizeof(desc)));xbytesview key={(const unsigned char*)&hash,sizeof(hash)};
    cache_texture *first=xrtMapGetPtr(&c->texture_keys,key);
    for(cache_texture *p=first;p;p=p->next){xge3d_texture *q=p->texture;
        if(q->width==t->width&&q->height==t->height&&q->srgb==t->srgb&&!memcmp(&q->sampler,&t->sampler,sizeof(t->sampler))&&q->pixels.Count==t->pixels.Count&&!memcmp(q->pixels.Data,t->pixels.Data,t->pixels.Count)){
            *out=q;if(q!=t){++c->counters.shared_textures;}return XGE_OK;}}
    cache_texture *entry=xrtMalloc(sizeof(*entry));if(!entry){return XGE_ERROR_OUT_OF_MEMORY;}
    if(!d3_texture_retain(t)){xrtFree(entry);return XGE_ERROR_INVALID_STATE;}*entry=(cache_texture){t,hash,first};
    if(!xrtArrayPush(&c->textures,&entry)){xge3dTextureFree(t);xrtFree(entry);return XGE_ERROR_OUT_OF_MEMORY;}
    if(!xrtMapSetPtr(&c->texture_keys,key,entry)){xrtArrayRemoveSwap(&c->textures,c->textures.Count-1);xge3dTextureFree(t);xrtFree(entry);return XGE_ERROR_OUT_OF_MEMORY;}
    *out=t;return XGE_OK;
}
static void trim_textures(xge3d_cache *c)
{
    for(size_t i=0;i<c->textures.Count;){cache_texture *e=*(cache_texture**)xrtArrayGet(&c->textures,i);if(e->texture->refs!=1){++i;continue;}
        xbytesview key={(const unsigned char*)&e->hash,sizeof(e->hash)};cache_texture **link=xrtMapGet(&c->texture_keys,key);
        while(*link!=e){link=&(*link)->next;}*link=e->next;if(!xrtMapGetPtr(&c->texture_keys,key)){xrtMapRemove(&c->texture_keys,key);}
        xrtArrayRemoveSwap(&c->textures,i);xge3dTextureFree(e->texture);xrtFree(e);
    }
}
xge3d_cache_desc_t xge3dCacheDefault(void)
{return (xge3d_cache_desc_t){.size=sizeof(xge3d_cache_desc_t),.max_models=64,.max_requests=256};}
int xge3dCacheCreate(const xge3d_cache_desc_t *desc,xge3d_cache **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;xge3d_cache_desc_t d=desc?*desc:xge3dCacheDefault();
    if(d.size!=sizeof(d)||(d.flags&~XGE3D_CACHE_RESOURCE_PROVIDERS)||d.max_requests>65536){return XGE_ERROR_INVALID_ARGUMENT;}
    if(!d.max_models){d.max_models=64;}if(!d.max_requests){d.max_requests=256;}
    xge3d_cache *c=xrtMalloc(sizeof(*c));if(!c){return XGE_ERROR_OUT_OF_MEMORY;}memset(c,0,sizeof(*c));c->desc=d;c->owner=xrtThreadCurrentId();c->context=__xge3dContext();c->id=xrtAtomic64FetchAdd(&cache_ids,1,XMEMORY_RELAXED)+1;
    xrtArrayInit(&c->entries,sizeof(cache_entry*));xrtArrayInit(&c->textures,sizeof(cache_texture*));xrtMapInit(&c->keys,sizeof(void*));xrtMapInit(&c->texture_keys,sizeof(void*));
    if(d.decode_budget&&!xrtAllocationBudgetAddRef(d.decode_budget)){xrtFree(c);return XGE_ERROR_INVALID_STATE;}
#if XGE3D_ENABLE_ASYNC
    xrtSlotMapInit(&c->slots);xrtArrayInit(&c->views,sizeof(cache_view*));
#endif
    if(!c->id){xge3dCacheFree(c);return XGE_ERROR_INVALID_STATE;}*out=c;return XGE_OK;
}
int xge3dCacheFree(xge3d_cache *c)
{
    if(!c){return XGE_OK;}if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}
#if XGE3D_ENABLE_ASYNC
    int result=xge3dLoaderFree(c->loader);if(result){return result;}c->loader=NULL;
    for(size_t i=0;i<c->views.Count;++i){xrtFree(*(cache_view**)xrtArrayGet(&c->views,i));}xrtArrayUnit(&c->views);xrtSlotMapUnit(&c->slots);
    for(size_t i=0;i<c->entries.Count;++i){(*(cache_entry**)xrtArrayGet(&c->entries,i))->job=(xge3d_request_t){0};}
#endif
    while(c->entries.Count){drop_entry(c,*(cache_entry**)xrtArrayGet(&c->entries,c->entries.Count-1));}
    for(size_t i=0;i<c->textures.Count;++i){cache_texture *e=*(cache_texture**)xrtArrayGet(&c->textures,i);xge3dTextureFree(e->texture);xrtFree(e);}
    xrtArrayUnit(&c->entries);xrtArrayUnit(&c->textures);xrtMapUnit(&c->keys);xrtMapUnit(&c->texture_keys);xrtAllocationBudgetFree(c->desc.decode_budget);xrtFree(c);return XGE_OK;
}
int xge3dCacheStats(xge3d_cache *c,xge3d_cache_stats_t *out)
{
    if(!out||out->size!=sizeof(*out)){return XGE_ERROR_INVALID_ARGUMENT;}if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}
    xge3d_cache_stats_t info=c->counters;info.size=sizeof(info);info.textures=c->textures.Count;
#if XGE3D_ENABLE_ASYNC
    info.requests=c->views.Count;
#endif
    for(size_t i=0;i<c->entries.Count;++i){cache_entry *e=*(cache_entry**)xrtArrayGet(&c->entries,i);
#if XGE3D_ENABLE_ASYNC
        info.pending+=e->job.slot!=0;
#endif
        if(!e->model){continue;}++info.models;info.cpu_storage_bytes+=e->cpu;info.gpu_required_bytes+=e->gpu;}
    for(size_t i=0;i<c->textures.Count;++i){xge3d_texture *t=(*(cache_texture**)xrtArrayGet(&c->textures,i))->texture;
        info.cpu_storage_bytes+=sizeof(*t)+(uint64_t)t->pixels.Capacity*t->pixels.ItemSize;
        int w=t->width,h=t->height;info.gpu_required_bytes+=(uint64_t)w*h*4;if(t->sampler.min_filter>=0x2700)while(w>1||h>1){w=w>1?w/2:1;h=h>1?h/2:1;info.gpu_required_bytes+=(uint64_t)w*h*4;}}
    info.over_budget=info.models>c->desc.max_models||(c->desc.cpu_bytes&&info.cpu_storage_bytes>c->desc.cpu_bytes)||(c->desc.gpu_bytes&&info.gpu_required_bytes>c->desc.gpu_bytes);*out=info;return XGE_OK;
}
static int trim_cache(xge3d_cache *c,int clear)
{
    if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}
    for(;;){trim_textures(c);xge3d_cache_stats_t info={.size=sizeof(info)};int result=xge3dCacheStats(c,&info);if(result){return result;}if(!clear&&!info.over_budget){return XGE_OK;}
        cache_entry *oldest=NULL;for(size_t i=0;i<c->entries.Count;++i){cache_entry *e=*(cache_entry**)xrtArrayGet(&c->entries,i);if(!e->model||e->views){continue;}
            xge3d_resource_info_t resource={.size=sizeof(resource)};xge3dModelResourceInfo(e->model,&resource);if(resource.references!=1){continue;}if(!oldest||e->stamp<oldest->stamp){oldest=e;}}
        if(!oldest){return XGE_OK;}drop_entry(c,oldest);++c->counters.evictions;
    }
}
int xge3dCacheTrim(xge3d_cache *c){return trim_cache(c,0);}
int xge3dCacheClear(xge3d_cache *c){return trim_cache(c,1);}
static int adopt_model(xge3d_cache *c,cache_entry *e,xge3d_model *m)
{
    int result=d3_model_share_textures(m,intern_texture,c);if(!result)result=d3_model_cache_bytes(m,&e->cpu,&e->gpu);
    if(result){xge3dModelFree(m);e->error=result;unmap_entry(c,e);trim_textures(c);return result;}
    e->model=m;e->stamp=++c->clock;return XGE_OK;
}
int xge3dCacheLoad(xge3d_cache *c,const char *uri,xge3d_model **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}
    int result=0;cache_entry *e=get_entry(c,uri,&result);if(!e){return result;}
    if(e->model){++c->counters.hits;}
#if XGE3D_ENABLE_ASYNC
    else if(e->job.slot){++c->counters.coalesced;
        while(e->job.slot){xge3d_upload_budget_t budget={UINT64_MAX,0,UINT32_MAX};result=xge3dCachePump(c,&budget,NULL);if(result){return result;}if(e->job.slot){xrtSleep(1);}}
        if(e->error){return e->error;}}
#endif
    else{xge3d_model *m=NULL;result=c->desc.decode_budget?xge3dModelLoadBudget(e->key,c->desc.decode_budget,&m):xge3dModelLoad(e->key,&m);
        if(!result){result=adopt_model(c,e,m);}else{e->error=result;unmap_entry(c,e);}if(result){if(!e->views){drop_entry(c,e);}return result;}}
    result=xge3dModelAddRef(e->model);if(result){return result;}e->stamp=++c->clock;*out=e->model;result=xge3dCacheTrim(c);
    if(result){xge3dModelFree(*out);*out=NULL;}return result;
}
#if XGE3D_ENABLE_ASYNC
static cache_view *get_view(xge3d_cache *c,xge3d_cache_request_t h)
{return h.cache==c->id?xrtSlotMapGet(&c->slots,h.slot):NULL;}
static void cancel_view(xge3d_cache *c,cache_view *view)
{
    if(view->cancelled){return;}view->cancelled=1;cache_entry *e=view->entry;--e->active;
    if(!e->active&&e->job.slot){xge3dLoaderCancel(c->loader,e->job);unmap_entry(c,e);e->error=XGE_ERROR;}
}
static void detach_view(xge3d_cache *c,cache_view *view)
{
    cache_entry *e=view->entry;cancel_view(c,view);view->entry=NULL;--e->views;
    if(!e->views&&!e->model&&!e->job.slot){drop_entry(c,e);}
}
int xge3dCacheRequest(xge3d_cache *c,const char *uri,xge3d_cache_request_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_cache_request_t){0};if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}if(c->views.Count>=c->desc.max_requests){return XGE_ERROR_OUT_OF_MEMORY;}
    int result=0;cache_entry *e=get_entry(c,uri,&result);if(!e){return result;}
    cache_view *view=xrtMalloc(sizeof(*view));if(!view){if(!e->views&&!e->model){drop_entry(c,e);}return XGE_ERROR_OUT_OF_MEMORY;}memset(view,0,sizeof(*view));view->entry=e;view->index=c->views.Count;
    view->handle=(xge3d_cache_request_t){xrtSlotMapInsert(&c->slots,view),c->id};
    if(!view->handle.slot||!xrtArrayPush(&c->views,&view)){if(view->handle.slot){xrtSlotMapRemove(&c->slots,view->handle.slot,NULL);}xrtFree(view);if(!e->views&&!e->model){drop_entry(c,e);}return XGE_ERROR_OUT_OF_MEMORY;}++e->views;++e->active;
    if(e->model){++c->counters.hits;e->stamp=++c->clock;}
    else if(e->job.slot){++c->counters.coalesced;}
    else{
        if(!c->loader){xge3d_loader_desc_t ld={c->desc.max_requests>1024?1024:c->desc.max_requests,c->desc.flags};result=xge3dLoaderCreate(&ld,&c->loader);
            if(!result&&c->desc.decode_budget)result=xge3dLoaderSetDecodeBudget(c->loader,c->desc.decode_budget);
            if(result){xge3dLoaderFree(c->loader);c->loader=NULL;}}
        if(!result)result=xge3dLoaderRequest(c->loader,e->key,&e->job);
    }
    if(result){xge3dCacheRelease(c,view->handle);return result;}*out=view->handle;return XGE_OK;
}
int xge3dCacheStatus(xge3d_cache *c,xge3d_cache_request_t h,xge3d_request_info_t *out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}memset(out,0,sizeof(*out));if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}cache_view *v=get_view(c,h);if(!v){return XGE_ERROR_INVALID_ARGUMENT;}
    cache_entry *e=v->entry;if(v->cancelled){if(e->job.slot&&!e->active){xge3d_request_info_t info;int r=xge3dLoaderStatus(c->loader,e->job,&info);if(r){return r;}out->pending_cleanup=info.pending_cleanup;}
        out->state=XGE3D_REQUEST_CANCELLED;out->result=XGE_ERROR;return XGE_OK;}
    if(e->job.slot){int r=xge3dLoaderStatus(c->loader,e->job,out);if(!r&&e->error){out->state=XGE3D_REQUEST_FAILED;out->result=e->error;}return r;}
    out->state=e->model?XGE3D_REQUEST_READY:XGE3D_REQUEST_FAILED;out->result=e->error;return XGE_OK;
}
int xge3dCacheCancel(xge3d_cache *c,xge3d_cache_request_t h)
{if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}cache_view *v=get_view(c,h);if(!v){return XGE_ERROR_INVALID_ARGUMENT;}cancel_view(c,v);return XGE_OK;}
int xge3dCacheRelease(xge3d_cache *c,xge3d_cache_request_t h)
{
    if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}cache_view *v=get_view(c,h);if(!v){return XGE_ERROR_INVALID_ARGUMENT;}detach_view(c,v);
    xrtSlotMapRemove(&c->slots,h.slot,NULL);cache_view *last=*(cache_view**)xrtArrayGet(&c->views,c->views.Count-1);last->index=v->index;xrtArrayRemoveSwap(&c->views,v->index);xrtFree(v);return XGE_OK;
}
int xge3dCacheTake(xge3d_cache *c,xge3d_cache_request_t h,xge3d_model **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}cache_view *v=get_view(c,h);
    if(!v||v->cancelled||!v->entry->model){return XGE_ERROR_INVALID_STATE;}int result=xge3dModelAddRef(v->entry->model);if(result){return result;}
    *out=v->entry->model;v->entry->stamp=++c->clock;return xge3dCacheRelease(c,h);
}
int xge3dCachePump(xge3d_cache *c,const xge3d_upload_budget_t *budget,xge3d_upload_stats_t *out)
{
    if(out){memset(out,0,sizeof(*out));}if(!cache_owner(c)){return XGE_ERROR_INVALID_STATE;}
    if(!budget)return XGE_ERROR_INVALID_ARGUMENT;
    if(!c->loader)return xge3dCacheTrim(c);
    int result=xge3dLoaderPump(c->loader,budget,out);if(result){return result;}
    for(size_t i=0;i<c->entries.Count;++i){cache_entry *e=*(cache_entry**)xrtArrayGet(&c->entries,i);if(!e->job.slot){continue;}
        xge3d_request_info_t info;result=xge3dLoaderStatus(c->loader,e->job,&info);if(result){return result;}
        if(info.state==XGE3D_REQUEST_READY){xge3d_model *m=NULL;result=xge3dLoaderTake(c->loader,e->job,&m);if(result){return result;}e->job=(xge3d_request_t){0};
            if(adopt_model(c,e,m)&&out){if(out->completed)--out->completed;++out->failed;}}
        else if(info.state==XGE3D_REQUEST_FAILED){e->error=info.result;xge3dLoaderCancel(c->loader,e->job);unmap_entry(c,e);}
        else if(info.state==XGE3D_REQUEST_CANCELLED&&!info.pending_cleanup){xge3dLoaderRelease(c->loader,e->job);e->job=(xge3d_request_t){0};}
    }
    for(size_t i=0;i<c->entries.Count;){cache_entry *e=*(cache_entry**)xrtArrayGet(&c->entries,i);if(!e->job.slot&&!e->model&&!e->views){drop_entry(c,e);}else ++i;}
    return xge3dCacheTrim(c);
}
#endif
#endif
