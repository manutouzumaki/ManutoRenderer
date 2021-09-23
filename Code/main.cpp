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

static v3
MouseRayCameraPlaneIntersection(app_input *Input, arc_camera *Camera, v3 PlaneOffset, mat4 Proj)
{
    v3 C = Camera->Target;
    v3 O = Camera->Position;
    v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, Camera->View, Proj);
    D = NormalizeV3(D);
    v3 N = C - O;
    N = NormalizeV3(N);
    
    float T = 0;
    if(DotV3(N, D) > 0 || DotV3(N, D) < 0)
    {
        T = DotV3((N*-1.0f), (O - PlaneOffset)) / DotV3(N, D);
    }
    v3 MousePositionOnPlane = LerpV3(O, D, T);
    return MousePositionOnPlane;
}

static v3
MouseRayPlaneIntersection(app_input *Input, arc_camera *Camera, v3 PlaneOffset, mat4 Proj)
{
    v3 O = Camera->Position;
    v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, Camera->View, Proj);
    D = NormalizeV3(D);
    v3 N = {0.0f, 1.0f, 0.0f};
    
    float T = 0;
    if(DotV3(N, D) > 0 || DotV3(N, D) < 0)
    {
        T = DotV3((N*-1.0f), (O - PlaneOffset)) / DotV3(N, D);
    }
    
    v3 MousePositionOnPlane = {};
    if(T >= 0)
    {
        MousePositionOnPlane = LerpV3(O, D, T);
    }
    else
    {
        MousePositionOnPlane.Y = MousePositionOnPlane.Y - 1.0f;
    }
    return MousePositionOnPlane;
}

static float
MouseRaySphereIntersection(v3 CameraPos, mat4 View, mat4 Proj,
                      bounding_sphere BoundingSphere, 
                      app_input *Input)
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
    return TF;    
}

static int
SearchCloserMesh(float *TValues, int TValuesCount)
{
    int Result = 0;
    float SmallValue = TValues[0];
    for(int I = 0;
        I < TValuesCount;
        ++I)
    {
        if((SmallValue > TValues[I] || SmallValue < 0.0f) && TValues[I] > 0)
        {
            SmallValue = TValues[I];
            Result = I;
        }
    }
    if(SmallValue < 0)
    {
        Result = -1;
    }
    return Result;
}

static void
ProcessMeshShouldMove(app_input *Input, game_state *GameState)
{
    float TValues[2];  // TODO: make the number of meshes not be fix
    for(int Index = 0;
        Index < 2;
        ++Index)
    {
        TValues[Index] = MouseRaySphereIntersection(GameState->Camera.Position,
                         GameState->Camera.View, GameState->Proj,
                         GameState->BoundingSpheres[Index], Input);
    }
    int Index = SearchCloserMesh(TValues, 2);
    if(Index >= 0)
    {
        GameState->SphereSelected = &GameState->BoundingSpheres[Index];
        GameState->SpherePositionWhenClick = GameState->SphereSelected->Position;
        v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->SpherePositionWhenClick, GameState->Proj);
        GameState->Offset = MousePositionOnPlane - GameState->SphereSelected->Position;
        GameState->MoveMesh = true;
    }
}

static void
ProcessInput(app_input *Input, game_state *GameState, float DeltaTime)
{
    // handle camera movement
    if(MouseOnClick(Input, MIDDLE_CLICK))
    {
        PlatformShowCursor(false);
        PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY);
    }
    else if(MouseDown(Input, MIDDLE_CLICK) && !MouseDown(Input, SHIFT_CLICK))
    {
        // click the middle button to rotate the camera around
        ProcessCameraRotation(Input, &GameState->Camera, DeltaTime);
    }
    else if(MouseDown(Input, MIDDLE_CLICK) && MouseDown(Input, SHIFT_CLICK))
    {
        // click shift and the middle button to move the camera around
        ProcessCameraMovement(Input, &GameState->Camera, DeltaTime);
    }
    if(MouseOnUp(Input, MIDDLE_CLICK))
    {
        PlatformShowCursor(true);
    }
    ProcessCameraDistance(Input, &GameState->Camera, DeltaTime);
    // handle meshes movement
    if(MouseOnClick(Input, LEFT_CLICK))
    {
        ProcessMeshShouldMove(Input, GameState);
    }
    if(MouseOnUp(Input, LEFT_CLICK))
    {
        GameState->MoveMesh = false;
    }

    if(MouseDown(Input, RIGHT_CLICK))
    {
        v3 MousePositionOnTerrain = MouseRayPlaneIntersection(Input, &GameState->Camera, {0.0f, 0.0f, 0.0f}, GameState->Proj);
        ModifyTerrainHeight(MousePositionOnTerrain, GameState->Terrain, GameState->Renderer, DeltaTime);
    }
}

static void
GameSetUp(app_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->Memory;
    Memory->Use = sizeof(game_state);

    InitArena(Memory, &GameState->FileArena,   Megabytes(200));
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
            GameState->SkyboxShader = PlatformCreateShadersFromFile(GameState->Renderer,
                                                                    "../Code/sky_vertex_shader.hlsl", "VS_Main",
                                                                    "../Code/sky_pixel_shader.hlsl", "PS_Main",
                                                                    &GameState->FileArena);
            GameState->TreeMesh = LoadMesh("../Data/tree.obj", GameState->Renderer, &GameState->FileArena);
            GameState->HouseMesh = LoadMesh("../Data/house.obj", GameState->Renderer, &GameState->FileArena);
            GameState->SphereMesh = LoadMesh("../Data/sphere_low.obj", GameState->Renderer, &GameState->FileArena);
            GameState->TreeTexture = LoadTexture("../Data/tree.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->HouseTexture = LoadTexture("../Data/house.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->SphereTexture = LoadTexture("../Data/green.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->TerrainTexture = LoadTexture("../Data/rock.bmp", GameState->Renderer, &GameState->FileArena);
            
            GameState->Terrain = LoadTerrain(-40.0f, 0.0f, -20.0f, 40, 40, 1, GameState->Renderer, &GameState->FileArena);

            GameState->SkyBox = LoadCube(GameState->Renderer, &GameState->FileArena);
            GameState->SkyBoxTexture = LoadCubeTexture("../Data/left.bmp", "../Data/right.bmp",
                                                       "../Data/bottom.bmp", "../Data/top.bmp",
                                                       "../Data/back.bmp","../Data/front.bmp",
                                                       GameState->Renderer, &GameState->FileArena);

            InitializeCamera(&GameState->Camera);

            mat4 World = IdentityMat4();
            GameState->Proj = PerspectiveProjMat4(ToRad(60), (float)WND_WIDTH/(float)WND_HEIGHT, 0.1f, 100.0f);
            SetWorldMat4(GameState->Renderer, World);
            SetProjectionMat4(GameState->Renderer, GameState->Proj);

            GameState->BoundingSpheres[0].Position = {3.0f, 0.0f, 0.0f};
            GameState->BoundingSpheres[0].Radius = 1.0f; 
            GameState->BoundingSpheres[1].Position = {0.0f, 0.0f, 0.0f};
            GameState->BoundingSpheres[1].Radius = 4.0f;

        }
    }   
}


static void
GameUpdateAndRender(app_memory *Memory, app_input *Input, float DeltaTime)
{
    game_state *GameState = (game_state *)Memory->Memory;
    
    // Update... 
    ProcessInput(Input, GameState, DeltaTime); 
    UpdateCameraView(&GameState->Camera, Input);
    SetViewMat4(GameState->Renderer, GameState->Camera.View);
    
    // move selected mesh on the camera plane...
    if(GameState->MoveMesh)
    {
        v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->SpherePositionWhenClick, GameState->Proj);
        GameState->SphereSelected->Position = MousePositionOnPlane - GameState->Offset;
    }
    
    // Render...
    SetFillType(GameState->Renderer, SOLID);
    
    mat4 SkyBoxView = Mat3ToMat4(Mat4ToMat3(GameState->Camera.View));
    SetViewMat4(GameState->Renderer, SkyBoxView);
    SetTexture(GameState->SkyBoxTexture, GameState->Renderer);
    //RenderMeshIndexed(GameState->SkyBox, GameState->SkyboxShader, GameState->Renderer);
    SetViewMat4(GameState->Renderer, GameState->Camera.View);
    
    SetFillType(GameState->Renderer, WIREFRAME);

    float Scale = GameState->BoundingSpheres[0].Radius;
    mat4 World = TranslationMat4(GameState->BoundingSpheres[0].Position) * ScaleMat4({Scale, Scale, Scale});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->SphereTexture, GameState->Renderer);
    RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);

    Scale = GameState->BoundingSpheres[1].Radius;
    World = TranslationMat4(GameState->BoundingSpheres[1].Position) * ScaleMat4({Scale, Scale, Scale});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->SphereTexture, GameState->Renderer);
    RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);

    SetFillType(GameState->Renderer, SOLID);

    World = TranslationMat4(GameState->BoundingSpheres[0].Position);// * ScaleMat4({Scale, Scale, Scale});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TreeTexture, GameState->Renderer);
    RenderMesh(GameState->TreeMesh, GameState->Shader, GameState->Renderer);

    World = TranslationMat4(GameState->BoundingSpheres[1].Position);// * ScaleMat4({Scale, Scale, Scale});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->HouseTexture, GameState->Renderer);
    RenderMesh(GameState->HouseMesh, GameState->Shader, GameState->Renderer);   

    World = TranslationMat4(GameState->Camera.Target) * ScaleMat4({0.3f, 0.3f, 0.3f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->SphereTexture, GameState->Renderer);
    RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);

    World = TranslationMat4({0.0f, 0.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TerrainTexture, GameState->Renderer);
    RenderMeshIndexed(GameState->Terrain->Mesh, GameState->Shader, GameState->Renderer);
}



