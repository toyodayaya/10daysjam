#pragma once
#include <memory>
#include "MathManager.h"
using namespace MathManager;

class Object3d;

class BaseCharacter
{
public:
	// getter
	QuaternionTransform GetTransform() { return transform_; }
	bool IsDead() { return isDead_; }
	Object3d* GetObject3d() { return object3d_.get(); }

	// HP加算関数
	virtual void AddHP(const float& hp) = 0;
	// 最大HPを設定
	virtual void SetMaxHP(const float& hp) = 0;
	
	// 更新
	virtual void Update() = 0;
	// 描画
	virtual void Draw() = 0;
	// 終了
	virtual void Finalize() = 0;

	// オブジェクトの衝突応答
	virtual void OnCollision(std::string hitObjectType,BaseCharacter* hitObject) = 0;

	// 仮想デストラクタ
	virtual ~BaseCharacter() = default;

protected:
	QuaternionTransform transform_;
	// デスフラグ
	bool isDead_ = false;
	// 3dオブジェクト
	std::unique_ptr<Object3d> object3d_;
	// 接触したオブジェクトのタイプ
	std::string hitObjectType_;
	// 接触したオブジェクトのポインタ
	BaseCharacter* hitObject_ = nullptr;
};

