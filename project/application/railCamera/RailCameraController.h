#pragma once
#include <vector>
#include "Camera.h"
#include "DebugDraw.h"
#include "MathManager.h"
using namespace MathManager;

class RailCameraController
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform);
	// 更新
	void Update();
	// 描画
	void Draw();

	// setter
	void SetCamera(Camera* camera) { this->camera_ = camera; }
	void SetRailPoint(const std::vector<Vector3>& railPoint) { railPoints_ = railPoint; }

private:
	// ワールド座標
	QuaternionTransform transform_;
	// カメラ
	Camera* camera_ = nullptr;

	// カメラの制御点
	std::vector<Vector3> railPoints_;

	// デバッグ描画
	std::unique_ptr<DebugDraw> debugLine_;
};

