#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "mat.h"

#define STYLE_INIT(...)                                                  \
    _Pragma("clang diagnostic push")                                     \
    _Pragma("clang diagnostic ignored \"-Winitializer-overrides\"")      \
    _Pragma("clang diagnostic ignored \"-Wmissing-braces\"")             \
    (Style){                                                             \
        .color = 0,                                                      \
        .left = 0,                                                       \
        .top = 0,                                                        \
        .antialiasing = 1.0f,                                            \
        .alpha = 1.0f,                                                   \
        .transform = MAT_IDENTITY,                                       \
        .anchor = LEFT_TOP,                                              \
        __VA_ARGS__                                                      \
    }                                                                    \
    _Pragma("clang diagnostic pop")

typedef void (*Onclick)(void);

typedef enum {
    LEFT_TOP      = 0,
    LEFT_CENTER   = 1,
    LEFT_BOTTOM   = 2,
    CENTER_TOP    = 3,
    CENTER        = 4,
    CENTER_BOTTOM = 5,
    RIGHT_TOP     = 6,
    RIGHT_CENTER  = 7,
    RIGHT_BOTTOM  = 8,
} Anchor;

typedef float Unit;

typedef struct {
    void* _data;
    float (*inside)(void* data, float x, float y);
    void  (*sizes)(void* data, Unit* width, Unit* height);
    void  (*free)(void* data);
} Shape;

typedef struct {
    int color;
    Unit left;
    Unit top;
    float alpha;
    float antialiasing;
    Mat transform;
    Anchor anchor;
} Style;

typedef struct div {
    //private:
    int _left, _top, _width, _height;
    Mat _inv;
    Mat _world;
    Mat _world_inv;
    bool _dirty;
    struct div* _parent;
    struct div* _first_child;
    struct div* _next_sibling;
    //public:
    Onclick onclick;
    Shape shape;
    Style style;
} Div;

typedef struct {
    uint32_t* bits;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
} Buffer;

Shape make_rect_shape(Unit width, Unit height);
Shape make_circle_shape(Unit radius);

Div make_div(Shape shape, Style style);
void  div_onclick(Div* div, Onclick onclick);
float div_signed_distance(Div* div, float x, float y);
bool  div_hit(Div* div, float x, float y);
void  div_add_child(Div* parent, Div* child);
void  div_update(Div* div, int w, int h);
void  div_tree_update(Div* root, int wp, int hp);
void  div_draw(Div* div, Buffer* buffer);
void  div_free(Div* div);