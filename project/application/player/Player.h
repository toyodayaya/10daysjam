#pragma once
#include "Object3d.h"
#include "Sprite.h"
#include <memory>
#include "BaseCharacter.h"

class Player : public BaseCharacter
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath, bool isRailcamera);
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 終了
	void Finalize() override;

	// 衝突応答
	void OnCollision(std::string hitObjectType, BaseCharacter* hitObject) override;
private:
	
	// 当たり判定フラグ
	bool isHit_;
	
	// 移動限界
	const float kMoveLimitX_ = 7.0f;
	const float kMoveLimitY_ = 4.0f;
	
	// HP
	int hp_ = 2;
};

