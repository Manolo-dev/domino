# Domino natif Android

Jeu de domino en C pur, via `NativeActivity` + `android_native_app_glu`e (sans Java).

## Setup

1. NDK installé dans `~/ndk/android-ndk-r29` (adapter le chemin dans `build.sh` sinon).
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
