#pragma once
#include <math.h>

typedef struct {
    enum { M2, M3 } type;
    union {
        struct { float a, b, c, d; } m2;
        struct { float a, b, d, e, c, f, g, h, i; } m3;
    };
} Mat;

#define MAT_IDENTITY ((Mat){ .type = M2, .m2 = {1, 0, 0, 1} })

void mat_apply(Mat m, float*  x, float*  y);
Mat mat_inverse(Mat m);
Mat mat_mul(Mat m, Mat n);
Mat rotate(float angle);
Mat rotate_x(float angle, float focal_length);
Mat rotate_y(float angle, float focal_length);
Mat translate(float x, float y);
Mat scale(float x, float y);