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
	if (!mod->pDXManager->bIsDXHooked)
	{
		return false;
	}

	vr::HmdError error;
	vr::IVRSystem* pSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
	pSystem->GetRecommendedRenderTargetSize(&mod->pDXManager->pnWidth, &mod->pDXManager->pnHeight);
	Log::Info("[GhostrunnerVRMod] Initialized VR system and compositor");
	mod->bIsVrInitialized = true;

	UE4Helpers::TriggerEvent(L"SetResolutionForVR", { std::to_wstring(mod->pDXManager->pnWidth), std::to_wstring(mod->pDXManager->pnHeight) });
	Log::Info("[GhostrunnerVRMod] Set resolution for VR");
	mod->pDXManager->bIsResolutionSet = true;

	return true;
}

void SubmitToVR(IDXGISwapChain* pSwapChain)
{
	ID3D11RenderTargetView* rtvs[1];
	ID3D11DepthStencilView* dsv;
	DX11Manager::GetDXManager()->pGameContext->OMGetRenderTargets(1, rtvs, &dsv);
	ID3D11Resource* resource;
	rtvs[0]->GetResource(&resource);
	ID3D11Texture2D* texture2D;
	resource->QueryInterface<ID3D11Texture2D>(&texture2D);

	vr::Texture_t texture = { (void*)texture2D, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
	vr::VRTextureBounds_t bounds;
	bounds.uMin = 0.0f;
	bounds.uMax = 1.0f;
	bounds.vMin = 0.0f;
	bounds.vMax = 1.0f;

	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds, vr::Submit_Default);
	vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds, vr::Submit_Default);

	texture2D->Release();
	rtvs[0]->Release();
	if (dsv) dsv->Release();
}

HRESULT(*D3D11Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

HRESULT __stdcall HookDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	ExampleMod* mod = ExampleMod::GetMod();
	if (!mod->bIsVrInitialized)
	{
		if (!InitVR())
		{
			return S_OK;
		}
	}
	SubmitToVR(pSwapChain);
	return S_OK;
}

HRESULT(*D3D11CreateDevicePtr) (IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

HRESULT __stdcall HookDX11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	DX11Manager::GetDXManager()->pGameContext = *ppImmediateContext;
	Log::Info("[GhostrunnerVR] Intercepted DX11 device and context");
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

	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };
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

	pDXManager->pHookD3D11Present = (D3D11PresentHook)pDXManager->pSwapChainVTable[8];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11Present, &HookDX11Present, &D3D11Present, "VR-DX11-Present");
	pDXManager->pHookD3D11CreateDevice = (D3D11CreateDeviceHook)D3D11CreateDevice;
	MinHook::Add((DWORD64)pDXManager->pHookD3D11CreateDevice, &HookDX11CreateDevice, &D3D11CreateDevicePtr, "VR-DX11-CreateDevice");

	DWORD dPresentOld;
	VirtualProtect(pDXManager->pHookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dPresentOld);
	DWORD dCreateDeviceOld;
	VirtualProtect(pDXManager->pHookD3D11CreateDevice, 2, PAGE_EXECUTE_READWRITE, &dCreateDeviceOld);

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