#include <windows.h>
#include <Windowsx.h>
#include <shobjidl.h>

#include <stdio.h>

#include "math.h"
#include "arena.h"
#include "bitmap.h"
#include "mesh.h"
#include "bounding_volumes.h"
#include "platform.h"
#include "arc_camera.h"
#include "main.h"

#include "arena.cpp"
#include "bitmap.cpp"
#include "mesh.cpp"
#include "bounding_volumes.cpp"
#include "arc_camera.cpp"

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
static int GlobalWindowPosX;
static int GlobalWindowPosY;

#if 1
static COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"Source Code (*.cpp)",   L"*.cpp"},
    {L"Header File (*.h)",     L"*.h"},
    {L"Text Document (*.txt)", L"*.txt"},
    {L"All Documents (*.*)",   L"*.*"}
};

#define INDEX_SOURCE_CODE_FILES 1
#define INDEX_HEADER_FILES      2
#define INDEX_TEXT_FILES        3
#define INDEX_ALL_FILES         4
#endif


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
        case WM_MOVE:
        {
            GlobalWindowPosX = (int)(short)LOWORD(LParam); 
            GlobalWindowPosY = (int)(short)HIWORD(LParam); 
        }
        default:
        {
           Result = DefWindowProcA(Window, Message, WParam, LParam);
        }break; 
    }
    return Result;
}

static void 
ProcesInputMessages(app_input *Input, mouse_buttons *ActualMouseButtons, mouse_buttons *OldMouseButtons,
                    keys *ActualKeys, keys *OldKeys)
{
    for(int Index = 0;
        Index < 256;
        ++Index)
    {
        ActualKeys->Keys[Index].IsDown = false;
    }

    MSG Message = {};
    while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                DWORD VKCode = (DWORD)Message.wParam;
                bool IsDown = ((Message.lParam & (1 << 31)) == 0);
                bool WasDown = ((Message.lParam & (1 << 30)) != 0);
                if(VKCode < 0) VKCode = 0;
                if(VKCode > 255) VKCode = 255;
                                
                if(IsDown && !WasDown)
                {
                    ActualKeys->Keys[VKCode].IsDown = true;
                }
            }
            break;
            case WM_MOUSEMOVE:
            {
                Input->MouseX = (int)GET_X_LPARAM(Message.lParam); 
                Input->MouseY = (int)GET_Y_LPARAM(Message.lParam); 
            }break;
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            {
                ActualMouseButtons->Buttons[0].IsDown = ((Message.wParam & MK_LBUTTON) != 0);
                ActualMouseButtons->Buttons[1].IsDown = ((Message.wParam & MK_MBUTTON) != 0);
                ActualMouseButtons->Buttons[2].IsDown = ((Message.wParam & MK_RBUTTON) != 0);
                ActualMouseButtons->Buttons[3].IsDown = ((Message.wParam & MK_SHIFT) != 0);

            }break;
            case WM_MOUSEWHEEL:
            {
                Input->MouseWheel = GET_WHEEL_DELTA_WPARAM(Message.wParam);
            }break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);   
            }break;
        }
    }
    for(int MouseIndex = 0;
        MouseIndex < 4;
        ++MouseIndex)
    {
        if(OldMouseButtons->Buttons[MouseIndex].IsDown)
        {  
            ActualMouseButtons->Buttons[MouseIndex].WasDown = 1;
        }
        else
        { 
            ActualMouseButtons->Buttons[MouseIndex].WasDown = 0;
        }
    }
}

#if 1
PLATFORM_OPEN_FILE_EXPLORER(BasicFileOpenTest)
{
    // cocreate the file open dialog object
    IFileDialog *FileDialog = NULL;
    HRESULT Result = CoCreateInstance(CLSID_FileOpenDialog,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&FileDialog));
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    
    // set the options on the dialog
    DWORD Flags;

    // before setting, always get the options first in order
    // not to override existing options
    Result = FileDialog->GetOptions(&Flags);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    // in this case, get shell items only for file system items
    Result = FileDialog->SetOptions(Flags | FOS_FORCEFILESYSTEM);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    // set the file type to display only
    // notice that this is a 1-based array
    Result = FileDialog->SetFileTypes(ArrayCount(c_rgSaveTypes), c_rgSaveTypes);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    Result = FileDialog->SetFileTypeIndex(INDEX_ALL_FILES);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    // set the default extencion to be ".cpp" file.
    Result = FileDialog->SetDefaultExtension(L"cpp");
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    // show the dialog
    Result = FileDialog->Show(NULL);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    // obtain the result once the user clicks
    // the 'Open' button
    // the result if an IShellItem object
    IShellItem *SearchResult;
    Result = FileDialog->GetResult(&SearchResult);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    // we are just going to print out the
    // name of the file for sample sake
    wchar_t *FilePath = NULL;
    Result = SearchResult->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);
    if(!SUCCEEDED(Result))
    {
        return false;
    }
    /*
    TaskDialog(NULL, NULL, L"CommonFileDialogApp", FilePath, NULL,
               TDCBF_OK_BUTTON, TD_INFORMATION_ICON, NULL);
    */

    *FileData = ReadEntireFileUnicode(FilePath, NULL, Arena);
    
    CoTaskMemFree(FilePath);

    SearchResult->Release();

    FileDialog->Release();

    return true;
}
#endif

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
                                  0, 0,
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
                    &Renderer->WireFrameRasterizer,
                    &Renderer->FillRasterizerCullBack,
                    &Renderer->FillRasterizerCullFront,
                    &Renderer->DepthStencilOn,
                    &Renderer->DepthStencilOff,
                    Window->Width, Window->Height);
    InitConstBuffers(Renderer);
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

PLATFORM_SHOW_CURSOR(PlatformShowCursor)
{
    ShowCursor(Value);
}

PLATFORM_SET_CURSOR_POSITION(PlatformSetCursorPosition)
{
    SetCursorPos(PosX, PosY);
}

int WINAPI WinMain(HINSTANCE Instance,
                   HINSTANCE PrevInstance,
                   LPSTR     lpCmdLine,
                   int       nShowCmd)
{
    
    GlobalInstance = Instance;
    // Init the Application Storage
    app_memory AppMemory = {};
    AppMemory.Size = Megabytes(512);
    AppMemory.Memory = VirtualAlloc(0, AppMemory.Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);   
    GameSetUp(&AppMemory);
    
    app_input AppInput = {};
    mouse_buttons ActualMouseButtons = {};
    mouse_buttons OldMouseButtons = {};
    keys ActualKeys = {};
    keys OldKeys = {};
     

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
    
        //AppInput.MouseDefaultX = WND_WIDTH / 2;
        //AppInput.MouseDefaultY = WND_HEIGHT / 2;

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

            ProcesInputMessages(&AppInput, &ActualMouseButtons, &OldMouseButtons, &ActualKeys, &OldKeys);
            AppInput.MouseButtons = &ActualMouseButtons;
            AppInput.KeyboardKeys = &ActualKeys;
            AppInput.MouseDefaultX = (WND_WIDTH / 2) + GlobalWindowPosX;
            AppInput.MouseDefaultY = (WND_HEIGHT / 2) + GlobalWindowPosY;

            // Render...
            float ClearColor[4] = {0.0f, 0.0f, 0.3f, 1.0f};
            Renderer->RenderContext->ClearRenderTargetView(Renderer->BackBuffer, ClearColor);
            Renderer->RenderContext->ClearDepthStencilView(Renderer->DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

            GameUpdateAndRender(&AppMemory, &AppInput, DeltaTime);

            Renderer->SwapChain->Present(0, 0);
            
            OldMouseButtons = ActualMouseButtons;
            OldKeys = ActualKeys;
            AppInput.MouseWheel = 0;
            LastCount = ActualCount;

        }

        if(Renderer->DepthStencilOff) Renderer->DepthStencilOff->Release();
        if(Renderer->DepthStencilOn) Renderer->DepthStencilOn->Release();
        if(Renderer->WireFrameRasterizer) Renderer->WireFrameRasterizer->Release();
        if(Renderer->FillRasterizerCullFront) Renderer->FillRasterizerCullFront->Release();
        if(Renderer->FillRasterizerCullBack) Renderer->FillRasterizerCullBack->Release();
        if(Renderer->DepthStencilView) Renderer->DepthStencilView->Release();
        if(Renderer->BackBuffer) Renderer->BackBuffer->Release();
        if(Renderer->SwapChain) Renderer->SwapChain->Release();
        if(Renderer->RenderContext) Renderer->RenderContext->Release(); 
        if(Renderer->Device) Renderer->Device->Release(); 
    }
    return 0;
}


