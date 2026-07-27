#pragma once
#include "ModelCommon.h"
#include "MathManager.h"
#include <wrl.h>
#include <d3d12.h>
#include <assimp/scene.h>
#include <map>
#include <span>

#ifdef _DEBUG
#include "DebugDraw.h"
#endif // _DEBUG

class Model
{
public:
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct  Material
	{
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
		float shininess;
		float environmentCoefficient;
		int32_t useEnvironmentMap;
	};

	struct MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	struct Node
	{
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	struct VertexWeightData
	{
		float weight;
		uint32_t vertexIndex;
	};

	struct JointWeightData
	{
		Matrix4x4 inverseBindPoseMatrix;
		std::vector<VertexWeightData> vertexWeights;
	};

	struct ModelData
	{
		std::map<std::string, JointWeightData> skinClusterData;
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Node rootNode;
	};

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

	struct Animation
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

	static const uint32_t kNumMaxInfluence = 4;
	struct VertexInfluence
	{
		std::array<float, kNumMaxInfluence> weights;
		std::array<int32_t, kNumMaxInfluence> jointIndices;
	};

	struct WellForGPU
	{
		Matrix4x4 skeletonSpaceMatrix;
		Matrix4x4 skeletonSpaceInverseTransposeMatrix;
	};

	struct SkinCluster
	{
		std::vector<Matrix4x4> inverseBindPoseMatrix;
		Microsoft::WRL::ComPtr <ID3D12Resource> influenceResource;
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
		std::span<VertexInfluence> mappedInfluence;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>influenceSrvHandle;
		uint32_t influenceIndex;
		Microsoft::WRL::ComPtr <ID3D12Resource> paletteResource;
		std::span<WellForGPU> mappedPalette;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
		uint32_t srvIndex;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> inputSrvHandle;
		uint32_t inputIndex;
		Microsoft::WRL::ComPtr <ID3D12Resource> outputResource;
		uint32_t outputIndex;
		D3D12_VERTEX_BUFFER_VIEW outputBufferView;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> outputUavHandle;
		Microsoft::WRL::ComPtr<ID3D12Resource> skinningInformationResource;
	};

	struct SkinningInformation
	{
		uint32_t numVertices;
	};

	enum AnimationType
	{
		kNone,
		kMove
	};
	
public:
	// 初期化
	void Initialize(ModelCommon* modelManager, const std::string& directoryPath, const std::string& filePath,AnimationType);
	// 更新
	void Update(Matrix4x4 transform);
	// 描画
	void Draw();
	// mtlファイルを読む関数
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// Objファイルを読み込む関数
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& fileName);
	// Node変換関数
	static Node ReadNode(aiNode* node);
	// 頂点データを作成
	void CreateVertexData3d();
	// マテリアルデータを作成
	void CreateMaterialData3d();

	// アニメーションを解析する関数
	Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	// SkinClusterを解析する関数
	SkinCluster CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData);
	// 任意の時刻の値を取得する関数
	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	Quaternion CalculateValueQuaternion(const AnimationCurve<Quaternion>& keyframes, float time);

	// 階層構造からskeletonを作る関数
	Skeleton CreateSkeleton(const Node& rootNode);
	// NodeからJointを作る関数
	int32_t CreateJoint(const Node& node, const std::optional<int32_t> parent, std::vector<Joint>& joints);
	// skeletonの更新
	void SkeletonUpdate(Skeleton& skeleton);
	// skinClusterの更新
	void SkinClusterUpdate(SkinCluster& skinCluster, Skeleton& skeleton);
	// アニメーションを適用する関数
	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);
	// getter
	ModelData GetModelData() { return modelData_; }
	Skeleton GetSkeleton() { return skeleton_; }

private:
	ModelCommon* modelManager = nullptr;
	DirectXBasis* dxBasis_;
	SrvManager* srvManager_;
	// objファイルのデータ
	ModelData modelData_;
	// VertexResource
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceDebug_;
	// IndexResource
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource_;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	// 頂点データ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	// マテリアルリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// アニメーションの再生時間
	float animationTime_ = 0.0f;
	// アニメーションデータ
	Animation animation_;
	// スケルトンデータ
	Skeleton skeleton_;
	// スキンクラスター
	SkinCluster skinCluster_;

	// アニメーションタイプ
	AnimationType type_ = kNone;


#ifdef _DEBUG

	// デバッグ描画
	std::vector<std::unique_ptr<DebugDraw>> debugSpheres_;
	std::unique_ptr<DebugDraw> debugLines_;
#endif // _DEBUG

};

