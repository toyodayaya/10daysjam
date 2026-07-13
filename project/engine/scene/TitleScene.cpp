
#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"
#include "SkyboxCommon.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include <random>

void TitleScene::Initialize()
{
	// 音声読み込み
	soundData1 = Audio::GetInstance()->SoundLoadFile("resources/sound/Alarm01.wav");

	// スプライトの初期化
	TextureManager::GetInstance()->LoadTexture("resources/sprite/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/sprite/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	for (uint32_t i = 0; i < 5; ++i)
	{
		/*std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Initialize(SpriteCommon::GetInstance(), "resources/sprite/uvChecker.png");
		sprites.push_back(std::move(sprite));*/
	}

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/model", "plane.obj",Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "axis.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "fence.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "terrain.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/human", "walk.gltf", Model::AnimationType::kMove);
	
	// テクスチャの読み込み
	//TextureManager::GetInstance()->LoadTexture("resources/rostock_laage_airport_4k.dds");

	// 3Dオブジェクトの初期化
	for (uint32_t i = 0; i < 1; ++i)
	{
		std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();
		object3d->Initialize(Object3dCommon::GetInstance());
		object3d->SetModel("walk.gltf");
		object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
		Vector3 pos = object3d->GetTranslate();
		pos.x += (1.0f * (i + 1));
		object3d->SetTranslate(pos);
		object3ds.push_back(std::move(object3d));
	}

	// アニメーションモデルの初期化
	/*animation = std::make_unique<Animation>();
	animation->Initialize(AnimationCommon::GetInstance(),"./resources/human", "walk.gltf");
	animation->SetEnvironmentMapTextureFilePath("resources/human/white.png");*/

	// Skyboxの初期化
	/*skybox = std::make_unique<Skybox>();
	skybox->Initialize(SkyboxCommon::GetInstance(), "resources/model/rostock_laage_airport_4k.dds");*/

	// 音声再生
	//Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), soundData1);


	// パーティクルグループの作成
	ParticleManager::GetInstance()->CreateParticleGroup("Effect", "resources/sprite/circle2.png", ParticleManager::ShapeType::kNormal);
	ParticleManager::GetInstance()->CreateParticleGroup("Cylinder", "resources/sprite/gradationLine.png", ParticleManager::ShapeType::kCylinder);

	//// パーティクルエミッターの宣言
	ParticleManager::EmitterSphere emitterSphere;
	emitterSphere.translate = { 0.0f,0.0f,0.0f };
	emitterSphere.scale = { 1.0f,1.0f,1.0f };
	emitterSphere.velocity = { 0.0f,0.0f,0.0f };
	emitterSphere.color = { 1.0f,1.0f,1.0f,0.5f };
	emitterSphere.count = 10;
	emitterSphere.lifeTime = 30.0f;
	emitterSphere.currentTime = 0.0f;
	emitterSphere.frequency = 1.0f;
	emitterSphere.frequencyTime = 0.0f;
	emitterSphere.type = ParticleManager::MoveType::kDiffusion;
	emitter = std::make_unique <ParticleEmitter>("Effect", emitterSphere);

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
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), soundData1);
	Audio::GetInstance()->SoundUnload(&soundData1);
}

void TitleScene::Update()
{
	// Enterキーを押したら
	if (Input::GetInstance()->TriggerKey(DIK_RETURN))
	{
		//SceneManager::GetInstance()->ChangeScene("GamePlayScene");
	}

	// 3Dモデルの更新処理
	for (const std::unique_ptr<Object3d>& object3d : object3ds)
	{
		object3d->Update();

	}

	// スプライトの更新処理
	for (const std::unique_ptr <Sprite>& sprite : sprites)
	{
		sprite->Update();
	}

	// アニメーションモデルの更新処理
	//animation->Update();

	// Skyboxの更新処理
	//skybox->Update();

	// パーティクルの更新処理
	emitter->Update();
	emitterCylinder->Update(); 
	/*emitterHit->Update();
	emitterRing->Update();*/
	ParticleManager::GetInstance()->Update();

}

void TitleScene::Draw()
{


	// Spriteの描画
	for (const std::unique_ptr <Sprite>& sprite : sprites)
	{
		//sprite->Draw();
	}

	// 3dモデルの描画
	for (const std::unique_ptr <Object3d>& object3d : object3ds)
	{
		//object3d->Draw();
	}

	// アニメーションモデルの描画
	//animation->Draw();

	// Skyboxの描画
	//skybox->Draw();

	// パーティクルの描画
	ParticleManager::GetInstance()->Draw();
}