#include "StageData.h"
#include "StageManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <externals/nlohmannJson/Json.hpp>
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "EventManager.h"
#include "EnemyManager.h"
#include "CollisionManager.h"
#include "MathManager.h"
#include <numbers>
using namespace MathManager;

void StageData::Initialize(const std::string& directoryPath, const std::string& filePath, StageManager* stageManager)
{
	// 引数として受け取ってメンバ変数に記録
	this->stageManager_ = stageManager;
	// デフォルトカメラをセット
	this->camera_ = stageManager_->GetDefaultCamera();

	// レベルデータ読み込み
	levelData_ = LoadJsonFile(directoryPath, filePath);
}

void StageData::Update()
{
	// 3Dモデルの更新処理
	for (const std::unique_ptr<Object3d>& object3d : object3ds)
	{
		object3d->Update();
	}

	// プレイヤーの更新処理
	for (const std::unique_ptr<Player>& player : players_)
	{
		player->Update();
	}

	// 敵の更新処理
	EnemyManager::GetInstance()->Update();

	// イベントの更新処理
	EventManager::GetInstance()->Update();

	// レールカメラの更新処理
	if (levelData_.railPoints.size() != 0)
	{
		railCamera_->Update();
	}

	// デバッグ更新
	for (const std::unique_ptr<DebugDraw>& debugBox : debugBoxs_)
	{
		debugBox->UpdateBox();
	}


}

void StageData::Draw()
{
	// 3dモデルの描画
	for (const std::unique_ptr <Object3d>& object3d : object3ds)
	{
		object3d->Draw();
	}

	// プレイヤーの描画処理
	for (const std::unique_ptr<Player>& player : players_)
	{
		player->Draw();
	}

	// 敵の描画処理
	EnemyManager::GetInstance()->Draw();

	// イベントの描画処理
	EventManager::GetInstance()->Draw();

	// レールカメラの描画処理
	if (levelData_.railPoints.size() != 0)
	{
		railCamera_->Draw();
	}

	// デバッグ描画
	for (const std::unique_ptr<DebugDraw>& debugBox : debugBoxs_)
	{
		debugBox->DrawBox();
	}
}

void StageData::CheckAllCollision()
{
	// 登録された当たり判定データを走査
	for (auto& colliders : colliders_)
	{
		// 判定用のAABBを作成
		QuaternionTransform transform = colliders.parent->GetTransform();
		transform.translate = Vector3Add(colliders.center, transform.translate);
		AABB colliderAABB = CollisionManager::GetInstance()->MakeAABB(transform.translate, colliders.size);

		for (auto& collidersHit : colliders_)
		{
			// 同一オブジェクトもしくは同一タイプの場合はスキップ
			if (colliders.objectType == collidersHit.objectType)
			{
				continue;
			}

			// 判定用のAABBを作成
			QuaternionTransform transformHit = collidersHit.parent->GetTransform();
			transformHit.translate = Vector3Add(collidersHit.center, transformHit.translate);
			AABB colliderHitAABB = CollisionManager::GetInstance()->MakeAABB(transformHit.translate, collidersHit.size);

			// 判定
			if (CollisionManager::GetInstance()->IsCollision(colliderAABB, colliderHitAABB))
			{
				// オブジェクトのヒット処理
				colliders.parent->OnCollision();
				collidersHit.parent->OnCollision();
			}

		}
	}
}

StageData::LevelData StageData::LoadJsonFile(const std::string& directoryPath, const std::string& fileName)
{
	// レベルデータを格納する変数を宣言
	LevelData levelData;

	// 連結してフルパスを取得
	std::string filePath = directoryPath + "/" + fileName;

	// ファイルストリーム
	std::ifstream file;

	// ファイルを開く
	file.open(filePath);
	// ファイルオープンが失敗したら
	if (file.fail())
	{
		// assertで停止
		assert(0 && "Jsonファイルを開けませんでした");
	}

	// Json文字列から解凍したデータを格納する変数を宣言
	nlohmann::json deserialized;

	// データを解凍して格納
	file >> deserialized;

	// 正しいレベルデータファイルかチェック
	assert(deserialized.is_object());
	assert(deserialized.contains("name"));
	assert(deserialized["name"].is_string());

	// nameを文字列として取得
	std::string name = deserialized["name"].get<std::string>();
	// 正しいレベルデータファイルかチェック
	assert(name.compare("scene") == 0);

	// objectsの全オブジェクトを走査
	for (nlohmann::json& object : deserialized["objects"])
	{
		assert(object.contains("type"));

		// 無効化オプションがあったら
		if (object.contains("disabled_option"))
		{
			// 無効か有効かを判定
			bool disabled = object["disabled_option"].get<bool>();

			if (disabled)
			{
				// 無効ならスキップ
				continue;
			}
		}

		// 種別を取得
		std::string type = object["type"].get<std::string>();

		// メッシュの読み込み
		if (type.compare("MESH") == 0)
		{
			// オブジェクトを読み込む
			levelData.objects.push_back(LoadObject(object));
		}
		else if (type.compare("PlayerSpawn") == 0)
		{
			// プレイヤーの読み込み
			levelData.players.push_back(LoadPlayer(object));
		}
		else if (type.compare("EnemySpawn") == 0)
		{
			// 敵の読み込み
			levelData.enemies.push_back(LoadEnemy(object));
		}
		else if (type.compare("EventSpawn") == 0)
		{
			// イベントデータの読み込み
			levelData.events.push_back(LoadEvent(object));
		}
		else if (type.compare("CAMERA") == 0)
		{
			// カメラデータの読み込み
			levelData.cameraData = LoadCameraData(object);
		}

	}


	// railsの全オブジェクトを走査
	for (nlohmann::json& rails : deserialized["rails"])
	{
		for (nlohmann::json& point : rails["points"])
		{
			// 無効化オプションがあったら
			if (point.contains("disabled_option"))
			{
				// 無効か有効かを判定
				bool disabled = point["disabled_option"].get<bool>();

				if (disabled)
				{
					// 無効ならスキップ
					continue;
				}
			}

			// 種別を取得
			std::string type = point["type"].get<std::string>();

			// 制御点の読み込み
			if (type.compare("RailPoint") == 0)
			{
				// 制御点データを読み込む
				levelData.railPoints.push_back(LoadRailPoint(point));
			}
		}
	}

	// レベルデータを返す
	return levelData;

}

void StageData::CreateStage(const std::string& fileName)
{
	// レベルデータ格納用の変数を宣言
	LevelData levelData;

	// ファイル名からレベルデータを検索して格納
	levelData = StageManager::GetInstance()->FindJsonData(fileName)->levelData_;

	// 再帰呼び出しでオブジェクトのツリー構造を生成
	for (const auto& objectData : levelData.objects)
	{
		CreateObject(objectData, nullptr);
	}

	// プレイヤーを生成
	for (const auto& playerData : levelData.players)
	{
		CreatePlayer(playerData);
	}

	// 敵を生成
	for (const auto& enemyData : levelData.enemies)
	{
		CreateEnemy(enemyData);
	}

	// イベントを生成
	for (const auto& eventData : levelData.events)
	{
		CreateEvents(eventData);
	}

	// カメラデータをセット
	if (&levelData.cameraData)
	{
		SetCameraData(levelData.cameraData);
	}

	// 制御点データをセット
	if(levelData.railPoints.size() != 0)
	{
		SetRailPoint(levelData.railPoints);
	}
}

StageData::ObjectData StageData::LoadObject(nlohmann::json& object)
{
	// データ格納用の変数を宣言
	ObjectData objectData;

	if (object.contains("file_name"))
	{
		// ファイル名を登録
		objectData.filePath = object["file_name"].get<std::string>();
	}

	// トランスフォームのパラメータ読み込み
	nlohmann::json& transform = object["transform"];
	// 平行移動データを格納
	objectData.transform.translate.x = (float)transform["translation"][0];
	objectData.transform.translate.y = (float)transform["translation"][2];
	objectData.transform.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	objectData.transform.rotate.x = -(float)transform["rotation"][0];
	objectData.transform.rotate.y = -(float)transform["rotation"][2];
	objectData.transform.rotate.z = -(float)transform["rotation"][1];
	objectData.transform.rotate.w = (float)transform["rotation"][3];
	// スケーリングデータを格納
	objectData.transform.scale.x = (float)transform["scaling"][0];
	objectData.transform.scale.y = (float)transform["scaling"][2];
	objectData.transform.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (object.contains("collider"))
	{
		nlohmann::json& collider = object["collider"];
		// コライダーデータを読み込む
		objectData.collider = LoadCollider(collider);
	}
	else
	{
		// コライダーフラグを立てる
		objectData.collider.hasCollier = false;
	}

	// 再帰呼び出しでツリー構造を走査
	if (object.contains("children"))
	{
		for (auto& child : object["children"])
		{
			objectData.children.push_back(LoadObject(child));
		}
	}

	return objectData;
}



void StageData::CreateObject(const ObjectData& objectData, Object3d* parent)
{
	// レベルデータからオブジェクトを生成、配置
	std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();
	// オブジェクトの初期化
	object3d->Initialize(Object3dCommon::GetInstance());
	// モデルをセット
	object3d->SetModel(objectData.filePath);
	// 環境マップ用テクスチャデータをセット
	object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	// Transformデータをセット
	object3d->SetTransform(objectData.transform);
	// 現在のオブジェクトを記録
	Object3d* current = object3d.get();

	// コライダーがあれば生成、配置
	if (objectData.collider.hasCollier)
	{
		// コライダーを生成
		//CreateCollider(objectData.transform, objectData.collider,);
	}

	// 親オブジェクトがあればセット
	if (parent)
	{
		object3d->SetParent(parent);
	}


	// オブジェクトデータをセット
	object3ds.push_back(std::move(object3d));

	for (const auto& child : objectData.children)
	{
		CreateObject(child, current);
	}

}

StageData::PlayerSpawnData StageData::LoadPlayer(nlohmann::json& player)
{
	// データ格納用の変数を宣言
	PlayerSpawnData playerSpawnData;

	if (player.contains("file_name"))
	{
		// ファイル名を登録
		playerSpawnData.filePath = player["file_name"].get<std::string>();
	}

	// トランスフォームのパラメータ読み込み
	nlohmann::json& transform = player["transform"];
	// 平行移動データを格納
	playerSpawnData.transform.translate.x = (float)transform["translation"][0];
	playerSpawnData.transform.translate.y = (float)transform["translation"][2];
	playerSpawnData.transform.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	playerSpawnData.transform.rotate.x = -(float)transform["rotation"][0];
	playerSpawnData.transform.rotate.y = -(float)transform["rotation"][2];
	playerSpawnData.transform.rotate.z = -(float)transform["rotation"][1];
	playerSpawnData.transform.rotate.w = (float)transform["rotation"][3];
	// スケーリングデータを格納
	playerSpawnData.transform.scale.x = (float)transform["scaling"][0];
	playerSpawnData.transform.scale.y = (float)transform["scaling"][2];
	playerSpawnData.transform.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (player.contains("collider"))
	{
		playerSpawnData.collider = LoadCollider(player);
	}
	else
	{
		// コライダーフラグを立てる
		playerSpawnData.collider.hasCollier = false;
	}

	return playerSpawnData;
}



void StageData::CreatePlayer(const PlayerSpawnData& playerData)
{
	std::unique_ptr<Player> player = std::make_unique<Player>();
	player->Initialize(playerData.transform, playerData.filePath);

	// コライダーがあれば生成、配置
	if (playerData.collider.hasCollier)
	{
		CreateCollider(playerData.collider, player.get());
	}

	players_.push_back(std::move(player));
}

StageData::EnemySpawnData StageData::LoadEnemy(nlohmann::json& enemy)
{
	// データ格納用の変数を宣言
	EnemySpawnData enemySpawnData;

	if (enemy.contains("file_name"))
	{
		// ファイル名を登録
		enemySpawnData.filePath = enemy["file_name"].get<std::string>();
	}

	// トランスフォームのパラメータ読み込み
	nlohmann::json& transform = enemy["transform"];
	// 平行移動データを格納
	enemySpawnData.transform.translate.x = (float)transform["translation"][0];
	enemySpawnData.transform.translate.y = (float)transform["translation"][2];
	enemySpawnData.transform.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	enemySpawnData.transform.rotate.x = -(float)transform["rotation"][0];
	enemySpawnData.transform.rotate.y = -(float)transform["rotation"][2];
	enemySpawnData.transform.rotate.z = -(float)transform["rotation"][1];
	enemySpawnData.transform.rotate.w = (float)transform["rotation"][3];
	// スケーリングデータを格納
	enemySpawnData.transform.scale.x = (float)transform["scaling"][0];
	enemySpawnData.transform.scale.y = (float)transform["scaling"][2];
	enemySpawnData.transform.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (enemy.contains("collider"))
	{
		// コライダーデータを読み込む
		enemySpawnData.collider = LoadCollider(enemy);
	}
	else
	{
		// コライダーフラグを立てる
		enemySpawnData.collider.hasCollier = false;
	}

	return enemySpawnData;
}

void StageData::CreateEnemy(const EnemySpawnData& enemyData)
{
	std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
	enemy->Initialize(enemyData.transform, enemyData.filePath);

	// コライダーがあれば生成、配置
	if (enemyData.collider.hasCollier)
	{
		CreateCollider(enemyData.collider, enemy.get());
	}

	EnemyManager::GetInstance()->SetEnemies(std::move(enemy));
}

StageData::ColliderSpawnData StageData::LoadCollider(nlohmann::json& collider)
{
	// データ格納用の変数を宣言
	ColliderSpawnData colliderData;

	// オブジェクトタイプを記録
	colliderData.objectType = collider["type"].get<std::string>();

	nlohmann::json& colliders = collider["collider"];

	// コライダーフラグを立てる
	colliderData.hasCollier = true;
	// 中心点のデータを格納
	colliderData.center.x = (float)colliders["center"][0];
	colliderData.center.y = (float)colliders["center"][2];
	colliderData.center.z = (float)colliders["center"][1];
	// サイズデータを格納
	colliderData.size.x = (float)(colliders["size"][0]);
	colliderData.size.y = (float)(colliders["size"][2]);
	colliderData.size.z = (float)(colliders["size"][1]);

	// 配列に格納
	levelData_.colliders.push_back(colliderData);

	return colliderData;
}

void StageData::CreateCollider(const ColliderSpawnData& collider, BaseCharacter* parent)
{
	// デバッグ描画用の箱を初期化、生成
	std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
	debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
	debugDraw->SetBoxScale(collider.size);
	debugDraw->SetBoxTranslate(collider.center);

	// 親オブジェクトがあればセット
	debugDraw->SetParent(parent);
	ColliderSpawnData colliders = collider;
	colliders.parent = parent;

	// 登録
	debugBoxs_.push_back(std::move(debugDraw));
	colliders_.push_back(colliders);

}

StageData::EventSpawnData StageData::LoadEvent(nlohmann::json& event)
{
	// データ格納用の変数を宣言
	EventSpawnData eventSpawnData;

	// トランスフォームのパラメータ読み込み
	nlohmann::json& transform = event["transform"];
	// 平行移動データを格納
	eventSpawnData.transform.translate.x = (float)transform["translation"][0];
	eventSpawnData.transform.translate.y = (float)transform["translation"][2];
	eventSpawnData.transform.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	eventSpawnData.transform.rotate.x = -(float)transform["rotation"][0];
	eventSpawnData.transform.rotate.y = -(float)transform["rotation"][2];
	eventSpawnData.transform.rotate.z = -(float)transform["rotation"][1];
	eventSpawnData.transform.rotate.w = (float)transform["rotation"][3];
	// スケーリングデータを格納
	eventSpawnData.transform.scale.x = (float)transform["scaling"][0];
	eventSpawnData.transform.scale.y = (float)transform["scaling"][2];
	eventSpawnData.transform.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (event.contains("collider"))
	{
		// コライダーデータを読み込む
		eventSpawnData.collider = LoadCollider(event);
	}
	else
	{
		// コライダーフラグを立てる
		eventSpawnData.collider.hasCollier = false;
	}

	return eventSpawnData;
}

void StageData::CreateEvents(const EventSpawnData& eventData)
{
	std::unique_ptr<ChangePostEffectEvent> event = std::make_unique<ChangePostEffectEvent>();
	event->Initialize(eventData.transform);

	// コライダーがあれば生成、配置
	if (eventData.collider.hasCollier)
	{
		CreateCollider(eventData.collider, event.get());
	}

	EventManager::GetInstance()->SetEvents(std::move(event));
}

StageData::CameraData StageData::LoadCameraData(nlohmann::json& camera)
{
	// データ格納用の変数を宣言
	CameraData cameraData;

	// トランスフォームのパラメータ読み込み
	nlohmann::json& transform = camera["transform"];
	// 平行移動データを格納
	cameraData.transform.translate.x = (float)transform["translation"][0];
	cameraData.transform.translate.y = (float)transform["translation"][2];
	cameraData.transform.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	cameraData.transform.rotate.x = (float)transform["rotation"][0];
	cameraData.transform.rotate.y = (float)transform["rotation"][2];
	cameraData.transform.rotate.z = (float)transform["rotation"][1];
	cameraData.transform.rotate.w = (float)transform["rotation"][3];
	// スケーリングデータを格納
	cameraData.transform.scale.x = (float)transform["scaling"][0];
	cameraData.transform.scale.y = (float)transform["scaling"][2];
	cameraData.transform.scale.z = (float)transform["scaling"][1];

	return cameraData;
}

void StageData::SetCameraData(CameraData& cameraData)
{
	// カメラの回転角を補正
	Quaternion correction = MakeRotateXQuaternion(-std::numbers::pi_v<float> / 2.0f);
	cameraData.transform.rotate = QuaternionNormalize(QuaternionMultiply(correction, cameraData.transform.rotate));
	cameraData.transform.rotate.w *= -1.0f;

	// カメラデータをセット
	camera_->SetTransform(cameraData.transform);
}

StageData::RailPointData StageData::LoadRailPoint(nlohmann::json& railPoint)
{
	// データ格納用の変数を宣言
	RailPointData railPointData;

	// トランスフォームのパラメータ読み込み
	nlohmann::json& transform = railPoint["transform"];
	// 平行移動データを格納
	railPointData.translate.x = (float)transform["translation"][0];
	railPointData.translate.y = (float)transform["translation"][2];
	railPointData.translate.z = (float)transform["translation"][1];

	return railPointData;
}

void StageData::SetRailPoint(const std::vector<RailPointData>& railPointData)
{
	// レールカメラを初期化
	railCamera_ = std::make_unique<RailCameraController>();
	
	// レールカメラをセット
	railCamera_->SetCamera(camera_);

	for (const auto& railPoint : railPointData)
	{
		// 制御点データをセット
		railPoints_.push_back(railPoint.translate);
	}

	// 制御点をセットし初期化
	railCamera_->SetRailPoint(railPoints_);
	railCamera_->Initialize(camera_->GetTransform());

}
