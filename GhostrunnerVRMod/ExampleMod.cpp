#include "ExampleMod.h"

namespace GhostrunnerVRModCallbacks
{
	void InitVR()
	{
		ExampleMod* mod = (ExampleMod*)ExampleMod::ModRef;
		DX11Manager* dxManager = mod->pDXManager;
		if (!dxManager->bIsDXHooked)
		{
			return;
		}

		vr::HmdError error;
		vr::IVRSystem* pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
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

		dxManager->pDevice->CreateTexture2D(&textureDesc, NULL, &mod->pLeftTexture);
		dxManager->pDevice->CreateRenderTargetView(mod->pLeftTexture, &rtvDesc, &mod->pLeftRTV);
		dxManager->pDevice->CreateTexture2D(&textureDesc, NULL, &mod->pRightTexture);
		dxManager->pDevice->CreateRenderTargetView(mod->pRightTexture, &rtvDesc, &mod->pRightRTV);
		Log::Info("[GhostrunnerVRMod] Initialized VR render view targets");

		mod->bIsVrInitialized = true;
	}

	void OnDraw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
	{
		ExampleMod* mod = (ExampleMod*)ExampleMod::ModRef;
		if (!mod->bIsVrInitialized) 
		{
			InitVR();
		}
		/*
		ID3D11CommandList* commands;
		pContext->FinishCommandList(true, &commands);
		ID3D11RenderTargetView* oldRTVs[1];
		ID3D11DepthStencilView* oldDSV;
		// TODO: Find out how many RTVs Ghostrunner uses
		pContext->OMGetRenderTargets(1, oldRTVs, &oldDSV);
		ID3D11RenderTargetView* vrRTVs[]{ mod->pLeftRTV, mod->pRightRTV };
		pContext->OMSetRenderTargets(2, vrRTVs, oldDSV);
		pContext->Draw(VertexCount, StartVertexLocation);
		pContext->OMSetRenderTargets(1, oldRTVs, oldDSV);
		oldDSV->Release();
		pContext->ExecuteCommandList(commands, false);
		commands->Release();
		*/
	}

	void OnDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		ExampleMod* mod = (ExampleMod*)ExampleMod::ModRef;
		if (!mod->bIsVrInitialized)
		{
			InitVR();
		}
		/*
		ID3D11CommandList* commands;
		pContext->FinishCommandList(true, &commands);
		ID3D11RenderTargetView* oldRTVs[1];
		ID3D11DepthStencilView* oldDSV;
		// TODO: Find out how many RTVs Ghostrunner uses
		pContext->OMGetRenderTargets(1, oldRTVs, &oldDSV);
		ID3D11RenderTargetView* vrRTVs[]{ mod->pLeftRTV, mod->pRightRTV };
		pContext->OMSetRenderTargets(2, vrRTVs, oldDSV);
		pContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
		pContext->OMSetRenderTargets(1, oldRTVs, oldDSV);
		oldDSV->Release();
		pContext->ExecuteCommandList(commands, false);
		commands->Release();
		*/
	}

	void OnDrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		ExampleMod* mod = (ExampleMod*)ExampleMod::ModRef;
		if (!mod->bIsVrInitialized)
		{
			InitVR();
		}
		/*
		ID3D11CommandList* commands;
		pContext->FinishCommandList(true, &commands);
		ID3D11RenderTargetView* oldRTVs[1];
		ID3D11DepthStencilView* oldDSV;
		// TODO: Find out how many RTVs Ghostrunner uses
		pContext->OMGetRenderTargets(1, oldRTVs, &oldDSV);
		ID3D11RenderTargetView* vrRTVs[]{ mod->pLeftRTV, mod->pRightRTV };
		pContext->OMSetRenderTargets(2, vrRTVs, oldDSV);
		pContext->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
		pContext->OMSetRenderTargets(1, oldRTVs, oldDSV);
		oldDSV->Release();
		pContext->ExecuteCommandList(commands, false);
		commands->Release();
		*/

		// Submit to compositor
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