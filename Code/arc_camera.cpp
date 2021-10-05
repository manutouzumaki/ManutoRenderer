static void 
InitializeCamera(arc_camera *Camera)
{
    Camera->Position = {0.0f, 0.0f, 1.0f};
    Camera->Target = {0.0f, 0.0f, -10.0f};
    Camera->Front = NormalizeV3(Camera->Target - Camera->Position);
    Camera->PosRelativeToTarget = Camera->Target - Camera->Position;
    Camera->Up = {0.0f, 1.0f, 0.0f};
    Camera->Right = NormalizeV3(CrossV3(Camera->Up, Camera->Front));
    Camera->RealUp = {0.0f, 1.0f, 0.0f};
    Camera->Yaw = ToRad(-90.0f);
    Camera->Pitch = 0.0f;
}

static void
UpdateCameraView(arc_camera *Camera, app_input *Input)
{
    Camera->RealUp = NormalizeV3(CrossV3(Camera->Front, Camera->Right));
    Camera->View = ViewMat4(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
}

static void 
ProcessCameraRotation(app_input *Input, arc_camera *Camera, float DeltaTime)
{
    float MouseOffsetX = 0;
    float MouseOffsetY = 0;
    MouseOffsetX = (Input->MouseX - (WND_WIDTH/2.0f)) * DeltaTime;
    MouseOffsetY = (Input->MouseY - (WND_HEIGHT/2.0f)) * DeltaTime; 
    PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY); 

    Camera->Yaw -= MouseOffsetX * 0.5f;
    Camera->Pitch -= MouseOffsetY * 0.5f;

    if(Camera->Pitch > ToRad(89.0f))
    {
        Camera->Pitch = ToRad(89.0f);
    }
    if(Camera->Pitch < ToRad(-89.0f))
    {
        Camera->Pitch = ToRad(-89.0f);
    }

    v3 Front = {};
    Front.X = cosf(Camera->Yaw) * cosf(Camera->Pitch);
    Front.Y = sinf(Camera->Pitch);
    Front.Z = sinf(Camera->Yaw) * cosf(Camera->Pitch);
    Camera->Front = NormalizeV3(Front);
    Camera->Right = NormalizeV3(CrossV3(Camera->Up, Camera->Front));
            
    v4 CameraRelativeToTarget4V = V3ToV4(Camera->PosRelativeToTarget, 1.0f);        
    CameraRelativeToTarget4V = RotationYMat(Camera->Yaw + ToRad(90.0f)) * CameraRelativeToTarget4V;
    CameraRelativeToTarget4V = RotationV3Mat({Camera->Right}, Camera->Pitch) * CameraRelativeToTarget4V;
    v3 Result = V4ToV3(CameraRelativeToTarget4V);
    Camera->Position = Camera->Target - Result;
}

static void 
ProcessCameraMovement(app_input *Input, arc_camera *Camera, float DeltaTime)
{
    float MouseOffsetX = 0;
    float MouseOffsetY = 0;
    MouseOffsetX = (Input->MouseX - (WND_WIDTH/2.0f)) * DeltaTime;
    MouseOffsetY = (Input->MouseY - (WND_HEIGHT/2.0f)) * DeltaTime; 
    PlatformSetCursorPosition(Input->MouseDefaultX, Input->MouseDefaultY); 
    
    Camera->Position = Camera->Position - Camera->Right * MouseOffsetX;
    Camera->Position = Camera->Position + Camera->RealUp * MouseOffsetY;
    Camera->Target = Camera->Target - Camera->Right * MouseOffsetX;
    Camera->Target = Camera->Target + Camera->RealUp * MouseOffsetY;
    Camera->Front = NormalizeV3(Camera->Target - Camera->Position);
}

static void
ProcessCameraDistance(app_input *Input, arc_camera *Camera, float DeltaTime)
{
    if(Input->MouseWheel != 0)
    {
        v3 NewPosition = Camera->Position + (Camera->Front * Input->MouseWheel) * DeltaTime;
        float NewDistance = LengthV3(Camera->Target - NewPosition);
        v3 CameraToTarget = NormalizeV3(Camera->Target - NewPosition);
        if(DotV3(CameraToTarget, Camera->Front) >= 0.0f)
        {
            Camera->Position = NewPosition;
            v3 Temp = NormalizeV3(Camera->PosRelativeToTarget);
            Temp = Temp * NewDistance;
            Camera->PosRelativeToTarget = Temp;
        }
    }
}


