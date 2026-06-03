#include "Object3d.h" 
#include "Object3dCommon.h"
#include "MathManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include <numbers>
#include "TextureManager.h"
#include "DebugDrawCommon.h"

using namespace MathManager;

using namespace MathManager;

void Object3d::Initialize(Object3dCommon* object3dManager)
{
	// 引数で受け取ってメンバ変数として記録する
	this->object3dManager = object3dManager;
	dxBasis_ = object3dManager->GetDxBasis();


	// デフォルトカメラをセット
	this->camera = object3dManager->GetDefaultCamera();

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

}

void Object3d::CreateTransformMatrixData3d()
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

void Object3d::CreateDirectionalLight()
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

void Object3d::CreatePointLight()
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

void Object3d::CreateSpotLight()
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

void Object3d::CreateCameraResource()
{
	// カメラリソースの生成
	cameraResource = dxBasis_->CreateBufferResources(sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	cameraData_->worldPosition = camera->GetTranslate();

}

void Object3d::SetModel(const std::string& filePath)
{
	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::SetAnimationModel(const std::string& directoryPath, const std::string& filename)
{
	// アニメーションデータを読み込む
	animation = Animation::GetInstance()->LoadAnimationFile(directoryPath, filename);

	// スケルトンデータを作成
	skeleton = Animation::GetInstance()->CreateSkeleton(model->GetModelData().rootNode);

#ifdef _DEBUG

	// デバッグ描画の初期化
	for (uint32_t i = 0; i < skeleton.joints.size(); ++i)
	{
		std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
		debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png",DebugDraw::DrawState::kSphere);
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

void Object3d::Update()
{
	// アニメーションを再生
	animationTime += 1.0f / 60.0f;
	animationTime = std::fmod(animationTime, animation.duration);

	/*Animation::NodeAnimation& rootNodeAnimation = animation.nodeAnimations[model->GetModelData().rootNode.name];
	Vector3 translate = Animation::GetInstance()->CalculateValue(rootNodeAnimation.translate, animationTime);
	Quaternion rotate = Animation::GetInstance()->CalculateValueQuaternion(rootNodeAnimation.rotate, animationTime);
	Vector3 scale = Animation::GetInstance()->CalculateValue(rootNodeAnimation.scale, animationTime);
	Matrix4x4 localMatrix = MakeAffineMatrixQuat(scale, rotate, translate);*/

	Animation::GetInstance()->ApplyAnimation(skeleton, animation, animationTime);
	Animation::GetInstance()->Update(skeleton);

	
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

	/*transformationData->WVP = Multiply(model->GetModelData().rootNode.localMatrix, worldViewProjectionMatrix);
	transformationData->World = Multiply(model->GetModelData().rootNode.localMatrix, worldMatrix);
	transformationData->WVP = Multiply(localMatrix,worldViewProjectionMatrix);
	transformationData->World = Multiply(localMatrix, worldMatrix);*/
	transformationData->WVP = worldViewProjectionMatrix;
	transformationData->World = worldMatrix;

	transformationData->WorldInverseTranspose = Transpose(Inverse(transformationData->World));
	if (camera)
	{
		cameraData_->worldPosition = camera->GetTranslate();
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

#ifdef USE_IMGUI
	ImGui::Begin("SpotLight");
	ImGui::DragFloat3("pos", &spotLightData->position.x);
	ImGui::SliderFloat("intensity", &spotLightData->intensity, 0.0f, 10.0f);
	ImGui::SliderFloat("cosFalloffStart", &spotLightData->cosFalloffStart, 0.0f, 2.0f);
	ImGui::SliderFloat("cosAngle", &spotLightData->cosAngle, -1.0f, 1.0f);

	if (spotLightData->cosFalloffStart < spotLightData->cosAngle)
	{
		spotLightData->cosAngle = spotLightData->cosFalloffStart;
	}

	ImGui::End();

#endif // USE_IMGUI


}

void Object3d::Draw()
{

	// wvp用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResource->GetGPUVirtualAddress());
	// 平行光源用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 点光源用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	// スポットライト用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	// カメラリソース用のCBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(7, TextureManager::GetInstance()->GetSRVHandleGPU(environmentMapTextureFilePath));


	// 3Dモデルが割り当てられていれば描画する
	if (model)
	{
		model->Draw();
	}

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