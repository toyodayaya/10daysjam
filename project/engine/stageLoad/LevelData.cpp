//#include "LevelData.h"
//#include <fstream>
//#include <sstream>
//#include <cassert>
//#include <externals/nlohmannJson/Json.hpp>
//
//void LevelData::Initialize(const std::string& directoryPath, const std::string& filePath)
//{
//    // レベルデータ読み込み
//    levelData_ = LoadJsonFile(directoryPath, filePath);
//
//
//}
//
//LevelData::ObjectData LevelData::LoadJsonFile(const std::string& directoryPath, const std::string& fileName)
//{
//    // レベルデータを格納する変数を宣言
//    LevelData levelData;
//
//    // 連結してフルパスを取得
//    std::string filePath = directoryPath + "/" + fileName;
//
//    // ファイルストリーム
//    std::ifstream file;
//
//    // ファイルを開く
//    file.open(filePath);
//    // ファイルオープンが失敗したら
//    if (file.fail())
//    {
//        // assertで停止
//        assert(0 && "Jsonファイルを開けませんでした");
//    }
//
//    // Json文字列から解凍したデータを格納する変数を宣言
//    nlohmann::json deserialized;
//
//    // データを解凍して格納
//    file >> deserialized;
//
//    // 正しいレベルデータファイルかチェック
//    assert(deserialized.is_object());
//    assert(deserialized.contains("name"));
//    assert(deserialized["name"].is_string());
//
//    // nameを文字列として取得
//    std::string name = deserialized["name"].get<std::string>();
//    // 正しいレベルデータファイルかチェック
//    assert(name.compare("scene") == 0);
//
//    // objectsの全オブジェクトを走査
//    if (nlohmann::json& object : deserialized["objects"])
//    {
//        assert(object.contains("type"));
//
//        // 種別を取得
//        std::string type = object["type"].get<std::string>();
//
//        // メッシュの読み込み
//        if (type.compare("MESH") == 0)
//        {
//            // 要素を追加
//
//        }
//    }
//
//}
