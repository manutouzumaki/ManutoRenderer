#ifndef DIRECTX_H
#define DIRECTX_H

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <d3dcompiler.h>

struct renderer
{
    ID3D11Device *Device;
    ID3D11DeviceContext *RenderContext;
    IDXGISwapChain *SwapChain;
    ID3D11RenderTargetView *BackBuffer;
};

struct mesh
{
    ID3D11Buffer *VertexBuffer;
    ID3D11Buffer *IndexBuffer;
};

#endif
