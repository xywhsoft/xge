/**
 * @file xge_input.h
 * @brief XGE 输入处理模块（键码定义）
 */

#ifndef XGE_INPUT_H
#define XGE_INPUT_H

/* ============================================================================
 * 键码定义 (与 sokol_app.h 兼容)
 * ============================================================================ */

#define XGE_KEY_INVALID         0
#define XGE_KEY_SPACE           32
#define XGE_KEY_APOSTROPHE      39  /* ' */
#define XGE_KEY_COMMA           44  /* , */
#define XGE_KEY_MINUS           45  /* - */
#define XGE_KEY_PERIOD          46  /* . */
#define XGE_KEY_SLASH           47  /* / */
#define XGE_KEY_0               48
#define XGE_KEY_1               49
#define XGE_KEY_2               50
#define XGE_KEY_3               51
#define XGE_KEY_4               52
#define XGE_KEY_5               53
#define XGE_KEY_6               54
#define XGE_KEY_7               55
#define XGE_KEY_8               56
#define XGE_KEY_9               57
#define XGE_KEY_SEMICOLON       59  /* ; */
#define XGE_KEY_EQUAL           61  /* = */
#define XGE_KEY_A               65
#define XGE_KEY_B               66
#define XGE_KEY_C               67
#define XGE_KEY_D               68
#define XGE_KEY_E               69
#define XGE_KEY_F               70
#define XGE_KEY_G               71
#define XGE_KEY_H               72
#define XGE_KEY_I               73
#define XGE_KEY_J               74
#define XGE_KEY_K               75
#define XGE_KEY_L               76
#define XGE_KEY_M               77
#define XGE_KEY_N               78
#define XGE_KEY_O               79
#define XGE_KEY_P               80
#define XGE_KEY_Q               81
#define XGE_KEY_R               82
#define XGE_KEY_S               83
#define XGE_KEY_T               84
#define XGE_KEY_U               85
#define XGE_KEY_V               86
#define XGE_KEY_W               87
#define XGE_KEY_X               88
#define XGE_KEY_Y               89
#define XGE_KEY_Z               90
#define XGE_KEY_LEFT_BRACKET    91  /* [ */
#define XGE_KEY_BACKSLASH       92  /* \ */
#define XGE_KEY_RIGHT_BRACKET   93  /* ] */
#define XGE_KEY_GRAVE_ACCENT    96  /* ` */
#define XGE_KEY_WORLD_1         161
#define XGE_KEY_WORLD_2         162
#define XGE_KEY_ESCAPE          256
#define XGE_KEY_ENTER           257
#define XGE_KEY_TAB             258
#define XGE_KEY_BACKSPACE       259
#define XGE_KEY_INSERT          260
#define XGE_KEY_DELETE          261
#define XGE_KEY_RIGHT           262
#define XGE_KEY_LEFT            263
#define XGE_KEY_DOWN            264
#define XGE_KEY_UP              265
#define XGE_KEY_PAGE_UP         266
#define XGE_KEY_PAGE_DOWN       267
#define XGE_KEY_HOME            268
#define XGE_KEY_END             269
#define XGE_KEY_CAPS_LOCK       280
#define XGE_KEY_SCROLL_LOCK     281
#define XGE_KEY_NUM_LOCK        282
#define XGE_KEY_PRINT_SCREEN    283
#define XGE_KEY_PAUSE           284
#define XGE_KEY_F1              290
#define XGE_KEY_F2              291
#define XGE_KEY_F3              292
#define XGE_KEY_F4              293
#define XGE_KEY_F5              294
#define XGE_KEY_F6              295
#define XGE_KEY_F7              296
#define XGE_KEY_F8              297
#define XGE_KEY_F9              298
#define XGE_KEY_F10             299
#define XGE_KEY_F11             300
#define XGE_KEY_F12             301
#define XGE_KEY_F13             302
#define XGE_KEY_F14             303
#define XGE_KEY_F15             304
#define XGE_KEY_F16             305
#define XGE_KEY_F17             306
#define XGE_KEY_F18             307
#define XGE_KEY_F19             308
#define XGE_KEY_F20             309
#define XGE_KEY_F21             310
#define XGE_KEY_F22             311
#define XGE_KEY_F23             312
#define XGE_KEY_F24             313
#define XGE_KEY_F25             314
#define XGE_KEY_KP_0            320
#define XGE_KEY_KP_1            321
#define XGE_KEY_KP_2            322
#define XGE_KEY_KP_3            323
#define XGE_KEY_KP_4            324
#define XGE_KEY_KP_5            325
#define XGE_KEY_KP_6            326
#define XGE_KEY_KP_7            327
#define XGE_KEY_KP_8            328
#define XGE_KEY_KP_9            329
#define XGE_KEY_KP_DECIMAL      330
#define XGE_KEY_KP_DIVIDE       331
#define XGE_KEY_KP_MULTIPLY     332
#define XGE_KEY_KP_SUBTRACT     333
#define XGE_KEY_KP_ADD          334
#define XGE_KEY_KP_ENTER        335
#define XGE_KEY_KP_EQUAL        336
#define XGE_KEY_LEFT_SHIFT      340
#define XGE_KEY_LEFT_CONTROL    341
#define XGE_KEY_LEFT_ALT        342
#define XGE_KEY_LEFT_SUPER      343
#define XGE_KEY_RIGHT_SHIFT     344
#define XGE_KEY_RIGHT_CONTROL   345
#define XGE_KEY_RIGHT_ALT       346
#define XGE_KEY_RIGHT_SUPER     347
#define XGE_KEY_MENU            348

/* ============================================================================
 * 修饰键标志
 * ============================================================================ */

#define XGE_MOD_SHIFT   0x01
#define XGE_MOD_CTRL    0x02
#define XGE_MOD_ALT     0x04
#define XGE_MOD_SUPER   0x08

/* ============================================================================
 * 输入状态检测辅助函数
 * ============================================================================ */

/**
 * @brief 检查按键是否刚刚按下（用于单次触发）
 */
static int s_arrKeyPressed[512] = {0};

static int xgeKeyPressed(int iKeyCode)
{
	int bResult = 0;
	if ( (iKeyCode >= 0) && (iKeyCode < 512) ) {
		if ( g_xge.arrKeyState[iKeyCode] && !s_arrKeyPressed[iKeyCode] ) {
			bResult = 1;
		}
		s_arrKeyPressed[iKeyCode] = g_xge.arrKeyState[iKeyCode];
	}
	return bResult;
}

/**
 * @brief 检查按键是否刚刚释放
 */
static int s_arrKeyReleased[512] = {0};

static int xgeKeyReleased(int iKeyCode)
{
	int bResult = 0;
	if ( (iKeyCode >= 0) && (iKeyCode < 512) ) {
		if ( !g_xge.arrKeyState[iKeyCode] && s_arrKeyReleased[iKeyCode] ) {
			bResult = 1;
		}
		s_arrKeyReleased[iKeyCode] = g_xge.arrKeyState[iKeyCode];
	}
	return bResult;
}

#endif /* XGE_INPUT_H */
