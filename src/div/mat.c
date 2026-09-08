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