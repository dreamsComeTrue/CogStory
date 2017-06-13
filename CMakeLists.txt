project(RobotTale)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

file(GLOB_RECURSE SRC_LIST   "Sources/*")

include_directories(Sources/
                    Sources/addons/nine-patch/
                    Sources/addons/tweeny/
                    )

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} allegro allegro_image allegro_dialog allegro_font allegro_ttf)
