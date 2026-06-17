#pragma once
#include "MathManager.h"
#include "AnimationCommon.h"
#include "ModelCommon.h"
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <map>
#include <array>
#include <span>
#include <assimp/scene.h>
#include "DebugDraw.h"

class Animation
{
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

	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
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
		Microsoft::WRL::ComPtr <ID3D12Resource> paletteResource;
		std::span<WellForGPU> mappedPalette;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
		uint32_t srvIndex;
	};

	struct CameraForGPU
	{
		Vector3 worldPosition;
	};

	struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};

	struct DirectionalLight
	{
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	struct PointLight
	{
		Vector4 color;
		Vector3 position;
		float intensity;
		float radius;
		float decay;
		float padding[2];
	};

	struct SpotLight
	{
		Vector4 color;
		Vector3 position;
		float intensity;
		Vector3 direction;
		float distance;
		float decay;
		float cosAngle;
		float cosFalloffStart;
		float padding[2];
	};


	// 初期化
	void Initialize(AnimationCommon* animationCommon, const std::string& directoryPath, const std::string& filename);
	// アニメーション更新
	void Update();
	// 描画
	void Draw();

	// アニメーションつきObjファイルを読み込む関数
	static ModelData LoadAnimationModelFile(const std::string& directoryPath, const std::string& fileName);

	// Node変換関数
	static Node ReadNode(aiNode* node);
	// 座標変換行列データ作成
	void CreateTransformMatrixData3d();
	// 頂点データを作成
	void CreateVertexData3d();
	// マテリアルデータを作成
	void CreateMaterialData3d();
	// カメラデータの作成
	void CreateCameraResource();
	// 平行光源データ作成
	void CreateDirectionalLight();
	// 点光源データ作成
	void CreatePointLight();
	// スポットライトデータ作成
	void CreateSpotLight();
	
	// アニメーションを解析する関数
	Animations LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
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
	void ApplyAnimation(Skeleton& skeleton, const Animations& animation, float animationTime);


	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }
	void SetCamera(Camera* camera) { this->camera_ = camera; }
	void SetEnvironmentMapTextureFilePath(const std::string& filePath) { environmentMapTextureFilePath = filePath; }

	// getter
	ModelData GetModelData() { return modelData; }

	// 終了
	void Finalize();

private:
	// ポインタ
	DirectXBasis* dxBasis_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	AnimationCommon* animationCommon_ = nullptr;
	ModelCommon* modelCommon_ = nullptr;
	Camera* camera_ = nullptr;
	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;
	
	// モデルデータ
	ModelData modelData;
	// WVP用のリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationResource;
	// データを書き込む
	TransformationMatrix* transformationData = nullptr;
	// カメラデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	CameraForGPU* cameraData_ = nullptr;
	// 平行光源リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	// データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	// 点光源リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> pointLightResource;
	PointLight* pointLightData = nullptr;
	// スポットライトリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> spotLightResource;
	SpotLight* spotLightData = nullptr;
	// 環境マップ用のテクスチャパス
	std::string environmentMapTextureFilePath;

	// VertexResource
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceDebug;
	// IndexResource
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// 頂点データ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	// マテリアルリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	Material* materialData = nullptr;

	SkinCluster skinCluster;

	// アニメーションの再生時間
	float animationTime = 0.0f;
	// アニメーションデータ
	Animations animation;
	// スケルトンデータ
	Skeleton skeleton;

	EulerTransform cameraTransform;
	QuaternionTransform transform;

#ifdef _DEBUG

	// デバッグ描画
	std::vector<std::unique_ptr<DebugDraw>> debugSpheres_;
	std::vector<std::unique_ptr<DebugDraw>> debugLines_;
#endif // _DEBUG


};

