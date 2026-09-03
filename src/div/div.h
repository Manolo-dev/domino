#pragma once
#include <android/native_window_jni.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef void (*Onclick)(void);
typedef void (*Transform)(int *, int *);

typedef struct transform_node {
    Transform fn;
    struct transform_node *next;
} TransformNode;

typedef struct {
    enum { PX, VW, VH } tag;
    union { int i; float f; } value;
} Unit;

typedef struct {
    //private:
    void *_data;
    //public:
    float (*is_inside)(void *data, float x, float y);
    void  (*get_sizes)(void *data, Unit *width, Unit *height);
    void  (*free_data)(void *data);
} Shape;

typedef struct {
    int color;
    Unit left;
    Unit top;
    TransformNode *transform;
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
void div_draw(Div *div, ANativeWindow_Buffer *buffer);
void div_transform(Div *div, Transform transform);

int to_pixels(Unit u);