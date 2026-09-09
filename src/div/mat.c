#include "mat.h"

void mat_apply(Mat m, float *x, float *y) {
    if (m.type == M2) {
        float nx = m.m2.a * *x + m.m2.b * *y;
        float ny = m.m2.c * *x + m.m2.d * *y;
        *x = nx; *y = ny;
    } else {
        float px = *x, py = *y;
        float nx = m.m3.a*px + m.m3.b*py + m.m3.c;
        float ny = m.m3.d*px + m.m3.e*py + m.m3.f;
        float nw = m.m3.g*px + m.m3.h*py + m.m3.i;
        if (fabsf(nw) < 1e-6f) nw = (nw < 0) ? -1e-6f : 1e-6f;
        *x = nx / nw;
        *y = ny / nw;
    }
}

Mat mat_inverse(Mat m) {
    if (m.type == M2) {
        float det = m.m2.a*m.m2.d - m.m2.b*m.m2.c;
        float id = (det != 0.0f) ? 1.0f/det : 0.0f;
        return (Mat){ .type = M2, .m2 = {
            .a =  m.m2.d*id, .b = -m.m2.b*id,
            .c = -m.m2.c*id, .d =  m.m2.a*id
        }};
    }

    float a=m.m3.a, b=m.m3.b, c=m.m3.c,
          d=m.m3.d, e=m.m3.e, f=m.m3.f,
          g=m.m3.g, h=m.m3.h, i=m.m3.i;

    float C00 = e*i - f*h;
    float C01 = f*g - d*i;
    float C02 = d*h - e*g;
    float C10 = c*h - b*i;
    float C11 = a*i - c*g;
    float C12 = b*g - a*h;
    float C20 = b*f - c*e;
    float C21 = c*d - a*f;
    float C22 = a*e - b*d;

    float det = a*C00 + b*C01 + c*C02;
    float id = (det != 0.0f) ? 1.0f/det : 0.0f;

    return (Mat){ .type = M3, .m3 = {
        .a = C00*id, .b = C10*id, .c = C20*id,
        .d = C01*id, .e = C11*id, .f = C21*id,
        .g = C02*id, .h = C12*id, .i = C22*id
    }};
}

Mat mat_mul(Mat m, Mat n) {
    if (m.type == M2 && n.type == M2) {
        return (Mat){ .type = M2, .m2 = {
            .a = m.m2.a*n.m2.a + m.m2.b*n.m2.c,
            .b = m.m2.a*n.m2.b + m.m2.b*n.m2.d,
            .c = m.m2.c*n.m2.a + m.m2.d*n.m2.c,
            .d = m.m2.c*n.m2.b + m.m2.d*n.m2.d,
        }};
    }

    if (m.type == M2) {
        m.type = M3;
        m.m3.c = 0;
        m.m3.f = 0;
        m.m3.g = 0;
        m.m3.h = 0;
        m.m3.i = 1;
    }

    if (n.type == M2) {
        n.type = M3;
        n.m3.c = 0;
        n.m3.f = 0;
        n.m3.g = 0;
        n.m3.h = 0;
        n.m3.i = 1;
    }

    return (Mat){ .type = M3, .m3 = {
        .a = m.m3.a*n.m3.a + m.m3.b*n.m3.d + m.m3.c*n.m3.g,
        .b = m.m3.a*n.m3.b + m.m3.b*n.m3.e + m.m3.c*n.m3.h,
        .c = m.m3.a*n.m3.c + m.m3.b*n.m3.f + m.m3.c*n.m3.i,

        .d = m.m3.d*n.m3.a + m.m3.e*n.m3.d + m.m3.f*n.m3.g,
        .e = m.m3.d*n.m3.b + m.m3.e*n.m3.e + m.m3.f*n.m3.h,
        .f = m.m3.d*n.m3.c + m.m3.e*n.m3.f + m.m3.f*n.m3.i,

        .g = m.m3.g*n.m3.a + m.m3.h*n.m3.d + m.m3.i*n.m3.g,
        .h = m.m3.g*n.m3.b + m.m3.h*n.m3.e + m.m3.i*n.m3.h,
        .i = m.m3.g*n.m3.c + m.m3.h*n.m3.f + m.m3.i*n.m3.i,
    }};
}

// Transform
Mat rotate(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);

    // [ c   -s ]
    // [ s    c ]
    return (Mat){ .type = M2, .m2 = { c, -s, s, c } };
}

Mat rotate_x(float angle, float focal_length) {
    float c = cosf(angle);
    float s = sinf(angle);

    // [ 1      0       0 ]
    // [ 0    cos(θ)    0 ]
    // [ 0   sin(θ)/f   1 ]
    return (Mat){
        .type = M3,
        .m3 = {
            .a = 1, .b = 0, .c = 0,
            .d = 0, .e = c, .f = 0,
            .g = 0, .h = s / focal_length, .i = 1
        }
    };
}

Mat rotate_y(float angle, float focal_length) {
    float c = cosf(angle);
    float s = sinf(angle);

    // [  c     0   0 ]
    // [  0     1   0 ]
    // [ -s/f   0   1 ]
    return (Mat){
        .type = M3,
        .m3 = {
            .a = c, .b = 0, .c = 0,
            .d = 0, .e = 1, .f = 0,
            .g = -s / focal_length, .h = 0, .i = 1
        }
    };
}


Mat translate(float x, float y) {
    // [ 1   0   x ]
    // [ 0   1   y ]
    // [ 0   0   1 ]
    return (Mat){
        .type = M3,
        .m3 = {
            .a = 1, .b = 0, .c = x,
            .d = 0, .e = 1, .f = y,
            .g = 0, .h = 0, .i = 1
        }
    };
}

Mat scale(float x, float y) {
    // [ x   0 ]
    // [ 0   y ]
    return (Mat){
        .type = M2,
        .m2 = {
            .a = x, .b = 0,
            .c = 0, .d = y
        }
    };
}