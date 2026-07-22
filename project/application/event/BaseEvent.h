#pragma once
#include "MathManager.h"
using namespace MathManager;

class BaseEvent
{
public:
	// 初期化
	virtual void Initialize(QuaternionTransform transform) = 0;
	// 更新
	virtual void Update() = 0;
	// 描画
	virtual void Draw() = 0;
	// 終了
	virtual void Finalize() = 0;

	// 仮想デストラクタ
	virtual ~BaseEvent() = default;

private:
};

