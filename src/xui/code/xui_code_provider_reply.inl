/* Reply strings are stored in native blocks owned by the requesting scope.
 * A response is built on its callback's stack and committed only after the
 * request and the caller's borrowed-slot receipt have both succeeded. */
typedef struct xui_code_provider_reply_block_t {
    struct xui_code_provider_reply_block_t* next;
    xui_code_provider_scope_t* scope;
    int role;
} xui_code_provider_reply_block_t;

struct xui_code_completion_reply_t {
    xui_code_provider_reply_block_t* blocks;
    xui_code_completion_item_t* items;
    const char* prefix;
    int capacity;
    int count;
};
struct xui_code_hover_reply_t {
    xui_code_provider_reply_block_t* block;
    xui_code_hover_t value;
};
struct xui_code_signature_reply_t {
    xui_code_provider_reply_block_t* block;
    xui_code_signature_help_t value;
};

static bool __xuiCodeReplySizeAdd(size_t* bytes, const char* text)
{
    if (!text) return true;
    size_t length = strlen(text);
    if (length >= SIZE_MAX - *bytes) return false;
    *bytes += length + 1;
    return true;
}

static const char* __xuiCodeReplyTextCopy(char** target, const char* text)
{
    if (!text) return NULL;
    char* start = *target;
    while ((*(*target)++ = *text++) != '\0') {}
    return start;
}

static void __xuiCodeReplyFree(xui_code_provider_reply_block_t* block)
{
    while (block) {
        xui_code_provider_reply_block_t* next = block->next;
        xrtFree(block);
        block = next;
    }
}

static void __xuiCodeProviderExpireReplies(xui_code_provider_set providers,
    const xui_code_provider_scope_t* scope, int role)
{
    xui_code_provider_reply_block_t** link = &providers->replies;
    while (*link) {
        xui_code_provider_reply_block_t* block = *link;
        if (block->scope == scope && (role < 0 || block->role == role)) {
            *link = block->next;
            xrtFree(block);
        } else {
            link = &block->next;
        }
    }
}

static void __xuiCodeProviderSaveReply(xui_code_provider_set providers,
    xui_code_provider_scope_t* scope, int role, xui_code_provider_reply_block_t* blocks)
{
    __xuiCodeProviderExpireReplies(providers, scope, role);
    while (blocks) {
        xui_code_provider_reply_block_t* next = blocks->next;
        blocks->scope = scope;
        blocks->role = role;
        blocks->next = providers->replies;
        providers->replies = blocks;
        blocks = next;
    }
}

XUI_API const char* xuiCodeCompletionReplyGetPrefix(const xui_code_completion_reply_t* reply)
{
    return reply ? reply->prefix : NULL;
}

XUI_API int xuiCodeCompletionReplyGetRemaining(const xui_code_completion_reply_t* reply)
{
    return reply ? reply->capacity - reply->count : 0;
}

#define XUI_CODE_COMPLETION_TEXT_FIELDS(visit) \
    visit(sLabel) visit(sInsertText) visit(sDetail) visit(sDocumentation) \
    visit(sFilterText) visit(sSortText) visit(sCommitCharacters)
XUI_API int xuiCodeCompletionReplyAppendItems(xui_code_completion_reply_t* reply,
    const xui_code_completion_item_t* items, int count)
{
    if (!reply || count < 0 || (!items && count)) return XUI_ERROR_INVALID_ARGUMENT;
    if (count > reply->capacity - reply->count) return XUI_ERROR_LIMIT_EXCEEDED;
    if (count == 0) return XUI_OK;
    size_t bytes = sizeof(xui_code_provider_reply_block_t);
    for (int i = 0; i < count; ++i) {
        const xui_code_completion_item_t* item = &items[i];
        if (item->iSize < sizeof(*item) || !item->sLabel || !item->sLabel[0]) return XUI_ERROR_INVALID_ARGUMENT;
#define XUI_MEASURE_TEXT(field) if (!__xuiCodeReplySizeAdd(&bytes, item->field)) return XUI_ERROR_LIMIT_EXCEEDED;
        XUI_CODE_COMPLETION_TEXT_FIELDS(XUI_MEASURE_TEXT)
#undef XUI_MEASURE_TEXT
    }
    xui_code_provider_reply_block_t* block = xrtMalloc(bytes);
    if (!block) return XUI_ERROR_OUT_OF_MEMORY;
    char* write = (char*)(block + 1);
    for (int i = 0; i < count; ++i) {
        const xui_code_completion_item_t* item = &items[i];
        xui_code_completion_item_t* result = &reply->items[reply->count + i];
        *result = *item;
        result->iSize = sizeof(*result);
#define XUI_COPY_TEXT(field) result->field = __xuiCodeReplyTextCopy(&write, item->field);
        XUI_CODE_COMPLETION_TEXT_FIELDS(XUI_COPY_TEXT)
#undef XUI_COPY_TEXT
    }
    reply->count += count;
    block->next = reply->blocks;
    reply->blocks = block;
    return XUI_OK;
}
#undef XUI_CODE_COMPLETION_TEXT_FIELDS

XUI_API int xuiCodeCompletionReplyAppendItem(xui_code_completion_reply_t* reply,
    const xui_code_completion_item_t* item)
{
    return xuiCodeCompletionReplyAppendItems(reply, item, 1);
}

XUI_API int xuiCodeHoverReplySet(xui_code_hover_reply_t* reply, const xui_code_hover_t* hover)
{
    if (!reply || !xuiInternalCodeProviderHoverValid(hover)) return XUI_ERROR_INVALID_ARGUMENT;
    size_t bytes = sizeof(xui_code_provider_reply_block_t);
    if (!__xuiCodeReplySizeAdd(&bytes, hover->sText) || !__xuiCodeReplySizeAdd(&bytes, hover->sContentType))
        return XUI_ERROR_LIMIT_EXCEEDED;
    xui_code_provider_reply_block_t* block = xrtMalloc(bytes);
    if (!block) return XUI_ERROR_OUT_OF_MEMORY;
    block->next = NULL;
    char* write = (char*)(block + 1);
    xui_code_hover_t value = *hover;
    value.iSize = sizeof(value);
    value.sText = __xuiCodeReplyTextCopy(&write, hover->sText);
    value.sContentType = __xuiCodeReplyTextCopy(&write, hover->sContentType);
    __xuiCodeReplyFree(reply->block);
    reply->block = block;
    reply->value = value;
    return XUI_OK;
}

XUI_API int xuiCodeSignatureReplySet(xui_code_signature_reply_t* reply, const xui_code_signature_help_t* help)
{
    if (!reply || !xuiInternalCodeProviderSignatureValid(help)) return XUI_ERROR_INVALID_ARGUMENT;
    size_t bytes = sizeof(xui_code_provider_reply_block_t);
    if ((size_t)help->iParameterCount > (SIZE_MAX - bytes) / sizeof(*help->pParameters))
        return XUI_ERROR_LIMIT_EXCEEDED;
    bytes += sizeof(*help->pParameters) * (size_t)help->iParameterCount;
    if (!__xuiCodeReplySizeAdd(&bytes, help->sLabel) || !__xuiCodeReplySizeAdd(&bytes, help->sDocumentation))
        return XUI_ERROR_LIMIT_EXCEEDED;
    for (int i = 0; i < help->iParameterCount; ++i) {
        const xui_code_signature_parameter_t* parameter = &help->pParameters[i];
        if (!__xuiCodeReplySizeAdd(&bytes, parameter->sLabel) || !__xuiCodeReplySizeAdd(&bytes, parameter->sDocumentation))
            return XUI_ERROR_LIMIT_EXCEEDED;
    }
    xui_code_provider_reply_block_t* block = xrtMalloc(bytes);
    if (!block) return XUI_ERROR_OUT_OF_MEMORY;
    block->next = NULL;
    _Static_assert(_Alignof(xui_code_provider_reply_block_t) >= _Alignof(xui_code_signature_parameter_t),
        "reply block header must align signature parameters");
    xui_code_signature_parameter_t* parameters = (xui_code_signature_parameter_t*)(block + 1);
    char* write = (char*)(parameters + help->iParameterCount);
    xui_code_signature_help_t value = *help;
    value.iSize = sizeof(value);
    value.pParameters = help->iParameterCount ? parameters : NULL;
    value.sLabel = __xuiCodeReplyTextCopy(&write, help->sLabel);
    value.sDocumentation = __xuiCodeReplyTextCopy(&write, help->sDocumentation);
    for (int i = 0; i < value.iParameterCount; ++i) {
        parameters[i] = help->pParameters[i];
        parameters[i].iSize = sizeof(parameters[i]);
        parameters[i].sLabel = __xuiCodeReplyTextCopy(&write, parameters[i].sLabel);
        parameters[i].sDocumentation = __xuiCodeReplyTextCopy(&write, parameters[i].sDocumentation);
    }
    __xuiCodeReplyFree(reply->block);
    reply->block = block;
    reply->value = value;
    return XUI_OK;
}
