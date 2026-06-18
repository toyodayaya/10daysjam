#include "MathManager.h"
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cassert>

namespace MathManager
{
	Matrix4x4 MakeIdentity4x4()
	{
		Matrix4x4 ret;
		ret.m[0][0] = 1.0f; ret.m[0][1] = 0.0f; ret.m[0][2] = 0.0f; ret.m[0][3] = 0.0f;
		ret.m[1][0] = 0.0f; ret.m[1][1] = 1.0f; ret.m[1][2] = 0.0f; ret.m[1][3] = 0.0f;
		ret.m[2][0] = 0.0f; ret.m[2][1] = 0.0f; ret.m[2][2] = 1.0f; ret.m[2][3] = 0.0f;
		ret.m[3][0] = 0.0f; ret.m[3][1] = 0.0f; ret.m[3][2] = 0.0f; ret.m[3][3] = 1.0f;

		return ret;
	}

	Matrix4x4 MakeRotateXMatrix(float radian)
	{
		Matrix4x4 ret;
		ret.m[0][0] = 1.0f; ret.m[0][1] = 0.0f; ret.m[0][2] = 0.0f; ret.m[0][3] = 0.0f;
		ret.m[1][0] = 0.0f; ret.m[1][1] = std::cos(radian); ret.m[1][2] = std::sin(radian); ret.m[1][3] = 0.0f;
		ret.m[2][0] = 0.0f; ret.m[2][1] = -std::sin(radian); ret.m[2][2] = std::cos(radian); ret.m[2][3] = 0.0f;
		ret.m[3][0] = 0.0f; ret.m[3][1] = 0.0f; ret.m[3][2] = 0.0f; ret.m[3][3] = 1.0f;
		return ret;
	}


	Matrix4x4 MakeRotateYMatrix(float radian)
	{
		Matrix4x4 ret;
		ret.m[0][0] = std::cos(radian); ret.m[0][1] = 0.0f; ret.m[0][2] = -std::sin(radian); ret.m[0][3] = 0.0f;
		ret.m[1][0] = 0.0f; ret.m[1][1] = 1.0f; ret.m[1][2] = 0.0f; ret.m[1][3] = 0.0f;
		ret.m[2][0] = std::sin(radian); ret.m[2][1] = 0.0f; ret.m[2][2] = std::cos(radian); ret.m[2][3] = 0.0f;
		ret.m[3][0] = 0.0f; ret.m[3][1] = 0.0f; ret.m[3][2] = 0.0f; ret.m[3][3] = 1.0f;
		return ret;
	}

	Matrix4x4 MakeRotateZMatrix(float radian)
	{
		Matrix4x4 ret;
		ret.m[0][0] = std::cos(radian); ret.m[0][1] = std::sin(radian); ret.m[0][2] = 0.0f; ret.m[0][3] = 0.0f;
		ret.m[1][0] = -std::sin(radian); ret.m[1][1] = std::cos(radian); ret.m[1][2] = 0.0f; ret.m[1][3] = 0.0f;
		ret.m[2][0] = 0.0f; ret.m[2][1] = 0.0f; ret.m[2][2] = 1.0f; ret.m[2][3] = 0.0f;
		ret.m[3][0] = 0.0f; ret.m[3][1] = 0.0f; ret.m[3][2] = 0.0f; ret.m[3][3] = 1.0f;
		return ret;
	}

	Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion)
	{
		Matrix4x4 ret;
		float xx = quaternion.x * quaternion.x;
		float yy = quaternion.y * quaternion.y;
		float zz = quaternion.z * quaternion.z;
		float xy = quaternion.x * quaternion.y;
		float xz = quaternion.x * quaternion.z;
		float yz = quaternion.y * quaternion.z;
		float wx = quaternion.w * quaternion.x;
		float wy = quaternion.w * quaternion.y;
		float wz = quaternion.w * quaternion.z;

		// 1行目
		ret.m[0][0] = 1.0f - 2.0f * (yy + zz);
		ret.m[0][1] = 2.0f * (xy + wz);
		ret.m[0][2] = 2.0f * (xz - wy);
		ret.m[0][3] = 0.0f;

		// 2行目
		ret.m[1][0] = 2.0f * (xy - wz);
		ret.m[1][1] = 1.0f - 2.0f * (xx + zz);
		ret.m[1][2] = 2.0f * (yz + wx);
		ret.m[1][3] = 0.0f;

		// 3行目
		ret.m[2][0] = 2.0f * (xz + wy);
		ret.m[2][1] = 2.0f * (yz - wx);
		ret.m[2][2] = 1.0f - 2.0f * (xx + yy);
		ret.m[2][3] = 0.0f;

		// 4行目
		ret.m[3][0] = 0.0f;
		ret.m[3][1] = 0.0f;
		ret.m[3][2] = 0.0f;
		ret.m[3][3] = 1.0f;

		return ret;
	}

	Matrix4x4 MakeScaleMatrix(const Vector3& scale)
	{
		Matrix4x4 m;
		m.m[0][0] = scale.x; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
		m.m[1][0] = 0.0f; m.m[1][1] = scale.y; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
		m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = scale.z; m.m[2][3] = 0.0f;
		m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
		return m;
	}


	Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
	{
		Matrix4x4 m;
		m.m[0][0] = 1.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
		m.m[1][0] = 0.0f; m.m[1][1] = 1.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
		m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 1.0f; m.m[2][3] = 0.0f;
		m.m[3][0] = translate.x; m.m[3][1] = translate.y; m.m[3][2] = translate.z; m.m[3][3] = 1.0f;

		return m;
	}

	// 行列の積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
	{
		Matrix4x4 ret;
		ret.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
		ret.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
		ret.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
		ret.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];
		ret.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
		ret.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
		ret.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
		ret.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];
		ret.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
		ret.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
		ret.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
		ret.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];
		ret.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
		ret.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
		ret.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
		ret.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
		return ret;
	}

	// アフィン変換行列
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
	{
		Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);
		Matrix4x4 rotateXYZ = Multiply(rotateX, Multiply(rotateY, rotateZ));

		Matrix4x4 ret;
		ret.m[0][0] = scale.x * rotateXYZ.m[0][0]; ret.m[0][1] = scale.x * rotateXYZ.m[0][1]; ret.m[0][2] = scale.x * rotateXYZ.m[0][2]; ret.m[0][3] = 0.0f;
		ret.m[1][0] = scale.y * rotateXYZ.m[1][0]; ret.m[1][1] = scale.y * rotateXYZ.m[1][1]; ret.m[1][2] = scale.y * rotateXYZ.m[1][2]; ret.m[1][3] = 0.0f;
		ret.m[2][0] = scale.z * rotateXYZ.m[2][0]; ret.m[2][1] = scale.z * rotateXYZ.m[2][1]; ret.m[2][2] = scale.z * rotateXYZ.m[2][2]; ret.m[2][3] = 0.0f;
		ret.m[3][0] = translate.x; ret.m[3][1] = translate.y; ret.m[3][2] = translate.z; ret.m[3][3] = 1.0f;

		return ret;

	}

	Matrix4x4 MakeAffineMatrixQuat(const Vector3& scale, const Quaternion& rotate, const Vector3& translate)
	{
		Matrix4x4 rotateXYZ = MakeRotateMatrix(rotate);

		Matrix4x4 ret;
		ret.m[0][0] = scale.x * rotateXYZ.m[0][0]; ret.m[0][1] = scale.x * rotateXYZ.m[0][1]; ret.m[0][2] = scale.x * rotateXYZ.m[0][2]; ret.m[0][3] = 0.0f;
		ret.m[1][0] = scale.y * rotateXYZ.m[1][0]; ret.m[1][1] = scale.y * rotateXYZ.m[1][1]; ret.m[1][2] = scale.y * rotateXYZ.m[1][2]; ret.m[1][3] = 0.0f;
		ret.m[2][0] = scale.z * rotateXYZ.m[2][0]; ret.m[2][1] = scale.z * rotateXYZ.m[2][1]; ret.m[2][2] = scale.z * rotateXYZ.m[2][2]; ret.m[2][3] = 0.0f;
		ret.m[3][0] = translate.x; ret.m[3][1] = translate.y; ret.m[3][2] = translate.z; ret.m[3][3] = 1.0f;

		return ret;
	}

	// 透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
	{
		Matrix4x4 ret = {};
		float cot = 1.0f / tanf(fovY / 2.0f);
		ret.m[0][0] = (1.0f / aspectRatio) * cot; 
		ret.m[0][1] = 0.0f; ret.m[0][2] = 0.0f; 
		ret.m[0][3] = 0.0f;
		ret.m[1][0] = 0.0f; 
		ret.m[1][1] = cot; 
		ret.m[1][2] = 0.0f; 
		ret.m[1][3] = 0.0f;
		ret.m[2][0] = 0.0f;
		ret.m[2][1] = 0.0f; 
		ret.m[2][2] = farClip / (farClip - nearClip); 
		ret.m[2][3] = 1.0f;
		ret.m[3][0] = 0.0f; 
		ret.m[3][1] = 0.0f; 
		ret.m[3][2] = (-1.0f * nearClip) * farClip / (farClip - nearClip); 
		ret.m[3][3] = 0.0f;

		return ret;
	}

	// 正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
	{
		Matrix4x4 ret;
		ret.m[0][0] = 2.0f / (right - left); ret.m[0][1] = 0.0f; ret.m[0][2] = 0.0f; ret.m[0][3] = 0.0f;
		ret.m[1][0] = 0.0f; ret.m[1][1] = 2.0f / (top - bottom); ret.m[1][2] = 0.0f; ret.m[1][3] = 0.0f;
		ret.m[2][0] = 0.0f; ret.m[2][1] = 0.0f; ret.m[2][2] = 1.0f / (farClip - nearClip); ret.m[2][3] = 0.0f;
		ret.m[3][0] = (left + right) / (left - right); ret.m[3][1] = (top + bottom) / (bottom - top); ret.m[3][2] = nearClip / (nearClip - farClip); ret.m[3][3] = 1.0f;
		return ret;
	}


	Matrix4x4 Transpose(const Matrix4x4& m)
	{
		Matrix4x4 ret;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				ret.m[i][j] = m.m[j][i]; // 行と列を入れ替える
			}
		}
		return ret;
	}

	// 逆行列
	Matrix4x4 Inverse(const Matrix4x4& m)
	{
		Matrix4x4 ret;
		// 行列式 (Determinant) の計算
		float a =
			m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
			- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
			- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
			+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
			+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
			- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
			- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
			+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

		// 0除算防止
		if (std::abs(a) < 1e-30f) return MakeIdentity4x4();

		float invA = 1.0f / a;

		// 各要素の計算 (余因子行列の転置)
		ret.m[0][0] = invA * (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);
		ret.m[0][1] = invA * (m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2] - m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2]);
		ret.m[0][2] = invA * (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);
		ret.m[0][3] = invA * (m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2] - m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2]);

		ret.m[1][0] = invA * (m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2] - m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2]);
		ret.m[1][1] = invA * (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);
		ret.m[1][2] = invA * (m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2] - m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2]);
		ret.m[1][3] = invA * (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);

		ret.m[2][0] = invA * (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);
		ret.m[2][1] = invA * (m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1] - m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1]);
		ret.m[2][2] = invA * (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);
		ret.m[2][3] = invA * (m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1] - m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1]);

		ret.m[3][0] = invA * (m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1] - m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1]);
		ret.m[3][1] = invA * (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);
		ret.m[3][2] = invA * (m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1] - m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1]);
		ret.m[3][3] = invA * (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

		return ret;
	}

	Vector3 Vector3Multiply(const Vector3& v1, const Vector3& v2)
	{
		Vector3 ret;
		ret.x = v1.x * v2.x;
		ret.y = v1.y * v2.y;
		ret.z = v1.z * v2.z;
		return ret;
	}

	Vector3 FloatMultiply(const Vector3& v1, const float v2)
	{
		Vector3 ret;
		ret.x = v1.x * v2;
		ret.y = v1.y * v2;
		ret.z = v1.z * v2;
		return ret;
	}

	Vector3 Vector3Add(const Vector3& v1, const Vector3& v2)
	{
		Vector3 ret;
		ret.x = v1.x + v2.x;
		ret.y = v1.y + v2.y;
		ret.z = v1.z + v2.z;
		return ret;
	}

	Vector3 Vector3Subtract(const Vector3& v1, const Vector3& v2)
	{
		Vector3 ret;
		ret.x = v1.x - v2.x;
		ret.y = v1.y - v2.y;
		ret.z = v1.z - v2.z;
		return ret;
	}

	Matrix4x4 Subtruct(const Matrix4x4& m1, const Matrix4x4& m2)
	{
		Matrix4x4 ret;
		ret.m[0][0] = m1.m[0][0] - m2.m[0][0]; ret.m[0][1] = m1.m[0][1] - m2.m[0][1]; ret.m[0][2] = m1.m[0][2] - m2.m[0][2]; ret.m[0][3] = m1.m[0][3] - m2.m[0][3];
		ret.m[1][0] = m1.m[1][0] - m2.m[1][0]; ret.m[1][1] = m1.m[1][1] - m2.m[1][1]; ret.m[1][2] = m1.m[1][2] - m2.m[1][2]; ret.m[1][3] = m1.m[1][3] - m2.m[1][3];
		ret.m[2][0] = m1.m[2][0] - m2.m[2][0]; ret.m[2][1] = m1.m[2][1] - m2.m[2][1]; ret.m[2][2] = m1.m[2][2] - m2.m[2][2]; ret.m[2][3] = m1.m[2][3] - m2.m[2][3];
		ret.m[3][0] = m1.m[3][0] - m2.m[3][0]; ret.m[3][1] = m1.m[3][1] - m2.m[3][1]; ret.m[3][2] = m1.m[3][2] - m2.m[3][2]; ret.m[3][3] = m1.m[3][3] - m2.m[3][3];
		return ret;
	}

	float Length(const Vector3& v)
	{
		float ret;
		ret = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
		return ret;
	}

	float LengthSquared(const Vector3& v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	float Dot(const Vector3& v1, const Vector3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	Vector3 Cross(const Vector3& v1, const Vector3& v2)
	{
		Vector3 ret;
		ret.x = v1.y * v2.z - v1.z * v2.y;
		ret.y = v1.z * v2.x - v1.x * v2.z;
		ret.z = v1.x * v2.y - v1.y * v2.x;
		return ret;
	}

	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix)
	{
		Vector3 result;
		result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
		result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
		result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
		float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
		assert(w != 0);
		result.x /= w;
		result.y /= w;
		result.z /= w;
		return result;
	}

	Vector3 Lerp(const Vector3& start, const Vector3& end, float t)
	{
		Vector3 ret;
		ret.x = start.x + (end.x - start.x) * t;
		ret.y = start.y + (end.y - start.y) * t;
		ret.z = start.z + (end.z - start.z) * t;
		return ret;
	}

	Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t)
	{
		float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
		Quaternion targetQ0 = q0;
		if (dot < 0.0f)
		{
			targetQ0.x = -1.0f * targetQ0.x;
			targetQ0.y = -1.0f * targetQ0.y;
			targetQ0.z = -1.0f * targetQ0.z;
			targetQ0.w = -1.0f * targetQ0.w;
			dot = -dot;
		}

			if (dot > 0.9995f)
		{
			// 0除算を避けるため、通常の線形補間（Lerp）を行う
			Quaternion ret{};
			ret.x = (1.0f - t) * targetQ0.x + t * q1.x;
			ret.y = (1.0f - t) * targetQ0.y + t * q1.y;
			ret.z = (1.0f - t) * targetQ0.z + t * q1.z;
			ret.w = (1.0f - t) * targetQ0.w + t * q1.w;

			// 念のため正規化（長さを1にする）して返す
			float length = std::sqrtf(ret.x * ret.x + ret.y * ret.y + ret.z * ret.z + ret.w * ret.w);
			if (length > 0.0f)
			{
				ret.x /= length; ret.y /= length; ret.z /= length; ret.w /= length;
			}
			return ret;
		}

		// --- ここから下は dot <= 0.9995f の時だけ実行されるため、安全が保証されます ---
		float theta = std::acosf(dot);

		float scale0 = std::sinf((1.0f - t) * theta) / std::sinf(theta);
		float scale1 = std::sinf(t * theta) / std::sinf(theta);

		Quaternion ret{};
		ret.x = scale0 * targetQ0.x + scale1 * q1.x;
		ret.y = scale0 * targetQ0.y + scale1 * q1.y;
		ret.z = scale0 * targetQ0.z + scale1 * q1.z;
		ret.w = scale0 * targetQ0.w + scale1 * q1.w;

		return ret;
	}

	Quaternion MakeQuaternionAxisAngle(const Vector3& axis, float radian)
	{
		Vector3 normAxis = Normalize(axis);
		float sinHalf = std::sinf(radian / 2.0f);
		float cosHalf = std::cosf(radian / 2.0f);

		Quaternion ret;
		ret.x = normAxis.x * sinHalf;
		ret.y = normAxis.y * sinHalf;
		ret.z = normAxis.z * sinHalf;
		ret.w = cosHalf;
		return ret;
	}

	Quaternion FromToRotation(const Vector3& from, const Vector3& to)
	{
		Vector3 f = Normalize(from);
		Vector3 t = Normalize(to);

		float dot = Dot(f, t);

		// ほぼ同じ方向を向いている場合
		if (dot > 0.9999f)
		{
			return Quaternion{ 0.0f, 0.0f, 0.0f, 1.0f };
		}

		// 完全に真逆（180度）を向いている場合
		if (dot < -0.9999f)
		{
			// Y軸に平行ならX軸を、それ以外ならY軸を基準に外積用の補助軸にする
			Vector3 axis = std::abs(f.y) < 0.9f ? Vector3{ 0.0f, 1.0f, 0.0f } : Vector3{ 1.0f, 0.0f, 0.0f };
			Vector3 rotateAxis = Normalize(Cross(f, axis));
			return MakeQuaternionAxisAngle(rotateAxis, float(M_PI));
		}

		// 通常時の回転クォータニオンの算出
		Vector3 cross = Cross(f, t);

		Quaternion q;
		q.x = cross.x;
		q.y = cross.y;
		q.z = cross.z;
		q.w = dot + std::sqrtf(LengthSquared(f) * LengthSquared(t));

		return QuaternionNormalize(q);
	}

	Vector3 Normalize(const Vector3& v)
	{
		Vector3 ret;
		float length = Length(v);

		if (length != 0)
		{
			ret.x = v.x / length;
			ret.y = v.y / length;
			ret.z = v.z / length;
		}
		else
		{
			ret.x = 0;
			ret.y = 0;
			ret.z = 0;
		}

		return ret;
	}

	Quaternion QuaternionNormalize(const Quaternion& q)
	{
		float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
		if (len == 0.0f) return Quaternion{ 0.0f, 0.0f, 0.0f, 1.0f };

		Quaternion ret;
		ret.x = q.x / len;
		ret.y = q.y / len;
		ret.z = q.z / len;
		ret.w = q.w / len;
		return ret;
	}

}