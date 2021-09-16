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
#define PLATFORM_ALLOC_MEMORY(name) void* name(SIZE_T Size)
#define PLATFORM_FREE_MEMORY(name) void name(void *Memory)

PLATFORM_CREATE_WINDOW(PlatformCreateWindow);
PLATFORM_CREATE_RENDERER(PlatformCreateRenderer);
PLATFORM_CREATE_SHADERS_FROM_FILE(PlatformCreateShadersFromFile);
PLATFORM_ALLOC_MEMORY(PlatformAllocMemory);
PLATFORM_FREE_MEMORY(PlatformFreeMemory);

struct app_memory
{
    void *Memory;
    size_t Size;
    size_t Use;
};

#endif
