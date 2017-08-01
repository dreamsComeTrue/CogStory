project(RobotTale)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

LINK_DIRECTORIES(   C:/Users/B0634624/RobotTales/allegro-5.0.10-mingw-4.7.0/lib
                    C:/Users/B0634624/RobotTales/anglescript_2.31.2/angelscript/lib)

file(GLOB_RECURSE SRC_LIST   "Sources/*")

include_directories(Sources/
                    Sources/editor/
                    Sources/editor/ui/
                    Sources/addons/nine-patch/
                    Sources/addons/tweeny/
                    Sources/addons/json/
                    C:/Users/B0634624/RobotTales/allegro-5.0.10-mingw-4.7.0/include
                    C:/Users/B0634624/RobotTales/anglescript_2.31.2/angelscript/include
                    C:/Users/B0634624/RobotTales/anglescript_2.31.2/add_on
                    )

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
    liballegro-5.0.10-static-mt.a
    liballegro_image-5.0.10-static-mt.a
    liballegro_dialog-5.0.10-static-mt.a
    liballegro_font-5.0.10-static-mt.a
    liballegro_ttf-5.0.10-static-mt.a
    liballegro_primitives-5.0.10-static-mt.a
    libfreetype-2.4.8-static-mt.a
    angelscript
    winmm
    gdiplus
    psapi
    shlwapi
    opengl32)
