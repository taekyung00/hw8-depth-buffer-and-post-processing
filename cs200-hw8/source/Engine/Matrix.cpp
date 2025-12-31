/**
 * \file
 * \author Rudy Castan
 * \author Jonathan Holmes
 * \author Taekyung ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "Matrix.h"
#include <cmath>

namespace Math
{
    TransformationMatrix::TransformationMatrix()
    {
        Reset();
    }

    double* TransformationMatrix::operator[](int index)
    {
        return const_cast<double*>((static_cast<TransformationMatrix const&>(*this))[index]);
    }

    const double* TransformationMatrix::operator[](int index) const
    {
        return matrix[index];
    }

    TransformationMatrix TransformationMatrix::operator*(TransformationMatrix m) const
    {
        TransformationMatrix result;

        result.matrix[0][0] = matrix[0][0] * m[0][0] + matrix[0][1] * m[1][0] + matrix[0][2] * m[2][0];
        result.matrix[0][1] = matrix[0][0] * m[0][1] + matrix[0][1] * m[1][1] + matrix[0][2] * m[2][1];
        result.matrix[0][2] = matrix[0][0] * m[0][2] + matrix[0][1] * m[1][2] + matrix[0][2] * m[2][2];
        result.matrix[1][0] = matrix[1][0] * m[0][0] + matrix[1][1] * m[1][0] + matrix[1][2] * m[2][0];
        result.matrix[1][1] = matrix[1][0] * m[0][1] + matrix[1][1] * m[1][1] + matrix[1][2] * m[2][1];
        result.matrix[1][2] = matrix[1][0] * m[0][2] + matrix[1][1] * m[1][2] + matrix[1][2] * m[2][2];
        result.matrix[2][0] = matrix[2][0] * m[0][0] + matrix[2][1] * m[1][0] + matrix[2][2] * m[2][0];
        result.matrix[2][1] = matrix[2][0] * m[0][1] + matrix[2][1] * m[1][1] + matrix[2][2] * m[2][1];
        result.matrix[2][2] = matrix[2][0] * m[0][2] + matrix[2][1] * m[1][2] + matrix[2][2] * m[2][2];

        return result;
    }

	TransformationMatrix TransformationMatrix::Inverse() const
	{
        TransformationMatrix inv;

        const double m00 = matrix[0][0]; const double m01 = matrix[0][1]; const double m02 = matrix[0][2];
        const double m10 = matrix[1][0]; const double m11 = matrix[1][1]; const double m12 = matrix[1][2];
        const double m20 = matrix[2][0]; const double m21 = matrix[2][1]; const double m22 = matrix[2][2];

        double det = m00 * (m11 * m22 - m12 * m21) 
                   - m01 * (m10 * m22 - m12 * m20) 
                   + m02 * (m10 * m21 - m11 * m20);

        if (std::abs(det) < 1e-5) 
        {
            inv.Reset(); 
            return inv;
        }

        double invDet = 1.0 / det;
		// Row 0
        inv[0][0] =  (m11 * m22 - m12 * m21) * invDet;
        inv[0][1] = -(m01 * m22 - m02 * m21) * invDet; // Transposed indices (0,1 -> cofactor of 1,0)
        inv[0][2] =  (m01 * m12 - m02 * m11) * invDet; 

        // Row 1
        inv[1][0] = -(m10 * m22 - m12 * m20) * invDet;
        inv[1][1] =  (m00 * m22 - m02 * m20) * invDet;
        inv[1][2] = -(m00 * m12 - m02 * m10) * invDet;

        // Row 2
        inv[2][0] =  (m10 * m21 - m11 * m20) * invDet;
        inv[2][1] = -(m00 * m21 - m01 * m20) * invDet;
        inv[2][2] =  (m00 * m11 - m01 * m10) * invDet;

        return inv;
	}


	TransformationMatrix& TransformationMatrix::operator*=(TransformationMatrix m)
    {
        (*this) = (*this) * m;
        return (*this);
    }

    vec2 TransformationMatrix::operator*(vec2 v) const
    {
        Math::vec2 result;
        result.x = matrix[0][0] * v.x + matrix[0][1] * v.y + matrix[0][2];
        result.y = matrix[1][0] * v.x + matrix[1][1] * v.y + matrix[1][2];
        return result;
    }

    vec3 TransformationMatrix::operator*(vec3 v) const
    {
        Math::vec3 result;
        result.x = matrix[0][0] * v.x + matrix[0][1] * v.y + matrix[0][2] * v.x;
        result.y = matrix[1][0] * v.x + matrix[1][1] * v.y + matrix[1][2] * v.z;
        return result;
    }

    void TransformationMatrix::Reset()
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                matrix[i][j] = 0;
            }
        }
        matrix[0][0] = 1.0;
        matrix[1][1] = 1.0;
        matrix[2][2] = 1.0;
    }

    TranslationMatrix::TranslationMatrix(ivec2 translate)
    {
        Reset();
        matrix[0][2] = translate.x;
        matrix[1][2] = translate.y;
    }

    TranslationMatrix::TranslationMatrix(vec2 translate)
    {
        Reset();
        matrix[0][2] = translate.x;
        matrix[1][2] = translate.y;
    }

    RotationMatrix::RotationMatrix(double theta)
    {
        Reset();
        matrix[0][0] = std::cos(theta);
        matrix[0][1] = -std::sin(theta);
        matrix[1][0] = std::sin(theta);
        matrix[1][1] = std::cos(theta);
    }

    ScaleMatrix::ScaleMatrix(double scale)
    {
        Reset();
        matrix[0][0] *= scale;
        matrix[1][1] *= scale;
    }

    ScaleMatrix::ScaleMatrix(vec2 scale)
    {
        Reset();
        matrix[0][0] *= scale.x;
        matrix[1][1] *= scale.y;
    }
}
