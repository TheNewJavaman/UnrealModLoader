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

HRESULT(*D3D11Draw) (ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);

HRESULT __stdcall HookDX11Draw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
{
	Global::GetGlobals()->eventSystem.dispatchEvent("DX11Draw", pContext, VertexCount, StartVertexLocation);
	return D3D11Draw(pContext, VertexCount, StartVertexLocation);
}

HRESULT(*D3D11DrawIndexed) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

HRESULT __stdcall HookDX11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	Global::GetGlobals()->eventSystem.dispatchEvent("DX11DrawIndexed", pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
	return D3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

HRESULT(*D3D11DrawIndexedInstanced) (ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

HRESULT __stdcall HookDX11DrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	Global::GetGlobals()->eventSystem.dispatchEvent("DX11DrawIndexedInstanced", pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	return D3D11DrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
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

	IDXGISwapChain* pSwapChain;

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
		&pSwapChain,
		&pDXManager->pDevice,
		&obtainedLevel,
		&pDXManager->pContext
	);

	pDXManager->pContextVTable = (DWORD_PTR*)pDXManager->pContext;
	pDXManager->pContextVTable = (DWORD_PTR*)pDXManager->pContextVTable[0];

	pDXManager->pHookD3D11Draw = (D3D11DrawHook)pDXManager->pContextVTable[13];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11Draw, &HookDX11Draw, &D3D11Draw, "DX11-Draw");
	pDXManager->pHookD3D11DrawIndexed = (D3D11DrawIndexedHook)pDXManager->pContextVTable[12];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawIndexed, &HookDX11DrawIndexed, &D3D11DrawIndexed, "DX11-DrawIndexed");
	pDXManager->pHookD3D11DrawIndexedInstanced = (D3D11DrawIndexedInstancedHook)pDXManager->pContextVTable[20];
	MinHook::Add((DWORD64)pDXManager->pHookD3D11DrawIndexedInstanced, &HookDX11DrawIndexedInstanced, &D3D11DrawIndexedInstanced, "DX11-DrawIndexedInstanced");

	DWORD dDrawOld;
	VirtualProtect(pDXManager->pHookD3D11Draw, 2, PAGE_EXECUTE_READWRITE, &dDrawOld);
	DWORD dDrawIndexedOld;
	VirtualProtect(pDXManager->pHookD3D11DrawIndexed, 2, PAGE_EXECUTE_READWRITE, &dDrawIndexedOld);
	DWORD dDrawIndexedInstancedOld;
	VirtualProtect(pDXManager->pHookD3D11DrawIndexedInstanced, 2, PAGE_EXECUTE_READWRITE, &dDrawIndexedInstancedOld);

	while (true)
	{
		Sleep(10);
	}

	pDXManager->pDevice->Release();
	pDXManager->pContext->Release();
	pSwapChain->Release();
	return NULL;
}

void DX11Manager::HookDX11()
{
	if (!GetDXManager()->bIsDXHooked)
	{
		CreateThread(NULL, 0, InitDX11Hook, NULL, 0, NULL);
		GetDXManager()->bIsDXHooked = true;
	}
}