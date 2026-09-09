#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/choreographer.h>
#include <android/log.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "../render/render.h"
#include "../input/input.h"
#include "../div/div.h"
#include "../div/shapes.h"
#include "../div/mat.h"

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "Domino", __VA_ARGS__)

static bool g_dirty = true;
static int g_screen_w = 0;
static int g_screen_h = 0;

#define VW(pct) ((int)(g_screen_w * (pct) / 100.0f))
#define VH(pct) ((int)(g_screen_h * (pct) / 100.0f))

static struct android_app* g_app = NULL;
static int64_t g_last_frame_ms = 0;
static bool g_running = true;

static void handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL)
                render_init(app->window);
            g_dirty = true;
            break;
        case APP_CMD_TERM_WINDOW:
            render_shutdown();
            break;
        case APP_CMD_WINDOW_RESIZED:
        case APP_CMD_WINDOW_REDRAW_NEEDED:
        case APP_CMD_CONTENT_RECT_CHANGED:
            g_dirty = true;
            break;
    }
}

static inline Mat transform(float g_angle) {
    return mat_mul(
        mat_mul(
            rotate(g_angle),
            rotate_x(g_angle, g_screen_w * 2.0)
        ),
        translate(100, 0)
    );
}

static Div domino;
static float g_angle = 0.0f;

static Div bar, cc;
static Div c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12;

static void build_scene(void) {
    // root = make_div(
    //     make_rect(g_screen_w, g_screen_h),
    //     STYLE_INIT(.color=0xFF003309, .left=0, .top=0)
    // );

    domino = make_div(
        make_squircle(VW(22), VW(44), 6),
        STYLE_INIT(.color=0xFFD0E8ED, .left=VW(20), .top=VH(20), .transform=transform(g_angle), .anchor=CENTER)
    );
    // div_add_child(&root, &domino);

    bar = make_div(
        make_squircle(VW(20), VW(1) / 2, 8),
        STYLE_INIT(.color=0xFF000A00, .left=VW(1), .top=VW(21) + VW(1) / 2 + VW(1) / 4)
    );
    cc = make_div(
        make_circle(VW(1) / 2),
        STYLE_INIT(.color=0xFF00D7FF, .left=VW(10) + VW(1) / 2, .top=VW(21) + VW(1) / 2)
    );
    c1  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(4),  .top=VW(4)));
    c2  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(4),  .top=VW(9)));
    c3  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(4),  .top=VW(14)));
    c4  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(14), .top=VW(4)));
    c5  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(14), .top=VW(9)));
    c6  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(14), .top=VW(14)));
    c7  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(4),  .top=VW(26)));
    c8  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(4),  .top=VW(31)));
    c9  = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(4),  .top=VW(36)));
    c10 = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(14), .top=VW(26)));
    c11 = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(14), .top=VW(31)));
    c12 = make_div(make_circle(VW(2)), STYLE_INIT(.color=0xFF000000, .left=VW(14), .top=VW(36)));

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
}

static void frame_callback(long frame_time_ns, void* data) {
    (void)data;
    if (!g_running) return;

    int64_t now = frame_time_ns / 1000000;
    float delta = g_last_frame_ms ? (now - g_last_frame_ms) / 1000.0f : 0.0f;
    g_last_frame_ms = now;

    g_angle += delta * 0.8f;
    domino.style.transform = transform(g_angle);
    domino._dirty = true;
    Div *divs[] = {&domino};

    if (g_app->window != NULL) render_frame(1, divs, 0xFF003309);

    AChoreographer_postFrameCallback(AChoreographer_getInstance(), frame_callback, NULL);
}

void android_main(struct android_app* app) {
    app->onAppCmd = handle_cmd;
    g_app = app;

    int events;
    struct android_poll_source* source;

    while (app->window == NULL) {
        if (ALooper_pollOnce(-1, NULL, &events, (void**)&source) >= 0) {
            if (source != NULL) source->process(app, source);
            if (app->destroyRequested != 0) return;
        }
    }

    g_screen_w = ANativeWindow_getWidth(app->window);
    g_screen_h = ANativeWindow_getHeight(app->window);
    LOG("screen: %dx%d", g_screen_w, g_screen_h);

    build_scene();

    AChoreographer_postFrameCallback(AChoreographer_getInstance(), frame_callback, NULL);

    while (1) {
        if (ALooper_pollOnce(-1, NULL, &events, (void**)&source) >= 0) {
            if (source != NULL) source->process(app, source);
            if (app->destroyRequested != 0) {
                g_running = false;
                return;
            }
        }
    }
}