#pragma once
#include <string>
#include <map>
#include "Camera.h"
#include "StageData.h"

class StageManager
{
public:
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class StageManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit StageManager(ConstructorKey) {}

private:
	// レベルデータ
	std::map<std::string, std::unique_ptr<StageData>> stageDatas;

	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

	// インスタンス
	friend std::default_delete<StageManager>;
	static std::unique_ptr<StageManager> instance;

	~StageManager() = default;
	StageManager(StageManager&) = delete;
	StageManager& operator = (StageManager&) = delete;
public:
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