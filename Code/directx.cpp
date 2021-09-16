static mat4_constant_buffer GlobalMat4ConstBuffer; 
static ID3D11Buffer *GlobalMat4Buffer;

static void
D3D11Initialize(HWND Window,
                ID3D11Device **Device,
                ID3D11DeviceContext **RenderContext,
                IDXGISwapChain **SwapChain,
                ID3D11RenderTargetView **BackBuffer,
                ID3D11DepthStencilView **DepthStencilView,
                unsigned int WindowWidth,
                unsigned int WindowHeight)
{
    // -1: Define the device types and feature level we want to check for.
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_SOFTWARE
    };
    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    unsigned int DriverTypesCout = ArrayCount(DriverTypes);
    unsigned int FeatureLevelsCount = ArrayCount(FeatureLevels);
 
    // -2: Create the Direct3D device, rendering context, and swap chain.
    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    SwapChainDesc.BufferCount = 1;
    SwapChainDesc.BufferDesc.Width = WindowWidth;
    SwapChainDesc.BufferDesc.Height = WindowHeight;
    SwapChainDesc.BufferDesc.Format = /*DXGI_FORMAT_R8G8B8A8_UNORM;*/ DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.Windowed = true;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
   
    D3D_FEATURE_LEVEL FeatureLevel = {}; 
    D3D_DRIVER_TYPE DriverType = {};
    HRESULT Result = {};
    for(unsigned int Driver = 0;
        Driver < DriverTypesCout;
        ++Driver)
    {
        Result = D3D11CreateDeviceAndSwapChain(NULL, DriverTypes[Driver], NULL, 0,
                                               FeatureLevels, FeatureLevelsCount,
                                               D3D11_SDK_VERSION, &SwapChainDesc, 
                                               SwapChain, Device, &FeatureLevel,
                                               RenderContext);
        if(SUCCEEDED(Result))
        {
            DriverType = DriverTypes[Driver];
            break;
        } 
    }

    // -3: Create render target.
    ID3D11Texture2D *BackBufferTexture = 0;

    Result = (*SwapChain)->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&BackBufferTexture);
    Result = (*Device)->CreateRenderTargetView(BackBufferTexture, 0, BackBuffer);
    if(BackBufferTexture)
    {
        BackBufferTexture->Release();
    }

    ID3D11Texture2D* DepthTexture = 0;
    D3D11_TEXTURE2D_DESC DepthTexDesc = {};
    DepthTexDesc.Width = WindowWidth;
    DepthTexDesc.Height = WindowHeight;
    DepthTexDesc.MipLevels = 1;
    DepthTexDesc.ArraySize = 1;
    DepthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthTexDesc.SampleDesc.Count = 1;
    DepthTexDesc.SampleDesc.Quality = 0;
    DepthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthTexDesc.CPUAccessFlags = 0;
    DepthTexDesc.MiscFlags = 0; 
    
    Result = (*Device)->CreateTexture2D(&DepthTexDesc, NULL, &DepthTexture);
    // create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC DescDSV = {};
    DescDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DescDSV.Texture2D.MipSlice = 0;

    Result = (*Device)->CreateDepthStencilView(DepthTexture, &DescDSV, DepthStencilView);
    if(DepthTexture)
    {
        DepthTexture->Release();
    }
    
    (*RenderContext)->OMSetRenderTargets(1, BackBuffer, *DepthStencilView);

    // Turn on Alpha blending
    ID3D11BlendState* AlphaBlend = 0;
    D3D11_BLEND_DESC BlendStateDesc = {};
    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    (*Device)->CreateBlendState(&BlendStateDesc, &AlphaBlend);

    (*RenderContext)->OMSetBlendState(AlphaBlend, 0, 0xffffffff);

    // -4: Set the viewport.
    D3D11_VIEWPORT Viewport;
    Viewport.Width  = (float)WindowWidth;
    Viewport.Height = (float)WindowHeight;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    Viewport.TopLeftX = 0.0f;
    Viewport.TopLeftY = 0.0f;
    (*RenderContext)->RSSetViewports(1, &Viewport);
}

static ID3DBlob *
D3D11CompileShader(char *FileName, char *MainFuncName, char *ShaderVersion, arena *Arena)
{
    char * FileData = (char *)ReadEntireFile(FileName, NULL, Arena); 
    ID3DBlob *ShaderCompiled = 0;
    ID3DBlob *ErrorShader    = 0;

    int Length = StringLength(FileData);

    HRESULT Result = D3DCompile((void *)FileData,
                                (SIZE_T)StringLength(FileData),
                                0, 0, 0, MainFuncName, ShaderVersion,
                                D3DCOMPILE_ENABLE_STRICTNESS, 0,
                                &ShaderCompiled, &ErrorShader);
    if(ErrorShader == NULL)
    {
        return ShaderCompiled;
    }
    OutputDebugString((char *)ErrorShader->GetBufferPointer());
    ErrorShader->Release();
    return NULL;
}

static bool
D3D11CreateVertexShader(ID3D11Device *Device, char *FileName, char *MainFuncName,
                        ID3D11VertexShader **VertexShader,
                        ID3D11InputLayout **InputLayout,
                        D3D11_INPUT_ELEMENT_DESC *InputLayoutDesc,
                        unsigned int TotalLayoutElements, arena *Arena)
{
    HRESULT Result;
    ID3DBlob *ShaderCompiled = D3D11CompileShader(FileName, MainFuncName, "vs_4_0", Arena);
    Result = Device->CreateVertexShader(ShaderCompiled->GetBufferPointer(),
                                        ShaderCompiled->GetBufferSize(), 0,
                                        VertexShader);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    Result = Device->CreateInputLayout(InputLayoutDesc,
                                       TotalLayoutElements,
                                       ShaderCompiled->GetBufferPointer(),
                                       ShaderCompiled->GetBufferSize(),
                                       InputLayout);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    ShaderCompiled->Release();
    OutputDebugString("VERTEX_SHADER::CREATED\n");
    return true;
}

static bool
D3D11CreatePixelShader(ID3D11Device *Device, char *FileName, char *MainFuncName,
                       ID3D11PixelShader **PixelShader, arena *Arena)
{
    HRESULT Result;
    ID3DBlob *ShaderCompiled = D3D11CompileShader(FileName, MainFuncName, "ps_4_0", Arena);
    Result = Device->CreatePixelShader(ShaderCompiled->GetBufferPointer(),
                                       ShaderCompiled->GetBufferSize(), 0,
                                       PixelShader);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    ShaderCompiled->Release();
    OutputDebugString("PIXEL_SHADER::CREATED\n");
    return true;
}

static void 
InitMa4ConstBuffer(renderer *Renderer)
{
    // Create constant Buffers and  
    D3D11_BUFFER_DESC ConstantBufferDesc;
    ConstantBufferDesc.ByteWidth = sizeof(mat4_constant_buffer);
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.MiscFlags = 0;
    ConstantBufferDesc.StructureByteStride = 0;
    HRESULT Result = Renderer->Device->CreateBuffer(&ConstantBufferDesc, 0, &GlobalMat4Buffer);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("Mat4Buffer Created!\n");
    }
}

#define MapConstantBuffer(RenderContext, ConstBuffer, Type, Buffer) \
    do { \
        D3D11_MAPPED_SUBRESOURCE GPUConstantBufferData = {}; \
        RenderContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &GPUConstantBufferData); \
        memcpy(GPUConstantBufferData.pData, &ConstBuffer, sizeof(Type)); \
        RenderContext->Unmap(Buffer, 0); \
        RenderContext->VSSetConstantBuffers( 0, 1, &Buffer); \
    } while(0);

static void
SetWorldMat4(renderer *Renderer, mat4 World)
{
    GlobalMat4ConstBuffer.World = World;
    MapConstantBuffer(Renderer->RenderContext, GlobalMat4ConstBuffer,
                      mat4_constant_buffer, GlobalMat4Buffer);
}

static void
SetProjectionMat4(renderer *Renderer, mat4 Projection)
{
    GlobalMat4ConstBuffer.Proj = Projection;
    MapConstantBuffer(Renderer->RenderContext, GlobalMat4ConstBuffer,
                      mat4_constant_buffer, GlobalMat4Buffer);
}

static void
SetViewMat4(renderer *Renderer, mat4 View)
{
    GlobalMat4ConstBuffer.View = View;
    MapConstantBuffer(Renderer->RenderContext, GlobalMat4ConstBuffer,
                      mat4_constant_buffer, GlobalMat4Buffer); 
}

static mesh *
LoadCube(renderer *Renderer, arena *Arena)
{
    mesh *Mesh = (mesh *)PushStruct(Arena, mesh);
    float Vertices[] = {
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        
        1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f
    };
    unsigned int Indices[] =
    {
        3,1,0,2,1,3,
        6,4,5,7,4,6,
        11,9,8, 10,9, 11,
        14, 12, 13, 15, 12, 14,
        19, 17, 16, 18, 17, 19,
        22, 20, 21, 23, 20, 22
    };

    D3D11_BUFFER_DESC VertexBufferDesc = {};
    VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VertexBufferDesc.ByteWidth = sizeof(float)*ArrayCount(Vertices);
    // Add the Vertices
    D3D11_SUBRESOURCE_DATA ResourceData = {};
    ResourceData.pSysMem = Vertices;
    // Create the Buffer
    HRESULT Result = Renderer->Device->CreateBuffer(&VertexBufferDesc, &ResourceData, &Mesh->VertexBuffer);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("Vertex Buffer Created!\n");
    }
    // Create Index Buffer
    Mesh->IndexCount = ArrayCount(Indices);
    D3D11_BUFFER_DESC IndexBufferDesc = {};
    IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.ByteWidth = sizeof( int )*ArrayCount(Indices);
    IndexBufferDesc.CPUAccessFlags = 0;
    ResourceData.pSysMem = Indices;

    Result = Renderer->Device->CreateBuffer(&IndexBufferDesc, &ResourceData, &Mesh->IndexBuffer);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("Index Buffer Created!\n");
    }
    
    return Mesh;
}

static mesh *
LoadMesh(char *OBJFileName, renderer *Renderer, arena *Arena)
{
    mesh *Mesh = (mesh *)PushStruct(Arena, mesh);
    obj OBJ = LoadOBJFile(OBJFileName, Arena);

    Mesh->VertexCount = OBJ.VerticesCount;
    D3D11_BUFFER_DESC VertexBufferDesc = {};
    VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VertexBufferDesc.ByteWidth = sizeof(float)*OBJ.VerticesCount;
    // Add the Vertices
    D3D11_SUBRESOURCE_DATA ResourceData = {};
    ResourceData.pSysMem = OBJ.Vertices;
    // Create the Buffer
    HRESULT Result = Renderer->Device->CreateBuffer(&VertexBufferDesc, &ResourceData, &Mesh->VertexBuffer);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("Vertex Buffer Created!\n");
    }
    return Mesh;
}

static texture *
LoadTexture(char *TextureFileName, renderer *Renderer, arena *Arena)
{
    texture *Texture = (texture *)PushStruct(Arena, texture);
    bit_map Bitmap = LoadBMP(TextureFileName, Arena);

    D3D11_SUBRESOURCE_DATA Data = {};
    Data.pSysMem = (void *)Bitmap.Pixels;
    Data.SysMemPitch = Bitmap.Width*sizeof(unsigned int);
    Data.SysMemSlicePitch = 0;

    D3D11_TEXTURE2D_DESC TextureDesc = {}; 
    TextureDesc.Width = Bitmap.Width;
    TextureDesc.Height = Bitmap.Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;//DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    ID3D11Texture2D *TempTexture;
    HRESULT Result = Renderer->Device->CreateTexture2D(&TextureDesc, &Data, &TempTexture);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("SUCCEEDED Creating texture\n");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceDesc = {};
    ShaderResourceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;
    ShaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceDesc.Texture2D.MostDetailedMip = 0;
    ShaderResourceDesc.Texture2D.MipLevels = 1;
    Result = Renderer->Device->CreateShaderResourceView(TempTexture, &ShaderResourceDesc, &Texture->ColorMap);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("SUCCEEDED Creating Shader resource view\n");
    }
    TempTexture->Release();

    D3D11_SAMPLER_DESC ColorMapDesc = {};
    ColorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    ColorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    ColorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    ColorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    ColorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; //D3D11_FILTER_MIN_MAG_MIP_LINEAR | D3D11_FILTER_MIN_MAG_MIP_POINT
    ColorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Result = Renderer->Device->CreateSamplerState(&ColorMapDesc, &Texture->ColorMapSampler);
    if(SUCCEEDED(Result))
    {
        OutputDebugString("SUCCEEDED Creating sampler state\n");
    }
    return Texture;
}

static void
SetTexture(texture *Texture, renderer *Renderer)
{
    Renderer->RenderContext->PSSetShaderResources(0, 1, &Texture->ColorMap);
    Renderer->RenderContext->PSSetSamplers(0, 1, &Texture->ColorMapSampler);
}

static void
RenderMesh(mesh *Mesh, shader *Shader, renderer *Renderer)
{
    unsigned int Stride = sizeof(float)*8;
    unsigned int Offset = 0;
    Renderer->RenderContext->IASetInputLayout(Shader->InputLayout);
    Renderer->RenderContext->IASetVertexBuffers(0, 1, &Mesh->VertexBuffer, &Stride, &Offset);
    Renderer->RenderContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer->RenderContext->VSSetShader(Shader->VertexShader, 0, 0);
    Renderer->RenderContext->PSSetShader(Shader->PixelShader,  0, 0);
    Renderer->RenderContext->Draw(Mesh->VertexCount/8, 0);
}


static void
RenderMeshIndexed(mesh *Mesh, shader *Shader, renderer *Renderer)
{
    unsigned int Stride = sizeof(float)*8;
    unsigned int Offset = 0;
    Renderer->RenderContext->IASetInputLayout(Shader->InputLayout);
    Renderer->RenderContext->IASetVertexBuffers(0, 1, &Mesh->VertexBuffer, &Stride, &Offset);
    Renderer->RenderContext->IASetIndexBuffer(Mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Renderer->RenderContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer->RenderContext->VSSetShader(Shader->VertexShader, 0, 0);
    Renderer->RenderContext->PSSetShader(Shader->PixelShader,  0, 0);
    Renderer->RenderContext->DrawIndexed(Mesh->IndexCount, 0, 0);
}

