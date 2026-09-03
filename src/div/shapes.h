#pragma once
#include "div.h"
#include <float.h>

Shape make_rect_shape(Unit width, Unit height);
Shape make_circle_shape(Unit radius);
Shape make_squircle_shape(Unit width, Unit height, float squareness);