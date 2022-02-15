#pragma once

#include "Mod/Mod.h"
#include "openvr/openvr.h"
#include "DX11Manager.h"
#include "Utilities/MinHook.h"

class ExampleMod : public Mod
{
public:
	//Basic Mod Info
	ExampleMod()
	{
		ModName = "ExampleMod"; // Mod Name -- If Using BP ModActor, Should Be The Same Name As Your Pak
		ModVersion = "1.0.0"; // Mod Version
		ModDescription = "HAHAHAHA MOD GO BURR"; // Mod Description
		ModAuthors = "RussellJ"; // Mod Author
		ModLoaderVersion = "2.2.0";

		// Dont Touch The Internal Stuff
		ModRef = this;
		CompleteModCreation();
	}

	//Called When Internal Mod Setup is finished
	virtual void InitializeMod() override;

	//InitGameState Call
	virtual void InitGameState() override;

	//Beginplay Hook of Every Actor
	virtual void BeginPlay(UE4::AActor* Actor) override;

	//PostBeginPlay of EVERY Blueprint ModActor
	virtual void PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor) override;

	//DX11 hook for when an image will be presented to the screen
	virtual void DX11Present(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView) override;

	virtual void OnModMenuButtonPressed() override;

	//Call ImGui Here (CALLED EVERY FRAME ON DX HOOK)
	virtual void DrawImGui() override;

	typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	D3D11PresentHook phookD3D11Present = NULL;

	DWORD_PTR* pSwapChainVtable = NULL;
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;
	ID3D11RenderTargetView* pRenderTargetView = NULL;

	ID3D11Texture2D* pLeftTexture;
	ID3D11RenderTargetView* pLeftRTV;
	ID3D11Texture2D* pRightTexture;
	ID3D11RenderTargetView* pRightRTV;

	DX11Manager* pDXManager;
	bool bIsVrInitialized;

private:
	// If you have a BP Mod Actor, This is a straight refrence to it
	UE4::AActor* ModActor;
};