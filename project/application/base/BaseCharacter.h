#pragma once
#include "MathManager.h"
using namespace MathManager;

class BaseCharacter
{
public:
	// getter
	QuaternionTransform GetTransform() { return transform_; }
	
	// 更新
	virtual void Update() = 0;
	// 描画
	virtual void Draw() = 0;
	// 終了
	virtual void Finalize() = 0;

	// オブジェクトの衝突応答
	virtual void OnCollision() = 0;

	// 仮想デストラクタ
	virtual ~BaseCharacter() = default;

protected:
	QuaternionTransform transform_;
};

