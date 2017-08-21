project(ffmpeg-mux)

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec avutil avformat)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(ffmpeg-mux_SOURCES
	ffmpeg-mux.c)

set(ffmpeg-mux_HEADERS
	ffmpeg-mux.h)

add_executable(ffmpeg-mux
	${ffmpeg-mux_SOURCES}
	${ffmpeg-mux_HEADERS})

find_library(COREFOUNDATION CoreFoundation)
mark_as_advanced(COREFOUNDATION)
include_directories(${COREFOUNDATION})
link_directories(/usr/local/lib)

target_link_libraries(ffmpeg-mux
	bz2
	z
	x264
	mp3lame
	xvidcore
	swresample
	iconv
	${FFMPEG_LIBRARIES}
	${APPKIT_LIBRARIES}
	${COREFOUNDATION}
	${AUDIOTOOLBOX}
	${COREVIDEO}
	${COREMEDIA}
	${SECURITY}
	${VIDEODECODEACCELERATION}
	${VIDEOTOOLBOX})

if(WIN32)
	set_target_properties(ffmpeg-mux
		PROPERTIES
			OUTPUT_NAME "ffmpeg-mux${_output_suffix}")
endif()

install_obs_datatarget(ffmpeg-mux "obs-plugins/obs-ffmpeg")
