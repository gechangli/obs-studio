option(ENABLE_UI "Enables the OBS user interfaces" ON)
if(DISABLE_UI)
	message(STATUS "UI disabled")
	return()
elseif(ENABLE_UI)
	set(FIND_MODE REQUIRED)
else()
	set(FIND_MODE QUIET)
endif()

add_subdirectory(obs-frontend-api)

# ----------------------------------------------------------------------------

project(xiaomeilive)

set(ENABLE_WIN_UPDATER FALSE CACHE BOOL "Enable the windows updater")

if(DEFINED QTDIR${_lib_suffix})
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR${_lib_suffix}}")
elseif(DEFINED QTDIR)
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
elseif(DEFINED ENV{QTDIR${_lib_suffix}})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR${_lib_suffix}}")
elseif(DEFINED ENV{QTDIR})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()

find_package(Qt5Widgets ${FIND_MODE})
find_package(FFmpeg REQUIRED COMPONENTS avcodec avutil avformat)

if(NOT Qt5Widgets_FOUND)
	if (ENABLE_UI)
		message(FATAL_ERROR "Failed to find Qt5")
	else()
		message(STATUS "Qt5 not found - UI disabled")
		return()
	endif()
endif()


include_directories(${FFMPEG_INCLUDE_DIRS})
include_directories(SYSTEM "obs-frontend-api")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/libobs")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/deps/libff")
link_directories(/usr/local/lib)

find_package(Libcurl REQUIRED)
include_directories(${LIBCURL_INCLUDE_DIRS})
add_definitions(${LIBCURL_DEFINITIONS})

if(WIN32)
	include_directories(${OBS_JANSSON_INCLUDE_DIRS})
	include_directories(${BLAKE2_INCLUDE_DIR})

	set(xiaomeilive_PLATFORM_SOURCES
		platform-windows.cpp
		win-update/update-window.cpp
		win-update/win-update.cpp
		win-update/win-update-helpers.cpp
		obs.rc)
	set(xiaomeilive_PLATFORM_HEADERS
		win-update/update-window.hpp
		win-update/win-update.hpp
		win-update/win-update-helpers.hpp)
	set(xiaomeilive_PLATFORM_LIBRARIES
		crypt32
		blake2
		${OBS_JANSSON_IMPORT})

	if(ENABLE_WIN_UPDATER)
		add_definitions(-DENABLE_WIN_UPDATER)
	endif()
elseif(APPLE)
	set(xiaomeilive_PLATFORM_SOURCES
		platform-osx.mm)

	find_package(AppKit REQUIRED)

	find_library(AUDIOTOOLBOX AudioToolbox)
	mark_as_advanced(AUDIOTOOLBOX)
	include_directories(${AUDIOTOOLBOX})

	find_library(COREVIDEO CoreVideo)
	mark_as_advanced(COREVIDEO)
	include_directories(${COREVIDEO})

	find_library(COREMEDIA CoreMedia)
	mark_as_advanced(COREMEDIA)
	include_directories(${COREMEDIA})

	find_library(SECURITY Security)
	mark_as_advanced(SECURITY)
	include_directories(${SECURITY})

	find_library(VIDEODECODEACCELERATION VideoDecodeAcceleration)
	mark_as_advanced(VIDEODECODEACCELERATION)
	include_directories(${VIDEODECODEACCELERATION})

	find_library(VIDEOTOOLBOX VideoToolbox)
	mark_as_advanced(VIDEOTOOLBOX)
	include_directories(${VIDEOTOOLBOX})

	set(xiaomeilive_PLATFORM_LIBRARIES
		bz2
		z
		mp3lame
		x264
		iconv
		${APPKIT_LIBRARIES}
		${AUDIOTOOLBOX}
		${COREVIDEO}
		${COREMEDIA}
		${SECURITY}
		${VIDEODECODEACCELERATION}
		${VIDEOTOOLBOX})

	add_definitions(-fobjc-arc)

	option(ENABLE_SPARKLE_UPDATER "Enables updates via the Sparkle framework (don't forget to update the Info.plist for your .app)" OFF)
	if(ENABLE_SPARKLE_UPDATER)
		find_library(SPARKLE Sparkle)
		include_directories(${SPARKLE})
		set(xiaomeilive_PLATFORM_SOURCES
			${xiaomeilive_PLATFORM_SOURCES}
			sparkle-updater.mm)
		set(xiaomeilive_PLATFORM_LIBRARIES
			${xiaomeilive_PLATFORM_LIBRARIES}
			${SPARKLE})
		add_definitions(-DUPDATE_SPARKLE=1)
	endif()

elseif(UNIX)
        find_package(Qt5X11Extras REQUIRED)

	set(xiaomeilive_PLATFORM_SOURCES
		platform-x11.cpp)

        set(xiaomeilive_PLATFORM_LIBRARIES
                Qt5::X11Extras)
endif()

set(xiaomeilive_libffutil_SOURCES
	../deps/libff/libff/ff-util.c
	)
set(xiaomeilive_libffutil_HEADERS
	../deps/libff/libff/ff-util.h
	)

if(MSVC)
	set_source_files_properties(
		../deps/libff/libff/ff-util.c
		PROPERTIES COMPILE_FLAGS -Dinline=__inline
		)
endif()

set(xiaomeilive_SOURCES
	${xiaomeilive_PLATFORM_SOURCES}
	${xiaomeilive_libffutil_SOURCES}
	obs-app.cpp
	api-interface.cpp
	window-basic-main.cpp
	window-basic-stats.cpp
	window-basic-filters.cpp
	window-basic-settings.cpp
	window-basic-interaction.cpp
	window-basic-properties.cpp
	window-basic-auto-config.cpp
	window-basic-main-outputs.cpp
	window-basic-source-select.cpp
	window-basic-auto-config-test.cpp
	window-basic-main-scene-collections.cpp
	window-basic-main-transitions.cpp
	window-basic-main-dropfiles.cpp
	window-basic-main-profiles.cpp
	window-license-agreement.cpp
	window-basic-status-bar.cpp
	window-basic-adv-audio.cpp
	window-basic-transform.cpp
	window-basic-preview.cpp
	window-namedialog.cpp
	window-log-reply.cpp
	window-projector.cpp
	window-remux.cpp
	properties-view.cpp
	focus-list.cpp
	menu-button.cpp
	double-slider.cpp
	volume-control.cpp
	adv-audio-control.cpp
	item-widget-helpers.cpp
	visibility-checkbox.cpp
	locked-checkbox.cpp
	vertical-scroll-area.cpp
	visibility-item-widget.cpp
	slider-absoluteset-style.cpp
	source-list-widget.cpp
	qt-display.cpp
	crash-report.cpp
	hotkey-edit.cpp
	source-label.cpp
	remote-text.cpp
	audio-encoders.cpp
	qt-wrappers.cpp
	live-cast.cpp)

set(xiaomeilive_HEADERS
	${xiaomeilive_PLATFORM_HEADERS}
	${xiaomeilive_libffutil_HEADERS}
	obs-app.hpp
	platform.hpp
	window-main.hpp
	window-basic-main.hpp
	window-basic-stats.hpp
	window-basic-filters.hpp
	window-basic-settings.hpp
	window-basic-interaction.hpp
	window-basic-properties.hpp
	window-basic-auto-config.hpp
	window-basic-main-outputs.hpp
	window-basic-source-select.hpp
	window-license-agreement.hpp
	window-basic-status-bar.hpp
	window-basic-adv-audio.hpp
	window-basic-transform.hpp
	window-basic-preview.hpp
	window-namedialog.hpp
	window-log-reply.hpp
	window-projector.hpp
	window-remux.hpp
	properties-view.hpp
	properties-view.moc.hpp
	display-helpers.hpp
	double-slider.hpp
	focus-list.hpp
	menu-button.hpp
	mute-checkbox.hpp
	volume-control.hpp
	adv-audio-control.hpp
	item-widget-helpers.hpp
	visibility-checkbox.hpp
	locked-checkbox.hpp
	vertical-scroll-area.hpp
	visibility-item-widget.hpp
	slider-absoluteset-style.hpp
	source-list-widget.hpp
	qt-display.hpp
	crash-report.hpp
	hotkey-edit.hpp
	source-label.hpp
	remote-text.hpp
	audio-encoders.hpp
	qt-wrappers.hpp
	live-cast.hpp)

set(xiaomeilive_UI
	forms/NameDialog.ui
	forms/AutoConfigStartPage.ui
	forms/AutoConfigVideoPage.ui
	forms/AutoConfigStreamPage.ui
	forms/AutoConfigTestPage.ui
	forms/OBSLicenseAgreement.ui
	forms/OBSLogReply.ui
	forms/OBSBasic.ui
	forms/OBSBasicTransform.ui
	forms/OBSBasicFilters.ui
	forms/OBSBasicSettings.ui
	forms/OBSBasicSourceSelect.ui
	forms/OBSBasicInteraction.ui
	forms/OBSUpdate.ui
	forms/OBSRemux.ui)

set(xiaomeilive_QRC
	forms/obs.qrc)

qt5_wrap_ui(xiaomeilive_UI_HEADERS ${xiaomeilive_UI})
qt5_add_resources(xiaomeilive_QRC_SOURCES ${xiaomeilive_QRC})

add_executable(xiaomeilive WIN32
	${xiaomeilive_SOURCES}
	${xiaomeilive_HEADERS}
	${xiaomeilive_UI_HEADERS}
	${xiaomeilive_QRC_SOURCES})

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_output_suffix "64")
	else()
		set(_output_suffix "32")
	endif()

	set_target_properties(xiaomeilive
		PROPERTIES
			OUTPUT_NAME "xiaomeilive{_output_suffix}")
endif()

target_link_libraries(xiaomeilive
	libobs
	Qt5::Widgets
	obs-frontend-api
	${FFMPEG_LIBRARIES}
	${LIBCURL_LIBRARIES}
	${xiaomeilive_PLATFORM_LIBRARIES})

define_graphic_modules(xiaomeilive)

install_obs_core(xiaomeilive)
install_obs_data(xiaomeilive data obs-studio)

if (UNIX AND UNIX_STRUCTURE AND NOT APPLE)
	install(FILES dist/obs.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications)
	install(FILES forms/images/obs.png
		DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/256x256/apps)
endif()

add_subdirectory(frontend-plugins)
if(WIN32)
	add_subdirectory(win-update/updater)
endif()
