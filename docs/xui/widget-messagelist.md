# MessageList widget

`messagelist` is a cache-first conversation history widget for QQ/WeChat-style chat windows.

## Node model

The widget owns an ordered list of `xui_message_node_t` records.

- `XUI_MESSAGE_NODE_SELF`: local-user message, right aligned with a blue bubble.
- `XUI_MESSAGE_NODE_OTHER`: peer message, left aligned with a white bubble.
- `XUI_MESSAGE_NODE_SYSTEM`: centered system marker, for timestamps such as `昨天 10:56`.

Each node carries `sId`, `sSender`, `sTime`, `sText`, `iFlags`, and `pUser`. The widget deep-copies strings so callers can keep their own storage model.

## Events

Use `xuiMessageListSetEvent` to receive:

- `XUI_MESSAGE_EVENT_HOVER`
- `XUI_MESSAGE_EVENT_SELECT`
- `XUI_MESSAGE_EVENT_CLICK`
- `XUI_MESSAGE_EVENT_DOUBLE_CLICK`
- `XUI_MESSAGE_EVENT_CONTEXT_MENU`
- `XUI_MESSAGE_EVENT_SCROLL`

The event payload includes the node index, node type, pointer position, button/modifier state, and a read-only public node view.

## Data APIs

Core history APIs:

- `xuiMessageListSetNodes`
- `xuiMessageListAddNode`
- `xuiMessageListClear`
- `xuiMessageListGetNodeCount`
- `xuiMessageListGetNode`

Persistence APIs:

- `xuiMessageListExportText`
- `xuiMessageListImportText`
- `xuiMessageListSaveFile`
- `xuiMessageListLoadFile`

The text format starts with `MESSAGELIST1` and stores tab-separated escaped node lines. It is intended as a stable XUI-side interchange format; applications can still map it to JSON, databases, or network records externally.

## Rendering and customization

Default rendering provides avatar circles, sender/time meta space, chat bubbles, hover and selected overlays. Use `xuiMessageListSetNodeRenderer` to override individual node rendering while keeping layout, hit testing, scrolling, and persistence.

Metrics and colors are configurable with `xuiMessageListSetMetrics` and `xuiMessageListSetColors`.
