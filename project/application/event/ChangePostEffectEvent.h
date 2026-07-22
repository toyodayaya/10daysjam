#pragma once
#include "BaseEvent.h"
#include "EventManager.h"
#include "DebugDraw.h"

class ChangePostEffectEvent : public BaseEvent
{
public:
	// 初期化
	void Initialize(QuaternionTransform transform) override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;

private:
	std::unique_ptr<DebugDraw> debugDraw;
};

