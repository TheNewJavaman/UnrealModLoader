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
		InitVR(pDevice, pRenderTargetView);
	}
	else
	{
		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0f;
		bounds.uMax = 1.0f;
		bounds.vMin = 0.0f;
		bounds.vMax = 1.0f;

		/*
		ID3D11Resource* leftResource;
		pLeftRenderViewTarget->GetResource(&leftResource);
		vr::Texture_t leftTexture = { (void*)&leftResource, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		pCompositor->Submit(vr::Eye_Left, &leftTexture, &bounds, vr::Submit_Default);

		ID3D11Resource* rightResource;
		pRightRenderViewTarget->GetResource(&rightResource);
		vr::Texture_t rightTexture = { (void*)&rightResource, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		pCompositor->Submit(vr::Eye_Right, &rightTexture, &bounds, vr::Submit_Default);
		*/

		ID3D11Resource* resource;
		pRenderTargetView->GetResource(&resource);
		vr::Texture_t texture = { (void*)&resource, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		pCompositor->Submit(vr::Eye_Left, &texture, &bounds, vr::Submit_Default);
		pCompositor->Submit(vr::Eye_Right, &texture, &bounds, vr::Submit_Default);
	}
}

void ExampleMod::OnModMenuButtonPressed()
{
}

void ExampleMod::DrawImGui()
{
}

void ExampleMod::InitVR(ID3D11Device* pDevice, ID3D11RenderTargetView* pFlatRenderTargetView)
{
	vr::HmdError error;
	pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
	pCompositor = vr::VRCompositor();
	uint32_t pnWidth;
	uint32_t pnHeight;
	pSystem->GetRecommendedRenderTargetSize(&pnWidth, &pnHeight);
	Log::Info("[GhostrunnerVRMod] Initialized VR system and compositor");

	/*
	ID3D11Resource* pResource;
	pFlatRenderTargetView->GetResource(&pResource);
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	pFlatRenderTargetView->GetDesc(&desc);
	pDevice->CreateRenderTargetView(pResource, &desc, &pLeftRenderViewTarget);
	pDevice->CreateRenderTargetView(pResource, &desc, &pRightRenderViewTarget);
	Log::Info("[GhostrunnerVRMod] Initialized VR render view targets");
	*/

	bVRStarted = true;
	Log::Info("[GhostrunnerVRMod] VR initialization complete");
}