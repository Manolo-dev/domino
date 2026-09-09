#include "div.h"

static int g_screen_w = 0;
static int g_screen_h = 0;

// Utils graphiques
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

// Ma5
static inline void anchor_offset(Div *div, float *ox, float *oy) {
    *ox = ((float)(div->style.anchor / 3) * 0.5f) * div->_width;
    *oy = ((float)(div->style.anchor % 3) * 0.5f) * div->_height;
}

// Contenance div
#define SS_N 2

float div_signed_distance(Div *div, float sx, float sy) {
    mat_apply(div->_world_inv, &sx, &sy);
    float rx = sx - div->_left;
    float ry = sy - div->_top;
    return div->shape.inside(div->shape._data, rx, ry);
}

bool div_hit(Div *div, float x, float y) {
    return div_signed_distance(div, x, y) <= 0.0f;
}

static float div_coverage(Div *div, int screen_x, int screen_y) {
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

// Utils div
Div make_div(Shape shape, Style style) {
    Div div = {0};
    div.shape         = shape;
    div.style         = style;
    div._inv          = MAT_IDENTITY;
    div._dirty        = true;
    div.onclick       = NULL;
    div._parent       = NULL;
    div._first_child  = NULL;
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
    else { w = 0; h = 0; }

    int parent_x = div->_parent ? div->_parent->_left : 0;
    int parent_y = div->_parent ? div->_parent->_top : 0;

    div->_left   = (int)div->style.left + parent_x;
    div->_top    = (int)div->style.top + parent_y;
    div->_width  = (int)w;
    div->_height = (int)h;

    float ox, oy;
    anchor_offset(div, &ox, &oy);
    float px = div->_left + ox, py = div->_top + oy;

    Mat local = mat_mul(translate(px, py), div->style.transform);
    local = mat_mul(local, translate(-px, -py));

    div->_world     = div->_parent ? mat_mul(div->_parent->_world, local) : local;
    div->_world_inv = mat_inverse(div->_world);
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

static void div_screen_bbox(Div *div, int *x0, int *y0, int *x1, int *y1) {
    float cx[4] = {0, (float)div->_width, 0, (float)div->_width};
    float cy[4] = {0, 0, (float)div->_height, (float)div->_height};

    float minx = 1e9f, miny = 1e9f, maxx = -1e9f, maxy = -1e9f;
    for (int k = 0; k < 4; k++) {
        float x = div->_left + cx[k];
        float y = div->_top  + cy[k];
        mat_apply(div->_world, &x, &y);
        if (x < minx) minx = x; if (x > maxx) maxx = x;
        if (y < miny) miny = y; if (y > maxy) maxy = y;
    }

    int pad = (int)ceilf(div->style.antialiasing > 0.0f ? div->style.antialiasing : 1.0f);
    *x0 = (int)floorf(minx) - pad;
    *y0 = (int)floorf(miny) - pad;
    *x1 = (int)ceilf(maxx) + pad;
    *y1 = (int)ceilf(maxy) + pad;
}

static void real_div_draw(Div *div, Buffer *buffer, float accumulated_alpha) {
    float effective_alpha = accumulated_alpha * div->style.alpha;
    if (effective_alpha <= 0.0f) return;

    uint32_t *pixels = buffer->bits;

    int x0, y0, x1, y1;
    div_screen_bbox(div, &x0, &y0, &x1, &y1);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int)buffer->width)  x1 = buffer->width;
    if (y1 > (int)buffer->height) y1 = buffer->height;

    for (int ty = y0; ty < y1; ty++) {
        for (int tx = x0; tx < x1; tx++) {
            float cov = div_coverage(div, tx, ty);
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