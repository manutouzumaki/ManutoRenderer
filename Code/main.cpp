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
UpdateCameraView(arc_camera *Camera, app_input *Input)
{
    Camera->RealUp = NormalizeV3(CrossV3(Camera->Front, Camera->Right));
    Camera->View = ViewMat4(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
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
            GameState->SphereMesh = LoadMesh("../Data/sphere_low.obj", GameState->Renderer, &GameState->FileArena);
            GameState->TreeTexture = LoadTexture("../Data/tree.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->HouseTexture = LoadTexture("../Data/house.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->SphereTexture = LoadTexture("../Data/green.bmp", GameState->Renderer, &GameState->FileArena);
            
            GameState->Camera.Position = {0.0f, 0.0f, 1.0f};
            GameState->Camera.Target = {0.0f, 0.0f, -2.0f};
            GameState->Camera.Front = NormalizeV3(GameState->Camera.Target - GameState->Camera.Position);
            GameState->Camera.PosRelativeToTarget = GameState->Camera.Target - GameState->Camera.Position;
            GameState->Camera.Up = {0.0f, 1.0f, 0.0f};
            GameState->Camera.Yaw = ToRad(-90.0f);
            GameState->Camera.Pitch = 0.0f;

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
ProcessInput(app_input *Input, game_state *GameState, float DeltaTime)
{
    if(MouseOnClick(Input, MIDDLE_CLICK))
    {
        PlatformShowCursor(false);
        PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY);
    }
    else if(MouseDown(Input, MIDDLE_CLICK))
    {
        static float MouseOffsetX = 0;
        static float MouseOffsetY = 0;
        MouseOffsetX = (Input->MouseX - (WND_WIDTH/2.0f)) * DeltaTime;
        MouseOffsetY = (Input->MouseY - (WND_HEIGHT/2.0f)) * DeltaTime; 
        PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY); 

        GameState->Camera.Yaw -= MouseOffsetX *0.5f;
        GameState->Camera.Pitch -= MouseOffsetY *0.5f;

        if(GameState->Camera.Pitch > ToRad(89.0f))
        {
            GameState->Camera.Pitch = ToRad(89.0f);
        }
        if(GameState->Camera.Pitch < ToRad(-89.0f))
        {
            GameState->Camera.Pitch = ToRad(-89.0f);
        }

        v3 Front = {};
        Front.X = cosf(GameState->Camera.Yaw) * cosf(GameState->Camera.Pitch);
        Front.Y = sinf(GameState->Camera.Pitch);
        Front.Z = sinf(GameState->Camera.Yaw) * cosf(GameState->Camera.Pitch);
        GameState->Camera.Front= NormalizeV3(Front);
        GameState->Camera.Right = NormalizeV3(CrossV3(GameState->Camera.Up, GameState->Camera.Front));
        
        v4 CameraRelativeToTarget4V = {GameState->Camera.PosRelativeToTarget.X,
                                       GameState->Camera.PosRelativeToTarget.Y,
                                       GameState->Camera.PosRelativeToTarget.Z, 1.0f};        
        CameraRelativeToTarget4V = RotationYMat(GameState->Camera.Yaw + ToRad(90.0f)) * CameraRelativeToTarget4V;
        CameraRelativeToTarget4V = RotationV3Mat({GameState->Camera.Right}, GameState->Camera.Pitch) * CameraRelativeToTarget4V;
        v3 Result = {};
        Result.X = CameraRelativeToTarget4V.X;
        Result.Y = CameraRelativeToTarget4V.Y;
        Result.Z = CameraRelativeToTarget4V.Z;
        GameState->Camera.Position = GameState->Camera.Target - Result;
    }
    if(MouseOnUp(Input, MIDDLE_CLICK))
    {
        PlatformShowCursor(true);
    }

    if(MouseOnClick(Input, SHIFT_MIDDLE_CLICK))
    {
        PlatformShowCursor(false);
        PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY);
    }
    else if(MouseDown(Input, SHIFT_MIDDLE_CLICK))
    {
        static float MouseOffsetX = 0;
        static float MouseOffsetY = 0;
        MouseOffsetX = (Input->MouseX - (WND_WIDTH/2.0f)) * DeltaTime;
        MouseOffsetY = (Input->MouseY - (WND_HEIGHT/2.0f)) * DeltaTime; 
        PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY); 
        
        GameState->Camera.Position = GameState->Camera.Position - GameState->Camera.Right * MouseOffsetX;
        GameState->Camera.Position = GameState->Camera.Position + GameState->Camera.RealUp * MouseOffsetY;
        GameState->Camera.Target = GameState->Camera.Target - GameState->Camera.Right * MouseOffsetX;
        GameState->Camera.Target = GameState->Camera.Target + GameState->Camera.RealUp * MouseOffsetY;
        GameState->Camera.Front = NormalizeV3(GameState->Camera.Target - GameState->Camera.Position);

    }
    if(MouseOnUp(Input, SHIFT_MIDDLE_CLICK))
    {
        PlatformShowCursor(true);
    }

    if(MouseDown(Input, RIGHT_CLICK))
    {
        GameState->Camera.Position = GameState->Camera.Position + GameState->Camera.Front * DeltaTime;
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
    

    static bool MoveMesh = false;
    for(int Index = 0;
        Index < 2;
        ++Index)
    {
        if(ClickOnBoundingSphere(GameState->Camera.Position, GameState->Camera.View, GameState->Proj,
                                 GameState->BoundingSpheres[Index], 
                                 Input, LEFT_CLICK))
        {
            GameState->SphereSelected = &GameState->BoundingSpheres[Index];
            GameState->SpherePositionWhenClick = GameState->SphereSelected->Position;
            v3 C = GameState->Camera.Target;
            v3 O = GameState->Camera.Position;
            v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, GameState->Camera.View, GameState->Proj);
            D = NormalizeV3(D);
            v3 N = C - O;
            N = NormalizeV3(N);
            
            float T = 0;
            if(DotV3(N, D) > 0 || DotV3(N, D) < 0)
            {
                T = DotV3((N*-1.0f), (O - GameState->SpherePositionWhenClick)) / DotV3(N, D);
            }
            v3 MousePositionOnPlane = LerpV3(O, D, T);
            GameState->Offset = MousePositionOnPlane - GameState->SphereSelected->Position;
            MoveMesh = true;
        }
    }

    if(MouseOnUp(Input, LEFT_CLICK))
    {
        MoveMesh = false;
    }

    if(MouseOnClick(Input, SHIFT_MIDDLE_CLICK))
    {
        OutputDebugString("Shift Middle Click!!\n");
    }

    if(MoveMesh)
    {
        v3 C = GameState->Camera.Target;
        v3 O = GameState->Camera.Position;
        v3 D = GetV3RayFrom2DPos(Input->MouseX, Input->MouseY, GameState->Camera.View, GameState->Proj);
        D = NormalizeV3(D);
        v3 N = C - O;
        N = NormalizeV3(N);
        
        float T = 0;
        if(DotV3(N, D) > 0 || DotV3(N, D) < 0)
        {
            T = DotV3((N*-1.0f), (O - GameState->SpherePositionWhenClick)) / DotV3(N, D);
        }
        GameState->SphereSelected->Position = LerpV3(O, D, T) - GameState->Offset;
    }
  

    // Render...
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
}



