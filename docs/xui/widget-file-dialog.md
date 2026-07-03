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
    path row: path label, path bar, up button, refresh button
      path bar: breadcrumb and path input overlay
    list row: root list and file list
    field row: file/folder name input and filter ComboBox
    button row: OK and Cancel buttons
```

The Window owns title, close, activation, movement, resizing, and overlay placement. The client widget uses XUI layout rows so the browser area and bottom controls resize with the window.

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

`xui_file_dialog_desc_t.sFilter` accepts Windows-style filter pairs:

```c
"Text Files (*.txt)|*.txt|All Files (*.*)|*.*"
"Images (*.png;*.jpg)|*.png;*.jpg|All Files (*.*)|*.*"
```

The dialog shows a file type ComboBox when file filtering is available. Selecting a filter refreshes the file list immediately.

Directories are always shown. Files are matched case-insensitively with `xrtStrLike`; patterns inside one filter can be separated with `;` or `,`. Empty filter specs default to `All Files (*.*)|*.*`.

## Interaction

- selecting a root entry navigates to that root
- the path bar shows a breadcrumb by default; clicking a breadcrumb segment navigates to that directory
- clicking the empty path bar area switches to path edit mode
- in path edit mode, pressing Enter or losing focus applies the typed directory and returns to breadcrumb mode
- an invalid typed directory keeps the dialog open, marks the path input as error, and shows a MsgTip
- double-selecting a directory in the file list enters that directory
- `Up` navigates to the parent directory; from a platform root it returns to the virtual root
- `Refresh` rescans the current directory
- when the path input is actively being edited, `Up` and `Refresh` first apply a valid edited path
- editing the file/folder field and pressing Enter behaves like the OK button
- in Open File mode, entering an existing directory navigates into it; entering a missing file keeps the dialog open and marks the field as error
- in Save File mode, entering an existing directory navigates into it
- in Save File mode, a file name without an extension appends the first concrete extension from the active filter, such as `.txt` from `*.txt`
- in Save File mode, committing an existing file opens an overwrite confirmation MsgBox; only `Yes` completes the save result
- double-selecting a file in Save File mode fills the file name field but does not immediately commit
- in Select Folder mode, the main list only shows directories
- in Select Folder mode, the bottom `Folder` field is readonly and mirrors the selected directory name
- in Select Folder mode, double-selecting a directory enters it instead of completing the dialog
- in Select Folder mode, pressing `Select` with no selected entry returns the current directory
- in Select Folder mode, the virtual root `""` is only a navigation container; pressing `Select` at the virtual root requires choosing a concrete root or child directory first
- Escape runs cancel and reports `XUI_FILE_DIALOG_RESULT_CANCEL`

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
xuiFileDialogSetFilter
xuiFileDialogGetFilter
xuiFileDialogSetDirectory
xuiFileDialogGetDirectory
xuiFileDialogGoUp
xuiFileDialogRefresh
xuiFileDialogSelectIndex
xuiFileDialogCommit
xuiFileDialogCancel
xuiFileDialogGetResult
xuiFileDialogGetResultPath
xuiFileDialogGetWindowWidget
xuiFileDialogGetRootListWidget
xuiFileDialogGetFileListWidget
xuiFileDialogGetPathBreadcrumbWidget
xuiFileDialogGetPathInputWidget
xuiFileDialogGetNameInputWidget
xuiFileDialogGetFilterComboWidget
xuiFileDialogGetUpButtonWidget
xuiFileDialogGetRefreshButtonWidget
xuiFileDialogGetOkButtonWidget
xuiFileDialogGetCancelButtonWidget
xuiFileDialogGetOverwriteMsgBox
xuiFileDialogGetFilterCount
xuiFileDialogGetFilterName
xuiFileDialogGetFilterPattern
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
desc.sFilter = "Source Files (*.c;*.h)|*.c;*.h|All Files (*.*)|*.*";
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

The test covers open-file selection, save path construction, folder selection, filtering, virtual root scanning, editable path navigation, typed directory navigation, missing-file validation, Up, and Escape cancel.
