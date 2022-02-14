#include "ExampleMod.h"
#include "Utilities/MinHook.h"

BPFUNCTION(WriteToFile)
{
	std::cout << "WriteToFile" << std::endl;
	struct InputParams
	{
		UE4::FString NameTest;
	};
	auto Inputs = stack->GetInputParams<InputParams>();
	stack->SetOutput<UE4::FString>("OutPutString", L"KboyGang");
	stack->SetOutput<bool>("ReturnValue", true);
}

// Only Called Once, if you need to hook shit, declare some global non changing values
void ExampleMod::InitializeMod()
{
	UE4::InitSDK();
	SetupHooks();

	REGISTER_FUNCTION(WriteToFile);

	//MinHook::Init(); //Uncomment if you plan to do hooks

	//UseMenuButton = true; // Allows Mod Loader To Show Button
	Log::Info("[GhostrunnerVRMod] Initialized mod");
}

void ExampleMod::InitGameState()
{
}

void ExampleMod::BeginPlay(UE4::AActor* Actor)
{
}

void ExampleMod::PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor)
{
	// Filters Out All Mod Actors Not Related To Your Mod
	std::wstring TmpModName(ModName.begin(), ModName.end());
	if (ModActorName == TmpModName)
	{
		//Sets ModActor Ref
		ModActor = Actor;
	}
}

void ExampleMod::DX11Present(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView)
{
	if (!bVRStarted)
	{
		InitVR(pDevice, pContext, pRenderTargetView);
	}
	else
	{
		vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
		pCompositor->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0f;
		bounds.uMax = 1.0f;
		bounds.vMin = 0.0f;
		bounds.vMax = 1.0f;

		vr::Texture_t leftTexture = { (void*)pLeftTexture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		pCompositor->Submit(vr::Eye_Left, &leftTexture, &bounds, vr::Submit_Default);
		vr::Texture_t rightTexture = { (void*)pRightTexture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		pCompositor->Submit(vr::Eye_Right, &rightTexture, &bounds, vr::Submit_Default);
	}
}

void ExampleMod::OnModMenuButtonPressed()
{
}

void ExampleMod::DrawImGui()
{
}

void ExampleMod::InitVR(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pFlatRTV)
{
	vr::HmdError error;
	pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
	pCompositor = vr::VRCompositor();
	uint32_t pnWidth;
	uint32_t pnHeight;
	pSystem->GetRecommendedRenderTargetSize(&pnWidth, &pnHeight);
	Log::Info("[GhostrunnerVRMod] Initialized VR system and compositor");

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = pnWidth;
	textureDesc.Height = pnHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	pDevice->CreateTexture2D(&textureDesc, NULL, &pLeftTexture);
	pDevice->CreateRenderTargetView(pLeftTexture, &rtvDesc, &pLeftRTV);
	pDevice->CreateTexture2D(&textureDesc, NULL, &pRightTexture);
	pDevice->CreateRenderTargetView(pRightTexture, &rtvDesc, &pRightRTV);
	Log::Info("[GhostrunnerVRMod] Initialized VR render view targets");

	ID3D11RenderTargetView* originalRTVs[3];
	ID3D11DepthStencilView* originalDSV;
	pContext->OMGetRenderTargets(3, originalRTVs, &originalDSV);
	if (originalRTVs[1] == nullptr)
	{
		ID3D11RenderTargetView* newRTVs[]{ originalRTVs[0], pLeftRTV, pRightRTV };
		pContext->OMSetRenderTargets(3, newRTVs, originalDSV);
	}
	else
	{
		originalRTVs[1]->Release();
		originalRTVs[2]->Release();
	}
	originalRTVs[0]->Release();
	if (originalDSV != nullptr)
	{
		originalDSV->Release();
	}
	Log::Info("[GhostrunnerVRMod] Added VR render view targets to the graphics pipeline");

	bVRStarted = true;
}