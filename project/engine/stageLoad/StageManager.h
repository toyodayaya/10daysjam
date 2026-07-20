#pragma once
#include <string>
#include <map>
#include "Camera.h"
#include "StageData.h"

class StageManager
{
private:
	// レベルデータ
	std::map<std::string, std::unique_ptr<StageData>> stageDatas;

	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

public:
	static std::unique_ptr<StageManager> instance;

	StageManager() = default;
	~StageManager() = default;
	StageManager(StageManager&) = delete;
	StageManager& operator = (StageManager&) = delete;

	// シングルトンインスタンスの取得
	static StageManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize();

	// Jsonデータ読み込み
	void LoadJsonData(const std::string& directoryPath, const std::string& filePath);
	// Jsonデータ取得
	StageData* FindJsonData(const std::string& filePath);

	// getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }
};