#pragma once

#include "Utilities/MinHook.h"
#include <d3d11.h>
#include <functional>
#include "ExampleMod.h"
#pragma comment(lib, "d3d11.lib")

typedef void(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall* D3D11DrawHook) (ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);
typedef void(__stdcall* D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall* D3D11DrawInstancedHook) (ID3D11DeviceContext* pContext, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
typedef void(__stdcall* D3D11DrawIndexedInstancedHook) (ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void(__stdcall* D3D11DrawAutoHook) (ID3D11DeviceContext* pContext);
typedef void(__stdcall* D3D11DrawInstancedIndirectHook) (ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);
typedef void(__stdcall* D3D11DrawIndexedInstancedIndirectHook) (ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);
typedef void(__stdcall* D3D11RSSetViewportsHook) (ID3D11DeviceContext* pContext, UINT NumViewports, D3D11_VIEWPORT* pViewports);
typedef void(__stdcall* D3D11RSSetScissorRectsHook) (ID3D11DeviceContext* pContext, UINT NumRects, D3D11_RECT* pRects);

// Overwrite VS CB's first entry (assume to be WVP)

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
	DWORD_PTR* pContextVTable = NULL;

	D3D11PresentHook pHookD3D11Present = NULL;
	D3D11DrawHook pHookD3D11Draw = NULL;
	D3D11DrawIndexedHook pHookD3D11DrawIndexed = NULL;
	D3D11DrawInstancedHook pHookD3D11DrawInstanced = NULL;
	D3D11DrawIndexedInstancedHook pHookD3D11DrawIndexedInstanced = NULL;
	D3D11DrawAutoHook pHookD3D11DrawAuto = NULL;
	D3D11DrawInstancedIndirectHook pHookD3D11DrawInstancedIndirect = NULL;
	D3D11DrawIndexedInstancedIndirectHook pHookD3D11DrawIndexedInstancedIndirect = NULL;
	D3D11RSSetViewportsHook pHookD3D11RSSetViewports = NULL;
	D3D11RSSetScissorRectsHook pHookD3D11RSSetScissorRects = NULL;

	bool bAreRTVsSet;

	static DX11Manager* GetDXManager();

private:
	static DX11Manager* pDXManager;
};