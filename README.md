# Domino natif Android

Jeu de domino en C pur, via `NativeActivity` + `android_native_app_glu`e (sans Java).

## Setup

1. Installer NDK dans `~/ndk/android-ndk-r29` :
    ```bash
    wget https://dl.google.com/android/repository/android-ndk-r29-<OS>.zip
    unzip android-ndk-r29-linux.zip -d ~/ndk/
    rm android-ndk-r29-linux.zip

    echo 'export ANDROID_NDK=~/ndk/android-ndk-r29' >> ~/.bashrc
    source ~/.bashrc
    ```
2. Installer build-tools (aapt, zipalign et apksigner) :
    ```bash
    mkdir -p ~/android-sdk/build-tools
    cd ~/android-sdk/build-tools
    wget https://dl.google.com/android/repository/build-tools_r36.1_linux.zip
    unzip build-tools_r36.1_linux.zip
    mv android-* 36.1.0
    rm build-tools_r36.1_linux.zip

    echo 'export PATH=$PATH:$HOME/android-sdk/build-tools/36.1.0' >> ~/.bashrc
    source ~/.bashrc
    ```
2. Récupérer `android.jar` (non versionné, ~34 Mo) :
    ```bash
    wget https://raw.githubusercontent.com/Sable/android-platforms/master/android-24/android.jar -O toolz/android.jar
    ```
3. Build :
    ```bash
    ./build.sh
    ```
4. build/app.apk est prêt à installer.

## Architecture

- `src/platform/` :  pont avec Android (android_main, cycle de vie)
- `src/render/` : dessin à l'écran (ANativeWindow)
- `src/input/` : gestion tactile
- `src/game/` : logique du domino, indépendante d'Android
