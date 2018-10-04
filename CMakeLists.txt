project(RobotTales)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(ROOT_THIRD_PARTY_DIR ./ThirdParty)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Cotire/CMake)
include(cotire)

cmake_policy(SET CMP0015 NEW)
link_directories(   ${ROOT_THIRD_PARTY_DIR}/angelscript/angelscript/lib/
                    ./External/allegro/lib/
                    ./External/allegro_deps/lib/
                 )

file(GLOB_RECURSE SRC_LIST  Sources/*
	                        ThirdParty/imgui/imgui.cpp
							ThirdParty/imgui/imgui_draw.cpp
							ThirdParty/imgui/imgui_widgets.cpp
							ThirdParty/imgui/imgui_demo.cpp
							ThirdParty/imgui/examples/imgui_impl_allegro5.cpp)

list(APPEND INCLUDE_DIRS
        Sources/
        Sources/editor/
        Sources/editor/ui/
        Sources/addons/nine-patch/
        Sources/addons/tweeny/
        Sources/addons/json/
        Sources/addons/file-watcher/
        ThirdParty/angelscript/angelscript/include
        ThirdParty/angelscript/add_on
		ThirdParty/imgui/
		ThirdParty/imgui/examples/
    )

if (WIN32)
list(APPEND INCLUDE_DIRS
    External/allegro/include
    )
else()
endif (WIN32)

include_directories(${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC_LIST})

if (WIN32)
list(APPEND LIBRARIES_LIST
                        libangelscript.dll.a
                        freetype
    )
else()
list(APPEND LIBRARIES_LIST
                        libangelscript_LINUX.a
    )
endif (WIN32)

list(APPEND LIBRARIES_LIST
    allegro
    allegro_image
    allegro_font
    allegro_ttf
    allegro_primitives
	allegro_dialog
	allegro_audio
    allegro_acodec
    pthread
)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ../
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ../
                      )

#set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(${PROJECT_NAME})
