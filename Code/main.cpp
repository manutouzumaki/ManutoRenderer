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
            GameState->TreeMesh = LoadMesh("../Data/tree.obj", GameState->Renderer, &GameState->FileArena);
            GameState->HouseMesh = LoadMesh("../Data/house.obj", GameState->Renderer, &GameState->FileArena);
            GameState->TreeTexture = LoadTexture("../Data/tree.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->HouseTexture = LoadTexture("../Data/house.bmp", GameState->Renderer, &GameState->FileArena);

            mat4 World = IdentityMat4();
            mat4 View = ViewMat4({5.0f, 1.0f, 10.0f}, {0.0f, 0.0f,  0.0f}, {0.0f, 1.0f,  0.0f});
            mat4 Proj = PerspectiveProjMat4(ToRad(60), (float)WND_WIDTH/(float)WND_HEIGHT, 0.1f, 100.0f);
            SetWorldMat4(GameState->Renderer, World);
            SetViewMat4(GameState->Renderer, View);
            SetProjectionMat4(GameState->Renderer, Proj);
        }
    }   
}
    
static void
GameUpdateAndRender(app_memory *Memory, float DeltaTime)
{
    game_state *GameState = (game_state *)Memory->Memory;
    
    mat4 World = TranslationMat4({6.0f, .0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TreeTexture, GameState->Renderer);
    RenderMesh(GameState->TreeMesh, GameState->Shader, GameState->Renderer);

    World = TranslationMat4({0.0f, 0.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->HouseTexture, GameState->Renderer);
    RenderMesh(GameState->HouseMesh, GameState->Shader, GameState->Renderer);

}
