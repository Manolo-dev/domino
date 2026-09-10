#pragma once
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdint.h>
#include <pthread.h>
#include "../div/div.h"

void render_init(ANativeWindow* window);
void render_shutdown(void);
void render_frame(int divc, Div* root[], uint32_t bg);