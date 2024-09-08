#include "Matrix.h"
#include "Vec.h"

// TODO: refactor, got this from FNA repo
CDraw_Matrix4x4 Matrix_Multiply_Matrix(CDraw_Matrix4x4 matrix1, CDraw_Matrix4x4 matrix2)
{
    CDraw_Matrix4x4 result;
    
    float m11 = (
        (matrix1.M11 * matrix2.M11) +
        (matrix1.M12 * matrix2.M21) +
        (matrix1.M13 * matrix2.M31) +
        (matrix1.M14 * matrix2.M41)
    );
    float m12 = (
        (matrix1.M11 * matrix2.M12) +
        (matrix1.M12 * matrix2.M22) +
        (matrix1.M13 * matrix2.M32) +
        (matrix1.M14 * matrix2.M42)
    );
    float m13 = (
        (matrix1.M11 * matrix2.M13) +
        (matrix1.M12 * matrix2.M23) +
        (matrix1.M13 * matrix2.M33) +
        (matrix1.M14 * matrix2.M43)
    );
    float m14 = (
        (matrix1.M11 * matrix2.M14) +
        (matrix1.M12 * matrix2.M24) +
        (matrix1.M13 * matrix2.M34) +
        (matrix1.M14 * matrix2.M44)
    );
    float m21 = (
        (matrix1.M21 * matrix2.M11) +
        (matrix1.M22 * matrix2.M21) +
        (matrix1.M23 * matrix2.M31) +
        (matrix1.M24 * matrix2.M41)
    );
    float m22 = (
        (matrix1.M21 * matrix2.M12) +
        (matrix1.M22 * matrix2.M22) +
        (matrix1.M23 * matrix2.M32) +
        (matrix1.M24 * matrix2.M42)
    );
    float m23 = (
        (matrix1.M21 * matrix2.M13) +
        (matrix1.M22 * matrix2.M23) +
        (matrix1.M23 * matrix2.M33) +
        (matrix1.M24 * matrix2.M43)
        );
    float m24 = (
        (matrix1.M21 * matrix2.M14) +
        (matrix1.M22 * matrix2.M24) +
        (matrix1.M23 * matrix2.M34) +
        (matrix1.M24 * matrix2.M44)
    );
    float m31 = (
        (matrix1.M31 * matrix2.M11) +
        (matrix1.M32 * matrix2.M21) +
        (matrix1.M33 * matrix2.M31) +
        (matrix1.M34 * matrix2.M41)
    );
    float m32 = (
        (matrix1.M31 * matrix2.M12) +
        (matrix1.M32 * matrix2.M22) +
        (matrix1.M33 * matrix2.M32) +
        (matrix1.M34 * matrix2.M42)
    );
    float m33 = (
        (matrix1.M31 * matrix2.M13) +
        (matrix1.M32 * matrix2.M23) +
        (matrix1.M33 * matrix2.M33) +
        (matrix1.M34 * matrix2.M43)
    );
    float m34 = (
        (matrix1.M31 * matrix2.M14) +
        (matrix1.M32 * matrix2.M24) +
        (matrix1.M33 * matrix2.M34) +
        (matrix1.M34 * matrix2.M44)
    );
    float m41 = (
        (matrix1.M41 * matrix2.M11) +
        (matrix1.M42 * matrix2.M21) +
        (matrix1.M43 * matrix2.M31) +
        (matrix1.M44 * matrix2.M41)
    );
    float m42 = (
        (matrix1.M41 * matrix2.M12) +
        (matrix1.M42 * matrix2.M22) +
        (matrix1.M43 * matrix2.M32) +
        (matrix1.M44 * matrix2.M42)
    );
    float m43 = (
        (matrix1.M41 * matrix2.M13) +
        (matrix1.M42 * matrix2.M23) +
        (matrix1.M43 * matrix2.M33) +
        (matrix1.M44 * matrix2.M43)
    );
    float m44 = (
        (matrix1.M41 * matrix2.M14) +
        (matrix1.M42 * matrix2.M24) +
        (matrix1.M43 * matrix2.M34) +
        (matrix1.M44 * matrix2.M44)
    );
    
    result.M11 = m11;
    result.M12 = m12;
    result.M13 = m13;
    result.M14 = m14;
    result.M21 = m21;
    result.M22 = m22;
    result.M23 = m23;
    result.M24 = m24;
    result.M31 = m31;
    result.M32 = m32;
    result.M33 = m33;
    result.M34 = m34;
    result.M41 = m41;
    result.M42 = m42;
    result.M43 = m43;
    result.M44 = m44;
    
    return result;
}

CDraw_Vec4 Matrix_Multiply_Vec4(CDraw_Matrix4x4 matrix, CDraw_Vec4 vec)
{
    return (CDraw_Vec4){
        .x = (
            (vec.x * matrix.M11) +
            (vec.y * matrix.M12) +
            (vec.z * matrix.M13) +
            (vec.w * matrix.M14)
        ),
        .y = (
            (vec.x * matrix.M21) +
            (vec.y * matrix.M22) +
            (vec.z * matrix.M23) +
            (vec.w * matrix.M24)
        ),
        .z = (
            (vec.x * matrix.M31) +
            (vec.y * matrix.M32) +
            (vec.z * matrix.M33) +
            (vec.w * matrix.M34)
        ),
        .w = (
            (vec.x * matrix.M41) +
            (vec.y * matrix.M42) +
            (vec.z * matrix.M43) +
            (vec.w * matrix.M44)
        ),
    };
}
