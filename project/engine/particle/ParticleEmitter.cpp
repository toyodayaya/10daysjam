#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include "DirectXBasis.h"
#include "Logger.h"
using namespace Logger;

ParticleEmitter::ParticleEmitter(const std::string name,const ParticleManager::EmitterSphere& emitterSphere)

	// 引数で受け取ってメンバ変数として記録する
	: name(name)
{
	emitter = emitterSphere;
}

void ParticleEmitter::Emit()
{
	ParticleManager::GetInstance()->Emit(name,emitter);
}

void ParticleEmitter::Update()
{
	const float kDeltaTime = 1.0f / 60.0f;
	emitter.frequencyTime += kDeltaTime;

	if (emitter.frequencyTime >= emitter.frequency)
	{
		// 射出間隔を過ぎていたら射出許可を送信
		emitter.frequencyTime -= emitter.frequency;
		// Particleデータを転送
		Emit();
	}
	
}