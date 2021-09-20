#ifndef ARC_CAMERA_H
#define ARC_CAMERA_H

struct arc_camera
{
    mat4 View;
    v3 Target;
    v3 Position;
    v3 Front;
    v3 Up;
    v3 Right;
    v3 RealUp; 
    v3 PosRelativeToTarget;
    float Yaw;
    float Pitch;
    float Distance;
};

#endif
