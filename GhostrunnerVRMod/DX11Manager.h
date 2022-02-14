#pragma once

#include "Utilities/MinHook.h"
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

typedef void(__stdcall* D3D11DrawHook) (ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);
typedef void(__stdcall* D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall* D3D11DrawIndexedInstancedHook) (ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

class DX11Manager
{
public:
	void HookDX11();

	bool bIsDXHooked;

	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;
	DWORD_PTR* pContextVTable = NULL;

	D3D11DrawHook pHookD3D11Draw = NULL;
	D3D11DrawIndexedHook pHookD3D11DrawIndexed = NULL;
	D3D11DrawIndexedInstancedHook pHookD3D11DrawIndexedInstanced = NULL;

	static DX11Manager* GetDXManager();

private:
	static DX11Manager* pDXManager;
};