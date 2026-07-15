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

		// 種別を取得
		std::string type = object["type"].get<std::string>();

		// メッシュの読み込み
		if (type.compare("MESH") == 0)
		{
			// オブジェクトを読み込む
			levelData.objects.push_back(LoadObject(object));
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

	// 再帰呼び出しでツリー構造を生成
	for (const auto& objectData : levelData.objects)
	{
		CreateObject(objectData, nullptr);
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
		std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
		debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
		debugDraw->SetBoxScale(Vector3Add(objectData.size,objectData.scale));
		debugDraw->SetBoxTranslate(Vector3Add(objectData.center,objectData.translate));

		// 親オブジェクトがあればセット
		if (parent)
		{
			debugDraw->SetParent(parent);
		}

		debugBoxs_.push_back(std::move(debugDraw));
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
