/* Included by xge_miniprogram.c for the single-threaded wasm32 build. */
enum { XGE_MINI_JS_RESOURCE = 1, XGE_MINI_JS_AUDIO = 2 };

extern int xgeMiniProgramJsOnFrame(uint32_t id);
extern int xgeMiniProgramJsOnResource(uint32_t id, const char* uri, size_t size,
	xge_resource_output_t* output);
extern int xgeMiniProgramJsOnAudio(uint32_t id, int command, int handle,
	const void* data, size_t size);
extern void xgeMiniProgramJsOnRetired(uint32_t id);

typedef struct xge_mini_js_owner_t {
	uint32_t id;
	size_t refs;
} xge_mini_js_owner_t;

static bool __xgeMiniJsCount(const void* raw, size_t* count)
{
	*count = ((const xge_mini_js_owner_t*)raw)->refs;
	return true;
}

static bool __xgeMiniJsTrace(const void* raw, xrtownershipvisitor visit, void* user)
{
	(void)raw;
	(void)visit;
	(void)user;
	return true; /* The JS registry owns the closure on the JS heap. */
}

static const xrtownershipops g_xgeMiniJsOps = {__xgeMiniJsCount, __xgeMiniJsTrace};

static bool __xgeMiniJsHold(const void* raw)
{
	xge_mini_js_owner_t* owner = (xge_mini_js_owner_t*)raw;
	if ( owner->refs == SIZE_MAX ) return false;
	++owner->refs;
	return true;
}

static bool __xgeMiniJsDrop(const void* raw)
{
	xge_mini_js_owner_t* owner = (xge_mini_js_owner_t*)raw;
	if ( --owner->refs == 0 ) {
		uint32_t id = owner->id;
		xrtFree(owner);
		xgeMiniProgramJsOnRetired(id);
	}
	return true;
}

static int __xgeMiniJsFrame(void* user)
{
	return xgeMiniProgramJsOnFrame(((const xge_mini_js_owner_t*)user)->id);
}

static int __xgeMiniJsResource(const xstrview* uri, xge_resource_output_t* output, void* user)
{
	return xgeMiniProgramJsOnResource(((const xge_mini_js_owner_t*)user)->id,
		uri->Data, uri->Size, output);
}

static int __xgeMiniJsAudio(int command, int handle, const xbytesview* data, void* user)
{
	return xgeMiniProgramJsOnAudio(((const xge_mini_js_owner_t*)user)->id,
		command, handle, data->Data, data->Size);
}

/* JS gets a writable range in the real reply. The existing resource callback
 * transaction frees it if JS fails before returning success. */
__attribute__((export_name("xgeMiniProgramJsResourceReserve")))
void* xgeMiniProgramJsResourceReserve(xge_resource_output_t* output, int size)
{
	if ( !output || output->_iStatus != XGE_OK ) return NULL;
	if ( output->pData || size <= 0 ) {
		output->_iStatus = XGE_ERROR_INVALID_ARGUMENT;
		return NULL;
	}
	output->pData = xrtMalloc((size_t)size);
	if ( !output->pData ) {
		output->_iStatus = XGE_ERROR_OUT_OF_MEMORY;
		return NULL;
	}
	output->iSize = size;
	return output->pData;
}

__attribute__((export_name("xgeMiniProgramJsInstall")))
int xgeMiniProgramJsInstall(uint32_t id, uint32_t flags, int width, int height, float dpr)
{
	if ( !id ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( flags & ~(uint32_t)(XGE_MINI_JS_RESOURCE | XGE_MINI_JS_AUDIO) ||
		!__xgeMiniProgramDimensionsValid(width, height, dpr) ) {
		xgeMiniProgramJsOnRetired(id);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xge_mini_js_owner_t* memory = xrtMalloc(sizeof(*memory));
	if ( !memory ) {
		xgeMiniProgramJsOnRetired(id);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*memory = (xge_mini_js_owner_t){id, 1};
	xge_callback_owner_t owner = {{memory, &g_xgeMiniJsOps}, __xgeMiniJsHold, __xgeMiniJsDrop};
	xge_mini_bridge_state_t next = {0};
	next.owned.request_frame = __xgeMiniJsFrame;
	if ( flags & XGE_MINI_JS_RESOURCE ) next.owned.load_resource = __xgeMiniJsResource;
	if ( flags & XGE_MINI_JS_AUDIO ) next.owned.audio_command = __xgeMiniJsAudio;
	next.owned.pUser = memory;
	next.creatorThread = xrtThreadCurrentId();
	xge_miniprogram_desc_t desc = {.iWidth=width,.iHeight=height,.fDevicePixelRatio=dpr};
	int status = __xgeMiniSetBridge(&next, &owner, true, &desc);
	(void)__xgeMiniJsDrop(memory); /* Retires the JS entry if publication failed. */
	return status;
}

__attribute__((export_name("xgeMiniProgramJsUnit")))
int xgeMiniProgramJsUnit(uint32_t id)
{
	int status = __xgeMiniConfigBegin(false);
	if ( status != XGE_OK ) return status;
	xerror* ambient = xrtTakeError();
	const xge_callback_node_t* owner = g_xgeMiniBridge.owner;
	if ( owner && owner->tOwner.tReference.Ops == &g_xgeMiniJsOps &&
		((const xge_mini_js_owner_t*)owner->tOwner.tReference.Data)->id == id )
		return __xgeMiniUnitReserved(ambient);
	return __xgeMiniConfigFinish(NULL, NULL, XGE_OK, ambient);
}
