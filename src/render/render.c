#include "render.h"
#include <android/native_window_jni.h>

static ANativeWindow* g_window = NULL;

void render_init(ANativeWindow* window) {
    g_window = window;
    ANativeWindow_setBuffersGeometry(g_window, 0, 0, WINDOW_FORMAT_RGBA_8888);
}

void render_shutdown(void) {
    g_window = NULL;
}

void render_frame(void) {
    if (!g_window) return;
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(g_window, &buffer, NULL) == 0) {
        uint32_t* pixels = (uint32_t*)buffer.bits;
        for (int y = 0; y < buffer.height; y++)
            for (int x = 0; x < buffer.width; x++)
                pixels[y * buffer.stride + x] = 0xFF3366CC;
        ANativeWindow_unlockAndPost(g_window);
    }
}
