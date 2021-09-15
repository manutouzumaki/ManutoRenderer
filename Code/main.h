#ifndef MAIN_H
#define MAIN_H

// opaque struuct define in win32_platform.cpp
struct window;
// opaque struct define in directx.h and opengl.h
struct renderer;
struct shader;

window *PlatformCreateWindow(char * WindowName, int WindowWidth, int WindowHeight, arena *Arena);
renderer *PlatformCreateRenderer(window *Window, arena *Arena);
shader *PlatformCreateShadersFromFile(renderer *Renderer,
                                      char * VertexShaderFileName, char *VSMainFunc,
                                      char *PixelShaderFileName, char *PSMainFunc,
                                      arena *Arena);
void* PlatformAllocMemory(SIZE_T Size);
void PlatformFreeMemory(void *Memory);

struct game_state
{
    window *Window;
    renderer *Renderer;

    arena RenderArena;
    arena FileArena;

    shader *Shader;
    mesh *Cube;
}; 
  

#endif
