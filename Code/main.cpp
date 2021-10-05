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
        bounding_sphere *SphereSelected = &ActualEntity->BoundingSphere;
        GameState->EntitySelectedID = ActualEntity->ID;
        GameState->SpherePositionWhenClick = SphereSelected->Position;
        v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->SpherePositionWhenClick, GameState->PerspectiveProj);
        GameState->Offset = MousePositionOnPlane - SphereSelected->Position;
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
ProcessEntitySetShader(app_input *Input, game_state *GameState)
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
        ActualEntity->ShaderIndex = GameState->ShaderSelectedIndex;
    }
    PlatformFreeMemory(TValues);
}

static void
ProcessEntitySetAlpha(app_input *Input, game_state *GameState)
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
        ActualEntity->HasAlpha = GameState->AlphaValueSelected;
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
LoadShaderFromFileExplorer(game_state *GameState, renderer *Renderer, arena *Arena)
{
    void *VertexShaderData = NULL;
    void *PixelShaderData = NULL;
    int VertexShaderSize = 0;
    int PixelShaderSize = 0;
    if(BasicFileOpenTest(&VertexShaderData, Arena))
    {
        VertexShaderSize = StringLength((char *)VertexShaderData);
        if(BasicFileOpenTest(&PixelShaderData, Arena))
        {
            PixelShaderSize = StringLength((char *)PixelShaderData); 
            GameState->ShaderList.Shader = 
                PlatformCreateShadersFromData(Renderer,
                                              VertexShaderData, VertexShaderSize, "VS_Main",
                                              PixelShaderData, PixelShaderSize, "PS_Main",
                                              &GameState->ShaderListArena); 
            if(GameState->ShaderList.Shader)
            {
                OutputDebugString("Shader Loaded!\n");
            }
            ++GameState->ShaderList.Counter;
        }
    }
}

static void
OrderEntitiesByDistance(game_state *GameState, entity *Entity, int Count, v3 ViewPosition)
{
    entity *FirstEntity = Entity;
    FirstEntity -= (Count - 1);
 
    for(int I = 0;
        I < Count; 
        ++I)
    {
        for(int J = 0;
            J < Count;
            ++J)
        {
            if(I != J)
            {
                entity *ActualEntity = FirstEntity + I;
                entity *EntityToCheck = FirstEntity + J;
                float ActualEntityDistance = LengthV3(ViewPosition - ActualEntity->Position);
                float EntityToCheckDistance = LengthV3(ViewPosition - EntityToCheck->Position);
                if(ActualEntityDistance > EntityToCheckDistance)
                {
                    entity TempEntity = *ActualEntity;
                    *ActualEntity = *EntityToCheck;
                    *EntityToCheck = TempEntity; 
                }
            }
        }
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

        if(Input->KeyboardKeys->Keys['X'].IsDown)
        {
            GameState->MoveMeshState = X_AXIS;
        } 
        if(Input->KeyboardKeys->Keys['Y'].IsDown)
        {
            GameState->MoveMeshState = Y_AXIS;
        }
        if(Input->KeyboardKeys->Keys['Z'].IsDown)
        {
            GameState->MoveMeshState = Z_AXIS;
        }
        if(Input->KeyboardKeys->Keys['A'].IsDown)
        {
            GameState->MoveMeshState = ALL_AXIS;
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
        if(Input->KeyboardKeys->Keys['S'].IsDown)
        {
            LoadShaderFromFileExplorer(GameState, GameState->Renderer, &GameState->FileArena);
        }
        // when left-click on the screen we have to add an entity in that position
        // to the eneity-list
        if(!GameState->MouseOnUI)
        {
            if(MouseOnClick(Input, LEFT_CLICK))
            {
                if(GameState->UIStateSelected == MESH_SELECTED &&
                   GameState->MeshList.Counter > 0)
                {
                    v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera, GameState->Camera.Target, GameState->PerspectiveProj);
                    GameState->EntityList.Entities = (entity *)PushStruct(&GameState->EntityArena, entity);
                    GameState->EntityList.Entities->Position = MousePositionOnPlane;
                    GameState->EntityList.Entities->MeshIndex = GameState->MeshSelectedIndex;
                    GameState->EntityList.Entities->TextureIndex = -1;
                    GameState->EntityList.Entities->ShaderIndex = -1;
                    GameState->EntityList.Entities->BoundingSphere.Position = GameState->EntityList.Entities->Position;
                    GameState->EntityList.Entities->BoundingSphere.Radius = 1.0f;
                    GameState->EntityList.Entities->ID = GameState->EntityList.Counter; 
                    GameState->EntityList.Entities->HasAlpha = false;
                    ++GameState->EntityList.Counter; 
                }
                if(GameState->UIStateSelected == TEXTURE_SELECTED &&
                   GameState->TextureList.Counter > 0)
                {
                    ProcessEntitySetTexture(Input, GameState);
                }
                if(GameState->UIStateSelected == SHADER_SELECTED &&
                   GameState->ShaderList.Counter > 0)
                {
                    ProcessEntitySetShader(Input, GameState);
                }
                if(GameState->UIStateSelected == ALPHA_SELECTED)
                {
                    ProcessEntitySetAlpha(Input, GameState);
                } 
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
    InitArena(Memory, &GameState->ShaderListArena,  Kilobytes(1));
    
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
            // Initialize constant Buffers
            CreateConstantBuffer(GameState->UIBuffer, ui_constant_buffer_data, GameState->Renderer, &GameState->RenderArena);

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
        char Buffer[100];
        sprintf(Buffer, "MoveMeshState: %d\n", GameState->MoveMeshState);
        OutputDebugString(Buffer);

        entity *FirstEntity = GameState->EntityList.Entities;
        FirstEntity -= (GameState->EntityList.Counter - 1);
        entity *EntitySelected = FirstEntity; 
        for(int Index = 0;
            Index < GameState->EntityList.Counter;
            ++Index)
        {
            if(EntitySelected->ID == GameState->EntitySelectedID)
            {
                break;
            }
            ++EntitySelected;
        }

        v3 MousePositionOnPlane = MouseRayCameraPlaneIntersection(Input, &GameState->Camera,
                                                                  GameState->SpherePositionWhenClick,
                                                                  GameState->PerspectiveProj);
        switch(GameState->MoveMeshState)
        {
            case ALL_AXIS:
            {
                EntitySelected->BoundingSphere.Position = MousePositionOnPlane - GameState->Offset;
                EntitySelected->Position = EntitySelected->BoundingSphere.Position;
            }break;
            case X_AXIS:
            {
                EntitySelected->BoundingSphere.Position.X = MousePositionOnPlane.X - GameState->Offset.X;
                EntitySelected->Position.X = EntitySelected->BoundingSphere.Position.X;
            }break;
            case Y_AXIS:
            {
                EntitySelected->BoundingSphere.Position.Y = MousePositionOnPlane.Y - GameState->Offset.Y;
                EntitySelected->Position.Y = EntitySelected->BoundingSphere.Position.Y;
            }break;
            case Z_AXIS:
            {
                EntitySelected->BoundingSphere.Position.Z = MousePositionOnPlane.Z - GameState->Offset.Z;
                EntitySelected->Position.Z = EntitySelected->BoundingSphere.Position.Z;
            }break;
        }

    }

    // Render...
    static float Time = 0.0f;
    SetTime(GameState->Renderer, Time);
    Time += DeltaTime;
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
    
    // render terrain
    mat4 World = TranslationMat4({0.0f, 0.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->TerrainTexture, GameState->Renderer);
    RenderMeshIndexed(GameState->Terrain->Mesh, GameState->Shader, GameState->Renderer);

    // render target position
    World = TranslationMat4(GameState->Camera.Target) * ScaleMat4({0.3f, 0.3f, 0.3f});
    SetWorldMat4(GameState->Renderer, World);
    SetTexture(GameState->SphereTexture, GameState->Renderer);
    RenderMesh(GameState->SphereMesh, GameState->Shader, GameState->Renderer);

    // render entities
    // fist we render all the solid opaque entities
    if(GameState->EntityList.Counter > 0)
    {
        entity *FirstEntity = GameState->EntityList.Entities;
        FirstEntity -= (GameState->EntityList.Counter - 1);
        for(int Index = 0;
            Index < GameState->EntityList.Counter;
            ++Index)
        {
            if(!FirstEntity->HasAlpha)
            {
                mesh *FirstMesh = GameState->MeshList.Mesh;
                FirstMesh -= (GameState->MeshList.Counter - 1);
                
                texture *FirstTexture = GameState->TextureList.Texture;
                FirstTexture -= (GameState->TextureList.Counter - 1);

                shader *FirstShader = GameState->ShaderList.Shader;
                FirstShader -= (GameState->ShaderList.Counter - 1);

                mesh *ActualMesh = FirstMesh + FirstEntity->MeshIndex;
                texture *ActualTexture = FirstTexture + FirstEntity->TextureIndex;
                if(GameState->TextureList.Counter <= 0 || FirstEntity->TextureIndex == -1) ActualTexture = GameState->SphereTexture;
                shader *ActualShader = FirstShader + FirstEntity->ShaderIndex;
                if(GameState->ShaderList.Counter <= 0 || FirstEntity->ShaderIndex == -1) ActualShader = GameState->Shader;

                World = TranslationMat4(FirstEntity->Position);
                SetWorldMat4(GameState->Renderer, World);
                SetTexture(ActualTexture, GameState->Renderer);
                RenderMesh(ActualMesh, ActualShader, GameState->Renderer);
            }
            ++FirstEntity;
        } 
    }
    // then render all transparent and translucid entities
    SetAlphaBlend(GameState->Renderer, true);
    if(GameState->EntityList.Counter > 0)
    {
        if(GameState->EntityList.Counter >= 3)
        {
            OrderEntitiesByDistance(GameState, GameState->EntityList.Entities, GameState->EntityList.Counter, GameState->Camera.Position);
        }
        entity *FirstEntity = GameState->EntityList.Entities;
        FirstEntity -= (GameState->EntityList.Counter - 1);

        for(int Index = 0;
            Index < GameState->EntityList.Counter;
            ++Index)
        {
            if(FirstEntity->HasAlpha)
            {
                mesh *FirstMesh = GameState->MeshList.Mesh;
                FirstMesh -= (GameState->MeshList.Counter - 1);
                
                texture *FirstTexture = GameState->TextureList.Texture;
                FirstTexture -= (GameState->TextureList.Counter - 1);

                shader *FirstShader = GameState->ShaderList.Shader;
                FirstShader -= (GameState->ShaderList.Counter - 1);

                mesh *ActualMesh = FirstMesh + FirstEntity->MeshIndex;
                texture *ActualTexture = FirstTexture + FirstEntity->TextureIndex;
                if(GameState->TextureList.Counter <= 0 || FirstEntity->TextureIndex == -1) ActualTexture = GameState->SphereTexture;
                shader *ActualShader = FirstShader + FirstEntity->ShaderIndex;
                if(GameState->ShaderList.Counter <= 0 || FirstEntity->ShaderIndex == -1) ActualShader = GameState->Shader;

                World = TranslationMat4(FirstEntity->Position);
                SetWorldMat4(GameState->Renderer, World);
                SetTexture(ActualTexture, GameState->Renderer);

                SetFillType(GameState->Renderer, SOLID_FRONT_CULL);
                RenderMesh(ActualMesh, ActualShader, GameState->Renderer);
                SetFillType(GameState->Renderer, SOLID_BACK_CULL);
                RenderMesh(ActualMesh, ActualShader, GameState->Renderer);
            }
            ++FirstEntity;
        } 
    }
    SetAlphaBlend(GameState->Renderer, false);

    // UI Update and Render
    // set the projection matrix to orthogonal for 2d rendering
    SetProjectionMat4(GameState->Renderer,  GameState->OrthogonalProj);     
    // memory data bars
    // first we must get the range of memory used 
    // we have to render every arena memory bar 
    SetTexture(GameState->SphereTexture, GameState->Renderer); 
    // FILE ARENA
    float XPos = (WND_WIDTH*0.5f) - 220.0f;
    float YPos = (WND_HEIGHT*0.5f) - 30.0f;
    GameState->UIData.MemoryData = (float)((double)GameState->FileArena.Use / (double)GameState->FileArena.Size);
    MapConstantBuffer(GameState->Renderer->RenderContext, GameState->UIData, ui_constant_buffer_data, GameState->UIBuffer->Buffer, 1);
    World = TranslationMat4({XPos, YPos, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer); 
    // RENDER ARENA
    GameState->UIData.MemoryData = (float)((double)GameState->RenderArena.Use / (double)GameState->RenderArena.Size);
    MapConstantBuffer(GameState->Renderer->RenderContext, GameState->UIData, ui_constant_buffer_data, GameState->UIBuffer->Buffer, 1);
    World = TranslationMat4({XPos, YPos - 30.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // MESH ARENA
    GameState->UIData.MemoryData = float((double)GameState->MeshListArena.Use / (double)GameState->MeshListArena.Size);
    MapConstantBuffer(GameState->Renderer->RenderContext, GameState->UIData, ui_constant_buffer_data, GameState->UIBuffer->Buffer, 1);
    World = TranslationMat4({XPos, YPos - 60.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // TEXTURE ARENA
    GameState->UIData.MemoryData = float((double)GameState->TextureListArena.Use / (double)GameState->TextureListArena.Size);
    MapConstantBuffer(GameState->Renderer->RenderContext, GameState->UIData, ui_constant_buffer_data, GameState->UIBuffer->Buffer, 1);
    World = TranslationMat4({XPos, YPos - 90.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // SHADER ARENA
    GameState->UIData.MemoryData = float((double)GameState->ShaderListArena.Use / (double)GameState->ShaderListArena.Size);
    MapConstantBuffer(GameState->Renderer->RenderContext, GameState->UIData, ui_constant_buffer_data, GameState->UIBuffer->Buffer, 1);
    World = TranslationMat4({XPos, YPos - 120.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
    SetWorldMat4(GameState->Renderer, World);
    RenderMesh(GameState->UIQuad, GameState->MemoryUIShader, GameState->Renderer);
    // ENTITY ARENA
    GameState->UIData.MemoryData = float((double)GameState->EntityArena.Use / (double)GameState->EntityArena.Size);
    MapConstantBuffer(GameState->Renderer->RenderContext, GameState->UIData, ui_constant_buffer_data, GameState->UIBuffer->Buffer, 1);
    World = TranslationMat4({XPos, YPos - 150.0f, 0.0f}) * ScaleMat4({200.0f, 20.0f, 0.0f});
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
                    GameState->UIStateSelected = MESH_SELECTED;
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
        --YPos;// = (WND_HEIGHT*-0.5f) / Height;

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
                    GameState->UIStateSelected = TEXTURE_SELECTED;
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

        // shader list ui 
        XPos = 0.0f;
        --YPos;// = ((WND_HEIGHT*-0.5f) + ((WND_HEIGHT*-0.5f)*0.5f)) / Height;

        shader *FirstShader = GameState->ShaderList.Shader;
        FirstShader -= (GameState->ShaderList.Counter - 1);

        for(int Index = 0;
            Index < GameState->ShaderList.Counter;
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
                    GameState->ShaderSelectedIndex = Index;
                    GameState->UIStateSelected = SHADER_SELECTED;
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

        // alpha render ui 
        XPos = 0.0f;
        --YPos;// = ((WND_HEIGHT*-0.5f) + ((WND_HEIGHT*-0.5f)*0.5f)) / Height;

        for(int Index = 0;
            Index < 2;
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
                    GameState->AlphaValueSelected = Index;
                    GameState->UIStateSelected = ALPHA_SELECTED;
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



