#pragma once
#include "Vector3.h"
#include "Vector4.h"

namespace Aurora::Math
{
    // We do our matrix operations in the engine as row-major, but represent the memory of our matrix as column-major for HLSL.

    class Matrix
    {
    public:
        Matrix()
        {
            SetIdentity();
        }

        Matrix(float m00, float m01, float m02, float m03,
               float m10, float m11, float m12, float m13,
               float m20, float m21, float m22, float m23,
               float m30, float m31, float m32, float m33)
        {
            this->m00 = m00; this->m01 = m01; this->m02 = m02; this->m03 = m03;
            this->m10 = m10; this->m11 = m11; this->m12 = m12; this->m13 = m13;
            this->m20 = m20; this->m21 = m21; this->m22 = m22; this->m23 = m23;
            this->m30 = m30; this->m31 = m31; this->m32 = m32; this->m33 = m33;
        }

        void SetIdentity()
        {
            m00 = 1; m01 = 0; m02 = 0; m03 = 0;
            m10 = 0; m11 = 1; m12 = 0; m13 = 0;
            m20 = 0; m21 = 0; m22 = 1; m23 = 0;
            m30 = 0; m31 = 0; m32 = 0; m33 = 1;
        }

        // ==== Translation ====
        Vector3 GetTranslation() const { return Vector3(m30, m31, m32); }

        static inline Matrix CreateTranslation(const Vector3& translation)
        {
            return Matrix(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                translation.x, translation.y, translation.z, 1
            );
        }

        // ==== Rotation ====

        // ==== Scale ==== 
        Vector3 GetScale()
        {
            // 00 , 11, 22
        }

        static inline Matrix CreateScale(float scale) { return CreateScale(scale, scale, scale); }
        static inline Matrix CreateScale(const Vector3& scale) { return CreateScale(scale.x, scale.y, scale.z); }
        static inline Matrix CreateScale(float scaleX, float scaleY, float scaleZ)
        {
            return Matrix(
                scaleX, 0, 0, 0,
                0, scaleY, 0, 0,
                0, 0, scaleZ, 0,
                0, 0, 0, 1
            );
        }

        // Translate to the origin, rotate it according to the orientation.
        static inline Matrix CreateLookAtLH(const Vector3& cameraPosition, const Vector3& target, const Vector3& upDirection)
        {
            const Vector3 zAxis = Vector3::Normalize(target - cameraPosition); // Forward Direction
            const Vector3 xAxis = Vector3::Normalize(Vector3::Cross(upDirection, zAxis));
            const Vector3 yAxis = Vector3::Cross(zAxis, xAxis);

            return Matrix(
                xAxis.x, yAxis.x, zAxis.x, 0,
                xAxis.y, yAxis.y, zAxis.y, 0,
                xAxis.z, yAxis.z, zAxis.z, 0,
                -Vector3::Dot(xAxis, cameraPosition), -Vector3::Dot(yAxis, cameraPosition), -Vector3::Dot(zAxis, cameraPosition), 1
            );  
        }

        // Addition 

        // Substraction

        // Multiplication - Row by Column.
        Matrix operator*(const Matrix& otherMatrix) const
        {
            return Matrix(
                this->m00 * otherMatrix.m00 + this->m01 * otherMatrix.m10 + this->m02 * otherMatrix.m20 + this->m03 * otherMatrix.m30,
                this->m00 * otherMatrix.m01 + this->m01 * otherMatrix.m11 + this->m02 * otherMatrix.m21 + this->m03 * otherMatrix.m31,
                this->m00 * otherMatrix.m02 + this->m01 * otherMatrix.m12 + this->m02 * otherMatrix.m22 + this->m03 * otherMatrix.m32,
                this->m00 * otherMatrix.m03 + this->m01 * otherMatrix.m13 + this->m02 * otherMatrix.m23 + this->m03 * otherMatrix.m33,

                this->m10 * otherMatrix.m00 + this->m11 * otherMatrix.m10 + this->m12 * otherMatrix.m20 + this->m13 * otherMatrix.m30,
                this->m10 * otherMatrix.m01 + this->m11 * otherMatrix.m11 + this->m12 * otherMatrix.m21 + this->m13 * otherMatrix.m31,
                this->m10 * otherMatrix.m02 + this->m11 * otherMatrix.m12 + this->m12 * otherMatrix.m22 + this->m13 * otherMatrix.m32,
                this->m10 * otherMatrix.m03 + this->m11 * otherMatrix.m13 + this->m12 * otherMatrix.m23 + this->m13 * otherMatrix.m33,

                this->m20 * otherMatrix.m00 + this->m21 * otherMatrix.m10 + this->m22 * otherMatrix.m20 + this->m23 * otherMatrix.m30,
                this->m20 * otherMatrix.m01 + this->m21 * otherMatrix.m11 + this->m22 * otherMatrix.m21 + this->m23 * otherMatrix.m31,
                this->m20 * otherMatrix.m02 + this->m21 * otherMatrix.m12 + this->m22 * otherMatrix.m22 + this->m23 * otherMatrix.m32,
                this->m20 * otherMatrix.m03 + this->m21 * otherMatrix.m13 + this->m22 * otherMatrix.m22 + this->m23 * otherMatrix.m33,

                this->m30 * otherMatrix.m00 + this->m31 * otherMatrix.m10 + this->m32 * otherMatrix.m20 + this->m33 * otherMatrix.m30,
                this->m30 * otherMatrix.m01 + this->m31 * otherMatrix.m11 + this->m32 * otherMatrix.m21 + this->m33 * otherMatrix.m31,
                this->m30 * otherMatrix.m02 + this->m31 * otherMatrix.m12 + this->m32 * otherMatrix.m22 + this->m33 * otherMatrix.m32,
                this->m30 * otherMatrix.m03 + this->m31 * otherMatrix.m13 + this->m32 * otherMatrix.m23 + this->m33 * otherMatrix.m33
            );
        }

        void operator*=(const Matrix& otherMatrix)
        {
            (*this) = (*this) * otherMatrix;
        }

        // Returns a 3x1. Hence, we insert a 4th W component and make it a Vector4 for multiplication purposes and return a Vector3 multiplied by the W component.
        Vector3 operator*(const Vector3& otherVector) const
        {
            // Hence, we are essentially working with a vector 4, but with 1 as the W componenot.
            Vector4 workingVector;

            workingVector.x = (workingVector.x * m00) + (workingVector.y * m10) + (workingVector.z * m20) + m30;
            workingVector.y = (workingVector.x * m01) + (workingVector.y * m11) + (workingVector.z * m21) + m31;
            workingVector.z = (workingVector.x * m02) + (workingVector.y * m12) + (workingVector.z * m22) + m32;
            workingVector.w = 1 / ((workingVector.x * m03) + (workingVector.y * m13) + (workingVector.z * m23) + m33); 

            return Vector3(workingVector.x * workingVector.w, workingVector.y * workingVector.w, workingVector.z * workingVector.w);
        }

        // Returns a 4x1 Matrix, essentially our Vector4.
        Vector4 operator*(const Vector4& otherVector) const
        {
            return Vector4(
                (otherVector.x * m00) + (otherVector.y * m10) + (otherVector.z * m20) + (otherVector.w * m30),
                (otherVector.x * m01) + (otherVector.y * m11) + (otherVector.z * m21) + (otherVector.w + m31),
                (otherVector.x * m02) + (otherVector.y * m12) + (otherVector.z * m22) + (otherVector.w + m32),
                (otherVector.x * m03) + (otherVector.y * m13) + (otherVector.z * m23) + (otherVector.w * m33)
            );
        }

    public:
        float* Data() { return &m00; }
        std::string ToString() const;

        // Column-major memory representation.
        float m00 = 0.0f, m10 = 0.0f, m20 = 0.0f, m30 = 0.0f;
        float m01 = 0.0f, m11 = 0.0f, m21 = 0.0f, m31 = 0.0f;
        float m02 = 0.0f, m12 = 0.0f, m22 = 0.0f, m32 = 0.0f;
        float m03 = 0.0f, m13 = 0.0f, m23 = 0.0f, m33 = 0.0f;
        // Note: HLSL expects column-major by default.

        static const Matrix Identity;
    };
}