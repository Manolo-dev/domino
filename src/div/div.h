#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define STYLE_INIT(...)                                                  \
    _Pragma("clang diagnostic push")                                     \
    _Pragma("clang diagnostic ignored \"-Winitializer-overrides\"")      \
    _Pragma("clang diagnostic ignored \"-Wmissing-braces\"")             \
    (Style){                                                             \
        .color = 0,                                                      \
        .left = {0},        /* accolades pour éviter -Wmissing-braces */ \
        .top = {0},         /* idem */                                   \
        .antialiasing = 1.0f,                                            \
        .alpha = 1.0f,                                                   \
        .transform = NULL,                                               \
        __VA_ARGS__                                                      \
    }                                                                    \
    _Pragma("clang diagnostic pop")

typedef void (*Onclick)(void);
typedef void (*Transform)(float *, float *);

typedef struct {
    enum { PX, VW, VH } tag;
    union { int i; float f; } value;
} Unit;

typedef struct {
    //private:
    void *_data;
    //public:
    float (*inside)(void *data, float x, float y);
    void  (*sizes)(void *data, Unit *width, Unit *height);
    void  (*free)(void *data);
} Shape;

typedef struct {
    int color;
    Unit left;
    Unit top;
    float alpha;
    float antialiasing;
    Transform transform;
} Style;

typedef struct div {
    //private:
    int _left, _top, _width, _height;
    struct div *_parent;
    struct div *_first_child;
    struct div *_next_sibling;
    //public:
    Onclick onclick;
    Shape shape;
    Style style;
} Div;

typedef struct {
    uint32_t *bits;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
} Buffer;

static inline Unit make_px(  int v) { return (Unit){PX, {.i = v}}; }
static inline Unit make_vw(float v) { return (Unit){VW, {.f = v}}; }
static inline Unit make_vh(float v) { return (Unit){VH, {.f = v}}; }

Shape make_rect_shape(Unit width, Unit height);
Shape make_circle_shape(Unit radius);

Div make_div(Shape shape, Style style);
void div_onclick(Div *div, Onclick onclick);
void div_add_child(Div *parent, Div *child);
void div_update(Div *div, int w, int h);
void div_tree_update(Div *root, int wp, int hp);
void div_draw(Div *div, Buffer *buffer);
void div_free(Div *div);

int to_pixels(Unit u);