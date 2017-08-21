project(obs-ffmpeg)

if(MSVC)
	set(obs-ffmpeg_PLATFORM_DEPS
		w32-pthreads)
elseif(APPLE)
	find_library(FOUNDATION Foundation)
	mark_as_advanced(FOUNDATION)
	include_directories(${FOUNDATION})

	find_library(OPENCL OpenCL)
	mark_as_advanced(OPENCL)
	include_directories(${OPENCL})

	find_library(COREIMAGE CoreImage)
	mark_as_advanced(COREIMAGE)
	include_directories(${COREIMAGE})

	find_library(COREGRAPHICS CoreGraphics)
	mark_as_advanced(COREGRAPHICS)
	include_directories(${COREGRAPHICS})

	find_library(AVFOUNDATION AVFoundation)
	mark_as_advanced(AVFOUNDATION)
	include_directories(${AVFOUNDATION})

	find_library(OPENGL OpenGL)
	mark_as_advanced(OPENGL)
	include_directories(${OPENGL})

	find_library(APPKIT AppKit)
	mark_as_advanced(APPKIT)
	include_directories(${APPKIT})

	set(obs-ffmpeg_PLATFORM_DEPS
		postproc
		avresample
		${APPKIT}
		${FOUNDATION}
		${COREGRAPHICS}
		${AVFOUNDATION}
		${COREIMAGE}
		${OPENGL}
		${OPENCL}
		${COREFOUNDATION}
		${AUDIOTOOLBOX}
		${COREVIDEO}
		${COREMEDIA}
		${SECURITY}
		${VIDEODECODEACCELERATION}
		${VIDEOTOOLBOX})
endif()

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec avfilter avdevice avutil swscale avformat swresample)
include_directories(${FFMPEG_INCLUDE_DIRS})
link_directories(/usr/local/lib)

set(obs-ffmpeg_HEADERS
	obs-ffmpeg-formats.h
	obs-ffmpeg-compat.h
	closest-pixel-format.h)
set(obs-ffmpeg_SOURCES
	obs-ffmpeg.c
	obs-ffmpeg-audio-encoders.c
	obs-ffmpeg-nvenc.c
	obs-ffmpeg-output.c
	obs-ffmpeg-mux.c
	obs-ffmpeg-source.c)

add_library(obs-ffmpeg MODULE
	${obs-ffmpeg_HEADERS}
	${obs-ffmpeg_SOURCES})
target_link_libraries(obs-ffmpeg
	libobs
	media-playback
	${obs-ffmpeg_PLATFORM_DEPS}
	${FFMPEG_LIBRARIES})

install_obs_plugin_with_data(obs-ffmpeg data)

add_subdirectory(ffmpeg-mux)
