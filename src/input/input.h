#pragma once
#include <android_native_app_glue.h>

typedef struct {
    float touch_x, touch_y;
    int touch_down;
} InputState;

InputState input_poll(struct android_app* app);
