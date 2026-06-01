# CheckCard Widget

`CheckCard` is a selectable card container. It behaves like a checkbox in standalone mode and like a radio option when bound to a `RadioGroup`.

The card can contain normal child widgets. The card itself renders the selection chrome: background, border, focus outline, and the checked corner mark.

## Creation

```c
xui_check_card_desc_t desc;
xui_widget card;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.bChecked = 1;
desc.fMinWidth = 320.0f;
desc.fMinHeight = 96.0f;

xuiCheckCardCreate(context, &card, &desc);
```

Add content with normal widget APIs:

```c
xuiWidgetAddChild(card, icon);
xuiWidgetAddChild(card, title);
xuiWidgetAddChild(card, description);
```

## Selection

Standalone cards toggle independently:

```c
xuiCheckCardSetChecked(card, 1);
```

Cards inside a `RadioGroup` are mutually exclusive:

```c
xuiRadioGroupCreate(context, &group, &groupDesc);
xuiRadioGroupAddCheckCard(group, card);
xuiRadioGroupSetSelectedIndex(group, 0);
```

`xuiRadioGroupGetSelectedIndex` and `xuiRadioGroupGetSelectedWidget` work for CheckCard options.

## Styling

Style properties:

- `checkcard.background.color`
- `checkcard.background.hover_color`
- `checkcard.background.active_color`
- `checkcard.background.checked_color`
- `checkcard.border.color`
- `checkcard.border.hover_color`
- `checkcard.border.checked_color`
- `checkcard.border.disabled_color`
- `checkcard.corner.color`
- `checkcard.check.color`
- `checkcard.focus.color`
- `checkcard.radius`
- `checkcard.border.width`
- `checkcard.border.checked_width`
- `checkcard.corner.size`
- `checkcard.focus.width`
