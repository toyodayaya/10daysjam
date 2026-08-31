#pragma once
#include "MathManager.h"
#include "DirectXBasis.h"
#include "Object3d.h"
#include "Player.h"
#include "Enemy.h"
#include "ChangePostEffectEvent.h"
#include <string>
#include <vector>
#include <externals/nlohmannJson/Json.hpp>
#ifdef _DEBUG
#include "DebugDraw.h"
#include "DebugDrawCommon.h"
#endif // _DEBUG
#include "BaseCharacter.h"

class StageManager;

class StageData
{
public:

	// コライダーの生成データ
	struct ColliderSpawnData
	{
		Vector3 center;
		Vector3 size;
		int32_t hasCollier;
		std::string objectType;
		BaseCharacter* parent;
	};

	// オブジェクトの生成データ
	struct ObjectData
	{
		QuaternionTransform transform;
		std::string filePath;
		std::vector<ObjectData> children;
		ColliderSpawnData collider;
	};

	// プレイヤーの生成データ
	struct PlayerSpawnData
	{
		QuaternionTransform transform;
		std::string filePath;
		ColliderSpawnData collider;
		bool hasBullet;
	};

	// 敵の生成データ
	struct EnemySpawnData
	{
		QuaternionTransform transform;
		std::string filePath;
		ColliderSpawnData collider;
	};


	// イベントの生成データ
	struct EventSpawnData
	{
		QuaternionTransform transform;
		ColliderSpawnData collider;
	};

	// カメラデータ
	struct CameraData
	{
		QuaternionTransform transform;
	};

	// レベルデータ
	struct LevelData
	{
		std::vector<ObjectData> objects;
		std::vector<PlayerSpawnData> players;
		std::vector<EnemySpawnData> enemies;
		std::vector<EventSpawnData> events;
		std::vector<ColliderSpawnData> colliders;
		CameraData cameraData;
	};


public:

	// 初期化
	void Initialize(const std::string& directoryPath, const std::string& filePath, StageManager* stageManager);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 全ての当たり判定を走査
	void CheckAllCollision();

	// 終了
	void Finalize();

	// ステージをクリア
	void ClearStage();

	// Jsonファイル読み込み
	LevelData LoadJsonFile(const std::string& directoryPath, const std::string& fileName);

	// Jsonファイルからオブジェクトを配置する関数
	void CreateStage(const std::string& fileName);

	// オブジェクト読み込みの再帰関数
	ObjectData LoadObject(nlohmann::json& object);
	// オブジェクト生成の再帰関数
	void CreateObject(const ObjectData& objectData, Object3d* parent);
	// プレイヤー読み込みの関数
	PlayerSpawnData LoadPlayer(nlohmann::json& player);
	// プレイヤー生成の関数
	void CreatePlayer(const PlayerSpawnData& playerData);
	// 敵読み込みの関数
	EnemySpawnData LoadEnemy(nlohmann::json& enemy);
	// 敵生成の関数
	void CreateEnemy(const EnemySpawnData& enemyData);
	// コライダー読み込みの関数
	ColliderSpawnData LoadCollider(nlohmann::json& collider);
	// コライダー生成の関数
	void CreateCollider(const ColliderSpawnData& colliderData, BaseCharacter* parent);
	// イベントデータ読み込みの関数
	EventSpawnData LoadEvent(nlohmann::json& event);
	// イベント生成の関数
	void CreateEvents(const EventSpawnData& eventData);
	// カメラデータ読み込みの関数
	CameraData LoadCameraData(nlohmann::json& camera);
	// カメラデータセットの関数
	void SetCameraData(CameraData& cameraData);
	
private:
	// jsonファイルのデータ
	LevelData levelData_;
	// オブジェクトデータ
	std::vector<std::unique_ptr<Object3d>> object3ds;
#ifdef _DEBUG
	// デバッグ描画データ
	std::vector<std::unique_ptr<DebugDraw>> debugBoxs_;
#endif // _DEBUG
	// プレイヤーデータ
	std::vector<std::unique_ptr<Player>> players_;
	// デフォルトカメラ
	Camera* camera_ = nullptr;
	// ポインタ
	StageManager* stageManager_ = nullptr;
};