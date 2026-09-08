#pragma once
#include <math.h>

typedef struct {
    enum { M2, M3 } type;
    union {
        struct { float a, b, c, d; } m2;
        struct { float a, b, c, d, e, f, g, h, i; } m3;
    };
} Mat;

#define MAT_IDENTITY ((Mat){ .type = M2, .m2 = {1, 0, 0, 1} })

void mat_apply(Mat m, float *x, float *y);
Mat mat_inverse(Mat m);