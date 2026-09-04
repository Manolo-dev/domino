#include "div.h"

static int g_screen_w = 0;
static int g_screen_h = 0;

int to_pixels(Unit u) {
    switch (u.tag) {
        case PX: return u.value.i;
        case VW: return (int)(u.value.f * g_screen_w / 100.0f);
        case VH: return (int)(u.value.f * g_screen_h / 100.0f);
        default: return 0;
    }
}

static inline uint32_t blend(uint32_t bg, uint32_t fg, float a) {
    if (a <= 0.0f) return bg;
    if (a >= 1.0f) return fg;
    uint8_t br = bg & 0xFF, bgc = (bg>>8)&0xFF, bb = (bg>>16)&0xFF, ba = (bg>>24)&0xFF;
    uint8_t fr = fg & 0xFF, fgc = (fg>>8)&0xFF, fb = (fg>>16)&0xFF, fa = (fg>>24)&0xFF;
    uint8_t  r = (uint8_t)(fr*a + br*(1.0f-a));
    uint8_t  g = (uint8_t)(fgc*a + bgc*(1.0f-a));
    uint8_t  b = (uint8_t)(fb*a + bb*(1.0f-a));
    uint8_t al = (uint8_t)(fa*a + ba*(1.0f-a));
    return ((uint32_t)al<<24) | ((uint32_t)b<<16) | ((uint32_t)g<<8) | r;
}

// Div
static void apply_inverse_transforms_f(Div *div, float *x, float *y) {
    if (!div) return;
    apply_inverse_transforms_f(div->_parent, x, y);
    if (div->style.transform) {
        float lx = *x - div->_left;
        float ly = *y - div->_top;
        div->style.transform(&lx, &ly);
        *x = lx + div->_left;
        *y = ly + div->_top;
    }
}

#define SS_N 2

float div_signed_distance(Div *div, float sx, float sy) {
    apply_inverse_transforms_f(div, &sx, &sy);
    float rx = sx - div->_left;
    float ry = sy - div->_top;
    return div->shape.inside(div->shape._data, rx, ry);
}

bool div_hit(Div *div, float x, float y) {
    return div_signed_distance(div, x, y) <= 0.0f;
}

Div make_div(Shape shape, Style style) {
    Div div = {0};
    div.shape = shape;
    div.style = style;
    div.onclick = NULL;
    div._parent = NULL;
    div._first_child = NULL;
    div._next_sibling = NULL;
    return div;
}

void div_onclick(Div *div, Onclick onclick) {
    div->onclick = onclick;
}

void div_add_child(Div *parent, Div *child) {
    child->_parent = parent;
    child->_next_sibling = NULL;

    if (!parent->_first_child) parent->_first_child = child;
    else {
        Div *last = parent->_first_child;

        while (last->_next_sibling)
            last = last->_next_sibling;
        
        last->_next_sibling = child;
    }
}

void div_update(Div *div, int screen_w, int screen_h) {
    g_screen_w = screen_w;
    g_screen_h = screen_h;

    Unit w, h;
    if (div->shape.sizes) div->shape.sizes(div->shape._data, &w, &h);
    else {
        w = make_px(0);
        h = make_px(0);
    }

    int parent_x = div->_parent ? div->_parent->_left : 0;
    int parent_y = div->_parent ? div->_parent->_top : 0;

    div->_left   = to_pixels(div->style.left) + parent_x;
    div->_top    = to_pixels(div->style.top) + parent_y;
    div->_width  = to_pixels(w);
    div->_height = to_pixels(h);
}

void div_tree_update(Div *root, int screen_w, int screen_h) {
    if (!root) return;
    div_update(root, screen_w, screen_h);
    Div *child = root->_first_child;
    while (child) {
        div_tree_update(child, screen_w, screen_h);
        child = child->_next_sibling;
    }
}

static float div_coverage_supersampled(Div *div, int screen_x, int screen_y) {
    float hits = 0.0f;
    for (int j = 0; j < SS_N; j++) {
        for (int i = 0; i < SS_N; i++) {
            float sx = screen_x + (i + 0.5f) / SS_N;
            float sy = screen_y + (j + 0.5f) / SS_N;
            if (div_signed_distance(div, sx, sy) <= 0.0f) hits += 1.0f;
        }
    }
    return hits / (SS_N * SS_N);
}

float div_coverage(Div *div, int x, int y) {
    float dist = div_signed_distance(div, (float)x, (float)y);
    float aa = div->style.antialiasing > 0.0f ? div->style.antialiasing : 1.0f;
    float cov = 0.5f - dist / aa;
    if (cov < 0.0f) cov = 0.0f;
    if (cov > 1.0f) cov = 1.0f;
    return cov;
}

static void real_div_draw(Div *div, Buffer *buffer, float accumulated_alpha) {
    float effective_alpha = accumulated_alpha * div->style.alpha;
    if (effective_alpha <= 0.0f) return;

    uint32_t *pixels = buffer->bits;

    for (int ty = 0; ty < (int)buffer->height; ty++) {
        for (int tx = 0; tx < (int)buffer->width; tx++) {
            float cov = div_coverage_supersampled(div, tx, ty);
            if (cov <= 0.0f) continue;

            uint32_t *p = &pixels[ty * buffer->stride + tx];
            *p = blend(*p, (uint32_t)div->style.color, cov * effective_alpha);
        }
    }

    Div *child = div->_first_child;
    while (child) {
        real_div_draw(child, buffer, effective_alpha);
        child = child->_next_sibling;
    }
}

void div_draw(Div *div, Buffer *buffer) {
    real_div_draw(div, buffer, 1.0f);
}

void div_free(Div *div) {
    if (!div) return;

    Div *child = div->_first_child;
    while (child) {
        Div *next = child->_next_sibling;
        div_free(child);
        child = next;
    }

    if (div->shape.free) div->shape.free(div->shape._data);

    free(div);
}