
#pragma once
#include <string>
#include <MathManager.h>
#include <wrl.h>

class ParticleEmitter
{
public:
	enum Type
	{
		// 通常
		kNormal,
		// 斬撃
		kHitEffect,
		// 円
		kRing,
		// 円柱
		kCylinder,
	};

private:

	struct Emitter
	{
		EulerTransform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
		uint32_t count;
		float frequency;
		float frequencyTime;
	};

public:
	static ParticleEmitter* instance;

	ParticleEmitter(const std::string name, const EulerTransform& transform,
		const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime, 
		float frequency, uint32_t count);
	~ParticleEmitter() = default;
	ParticleEmitter(ParticleEmitter&) = delete;
	ParticleEmitter& operator=(ParticleEmitter&) = delete;

	// メンバ変数
	std::string name;
	Emitter emitter{};

public:

	// パーティクル発生
	void Emit();

	// 更新処理
	void Update();
};
