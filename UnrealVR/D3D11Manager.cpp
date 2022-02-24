#include "D3D11Manager.h"

namespace UnrealVR
{
	void D3D11Manager::SubmitToVR(IDXGISwapChain* pSwapChain)
	{
		ID3D11Texture2D* texture2D = NULL;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture2D);

		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);
		if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			VRManager::Get()->SubmitFrame(texture2D);
		}
		else if (desc.Format == DXGI_FORMAT_R10G10B10A2_UNORM)
		{
			ID3D11Texture2D* converted;
			Convert_R10G10B10A2UNORM(pSwapChain, texture2D, &converted);
			VRManager::Get()->SubmitFrame(converted);
			converted->Release();
		}
		else {
			vr::EVRCompositorError error = VRManager::Get()->SubmitFrame(texture2D);
			if (error != vr::VRCompositorError_None)
			{
				Log::Warn("[GhostrunnerVR] VR compositor error (" + std::to_string(error) + ")! This probably has to do with an unsupported texture format");
			}
		}

		texture2D->Release();
	}

	HRESULT __stdcall D3D11Manager::PresentDetour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (!VRManager::IsVRInitialized)
		{
			if (!VRManager::InitVR())
			{
				Log::Error("[UnrealVR] Failed to init VR");
			}
			return S_OK;
		}
		SubmitToVR(pSwapChain);
		if (VRManager::LastEyeRendered == vr::Eye_Left)
		{
			PresentOriginal(pSwapChain, SyncInterval, Flags);
		}
		return S_OK;
	}

	DWORD __stdcall D3D11Manager::HookThread(LPVOID)
	{
		HMODULE dxgiDLL = 0;
		do
		{
			dxgiDLL = GetModuleHandle(L"dxgi.dll");
			Sleep(100);
		} while (!dxgiDLL);
		Sleep(100);

		WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
		RegisterClassExA(&wc);
		HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

		D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0 };
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

		D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createFlags,
			requestedLevels,
			sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,
			&scd,
			&SwapChain,
			&Device,
			&obtainedLevel,
			&Context
		);
		Log::Info("[UnrealVR] Created D3D11 resources");

		SwapChainVTable = (DWORD_PTR*)((DWORD_PTR*)SwapChain)[0];

		PresentTarget = (PresentHook*)SwapChainVTable[8];
		MinHook::Add((DWORD64)PresentTarget, &PresentDetour, &PresentOriginal, "UnrealVR-D3D11-Present");
		
		DWORD presentOld;
		VirtualProtect(PresentTarget, 2, PAGE_EXECUTE_READWRITE, &presentOld);

		Log::Info("[UnrealVR] Added D3D11 hooks");
		IsHooked = true;

		while (true)
		{
			Sleep(10);
		}

		Context->Release();
		Device->Release();
		SwapChain->Release();

		return NULL;
	}

	void D3D11Manager::Hook()
	{
		if (!IsHooked)
		{
			CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
		}
	}
}