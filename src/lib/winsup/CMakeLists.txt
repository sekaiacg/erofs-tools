# This lib provides symbols and headers which the Windows runtime lacks,
# acting as a small POSIX compatibility shim.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

    set(TARGET winsup)

    set(MMAN_WIN32_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mman-win32")

    set(winsup_srcs
        "src/errno.c"
        "src/fs.c"
        "src/fsinfo.c"
        "src/init.c"
        "src/io.c"
        "src/mmap.c"
        "src/path.c"
        "src/random.c"
        "src/stdio.c"
        "src/symlink.c"
        "src/sysconf.c"
        "src/xattr.c"
        "src/fnmatch.c"
    )
    list(TRANSFORM winsup_srcs PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
    list(APPEND winsup_srcs "${MMAN_WIN32_DIR}/mman.c")

    # the POSIX entry points live in src/mmap.c, they add the 64 KiB alignment
    # handling the Win32 API requires
    set_source_files_properties("${MMAN_WIN32_DIR}/mman.c" PROPERTIES
        COMPILE_DEFINITIONS "mmap=mman_mmap;munmap=mman_munmap"
    )

    add_library(${TARGET} STATIC ${winsup_srcs})

    target_include_directories(${TARGET} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${MMAN_WIN32_DIR}"
    )

endif ()
