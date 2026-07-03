#!/usr/bin/env python3
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]


REQUIRED_FILES = [
	"build_android_ndk.sh",
	"build_android_ndk.bat",
	"check_platform_all.bat",
	"check_platform_all.sh",
	"check_script_docs.bat",
	"check_script_docs.sh",
	"build_android_apk.sh",
	"build_android_apk.bat",
	"run_android_apk.sh",
	"run_android_apk.bat",
	"build_ios_sim_exe.sh",
	"run_ios_sim_exe.sh",
	"build_web_exe.sh",
	"build_web_exe.bat",
	"serve_web_exe.sh",
	"serve_web_exe.bat",
	"build_linux_x11_exe.sh",
	"build_linux_egl_exe.sh",
	"build_board_linux_egl_exe.sh",
	"build_egl_surfaceless_exe.sh",
	"check_board_linux_egl.sh",
	"build_macos_exe.sh",
	"examples/offscreen/main.c",
	"examples/egl_surfaceless/main.c",
	"platform/android/AndroidManifest.xml",
	"platform/android/CMakeLists.txt",
	"platform/android/xge_android_main.c",
	"platform/android/gradle/settings.gradle",
	"platform/android/gradle/build.gradle",
	"platform/android/gradle/app/build.gradle",
	"platform/ios/Info.plist",
	"platform/ios/xge_ios_main.m",
	"platform/web/shell.html",
]


def _read(relpath):
	return (ROOT / relpath).read_text(encoding="utf-8")


def _check_contains(relpath, needle, errors):
	text = _read(relpath)
	if needle not in text:
		errors.append(f"{relpath}: missing `{needle}`")


def main():
	errors = []
	for relpath in REQUIRED_FILES:
		if not (ROOT / relpath).exists():
			errors.append(f"missing file: {relpath}")

	if not errors:
		_check_contains("platform/android/AndroidManifest.xml", 'android:value="xge_android"', errors)
		_check_contains("platform/android/AndroidManifest.xml", 'android.app.NativeActivity', errors)
		_check_contains("platform/android/AndroidManifest.xml", 'android:glEsVersion="0x00030000"', errors)
		_check_contains("platform/android/gradle/app/build.gradle", 'applicationId "dev.xge.smoke"', errors)
		_check_contains("platform/android/gradle/app/build.gradle", 'path "../../CMakeLists.txt"', errors)
		_check_contains("platform/android/CMakeLists.txt", "add_library(xge_android SHARED", errors)
		_check_contains("platform/android/xge_android_main.c", "sokol_main", errors)
		_check_contains("platform/android/xge_android_main.c", "SOKOL_GLES3", errors)

		_check_contains("platform/ios/Info.plist", "dev.xge.mvp", errors)
		_check_contains("platform/ios/Info.plist", "opengles-3", errors)
		_check_contains("platform/ios/xge_ios_main.m", "sokol_main", errors)
		_check_contains("platform/ios/xge_ios_main.m", "SOKOL_GLES3", errors)

		_check_contains("platform/web/shell.html", 'id="canvas"', errors)
		_check_contains("platform/web/shell.html", "{{{ SCRIPT }}}", errors)
		_check_contains("build_web_exe.sh", "--shell-file platform/web/shell.html", errors)
		_check_contains("build_web_exe.bat", "--shell-file platform/web/shell.html", errors)
		_check_contains("examples/offscreen/main.c", "xgeEGLInit", errors)
		_check_contains("examples/offscreen/main.c", "sLastStage", errors)
		_check_contains("examples/egl_surfaceless/main.c", "bSurfaceless", errors)
		_check_contains("examples/egl_surfaceless/main.c", "sLastStage", errors)
		_check_contains("check_board_linux_egl.sh", "build_board_linux_egl_exe.sh", errors)
		_check_contains("check_board_linux_egl.sh", "build_egl_surfaceless_exe.sh", errors)

	if errors:
		print("[XGE] Platform scaffold check failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1

	print("[XGE] Platform scaffold check passed")
	return 0


if __name__ == "__main__":
	sys.exit(main())
