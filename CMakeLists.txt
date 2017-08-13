project(RobotTale)
cmake_minimum_required(VERSION 2.8)

set(BIN_DIR Binaries)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(ROOT_THIRD_PARTY_DIR "/home/squall/RobotGame/ThirdParty")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

LINK_DIRECTORIES(${ROOT_THIRD_PARTY_DIR}/anglescript_2.31.2/angelscript/lib/
                 ${ROOT_THIRD_PARTY_DIR}/GWork/lib/)

file(GLOB_RECURSE SRC_LIST   "Sources/*")

include_directories(Sources/
                    Sources/editor/
                    Sources/editor/ui/
                    Sources/addons/nine-patch/
                    Sources/addons/tweeny/
                    Sources/addons/json/                    
                    ThirdParty/anglescript_2.31.2/angelscript/include
                    ThirdParty/anglescript_2.31.2/add_on
                    ThirdParty/GWork/source/gwork/include
                    ThirdParty/GWork/source/platform/include
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
    libGwork.a
    libGworkAllegro5.a
    freetype
    pthread
    stdc++fs
    )
