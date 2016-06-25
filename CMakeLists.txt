cmake_minimum_required(VERSION 2.8)
find_package(PkgConfig)


pkg_check_modules(GSTREAMER_ALL REQUIRED gstreamer-1.0 gstreamer-base-1.0)

add_compile_options( ${GSTREAMER_ALL_CFLAGS} )

add_library(gstterminator SHARED gstterminator.c)

target_link_libraries(gstterminator ${GSTREAMER_ALL_LDFLAGS})




# make VERBOSE=1


# <XPREFIX>_FOUND
# <XPREFIX>_LIBRARIES
# <XPREFIX>_LIBRARY_DIRS
# <XPREFIX>_LDFLAGS
# <XPREFIX>_LDFLAGS_OTHER
# <XPREFIX>_INCLUDE_DIRS
# <XPREFIX>_CFLAGS
# <XPREFIX>_CFLAGS_OTHER


# foreach (var ${GSTREAMER_ALL_LDFLAGS})
#     message(${var})
# endforeach(var)
