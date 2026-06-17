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

using namespace MathManager;

void Model::Initialize(ModelCommon* modelManager, const std::string& directoryPath, const std::string& filePath)
{
	// 引数で受け取ってメンバ変数として記録する
	this->modelManager = modelManager;
	dxBasis_ = modelManager->GetDxBasis();

	// モデル読み込み
	modelData = LoadModelFile(directoryPath, filePath);

	// objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	// 読み込んだテクスチャの番号を取得
	modelData.material.textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

	// 頂点データ作成
	CreateVertexData3d();

	// マテリアルデータ作成
	CreateMaterialData3d();

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
	vertexResource = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelData.vertices.size());
	indexResource = dxBasis_->CreateBufferResources(sizeof(uint32_t) * modelData.indices.size());

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

void Model::CreateMaterialData3d()
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



void Model::Draw()
{
	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	// IBVを設定
	dxBasis_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	// マテリアルCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelData.material.textureFilePath));
	// 描画
	dxBasis_->GetCommandList()->DrawIndexedInstanced(UINT(modelData.indices.size()), 1, 0, 0, 0);
}