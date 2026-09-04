#pragma once
#include "Skydome.h"
#include "BaseScene.h"
#include <memory>
#include "StageData.h"

class TutorialScene : public BaseScene
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

	// チュートリアルの段階分け
	enum Phase
	{
		kLighting, // 灯台を点灯させるフェーズ
		kRespawn, // 自爆してリスポーンするフェーズ
		kDefeat, // 敵を倒すフェーズ
		kNextScene // 次のシーンへ移行するフェーズ
	};

private:
	// ステージデータ
	StageData* stageData_ = nullptr;
	// skybox
	std::unique_ptr<Skydome> skydome;
	QuaternionTransform skydomeTransform;
	// チュートリアルフェーズ
	Phase phase_ = kLighting;

};
