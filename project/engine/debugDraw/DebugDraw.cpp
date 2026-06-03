#include "DebugDraw.h"
#include "DebugDrawCommon.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
using namespace MathManager;

void DebugDraw::Initialize(DebugDrawCommon* debugDrawCommon, std::string textureFilePath, DrawState state)
{
	// 引数で受け取ってメンバ変数に記録する
	this->debugDraw_ = debugDrawCommon;
	dxBasis_ = debugDraw_->GetDxBasis();
	filePath = textureFilePath;

	// デフォルトカメラをセット
	this->camera = debugDraw_->GetDefaultCamera();

	// 座標変換行列データ作成
	CreateTransformMatrixData3d();

	// 頂点データを作成
	switch (state)
	{
	case kSphere:
		CreateVertexData();
		break;

	case kLine:
		CreateVertexDataLine();
		break;
	}

	// カメラデータ作成
	CreateCameraResource();

	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}

void DebugDraw::CreateVertexData()
{
	
	// 経度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex)
	{
		// 緯度の方向に分割しながら線を描く
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex)
		{
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			float lon1 = (lonIndex + 1) * kLonEvery;
			float lat = float(-M_PI) / 2.0f + kLatEvery * float(latIndex);
			float lat1 = lat + kLatEvery;
			float v0 = 1.0f - float(latIndex) / float(kSubdivision);
			float v1 = 1.0f - float(latIndex + 1) / float(kSubdivision);
			float u0 = float(lonIndex) / float(kSubdivision);
			float u1 = float(lonIndex + 1) / float(kSubdivision);

			// 頂点にデータを入力する
			// 1つ目
			modelData.vertices.push_back({ .position = {cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon),1.0f},.texcoord = {u0,v0},.normal = {0.0f,0.0f,1.0f} });
			modelData.vertices.push_back({ .position = {cos(lat1) * cos(lon),sin(lat1),cos(lat1) * sin(lon),1.0f},.texcoord = {u0,v1},.normal = {0.0f,0.0f,1.0f} });
			modelData.vertices.push_back({ .position = {cos(lat) * cos(lon1),sin(lat),cos(lat) * sin(lon1),1.0f},.texcoord = {u1,v0},.normal = {0.0f,0.0f,1.0f} });
			modelData.vertices.push_back({ .position = {cos(lat) * cos(lon1),sin(lat),cos(lat) * sin(lon1),1.0f},.texcoord = {u1,v0},.normal = {0.0f,0.0f,1.0f} });
			modelData.vertices.push_back({ .position = {cos(lat1) * cos(lon),sin(lat1),cos(lat1) * sin(lon),1.0f},.texcoord = {u0,v1},.normal = {0.0f,0.0f,1.0f} });
			modelData.vertices.push_back({ .position = {cos(lat1) * cos(lon1),sin(lat1),cos(lat1) * sin(lon1),1.0f},.texcoord = {u1,v1},.normal = {0.0f,0.0f,1.0f} });
		}
	}
	modelData.material.textureFilePath = filePath;

	vertexResource = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelData.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexData));
	// 頂点データにリソースをコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

void DebugDraw::CreateVertexDataLine()
{
	modelDataLine.vertices.push_back({ .position = {0.5f,1.0f,0.0f,1.0f},.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });
	modelDataLine.vertices.push_back({ .position = {-0.5f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });
	modelDataLine.vertices.push_back({ .position = {0.5f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });
	modelDataLine.vertices.push_back({ .position = {0.5f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });
	modelDataLine.vertices.push_back({ .position = {-0.5f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });
	modelDataLine.vertices.push_back({ .position = {-0.5f,-1.0f,0.0f,1.0f},.texcoord = {1.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });
	modelDataLine.material.textureFilePath = filePath;

	vertexResourceLine = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelDataLine.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferViewLine.BufferLocation = vertexResourceLine->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewLine.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelDataLine.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferViewLine.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResourceLine->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexDataLine));
	// 頂点データにリソースをコピー
	std::memcpy(vertexDataLine, modelDataLine.vertices.data(), sizeof(VertexData) * modelDataLine.vertices.size());
}

void DebugDraw::CreateTransformMatrixData3d()
{
	// WVP用のリソースを作る
	transformationResource = dxBasis_->CreateBufferResources(sizeof(TransformationMatrix));
	transformationResourceLine = dxBasis_->CreateBufferResources(sizeof(TransformationMatrix));

	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationData));
	transformationResourceLine->Map(0, nullptr, reinterpret_cast<void**>(&transformationDataLine));

	// 単位行列を書き込んでおく
	transformationData->World = MakeIdentity4x4();
	transformationData->WVP = MakeIdentity4x4();
	transformationData->WorldInverseTranspose = Inverse(transformationData->World);
	transformationData->WorldInverseTranspose = Transpose(transformationData->WorldInverseTranspose);
	transformationDataLine->World = MakeIdentity4x4();
	transformationDataLine->WVP = MakeIdentity4x4();
	transformationDataLine->WorldInverseTranspose = Inverse(transformationDataLine->World);
	transformationDataLine->WorldInverseTranspose = Transpose(transformationDataLine->WorldInverseTranspose);
}

void DebugDraw::CreateCameraResource()
{
	// カメラリソースの生成
	cameraResource = dxBasis_->CreateBufferResources(sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	cameraData_->worldPosition = camera->GetTranslate();
}


void DebugDraw::Update()
{	
	Matrix4x4 worldMatrix = MakeAffineMatrixQuat(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationData->WVP = worldViewProjectionMatrix;
	transformationData->World = worldMatrix;

	transformationData->WorldInverseTranspose = Transpose(Inverse(transformationData->World));

	
	if (camera)
	{
		cameraData_->worldPosition = camera->GetTranslate();
	}

	
}

void DebugDraw::UpdateLine()
{
	Matrix4x4 worldMatrix = MakeAffineMatrixQuat(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationDataLine->WVP = worldViewProjectionMatrix;
	transformationDataLine->World = worldMatrix;

	transformationDataLine->WorldInverseTranspose = Transpose(Inverse(transformationDataLine->World));

}


void DebugDraw::Draw()
{
	// 描画準備
	debugDraw_->DrawSettingCommon();
	// テクスチャのSRVのDescriptorTableを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelData.material.textureFilePath));
	// wvp用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResource->GetGPUVirtualAddress());
	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	// 描画
	dxBasis_->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

void DebugDraw::DrawLine()
{
	// テクスチャのSRVのDescriptorTableを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelDataLine.material.textureFilePath));
	// wvp用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResourceLine->GetGPUVirtualAddress());
	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewLine);
	// 描画
	dxBasis_->GetCommandList()->DrawInstanced(UINT(modelDataLine.vertices.size()), 1, 0, 0);
}

