#ifndef XGE_H
#define XGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "lib/xrt/xrt.h"

#define XGE_VERSION_MAJOR	2
#define XGE_VERSION_MINOR	0
#define XGE_VERSION_PATCH	0

#if !defined(XGE_DEBUG) && !defined(XGE_RELEASE)
	#if defined(NDEBUG)
		#define XGE_RELEASE	1
	#else
		#define XGE_DEBUG	1
	#endif
#endif

#if !defined(XGE_DEBUGMODE)
	#define XGE_DEBUGMODE	0
#endif

#if XGE_DEBUGMODE
	#define XGE_HAS_DEBUGMODE	1
#else
	#define XGE_HAS_DEBUGMODE	0
#endif

#if defined(_WIN32) || defined(_WIN64)
	#if defined(XGE_DLL)
		#if defined(XGE_BUILD_DLL)
			#define XGE_API __declspec(dllexport)
		#else
			#define XGE_API __declspec(dllimport)
		#endif
	#else
		#define XGE_API
	#endif
#else
	#define XGE_API
#endif

#define XGE_INIT_WINDOW		0x0000
#define XGE_INIT_FULLSCREEN	0x0001
#define XGE_INIT_NOFRAME	0x0002
#define XGE_INIT_RESIZABLE	0x0004
#define XGE_INIT_HIGHDPI	0x0008
#define XGE_INIT_VSYNC		0x0010
#define XGE_INIT_OFFSCREEN	0x0020

#define XGE_RUN_GAME_LOOP	0
#define XGE_RUN_MANUAL		1

#define XGE_PLATFORM_BACKEND_NONE	0
#define XGE_PLATFORM_BACKEND_SOKOL	1
#define XGE_PLATFORM_BACKEND_MINIPROGRAM	2
#define XGE_PLATFORM_BACKEND_EGL	3
#define XGE_PLATFORM_BACKEND_CUSTOM	100

#define XGE_GPU_BACKEND_NONE		0
#define XGE_GPU_BACKEND_OPENGL33	1
#define XGE_GPU_BACKEND_GLES30		2
#define XGE_GPU_BACKEND_WEBGL2		3
#define XGE_GPU_BACKEND_CUSTOM		100

#define XGE_UPDATE_VARIABLE	0
#define XGE_UPDATE_FIXED	1

#define XGE_LOG_TRACE	0
#define XGE_LOG_DEBUG	1
#define XGE_LOG_INFO	2
#define XGE_LOG_WARN	3
#define XGE_LOG_ERROR	4
#define XGE_LOG_FATAL	5
#define XGE_LOG_OFF	6

#define XGE_KEY_COUNT		512
#define XGE_KEY_SPACE		32
#define XGE_KEY_ESCAPE		256
#define XGE_KEY_ENTER		257
#define XGE_KEY_TAB			258
#define XGE_KEY_BACKSPACE	259
#define XGE_KEY_DELETE		261
#define XGE_KEY_RIGHT		262
#define XGE_KEY_LEFT		263
#define XGE_KEY_DOWN		264
#define XGE_KEY_UP			265
#define XGE_KEY_PAGE_UP		266
#define XGE_KEY_PAGE_DOWN	267
#define XGE_KEY_HOME		268
#define XGE_KEY_END			269
#define XGE_KEY_F10			299
#define XGE_KEY_MENU		348
#define XGE_TEXT_MAX		32
#define XGE_TOUCH_MAX		8
#define XGE_GAMEPAD_MAX		4
#define XGE_GAMEPAD_BUTTON_COUNT	32
#define XGE_GAMEPAD_AXIS_COUNT	8
#define XGE_MOUSE_LEFT		0x01
#define XGE_MOUSE_RIGHT		0x02
#define XGE_MOUSE_MIDDLE	0x04
#define XGE_DIRTY_RECT_MAX	64
#define XGE_SCENE_STACK_MAX	16

#define XGE_PIXEL_RGBA8		1
#define XGE_PIXEL_A8		2

#define XGE_IMAGE_PREMULTIPLIED		0x0001
#define XGE_IMAGE_STRAIGHT_ALPHA	0x0002

#define XGE_TEXTURE_FALLBACK		0x0001
#define XGE_TEXTURE_UPLOAD_QUEUED	0x0002
#define XGE_FONT_FALLBACK			0x0001
#define XGE_SOUND_FALLBACK			0x80000000u

#define XGE_RENDER_TARGET_WINDOW	0x0001
#define XGE_RENDER_TARGET_TEXTURE	0x0002
#define XGE_PASS_CLEAR_COLOR		0x0001

#define XGE_BUFFER_VERTEX		1
#define XGE_BUFFER_INDEX		2
#define XGE_BUFFER_UNIFORM		3

#define XGE_BUFFER_STATIC		1
#define XGE_BUFFER_DYNAMIC		2

#define XGE_FILTER_NEAREST		1
#define XGE_FILTER_LINEAR		2

#define XGE_WRAP_CLAMP			1
#define XGE_WRAP_REPEAT		2

#define XGE_BLEND_NONE			0
#define XGE_BLEND_ALPHA			1
#define XGE_BLEND_ADD			2
#define XGE_BLEND_MULTIPLY		3
#define XGE_BLEND_SCREEN		4
#define XGE_BLEND_CUSTOM		5

#define XGE_MATERIAL_DEFAULT_BLEND	-1
#define XGE_SHADER_DEFINE_MAX		8
#define XGE_SHADER_DEFINE_NAME_MAX	32

#define XGE_COORD_Y_DOWN	0
#define XGE_COORD_CENTER	1

#define XGE_CAMERA_ORTHO		0
#define XGE_CAMERA_PERSPECTIVE	1

#define XGE_DRAW_FLIP_X		0x0001
#define XGE_DRAW_FLIP_Y		0x0002
#define XGE_DRAW_SCREEN_SPACE	0x0004
#define XGE_NINE_PATCH_STRETCH	0
#define XGE_NINE_PATCH_TILE		1

#define XGE_MESH_DYNAMIC	0x0001

#define XGE_TEXT_ALIGN_LEFT		0x0000
#define XGE_TEXT_ALIGN_CENTER	0x0001
#define XGE_TEXT_ALIGN_RIGHT	0x0002
#define XGE_TEXT_ALIGN_TOP		0x0000
#define XGE_TEXT_ALIGN_MIDDLE	0x0010
#define XGE_TEXT_ALIGN_BOTTOM	0x0020
#define XGE_TEXT_CLIP			0x0100
#define XGE_TEXT_UNDERLINE		0x0200
#define XGE_TEXT_SCREEN_SPACE	0x0400

#define XGE_XRF_MAGIC			0x32465258u
#define XGE_XRF_VERSION			1
#define XGE_XRF_PAGE_A8			1
#define XGE_XRF_PAGE_RGBA8		2
#define XGE_XRF_FLAG_KERNING	0x0001

#define XGE_AUDIO_SOUND		1
#define XGE_AUDIO_MUSIC		2
#define XGE_AUDIO_STREAM	3

#define XGE_ASYNC_PENDING		0
#define XGE_ASYNC_LOADING		1
#define XGE_ASYNC_READY		2
#define XGE_ASYNC_FAILED		3
#define XGE_ASYNC_CANCELLED	4

#define XGE_ASYNC_IMAGE		1
#define XGE_ASYNC_TEXTURE		2
#define XGE_ASYNC_FONT			3
#define XGE_ASYNC_SOUND		4

#define XGE_EVENT_NONE			0
#define XGE_EVENT_KEY_DOWN		1
#define XGE_EVENT_KEY_UP		2
#define XGE_EVENT_TEXT			3
#define XGE_EVENT_MOUSE_DOWN	4
#define XGE_EVENT_MOUSE_UP		5
#define XGE_EVENT_MOUSE_MOVE	6
#define XGE_EVENT_MOUSE_WHEEL	7
#define XGE_EVENT_TOUCH_BEGIN	8
#define XGE_EVENT_TOUCH_MOVE	9
#define XGE_EVENT_TOUCH_END		10
#define XGE_EVENT_TOUCH_CANCEL	11
#define XGE_EVENT_RESIZE		12
#define XGE_EVENT_QUIT			13
#define XGE_EVENT_IME_START		14
#define XGE_EVENT_IME_UPDATE	15
#define XGE_EVENT_IME_END		16
#define XGE_EVENT_XUI_FOCUS_IN	17
#define XGE_EVENT_XUI_FOCUS_OUT	18
#define XGE_EVENT_GAMEPAD_CONNECTED	19
#define XGE_EVENT_GAMEPAD_DISCONNECTED	20
#define XGE_EVENT_XUI_POINTER_ENTER	21
#define XGE_EVENT_XUI_POINTER_LEAVE	22
#define XGE_EVENT_XUI_CAPTURE_LOST	23
#define XGE_EVENT_XUI_CONTEXT_BEGIN	24
#define XGE_EVENT_XUI_CONTEXT_UPDATE	25
#define XGE_EVENT_XUI_CONTEXT_END		26
#define XGE_EVENT_XUI_CONTEXT_CANCEL	27
#define XGE_EVENT_XUI_CAPTURE_CANCEL	28
#define XGE_EVENT_XUI_CLICK			29
#define XGE_EVENT_XUI_DOUBLE_CLICK	30
#define XGE_EVENT_XUI_CONTEXT_MENU	31
#define XGE_EVENT_XUI_HOTKEY		32
#define XGE_EVENT_XUI_COMMAND		33
#define XGE_EVENT_XUI_DRAG_BEGIN	34
#define XGE_EVENT_XUI_DRAG_MOVE		35
#define XGE_EVENT_XUI_DRAG_END		36
#define XGE_EVENT_XUI_DRAG_CANCEL	37
#define XGE_EVENT_XUI_BOUNDS_CHANGED	38
#define XGE_EVENT_XUI_VISIBLE_CHANGED	39
#define XGE_EVENT_XUI_ENABLED_CHANGED	40
#define XGE_EVENT_XUI_MOUSE_ENTER	XGE_EVENT_XUI_POINTER_ENTER
#define XGE_EVENT_XUI_MOUSE_LEAVE	XGE_EVENT_XUI_POINTER_LEAVE

#define XGE_XUI_WIDGET_EVENT_SLOT_COUNT	48

#define XGE_XUI_EVENT_MASK_RAW			0x00000001u
#define XGE_XUI_EVENT_MASK_MOUSE_ENTER	0x00000002u
#define XGE_XUI_EVENT_MASK_MOUSE_LEAVE	0x00000004u
#define XGE_XUI_EVENT_MASK_MOUSE_MOVE	0x00000008u
#define XGE_XUI_EVENT_MASK_MOUSE_DOWN	0x00000010u
#define XGE_XUI_EVENT_MASK_MOUSE_UP		0x00000020u
#define XGE_XUI_EVENT_MASK_MOUSE_WHEEL	0x00000040u
#define XGE_XUI_EVENT_MASK_CLICK		0x00000080u
#define XGE_XUI_EVENT_MASK_DOUBLE_CLICK	0x00000100u
#define XGE_XUI_EVENT_MASK_CONTEXT_MENU	0x00000200u
#define XGE_XUI_EVENT_MASK_KEY_DOWN		0x00000400u
#define XGE_XUI_EVENT_MASK_KEY_UP		0x00000800u
#define XGE_XUI_EVENT_MASK_TEXT_INPUT	0x00001000u
#define XGE_XUI_EVENT_MASK_HOTKEY		0x00002000u
#define XGE_XUI_EVENT_MASK_COMMAND		0x00004000u
#define XGE_XUI_EVENT_MASK_FOCUS		0x00008000u
#define XGE_XUI_EVENT_MASK_CAPTURE		0x00010000u
#define XGE_XUI_EVENT_MASK_DRAG			0x00020000u
#define XGE_XUI_EVENT_MASK_STATE		0x00040000u
#define XGE_XUI_EVENT_MASK_TOOLTIP		0x00080000u
#define XGE_XUI_EVENT_MASK_POINTER		(XGE_XUI_EVENT_MASK_MOUSE_ENTER | XGE_XUI_EVENT_MASK_MOUSE_LEAVE | XGE_XUI_EVENT_MASK_MOUSE_MOVE | XGE_XUI_EVENT_MASK_MOUSE_DOWN | XGE_XUI_EVENT_MASK_MOUSE_UP | XGE_XUI_EVENT_MASK_MOUSE_WHEEL | XGE_XUI_EVENT_MASK_CLICK | XGE_XUI_EVENT_MASK_DOUBLE_CLICK | XGE_XUI_EVENT_MASK_CONTEXT_MENU)
#define XGE_XUI_EVENT_MASK_KEYBOARD		(XGE_XUI_EVENT_MASK_KEY_DOWN | XGE_XUI_EVENT_MASK_KEY_UP | XGE_XUI_EVENT_MASK_TEXT_INPUT | XGE_XUI_EVENT_MASK_HOTKEY | XGE_XUI_EVENT_MASK_COMMAND)

#define XGE_KEY_MOD_SHIFT	0x0001
#define XGE_KEY_MOD_CTRL	0x0002
#define XGE_KEY_MOD_ALT		0x0004
#define XGE_KEY_MOD_SUPER	0x0008

#define XGE_GAMEPAD_A				0x00000001u
#define XGE_GAMEPAD_B				0x00000002u
#define XGE_GAMEPAD_X				0x00000004u
#define XGE_GAMEPAD_Y				0x00000008u
#define XGE_GAMEPAD_LEFT_SHOULDER	0x00000010u
#define XGE_GAMEPAD_RIGHT_SHOULDER	0x00000020u
#define XGE_GAMEPAD_BACK			0x00000040u
#define XGE_GAMEPAD_START			0x00000080u
#define XGE_GAMEPAD_LEFT_STICK		0x00000100u
#define XGE_GAMEPAD_RIGHT_STICK		0x00000200u
#define XGE_GAMEPAD_DPAD_UP		0x00000400u
#define XGE_GAMEPAD_DPAD_DOWN		0x00000800u
#define XGE_GAMEPAD_DPAD_LEFT		0x00001000u
#define XGE_GAMEPAD_DPAD_RIGHT		0x00002000u
#define XGE_GAMEPAD_GUIDE			0x00004000u

#define XGE_XUI_LAYOUT_ABSOLUTE	0
#define XGE_XUI_LAYOUT_ROW		1
#define XGE_XUI_LAYOUT_COLUMN	2
#define XGE_XUI_LAYOUT_STACK	3
#define XGE_XUI_LAYOUT_GRID		4
#define XGE_XUI_LAYOUT_DOCK		5

#define XGE_XUI_WIDGET_ROLE_CONTROL		0
#define XGE_XUI_WIDGET_ROLE_CONTAINER	1
#define XGE_XUI_WIDGET_ROLE_VIEWPORT	2
#define XGE_XUI_WIDGET_ROLE_OVERLAY		3

#define XGE_XUI_LAYER_NORMAL		0
#define XGE_XUI_LAYER_FLOATING		1
#define XGE_XUI_LAYER_POPUP			2
#define XGE_XUI_LAYER_MODAL			3
#define XGE_XUI_LAYER_TOOLTIP		4
#define XGE_XUI_LAYER_DRAG_ADORNER	5
#define XGE_XUI_LAYER_DEBUG			6

#define XGE_XUI_OVERFLOW_VISIBLE	0
#define XGE_XUI_OVERFLOW_CLIP		1
#define XGE_XUI_OVERFLOW_HIDDEN		2
#define XGE_XUI_OVERFLOW_SCROLL		3

#define XGE_XUI_IME_DISABLED	0
#define XGE_XUI_IME_ENABLED		1
#define XGE_XUI_IME_AUTO		2

#define XGE_XUI_DOCK_FILL		0
#define XGE_XUI_DOCK_CENTER		0
#define XGE_XUI_DOCK_LEFT		1
#define XGE_XUI_DOCK_TOP		2
#define XGE_XUI_DOCK_RIGHT		3
#define XGE_XUI_DOCK_BOTTOM		4

#define XGE_XUI_ALIGN_START		0
#define XGE_XUI_ALIGN_CENTER	1
#define XGE_XUI_ALIGN_END		2
#define XGE_XUI_ALIGN_STRETCH	3

#define XGE_XUI_JUSTIFY_START			0
#define XGE_XUI_JUSTIFY_CENTER			1
#define XGE_XUI_JUSTIFY_END			2
#define XGE_XUI_JUSTIFY_SPACE_BETWEEN	3

#define XGE_XUI_ANCHOR_LEFT		0x0001
#define XGE_XUI_ANCHOR_TOP		0x0002
#define XGE_XUI_ANCHOR_RIGHT	0x0004
#define XGE_XUI_ANCHOR_BOTTOM	0x0008

#define XGE_XUI_SIZE_PX		0
#define XGE_XUI_SIZE_DIP	1
#define XGE_XUI_SIZE_PERCENT	2
#define XGE_XUI_SIZE_CONTENT	3
#define XGE_XUI_SIZE_GROW	4

#define XGE_XUI_WIDGET_VISIBLE		0x0001
#define XGE_XUI_WIDGET_ENABLED		0x0002
#define XGE_XUI_WIDGET_FOCUSABLE	0x0004
#define XGE_XUI_WIDGET_CLIP			0x0008
#define XGE_XUI_WIDGET_DIRTY_LAYOUT	0x0010
#define XGE_XUI_WIDGET_DIRTY_PAINT	0x0020
#define XGE_XUI_WIDGET_DIRTY_STYLE	0x0040
#define XGE_XUI_WIDGET_HIT_TEST_VISIBLE	0x0080
#define XGE_XUI_WIDGET_INPUT_TRANSPARENT	0x0100
#define XGE_XUI_WIDGET_TAB_STOP		0x0200
#define XGE_XUI_WIDGET_FOCUS_SCOPE	0x0400
#define XGE_XUI_WIDGET_DRAG_ENABLED	0x0800

#define XGE_XUI_CLIP_STACK_MAX		32

#define XGE_XUI_WIDGET_CALLBACK_EVENT		0x0001
#define XGE_XUI_WIDGET_CALLBACK_UPDATE		0x0002
#define XGE_XUI_WIDGET_CALLBACK_MEASURE		0x0004
#define XGE_XUI_WIDGET_CALLBACK_PAINT		0x0008
#define XGE_XUI_WIDGET_CALLBACK_PAINT_AFTER	0x0010
#define XGE_XUI_WIDGET_CALLBACK_PAINT_BEFORE	0x0020

#define XGE_XUI_STATE_NORMAL	0x0000
#define XGE_XUI_STATE_HOVER		0x0001
#define XGE_XUI_STATE_ACTIVE	0x0002
#define XGE_XUI_STATE_FOCUS		0x0004
#define XGE_XUI_STATE_DISABLED	0x0008
#define XGE_XUI_STATE_CHECKED	0x0010

#define XGE_XUI_STATE_STYLE_BACKGROUND			0x0001
#define XGE_XUI_STATE_STYLE_BORDER_COLOR		0x0002
#define XGE_XUI_STATE_STYLE_BORDER_WIDTH		0x0004
#define XGE_XUI_STATE_STYLE_FOCUS_RING_COLOR	0x0008
#define XGE_XUI_STATE_STYLE_FOCUS_RING_WIDTH	0x0010
#define XGE_XUI_STATE_STYLE_DISABLED_OVERLAY	0x0020
#define XGE_XUI_WIDGET_STATE_STYLE_COUNT		6

#define XGE_XUI_OWNER_DRAW_NONE				0
#define XGE_XUI_OWNER_DRAW_CONTENT			1
#define XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN	2
#define XGE_XUI_OWNER_DRAW_FULL				3

#define XGE_XUI_PAINT_PART_WIDGET		0
#define XGE_XUI_PAINT_PART_CONTENT		1
#define XGE_XUI_PAINT_PART_ITEM			2
#define XGE_XUI_PAINT_PART_HEADER		3
#define XGE_XUI_PAINT_PART_CELL			4
#define XGE_XUI_PAINT_PART_THUMB		5
#define XGE_XUI_PAINT_PART_TRACK		6
#define XGE_XUI_PAINT_PART_ICON			7
#define XGE_XUI_PAINT_PART_TEXT			8
#define XGE_XUI_PAINT_PART_SEPARATOR	9

#define XGE_XUI_MSG_BOX_ICON_NONE		-1
#define XGE_XUI_MSG_BOX_ICON_INFO		0
#define XGE_XUI_MSG_BOX_ICON_QUEST		1
#define XGE_XUI_MSG_BOX_ICON_WAR		2
#define XGE_XUI_MSG_BOX_ICON_ERROR		3
#define XGE_XUI_MSG_TIP_ICON_NONE		XGE_XUI_MSG_BOX_ICON_NONE
#define XGE_XUI_MSG_TIP_ICON_INFO		XGE_XUI_MSG_BOX_ICON_INFO
#define XGE_XUI_MSG_TIP_ICON_QUEST		XGE_XUI_MSG_BOX_ICON_QUEST
#define XGE_XUI_MSG_TIP_ICON_WAR		XGE_XUI_MSG_BOX_ICON_WAR
#define XGE_XUI_MSG_TIP_ICON_ERROR		XGE_XUI_MSG_BOX_ICON_ERROR
#define XGE_XUI_MSG_BOX_BUTTON_OK			0
#define XGE_XUI_MSG_BOX_BUTTON_OK_CANCEL	1
#define XGE_XUI_MSG_BOX_BUTTON_YES_NO		2
#define XGE_XUI_MSG_BOX_BUTTON_YES_NO_CANCEL	3
#define XGE_XUI_MSG_BOX_BUTTON_CUSTOM		4
#define XGE_XUI_MSG_BOX_RESULT_CLOSE		-1
#define XGE_XUI_MSG_BOX_RESULT_OK			0
#define XGE_XUI_MSG_BOX_RESULT_CANCEL		1
#define XGE_XUI_MSG_BOX_RESULT_YES			0
#define XGE_XUI_MSG_BOX_RESULT_NO			2
#define XGE_XUI_MSG_BOX_BUTTON_CAPACITY	8
#define XGE_XUI_BUTTON_SEMANTIC_DEFAULT	0
#define XGE_XUI_BUTTON_SEMANTIC_PRIMARY	1
#define XGE_XUI_BUTTON_SEMANTIC_DANGER	2
#define XGE_XUI_BUTTON_ICON_LEFT		0
#define XGE_XUI_BUTTON_ICON_RIGHT		1
#define XGE_XUI_BUTTON_ICON_TOP			2
#define XGE_XUI_BUTTON_ICON_BOTTOM		3
#define XGE_XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT	0
#define XGE_XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT	1
#define XGE_XUI_BUTTON_BADGE_ICON_TOP_RIGHT		2
#define XGE_XUI_BUTTON_BADGE_TEXT_TOP_RIGHT		3
#define XGE_XUI_INPUT_ICON_NONE		0
#define XGE_XUI_INPUT_ICON_SEARCH	1
#define XGE_XUI_INPUT_ICON_USER		2
#define XGE_XUI_INPUT_ICON_LOCK		3
#define XGE_XUI_INPUT_ICON_EYE		4
#define XGE_XUI_INPUT_DECORATION_SIDE_LEADING	0
#define XGE_XUI_INPUT_DECORATION_SIDE_TRAILING	1
#define XGE_XUI_INPUT_DECORATION_NONE			0
#define XGE_XUI_INPUT_DECORATION_ICON			1
#define XGE_XUI_INPUT_DECORATION_TEXT			2
#define XGE_XUI_INPUT_DECORATION_TEXTURE		3
#define XGE_XUI_INPUT_DECORATION_CLEAR			4
#define XGE_XUI_INPUT_DECORATION_CUSTOM_PAINT	5
#define XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS				0
#define XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY		1
#define XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED		2
#define XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED_NOT_EMPTY	3
#define XGE_XUI_INPUT_TEXT_ALIGN_LEFT				0
#define XGE_XUI_INPUT_TEXT_ALIGN_CENTER				1
#define XGE_XUI_INPUT_TEXT_ALIGN_RIGHT				2
#define XGE_XUI_NUMERIC_INPUT_BUTTON_NONE			0
#define XGE_XUI_NUMERIC_INPUT_BUTTON_UP				1
#define XGE_XUI_NUMERIC_INPUT_BUTTON_DOWN			2

#define XGE_XUI_WINDOW_EDGE_LEFT	0x0001
#define XGE_XUI_WINDOW_EDGE_TOP		0x0002
#define XGE_XUI_WINDOW_EDGE_RIGHT	0x0004
#define XGE_XUI_WINDOW_EDGE_BOTTOM	0x0008

#define XGE_XUI_IMAGE_NATURAL		0
#define XGE_XUI_IMAGE_STRETCH		1
#define XGE_XUI_IMAGE_CONTAIN		2
#define XGE_XUI_IMAGE_FIT			XGE_XUI_IMAGE_CONTAIN
#define XGE_XUI_IMAGE_COVER			3
#define XGE_XUI_IMAGE_SCALE_DOWN	4
#define XGE_XUI_IMAGE_CUSTOM		5
#define XGE_XUI_IMAGE_CENTER		XGE_XUI_IMAGE_NATURAL
#define XGE_XUI_SEPARATOR_HORIZONTAL	0
#define XGE_XUI_SEPARATOR_VERTICAL		1
#define XGE_XUI_ORIENTATION_HORIZONTAL	XGE_XUI_SEPARATOR_HORIZONTAL
#define XGE_XUI_ORIENTATION_VERTICAL	XGE_XUI_SEPARATOR_VERTICAL
#define XGE_XUI_SEPARATOR_SOLID		0
#define XGE_XUI_SEPARATOR_DOT		1
#define XGE_XUI_SEPARATOR_DASH		2
#define XGE_XUI_SEPARATOR_DASH_DOT	3
#define XGE_XUI_SPLIT_LAYOUT_MAX_PANES	16
#define XGE_XUI_SPLIT_PANE_GROW		0
#define XGE_XUI_SPLIT_PANE_FIXED		1
#define XGE_XUI_DOCK_REGION_COUNT	5
#define XGE_XUI_DOCK_REGION_DOCUMENT	0
#define XGE_XUI_DOCK_REGION_LEFT		1
#define XGE_XUI_DOCK_REGION_RIGHT		2
#define XGE_XUI_DOCK_REGION_TOP		3
#define XGE_XUI_DOCK_REGION_BOTTOM		4
#define XGE_XUI_DOCK_SIDE_NONE			0
#define XGE_XUI_DOCK_SIDE_LEFT			1
#define XGE_XUI_DOCK_SIDE_RIGHT		2
#define XGE_XUI_DOCK_SIDE_TOP			3
#define XGE_XUI_DOCK_SIDE_BOTTOM		4
#define XGE_XUI_DOCK_SIDE_FILL			5
#define XGE_XUI_DOCK_NODE_EMPTY		0
#define XGE_XUI_DOCK_NODE_SPLIT		1
#define XGE_XUI_DOCK_NODE_PANE			2
#define XGE_XUI_DOCK_WINDOW_FLOATING	0
#define XGE_XUI_DOCK_WINDOW_DOCKED		1
#define XGE_XUI_DOCK_WINDOW_HIDDEN		2
#define XGE_XUI_DOCK_WINDOW_AUTO_HIDE	3
#define XGE_XUI_DOCK_DRAG_IDLE			0
#define XGE_XUI_DOCK_DRAG_PENDING		1
#define XGE_XUI_DOCK_DRAG_DRAGGING		2
#define XGE_XUI_DOCK_DRAG_COMMITTING	3
#define XGE_XUI_DOCK_DRAG_CANCELING	4
#define XGE_XUI_PROGRESS_LEFT_TO_RIGHT	0
#define XGE_XUI_PROGRESS_RIGHT_TO_LEFT	1
#define XGE_XUI_PROGRESS_BOTTOM_TO_TOP	2
#define XGE_XUI_PROGRESS_TOP_TO_BOTTOM	3
#define XGE_XUI_PROGRESS_FILL_STRETCH	0
#define XGE_XUI_PROGRESS_FILL_REVEAL	1
#define XGE_XUI_SCROLLBAR_POLICY_AUTO		0
#define XGE_XUI_SCROLLBAR_POLICY_ALWAYS		1
#define XGE_XUI_SCROLLBAR_POLICY_HIDDEN		2
#define XGE_XUI_SCROLLBAR_MODE_FULL		0
#define XGE_XUI_SCROLLBAR_MODE_COMPACT		1
#define XGE_XUI_SCROLLBAR_BUTTONS_AUTO		0
#define XGE_XUI_SCROLLBAR_BUTTONS_OFF		1
#define XGE_XUI_SCROLLBAR_BUTTONS_ON		2
#define XGE_XUI_SCROLLBAR_PART_NONE		0
#define XGE_XUI_SCROLLBAR_PART_BUTTON_START	1
#define XGE_XUI_SCROLLBAR_PART_BUTTON_END	2
#define XGE_XUI_SCROLLBAR_PART_THUMB		3
#define XGE_XUI_SCROLLBAR_PART_TRACK		4
#define XGE_XUI_WHEEL_AXIS_VERTICAL		0
#define XGE_XUI_WHEEL_AXIS_HORIZONTAL	1
#define XGE_XUI_WHEEL_AXIS_BOTH			2
#define XGE_XUI_SCROLL_FRAME_CORNER_NONE	0
#define XGE_XUI_SCROLL_FRAME_CORNER_AUTO	1
#define XGE_XUI_SCROLL_FRAME_CORNER_GRIP	2
#define XGE_XUI_SELECTION_SINGLE	0
#define XGE_XUI_SELECTION_MULTI	1
#define XGE_XUI_SELECTION_RANGE	2
#define XGE_XUI_LIST_ITEM_SELECTED	0x0001
#define XGE_XUI_LIST_ITEM_HOVER		0x0002
#define XGE_XUI_LIST_ITEM_DISABLED	0x0004
#define XGE_XUI_LIST_ITEM_FOCUS		0x0008
#define XGE_XUI_TREE_ITEM_SELECTED	XGE_XUI_LIST_ITEM_SELECTED
#define XGE_XUI_TREE_ITEM_HOVER		XGE_XUI_LIST_ITEM_HOVER
#define XGE_XUI_TREE_ITEM_DISABLED	XGE_XUI_LIST_ITEM_DISABLED
#define XGE_XUI_TREE_ITEM_FOCUS		XGE_XUI_LIST_ITEM_FOCUS
#define XGE_XUI_TREE_ITEM_EXPANDED	0x0010
#define XGE_XUI_TREE_ITEM_HAS_CHILDREN	0x0020
#define XGE_XUI_TREE_ITEM_CHECKED	0x0040
#define XGE_XUI_TOOLBAR_ITEM_BUTTON		0
#define XGE_XUI_TOOLBAR_ITEM_TOGGLE		1
#define XGE_XUI_TOOLBAR_ITEM_SEPARATOR	2
#define XGE_XUI_TOOLBAR_CAPACITY		32
#define XGE_XUI_MENUBAR_CAPACITY		16
#define XGE_XUI_MENUBAR_ITEM_ENABLED	0x0001
#define XGE_XUI_STATUS_BAR_ITEM_TEXT		0
#define XGE_XUI_STATUS_BAR_ITEM_PROGRESS	1
#define XGE_XUI_STATUS_BAR_ITEM_SPACER		2
#define XGE_XUI_STATUS_BAR_SECTION_LEFT		0
#define XGE_XUI_STATUS_BAR_SECTION_CENTER	1
#define XGE_XUI_STATUS_BAR_SECTION_RIGHT	2
#define XGE_XUI_STATUS_BAR_CAPACITY		32
#define XGE_XUI_TREE_VIEW_NODE_CAPACITY		256
#define XGE_XUI_TREE_VIEW_VISIBLE_CAPACITY	256
#define XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY	64
#define XGE_XUI_TABLE_VIEW_SELECTION_CELL	0
#define XGE_XUI_TABLE_VIEW_SELECTION_ROW		1
#define XGE_XUI_TABLE_CELL_SELECTED		0x0001
#define XGE_XUI_TABLE_CELL_HOVER		0x0002
#define XGE_XUI_TABLE_CELL_DISABLED		0x0004
#define XGE_XUI_TABLE_CELL_FOCUS		0x0008
#define XGE_XUI_TABLE_CELL_HEADER		0x0010
#define XGE_XUI_TABLE_CELL_MERGED		0x0020
#define XGE_XUI_TABLE_CELL_EDITING		0x0040
#define XGE_XUI_TABLE_CELL_INVALID		0x0080
#define XGE_XUI_TABLE_CELL_DIRTY		0x0100
#define XGE_XUI_TABLE_CELL_TYPE_TEXT		0
#define XGE_XUI_TABLE_CELL_TYPE_INT		1
#define XGE_XUI_TABLE_CELL_TYPE_FLOAT		2
#define XGE_XUI_TABLE_CELL_TYPE_BOOL		3
#define XGE_XUI_TABLE_CELL_TYPE_TEXTAREA	4
#define XGE_XUI_TABLE_CELL_TYPE_DATE		5
#define XGE_XUI_TABLE_CELL_TYPE_TIME		6
#define XGE_XUI_TABLE_CELL_TYPE_DATETIME	7
#define XGE_XUI_TABLE_CELL_TYPE_ENUM		8
#define XGE_XUI_TABLE_CELL_TYPE_COLOR		9
#define XGE_XUI_TABLE_CELL_TYPE_PICKER		10
#define XGE_XUI_TABLE_CELL_TYPE_CUSTOM		11
#define XGE_XUI_TABLE_CELL_TYPE_FILE		12
#define XGE_XUI_TABLE_CELL_TYPE_IMAGE		13
#define XGE_XUI_TABLE_GRID_VALUE_CAPACITY	256
#define XGE_XUI_TABLE_GRID_EDIT_DISPLAY		0
#define XGE_XUI_TABLE_GRID_EDIT_QUICK		1
#define XGE_XUI_TABLE_GRID_EDIT_IMMEDIATE	2
#define XGE_XUI_TIMELINE_FRAME_EMPTY		0
#define XGE_XUI_TIMELINE_FRAME_NORMAL		1
#define XGE_XUI_TIMELINE_FRAME_KEY			2
#define XGE_XUI_TIMELINE_FRAME_BLANK_KEY	3
#define XGE_XUI_TIMELINE_SPAN_CUSTOM		0
#define XGE_XUI_TIMELINE_SPAN_MOTION		1
#define XGE_XUI_TIMELINE_SPAN_SHAPE			2
#define XGE_XUI_TIMELINE_SPAN_EVENT			3
#define XGE_XUI_TIMELINE_SPAN_AUDIO			4
#define XGE_XUI_TIMELINE_SPAN_HOLD			5
#define XGE_XUI_TIMELINE_STATE_HOVER		0x0001
#define XGE_XUI_TIMELINE_STATE_SELECTED		0x0002
#define XGE_XUI_TIMELINE_STATE_LOCKED		0x0004
#define XGE_XUI_TIMELINE_STATE_HIDDEN		0x0008
#define XGE_XUI_TIMELINE_STATE_CURRENT		0x0010
#define XGE_XUI_TIMELINE_STATE_KEY			0x0020
#define XGE_XUI_TIMELINE_STATE_BLANK_KEY	0x0040
#define XGE_XUI_TIMELINE_STATE_SPAN			0x0080
#define XGE_XUI_TIMELINE_HIT_NONE			0
#define XGE_XUI_TIMELINE_HIT_CORNER			1
#define XGE_XUI_TIMELINE_HIT_RULER			2
#define XGE_XUI_TIMELINE_HIT_PLAYHEAD		3
#define XGE_XUI_TIMELINE_HIT_LAYER_ROW		4
#define XGE_XUI_TIMELINE_HIT_LAYER_NAME		5
#define XGE_XUI_TIMELINE_HIT_LAYER_VISIBLE	6
#define XGE_XUI_TIMELINE_HIT_LAYER_LOCK		7
#define XGE_XUI_TIMELINE_HIT_FRAME			8
#define XGE_XUI_TIMELINE_HIT_SPAN			9
#define XGE_XUI_TIMELINE_HIT_SELECTION		10
#define XGE_XUI_TIMELINE_HIT_HSCROLLBAR		11
#define XGE_XUI_TIMELINE_HIT_VSCROLLBAR		12
#define XGE_XUI_TIMELINE_LAYER_CHANGE_NAME		1
#define XGE_XUI_TIMELINE_LAYER_CHANGE_VISIBLE	2
#define XGE_XUI_TIMELINE_LAYER_CHANGE_LOCKED	3
#define XGE_XUI_TIMELINE_LAYER_CHANGE_SELECT	4
#define XGE_XUI_TIMELINE_MENU_LAYER_RENAME		1
#define XGE_XUI_TIMELINE_MENU_LAYER_SHOW_HIDE	2
#define XGE_XUI_TIMELINE_MENU_LAYER_LOCK_UNLOCK	3
#define XGE_XUI_TIMELINE_MENU_LAYER_ADD			4
#define XGE_XUI_TIMELINE_MENU_LAYER_DELETE		5
#define XGE_XUI_TIMELINE_MENU_LAYER_MOVE_UP		6
#define XGE_XUI_TIMELINE_MENU_LAYER_MOVE_DOWN	7
#define XGE_XUI_TIMELINE_MENU_FRAME_INSERT		20
#define XGE_XUI_TIMELINE_MENU_FRAME_KEY			21
#define XGE_XUI_TIMELINE_MENU_FRAME_BLANK_KEY	22
#define XGE_XUI_TIMELINE_MENU_FRAME_CLEAR		23
#define XGE_XUI_TIMELINE_MENU_SPAN_CREATE		24
#define XGE_XUI_TIMELINE_MENU_SPAN_CLEAR		25
#define XGE_XUI_PROPERTY_GRID_CATEGORY_CAPACITY	32
#define XGE_XUI_PROPERTY_GRID_PROPERTY_CAPACITY	128
#define XGE_XUI_PROPERTY_GRID_VISIBLE_CAPACITY	(XGE_XUI_PROPERTY_GRID_CATEGORY_CAPACITY + XGE_XUI_PROPERTY_GRID_PROPERTY_CAPACITY)
#define XGE_XUI_PROPERTY_GRID_ID_CAPACITY	64
#define XGE_XUI_PROPERTY_GRID_NAME_CAPACITY	96
#define XGE_XUI_PROPERTY_GRID_DESCRIPTION_CAPACITY	256
#define XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY	256
#define XGE_XUI_PROPERTY_GRID_OPTION_CAPACITY	32
#define XGE_XUI_PROPERTY_GRID_DESCRIPTION_NONE	0
#define XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP	1
#define XGE_XUI_PROPERTY_GRID_DESCRIPTION_PANEL	2
#define XGE_XUI_PROPERTY_GRID_DESCRIPTION_BOTH	3
#define XGE_XUI_PROPERTY_FLAG_READONLY	0x0001
#define XGE_XUI_PROPERTY_FLAG_DISABLED	0x0002
#define XGE_XUI_PROPERTY_FLAG_DIRTY	0x0004
#define XGE_XUI_PROPERTY_FLAG_INVALID	0x0008
#define XGE_XUI_PROPERTY_FLAG_HIDDEN	0x0010
#define XGE_XUI_ACCORDION_SECTION_CAPACITY	16
#define XGE_XUI_ACCORDION_TITLE_CAPACITY	64
#define XGE_XUI_ACCORDION_MODE_MULTIPLE		0
#define XGE_XUI_ACCORDION_MODE_SINGLE		1
#define XGE_XUI_COLOR_PICKER_PALETTE_CAPACITY	16
#define XGE_XUI_DATE_PICKER_MODE_DATE		0
#define XGE_XUI_DATE_PICKER_MODE_TIME		1
#define XGE_XUI_DATE_PICKER_MODE_DATETIME	2
#define XGE_XUI_DATE_PICKER_MODE_DATE_RANGE	3
#define XGE_XUI_DATE_PICKER_MODE_TIME_RANGE	4
#define XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE	5
#define XGE_XUI_DATE_PICKER_PANEL_CAPACITY	2
#define XGE_XUI_TOAST_VISIBLE_CAPACITY		8
#define XGE_XUI_TOAST_QUEUE_CAPACITY		32
#define XGE_XUI_TOAST_CAPACITY			XGE_XUI_TOAST_VISIBLE_CAPACITY
#define XGE_XUI_MENU_ITEM_CAPACITY		64
#define XGE_XUI_MENU_ITEM_NORMAL		0
#define XGE_XUI_MENU_ITEM_SEPARATOR		1
#define XGE_XUI_MENU_ITEM_CHECK			2
#define XGE_XUI_MENU_ITEM_RADIO			3
#define XGE_XUI_MENU_ITEM_SUBMENU		4
#define XGE_XUI_MENU_ITEM_ENABLED		0x0001
#define XGE_XUI_MENU_ITEM_CHECKED		0x0002
#define XGE_XUI_MENU_ITEM_DEFAULT		0x0004
#define XGE_XUI_MENU_ITEM_DANGER		0x0008
#define XGE_XUI_TOAST_TYPE_INFO			0
#define XGE_XUI_TOAST_TYPE_SUCCESS		1
#define XGE_XUI_TOAST_TYPE_WARNING		2
#define XGE_XUI_TOAST_TYPE_ERROR		3
#define XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT	0
#define XGE_XUI_TOAST_PLACEMENT_TOP_LEFT	1
#define XGE_XUI_TOAST_PLACEMENT_BOTTOM_RIGHT	2
#define XGE_XUI_TOAST_PLACEMENT_BOTTOM_LEFT	3
#define XGE_XUI_TOAST_PLACEMENT_TOP_CENTER	4
#define XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER	5
#define XGE_XUI_TOAST_DIRECTION_AUTO		0
#define XGE_XUI_TOAST_DIRECTION_DOWN		1
#define XGE_XUI_TOAST_DIRECTION_UP		2
#define XGE_XUI_TOAST_CLOSE_TIMEOUT		0
#define XGE_XUI_TOAST_CLOSE_CLICK		1
#define XGE_XUI_TOAST_CLOSE_BUTTON		2
#define XGE_XUI_TOAST_CLOSE_CLEAR		3
#define XGE_XUI_TOAST_CLOSE_CAPACITY_DROP	4
#define XGE_XUI_TOAST_CLOSE_API		5

#define XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT	0
#define XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_RIGHT	1
#define XGE_XUI_OVERLAY_PLACEMENT_TOP_LEFT	2
#define XGE_XUI_OVERLAY_PLACEMENT_TOP_RIGHT	3
#define XGE_XUI_OVERLAY_PLACEMENT_RIGHT_TOP	4
#define XGE_XUI_OVERLAY_PLACEMENT_LEFT_TOP	5
#define XGE_XUI_OVERLAY_PLACEMENT_CENTER	6
#define XGE_XUI_OVERLAY_PLACEMENT_CURSOR	7
#define XGE_XUI_OVERLAY_PLACEMENT_MANUAL	8
#define XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT	0
#define XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT	1
#define XGE_XUI_POPUP_ANCHOR_TOP_LEFT		2
#define XGE_XUI_POPUP_ANCHOR_TOP_RIGHT		3
#define XGE_XUI_POPUP_ANCHOR_CURSOR		4
#define XGE_XUI_POPUP_ANCHOR_FIXED		5
#define XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN	0
#define XGE_XUI_POPUP_DIRECTION_RIGHT_UP	1
#define XGE_XUI_POPUP_DIRECTION_LEFT_DOWN	2
#define XGE_XUI_POPUP_DIRECTION_LEFT_UP		3
#define XGE_XUI_POPUP_OUTSIDE_CLOSE		0
#define XGE_XUI_POPUP_OUTSIDE_IGNORE		1
#define XGE_XUI_POPUP_OUTSIDE_CONSUME		2
#define XGE_XUI_POPUP_OWNER_PASSTHROUGH		0
#define XGE_XUI_POPUP_OWNER_CLOSE		1
#define XGE_XUI_POPUP_OWNER_TOGGLE		2
#define XGE_XUI_POPUP_OWNER_CONSUME		3
#define XGE_XUI_POPUP_ESCAPE_CLOSE		0
#define XGE_XUI_POPUP_ESCAPE_IGNORE		1
#define XGE_XUI_POPUP_FOCUS_NONE		0
#define XGE_XUI_POPUP_FOCUS_POPUP		1
#define XGE_XUI_POPUP_FOCUS_FIRST_CHILD		2
#define XGE_XUI_POPUP_FOCUS_CUSTOM		3
#define XGE_XUI_COMBO_POPUP_AUTO		0
#define XGE_XUI_COMBO_POPUP_BOTTOM		1
#define XGE_XUI_COMBO_POPUP_TOP			2

#define XGE_XUI_TOOLTIP_NONE				0
#define XGE_XUI_TOOLTIP_TEXT				1
#define XGE_XUI_TOOLTIP_CUSTOM			2
#define XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM	0
#define XGE_XUI_TOOLTIP_ANCHOR_WIDGET_TOP		1
#define XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT		2
#define XGE_XUI_TOOLTIP_ANCHOR_WIDGET_LEFT		3
#define XGE_XUI_TOOLTIP_ANCHOR_CURSOR			4

#define XGE_XUI_PAINT_RECT		1
#define XGE_XUI_PAINT_ROUNDED_RECT	2
#define XGE_XUI_PAINT_IMAGE		3
#define XGE_XUI_PAINT_TEXT		4
#define XGE_XUI_PAINT_CLIP_SET	5
#define XGE_XUI_PAINT_CLIP_CLEAR	6
#define XGE_XUI_PAINT_CUSTOM	7

#define XGE_XUI_EVENT_CONTINUE	0
#define XGE_XUI_EVENT_HANDLED	1
#define XGE_XUI_EVENT_CONSUMED	2
#define XGE_XUI_EVENT_PHASE_NONE	0
#define XGE_XUI_EVENT_PHASE_TUNNEL	1
#define XGE_XUI_EVENT_PHASE_TARGET	2
#define XGE_XUI_EVENT_PHASE_BUBBLE	3
#define XGE_XUI_EVENT_QUEUE_CAPACITY	64
#define XGE_XUI_HOTKEY_CAPACITY	64
#define XGE_XUI_FONT_CAPACITY	32
#define XGE_XUI_FONT_NAME_CAPACITY	64
#define XGE_XUI_POINTER_CAPTURE_CAPACITY	(XGE_TOUCH_MAX + 1)
#define XGE_XUI_PAGE_ERROR_CAPACITY	256
#define XGE_XUI_PAGE_IMPORT_CAPACITY	16
#define XGE_XUI_PAGE_IMPORT_URI_CAPACITY	128
#define XGE_XUI_PAGE_INDEX_CAPACITY	256
#define XGE_XUI_PAGE_CLICK_BINDING_CAPACITY	128
#define XGE_XUI_PAGE_EVENT_BINDING_CAPACITY	256
#define XGE_XUI_PAGE_HOTKEY_BINDING_CAPACITY	128
#define XGE_XUI_PAGE_MODEL_BINDING_CAPACITY	128
#define XGE_XUI_PAGE_BUTTON_CAPACITY	64
#define XGE_XUI_PAGE_IMAGE_CAPACITY	64
#define XGE_XUI_PAGE_INPUT_CAPACITY	32
#define XGE_XUI_PAGE_TEXT_EDIT_CAPACITY	32
#define XGE_XUI_PAGE_NUMERIC_INPUT_CAPACITY	32
#define XGE_XUI_PAGE_COLOR_PICKER_CAPACITY	32
#define XGE_XUI_PAGE_DATE_PICKER_CAPACITY	32
#define XGE_XUI_PAGE_CHECKBOX_CAPACITY	32
#define XGE_XUI_PAGE_RADIO_CAPACITY	32
#define XGE_XUI_PAGE_TOGGLE_CAPACITY	32
#define XGE_XUI_PAGE_SLIDER_CAPACITY	32
#define XGE_XUI_PAGE_SCROLLBAR_CAPACITY	32
#define XGE_XUI_PAGE_PROGRESS_CAPACITY	32
#define XGE_XUI_PAGE_PAGER_CAPACITY	32
#define XGE_XUI_PAGE_WINDOW_CAPACITY	16
#define XGE_XUI_PAGE_PANEL_CAPACITY	32
#define XGE_XUI_PAGE_TABS_CAPACITY	32
#define XGE_XUI_PAGE_SPLIT_LAYOUT_CAPACITY	32
#define XGE_XUI_PAGE_DOCK_LAYOUT_CAPACITY	8
#define XGE_XUI_PAGE_DOCK_WINDOW_CAPACITY	64
#define XGE_XUI_PAGE_TOOLBAR_CAPACITY	32
#define XGE_XUI_PAGE_MENUBAR_CAPACITY	16
#define XGE_XUI_PAGE_STATUS_BAR_CAPACITY	32
#define XGE_XUI_PAGE_COMBO_BOX_CAPACITY	32
#define XGE_XUI_PAGE_COMBO_BOX_ITEM_CAPACITY	64
#define XGE_XUI_PAGE_POPUP_CAPACITY	32
#define XGE_XUI_PAGE_MENU_CAPACITY	32
#define XGE_XUI_PAGE_OVERLAY_PORTAL_CAPACITY	64
#define XGE_XUI_PAGE_OVERLAY_PORTAL_PATH_CAPACITY	128
#define XGE_XUI_PAGE_LABEL_CAPACITY	64
#define XGE_XUI_PAGE_SEPARATOR_CAPACITY	64
#define XGE_XUI_PAGE_SCROLL_VIEW_CAPACITY	32
#define XGE_XUI_PAGE_LIST_VIEW_CAPACITY	32
#define XGE_XUI_PAGE_LIST_VIEW_ITEM_CAPACITY	256
#define XGE_XUI_PAGE_TREE_VIEW_CAPACITY	32
#define XGE_XUI_PAGE_TABLE_VIEW_CAPACITY	32
#define XGE_XUI_PAGE_TABLE_GRID_CAPACITY	32
#define XGE_XUI_PAGE_TIMELINE_VIEW_CAPACITY	32
#define XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY	128
#define XGE_XUI_PAGE_TABLE_VIEW_CELL_CAPACITY	64
#define XGE_XUI_PAGE_PROPERTY_GRID_CAPACITY	32
#define XGE_XUI_PAGE_ACCORDION_CAPACITY	32
#define XGE_XUI_TABS_PAGE_CAPACITY	32
#define XGE_XUI_TABS_TITLE_CAPACITY	64
#define XGE_XUI_TABS_PLACEMENT_TOP		0
#define XGE_XUI_TABS_PLACEMENT_BOTTOM	1
#define XGE_XUI_TABS_PLACEMENT_LEFT		2
#define XGE_XUI_TABS_PLACEMENT_RIGHT	3
#define XGE_XUI_PAGER_ITEM_CAPACITY	32
#define XGE_XUI_PAGER_ITEM_PREV		1
#define XGE_XUI_PAGER_ITEM_FIRST	2
#define XGE_XUI_PAGER_ITEM_ELLIPSIS	3
#define XGE_XUI_PAGER_ITEM_PAGE		4
#define XGE_XUI_PAGER_ITEM_LAST		5
#define XGE_XUI_PAGER_ITEM_NEXT		6
#define XGE_XUI_BINDER_ENTRY_CAPACITY	64
#define XGE_XUI_MODEL_ENTRY_CAPACITY	128
#define XGE_XUI_MODEL_KEY_CAPACITY	64
#define XGE_XUI_MODEL_VALUE_CAPACITY	128

#define XGE_TOUCH_NONE			0
#define XGE_TOUCH_BEGIN			1
#define XGE_TOUCH_MOVE			2
#define XGE_TOUCH_STATIONARY	3
#define XGE_TOUCH_END			4
#define XGE_TOUCH_CANCEL		5

#define XGE_COLOR_RGBA(r, g, b, a)	((((uint32_t)(r) & 0xFFu) << 24) | (((uint32_t)(g) & 0xFFu) << 16) | (((uint32_t)(b) & 0xFFu) << 8) | ((uint32_t)(a) & 0xFFu))
#define XGE_COLOR_GET_R(c)			(((uint32_t)(c) >> 24) & 0xFFu)
#define XGE_COLOR_GET_G(c)			(((uint32_t)(c) >> 16) & 0xFFu)
#define XGE_COLOR_GET_B(c)			(((uint32_t)(c) >> 8) & 0xFFu)
#define XGE_COLOR_GET_A(c)			((uint32_t)(c) & 0xFFu)

typedef enum xge_result_t {
	XGE_OK = 0,
	XGE_ERROR = -1,
	XGE_ERROR_INVALID_ARGUMENT = -2,
	XGE_ERROR_NOT_INITIALIZED = -3,
	XGE_ERROR_ALREADY_INITIALIZED = -4,
	XGE_ERROR_OUT_OF_MEMORY = -5,
	XGE_ERROR_FILE_NOT_FOUND = -6,
	XGE_ERROR_UNSUPPORTED = -7,
	XGE_ERROR_BACKEND_FAILED = -8,
	XGE_ERROR_GPU_FAILED = -9,
	XGE_ERROR_RESOURCE_FAILED = -10,
	XGE_ERROR_AUDIO_FAILED = -11,
	XGE_ERROR_THREAD_FAILED = -12
} xge_result_t;

typedef struct xge_desc_t {
	int iWidth;
	int iHeight;
	const char* sTitle;
	int iFlags;
	int iRunMode;
	int iTargetFPS;
	void* pNativeWindow;
	void* pUser;
} xge_desc_t;

typedef struct xge_rect_t {
	float fX;
	float fY;
	float fW;
	float fH;
} xge_rect_t;

typedef struct xge_vec2_t {
	float fX;
	float fY;
} xge_vec2_t;

typedef struct xge_vec3_t {
	float fX;
	float fY;
	float fZ;
} xge_vec3_t;

typedef struct xge_mat3_t {
	float m[9];
} xge_mat3_t;

typedef struct xge_mat4_t {
	float m[16];
} xge_mat4_t;

typedef struct xge_color_t {
	float fR;
	float fG;
	float fB;
	float fA;
} xge_color_t;

typedef struct xge_frame_stats_t {
	int iFrameCount;
	int iDrawCallCount;
	int iBatchCount;
	int iDirtyRectCount;
	float fFrameTimeMs;
	float fFrameTimeAvgMs;
	float fFrameTimeMaxMs;
} xge_frame_stats_t;

#if XGE_HAS_DEBUGMODE
typedef struct xge_debug_stats_t {
	xge_frame_stats_t tFrame;
	int iTextureCount;
	uint64_t iTextureMemoryBytes;
	int iFontCount;
	int iAudioCount;
	int iLastGLError;
} xge_debug_stats_t;
#endif

typedef struct xge_platform_backend_t {
	int iType;
	const char* sName;
	int (*init)(void* pUser);
	void (*unit)(void* pUser);
	int (*poll)(void* pUser);
	void (*present)(void* pUser);
	void* pUser;
} xge_platform_backend_t;

typedef struct xge_graphics_backend_t {
	int iType;
	const char* sName;
	int (*init)(void* pUser);
	void (*unit)(void* pUser);
	int (*begin)(void* pUser);
	int (*end)(void* pUser);
	void* pUser;
} xge_graphics_backend_t;

typedef struct xge_gpu_caps_t {
	int iBackend;
	int iMajorVersion;
	int iMinorVersion;
	int iMaxTextureSize;
	char sVendor[64];
	char sRenderer[128];
	char sVersion[64];
	char sShadingLanguage[64];
} xge_gpu_caps_t;

typedef struct xge_graphics_mapping_t {
	int iBackend;
	int bOpenGLCore;
	int bGLES;
	int bWebGL;
	int bVAO;
	int bFramebufferObject;
	int bDepthTexture;
	int bClientSideVertexArray;
	int iRGBA8InternalFormat;
	int iRGBAFormat;
	int iUnsignedByteType;
	char sShaderHeader[128];
} xge_graphics_mapping_t;

typedef struct xge_platform_caps_t {
	int iPlatformBackend;
	int iGraphicsBackend;
	char sPlatformName[64];
	char sGraphicsName[64];
	char sSokolTargetName[64];
	int bWindow;
	int bOffscreen;
	int bRenderTarget;
	int bPBuffer;
	int bSurfaceless;
	int bBoardLinux;
	int bMiniProgram;
	int bWASM;
	int bTouch;
	int bMouse;
	int bKeyboard;
	int bTextInput;
	int bGamepad;
	int bAudio;
	int bResourceProvider;
	int bThreadSafeSubmit;
	int bHighDPI;
	int bSokol;
	int bSokolWindows;
	int bSokolLinuxX11;
	int bSokolLinuxWayland;
	int bSokolMacOS;
	int bSokolAndroid;
	int bSokolIOS;
	int bSokolWeb;
	int bSokolGLCore;
	int bSokolGLES3;
	int bSokolD3D11;
	int bSokolMetal;
	int bSokolDummy;
} xge_platform_caps_t;

typedef struct xge_platform_runtime_t {
	int bRunning;
	int iWindowWidth;
	int iWindowHeight;
	int iFramebufferWidth;
	int iFramebufferHeight;
	float fDpiScale;
	int iKeyEventCount;
	int iTextEventCount;
	int iMouseEventCount;
	int iTouchEventCount;
	int iGamepadEventCount;
	int iResizeEventCount;
	int iQuitEventCount;
} xge_platform_runtime_t;

typedef struct xge_render_thread_caps_t {
	int bSupported;
	int bEnabled;
	int bWorkerDrain;
	int bGLContextOwned;
	int bAsyncFlush;
	int bCanUseWithCurrentContext;
} xge_render_thread_caps_t;

typedef struct xge_camera_t {
	xge_vec2_t tPosition;
	xge_vec2_t tScale;
	float fRotation;
	xge_rect_t tViewport;
	int iCoordinateMode;
	int iProjectionMode;
	float fFovY;
	float fNearZ;
	float fFarZ;
	float fPerspectiveDistance;
} xge_camera_t;

typedef struct xge_texture_t xge_texture_t;
typedef xge_texture_t* xge_texture;
typedef struct xge_render_target_t xge_render_target_t;
typedef xge_render_target_t* xge_render_target;
typedef struct xge_buffer_t xge_buffer_t;
typedef xge_buffer_t* xge_buffer;
typedef struct xge_pass_t xge_pass_t;
typedef xge_pass_t* xge_pass;
typedef struct xge_sprite_batch_t xge_sprite_batch_t;
typedef xge_sprite_batch_t* xge_sprite_batch;
typedef struct xge_shape_batch_t xge_shape_batch_t;
typedef xge_shape_batch_t* xge_shape_batch;
typedef struct xge_shader_t xge_shader_t;
typedef xge_shader_t* xge_shader;
typedef struct xge_material_t xge_material_t;
typedef xge_material_t* xge_material;
typedef struct xge_shader_variant_t xge_shader_variant_t;
typedef xge_shader_variant_t* xge_shader_variant;
typedef struct xge_shader_variant_set_t xge_shader_variant_set_t;
typedef xge_shader_variant_set_t* xge_shader_variant_set;
typedef struct xge_mesh_t xge_mesh_t;
typedef xge_mesh_t* xge_mesh;
typedef struct xge_nine_patch_t xge_nine_patch_t;
typedef xge_nine_patch_t* xge_nine_patch;

typedef struct xge_draw_t {
	xge_texture pTexture;
	xge_rect_t tSrc;
	xge_rect_t tDst;
	xge_vec2_t tOrigin;
	float fRotation;
	uint32_t iColor;
	uint32_t iFlags;
} xge_draw_t;

struct xge_nine_patch_t {
	xge_texture pTexture;
	xge_rect_t tSrc;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	uint32_t iColor;
	int iMode;
	int bEasyMode;
};

typedef struct xge_vertex_t {
	float fX;
	float fY;
	float fZ;
	float fW;
	float fU;
	float fV;
	uint32_t iColor;
} xge_vertex_t;

typedef struct xge_sampler_t {
	int iMinFilter;
	int iMagFilter;
	int iWrapS;
	int iWrapT;
} xge_sampler_t;

struct xge_sprite_batch_t {
	xge_texture pTexture;
	int iCapacity;
	int iCount;
	uint32_t iFlags;
	void* pVertices;
};

typedef struct xge_pipeline_state_t {
	int iBlend;
	int bDepthTest;
	uint32_t iFlags;
} xge_pipeline_state_t;

struct xge_shader_t {
	int iRefCount;
	uint32_t iProgram;
	int iLocResolution;
	int iLocTexture;
	int iLocColor;
	void* pBackend;
};

typedef struct xge_shader_define_t {
	char sName[XGE_SHADER_DEFINE_NAME_MAX];
	int iValue;
} xge_shader_define_t;

struct xge_shader_variant_t {
	uint32_t iKey;
	xge_shader_t tShader;
	int iDefineCount;
	xge_shader_define_t arrDefines[XGE_SHADER_DEFINE_MAX];
	struct xge_shader_variant_t* pNext;
};

struct xge_shader_variant_set_t {
	char* sVertexSource;
	char* sFragmentSource;
	xge_shader_variant pVariants;
	int iCount;
};

struct xge_buffer_t {
	int iType;
	int iUsage;
	int iSize;
	uint32_t iBackendId;
	void* pData;
};

struct xge_material_t {
	xge_shader pShader;
	xge_texture pTexture;
	uint32_t iColor;
	xge_pipeline_state_t tPipeline;
};

struct xge_mesh_t {
	int iVertexCount;
	int iIndexCount;
	uint32_t iFlags;
	uint32_t iVertexBufferId;
	uint32_t iIndexBufferId;
	void* pVertices;
	void* pIndices;
};

struct xge_shape_batch_t {
	uint32_t iColor;
	uint32_t iFlags;
	int iTriangleCount;
	int iTriangleCapacity;
	void* pVertices;
};

struct xge_texture_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iRefCount;
	uint32_t iFlags;
	uint32_t iBackendId;
	xge_sampler_t tSampler;
	void* pBackend;
};

struct xge_render_target_t {
	int iWidth;
	int iHeight;
	uint32_t iFlags;
	uint32_t iFramebufferId;
	xge_texture_t tTexture;
};

struct xge_pass_t {
	xge_render_target pTarget;
	uint32_t iClearFlags;
	uint32_t iClearColor;
	int iPrevWidth;
	int iPrevHeight;
	uint32_t iPrevFramebufferId;
	xge_camera_t tPrevCamera;
	int bPrevViewportEnabled;
	xge_rect_t tPrevViewportRect;
	int bActive;
};

typedef struct xge_image_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iStride;
	void* pPixels;
	uint32_t iFlags;
} xge_image_t, *xge_image;

typedef struct xge_resource_t {
	void* pData;
	int iSize;
	int iProvider;
	void (*free)(void* pData, void* pUser);
	void* pUser;
} xge_resource_t;

typedef struct xge_resource_provider_t {
	const char* sScheme;
	int (*load)(const char* sURI, void** ppData, int* pSize, void* pUser);
	void (*free)(void* pData, void* pUser);
	void* pUser;
} xge_resource_provider_t;

typedef void* (*xge_xpack_read_proc)(void* pPack, const char* sPath, uint64_t* pSize, void* pUser);
typedef void (*xge_xpack_free_proc)(void* pData, void* pUser);

typedef struct xge_xpack_provider_t {
	void* pPack;
	const char* sRoot;
	xge_xpack_read_proc read;
	xge_xpack_free_proc free;
	void* pUser;
} xge_xpack_provider_t;

typedef struct xge_miniprogram_desc_t {
	void* pCanvas;
	void* pWebGLContext;
	int iWidth;
	int iHeight;
	float fDevicePixelRatio;
	void* pUser;
} xge_miniprogram_desc_t;

typedef struct xge_miniprogram_touch_t {
	uint64_t iId;
	float fX;
	float fY;
	float fForce;
} xge_miniprogram_touch_t;

typedef struct xge_miniprogram_bridge_t {
	int (*request_frame)(void* pUser);
	int (*load_resource)(const char* sURI, void** ppData, int* pSize, void* pUser);
	void (*free_resource)(void* pData, void* pUser);
	int (*audio_command)(int iCommand, int iHandle, const void* pData, int iSize, void* pUser);
	void* pUser;
} xge_miniprogram_bridge_t;

typedef struct xge_egl_desc_t {
	void* pNativeDisplay;
	void* pNativeWindow;
	int iWidth;
	int iHeight;
	int bPBuffer;
	int bSurfaceless;
	int bBoardLinux;
	void* pUser;
} xge_egl_desc_t;

typedef struct xge_egl_caps_t {
	int bCompiled;
	int bOffscreen;
	int bPBuffer;
	int bSurfaceless;
	int bNativeWindow;
	int bBoardLinux;
	int iLastError;
	char sBackendName[64];
	char sLastStage[64];
} xge_egl_caps_t;

typedef struct xge_egl_context_t {
	int bInitialized;
	int bPBuffer;
	int bSurfaceless;
	int bBoardLinux;
	int iWidth;
	int iHeight;
	void* pDisplay;
	void* pConfig;
	void* pSurface;
	void* pContext;
	void* pUser;
	int iLastError;
	char sLastStage[64];
} xge_egl_context_t;

typedef struct xge_offscreen_t {
	int iWidth;
	int iHeight;
	xge_egl_context_t tEGL;
	xge_render_target_t tTarget;
	int bActive;
	int bEGLContext;
	int bFallbackRenderTarget;
} xge_offscreen_t, *xge_offscreen;

typedef struct xge_async_request_t xge_async_request_t;
typedef xge_async_request_t* xge_async_request;
typedef void (*xge_async_proc)(xge_async_request pRequest, void* pUser);

typedef struct xge_sound_t {
	int iRefCount;
	int iType;
	uint32_t iFlags;
	void* pBackend;
} xge_sound_t, *xge_sound;

typedef struct xge_music_t {
	int iRefCount;
	int iType;
	uint32_t iFlags;
	void* pBackend;
} xge_music_t, *xge_music;

typedef struct xge_stream_t {
	int iRefCount;
	int iType;
	uint32_t iFlags;
	void* pBackend;
} xge_stream_t, *xge_stream;

typedef struct xge_audio_group_t {
	float fVolume;
	void* pBackend;
} xge_audio_group_t, *xge_audio_group;

typedef struct xge_audio_listener_t {
	xge_vec3_t tPosition;
	xge_vec3_t tForward;
	xge_vec3_t tUp;
} xge_audio_listener_t;

struct xge_async_request_t {
	int iType;
	int iStatus;
	int iResult;
	float fProgress;
	void* pTarget;
	char* sURI;
	xge_async_proc onComplete;
	void* pUser;
	void* pThread;
	uint32_t iFlags;
	float fSize;
	int bCancel;
	int bThreaded;
	int bCallbackPending;
};

typedef struct xge_font_t xge_font_t;
typedef xge_font_t* xge_font;

typedef struct xge_glyph_metrics_t {
	uint32_t iCodepoint;
	int iGlyph;
	float fAdvanceX;
	float fLeftSideBearing;
	float fX0;
	float fY0;
	float fX1;
	float fY1;
} xge_glyph_metrics_t;

typedef struct xge_glyph_bitmap_t {
	uint32_t iCodepoint;
	int iWidth;
	int iHeight;
	int iStride;
	int iOffsetX;
	int iOffsetY;
	int iFormat;
	void* pPixels;
} xge_glyph_bitmap_t;

typedef struct xge_glyph_t {
	uint32_t iCodepoint;
	int iGlyph;
	int iPage;
	int iX;
	int iY;
	int iWidth;
	int iHeight;
	float fOffsetX;
	float fOffsetY;
	float fAdvanceX;
} xge_glyph_t;

typedef struct xge_glyph_atlas_t {
	int iPageWidth;
	int iPageHeight;
	int iPageCount;
	int iFormat;
	void* pPages;
} xge_glyph_atlas_t;

struct xge_font_t {
	int iRefCount;
	float fSize;
	float fScale;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
	uint32_t iFlags;
	void* pData;
	int iDataSize;
	void* pBackend;
	xge_font pFallback;
	xge_glyph_atlas_t tAtlas;
	void* pGlyphs;
};

typedef struct xge_xrf_header_t {
	uint32_t iMagic;
	uint16_t iVersion;
	uint16_t iHeaderSize;
	uint32_t iFlags;
	uint32_t iGlyphCount;
	uint32_t iPageCount;
	uint32_t iRangeCount;
	uint32_t iKerningCount;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
	uint32_t iGlyphOffset;
	uint32_t iPageOffset;
	uint32_t iRangeOffset;
	uint32_t iKerningOffset;
	uint32_t iPixelOffset;
} xge_xrf_header_t;

typedef struct xge_xrf_range_t {
	uint32_t iFirstCodepoint;
	uint32_t iCount;
} xge_xrf_range_t;

typedef struct xge_xrf_glyph_t {
	uint32_t iCodepoint;
	uint16_t iPage;
	uint16_t iGlyph;
	uint16_t iX;
	uint16_t iY;
	uint16_t iWidth;
	uint16_t iHeight;
	float fOffsetX;
	float fOffsetY;
	float fAdvanceX;
} xge_xrf_glyph_t;

typedef struct xge_xrf_page_t {
	uint16_t iWidth;
	uint16_t iHeight;
	uint16_t iFormat;
	uint16_t iReserved;
	uint32_t iPixelOffset;
	uint32_t iPixelSize;
} xge_xrf_page_t;

typedef struct xge_xrf_kerning_t {
	uint32_t iLeftCodepoint;
	uint32_t iRightCodepoint;
	float fAdvanceX;
} xge_xrf_kerning_t;

typedef struct xge_xui_widget_t xge_xui_widget_t;
typedef xge_xui_widget_t* xge_xui_widget;

typedef struct xge_event_t {
	int iType;
	int iParam1;
	int iParam2;
	float fX;
	float fY;
	float fDX;
	float fDY;
	uint64_t iPointerId;
	uint32_t iCodepoint;
	double fTime;
	int iXuiPhase;
	int bXuiCaptured;
	xge_xui_widget pXuiOriginalTarget;
	xge_xui_widget pXuiCurrentTarget;
	xge_xui_widget pXuiCapture;
	void* pData;
} xge_event_t;

typedef struct xge_scene_t xge_scene_t;
typedef xge_scene_t* xge_scene;
typedef int (*xge_scene_lifecycle_proc)(xge_scene pScene);
typedef int (*xge_scene_event_proc)(xge_scene pScene, const xge_event_t* pEvent);
typedef int (*xge_scene_update_proc)(xge_scene pScene, float fDelta);
typedef int (*xge_scene_draw_proc)(xge_scene pScene);

struct xge_scene_t {
	void* pUser;
	xge_scene_lifecycle_proc onEnter;
	xge_scene_lifecycle_proc onLeave;
	xge_scene_lifecycle_proc onPause;
	xge_scene_lifecycle_proc onResume;
	xge_scene_event_proc onEvent;
	xge_scene_update_proc onUpdate;
	xge_scene_draw_proc onDraw;
	xge_scene_lifecycle_proc onFree;
	int bEntered;
	int bPaused;
};

typedef struct xge_ime_event_t {
	const char* sText;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_ime_event_t;

typedef struct xge_touch_point_t {
	uint64_t iId;
	float fX;
	float fY;
	float fDX;
	float fDY;
	int iPhase;
	int bChanged;
	int bDown;
} xge_touch_point_t;

typedef struct xge_touch_event_t {
	int iPhase;
	int iCount;
	xge_touch_point_t arrPoints[XGE_TOUCH_MAX];
} xge_touch_event_t;

typedef struct xge_gamepad_state_t {
	int bConnected;
	uint32_t iButtons;
	uint32_t iButtonsPressed;
	uint32_t iButtonsReleased;
	float arrAxes[XGE_GAMEPAD_AXIS_COUNT];
} xge_gamepad_state_t;

typedef void* xui_texture;
typedef void* xui_font;

typedef struct xge_xui_size_t {
	float fValue;
	int iUnit;
} xge_xui_size_t;

typedef struct xge_xui_edges_t {
	xge_xui_size_t tLeft;
	xge_xui_size_t tTop;
	xge_xui_size_t tRight;
	xge_xui_size_t tBottom;
} xge_xui_edges_t;

typedef struct xge_xui_paint_command_t {
	int iType;
	xge_rect_t tRect;
	xge_draw_t tDraw;
	xui_font pFont;
	const char* sText;
	uint32_t iColor;
	uint32_t iFlags;
	float fRadius;
	void* pUser;
} xge_xui_paint_command_t;

typedef struct xge_xui_style_t {
	int iLayout;
	int iDock;
	xge_xui_size_t tWidth;
	xge_xui_size_t tHeight;
	xge_xui_size_t tMinWidth;
	xge_xui_size_t tMinHeight;
	xge_xui_size_t tMaxWidth;
	xge_xui_size_t tMaxHeight;
	xge_xui_edges_t tMargin;
	xge_xui_edges_t tPadding;
	xge_xui_edges_t tAnchor;
	int iGridColumns;
	int iGridColumnSpan;
	float fGridRowHeight;
	float fGridColumnGap;
	float fGridRowGap;
	float fGap;
	int iAlignX;
	int iAlignY;
	int iJustify;
	int iLayer;
	int iZ;
	int iAnchor;
	int iClip;
	int iOverflow;
	uint32_t iBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iFocusRingColor;
	uint32_t iDisabledOverlayColor;
	uint32_t iDebugOutlineColor;
	float fRadius;
	float fBorderWidth;
	float fFocusRingWidth;
	float fDebugOutlineWidth;
} xge_xui_style_t;

typedef struct xge_xui_theme_t {
	xui_font pFont;
	uint32_t iTextColor;
	uint32_t iBackgroundColor;
	uint32_t iPanelColor;
	uint32_t iBorderColor;
	uint32_t iAccentColor;
	uint32_t iSelectionColor;
	uint32_t iStateNormal;
	uint32_t iStateHover;
	uint32_t iStateActive;
	uint32_t iStateFocus;
	uint32_t iStateDisabled;
	float fRadius;
	float fPadding;
	float fSpacing;
	float fBorderWidth;
} xge_xui_theme_t, *xge_xui_theme;

typedef struct xge_xui_state_style_t {
	uint32_t iMask;
	uint32_t iBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iFocusRingColor;
	uint32_t iDisabledOverlayColor;
	float fBorderWidth;
	float fFocusRingWidth;
} xge_xui_state_style_t;

typedef struct xge_xui_context_t xge_xui_context_t;
typedef xge_xui_context_t* xge_xui_context;
typedef struct xge_xui_button_t xge_xui_button_t;
typedef xge_xui_button_t* xge_xui_button;
typedef struct xge_xui_label_t xge_xui_label_t;
typedef xge_xui_label_t* xge_xui_label;
typedef struct xge_xui_image_t xge_xui_image_t;
typedef xge_xui_image_t* xge_xui_image;
typedef struct xge_xui_input_t xge_xui_input_t;
typedef xge_xui_input_t* xge_xui_input;
typedef struct xge_xui_input_decoration_t xge_xui_input_decoration_t;
typedef xge_xui_input_decoration_t* xge_xui_input_decoration;
typedef struct xge_xui_color_picker_t xge_xui_color_picker_t;
typedef xge_xui_color_picker_t* xge_xui_color_picker;
typedef struct xge_xui_date_picker_t xge_xui_date_picker_t;
typedef xge_xui_date_picker_t* xge_xui_date_picker;
typedef struct xge_xui_numeric_input_t xge_xui_numeric_input_t;
typedef xge_xui_numeric_input_t* xge_xui_numeric_input;
typedef struct xge_xui_text_edit_t xge_xui_text_edit_t;
typedef xge_xui_text_edit_t* xge_xui_text_edit;
typedef struct xge_xui_checkbox_t xge_xui_checkbox_t;
typedef xge_xui_checkbox_t* xge_xui_checkbox;
typedef struct xge_xui_radio_group_t xge_xui_radio_group_t;
typedef xge_xui_radio_group_t* xge_xui_radio_group;
typedef struct xge_xui_radio_t xge_xui_radio_t;
typedef xge_xui_radio_t* xge_xui_radio;
typedef struct xge_xui_toggle_t xge_xui_toggle_t;
typedef xge_xui_toggle_t* xge_xui_toggle;
typedef struct xge_xui_separator_t xge_xui_separator_t;
typedef xge_xui_separator_t* xge_xui_separator;
typedef struct xge_xui_toolbar_item_t xge_xui_toolbar_item_t;
typedef struct xge_xui_toolbar_t xge_xui_toolbar_t;
typedef xge_xui_toolbar_t* xge_xui_toolbar;
typedef struct xge_xui_menubar_item_t xge_xui_menubar_item_t;
typedef struct xge_xui_menubar_t xge_xui_menubar_t;
typedef xge_xui_menubar_t* xge_xui_menubar;
typedef struct xge_xui_status_bar_item_t xge_xui_status_bar_item_t;
typedef struct xge_xui_status_bar_t xge_xui_status_bar_t;
typedef xge_xui_status_bar_t* xge_xui_status_bar;
typedef struct xge_xui_tabs_t xge_xui_tabs_t;
typedef xge_xui_tabs_t* xge_xui_tabs;
typedef struct xge_xui_scrollbar_t xge_xui_scrollbar_t;
typedef xge_xui_scrollbar_t* xge_xui_scrollbar;
typedef struct xge_xui_slider_t xge_xui_slider_t;
typedef xge_xui_slider_t* xge_xui_slider;
typedef struct xge_xui_progress_t xge_xui_progress_t;
typedef xge_xui_progress_t* xge_xui_progress;
typedef struct xge_xui_pager_item_t xge_xui_pager_item_t;
typedef struct xge_xui_pager_t xge_xui_pager_t;
typedef xge_xui_pager_t* xge_xui_pager;
typedef struct xge_xui_window_t xge_xui_window_t;
typedef xge_xui_window_t* xge_xui_window;
typedef struct xge_xui_panel_t xge_xui_panel_t;
typedef xge_xui_panel_t* xge_xui_panel;
typedef struct xge_xui_scroll_model_t xge_xui_scroll_model_t;
typedef xge_xui_scroll_model_t* xge_xui_scroll_model;
typedef struct xge_xui_scroll_frame_t xge_xui_scroll_frame_t;
typedef xge_xui_scroll_frame_t* xge_xui_scroll_frame;
typedef struct xge_xui_scroll_view_t xge_xui_scroll_view_t;
typedef xge_xui_scroll_view_t* xge_xui_scroll_view;
typedef struct xge_xui_list_view_t xge_xui_list_view_t;
typedef xge_xui_list_view_t* xge_xui_list_view;
typedef struct xge_xui_tree_view_node_t xge_xui_tree_view_node_t;
typedef struct xge_xui_tree_view_t xge_xui_tree_view_t;
typedef xge_xui_tree_view_t* xge_xui_tree_view;
typedef struct xge_xui_table_view_column_t xge_xui_table_view_column_t;
typedef struct xge_xui_table_view_row_t xge_xui_table_view_row_t;
typedef struct xge_xui_table_view_cell_t xge_xui_table_view_cell_t;
typedef struct xge_xui_table_view_colors_t xge_xui_table_view_colors_t;
typedef struct xge_xui_table_view_t xge_xui_table_view_t;
typedef xge_xui_table_view_t* xge_xui_table_view;
typedef struct xge_xui_table_grid_t xge_xui_table_grid_t;
typedef xge_xui_table_grid_t* xge_xui_table_grid;
typedef struct xge_xui_timeline_frame_t xge_xui_timeline_frame_t;
typedef struct xge_xui_timeline_span_t xge_xui_timeline_span_t;
typedef struct xge_xui_timeline_layer_t xge_xui_timeline_layer_t;
typedef struct xge_xui_timeline_selection_t xge_xui_timeline_selection_t;
typedef struct xge_xui_timeline_hit_t xge_xui_timeline_hit_t;
typedef struct xge_xui_timeline_view_t xge_xui_timeline_view_t;
typedef xge_xui_timeline_view_t* xge_xui_timeline_view;
typedef struct xge_xui_property_desc_t xge_xui_property_desc_t;
typedef struct xge_xui_property_grid_style_t xge_xui_property_grid_style_t;
typedef struct xge_xui_property_grid_category_t xge_xui_property_grid_category_t;
typedef struct xge_xui_property_grid_property_t xge_xui_property_grid_property_t;
typedef struct xge_xui_property_grid_t xge_xui_property_grid_t;
typedef xge_xui_property_grid_t* xge_xui_property_grid;
typedef struct xge_xui_accordion_section_t xge_xui_accordion_section_t;
typedef struct xge_xui_accordion_t xge_xui_accordion_t;
typedef xge_xui_accordion_t* xge_xui_accordion;
typedef struct xge_xui_toast_item_t xge_xui_toast_item_t;
typedef struct xge_xui_toast_t xge_xui_toast_t;
typedef xge_xui_toast_t* xge_xui_toast;
typedef struct xge_xui_msg_tip_t xge_xui_msg_tip_t;
typedef xge_xui_msg_tip_t* xge_xui_msg_tip;
typedef struct xge_xui_msg_box_t xge_xui_msg_box_t;
typedef xge_xui_msg_box_t* xge_xui_msg_box;
typedef struct xge_xui_input_box_t xge_xui_input_box_t;
typedef xge_xui_input_box_t* xge_xui_input_box;
typedef struct xge_xui_popup_t xge_xui_popup_t;
typedef xge_xui_popup_t* xge_xui_popup;
typedef struct xge_xui_combo_box_t xge_xui_combo_box_t;
typedef xge_xui_combo_box_t* xge_xui_combo_box;
typedef struct xge_xui_combo_box_item_t xge_xui_combo_box_item_t;
typedef xge_xui_combo_box_item_t* xge_xui_combo_box_item;
typedef struct xge_xui_menu_t xge_xui_menu_t;
typedef xge_xui_menu_t* xge_xui_menu;
typedef struct xge_xui_split_layout_t xge_xui_split_layout_t;
typedef xge_xui_split_layout_t* xge_xui_split_layout;
typedef struct xge_xui_dock_layout_t xge_xui_dock_layout_t;
typedef xge_xui_dock_layout_t* xge_xui_dock_layout;
typedef struct xge_xui_dock_region_t xge_xui_dock_region_t;
typedef xge_xui_dock_region_t* xge_xui_dock_region;
typedef struct xge_xui_dock_node_t xge_xui_dock_node_t;
typedef xge_xui_dock_node_t* xge_xui_dock_node;
typedef struct xge_xui_dock_pane_t xge_xui_dock_pane_t;
typedef xge_xui_dock_pane_t* xge_xui_dock_pane;
typedef struct xge_xui_dock_window_t xge_xui_dock_window_t;
typedef xge_xui_dock_window_t* xge_xui_dock_window;
typedef struct xge_xui_page_t xge_xui_page_t;
typedef xge_xui_page_t* xge_xui_page;
typedef struct xge_xui_page_index_t xge_xui_page_index_t;
typedef struct xge_xui_page_click_binding_t xge_xui_page_click_binding_t;
typedef struct xge_xui_loader_t xge_xui_loader_t;
typedef xge_xui_loader_t* xge_xui_loader;
typedef struct xge_xui_binder_t xge_xui_binder_t;
typedef xge_xui_binder_t* xge_xui_binder;
typedef struct xge_xui_model_t xge_xui_model_t;
typedef xge_xui_model_t* xge_xui_model;

typedef struct xge_xui_host_t {
	void (*draw_rect)(xge_rect_t tRect, uint32_t iColor, void* pUser);
	void (*draw_image)(const xge_draw_t* pDraw, void* pUser);
	void (*draw_text_rect)(xui_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser);
	xge_vec2_t (*measure_text)(xui_font pFont, const char* sText, void* pUser);
	void (*clip_set)(xge_rect_t tRect, void* pUser);
	void (*clip_clear)(void* pUser);
	void (*request_refresh)(void* pUser);
	void* pUser;
} xge_xui_host_t, *xge_xui_host;

typedef struct xui_font_metrics_t {
	float fSize;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
} xui_font_metrics_t;

typedef struct xui_texture_desc_t {
	int iWidth;
	int iHeight;
	int iFormat;
	uint32_t iFlags;
} xui_texture_desc_t;

typedef struct xge_xui_host_v2_t {
	uint32_t iSize;
	uint32_t iVersion;
	void* pUser;
	void (*draw_rect)(xge_rect_t tRect, uint32_t iColor, void* pUser);
	void (*draw_border_rect)(xge_rect_t tRect, float fWidth, uint32_t iColor, void* pUser);
	void (*draw_rounded_rect)(xge_rect_t tRect, uint32_t iColor, float fRadius, void* pUser);
	void (*draw_line)(float fX1, float fY1, float fX2, float fY2, float fWidth, uint32_t iColor, void* pUser);
	void (*draw_triangle)(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor, void* pUser);
	void (*draw_circle)(float fX, float fY, float fRadius, uint32_t iColor, void* pUser);
	void (*draw_circle_stroke)(float fX, float fY, float fRadius, float fWidth, uint32_t iColor, void* pUser);
	void (*draw_texture)(xui_texture pTexture, xge_rect_t tSrc, xge_rect_t tDst, uint32_t iColor, uint32_t iFlags, void* pUser);
	void (*draw_text)(xui_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser);
	xge_vec2_t (*measure_text)(xui_font pFont, const char* sText, uint32_t iFlags, void* pUser);
	int (*font_metrics)(xui_font pFont, xui_font_metrics_t* pMetrics, void* pUser);
	void (*clip_push)(xge_rect_t tRect, void* pUser);
	void (*clip_clear)(void* pUser);
	int (*clip_get)(xge_rect_t* pRect, int* pEnabled, void* pUser);
	int (*texture_create_rgba)(xui_texture* pTexture, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags, void* pUser);
	int (*texture_create_memory)(xui_texture* pTexture, const void* pData, int iSize, uint32_t iFlags, void* pUser);
	int (*texture_create_file)(xui_texture* pTexture, const char* sPath, uint32_t iFlags, void* pUser);
	int (*texture_get_desc)(xui_texture pTexture, xui_texture_desc_t* pDesc, void* pUser);
	void (*texture_destroy)(xui_texture pTexture, void* pUser);
	int (*font_create_file)(xui_font* pFont, const char* sPath, float fSize, uint32_t iFlags, void* pUser);
	int (*font_create_memory)(xui_font* pFont, const void* pData, int iSize, float fSize, uint32_t iFlags, void* pUser);
	void (*font_destroy)(xui_font pFont, void* pUser);
	void (*clipboard_set_text)(const char* sText, void* pUser);
	const char* (*clipboard_get_text)(void* pUser);
	double (*get_time)(void* pUser);
	void (*get_viewport_size)(float* pWidth, float* pHeight, void* pUser);
	void (*flush)(void* pUser);
	void (*request_refresh)(void* pUser);
} xge_xui_host_v2_t, *xge_xui_host_v2;

typedef struct xge_xui_text_t {
	char* sText;
	char* sComposition;
	int iSize;
	int iCapacity;
	int iCompositionSize;
	int iCompositionCapacity;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_xui_text_t, *xge_xui_text;

typedef int (*xge_xui_event_proc)(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
typedef void (*xge_xui_update_proc)(xge_xui_widget pWidget, float fDelta, void* pUser);
typedef xge_vec2_t (*xge_xui_measure_proc)(xge_xui_widget pWidget, void* pUser);
typedef void (*xge_xui_layout_proc)(xge_xui_widget pWidget, void* pUser);
typedef void (*xge_xui_paint_proc)(xge_xui_widget pWidget, void* pUser);
typedef void (*xge_xui_click_proc)(xge_xui_widget pWidget, void* pUser);

struct xge_xui_popup_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pOwner;
	xge_xui_widget pScrollWidget;
	xge_xui_widget pContentWidget;
	xge_xui_widget pFocusRestore;
	xge_xui_widget pFocusWidget;
	xge_xui_scroll_view pScrollView;
	xge_rect_t tAnchorRect;
	xge_rect_t tViewportRect;
	xge_rect_t tContentRect;
	float fOffsetX;
	float fOffsetY;
	float fContentW;
	float fContentH;
	float fGap;
	xge_xui_click_proc procClose;
	void* pUser;
	int iPlacement;
	int iAnchorPoint;
	int iDirection;
	int iOutsidePolicy;
	int iOwnerPolicy;
	int iEscapePolicy;
	int iFocusPolicy;
	int bOpen;
	int bModal;
	int bAnchorRectSet;
	int bCloseOnOutside;
	int bCloseOnEscape;
	int bMatchOwnerWidth;
	int bConsumeInside;
	int bFocusRestoreExplicit;
	int iCloseCount;
};

typedef void (*xge_xui_checked_proc)(xge_xui_widget pWidget, int bChecked, void* pUser);
typedef void (*xge_xui_slider_proc)(xge_xui_widget pWidget, float fValue, void* pUser);
typedef void (*xge_xui_select_proc)(xge_xui_widget pWidget, int iIndex, void* pUser);
typedef void (*xge_xui_pager_change_proc)(xge_xui_widget pWidget, int iOldPage, int iNewPage, void* pUser);
typedef void (*xge_xui_menu_select_proc)(xge_xui_widget pOwner, int iIndex, int iValue, void* pUser);
typedef int (*xge_xui_list_view_item_proc)(xge_xui_widget pWidget, int iIndex, xge_rect_t tRect, int iState, void* pUser);
typedef void (*xge_xui_scroll_frame_change_proc)(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser);
typedef void (*xge_xui_text_submit_proc)(xge_xui_widget pWidget, const char* sText, void* pUser);
typedef int (*xge_xui_input_filter_proc)(xge_xui_widget pWidget, const char* sOldText, const char* sNewText, void* pUser);
typedef void (*xge_xui_input_error_proc)(xge_xui_widget pWidget, int bError, void* pUser);
typedef void (*xge_xui_input_decoration_paint_proc)(xge_xui_input pInput, xge_xui_input_decoration pDecoration, xge_rect_t tRect, int iState, void* pUser);
typedef void (*xge_xui_color_proc)(xge_xui_widget pWidget, uint32_t iColor, void* pUser);
typedef void (*xge_xui_date_proc)(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser);
typedef void (*xge_xui_toast_click_proc)(xge_xui_context pContext, int iToastId, void* pUser);
typedef void (*xge_xui_toast_close_proc)(xge_xui_context pContext, int iToastId, int iReason, void* pUser);
typedef int (*xge_xui_numeric_format_proc)(float fValue, char* sBuffer, int iSize, void* pUser);
typedef xge_rect_t (*xge_xui_ime_candidate_rect_proc)(xge_xui_widget pWidget, void* pUser);
typedef int (*xge_xui_tree_view_count_proc)(xge_xui_widget pWidget, void* pUser);
typedef int (*xge_xui_tree_view_node_proc)(xge_xui_widget pWidget, int iIndex, xge_xui_tree_view_node_t* pNode, void* pUser);
typedef int (*xge_xui_tree_view_item_proc)(xge_xui_widget pWidget, int iNodeId, int iVisible, const xge_xui_tree_view_node_t* pNode, xge_rect_t tRect, int iState, void* pUser);
typedef int (*xge_xui_table_view_count_proc)(xge_xui_widget pWidget, void* pUser);
typedef int (*xge_xui_table_view_cell_proc)(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser);
typedef int (*xge_xui_table_view_format_proc)(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, char* sBuffer, int iSize, void* pUser);
typedef int (*xge_xui_table_view_cell_renderer_proc)(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser);
typedef int (*xge_xui_table_view_header_renderer_proc)(xge_xui_widget pWidget, int iColumn, const xge_xui_table_view_column_t* pColumn, xge_rect_t tRect, int iState, void* pUser);
typedef void (*xge_xui_table_view_sort_proc)(xge_xui_widget pWidget, int iColumn, int bDescending, void* pUser);
typedef void (*xge_xui_table_view_select_proc)(xge_xui_widget pWidget, int iRow, int iColumn, int iSelectionMode, void* pUser);
typedef void (*xge_xui_table_view_column_resize_proc)(xge_xui_widget pWidget, int iColumn, float fWidth, void* pUser);
typedef void (*xge_xui_table_view_hover_proc)(xge_xui_widget pWidget, int iRow, int iColumn, int iSelectionMode, void* pUser);
typedef int (*xge_xui_table_view_merge_proc)(xge_xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser);
typedef struct xge_xui_table_grid_editor_config_t {
	const char** arrEnumItems;
	const xge_xui_combo_box_item_t* arrEnumItemData;
	const int* arrEnumEnabled;
	int iEnumItemCount;
	int iEnumSelected;
	int bEnumUseValue;
	int iEnumSelectedValue;
	const uint32_t* arrPalette;
	int iPaletteCount;
	int bAlphaEnabled;
	int bShowSecond;
	int bDateModeSet;
	int iDateMode;
	int bDateHasMin;
	int bDateHasMax;
	xtime tDateMin;
	xtime tDateMax;
	xtime tDefaultRangeSpan;
	const char* sDateFormat;
	const char* sRangeSeparator;
	void* pUser;
} xge_xui_table_grid_editor_config_t;
typedef void (*xge_xui_table_grid_set_proc)(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser);
typedef int (*xge_xui_table_grid_validate_proc)(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser);
typedef void (*xge_xui_table_grid_change_proc)(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser);
typedef int (*xge_xui_table_grid_editor_proc)(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, void* pUser);
typedef int (*xge_xui_table_grid_editor_config_proc)(xge_xui_widget pWidget, int iRow, int iColumn, int iType, xge_xui_table_grid_editor_config_t* pConfig, void* pUser);
typedef int (*xge_xui_timeline_current_frame_changing_proc)(xge_xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser);
typedef void (*xge_xui_timeline_current_frame_changed_proc)(xge_xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser);
typedef int (*xge_xui_timeline_layer_changing_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iChangeType, const char* sOldText, const char* sNewText, int iOldValue, int iNewValue, void* pUser);
typedef void (*xge_xui_timeline_layer_changed_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iChangeType, void* pUser);
typedef int (*xge_xui_timeline_frame_changing_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iFrame, int iOldType, int iNewType, void* pUser);
typedef void (*xge_xui_timeline_frame_changed_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iFrame, int iOldType, int iNewType, void* pUser);
typedef int (*xge_xui_timeline_span_changing_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iSpanId, int iOldStartFrame, int iOldEndFrame, int iNewStartFrame, int iNewEndFrame, int iOldType, int iNewType, const char* sOldLabel, const char* sNewLabel, void* pUser);
typedef void (*xge_xui_timeline_span_changed_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iSpanId, int iOldStartFrame, int iOldEndFrame, int iNewStartFrame, int iNewEndFrame, int iOldType, int iNewType, void* pUser);
typedef void (*xge_xui_timeline_layer_selected_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, void* pUser);
typedef int (*xge_xui_timeline_context_menu_opening_proc)(xge_xui_widget pWidget, const xge_xui_timeline_hit_t* pHit, void* pUser);
typedef void (*xge_xui_timeline_context_menu_command_proc)(xge_xui_widget pWidget, const xge_xui_timeline_hit_t* pHit, int iCommand, void* pUser);
typedef void (*xge_xui_timeline_frame_click_proc)(xge_xui_widget pWidget, int iLayer, int iLayerId, int iFrame, int iModifiers, void* pUser);
typedef void (*xge_xui_timeline_selection_proc)(xge_xui_widget pWidget, int iSelectionCount, void* pUser);
typedef int (*xge_xui_timeline_layer_renderer_proc)(xge_xui_widget pWidget, int iLayer, const xge_xui_timeline_layer_t* pLayer, xge_rect_t tRect, int iState, void* pUser);
typedef int (*xge_xui_timeline_ruler_renderer_proc)(xge_xui_widget pWidget, int iFrame, xge_rect_t tRect, int iState, void* pUser);
typedef int (*xge_xui_timeline_frame_renderer_proc)(xge_xui_widget pWidget, int iLayer, int iFrame, const xge_xui_timeline_frame_t* pFrame, xge_rect_t tRect, int iState, void* pUser);
typedef int (*xge_xui_timeline_span_renderer_proc)(xge_xui_widget pWidget, int iLayer, const xge_xui_timeline_span_t* pSpan, xge_rect_t tRect, int iState, void* pUser);
typedef void (*xge_xui_property_grid_select_proc)(xge_xui_widget pWidget, int iProperty, const char* sId, void* pUser);
typedef int (*xge_xui_property_grid_validate_proc)(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser);
typedef void (*xge_xui_property_grid_change_proc)(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser);
typedef int (*xge_xui_property_grid_action_proc)(xge_xui_widget pWidget, int iProperty, const char* sId, xge_rect_t tRect, void* pUser);
typedef int (*xge_xui_property_grid_render_proc)(xge_xui_widget pWidget, int iProperty, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser);
typedef void (*xge_xui_split_layout_change_proc)(xge_xui_widget pWidget, int iDivider, void* pUser);
typedef xge_vec2_t (*xge_xui_tooltip_measure_proc)(xge_xui_context pContext, xge_xui_widget pOwner, void* pUser);
typedef void (*xge_xui_tooltip_paint_proc)(xge_xui_context pContext, xge_xui_widget pOwner, xge_rect_t tRect, void* pUser);

struct xge_xui_toast_item_t {
	char* sTitle;
	char* sMessage;
	int iId;
	int iType;
	float fDuration;
	float fElapsed;
	float fHeight;
	xge_rect_t tRect;
	xge_rect_t tCloseRect;
	xge_rect_t tTitleRect;
	xge_rect_t tMessageRect;
	xge_xui_toast_click_proc procClick;
	void* pUser;
	int bClosing;
	int iCloseReason;
};

struct xge_xui_toast_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget arrHitWidget[XGE_XUI_TOAST_VISIBLE_CAPACITY];
	xui_font pFont;
	xge_xui_toast_item_t arrActive[XGE_XUI_TOAST_VISIBLE_CAPACITY];
	int iActiveCount;
	xge_xui_toast_item_t arrPending[XGE_XUI_TOAST_QUEUE_CAPACITY];
	int iPendingCount;
	int iNextId;
	int iPlacement;
	int iDirection;
	int iMaxVisible;
	float fWidth;
	float fMargin;
	float fGap;
	float fMinHeight;
	xge_xui_style_t tItemStyle;
	uint32_t iTextColor;
	uint32_t iMutedTextColor;
	uint32_t iInfoColor;
	uint32_t iSuccessColor;
	uint32_t iWarningColor;
	uint32_t iErrorColor;
	uint32_t iCloseColor;
	uint32_t iCloseHoverColor;
	int iHoverItem;
	int iHoverClose;
	xge_xui_toast_close_proc procClose;
	void* pCloseUser;
	int bDefaultsReady;
	int iShowCount;
	int iCloseCount;
	int iExpireCount;
	int iDropCount;
};

typedef struct xge_xui_menu_item_t {
	const char* sText;
	const char* sShortcut;
	int iType;
	int iState;
	int iValue;
	int iIcon;
	xge_xui_menu pSubmenu;
	void* pUser;
} xge_xui_menu_item_t, *xge_xui_menu_item;

typedef struct xge_xui_menu_metrics_t {
	float fItemHeight;
	float fSeparatorHeight;
	float fPaddingX;
	float fPaddingY;
	float fMarkWidth;
	float fIconWidth;
	float fShortcutGap;
	float fArrowWidth;
	float fMinWidth;
	float fMaxHeight;
} xge_xui_menu_metrics_t, *xge_xui_menu_metrics;

typedef struct xge_xui_menu_colors_t {
	uint32_t iPanel;
	uint32_t iBorder;
	uint32_t iRow;
	uint32_t iHover;
	uint32_t iText;
	uint32_t iDisabledText;
	uint32_t iShortcutText;
	uint32_t iDangerText;
	uint32_t iMark;
	uint32_t iSeparator;
} xge_xui_menu_colors_t, *xge_xui_menu_colors;

typedef struct xge_xui_bar_metrics_t {
	float fHeight;
	float fPaddingX;
	float fPaddingY;
	float fItemPaddingX;
	float fItemGap;
	float fGroupGap;
	float fSeparatorSize;
	float fOverflowSize;
} xge_xui_bar_metrics_t, *xge_xui_bar_metrics;

typedef struct xge_xui_bar_colors_t {
	uint32_t iBackground;
	uint32_t iBorder;
	uint32_t iItem;
	uint32_t iHover;
	uint32_t iActive;
	uint32_t iChecked;
	uint32_t iDisabled;
	uint32_t iText;
	uint32_t iDisabledText;
	uint32_t iSeparator;
	uint32_t iAccent;
} xge_xui_bar_colors_t, *xge_xui_bar_colors;

typedef struct xge_xui_window_frame_colors_t {
	uint32_t iFrameBackground;
	uint32_t iClientBackground;
	uint32_t iTitleBackground;
	uint32_t iTitleText;
	uint32_t iBorder;
	uint32_t iButtonNormal;
	uint32_t iButtonHover;
	uint32_t iButtonActive;
} xge_xui_window_frame_colors_t, *xge_xui_window_frame_colors;

typedef struct xge_xui_chrome_style_t {
	xge_xui_window_frame_colors_t tWindow;
	xge_xui_bar_metrics_t tMenuBarMetrics;
	xge_xui_bar_metrics_t tToolbarMetrics;
	xge_xui_bar_metrics_t tStatusBarMetrics;
	xge_xui_bar_colors_t tBarColors;
	xge_xui_menu_metrics_t tMenuMetrics;
	xge_xui_menu_colors_t tMenuColors;
} xge_xui_chrome_style_t, *xge_xui_chrome_style;

typedef struct xge_xui_tooltip_desc_t {
	int iType;
	const char* sText;
	int iAnchor;
	float fOffsetX;
	float fOffsetY;
	float fDelay;
	int bFollowCursor;
	xge_xui_tooltip_measure_proc procMeasure;
	xge_xui_tooltip_paint_proc procPaint;
	void* pUser;
} xge_xui_tooltip_desc_t, *xge_xui_tooltip_desc;
typedef int (*xge_xui_tooltip_resolve_proc)(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser);

typedef struct xge_xui_hotkey_t {
	xge_xui_widget pWidget;
	int iKey;
	int iModifiers;
	xge_xui_event_proc procEvent;
	void* pUser;
	int iCommand;
	const char* sCommand;
	void* pCommandData;
} xge_xui_hotkey_t, *xge_xui_hotkey;

typedef struct xge_xui_command_t {
	int iCommand;
	const char* sCommand;
	xge_xui_widget pSource;
	void* pData;
} xge_xui_command_t, *xge_xui_command;

typedef struct xge_xui_paint_info_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	int iRole;
	int iState;
	int iOwnerDrawMode;
	int iPart;
	xge_rect_t tOuterRect;
	xge_rect_t tBorderRect;
	xge_rect_t tPaddingRect;
	xge_rect_t tContentRect;
	const xge_xui_style_t* pStyle;
	float fDipScale;
	void* pControl;
	void* pItemData;
	int iItemIndex;
	int iRow;
	int iColumn;
} xge_xui_paint_info_t, *xge_xui_paint_info;

typedef void (*xge_xui_owner_draw_proc)(const xge_xui_paint_info_t* pInfo, void* pUser);

struct xge_xui_widget_t {
	xge_xui_widget pParent;
	xge_xui_widget pFirstChild;
	xge_xui_widget pLastChild;
	xge_xui_widget pPrevSibling;
	xge_xui_widget pNextSibling;
	int iId;
	int iRole;
	uint32_t iTreeOrder;
	int iTabIndex;
	int iImeMode;
	const char* sName;
	xge_xui_style_t tStyle;
	xge_xui_state_style_t arrStateStyle[XGE_XUI_WIDGET_STATE_STYLE_COUNT];
	int iVisualState;
	xge_rect_t tLocalRect;
	xge_rect_t tRect;
	xge_rect_t tOuterRect;
	xge_rect_t tBorderRect;
	xge_rect_t tPaddingRect;
	xge_rect_t tContentRect;
	xge_vec2_t tDesiredSize;
	uint32_t iFlags;
	uint32_t iCallbackFlags;
	void* pUser;
	xge_xui_event_proc procCaptureEvent;
	void* pCaptureUser;
	xge_xui_event_proc procEvent;
	void* pEventUser;
	xge_xui_event_proc arrEventProc[XGE_XUI_WIDGET_EVENT_SLOT_COUNT];
	void* arrEventUser[XGE_XUI_WIDGET_EVENT_SLOT_COUNT];
	uint32_t iEventMask;
	uint32_t iSubtreeEventMask;
	xge_xui_update_proc procUpdate;
	void* pUpdateUser;
	xge_xui_measure_proc procMeasure;
	void* pMeasureUser;
	xge_xui_layout_proc procLayout;
	void* pLayoutUser;
	xge_xui_paint_proc procPaintBefore;
	xge_xui_paint_proc procPaint;
	xge_xui_paint_proc procPaintAfter;
	xge_xui_owner_draw_proc procOwnerDraw;
	void* pPaintBeforeUser;
	void* pPaintUser;
	void* pPaintAfterUser;
	void* pOwnerDrawUser;
	void* pOwnerDrawControl;
	int iOwnerDrawMode;
	void* pInternal;
	xge_xui_widget pOverlayOwner;
	xge_xui_tooltip_desc_t tTooltip;
	xge_xui_tooltip_resolve_proc procTooltipResolve;
	void* pTooltipUser;
	xge_xui_click_proc procDefaultAction;
	void* pDefaultActionUser;
	xge_xui_click_proc procCancelAction;
	void* pCancelActionUser;
	xge_xui_ime_candidate_rect_proc procImeCandidateRect;
	void* pImeCandidateRectUser;
	uint32_t iStyleVersion;
};

struct xge_xui_context_t {
	int bInitialized;
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;
	xge_xui_window pActiveWindow;
	xge_xui_widget pFocus;
	xge_xui_widget pCapture;
	uint64_t arrPointerCaptureId[XGE_XUI_POINTER_CAPTURE_CAPACITY];
	xge_xui_widget arrPointerCaptureWidget[XGE_XUI_POINTER_CAPTURE_CAPACITY];
	xge_xui_widget pHover;
	xge_xui_widget arrHoverPath[64];
	int iHoverPathCount;
	xge_event_t arrEventQueue[XGE_XUI_EVENT_QUEUE_CAPACITY];
	int iEventHead;
	int iEventTail;
	int iEventCount;
	uint32_t iNextTreeOrder;
	float fDipScale;
	int iLayoutBatchDepth;
	int bLayoutBatchDirtyLayout;
	int bLayoutBatchDirtyPaint;
#if XGE_HAS_DEBUGMODE
	int iDirtyLayoutCount;
	int iDirtyPaintCount;
#endif
	int iPaintCommandCount;
	int iPaintFlushCount;
	int iPaintClipStackCount;
	int bPaintClipBaseEnabled;
	xge_rect_t tPaintClipBaseRect;
	xge_rect_t arrPaintClipStack[XGE_XUI_CLIP_STACK_MAX];
	int bRefreshRequested;
	int bAutoDispatchProcFrameEvents;
	int bContextPressActive;
	int bContextPressMoved;
	int bContextPressFired;
	int bContextRightActive;
	uint64_t iContextPressPointerId;
	int bClickPressActive;
	int bClickPressMoved;
	int iClickPressButton;
	uint64_t iClickPressPointerId;
	int bDragPressActive;
	int bDragActive;
	uint64_t iDragPointerId;
	int bImeManaged;
	int bImeEnabled;
	int bImeEnabledPrev;
	xge_rect_t tImeCandidateRect;
	int bImeCandidateRectValid;
	float fContextPressTime;
	float fContextPressStartX;
	float fContextPressStartY;
	float fContextPressLastX;
	float fContextPressLastY;
	xge_xui_widget pContextPressTarget;
	float fClickPressX;
	float fClickPressY;
	xge_xui_widget pClickPressTarget;
	float fDragStartX;
	float fDragStartY;
	float fDragLastX;
	float fDragLastY;
	xge_xui_widget pDragTarget;
	xge_xui_widget pLastClickTarget;
	int iLastClickButton;
	uint64_t iLastClickPointerId;
	double fLastClickTime;
	float fLastClickX;
	float fLastClickY;
	xge_xui_widget pTooltipOwner;
	xge_xui_widget pTooltipPopupWidget;
	xge_xui_tooltip_desc_t tActiveTooltip;
	xge_rect_t tTooltipRect;
	float fTooltipHoverTime;
	float fTooltipMouseX;
	float fTooltipMouseY;
	int bTooltipOpen;
	xge_xui_toast_t tToast;
	xge_xui_theme_t tTheme;
	xge_xui_chrome_style_t tChromeStyle;
	xui_font pDefaultFont;
	xui_font arrFont[XGE_XUI_FONT_CAPACITY];
	char arrFontName[XGE_XUI_FONT_CAPACITY][XGE_XUI_FONT_NAME_CAPACITY];
	int arrFontOwned[XGE_XUI_FONT_CAPACITY];
	int iFontCount;
	uint32_t iThemeVersion;
	int bDefaultFontReady;
	int iPaintSurfaceDirtyGeneration;
	xge_xui_hotkey_t arrHotKey[XGE_XUI_HOTKEY_CAPACITY];
	int iHotKeyCount;
	void* pRegisteredTokens;
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	void* pUser;
	void* pInternal;
};

typedef struct xge_xui_binder_entry_t {
	const char* sName;
	xge_xui_click_proc procClick;
	void* pUser;
} xge_xui_binder_entry_t;

typedef struct xge_xui_binder_event_entry_t {
	const char* sName;
	xge_xui_event_proc procEvent;
	void* pUser;
} xge_xui_binder_event_entry_t;

struct xge_xui_binder_t {
	xge_xui_binder_entry_t arrClick[XGE_XUI_BINDER_ENTRY_CAPACITY];
	xge_xui_binder_event_entry_t arrEvent[XGE_XUI_BINDER_ENTRY_CAPACITY];
	int iClickCount;
	int iEventCount;
};

typedef struct xge_xui_model_entry_t {
	char sKey[XGE_XUI_MODEL_KEY_CAPACITY];
	char sValue[XGE_XUI_MODEL_VALUE_CAPACITY];
} xge_xui_model_entry_t;

struct xge_xui_model_t {
	uint32_t iVersion;
	xge_xui_model_entry_t arrEntry[XGE_XUI_MODEL_ENTRY_CAPACITY];
	int iEntryCount;
};

struct xge_xui_page_index_t {
	const char* sKey;
	int iNumericId;
	int bNumeric;
	xge_xui_widget pWidget;
};

typedef struct xge_xui_text_edit_state_t {
	char* sText;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_xui_text_edit_state_t;

typedef struct xge_xui_text_edit_visual_line_t {
	int iStart;
	int iEnd;
} xge_xui_text_edit_visual_line_t;

typedef struct xge_xui_text_edit_highlight_t {
	int iStart;
	int iEnd;
} xge_xui_text_edit_highlight_t;

struct xge_xui_text_edit_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_text_t tText;
	xui_font pFont;
	xge_xui_menu pDefaultMenu;
	int* arrLineStarts;
	xge_xui_text_edit_state_t* arrUndo;
	xge_xui_text_edit_state_t* arrRedo;
	xge_xui_text_edit_visual_line_t* arrVisualLines;
	const xge_xui_text_edit_highlight_t* arrFindHighlights;
	int arrDefaultMenuEnabled[5];
	uint32_t iTextColor;
	uint32_t iBackgroundColor;
	uint32_t iFocusColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iDisabledTextColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iDisabledBorderColor;
	uint32_t iCursorColor;
	uint32_t iSelectionColor;
	uint32_t iFindHighlightColor;
	uint32_t iCurrentLineColor;
	uint32_t iLineNumberTextColor;
	uint32_t iLineNumberBackgroundColor;
	uint32_t iScrollbarTrackColor;
	uint32_t iScrollbarBorderColor;
	uint32_t iScrollbarThumbColor;
	float fScrollX;
	float fScrollY;
	float fDragY;
	float fDragScrollY;
	float fLineHeight;
	float fLineNumberWidth;
	float fCursorBlinkTime;
	float fPreferredX;
	float fVisualLayoutWidth;
	double fLastClickTime;
	float fLastClickX;
	float fLastClickY;
	float fPressX;
	float fPressY;
	int iLineCount;
	int iLineCapacity;
	int iUndoCount;
	int iUndoCapacity;
	int iRedoCount;
	int iRedoCapacity;
	int iUndoLimit;
	int iVisualLineCount;
	int iVisualLineCapacity;
	int iFindHighlightCount;
	int iSelectionAnchor;
	int iPressCursor;
	int bPressPending;
	int bPressInsideSelection;
	int bSelecting;
	int bReadonly;
	int bWordWrap;
	int bLineNumbers;
	int bLineLayoutDirty;
	int bVisualLayoutDirty;
	int bCursorVisible;
	int bDraggingThumb;
	int iScrollbarMode;
	int bInitialized;
};

typedef struct xge_xui_page_table_view_adapter_t {
	int iRowCount;
	int iColumnCount;
	xge_xui_table_view_row_t* arrRows;
	char arrCell[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY][XGE_XUI_PAGE_TABLE_VIEW_CELL_CAPACITY];
	char arrCellTooltip[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY][XGE_XUI_PAGE_TABLE_VIEW_CELL_CAPACITY];
	int arrCellRowSpan[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	int arrCellColSpan[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	int arrCellDisabled[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	int arrCellEditing[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	int arrCellInvalid[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	int arrCellDirty[XGE_XUI_PAGE_TABLE_VIEW_ROW_CAPACITY][XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
} xge_xui_page_table_view_adapter_t;

struct xge_xui_page_click_binding_t {
	xge_xui_click_proc procClick;
	void* pUser;
};

typedef struct xge_xui_page_event_binding_t {
	xge_xui_event_proc procEvent;
	void* pUser;
} xge_xui_page_event_binding_t;

typedef struct xge_xui_page_hotkey_binding_t {
	xge_xui_widget pWidget;
	int iKey;
	int iModifiers;
	xge_xui_event_proc procEvent;
	void* pUser;
	int iCommand;
	const char* sCommand;
	void* pCommandData;
} xge_xui_page_hotkey_binding_t;

typedef struct xge_xui_page_model_binding_t {
	xge_xui_widget pWidget;
	void* pControl;
	int iKind;
	int iControlIndex;
	char sKey[XGE_XUI_MODEL_KEY_CAPACITY];
} xge_xui_page_model_binding_t;

struct xge_xui_button_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	const char* sText;
	xge_xui_click_proc procClick;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iTextFlags;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	uint32_t iColorChecked;
	xui_texture pIconTexture;
	xge_rect_t tIconSrc;
	xge_rect_t tIconRect;
	xge_rect_t tTextRect;
	xge_rect_t tContentGroupRect;
	uint32_t iIconColor;
	float fIconSize;
	float fIconGap;
	int iIconPlacement;
	int bSelectable;
	int bSelected;
	int iSemantic;
	xge_nine_patch_t arrPatch[XGE_XUI_WIDGET_STATE_STYLE_COUNT];
	int arrHasPatch[XGE_XUI_WIDGET_STATE_STYLE_COUNT];
	int bBadgeVisible;
	int iBadgeAnchor;
	float fBadgeOffsetX;
	float fBadgeOffsetY;
	float fBadgeSize;
	xui_texture pBadgeTexture;
	xge_rect_t tBadgeSrc;
	xge_rect_t tBadgeRect;
	int iState;
	int iClickCount;
};

struct xge_xui_image_t {
	xge_xui_widget pWidget;
	xui_texture pTexture;
	xge_rect_t tSrc;
	xge_rect_t tDst;
	uint32_t iColor;
	int iMode;
	int iAlignX;
	int iAlignY;
};

struct xge_xui_label_t {
	xge_xui_widget pWidget;
	xui_font pFont;
	const char* sText;
	char* sTextOwned;
	int iTextCapacity;
	uint32_t iColor;
	uint32_t iDisabledColor;
	uint32_t iTextFlags;
	int bUnderline;
	xge_vec2_t tMeasuredSize;
};

struct xge_xui_panel_t {
	xge_xui_widget pWidget;
	xge_xui_widget pHeaderWidget;
	xge_xui_widget pIconWidget;
	xge_xui_widget pTitleWidget;
	xge_xui_widget pClientWidget;
	xge_xui_image_t tIconImage;
	xge_xui_label_t tTitleLabel;
	xui_font pFont;
	const char* sTitle;
	uint32_t iTitleColor;
	uint32_t iTitleFlags;
	uint32_t iHeaderColor;
	uint32_t iClientColor;
	float fHeaderHeight;
	float fIconSize;
	float fHeaderGap;
	int bClip;
};

struct xge_xui_separator_t {
	xge_xui_widget pWidget;
	uint32_t iColor;
	float fThickness;
	int iOrientation;
	int iAlign;
	int iLineStyle;
};

typedef struct xge_xui_input_decoration_desc_t {
	int iKind;
	int iVisibleMode;
	float fWidth;
	float fPadding;
	int iIcon;
	const char* sText;
	xui_texture pTexture;
	xge_rect_t tSrc;
	uint32_t iColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iDisabledColor;
	xge_xui_click_proc procClick;
	xge_xui_input_decoration_paint_proc procPaint;
	void* pUser;
} xge_xui_input_decoration_desc_t, *xge_xui_input_decoration_desc;

struct xge_xui_input_decoration_t {
	xge_xui_input_decoration pNext;
	int iSide;
	int iKind;
	int iVisibleMode;
	int iState;
	float fWidth;
	float fPadding;
	xge_rect_t tRect;
	int iIcon;
	const char* sText;
	xui_texture pTexture;
	xge_rect_t tSrc;
	uint32_t iColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iDisabledColor;
	xge_xui_click_proc procClick;
	xge_xui_input_decoration_paint_proc procPaint;
	void* pUser;
};

struct xge_xui_input_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_text_t tText;
	xui_font pFont;
	xge_xui_menu pDefaultMenu;
	xge_xui_text_submit_proc procChange;
	xge_xui_text_submit_proc procSubmit;
	xge_xui_input_filter_proc procFilter;
	xge_xui_input_error_proc procError;
	void* pChangeUser;
	void* pSubmitUser;
	void* pFilterUser;
	void* pErrorUser;
	int arrDefaultMenuEnabled[5];
	const char* sPlaceholder;
	uint32_t iTextColor;
	uint32_t iPlaceholderColor;
	uint32_t iNormalBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iFocusColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iErrorBackgroundColor;
	uint32_t iErrorBorderColor;
	uint32_t iCursorColor;
	uint32_t iSelectionColor;
	uint32_t iDisabledTextColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iDisabledBorderColor;
	xge_xui_input_decoration pLeadingDecoration;
	xge_xui_input_decoration pTrailingDecoration;
	xge_xui_input_decoration pHoverDecoration;
	xge_xui_input_decoration pActiveDecoration;
	float fLeadingDecorationWidth;
	float fTrailingDecorationWidth;
	float fScrollX;
	double fLastClickTime;
	float fLastClickX;
	float fLastClickY;
	float fPressX;
	float fPressY;
	float fCursorBlinkTime;
	int iPressCursor;
	int bPressPending;
	int bPressInsideSelection;
	int bSelecting;
	int bPassword;
	int bReadonly;
	int bDisabled;
	int bError;
	int bDecorationDirty;
	int bCursorVisible;
	int bInitialized;
	int iTextAlign;
	int iMaxLength;
	int iChangeCount;
	int iSubmitCount;
	int iClearCount;
	int iFilterRejectCount;
};

struct xge_xui_color_picker_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_popup_t tPopup;
	xui_font pFont;
	uint32_t iColor;
	uint32_t iOldColor;
	uint32_t arrPalette[XGE_XUI_COLOR_PICKER_PALETTE_CAPACITY];
	xge_rect_t arrPaletteRect[XGE_XUI_COLOR_PICKER_PALETTE_CAPACITY];
	int iPaletteCount;
	xge_rect_t tSwatchRect;
	xge_rect_t tButtonRect;
	xge_rect_t tSvRect;
	xge_rect_t tHueRect;
	xge_rect_t tOldRect;
	xge_rect_t tNewRect;
	xge_rect_t tHexRect;
	xge_rect_t tHexCopyRect;
	xge_rect_t arrFieldRect[4];
	xge_rect_t arrSliderRect[4];
	char sHex[16];
	xge_xui_text_t tEditText;
	int iSelectedPalette;
	int iActiveField;
	int iActiveArea;
	int bCopyHover;
	int bCopyActive;
	int bEditError;
	int bAlphaEnabled;
	float fHue;
	float fSaturation;
	float fValue;
	xge_xui_color_proc procChange;
	void* pUser;
	uint32_t iPanelColor;
	uint32_t iBorderColor;
	uint32_t iTextColor;
	uint32_t iAccentColor;
	uint32_t iFieldColor;
	uint32_t iHoverColor;
	uint32_t iMutedTextColor;
	int iChangeCount;
};

struct xge_xui_date_picker_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pPopupWidget;
	xui_font pFont;
	xtime tValue;
	xtime tStart;
	xtime tEnd;
	xtime tDraftValue;
	xtime tDraftStart;
	xtime tDraftEnd;
	xtime tViewMonth[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xtime tMin;
	xtime tMax;
	xtime tDefaultRangeSpan;
	char sFormat[64];
	char sRangeSeparator[16];
	int bHasMin;
	int bHasMax;
	int bNullable;
	int bHasValue;
	int bDraftHasValue;
	int iMode;
	int iState;
	int iHoverPart;
	int iActivePart;
	int iHoverDayPanel;
	int iHoverDayCell;
	int iActivePanel;
	int iRangeAnchor;
	int iFirstDayOfWeek;
	int bShowSecond;
	int iClosingMode;
	int iChangeCount;
	int iChangingCount;
	int iCommitCount;
	int iCancelCount;
	int iClearCount;
	xge_xui_date_proc procChange;
	xge_xui_date_proc procChanging;
	xge_xui_date_proc procCommit;
	xge_xui_date_proc procCancel;
	xge_xui_date_proc procClear;
	void* pChangeUser;
	void* pChangingUser;
	void* pCommitUser;
	void* pCancelUser;
	void* pClearUser;
	xge_rect_t tFieldRect;
	xge_rect_t tButtonRect;
	xge_rect_t tPopupRect;
	xge_rect_t tTimePanelRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t tPanelRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t tHeaderRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t tPrevRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t tNextRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t tYearRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t tMonthRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_rect_t arrDayRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY][42];
	xge_rect_t arrTimeRect[XGE_XUI_DATE_PICKER_PANEL_CAPACITY][3];
	xge_rect_t arrFooterRect[4];
	uint32_t iPanelColor;
	uint32_t iPopupColor;
	uint32_t iHeaderColor;
	uint32_t iGridColor;
	uint32_t iTextColor;
	uint32_t iMutedTextColor;
	uint32_t iBorderColor;
	uint32_t iFieldColor;
	uint32_t iSelectedColor;
	uint32_t iRangeColor;
	uint32_t iHoverColor;
	uint32_t iFocusColor;
	uint32_t iDisabledTextColor;
	uint32_t iErrorColor;
};

struct xge_xui_numeric_input_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_input_t tInput;
	xge_xui_slider_proc procChange;
	xge_xui_numeric_format_proc procFormat;
	void* pUser;
	void* pFormatUser;
	float fMin;
	float fMax;
	float fStep;
	float fValue;
	float fSpinnerWidth;
	xge_xui_input_decoration pSpinnerPaddingDecoration;
	uint32_t iSpinnerColor;
	uint32_t iSpinnerHoverColor;
	uint32_t iSpinnerActiveColor;
	uint32_t iSpinnerDisabledColor;
	uint32_t iSpinnerIconColor;
	uint32_t iSpinnerDisabledIconColor;
	int bInteger;
	int bShowSpinner;
	int bError;
	int iHoverButton;
	int iActiveButton;
	int iPrecision;
	int iState;
	int iChangeCount;
};

struct xge_xui_checkbox_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	const char* sText;
	xge_xui_checked_proc procChange;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iTextFlags;
	uint32_t iColorBox;
	uint32_t iColorChecked;
	xui_texture pUncheckedTexture;
	xui_texture pCheckedTexture;
	xge_rect_t tUncheckedSrc;
	xge_rect_t tCheckedSrc;
	float fIndicatorSize;
	float fGap;
	int iState;
	int iChangeCount;
};

struct xge_xui_radio_group_t {
	xge_xui_radio pFirst;
	xge_xui_select_proc procChange;
	void* pUser;
	int iSelectedValue;
	int iChangeCount;
};

struct xge_xui_radio_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_radio_group pGroup;
	xge_xui_radio pNextInGroup;
	xui_font pFont;
	const char* sText;
	xge_xui_checked_proc procChange;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iTextFlags;
	uint32_t iColorRing;
	uint32_t iColorChecked;
	xui_texture pUncheckedTexture;
	xui_texture pCheckedTexture;
	xge_rect_t tUncheckedSrc;
	xge_rect_t tCheckedSrc;
	float fIndicatorSize;
	float fGap;
	int iValue;
	int iState;
	int iChangeCount;
};

struct xge_xui_toggle_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pInnerFont;
	const char* sCheckedText;
	const char* sUncheckedText;
	xge_xui_checked_proc procChange;
	void* pUser;
	uint32_t iColorTrack;
	uint32_t iColorChecked;
	uint32_t iColorKnob;
	uint32_t iColorTrackBorder;
	uint32_t iColorCheckedText;
	uint32_t iColorUncheckedText;
	xui_texture pUncheckedTexture;
	xui_texture pCheckedTexture;
	xge_rect_t tUncheckedSrc;
	xge_rect_t tCheckedSrc;
	float fTrackHeight;
	float fTrackWidth;
	float fKnobInset;
	float fTextPadding;
	float fTextGap;
	int iState;
	int iChangeCount;
};

struct xge_xui_slider_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_slider_proc procChange;
	void* pUser;
	float fMin;
	float fMax;
	float fValue;
	float fStep;
	float fPageStep;
	float fTrackSize;
	float fKnobSize;
	float fTrackRadius;
	float fKnobRadius;
	uint32_t iColorTrack;
	uint32_t iColorFill;
	uint32_t iColorKnob;
	uint32_t iColorKnobBorder;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	int iOrientation;
	int iState;
	int iChangeCount;
};

struct xge_xui_progress_t {
	xge_xui_widget pWidget;
	xui_font pFont;
	char* sTextTemplate;
	char* sDisplayText;
	float fMin;
	float fMax;
	float fValue;
	uint32_t iColorTrack;
	uint32_t iColorFill;
	uint32_t iTextColor;
	uint32_t iFillTextColor;
	uint32_t iTextFlags;
	xge_nine_patch_t tTrackPatch;
	xge_nine_patch_t tFillPatch;
	int iFillDirection;
	int iFillPatchMode;
	int bTemplateString;
	int bHasTrackPatch;
	int bHasFillPatch;
};

struct xge_xui_pager_item_t {
	xge_rect_t tRect;
	int iType;
	int iPage;
	int bEnabled;
	char sText[16];
};

struct xge_xui_pager_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	const char* sFirstText;
	const char* sLastText;
	const char* sPrevText;
	const char* sNextText;
	xge_xui_pager_change_proc procChange;
	void* pUser;
	xge_xui_pager_item_t arrItem[XGE_XUI_PAGER_ITEM_CAPACITY];
	int iItemCount;
	int iPageCount;
	int iCurrentPage;
	int iWindowSize;
	int iHover;
	int iActive;
	int iState;
	int iChangeCount;
	float fItemHeight;
	float fPageWidth;
	float fTextWidth;
	float fNavWidth;
	float fEllipsisWidth;
	uint32_t iBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iTextColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iCurrentColor;
	uint32_t iCurrentTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iFocusColor;
};

struct xge_xui_tabs_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pTabBarWidget;
	xge_xui_widget pClientWidget;
	xge_xui_widget arrButtonWidget[XGE_XUI_TABS_PAGE_CAPACITY];
	xge_xui_widget arrPageWidget[XGE_XUI_TABS_PAGE_CAPACITY];
	xge_xui_button_t arrButton[XGE_XUI_TABS_PAGE_CAPACITY];
	char arrTitle[XGE_XUI_TABS_PAGE_CAPACITY][XGE_XUI_TABS_TITLE_CAPACITY];
	int arrEnabledLocal[XGE_XUI_TABS_PAGE_CAPACITY];
	int arrDirtyLocal[XGE_XUI_TABS_PAGE_CAPACITY];
	xui_texture arrIconLocal[XGE_XUI_TABS_PAGE_CAPACITY];
	xge_rect_t arrIconSrcLocal[XGE_XUI_TABS_PAGE_CAPACITY];
	xui_font pFont;
	const char** arrItems;
	const int* arrEnabled;
	const int* arrDirty;
	const xui_texture* arrIcons;
	const xge_rect_t* arrIconSrc;
	int iItemCount;
	int iEnabledCount;
	int iDirtyCount;
	int iIconCount;
	int iSelected;
	int iHover;
	int iActive;
	int bActiveClose;
	float fTabWidth;
	float fTabHeight;
	float fScrollX;
	int iTabPlacement;
	int bScrollable;
	int bCloseButtons;
	xge_xui_select_proc procSelect;
	xge_xui_select_proc procClose;
	void* pUser;
	uint32_t iTabColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	uint32_t iBorderColor;
	uint32_t iClientColor;
	int iState;
	int iChangeCount;
};

struct xge_xui_toolbar_item_t {
	const char* sText;
	const char* sTooltip;
	int iType;
	int bEnabled;
	int bChecked;
	int iGroup;
	xge_rect_t tRect;
};

struct xge_xui_toolbar_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	xge_xui_toolbar_item_t arrItems[XGE_XUI_TOOLBAR_CAPACITY];
	int iItemCount;
	int iOrientation;
	int iHover;
	int iActive;
	int bOverflowEnabled;
	int bOverflowActive;
	int iOverflowFirst;
	int iOverflowCount;
	float fItemWidth;
	float fItemHeight;
	float fSeparatorSize;
	float fGroupGap;
	float fOverflowSize;
	xge_rect_t tOverflowRect;
	xge_xui_select_proc procSelect;
	xge_xui_click_proc procOverflow;
	void* pUser;
	void* pOverflowUser;
	uint32_t iItemColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iCheckedColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	uint32_t iSeparatorColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	int iState;
	int iSelectCount;
};

struct xge_xui_menubar_item_t {
	const char* sText;
	int iState;
	int iValue;
	int iMnemonic;
	xge_xui_menu pMenu;
	xge_rect_t tRect;
};

struct xge_xui_menubar_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	xge_xui_menubar_item_t arrItems[XGE_XUI_MENUBAR_CAPACITY];
	int iItemCount;
	int iHover;
	int iActive;
	int iOpen;
	xge_xui_bar_metrics_t tMetrics;
	xge_xui_bar_colors_t tColors;
	xge_xui_menu_select_proc procSelect;
	void* pUser;
	int iState;
	int iSelectCount;
};

struct xge_xui_status_bar_item_t {
	const char* sText;
	int iType;
	int iSection;
	int bClickable;
	int bEnabled;
	float fWidth;
	float fFlex;
	float fMin;
	float fMax;
	float fValue;
	xge_rect_t tRect;
};

struct xge_xui_status_bar_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	xge_xui_status_bar_item_t arrItems[XGE_XUI_STATUS_BAR_CAPACITY];
	int iItemCount;
	int iHover;
	int iActive;
	float fHeight;
	float fGap;
	float fItemPadding;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iBorderColor;
	uint32_t iItemColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iProgressTrackColor;
	uint32_t iProgressFillColor;
	int iState;
	int iSelectCount;
};

struct xge_xui_tree_view_node_t {
	int iId;
	int iParent;
	int iDepth;
	int bExpanded;
	int bHasChildren;
	int bEnabled;
	int bIconReserved;
	int bCheckReserved;
	int bChecked;
	const char* sText;
	xge_rect_t tRect;
};

struct xge_xui_scroll_model_t {
	xge_rect_t tViewportRect;
	float fContentW;
	float fContentH;
	float fScrollX;
	float fScrollY;
};

struct xge_xui_table_view_column_t {
	int iId;
	const char* sId;
	const char* sTitle;
	float fWidth;
	float fMinWidth;
	float fMaxWidth;
	int bVisible;
	int bVisibleSet;
	int bResizable;
	int bResizableSet;
	int iAlign;
	int iType;
	int bHasStyle;
	xge_xui_style_t tStyle;
	xge_xui_table_view_format_proc procFormatter;
	void* pFormatterUser;
	xge_xui_table_view_cell_renderer_proc procRenderer;
	void* pRendererUser;
	xge_rect_t tRect;
};

struct xge_xui_table_view_row_t {
	float fHeight;
	int bSelected;
	int bDisabled;
	int bHasStyle;
	xge_xui_style_t tStyle;
};

struct xge_xui_table_view_cell_t {
	const char* sText;
	const void* pValue;
	const char* sTooltip;
	int iType;
	int iRowSpan;
	int iColSpan;
	int bDisabled;
	int bEditing;
	int bInvalid;
	int bDirty;
	int bHasStyle;
	xge_xui_style_t tStyle;
	xge_xui_table_view_format_proc procFormatter;
	void* pFormatterUser;
	xge_xui_table_view_cell_renderer_proc procRenderer;
	void* pRendererUser;
};

struct xge_xui_table_view_colors_t {
	uint32_t iBackgroundColor;
	uint32_t iHeaderColor;
	uint32_t iHeaderTextColor;
	uint32_t iRowColor;
	uint32_t iAltRowColor;
	uint32_t iHoverColor;
	uint32_t iSelectedColor;
	uint32_t iDisabledColor;
	uint32_t iGridColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iFocusRingColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
};

struct xge_xui_table_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pBodyWidget;
	xge_xui_scroll_model_t tScroll;
	xge_xui_scroll_frame_t* pFrame;
	xui_font pFont;
	xge_xui_table_view_column_t arrColumns[XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY];
	const xge_xui_table_view_row_t* arrRows;
	int iColumnCount;
	int iRowCount;
	int iRowStateCount;
	int iSelectionMode;
	int iSelectedRow;
	int iSelectedColumn;
	int iHoverRow;
	int iHoverColumn;
	int iFocusRow;
	int iFocusColumn;
	int iActiveRow;
	int iActiveColumn;
	int iFirstVisible;
	int iPaintVisibleCount;
	int iSortColumn;
	int bSortDescending;
	float fDefaultColumnWidth;
	float fDefaultRowHeight;
	float fHeaderHeight;
	float fDragX;
	float fDragWidth;
	int iResizeColumn;
	xge_xui_table_view_count_proc procCount;
	xge_xui_table_view_cell_proc procCell;
	xge_xui_table_view_sort_proc procSort;
	xge_xui_table_view_select_proc procSelect;
	xge_xui_table_view_column_resize_proc procColumnResize;
	xge_xui_table_view_hover_proc procHover;
	xge_xui_table_view_merge_proc procMerge;
	xge_xui_table_view_header_renderer_proc procHeaderRenderer;
	xge_xui_table_view_cell_renderer_proc procCellRenderer;
	void* pAdapterUser;
	void* pSortUser;
	void* pSelectUser;
	void* pColumnResizeUser;
	void* pHoverUser;
	void* pMergeUser;
	void* pHeaderRendererUser;
	void* pCellRendererUser;
	uint32_t iBackgroundColor;
	uint32_t iHeaderColor;
	uint32_t iHeaderTextColor;
	uint32_t iRowColor;
	uint32_t iAltRowColor;
	uint32_t iHoverColor;
	uint32_t iSelectedColor;
	uint32_t iDisabledColor;
	uint32_t iGridColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iFocusRingColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
	int iState;
	int iSelectCount;
	int iSortCount;
	int iColumnResizeCount;
};

struct xge_xui_table_grid_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_table_view_t tTable;
	xge_xui_widget pEditWidget;
	xge_xui_input_t tEditInput;
	xge_xui_widget pNumericWidget;
	xge_xui_numeric_input_t tEditNumeric;
	xge_xui_widget pComboWidget;
	xge_xui_combo_box pCombo;
	xge_xui_widget pColorWidget;
	xge_xui_color_picker_t tColorPicker;
	xge_xui_widget pDateWidget;
	xge_xui_date_picker_t tDatePicker;
	xge_xui_widget pTextAreaPopupWidget;
	xge_xui_widget pTextAreaContentWidget;
	xge_xui_widget pTextAreaEditWidget;
	xge_xui_widget pTextAreaOkWidget;
	xge_xui_widget pTextAreaCancelWidget;
	xge_xui_popup_t tTextAreaPopup;
	xge_xui_text_edit_t tTextAreaEdit;
	xge_xui_button_t tTextAreaOk;
	xge_xui_button_t tTextAreaCancel;
	xui_font pFont;
	xge_xui_table_view_count_proc procCount;
	xge_xui_table_view_cell_proc procCell;
	xge_xui_table_grid_set_proc procSet;
	xge_xui_table_grid_validate_proc procValidate;
	xge_xui_table_grid_change_proc procChange;
	xge_xui_table_grid_editor_proc procEditor;
	xge_xui_table_grid_editor_config_proc procEditorConfig;
	void* pAdapterUser;
	void* pSetUser;
	void* pValidateUser;
	void* pChangeUser;
	void* pEditorUser;
	void* pEditorConfigUser;
	xge_xui_table_grid_editor_config_t tEditorConfig;
	int iEditMode;
	int iEditingRow;
	int iEditingColumn;
	int iEditingType;
	int iActiveEditor;
	int iCommitCount;
	int iCancelCount;
	int iRejectCount;
	int iPickerCount;
	char sOriginalValue[XGE_XUI_TABLE_GRID_VALUE_CAPACITY];
};

struct xge_xui_timeline_frame_t {
	int iFrame;
	int iType;
	void* pUser;
};

struct xge_xui_timeline_span_t {
	int iId;
	int iStartFrame;
	int iEndFrame;
	int iType;
	char* sCustomType;
	char* sLabel;
	uint32_t iColor;
	void* pUser;
};

struct xge_xui_timeline_layer_t {
	int iId;
	char* sName;
	int bVisible;
	int bLocked;
	int bSelected;
	float fHeight;
	uint32_t iColor;
	void* pUser;
	xlist_struct tFrames;
	xlist_struct tSpans;
};

struct xge_xui_timeline_selection_t {
	int iLayer;
	int iFrame;
};

struct xge_xui_timeline_hit_t {
	int iType;
	int iLayer;
	int iLayerId;
	int iFrame;
	int iSpanId;
	xge_rect_t tRect;
};

struct xge_xui_timeline_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_scroll_model_t tScroll;
	xge_xui_scroll_frame_t* pFrame;
	xui_font pFont;
	xparray_struct arrLayers;
	xlist_struct tSelection;
	int iFrameCount;
	int iFrameRate;
	int iCurrentFrame;
	int iAnchorLayer;
	int iAnchorFrame;
	int iHoverLayer;
	int iHoverFrame;
	int iActiveLayer;
	int iActiveFrame;
	int iNextLayerId;
	int iNextSpanId;
	int bShowVisibilityFeature;
	int bShowLockFeature;
	int bDraggingSelection;
	int bDraggingPlayhead;
	float fLayerHeaderWidth;
	float fFrameWidth;
	float fMinFrameWidth;
	float fMaxFrameWidth;
	float fRowHeight;
	float fRulerHeight;
	float fDragStartX;
	float fDragStartY;
	float fDragCurrentX;
	float fDragCurrentY;
	uint32_t iBackgroundColor;
	uint32_t iHeaderColor;
	uint32_t iHeaderTextColor;
	uint32_t iLayerColor;
	uint32_t iLayerAltColor;
	uint32_t iLayerSelectedColor;
	uint32_t iGridColor;
	uint32_t iMajorGridColor;
	uint32_t iTextColor;
	uint32_t iMutedTextColor;
	uint32_t iSelectionColor;
	uint32_t iCurrentFrameColor;
	uint32_t iKeyFrameColor;
	uint32_t iSpanColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
	xge_xui_timeline_current_frame_changing_proc procCurrentFrameChanging;
	xge_xui_timeline_current_frame_changed_proc procCurrentFrameChanged;
	xge_xui_timeline_layer_changing_proc procLayerChanging;
	xge_xui_timeline_layer_changed_proc procLayerChanged;
	xge_xui_timeline_frame_changing_proc procFrameChanging;
	xge_xui_timeline_frame_changed_proc procFrameChanged;
	xge_xui_timeline_span_changing_proc procSpanChanging;
	xge_xui_timeline_span_changed_proc procSpanChanged;
	xge_xui_timeline_layer_selected_proc procLayerSelected;
	xge_xui_timeline_context_menu_opening_proc procContextMenuOpening;
	xge_xui_timeline_context_menu_command_proc procContextMenuCommand;
	xge_xui_timeline_frame_click_proc procFrameClick;
	xge_xui_timeline_frame_click_proc procFrameDoubleClick;
	xge_xui_timeline_selection_proc procSelection;
	xge_xui_timeline_layer_renderer_proc procLayerRenderer;
	xge_xui_timeline_ruler_renderer_proc procRulerRenderer;
	xge_xui_timeline_frame_renderer_proc procFrameRenderer;
	xge_xui_timeline_span_renderer_proc procSpanRenderer;
	void* pCurrentFrameUser;
	void* pLayerUser;
	void* pFrameUser;
	void* pSpanUser;
	void* pLayerSelectedUser;
	void* pContextMenuUser;
	void* pFrameClickUser;
	void* pFrameDoubleClickUser;
	void* pSelectionUser;
	void* pLayerRendererUser;
	void* pRulerRendererUser;
	void* pFrameRendererUser;
	void* pSpanRendererUser;
	int iCurrentFrameChangeCount;
	int iSelectionChangeCount;
	int iFrameChangeCount;
	int iLayerChangeCount;
	int iSpanChangeCount;
	int iLayerSelectCount;
	int iContextMenuOpenCount;
	int iContextMenuCommandCount;
	xge_xui_menu pLayerMenu;
	xge_xui_menu pFrameMenu;
	xge_xui_widget pRenameWidget;
	xge_xui_input_box pRenameBox;
	xge_xui_timeline_hit_t tContextHit;
	int iRenameLayer;
	char sTooltipText[256];
};

struct xge_xui_property_desc_t {
	const char* sId;
	const char* sName;
	const char* sDescription;
	int iType;
	const char* sValue;
	const char* sDefaultValue;
	int iFlags;
};

struct xge_xui_property_grid_style_t {
	uint32_t iBackgroundColor;
	uint32_t iGridColor;
	uint32_t iCategoryBackgroundColor;
	uint32_t iCategoryHoverColor;
	uint32_t iCategoryTextColor;
	uint32_t iCategoryIconColor;
	uint32_t iNameBackgroundColor;
	uint32_t iNameTextColor;
	uint32_t iNameHoverColor;
	uint32_t iValueBackgroundColor;
	uint32_t iValueTextColor;
	uint32_t iSelectedColor;
	uint32_t iReadonlyTextColor;
	uint32_t iInvalidColor;
	uint32_t iDirtyColor;
};

struct xge_xui_property_grid_category_t {
	char sId[XGE_XUI_PROPERTY_GRID_ID_CAPACITY];
	char sName[XGE_XUI_PROPERTY_GRID_NAME_CAPACITY];
	int bExpanded;
	int iPropertyCount;
};

struct xge_xui_property_grid_property_t {
	char sId[XGE_XUI_PROPERTY_GRID_ID_CAPACITY];
	char sName[XGE_XUI_PROPERTY_GRID_NAME_CAPACITY];
	char sDescription[XGE_XUI_PROPERTY_GRID_DESCRIPTION_CAPACITY];
	char sValue[XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY];
	char sDefaultValue[XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY];
	int iCategory;
	int iType;
	int iFlags;
	int bAutoDirty;
	xge_xui_table_grid_editor_config_t tEditorConfig;
	const char* arrEnumItems[XGE_XUI_PROPERTY_GRID_OPTION_CAPACITY];
	int arrEnumEnabled[XGE_XUI_PROPERTY_GRID_OPTION_CAPACITY];
	int bHasEditorConfig;
	xge_xui_property_grid_render_proc procRenderer;
	void* pRendererUser;
	xge_xui_property_grid_action_proc procAction;
	void* pActionUser;
};

struct xge_xui_property_grid_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_table_grid_t tGrid;
	xui_font pFont;
	xge_xui_table_view_column_t arrColumns[2];
	xge_xui_table_view_row_t arrRows[XGE_XUI_PROPERTY_GRID_VISIBLE_CAPACITY];
	xge_xui_property_grid_category_t arrCategories[XGE_XUI_PROPERTY_GRID_CATEGORY_CAPACITY];
	xge_xui_property_grid_property_t arrProperties[XGE_XUI_PROPERTY_GRID_PROPERTY_CAPACITY];
	int arrVisibleKind[XGE_XUI_PROPERTY_GRID_VISIBLE_CAPACITY];
	int arrVisibleIndex[XGE_XUI_PROPERTY_GRID_VISIBLE_CAPACITY];
	int iCategoryCount;
	int iPropertyCount;
	int iVisibleCount;
	int iSelectedProperty;
	float fNameWidth;
	float fRowHeight;
	float fCategoryHeight;
	int iDescriptionMode;
	float fDescriptionPanelHeight;
	xge_xui_property_grid_style_t tStyle;
	xge_xui_property_grid_select_proc procSelect;
	xge_xui_property_grid_validate_proc procValidate;
	xge_xui_property_grid_change_proc procChange;
	xge_xui_property_grid_action_proc procAction;
	xge_xui_property_grid_render_proc procRenderer;
	void* pSelectUser;
	void* pValidateUser;
	void* pChangeUser;
	void* pActionUser;
	void* pRendererUser;
	int iSelectCount;
	int iToggleCount;
};

struct xge_xui_accordion_section_t {
	char sTitle[XGE_XUI_ACCORDION_TITLE_CAPACITY];
	char sHeaderText[XGE_XUI_ACCORDION_TITLE_CAPACITY + 8];
	int iId;
	int bExpanded;
	int bEnabled;
	xge_xui_widget pSectionWidget;
	xge_xui_widget pHeaderWidget;
	xge_xui_widget pClientWidget;
	xge_xui_button_t tHeaderButton;
};

struct xge_xui_accordion_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xui_font pFont;
	xge_xui_accordion_section_t arrSections[XGE_XUI_ACCORDION_SECTION_CAPACITY];
	int iSectionCount;
	int iMode;
	int iSelected;
	float fHeaderHeight;
	float fSpacing;
	float fContentPadding;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iHeaderColor;
	uint32_t iHoverColor;
	uint32_t iExpandedColor;
	uint32_t iContentColor;
	uint32_t iBorderColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	uint32_t iDisabledTextColor;
	int iState;
	int iSelectCount;
};

struct xge_xui_msg_tip_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pHitWidget;
	xui_font pFont;
	char* sText;
	int iType;
	int bOpen;
	float fDuration;
	float fElapsed;
	xui_texture pIconTexture;
	xge_rect_t tIconSrc;
	int bCustomIcon;
	int bShowIcon;
	xge_rect_t tRect;
	xge_rect_t tIconRect;
	xge_rect_t tTextRect;
	float fMinWidth;
	float fMaxWidth;
	float fMinHeight;
	float fPaddingX;
	float fPaddingY;
	float fIconSize;
	float fIconGap;
	float fOffsetY;
	uint32_t iBackgroundColor;
	uint32_t iTextColor;
	uint32_t iIconColor;
	int iShowCount;
	int iCloseCount;
	int iExpireCount;
};

struct xge_xui_loader_t {
	const char* sURI;
	const xge_xui_binder_t* pBinder;
	xge_resource_t tResource;
	void* pDocument;
};

struct xge_xui_scrollbar_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_slider_proc procChange;
	void* pUser;
	float fMin;
	float fMax;
	float fPage;
	float fValue;
	float fDragStartMouse;
	float fDragStartValue;
	float fRepeatX;
	float fRepeatY;
	float fRepeatTimer;
	float fRepeatInterval;
	float fTrackSize;
	float fMinThumbSize;
	float fThumbRadius;
	float fButtonSize;
	uint32_t iColorTrack;
	uint32_t iColorThumb;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	uint32_t iColorButton;
	uint32_t iColorButtonIcon;
	int iOrientation;
	int iMode;
	int iButtonMode;
	int iHoverPart;
	int iActivePart;
	int iRepeatPart;
	int iRepeatDirection;
	int iRepeatPointerId;
	int iState;
	int bDraggingThumb;
	int iChangeCount;
};

struct xge_xui_scroll_frame_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pViewportWidget;
	xge_xui_widget pHScrollWidget;
	xge_xui_widget pVScrollWidget;
	xge_xui_widget pCornerWidget;
	xge_xui_scroll_model pModel;
	xge_xui_scrollbar_t tHScrollBar;
	xge_xui_scrollbar_t tVScrollBar;
	xge_xui_scroll_frame_change_proc procChange;
	void* pUser;
	xge_rect_t tFrameRect;
	xge_rect_t tViewportRect;
	xge_rect_t tHScrollRect;
	xge_rect_t tVScrollRect;
	xge_rect_t tCornerRect;
	float fScrollbarSize;
	float fWheelStep;
	float fDragX;
	float fDragY;
	float fDragScrollX;
	float fDragScrollY;
	uint32_t iCornerColor;
	uint32_t iCornerGripColor;
	int iScrollbarPolicyX;
	int iScrollbarPolicyY;
	int iScrollbarMode;
	int iWheelAxis;
	int iCornerMode;
	int bContentDragEnabled;
	int bShowHScroll;
	int bShowVScroll;
	int bShowCorner;
	int bDraggingContent;
	int iChangeCount;
};

struct xge_xui_scroll_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pContentWidget;
	xge_xui_scroll_model_t tModel;
	xge_xui_scroll_frame_t tFrame;
};

struct xge_xui_tree_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_scroll_model_t tScroll;
	xge_xui_scroll_frame_t tFrame;
	xui_font pFont;
	xge_xui_tree_view_node_t arrNodes[XGE_XUI_TREE_VIEW_NODE_CAPACITY];
	int arrVisible[XGE_XUI_TREE_VIEW_VISIBLE_CAPACITY];
	int iNodeCount;
	int iVisibleCount;
	int iFirstVisible;
	int iPaintVisibleCount;
	int iSelectedId;
	int iHoverVisible;
	int iFocusVisible;
	int iActiveVisible;
	int iActivePart;
	int bEnsureSelectedPending;
	float fItemHeight;
	float fIndent;
	xge_xui_tree_view_count_proc procCount;
	xge_xui_tree_view_node_proc procNode;
	void* pAdapterUser;
	xge_xui_select_proc procSelect;
	void* pSelectUser;
	xge_xui_tree_view_item_proc procItem;
	void* pItemUser;
	uint32_t iBorderColor;
	uint32_t iBackgroundColor;
	uint32_t iRowColor;
	uint32_t iHoverColor;
	uint32_t iSelectedColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iExpanderColor;
	uint32_t iIconColor;
	uint32_t iCheckColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
	int iSelectCount;
};

struct xge_xui_page_t {
	xge_xui_context pContext;
	xge_xui_widget pRoot;
	xge_xui_loader_t tLoader;
	void* pDocument;
	xge_resource_t arrImportResource[XGE_XUI_PAGE_IMPORT_CAPACITY];
	void* arrImportDocument[XGE_XUI_PAGE_IMPORT_CAPACITY];
	char arrImportURI[XGE_XUI_PAGE_IMPORT_CAPACITY][XGE_XUI_PAGE_IMPORT_URI_CAPACITY];
	int iImportCount;
	void* pMergedStyles;
	void* pMergedTokens;
	void* pMergedTemplates;
	xge_xui_page_index_t arrIndex[XGE_XUI_PAGE_INDEX_CAPACITY];
	int iIndexCount;
	int bIndexOverflow;
	xge_xui_page_click_binding_t arrClickBinding[XGE_XUI_PAGE_CLICK_BINDING_CAPACITY];
	int iClickBindingCount;
	xge_xui_page_event_binding_t arrEventBinding[XGE_XUI_PAGE_EVENT_BINDING_CAPACITY];
	int iEventBindingCount;
	xge_xui_page_hotkey_binding_t arrHotKeyBinding[XGE_XUI_PAGE_HOTKEY_BINDING_CAPACITY];
	int iHotKeyBindingCount;
	xge_xui_page_model_binding_t arrModelBinding[XGE_XUI_PAGE_MODEL_BINDING_CAPACITY];
	int iModelBindingCount;
	xge_xui_button_t arrButton[XGE_XUI_PAGE_BUTTON_CAPACITY];
	int iButtonCount;
	xge_xui_image_t arrImage[XGE_XUI_PAGE_IMAGE_CAPACITY];
	xui_texture arrImageTexture[XGE_XUI_PAGE_IMAGE_CAPACITY];
	int arrImageTextureOwned[XGE_XUI_PAGE_IMAGE_CAPACITY];
	int iImageCount;
	xge_xui_input_t arrInput[XGE_XUI_PAGE_INPUT_CAPACITY];
	int iInputCount;
	xge_xui_text_edit_t arrTextEdit[XGE_XUI_PAGE_TEXT_EDIT_CAPACITY];
	int iTextEditCount;
	xge_xui_numeric_input_t arrNumericInput[XGE_XUI_PAGE_NUMERIC_INPUT_CAPACITY];
	int iNumericInputCount;
	xge_xui_color_picker_t arrColorPicker[XGE_XUI_PAGE_COLOR_PICKER_CAPACITY];
	int iColorPickerCount;
	xge_xui_date_picker_t arrDatePicker[XGE_XUI_PAGE_DATE_PICKER_CAPACITY];
	int iDatePickerCount;
	xge_xui_checkbox_t arrCheckBox[XGE_XUI_PAGE_CHECKBOX_CAPACITY];
	int iCheckBoxCount;
	xge_xui_radio_t arrRadio[XGE_XUI_PAGE_RADIO_CAPACITY];
	int iRadioCount;
	xge_xui_toggle_t arrToggle[XGE_XUI_PAGE_TOGGLE_CAPACITY];
	int iToggleCount;
	xge_xui_slider_t arrSlider[XGE_XUI_PAGE_SLIDER_CAPACITY];
	int iSliderCount;
	xge_xui_scrollbar_t arrScrollBar[XGE_XUI_PAGE_SCROLLBAR_CAPACITY];
	int iScrollBarCount;
	xge_xui_progress_t arrProgress[XGE_XUI_PAGE_PROGRESS_CAPACITY];
	int iProgressCount;
	xge_xui_pager_t arrPager[XGE_XUI_PAGE_PAGER_CAPACITY];
	int iPagerCount;
	xge_xui_window arrWindow[XGE_XUI_PAGE_WINDOW_CAPACITY];
	int iWindowCount;
	xge_xui_panel_t arrPanel[XGE_XUI_PAGE_PANEL_CAPACITY];
	int iPanelCount;
	xge_xui_tabs_t arrTabs[XGE_XUI_PAGE_TABS_CAPACITY];
	int iTabsCount;
	xge_xui_split_layout arrSplitLayout[XGE_XUI_PAGE_SPLIT_LAYOUT_CAPACITY];
	int iSplitLayoutCount;
	xge_xui_dock_layout arrDockLayout[XGE_XUI_PAGE_DOCK_LAYOUT_CAPACITY];
	int iDockLayoutCount;
	xge_xui_dock_window arrDockWindow[XGE_XUI_PAGE_DOCK_WINDOW_CAPACITY];
	int iDockWindowCount;
	xge_xui_toolbar_t arrToolbar[XGE_XUI_PAGE_TOOLBAR_CAPACITY];
	int iToolbarCount;
	xge_xui_menubar_t arrMenuBar[XGE_XUI_PAGE_MENUBAR_CAPACITY];
	int iMenuBarCount;
	xge_xui_status_bar_t arrStatusBar[XGE_XUI_PAGE_STATUS_BAR_CAPACITY];
	int iStatusBarCount;
	xge_xui_combo_box arrComboBox[XGE_XUI_PAGE_COMBO_BOX_CAPACITY];
	int iComboBoxCount;
	xge_xui_popup arrPopup[XGE_XUI_PAGE_POPUP_CAPACITY];
	int iPopupCount;
	xge_xui_menu arrMenu[XGE_XUI_PAGE_MENU_CAPACITY];
	xge_xui_widget arrMenuWidget[XGE_XUI_PAGE_MENU_CAPACITY];
	int iMenuCount;
	xge_xui_widget arrOverlayPortal[XGE_XUI_PAGE_OVERLAY_PORTAL_CAPACITY];
	char arrOverlayPortalPath[XGE_XUI_PAGE_OVERLAY_PORTAL_CAPACITY][XGE_XUI_PAGE_OVERLAY_PORTAL_PATH_CAPACITY];
	int iOverlayPortalCount;
	xge_xui_label_t arrLabel[XGE_XUI_PAGE_LABEL_CAPACITY];
	int iLabelCount;
	xge_xui_separator_t arrSeparator[XGE_XUI_PAGE_SEPARATOR_CAPACITY];
	int iSeparatorCount;
	xge_xui_scroll_view_t arrScrollView[XGE_XUI_PAGE_SCROLL_VIEW_CAPACITY];
	int iScrollViewCount;
	xge_xui_list_view arrListView[XGE_XUI_PAGE_LIST_VIEW_CAPACITY];
	const char* arrListViewItems[XGE_XUI_PAGE_LIST_VIEW_CAPACITY][XGE_XUI_PAGE_LIST_VIEW_ITEM_CAPACITY];
	int arrListViewEnabled[XGE_XUI_PAGE_LIST_VIEW_CAPACITY][XGE_XUI_PAGE_LIST_VIEW_ITEM_CAPACITY];
	int arrListViewSelected[XGE_XUI_PAGE_LIST_VIEW_CAPACITY][XGE_XUI_PAGE_LIST_VIEW_ITEM_CAPACITY];
	int iListViewCount;
	xge_xui_tree_view_t arrTreeView[XGE_XUI_PAGE_TREE_VIEW_CAPACITY];
	int iTreeViewCount;
	xge_xui_table_view_t arrTableView[XGE_XUI_PAGE_TABLE_VIEW_CAPACITY];
	xge_xui_page_table_view_adapter_t* arrTableViewAdapter[XGE_XUI_PAGE_TABLE_VIEW_CAPACITY];
	int iTableViewCount;
	xge_xui_table_grid_t arrTableGrid[XGE_XUI_PAGE_TABLE_GRID_CAPACITY];
	xge_xui_page_table_view_adapter_t* arrTableGridAdapter[XGE_XUI_PAGE_TABLE_GRID_CAPACITY];
	int iTableGridCount;
	xge_xui_timeline_view_t arrTimelineView[XGE_XUI_PAGE_TIMELINE_VIEW_CAPACITY];
	int iTimelineViewCount;
	xge_xui_property_grid_t arrPropertyGrid[XGE_XUI_PAGE_PROPERTY_GRID_CAPACITY];
	int iPropertyGridCount;
	xge_xui_accordion_t arrAccordion[XGE_XUI_PAGE_ACCORDION_CAPACITY];
	int iAccordionCount;
	uint32_t iStyleVersion;
	uint32_t iThemeVersion;
	uint32_t iModelVersion;
	xge_xui_edges_t tSafeAreaPrev;
	int bSafeAreaApplied;
	char sError[XGE_XUI_PAGE_ERROR_CAPACITY];
};

struct xge_xui_window_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pClientWidget;
	xge_xui_widget pCollapseButtonWidget;
	xge_xui_widget pMaximizeButtonWidget;
	xge_xui_widget pCloseButtonWidget;
	xge_xui_button_t tCollapseButton;
	xge_xui_button_t tMaximizeButton;
	xge_xui_button_t tCloseButton;
	xui_font pFont;
	xui_texture pIconTexture;
	xge_rect_t tIconSrc;
	const char* sTitle;
	xge_xui_click_proc procClose;
	void* pUser;
	xge_rect_t tRestoreRect;
	xge_rect_t tDragStartRect;
	xge_rect_t tPreviewRect;
	uint32_t iTitleBarColor;
	uint32_t iTitleTextColor;
	uint32_t iBorderColor;
	uint32_t iButtonColorNormal;
	uint32_t iButtonColorHover;
	uint32_t iButtonColorActive;
	float fTitleBarHeight;
	float fBorderWidth;
	float fResizeGrip;
	float fButtonSize;
	float fIconSize;
	float fDragStartX;
	float fDragStartY;
	float fExpandedHeight;
	uint32_t iResizeEdges;
	int iInteractionEdges;
	int bPreviewActive;
	int bOpen;
	int bShowTitleBar;
	int bMovable;
	int bDragAnywhere;
	int bResizable;
	int bShowCollapse;
	int bShowMaximize;
	int bShowClose;
	int bCollapsed;
	int bMaximized;
	int bTopMost;
	int bActive;
};

struct xge_xui_list_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_scroll_model_t tScroll;
	xge_xui_scroll_frame_t tFrame;
	xui_font pFont;
	const char** arrItems;
	const int* arrEnabled;
	int iEnabledCount;
	int iItemCount;
	int iSelected;
	int iHover;
	int iFocus;
	int bEnsureSelectedPending;
	float fItemHeight;
	int iSelectionMode;
	int iSelectionAnchor;
	int* arrSelected;
	int iSelectionCount;
	int bNotifyRepeatSelect;
	xge_xui_select_proc procSelect;
	void* pSelectUser;
	xge_xui_list_view_item_proc procItem;
	void* pItemUser;
	uint32_t iBorderColor;
	uint32_t iRowColor;
	uint32_t iHoverColor;
	uint32_t iSelectedColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
};

struct xge_xui_msg_box_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_window_t tWindow;
	xge_xui_widget pContentWidget;
	xge_xui_widget arrButtonWidget[XGE_XUI_MSG_BOX_BUTTON_CAPACITY];
	xge_xui_button_t arrButton[XGE_XUI_MSG_BOX_BUTTON_CAPACITY];
	xui_font pFont;
	const char* sTitle;
	const char* sMessage;
	xge_xui_select_proc procResult;
	void* pUser;
	xui_texture pIconTexture;
	xge_rect_t tIconSrc;
	xge_rect_t tIconRect;
	xge_rect_t tMessageRect;
	xge_rect_t tBackdropRect;
	int iType;
	int iButtons;
	int iResult;
	int bModal;
	int bShowIcon;
	int bCustomIcon;
	uint32_t iBackdropColor;
	uint32_t iBackgroundColor;
	uint32_t iTitleColor;
	uint32_t iCloseColor;
	uint32_t iMessageColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonActiveColor;
	uint32_t iButtonTextColor;
	uint32_t iIconColor;
	xge_rect_t arrButtonRect[XGE_XUI_MSG_BOX_BUTTON_CAPACITY];
	int arrButtonResult[XGE_XUI_MSG_BOX_BUTTON_CAPACITY];
	int arrButtonSemantic[XGE_XUI_MSG_BOX_BUTTON_CAPACITY];
	const char* arrButtonText[XGE_XUI_MSG_BOX_BUTTON_CAPACITY];
	char arrButtonTextOwned[XGE_XUI_MSG_BOX_BUTTON_CAPACITY][64];
	int iButtonCount;
	int iLayoutDirty;
};

struct xge_xui_input_box_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_window_t tWindow;
	xge_xui_widget pContentWidget;
	xge_xui_widget pInputWidget;
	xge_xui_widget pOkWidget;
	xge_xui_widget pCancelWidget;
	xge_xui_input_t tInput;
	xge_xui_button_t tOkButton;
	xge_xui_button_t tCancelButton;
	xui_font pFont;
	const char* sTitle;
	const char* sPrompt;
	char* sResult;
	xge_xui_text_submit_proc procResult;
	void* pUser;
	xge_rect_t tPromptRect;
	xge_rect_t tBackdropRect;
	int iResult;
	int bModal;
	uint32_t iBackdropColor;
	uint32_t iBackgroundColor;
	uint32_t iTitleColor;
	uint32_t iCloseColor;
	uint32_t iPromptColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonActiveColor;
	uint32_t iButtonTextColor;
	int iLayoutDirty;
};

struct xge_xui_combo_box_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pListWidget;
	xge_xui_popup_t tPopup;
	xge_xui_list_view_t tList;
	xui_font pFont;
	const char** arrItems;
	const xge_xui_combo_box_item_t* arrItemData;
	const char* arrListItems[XGE_XUI_PAGE_COMBO_BOX_ITEM_CAPACITY];
	const int* arrEnabled;
	int iItemCount;
	int iSelected;
	int iHighlight;
	float fPopupHeight;
	float fPopupMaxHeight;
	float fItemHeight;
	int iPopupPlacement;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	uint32_t iBorderColor;
	uint32_t iArrowColor;
	uint32_t iPopupColor;
	uint32_t iItemHoverColor;
	uint32_t iItemSelectedColor;
	uint32_t iItemDisabledColor;
	int iState;
	int iChangeCount;
};

struct xge_xui_combo_box_item_t {
	const char* sText;
	int iValue;
	int bEnabled;
	int bSeparator;
	int iIcon;
	void* pUser;
};

struct xge_xui_menu_t {
	xge_xui_context pContext;
	xge_xui_widget pOwner;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pContentWidget;
	xge_xui_popup_t tPopup;
	xui_font pFont;
	xge_xui_menu_item_t arrItems[XGE_XUI_MENU_ITEM_CAPACITY];
	xge_rect_t arrItemRect[XGE_XUI_MENU_ITEM_CAPACITY];
	int iItemCount;
	xge_xui_menu pParentMenu;
	xge_xui_menu pOpenSubmenu;
	int iParentItem;
	int iHover;
	int iUpdateLock;
	int bLayoutDirty;
	float fContentW;
	float fContentH;
	float fTextW;
	float fShortcutW;
	xge_xui_menu_metrics_t tMetrics;
	xge_xui_menu_colors_t tColors;
	xge_xui_menu_select_proc procSelect;
	void* pUser;
	int iSelectCount;
};

typedef struct xge_xui_split_layout_pane_t {
	xge_xui_widget pWidget;
	float fWeight;
	float fFixedSize;
	float fResolvedSize;
	float fMinSize;
	float fMaxSize;
	int iMode;
	int bCollapsed;
} xge_xui_split_layout_pane_t;

typedef struct xge_xui_split_layout_divider_t {
	xge_xui_widget pWidget;
	xge_rect_t tLayoutRect;
	xge_rect_t tVisualRect;
	xge_rect_t tHitRect;
} xge_xui_split_layout_divider_t;

struct xge_xui_split_layout_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_split_layout_pane_t* arrPanes;
	xge_xui_split_layout_divider_t* arrDividers;
	xge_xui_widget pShadowWidget;
	int iPaneCount;
	int iOrientation;
	int iActiveDivider;
	int iHoverDivider;
	int bShadowDrag;
	float fDividerSize;
	float fDividerVisualSize;
	float fDividerHitSize;
	float fDragStartMouse;
	float fDragCurrentMouse;
	float fDragStartBefore;
	float fDragStartAfter;
	float fResolvedAxis;
	uint32_t iDividerColor;
	uint32_t iDividerHoverColor;
	uint32_t iDividerActiveColor;
	uint32_t iShadowColor;
	xge_xui_split_layout_change_proc procChange;
	void* pChangeUser;
};

struct xge_xui_dock_region_t {
	xge_xui_dock_layout pLayout;
	xge_xui_dock_node pRoot;
	xge_rect_t tRect;
	xge_rect_t tSplitterRect;
	float fPortion;
	float fMinSize;
	float fMaxSize;
	int iKind;
	int bVisible;
};

struct xge_xui_dock_pane_t {
	xge_xui_dock_layout pLayout;
	xge_xui_dock_node pNode;
	xarray_struct arrWindows;
	xge_rect_t tRect;
	xge_rect_t tTabStripRect;
	xge_rect_t tCaptionRect;
	xge_rect_t tClientRect;
	xge_rect_t tCloseRect;
	xge_rect_t tAutoHideRect;
	xge_rect_t tDockRect;
	xge_rect_t tOptionRect;
	xge_rect_t tOverflowRect;
	int iActive;
	int iHoverPart;
	int iActivePart;
};

struct xge_xui_dock_node_t {
	xge_xui_dock_node pParent;
	xge_xui_dock_node pFirst;
	xge_xui_dock_node pSecond;
	xge_xui_dock_pane pPane;
	xge_rect_t tRect;
	xge_rect_t tSplitterRect;
	float fRatio;
	float fMinWidth;
	float fMinHeight;
	int iType;
	int iAxis;
};

struct xge_xui_dock_layout_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pDragOverlayWidget;
	xge_xui_widget pAutoHideOverlayWidget;
	xge_xui_dock_region_t arrRegions[XGE_XUI_DOCK_REGION_COUNT];
	xarray_struct arrWindows;
	xarray_struct arrFloatingWindows;
	xge_xui_menu_t tOptionMenu;
	xge_xui_menu_t tOverflowMenu;
	xge_xui_dock_window pDragWindow;
	xge_xui_dock_window pPendingFocusWindow;
	xge_xui_dock_pane pDragSourcePane;
	xge_xui_dock_pane pOptionMenuPane;
	xge_xui_dock_pane pOverflowMenuPane;
	xge_xui_dock_pane pHoverPane;
	xge_xui_dock_window pAutoHideHoverWindow;
	xge_xui_dock_window pAutoHideActiveWindow;
	xge_xui_dock_window pAutoHideExpandWindow;
	xge_xui_dock_region pHoverRegion;
	xge_xui_dock_region pSplitterDragRegion;
	xge_xui_dock_node pSplitterDragNode;
	xge_rect_t tPreviewRect;
	xge_rect_t tIndicatorRect;
	xge_rect_t tAutoHideExpandRect;
	xge_rect_t tAutoHideExpandCaptionRect;
	xge_rect_t tAutoHideExpandClientRect;
	xge_rect_t tAutoHideExpandDockRect;
	xge_rect_t tAutoHideExpandCloseRect;
	xge_vec2_t tDragStartMouse;
	xge_vec2_t tDragLastMouse;
	xge_vec2_t tSplitterDragStartMouse;
	float fSidePortionDefault;
	float fSplitterSize;
	float fSplitterDragStartRatio;
	float fTabStripHeight;
	float fCaptionHeight;
	float fButtonWidth;
	float fButtonHeight;
	int iDragPhase;
	int iHoverSide;
	int iHoverTabIndex;
	int iDragSourceIndex;
	int iActiveButton;
	int iAutoHideExpandHoverPart;
	int iAutoHideExpandActivePart;
	int bOptionMenuInit;
	int bOverflowMenuInit;
};

struct xge_xui_dock_window_t {
	xge_xui_context pContext;
	xge_xui_dock_layout pLayout;
	xge_xui_dock_pane pPane;
	xge_xui_widget pWindowWidget;
	xge_xui_widget pClientWidget;
	xge_xui_widget pContentWidget;
	xge_xui_window_t tWindow;
	xge_rect_t tLastFloatRect;
	xge_rect_t tLastDockRect;
	xge_rect_t tAutoHideStripRect;
	xui_texture pIconTexture;
	xge_rect_t tIconSrc;
	const char* sTitle;
	int iState;
	int iLastRegion;
	int iLastSide;
	int iLastTabIndex;
	int iAutoHideRegion;
	int bClosable;
	int bDockable;
	int bVisible;
};

typedef int (*xge_scene_proc)(void* pUser);

XGE_API int xgeInit(const xge_desc_t* pDesc);
XGE_API void xgeUnit(void);
XGE_API void xgeMemoryFree(void* pData);
XGE_API int xgeRun(xge_scene_proc procFrame, void* pUser);
XGE_API void xgeQuit(void);
XGE_API int xgeFrame(void);
XGE_API int xgeRender(void);
XGE_API void xgeFrameStatsReset(void);
XGE_API xge_frame_stats_t xgeFrameStatsGet(void);
#if XGE_HAS_DEBUGMODE
/* Debug APIs are only declared for xgedbg builds.
 * Build with -DXGE_DEBUGMODE=1 and link xgedbg instead of xge. */
XGE_API int xgeDebugGetStats(xge_debug_stats_t* pStats);
XGE_API int xgeDebugDumpCaps(char* sBuffer, int iSize);
#endif
XGE_API int xgeLogSetLevel(int iLevel);
XGE_API int xgeLogGetLevel(void);
XGE_API int xgeLogWrite(int iLevel, const char* sTag, const char* sMessage);
XGE_API int xgeLogFlush(void);
XGE_API xge_platform_backend_t xgePlatformBackendDefault(void);
XGE_API int xgePlatformBackendSet(const xge_platform_backend_t* pBackend);
XGE_API xge_platform_backend_t xgePlatformBackendGet(void);
XGE_API xge_graphics_backend_t xgeGraphicsBackendDefault(void);
XGE_API int xgeGraphicsBackendSet(const xge_graphics_backend_t* pBackend);
XGE_API xge_graphics_backend_t xgeGraphicsBackendGet(void);
XGE_API int xgeGpuCapsGet(xge_gpu_caps_t* pCaps);
XGE_API int xgePlatformCapsGet(xge_platform_caps_t* pCaps);
XGE_API int xgePlatformRuntimeGet(xge_platform_runtime_t* pRuntime);
XGE_API int xgeGraphicsShaderHeaderGet(int iBackend, char* sBuffer, int iSize);
XGE_API int xgeGraphicsLibraryNameGet(int iBackend, int iIndex, char* sBuffer, int iSize);
XGE_API int xgeGraphicsMappingGet(int iBackend, xge_graphics_mapping_t* pMapping);
XGE_API int xgeSceneSet(xge_scene pScene);
XGE_API int xgeScenePush(xge_scene pScene);
XGE_API int xgeScenePop(void);
XGE_API int xgeSceneReplace(xge_scene pScene);
XGE_API xge_scene xgeSceneCurrent(void);
XGE_API int xgeSceneCount(void);
XGE_API int xgeSceneDispatchEvent(const xge_event_t* pEvent);
XGE_API int xgeSceneUpdateStrategySet(int iMode, float fFixedStep, int iMaxUpdates);
XGE_API void xgeSceneUpdateStrategyGet(int* pMode, float* pFixedStep, int* pMaxUpdates);

XGE_API int xgeGetWidth(void);
XGE_API int xgeGetHeight(void);
XGE_API float xgeGetDelta(void);
XGE_API int xgeGetFPS(void);
XGE_API double xgeTimer(void);
XGE_API int64_t xgeTimeNow(void);
XGE_API void xgeSleep(uint32_t iMilliseconds);
XGE_API void xgeSetTitle(const char* sTitle);

XGE_API int xgeBegin(void);
XGE_API int xgeEnd(void);
XGE_API int xgeFlush(void);
XGE_API int xgeRenderThreadCapsGet(xge_render_thread_caps_t* pCaps);
XGE_API int xgeRenderThreadEGLSet(const xge_egl_desc_t* pDesc);
XGE_API int xgeRenderThreadSet(int bEnabled);
XGE_API int xgeRenderThreadGet(void);
XGE_API void xgeClear(uint32_t iColor);
XGE_API void xgePresent(void);
XGE_API void xgeInvalidateRect(xge_rect_t tRect);
#if XGE_HAS_DEBUGMODE
/* Dirty rect inspection is a debug feature; xge keeps only the runtime
 * invalidation mechanism. Build with -DXGE_DEBUGMODE=1 and link xgedbg. */
XGE_API int xgedbgDirtyRectCount(void);
XGE_API int xgedbgDirtyRectGet(int iIndex, xge_rect_t* pRect);
XGE_API void xgedbgDirtyRectClear(void);
#endif
XGE_API uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA);
XGE_API xge_color_t xgeColorUnpack(uint32_t iColor);
XGE_API void xgeBlendSet(int iBlend);
XGE_API int xgeBlendGet(void);
XGE_API void xgeDepthTestSet(int bEnabled);
XGE_API int xgeDepthTestGet(void);
XGE_API xge_camera_t xgeCameraDefault(float fWidth, float fHeight);
XGE_API xge_camera_t xgeCameraPerspective(float fWidth, float fHeight, float fFovY, float fNearZ, float fFarZ);
XGE_API void xgeCameraSet(const xge_camera_t* pCamera);
XGE_API xge_camera_t xgeCameraGet(void);
XGE_API xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint);
XGE_API xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint);
XGE_API int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider);
XGE_API void xgeResourceProviderClear(void);
XGE_API int xgeResourceXPackProviderAdd(const xge_xpack_provider_t* pProvider);
XGE_API int xgeResourceLoad(const char* sURI, xge_resource_t* pResource);
XGE_API int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource);
XGE_API void xgeResourceFree(xge_resource_t* pResource);
XGE_API int xgeMiniProgramInit(const xge_miniprogram_desc_t* pDesc);
XGE_API int xgeMiniProgramInitSimple(int iWidth, int iHeight, float fDevicePixelRatio);
XGE_API void xgeMiniProgramUnit(void);
XGE_API int xgeMiniProgramSetBridge(const xge_miniprogram_bridge_t* pBridge);
XGE_API int xgeMiniProgramFrame(double fTimeSeconds);
XGE_API int xgeMiniProgramResize(int iWidth, int iHeight, float fDevicePixelRatio);
XGE_API int xgeMiniProgramTouch(int iPhase, const xge_miniprogram_touch_t* pTouches, int iCount);
XGE_API int xgeMiniProgramTouchOne(int iPhase, int iId, float fX, float fY, float fForce);
XGE_API int xgeMiniProgramText(uint32_t iCodepoint);
XGE_API int xgeMiniProgramRequestFrame(void);
XGE_API int xgeMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize);
XGE_API int xgeEGLCapsGet(xge_egl_caps_t* pCaps);
XGE_API int xgeEGLInit(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc);
XGE_API void xgeEGLUnit(xge_egl_context_t* pContext);
XGE_API int xgeEGLMakeCurrent(xge_egl_context_t* pContext);
XGE_API int xgeOffscreenInit(xge_offscreen pOffscreen, int iWidth, int iHeight);
XGE_API void xgeOffscreenUnit(xge_offscreen pOffscreen);
XGE_API xge_render_target xgeOffscreenRenderTarget(xge_offscreen pOffscreen);
XGE_API int xgeOffscreenReadPixels(xge_offscreen pOffscreen, void* pPixels, int iStride);
XGE_API void xgeAsyncRequestInit(xge_async_request pRequest);
XGE_API void xgeAsyncRequestFree(xge_async_request pRequest);
XGE_API int xgeAsyncRequestCancel(xge_async_request pRequest);
XGE_API int xgeAsyncThreadingSet(int bEnabled);
XGE_API int xgeAsyncThreadingGet(void);
XGE_API int xgeAsyncPoll(xge_async_request pRequest);
XGE_API int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAsyncTextureLoad(xge_async_request pRequest, xge_texture pTexture, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAsyncFontLoad(xge_async_request pRequest, xge_font pFont, const char* sPath, float fSize, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAsyncSoundLoad(xge_async_request pRequest, xge_sound pSound, const char* sPath, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAudioInit(void);
XGE_API void xgeAudioUnit(void);
XGE_API int xgeAudioIsReady(void);
XGE_API void xgeAudioSetVolume(float fVolume);
XGE_API float xgeAudioGetVolume(void);
XGE_API void xgeAudioListenerSet(const xge_audio_listener_t* pListener);
XGE_API xge_audio_listener_t xgeAudioListenerGet(void);
XGE_API int xgeAudioGroupInit(xge_audio_group pGroup);
XGE_API void xgeAudioGroupFree(xge_audio_group pGroup);
XGE_API void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume);
XGE_API float xgeAudioGroupGetVolume(xge_audio_group pGroup);
XGE_API void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeSoundLoad(xge_sound pSound, const char* sPath);
XGE_API int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup);
XGE_API int xgeSoundFallbackSet(const char* sPath);
XGE_API int xgeSoundFallbackGet(xge_sound pSound);
XGE_API void xgeSoundFallbackClear(void);
XGE_API int xgeSoundPlay(xge_sound pSound);
XGE_API int xgeSoundStop(xge_sound pSound);
XGE_API int xgeSoundPause(xge_sound pSound);
XGE_API int xgeSoundResume(xge_sound pSound);
XGE_API int xgeSoundAddRef(xge_sound pSound);
XGE_API void xgeSoundFree(xge_sound pSound);
XGE_API void xgeSoundSetLoop(xge_sound pSound, int bLoop);
XGE_API void xgeSoundSetVolume(xge_sound pSound, float fVolume);
XGE_API void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ);
XGE_API void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeSoundIsPlaying(xge_sound pSound);
XGE_API int xgeMusicLoad(xge_music pMusic, const char* sPath);
XGE_API int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup);
XGE_API int xgeMusicPlay(xge_music pMusic);
XGE_API int xgeMusicStop(xge_music pMusic);
XGE_API int xgeMusicPause(xge_music pMusic);
XGE_API int xgeMusicResume(xge_music pMusic);
XGE_API void xgeMusicFree(xge_music pMusic);
XGE_API void xgeMusicSetLoop(xge_music pMusic, int bLoop);
XGE_API void xgeMusicSetVolume(xge_music pMusic, float fVolume);
XGE_API void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeMusicIsPlaying(xge_music pMusic);
XGE_API int xgeStreamOpen(xge_stream pStream, const char* sPath);
XGE_API int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup);
XGE_API int xgeStreamPlay(xge_stream pStream);
XGE_API int xgeStreamStop(xge_stream pStream);
XGE_API int xgeStreamPause(xge_stream pStream);
XGE_API int xgeStreamResume(xge_stream pStream);
XGE_API void xgeStreamClose(xge_stream pStream);
XGE_API void xgeStreamSetLoop(xge_stream pStream, int bLoop);
XGE_API void xgeStreamSetVolume(xge_stream pStream, float fVolume);
XGE_API void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ);
XGE_API void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeStreamIsPlaying(xge_stream pStream);
XGE_API int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint);
XGE_API int xgeFontLoad(xge_font pFont, const char* sPath, float fSize);
XGE_API int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize);
XGE_API int xgeFontLoadXRF(xge_font pFont, const char* sPath);
XGE_API int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize);
XGE_API int xgeFontBuildXRFMemory(xge_font pFont, uint32_t iFirstCodepoint, uint32_t iCount, void** ppData, int* pSize);
XGE_API int xgeFontSaveXRF(xge_font pFont, const char* sPath, uint32_t iFirstCodepoint, uint32_t iCount);
XGE_API int xgeFontLoadCached(xge_font pFont, const char* sTTFPath, const char* sXRFPath, float fSize, uint32_t iFirstCodepoint, uint32_t iCount);
XGE_API int xgeFontAddRef(xge_font pFont);
XGE_API void xgeFontFree(xge_font pFont);
XGE_API void xgeFontSetFallback(xge_font pFont, xge_font pFallback);
XGE_API int xgeFontFallbackSet(const char* sPath, float fSize);
XGE_API int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize);
XGE_API int xgeFontFallbackGet(xge_font pFont, float fSize);
XGE_API void xgeFontFallbackClear(void);
XGE_API int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics);
XGE_API int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap);
XGE_API int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph);
XGE_API void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap);
XGE_API xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText);
XGE_API void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor);
XGE_API void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags);
XGE_API int xgeImageLoad(xge_image pImage, const char* sPath);
XGE_API int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags);
XGE_API int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize);
XGE_API int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags);
XGE_API void* xgeImageGetPixels(xge_image pImage);
XGE_API void xgeImagePremultiply(xge_image pImage);
XGE_API int xgeImageSavePNG(const char* sPath, int iWidth, int iHeight, const void* pPixels, int iStride);
XGE_API void xgeImageFree(xge_image pImage);
XGE_API int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels);
XGE_API int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage);
XGE_API int xgeTextureLoad(xge_texture pTexture, const char* sPath);
XGE_API int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags);
XGE_API int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize);
XGE_API int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags);
XGE_API int xgeTextureAddRef(xge_texture pTexture);
XGE_API int xgeTextureUpdateRGBA(xge_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride);
XGE_API xge_sampler_t xgeSamplerDefault(void);
XGE_API int xgeTextureSetSampler(xge_texture pTexture, const xge_sampler_t* pSampler);
XGE_API xge_sampler_t xgeTextureGetSampler(xge_texture pTexture);
XGE_API int xgeTextureFallbackSetRGBA(int iWidth, int iHeight, const void* pPixels);
XGE_API int xgeTextureFallbackGet(xge_texture pTexture);
XGE_API void xgeTextureFallbackClear(void);
XGE_API int xgeTextureUploadQueue(xge_texture pTexture);
XGE_API int xgeTextureUploadFlush(void);
XGE_API int xgeTextureReadPixels(xge_texture pTexture, void* pPixels, int iStride);
XGE_API void xgeTextureFree(xge_texture pTexture);
XGE_API int xgeRenderTargetWindow(xge_render_target pTarget);
XGE_API int xgeRenderTargetCreate(xge_render_target pTarget, int iWidth, int iHeight);
XGE_API int xgeRenderTargetResize(xge_render_target pTarget, int iWidth, int iHeight);
XGE_API int xgeRenderTargetReadPixels(xge_render_target pTarget, void* pPixels, int iStride);
XGE_API xge_texture xgeRenderTargetTexture(xge_render_target pTarget);
XGE_API void xgeRenderTargetFree(xge_render_target pTarget);
XGE_API int xgeBufferCreate(xge_buffer pBuffer, int iType, int iUsage, const void* pData, int iSize);
XGE_API int xgeBufferUpdate(xge_buffer pBuffer, int iOffset, const void* pData, int iSize);
XGE_API int xgeBufferUpload(xge_buffer pBuffer);
XGE_API void xgeBufferFree(xge_buffer pBuffer);
XGE_API void xgePassInit(xge_pass pPass, xge_render_target pTarget, uint32_t iClearFlags, uint32_t iClearColor);
XGE_API int xgePassBegin(xge_pass pPass);
XGE_API int xgePassEnd(xge_pass pPass);
XGE_API int xgeShaderCreate(xge_shader pShader, const char* sVertexSource, const char* sFragmentSource);
XGE_API int xgeShaderAddRef(xge_shader pShader);
XGE_API void xgeShaderFree(xge_shader pShader);
XGE_API int xgeShaderUniform1f(xge_shader pShader, const char* sName, float fX);
XGE_API int xgeShaderUniform2f(xge_shader pShader, const char* sName, float fX, float fY);
XGE_API int xgeShaderUniform3f(xge_shader pShader, const char* sName, float fX, float fY, float fZ);
XGE_API int xgeShaderUniform4f(xge_shader pShader, const char* sName, float fX, float fY, float fZ, float fW);
XGE_API int xgeShaderVariantSetInit(xge_shader_variant_set pSet, const char* sVertexSource, const char* sFragmentSource);
XGE_API void xgeShaderVariantSetFree(xge_shader_variant_set pSet);
XGE_API int xgeShaderVariantGet(xge_shader_variant_set pSet, uint32_t iKey, const xge_shader_define_t* pDefines, int iDefineCount, xge_shader* ppShader);
XGE_API void xgeMaterialInit(xge_material pMaterial);
XGE_API void xgeMaterialFree(xge_material pMaterial);
XGE_API void xgeMaterialSetShader(xge_material pMaterial, xge_shader pShader);
XGE_API void xgeMaterialSetTexture(xge_material pMaterial, xge_texture pTexture);
XGE_API void xgeMaterialSetColor(xge_material pMaterial, uint32_t iColor);
XGE_API void xgeMaterialSetBlend(xge_material pMaterial, int iBlend);
XGE_API void xgeMaterialDraw(xge_material pMaterial, const xge_draw_t* pDraw);
XGE_API int xgeMeshCreate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount, uint32_t iFlags);
XGE_API int xgeMeshUpdate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount);
XGE_API void xgeMeshFree(xge_mesh pMesh);
XGE_API void xgeMeshDraw(xge_mesh pMesh, xge_texture pTexture, uint32_t iFlags);
XGE_API void xgeDraw(xge_texture pTexture, float fX, float fY);
XGE_API void xgeDrawEx(const xge_draw_t* pDraw);
XGE_API void xgeDrawQuad3D(xge_texture pTexture, const xge_vertex_t* pVertices, uint32_t iFlags);
XGE_API void xgeDrawPx(xge_texture pTexture, int iX, int iY);
XGE_API void xgeNinePatchInitSimple(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeNinePatchInit(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc, float fX1, float fY1, float fX2, float fY2);
XGE_API void xgeNinePatchSetMode(xge_nine_patch pPatch, int iMode);
XGE_API void xgeNinePatchSetColor(xge_nine_patch pPatch, uint32_t iColor);
XGE_API void xgeNinePatchDraw(const xge_nine_patch_t* pPatch, xge_rect_t tDst, uint32_t iFlags);
XGE_API int xgeSpriteBatchInit(xge_sprite_batch pBatch, xge_texture pTexture, int iCapacity, uint32_t iFlags);
XGE_API void xgeSpriteBatchFree(xge_sprite_batch pBatch);
XGE_API void xgeSpriteBatchClear(xge_sprite_batch pBatch);
XGE_API int xgeSpriteBatchAdd(xge_sprite_batch pBatch, const xge_draw_t* pDraw);
XGE_API int xgeSpriteBatchFlush(xge_sprite_batch pBatch);
XGE_API void xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor);
XGE_API void xgeShapePointPx(float fX, float fY, float fSize, uint32_t iColor);
XGE_API void xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
XGE_API void xgeShapeLinePx(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
XGE_API void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor);
XGE_API void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor);
XGE_API void xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor);
XGE_API void xgeShapeRectStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor);
XGE_API void xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor);
XGE_API void xgeShapeCircleFillPx(float fX, float fY, float fRadius, uint32_t iColor);
XGE_API void xgeShapeCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
XGE_API void xgeShapeCircleStrokePx(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
XGE_API void xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
XGE_API void xgeShapeArcPx(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
XGE_API void xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
XGE_API void xgeShapeTriangleFillPx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
XGE_API void xgeShapePolygonFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
XGE_API void xgeShapePolygonFillPx(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
XGE_API int xgeShapeBatchInit(xge_shape_batch pBatch, uint32_t iColor, int iTriangleCapacity, uint32_t iFlags);
XGE_API void xgeShapeBatchFree(xge_shape_batch pBatch);
XGE_API void xgeShapeBatchClear(xge_shape_batch pBatch);
XGE_API int xgeShapeBatchTriangleFill(xge_shape_batch pBatch, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC);
XGE_API int xgeShapeBatchRectFill(xge_shape_batch pBatch, xge_rect_t tRect);
XGE_API int xgeShapeBatchFlush(xge_shape_batch pBatch);
XGE_API void xgeViewportSet(xge_rect_t tRect);
XGE_API xge_rect_t xgeViewportGet(void);
XGE_API void xgeViewportClear(void);
XGE_API void xgeClipSet(xge_rect_t tRect);
XGE_API xge_rect_t xgeClipGet(void);
XGE_API void xgeClipClear(void);
XGE_API void xgeClipboardSetText(const char* sText);
XGE_API const char* xgeClipboardGetText(void);

XGE_API int xgeKeyDown(int iKey);
XGE_API int xgeKeyPressed(int iKey);
XGE_API int xgeKeyReleased(int iKey);
XGE_API void xgeMouseGet(float* pX, float* pY);
XGE_API void xgeMouseGetDelta(float* pDX, float* pDY);
XGE_API void xgeMouseGetWheel(float* pX, float* pY);
XGE_API int xgeMouseDown(int iButton);
XGE_API uint32_t xgeTextGet(void);
XGE_API int xgeImeGetEnabled(void);
XGE_API int xgeImeSetEnabled(int bEnabled);
XGE_API int xgeTouchGetCount(void);
XGE_API int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint);
XGE_API int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint);
XGE_API int xgeGamepadConnected(int iGamepad);
XGE_API int xgeGamepadGetState(int iGamepad, xge_gamepad_state_t* pState);
XGE_API int xgeGamepadButtonDown(int iGamepad, uint32_t iButton);
XGE_API int xgeGamepadButtonPressed(int iGamepad, uint32_t iButton);
XGE_API int xgeGamepadButtonReleased(int iGamepad, uint32_t iButton);
XGE_API float xgeGamepadAxis(int iGamepad, int iAxis);
XGE_API int xgeGamepadSetConnected(int iGamepad, int bConnected);
XGE_API int xgeGamepadSetState(int iGamepad, const xge_gamepad_state_t* pState);

XGE_API xge_xui_size_t xgeXuiSizePx(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeDip(float fValue);
XGE_API xge_xui_size_t xgeXuiSizePercent(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeGrow(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeContent(void);
XGE_API int xgeXuiInit(xge_xui_context pContext);
XGE_API void xgeXuiUnit(xge_xui_context pContext);
XGE_API void xgeXuiSetProcFrameEventDispatch(xge_xui_context pContext, int bEnabled);
XGE_API xge_xui_widget xgeXuiRoot(xge_xui_context pContext);
XGE_API xge_xui_widget xgeXuiOverlayRoot(xge_xui_context pContext);
XGE_API int xgeXuiOverlayAttach(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_widget pOwner, int iLayer);
XGE_API void xgeXuiOverlayDetach(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiOverlayBringToFront(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiOverlayGetOwner(xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiOverlayTop(xge_xui_context pContext);
XGE_API void xgeXuiSetDipScale(xge_xui_context pContext, float fScale);
XGE_API float xgeXuiGetDipScale(xge_xui_context pContext);
XGE_API void xgeXuiSetSafeAreaPx(xge_xui_context pContext, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiThemeDefault(xge_xui_theme pTheme);
XGE_API void xgeXuiSetTheme(xge_xui_context pContext, const xge_xui_theme_t* pTheme);
XGE_API const xge_xui_theme_t* xgeXuiGetTheme(xge_xui_context pContext);
XGE_API uint32_t xgeXuiGetThemeVersion(xge_xui_context pContext);
XGE_API int xgeXuiTokenSetColor(xge_xui_context pContext, const char* sName, uint32_t iColor);
XGE_API int xgeXuiTokenSetSpacing(xge_xui_context pContext, const char* sName, float fValue);
XGE_API int xgeXuiTokenSetFont(xge_xui_context pContext, const char* sName, xui_font pFont);
XGE_API int xgeXuiTokenSetTexture(xge_xui_context pContext, const char* sName, xui_texture pTexture);
XGE_API void xgeXuiStyleDefault(xge_xui_style_t* pStyle);
XGE_API void xgeXuiStyleFromTheme(xge_xui_style_t* pStyle, const xge_xui_theme_t* pTheme);
XGE_API void xgeXuiRefreshRequest(xge_xui_context pContext);
XGE_API int xgeXuiRefreshNeeded(xge_xui_context pContext);
XGE_API void xgeXuiRefreshClear(xge_xui_context pContext);
XGE_API void xgeXuiLayoutBatchBegin(xge_xui_context pContext);
XGE_API void xgeXuiLayoutBatchEnd(xge_xui_context pContext);
XGE_API void xgeXuiSetHost(xge_xui_context pContext, const xge_xui_host_t* pHost);
XGE_API const xge_xui_host_t* xgeXuiGetHost(xge_xui_context pContext);
XGE_API void xgeXuiSetHostV2(xge_xui_context pContext, const xge_xui_host_v2_t* pHost);
XGE_API const xge_xui_host_v2_t* xgeXuiGetHostV2(xge_xui_context pContext);
XGE_API const xge_xui_host_v2_t* xgeXuiHostV2Null(void);
XGE_API const xge_xui_host_v2_t* xgeXuiHostV2Xge(void);
XGE_API int xgeXuiFontCreateFile(xge_xui_context pContext, const char* sName, const char* sPath, float fSize, xui_font* pFont);
XGE_API int xgeXuiFontCreateMemory(xge_xui_context pContext, const char* sName, const void* pData, int iSize, float fSize, xui_font* pFont);
XGE_API int xgeXuiFontRegister(xge_xui_context pContext, const char* sName, xui_font pFont);
XGE_API xui_font xgeXuiFontGet(xge_xui_context pContext, const char* sName);
XGE_API void xgeXuiFontDestroy(xge_xui_context pContext, xui_font pFont);
XGE_API void xgeXuiSetDefaultFont(xge_xui_context pContext, xui_font pFont);
XGE_API xui_font xgeXuiGetDefaultFont(xge_xui_context pContext);
XGE_API int xgeXuiTextureCreateRGBA(xge_xui_context pContext, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags, xui_texture* pTexture);
XGE_API int xgeXuiTextureCreateMemory(xge_xui_context pContext, const void* pData, int iSize, uint32_t iFlags, xui_texture* pTexture);
XGE_API int xgeXuiTextureCreateFile(xge_xui_context pContext, const char* sPath, uint32_t iFlags, xui_texture* pTexture);
XGE_API int xgeXuiTextureGetDesc(xge_xui_context pContext, xui_texture pTexture, xui_texture_desc_t* pDesc);
XGE_API void xgeXuiTextureDestroy(xge_xui_context pContext, xui_texture pTexture);
XGE_API int xgeXuiPageLoad(xge_xui_context pContext, const char* sURI, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
XGE_API int xgeXuiPageLoadMemory(xge_xui_context pContext, const void* pData, int iSize, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
XGE_API void xgeXuiPageUnload(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageRoot(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageFind(xge_xui_page_t* pPage, const char* sId);
XGE_API uint32_t xgeXuiPageStyleVersion(xge_xui_page_t* pPage);
XGE_API int xgeXuiPageRefreshStyle(xge_xui_page_t* pPage);
XGE_API int xgeXuiPageSyncStyle(xge_xui_page_t* pPage);
XGE_API int xgeXuiPageApplyModel(xge_xui_page_t* pPage, const xge_xui_model_t* pModel);
XGE_API const char* xgeXuiPageGetError(xge_xui_page_t* pPage);
XGE_API void xgeXuiBinderInit(xge_xui_binder_t* pBinder);
XGE_API int xgeXuiBinderSetClick(xge_xui_binder_t* pBinder, const char* sName, xge_xui_click_proc procClick, void* pUser);
XGE_API int xgeXuiBinderSetEvent(xge_xui_binder_t* pBinder, const char* sName, xge_xui_event_proc procEvent, void* pUser);
XGE_API void xgeXuiModelInit(xge_xui_model_t* pModel);
XGE_API int xgeXuiModelSetText(xge_xui_model_t* pModel, const char* sKey, const char* sValue);
XGE_API int xgeXuiModelSetInt(xge_xui_model_t* pModel, const char* sKey, int iValue);
XGE_API int xgeXuiModelSetFloat(xge_xui_model_t* pModel, const char* sKey, float fValue);
XGE_API const char* xgeXuiModelGetText(const xge_xui_model_t* pModel, const char* sKey);
XGE_API uint32_t xgeXuiModelVersion(const xge_xui_model_t* pModel);
XGE_API xge_xui_widget xgeXuiWidgetCreate(void);
XGE_API void xgeXuiWidgetFree(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
#if defined(XGE_BUILD_DLL)
int xgeXuiWidgetAddInternal(xge_xui_widget pParent, xge_xui_widget pChild);
#endif
XGE_API void xgeXuiWidgetRemove(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetId(xge_xui_widget pWidget, int iId);
XGE_API int xgeXuiWidgetGetId(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetName(xge_xui_widget pWidget, const char* sName);
XGE_API const char* xgeXuiWidgetGetName(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetRole(xge_xui_widget pWidget, int iRole);
XGE_API int xgeXuiWidgetGetRole(xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget pRoot, int iId);
XGE_API xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget pRoot, const char* sName);
XGE_API void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect);
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
XGE_API xge_rect_t xgeXuiWidgetGetOuterRect(xge_xui_widget pWidget);
XGE_API xge_rect_t xgeXuiWidgetGetBorderRect(xge_xui_widget pWidget);
XGE_API xge_rect_t xgeXuiWidgetGetPaddingRect(xge_xui_widget pWidget);
XGE_API xge_rect_t xgeXuiWidgetGetContentRect(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetStyle(xge_xui_widget pWidget, const xge_xui_style_t* pStyle);
XGE_API const xge_xui_style_t* xgeXuiWidgetGetStyle(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout);
XGE_API void xgeXuiWidgetSetDock(xge_xui_widget pWidget, int iDock);
XGE_API int xgeXuiWidgetGetDock(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetMinSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetMaxSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetGrid(xge_xui_widget pWidget, int iColumns, float fRowHeight, float fColumnGap, float fRowGap);
XGE_API void xgeXuiWidgetSetGridColumnSpan(xge_xui_widget pWidget, int iColumnSpan);
XGE_API void xgeXuiWidgetSetGap(xge_xui_widget pWidget, float fGap);
XGE_API void xgeXuiWidgetSetAlign(xge_xui_widget pWidget, int iAlignX, int iAlignY);
XGE_API void xgeXuiWidgetSetJustify(xge_xui_widget pWidget, int iJustify);
XGE_API void xgeXuiWidgetSetZ(xge_xui_widget pWidget, int iZ);
XGE_API int xgeXuiWidgetGetZ(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetLayer(xge_xui_widget pWidget, int iLayer);
XGE_API int xgeXuiWidgetGetLayer(xge_xui_widget pWidget);
XGE_API uint32_t xgeXuiWidgetGetTreeOrder(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetOverflow(xge_xui_widget pWidget, int iOverflow);
XGE_API int xgeXuiWidgetGetOverflow(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetAnchorPx(xge_xui_widget pWidget, int iAnchor, float fLeft, float fTop, float fRight, float fBottom);
XGE_API int xgeXuiWidgetGetAnchor(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor);
XGE_API void xgeXuiWidgetSetRadius(xge_xui_widget pWidget, float fRadius);
XGE_API void xgeXuiWidgetSetBorder(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
XGE_API void xgeXuiWidgetSetFocusRing(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
XGE_API void xgeXuiWidgetSetDisabledOverlay(xge_xui_widget pWidget, uint32_t iColor);
XGE_API void xgeXuiWidgetSetDebugOutline(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
XGE_API void xgeXuiWidgetSetStateStyle(xge_xui_widget pWidget, int iState, const xge_xui_state_style_t* pStyle);
XGE_API const xge_xui_state_style_t* xgeXuiWidgetGetStateStyle(xge_xui_widget pWidget, int iState);
XGE_API void xgeXuiWidgetClearStateStyle(xge_xui_widget pWidget, int iState);
XGE_API void xgeXuiWidgetSetStateBackground(xge_xui_widget pWidget, int iState, uint32_t iColor);
XGE_API void xgeXuiWidgetSetStateBorder(xge_xui_widget pWidget, int iState, float fWidth, uint32_t iColor);
XGE_API void xgeXuiWidgetSetStateFocusRing(xge_xui_widget pWidget, int iState, float fWidth, uint32_t iColor);
XGE_API void xgeXuiWidgetSetStateDisabledOverlay(xge_xui_widget pWidget, int iState, uint32_t iColor);
XGE_API void xgeXuiWidgetSetVisualState(xge_xui_widget pWidget, int iState);
XGE_API int xgeXuiWidgetGetVisualState(xge_xui_widget pWidget);
XGE_API uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible);
XGE_API void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled);
XGE_API void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable);
XGE_API void xgeXuiWidgetSetTabStop(xge_xui_widget pWidget, int bTabStop);
XGE_API int xgeXuiWidgetIsTabStop(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetTabIndex(xge_xui_widget pWidget, int iTabIndex);
XGE_API int xgeXuiWidgetGetTabIndex(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetFocusScope(xge_xui_widget pWidget, int bFocusScope);
XGE_API int xgeXuiWidgetIsFocusScope(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetDefaultAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser);
XGE_API void xgeXuiWidgetSetCancelAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser);
XGE_API void xgeXuiWidgetSetImeMode(xge_xui_widget pWidget, int iImeMode);
XGE_API int xgeXuiWidgetGetImeMode(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetImeCandidateRect(xge_xui_widget pWidget, xge_xui_ime_candidate_rect_proc procRect, void* pUser);
XGE_API xge_rect_t xgeXuiWidgetGetImeCandidateRect(xge_xui_widget pWidget);
XGE_API int xgeXuiHasImeCandidateRect(xge_xui_context pContext);
XGE_API xge_rect_t xgeXuiGetImeCandidateRect(xge_xui_context pContext);
XGE_API void xgeXuiWidgetSetHitTestVisible(xge_xui_widget pWidget, int bVisible);
XGE_API int xgeXuiWidgetIsHitTestVisible(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetInputTransparent(xge_xui_widget pWidget, int bTransparent);
XGE_API int xgeXuiWidgetIsInputTransparent(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip);
XGE_API void xgeXuiWidgetSetDragEnabled(xge_xui_widget pWidget, int bEnabled);
XGE_API int xgeXuiWidgetIsDragEnabled(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent, void* pUser);
XGE_API void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent);
XGE_API void xgeXuiWidgetSetCaptureEventUser(xge_xui_widget pWidget, xge_xui_event_proc procEvent, void* pUser);
XGE_API void xgeXuiWidgetSetEventHandler(xge_xui_widget pWidget, int iEventType, xge_xui_event_proc procEvent, void* pUser);
XGE_API void xgeXuiWidgetSetEventInterest(xge_xui_widget pWidget, uint32_t iEventMask, int bEnabled);
XGE_API uint32_t xgeXuiWidgetGetEventMask(xge_xui_widget pWidget);
XGE_API uint32_t xgeXuiWidgetGetSubtreeEventMask(xge_xui_widget pWidget);
XGE_API int xgeXuiHotKeyRegister(xge_xui_context pContext, xge_xui_widget pWidget, int iKey, int iModifiers, xge_xui_event_proc procEvent, void* pUser);
XGE_API int xgeXuiHotKeyRegisterCommand(xge_xui_context pContext, xge_xui_widget pWidget, int iKey, int iModifiers, int iCommand, const char* sCommand, void* pData);
XGE_API void xgeXuiHotKeyUnregister(xge_xui_context pContext, xge_xui_widget pWidget, int iKey, int iModifiers);
XGE_API void xgeXuiHotKeyClearWidget(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API int xgeXuiCommandDispatch(xge_xui_context pContext, xge_xui_widget pTarget, xge_xui_widget pSource, int iCommand, const char* sCommand, void* pData);
XGE_API void xgeXuiWidgetSetUpdate(xge_xui_widget pWidget, xge_xui_update_proc procUpdate, void* pUser);
XGE_API void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure);
XGE_API void xgeXuiWidgetSetMeasureUser(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure, void* pUser);
XGE_API void xgeXuiWidgetSetLayoutProc(xge_xui_widget pWidget, xge_xui_layout_proc procLayout, void* pUser);
XGE_API void xgeXuiWidgetSetPaintBefore(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
XGE_API void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
XGE_API void xgeXuiWidgetSetPaintAfter(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
XGE_API void xgeXuiWidgetSetOwnerDraw(xge_xui_widget pWidget, int iMode, xge_xui_owner_draw_proc procDraw, void* pUser);
XGE_API void xgeXuiWidgetSetOwnerDrawControl(xge_xui_widget pWidget, void* pControl);
XGE_API int xgeXuiWidgetGetOwnerDrawMode(xge_xui_widget pWidget);
XGE_API xge_vec2_t xgeXuiWidgetGetDesiredSize(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsVisible(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetTooltipText(xge_xui_widget pWidget, const char* sText);
XGE_API void xgeXuiWidgetSetTooltip(xge_xui_widget pWidget, const xge_xui_tooltip_desc_t* pDesc);
XGE_API void xgeXuiWidgetSetTooltipResolver(xge_xui_widget pWidget, xge_xui_tooltip_resolve_proc procResolve, void* pUser);
XGE_API void xgeXuiWidgetClearTooltip(xge_xui_widget pWidget);
XGE_API const xge_xui_tooltip_desc_t* xgeXuiWidgetGetTooltip(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetTooltipIsOpen(xge_xui_context pContext);
XGE_API xge_xui_widget xgeXuiWidgetTooltipGetOwner(xge_xui_context pContext);
XGE_API xge_rect_t xgeXuiWidgetTooltipGetRect(xge_xui_context pContext);
XGE_API void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkStyle(xge_xui_widget pWidget);
#if XGE_HAS_DEBUGMODE
/* XUI inspection APIs are debug-only and exported by xgedbg builds. */
#define XGEDBG_XUI_OVERLAY_RECTS		0x00000001u
#define XGEDBG_XUI_OVERLAY_CONTENT_RECTS	0x00000002u
#define XGEDBG_XUI_OVERLAY_FOCUS		0x00000004u
#define XGEDBG_XUI_OVERLAY_HOVER		0x00000008u
#define XGEDBG_XUI_OVERLAY_CAPTURE		0x00000010u
#define XGEDBG_XUI_OVERLAY_LABELS		0x00000020u
#define XGEDBG_XUI_OVERLAY_MARGIN		0x00000040u
#define XGEDBG_XUI_OVERLAY_PADDING		0x00000080u
#define XGEDBG_XUI_OVERLAY_DIRTY		0x00000100u
#define XGEDBG_XUI_OVERLAY_DEFAULT		(XGEDBG_XUI_OVERLAY_RECTS | XGEDBG_XUI_OVERLAY_CONTENT_RECTS | XGEDBG_XUI_OVERLAY_FOCUS | XGEDBG_XUI_OVERLAY_HOVER | XGEDBG_XUI_OVERLAY_CAPTURE)

typedef struct xgedbg_xui_widget_info_t {
	xge_xui_widget pWidget;
	xge_xui_widget pParent;
	int iId;
	const char* sName;
	uint32_t iFlags;
	int iLayout;
	int iLayer;
	int iZ;
	uint32_t iTreeOrder;
	int iDepth;
	int iChildCount;
	int bVisible;
	int bEnabled;
	int bFocusable;
	int bTabStop;
	int bFocusScope;
	int iTabIndex;
	int iImeMode;
	int bClipped;
	int bHitTestVisible;
	int bInputTransparent;
	int bFocus;
	int bHover;
	int bCapture;
	int bDirtyStyle;
	int bDirtyLayout;
	int bDirtyPaint;
	xge_rect_t tRect;
	xge_rect_t tLocalRect;
	xge_rect_t tOuterRect;
	xge_rect_t tBorderRect;
	xge_rect_t tPaddingRect;
	xge_rect_t tContentRect;
	xge_vec2_t tDesiredSize;
	xge_xui_edges_t tMargin;
	xge_xui_edges_t tPadding;
	uint32_t iBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iFocusRingColor;
	uint32_t iDisabledOverlayColor;
	uint32_t iDebugOutlineColor;
	float fRadius;
	float fBorderWidth;
	float fFocusRingWidth;
	float fDebugOutlineWidth;
} xgedbg_xui_widget_info_t;

XGE_API int xgedbgXuiWidgetTreeDump(xge_xui_widget pRoot, char* sBuffer, int iSize);
XGE_API int xgedbgXuiLayoutSnapshot(xge_xui_context pContext, char* sBuffer, int iSize);
XGE_API int xgedbgXuiWidgetInspect(xge_xui_context pContext, xge_xui_widget pWidget, xgedbg_xui_widget_info_t* pInfo);
XGE_API int xgedbgXuiWidgetInspectAt(xge_xui_context pContext, float fX, float fY, xgedbg_xui_widget_info_t* pInfo);
XGE_API int xgedbgXuiDebugOverlayPaint(xge_xui_context pContext, uint32_t iFlags, xui_font pFont);
XGE_API int xgedbgXuiPageTrace(xge_xui_page_t* pPage, char* sBuffer, int iSize);
XGE_API int xgedbgXuiEventTrace(xge_xui_context pContext, const xge_event_t* pEvent, char* sBuffer, int iSize);
#endif
XGE_API xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY);
XGE_API void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSetPointerCapture(xge_xui_context pContext, uint64_t iPointerId, xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiGetPointerCapture(xge_xui_context pContext, uint64_t iPointerId);
XGE_API int xgeXuiHasCapture(xge_xui_context pContext);
XGE_API int xgeXuiWidgetHasCapture(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiReleaseWidgetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent);
XGE_API int xgeXuiDispatchProcFrameEventAll(const xge_event_t* pEvent);
XGE_API int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent);
XGE_API int xgeXuiEventPop(xge_xui_context pContext, xge_event_t* pEvent);
XGE_API int xgeXuiEventCount(xge_xui_context pContext);
XGE_API int xgeXuiDispatchQueuedEvents(xge_xui_context pContext);
XGE_API int xgeXuiUpdate(xge_xui_context pContext, float fDelta);
XGE_API int xgeXuiPaint(xge_xui_context pContext);
XGE_API int xgeXuiTextInit(xge_xui_text pText);
XGE_API void xgeXuiTextUnit(xge_xui_text pText);
XGE_API int xgeXuiTextSet(xge_xui_text pText, const char* sText);
XGE_API int xgeXuiTextInsert(xge_xui_text pText, const char* sText);
XGE_API int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint);
XGE_API int xgeXuiTextDeleteBack(xge_xui_text pText);
XGE_API int xgeXuiTextDeleteForward(xge_xui_text pText);
XGE_API void xgeXuiTextSetCursor(xge_xui_text pText, int iCursor);
XGE_API int xgeXuiTextGetCursor(xge_xui_text pText);
XGE_API void xgeXuiTextSetSelection(xge_xui_text pText, int iStart, int iEnd);
XGE_API void xgeXuiTextGetSelection(xge_xui_text pText, int* pStart, int* pEnd);
XGE_API int xgeXuiTextSetComposition(xge_xui_text pText, const char* sText);
XGE_API void xgeXuiTextClearComposition(xge_xui_text pText);
XGE_API const char* xgeXuiTextGetComposition(xge_xui_text pText);
XGE_API int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent);
XGE_API int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiButtonUnit(xge_xui_button pButton);
XGE_API void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser);
XGE_API void xgeXuiButtonSetText(xge_xui_button pButton, xui_font pFont, const char* sText);
XGE_API void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor);
XGE_API void xgeXuiButtonSetSelectable(xge_xui_button pButton, int bSelectable);
XGE_API void xgeXuiButtonSetSelected(xge_xui_button pButton, int bSelected);
XGE_API int xgeXuiButtonIsSelected(xge_xui_button pButton);
XGE_API void xgeXuiButtonSetSemantic(xge_xui_button pButton, int iSemantic);
XGE_API int xgeXuiButtonGetSemantic(xge_xui_button pButton);
XGE_API void xgeXuiButtonSetIcon(xge_xui_button pButton, xui_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiButtonSetIconColor(xge_xui_button pButton, uint32_t iColor);
XGE_API void xgeXuiButtonSetIconLayout(xge_xui_button pButton, int iPlacement, float fIconSize, float fGap);
XGE_API void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API void xgeXuiButtonSetPatch(xge_xui_button pButton, int iState, const xge_nine_patch_t* pPatch);
XGE_API void xgeXuiButtonClearPatch(xge_xui_button pButton, int iState);
XGE_API void xgeXuiButtonSetBadgeVisible(xge_xui_button pButton, int bVisible);
XGE_API void xgeXuiButtonSetBadgeAnchor(xge_xui_button pButton, int iAnchor);
XGE_API void xgeXuiButtonSetBadgeOffset(xge_xui_button pButton, float fX, float fY);
XGE_API void xgeXuiButtonSetBadgeSize(xge_xui_button pButton, float fSize);
XGE_API void xgeXuiButtonSetBadgeTexture(xge_xui_button pButton, xui_texture pTexture, xge_rect_t tSrc);
XGE_API int xgeXuiButtonGetState(xge_xui_button pButton);
XGE_API int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent);
XGE_API int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xui_font pFont, const char* sText);
XGE_API void xgeXuiLabelUnit(xge_xui_label pLabel);
XGE_API void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText);
XGE_API void xgeXuiLabelSetFont(xge_xui_label pLabel, xui_font pFont);
XGE_API void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor);
XGE_API void xgeXuiLabelSetDisabledColor(xge_xui_label pLabel, uint32_t iColor);
XGE_API void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags);
XGE_API void xgeXuiLabelSetUnderline(xge_xui_label pLabel, int bUnderline);
XGE_API xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel);
XGE_API xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xui_texture pTexture);
XGE_API void xgeXuiImageUnit(xge_xui_image pImage);
XGE_API void xgeXuiImageSetTexture(xge_xui_image pImage, xui_texture pTexture);
XGE_API void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc);
XGE_API void xgeXuiImageSetSourceRect(xge_xui_image pImage, float fX1, float fY1, float fX2, float fY2);
XGE_API void xgeXuiImageClearSource(xge_xui_image pImage);
XGE_API void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor);
XGE_API void xgeXuiImageSetTint(xge_xui_image pImage, uint32_t iColor);
XGE_API void xgeXuiImageSetMode(xge_xui_image pImage, int iMode);
XGE_API void xgeXuiImageSetAlign(xge_xui_image pImage, int iAlignX, int iAlignY);
XGE_API void xgeXuiImageSetCustomRect(xge_xui_image pImage, float fX1, float fY1, float fX2, float fY2);
XGE_API xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiInputUnit(xge_xui_input pInput);
XGE_API void xgeXuiInputSetText(xge_xui_input pInput, const char* sText);
XGE_API const char* xgeXuiInputGetText(xge_xui_input pInput);
XGE_API void xgeXuiInputSetFont(xge_xui_input pInput, xui_font pFont);
XGE_API void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
XGE_API void xgeXuiInputSetFrameColors(xge_xui_input pInput, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);
XGE_API void xgeXuiInputSetDisabledColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iBorder);
XGE_API void xgeXuiInputSetPlaceholder(xge_xui_input pInput, const char* sText);
XGE_API void xgeXuiInputSetChange(xge_xui_input pInput, xge_xui_text_submit_proc procChange, void* pUser);
XGE_API void xgeXuiInputSetSubmit(xge_xui_input pInput, xge_xui_text_submit_proc procSubmit, void* pUser);
XGE_API void xgeXuiInputSetFilter(xge_xui_input pInput, xge_xui_input_filter_proc procFilter, void* pUser);
XGE_API void xgeXuiInputSetErrorChange(xge_xui_input pInput, xge_xui_input_error_proc procError, void* pUser);
XGE_API void xgeXuiInputSetMaxLength(xge_xui_input pInput, int iMaxLength);
XGE_API int xgeXuiInputGetMaxLength(xge_xui_input pInput);
XGE_API void xgeXuiInputSetTextAlign(xge_xui_input pInput, int iAlign);
XGE_API int xgeXuiInputGetTextAlign(xge_xui_input pInput);
XGE_API void xgeXuiInputSetError(xge_xui_input pInput, int bError);
XGE_API int xgeXuiInputGetError(xge_xui_input pInput);
XGE_API void xgeXuiInputSetErrorColors(xge_xui_input pInput, uint32_t iBackground, uint32_t iBorder);
XGE_API xge_xui_input_decoration xgeXuiInputDecorationAdd(xge_xui_input pInput, int iSide, const xge_xui_input_decoration_desc_t* pDesc);
XGE_API void xgeXuiInputDecorationSet(xge_xui_input pInput, xge_xui_input_decoration pDecoration, const xge_xui_input_decoration_desc_t* pDesc);
XGE_API void xgeXuiInputDecorationRemove(xge_xui_input pInput, xge_xui_input_decoration pDecoration);
XGE_API void xgeXuiInputDecorationClear(xge_xui_input pInput, int iSide);
XGE_API xge_rect_t xgeXuiInputDecorationGetRect(xge_xui_input pInput, xge_xui_input_decoration pDecoration);
XGE_API void xgeXuiInputSetPassword(xge_xui_input pInput, int bPassword);
XGE_API void xgeXuiInputSetReadonly(xge_xui_input pInput, int bReadonly);
XGE_API void xgeXuiInputSetDisabled(xge_xui_input pInput, int bDisabled);
XGE_API void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd);
XGE_API void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd);
XGE_API xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput);
XGE_API int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent);
XGE_API int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiInputUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
XGE_API void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiColorPickerInit(xge_xui_color_picker pPicker, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiColorPickerUnit(xge_xui_color_picker pPicker);
XGE_API void xgeXuiColorPickerSetChange(xge_xui_color_picker pPicker, xge_xui_color_proc procChange, void* pUser);
XGE_API void xgeXuiColorPickerSetColor(xge_xui_color_picker pPicker, uint32_t iColor);
XGE_API uint32_t xgeXuiColorPickerGetColor(xge_xui_color_picker pPicker);
XGE_API void xgeXuiColorPickerSetRGBA(xge_xui_color_picker pPicker, int iR, int iG, int iB, int iA);
XGE_API void xgeXuiColorPickerGetRGBA(xge_xui_color_picker pPicker, int* pR, int* pG, int* pB, int* pA);
XGE_API void xgeXuiColorPickerSetAlphaEnabled(xge_xui_color_picker pPicker, int bEnabled);
XGE_API int xgeXuiColorPickerGetAlphaEnabled(xge_xui_color_picker pPicker);
XGE_API int xgeXuiColorPickerSetHex(xge_xui_color_picker pPicker, const char* sHex);
XGE_API const char* xgeXuiColorPickerGetHex(xge_xui_color_picker pPicker);
XGE_API void xgeXuiColorPickerSetPalette(xge_xui_color_picker pPicker, const uint32_t* pColors, int iCount);
XGE_API int xgeXuiColorPickerGetPaletteCount(xge_xui_color_picker pPicker);
XGE_API void xgeXuiColorPickerSetColors(xge_xui_color_picker pPicker, uint32_t iBackground, uint32_t iPanel, uint32_t iBorder, uint32_t iText, uint32_t iAccent, uint32_t iField);
XGE_API int xgeXuiColorPickerIsPopupOpen(xge_xui_color_picker pPicker);
XGE_API int xgeXuiColorPickerEvent(xge_xui_color_picker pPicker, const xge_event_t* pEvent);
XGE_API int xgeXuiColorPickerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiColorPickerPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiDatePickerInit(xge_xui_date_picker pPicker, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiDatePickerUnit(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetMode(xge_xui_date_picker pPicker, int iMode);
XGE_API int xgeXuiDatePickerGetMode(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetNullable(xge_xui_date_picker pPicker, int bNullable);
XGE_API int xgeXuiDatePickerGetNullable(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetValue(xge_xui_date_picker pPicker, xtime tValue);
XGE_API xtime xgeXuiDatePickerGetValue(xge_xui_date_picker pPicker);
XGE_API int xgeXuiDatePickerHasValue(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerClearValue(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetRangeValue(xge_xui_date_picker pPicker, xtime tStart, xtime tEnd);
XGE_API void xgeXuiDatePickerGetRangeValue(xge_xui_date_picker pPicker, xtime* pStart, xtime* pEnd);
XGE_API int xgeXuiDatePickerHasRangeValue(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetLimits(xge_xui_date_picker pPicker, xtime tMin, xtime tMax);
XGE_API void xgeXuiDatePickerClearLimits(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetFormat(xge_xui_date_picker pPicker, const char* sFormat);
XGE_API const char* xgeXuiDatePickerGetFormat(xge_xui_date_picker pPicker);
XGE_API void xgeXuiDatePickerSetRangeSeparator(xge_xui_date_picker pPicker, const char* sSeparator);
XGE_API void xgeXuiDatePickerSetShowSecond(xge_xui_date_picker pPicker, int bShowSecond);
XGE_API void xgeXuiDatePickerSetFirstDayOfWeek(xge_xui_date_picker pPicker, int iFirstDayOfWeek);
XGE_API void xgeXuiDatePickerSetDefaultRangeSpan(xge_xui_date_picker pPicker, xtime tSpan);
XGE_API void xgeXuiDatePickerSetChanging(xge_xui_date_picker pPicker, xge_xui_date_proc procChanging, void* pUser);
XGE_API void xgeXuiDatePickerSetChange(xge_xui_date_picker pPicker, xge_xui_date_proc procChange, void* pUser);
XGE_API void xgeXuiDatePickerSetCommit(xge_xui_date_picker pPicker, xge_xui_date_proc procCommit, void* pUser);
XGE_API void xgeXuiDatePickerSetCancel(xge_xui_date_picker pPicker, xge_xui_date_proc procCancel, void* pUser);
XGE_API void xgeXuiDatePickerSetClear(xge_xui_date_picker pPicker, xge_xui_date_proc procClear, void* pUser);
XGE_API void xgeXuiDatePickerSetColors(xge_xui_date_picker pPicker, uint32_t iBackground, uint32_t iPanel, uint32_t iHeader, uint32_t iGrid, uint32_t iText, uint32_t iSelected);
XGE_API int xgeXuiDatePickerIsPopupOpen(xge_xui_date_picker pPicker);
XGE_API int xgeXuiDatePickerEvent(xge_xui_date_picker pPicker, const xge_event_t* pEvent);
XGE_API int xgeXuiDatePickerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiDatePickerPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiDatePickerPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiDatePickerPopupPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiNumericInputInit(xge_xui_numeric_input pNumeric, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiNumericInputUnit(xge_xui_numeric_input pNumeric);
XGE_API void xgeXuiNumericInputSetChange(xge_xui_numeric_input pNumeric, xge_xui_slider_proc procChange, void* pUser);
XGE_API void xgeXuiNumericInputSetErrorChange(xge_xui_numeric_input pNumeric, xge_xui_input_error_proc procError, void* pUser);
XGE_API void xgeXuiNumericInputSetFormatter(xge_xui_numeric_input pNumeric, xge_xui_numeric_format_proc procFormat, void* pUser);
XGE_API void xgeXuiNumericInputSetRange(xge_xui_numeric_input pNumeric, float fMin, float fMax);
XGE_API void xgeXuiNumericInputSetStep(xge_xui_numeric_input pNumeric, float fStep);
XGE_API void xgeXuiNumericInputSetInteger(xge_xui_numeric_input pNumeric, int bInteger);
XGE_API void xgeXuiNumericInputSetPrecision(xge_xui_numeric_input pNumeric, int iPrecision);
XGE_API void xgeXuiNumericInputSetSpinnerVisible(xge_xui_numeric_input pNumeric, int bVisible);
XGE_API void xgeXuiNumericInputSetSpinnerWidth(xge_xui_numeric_input pNumeric, float fWidth);
XGE_API void xgeXuiNumericInputSetSpinnerColors(xge_xui_numeric_input pNumeric, uint32_t iColor, uint32_t iHoverColor, uint32_t iActiveColor, uint32_t iDisabledColor, uint32_t iIconColor, uint32_t iDisabledIconColor);
XGE_API void xgeXuiNumericInputSetValue(xge_xui_numeric_input pNumeric, float fValue);
XGE_API float xgeXuiNumericInputGetValue(xge_xui_numeric_input pNumeric);
XGE_API int xgeXuiNumericInputGetState(xge_xui_numeric_input pNumeric);
XGE_API int xgeXuiNumericInputEvent(xge_xui_numeric_input pNumeric, const xge_event_t* pEvent);
XGE_API int xgeXuiNumericInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiNumericInputUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
XGE_API void xgeXuiNumericInputPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTextEditInit(xge_xui_text_edit pEdit, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiTextEditUnit(xge_xui_text_edit pEdit);
XGE_API void xgeXuiTextEditSetText(xge_xui_text_edit pEdit, const char* sText);
XGE_API const char* xgeXuiTextEditGetText(xge_xui_text_edit pEdit);
XGE_API void xgeXuiTextEditSetFont(xge_xui_text_edit pEdit, xui_font pFont);
XGE_API void xgeXuiTextEditSetColors(xge_xui_text_edit pEdit, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
XGE_API void xgeXuiTextEditSetFrameColors(xge_xui_text_edit pEdit, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);
XGE_API void xgeXuiTextEditSetDisabledColors(xge_xui_text_edit pEdit, uint32_t iText, uint32_t iBackground, uint32_t iBorder);
XGE_API void xgeXuiTextEditSetReadonly(xge_xui_text_edit pEdit, int bReadonly);
XGE_API void xgeXuiTextEditSetWordWrap(xge_xui_text_edit pEdit, int bWordWrap);
XGE_API void xgeXuiTextEditSetFindHighlights(xge_xui_text_edit pEdit, const xge_xui_text_edit_highlight_t* arrHighlights, int iCount);
XGE_API void xgeXuiTextEditSetLineNumbers(xge_xui_text_edit pEdit, int bEnabled, float fWidth);
XGE_API void xgeXuiTextEditSetReserveColors(xge_xui_text_edit pEdit, uint32_t iFindHighlight, uint32_t iLineNumberText, uint32_t iLineNumberBackground);
XGE_API void xgeXuiTextEditSetCurrentLineColor(xge_xui_text_edit pEdit, uint32_t iColor);
XGE_API void xgeXuiTextEditSetScrollbarColors(xge_xui_text_edit pEdit, uint32_t iTrack, uint32_t iBorder, uint32_t iThumb);
XGE_API void xgeXuiTextEditSetScroll(xge_xui_text_edit pEdit, float fX, float fY);
XGE_API void xgeXuiTextEditSetScrollbarMode(xge_xui_text_edit pEdit, int iMode);
XGE_API int xgeXuiTextEditGetScrollbarMode(xge_xui_text_edit pEdit);
XGE_API int xgeXuiTextEditUndo(xge_xui_text_edit pEdit);
XGE_API int xgeXuiTextEditRedo(xge_xui_text_edit pEdit);
XGE_API xge_rect_t xgeXuiTextEditGetCandidateRect(xge_xui_text_edit pEdit);
XGE_API int xgeXuiTextEditEvent(xge_xui_text_edit pEdit, const xge_event_t* pEvent);
XGE_API int xgeXuiTextEditEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTextEditUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
XGE_API void xgeXuiTextEditPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiCheckBoxInit(xge_xui_checkbox pCheckBox, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiCheckBoxUnit(xge_xui_checkbox pCheckBox);
XGE_API void xgeXuiCheckBoxSetChange(xge_xui_checkbox pCheckBox, xge_xui_checked_proc procChange, void* pUser);
XGE_API void xgeXuiCheckBoxSetText(xge_xui_checkbox pCheckBox, xui_font pFont, const char* sText);
XGE_API void xgeXuiCheckBoxSetChecked(xge_xui_checkbox pCheckBox, int bChecked);
XGE_API int xgeXuiCheckBoxGetChecked(xge_xui_checkbox pCheckBox);
XGE_API void xgeXuiCheckBoxSetTextColor(xge_xui_checkbox pCheckBox, uint32_t iColor);
XGE_API void xgeXuiCheckBoxSetColors(xge_xui_checkbox pCheckBox, uint32_t iBox, uint32_t iChecked);
XGE_API void xgeXuiCheckBoxSetTextures(xge_xui_checkbox pCheckBox, xui_texture pUncheckedTexture, xge_rect_t tUncheckedSrc, xui_texture pCheckedTexture, xge_rect_t tCheckedSrc);
XGE_API void xgeXuiCheckBoxSetIndicatorSize(xge_xui_checkbox pCheckBox, float fSize);
XGE_API void xgeXuiCheckBoxSetGap(xge_xui_checkbox pCheckBox, float fGap);
XGE_API int xgeXuiCheckBoxGetState(xge_xui_checkbox pCheckBox);
XGE_API int xgeXuiCheckBoxEvent(xge_xui_checkbox pCheckBox, const xge_event_t* pEvent);
XGE_API int xgeXuiCheckBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiCheckBoxPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiRadioGroupInit(xge_xui_radio_group pGroup);
XGE_API void xgeXuiRadioGroupUnit(xge_xui_radio_group pGroup);
XGE_API void xgeXuiRadioGroupSetChange(xge_xui_radio_group pGroup, xge_xui_select_proc procChange, void* pUser);
XGE_API void xgeXuiRadioGroupSetSelected(xge_xui_radio_group pGroup, int iValue);
XGE_API int xgeXuiRadioGroupGetSelected(xge_xui_radio_group pGroup);
XGE_API int xgeXuiRadioInit(xge_xui_radio pRadio, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiRadioUnit(xge_xui_radio pRadio);
XGE_API void xgeXuiRadioSetGroup(xge_xui_radio pRadio, xge_xui_radio_group pGroup, int iValue);
XGE_API void xgeXuiRadioSetChange(xge_xui_radio pRadio, xge_xui_checked_proc procChange, void* pUser);
XGE_API void xgeXuiRadioSetText(xge_xui_radio pRadio, xui_font pFont, const char* sText);
XGE_API void xgeXuiRadioSetChecked(xge_xui_radio pRadio, int bChecked);
XGE_API int xgeXuiRadioGetChecked(xge_xui_radio pRadio);
XGE_API void xgeXuiRadioSetTextColor(xge_xui_radio pRadio, uint32_t iColor);
XGE_API void xgeXuiRadioSetColors(xge_xui_radio pRadio, uint32_t iRing, uint32_t iChecked);
XGE_API void xgeXuiRadioSetTextures(xge_xui_radio pRadio, xui_texture pUncheckedTexture, xge_rect_t tUncheckedSrc, xui_texture pCheckedTexture, xge_rect_t tCheckedSrc);
XGE_API void xgeXuiRadioSetIndicatorSize(xge_xui_radio pRadio, float fSize);
XGE_API void xgeXuiRadioSetGap(xge_xui_radio pRadio, float fGap);
XGE_API int xgeXuiRadioGetState(xge_xui_radio pRadio);
XGE_API int xgeXuiRadioEvent(xge_xui_radio pRadio, const xge_event_t* pEvent);
XGE_API int xgeXuiRadioEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiRadioPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiToggleUnit(xge_xui_toggle pToggle);
XGE_API void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_checked_proc procChange, void* pUser);
XGE_API void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked);
XGE_API int xgeXuiToggleGetChecked(xge_xui_toggle pToggle);
XGE_API void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iTrack, uint32_t iChecked, uint32_t iKnob, uint32_t iTrackBorder);
XGE_API void xgeXuiToggleSetInnerText(xge_xui_toggle pToggle, xui_font pFont, const char* sUncheckedText, const char* sCheckedText);
XGE_API void xgeXuiToggleSetInnerTextColor(xge_xui_toggle pToggle, uint32_t iUncheckedColor, uint32_t iCheckedColor);
XGE_API void xgeXuiToggleSetTextures(xge_xui_toggle pToggle, xui_texture pUncheckedTexture, xge_rect_t tUncheckedSrc, xui_texture pCheckedTexture, xge_rect_t tCheckedSrc);
XGE_API void xgeXuiToggleSetMetrics(xge_xui_toggle pToggle, float fTrackWidth, float fTrackHeight, float fKnobInset, float fTextPadding, float fTextGap);
XGE_API int xgeXuiToggleGetState(xge_xui_toggle pToggle);
XGE_API int xgeXuiToggleEvent(xge_xui_toggle pToggle, const xge_event_t* pEvent);
XGE_API int xgeXuiToggleEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTogglePaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSeparatorInit(xge_xui_separator pSeparator, xge_xui_widget pWidget);
XGE_API void xgeXuiSeparatorUnit(xge_xui_separator pSeparator);
XGE_API void xgeXuiSeparatorSetColor(xge_xui_separator pSeparator, uint32_t iColor);
XGE_API void xgeXuiSeparatorSetThickness(xge_xui_separator pSeparator, float fThickness);
XGE_API void xgeXuiSeparatorSetOrientation(xge_xui_separator pSeparator, int iOrientation);
XGE_API void xgeXuiSeparatorSetAlign(xge_xui_separator pSeparator, int iAlign);
XGE_API void xgeXuiSeparatorSetLineStyle(xge_xui_separator pSeparator, int iLineStyle);
XGE_API xge_vec2_t xgeXuiSeparatorMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiSeparatorPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiChromeStyleDefault(xge_xui_chrome_style pStyle, const xge_xui_theme_t* pTheme);
XGE_API void xgeXuiSetChromeStyle(xge_xui_context pContext, const xge_xui_chrome_style_t* pStyle);
XGE_API const xge_xui_chrome_style_t* xgeXuiGetChromeStyle(xge_xui_context pContext);
XGE_API int xgeXuiToolbarInit(xge_xui_toolbar pToolbar, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiToolbarUnit(xge_xui_toolbar pToolbar);
XGE_API void xgeXuiToolbarSetItems(xge_xui_toolbar pToolbar, const char** arrText, const int* arrTypes, int iCount);
XGE_API void xgeXuiToolbarSetFont(xge_xui_toolbar pToolbar, xui_font pFont);
XGE_API void xgeXuiToolbarSetSelect(xge_xui_toolbar pToolbar, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiToolbarSetOrientation(xge_xui_toolbar pToolbar, int iOrientation);
XGE_API void xgeXuiToolbarSetItemSize(xge_xui_toolbar pToolbar, float fWidth, float fHeight, float fSeparatorSize);
XGE_API void xgeXuiToolbarSetGroupGap(xge_xui_toolbar pToolbar, float fGap);
XGE_API void xgeXuiToolbarSetItemGroup(xge_xui_toolbar pToolbar, int iIndex, int iGroup);
XGE_API int xgeXuiToolbarGetItemGroup(xge_xui_toolbar pToolbar, int iIndex);
XGE_API void xgeXuiToolbarSetItemTooltip(xge_xui_toolbar pToolbar, int iIndex, const char* sText);
XGE_API const char* xgeXuiToolbarGetItemTooltip(xge_xui_toolbar pToolbar, int iIndex);
XGE_API const char* xgeXuiToolbarGetHoverTooltip(xge_xui_toolbar pToolbar);
XGE_API void xgeXuiToolbarSetOverflow(xge_xui_toolbar pToolbar, int bEnabled, float fButtonSize, xge_xui_click_proc procOverflow, void* pUser);
XGE_API int xgeXuiToolbarGetOverflowFirst(xge_xui_toolbar pToolbar);
XGE_API int xgeXuiToolbarGetOverflowCount(xge_xui_toolbar pToolbar);
XGE_API xge_rect_t xgeXuiToolbarGetOverflowRect(xge_xui_toolbar pToolbar);
XGE_API void xgeXuiToolbarSetItemEnabled(xge_xui_toolbar pToolbar, int iIndex, int bEnabled);
XGE_API void xgeXuiToolbarSetItemChecked(xge_xui_toolbar pToolbar, int iIndex, int bChecked);
XGE_API int xgeXuiToolbarGetItemChecked(xge_xui_toolbar pToolbar, int iIndex);
XGE_API void xgeXuiToolbarSetColors(xge_xui_toolbar pToolbar, uint32_t iBackground, uint32_t iItem, uint32_t iHover, uint32_t iActive, uint32_t iChecked, uint32_t iFocus, uint32_t iDisabled, uint32_t iSeparator, uint32_t iText, uint32_t iDisabledText);
XGE_API int xgeXuiToolbarGetState(xge_xui_toolbar pToolbar);
XGE_API int xgeXuiToolbarEvent(xge_xui_toolbar pToolbar, const xge_event_t* pEvent);
XGE_API int xgeXuiToolbarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiToolbarPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiMenuBarInit(xge_xui_menubar pMenuBar, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiMenuBarUnit(xge_xui_menubar pMenuBar);
XGE_API void xgeXuiMenuBarSetItems(xge_xui_menubar pMenuBar, const xge_xui_menubar_item_t* arrItems, int iCount);
XGE_API int xgeXuiMenuBarAddItem(xge_xui_menubar pMenuBar, const char* sText, xge_xui_menu pMenu, int iValue);
XGE_API void xgeXuiMenuBarSetItemMenu(xge_xui_menubar pMenuBar, int iIndex, xge_xui_menu pMenu);
XGE_API void xgeXuiMenuBarSetItemEnabled(xge_xui_menubar pMenuBar, int iIndex, int bEnabled);
XGE_API void xgeXuiMenuBarSetFont(xge_xui_menubar pMenuBar, xui_font pFont);
XGE_API void xgeXuiMenuBarSetSelect(xge_xui_menubar pMenuBar, xge_xui_menu_select_proc procSelect, void* pUser);
XGE_API void xgeXuiMenuBarSetMetrics(xge_xui_menubar pMenuBar, const xge_xui_bar_metrics_t* pMetrics);
XGE_API void xgeXuiMenuBarSetColors(xge_xui_menubar pMenuBar, const xge_xui_bar_colors_t* pColors);
XGE_API int xgeXuiMenuBarEvent(xge_xui_menubar pMenuBar, const xge_event_t* pEvent);
XGE_API int xgeXuiMenuBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiMenuBarPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiStatusBarInit(xge_xui_status_bar pStatusBar, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiStatusBarUnit(xge_xui_status_bar pStatusBar);
XGE_API void xgeXuiStatusBarClear(xge_xui_status_bar pStatusBar);
XGE_API int xgeXuiStatusBarAddText(xge_xui_status_bar pStatusBar, int iSection, const char* sText, float fWidth, int bClickable);
XGE_API int xgeXuiStatusBarAddProgress(xge_xui_status_bar pStatusBar, int iSection, float fMin, float fMax, float fValue, float fWidth);
XGE_API int xgeXuiStatusBarAddSpacer(xge_xui_status_bar pStatusBar, int iSection, float fWidth);
XGE_API int xgeXuiStatusBarAddFlexibleSpacer(xge_xui_status_bar pStatusBar, int iSection, float fWeight);
XGE_API void xgeXuiStatusBarSetFont(xge_xui_status_bar pStatusBar, xui_font pFont);
XGE_API void xgeXuiStatusBarSetSelect(xge_xui_status_bar pStatusBar, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiStatusBarSetItemEnabled(xge_xui_status_bar pStatusBar, int iIndex, int bEnabled);
XGE_API void xgeXuiStatusBarSetItemText(xge_xui_status_bar pStatusBar, int iIndex, const char* sText);
XGE_API void xgeXuiStatusBarSetProgress(xge_xui_status_bar pStatusBar, int iIndex, float fValue);
XGE_API void xgeXuiStatusBarSetMetrics(xge_xui_status_bar pStatusBar, float fHeight, float fGap, float fItemPadding);
XGE_API void xgeXuiStatusBarSetColors(xge_xui_status_bar pStatusBar, uint32_t iBackground, uint32_t iBorder, uint32_t iItem, uint32_t iHover, uint32_t iActive, uint32_t iText, uint32_t iDisabledText, uint32_t iProgressTrack, uint32_t iProgressFill);
XGE_API int xgeXuiStatusBarGetState(xge_xui_status_bar pStatusBar);
XGE_API int xgeXuiStatusBarEvent(xge_xui_status_bar pStatusBar, const xge_event_t* pEvent);
XGE_API int xgeXuiStatusBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiStatusBarPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSplitLayoutInit(xge_xui_split_layout pSplitLayout, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSplitLayoutUnit(xge_xui_split_layout pSplitLayout);
XGE_API void xgeXuiSplitLayoutSetOrientation(xge_xui_split_layout pSplitLayout, int iOrientation);
XGE_API void xgeXuiSplitLayoutSetPaneCount(xge_xui_split_layout pSplitLayout, int iCount);
XGE_API int xgeXuiSplitLayoutGetPaneCount(xge_xui_split_layout pSplitLayout);
XGE_API xge_xui_widget xgeXuiSplitLayoutGetPaneWidget(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex, float fWeight);
XGE_API float xgeXuiSplitLayoutGetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetPaneMode(xge_xui_split_layout pSplitLayout, int iIndex, int iMode);
XGE_API int xgeXuiSplitLayoutGetPaneMode(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetPaneFixedSize(xge_xui_split_layout pSplitLayout, int iIndex, float fSize);
XGE_API float xgeXuiSplitLayoutGetPaneFixedSize(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetPaneMinSize(xge_xui_split_layout pSplitLayout, int iIndex, float fMinSize);
XGE_API void xgeXuiSplitLayoutSetPaneMaxSize(xge_xui_split_layout pSplitLayout, int iIndex, float fMaxSize);
XGE_API float xgeXuiSplitLayoutGetPaneSize(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetDividerSize(xge_xui_split_layout pSplitLayout, float fSize);
XGE_API void xgeXuiSplitLayoutSetDividerVisualSize(xge_xui_split_layout pSplitLayout, float fSize);
XGE_API void xgeXuiSplitLayoutSetDividerHitSize(xge_xui_split_layout pSplitLayout, float fSize);
XGE_API void xgeXuiSplitLayoutSetShadowDrag(xge_xui_split_layout pSplitLayout, int bEnabled);
XGE_API void xgeXuiSplitLayoutSetColors(xge_xui_split_layout pSplitLayout, uint32_t iDivider, uint32_t iHover, uint32_t iActive, uint32_t iShadow);
XGE_API void xgeXuiSplitLayoutSetChange(xge_xui_split_layout pSplitLayout, xge_xui_split_layout_change_proc procChange, void* pUser);
XGE_API void xgeXuiSplitLayoutLayoutProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSplitLayoutDividerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiSplitLayoutDividerPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTabsInit(xge_xui_tabs pTabs, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiTabsUnit(xge_xui_tabs pTabs);
XGE_API void xgeXuiTabsSetItems(xge_xui_tabs pTabs, const char** arrItems, int iCount);
XGE_API int xgeXuiTabsAddPage(xge_xui_tabs pTabs, const char* sTitle);
XGE_API xge_xui_widget xgeXuiTabsGetTabBarWidget(xge_xui_tabs pTabs);
XGE_API xge_xui_widget xgeXuiTabsGetClientWidget(xge_xui_tabs pTabs);
XGE_API xge_xui_widget xgeXuiTabsGetPageWidget(xge_xui_tabs pTabs, int iIndex);
XGE_API xge_xui_widget xgeXuiTabsGetButtonWidget(xge_xui_tabs pTabs, int iIndex);
XGE_API void xgeXuiTabsSetEnabledItems(xge_xui_tabs pTabs, const int* arrEnabled, int iCount);
XGE_API void xgeXuiTabsSetDirtyItems(xge_xui_tabs pTabs, const int* arrDirty, int iCount);
XGE_API void xgeXuiTabsSetIcons(xge_xui_tabs pTabs, const xui_texture* arrIcons, const xge_rect_t* arrSrc, int iCount);
XGE_API void xgeXuiTabsSetFont(xge_xui_tabs pTabs, xui_font pFont);
XGE_API void xgeXuiTabsSetSelect(xge_xui_tabs pTabs, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiTabsSetClose(xge_xui_tabs pTabs, xge_xui_select_proc procClose, int bCloseButtons, void* pUser);
XGE_API void xgeXuiTabsSetSelected(xge_xui_tabs pTabs, int iIndex);
XGE_API int xgeXuiTabsGetSelected(xge_xui_tabs pTabs);
XGE_API void xgeXuiTabsSetTabSize(xge_xui_tabs pTabs, float fWidth, float fHeight);
XGE_API void xgeXuiTabsSetTabPlacement(xge_xui_tabs pTabs, int iPlacement);
XGE_API int xgeXuiTabsGetTabPlacement(xge_xui_tabs pTabs);
XGE_API void xgeXuiTabsSetScrollable(xge_xui_tabs pTabs, int bScrollable);
XGE_API void xgeXuiTabsSetScroll(xge_xui_tabs pTabs, float fScrollX);
XGE_API float xgeXuiTabsGetScroll(xge_xui_tabs pTabs);
XGE_API void xgeXuiTabsSetColors(xge_xui_tabs pTabs, uint32_t iBackground, uint32_t iTab, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iActiveText);
XGE_API int xgeXuiTabsGetState(xge_xui_tabs pTabs);
XGE_API int xgeXuiTabsEvent(xge_xui_tabs pTabs, const xge_event_t* pEvent);
XGE_API int xgeXuiTabsEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTabsPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiScrollBarInit(xge_xui_scrollbar pScrollBar, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiScrollBarUnit(xge_xui_scrollbar pScrollBar);
XGE_API void xgeXuiScrollBarSetChange(xge_xui_scrollbar pScrollBar, xge_xui_slider_proc procChange, void* pUser);
XGE_API void xgeXuiScrollBarSetRange(xge_xui_scrollbar pScrollBar, float fMin, float fMax, float fPage);
XGE_API void xgeXuiScrollBarSetPage(xge_xui_scrollbar pScrollBar, float fPage);
XGE_API void xgeXuiScrollBarSetValue(xge_xui_scrollbar pScrollBar, float fValue);
XGE_API float xgeXuiScrollBarGetValue(xge_xui_scrollbar pScrollBar);
XGE_API void xgeXuiScrollBarSetOrientation(xge_xui_scrollbar pScrollBar, int iOrientation);
XGE_API void xgeXuiScrollBarSetMode(xge_xui_scrollbar pScrollBar, int iMode);
XGE_API int xgeXuiScrollBarGetMode(xge_xui_scrollbar pScrollBar);
XGE_API void xgeXuiScrollBarSetButtonMode(xge_xui_scrollbar pScrollBar, int iMode);
XGE_API void xgeXuiScrollBarSetMetrics(xge_xui_scrollbar pScrollBar, float fTrackSize, float fMinThumbSize, float fThumbRadius, float fButtonSize);
XGE_API void xgeXuiScrollBarSetColors(xge_xui_scrollbar pScrollBar, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API void xgeXuiScrollBarSetButtonColors(xge_xui_scrollbar pScrollBar, uint32_t iButton, uint32_t iIcon);
XGE_API int xgeXuiScrollBarGetState(xge_xui_scrollbar pScrollBar);
XGE_API int xgeXuiScrollBarEvent(xge_xui_scrollbar pScrollBar, const xge_event_t* pEvent);
XGE_API int xgeXuiScrollBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiScrollBarPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSliderUnit(xge_xui_slider pSlider);
XGE_API void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser);
XGE_API void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax);
XGE_API void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue);
XGE_API float xgeXuiSliderGetValue(xge_xui_slider pSlider);
XGE_API void xgeXuiSliderSetOrientation(xge_xui_slider pSlider, int iOrientation);
XGE_API void xgeXuiSliderSetStep(xge_xui_slider pSlider, float fStep, float fPageStep);
XGE_API void xgeXuiSliderSetMetrics(xge_xui_slider pSlider, float fTrackSize, float fKnobSize, float fTrackRadius, float fKnobRadius);
XGE_API void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);
XGE_API void xgeXuiSliderSetKnobBorderColor(xge_xui_slider pSlider, uint32_t iColor);
XGE_API int xgeXuiSliderGetState(xge_xui_slider pSlider);
XGE_API int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent);
XGE_API int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget);
XGE_API void xgeXuiProgressUnit(xge_xui_progress pProgress);
XGE_API void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax);
XGE_API void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue);
XGE_API float xgeXuiProgressGetValue(xge_xui_progress pProgress);
XGE_API void xgeXuiProgressSetText(xge_xui_progress pProgress, xui_font pFont, const char* sText);
XGE_API void xgeXuiProgressSetTextTemplate(xge_xui_progress pProgress, const char* sTextTemplate);
XGE_API void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor);
XGE_API void xgeXuiProgressSetFillTextColor(xge_xui_progress pProgress, uint32_t iColor);
XGE_API void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill);
XGE_API void xgeXuiProgressSetFillDirection(xge_xui_progress pProgress, int iFillDirection);
XGE_API void xgeXuiProgressSetTrackPatch(xge_xui_progress pProgress, const xge_nine_patch_t* pPatch);
XGE_API void xgeXuiProgressSetFillPatch(xge_xui_progress pProgress, const xge_nine_patch_t* pPatch);
XGE_API void xgeXuiProgressSetFillPatchMode(xge_xui_progress pProgress, int iMode);
XGE_API xge_vec2_t xgeXuiProgressMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiPagerInit(xge_xui_pager pPager, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiPagerUnit(xge_xui_pager pPager);
XGE_API void xgeXuiPagerSetPageCount(xge_xui_pager pPager, int iPageCount);
XGE_API void xgeXuiPagerSetCurrent(xge_xui_pager pPager, int iPage, int bNotify);
XGE_API void xgeXuiPagerSetTotal(xge_xui_pager pPager, int iTotalCount, int iPageSize);
XGE_API int xgeXuiPagerGetCurrent(xge_xui_pager pPager);
XGE_API int xgeXuiPagerGetPageCount(xge_xui_pager pPager);
XGE_API void xgeXuiPagerSetWindowSize(xge_xui_pager pPager, int iWindowSize);
XGE_API void xgeXuiPagerSetText(xge_xui_pager pPager, const char* sFirst, const char* sLast, const char* sPrev, const char* sNext);
XGE_API void xgeXuiPagerSetFont(xge_xui_pager pPager, xui_font pFont);
XGE_API void xgeXuiPagerSetMetrics(xge_xui_pager pPager, float fItemHeight, float fPageWidth, float fTextWidth, float fNavWidth, float fEllipsisWidth);
XGE_API void xgeXuiPagerSetColors(xge_xui_pager pPager, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iHover, uint32_t iActive, uint32_t iCurrent, uint32_t iCurrentText, uint32_t iDisabledText);
XGE_API void xgeXuiPagerSetChange(xge_xui_pager pPager, xge_xui_pager_change_proc procChange, void* pUser);
XGE_API int xgeXuiPagerEvent(xge_xui_pager pPager, const xge_event_t* pEvent);
XGE_API int xgeXuiPagerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API xge_vec2_t xgeXuiPagerMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiPagerPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiWindowInit(xge_xui_window pWindow, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiWindowUnit(xge_xui_window pWindow);
XGE_API xge_xui_widget xgeXuiWindowGetClientWidget(xge_xui_window pWindow);
XGE_API void xgeXuiWindowSetTitle(xge_xui_window pWindow, xui_font pFont, const char* sTitle);
XGE_API void xgeXuiWindowSetIcon(xge_xui_window pWindow, xui_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiWindowSetClose(xge_xui_window pWindow, xge_xui_click_proc procClose, void* pUser);
XGE_API void xgeXuiWindowSetOpen(xge_xui_window pWindow, int bOpen);
XGE_API int xgeXuiWindowIsOpen(xge_xui_window pWindow);
XGE_API void xgeXuiWindowSetShowTitleBar(xge_xui_window pWindow, int bShow);
XGE_API void xgeXuiWindowSetMovable(xge_xui_window pWindow, int bEnabled);
XGE_API void xgeXuiWindowSetDragAnywhere(xge_xui_window pWindow, int bEnabled);
XGE_API void xgeXuiWindowSetResizable(xge_xui_window pWindow, int bEnabled);
XGE_API void xgeXuiWindowSetResizeEdges(xge_xui_window pWindow, uint32_t iEdges);
XGE_API void xgeXuiWindowSetShowCollapse(xge_xui_window pWindow, int bShow);
XGE_API void xgeXuiWindowSetShowMaximize(xge_xui_window pWindow, int bShow);
XGE_API void xgeXuiWindowSetShowClose(xge_xui_window pWindow, int bShow);
XGE_API void xgeXuiWindowSetCollapsed(xge_xui_window pWindow, int bCollapsed);
XGE_API int xgeXuiWindowIsCollapsed(xge_xui_window pWindow);
XGE_API void xgeXuiWindowSetMaximized(xge_xui_window pWindow, int bMaximized);
XGE_API int xgeXuiWindowIsMaximized(xge_xui_window pWindow);
XGE_API void xgeXuiWindowBringToFront(xge_xui_window pWindow);
XGE_API void xgeXuiWindowSetTopMost(xge_xui_window pWindow, int bTopMost);
XGE_API int xgeXuiWindowIsTopMost(xge_xui_window pWindow);
XGE_API int xgeXuiWindowIsActive(xge_xui_window pWindow);
XGE_API xge_xui_window xgeXuiWindowGetActive(xge_xui_context pContext);
XGE_API void xgeXuiWindowSetChrome(xge_xui_window pWindow, float fTitleBarHeight, float fBorderWidth, float fResizeGrip, float fButtonSize);
XGE_API void xgeXuiWindowSetColors(xge_xui_window pWindow, uint32_t iBackground, uint32_t iTitleBar, uint32_t iTitleText, uint32_t iBorder, uint32_t iButtonNormal, uint32_t iButtonHover, uint32_t iButtonActive);
XGE_API int xgeXuiWindowEvent(xge_xui_window pWindow, const xge_event_t* pEvent);
XGE_API int xgeXuiWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiWindowPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiBuiltinAssetGetCount(void);
XGE_API int xgeXuiBuiltinAssetGetRect(const char* sName, xge_rect_t* pRect);
XGE_API int xgeXuiDockLayoutInit(xge_xui_dock_layout pLayout, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiDockLayoutUnit(xge_xui_dock_layout pLayout);
XGE_API xge_xui_widget xgeXuiDockLayoutWidget(xge_xui_dock_layout pLayout);
XGE_API xge_xui_dock_pane xgeXuiDockLayoutDockWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, int iRegion, int iSide, float fProportion);
XGE_API int xgeXuiDockLayoutFloatWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_rect_t tRect);
XGE_API int xgeXuiDockLayoutHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockLayoutAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockLayoutDockAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API void xgeXuiDockLayoutSetRegionPortion(xge_xui_dock_layout pLayout, int iRegion, float fPortion);
XGE_API xvalue xgeXuiDockLayoutSaveState(const xge_xui_dock_layout pLayout);
XGE_API int xgeXuiDockLayoutLoadState(xge_xui_dock_layout pLayout, xvalue pState);
XGE_API void xgeXuiDockLayoutStateFree(xvalue pState);
XGE_API int xgeXuiDockLayoutStateGetCounts(xvalue pState, int* pRegionCount, int* pWindowCount, int* pFloatingCount);
XGE_API int xgeXuiDockWindowInit(xge_xui_dock_window pWindow, xge_xui_context pContext);
XGE_API void xgeXuiDockWindowUnit(xge_xui_dock_window pWindow);
XGE_API xge_xui_window xgeXuiDockWindowBaseWindow(xge_xui_dock_window pWindow);
XGE_API xge_xui_widget xgeXuiDockWindowClientWidget(xge_xui_dock_window pWindow);
XGE_API void xgeXuiDockWindowSetClientWidget(xge_xui_dock_window pWindow, xge_xui_widget pClient);
XGE_API void xgeXuiDockWindowSetTitle(xge_xui_dock_window pWindow, const char* sTitle);
XGE_API void xgeXuiDockWindowSetIcon(xge_xui_dock_window pWindow, xui_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiDockWindowSetClosable(xge_xui_dock_window pWindow, int bClosable);
XGE_API void xgeXuiDockWindowSetDockable(xge_xui_dock_window pWindow, int bDockable);
XGE_API int xgeXuiDockWindowGetState(const xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockPaneGetWindowCount(const xge_xui_dock_pane pPane);
XGE_API xge_xui_dock_window xgeXuiDockPaneGetWindow(const xge_xui_dock_pane pPane, int iIndex);
XGE_API xge_xui_dock_window xgeXuiDockPaneGetActiveWindow(const xge_xui_dock_pane pPane);
XGE_API void xgeXuiDockPaneSetActiveIndex(xge_xui_dock_pane pPane, int iIndex);
XGE_API int xgeXuiDockPaneGetActiveIndex(const xge_xui_dock_pane pPane);
XGE_API int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget);
XGE_API void xgeXuiPanelUnit(xge_xui_panel pPanel);
XGE_API xge_xui_widget xgeXuiPanelGetHeaderWidget(xge_xui_panel pPanel);
XGE_API xge_xui_widget xgeXuiPanelGetIconWidget(xge_xui_panel pPanel);
XGE_API xge_xui_widget xgeXuiPanelGetTitleWidget(xge_xui_panel pPanel);
XGE_API xge_xui_widget xgeXuiPanelGetClientWidget(xge_xui_panel pPanel);
XGE_API void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor);
XGE_API void xgeXuiPanelSetHeaderColor(xge_xui_panel pPanel, uint32_t iColor);
XGE_API void xgeXuiPanelSetClientColor(xge_xui_panel pPanel, uint32_t iColor);
XGE_API void xgeXuiPanelSetHeaderHeight(xge_xui_panel pPanel, float fHeight);
XGE_API void xgeXuiPanelSetIconSize(xge_xui_panel pPanel, float fSize);
XGE_API void xgeXuiPanelSetIcon(xge_xui_panel pPanel, xui_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xui_font pFont, const char* sTitle);
XGE_API void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor);
XGE_API void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags);
XGE_API void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip);
XGE_API void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiScrollModelInit(xge_xui_scroll_model pModel);
XGE_API int xgeXuiScrollModelSetViewport(xge_xui_scroll_model pModel, xge_rect_t tViewport);
XGE_API xge_rect_t xgeXuiScrollModelGetViewport(xge_xui_scroll_model pModel);
XGE_API int xgeXuiScrollModelSetContentSize(xge_xui_scroll_model pModel, float fWidth, float fHeight);
XGE_API int xgeXuiScrollModelSetOffset(xge_xui_scroll_model pModel, float fX, float fY);
XGE_API int xgeXuiScrollModelScrollBy(xge_xui_scroll_model pModel, float fDX, float fDY);
XGE_API void xgeXuiScrollModelGetOffset(xge_xui_scroll_model pModel, float* pX, float* pY);
XGE_API void xgeXuiScrollModelGetMaxOffset(xge_xui_scroll_model pModel, float* pX, float* pY);
XGE_API int xgeXuiScrollModelEnsureRectVisible(xge_xui_scroll_model pModel, xge_rect_t tContentRect);
XGE_API void xgeXuiScrollModelScreenToViewport(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pViewportX, float* pViewportY);
XGE_API void xgeXuiScrollModelViewportToContent(xge_xui_scroll_model pModel, float fViewportX, float fViewportY, float* pContentX, float* pContentY);
XGE_API void xgeXuiScrollModelScreenToContent(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pContentX, float* pContentY);
XGE_API void xgeXuiScrollModelContentToViewport(xge_xui_scroll_model pModel, float fContentX, float fContentY, float* pViewportX, float* pViewportY);
XGE_API void xgeXuiScrollModelContentToScreen(xge_xui_scroll_model pModel, float fContentX, float fContentY, float* pScreenX, float* pScreenY);
XGE_API int xgeXuiScrollFrameInit(xge_xui_scroll_frame pFrame, xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_scroll_model pModel);
XGE_API void xgeXuiScrollFrameUnit(xge_xui_scroll_frame pFrame);
XGE_API xge_xui_widget xgeXuiScrollFrameGetViewportWidget(xge_xui_scroll_frame pFrame);
XGE_API xge_xui_widget xgeXuiScrollFrameGetHScrollBarWidget(xge_xui_scroll_frame pFrame);
XGE_API xge_xui_widget xgeXuiScrollFrameGetVScrollBarWidget(xge_xui_scroll_frame pFrame);
XGE_API xge_xui_widget xgeXuiScrollFrameGetCornerWidget(xge_xui_scroll_frame pFrame);
XGE_API xge_rect_t xgeXuiScrollFrameGetViewportRect(xge_xui_scroll_frame pFrame);
XGE_API void xgeXuiScrollFrameSetChange(xge_xui_scroll_frame pFrame, xge_xui_scroll_frame_change_proc procChange, void* pUser);
XGE_API int xgeXuiScrollFrameSetContentSize(xge_xui_scroll_frame pFrame, float fWidth, float fHeight);
XGE_API int xgeXuiScrollFrameSetOffset(xge_xui_scroll_frame pFrame, float fX, float fY);
XGE_API int xgeXuiScrollFrameScrollBy(xge_xui_scroll_frame pFrame, float fDX, float fDY);
XGE_API void xgeXuiScrollFrameGetOffset(xge_xui_scroll_frame pFrame, float* pX, float* pY);
XGE_API void xgeXuiScrollFrameSetScrollbarPolicy(xge_xui_scroll_frame pFrame, int iPolicyX, int iPolicyY);
XGE_API void xgeXuiScrollFrameSetScrollbarMode(xge_xui_scroll_frame pFrame, int iMode);
XGE_API int xgeXuiScrollFrameGetScrollbarMode(xge_xui_scroll_frame pFrame);
XGE_API void xgeXuiScrollFrameSetWheelAxis(xge_xui_scroll_frame pFrame, int iAxis);
XGE_API int xgeXuiScrollFrameGetWheelAxis(xge_xui_scroll_frame pFrame);
XGE_API void xgeXuiScrollFrameSetWheelStep(xge_xui_scroll_frame pFrame, float fStep);
XGE_API void xgeXuiScrollFrameSetContentDragEnabled(xge_xui_scroll_frame pFrame, int bEnabled);
XGE_API int xgeXuiScrollFrameIsContentDragEnabled(xge_xui_scroll_frame pFrame);
XGE_API void xgeXuiScrollFrameSetCornerMode(xge_xui_scroll_frame pFrame, int iMode);
XGE_API void xgeXuiScrollFrameSetMetrics(xge_xui_scroll_frame pFrame, float fScrollbarSize, float fMinThumbSize, float fThumbRadius, float fButtonSize);
XGE_API void xgeXuiScrollFrameSetColors(xge_xui_scroll_frame pFrame, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API void xgeXuiScrollFrameSetButtonColors(xge_xui_scroll_frame pFrame, uint32_t iButton, uint32_t iIcon);
XGE_API void xgeXuiScrollFrameSetCornerColors(xge_xui_scroll_frame pFrame, uint32_t iCorner, uint32_t iGrip);
XGE_API void xgeXuiScrollFrameLayout(xge_xui_scroll_frame pFrame);
XGE_API int xgeXuiScrollFrameEvent(xge_xui_scroll_frame pFrame, const xge_event_t* pEvent);
XGE_API int xgeXuiScrollFrameEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiScrollFrameLayoutProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiScrollFramePaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll);
XGE_API xge_xui_widget xgeXuiScrollViewGetContentWidget(xge_xui_scroll_view pScroll);
XGE_API xge_xui_widget xgeXuiScrollViewGetViewportWidget(xge_xui_scroll_view pScroll);
XGE_API xge_xui_scroll_model xgeXuiScrollViewGetModel(xge_xui_scroll_view pScroll);
XGE_API xge_xui_scroll_frame xgeXuiScrollViewGetFrame(xge_xui_scroll_view pScroll);
XGE_API void xgeXuiScrollViewLayout(xge_xui_scroll_view pScroll);
XGE_API void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight);
XGE_API void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY);
XGE_API void xgeXuiScrollViewScrollBy(xge_xui_scroll_view pScroll, float fDX, float fDY);
XGE_API void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY);
XGE_API void xgeXuiScrollViewEnsureRectVisible(xge_xui_scroll_view pScroll, xge_rect_t tRect);
XGE_API void xgeXuiScrollViewEnsureChildVisible(xge_xui_scroll_view pScroll, xge_xui_widget pChild);
XGE_API void xgeXuiScrollViewSetScrollbarPolicy(xge_xui_scroll_view pScroll, int iPolicy);
XGE_API void xgeXuiScrollViewSetScrollbarPolicyXY(xge_xui_scroll_view pScroll, int iPolicyX, int iPolicyY);
XGE_API void xgeXuiScrollViewSetScrollbarMode(xge_xui_scroll_view pScroll, int iMode);
XGE_API int xgeXuiScrollViewGetScrollbarMode(xge_xui_scroll_view pScroll);
XGE_API void xgeXuiScrollViewSetWheelAxis(xge_xui_scroll_view pScroll, int iAxis);
XGE_API int xgeXuiScrollViewGetWheelAxis(xge_xui_scroll_view pScroll);
XGE_API void xgeXuiScrollViewSetWheelStep(xge_xui_scroll_view pScroll, float fStep);
XGE_API void xgeXuiScrollViewSetContentDragEnabled(xge_xui_scroll_view pScroll, int bEnabled);
XGE_API int xgeXuiScrollViewIsContentDragEnabled(xge_xui_scroll_view pScroll);
XGE_API void xgeXuiScrollViewSetMetrics(xge_xui_scroll_view pScroll, float fScrollbarSize, float fMinThumbSize, float fThumbRadius, float fButtonSize);
XGE_API void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb);
XGE_API int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent);
XGE_API int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiScrollViewLayoutProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiListViewUnit(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetItems(xge_xui_list_view pList, const char** arrItems, int iCount);
XGE_API void xgeXuiListViewSetEnabledItems(xge_xui_list_view pList, const int* arrEnabled, int iCount);
XGE_API void xgeXuiListViewSetFont(xge_xui_list_view pList, xui_font pFont);
XGE_API void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight);
XGE_API void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex);
XGE_API int xgeXuiListViewGetSelected(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetSelectionMode(xge_xui_list_view pList, int iMode);
XGE_API int xgeXuiListViewGetSelectionMode(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetSelectionBuffer(xge_xui_list_view pList, int* arrSelected, int iCount);
XGE_API void xgeXuiListViewClearSelection(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetItemSelected(xge_xui_list_view pList, int iIndex, int bSelected);
XGE_API int xgeXuiListViewIsItemSelected(xge_xui_list_view pList, int iIndex);
XGE_API void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY);
XGE_API float xgeXuiListViewGetScroll(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetScrollbarMode(xge_xui_list_view pList, int iMode);
XGE_API int xgeXuiListViewGetScrollbarMode(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiListViewSetItemRenderer(xge_xui_list_view pList, xge_xui_list_view_item_proc procItem, void* pUser);
XGE_API void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb);
XGE_API void xgeXuiListViewSetDisabledTextColor(xge_xui_list_view pList, uint32_t iColor);
XGE_API int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent);
XGE_API int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTreeViewInit(xge_xui_tree_view pTree, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiTreeViewUnit(xge_xui_tree_view pTree);
XGE_API void xgeXuiTreeViewClear(xge_xui_tree_view pTree);
XGE_API int xgeXuiTreeViewAddNode(xge_xui_tree_view pTree, int iId, int iParentId, const char* sText);
XGE_API void xgeXuiTreeViewSetAdapter(xge_xui_tree_view pTree, xge_xui_tree_view_count_proc procCount, xge_xui_tree_view_node_proc procNode, void* pUser);
XGE_API int xgeXuiTreeViewRefreshAdapter(xge_xui_tree_view pTree);
XGE_API void xgeXuiTreeViewSetNodeExpanded(xge_xui_tree_view pTree, int iId, int bExpanded);
XGE_API int xgeXuiTreeViewGetNodeExpanded(xge_xui_tree_view pTree, int iId);
XGE_API void xgeXuiTreeViewSetNodeEnabled(xge_xui_tree_view pTree, int iId, int bEnabled);
XGE_API int xgeXuiTreeViewGetNodeEnabled(xge_xui_tree_view pTree, int iId);
XGE_API void xgeXuiTreeViewSetNodeChecked(xge_xui_tree_view pTree, int iId, int bChecked);
XGE_API int xgeXuiTreeViewGetNodeChecked(xge_xui_tree_view pTree, int iId);
XGE_API void xgeXuiTreeViewSetNodeDecorations(xge_xui_tree_view pTree, int iId, int bIcon, int bCheck);
XGE_API void xgeXuiTreeViewSetSelected(xge_xui_tree_view pTree, int iId);
XGE_API int xgeXuiTreeViewGetSelected(xge_xui_tree_view pTree);
XGE_API int xgeXuiTreeViewGetVisibleCount(xge_xui_tree_view pTree);
XGE_API int xgeXuiTreeViewGetVisibleNodeId(xge_xui_tree_view pTree, int iVisible);
XGE_API int xgeXuiTreeViewGetFirstVisible(xge_xui_tree_view pTree);
XGE_API int xgeXuiTreeViewGetPaintVisibleCount(xge_xui_tree_view pTree);
XGE_API void xgeXuiTreeViewSetFont(xge_xui_tree_view pTree, xui_font pFont);
XGE_API void xgeXuiTreeViewSetMetrics(xge_xui_tree_view pTree, float fItemHeight, float fIndent);
XGE_API void xgeXuiTreeViewSetScroll(xge_xui_tree_view pTree, float fScrollY);
XGE_API float xgeXuiTreeViewGetScroll(xge_xui_tree_view pTree);
XGE_API void xgeXuiTreeViewSetScrollbarMode(xge_xui_tree_view pTree, int iMode);
XGE_API int xgeXuiTreeViewGetScrollbarMode(xge_xui_tree_view pTree);
XGE_API void xgeXuiTreeViewSetSelect(xge_xui_tree_view pTree, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiTreeViewSetItemRenderer(xge_xui_tree_view pTree, xge_xui_tree_view_item_proc procItem, void* pUser);
XGE_API void xgeXuiTreeViewSetColors(xge_xui_tree_view pTree, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb);
XGE_API void xgeXuiTreeViewSetDisabledTextColor(xge_xui_tree_view pTree, uint32_t iColor);
XGE_API int xgeXuiTreeViewEvent(xge_xui_tree_view pTree, const xge_event_t* pEvent);
XGE_API int xgeXuiTreeViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTreeViewPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTableViewInit(xge_xui_table_view pTable, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiTableViewUnit(xge_xui_table_view pTable);
XGE_API void xgeXuiTableViewSetColumns(xge_xui_table_view pTable, const xge_xui_table_view_column_t* arrColumns, int iCount);
XGE_API void xgeXuiTableViewSetRows(xge_xui_table_view pTable, const xge_xui_table_view_row_t* arrRows, int iCount);
XGE_API void xgeXuiTableViewSetAdapter(xge_xui_table_view pTable, xge_xui_table_view_count_proc procCount, xge_xui_table_view_cell_proc procCell, void* pUser);
XGE_API void xgeXuiTableViewSetSort(xge_xui_table_view pTable, xge_xui_table_view_sort_proc procSort, void* pUser);
XGE_API void xgeXuiTableViewSetSelect(xge_xui_table_view pTable, xge_xui_table_view_select_proc procSelect, void* pUser);
XGE_API void xgeXuiTableViewSetColumnResize(xge_xui_table_view pTable, xge_xui_table_view_column_resize_proc procResize, void* pUser);
XGE_API void xgeXuiTableViewSetHover(xge_xui_table_view pTable, xge_xui_table_view_hover_proc procHover, void* pUser);
XGE_API void xgeXuiTableViewSetMergeProvider(xge_xui_table_view pTable, xge_xui_table_view_merge_proc procMerge, void* pUser);
XGE_API void xgeXuiTableViewSetHeaderRenderer(xge_xui_table_view pTable, xge_xui_table_view_header_renderer_proc procRenderer, void* pUser);
XGE_API void xgeXuiTableViewSetCellRenderer(xge_xui_table_view pTable, xge_xui_table_view_cell_renderer_proc procRenderer, void* pUser);
XGE_API void xgeXuiTableViewSetColumnFormatter(xge_xui_table_view pTable, int iColumn, xge_xui_table_view_format_proc procFormatter, void* pUser);
XGE_API void xgeXuiTableViewSetFont(xge_xui_table_view pTable, xui_font pFont);
XGE_API void xgeXuiTableViewSetDefaultMetrics(xge_xui_table_view pTable, float fColumnWidth, float fRowHeight, float fHeaderHeight);
XGE_API void xgeXuiTableViewSetSelectionMode(xge_xui_table_view pTable, int iMode);
XGE_API int xgeXuiTableViewGetSelectionMode(xge_xui_table_view pTable);
XGE_API void xgeXuiTableViewSetSelectedRow(xge_xui_table_view pTable, int iRow);
XGE_API int xgeXuiTableViewGetSelectedRow(xge_xui_table_view pTable);
XGE_API void xgeXuiTableViewSetSelectedCell(xge_xui_table_view pTable, int iRow, int iColumn);
XGE_API void xgeXuiTableViewGetSelectedCell(xge_xui_table_view pTable, int* pRow, int* pColumn);
XGE_API void xgeXuiTableViewSetOffset(xge_xui_table_view pTable, float fScrollX, float fScrollY);
XGE_API void xgeXuiTableViewGetOffset(xge_xui_table_view pTable, float* pScrollX, float* pScrollY);
XGE_API void xgeXuiTableViewSetScrollbarMode(xge_xui_table_view pTable, int iMode);
XGE_API int xgeXuiTableViewGetScrollbarMode(xge_xui_table_view pTable);
XGE_API int xgeXuiTableViewGetRowCount(xge_xui_table_view pTable);
XGE_API int xgeXuiTableViewGetFirstVisible(xge_xui_table_view pTable);
XGE_API int xgeXuiTableViewGetPaintVisibleCount(xge_xui_table_view pTable);
XGE_API void xgeXuiTableViewGetActiveCell(xge_xui_table_view pTable, int* pRow, int* pColumn);
XGE_API int xgeXuiTableViewGetCellContentRect(xge_xui_table_view pTable, int iRow, int iColumn, xge_rect_t* pRect);
XGE_API int xgeXuiTableViewGetCellRect(xge_xui_table_view pTable, int iRow, int iColumn, xge_rect_t* pRect);
XGE_API xge_xui_widget xgeXuiTableViewGetBodyWidget(xge_xui_table_view pTable);
XGE_API xge_xui_widget xgeXuiTableViewGetViewportWidget(xge_xui_table_view pTable);
XGE_API void xgeXuiTableViewSetColors(xge_xui_table_view pTable, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText);
XGE_API void xgeXuiTableViewSetDisabledTextColor(xge_xui_table_view pTable, uint32_t iColor);
XGE_API void xgeXuiTableViewRefresh(xge_xui_table_view pTable);
XGE_API void xgeXuiTableViewEnsureCellVisible(xge_xui_table_view pTable, int iRow, int iColumn);
XGE_API int xgeXuiTableViewEvent(xge_xui_table_view pTable, const xge_event_t* pEvent);
XGE_API int xgeXuiTableViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTableViewPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTableGridInit(xge_xui_table_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiTableGridUnit(xge_xui_table_grid pGrid);
XGE_API xge_xui_table_view xgeXuiTableGridGetTableView(xge_xui_table_grid pGrid);
XGE_API void xgeXuiTableGridSetColumns(xge_xui_table_grid pGrid, const xge_xui_table_view_column_t* arrColumns, int iCount);
XGE_API void xgeXuiTableGridSetRows(xge_xui_table_grid pGrid, const xge_xui_table_view_row_t* arrRows, int iCount);
XGE_API void xgeXuiTableGridSetAdapter(xge_xui_table_grid pGrid, xge_xui_table_view_count_proc procCount, xge_xui_table_view_cell_proc procCell, xge_xui_table_grid_set_proc procSet, void* pUser);
XGE_API void xgeXuiTableGridSetValidate(xge_xui_table_grid pGrid, xge_xui_table_grid_validate_proc procValidate, void* pUser);
XGE_API void xgeXuiTableGridSetChange(xge_xui_table_grid pGrid, xge_xui_table_grid_change_proc procChange, void* pUser);
XGE_API void xgeXuiTableGridSetEditor(xge_xui_table_grid pGrid, xge_xui_table_grid_editor_proc procEditor, void* pUser);
XGE_API void xgeXuiTableGridSetEditorConfig(xge_xui_table_grid pGrid, xge_xui_table_grid_editor_config_proc procConfig, void* pUser);
XGE_API void xgeXuiTableGridSetEditMode(xge_xui_table_grid pGrid, int iMode);
XGE_API int xgeXuiTableGridGetEditMode(xge_xui_table_grid pGrid);
XGE_API void xgeXuiTableGridSetFont(xge_xui_table_grid pGrid, xui_font pFont);
XGE_API void xgeXuiTableGridSetDefaultMetrics(xge_xui_table_grid pGrid, float fColumnWidth, float fRowHeight, float fHeaderHeight);
XGE_API void xgeXuiTableGridSetSelectionMode(xge_xui_table_grid pGrid, int iMode);
XGE_API void xgeXuiTableGridSetScrollbarMode(xge_xui_table_grid pGrid, int iMode);
XGE_API void xgeXuiTableGridSetColors(xge_xui_table_grid pGrid, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText);
XGE_API int xgeXuiTableGridBeginEdit(xge_xui_table_grid pGrid, int iRow, int iColumn);
XGE_API int xgeXuiTableGridEndEdit(xge_xui_table_grid pGrid, int bCommit);
XGE_API int xgeXuiTableGridIsEditing(xge_xui_table_grid pGrid);
XGE_API void xgeXuiTableGridGetEditingCell(xge_xui_table_grid pGrid, int* pRow, int* pColumn);
XGE_API int xgeXuiTableGridEvent(xge_xui_table_grid pGrid, const xge_event_t* pEvent);
XGE_API int xgeXuiTableGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API int xgeXuiTimelineViewInit(xge_xui_timeline_view pTimeline, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiTimelineViewUnit(xge_xui_timeline_view pTimeline);
XGE_API void xgeXuiTimelineViewClear(xge_xui_timeline_view pTimeline);
XGE_API void xgeXuiTimelineViewSetFont(xge_xui_timeline_view pTimeline, xui_font pFont);
XGE_API void xgeXuiTimelineViewSetFrameCount(xge_xui_timeline_view pTimeline, int iFrameCount);
XGE_API int xgeXuiTimelineViewGetFrameCount(xge_xui_timeline_view pTimeline);
XGE_API void xgeXuiTimelineViewSetFrameRate(xge_xui_timeline_view pTimeline, int iFrameRate);
XGE_API int xgeXuiTimelineViewGetFrameRate(xge_xui_timeline_view pTimeline);
XGE_API int xgeXuiTimelineViewSetCurrentFrame(xge_xui_timeline_view pTimeline, int iFrame);
XGE_API int xgeXuiTimelineViewGetCurrentFrame(xge_xui_timeline_view pTimeline);
XGE_API void xgeXuiTimelineViewSetMetrics(xge_xui_timeline_view pTimeline, float fLayerHeaderWidth, float fFrameWidth, float fRowHeight, float fRulerHeight);
XGE_API void xgeXuiTimelineViewSetFrameWidth(xge_xui_timeline_view pTimeline, float fFrameWidth);
XGE_API void xgeXuiTimelineViewSetLayerHeaderWidth(xge_xui_timeline_view pTimeline, float fWidth);
XGE_API void xgeXuiTimelineViewSetRowHeight(xge_xui_timeline_view pTimeline, float fHeight);
XGE_API void xgeXuiTimelineViewSetRulerHeight(xge_xui_timeline_view pTimeline, float fHeight);
XGE_API void xgeXuiTimelineViewSetFeatureFlags(xge_xui_timeline_view pTimeline, int bVisibility, int bLock);
XGE_API void xgeXuiTimelineViewSetScrollbarMode(xge_xui_timeline_view pTimeline, int iMode);
XGE_API int xgeXuiTimelineViewGetScrollbarMode(xge_xui_timeline_view pTimeline);
XGE_API int xgeXuiTimelineViewAddLayer(xge_xui_timeline_view pTimeline, int iId, const char* sName);
XGE_API int xgeXuiTimelineViewRemoveLayer(xge_xui_timeline_view pTimeline, int iLayer);
XGE_API int xgeXuiTimelineViewMoveLayer(xge_xui_timeline_view pTimeline, int iFrom, int iTo);
XGE_API int xgeXuiTimelineViewGetLayerCount(xge_xui_timeline_view pTimeline);
XGE_API xge_xui_timeline_layer_t* xgeXuiTimelineViewGetLayer(xge_xui_timeline_view pTimeline, int iLayer);
XGE_API int xgeXuiTimelineViewSetLayerName(xge_xui_timeline_view pTimeline, int iLayer, const char* sName);
XGE_API int xgeXuiTimelineViewSetLayerVisible(xge_xui_timeline_view pTimeline, int iLayer, int bVisible);
XGE_API int xgeXuiTimelineViewSetLayerLocked(xge_xui_timeline_view pTimeline, int iLayer, int bLocked);
XGE_API int xgeXuiTimelineViewSetLayerHeight(xge_xui_timeline_view pTimeline, int iLayer, float fHeight);
XGE_API int xgeXuiTimelineViewSetLayerColor(xge_xui_timeline_view pTimeline, int iLayer, uint32_t iColor);
XGE_API int xgeXuiTimelineViewSetFrameType(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, int iType);
XGE_API int xgeXuiTimelineViewGetFrameType(xge_xui_timeline_view pTimeline, int iLayer, int iFrame);
XGE_API int xgeXuiTimelineViewClearFrame(xge_xui_timeline_view pTimeline, int iLayer, int iFrame);
XGE_API int xgeXuiTimelineViewAddSpan(xge_xui_timeline_view pTimeline, int iLayer, int iId, int iStartFrame, int iEndFrame, int iType, const char* sLabel);
XGE_API int xgeXuiTimelineViewRemoveSpan(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId);
XGE_API int xgeXuiTimelineViewSetSpan(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId, int iStartFrame, int iEndFrame, int iType, const char* sLabel);
XGE_API xge_xui_timeline_span_t* xgeXuiTimelineViewGetSpan(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId);
XGE_API int xgeXuiTimelineViewSetLayerUserData(xge_xui_timeline_view pTimeline, int iLayer, void* pUser);
XGE_API void* xgeXuiTimelineViewGetLayerUserData(xge_xui_timeline_view pTimeline, int iLayer);
XGE_API int xgeXuiTimelineViewSetFrameUserData(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, void* pUser);
XGE_API void* xgeXuiTimelineViewGetFrameUserData(xge_xui_timeline_view pTimeline, int iLayer, int iFrame);
XGE_API int xgeXuiTimelineViewSetSpanUserData(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId, void* pUser);
XGE_API void* xgeXuiTimelineViewGetSpanUserData(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId);
XGE_API int xgeXuiTimelineViewClearSelection(xge_xui_timeline_view pTimeline);
XGE_API int xgeXuiTimelineViewSelectFrame(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, int bSelected);
XGE_API int xgeXuiTimelineViewSelectRange(xge_xui_timeline_view pTimeline, int iLayerA, int iFrameA, int iLayerB, int iFrameB);
XGE_API int xgeXuiTimelineViewIsFrameSelected(xge_xui_timeline_view pTimeline, int iLayer, int iFrame);
XGE_API int xgeXuiTimelineViewGetSelectionCount(xge_xui_timeline_view pTimeline);
XGE_API void xgeXuiTimelineViewSetCurrentFrameProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_current_frame_changing_proc procChanging, xge_xui_timeline_current_frame_changed_proc procChanged, void* pUser);
XGE_API void xgeXuiTimelineViewSetLayerProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_layer_changing_proc procChanging, xge_xui_timeline_layer_changed_proc procChanged, void* pUser);
XGE_API void xgeXuiTimelineViewSetFrameProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_frame_changing_proc procChanging, xge_xui_timeline_frame_changed_proc procChanged, void* pUser);
XGE_API void xgeXuiTimelineViewSetSpanProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_span_changing_proc procChanging, xge_xui_timeline_span_changed_proc procChanged, void* pUser);
XGE_API void xgeXuiTimelineViewSetLayerSelectedProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_layer_selected_proc procSelected, void* pUser);
XGE_API void xgeXuiTimelineViewSetContextMenuProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_context_menu_opening_proc procOpening, xge_xui_timeline_context_menu_command_proc procCommand, void* pUser);
XGE_API void xgeXuiTimelineViewSetFrameClick(xge_xui_timeline_view pTimeline, xge_xui_timeline_frame_click_proc procClick, void* pUser);
XGE_API void xgeXuiTimelineViewSetFrameDoubleClick(xge_xui_timeline_view pTimeline, xge_xui_timeline_frame_click_proc procDoubleClick, void* pUser);
XGE_API void xgeXuiTimelineViewSetSelectionProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_selection_proc procSelection, void* pUser);
XGE_API void xgeXuiTimelineViewSetRenderers(xge_xui_timeline_view pTimeline, xge_xui_timeline_layer_renderer_proc procLayer, void* pLayerUser, xge_xui_timeline_ruler_renderer_proc procRuler, void* pRulerUser, xge_xui_timeline_frame_renderer_proc procFrame, void* pFrameUser, xge_xui_timeline_span_renderer_proc procSpan, void* pSpanUser);
XGE_API void xgeXuiTimelineViewSetColors(xge_xui_timeline_view pTimeline, uint32_t iBackground, uint32_t iHeader, uint32_t iLayer, uint32_t iSelected, uint32_t iGrid, uint32_t iText);
XGE_API void xgeXuiTimelineViewEnsureFrameVisible(xge_xui_timeline_view pTimeline, int iLayer, int iFrame);
XGE_API int xgeXuiTimelineViewHitTest(xge_xui_timeline_view pTimeline, float fX, float fY, xge_xui_timeline_hit_t* pHit);
XGE_API int xgeXuiTimelineViewEvent(xge_xui_timeline_view pTimeline, const xge_event_t* pEvent);
XGE_API int xgeXuiTimelineViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTimelineViewPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiPropertyGridInit(xge_xui_property_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiPropertyGridUnit(xge_xui_property_grid pGrid);
XGE_API void xgeXuiPropertyGridClear(xge_xui_property_grid pGrid);
XGE_API int xgeXuiPropertyGridAddCategory(xge_xui_property_grid pGrid, const char* sId, const char* sName, int bExpanded);
XGE_API int xgeXuiPropertyGridAddProperty(xge_xui_property_grid pGrid, int iCategory, const xge_xui_property_desc_t* pDesc);
XGE_API int xgeXuiPropertyGridFindCategory(xge_xui_property_grid pGrid, const char* sId);
XGE_API int xgeXuiPropertyGridFindProperty(xge_xui_property_grid pGrid, const char* sId);
XGE_API void xgeXuiPropertyGridSetCategoryExpanded(xge_xui_property_grid pGrid, int iCategory, int bExpanded);
XGE_API int xgeXuiPropertyGridGetCategoryExpanded(xge_xui_property_grid pGrid, int iCategory);
XGE_API void xgeXuiPropertyGridSetSelected(xge_xui_property_grid pGrid, int iProperty);
XGE_API int xgeXuiPropertyGridGetSelected(xge_xui_property_grid pGrid);
XGE_API int xgeXuiPropertyGridGetVisibleCount(xge_xui_property_grid pGrid);
XGE_API int xgeXuiPropertyGridGetVisibleProperty(xge_xui_property_grid pGrid, int iVisible);
XGE_API int xgeXuiPropertyGridSetValue(xge_xui_property_grid pGrid, int iProperty, const char* sValue);
XGE_API const char* xgeXuiPropertyGridGetValue(xge_xui_property_grid pGrid, int iProperty);
XGE_API int xgeXuiPropertyGridGetBool(xge_xui_property_grid pGrid, int iProperty, int bDefault);
XGE_API int xgeXuiPropertyGridSetBool(xge_xui_property_grid pGrid, int iProperty, int bValue);
XGE_API int xgeXuiPropertyGridGetInt(xge_xui_property_grid pGrid, int iProperty, int iDefault);
XGE_API int xgeXuiPropertyGridSetInt(xge_xui_property_grid pGrid, int iProperty, int iValue);
XGE_API float xgeXuiPropertyGridGetFloat(xge_xui_property_grid pGrid, int iProperty, float fDefault);
XGE_API int xgeXuiPropertyGridSetFloat(xge_xui_property_grid pGrid, int iProperty, float fValue);
XGE_API uint32_t xgeXuiPropertyGridGetColor(xge_xui_property_grid pGrid, int iProperty, uint32_t iDefault);
XGE_API int xgeXuiPropertyGridSetColor(xge_xui_property_grid pGrid, int iProperty, uint32_t iColor);
XGE_API void xgeXuiPropertyGridSetPropertyFlags(xge_xui_property_grid pGrid, int iProperty, int iFlags);
XGE_API int xgeXuiPropertyGridGetPropertyFlags(xge_xui_property_grid pGrid, int iProperty);
XGE_API void xgeXuiPropertyGridSetEditorConfig(xge_xui_property_grid pGrid, int iProperty, const xge_xui_table_grid_editor_config_t* pConfig);
XGE_API void xgeXuiPropertyGridSetRenderer(xge_xui_property_grid pGrid, int iProperty, xge_xui_property_grid_render_proc procRender, void* pUser);
XGE_API void xgeXuiPropertyGridSetAction(xge_xui_property_grid pGrid, int iProperty, xge_xui_property_grid_action_proc procAction, void* pUser);
XGE_API int xgeXuiPropertyGridIsEditing(xge_xui_property_grid pGrid);
XGE_API int xgeXuiPropertyGridBeginEdit(xge_xui_property_grid pGrid, int iProperty);
XGE_API int xgeXuiPropertyGridEndEdit(xge_xui_property_grid pGrid, int bCommit);
XGE_API void xgeXuiPropertyGridSetFont(xge_xui_property_grid pGrid, xui_font pFont);
XGE_API void xgeXuiPropertyGridSetMetrics(xge_xui_property_grid pGrid, float fNameWidth, float fRowHeight, float fCategoryHeight);
XGE_API void xgeXuiPropertyGridSetDescriptionMode(xge_xui_property_grid pGrid, int iMode, float fPanelHeight);
XGE_API void xgeXuiPropertyGridSetEditMode(xge_xui_property_grid pGrid, int iMode);
XGE_API void xgeXuiPropertyGridSetScroll(xge_xui_property_grid pGrid, float fScrollY);
XGE_API float xgeXuiPropertyGridGetScroll(xge_xui_property_grid pGrid);
XGE_API void xgeXuiPropertyGridSetScrollbarMode(xge_xui_property_grid pGrid, int iMode);
XGE_API int xgeXuiPropertyGridGetScrollbarMode(xge_xui_property_grid pGrid);
XGE_API void xgeXuiPropertyGridSetSelect(xge_xui_property_grid pGrid, xge_xui_property_grid_select_proc procSelect, void* pUser);
XGE_API void xgeXuiPropertyGridSetValidate(xge_xui_property_grid pGrid, xge_xui_property_grid_validate_proc procValidate, void* pUser);
XGE_API void xgeXuiPropertyGridSetChange(xge_xui_property_grid pGrid, xge_xui_property_grid_change_proc procChange, void* pUser);
XGE_API void xgeXuiPropertyGridSetGlobalAction(xge_xui_property_grid pGrid, xge_xui_property_grid_action_proc procAction, void* pUser);
XGE_API void xgeXuiPropertyGridSetGlobalRenderer(xge_xui_property_grid pGrid, xge_xui_property_grid_render_proc procRender, void* pUser);
XGE_API void xgeXuiPropertyGridSetStyle(xge_xui_property_grid pGrid, const xge_xui_property_grid_style_t* pStyle);
XGE_API void xgeXuiPropertyGridSetColors(xge_xui_property_grid pGrid, uint32_t iBackground, uint32_t iCategory, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText);
XGE_API int xgeXuiPropertyGridEvent(xge_xui_property_grid pGrid, const xge_event_t* pEvent);
XGE_API int xgeXuiPropertyGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiPropertyGridPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiAccordionInit(xge_xui_accordion pAccordion, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiAccordionUnit(xge_xui_accordion pAccordion);
XGE_API void xgeXuiAccordionClear(xge_xui_accordion pAccordion);
XGE_API int xgeXuiAccordionAddSection(xge_xui_accordion pAccordion, const char* sTitle, int bExpanded, int iId);
XGE_API int xgeXuiAccordionGetSectionCount(xge_xui_accordion pAccordion);
XGE_API xge_xui_widget xgeXuiAccordionGetSectionWidget(xge_xui_accordion pAccordion, int iIndex);
XGE_API xge_xui_widget xgeXuiAccordionGetHeaderWidget(xge_xui_accordion pAccordion, int iIndex);
XGE_API xge_xui_widget xgeXuiAccordionGetButtonWidget(xge_xui_accordion pAccordion, int iIndex);
XGE_API xge_xui_widget xgeXuiAccordionGetClientWidget(xge_xui_accordion pAccordion, int iIndex);
XGE_API int xgeXuiAccordionIsExpanded(xge_xui_accordion pAccordion, int iIndex);
XGE_API void xgeXuiAccordionSetExpanded(xge_xui_accordion pAccordion, int iIndex, int bExpanded);
XGE_API void xgeXuiAccordionSetSectionEnabled(xge_xui_accordion pAccordion, int iIndex, int bEnabled);
XGE_API void xgeXuiAccordionSetMode(xge_xui_accordion pAccordion, int iMode);
XGE_API void xgeXuiAccordionSetFont(xge_xui_accordion pAccordion, xui_font pFont);
XGE_API void xgeXuiAccordionSetMetrics(xge_xui_accordion pAccordion, float fHeaderHeight, float fSpacing, float fContentPadding);
XGE_API void xgeXuiAccordionSetSelect(xge_xui_accordion pAccordion, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiAccordionSetColors(xge_xui_accordion pAccordion, uint32_t iBackground, uint32_t iHeader, uint32_t iHover, uint32_t iExpanded, uint32_t iContent, uint32_t iBorder, uint32_t iText);
XGE_API float xgeXuiAccordionGetContentHeight(xge_xui_accordion pAccordion);
XGE_API int xgeXuiAccordionEvent(xge_xui_accordion pAccordion, const xge_event_t* pEvent);
XGE_API int xgeXuiAccordionEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiAccordionPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiToastShow(xge_xui_context pContext, int iType, const char* sTitle, const char* sMessage, float fDuration, xge_xui_toast_click_proc procClick, void* pUser);
XGE_API int xgeXuiToastClose(xge_xui_context pContext, int iToastId);
XGE_API void xgeXuiToastClear(xge_xui_context pContext);
XGE_API int xgeXuiToastGetActiveCount(xge_xui_context pContext);
XGE_API int xgeXuiToastGetPendingCount(xge_xui_context pContext);
XGE_API void xgeXuiToastSetPlacement(xge_xui_context pContext, int iPlacement);
XGE_API void xgeXuiToastSetDirection(xge_xui_context pContext, int iDirection);
XGE_API void xgeXuiToastSetMetrics(xge_xui_context pContext, float fWidth, float fMargin, float fGap, int iMaxVisible);
XGE_API void xgeXuiToastSetFont(xge_xui_context pContext, xui_font pFont);
XGE_API void xgeXuiToastSetColors(xge_xui_context pContext, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iMutedText, uint32_t iInfo, uint32_t iSuccess, uint32_t iWarning, uint32_t iError);
XGE_API void xgeXuiToastSetClose(xge_xui_context pContext, xge_xui_toast_close_proc procClose, void* pUser);
XGE_API int xgeXuiMsgTipInit(xge_xui_msg_tip pTip, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiMsgTipUnit(xge_xui_msg_tip pTip);
XGE_API int xgeXuiMsgTipShow(xge_xui_msg_tip pTip, int iType, const char* sText, float fDuration);
XGE_API void xgeXuiMsgTipClose(xge_xui_msg_tip pTip);
XGE_API int xgeXuiMsgTipIsOpen(xge_xui_msg_tip pTip);
XGE_API void xgeXuiMsgTipSetText(xge_xui_msg_tip pTip, const char* sText);
XGE_API void xgeXuiMsgTipSetType(xge_xui_msg_tip pTip, int iType);
XGE_API void xgeXuiMsgTipSetIconTexture(xge_xui_msg_tip pTip, xui_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiMsgTipSetMetrics(xge_xui_msg_tip pTip, float fMinWidth, float fMaxWidth, float fMinHeight, float fOffsetY);
XGE_API void xgeXuiMsgTipSetColors(xge_xui_msg_tip pTip, uint32_t iBackground, uint32_t iText, uint32_t iIcon);
XGE_API int xgeXuiMsgTipEvent(xge_xui_msg_tip pTip, const xge_event_t* pEvent);
XGE_API int xgeXuiMsgTipEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiMsgTipUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
XGE_API void xgeXuiMsgTipPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiMsgBoxInit(xge_xui_msg_box pBox, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiMsgBoxUnit(xge_xui_msg_box pBox);
XGE_API void xgeXuiMsgBoxSetText(xge_xui_msg_box pBox, xui_font pFont, const char* sTitle, const char* sMessage);
XGE_API void xgeXuiMsgBoxSetType(xge_xui_msg_box pBox, int iType);
XGE_API void xgeXuiMsgBoxSetIconTexture(xge_xui_msg_box pBox, xui_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiMsgBoxSetButtons(xge_xui_msg_box pBox, int iButtons);
XGE_API void xgeXuiMsgBoxSetCustomButtons(xge_xui_msg_box pBox, xvalue arrButtons);
XGE_API void xgeXuiMsgBoxSetResult(xge_xui_msg_box pBox, xge_xui_select_proc procResult, void* pUser);
XGE_API void xgeXuiMsgBoxSetModal(xge_xui_msg_box pBox, int bModal);
XGE_API void xgeXuiMsgBoxSetOpen(xge_xui_msg_box pBox, int bOpen);
XGE_API int xgeXuiMsgBoxIsOpen(xge_xui_msg_box pBox);
XGE_API int xgeXuiMsgBoxGetResult(xge_xui_msg_box pBox);
XGE_API void xgeXuiMsgBoxSetColors(xge_xui_msg_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iMessage, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText);
XGE_API int xgeXuiMsgBoxEvent(xge_xui_msg_box pBox, const xge_event_t* pEvent);
XGE_API int xgeXuiMsgBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiMsgBoxPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiInputBoxInit(xge_xui_input_box pBox, xge_xui_context pContext, xge_xui_widget pWidget, xui_font pFont);
XGE_API void xgeXuiInputBoxUnit(xge_xui_input_box pBox);
XGE_API void xgeXuiInputBoxSetText(xge_xui_input_box pBox, xui_font pFont, const char* sTitle, const char* sPrompt, const char* sInitial);
XGE_API void xgeXuiInputBoxSetResult(xge_xui_input_box pBox, xge_xui_text_submit_proc procResult, void* pUser);
XGE_API void xgeXuiInputBoxSetModal(xge_xui_input_box pBox, int bModal);
XGE_API void xgeXuiInputBoxSetOpen(xge_xui_input_box pBox, int bOpen);
XGE_API int xgeXuiInputBoxIsOpen(xge_xui_input_box pBox);
XGE_API int xgeXuiInputBoxGetResultCode(xge_xui_input_box pBox);
XGE_API char* xgeXuiInputBoxGetResult(xge_xui_input_box pBox);
XGE_API void xgeXuiInputBoxSetColors(xge_xui_input_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iPrompt, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText);
XGE_API int xgeXuiInputBoxEvent(xge_xui_input_box pBox, const xge_event_t* pEvent);
XGE_API int xgeXuiInputBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiInputBoxPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiPopupInit(xge_xui_popup pPopup, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiPopupUnit(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetOwner(xge_xui_popup pPopup, xge_xui_widget pOwner);
XGE_API void xgeXuiPopupSetClose(xge_xui_popup pPopup, xge_xui_click_proc procClose, void* pUser);
XGE_API void xgeXuiPopupSetOpen(xge_xui_popup pPopup, int bOpen);
XGE_API int xgeXuiPopupIsOpen(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetModal(xge_xui_popup pPopup, int bModal);
XGE_API void xgeXuiPopupSetAutoClose(xge_xui_popup pPopup, int bOutside, int bEscape);
XGE_API void xgeXuiPopupSetPlacement(xge_xui_popup pPopup, int iPlacement);
XGE_API void xgeXuiPopupSetAnchorRect(xge_xui_popup pPopup, xge_rect_t tAnchor);
XGE_API void xgeXuiPopupSetOffset(xge_xui_popup pPopup, float fX, float fY);
XGE_API void xgeXuiPopupSetFocusRestore(xge_xui_popup pPopup, xge_xui_widget pWidget);
XGE_API void xgeXuiPopupSetContentWidget(xge_xui_popup pPopup, xge_xui_widget pContent);
XGE_API void xgeXuiPopupSetContentSize(xge_xui_popup pPopup, float fW, float fH);
XGE_API void xgeXuiPopupSetAnchorPoint(xge_xui_popup pPopup, int iAnchorPoint);
XGE_API void xgeXuiPopupSetDirection(xge_xui_popup pPopup, int iDirection);
XGE_API void xgeXuiPopupSetGap(xge_xui_popup pPopup, float fGap);
XGE_API void xgeXuiPopupSetMatchOwnerWidth(xge_xui_popup pPopup, int bEnabled);
XGE_API void xgeXuiPopupSetConsumeInside(xge_xui_popup pPopup, int bEnabled);
XGE_API void xgeXuiPopupSetClosePolicy(xge_xui_popup pPopup, int iOutsidePolicy, int iOwnerPolicy, int iEscapePolicy);
XGE_API void xgeXuiPopupSetFocusPolicy(xge_xui_popup pPopup, int iFocusPolicy, xge_xui_widget pCustomFocus);
XGE_API xge_rect_t xgeXuiPopupGetViewportRect(xge_xui_popup pPopup);
XGE_API xge_rect_t xgeXuiPopupGetContentRect(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetScroll(xge_xui_popup pPopup, float fX, float fY);
XGE_API void xgeXuiPopupGetScroll(xge_xui_popup pPopup, float* pX, float* pY);
XGE_API void xgeXuiPopupApplyPlacement(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetBackground(xge_xui_popup pPopup, uint32_t iColor);
XGE_API void xgeXuiPopupSetBorder(xge_xui_popup pPopup, uint32_t iColor);
XGE_API int xgeXuiPopupEvent(xge_xui_popup pPopup, const xge_event_t* pEvent);
XGE_API int xgeXuiPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiPopupPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiComboBoxInit(xge_xui_combo_box pCombo, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiComboBoxUnit(xge_xui_combo_box pCombo);
XGE_API void xgeXuiComboBoxSetItems(xge_xui_combo_box pCombo, const char** arrItems, int iCount);
XGE_API void xgeXuiComboBoxSetItemData(xge_xui_combo_box pCombo, const xge_xui_combo_box_item_t* arrItems, int iCount);
XGE_API void xgeXuiComboBoxSetFont(xge_xui_combo_box pCombo, xui_font pFont);
XGE_API void xgeXuiComboBoxSetSelect(xge_xui_combo_box pCombo, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiComboBoxSetSelected(xge_xui_combo_box pCombo, int iIndex);
XGE_API int xgeXuiComboBoxGetSelected(xge_xui_combo_box pCombo);
XGE_API int xgeXuiComboBoxGetSelectedValue(xge_xui_combo_box pCombo);
XGE_API void xgeXuiComboBoxSetSelectedValue(xge_xui_combo_box pCombo, int iValue);
XGE_API void xgeXuiComboBoxSetEnabledItems(xge_xui_combo_box pCombo, const int* arrEnabled, int iCount);
XGE_API void xgeXuiComboBoxSetPopupHeight(xge_xui_combo_box pCombo, float fHeight);
XGE_API void xgeXuiComboBoxSetPopupMaxHeight(xge_xui_combo_box pCombo, float fHeight);
XGE_API void xgeXuiComboBoxSetPopupPlacement(xge_xui_combo_box pCombo, int iPlacement);
XGE_API void xgeXuiComboBoxSetMetrics(xge_xui_combo_box pCombo, float fItemHeight);
XGE_API void xgeXuiComboBoxSetColors(xge_xui_combo_box pCombo, uint32_t iNormal, uint32_t iHover, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iPopup);
XGE_API void xgeXuiComboBoxSetItemColors(xge_xui_combo_box pCombo, uint32_t iHover, uint32_t iSelected, uint32_t iDisabled, uint32_t iDisabledText);
XGE_API int xgeXuiComboBoxIsOpen(xge_xui_combo_box pCombo);
XGE_API int xgeXuiComboBoxGetState(xge_xui_combo_box pCombo);
XGE_API int xgeXuiComboBoxEvent(xge_xui_combo_box pCombo, const xge_event_t* pEvent);
XGE_API int xgeXuiComboBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiComboBoxPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiMenuInit(xge_xui_menu pMenu, xge_xui_context pContext);
XGE_API void xgeXuiMenuUnit(xge_xui_menu pMenu);
XGE_API void xgeXuiMenuBeginUpdate(xge_xui_menu pMenu);
XGE_API void xgeXuiMenuEndUpdate(xge_xui_menu pMenu);
XGE_API void xgeXuiMenuClear(xge_xui_menu pMenu);
XGE_API int xgeXuiMenuAddItem(xge_xui_menu pMenu, const xge_xui_menu_item_t* pItem);
XGE_API int xgeXuiMenuAddSeparator(xge_xui_menu pMenu);
XGE_API void xgeXuiMenuSetItems(xge_xui_menu pMenu, const xge_xui_menu_item_t* arrItems, int iCount);
XGE_API void xgeXuiMenuSetItemState(xge_xui_menu pMenu, int iIndex, int iState);
XGE_API void xgeXuiMenuSetFont(xge_xui_menu pMenu, xui_font pFont);
XGE_API void xgeXuiMenuSetSelect(xge_xui_menu pMenu, xge_xui_menu_select_proc procSelect, void* pUser);
XGE_API void xgeXuiMenuSetMetrics(xge_xui_menu pMenu, const xge_xui_menu_metrics_t* pMetrics);
XGE_API void xgeXuiMenuSetColors(xge_xui_menu pMenu, const xge_xui_menu_colors_t* pColors);
XGE_API void xgeXuiMenuOpenAt(xge_xui_menu pMenu, xge_xui_widget pOwner, float fX, float fY);
XGE_API void xgeXuiMenuOpenForOwner(xge_xui_menu pMenu, xge_xui_widget pOwner);
XGE_API void xgeXuiMenuClose(xge_xui_menu pMenu);
XGE_API int xgeXuiMenuIsOpen(xge_xui_menu pMenu);

#ifdef __cplusplus
}
#endif

#endif /* XGE_H */
