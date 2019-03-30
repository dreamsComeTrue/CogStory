project("CogStory")
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(ROOT_THIRD_PARTY_DIR ./ThirdParty)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Cotire/CMake)

set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -static-libgcc -static-libstdc++ -Wl,-allow-multiple-definition -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic")

if (WIN32)
    set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic")
endif (WIN32)

include(cotire)

cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0058 NEW)
link_directories(   ${ROOT_THIRD_PARTY_DIR}/angelscript/angelscript/lib/Linux/
                    ${ROOT_THIRD_PARTY_DIR}/angelscript/angelscript/lib/Win64/
                    ${ROOT_THIRD_PARTY_DIR}/allegro5_static/lib/
#                    ${ROOT_THIRD_PARTY_DIR}/allegro5/bin/lib/
                 )

file(GLOB_RECURSE SRC_LIST  Sources/*
                            ${ROOT_THIRD_PARTY_DIR}/imgui/imgui.cpp
                            ${ROOT_THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
                            ${ROOT_THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
                            ${ROOT_THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
                            ${ROOT_THIRD_PARTY_DIR}/imgui/examples/imgui_impl_allegro5.cpp)

if (WIN32)
    list(APPEND INCLUDE_DIRS ${ROOT_THIRD_PARTY_DIR}/allegro5_static/include)
else()
    list(APPEND INCLUDE_DIRS    ${ROOT_THIRD_PARTY_DIR}/allegro5/include
                                ${ROOT_THIRD_PARTY_DIR}/allegro5/bin/include)
endif (WIN32)

list(APPEND INCLUDE_DIRS
        Sources/
        Sources/editor/
        Sources/addons/nine-patch/
        Sources/addons/tweeny/
        Sources/addons/json/
        Sources/addons/file-watcher/
        ${ROOT_THIRD_PARTY_DIR}/angelscript/angelscript/include
        ${ROOT_THIRD_PARTY_DIR}/angelscript/add_on
        ${ROOT_THIRD_PARTY_DIR}/imgui/
        ${ROOT_THIRD_PARTY_DIR}/imgui/examples/
    )

set(STATIC_BUILD 0)
if (STATIC_BUILD)
    set(GAME_BUILD_TYPE "-static")
else()
    set(GAME_BUILD_TYPE "")
endif (STATIC_BUILD)

list(APPEND LIBRARIES_LIST
    
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
                        allegro_monolith${GAME_BUILD_TYPE}
                        libangelscript.dll.a)
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
                        allegro${GAME_BUILD_TYPE}
                        allegro_image${GAME_BUILD_TYPE}
                        allegro_font${GAME_BUILD_TYPE}
                        allegro_ttf${GAME_BUILD_TYPE}
                        allegro_primitives${GAME_BUILD_TYPE}
                        allegro_audio${GAME_BUILD_TYPE}
                        allegro_acodec${GAME_BUILD_TYPE}                        
                        libangelscript.a
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

cotire(${PROJECT_NAME})
