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
                    ${ROOT_THIRD_PARTY_DIR}/GWork/lib/
                    ./External/allegro/lib/
                    ./External/allegro_deps/lib/
                 )

file(GLOB_RECURSE SRC_LIST  Sources/*)

list(APPEND INCLUDE_DIRS
        Sources/
        Sources/editor/
        Sources/editor/ui/
        Sources/addons/nine-patch/
        Sources/addons/tweeny/
        Sources/addons/json/
        Sources/addons/file-watcher/
        ThirdParty/GWork/source/gwork/include/
        ThirdParty/GWork/source/platform/include/
        ThirdParty/angelscript/angelscript/include
        ThirdParty/angelscript/add_on
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
                        libGwork.a
                        libGworkAllegro5.a
                        libangelscript.dll.a
                        freetype
    )
else()
list(APPEND LIBRARIES_LIST
                        libGwork_LINUX.a
                        libGworkAllegro5_LINUX.a
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
