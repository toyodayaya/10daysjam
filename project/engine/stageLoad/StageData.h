#pragma once
#include "MathManager.h"
#include "DirectXBasis.h"
#include "Object3d.h"
#include <string>
#include <vector>
#include <externals/nlohmannJson/Json.hpp>
#include "DebugDraw.h"
#include "DebugDrawCommon.h"

class StageData
{
public:
	struct ObjectData
	{
		Vector3 translate;
		Quaternion rotate;
		Vector3 scale;
		std::string filePath;
		std::vector<ObjectData> children;
		Vector3 center;
		Vector3 size;
		int32_t hasCollier;
	};

	struct LevelData
	{
		std::vector<ObjectData> objects;
	};

	static std::unique_ptr<StageData> instance;

	StageData() = default;
	~StageData() = default;
	StageData(StageData&) = delete;
	StageData& operator = (StageData&) = delete;

public:
	// シングルトンインスタンスの取得
	static StageData* GetInstance();

	// 初期化
	void Initialize(const std::string& directoryPath, const std::string& filePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 終了
	void Finalize();

	// Jsonファイル読み込み
	LevelData LoadJsonFile(const std::string& directoryPath, const std::string& fileName);

	// Jsonファイルからオブジェクトを配置する関数
	void CreateStage(const std::string& fileName);

	// オブジェクト読み込みの再帰関数
	ObjectData LoadObject(nlohmann::json& object);
	// オブジェクト生成の再帰関数
	void CreateObject(const ObjectData& objectData, Object3d* parent);
private:
	// jsonファイルのデータ
	LevelData levelData_;
	// オブジェクトデータ
	std::vector<std::unique_ptr<Object3d>> object3ds;
	// デバッグ描画データ
	std::vector<std::unique_ptr<DebugDraw>> debugBoxs_;
};