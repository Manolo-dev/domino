#pragma once
#include <android/native_window.h>

void render_init(ANativeWindow* window);
void render_shutdown(void);
void render_frame(void);
