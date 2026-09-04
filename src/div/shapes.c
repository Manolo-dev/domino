#include "shapes.h"

// Rectangle
typedef struct { Unit w, h; } RectData;

static float rect_signed_distance(void *data, float x, float y) {
    RectData *rd = data;
    float w = to_pixels(rd->w);
    float h = to_pixels(rd->h);
    float cx = w/2.0f, cy = h/2.0f;
    float dx = fabsf(x - cx) - w/2.0f;
    float dy = fabsf(y - cy) - h/2.0f;
    if (dx < 0 && dy < 0) return -fminf(-dx, -dy);
    else                  return sqrtf(dx*dx + dy*dy);
}

static void rect_sizes(void *data, Unit *w, Unit *h) {
    RectData *rd = data;
    *w = rd->w;
    *h = rd->h;
}

static void rect_free(void *data) { free(data); }

Shape make_rect(Unit width, Unit height) {
    RectData *rd = malloc(sizeof(RectData));
    rd->w = width;
    rd->h = height;
    Shape s;
    s._data = rd;
    s.inside = rect_signed_distance;
    s.sizes = rect_sizes;
    s.free = rect_free;
    return s;
}

// Cercle
typedef struct { Unit radius; } CircleData;

static float circle_signed_distance(void *data, float x, float y) {
    CircleData *cd = data;
    float r = to_pixels(cd->radius);
    float dx = x - r, dy = y - r;
    return sqrtf(dx*dx + dy*dy) - r;
}

static void circle_sizes(void *data, Unit *w, Unit *h) {
    CircleData *cd = data;
    int radius_px = to_pixels(cd->radius);
    *w = make_px(radius_px * 2);
    *h = make_px(radius_px * 2);
}

static void circle_free(void *data) { free(data); }

Shape make_circle(Unit radius) {
    CircleData *cd = malloc(sizeof(CircleData));
    cd->radius = radius;
    Shape s;
    s._data = cd;
    s.inside = circle_signed_distance;
    s.sizes = circle_sizes;
    s.free = circle_free;
    return s;
}

// Courbe de Lamé
typedef struct { Unit w, h; float s; } SquircleData;

static float squircle_signed_distance(void *data, float x, float y) {
    SquircleData *sd = data;
    float w = to_pixels(sd->w), h = to_pixels(sd->h);
    if (!w || !h) return FLT_MAX;
    float cx = w / 2.0f, cy = h / 2.0f;
    float dx = x - cx, dy = y - cy;
    float a = powf(fabsf(dx / cx), sd->s);
    float b = powf(fabsf(dy / cy), sd->s);
    float d = powf(a + b, 1.0f / sd->s) - 1.0f;
    float scale = (cx + cy) * 0.5f;
    return d * scale;
}

static void squircle_sizes(void *data, Unit *w, Unit *h) {
    SquircleData *rd = data;
    *w = rd->w;
    *h = rd->h;
}

static void squircle_free(void *data) { free(data); }

Shape make_squircle(Unit width, Unit height, float squareness) {
    SquircleData *sd = malloc(sizeof(SquircleData));
    sd->w = width;
    sd->h = height;
    sd->s = squareness;
    Shape s;
    s._data = sd;
    s.inside = squircle_signed_distance;
    s.sizes = squircle_sizes;
    s.free = squircle_free;
    return s;
}


/*
// Trait
typedef struct { Unit w, h, t; } LineData;

static float line_signed_distance(void *data, float x, float y) {
    LineData *rd = data;
    float w = to_pixels(rd->w);
    float h = to_pixels(rd->h);
}

static void line_sizes(void *data, Unit *w, Unit *h) {
    LineData *rd = data;
    *w = rd->w;
    *h = rd->h;
}

static void line_free(void *data) { free(data); }

Shape make_rect(Unit width, Unit height) {
    LineData *rd = malloc(sizeof(LineData));
    rd->w = width;
    rd->h = height;
    Shape s;
    s._data = rd;
    s.inside = line_signed_distance;
    s.sizes = line_sizes;
    s.free = line_free;
    return s;
}
*/