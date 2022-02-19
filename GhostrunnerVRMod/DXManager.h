#pragma once

#include <functional>
#include <d3d11.h>
#include "Utilities/Globals.h"
#include "Utilities/MinHook.h"
#include "UEHelper.h"
#include "VRManager.h"
#include "PixelShader.h"
#include "VertexShader.h"
#pragma comment(lib, "d3d11.lib")

namespace GhostrunnerVR
{
	typedef void(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

	class DXManager
	{
	public:
		void HookDX();

		bool IsDXHooked{ false };

		DWORD_PTR* SwapChainVTable;

		D3D11PresentHook D3D11PresentTarget = NULL;

		static DXManager* Get();

	private:
		static DXManager* Ref;

		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;
	};
}