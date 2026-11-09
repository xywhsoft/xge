/* Registry entries are immutable real owners. A load pins its entry under the
 * publisher lock, then invokes arbitrary provider/owner code after unlocking.
 * Keeping this registry outside g_xge preserves pre-init registrations. */
typedef struct xge_resource_registration_t {
    xatomic64 refs;
    xge_resource_provider_t raw;
    xge_xpack_provider_t pack;
    xge_resource_read_owned_proc read;
    xge_callback_node_t* owner;
    void (*releaseUser)(void*);
    bool packed;
    char* scheme;
    char* root;
} xge_resource_registration_t;
static xspinlock g_xgeResourceSpin=XRT_SPIN_INIT;
static xge_resource_registration_t* g_xgeResourceProviders[XGE_RESOURCE_PROVIDER_MAX];
static int g_xgeResourceProviderCount;
static int g_xgeResourcePackCount;

static bool __xgeResourceRegistrationPin(xge_resource_registration_t* entry)
{
    uint64 refs=xrtAtomic64Load(&entry->refs,XMEMORY_RELAXED);
    do {if(!refs || refs==UINT64_MAX)return false;}
    while(!xrtAtomic64CompareExchange(&entry->refs,&refs,refs+1,XMEMORY_RELAXED,XMEMORY_RELAXED));
    return true;
}
static int __xgeResourceRegistrationRelease(xge_resource_registration_t* entry)
{
    if(!entry || xrtAtomic64FetchSub(&entry->refs,1,XMEMORY_ACQ_REL)!=1)return XGE_OK;
    xge_callback_node_t* owner=entry->owner;
    void (*releaseUser)(void*)=entry->releaseUser;void* user=entry->raw.pUser;
    xrtFree(entry->scheme);xrtFree(entry->root);xrtFree(entry);
    xerror* ambient=xrtTakeError();int status=XGE_OK;xerror* first=NULL;
    __xgeCallbackOutcome(__xgeCallbackNodeRelease(owner),&status,&first);
    if(releaseUser)releaseUser(user);
    __xgeCallbackOutcome(XGE_OK,&status,&first);
    if(status==XGE_OK)xrtSetErrorTake(ambient);else{xrtErrorFree(ambient);xrtSetErrorTake(first);}return status;
}
static int __xgeResourceRegistryPublish(xge_resource_registration_t* entry)
{
    (void)xrtSpinLock(&g_xgeResourceSpin);
    int status=xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE) ? XGE_ERROR_INVALID_STATE:
        g_xgeResourceProviderCount==XGE_RESOURCE_PROVIDER_MAX ||
        (entry->packed && g_xgeResourcePackCount==XGE_XPACK_PROVIDER_MAX) ? XGE_ERROR_RESOURCE_FAILED:XGE_OK;
    if(status==XGE_OK){g_xgeResourceProviders[g_xgeResourceProviderCount++]=entry;if(entry->packed)++g_xgeResourcePackCount;}
    (void)xrtSpinUnlock(&g_xgeResourceSpin);return status;
}
/* Caller owns user until successful publication. For native bridge providers
 * this hook is the real registration reference, distinct from payload refs. */
static int __xgeResourceProviderStore(const xge_resource_provider_t* provider,void (*releaseUser)(void*),bool replace,bool* published)
{
    if(published)*published=false;
    if(!provider || !provider->sScheme || !provider->load)return XGE_ERROR_INVALID_ARGUMENT;
    xge_resource_registration_t* entry=xrtCalloc(1,sizeof(*entry));
    if(!entry)return XGE_ERROR_OUT_OF_MEMORY;
    xrtAtomic64Init(&entry->refs,1);entry->raw=*provider;
    entry->scheme=__xgeStrDup(provider->sScheme);
    if(!entry->scheme){xrtFree(entry);return XGE_ERROR_OUT_OF_MEMORY;}
    entry->releaseUser=releaseUser;
    xge_resource_registration_t* old=NULL;
    (void)xrtSpinLock(&g_xgeResourceSpin);int slot=-1;
    if(replace)for(int i=0;i<g_xgeResourceProviderCount;++i){
        xge_resource_registration_t* candidate=g_xgeResourceProviders[i];
        if(!candidate->read && candidate->raw.load==provider->load && candidate->raw.free==provider->free){slot=i;break;}
    }
    int status=xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE) ? XGE_ERROR_INVALID_STATE:
        slot<0 && g_xgeResourceProviderCount==XGE_RESOURCE_PROVIDER_MAX ? XGE_ERROR_RESOURCE_FAILED:XGE_OK;
    if(status==XGE_OK){if(slot<0)g_xgeResourceProviders[g_xgeResourceProviderCount++]=entry;
        else {old=g_xgeResourceProviders[slot];g_xgeResourceProviders[slot]=entry;}}
    (void)xrtSpinUnlock(&g_xgeResourceSpin);
    if(status!=XGE_OK){entry->releaseUser=NULL;(void)__xgeResourceRegistrationRelease(entry);return status;}
    if(published)*published=true;
    return __xgeResourceRegistrationRelease(old);
}
static int __xgeResourceProviderReplaceWithRelease(const xge_resource_provider_t* provider,void (*releaseUser)(void*),bool* published)
{return __xgeResourceProviderStore(provider,releaseUser,true,published);}
int xgeResourceProviderAdd(const xge_resource_provider_t* provider)
{return __xgeResourceProviderStore(provider,NULL,false,NULL);}

static int __xgeResourceRegistryRemove(int (*load)(const char*,void**,int*,void*),void (*freeData)(void*,void*))
{
    xge_resource_registration_t* removed[XGE_RESOURCE_PROVIDER_MAX];int count=0,write=0;
    (void)xrtSpinLock(&g_xgeResourceSpin);
    for(int i=0;i<g_xgeResourceProviderCount;++i){
        xge_resource_registration_t* entry=g_xgeResourceProviders[i];
        if(!entry->read && entry->raw.load==load && entry->raw.free==freeData){removed[count++]=entry;if(entry->packed)--g_xgeResourcePackCount;}
        else g_xgeResourceProviders[write++]=entry;
    }
    memset(g_xgeResourceProviders+write,0,(size_t)(g_xgeResourceProviderCount-write)*sizeof(*g_xgeResourceProviders));
    g_xgeResourceProviderCount=write;(void)xrtSpinUnlock(&g_xgeResourceSpin);
    xerror* ambient=xrtTakeError();int status=XGE_OK;xerror* first=NULL;
    for(int i=0;i<count;++i)__xgeCallbackOutcome(__xgeResourceRegistrationRelease(removed[i]),&status,&first);
    if(status==XGE_OK)xrtSetErrorTake(ambient);else{xrtErrorFree(ambient);xrtSetErrorTake(first);}return status;
}
int xgeResourceProviderClearChecked(void)
{
    xge_resource_registration_t* removed[XGE_RESOURCE_PROVIDER_MAX];int count;
    (void)xrtSpinLock(&g_xgeResourceSpin);count=g_xgeResourceProviderCount;
    memcpy(removed,g_xgeResourceProviders,(size_t)count*sizeof(*removed));
    memset(g_xgeResourceProviders,0,sizeof(g_xgeResourceProviders));g_xgeResourceProviderCount=g_xgeResourcePackCount=0;
    (void)xrtSpinUnlock(&g_xgeResourceSpin);
    xerror* ambient=xrtTakeError();int status=XGE_OK;xerror* first=NULL;
    for(int i=0;i<count;++i)__xgeCallbackOutcome(__xgeResourceRegistrationRelease(removed[i]),&status,&first);
    if(status==XGE_OK)xrtSetErrorTake(ambient);else{xrtErrorFree(ambient);xrtSetErrorTake(first);}return status;
}
void xgeResourceProviderClear(void)
{
    int status=xgeResourceProviderClearChecked();
    if(status!=XGE_OK && !xrtGetError())xrtSetErrorKind(XERR_STATE);
}
static xge_resource_registration_t* __xgeResourceRegistryFind(const char* uri,int schemeLength,bool pin,int* ordinal,int* failure)
{
    xge_resource_registration_t* found=NULL;
    (void)xrtSpinLock(&g_xgeResourceSpin);
    for(int i=g_xgeResourceProviderCount-1;i>=0;--i){
        xge_resource_registration_t* entry=g_xgeResourceProviders[i];
        if(__xgeSchemeEqual(uri,schemeLength,entry->scheme)){
            if(!pin || __xgeResourceRegistrationPin(entry)){found=entry;if(ordinal)*ordinal=i+1;}
            else if(failure)*failure=XGE_ERROR_OUT_OF_MEMORY;
            break;
        }
    }
    (void)xrtSpinUnlock(&g_xgeResourceSpin);return found;
}
static bool __xgeResourceHasProvider(const char* uri)
{
    int size=__xgeUriSchemeLen(uri);
    return size>0 && __xgeResourceRegistryFind(uri,size,false,NULL,NULL)!=NULL;
}
static char* __xgeResourceXPackMakePath(const char* sRoot, const char* sPath)
{
	char* sFull;
	size_t iRootLen;
	size_t iPathLen;
	size_t iNeed;
	int bNeedSlash;

	if ( sPath == NULL ) {
		return NULL;
	}
	while ( (*sPath == '/') || (*sPath == '\\') ) {
		sPath++;
	}
	if ( (sRoot == NULL) || (sRoot[0] == 0) ) {
		return __xgeStrDup(sPath);
	}
	iRootLen = strlen(sRoot);
	iPathLen = strlen(sPath);
	bNeedSlash = ((iRootLen > 0) && (sRoot[iRootLen - 1] != '/') && (sRoot[iRootLen - 1] != '\\')) ? 1 : 0;
	if ( iRootLen > (SIZE_MAX - iPathLen - 2u) ) {
		return NULL;
	}
	iNeed = iRootLen + iPathLen + (size_t)bNeedSlash + 1u;
	sFull = (char*)xrtMalloc(iNeed);
	if ( sFull == NULL ) {
		return NULL;
	}
	memcpy(sFull, sRoot, iRootLen);
	if ( bNeedSlash ) {
		sFull[iRootLen] = '/';
		iRootLen++;
	}
	memcpy(sFull + iRootLen, sPath, iPathLen + 1u);
	return sFull;
}

static int __xgeResourceXPackPathSafe(const char* sPath)
{
	const char* sSegment;
	const char* sCursor;

	if ( sPath == NULL ) return 0;
	for ( sCursor = sPath; ; ) {
		while ( *sCursor == '/' || *sCursor == '\\' ) sCursor++;
		sSegment = sCursor;
		while ( *sCursor != '\0' && *sCursor != '/' && *sCursor != '\\' ) sCursor++;
		if ( (sCursor - sSegment) == 2 && sSegment[0] == '.' && sSegment[1] == '.' ) return 0;
		if ( *sCursor == '\0' ) break;
	}
	return 1;
}


static bool __xgeResourceSchemeValid(const char* text)
{
    if(!text || !((*text>='A' && *text<='Z') || (*text>='a' && *text<='z')))return false;
    for(++text;*text;++text)if(!((*text>='A' && *text<='Z') || (*text>='a' && *text<='z') ||
        (*text>='0' && *text<='9') || *text=='+' || *text=='-' || *text=='.'))return false;
    return true;
}
static int __xgeResourceProviderOwned(const char* scheme,const char* root,bool packed,
    xge_resource_read_owned_proc read,void* user,const xge_callback_owner_t* owner)
{
    if(!read || !__xgeResourceSchemeValid(scheme))return XGE_ERROR_INVALID_ARGUMENT;
    xge_callback_node_t* node=NULL;int status=__xgeCallbackNodePrepare(true,false,owner,&node);
    if(status!=XGE_OK)return status;
    xge_resource_registration_t* entry=xrtCalloc(1,sizeof(*entry));
    if(!entry){xerror* primary=xrtTakeError();(void)__xgeCallbackNodeRelease(node);xrtClearError();xrtSetErrorTake(primary);return XGE_ERROR_OUT_OF_MEMORY;}
    xrtAtomic64Init(&entry->refs,1);entry->owner=node;entry->read=read;entry->raw.pUser=user;entry->packed=packed;
    entry->scheme=__xgeStrDup(scheme);if(root)entry->root=__xgeStrDup(root);
    if(!entry->scheme || (root && !entry->root))status=XGE_ERROR_OUT_OF_MEMORY;
    else status=__xgeResourceRegistryPublish(entry);
    if(status!=XGE_OK){xerror* first=xrtTakeError();__xgeCallbackOutcome(__xgeResourceRegistrationRelease(entry),&status,&first);xrtSetErrorTake(first);}
    return status;
}
static int __xgeResourceOperationFinish(int status,xerror* ambient)
{
    xerror* fresh=xrtTakeError();
    if(status==XGE_OK && fresh)status=XGE_ERROR_CALLBACK_FAILED;
    if(status==XGE_OK)xrtSetErrorTake(ambient);else{xrtErrorFree(ambient);xrtSetErrorTake(fresh);}
    return status;
}
int xgeResourceProviderAddOwned(const char* scheme,xge_resource_read_owned_proc read,void* user,const xge_callback_owner_t* owner)
{
    xerror* ambient=xrtTakeError();int status=__xgeResourceProviderOwned(scheme,NULL,false,read,user,owner);
    return __xgeResourceOperationFinish(status,ambient);
}
int xgeResourceXPackProviderAddOwned(const char* root,xge_resource_read_owned_proc read,void* user,const xge_callback_owner_t* owner)
{
    xerror* ambient=xrtTakeError();int status=__xgeResourceProviderOwned("res",root,true,read,user,owner);
    return __xgeResourceOperationFinish(status,ambient);
}
int xgeResourceXPackProviderAdd(const xge_xpack_provider_t* provider)
{
    if(!provider || !provider->pPack || !provider->read)return XGE_ERROR_INVALID_ARGUMENT;
    xge_resource_registration_t* entry=xrtCalloc(1,sizeof(*entry));
    if(!entry)return XGE_ERROR_OUT_OF_MEMORY;
    xrtAtomic64Init(&entry->refs,1);entry->packed=true;entry->pack=*provider;
    entry->scheme=__xgeStrDup("res");if(provider->sRoot)entry->root=__xgeStrDup(provider->sRoot);
    int status=!entry->scheme || (provider->sRoot && !entry->root) ? XGE_ERROR_OUT_OF_MEMORY:__xgeResourceRegistryPublish(entry);
    if(status!=XGE_OK)(void)__xgeResourceRegistrationRelease(entry);
    return status;
}
int xgeResourceOutputSet(xge_resource_output_t* output,const void* data,size_t bytes)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    if(output->_iStatus!=XGE_OK)return output->_iStatus;
    if(bytes>(size_t)INT32_MAX || (bytes && !data))return output->_iStatus=XGE_ERROR_INVALID_ARGUMENT;
    void* copy=bytes ? xrtMalloc(bytes):NULL;
    if(bytes && !copy)return output->_iStatus=XGE_ERROR_OUT_OF_MEMORY;
    if(bytes)memcpy(copy,data,bytes);
    xrtFree(output->pData);output->pData=copy;output->iSize=(int)bytes;return XGE_OK;
}
static void __xgeResourceRawDataFree(xge_resource_registration_t* entry,void* data)
{
    if(entry->packed){if(entry->pack.free)entry->pack.free(data,entry->pack.pUser);else xrtFree(data);}
    else if(entry->raw.free)entry->raw.free(data,entry->raw.pUser);
    else xrtFree(data);
}
static void __xgeResourceFreeRegistration(void* data,void* user)
{
    xge_resource_registration_t* entry=user;
    if(data)__xgeResourceRawDataFree(entry,data);
    int status=__xgeResourceRegistrationRelease(entry);
    if(status!=XGE_OK && !xrtGetError())xrtSetErrorKind(XERR_STATE);
}
static int __xgeResourceRegistrationLoad(xge_resource_registration_t* entry,const char* uri,xge_resource_t* output)
{
    char* packPath=NULL;
    if(entry->packed){
        int scheme=__xgeUriSchemeLen(uri);const char* path=scheme>0 ? uri+scheme+3:uri;
        if(!__xgeResourceXPackPathSafe(path))return XGE_ERROR_INVALID_ARGUMENT;
        packPath=__xgeResourceXPackMakePath(entry->root,path);
        if(!packPath)return XGE_ERROR_OUT_OF_MEMORY;
    }
    int status=XGE_OK;
    if(entry->read){
        xerror* ambient=xrtTakeError();xge_callback_snapshot_t snapshot={0};
        if(!__xgeCallbackNodePin(entry->owner))status=XGE_ERROR_INVALID_STATE;
        else status=__xgeCallbackSnapshotBegin(entry->owner,false,&snapshot);
        xge_resource_output_t value={0};
        if(status==XGE_OK){
            const char* path=packPath ? packPath:uri;xstrview text={path,strlen(path)};
            status=entry->read(&text,&value,entry->raw.pUser);
            if(status>0)status=XGE_ERROR_CALLBACK_FAILED;
            if(status==XGE_OK)status=value._iStatus;
        }
        xerror* first=status!=XGE_OK ? xrtTakeError():NULL;
        __xgeCallbackOutcome(XGE_OK,&status,&first);
        __xgeCallbackOutcome(__xgeCallbackSnapshotEnd(&snapshot),&status,&first);
        if(status==XGE_OK){output->pData=value.pData;output->iSize=value.iSize;}
        else xrtFree(value.pData);
        if(status==XGE_OK)xrtSetErrorTake(ambient);else{xrtErrorFree(ambient);xrtSetErrorTake(first);}
    }else{
        void* data=NULL;int size=0;
        if(entry->packed){
            uint64_t bytes=0;data=entry->pack.read(entry->pack.pPack,packPath,&bytes,entry->pack.pUser);
            if(!data)status=XGE_ERROR_FILE_NOT_FOUND;
            else if(bytes>(uint64_t)INT32_MAX)status=XGE_ERROR_RESOURCE_FAILED;
            else size=(int)bytes;
        }else status=entry->raw.load(uri,&data,&size,entry->raw.pUser);
        if(status==XGE_OK && (size<0 || (size && !data)))status=XGE_ERROR_RESOURCE_FAILED;
        if(status==XGE_OK){
            /* Transfer the in-flight reference into this independent payload. */
            output->pData=data;output->iSize=size;output->free=__xgeResourceFreeRegistration;output->pUser=entry;
        }else if(data){xerror* first=xrtTakeError();__xgeResourceRawDataFree(entry,data);
            __xgeCallbackOutcome(XGE_OK,&status,&first);xrtSetErrorTake(first);}
    }
    xrtFree(packPath);return status;
}
static int __xgeResourceLoad(const char* uri,xge_resource_t* output)
{
    if(!uri || !output)return XGE_ERROR_INVALID_ARGUMENT;
    memset(output,0,sizeof(*output));
    int scheme=__xgeUriSchemeLen(uri);
    if(scheme>0){
        int ordinal=0,failure=XGE_OK;
        xge_resource_registration_t* entry=__xgeResourceRegistryFind(uri,scheme,true,&ordinal,&failure);
        if(failure!=XGE_OK)return failure;
        if(entry){
            int status=__xgeResourceRegistrationLoad(entry,uri,output);
            if(status==XGE_OK)output->iProvider=ordinal;
            bool transferred=status==XGE_OK && !entry->read;
            if(!transferred){xerror* first=status!=XGE_OK ? xrtTakeError():NULL;
                __xgeCallbackOutcome(__xgeResourceRegistrationRelease(entry),&status,&first);xrtSetErrorTake(first);}
            if(status!=XGE_OK)xgeResourceFree(output);
            return status;
        }
        if(__xgeSchemeEqual(uri,scheme,"res") || __xgeSchemeEqual(uri,scheme,"file"))uri+=scheme+3;
        else return XGE_ERROR_UNSUPPORTED;
    }
    size_t bytes=0;void* data=__xgeFileGetAll(uri,&bytes);
    if(!data){xerrkind kind=xrtErrorKind(xrtGetError());return kind==XERR_MEMORY ? XGE_ERROR_OUT_OF_MEMORY:
        kind==XERR_NOT_FOUND ? XGE_ERROR_FILE_NOT_FOUND:XGE_ERROR_RESOURCE_FAILED;}
    if(bytes>(size_t)INT32_MAX){xrtFree(data);return XGE_ERROR_RESOURCE_FAILED;}
    output->pData=data;output->iSize=(int)bytes;return XGE_OK;
}
int xgeResourceLoad(const char* uri,xge_resource_t* output)
{
    if(!uri || !output)return XGE_ERROR_INVALID_ARGUMENT;
    xerror* ambient=xrtTakeError();int status=__xgeResourceLoad(uri,output);
    xerror* first=status!=XGE_OK ? xrtTakeError():NULL;
    __xgeCallbackOutcome(XGE_OK,&status,&first);
    if(status!=XGE_OK && output)xgeResourceFree(output);
    /* Cleanup may diagnose another failure; preserve the first outcome. */
    __xgeCallbackOutcome(XGE_OK,&status,&first);xrtSetErrorTake(first);
    return __xgeResourceOperationFinish(status,ambient);
}
int xgeResourceLoadMemory(const void* data,int bytes,xge_resource_t* output)
{
    if(!output || bytes<0 || (bytes && !data))return XGE_ERROR_INVALID_ARGUMENT;
    memset(output,0,sizeof(*output));xge_resource_output_t value={0};
    int status=xgeResourceOutputSet(&value,data,(size_t)bytes);
    if(status==XGE_OK){output->pData=value.pData;output->iSize=value.iSize;}
    return status;
}
void xgeResourceFree(xge_resource_t* output)
{
    if(!output)return;
    xge_resource_t value=*output;memset(output,0,sizeof(*output));
    /* Empty provider payloads can still own metadata and must release it. */
    if(value.free)value.free(value.pData,value.pUser);else xrtFree(value.pData);
}

int xgeZstdDecompress(void* pOutput, int iOutputCapacity, const void* pInput, int iInputSize, int* pOutputSize)
{
	size_t iDecoded;

	if ( (pOutput == NULL) || (iOutputCapacity <= 0) || (pInput == NULL) || (iInputSize <= 0) || (pOutputSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOutputSize = 0;
	iDecoded = ZSTD_decompress(pOutput, (size_t)iOutputCapacity, pInput, (size_t)iInputSize);
	if ( ZSTD_isError(iDecoded) || (iDecoded > (size_t)INT_MAX) ) {
		__xgeReportErrorInternal(XGE_ERROR_RESOURCE_FAILED, 1, "resource", "decompress Zstd data",
			ZSTD_isError(iDecoded) ? ZSTD_getErrorName(iDecoded) : "decoded data is too large");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	*pOutputSize = (int)iDecoded;
	return XGE_OK;
}
