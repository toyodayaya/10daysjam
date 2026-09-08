#pragma once
#include "Skydome.h"
#include "BaseScene.h"
#include <memory>
#include "StageData.h"
#include "Sprite.h"

class GamePlayScene : public BaseScene
{
public:
	// 初期化
	void Initialize() override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;

private:
	// ステージデータ
	StageData* stageData_ = nullptr;
	// skybox
	std::unique_ptr<Skydome> skydome;
	QuaternionTransform skydomeTransform;

	// スプライト
	std::unique_ptr<Sprite> space_;
	std::unique_ptr<Sprite> goTitle_;

};
