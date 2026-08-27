#include "CollisionManager.h"

std::unique_ptr<CollisionManager> CollisionManager::instance = nullptr;

CollisionManager* CollisionManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<CollisionManager>(ConstructorKey());
	}
	return instance.get();
}

void CollisionManager::Finalize()
{
	colliders_.clear();
	instance.reset();
}

bool CollisionManager::IsCollision(const AABB& aabb, const AABB& aabbHit)
{
	if (aabb.min.x <= aabbHit.max.x && aabb.max.x >= aabbHit.min.x &&
		aabb.min.y <= aabbHit.max.y && aabb.max.y >= aabbHit.min.y &&
		aabb.min.z <= aabbHit.max.z && aabb.max.z >= aabbHit.min.z) {
		return true;
	}

	return false;
}

AABB CollisionManager::MakeAABB(const Vector3& translate, const Vector3& size)
{
	AABB aabb;

	aabb.min = { translate.x - size.x, translate.y - size.y, translate.z - size.z };
	aabb.max = { translate.x + size.x, translate.y + size.y, translate.z + size.z };

	return aabb;
}
