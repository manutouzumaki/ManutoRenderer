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
    ID3D11RasterizerState *WireFrameRasterizer;
    ID3D11RasterizerState *FillRasterizerCullBack;
    ID3D11RasterizerState *FillRasterizerCullFront;
    ID3D11DepthStencilState *DepthStencilOn;
    ID3D11DepthStencilState *DepthStencilOff;
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
    ID3D11Buffer *IndexBuffer;
    unsigned int VertexCount;
    float *Vertices;
    unsigned int IndexCount;
    int *Indices;
};


struct mat4_constant_buffer
{
    mat4 World;
    mat4 Proj;
    mat4 View;
    v3 ViewPosition;
};

struct texture
{
    ID3D11ShaderResourceView *ColorMap;
    ID3D11SamplerState *ColorMapSampler;
};

#endif
