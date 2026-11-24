/* The reply builders already own their strings and signature parameters.
 * Detach those blocks before ending the enclosing request scope. */
struct xui_code_completion_snapshot_t {
    xui_code_provider_reply_block_t* blocks;
    int count;
    xui_code_completion_item_t items[];
};

struct xui_code_hover_snapshot_t {
    xui_code_provider_reply_block_t* block;
    xui_code_hover_t value;
};

struct xui_code_signature_snapshot_t {
    xui_code_provider_reply_block_t* block;
    xui_code_signature_help_t value;
};

static bool __xuiCodeProviderHasTypedReply(xui_code_provider_set providers, int role)
{
    const xui_code_provider_slot_t* slot = providers->slots[role];
    return slot && slot->flavor == XUI_PROVIDER_TYPED;
}

XUI_API int xuiCodeProviderCaptureCompletion(xui_code_provider_set providers, xui_widget widget,
    int offset, const char* prefix, int capacity, xui_code_completion_snapshot* output)
{
    if (!output) return XUI_ERROR_INVALID_ARGUMENT;
    *output = NULL;
    if (offset < 0 || capacity < 0) return XUI_ERROR_INVALID_ARGUMENT;
    if ((size_t)capacity > (SIZE_MAX - sizeof(struct xui_code_completion_snapshot_t)) /
        sizeof(xui_code_completion_item_t)) return XUI_ERROR_LIMIT_EXCEEDED;

    xui_code_provider_scope_t scope = {0};
    int status = xuiCodeProviderSetCallBegin(providers, &scope);
    if (status != XUI_OK) return status;
    if (!__xuiCodeProviderHasTypedReply(providers, XUI_PROVIDER_COMPLETE))
        return __xuiCodeProviderFinish(&scope, XUI_ERROR_UNSUPPORTED);

    xui_code_completion_snapshot snapshot = xrtMalloc(sizeof(*snapshot) +
        (size_t)capacity * sizeof(snapshot->items[0]));
    if (!snapshot) return __xuiCodeProviderFinish(&scope, XUI_ERROR_OUT_OF_MEMORY);
    snapshot->blocks = NULL;
    status = xuiCodeProviderRequestCompletion(providers, widget, offset, prefix,
        snapshot->items, capacity, &snapshot->count);
    if (status == XUI_OK)
        snapshot->blocks = __xuiCodeProviderTakeReplies(providers, &scope, XUI_PROVIDER_COMPLETE);
    status = __xuiCodeProviderFinish(&scope, status);
    if (status == XUI_OK) *output = snapshot;
    else xuiCodeCompletionSnapshotDestroy(snapshot);
    return status;
}

XUI_API void xuiCodeCompletionSnapshotDestroy(xui_code_completion_snapshot snapshot)
{
    if (!snapshot) return;
    __xuiCodeReplyFree(snapshot->blocks);
    xrtFree(snapshot);
}

XUI_API int xuiCodeCompletionSnapshotGetCount(const struct xui_code_completion_snapshot_t* snapshot)
{
    return snapshot ? snapshot->count : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiCodeCompletionSnapshotGetItem(const struct xui_code_completion_snapshot_t* snapshot,
    int index, xui_code_completion_item_t* output)
{
    if (!snapshot || !output || index < 0 || index >= snapshot->count) return XUI_ERROR_INVALID_ARGUMENT;
    *output = snapshot->items[index];
    return XUI_OK;
}

XUI_API int xuiCodeProviderCaptureHover(xui_code_provider_set providers, xui_widget widget,
    int offset, xui_code_hover_snapshot* output)
{
    if (!output) return XUI_ERROR_INVALID_ARGUMENT;
    *output = NULL;
    if (offset < 0) return XUI_ERROR_INVALID_ARGUMENT;

    xui_code_provider_scope_t scope = {0};
    int status = xuiCodeProviderSetCallBegin(providers, &scope);
    if (status != XUI_OK) return status;
    if (!__xuiCodeProviderHasTypedReply(providers, XUI_PROVIDER_HOVER))
        return __xuiCodeProviderFinish(&scope, XUI_ERROR_UNSUPPORTED);

    xui_code_hover_snapshot snapshot = xrtMalloc(sizeof(*snapshot));
    if (!snapshot) return __xuiCodeProviderFinish(&scope, XUI_ERROR_OUT_OF_MEMORY);
    snapshot->block = NULL;
    status = xuiCodeProviderRequestHover(providers, widget, offset, &snapshot->value);
    if (status == XUI_OK)
        snapshot->block = __xuiCodeProviderTakeReplies(providers, &scope, XUI_PROVIDER_HOVER);
    status = __xuiCodeProviderFinish(&scope, status);
    if (status == XUI_OK) *output = snapshot;
    else xuiCodeHoverSnapshotDestroy(snapshot);
    return status;
}

XUI_API void xuiCodeHoverSnapshotDestroy(xui_code_hover_snapshot snapshot)
{
    if (!snapshot) return;
    __xuiCodeReplyFree(snapshot->block);
    xrtFree(snapshot);
}

XUI_API int xuiCodeHoverSnapshotGetInfo(const struct xui_code_hover_snapshot_t* snapshot,
    xui_code_hover_t* output)
{
    if (!snapshot || !output) return XUI_ERROR_INVALID_ARGUMENT;
    *output = snapshot->value;
    return XUI_OK;
}

XUI_API int xuiCodeProviderCaptureSignature(xui_code_provider_set providers, xui_widget widget,
    int offset, xui_code_signature_snapshot* output)
{
    if (!output) return XUI_ERROR_INVALID_ARGUMENT;
    *output = NULL;
    if (offset < 0) return XUI_ERROR_INVALID_ARGUMENT;

    xui_code_provider_scope_t scope = {0};
    int status = xuiCodeProviderSetCallBegin(providers, &scope);
    if (status != XUI_OK) return status;
    if (!__xuiCodeProviderHasTypedReply(providers, XUI_PROVIDER_SIGNATURE))
        return __xuiCodeProviderFinish(&scope, XUI_ERROR_UNSUPPORTED);

    xui_code_signature_snapshot snapshot = xrtMalloc(sizeof(*snapshot));
    if (!snapshot) return __xuiCodeProviderFinish(&scope, XUI_ERROR_OUT_OF_MEMORY);
    snapshot->block = NULL;
    status = xuiCodeProviderRequestSignature(providers, widget, offset, &snapshot->value);
    if (status == XUI_OK)
        snapshot->block = __xuiCodeProviderTakeReplies(providers, &scope, XUI_PROVIDER_SIGNATURE);
    status = __xuiCodeProviderFinish(&scope, status);
    if (status == XUI_OK) *output = snapshot;
    else xuiCodeSignatureSnapshotDestroy(snapshot);
    return status;
}

XUI_API void xuiCodeSignatureSnapshotDestroy(xui_code_signature_snapshot snapshot)
{
    if (!snapshot) return;
    __xuiCodeReplyFree(snapshot->block);
    xrtFree(snapshot);
}

XUI_API int xuiCodeSignatureSnapshotGetInfo(const struct xui_code_signature_snapshot_t* snapshot,
    xui_code_signature_help_t* output)
{
    if (!snapshot || !output) return XUI_ERROR_INVALID_ARGUMENT;
    *output = snapshot->value;
    return XUI_OK;
}

XUI_API int xuiCodeSignatureSnapshotGetParameter(const struct xui_code_signature_snapshot_t* snapshot,
    int index, xui_code_signature_parameter_t* output)
{
    if (!snapshot || !output || index < 0 || index >= snapshot->value.iParameterCount)
        return XUI_ERROR_INVALID_ARGUMENT;
    *output = snapshot->value.pParameters[index];
    return XUI_OK;
}
