#include "GhostrunnerVRMod.h"
#include "Utilities/MinHook.h"

// Only Called Once, if you need to hook shit, declare some global non changing values
void GhostrunnerVRMod::InitializeMod()
{
	UE4::InitSDK();
	SetupHooks();

	//MinHook::Init(); //Uncomment if you plan to do hooks

	//UseMenuButton = true; // Allows Mod Loader To Show Button

	Log::Info("[GhostrunnerVRMod] Initialized mod");
}

void GhostrunnerVRMod::PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor)
{
	// Filters Out All Mod Actors Not Related To Your Mod
	std::wstring TmpModName(ModName.begin(), ModName.end());
	if (ModActorName == TmpModName)
	{
		//Sets ModActor Ref
		ModActor = Actor;
	}
}

void GhostrunnerVRMod::DX11Present(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView)
{
	Log::Info("[GhostrunnerVRMod] Received DX11 Present event");
}
