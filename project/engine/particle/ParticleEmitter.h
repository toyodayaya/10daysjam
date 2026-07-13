
#pragma once
#include <string>
#include <MathManager.h>
#include <ParticleManager.h>
#include <wrl.h>

class ParticleEmitter
{
public:
	static std::unique_ptr<ParticleEmitter> instance;

	ParticleEmitter(const std::string name,const ParticleManager::EmitterSphere& emitterSphere);
	~ParticleEmitter() = default;
	ParticleEmitter(ParticleEmitter&) = delete;
	ParticleEmitter& operator=(ParticleEmitter&) = delete;

	// メンバ変数
	std::string name;
	ParticleManager::EmitterSphere emitter{};

public:

	// パーティクル発生
	void Emit();

	// 更新処理
	void Update();
};
