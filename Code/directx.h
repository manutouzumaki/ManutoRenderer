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
    ID3D11DepthStencilView* DepthStencilView;
};

struct shader
{
    ID3D11VertexShader *VertexShader;
    ID3D11PixelShader *PixelShader;
    ID3D11InputLayout *InputLayout;
};

struct mesh
{
    ID3D11Buffer *VertexBuffer;
    unsigned int VertexCount;
    ID3D11Buffer *IndexBuffer;
    unsigned int IndexCount;
};

#endif
