#pragma once

#include "ExampleMod.h"

void OnDraw(D3D11DrawOrig** orig, ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);

void OnDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

void OnDrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
