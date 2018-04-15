project(RobotTales)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(ROOT_THIRD_PARTY_DIR "./ThirdParty")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Cotire/CMake")
include(cotire)

cmake_policy(SET CMP0015 NEW)
link_directories(   ${ROOT_THIRD_PARTY_DIR}/anglescript/angelscript/lib/
                    ${ROOT_THIRD_PARTY_DIR}/GWork/lib/
                    "./External/allegro/lib/"
                    "./External/allegro_deps/lib/"
                 )
file(GLOB_RECURSE SRC_LIST   "Sources/*")

if (WIN32)
include_directories(Sources/
                    Sources/editor/
                    Sources/editor/ui/
                    Sources/addons/nine-patch/
                    Sources/addons/tweeny/
                    Sources/addons/json/                    
                    ThirdParty/
                    ThirdParty/GWork/source/gwork/include/
                    ThirdParty/GWork/source/platform/include/
                    ThirdParty/anglescript/angelscript/include
                    ThirdParty/anglescript/add_on
                    External/allegro/include
                    )
else()
    include_directories(Sources/
                        Sources/editor/
                        Sources/editor/ui/
                        Sources/addons/nine-patch/
                        Sources/addons/tweeny/
                        Sources/addons/json/
                        ThirdParty/
                        ThirdParty/GWork/source/gwork/include/
                        ThirdParty/GWork/source/platform/include/
                        ThirdParty/anglescript/angelscript/include
                        ThirdParty/anglescript/add_on
                        )
endif (WIN32)


add_executable(${PROJECT_NAME} ${SRC_LIST} ${IMGUI_SRC_LIST})

if (WIN32)
target_link_libraries(${PROJECT_NAME}
    libGwork.a
    libGworkAllegro5.a
    libangelscript.dll.a

    allegro
    allegro_image
    allegro_font
    allegro_ttf
    allegro_primitives
    allegro_dialog
    allegro_audio
    allegro_acodec
    freetype
    pthread
    )
else()
    target_link_libraries(${PROJECT_NAME}
        libGwork_LINUX.a
        libGworkAllegro5_LINUX.a
        libangelscript_LINUX.a

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
endif (WIN32)

set_target_properties(${PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ../
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ../
                      )

#set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(${PROJECT_NAME})
