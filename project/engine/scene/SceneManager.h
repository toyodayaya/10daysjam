#pragma once
#include "BaseScene.h"
#include "SceneFactory.h"
#include <memory>

class SceneManager
{
public:
	// コンストラクタ
	SceneManager() = default;
	// デストラクタ
	~SceneManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	// インスタンス
	static std::unique_ptr<SceneManager> instance;

public:
	// 次シーン予約
	void ChangeScene(const std::string& sceneName);
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();

	// インスタンス
	static SceneManager* GetInstance();

	// シーンファクトリーのセット
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) { sceneFactory_ = std::move(sceneFactory); }

private:
	// 実行中のシーン
	std::unique_ptr<BaseScene> scene_;
	// 次シーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;
	// シーンファクトリー
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
};

