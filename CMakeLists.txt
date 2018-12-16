project("Cog Story")
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(ROOT_THIRD_PARTY_DIR ./ThirdParty)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Cotire/CMake)
include(cotire)

cmake_policy(SET CMP0015 NEW)
link_directories(   ${ROOT_THIRD_PARTY_DIR}/angelscript/angelscript/lib/
                    ./ThirdParty/allegro5/bin/lib/
					./ThirdParty/allegro5/bin/include/
                    ./ThirdParty/allegro5/deps/lib/
                 )

file(GLOB_RECURSE SRC_LIST  Sources/*
	                        ThirdParty/imgui/imgui.cpp
                            ThirdParty/imgui/imgui_draw.cpp
                            ThirdParty/imgui/imgui_widgets.cpp
							ThirdParty/imgui/imguifilesystem.cpp
							ThirdParty/imgui/imgui_demo.cpp
                            ThirdParty/imgui/examples/imgui_impl_allegro5.cpp)

if (WIN32)
list(APPEND INCLUDE_DIRS
        ThirdParty/allegro5/include
        ThirdParty/allegro5/addons/acodec
        ThirdParty/allegro5/addons/audio
        ThirdParty/allegro5/addons/font
        ThirdParty/allegro5/addons/image
        ThirdParty/allegro5/addons/primitives
        ThirdParty/allegro5/addons/ttf
    )
else()
endif (WIN32)


list(APPEND INCLUDE_DIRS
        Sources/
        Sources/editor/
        Sources/addons/nine-patch/
        Sources/addons/tweeny/
        Sources/addons/json/
        Sources/addons/file-watcher/
        ThirdParty/angelscript/angelscript/include
        ThirdParty/angelscript/add_on
        ThirdParty/imgui/
        ThirdParty/imgui/examples/
    )

set(STATIC_BUILD 0)
if (STATIC_BUILD)
    set(GAME_BUILD_TYPE "-static")
else()
    set(GAME_BUILD_TYPE "")
endif (STATIC_BUILD)

list(APPEND LIBRARIES_LIST
    allegro${GAME_BUILD_TYPE}
    allegro_image${GAME_BUILD_TYPE}
    allegro_font${GAME_BUILD_TYPE}
    allegro_ttf${GAME_BUILD_TYPE}
    allegro_primitives${GAME_BUILD_TYPE}
    allegro_audio${GAME_BUILD_TYPE}
    allegro_acodec${GAME_BUILD_TYPE}
)

if (WIN32)
    if (STATIC_BUILD)
        list(APPEND LIBRARIES_LIST
                        psapi
                        shlwapi
                        winmm
                        opengl32
                        dsound
						zlib						
                        )
    endif (STATIC_BUILD)

    list(APPEND LIBRARIES_LIST
                        libangelscript.dll.a
						)
else()
    if (STATIC_BUILD)
        list(APPEND LIBRARIES_LIST
                GL
                X11
                Xi
                Xcursor
                openal
                asound
				jpeg
                png16
                vorbisfile
                vorbis
                ogg
                freetype						
                )
    endif (STATIC_BUILD)

	list(APPEND LIBRARIES_LIST
                        libangelscript_LINUX.a
						pthread
						)
endif (WIN32)

include_directories(${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} ${LIBRARIES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ../
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ../
                      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ../
                      )

#set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(${PROJECT_NAME})
