#pragma once

// 構造体の宣言
struct Vector2
{
	float x, y;
};

struct Vector3
{
	float x, y, z;
};

struct Vector4
{
	float x, y, z, w;
};

struct Matrix3x3
{
	float m[3][3];
};

struct Matrix4x4
{
	float m[4][4];
};

struct Quaternion
{
	float x, y, z, w;
};

struct EulerTransform
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct QuaternionTransform
{
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};

struct AABB
{
	Vector3 min;
	Vector3 max;
};



namespace MathManager
{
	// 単位行列の作成
	Matrix4x4 MakeIdentity4x4();

	// 回転行列
	Matrix4x4 MakeRotateXMatrix(float radian);

	Matrix4x4 MakeRotateYMatrix(float radian);

	Matrix4x4 MakeRotateZMatrix(float radian);

	// Quaternionから回転行列を求める
	Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);

	// 拡縮行列
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	// 移動行列
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	// 行列の積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	// アフィン変換行列
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	Matrix4x4 MakeAffineMatrixQuat(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);

	// 透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	// 正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	// 逆行列
	Matrix4x4 Inverse(const Matrix4x4& m);

	// 5.転置行列
	Matrix4x4 Transpose(const Matrix4x4& m);

	// Vector3の足し算
	Vector3 Vector3Add(const Vector3& v1, const Vector3& v2);

	// 減算の関数
	Vector3 Vector3Subtract(const Vector3& v1, const Vector3& v2);

	// 行列の減法
	Matrix4x4 Subtruct(const Matrix4x4& m1, const Matrix4x4& m2);


	// 正規化の関数
	Vector3 Normalize(const Vector3& v);
	// クォータニオンの正規化
	Quaternion QuaternionNormalize(const Quaternion& q);

	// ノルンの関数
	float Length(const Vector3& v);
	float LengthSquared(const Vector3& v);

	// 内積
	float Dot(const Vector3& v1, const Vector3& v2);

	// 外積
	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	// 座標変換
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

	// 線形補間
	Vector3 Lerp(const Vector3& start, const Vector3& end, float t);
	Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);

	// 軸と回転角(ラジアン)からクォータニオンを作成
	Quaternion MakeQuaternionAxisAngle(const Vector3& axis, float radian);
	// 2つのベクトルから回転クォータニオンを作成
	Quaternion FromToRotation(const Vector3& from, const Vector3& to);

}