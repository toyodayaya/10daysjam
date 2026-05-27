#include "Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <cassert>

using namespace MathManager;

std::unique_ptr<Animation> Animation::instance = nullptr;

Animation* Animation::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::unique_ptr<Animation>(new Animation);
	}

	return instance.get();
}

void Animation::Finalize()
{
	instance.reset();
}

Animation::Animations Animation::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
    Animations animation;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0); // アニメーションがない
	aiAnimation* animationAssimp = scene->mAnimations[0];
	animation.duration = static_cast<float>(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

	// NodeAnimationを解析する
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex)
	{
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		// translateを取得
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y,keyAssimp.mValue.z };
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}

		// rotateを取得
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
		{
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y,keyAssimp.mValue.z,keyAssimp.mValue.w };
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}

		// scaleを取得
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y,keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}

	// 解析完了
	return animation;
}

Vector3 Animation::CalculateValue(const AnimationCurve<Vector3>& keyframes, float time)
{
	// 特殊なケースを除外する
	assert(!keyframes.keyframes.empty());
	// キーが一つもしくは時刻がキーフレーム前なら
	if (keyframes.keyframes.size() == 1 || time <= keyframes.keyframes[0].time)
	{
		// 最初の値にする
		return keyframes.keyframes[0].value;
	}

	// 先頭から時刻を調べる
	for (size_t index = 0; index < keyframes.keyframes.size() - 1; ++index)
	{
		size_t nextIndex = index + 1;
		// indexとnextIndexのkeyframeを取得して範囲内に時刻があるか判定
		if (keyframes.keyframes[index].time <= time && time <= keyframes.keyframes[nextIndex].time)
		{
			// 範囲内を補間する
			float t = (time - keyframes.keyframes[index].time) / (keyframes.keyframes[nextIndex].time - keyframes.keyframes[index].time);
			return Lerp(keyframes.keyframes[index].value, keyframes.keyframes[nextIndex].value, t);
		}
	}

	// ここまで来たら最後の値を返す
	return (*keyframes.keyframes.rbegin()).value;
}

Quaternion Animation::CalculateValueQuaternion(const AnimationCurve<Quaternion>& keyframes, float time)
{
	// 特殊なケースを除外する
	assert(!keyframes.keyframes.empty());
	// キーが一つもしくは時刻がキーフレーム前なら
	if (keyframes.keyframes.size() == 1 || time <= keyframes.keyframes[0].time)
	{
		// 最初の値にする
		return keyframes.keyframes[0].value;
	}

	// 先頭から時刻を調べる
	for (size_t index = 0; index < keyframes.keyframes.size() - 1; ++index)
	{
		size_t nextIndex = index + 1;
		// indexとnextIndexのkeyframeを取得して範囲内に時刻があるか判定
		if (keyframes.keyframes[index].time <= time && time <= keyframes.keyframes[nextIndex].time)
		{
			// 範囲内を補間する
			float t = (time - keyframes.keyframes[index].time) / (keyframes.keyframes[nextIndex].time - keyframes.keyframes[index].time);
			return Slerp(keyframes.keyframes[index].value, keyframes.keyframes[nextIndex].value, t);
		}
	}

	// ここまで来たら最後の値を返す
	return (*keyframes.keyframes.rbegin()).value;
}
