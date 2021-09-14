static void
GameSetUp(app_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->Memory;
    Memory->Use = sizeof(game_state);

    InitArena(Memory, &GameState->FileArena,   Megabytes(40));
    InitArena(Memory, &GameState->RenderArena, Megabytes(20)); 
    
    GameState->Window = PlatformCreateWindow("Renderer", WND_WIDTH, WND_HEIGHT, &GameState->RenderArena);
    if(GameState->Window)
    {
        GameState->Renderer = PlatformCreateRenderer(GameState->Window, &GameState->RenderArena);
        if(GameState->Renderer)
        {
            OutputDebugString("Renderer Initialize!\n");
            
            GameState->Shader = PlatformCreateShadersFromFile(GameState->Renderer,
                                                              "../Code/main_vertex_shader.hlsl", "VS_Main",
                                                              "../Code/main_pixel_shader.hlsl", "PS_Main",
                                                              &GameState->FileArena);
            GameState->Cube = LoadCube(GameState->Renderer, &GameState->RenderArena);

            mat4 World = IdentityMat4();
            mat4 View = ViewMat4({0.0f, 2.0f, 5.0f}, {0.0f, 0.0f,  0.0f}, {0.0f, 1.0f,  0.0f});
            mat4 Proj = PerspectiveProjMat4(ToRad(90), (float)WND_WIDTH/(float)WND_HEIGHT, 0.1f, 100.0f);
            SetWorldMat4(GameState->Renderer, World);
            SetViewMat4(GameState->Renderer, View);
            SetProjectionMat4(GameState->Renderer, Proj);


            // OBJ file Test...
            obj House = LoadOBJFile("../Data/cube.obj", &GameState->FileArena);
        }
    }   
}
    
static void
GameUpdateAndRender(app_memory *Memory, float DeltaTime)
{
    game_state *GameState = (game_state *)Memory->Memory;
    
    static float Time = 0;
    mat4 World = TranslationMat4({0.0f, sinf(Time)*2.0f, 0.0f}) * RotationYMat(Time);
    SetWorldMat4(GameState->Renderer, World);
    Time += DeltaTime;


    RenderMesh(GameState->Cube, GameState->Shader, GameState->Renderer);
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
