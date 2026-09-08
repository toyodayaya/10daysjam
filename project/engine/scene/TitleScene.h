#pragma once
#include "BaseScene.h"
#include <vector>
#include "Sprite.h"
#include <memory>

class TitleScene : public BaseScene
{
public:
	// 初期化
	void Initialize() override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;

private:
	std::vector<int> ranking_;
	std::unique_ptr<Sprite> backGround_;
	std::unique_ptr<Sprite> play_;
	std::unique_ptr<Sprite> tutorial_;
	std::unique_ptr<Sprite> cursor_;

	enum NextScene
	{
		kTutorial,
		kPlay
	};

	NextScene nextScene_ = kTutorial;
};
