#pragma once
#include "Skybox.h"
#include "BaseScene.h"
#include <memory>
#include "StageData.h"

class TitleScene : public BaseScene
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
	std::unique_ptr<Skybox> skybox;

};
