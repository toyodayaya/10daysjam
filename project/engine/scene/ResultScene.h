#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include <memory>
#include "Audio.h"


class ResultScene : public BaseScene
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
	std::unique_ptr<Sprite> backGround_;
	std::unique_ptr<Sprite> pressSpace_;

	// サウンドデータ
	Audio::SoundData resultBgm_;
	Audio::SoundData dramrollSE_;
	Audio::SoundData noticeSE_;

	bool isPlay_ = false;
};
