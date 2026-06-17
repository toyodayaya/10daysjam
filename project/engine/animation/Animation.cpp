#include "Animation.h"
#include "AnimationCommon.h"
#include "TextureManager.h"
#include "ModelCommon.h"
#include "DebugDrawCommon.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <vector>
#include <numbers>
#define _USE_MATH_DEFINES
using namespace MathManager;


void Animation::Initialize(AnimationCommon* animationCommon, const std::string& directoryPath, const std::string& filename)
{
	// 引数で受け取ってメンバ変数に記録する
	this->animationCommon_ = animationCommon;
	dxBasis_ = animationCommon->GetDxBasis();
	srvManager_ = animationCommon->GetSrvManager();
	camera_ = animationCommon->GetDefaultCamera();

	// モデル読み込み
	modelData = LoadAnimationModelFile(directoryPath, filename);

	// アニメーションデータを読み込む
	animation = LoadAnimationFile(directoryPath, filename);

	// スケルトンデータを作成
	skeleton = CreateSkeleton(modelData.rootNode);

	// SkinClusterを作成
	skinCluster = CreateSkinCluster(skeleton, modelData);


	// objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	// 読み込んだテクスチャの番号を取得
	modelData.material.textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

	// 頂点データ作成
	CreateVertexData3d();

	// マテリアルデータ作成
	CreateMaterialData3d();

	// 座標変換行列データ作成
	CreateTransformMatrixData3d();

	// 平行光源データ作成
	CreateDirectionalLight();

	// 点光源データ作成
	CreatePointLight();

	// スポットライトデータ作成
	CreateSpotLight();

	// カメラデータ作成
	CreateCameraResource();

	// Transform変数を作る
	cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


#ifdef _DEBUG

	// デバッグ描画の初期化
	for (uint32_t i = 0; i < skeleton.joints.size(); ++i)
	{
		std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
		debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kSphere);
		debugDraw->SetScale(Vector3{ 0.01f,0.01f,0.01f });
		debugDraw->SetTranslate(skeleton.joints[i].transform.translate);
		debugSpheres_.push_back(std::move(debugDraw));
	}

	for (uint32_t i = 0; i < skeleton.joints.size(); ++i)
	{
		if (!skeleton.joints[i].parent)
		{
			continue; // 親がいないルートノードはスキップ
		}

		std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
		debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kLine);
		debugDraw->SetScale(Vector3{ 0.1f,0.1f,0.1f });
		debugDraw->SetTranslate(Vector3{ 0.0f, 0.0f, 0.0f });
		debugLines_.push_back(std::move(debugDraw));
	}

#endif // _DEBUG

}

void Animation::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrixQuat(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera_)
	{
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;
	}

	// アニメーションを再生
	animationTime += 1.0f / 60.0f;
	animationTime = std::fmod(animationTime, animation.duration);

	/*Animation::NodeAnimation& rootNodeAnimation = animation.nodeAnimations[model->GetModelData().rootNode.name];
	Vector3 translate = Animation::GetInstance()->CalculateValue(rootNodeAnimation.translate, animationTime);
	Quaternion rotate = Animation::GetInstance()->CalculateValueQuaternion(rootNodeAnimation.rotate, animationTime);
	Vector3 scale = Animation::GetInstance()->CalculateValue(rootNodeAnimation.scale, animationTime);
	Matrix4x4 localMatrix = MakeAffineMatrixQuat(scale, rotate, translate);*/

	ApplyAnimation(skeleton, animation, animationTime);
	SkeletonUpdate(skeleton);
	SkinClusterUpdate(skinCluster, skeleton);

	transformationData->WVP = worldViewProjectionMatrix;
	transformationData->World = worldMatrix;

	transformationData->WorldInverseTranspose = Transpose(Inverse(transformationData->World));
	if (camera_)
	{
		cameraData_->worldPosition = camera_->GetTranslate();
	}

#ifdef _DEBUG



	// デバッグ描画の更新処理
	for (size_t i = 0; i < skeleton.joints.size(); ++i)
	{
		Matrix4x4 jointWorldMatrix = Multiply(skeleton.joints[i].skeletonSpaceMatrix, worldMatrix);
		Vector3 jointWorldPos = { jointWorldMatrix.m[3][0], jointWorldMatrix.m[3][1], jointWorldMatrix.m[3][2] };
		debugSpheres_[i]->SetTranslate(jointWorldPos);
		debugSpheres_[i]->Update();
	}

	size_t lineIndex = 0;
	for (size_t i = 0; i < skeleton.joints.size(); ++i)
	{
		if (!skeleton.joints[i].parent) { continue; }
		if (lineIndex >= debugLines_.size()) { break; }

		// 親と子の各ワールド行列・座標を計算
		Matrix4x4 childMat = Multiply(skeleton.joints[i].skeletonSpaceMatrix, worldMatrix);
		Vector3 childPos = { childMat.m[3][0], childMat.m[3][1], childMat.m[3][2] };

		uint32_t parentIdx = *skeleton.joints[i].parent;
		Matrix4x4 parentMat = Multiply(skeleton.joints[parentIdx].skeletonSpaceMatrix, worldMatrix);
		Vector3 parentPos = { parentMat.m[3][0], parentMat.m[3][1], parentMat.m[3][2] };

		Vector3 midPos = Vector3Add(parentPos, childPos);
		midPos.x /= 2.0f; midPos.y /= 2.0f; midPos.z /= 2.0f;
		debugLines_[lineIndex]->SetTranslate(midPos);

		Vector3 direction = Vector3Subtract(childPos, parentPos); // 親から子へのベクトル
		float distance = Length(direction);                      // 2点間の距離

		float lineThickness = 0.05f; // 線の太さ
		debugLines_[lineIndex]->SetScale(Vector3{ lineThickness, distance * 0.5f, lineThickness });

		if (distance > 0.0001f)
		{
			Vector3 baseDir = { 0.0f, 1.0f, 0.0f }; // 板ポリの初期方向
			Quaternion rot = FromToRotation(baseDir, direction);

			// DebugDrawに用意したSetRotateを呼ぶ
			debugLines_[lineIndex]->SetRotate(rot);
		}

		debugLines_[lineIndex]->UpdateLine();
		lineIndex++;
	}
#endif // _DEBUG


}

void Animation::Draw()
{
	// 共通描画設定
	AnimationCommon::GetInstance()->DrawSettingCommon();

	// VBVを設定
	D3D12_VERTEX_BUFFER_VIEW vbvs[2] =
	{
		vertexBufferView,
		skinCluster.influenceBufferView
	};
	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 2, vbvs);
	// IBVを設定
	dxBasis_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	// マテリアルCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelData.material.textureFilePath));
	// 平行光源用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// スキニングパレットを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(4, srvManager_->GetGPUDescriptorHandle(skinCluster.srvIndex));
	// カメラリソース用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(5, cameraResource->GetGPUVirtualAddress());
	// 点光源用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(6, pointLightResource->GetGPUVirtualAddress());
	// スポットライト用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(7, spotLightResource->GetGPUVirtualAddress());
	// 環境マップ用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(8, TextureManager::GetInstance()->GetSRVHandleGPU(environmentMapTextureFilePath));

	// 描画
	dxBasis_->GetCommandList()->DrawIndexedInstanced(UINT(modelData.indices.size()), 1, 0, 0, 0);


#ifdef _DEBUG

	// デバッグ描画
	for (const std::unique_ptr<DebugDraw>& debugDraw : debugSpheres_)
	{
		debugDraw->Draw();
	}

	for (const std::unique_ptr<DebugDraw>& debugDraw : debugLines_)
	{
		debugDraw->DrawLine();
	}
#endif // _DEBUG
}

Animation::ModelData Animation::LoadAnimationModelFile(const std::string& directoryPath, const std::string& fileName)
{
	ModelData modelData;

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	// メッシュを解析する
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		modelData.vertices.resize(mesh->mNumVertices);
		// 頂点を解析する
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
		{
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			// 左手系に変換
			modelData.vertices[vertexIndex].position = { -position.x,position.y,position.z,1.0f };
			modelData.vertices[vertexIndex].normal = { -normal.x,normal.y,normal.z };
			modelData.vertices[vertexIndex].texcoord = { texcoord.x,texcoord.y };
		}

		// indexを解析する
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
		{
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element)
			{
				uint32_t vertexIndex = face.mIndices[element];
				modelData.indices.push_back(vertexIndex);
			}
		}

		// skinCluster構築用のデータを取得
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			// Jointごとに格納領域を作る
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			// InverseBindPoseMatrixの抽出
			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			// 左手系のBindPoseMatrixを作る
			Matrix4x4 bindPoseMatrix = MakeAffineMatrixQuat
			(
				{ scale.x,scale.y,scale.z }, { rotate.x,-rotate.y,-rotate.z,rotate.w }, { -translate.x,translate.y,translate.z }
			);
			// InverseBindPoseMatrixにする
			jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);

			// Weight情報を持たせる
			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
			{
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight,bone->mWeights[weightIndex].mVertexId });
			}
		}
	}

	// マテリアルを解析する
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
	{
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);
	return modelData;
}

Animation::Node Animation::ReadNode(aiNode* node)
{
	Node result;
	// localMatrixを再構築する
	aiVector3D scale, translate;
	aiQuaternion rotate;

	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	result.transform.translate = { -translate.x,translate.y,translate.z };
	result.localMatrix = MakeAffineMatrixQuat(result.transform.scale, result.transform.rotate, result.transform.translate);


	// Node名を格納
	result.name = node->mName.C_Str();
	// 子供の数だけ確保
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
	{
		// 再帰的に読んで階層構造を作る
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}


	return result;
}

void Animation::CreateTransformMatrixData3d()
{
	// WVP用のリソースを作る
	transformationResource = dxBasis_->CreateBufferResources(sizeof(TransformationMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationData));
	// 単位行列を書き込んでおく
	transformationData->World = MakeIdentity4x4();
	transformationData->WVP = MakeIdentity4x4();
	transformationData->WorldInverseTranspose = Inverse(transformationData->World);
	transformationData->WorldInverseTranspose = Transpose(transformationData->WorldInverseTranspose);
}

void Animation::CreateVertexData3d()
{
	// VertexResourceを生成する
	// 頂点リソースを作る
	vertexResource = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelData.vertices.size());
	indexResource = dxBasis_->CreateBufferResources(sizeof(uint32_t) * modelData.indices.size());

	// 頂点バッファビューを作成する

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * modelData.indices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	// 頂点データにリソースをコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
	std::memcpy(indexData, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());
}

void Animation::CreateMaterialData3d()
{
	// マテリアルリソースを作る
	materialResource = dxBasis_->CreateBufferResources(sizeof(Material) * modelData.vertices.size());
	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 白色に設定
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// Lighting設定
	materialData->enableLighting = true;
	// UVTransform行列を単位行列で初期化
	materialData->uvTransform = MakeIdentity4x4();
	// 光沢度
	materialData->shininess = 2.0f;
	// 映り込み
	materialData->environmentCoefficient = 1.0f;
	materialData->useEnvironmentMap = false;
}

void Animation::CreateCameraResource()
{
	// カメラリソースの生成
	cameraResource = dxBasis_->CreateBufferResources(sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	cameraData_->worldPosition = camera_->GetTranslate();
}

void Animation::CreateDirectionalLight()
{
	// 平行光源用のリソースを作る
	directionalLightResource = dxBasis_->CreateBufferResources(sizeof(DirectionalLight));

	//書き込むためのアドレスを取得
	directionalLightResource->Map(
		0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 0.5f;
}

void Animation::CreatePointLight()
{
	// 点光源用のリソースを作る
	pointLightResource = dxBasis_->CreateBufferResources(sizeof(PointLight));
	// 書き込むためのアドレスを取得
	pointLightResource->Map(
		0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData->position = { 0.0f,2.0f,0.0f };
	pointLightData->intensity = 0.5f;
	pointLightData->radius = 100.0f;
	pointLightData->decay = 100.0f;
}

void Animation::CreateSpotLight()
{
	// スポットライト用のリソースを作る
	spotLightResource = dxBasis_->CreateBufferResources(sizeof(SpotLight));
	// 書き込むためのアドレスを取得
	spotLightResource->Map(
		0, nullptr, reinterpret_cast<void**>(&spotLightData));
	spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	spotLightData->position = { 2.0f,1.25f,0.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction =
		Normalize({ -1.0f,-1.0f,0.0f });
	spotLightData->intensity = 4.0f;
	spotLightData->decay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 3.0f);
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
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y,-keyAssimp.mValue.z,keyAssimp.mValue.w };
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}

		// scaleを取得
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y,keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}

	// 解析完了
	return animation;
}

Animation::SkinCluster Animation::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData)
{
	// palette用のResourceを確保
	skinCluster.paletteResource = dxBasis_->CreateBufferResources(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalatte = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalatte));
	skinCluster.mappedPalette = { mappedPalatte,skeleton.joints.size() };
	skinCluster.srvIndex = srvManager_->Allocate();
	skinCluster.paletteSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinCluster.srvIndex);
	skinCluster.paletteSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinCluster.srvIndex);

	// palette用のsrvを作成
	D3D12_SHADER_RESOURCE_VIEW_DESC palleteSrvDesc{};
	palleteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	palleteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	palleteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	palleteSrvDesc.Buffer.FirstElement = 0;
	palleteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	palleteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
	palleteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	dxBasis_->GetDevice()->CreateShaderResourceView(skinCluster.paletteResource.Get(), &palleteSrvDesc, skinCluster.paletteSrvHandle.first);

	// influence用のResourceを確保
	skinCluster.influenceResource = dxBasis_->CreateBufferResources(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.mappedInfluence = { mappedInfluence,modelData.vertices.size() };

	// Influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// InverseBindPoseMatrixの保存領域を作成
	skinCluster.inverseBindPoseMatrix.resize(skeleton.joints.size());
	std::generate(skinCluster.inverseBindPoseMatrix.begin(), skinCluster.inverseBindPoseMatrix.end(), MakeIdentity4x4);

	// ModelDataのSkinCluster情報を解析してInfluenceの中身を埋める
	for (const auto& jointWeight : modelData.skinClusterData)
	{
		// skeletonに対象となるjointが含まれているか確認
		auto it = skeleton.jointMap.find(jointWeight.first);
		if (it == skeleton.jointMap.end())
		{
			// 存在しなければ次へ
			continue;
		}

		skinCluster.inverseBindPoseMatrix[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights)
		{
			// 該当するinfluence情報を参照する
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];

			// 空いてるところに入れる
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index)
			{
				// weightとjointのindexを代入
				if (currentInfluence.weights[index] == 0.0f)
				{
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}
	}

	return skinCluster;
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


Animation::Skeleton Animation::CreateSkeleton(const Node& rootNode)
{
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// jointを更新
	SkeletonUpdate(skeleton);

	// 名前とindexのマッピングを行う
	for (const Joint& joint : skeleton.joints)
	{
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

int32_t Animation::CreateJoint(const Node& node, const std::optional<int32_t> parent, std::vector<Joint>& joints)
{
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());
	joint.parent = parent;
	joints.push_back(joint);

	for (const Node& child : node.children)
	{
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	return joint.index;
}

void Animation::SkeletonUpdate(Skeleton& skeleton)
{
	// すべてのjointを更新する
	for (Joint& joint : skeleton.joints)
	{
		joint.localMatrix = MakeAffineMatrixQuat(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		// 親がいれば親の行列をかける
		if (joint.parent)
		{
			joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skeleton.joints[*joint.parent].skeletonSpaceMatrix);
		}
		else
		{
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Animation::SkinClusterUpdate(SkinCluster& skinCluster, Skeleton& skeleton)
{
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex)
	{
		assert(jointIndex < skinCluster.inverseBindPoseMatrix.size());

		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
			Multiply(skinCluster.inverseBindPoseMatrix[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Transpose(Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}

}

void Animation::ApplyAnimation(Skeleton& skeleton, const Animations& animation, float animationTime)
{
	for (Joint& joint : skeleton.joints)
	{
		// 対象のjointのアニメーションがあれば適用する
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end())
		{
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
			joint.transform.rotate = CalculateValueQuaternion(rootNodeAnimation.rotate, animationTime);
			joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
		}
	}
}

