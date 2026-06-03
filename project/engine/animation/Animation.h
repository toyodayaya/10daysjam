#pragma once
#include "MathManager.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <Model.h>
#include <optional>
#include <vector>
#include <map>
#include <string>

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

	struct Joint
	{
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		Matrix4x4 skeletonSpaceMatrix;
		std::string name;
		std::vector<int32_t> children;
		int32_t index;
		std::optional<int32_t> parent;
	};

	struct Skeleton
	{
		int32_t root;
		std::map<std::string, int32_t> jointMap;
		std::vector<Joint> joints;
	};

	// アニメーションを解析する関数
	Animations LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	// 任意の時刻の値を取得する関数
	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	Quaternion CalculateValueQuaternion(const AnimationCurve<Quaternion>& keyframes, float time);

	// 階層構造からskeletonを作る関数
	Skeleton CreateSkeleton(const Model::Node& rootNode);
	// NodeからJointを作る関数
	int32_t CreateJoint(const Model::Node& node, const std::optional<int32_t> parent, std::vector<Joint>& joints);
	// skeletonの更新
	void Update(Skeleton& skeleton);
	// アニメーションを適用する関数
	void ApplyAnimation(Skeleton& skeleton, const Animations& animation, float animationTime);

	// インスタンス
	static Animation* GetInstance();
	// シングルトンインスタンス
	static std::unique_ptr<Animation> instance;
	// 終了
	void Finalize();
};

