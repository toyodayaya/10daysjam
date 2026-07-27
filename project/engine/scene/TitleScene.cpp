#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"
#include "Object3dCommon.h"

void TitleScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/model", "axis.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "terrain.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "fence.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/human", "walk.gltf", Model::AnimationType::kMove);
	
	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	StageManager::GetInstance()->LoadJsonData("resources/stages", "2.json");
	// ステージを設定する
	stageData_ = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData_->CreateStage("1.json");

	// 3Dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel("walk.gltf");
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");

	// パーティクル
	TextureManager::GetInstance()->LoadTexture("resources/sprite/gradationLine.png");
	ParticleManager::GetInstance()->CreateParticleGroup("Cylinder", "resources/sprite/gradationLine.png", ParticleManager::ShapeType::kCylinder);
	ParticleManager::EmitterSphere emitterSphereCylinder;
	emitterSphereCylinder.translate = { 0.0f,0.0f,0.0f };
	emitterSphereCylinder.scale = { 1.0f,1.0f,1.0f };
	emitterSphereCylinder.velocity = { 0.0f,0.0f,0.0f };
	emitterSphereCylinder.color = { 1.0f,1.0f,1.0f,0.5f };
	emitterSphereCylinder.count = 1;
	emitterSphereCylinder.lifeTime = 30.0f;
	emitterSphereCylinder.currentTime = 0.0f;
	emitterSphereCylinder.frequency = 1.0f;
	emitterSphereCylinder.frequencyTime = 0.0f;
	emitterSphereCylinder.type = ParticleManager::MoveType::kNone;
	emitterCylinder = std::make_unique <ParticleEmitter>("Cylinder", emitterSphereCylinder);
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	// アニメーションモデルを更新
	object3d_->Update();

	// ステージを更新
	stageData_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();

	// パーティクルを更新
	emitterCylinder->Update();
	ParticleManager::GetInstance()->Update();
}

void TitleScene::Draw()
{
	// アニメーションモデルを描画
	object3d_->Draw();

	// ステージを描画
	stageData_->Draw();

	// パーティクルの描画
	ParticleManager::GetInstance()->Draw();

}
