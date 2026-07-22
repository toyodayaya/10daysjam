#pragma once
#include "MathManager.h"
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include "Camera.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "SrvManager.h"

class DebugDrawCommon;
class DirectXBasis;
class Object3d;

class DebugDraw
{
private:

	struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};

	struct CameraForGPU
	{
		Vector3 worldPosition;
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

	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};

public:

	enum DrawState
	{
		kSphere,
		kLine,
		kBox
	};


	// 初期化
	void Initialize(DebugDrawCommon* debugDrawCommon, std::string textureFilePath,DrawState state);

	// 座標変換行列データ作成
	void CreateTransformMatrixData3d();
	// カメラデータの作成
	void CreateCameraResource();
	// 頂点データ作成
	void CreateVertexData();
	// 線の頂点データ作成
	void CreateVertexDataLine();
	// 箱の頂点データ作成
	void CreateVertexDataBox();

	// 更新
	void Update();
	void UpdateLine();
	void UpdateBox();

	// 描画
	void Draw();
	void DrawLine();
	void DrawBox();

	// setter
	void SetModel(const std::string& filePath);
	void SetAnimationModel(const std::string& directoryPath, const std::string& filename);
	void SetScale(const Vector3& scale) { this->transform.scale = scale; }
	void SetRotate(const Quaternion& rotate) { this->transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
	void SetLineScale(const Vector3& scale) { this->transformLine.scale = scale; }
	void SetLineRotate(const Quaternion& rotate) { this->transformLine.rotate = rotate; }
	void SetLineTranslate(const Vector3& translate) { this->transformLine.translate = translate; }
	void SetBoxScale(const Vector3& scale) { this->transformBox.scale = scale; }
	void SetBoxRotate(const Quaternion& rotate) { this->transformBox.rotate = rotate; }
	void SetBoxTranslate(const Vector3& translate) { this->transformBox.translate = translate; }
	void SetCamera(Camera* camera) { this->camera = camera; }
	void SetMatrix(const Matrix4x4& matrix) { transformationData->World = matrix; }
	void SetParent(QuaternionTransform parent) { this->parent = parent; }

	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Quaternion& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }

	
	
private:
	// ポインタ
	DebugDrawCommon* debugDraw_;
	DirectXBasis* dxBasis_;
	Camera* camera = nullptr;

	// WVP用のリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationResourceLine;
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationResourceBox;
	// データを書き込む
	TransformationMatrix* transformationData = nullptr;
	TransformationMatrix* transformationDataLine = nullptr;
	TransformationMatrix* transformationDataBox = nullptr;

	// カメラデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	CameraForGPU* cameraData_ = nullptr;

	// 頂点リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceLine;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceBox;

	// Index用の頂点リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;

	// 頂点データ
	VertexData* vertexData = nullptr;
	VertexData* vertexDataLine = nullptr;
	VertexData* vertexDataBox = nullptr;
	uint32_t* indexData = nullptr;

	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewLine;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewBox;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	// モデルデータ
	ModelData modelData;
	ModelData modelDataLine;
	ModelData modelDataBox;

	// 分割数
	const uint32_t kSubdivision = 16;
	// 経度分割1つ分の角度
	const float kLonEvery = (2.0f * float(M_PI)) / float(kSubdivision);
	// 緯度分割1つ分の角度
	const float kLatEvery = float(M_PI) / float(kSubdivision);

	// テクスチャ番号
	uint32_t textureIndex = 0;

	// ファイルパス
	std::string filePath;

	// トランスフォーム
	EulerTransform cameraTransform;
	QuaternionTransform transform;
	QuaternionTransform transformLine;
	QuaternionTransform transformBox;

	// 親オブジェクト
	QuaternionTransform parent;
};
