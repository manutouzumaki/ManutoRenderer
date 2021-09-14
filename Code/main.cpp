static void
GameSetUp(app_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->Memory;
    Memory->Use = sizeof(game_state);

    InitArena(Memory, &GameState->RenderArena, Megabytes(20)); 
    
    GameState->Window = CreatePlatformWindow("Renderer", WND_WIDTH, WND_HEIGHT, &GameState->RenderArena);
    if(GameState->Window)
    {
        GameState->Renderer = CreateRenderer(GameState->Window, &GameState->RenderArena);
        if(GameState->Renderer)
        {
            OutputDebugString("Renderer Initialize!\n");
        }
    }   
}
    
static void
GameUpdateAndRender(app_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->Memory;
}

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
