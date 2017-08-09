project(RobotTale)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

LINK_DIRECTORIES(/home/squall/RobotGame/anglescript_2.31.2/angelscript/lib/)

file(GLOB_RECURSE SRC_LIST   "Sources/*")

include_directories(Sources/
                    Sources/editor/
                    Sources/editor/ui/
                    Sources/addons/nine-patch/
                    Sources/addons/tweeny/
                    Sources/addons/json/                    
                    anglescript_2.31.2/angelscript/include
                    anglescript_2.31.2/add_on
                    )

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
    allegro
    allegro_image
    allegro_dialog
    allegro_font
    allegro_ttf
    allegro_primitives
    libangelscript.a
    freetype
    pthread
    )
