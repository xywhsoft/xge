cmake_minimum_required(VERSION 3.18)
project(xge_android C)

add_library(xge_android SHARED
	../../xge.c
	xge_android_main.c
)

target_include_directories(xge_android PRIVATE
	../..
)

target_compile_definitions(xge_android PRIVATE
	SOKOL_GLES3
	XGE_NO_AUDIO
)

target_link_libraries(xge_android
	GLESv3
	EGL
	android
	log
	m
)
