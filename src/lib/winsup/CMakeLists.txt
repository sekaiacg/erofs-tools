# This lib provides symbols and headers which the Windows runtime lacks,
# acting as a small POSIX compatibility shim.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

    set(TARGET winsup)

    set(winsup_srcs
        "src/compat.c"
        "src/fnmatch.c"
    )
    list(TRANSFORM winsup_srcs PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

    add_library(${TARGET} STATIC ${winsup_srcs})

    target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

endif ()
