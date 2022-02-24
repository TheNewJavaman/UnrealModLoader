#pragma once

#include <d3d11.h>
#include "Utilities/Globals.h"
#include "Utilities/MinHook.h"
#include "VRManager.h"

namespace UnrealVR
{

	class D3D11Manager
	{
	public:
		static void Hook();
		static bool IsHooked;

	private:
		static DWORD __stdcall HookThread(LPVOID);

		static IDXGISwapChain* SwapChain;
		static ID3D11Device* Device;
		static ID3D11DeviceContext* Context;

		static DWORD_PTR* SwapChainVTable;

		typedef HRESULT(__stdcall PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
		static PresentHook PresentDetour;
		static PresentHook* PresentTarget;
		static PresentHook* PresentOriginal;
		static void SubmitToVR(IDXGISwapChain* pSwapChain);
	};
}