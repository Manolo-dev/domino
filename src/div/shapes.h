#pragma once
#include "div.h"
#include <float.h>

Shape make_rect(Unit width, Unit height);
Shape make_circle(Unit radius);
Shape make_squircle(Unit width, Unit height, float squareness);
Shape make_line(Unit width, Unit height);