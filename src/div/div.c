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
    uint8_t r = (uint8_t)(fr*a + br*(1.0f-a));
    uint8_t g = (uint8_t)(fgc*a + bgc*(1.0f-a));
    uint8_t b = (uint8_t)(fb*a + bb*(1.0f-a));
    uint8_t al= (uint8_t)(fa*a + ba*(1.0f-a));
    return ((uint32_t)al<<24) | ((uint32_t)b<<16) | ((uint32_t)g<<8) | r;
}

// Div
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

    if (!parent->_first_child)
        parent->_first_child = child;
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
    if (div->shape.get_sizes)
        div->shape.get_sizes(div->shape._data, &w, &h);
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

float div_coverage(Div *div, int x, int y) {
    float rx = (float)(x - div->_left);
    float ry = (float)(y - div->_top);
    float dist = div->shape.is_inside(div->shape._data, rx, ry);
    float cov = 0.5f - dist;
    if (cov < 0.0f) cov = 0.0f;
    if (cov > 1.0f) cov = 1.0f;
    return cov;
}

void div_draw(Div *div, ANativeWindow_Buffer *buffer) {
    uint32_t *pixels = (uint32_t*)buffer->bits;

    int x0 = div->_left - 5;
    int y0 = div->_top - 5;
    int x1 = div->_left + div->_width + 5;
    int y1 = div->_top + div->_height + 5;

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            float cov = div_coverage(div, x, y);
            if (cov <= 0.0f) continue;

            int tx = x, ty = y;
            for (TransformNode *t = div->style.transform; t; t = t->next)
                t->fn(&tx, &ty);

            if (tx < 0 || tx >= buffer->width) continue;
            if (ty < 0 || ty >= buffer->height) continue;

            uint32_t *p = &pixels[ty * buffer->stride + tx];
            *p = blend(*p, (uint32_t)div->style.color, cov);
        }
    }

    Div *child = div->_first_child;
    while (child) {
        div_draw(child, buffer);
        child = child->_next_sibling;
    }
}

static void transform_add(Style *style, Transform transform) {
    TransformNode *node = malloc(sizeof(TransformNode));
    node->fn = transform;
    node->next = NULL;

    if (!style->transform) {
        style->transform = node;
    } else {
        TransformNode *last = style->transform;
        while (last->next) last = last->next;
        last->next = node;
    }
}

void div_transform(Div *div, Transform transform) {
    if (!div) return;
    transform_add(&div->style, transform);
    Div *child = div->_first_child;
    while (child) {
        div_transform(child, transform);
        child = child->_next_sibling;
    }
}