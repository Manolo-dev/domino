#pragma once
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "../div/div.h"

void render_init(ANativeWindow* window);
void render_shutdown(void);
void render_frame(Div *root);