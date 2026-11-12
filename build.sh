OUT="./out"
BUILD_DIR="./"
EROFS_VERSION="$(cd src/lib/erofs-utils;tag=$(git describe --exact-match 2>/dev/null);head=$(git rev-parse --verify HEAD 2>/dev/null); printf '%s-g%.8s' $tag $head)"

cmake_build()
{
    local TARGET=$1
    local METHOD=$2
    local ABI=$3

    local MAKE_CMD=""
    local BUILD_METHOD=""
    if [[ $METHOD == "Ninja" ]]; then
        BUILD_METHOD="-G Ninja"
        MAKE_CMD="time -p ninja -C $OUT"
    elif [[ $METHOD == "make" ]]; then
        MAKE_CMD="time -p make -C $OUT -j$(nproc)"
    fi

    if [[ $TARGET == "Android" ]]; then
        local ANDROID_PLATFORM=$4
        cmake -S ${BUILD_DIR} -B $OUT ${BUILD_METHOD} \
            -DNDK_CCACHE="ccache" \
            -DCMAKE_BUILD_TYPE="Release" \
            -DANDROID_PLATFORM="$ANDROID_PLATFORM" \
            -DANDROID_ABI="$ABI" \
            -DANDROID_STL="none" \
            -DUSE_LIBCXX="ON" \
            -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_LATEST_HOME/build/cmake/android.toolchain.cmake" \
            -DANDROID_USE_LEGACY_TOOLCHAIN_FILE="OFF" \
            -DCMAKE_C_FLAGS="" \
            -DCMAKE_CXX_FLAGS="" \
            -DENABLE_FULL_LTO="ON"
    elif [[ $TARGET == "Linux" ]]; then
        local LINUX_PLATFORM=$4
        local WSL="OFF"
        [ "${LINUX_PLATFORM}" == "WSL" ] && WSL="ON"
        if [[ ${ABI} == "x86_64" ]]; then
            cmake -S ${BUILD_DIR} -B ${OUT} ${BUILD_METHOD} \
                -DCMAKE_SYSTEM_NAME="Linux" \
                -DCMAKE_SYSTEM_PROCESSOR="x86_64" \
                -DCMAKE_BUILD_TYPE="Release" \
                -DRUN_ON_WSL="${WSL}" \
                -DUSE_LIBCXX="ON" \
                -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
                -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
                -DCMAKE_C_COMPILER="clang" \
                -DCMAKE_CXX_COMPILER="clang++" \
                -DCMAKE_C_FLAGS="" \
                -DCMAKE_CXX_FLAGS="" \
                -DENABLE_FULL_LTO="ON"
        elif [[ ${ABI} == "aarch64" ]]; then
            cmake -S ${BUILD_DIR} -B ${OUT} ${BUILD_METHOD} \
                -DCMAKE_SYSTEM_NAME="Linux" \
                -DCMAKE_SYSTEM_PROCESSOR="aarch64" \
                -DCMAKE_BUILD_TYPE="Release" \
                -DRUN_ON_WSL="${WSL}" \
                -DUSE_LIBCXX="ON" \
                -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
                -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
                -DCMAKE_C_COMPILER="clang" \
                -DCMAKE_CXX_COMPILER="clang++" \
                -DCMAKE_C_COMPILER_TARGET="aarch64-linux-gnu" \
                -DCMAKE_CXX_COMPILER_TARGET="aarch64-linux-gnu" \
                -DCMAKE_ASM_COMPILER_TARGET="aarch64-linux-gnu" \
                -DCMAKE_FIND_ROOT_PATH="/usr/lib/gcc/aarch64-linux-gnu/14/include" \
                -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM="NEVER" \
                -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY="ONLY" \
                -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE="ONLY" \
                -DCMAKE_C_FLAGS="" \
                -DCMAKE_CXX_FLAGS="" \
                -DENABLE_FULL_LTO="ON"
        elif [[ ${ABI} == "loongarch64" ]]; then
            cmake -S ${BUILD_DIR} -B ${OUT} ${BUILD_METHOD} \
                -DCMAKE_SYSTEM_NAME="Linux" \
                -DCMAKE_SYSTEM_PROCESSOR="loongarch64" \
                -DCMAKE_BUILD_TYPE="Release" \
                -DRUN_ON_WSL="${WSL}" \
                -DUSE_LIBCXX="ON" \
                -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
                -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
                -DCMAKE_C_COMPILER="clang" \
                -DCMAKE_CXX_COMPILER="clang++" \
                -DCMAKE_C_COMPILER_TARGET="loongarch64-linux-gnu" \
                -DCMAKE_CXX_COMPILER_TARGET="loongarch64-linux-gnu" \
                -DCMAKE_ASM_COMPILER_TARGET="loongarch64-linux-gnu" \
                -DCMAKE_C_FLAGS="" \
                -DCMAKE_CXX_FLAGS="" \
                -DENABLE_FULL_LTO="OFF"
        fi
    fi

    ${MAKE_CMD}
}

build()
{
    local TARGET=$1
    local ABI=$2
    local PLATFORM=$3

    rm -r $OUT > /dev/null 2>&1

    local NINJA=`which ninja`
    local METHOD=""
    if [[ -f $NINJA ]]; then
        METHOD="Ninja"
    else
        METHOD="make"
    fi

    cmake_build "${TARGET}" "${METHOD}" "${ABI}" "${PLATFORM}"

    [ "${PLATFORM}" == "WSL" ] && TARGET="WSL"

    local BUILD="$OUT/src/erofs-utils"
    local DUMP_BIN="$BUILD/dump.erofs"
    local FSCK_BIN="$BUILD/fsck.erofs"
    local MKFS_BIN="$BUILD/mkfs.erofs"
    local EXTRACT_BIN="$OUT/src/extract/extract.erofs"
    local TARGE_DIR_NAME="erofs-tools-${EROFS_VERSION}-$(TZ=UTC-8 date +%y%m%d)-${TARGET}_${ABI}"
    local TARGET_DIR_PATH="./target/${TARGET}_${ABI}/${TARGE_DIR_NAME}"

    if [ -f "$DUMP_BIN" -a -f "$FSCK_BIN" -a -f "$MKFS_BIN" -a -f "$EXTRACT_BIN" ]; then
        echo "复制文件中..."
        [[ ! -d "$TARGET_DIR_PATH" ]] && mkdir -p ${TARGET_DIR_PATH}
        cp -af $BUILD/*.erofs ${TARGET_DIR_PATH}
        cp -af ${EXTRACT_BIN} ${TARGET_DIR_PATH}
        touch -c -d "2009-01-01 00:00:00" ${TARGET_DIR_PATH}/*
        echo "编译成功: ${TARGE_DIR_NAME}"
    else
        echo "error"
        exit 1
    fi
}

build_android()
{
    build "Android" "arm64-v8a" "android-31"
    build "Android" "x86_64" "android-31"
}

build_linux()
{
    build "Linux" "x86_64"
    build "Linux" "x86_64" "WSL"
    build "Linux" "aarch64"
    build "Linux" "loongarch64"
}

# build
if [[ "$1" == "Android" ]]; then
    build_android
elif [[ "$1" == "Linux" ]]; then
    build_linux
else
    exit 1
fi

exit 0
