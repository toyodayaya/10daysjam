#include "RailCameraController.h"
#include "DebugDrawCommon.h"

void RailCameraController::Initialize(const QuaternionTransform& transform)
{
	// 引数で受け取ってメンバ変数として記録
	transform_ = transform;

	// デバッグ描画を初期化
	debugLine_ = std::make_unique<DebugDraw>();
	debugLine_->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kLine);
	// 線を追加
	if (railPoints_.size() >= 2)
	{
		for (uint32_t i = 0; i < railPoints_.size() - 1; i++)
		{
			Vector3 start = { railPoints_[i].x,railPoints_[i].y,railPoints_[i].z };
			Vector3 end = { railPoints_[i + 1].x,railPoints_[i + 1].y,railPoints_[i + 1].z };
			debugLine_->AddLine(start, end);
		}
	}
	
}

void RailCameraController::Update()
{
	// カメラを移動
	//transform_.translate.x += 1.0f;

	// カメラに反映
	camera_->SetTranslate(transform_.translate);

	// デバッグ描画の更新
	debugLine_->UpdateLine();
	
}

void RailCameraController::Draw()
{
	debugLine_->DrawLine();
}
