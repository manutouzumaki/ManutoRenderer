#ifndef PLATFORM_H
#define PLATFORM_H

// opaque struuct define in win32_platform.cpp
struct window;
// opaque struct define in directx.h and opengl.h
struct renderer;
struct shader;

#define PLATFORM_CREATE_WINDOW(name) window *name(char * WindowName, int WindowWidth, int WindowHeight, arena *Arena)
#define PLATFORM_CREATE_RENDERER(name) renderer *name(window *Window, arena *Arena)
#define PLATFORM_CREATE_SHADERS_FROM_FILE(name) shader *name(renderer *Renderer, char * VertexShaderFileName, \
                                                             char *VSMainFunc, char *PixelShaderFileName, \
                                                             char *PSMainFunc, arena *Arena)
#define PLATFORM_ALLOC_MEMORY(name) void* name(size_t Size)
#define PLATFORM_FREE_MEMORY(name) void name(void *Memory)

#define PLATFORM_SHOW_CURSOR(name) void name(bool Value)
#define PLATFORM_SET_CURSOR_POSITION(name) void name(int PosX, int PosY)

PLATFORM_CREATE_WINDOW(PlatformCreateWindow);
PLATFORM_CREATE_RENDERER(PlatformCreateRenderer);
PLATFORM_CREATE_SHADERS_FROM_FILE(PlatformCreateShadersFromFile);
PLATFORM_ALLOC_MEMORY(PlatformAllocMemory);
PLATFORM_FREE_MEMORY(PlatformFreeMemory);
PLATFORM_SHOW_CURSOR(PlatformShowCursor);
PLATFORM_SET_CURSOR_POSITION(PlatformSetCursorPosition);

struct app_memory
{
    void *Memory;
    size_t Size;
    size_t Use;
};

struct button_state
{
    bool IsDown;
    bool WasDown;
};

struct mouse_buttons
{
    button_state Buttons[4];
};

struct app_input
{
    int MouseX, MouseY;
    int MouseDefaultX, MouseDefaultY;
    mouse_buttons *MouseButtons;   
};

#endif
