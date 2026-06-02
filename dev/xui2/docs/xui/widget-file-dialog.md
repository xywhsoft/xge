# XUI File Dialog

File Dialog is a Window-backed utility object for choosing files and folders. It provides Open File, Save File, and Select Folder entry points while keeping the implementation inside XUI2 instead of depending on a native platform dialog.

## Goals

- expose one shared dialog object for open, save, and folder selection workflows
- use `xrtDirScan` and `xrtDirScanEx` for cross-platform directory enumeration
- treat `""` as the virtual root directory; `NULL` is invalid for directory scanning APIs
- keep directory, entry name, and full path available without forcing callers to split paths
- expose child widgets and entries for tests and advanced integrations

## Structure

`xuiFileDialogCreate` returns an opaque `xui_file_dialog` handle. Internally it owns:

```text
Window root widget
  client widget
    path label
    path input
    up and refresh buttons
    root list
    file list
    file/folder name input
    OK and Cancel buttons
```

The Window owns title, close, activation, movement, and overlay placement. The client widget uses manual layout because the dialog controls its own browser geometry.

## Modes

```c
XUI_FILE_DIALOG_MODE_OPEN_FILE
XUI_FILE_DIALOG_MODE_SAVE_FILE
XUI_FILE_DIALOG_MODE_SELECT_FOLDER
```

Convenience functions create and open the correct mode immediately:

```c
xuiOpenFileDialog
xuiSaveFileDialog
xuiSelectFolderDialog
```

`xuiFileDialogCreate` can also be used directly with `xui_file_dialog_desc_t.iMode`.

## Root Directory

The dialog follows xrt's root scan contract:

- `sInitialDir = ""` opens the virtual root directory
- on Windows, the virtual root entries are drive roots such as `C:\`
- on other systems, the virtual root entry is `/`
- `xuiFileDialogSetDirectory(dialog, NULL)` is invalid

This keeps callbacks and entry getters simple: directory, file name, and full path are always strings.

## Filtering

`xui_file_dialog_desc_t.sFilter` accepts simple extension filters:

```c
"*.c;*.h;*.txt"
".png,.jpg"
"*"
"*.*"
```

Directories are always shown. Files are matched case-insensitively by extension. Empty filter, `"*"`, and `"*.*"` show all files.

## Results

Results are reported through `xui_file_dialog_result_proc`:

```c
typedef void (*xui_file_dialog_result_proc)(
	xui_file_dialog_t* dialog,
	int result,
	const char* path,
	void* user);
```

Result constants:

```c
XUI_FILE_DIALOG_RESULT_NONE
XUI_FILE_DIALOG_RESULT_OK
XUI_FILE_DIALOG_RESULT_CANCEL
```

`xuiFileDialogGetResultPath` returns the last selected path. In folder mode, committing without a selected entry returns the current directory.

## Public API

```c
xuiFileDialogCreate
xuiFileDialogDestroy
xuiOpenFileDialog
xuiSaveFileDialog
xuiSelectFolderDialog
xuiFileDialogSetOpen
xuiFileDialogIsOpen
xuiFileDialogSetResult
xuiFileDialogSetDirectory
xuiFileDialogGetDirectory
xuiFileDialogRefresh
xuiFileDialogSelectIndex
xuiFileDialogCommit
xuiFileDialogCancel
xuiFileDialogGetResult
xuiFileDialogGetResultPath
xuiFileDialogGetWindowWidget
xuiFileDialogGetRootListWidget
xuiFileDialogGetFileListWidget
xuiFileDialogGetPathInputWidget
xuiFileDialogGetNameInputWidget
xuiFileDialogGetEntryCount
xuiFileDialogGetEntryName
xuiFileDialogGetEntryPath
xuiFileDialogEntryIsDir
```

## Example

```c
xui_file_dialog_desc_t desc;
xui_file_dialog dialog;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.sInitialDir = "";
desc.sFilter = "*.c;*.h";
desc.pFont = font;
desc.onResult = onFileDialogResult;
desc.pResultUser = user;

xuiOpenFileDialog(context, &dialog, &desc);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_file_dialog_test.bat
examples\xui_file_dialog\build.bat
build\xui_file_dialog.exe --frames 3
```

The test covers open-file selection, save path construction, folder selection, filtering, and virtual root scanning.
