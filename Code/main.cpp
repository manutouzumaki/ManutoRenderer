static void 
SetBit(unsigned int *BitField, int BitPosition)
{ 
    *BitField |= BIT(BitPosition);
}

static void
UnsetBit(unsigned int *BitField, int BitPosition)
{
    *BitField &= ~(BIT(BitPosition));
}

static void
SwitchBit(unsigned int *BitField, int BitPosition)
{
    *BitField ^= BIT(BitPosition);
}

static unsigned int
GetBit(unsigned int *BitField, int BitPosition)
{
    return (*BitField & BIT(BitPosition)) >> BitPosition;
}

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
    float *TValues = (float *)PlatformAllocMemory(GameState->EntityList.Counter * sizeof(float));
    entity *FirstEntity = GameState->EntityList.Entities;
    FirstEntity -= (GameState->EntityList.Counter - 1);
    for(int Index = 0;
        Index < GameState->EntityList.Counter;
        ++Index)
    {
        TValues[Index] = MouseRaySphereIntersection(GameState->Camera.Position,
                         GameState->Camera.View, GameState->PerspectiveProj,
                         FirstEntity->BoundingSphere, Input);
        ++FirstEntity;
    }
    
    FirstEntity = GameState->EntityList.Entities;
    FirstEntity -= (GameState->EntityList.Counter - 1);
    int Index = SearchCloserMesh(TValues, GameState->EntityList.Counter);
    entity *ActualEntity = FirstEntity + Index;
    if(Index >= 0)
    {
        GameState->SphereSelected = &ActualEntity->BoundingSphere;
        GameState->SpherePositionWhenClick = GameState->SphereSelected->Position;
        v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->SpherePositionWhenClick, GameState->PerspectiveProj);
        GameState->Offset = MousePositionOnPlane - GameState->SphereSelected->Position;
        GameState->MoveMesh = true;
    }

    PlatformFreeMemory(TValues);
}

static void
ProcessEntitySetTexture(app_input *Input, game_state *GameState)
{
    float *TValues = (float *)PlatformAllocMemory(GameState->EntityList.Counter * sizeof(float));
    entity *FirstEntity = GameState->EntityList.Entities;
    FirstEntity -= (GameState->EntityList.Counter - 1);
    for(int Index = 0;
        Index < GameState->EntityList.Counter;
        ++Index)
    {
        TValues[Index] = MouseRaySphereIntersection(GameState->Camera.Position,
                         GameState->Camera.View, GameState->PerspectiveProj,
                         FirstEntity->BoundingSphere, Input);
        ++FirstEntity;
    }
    
    FirstEntity = GameState->EntityList.Entities;
    FirstEntity -= (GameState->EntityList.Counter - 1);
    int Index = SearchCloserMesh(TValues, GameState->EntityList.Counter);
    entity *ActualEntity = FirstEntity + Index;
    if(Index >= 0)
    {
        ActualEntity->TextureIndex = GameState->TextureSelectedIndex;
    }

    PlatformFreeMemory(TValues);
}

static void
LoadMeshFromFileExplorer(game_state *GameState, renderer *Renderer, arena *Arena)
{ 
    void *FileData = NULL;
    if(BasicFileOpenTest(&FileData, Arena))
    {
        GameState->MeshList.Mesh = (mesh *)PushStruct(&GameState->MeshListArena, mesh);
        LoadMeshToMeshArray(FileData, GameState->MeshList.Mesh, Renderer, Arena);
        if(GameState->MeshList.Mesh)
        {
            OutputDebugString("Mesh Loaded!\n");
        }
        ++GameState->MeshList.Counter;
    }
}

static void
LoadTextureFromFileExplorer(game_state *GameState, renderer *Renderer, arena *Arena)
{ 
    void *FileData = NULL;
    if(BasicFileOpenTest(&FileData, Arena))
    {
        GameState->TextureList.Texture = (texture *)PushStruct(&GameState->TextureListArena, texture);
        LoadTextureToTextureArray(FileData, GameState->TextureList.Texture, Renderer, Arena);
        if(GameState->TextureList.Texture)
        {
            OutputDebugString("Texture Loaded!\n");
        }
        ++GameState->TextureList.Counter;
    }
}

static void
ProcessInput(app_input *Input, game_state *GameState, float DeltaTime)
{
    if(Input->KeyboardKeys->Keys['E'].IsDown)
    {
        SwitchBit(&GameState->StateBitField, ENTITY_SELECTOR);
        SwitchBit(&GameState->StateBitField, ENTITY_EDITOR);
    }
    
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

    if(GetBit(&GameState->StateBitField, ENTITY_EDITOR))
    {
        // handle meshes movement
        if(MouseOnClick(Input, LEFT_CLICK) && 
           GameState->MeshList.Counter > 0)
        {
            ProcessMeshShouldMove(Input, GameState);
        }
        if(MouseOnUp(Input, LEFT_CLICK))
        {
            GameState->MoveMesh = false;
        }       

        if(MouseDown(Input, RIGHT_CLICK))
        {
            v3 MousePositionOnTerrain = MouseRayPlaneIntersection(Input, &GameState->Camera, {0.0f, 0.0f, 0.0f}, GameState->PerspectiveProj);
            ModifyTerrainHeight(MousePositionOnTerrain, GameState->Terrain, GameState->Renderer, DeltaTime);
        } 
    }

    if(GetBit(&GameState->StateBitField, ENTITY_SELECTOR))
    {
        if(Input->KeyboardKeys->Keys['Q'].IsDown)
        {
            LoadMeshFromFileExplorer(GameState, GameState->Renderer, &GameState->FileArena);
        }
        if(Input->KeyboardKeys->Keys['W'].IsDown)
        {
            LoadTextureFromFileExplorer(GameState, GameState->Renderer, &GameState->FileArena);
        }
        // when left-click on the screen we have to add an entity in that position
        // to the eneity-list
        if(!GameState->MouseOnUI)
        {
            if(MouseOnClick(Input, LEFT_CLICK) &&
               GameState->MeshList.Counter > 0)
            { 
                v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->Camera.Target, GameState->PerspectiveProj);
                GameState->EntityList.Entities = (entity *)PushStruct(&GameState->EntityArena, entity);
                GameState->EntityList.Entities->Position = MousePositionOnPlane;
                GameState->EntityList.Entities->MeshIndex = GameState->MeshSelectedIndex;
                GameState->EntityList.Entities->TextureIndex = -1;
                GameState->EntityList.Entities->BoundingSphere.Position = GameState->EntityList.Entities->Position;
                GameState->EntityList.Entities->BoundingSphere.Radius = 1.0f; 
                ++GameState->EntityList.Counter;
            }
            if(MouseOnClick(Input, RIGHT_CLICK) &&
               GameState->TextureList.Counter > 0)
            {
                ProcessEntitySetTexture(Input, GameState);
            }
        }
    }

}

static void
GameSetUp(app_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->Memory;
    Memory->Use = sizeof(game_state);
    
    InitArena(Memory, &GameState->FileArena,        Megabytes(400));
    InitArena(Memory, &GameState->EntityArena,      Kilobytes(10)); 
    InitArena(Memory, &GameState->RenderArena,      Kilobytes(1));
    InitArena(Memory, &GameState->MeshListArena,    Kilobytes(1));
    InitArena(Memory, &GameState->TextureListArena, Kilobytes(1));
    
    GameState->Window = PlatformCreateWindow("Renderer", WND_WIDTH, WND_HEIGHT, &GameState->RenderArena);
    if(GameState->Window)
    {
        GameState->Renderer = PlatformCreateRenderer(GameState->Window, &GameState->RenderArena);
        if(GameState->Renderer)
        {
            OutputDebugString("Renderer Initialize!\n");
            
            SetBit(&GameState->StateBitField, ENTITY_EDITOR);

            GameState->Shader = PlatformCreateShadersFromFile(GameState->Renderer,
                                                              "../Code/main_vertex_shader.hlsl", "VS_Main",
                                                              "../Code/main_pixel_shader.hlsl", "PS_Main",
                                                              &GameState->FileArena);
            GameState->SkyboxShader = PlatformCreateShadersFromFile(GameState->Renderer,
                                                                    "../Code/sky_vertex_shader.hlsl", "VS_Main",
                                                                    "../Code/sky_pixel_shader.hlsl", "PS_Main",
                                                                    &GameState->FileArena);
            GameState->UIShader = PlatformCreateShadersFromFile(GameState->Renderer,
                                                                "../Code/ui_vertex_shader.hlsl", "VS_Main",
                                                                "../Code/ui_pixel_shader.hlsl", "PS_Main",
                                                                &GameState->FileArena);
            GameState->MemoryUIShader = PlatformCreateShadersFromFile(GameState->Renderer,
                                                                      "../Code/memory_vertex_shader.hlsl", "VS_Main",
                                                                      "../Code/memory_pixel_shader.hlsl", "PS_Main",
                                                                      &GameState->FileArena);

            GameState->SphereTexture = LoadTexture("../Data/green.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->TerrainTexture = LoadTexture("../Data/rock.bmp", GameState->Renderer, &GameState->FileArena);
            GameState->UITexture = LoadTexture("../Data/obj_texture.bmp", GameState->Renderer, &GameState->FileArena);
            
            GameState->SphereMesh = LoadMesh("../Data/sphere_low.obj", GameState->Renderer, &GameState->FileArena); 
            GameState->Terrain = LoadTerrain(-20.0f, 0.0f, -20.0f, 40, 40, 1, GameState->Renderer, &GameState->FileArena);
            GameState->UIQuad = LoadQuad(GameState->Renderer, &GameState->FileArena);
            
            GameState->SkyBox = LoadCube(GameState->Renderer, &GameState->FileArena);
            GameState->SkyBoxTexture = LoadCubeTexture("../Data/left.bmp", "../Data/right.bmp",
                                                       "../Data/bottom.bmp", "../Data/top.bmp",
                                                       "../Data/back.bmp","../Data/front.bmp",
                                                       GameState->Renderer, &GameState->FileArena);

            InitializeCamera(&GameState->Camera);

            mat4 World = IdentityMat4();
            GameState->PerspectiveProj = PerspectiveProjMat4(ToRad(60), (float)WND_WIDTH/(float)WND_HEIGHT, 0.1f, 100.0f);
            GameState->OrthogonalProj = OrthogonalProjMat4(WND_WIDTH, WND_HEIGHT, 0.1f, 100.0f);
            SetWorldMat4(GameState->Renderer, World);
            SetProjectionMat4(GameState->Renderer, GameState->PerspectiveProj); 
        }
    }   
}

static void
GameUpdateAndRender(app_memory *Memory, app_input *Input, float DeltaTime)
{
    game_state *GameState = (game_state *)Memory->Memory;
    
    ProcessInput(Input, GameState, DeltaTime);
    
    // Update... 
    UpdateCameraView(&GameState->Camera, Input);
    SetViewMat4(GameState->Renderer, GameState->Camera.View);
    SetViewPostion(GameState->Renderer, GameState->Camera.Position);
    
    // move selected mesh on the camera plane...
    if(GameState->MoveMesh)
    {
        v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->SpherePositionWhenClick, GameState->PerspectiveProj);
        GameState->SphereSelected->Position = MousePositionOnPlane - GameState->Offset;
    }

    // Render...
    
    // render sky box
    SetFillType(GameState->Renderer, SOLID_FRONT_CULL); 
    SetDepthStencilState(GameState->Renderer, DEPTH_STENCIL_OFF);
    mat4 SkyBoxView = Mat3ToMat4(Mat4ToMat3(GameState->Camera.View));
    SetViewMat4(GameState->Renderer, SkyBoxView);
    SetTexture(GameState->SkyBoxTexture, GameState->Renderer);
    RenderMeshIndexed(GameState->SkyBox, GameState->SkyboxShader, GameState->Renderer);
    SetViewMat4(GameState->Renderer, GameState->Camera.View);
    SetDepthStencilState(GameState->Renderer, DEPTH_STENCIL_ON);
    
    SetFillType(GameState->Renderer, SOLID_BACK_CULL);
    
    // render target position
    mat4 World = TranslationMat4(GameState->Camera.Target) * ScaleMat4({0.3f, 0.3f, 0.3f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->SphereTexture, GameState->Renderer);
    RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);
    
    // render terrain
    World = TranslationMat4({0.0f, 0.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TerrainTexture, GameState->Renderer);
    RenderMeshIndexed(GameState->Terrain->Mesh, GameState->Shader, GameState->Renderer);

    // render entities
    if(GameState->EntityList.Counter > 0)
    {
        entity *FirstEntity = GameState->EntityList.Entities;
        FirstEntity -= (GameState->EntityList.Counter - 1);
        for(int Index = 0;
            Index < GameState->EntityList.Counter;
            ++Index)
        {
            mesh *FirstMesh = GameState->MeshList.Mesh;
            FirstMesh -= (GameState->MeshList.Counter - 1);
            
            texture *FirstTexture = GameState->TextureList.Texture;
            FirstTexture -= (GameState->TextureList.Counter - 1);

            mesh *ActualMesh = FirstMesh + FirstEntity->MeshIndex;
            texture *ActualTexture = FirstTexture + FirstEntity->TextureIndex;
            if(GameState->TextureList.Counter <= 0 || FirstEntity->TextureIndex == -1) ActualTexture = GameState->SphereTexture;
            
            World = TranslationMat4(FirstEntity->BoundingSphere.Position);
            SetWorldMat4(GameState->Renderer, World);
            SetTexture(ActualTexture, GameState->Renderer); 
            RenderMesh(ActualMesh, GameState->Shader, GameState->Renderer);
            
            SetFillType(GameState->Renderer, WIREFRAME);
            SetTexture(GameState->SphereTexture, GameState->Renderer);
            RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);
            SetFillType(GameState->Renderer, SOLID_BACK_CULL);

            ++FirstEntity;
        } 
    }


    // UI Update and Render
    // set the projection matrix to orthogonal for 2d rendering
    SetProjectionMat4(GameState->Renderer,  GameState->OrthogonalProj);
    // memory data bars
    // first we must get the range of memory used 
    // we have to render every arena memory bar 
    SetTexture(GameState->SphereTexture, GameState->Renderer); 
    // FILE ARENA
    float MemoryUsedRange = (float)((double)GameState->FileArena.Use / (double)GameState->FileArena.Size);
    SetMemoryData(GameState->Renderer, MemoryUsedRange);
    World = ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer); 
    // RENDER ARENA
    MemoryUsedRange = (float)((double)GameState->RenderArena.Use / (double)GameState->RenderArena.Size);
    SetMemoryData(GameState->Renderer, MemoryUsedRange);
    World = TranslationMat4({0.0f, -30.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // MESH ARENA
    MemoryUsedRange = float((double)GameState->MeshListArena.Use / (double)GameState->MeshListArena.Size);
    SetMemoryData(GameState->Renderer, MemoryUsedRange);
    World = TranslationMat4({0.0f, -60.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // TEXTURE ARENA
    MemoryUsedRange = float((double)GameState->TextureListArena.Use / (double)GameState->TextureListArena.Size);
    SetMemoryData(GameState->Renderer, MemoryUsedRange);
    World = TranslationMat4({0.0f, -90.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // ENTITY ARENA
    MemoryUsedRange = float((double)GameState->EntityArena.Use / (double)GameState->EntityArena.Size);
    SetMemoryData(GameState->Renderer, MemoryUsedRange);
    World = TranslationMat4({0.0f, -120.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);



    // meshes and textures
    if(GetBit(&GameState->StateBitField, ENTITY_SELECTOR))
    {
        GameState->MouseOnUI = false;

        // mesh list ui
        mesh *FirstElement = GameState->MeshList.Mesh;
        FirstElement -= (GameState->MeshList.Counter - 1);
        
        float XPos = 0.0f;
        float YPos = 0.0f;
        float Width = 50.0f;
        float Height = 50.0f;
        float XOffset = 5.0f;
        float YOffset = 5.0f;
        
        for(int Index = 0;
            Index < GameState->MeshList.Counter;
            ++Index)
        {            
            float ActualX = (XPos * (Width + XOffset));
            float ActualY = (YPos * (Height + YOffset));

            SetTexture(GameState->UITexture, GameState->Renderer);
            
            if(Input->MouseX >= ActualX &&
               Input->MouseX <= (ActualX + Width) &&
               -Input->MouseY <= ActualY &&
               -Input->MouseY >= (ActualY - Height))
            {   
                GameState->MouseOnUI = true; 
                SetTexture(GameState->SphereTexture, GameState->Renderer);
                if(MouseOnClick(Input, LEFT_CLICK))
                {
                    GameState->MeshSelectedIndex = Index;
                }
            }
            
            World = TranslationMat4({ActualX - WND_WIDTH*0.5f, ActualY + (WND_HEIGHT*0.5f-Height), 0.0f}) * ScaleMat4({50.0f, 50.0f, 0.0f});
            SetWorldMat4(GameState->Renderer, World);
            RenderMesh(GameState->UIQuad, GameState->UIShader, GameState->Renderer);
            ++FirstElement;

            ++XPos;
            if(XPos >= 4)
            {
                XPos = 0;
                --YPos;
            }

        }
        

        // texture list ui 
        XPos = 0.0f;
        YPos = (WND_HEIGHT*-0.5f) / Height;

        texture *FirstTexture = GameState->TextureList.Texture;
        FirstTexture -= (GameState->TextureList.Counter - 1);

        for(int Index = 0;
            Index < GameState->TextureList.Counter;
            ++Index)
        {            
            float ActualX = (XPos * (Width + XOffset));
            float ActualY = (YPos * (Height + YOffset));

            SetTexture(FirstTexture, GameState->Renderer);
            if(Input->MouseX >= ActualX &&
               Input->MouseX <= (ActualX + Width) &&
               -Input->MouseY <= ActualY &&
               -Input->MouseY >= (ActualY - Height))
            {   
                GameState->MouseOnUI = true; 
                SetTexture(GameState->SphereTexture, GameState->Renderer);
                if(MouseOnClick(Input, LEFT_CLICK))
                {
                    GameState->TextureSelectedIndex = Index;
                }
            }
                 
            World = TranslationMat4({ActualX - WND_WIDTH*0.5f, ActualY + (WND_HEIGHT*0.5f-Height), 0.0f}) * ScaleMat4({50.0f, 50.0f, 0.0f});
            
            SetWorldMat4(GameState->Renderer, World);
            RenderMesh(GameState->UIQuad, GameState->UIShader, GameState->Renderer);
            ++FirstTexture;

            ++XPos;
            if(XPos >= 4)
            {
                XPos = 0;
                --YPos;
            }
        }
    }
    SetProjectionMat4(GameState->Renderer,  GameState->PerspectiveProj);
}



