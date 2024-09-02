#pragma once

#include "Color.h"

typedef struct CDraw_Vertex
{
    float x, y, z;
    float u, v;
    CDraw_Color color;
} CDraw_Vertex;
