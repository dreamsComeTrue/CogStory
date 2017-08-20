# Find Direct2D

find_path(DIRECT2D_INCLUDE_DIRS
    NAMES
        d2d1.h
    PATHS
        "$ENV{DXSDK_DIR}/Include"
        "$ENV{PROGRAMFILES}/Microsoft DirectX SDK*/Include"
        "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/*/Include"
    	"C:/Program Files (x86)/Windows Kits/*/include/um"
    	"C:/Program Files/Windows Kits/*/include/um"
        "C:/Program Files (x86)/Windows Kits/10/include/10.0.*.0/um"
        "C:/Program Files/Windows Kits/10/include/10.0.*.0/um"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(DIRECT2D_LIBRARY_PATHS
        "$ENV{DXSDK_DIR}/Lib/x64/"
        "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64/"
        "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64/"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.14393.0/um/x64/"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10586.0/um/x64/"
    )
else()
    SET(DIRECT2D_LIBRARY_PATHS
        "$ENV{DXSDK_DIR}/Lib/x86/"
        "C:/Program Files (x86)/Windows Kits/8.1/lib/winv6.3/um/x86/"
        "C:/Program Files (x86)/Windows Kits/8.0/lib/win8/um/x86/"
        "C:/Program Files (x86)/Windows Kits/10/lib/10.0.14393.0/um/x86/"
        "C:/Program Files (x86)/Windows Kits/10/lib/10.0.10586.0/um/x86/"
    )
endif()

find_library(D2D1_LIB d2d1 ${DIRECT2D_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(DWRITE_LIB dwrite ${DIRECT2D_LIBRARY_PATHS} NO_DEFAULT_PATH)
set(DIRECT2D_LIBRARIES ${D2D1_LIB} ${DWRITE_LIB})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DIRECT2D
    DEFAULT_MSG DIRECTX_ROOT_DIR
    DIRECT2D_LIBRARIES DIRECT2D_INCLUDE_DIRS
)
mark_as_advanced(DIRECT2D_INCLUDE_DIRS D2D1_LIB DWRITE_LIB)
