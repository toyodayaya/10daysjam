
#pragma once
#include "Audio.h"
#include "Sprite.h"
#include "Skybox.h"
#include "Object3d.h"
#include <numbers>
#include "BaseScene.h"
#include "Input.h"
#include "SceneManager.h"
#include <memory>
#include "ParticleEmitter.h"
#include <random>
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
	// 3dオブジェクト
	std::unique_ptr<Object3d> object3d_;

	// パーティクルエミッターのポインタ
	std::unique_ptr <ParticleEmitter> emitterCylinder;

};
