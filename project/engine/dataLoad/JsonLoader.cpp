//#include "JsonLoader.h"
//#include "LevelData.h"
//#include <cassert>
//#include <externals/nlohmannJson/json.hpp>
//
//void JsonLoader::LoadJsonFile(const std::string filename)
//{
//	// 連結してフルパスを得る
//	const std::string fullPath = kDefaultBaseDirectory + filename + kExtension;
//
//	// ファイルストリーム
//	std::ifstream file;
//
//	// ファイルを開く
//	file.open(fullPath);
//	// ファイルが開けなかったら停止
//	if (file.fail())
//	{
//		assert(0);
//	}
//
//	// Json文字列から解凍したデータを格納する変数
//	nlohmann::json deserialized;
//
//	// 解凍
//	file >> deserialized;
//
//	// 正しいレベルデータファイルかチェック
//	assert(deserialized.is_object());
//	assert(deserialized.contains("name"));
//	assert(deserialized["name"].is_string());
//
//	// nameを文字列として取得
//	std::string name = deserialized["name"].get<std::string>();
//	// 正しいレベルデータファイルかチェック
//	assert(name.compare("scene") == 0);
//
//	// レベルデータ格納用インスタンスを生成
//	LevelData* levelData = new LevelData();
//
//	// Objectsの全オブジェクトを走査
//	for (nlohmann::json& object : deserialized["object"])
//	{
//		assert(object.contains("type"));
//
//		// 種別を取得
//		std::string type = object["type"].get<std::string>();
//
//		// MESH
//		if (type.compare("MESH") == 0)
//		{
//			// 要素追加
//			levelData->GetObjectData().emplace_back(LevelData::ObjectData{});
//			// 今追加した要素の参照を取得
//			LevelData::ObjectData& objectData = levelData->GetObjectData().back();
//
//			if (object.contains("file_name"))
//			{
//				// ファイル名
//				objectData.fileName = object["file_name"];
//			}
//
//			// トランスフォームのパラメータ読み込み
//			nlohmann::json& transform = object["transform"];
//			// 平行移動データ
//			objectData.translate.x = (float)transform["translate"][0];
//			objectData.translate.z = (float)transform["translate"][2];
//			objectData.translate.y = (float)transform["translate"][1];
//			// 回転角データ
//			objectData.rotate.x = (float)transform["rotate"][0];
//			objectData.rotate.z = (float)transform["rotate"][2];
//			objectData.rotate.y = (float)transform["rotate"][1];
//			// スケーリングデータ
//			objectData.scale.x = (float)transform["scale"][0];
//			objectData.scale.z = (float)transform["scale"][2];
//			objectData.scale.y = (float)transform["scale"][1];
//		}
//
//
//	}
//}
