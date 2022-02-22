#include "DXManager.h"

HRESULT(*D3D11Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

namespace GhostrunnerVR
{
	DXManager* DXManager::Ref;

	DXManager* DXManager::Get()
	{
		if (!Ref)
		{
			Ref = new DXManager();
		}
		return Ref;
	}

	void Convert_R10G10B10A2UNORM(IDXGISwapChain* pSwapChain, ID3D11Texture2D* input, ID3D11Texture2D** output)
	{
		if (!DXManager::Get()->Convert.IsInitialized)
		{
			pSwapChain->GetDevice(_uuidof(ID3D11Device), (void**)&DXManager::Get()->Convert.Device);
			DXManager::Get()->Convert.Device->GetImmediateContext(&DXManager::Get()->Convert.Context);
		
			DXManager::Get()->Convert.Device->CreateVertexShader(VertexShaderBytecode, ARRAYSIZE(VertexShaderBytecode), NULL, &DXManager::Get()->Convert.VS);
			DXManager::Get()->Convert.Device->CreatePixelShader(PixelShaderBytecode, ARRAYSIZE(PixelShaderBytecode), NULL, &DXManager::Get()->Convert.PS);
		
			DXManager::Get()->Convert.IsInitialized = true;
		}

		D3D11_TEXTURE2D_DESC textureDesc;
		input->GetDesc(&textureDesc);
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		DXManager::Get()->Convert.Device->CreateTexture2D(&textureDesc, NULL, output);

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		ID3D11RenderTargetView* rtv;
		DXManager::Get()->Convert.Device->CreateRenderTargetView(*output, &rtvDesc, &rtv);
		
		input->GetDesc(&textureDesc);
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		ID3D11Texture2D* copy;
		DXManager::Get()->Convert.Device->CreateTexture2D(&textureDesc, NULL, &copy);
		DXManager::Get()->Convert.Context->CopyResource(copy, input);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		ID3D11ShaderResourceView* srv;
		DXManager::Get()->Convert.Device->CreateShaderResourceView(copy, &srvDesc, &srv);

		DXManager::Get()->Convert.Context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
		DXManager::Get()->Convert.Context->IASetIndexBuffer(NULL, (DXGI_FORMAT)0, 0);
		DXManager::Get()->Convert.Context->IASetInputLayout(NULL);
		DXManager::Get()->Convert.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DXManager::Get()->Convert.Context->VSSetShader(DXManager::Get()->Convert.VS, NULL, 0);
		DXManager::Get()->Convert.Context->PSSetShader(DXManager::Get()->Convert.PS, NULL, 0);

		DXManager::Get()->Convert.Context->OMSetRenderTargets(1, &rtv, nullptr);
		DXManager::Get()->Convert.Context->PSSetShaderResources(0, 1, &srv);

		DXManager::Get()->Convert.Context->Draw(3, 0);

		srv->Release();
		copy->Release();
		rtv->Release();
	}

	void SubmitToVR(IDXGISwapChain* pSwapChain)
	{
		ID3D11Texture2D* texture2D = NULL;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture2D);

		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);
		Log::Info(std::to_string(desc.Format));
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

	HRESULT __stdcall D3D11PresentDetour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (!VRManager::Get()->IsVRInitialized)
		{
			VRManager::Get()->InitVR();
			return S_OK;
		}
		SubmitToVR(pSwapChain);
		if (VRManager::Get()->LastEyeRendered == vr::Eye_Left)
		{
			D3D11Present(pSwapChain, SyncInterval, Flags);
		}
		return S_OK;
	}

	DWORD __stdcall InitDXHook(LPVOID)
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

		IDXGISwapChain* swapChain;
		ID3D11Device* device;
		ID3D11DeviceContext* context;

		D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createFlags,
			requestedLevels,
			sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,
			&scd,
			&swapChain,
			&device,
			&obtainedLevel,
			&context
		);
		Log::Info("[GhostrunnerVR] Created DX11 hardware");

		DXManager::Get()->SwapChainVTable = (DWORD_PTR*)swapChain;
		DXManager::Get()->SwapChainVTable = (DWORD_PTR*)DXManager::Get()->SwapChainVTable[0];

		DXManager::Get()->D3D11PresentTarget = (D3D11PresentHook)DXManager::Get()->SwapChainVTable[8];
		MinHook::Add((DWORD64)DXManager::Get()->D3D11PresentTarget, &D3D11PresentDetour, &D3D11Present, "VR-DX11-Present");

		DWORD dD3D11PresentOld;
		VirtualProtect(DXManager::Get()->D3D11PresentTarget, 2, PAGE_EXECUTE_READWRITE, &dD3D11PresentOld);

		Log::Info("[GhostrunnerVR] Added DX11 hooks");
		DXManager::Get()->IsDXHooked = true;

		while (true)
		{
			Sleep(10);
		}

		context->Release();
		device->Release();
		swapChain->Release();

		return NULL;
	}

	void DXManager::HookDX()
	{
		if (!IsDXHooked)
		{
			CreateThread(NULL, 0, InitDXHook, NULL, 0, NULL);
		}
	}
}