#include "ExampleMod.h"

namespace GhostrunnerVRModCallbacks
{
	void OnDraw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
	{
		Log::Info("draw called");
	}

	void OnDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		Log::Info("draw indexed called");
	}

	void OnDrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		Log::Info("draw indexed instanced called");
	}
}

// Only Called Once, if you need to hook shit, declare some global non changing values
void ExampleMod::InitializeMod()
{
	UE4::InitSDK();
	SetupHooks();

	MinHook::Init(); //Uncomment if you plan to do hooks

	//UseMenuButton = true; // Allows Mod Loader To Show Button

	pDXManager = DX11Manager::GetDXManager();
	pDXManager->HookDX11();

	Global::GetGlobals()->eventSystem.registerEvent(new Event<ID3D11DeviceContext*, UINT, UINT>("DX11Draw", &GhostrunnerVRModCallbacks::OnDraw));
	Global::GetGlobals()->eventSystem.registerEvent(new Event<ID3D11DeviceContext*, UINT, UINT, INT>("DX11DrawIndexed", &GhostrunnerVRModCallbacks::OnDrawIndexed));
	Global::GetGlobals()->eventSystem.registerEvent(new Event<ID3D11DeviceContext*, UINT, UINT, UINT, INT, UINT>("DX11DrawIndexedInstanced", &GhostrunnerVRModCallbacks::OnDrawIndexedInstanced));

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
}

void ExampleMod::OnModMenuButtonPressed()
{
}

void ExampleMod::DrawImGui()
{
}