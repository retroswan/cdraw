#pragma once

#include "Vec.h"

typedef struct CDraw_Matrix4x4
{
    float M11;
    float M12;
    float M13;
    float M14;
    
    float M21;
    float M22;
    float M23;
    float M24;
    
    float M31;
    float M32;
    float M33;
    float M34;
    
    float M41;
    float M42;
    float M43;
    float M44;
} CDraw_Matrix4x4;

CDraw_Matrix4x4 Matrix_Multiply_Matrix(CDraw_Matrix4x4 matrix1, CDraw_Matrix4x4 matrix2);

CDraw_Vec4 Matrix_Multiply_Vec4(CDraw_Matrix4x4 matrix, CDraw_Vec4 vec);
