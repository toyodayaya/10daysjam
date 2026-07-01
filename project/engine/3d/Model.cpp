#include "Model.h"
#include "ModelCommon.h"
#include "TextureManager.h"
#include "MathManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ImguiManager.h"
#include <numbers>
#include "DebugDrawCommon.h"
#include "DebugDraw.h"
#include "AnimationCommon.h"
#include "Object3dCommon.h"

using namespace MathManager;

void Model::Initialize(ModelCommon* modelManager, const std::string& directoryPath, const std::string& filePath, AnimationType type)
{
	// 引数で受け取ってメンバ変数として記録する
	this->modelManager = modelManager;
	dxBasis_ = modelManager->GetDxBasis();
	srvManager_ = modelManager->GetSrvManager();
	type_ = type;

	// モデル読み込み
	modelData_ = LoadModelFile(directoryPath, filePath);

	//　アニメーションがあるかどうか判定
	switch (type_)
	{
	case kNone:
		
		// 頂点データ作成
		CreateVertexData3d();
		break;

	case kMove:
		// アニメーションデータを読み込む
		animation_ = LoadAnimationFile(directoryPath, filePath);

		// スケルトンデータを作成
		skeleton_ = CreateSkeleton(modelData_.rootNode);

		// 頂点データ作成
		CreateVertexData3d();

		// SkinClusterを作成
		skinCluster_ = CreateSkinCluster(skeleton_, modelData_);
		break;
	}


	// objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);

	// 読み込んだテクスチャの番号を取得
	modelData_.material.textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);

	// マテリアルデータ作成
	CreateMaterialData3d();

#ifdef _DEBUG

	// デバッグ描画の初期化
	for (uint32_t i = 0; i < skeleton_.joints.size(); ++i)
	{
		std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
		debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kSphere);
		debugDraw->SetScale(Vector3{ 0.01f,0.01f,0.01f });
		debugDraw->SetTranslate(skeleton_.joints[i].transform.translate);
		debugSpheres_.push_back(std::move(debugDraw));
	}

	for (uint32_t i = 0; i < skeleton_.joints.size(); ++i)
	{
		if (!skeleton_.joints[i].parent)
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

void Model::Update(Matrix4x4 transform)
{
	// アニメーションを再生
	animationTime_ += 1.0f / 60.0f;
	animationTime_ = std::fmod(animationTime_, animation_.duration);

	ApplyAnimation(skeleton_, animation_, animationTime_);
	SkeletonUpdate(skeleton_);
	SkinClusterUpdate(skinCluster_, skeleton_);

#ifdef _DEBUG



	// デバッグ描画の更新処理
	for (size_t i = 0; i < skeleton_.joints.size(); ++i)
	{
		Matrix4x4 jointWorldMatrix = Multiply(skeleton_.joints[i].skeletonSpaceMatrix,transform);
		Vector3 jointWorldPos = { jointWorldMatrix.m[3][0], jointWorldMatrix.m[3][1], jointWorldMatrix.m[3][2] };
		debugSpheres_[i]->SetTranslate(jointWorldPos);
		debugSpheres_[i]->Update();
	}

	size_t lineIndex = 0;
	for (size_t i = 0; i < skeleton_.joints.size(); ++i)
	{
		if (!skeleton_.joints[i].parent) { continue; }
		if (lineIndex >= debugLines_.size()) { break; }

		// 親と子の各ワールド行列・座標を計算
		Matrix4x4 childMat = Multiply(skeleton_.joints[i].skeletonSpaceMatrix, transform);
		Vector3 childPos = { childMat.m[3][0], childMat.m[3][1], childMat.m[3][2] };

		uint32_t parentIdx = *skeleton_.joints[i].parent;
		Matrix4x4 parentMat = Multiply(skeleton_.joints[parentIdx].skeletonSpaceMatrix, transform);
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

Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	// 必要な変数を宣言する
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open()); // 開けなかったら止める

	// 実際にファイルを読み込みMaterialDataを構築する
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	// MaterialDataを返す
	return materialData;
}

Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& fileName)
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

Model::Node Model::ReadNode(aiNode* node)
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

void Model::CreateVertexData3d()
{
	// VertexResourceを生成する
	// 頂点リソースを作る
	vertexResource_ = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelData_.vertices.size());
	indexResource_ = dxBasis_->CreateBufferResources(sizeof(uint32_t) * modelData_.indices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData_.indices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	// 頂点データにリソースをコピー
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	std::memcpy(indexData_, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());

}

void Model::CreateMaterialData3d()
{
	// マテリアルリソースを作る
	materialResource_ = dxBasis_->CreateBufferResources(sizeof(Material) * modelData_.vertices.size());
	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 白色に設定
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// Lighting設定
	materialData_->enableLighting = true;
	// UVTransform行列を単位行列で初期化
	materialData_->uvTransform = MakeIdentity4x4();
	// 光沢度
	materialData_->shininess = 2.0f;
	// 映り込み
	materialData_->environmentCoefficient = 1.0f;
	materialData_->useEnvironmentMap = false;
}

Model::Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
	Animation animation;
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

Model::SkinCluster Model::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData)
{
	// palette用のResourceを確保
	skinCluster_.paletteResource = dxBasis_->CreateBufferResources(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalatte = nullptr;
	skinCluster_.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalatte));
	skinCluster_.mappedPalette = { mappedPalatte,skeleton.joints.size() };
	skinCluster_.srvIndex = srvManager_->Allocate();
	skinCluster_.paletteSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinCluster_.srvIndex);
	skinCluster_.paletteSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinCluster_.srvIndex);

	// palette用のsrvを作成
	D3D12_SHADER_RESOURCE_VIEW_DESC palleteSrvDesc{};
	palleteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	palleteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	palleteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	palleteSrvDesc.Buffer.FirstElement = 0;
	palleteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	palleteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
	palleteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	dxBasis_->GetDevice()->CreateShaderResourceView(skinCluster_.paletteResource.Get(), &palleteSrvDesc, skinCluster_.paletteSrvHandle.first);

	// influence用のResourceを確保
	skinCluster_.influenceResource = dxBasis_->CreateBufferResources(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster_.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster_.mappedInfluence = { mappedInfluence,modelData.vertices.size() };

	// Influence用のVBVを作成
	skinCluster_.influenceBufferView.BufferLocation = skinCluster_.influenceResource->GetGPUVirtualAddress();
	skinCluster_.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster_.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// Influence用のSRVを作成
	skinCluster_.influenceIndex = srvManager_->Allocate();
	skinCluster_.influenceSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinCluster_.influenceIndex);
	skinCluster_.influenceSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinCluster_.influenceIndex);
	D3D12_SHADER_RESOURCE_VIEW_DESC influenceSrvDesc{};
	influenceSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	influenceSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	influenceSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	influenceSrvDesc.Buffer.FirstElement = 0;
	influenceSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	influenceSrvDesc.Buffer.NumElements = UINT(modelData.vertices.size());
	influenceSrvDesc.Buffer.StructureByteStride = sizeof(VertexInfluence);
	dxBasis_->GetDevice()->CreateShaderResourceView(skinCluster_.influenceResource.Get(), &influenceSrvDesc, skinCluster_.influenceSrvHandle.first);


	// InverseBindPoseMatrixの保存領域を作成
	skinCluster_.inverseBindPoseMatrix.resize(skeleton.joints.size());
	std::generate(skinCluster_.inverseBindPoseMatrix.begin(), skinCluster_.inverseBindPoseMatrix.end(), MakeIdentity4x4);


	// UAVの生成
	skinCluster_.outputIndex = srvManager_->Allocate();
	skinCluster_.outputUavHandle.first = srvManager_->GetCPUDescriptorHandle(skinCluster_.outputIndex);
	skinCluster_.outputUavHandle.second = srvManager_->GetGPUDescriptorHandle(skinCluster_.outputIndex);
	skinCluster_.outputResource = dxBasis_->CreateOutputVertexBuffer(sizeof(VertexData) * modelData.vertices.size());
	skinCluster_.outputBufferView.BufferLocation = skinCluster_.outputResource->GetGPUVirtualAddress();
	skinCluster_.outputBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	skinCluster_.outputBufferView.StrideInBytes = sizeof(VertexData);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = UINT(modelData.vertices.size());
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = sizeof(VertexData);

	dxBasis_->GetDevice()->CreateUnorderedAccessView(skinCluster_.outputResource.Get(), nullptr, &uavDesc, skinCluster_.outputUavHandle.first);

	// inputVertexのResourceを作成
	skinCluster_.inputIndex = srvManager_->Allocate();
	skinCluster_.inputSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinCluster_.inputIndex);
	skinCluster_.inputSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinCluster_.inputIndex);
	D3D12_SHADER_RESOURCE_VIEW_DESC inputSrvDesc{};
	inputSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	inputSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	inputSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	inputSrvDesc.Buffer.FirstElement = 0;
	inputSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	inputSrvDesc.Buffer.NumElements = UINT(modelData.vertices.size());
	inputSrvDesc.Buffer.StructureByteStride = sizeof(VertexData);
	dxBasis_->GetDevice()->CreateShaderResourceView(vertexResource_.Get(), &inputSrvDesc, skinCluster_.inputSrvHandle.first);

	// OutputVertexのVBVを作成
	skinCluster_.outputBufferView.BufferLocation = skinCluster_.outputResource->GetGPUVirtualAddress();
	skinCluster_.outputBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	skinCluster_.outputBufferView.StrideInBytes = sizeof(VertexData);

	// InformationResourceを作成
	skinCluster_.skinningInformationResource = dxBasis_->CreateBufferResources(sizeof(SkinningInformation));
	SkinningInformation* info = nullptr;
	skinCluster_.skinningInformationResource->Map(0, nullptr, reinterpret_cast<void**>(&info));
	info->numVertices = UINT(modelData.vertices.size());


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

		skinCluster_.inverseBindPoseMatrix[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights)
		{
			// 該当するinfluence情報を参照する
			auto& currentInfluence = skinCluster_.mappedInfluence[vertexWeight.vertexIndex];

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

	return skinCluster_;
}

Vector3 Model::CalculateValue(const AnimationCurve<Vector3>& keyframes, float time)
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

Quaternion Model::CalculateValueQuaternion(const AnimationCurve<Quaternion>& keyframes, float time)
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

Model::Skeleton Model::CreateSkeleton(const Node& rootNode)
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

int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t> parent, std::vector<Joint>& joints)
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

void Model::SkeletonUpdate(Skeleton& skeleton)
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

void Model::SkinClusterUpdate(SkinCluster& skinCluster, Skeleton& skeleton)
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

void Model::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
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



void Model::Draw()
{
	// アニメーションの有無で分岐
	switch (type_)
	{
	case kNone:

		// VBVを設定
		dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);

		break;

	case kMove:
		// CS用の設定
		AnimationCommon::GetInstance()->DrawSettingCompute();

		dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(0, skinCluster_.paletteSrvHandle.second);
		dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(1, skinCluster_.inputSrvHandle.second);
		dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(2, skinCluster_.influenceSrvHandle.second);
		dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(3, skinCluster_.outputUavHandle.second);
		dxBasis_->GetCommandList()->SetComputeRootConstantBufferView(4, skinCluster_.skinningInformationResource->GetGPUVirtualAddress());

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = skinCluster_.outputResource.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		dxBasis_->GetCommandList()->ResourceBarrier(1, &barrier);

		dxBasis_->GetCommandList()->Dispatch(UINT(modelData_.vertices.size() + 1023) / 1024, 1, 1);

		D3D12_RESOURCE_BARRIER barriers{};
		barriers.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barriers.Transition.pResource = skinCluster_.outputResource.Get();
		barriers.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barriers.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barriers.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxBasis_->GetCommandList()->ResourceBarrier(1, &barriers);

		Object3dCommon::GetInstance()->DrawSettingCommon();

		// VBVを設定
		dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &skinCluster_.outputBufferView);

		break;
	}

	// IBVを設定
	dxBasis_->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	// マテリアルCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelData_.material.textureFilePath));
	// 描画
	dxBasis_->GetCommandList()->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);

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