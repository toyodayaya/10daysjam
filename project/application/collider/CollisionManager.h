#pragma once
#include <memory>
#include "StageData.h"
#include "MathManager.h"
using namespace MathManager;

class CollisionManager
{
public:
	
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class CollisionManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit CollisionManager(ConstructorKey) {}

	
	
private:
	// デストラクタ
	~CollisionManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;
	// インスタンス
	friend std::default_delete<CollisionManager>;
	static std::unique_ptr<CollisionManager> instance;

	// 全てのコライダーを記録する変数
	std::vector<StageData::ColliderSpawnData> colliders_;

public:
	// 当たり判定の関数
	bool IsCollision(const AABB& aabb, const AABB& aabbHit);
	// getter
	AABB MakeAABB(const Vector3& translate,const Vector3& size);
	std::vector<StageData::ColliderSpawnData> GetColliders() { return colliders_; }
	// setter
	void SetColliders(StageData::ColliderSpawnData collider) { colliders_.push_back(collider); }
	// インスタンス
	static CollisionManager* GetInstance();
};

