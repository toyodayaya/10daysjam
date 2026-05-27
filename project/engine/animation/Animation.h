#pragma once
#include "MathManager.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

class Animation
{
public:
	// コンストラクタ
	Animation() = default;
	// デストラクタ
	~Animation() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	Animation(const Animation&) = delete;
	Animation& operator=(const Animation&) = delete;
	
public:

	template <typename tValue>
	struct Keyframe
	{
		float time;
		tValue value;
	};

	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeQuaternion = Keyframe<Quaternion>;


	template <typename tValue>
	struct AnimationCurve
	{
		std::vector<Keyframe<tValue>> keyframes;
	};

	struct NodeAnimation
	{
		AnimationCurve<Vector3> translate;
		AnimationCurve<Quaternion> rotate;
		AnimationCurve<Vector3> scale;
	};

	struct Animations
	{
		float duration;
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

	// アニメーションを解析する関数
	Animations LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	// 任意の時刻の値を取得する関数
	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	Quaternion CalculateValueQuaternion(const AnimationCurve<Quaternion>& keyframes, float time);

	// インスタンス
	static Animation* GetInstance();
	// シングルトンインスタンス
	static std::unique_ptr<Animation> instance;
	// 終了
	void Finalize();
};

