#ifndef MAIN_H
#define MAIN_H

// opaque struuct define in win32_platform.cpp
struct window;
// opaque struct define in directx.h and opengl.h
struct renderer;

window *
CreatePlatformWindow(char * WindowName, int WindowWidth, int WindowHeight, arena *Arena);
renderer *
CreateRenderer(window *Window, arena *Arena);

struct game_state
{
    window *Window;
    renderer *Renderer;
    arena RenderArena;
}; 
  

#endif
