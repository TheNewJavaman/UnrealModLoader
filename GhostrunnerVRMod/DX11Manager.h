#pragma once

#include "Utilities/MinHook.h"
#include <d3d11.h>
#include <functional>
#include "ExampleMod.h"
#include "UE4Helpers.hpp"
#pragma comment(lib, "d3d11.lib")

typedef void(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall* D3D11CreateDeviceHook) (IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

class ExampleMod;

class DX11Manager
{
public:
	void HookDX11();

	bool bIsDXHooked;

	uint32_t pnWidth;
	uint32_t pnHeight;

	ID3D11Device* pDevice = NULL;
	IDXGISwapChain* pSwapChain = NULL;
	DWORD_PTR* pSwapChainVTable = NULL;
	ID3D11DeviceContext* pContext = NULL;

	ID3D11DeviceContext* pGameContext = NULL;

	D3D11PresentHook pHookD3D11Present = NULL;
	D3D11CreateDeviceHook pHookD3D11CreateDevice = NULL;

	bool bIsResolutionSet;

	vr::EVREye eyeLastRendered = vr::Eye_Right;

	static DX11Manager* GetDXManager();

private:
	static DX11Manager* pDXManager;
};