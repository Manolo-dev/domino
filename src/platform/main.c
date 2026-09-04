#include <android_native_app_glue.h>
#include <android/log.h>
#include <time.h>
#include "../render/render.h"
#include "../input/input.h"
#include "../div/div.h"
#include "../div/shapes.h"

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "Domino", __VA_ARGS__)

#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)

static void handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL)
                render_init(app->window);
            break;
        case APP_CMD_TERM_WINDOW:
            render_shutdown();
            break;
    }
}

static int64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void transform(int *x, int *y) {
    *x *= 0.01 * (*y);
    *y *= 0.01 * (*x);
}

void android_main(struct android_app* app) {
    app->onAppCmd = handle_cmd;

    int events;
    struct android_poll_source* source;

    int64_t last_frame = now_ms();

    Div root = make_div(
        make_rect_shape(make_vw(100), make_vh(100)),
        STYLE_INIT(.color=0xFF003309, .left=make_px(0), .top=make_px(0))
    );

    Div domino = make_div(
        make_squircle_shape(make_vw(22), make_vw(44), 6),
        STYLE_INIT(.color=0xFFD0E8ED, .left=make_vw(20), .top=make_vh(20))
    );
    div_add_child(&root, &domino);

    Div bar = make_div(
        make_squircle_shape(make_vw(20), make_vw(.5), 8),
        STYLE_INIT(.color=0xFF000A00, .left=make_vw(1), .top=make_vw(21.75))
    );
    Div cc = make_div(
        make_circle_shape(make_vw(.5)),
        STYLE_INIT(.color=0xFF00D7FF, .left=make_vw(10.5), .top=make_vw(21.5))
    );
    Div c1 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(4), .top=make_vw(4))
    );
    Div c2 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(4), .top=make_vw(9))
    );
    Div c3 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(4), .top=make_vw(14))
    );
    Div c4 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(14), .top=make_vw(4))
    );
    Div c5 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(14), .top=make_vw(9))
    );
    Div c6 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(14), .top=make_vw(14))
    );
    Div c7 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(4), .top=make_vw(26))
    );
    Div c8 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(4), .top=make_vw(31))
    );
    Div c9 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(4), .top=make_vw(36))
    );
    Div c10 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(14), .top=make_vw(26))
    );
    Div c11 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(14), .top=make_vw(31))
    );
    Div c12 = make_div(
        make_circle_shape(make_vw(2)),
        STYLE_INIT(.color=0xFF000000, .left=make_vw(14), .top=make_vw(36))
    );

    div_add_child(&domino, &bar);
    div_add_child(&domino, &cc);
    div_add_child(&domino, &c1);
    div_add_child(&domino, &c2);
    div_add_child(&domino, &c3);
    div_add_child(&domino, &c4);
    div_add_child(&domino, &c5);
    div_add_child(&domino, &c6);
    div_add_child(&domino, &c7);
    div_add_child(&domino, &c8);
    div_add_child(&domino, &c9);
    div_add_child(&domino, &c10);
    div_add_child(&domino, &c11);
    div_add_child(&domino, &c12);
    div_transform(&domino, transform);

    while (1) {
        int64_t elapsed = now_ms() - last_frame;
        int timeout_ms = (int)(FRAME_TIME_MS - elapsed);
        if (timeout_ms < 0) timeout_ms = 0;

        while (ALooper_pollOnce(timeout_ms, NULL, &events, (void**)&source) >= 0) {
            if (source != NULL) source->process(app, source);
            if (app->destroyRequested != 0) return;
            timeout_ms = 0;
        }

        int64_t frame_start = now_ms();
        if (frame_start - last_frame >= FRAME_TIME_MS) {
            InputState in = input_poll(app);
            (void)in; // TODO: transmettre au rendu quand la logique de clic sera branchée
            render_frame(&root);
            last_frame = frame_start;
        }
    }
}