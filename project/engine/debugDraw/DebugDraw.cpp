#include "DebugDraw.h"
#include "DebugDrawCommon.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3d.h"
#include "BaseCharacter.h"
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

	case kBox:
		CreateVertexDataBox();
		break;
	}

	// カメラデータ作成
	CreateCameraResource();

	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	// オフセットを初期化
	offset_ = { 0.0f,0.0f,0.0f };
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
	modelDataLine.material.textureFilePath = filePath;

	vertexResourceLine = dxBasis_->CreateBufferResources(sizeof(VertexData) * kMaxVertexLine);

	// リソースの先頭のアドレスから使う
	vertexBufferViewLine.BufferLocation = vertexResourceLine->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点2つ分のサイズ
	vertexBufferViewLine.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * kMaxVertexLine);
	// 1頂点あたりのサイズ
	vertexBufferViewLine.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResourceLine->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataLine));
}

void DebugDraw::CreateVertexDataBox()
{
	// 立方体の8つの角の座標（中心 0,0,0 からの距離）
	modelDataBox.vertices.push_back({ .position = { -1.0f,  1.0f, -1.0f, 1.0f },.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 0: 左上奥
	modelDataBox.vertices.push_back({ .position = { 1.0f,  1.0f, -1.0f, 1.0f },.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 1: 右上奥
	modelDataBox.vertices.push_back({ .position = { -1.0f, -1.0f, -1.0f, 1.0f },.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} }); // 2: 左下奥
	modelDataBox.vertices.push_back({ .position = { 1.0f, -1.0f, -1.0f, 1.0f },.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} }); // 3: 右下奥
	modelDataBox.vertices.push_back({ .position = { -1.0f,  1.0f,  1.0f, 1.0f },.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 4: 左上正面
	modelDataBox.vertices.push_back({ .position = { 1.0f,  1.0f,  1.0f, 1.0f },.texcoord = {1.0f,1.0f},.normal = {0.0f,0.0f,1.0f} }); // 5: 右上正面
	modelDataBox.vertices.push_back({ .position = { -1.0f, -1.0f,  1.0f, 1.0f },.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 6: 左下正面
	modelDataBox.vertices.push_back({ .position = { 1.0f, -1.0f,  1.0f, 1.0f },.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 7: 右下正面
	modelDataBox.material.textureFilePath = filePath;

	// 頂点リソースを作成
	vertexResourceBox = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelDataBox.vertices.size());
	indexResource = dxBasis_->CreateBufferResources(sizeof(uint32_t) * 36);


	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferViewBox.BufferLocation = vertexResourceBox->GetGPUVirtualAddress();
	vertexBufferViewBox.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(modelDataBox.vertices.size());
	// 1頂点辺りのサイズ
	vertexBufferViewBox.StrideInBytes = sizeof(VertexData);

	uint32_t indices[] = {
		// 前面
		4, 5, 6, 7, 6, 5,
		// 後面
		1, 0, 3, 2, 3, 0,
		// 左面
		0, 4, 2, 6, 2, 4,
		// 右面
		5, 1, 7, 3, 7, 1,
		// 上面
		0, 1, 4, 5, 4, 1,
		// 下面
		6, 7, 2, 3, 2, 7
	};

	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

	// 頂点データを設定する
	vertexResourceBox->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataBox));
	// 頂点データにリソースをコピー
	std::memcpy(vertexDataBox, modelDataBox.vertices.data(), sizeof(VertexData) * modelDataBox.vertices.size());

	// 使用するリソースのサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 36;
	// 1頂点辺りのサイズ
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	// インデックスリソースにデータを書き込む
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices, sizeof(uint32_t) * 36);

}

void DebugDraw::CreateTransformMatrixData3d()
{
	// WVP用のリソースを作る
	transformationResource = dxBasis_->CreateBufferResources(sizeof(TransformationMatrix));
	transformationResourceLine = dxBasis_->CreateBufferResources(sizeof(TransformationMatrix));
	transformationResourceBox = dxBasis_->CreateBufferResources(sizeof(TransformationMatrix));

	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationData));
	transformationResourceLine->Map(0, nullptr, reinterpret_cast<void**>(&transformationDataLine));
	transformationResourceBox->Map(0, nullptr, reinterpret_cast<void**>(&transformationDataBox));

	// 単位行列を書き込んでおく
	transformationData->World = MakeIdentity4x4();
	transformationData->WVP = MakeIdentity4x4();
	transformationData->WorldInverseTranspose = Inverse(transformationData->World);
	transformationData->WorldInverseTranspose = Transpose(transformationData->WorldInverseTranspose);
	transformationDataLine->World = MakeIdentity4x4();
	transformationDataLine->WVP = MakeIdentity4x4();
	transformationDataLine->WorldInverseTranspose = Inverse(transformationDataLine->World);
	transformationDataLine->WorldInverseTranspose = Transpose(transformationDataLine->WorldInverseTranspose);
	transformationDataBox->World = MakeIdentity4x4();
	transformationDataBox->WVP = MakeIdentity4x4();
	transformationDataBox->WorldInverseTranspose = Inverse(transformationDataBox->World);
	transformationDataBox->WorldInverseTranspose = Transpose(transformationDataBox->WorldInverseTranspose);
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
	if (isRailCamera_)
	{
		transform.translate = Vector3Add(transform.translate, offset_);
	}
	Matrix4x4 worldMatrix = MakeAffineMatrixQuat(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();

		if (isRailCamera_)
		{
			Matrix4x4 cameraMatrix = camera->GetWorldMatrix();
			worldMatrix = Multiply(worldMatrix, cameraMatrix);
		}

		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;
	}

	if (parent)
	{
		// 親オブジェクトのTransformをかける
		Matrix4x4 parentWorldMatrix = MakeAffineMatrixQuat(parent->GetTransform().scale, parent->GetTransform().rotate, parent->GetTransform().translate);
		worldMatrix = Multiply(worldMatrix, parentWorldMatrix);
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
	if (isRailCamera_)
	{
		transform.translate = Vector3Add(transform.translate, offset_);
	}
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	Matrix4x4 worldViewProjectionMatrix;

	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();

		if (isRailCamera_)
		{
			Matrix4x4 cameraMatrix = camera->GetWorldMatrix();
			worldMatrix = Multiply(worldMatrix, cameraMatrix);
		}

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

void DebugDraw::UpdateBox()
{
	
	if (isRailCamera_)
	{
		transformBox.translate = Vector3Add(transformBox.translate, offset_);
	}
	Matrix4x4 worldMatrix = MakeAffineMatrixQuat(transformBox.scale, transformBox.rotate, transformBox.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (parent)
	{
		// 親オブジェクトのTransformをかける
		Matrix4x4 parentWorldMatrix = MakeAffineMatrixQuat(parent->GetObject3d()->GetTransform().scale, parent->GetObject3d()->GetTransform().rotate, parent->GetObject3d()->GetTransform().translate);
		worldMatrix = Multiply(worldMatrix, parentWorldMatrix);
	}

	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();

		if (isRailCamera_)
		{
			Matrix4x4 cameraMatrix = camera->GetWorldMatrix();
			worldMatrix = Multiply(worldMatrix, cameraMatrix);
		}

		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;
	}



	transformationDataBox->WVP = worldViewProjectionMatrix;
	transformationDataBox->World = worldMatrix;

	transformationDataBox->WorldInverseTranspose = Transpose(Inverse(transformationDataBox->World));
}


void DebugDraw::Draw()
{
	// 描画準備
	debugDraw_->DrawSettingCommonTriangle();
	// 形状を設定
	dxBasis_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
	// 描画準備
	debugDraw_->DrawSettingCommonLine();
	// テクスチャのSRVのDescriptorTableを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelDataLine.material.textureFilePath));
	// wvp用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResourceLine->GetGPUVirtualAddress());
	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewLine);
	// 描画
	dxBasis_->GetCommandList()->DrawInstanced((lineCount_ * 2), 1, 0, 0);
}

void DebugDraw::DrawBox()
{
	// 描画準備
	debugDraw_->DrawSettingCommonTriangle();
	// 形状を設定
	dxBasis_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// テクスチャのSRVのDescriptorTableを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(modelDataBox.material.textureFilePath));
	// wvp用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResourceBox->GetGPUVirtualAddress());
	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewBox);
	// IBVを設定
	dxBasis_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	// 描画
	dxBasis_->GetCommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void DebugDraw::AddLine(const Vector3& start, const Vector3& end)
{
	// 最大本数を超えたらスキップ
	if (lineCount_ >= kMaxLineCount)
	{
		return;
	}

	// 頂点数を記録
	uint32_t index = lineCount_ * 2;

	// 線の始点を設定
	vertexDataLine[index].position =
	{
		start.x,
		start.y,
		start.z,
		1.0f
	};

	// 線の終点を設定
	vertexDataLine[index + 1].position =
	{
		end.x,
		end.y,
		end.z,
		1.0f
	};

	// 現在の本数を加算
	++lineCount_;
}

void DebugDraw::ClearLine()
{
	lineCount_ = 0;
}