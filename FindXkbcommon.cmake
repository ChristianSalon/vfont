# Find xkbcommon
#
#   Variables:
#
#       Xkbcommon_FOUND
#       Xkbcommon_INCLUDE_DIRS
#       Xkbcommon_LIBRARIES
#       Xkbcommon_DEFINITIONS

find_package(PkgConfig)
pkg_check_modules(PC_Xkbcommon QUIET xkbcommon)

# Find include directories for xkbcommon
find_path(Xkbcommon_INCLUDE_DIR
    NAMES xkbcommon/xkbcommon.h
)

# Find xkbcommon library
find_library(Xkbcommon_LIBRARY
    NAMES xkbcommon
)

set(Xkbcommon_INCLUDE_DIRS "${Xkbcommon_INCLUDE_DIR}")
set(Xkbcommon_LIBRARIES "${Xkbcommon_LIBRARY}")
set(Xkbcommon_DEFINITIONS "${PC_Xkbcommon_CFLAGS_OTHER}")

if(Xkbcommon_INCLUDE_DIR AND Xkbcommon_LIBRARIES)
    set(Xkbcommon_FOUND TRUE)
endif()
