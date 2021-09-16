#include <windows.h>
#include <stdio.h>

#include "math.h"
#include "arena.h"
#include "bitmap.h"
#include "mesh.h"
#include "platform.h"
#include "main.h"

#include "arena.cpp"
#include "bitmap.cpp"
#include "mesh.cpp"

struct window
{
    HWND Window;
    int Width;
    int Height;
};

#include "directx.h"
#include "directx.cpp"

#include "main.cpp"

static bool GlobalRunning;
static HINSTANCE GlobalInstance;

// function that pass data from the game to the platform specific layer 
static window *
GetWindow(app_memory *Memory)
{ 
    game_state *GameState = (game_state *)Memory->Memory;
    if(GameState->Window)
    {
        return GameState->Window;
    }
    return NULL;
}

static renderer *
GetRenderer(app_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->Memory;
    if(GameState->Renderer)
    {
        return GameState->Renderer;
    }
    return NULL;
}

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

PLATFORM_CREATE_WINDOW(PlatformCreateWindow)
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

PLATFORM_CREATE_RENDERER(PlatformCreateRenderer)
{
    renderer *Renderer = (renderer *)PushStruct(Arena, renderer);
    D3D11Initialize(Window->Window, &Renderer->Device,
                    &Renderer->RenderContext, &Renderer->SwapChain,
                    &Renderer->BackBuffer, &Renderer->DepthStencilView,
                    Window->Width, Window->Height);
    InitMa4ConstBuffer(Renderer);
    return Renderer;
}


// in the furture pass more information for the input layout desc
PLATFORM_CREATE_SHADERS_FROM_FILE(PlatformCreateShadersFromFile)
{
        shader *Shader = (shader *)PushStruct(Arena, shader); 
        D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
            0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        D3D11CreateVertexShader(Renderer->Device, VertexShaderFileName, VSMainFunc,
                                &Shader->VertexShader, &Shader->InputLayout, InputLayoutDesc,
                                ArrayCount(InputLayoutDesc), Arena);
        D3D11CreatePixelShader(Renderer->Device, PixelShaderFileName, PSMainFunc,
                               &Shader->PixelShader, Arena);
        return Shader;
}

PLATFORM_ALLOC_MEMORY(PlatformAllocMemory)
{
    void *Result = NULL; 
    Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return Result;    
}

PLATFORM_FREE_MEMORY(PlatformFreeMemory)
{
    VirtualFree(Memory, 0, MEM_RELEASE);
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

            // Render...
            float ClearColor[4] = {0.0f, 0.0f, 0.3f, 1.0f};
            Renderer->RenderContext->ClearRenderTargetView(Renderer->BackBuffer, ClearColor);
            Renderer->RenderContext->ClearDepthStencilView(Renderer->DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

            GameUpdateAndRender(&AppMemory, DeltaTime);

            Renderer->SwapChain->Present(0, 0);
            
            LastCount = ActualCount;

        }
        if(Renderer->DepthStencilView) Renderer->DepthStencilView->Release();
        if(Renderer->BackBuffer) Renderer->BackBuffer->Release();
        if(Renderer->SwapChain) Renderer->SwapChain->Release();
        if(Renderer->RenderContext) Renderer->RenderContext->Release(); 
        if(Renderer->Device) Renderer->Device->Release(); 
    }
    return 0;
}


