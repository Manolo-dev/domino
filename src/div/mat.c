#include "mat.h"

void mat_apply(Mat m, float *x, float *y) {
    if (m.type == M2) {
        float nx = m.m2.a * *x + m.m2.c * *y;
        float ny = m.m2.b * *x + m.m2.d * *y;
        *x = nx; *y = ny;
    } else {
        float px = *x, py = *y;
        float nx = m.m3.a*px + m.m3.c*py + m.m3.e;
        float ny = m.m3.b*px + m.m3.d*py + m.m3.f;
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
    float a=m.m3.a, b=m.m3.b, c=m.m3.c, d=m.m3.d, e=m.m3.e,
          f=m.m3.f, g=m.m3.g, h=m.m3.h, i=m.m3.i;

    float A =  (d*i - f*h), B = -(b*i - f*g), C =  (b*h - d*g);
    float D = -(c*i - e*h), E =  (a*i - e*g), F = -(a*h - c*g);
    float G =  (c*f - e*d), H = -(a*f - e*b), I =  (a*d - c*b);

    float det = a*A + c*B + e*C;
    float id = (det != 0.0f) ? 1.0f/det : 0.0f;

    return (Mat){ .type = M3, .m3 = {
        .a = A*id, .b = B*id, .c = D*id,
        .d = E*id, .e = G*id, .f = H*id,
        .g = C*id, .h = F*id, .i = I*id
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
        m.m3.f = 1;
    }

    if (n.type == M2) {
        n.type = M3;
        n.m3.c = 0;
        n.m3.f = 0;
        n.m3.g = 0;
        n.m3.h = 0;
        n.m3.f = 1;
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
            .d = c, .e = 0, .f = 0,
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