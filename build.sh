#!/usr/bin/env bash
set -e

NDK=~/.android-ndk/android-ndk-r29
HOST_TAG=$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m)
CLANG=$NDK/toolchains/llvm/prebuilt/$HOST_TAG/bin/clang
ANDROID_JAR=toolz/android.jar
API=24
GLUE=$NDK/sources/android/native_app_glue

# ABI Android -> triplet cible clang
declare -A TARGETS=(
    [arm64-v8a]="aarch64-linux-android$API"
    [x86_64]="x86_64-linux-android$API"
)

mkdir -p build

# Clé de debug, générée une seule fois pour toutes les archis
if [ ! -f debug.keystore ]; then
    echo "== Génération de la clé de debug =="
    keytool -genkeypair -v -keystore debug.keystore -alias debugkey \
        -storepass android -keypass android \
        -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Debug"
fi

for ARCH in "${!TARGETS[@]}"; do
    TRIPLE=${TARGETS[$ARCH]%$API}  # ex: aarch64-linux-android
    if [ ! -d "$NDK/toolchains/llvm/prebuilt/$HOST_TAG/sysroot/usr/lib/$TRIPLE/$API" ]; then
        unset TARGETS[$ARCH]
    fi
done

for ARCH in "${!TARGETS[@]}"; do
    TARGET=${TARGETS[$ARCH]}
    mkdir -p "lib/$ARCH"

    echo "== [$ARCH] Compilation de libmain.so =="
    $CLANG --target=$TARGET \
        -Wall -I$GLUE -Isrc \
        -shared -fPIC -u ANativeActivity_onCreate \
        -o "lib/$ARCH/libmain.so" \
        $GLUE/android_native_app_glue.c \
        src/*/*.c \
        -landroid -llog -lm

    echo "== [$ARCH] Packaging du manifeste =="
    aapt package -f -M AndroidManifest.xml -S res -I $ANDROID_JAR \
        -F "build/app-$ARCH.unsigned.apk"

    echo "== [$ARCH] Ajout de la lib native =="
    aapt add "build/app-$ARCH.unsigned.apk" "lib/$ARCH/libmain.so"

    echo "== [$ARCH] Zipalign =="
    zipalign -f 4 "build/app-$ARCH.unsigned.apk" "build/app-$ARCH.aligned.apk"

    echo "== [$ARCH] Signature =="
    apksigner sign --ks debug.keystore --ks-pass pass:android \
        --out "build/app-$ARCH.apk" "build/app-$ARCH.aligned.apk"

    if [ -d ~/storage/downloads ]; then
        echo "== [$ARCH] Copie storage =="
        cp "build/app-$ARCH.apk" ~/storage/downloads/domino-$ARCH.apk
    fi
done

echo "== Terminé =="
ls -lh build/app-*.apk
