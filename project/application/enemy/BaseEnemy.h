#pragma once
#include "Object3d.h"
#include "MathManager.h"
using namespace MathManager;

class BaseEnemy
{
public:
	// 初期化
	virtual void Initialize(QuaternionTransform transform, const std::string& filePath) = 0;
	// 更新
	virtual void Update() = 0;
	// 描画
	virtual void Draw() = 0;
	// 終了
	virtual void Finalize() = 0;

	// 仮想デストラクタ
	virtual ~BaseEnemy() = default;

protected:
	// モデルオブジェクト
	std::unique_ptr<Object3d> object3d_;

};

