#pragma once
#include "BaseEnemy.h"
#include "Object3d.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Enemy : public BaseEnemy
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath) override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 衝突応答
	void OnCollision(std::string hitObjectType, BaseCharacter* hitObject) override;

	void TakeDamage(int damage);

	// getter
	Vector3 GetTranslate() { return transform_.translate; }
	// HP加算関数
	void AddHP(const float& hp) override;
	// 最大HPを設定
	void SetMaxHP(const float& hp) override;
	// ダメージを受ける
	void TakeDamage(int damage) override;
	// 爆発との当たり判定に使用するAABBを取得
	AABB GetDamageAabb() const override;

private:

};

