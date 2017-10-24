project(RobotTale)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(ROOT_THIRD_PARTY_DIR "/home/squall/RobotGame/ThirdParty")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

LINK_DIRECTORIES(   ${ROOT_THIRD_PARTY_DIR}/anglescript_2.31.2/angelscript/lib/
                    ${ROOT_THIRD_PARTY_DIR}/Box2D/lib/Box2D/
                 )
file(GLOB_RECURSE SRC_LIST   "Sources/*")
file(GLOB_RECURSE IMGUI_SRC_LIST   "ThirdParty/imgui/*.cpp")

include_directories(Sources/
                    Sources/editor/
                    Sources/editor/ui/
                    Sources/addons/nine-patch/
                    Sources/addons/tweeny/
                    Sources/addons/json/                    
                    ThirdParty/
                    ThirdParty/Box2D/Box2D/
                    ThirdParty/anglescript_2.31.2/angelscript/include
                    ThirdParty/anglescript_2.31.2/add_on
                    )

add_executable(${PROJECT_NAME} ${SRC_LIST} ${IMGUI_SRC_LIST})
target_link_libraries(${PROJECT_NAME}
    allegro
    allegro_image
    allegro_font
    allegro_ttf
    allegro_primitives
    Box2D
    libangelscript.a
    freetype
    pthread
    boost_system
    boost_filesystem
    )
