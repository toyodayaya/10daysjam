
#pragma once
#include "Audio.h"
#include "Sprite.h"
#include "Skybox.h"
#include "Object3d.h"
#include "Animation.h"
#include <numbers>
#include "BaseScene.h"
#include "Input.h"
#include "SceneManager.h"
#include <memory>
#include "ParticleEmitter.h"
#include <random>
#include "Animation.h"

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
	// サウンドデータ
	Audio::SoundData soundData1;

	// スプライト
	std::vector<std::unique_ptr<Sprite>> sprites;
	// スプライト切り替えフラグ
	bool useMonsterBall = true;

	// 3dオブジェクト
	std::vector<std::unique_ptr<Object3d>> object3ds;
	// アニメーションオブジェクト
	std::unique_ptr<Animation> animation;

	// skybox
	std::unique_ptr<Skybox> skybox;

	// パーティクルエミッターのポインタ
	std::unique_ptr <ParticleEmitter> emitter;
	std::unique_ptr <ParticleEmitter> emitterHit;
	std::unique_ptr <ParticleEmitter> emitterRing;
	std::unique_ptr <ParticleEmitter> emitterCylinder;

};
