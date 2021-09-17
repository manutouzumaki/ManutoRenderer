static bool
MouseOnClick(app_input *Input, int MouseButton)
{
    if(Input->MouseButtons->Buttons[MouseButton].IsDown != Input->MouseButtons->Buttons[MouseButton].WasDown)
    {
        if(Input->MouseButtons->Buttons[MouseButton].IsDown)
        {
            return true;
        }
    }
    return false;
}

static bool
MouseOnUp(app_input *Input, int MouseButton)
{
    if(Input->MouseButtons->Buttons[MouseButton].WasDown != Input->MouseButtons->Buttons[MouseButton].IsDown)
    {
        if(Input->MouseButtons->Buttons[MouseButton].WasDown)
        {
            return true;
        }
    }
    return false;
}


static bool
MouseDown(app_input *Input, int MouseButton)
{
    if(Input->MouseButtons->Buttons[MouseButton].IsDown)
    {
        return true;
    }
    return false;
}

   
static v3
GetV3RayFrom2DPos(int XPos, int YPos, mat4 View, mat4 Proj)
{
    mat4 InvView = GetInverseMatrix(View);
    mat4 InvProj = GetInverseMatrix(Proj);

    // first we have to normalize the mouse position to be between -1:1, -1:1, -1:1
    float X = (2.0f * XPos) / WND_WIDTH - 1.0f;
    //float Y = (2.0f * YPos) / WND_HEIGHT - 1.0f;
    float Y = 1.0f - (2.0f * YPos) / WND_HEIGHT;
    float Z = 1.0f;
    v3 RayNormalised = {X, Y, Z};

    // then we get the homogeneus clip coordinates
    v4 RayClip = {RayNormalised.X, RayNormalised.Y, 1.0f, 1.0f};

    // next we get the eye camera coordinates
    v4 RayEye = InvProj * RayClip;
    RayEye.Z = 1.0f;
    RayEye.W = 0.0f; 

    v4 RayWorld = InvView * RayEye;
    RayWorld = NormalizeV4(RayWorld);

    v3 Result = {RayWorld.X, RayWorld.Y, RayWorld.Z};
    return Result;

}

static bool 
ClickOnBoundingSphere(v3 CameraPos, mat4 View, mat4 Proj,
                      bounding_sphere BoundingSphere, 
                      app_input *Input, int MouseButton)
{
    if(MouseOnClick(Input, MouseButton))
    {
        float R = BoundingSphere.Radius;
        v3 C = BoundingSphere.Position;
        v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, View, Proj);
        D = NormalizeV3(D);
        v3 O = CameraPos;

        float BF = DotV3(D, (O - C));
        float CF = DotV3((O - C),(O - C)) - (R*R);
        
        float TF = -1;
        if(((BF*BF) - CF) >= 0)
        {
            float T0 = (-1.0f*BF) - sqrtf((BF*BF) - CF);
            float T1 = (-1.0f*BF) + sqrtf((BF*BF) - CF);

            if(T0 < T1)
            {
                TF = T0;
            }
            else
            {
                TF = T1; 
            }
        }
        return TF >= 0;
    }
    return false;
}

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
            GameState->SphereMesh = LoadMesh("../Data/sphere.obj", GameState->Renderer, &GameState->FileArena);
            GameState->MousePosMesh = LoadMesh("../Data/sphere.obj", GameState->Renderer, &GameState->FileArena);
            GameState->TreeTexture = LoadTexture("../Data/tree.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->HouseTexture = LoadTexture("../Data/house.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->SphereTexture = LoadTexture("../Data/green.bmp", GameState->Renderer, &GameState->FileArena);
            
            mat4 World = IdentityMat4();
            GameState->CameraPos = {3.0f, 0.0f, 5.0f};
            GameState->CameraTarget = {0.0f, 1.0f,  0.0f};
            GameState->View = ViewMat4(GameState->CameraPos, GameState->CameraTarget, {0.0f, 1.0f,  0.0f});
            GameState->Proj = PerspectiveProjMat4(ToRad(60), (float)WND_WIDTH/(float)WND_HEIGHT, 0.1f, 100.0f);
            SetWorldMat4(GameState->Renderer, World);
            SetViewMat4(GameState->Renderer, GameState->View);
            SetProjectionMat4(GameState->Renderer, GameState->Proj);

            GameState->BoundingSphere;
            GameState->BoundingSphere.Position = {3.0f, 1.0f, -4.0f};
            GameState->BoundingSphere.Radius = 1.0f;
        }
    }   
}
    
static void
GameUpdateAndRender(app_memory *Memory, app_input *Input, float DeltaTime)
{
    game_state *GameState = (game_state *)Memory->Memory;

    static bool MoveMesh = false;
    static v3 MouseProjectedPosition {};
    static v3 B = {};
    static v3 Offset = {};
    if(ClickOnBoundingSphere(GameState->CameraPos, GameState->View, GameState->Proj,
                             GameState->BoundingSphere, 
                             Input, LEFT_CLICK))
    {
        B = GameState->BoundingSphere.Position;
        v3 C = GameState->CameraTarget;
        v3 O = GameState->CameraPos;
        v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, GameState->View, GameState->Proj);
        D = NormalizeV3(D);
        v3 N = C - O;
        N = NormalizeV3(N);
        
        float T = 0;
        if(DotV3(N, D) > 0 || DotV3(N, D) < 0)
        {
            T = DotV3((N*-1.0f), (O - B)) / DotV3(N, D);
        }
        v3 MousePositionOnPlane = LerpV3(O, D, T);
        Offset = MousePositionOnPlane - GameState->BoundingSphere.Position;
        MoveMesh = true;
    }

    if(MouseOnUp(Input, LEFT_CLICK))
    {
        MoveMesh = false;
    }

    if(MoveMesh)
    {
        v3 C = GameState->CameraTarget;
        v3 O = GameState->CameraPos;
        v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, GameState->View, GameState->Proj);
        D = NormalizeV3(D);
        v3 N = C - O;
        N = NormalizeV3(N);
        
        float T = 0;
        if(DotV3(N, D) > 0 || DotV3(N, D) < 0)
        {
            T = DotV3((N*-1.0f), (O - B)) / DotV3(N, D);
        }
        GameState->BoundingSphere.Position = LerpV3(O, D, T) - Offset;
    }





/*
    static v3 MouseProjectedPosition {};
    static v3 MouseProjectedPositionOnPlane = {};
    // Update...
    if(MouseOnClick(Input, LEFT_CLICK))
    {
        float R = GameState->BoundingSphere.Radius;
        v3 C = GameState->BoundingSphere.Position;
        v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, GameState->View, GameState->Proj);
        D = NormalizeV3(D);
        v3 O = GameState->CameraPos;

        float BF = DotV3(D, (O - C));
        float CF = DotV3((O - C),(O - C)) - (R*R);
        
        float TF = -1;
        if(((BF*BF) - CF) >= 0)
        {
            float T0 = (-1.0f*BF) - sqrtf((BF*BF) - CF);
            float T1 = (-1.0f*BF) + sqrtf((BF*BF) - CF);

            if(T0 < T1)
            {
                TF = T0;
            }
            else
            {
                TF = T1; 
            }
            MouseProjectedPosition = LerpV3(GameState->CameraPos, D, TF);
        }
    }

*/


    // Render...
#if 0  
    mat4 World = TranslationMat4({6.0f, .0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TreeTexture, GameState->Renderer);
    RenderMesh(GameState->TreeMesh, GameState->Shader, GameState->Renderer);

    World = TranslationMat4({0.0f, 0.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->HouseTexture, GameState->Renderer);
    RenderMesh(GameState->HouseMesh, GameState->Shader, GameState->Renderer);
#endif

    float Scale = GameState->BoundingSphere.Radius;
    mat4 World = TranslationMat4(GameState->BoundingSphere.Position) * ScaleMat4({Scale, Scale, Scale});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->SphereTexture, GameState->Renderer);
    RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);
    
/* 
    World = TranslationMat4(MouseProjectedPosition);// * ScaleMat4({0.3f, 0.3f, 0.3f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TreeTexture, GameState->Renderer);
    RenderMesh(GameState->MousePosMesh, GameState->Shader, GameState->Renderer);
*/

}



