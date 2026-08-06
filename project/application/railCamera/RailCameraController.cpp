#include "RailCameraController.h"
#ifdef _DEBUG
#include "DebugDrawCommon.h"
#endif // _DEBUG
#include "MathManager.h"
using namespace MathManager;

void RailCameraController::Initialize(const QuaternionTransform& transform)
{
	// 引数で受け取ってメンバ変数として記録
	transform_ = transform;

#ifdef _DEBUG
	// デバッグ描画を初期化
	debugLine_ = std::make_unique<DebugDraw>();
	debugLine_->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kLine);

	// デバッグ描画の初期化
	for (uint32_t i = 0; i < railPoints_.size(); ++i)
	{
		std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
		debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kSphere);
		debugDraw->SetScale(Vector3{ 0.01f,0.01f,0.01f });
		debugDraw->SetTranslate(railPoints_[i]);
		debugPoint_.push_back(std::move(debugDraw));
	}

	// 線分で描画する用の頂点リスト
	std::vector<Vector3> drawPoints;
	// 線分の数
	const size_t segmentCount = 100;
	// 線分の数+1の頂点座標を計算
	for (size_t i = 0; i < segmentCount + 1; i++)
	{
		float t = 1.0f / segmentCount * i;
		Vector3 pos = CatmullRomPosition(railPoints_, t);
		// 描画用の頂点リストに追加
		drawPoints.push_back(pos);
	}

	// 線を追加
	if (drawPoints.size() >= 2)
	{
		for (uint32_t i = 0; i < drawPoints.size() - 1; i++)
		{
			Vector3 start = { drawPoints[i].x,drawPoints[i].y,drawPoints[i].z };
			Vector3 end = { drawPoints[i + 1].x,drawPoints[i + 1].y,drawPoints[i + 1].z };
			debugLine_->AddLine(start, end);
		}
	}
#endif // _DEBUG
}

void RailCameraController::Update()
{
	// カメラの座標を求める
	t += 0.001f;
	Vector3 pos = CatmullRomPosition(railPoints_, t);
	transform_.translate = pos;

	// カメラの角度を求める
	float targetT = t + offset_;
	Vector3 target = CatmullRomPosition(railPoints_, targetT);
	Vector3 forward = Vector3Subtract(target,pos);
	forward = Normalize(forward);
	// Y軸周り角度
	float yaw = std::atan2(forward.x, forward.z);
	transform_.rotate.y = yaw;
	// Y軸周りの回転行列
	Matrix4x4 rotate = MakeRotateYMatrix(yaw);
	Vector3 velocityZ = TransformNormal(forward, rotate);
	// X軸周り角度
	transform_.rotate.x = std::atan2(-velocityZ.y, velocityZ.z);

	// カメラに反映
	camera_->SetTransform(transform_);

#ifdef _DEBUG
	// デバッグ描画の更新
	debugLine_->UpdateLine();

	for (auto& debugDraw : debugPoint_)
	{
		debugDraw->Update();
	}

#endif // _DEBUG
	
}

void RailCameraController::Draw()
{

#ifdef _DEBUG
	debugLine_->DrawLine();

	for (auto& debugDraw : debugPoint_)
	{
		debugDraw->Draw();
	}
#endif // _DEBUG
}
