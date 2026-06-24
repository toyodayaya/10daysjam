
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
	TextureManager::GetInstance()->LoadTexture("resources/sprite/circle2.png");
	TextureManager::GetInstance()->LoadTexture("resources/sprite/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	for (uint32_t i = 0; i < 5; ++i)
	{
		/*std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Initialize(SpriteCommon::GetInstance(), "resources/sprite/uvChecker.png");
		sprites.push_back(std::move(sprite));*/
	}

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/model", "plane.obj");
	ModelManager::GetInstance()->LoadModel("resources/model", "axis.obj");
	ModelManager::GetInstance()->LoadModel("resources/model", "fence.obj");
	ModelManager::GetInstance()->LoadModel("resources/model", "terrain.obj");
	
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
	/*animation = std::make_unique<Animation>();
	animation->Initialize(AnimationCommon::GetInstance(),"./resources/human", "walk.gltf");
	animation->SetEnvironmentMapTextureFilePath("resources/human/white.png");*/

	// Skyboxの初期化
	/*skybox = std::make_unique<Skybox>();
	skybox->Initialize(SkyboxCommon::GetInstance(), "resources/model/rostock_laage_airport_4k.dds");*/

	// 音声再生
	//Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), soundData1);


	// パーティクルグループの作成
	ParticleManager::GetInstance()->CreateParticleGroup("Effect", "resources/sprite/circle2.png", ParticleEmitter::Type::kNormal);
	ParticleManager::GetInstance()->CreateParticleGroup("HitEffect", "resources/sprite/circle2.png", ParticleEmitter::Type::kHitEffect);
	ParticleManager::GetInstance()->CreateParticleGroup("Ring", "resources/sprite/gradationLine.png", ParticleEmitter::Type::kRing);
	ParticleManager::GetInstance()->CreateParticleGroup("Cylinder", "resources/sprite/gradationLine.png", ParticleEmitter::Type::kCylinder);

	// パーティクルエミッターの宣言
	EulerTransform transform;
	transform.translate = { 0.0f,0.0f,0.0f };
	transform.rotate = { 0.0f,0.0f,0.0f };
	transform.scale = { 1.0f,1.0f,1.0f };
	Vector3 velocity = { 0.0f,0.0f,0.0f };
	Vector4 color = { 1.0f,1.0f,1.0f,0.5f };
	float lifeTime = 3.0f;
	float currentTime = 0.0f;
	emitter = std::make_unique <ParticleEmitter>("Effect", transform, velocity, color, lifeTime, currentTime, 5.6f, 8);


	EulerTransform hitTransform;
	hitTransform.translate = { 0.0f,0.0f,0.0f };
	hitTransform.rotate = { 0.0f,0.0f,0.0f };
	hitTransform.scale = { 0.05f,1.0f,1.0f };
	Vector3 hitVelocity = { 0.0f,0.0f,0.0f };
	Vector4 hitColor = { 1.0f,1.0f,1.0f,1.0f };
	float hitLifeTime = 3.0f;
	float hitCurrentTime = 0.0f;
	emitterHit = std::make_unique <ParticleEmitter>("HitEffect", hitTransform, hitVelocity, hitColor, hitLifeTime, hitCurrentTime, 5.5f, 8);

	EulerTransform ringTransform;
	ringTransform.translate = { 0.0f,0.0f,0.0f };
	ringTransform.rotate = { 0.0f,0.0f,0.0f };
	ringTransform.scale = { 1.0f,1.0f,1.0f };
	Vector3 ringVelocity = { 0.0f,0.0f,0.0f };
	Vector4 ringColor = { 1.0f,1.0f,1.0f,1.0f };
	float ringLifeTime = 5.5f;
	float ringCurrentTime = 0.0f;
	emitterRing = std::make_unique <ParticleEmitter>("Ring", ringTransform, ringVelocity, ringColor, ringLifeTime, ringCurrentTime, 4.5f, 1);

	EulerTransform cylinderTransform;
	cylinderTransform.translate = { 0.0f,-1.0f,0.0f };
	cylinderTransform.rotate = { 0.0f,0.0f,0.0f };
	cylinderTransform.scale = { 1.0f,0.5f,1.0f };
	Vector3 cylinderVelocity = { 0.0f,0.0f,0.0f };
	Vector4 cylinderColor = { 1.0f,1.0f,1.0f,1.0f };
	float cylinderLifeTime = 6.5f;
	float cylinderCurrentTime = 0.0f;
	emitterCylinder = std::make_unique <ParticleEmitter>("Cylinder", cylinderTransform, cylinderVelocity, cylinderColor, cylinderLifeTime, cylinderCurrentTime, 4.0f, 1);
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
	/*emitter->Update();
	emitterHit->Update();
	emitterRing->Update();
	emitterCylinder->Update();
	ParticleManager::GetInstance()->Update();*/

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
		object3d->Draw();
	}

	// アニメーションモデルの描画
	//animation->Draw();

	// Skyboxの描画
	//skybox->Draw();

	// パーティクルの描画
	ParticleManager::GetInstance()->Draw();
}