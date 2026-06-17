
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
	soundData1 = Audio::GetInstance()->SoundLoadFile("resources/Alarm01.wav");

	// スプライトの初期化
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/circle2.png");
	TextureManager::GetInstance()->LoadTexture("resources/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	for (uint32_t i = 0; i < 5; ++i)
	{
		/*std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Initialize(SpriteCommon::GetInstance(), "resources/uvChecker.png");
		sprites.push_back(std::move(sprite));*/
	}

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources", "plane.obj");
	ModelManager::GetInstance()->LoadModel("resources", "axis.obj");
	ModelManager::GetInstance()->LoadModel("resources", "fence.obj");
	ModelManager::GetInstance()->LoadModel("resources", "terrain.obj");
	
	// テクスチャの読み込み
	//TextureManager::GetInstance()->LoadTexture("resources/rostock_laage_airport_4k.dds");

	// 3Dオブジェクトの初期化
	for (uint32_t i = 0; i < 1; ++i)
	{
		std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();
		object3d->Initialize(Object3dCommon::GetInstance());
		object3d->SetModel("terrain.obj");
		object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
		Vector3 pos = object3d->GetTranslate();
		pos.x += (1.0f * (i + 1));
		object3d->SetTranslate(pos);
		object3ds.push_back(std::move(object3d));
	}

	// アニメーションモデルの初期化
	animation = std::make_unique<Animation>();
	animation->Initialize(AnimationCommon::GetInstance(),"./resources/human", "walk.gltf");
	animation->SetEnvironmentMapTextureFilePath("resources/human/white.png");

	// Skyboxの初期化
	/*skybox = std::make_unique<Skybox>();
	skybox->Initialize(SkyboxCommon::GetInstance(), "resources/rostock_laage_airport_4k.dds");*/

	// 音声再生
	//Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), soundData1);


	// パーティクルグループの作成
	ParticleManager::GetInstance()->CreateParticleGroup("Effect", "resources/circle2.png", ParticleEmitter::Type::kNormal);
	ParticleManager::GetInstance()->CreateParticleGroup("Ring", "resources/gradationLine.png", ParticleEmitter::Type::kRing);
	ParticleManager::GetInstance()->CreateParticleGroup("Cylinder", "resources/gradationLine.png", ParticleEmitter::Type::kCylinder);

	// パーティクルエミッターの宣言
	EulerTransform transform;
	transform.translate = { 0.0f,0.0f,0.0f };
	transform.rotate = { 0.0f,0.0f,0.0f };
	transform.scale = { 1.0f,1.0f,1.0f };
	Vector3 velocity = { 0.0f,0.0f,0.0f };
	Vector4 color = { 1.0f,1.0f,1.0f,1.0f };
	float lifeTime = 500.0f;
	float currentTime = 0.0f;
	emitter = std::make_unique <ParticleEmitter>("Ring", transform, velocity, color, lifeTime, currentTime, 1.0f, 8);
	emitter->Emit();

	EulerTransform ringTransform;
	ringTransform.translate = { 0.0f,0.0f,0.0f };
	ringTransform.rotate = { 0.0f,0.0f,0.0f };
	ringTransform.scale = { 1.0f,0.5f,1.0f };
	Vector3 ringVelocity = { 0.0f,0.0f,0.0f };
	Vector4 ringColor = { 1.0f,1.0f,1.0f,1.0f };
	float ringLifeTime = 500.0f;
	float ringCurrentTime = 0.0f;
	emitterRing = std::make_unique <ParticleEmitter>("Cylinder", ringTransform, ringVelocity, ringColor, ringLifeTime, ringCurrentTime, 1.0f, 1);
	emitterRing->Emit();
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
		SceneManager::GetInstance()->ChangeScene("GamePlayScene");
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
	animation->Update();

	// Skyboxの更新処理
	//skybox->Update();

	// パーティクルの更新処理
	//emitter->Update();
	//emitterRing->Update();
	//ParticleManager::GetInstance()->Update();

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
	animation->Draw();

	// Skyboxの描画
	//skybox->Draw();

	// パーティクルの描画
	//ParticleManager::GetInstance()->Draw();
}