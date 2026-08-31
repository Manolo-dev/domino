#include <android_native_app_glue.h>
#include <android/log.h>
#include "../render/render.h"
#include "../input/input.h"
#include "../game/domino.h"

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "Domino", __VA_ARGS__)

static void handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                render_init(app->window);
                domino_init();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            render_shutdown();
            break;
    }
}

void android_main(struct android_app* app) {
    app->onAppCmd = handle_cmd;

    int events;
    struct android_poll_source* source;

    while (1) {
        while (ALooper_pollOnce(0, NULL, &events, (void**)&source) >= 0) {
            if (source != NULL) source->process(app, source);
            if (app->destroyRequested != 0) return;
        }

        InputState in = input_poll(app);
        domino_update(&in);
        render_frame();
    }
}
