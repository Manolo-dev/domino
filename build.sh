#!/data/data/com.termux/files/usr/bin/bash
set -e

NDK=~/ndk/android-ndk-r29
CLANG=$NDK/toolchains/llvm/prebuilt/linux-aarch64/bin/clang
ANDROID_JAR=toolz/android.jar
API=24
GLUE=$NDK/sources/android/native_app_glue

mkdir -p lib/arm64-v8a build

echo "== Compilation de libmain.so =="
$CLANG --target=aarch64-linux-android$API \
    -Wall -I$GLUE -Isrc \
    -shared -fPIC -u ANativeActivity_onCreate \
    -o lib/arm64-v8a/libmain.so \
    $GLUE/android_native_app_glue.c \
    src/*/*.c \
    -landroid -llog

echo "== Packaging du manifeste =="
aapt package -f -M AndroidManifest.xml -S res -I $ANDROID_JAR -F build/app.unsigned.apk

echo "== Ajout de la lib native =="
aapt add build/app.unsigned.apk lib/arm64-v8a/libmain.so

if [ ! -f debug.keystore ]; then
    echo "== Génération de la clé de debug =="
    keytool -genkeypair -v -keystore debug.keystore -alias debugkey \
        -storepass android -keypass android \
        -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Debug"
fi

echo "== Zipalign =="
zipalign -f 4 build/app.unsigned.apk build/app.aligned.apk

echo "== Signature =="
apksigner sign --ks debug.keystore --ks-pass pass:android \
    --out build/app.apk build/app.aligned.apk

echo "== Copie storage =="
cp build/app.apk ~/storage/downloads/domino.apk

echo "== Terminé : build/app.apk =="
ls -lh build/app.apk ~/storage/downloads/domino.apk
