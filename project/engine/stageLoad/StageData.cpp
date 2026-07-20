#include "StageData.h"
#include "StageManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <externals/nlohmannJson/Json.hpp>
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "MathManager.h"
using namespace MathManager;

std::unique_ptr<StageData> StageData::instance = nullptr;

StageData* StageData::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<StageData>();
	}

	return instance.get();
}


void StageData::Initialize(const std::string& directoryPath, const std::string& filePath)
{
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
	for (const std::unique_ptr<Enemy>& enemy : enemies_)
	{
		enemy->Update();
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
	for (const std::unique_ptr<Enemy>& enemy : enemies_)
	{
		enemy->Draw();
	}


	// デバッグ描画
	for (const std::unique_ptr<DebugDraw>& debugBox : debugBoxs_)
	{
		debugBox->DrawBox();
	}
}

void StageData::Finalize()
{
	instance.reset();
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
			// プレイヤーの読み込み
			levelData.enemies.push_back(LoadEnemy(object));
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
	objectData.translate.x = (float)transform["translation"][0];
	objectData.translate.y = (float)transform["translation"][2];
	objectData.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	objectData.rotate.x = -(float)transform["rotation"][0];
	objectData.rotate.y = -(float)transform["rotation"][2];
	objectData.rotate.z = -(float)transform["rotation"][1];
	objectData.rotate.w = 1.0f;
	// スケーリングデータを格納
	objectData.scale.x = (float)transform["scaling"][0];
	objectData.scale.y = (float)transform["scaling"][2];
	objectData.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (object.contains("collider"))
	{
		nlohmann::json& collider = object["collider"];
		// コライダーフラグを立てる
		objectData.hasCollier = true;
		// 中心点のデータを格納
		objectData.center.x = (float)collider["center"][0];
		objectData.center.y = (float)collider["center"][2];
		objectData.center.z = (float)collider["center"][1];
		// サイズデータを格納
		objectData.size.x = (float)(collider["size"][0] / 2);
		objectData.size.y = (float)(collider["size"][2] / 2);
		objectData.size.z = (float)(collider["size"][1] / 2);
	}
	else
	{
		// コライダーフラグを立てる
		objectData.hasCollier = false;
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
	playerSpawnData.translate.x = (float)transform["translation"][0];
	playerSpawnData.translate.y = (float)transform["translation"][2];
	playerSpawnData.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	playerSpawnData.rotate.x = -(float)transform["rotation"][0];
	playerSpawnData.rotate.y = -(float)transform["rotation"][2];
	playerSpawnData.rotate.z = -(float)transform["rotation"][1];
	playerSpawnData.rotate.w = 1.0f;
	// スケーリングデータを格納
	playerSpawnData.scale.x = (float)transform["scaling"][0];
	playerSpawnData.scale.y = (float)transform["scaling"][2];
	playerSpawnData.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (player.contains("collider"))
	{
		nlohmann::json& collider = player["collider"];
		// コライダーフラグを立てる
		playerSpawnData.hasCollier = true;
		// 中心点のデータを格納
		playerSpawnData.center.x = (float)collider["center"][0];
		playerSpawnData.center.y = (float)collider["center"][2];
		playerSpawnData.center.z = (float)collider["center"][1];
		// サイズデータを格納
		playerSpawnData.size.x = (float)(collider["size"][0] / 2);
		playerSpawnData.size.y = (float)(collider["size"][2] / 2);
		playerSpawnData.size.z = (float)(collider["size"][1] / 2);
	}
	else
	{
		// コライダーフラグを立てる
		playerSpawnData.hasCollier = false;
	}

	return playerSpawnData;
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
	// 座標データをセット
	object3d->SetTranslate(objectData.translate);
	// 回転角データをセット
	object3d->SetRotate(objectData.rotate);
	// スケーリングデータをセット
	object3d->SetScale(objectData.scale);
	// 現在のオブジェクトを記録
	Object3d* current = object3d.get();

	// コライダーがあれば生成、配置
	if (objectData.hasCollier)
	{
		CreateCollider(objectData.size, objectData.translate, objectData.scale, objectData.center, object3d.get());
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


void StageData::CreatePlayer(const PlayerSpawnData& playerData)
{
	std::unique_ptr<Player> player = std::make_unique<Player>();
	player->Initialize();
	player->GetObject3d()->SetModel(playerData.filePath);
	player->GetObject3d()->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	player->GetObject3d()->SetScale(playerData.scale);
	player->GetObject3d()->SetTranslate(playerData.translate);
	player->GetObject3d()->SetRotate(playerData.rotate);

	// コライダーがあれば生成、配置
	if (playerData.hasCollier)
	{
		CreateCollider(playerData.size, playerData.translate, playerData.scale, playerData.center, player->GetObject3d());
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
	enemySpawnData.translate.x = (float)transform["translation"][0];
	enemySpawnData.translate.y = (float)transform["translation"][2];
	enemySpawnData.translate.z = (float)transform["translation"][1];
	// 回転角データを格納
	enemySpawnData.rotate.x = -(float)transform["rotation"][0];
	enemySpawnData.rotate.y = -(float)transform["rotation"][2];
	enemySpawnData.rotate.z = -(float)transform["rotation"][1];
	enemySpawnData.rotate.w = 1.0f;
	// スケーリングデータを格納
	enemySpawnData.scale.x = (float)transform["scaling"][0];
	enemySpawnData.scale.y = (float)transform["scaling"][2];
	enemySpawnData.scale.z = (float)transform["scaling"][1];

	// コライダーのパラメータ読み込み
	if (enemy.contains("collider"))
	{
		nlohmann::json& collider = enemy["collider"];
		// コライダーフラグを立てる
		enemySpawnData.hasCollier = true;
		// 中心点のデータを格納
		enemySpawnData.center.x = (float)collider["center"][0];
		enemySpawnData.center.y = (float)collider["center"][2];
		enemySpawnData.center.z = (float)collider["center"][1];
		// サイズデータを格納
		enemySpawnData.size.x = (float)(collider["size"][0] / 2);
		enemySpawnData.size.y = (float)(collider["size"][2] / 2);
		enemySpawnData.size.z = (float)(collider["size"][1] / 2);
	}
	else
	{
		// コライダーフラグを立てる
		enemySpawnData.hasCollier = false;
	}

	return enemySpawnData;
}

void StageData::CreateEnemy(const EnemySpawnData& enemyData)
{
	std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
	enemy->Initialize();
	enemy->GetObject3d()->SetModel(enemyData.filePath);
	enemy->GetObject3d()->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	enemy->GetObject3d()->SetScale(enemyData.scale);
	enemy->GetObject3d()->SetTranslate(enemyData.translate);
	enemy->GetObject3d()->SetRotate(enemyData.rotate);

	// コライダーがあれば生成、配置
	if (enemyData.hasCollier)
	{
		CreateCollider(enemyData.size, enemyData.translate, enemyData.scale, enemyData.center, enemy->GetObject3d());
	}

	enemies_.push_back(std::move(enemy));
}

void StageData::CreateCollider(const Vector3& size, const Vector3 translate, const Vector3& scale, const Vector3& center, 
	Object3d* parent)
{
	std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
	debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
	debugDraw->SetBoxScale(Vector3Add(size, scale));
	debugDraw->SetBoxTranslate(Vector3Add(center, translate));

	// 親オブジェクトがあればセット
	if (parent)
	{
		debugDraw->SetParent(parent);
	}

	debugBoxs_.push_back(std::move(debugDraw));
}
