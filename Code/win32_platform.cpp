#include <windows.h>
#include <stdio.h>

#define Assert(condition) if(!(condition)) { *(unsigned int *)0 = 0; } 
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

static int
StringLength(char * String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return Count;
}

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define WND_WIDTH 800
#define WND_HEIGHT 600

#include "math.h"
#include "arena.h"
#include "arena.cpp"
#include "bitmap.h"
#include "bitmap.cpp"
#include "mesh.h"
#include "main.h"
#include "main.cpp"

struct window
{
    HWND Window;
    int Width;
    int Height;
};

struct mat4_constant_buffer
{
    mat4 World;
    mat4 Proj;
    mat4 View;
};

#include "directx.h"
#include "directx.cpp"

static bool GlobalRunning;
static HINSTANCE GlobalInstance;

LRESULT CALLBACK WndProc(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = {};
    switch(Message)
    {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        }break;
        case WM_DESTROY:
        {
            GlobalRunning = false;
        }break;
        default:
        {
           Result = DefWindowProcA(Window, Message, WParam, LParam);
        }break; 
    }
    return Result;
}

static void 
ProcesInputMessages()
{
    MSG Message = {};
    while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);   
            }break;
        }
    }
}

void PrintMat4(mat4 M)
{
    for(int Y = 0; Y < 4; Y++)
    {
        for(int X = 0; X < 4; X++)
        {
            char Buffer[64];
            sprintf(Buffer, "%f ", M.m[Y][X]);
            OutputDebugString(Buffer);
        }
        OutputDebugString("\n");
    }
 
}

static window *
CreatePlatformWindow(char * WindowName, int WindowWidth, int WindowHeight, arena *Arena)
{
    window *Window = (window *)PushStruct(Arena, window);
    
    // Define and Create The Application Window
    WNDCLASSEX WindowClass = { 0 };
    WindowClass.cbSize = sizeof( WNDCLASSEX ) ;
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WndProc;
    WindowClass.hInstance = GlobalInstance;
    WindowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
    WindowClass.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = WindowName;
    RegisterClassEx(&WindowClass);

    RECT Rect = { 0, 0, WindowWidth, WindowHeight };
    AdjustWindowRect( &Rect, WS_OVERLAPPEDWINDOW, FALSE );

    Window->Window = CreateWindowA(WindowName,
                                  WindowName,
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  Rect.right - Rect.left,
                                  Rect.bottom - Rect.top,
                                  NULL, NULL, GlobalInstance, NULL);
    Window->Width = WindowWidth;
    Window->Height = WindowHeight;
    return Window;

}

int WINAPI WinMain(HINSTANCE Instance,
                   HINSTANCE PrevInstance,
                   LPSTR     lpCmdLine,
                   int       nShowCmd)
{
    
    GlobalInstance = Instance;
    // Init the Application Storage
    app_memory AppMemory = {};
    AppMemory.Size = Megabytes(256);
    AppMemory.Memory = VirtualAlloc(0, AppMemory.Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);    
    GameSetUp(&AppMemory);

    // Create the Arenas to Orginize the Memory of the Application
    arena FileArena = {};
    InitArena(&AppMemory, &FileArena, Megabytes(20));


    window *Window = GetWindow(&AppMemory);
    renderer *Renderer = GetRenderer(&AppMemory);
    if(Window->Window)
    {     
        LARGE_INTEGER Frequency = {};
        QueryPerformanceFrequency(&Frequency);
        bool SleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        float FPS = 60.0f;
        float TARGET_SECONDS_FRAME = (1.0f / FPS);

        RECT ClientDimensions = {};
        GetClientRect(Window->Window, &ClientDimensions);
        unsigned int Width  = ClientDimensions.right - ClientDimensions.left;
        unsigned int Height = ClientDimensions.bottom - ClientDimensions.top;

        /////////////////////////////////////////////////////////////
        // Test Code: Create A cube and Pass it to the Grafic Card
        /////////////////////////////////////////////////////////////
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

        // Create the buffer settings
        D3D11_BUFFER_DESC CubeBufferDesc = {};
        CubeBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        CubeBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        CubeBufferDesc.ByteWidth = sizeof(float)*ArrayCount(Vertices);
        // Add the Vertices
        D3D11_SUBRESOURCE_DATA ResourceData = {};
        ResourceData.pSysMem = Vertices;
        // Create the Buffer
        ID3D11Buffer *CubeBuffer = 0;
        HRESULT Result = Renderer->Device->CreateBuffer(&CubeBufferDesc, &ResourceData, &CubeBuffer);
        if(SUCCEEDED(Result))
        {
            OutputDebugString("Cube Buffer Created!\n");
        }
        // Create Index Buffer
        D3D11_BUFFER_DESC IndexDesc = {};
        IndexDesc.Usage = D3D11_USAGE_DEFAULT;
        IndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        IndexDesc.ByteWidth = sizeof( int )*ArrayCount(Indices);
        IndexDesc.CPUAccessFlags = 0;
        ResourceData.pSysMem = Indices;

        ID3D11Buffer *CubeIndex = 0;
        Result = Renderer->Device->CreateBuffer(&IndexDesc, &ResourceData, &CubeIndex);
        if(SUCCEEDED(Result))
        {
            OutputDebugString("Cube Indices Created!\n");
        }
        /////////////////////////////////////////////////////////////



        // Creating Vertex and Pixel Shader
        ID3D11VertexShader *VertexShader = 0;
        ID3D11PixelShader *PixelShader = 0;
        ID3D11InputLayout *InputLayout = 0;
       
        D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
            0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        D3D11CreateVertexShader(Renderer->Device, "../Code/main_vertex_shader.hlsl", "VS_Main",
                                &VertexShader, &InputLayout, InputLayoutDesc,
                                ArrayCount(InputLayoutDesc), &FileArena);
        D3D11CreatePixelShader(Renderer->Device, "../Code/main_pixel_shader.hlsl", "PS_Main",
                               &PixelShader, &FileArena);
       
        InitMa4ConstBuffer(Renderer->Device);
        mat4 World = IdentityMat4();
        mat4 View = ViewMat4({0.0f, 2.0f, 5.0f}, {0.0f, 0.0f,  0.0f}, {0.0f, 1.0f,  0.0f});
        mat4 Proj = PerspectiveProjMat4(ToRad(90), (float)WND_WIDTH/(float)WND_HEIGHT, 0.1f, 100.0f);
        SetWorldMat4(Renderer->RenderContext, World);
        SetViewMat4(Renderer->RenderContext, View);
        SetProjectionMat4(Renderer->RenderContext, Proj);
        
        GlobalRunning = true;
        ShowWindow(Window->Window, nShowCmd);
        
        LARGE_INTEGER LastCount = {};
        QueryPerformanceCounter(&LastCount); 
        while(GlobalRunning)
        { 
            LARGE_INTEGER WorkCount = {};
            QueryPerformanceCounter(&WorkCount);
            unsigned long long DeltaWorkCount = WorkCount.QuadPart - LastCount.QuadPart;            
            float SecondElapseForFrame = ((float)DeltaWorkCount / (float)Frequency.QuadPart);
            while(SecondElapseForFrame < TARGET_SECONDS_FRAME)
            {                
                if(SleepIsGranular)
                {
                    DWORD SleepMS = (DWORD)(1000.0f*(TARGET_SECONDS_FRAME-SecondElapseForFrame));
                    if(SleepMS > 0)
                    {
                        Sleep(SleepMS);
                    }
                    QueryPerformanceCounter(&WorkCount);
                    DeltaWorkCount = WorkCount.QuadPart - LastCount.QuadPart;            
                    SecondElapseForFrame = ((float)DeltaWorkCount / (float)Frequency.QuadPart);
                }
            }
            LARGE_INTEGER ActualCount = {};
            QueryPerformanceCounter(&ActualCount);
            unsigned long long DeltaCount = ActualCount.QuadPart - LastCount.QuadPart;            
            float DeltaTime = ((float)DeltaCount / (float)Frequency.QuadPart);

            ProcesInputMessages();
                
            static float Time = 0;
            World = TranslationMat4({0.0f, sinf(Time)*2.0f, 0.0f}) * RotationYMat(Time);
            SetWorldMat4(Renderer->RenderContext, World);
            Time += DeltaTime;

            // Render...
            float ClearColor[4] = {0.0f, 0.0f, 0.3f, 1.0f};
            Renderer->RenderContext->ClearRenderTargetView(Renderer->BackBuffer, ClearColor);
            

            GameUpdateAndRender(&AppMemory);
             
            unsigned int Stride = sizeof(float)*8;
            unsigned int Offset = 0;
            Renderer->RenderContext->IASetInputLayout(InputLayout);
            Renderer->RenderContext->IASetVertexBuffers(0, 1, &CubeBuffer, &Stride, &Offset);
            Renderer->RenderContext->IASetIndexBuffer(CubeIndex, DXGI_FORMAT_R32_UINT, 0);
            Renderer->RenderContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            Renderer->RenderContext->VSSetShader(VertexShader, 0, 0);
            Renderer->RenderContext->PSSetShader(PixelShader,  0, 0);
            Renderer->RenderContext->DrawIndexed(36, 0, 0);

            Renderer->SwapChain->Present(0, 0);
            
            LastCount = ActualCount;

        }
        if(Renderer->BackBuffer) Renderer->BackBuffer->Release();
        if(Renderer->SwapChain) Renderer->SwapChain->Release();
        if(Renderer->RenderContext) Renderer->RenderContext->Release(); 
        if(Renderer->Device) Renderer->Device->Release(); 
    }
    return 0;
}


