#!/usr/bin/env bash
set -e

# --------------------------------------------------------------------------- #
#                                Chemins système                              #
# --------------------------------------------------------------------------- #
NDK=~/.android-ndk/android-ndk-r29
HOST_TAG=$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m)
TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/$HOST_TAG"
SYSROOT="$TOOLCHAIN/sysroot"
SYSROOT_LIB="$SYSROOT/usr/lib"
CLANG="$TOOLCHAIN/bin/clang"
ANDROID_JAR="toolz/android.jar"
API=24
GLUE="$NDK/sources/android/native_app_glue"

# --------------------------------------------------------------------------- #
#                                    Flags                                    #
# --------------------------------------------------------------------------- #
CFLAGS_USER="-O3 -fPIC -Wall -Wextra -Werror"
CFLAGS_GLUE="-O3 -fPIC"
INCLUDES="-I$GLUE -Isrc -pthread"
LDFLAGS="-shared -fPIC -u ANativeActivity_onCreate -landroid -llog -lm"

# --------------------------------------------------------------------------- #
#                                 ABI cibles                                  #
# --------------------------------------------------------------------------- #
declare -A TARGETS=(
    [arm64-v8a]="aarch64-linux-android$API"
    [x86_64]="x86_64-linux-android$API"
)

mkdir -p build/obj

# --------------------------------------------------------------------------- #
#                                Clé de debug                                 #
# --------------------------------------------------------------------------- #
if [ ! -f debug.keystore ]; then
    printf "== Génération de la clé de debug ==\n"
    keytool -genkeypair -v -keystore debug.keystore -alias debugkey \
        -storepass android -keypass android \
        -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Debug"
fi

# --------------------------------------------------------------------------- #
#                           Filtrage des ABI cibles                           #
# --------------------------------------------------------------------------- #
for ARCH in "${!TARGETS[@]}"; do
    TRIPLE="${TARGETS[$ARCH]%$API}"
    if [ ! -d "$SYSROOT_LIB/$TRIPLE/$API" ]; then
        unset TARGETS[$ARCH]
    fi
done

# --------------------------------------------------------------------------- #
#                                 Compilation                                 #
# --------------------------------------------------------------------------- #
for ARCH in "${!TARGETS[@]}"; do
    TARGET="${TARGETS[$ARCH]}"
    mkdir -p "lib/$ARCH"

    printf "== [\033[32;1m$ARCH\033[0m] Compilation du code glue (tiers) ==\n"
    $CLANG --target="$TARGET" \
        $CFLAGS_GLUE $INCLUDES \
        -c "$GLUE/android_native_app_glue.c" \
        -o "build/obj/glue-$ARCH.o"

    printf "== [\033[32;1m$ARCH\033[0m] Compilation de vos sources ==\n"
    USER_OBJS=()
    for src in src/*/*.c; do
        obj="build/obj/$(basename "$src" .c)-$ARCH.o"
        $CLANG --target="$TARGET" \
            $CFLAGS_USER $INCLUDES \
            -c "$src" \
            -o "$obj"
        USER_OBJS+=("$obj")
    done

    printf "== [\033[32;1m$ARCH\033[0m] Édition de liens ==\n"
    $CLANG --target="$TARGET" \
        $LDFLAGS \
        "build/obj/glue-$ARCH.o" "${USER_OBJS[@]}" \
        -o "lib/$ARCH/libmain.so"

    printf "== [\033[32;1m$ARCH\033[0m] Packaging du manifeste ==\n"
    aapt package -f -M AndroidManifest.xml -S res -I "$ANDROID_JAR" \
        -F "build/app-$ARCH.unsigned.apk"

    printf "== [\033[32;1m$ARCH\033[0m] Ajout de la lib native ==\n"
    aapt add "build/app-$ARCH.unsigned.apk" "lib/$ARCH/libmain.so"

    printf "== [\033[32;1m$ARCH\033[0m] Zipalign ==\n"
    zipalign -f 4 "build/app-$ARCH.unsigned.apk" \
        "build/app-$ARCH.aligned.apk"

    printf "== [\033[32;1m$ARCH\033[0m] Signature ==\n"
    apksigner sign --ks debug.keystore --ks-pass pass:android \
        --out "build/app-$ARCH.apk" "build/app-$ARCH.aligned.apk"

    if [ -d ~/storage/downloads ]; then
        printf "== [\033[32;1m$ARCH\033[0m] Copie vers storage ==\n"
        cp "build/app-$ARCH.apk" ~/storage/downloads/domino-$ARCH.apk
    fi
done

printf "== Terminé ==\n"
ls -lh build/app-*.apk