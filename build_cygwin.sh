OUT="./out"
BUILD_DIR="./"
EROFS_VERSION="$(cd src/lib/erofs-utils;tag=$(git describe --exact-match 2>/dev/null);head=$(git rev-parse --verify HEAD 2>/dev/null); printf '%s-g%.8s' $tag $head)"

EXT=".exe"

cmake_build()
{
    local TARGET=$1
    local METHOD=$2
    local ABI=$3

    local MAKE_CMD=""
    local BUILD_METHOD=
    if [[ $METHOD == "Ninja" ]]; then
        BUILD_METHOD="-G Ninja"
        MAKE_CMD="ninja -C $OUT"
    elif [[ $METHOD == "make" ]]; then
        MAKE_CMD="make -C $OUT -j$(nproc --all)"
    fi

    if [[ $TARGET == "Cygwin" ]]; then
        cmake -S ${BUILD_DIR} -B ${OUT} ${BUILD_METHOD} \
            -DCMAKE_SYSTEM_NAME="CYGWIN" \
            -DCMAKE_SYSTEM_PROCESSOR="x86_64" \
            -DCMAKE_BUILD_TYPE="Release" \
            -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
            -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
            -DCMAKE_C_COMPILER="${LLVM_CYGWIN_PATH}/toolchain/bin/x86_64-pc-cygwin-clang" \
            -DCMAKE_CXX_COMPILER="${LLVM_CYGWIN_PATH}/toolchain/bin/x86_64-pc-cygwin-clang++" \
            -DCMAKE_SYSROOT="${LLVM_CYGWIN_PATH}/sysroot" \
            -DCMAKE_FIND_ROOT_PATH="${LLVM_CYGWIN_PATH}/sysroot" \
            -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM="NEVER" \
            -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY="ONLY" \
            -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE="ONLY" \
            -DCMAKE_C_FLAGS="" \
            -DCMAKE_CXX_FLAGS="" \
            -DENABLE_FULL_LTO="OFF" \
            -DMAX_BLOCK_SIZE="4096"
    fi

    ${MAKE_CMD}
}

build()
{
    local TARGET=$1
    local ABI=$2
    local PLATFORM=$3

    rm -rf $OUT > /dev/null 2>&1

    local NINJA=`which ninja`
    local METHOD=""
    if [[ -f $NINJA ]]; then
      METHOD="Ninja"
    else
      METHOD="make"
    fi

    cmake_build "${TARGET}" "${METHOD}" "${ABI}" "${PLATFORM}"

    local BUILD="$OUT/src/erofs-utils"
    local DUMP_BIN="$BUILD/dump.erofs${EXT}"
    local FSCK_BIN="$BUILD/fsck.erofs${EXT}"
    local MKFS_BIN="$BUILD/mkfs.erofs${EXT}"
    local EXTRACT_BIN="$OUT/src/extract/extract.erofs${EXT}"
    local TARGE_DIR_NAME="erofs-tools-${EROFS_VERSION}-$(TZ=UTC-8 date +%y%m%d)-${TARGET}_${ABI}"
    local TARGET_DIR_PATH="./target/${TARGET}_${ABI}/${TARGE_DIR_NAME}"

    if [ -f "$DUMP_BIN" -a -f "$FSCK_BIN" -a -f "$MKFS_BIN" -a -f "$EXTRACT_BIN" ]; then
        echo "复制文件中..."
        [[ ! -d "$TARGET_DIR_PATH" ]] && mkdir -p ${TARGET_DIR_PATH}
        cp -af $BUILD/*.erofs${EXT} ${TARGET_DIR_PATH}
        cp -af ${EXTRACT_BIN} ${TARGET_DIR_PATH}
        cp -af "${LLVM_CYGWIN_PATH}/sysroot/usr/bin/cygwin1.dll" ${TARGET_DIR_PATH}
        touch -c -d "2009-01-01 00:00:00" ${TARGET_DIR_PATH}/*
        echo "编译成功: ${TARGE_DIR_NAME}"
    else
        echo "error"
        exit 1
    fi
}

build "Cygwin" "x86_64"

exit 0
