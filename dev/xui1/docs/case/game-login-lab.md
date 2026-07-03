# Game Login Lab

## Build

Windows:

```bat
examples\game_login_lab\build.bat
```

Linux/macOS:

```sh
./examples/game_login_lab/build.sh
```

## Run

Windows:

```bat
build\xge_game_login_lab.exe --frames 180
```

Linux/macOS:

```sh
./build/xge_game_login_lab --frames 180
```

## Coverage

- `xgeSceneSet`
- `xgeSceneReplace`
- `xgeSceneCurrent`
- `xgeXuiPanelInit/SetTitle/SetBackground/SetClip`
- `xgeXuiImageInit/SetMode`
- `xgeXuiInputInit/SetPlaceholder/SetPassword/SetText/GetText`
- `xgeXuiCheckBoxInit/SetText/SetChange/GetChecked`
- `xgeXuiButtonInit/SetText/SetClick`
- `xgeXuiMenuInit/SetItems/SetSelect/Open/IsOpen`
- `xgeXuiMsgBoxInit/SetText/SetType/SetButtons/SetResult/SetOpen/IsOpen/SetModal`
- `xgeTextureCreateRGBA`
- `xgeTextureFree`

## Expected

- Login scene shows a centered login panel with banner, user/password input, remember checkbox, preset menu owner, and login/close buttons.
- Preset menu can fill `admin/admin` or `guest/guest`.
- Wrong password opens a modal MsgBox; `ESC` closes it.
- Successful login switches to a different scene with a green dashboard-style background.
- Program exits automatically and prints a final summary.

## Final Summary

```text
game-login-lab final-summary frames=... layout=... input=... controls=... feedback=... scene=...
```

Focus on:

- `layout/input/controls/feedback/scene`
- `attempts/success`
- `remember/menu/msgbox`
- `enter/leave`
- `msg=user=...`
