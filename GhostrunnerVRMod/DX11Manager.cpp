#include "DX11Manager.h"

DX11Manager* DX11Manager::pDXManager;

DX11Manager* DX11Manager::GetDXManager()
{
	if (!pDXManager)
	{
		pDXManager = new DX11Manager();
	}
	return pDXManager;
}

bool InitVR()
{
	ExampleMod* mod = ExampleMod::GetMod();
	DX11Manager* dxManager = mod->pDXManager;
	if (!dxManager->bIsDXHooked)
	{
		return false;
	}

	vr::HmdError error;
	vr::IVRSystem* pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
	pSystem->GetRecommendedRenderTargetSize(&dxManager->pnWidth, &dxManager->pnHeight);
	Log::Info("[GhostrunnerVRMod] Initialized VR system and compositor");

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = dxManager->pnWidth;
	textureDesc.Height = dxManager->pnHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
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
	return true;
}

void SubmitToVR()
{
	ExampleMod* mod = ExampleMod::GetMod();
	DX11Manager* dxManager = mod->pDXManager;
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	vr::VRTextureBounds_t bounds;
	bounds.uMin = 0.0f;
	bounds.uMax = 1.0f;
	bounds.vMin = 0.0f;
	bounds.vMax = 1.0f;

	/*
	if (dxManager->eyeLastRendered == vr::Eye_Right)
	{
		vr::Texture_t leftTexture = { (void*)mod->pLeftTexture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture, &bounds, vr::Submit_Default);
		dxManager->eyeLastRendered = vr::Eye_Left;
		UE4Helpers::TriggerEvent(L"MoveCameraToEye", std::vector<std::wstring>{ L"1" });
	}
	else
	{
		vr::Texture_t rightTexture = { (void*)mod->pRightTexture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture, &bounds, vr::Submit_Default);
		dxManager->eyeLastRendered = vr::Eye_Right;
		UE4Helpers::TriggerEvent(L"MoveCameraToEye", std::vector<std::wstring>{ L"0" });
	}
	*/

	// Temporary debugging -- left & eyes are the same
	vr::Texture_t leftTexture = { (void*)mod->pLeftTexture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture, &bounds, vr::Submit_Default);
	vr::VRCompositor()->Submit(vr::Eye_Right, &leftTexture, &bounds, vr::Submit_Default);
}

void DoDrawOperations(ID3D11DeviceContext* pContext, std::function<void(void)> drawFunc)
{
	ExampleMod* mod = ExampleMod::GetMod();
	DX11Manager* dxManager = mod->pDXManager;
	if (!mod->bIsVrInitialized && !InitVR())
	{
		return;
	}
	ID3D11RenderTargetView* oldRTVs[1];
	ID3D11DepthStencilView* oldDSV;
	// TODO: Find out how many RTVs Ghostrunner uses
	pContext->OMGetRenderTargets(1, oldRTVs, &oldDSV);
	ID3D11RenderTargetView* vrRTVs[1];
	if (dxManager->eyeLastRendered == vr::Eye_Left) {
		vrRTVs[0] = mod->pRightRTV;
	}
	else
	{
		vrRTVs[0] = mod->pLeftRTV;
	}
 	if (oldRTVs[0] != vrRTVs[0])
	{
		if (!dxManager->bIsResolutionSet) 
		{
			UE4Helpers::TriggerEvent(L"SetResolutionForVR", std::vector<std::wstring>{ std::to_wstring(dxManager->pnWidth), std::to_wstring(dxManager->pnHeight) });
			dxManager->bIsResolutionSet = true;
		}
		pContext->OMSetRenderTargets(1, vrRTVs, oldDSV);

	}
	drawFunc();
	if (oldRTVs[0]) oldRTVs[0]->Release();
	if (oldDSV) oldDSV->Release();
}

HRESULT(*D3D11Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

HRESULT __stdcall HookDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	ExampleMod* mod = ExampleMod::GetMod();
	if (!mod->bIsVrInitialized && !InitVR())
	{
		return S_OK;
	}
	SubmitToVR();
	return S_OK;
}

HRESULT(*D3D11Draw) (ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);

HRESULT __stdcall HookDX11Draw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
{
	DoDrawOperations(pContext, [pContext, VertexCount, StartVertexLocation]() 
		{
			D3D11Draw(pContext, VertexCount, StartVertexLocation); 
		}
	);
	return S_OK;
}

HRESULT(*D3D11DrawIndexed) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

HRESULT __stdcall HookDX11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	DoDrawOperations(pContext, [pContext, IndexCount, StartIndexLocation, BaseVertexLocation]()
		{
			D3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
		}
	);
	return S_OK;
}

HRESULT(*D3D11DrawInstanced) (ID3D11DeviceContext* pContext, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);

HRESULT __stdcall HookDX11DrawInstanced(ID3D11DeviceContext* pContext, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
	DoDrawOperations(pContext, [pContext, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation]()
		{
			D3D11DrawInstanced(pContext, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
		}
	);
	return S_OK;
}

HRESULT(*D3D11DrawIndexedInstanced) (ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

HRESULT __stdcall HookDX11DrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	DoDrawOperations(pContext, [pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation]()
		{
			D3D11DrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
		}
	);
	return S_OK;
}

HRESULT(*D3D11DrawAuto) (ID3D11DeviceContext* pContext);

HRESULT __stdcall HookDX11DrawAuto(ID3D11DeviceContext* pContext)
{
	DoDrawOperations(pContext, [pContext]()
		{
			D3D11DrawAuto(pContext);
		}
	);
	return S_OK;
}

HRESULT(*D3D11DrawInstancedIndirect) (ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);

HRESULT __stdcall HookDX11DrawInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	DoDrawOperations(pContext, [pContext, pBufferForArgs, AlignedByteOffsetForArgs]()
		{
			D3D11DrawInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
		}
	);
	return S_OK;
}

HRESULT(*D3D11DrawIndexedInstancedIndirect) (ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);

HRESULT __stdcall HookDX11DrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	DoDrawOperations(pContext, [pContext, pBufferForArgs, AlignedByteOffsetForArgs]()
		{
			D3D11DrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
		}
	);
	return S_OK;
}

HRESULT(*D3D11IASetVertexBuffers) (ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets);

HRESULT __stdcall HookDX11IASetVertexBuffers(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets)
{
	D3D11IASetVertexBuffers(pContext, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
	return S_OK;
}

HRESULT(*D3D11VSSetConstantBuffers) (ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

HRESULT __stdcall HookDX11VSSetConstantBuffers(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
	D3D11VSSetConstantBuffers(pContext, StartSlot, NumBuffers, ppConstantBuffers);
	return S_OK;
}

HRESULT(*D3D11RSSetViewports) (ID3D11DeviceContext* pContext, UINT NumViewports, D3D11_VIEWPORT* pViewports);

HRESULT __stdcall HookDX11RSSetViewports(ID3D11DeviceContext* pContext, UINT NumViewports, D3D11_VIEWPORT* pViewports)
{
	/*
	DX11Manager* dxManager = DX11Manager::GetDXManager();
	for (UINT i = 0; i < NumViewports; i++)
	{
		pViewports[i].Width = dxManager->pnWidth;
		pViewports[i].Height = dxManager->pnHeight;
	}
	*/
	D3D11RSSetViewports(pContext, NumViewports, pViewports);
	return S_OK;
}

HRESULT(*D3D11RSSetScissorRects) (ID3D11DeviceContext* pContext, UINT NumRects, D3D11_RECT* pRects);

HRESULT __stdcall HookDX11RSSSetScissorRects(ID3D11DeviceContext* pContext, UINT NumRects, D3D11_RECT* pRects)
{
	/*
	DX11Manager* dxManager = DX11Manager::GetDXManager();
	for (UINT i = 0; i < NumRects; i++)
	{
		pRects[i].left = 0;
		pRects[i].top = 0;
		pRects[i].right = dxManager->pnWidth;
		pRects[i].bottom = dxManager->pnHeight;
	}
	*/
	D3D11RSSetScissorRects(pContext, NumRects, pRects);
	return S_OK;
}

DWORD __stdcall InitDX11Hook(LPVOID)
{
	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandle(L"dxgi.dll");
		Sleep(100);
	} while (!hDXGIDLL);
	Sleep(100);

	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);

	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DX11Manager* pDXManager = DX11Manager::GetDXManager();

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pDXManager->pSwapChain,
		&pDXManager->pDevice,
		&obtainedLevel,
		&pDXManager->pContext
	);
	Log::Info("[GhostrunnerVRMod] Created DX11 hardware");

	pDXManager->pSwapChainVTable = (DWORD_PTR*)pDXManager->pSwapChain;
	pDXManager->pSwapChainVTable = (DWORD_PTR*)pDXManager->pSwapChainVTable[0];
	pDXManager->pContextVTable = (DWORD_PTR*)pDXManager->pContext;
	pDXManager->pContextVTable = (DWORD_PTR*)pDXManager->pContextVTable[0];

	pDXManager->pHookD3D11Present = (D3D11PresentHook)pDXManager->pSwapChainVTable[8];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11Present, &HookDX11Present, &D3D11Present, "VR-DX11-Present");
	pDXManager->pHookD3D11Draw = (D3D11DrawHook)pDXManager->pContextVTable[13];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11Draw, &HookDX11Draw, &D3D11Draw, "VR-DX11-Draw");
	pDXManager->pHookD3D11DrawIndexed = (D3D11DrawIndexedHook)pDXManager->pContextVTable[12];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawIndexed, &HookDX11DrawIndexed, &D3D11DrawIndexed, "VR-DX11-DrawIndexed");
	pDXManager->pHookD3D11DrawInstanced = (D3D11DrawInstancedHook)pDXManager->pContextVTable[21];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawInstanced, &HookDX11DrawInstanced, &D3D11DrawInstanced, "VR-DX11-DrawInstanced");
	pDXManager->pHookD3D11DrawIndexedInstanced = (D3D11DrawIndexedInstancedHook)pDXManager->pContextVTable[20];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawIndexedInstanced, &HookDX11DrawIndexedInstanced, &D3D11DrawIndexedInstanced, "VR-DX11-DrawIndexedInstanced");
	pDXManager->pHookD3D11DrawAuto = (D3D11DrawAutoHook)pDXManager->pContextVTable[38];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawAuto, &HookDX11DrawAuto, &D3D11DrawAuto, "VR-DX11-DrawAuto");
	pDXManager->pHookD3D11DrawInstancedIndirect = (D3D11DrawInstancedIndirectHook)pDXManager->pContextVTable[40];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawInstancedIndirect, &HookDX11DrawInstancedIndirect, &D3D11DrawInstancedIndirect, "VR-DX11-DrawInstancedIndirect");
	pDXManager->pHookD3D11DrawIndexedInstancedIndirect = (D3D11DrawIndexedInstancedIndirectHook)pDXManager->pContextVTable[39];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawIndexedInstancedIndirect, &HookDX11DrawIndexedInstancedIndirect, &D3D11DrawIndexedInstancedIndirect, "VR-DX11-DrawIndexedInstancedIndirect");
	pDXManager->pHookD3D11RSSetViewports = (D3D11RSSetViewportsHook)pDXManager->pContextVTable[44];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11RSSetViewports, &HookDX11RSSetViewports, &D3D11RSSetViewports, "VR-DX11-RSSetViewports");
	pDXManager->pHookD3D11RSSetScissorRects = (D3D11RSSetScissorRectsHook)pDXManager->pContextVTable[45];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11RSSetScissorRects, &HookDX11RSSSetScissorRects, &D3D11RSSetScissorRects, "VR-DX11-RSSetScissorRects");

	DWORD dPresentOld;
	VirtualProtect(pDXManager->pHookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dPresentOld);
	DWORD dDrawOld;
	VirtualProtect(pDXManager->pHookD3D11Draw, 2, PAGE_EXECUTE_READWRITE, &dDrawOld);
	DWORD dDrawIndexedOld;
	VirtualProtect(pDXManager->pHookD3D11DrawIndexed, 2, PAGE_EXECUTE_READWRITE, &dDrawIndexedOld);
	DWORD dDrawInstancedOld;
	VirtualProtect(pDXManager->pHookD3D11DrawInstanced, 2, PAGE_EXECUTE_READWRITE, &dDrawInstancedOld);
	DWORD dDrawIndexedInstancedOld;
	VirtualProtect(pDXManager->pHookD3D11DrawIndexedInstanced, 2, PAGE_EXECUTE_READWRITE, &dDrawIndexedInstancedOld);
	DWORD dDrawAutoOld;
	VirtualProtect(pDXManager->pHookD3D11DrawAuto, 2, PAGE_EXECUTE_READWRITE, &dDrawAutoOld);
	DWORD dDrawInstancedIndirectOld;
	VirtualProtect(pDXManager->pHookD3D11DrawInstancedIndirect, 2, PAGE_EXECUTE_READWRITE, &dDrawInstancedIndirectOld);
	DWORD dDrawIndexedInstancedIndirectOld;
	VirtualProtect(pDXManager->pHookD3D11DrawIndexedInstancedIndirect, 2, PAGE_EXECUTE_READWRITE, &dDrawIndexedInstancedIndirectOld);
	DWORD dRSSetViewportsOld;
	VirtualProtect(pDXManager->pHookD3D11RSSetViewports, 2, PAGE_EXECUTE_READWRITE, &dRSSetViewportsOld);
	DWORD dRSSetScissorRectsOld;
	VirtualProtect(pDXManager->pHookD3D11RSSetScissorRects, 2, PAGE_EXECUTE_READWRITE, &dRSSetScissorRectsOld);

	Log::Info("[GhostrunnerVRMod] Added DX11 hooks");
	pDXManager->bIsDXHooked = true;

	while (true)
	{
		Sleep(10);
	}

	pDXManager->pDevice->Release();
	pDXManager->pContext->Release();
	pDXManager->pSwapChain->Release();
	return NULL;
}

void DX11Manager::HookDX11()
{
	if (!GetDXManager()->bIsDXHooked)
	{
		CreateThread(NULL, 0, InitDX11Hook, NULL, 0, NULL);
	}
}