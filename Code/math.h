#ifndef MATH_H
#define MATH_H

#include <math.h>

#define PI 3.14159265
#define TWO_PI 6.28318530

struct v2
{
    float X;
    float Y;
};

struct v3
{
    float X, Y, Z;
};

struct mat4
{
    float m[4][4];
};

float 
ToRad(float Angle)
{
    return Angle * (PI/180);
}

float 
ToDeg(float Angle)
{
    return Angle * (180/PI);
}

v2 operator+(v2& A, v2& B)
{
    v2 Result = {};
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

v2 operator-(v2& A, v2& B)
{
    v2 Result = {};
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

v2 operator+(v2& A, float& S)
{
    v2 Result = {};
    Result.X = A.X + S;
    Result.Y = A.Y + S;
    return Result;
}

v2 operator-(v2& A, float& S)
{
    v2 Result = {};
    Result.X = A.X - S;
    Result.Y = A.Y - S;
    return Result;
}

v2 operator*(v2& A, float& S)
{
    v2 Result = {};
    Result.X = A.X * S;
    Result.Y = A.Y * S;
    return Result;
}

v2 operator/(v2& A, float& S)
{
    v2 Result = {};
    Result.X = A.X / S;
    Result.Y = A.Y / S;
    return Result;
}

v3 operator+(v3& A, v3& B)
{
    v3 Result = {};
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;
    return Result;
}

v3 operator-(v3& A, v3& B)
{
    v3 Result = {};
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    return Result;
}

v3 operator+(v3& A, float& S)
{
    v3 Result = {};
    Result.X = A.X + S;
    Result.Y = A.Y + S;
    Result.Z = A.Z + S;
    return Result;
}

v3 operator-(v3& A, float& S)
{
    v3 Result = {};
    Result.X = A.X - S;
    Result.Y = A.Y - S;
    Result.Z = A.Z - S;
    return Result;
}

v3 operator*(v3& A, float& S)
{
    v3 Result = {};
    Result.X = A.X * S;
    Result.Y = A.Y * S;
    Result.Z = A.Z * S;
    return Result;
}

v3 operator/(v3& A, float& S)
{
    v3 Result = {};
    Result.X = A.X / S;
    Result.Y = A.Y / S;
    Result.Z = A.Z / S;
    return Result;
}



mat4 operator*(mat4& A, mat4& B)
{
    mat4 Result;
    for(int Y = 0; Y < 4; Y++)
    {
        for(int X = 0; X < 4; X++)
        {
                Result.m[Y][X] =
                A.m[Y][0] * B.m[0][X] +
                A.m[Y][1] * B.m[1][X] +
                A.m[Y][2] * B.m[2][X] +
                A.m[Y][3] * B.m[3][X];
        }
    }
    return Result;
}

mat4 TransposeMat4(mat4 M)
{
    mat4 Result = {{
        {M.m[0][0], M.m[1][0], M.m[2][0], M.m[3][0]},
        {M.m[0][1], M.m[1][1], M.m[2][1], M.m[3][1]},
        {M.m[0][2], M.m[1][2], M.m[2][2], M.m[3][2]},
        {M.m[0][3], M.m[1][3], M.m[2][3], M.m[3][3]}
    }};
    return Result;
}

mat4 IdentityMat4()
{
    mat4 Result = {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return Result;
}

mat4 TranslationMat4(v3 V)
{
    mat4 Result = {{
        {1.0f, 0.0f, 0.0f, V.X},
        {0.0f, 1.0f, 0.0f, V.Y},
        {0.0f, 0.0f, 1.0f, V.Z},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return Result;
}

mat4 ScaleMat4(v3 V)
{
    mat4 Result = {{
        {V.X,  0.0f, 0.0f, 0.0f},
        {0.0f, V.Y,  0.0f, 0.0f},
        {0.0f, 0.0f, V.Z,  0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return Result;
}

mat4 RotationXMat(float Angle)
{
    mat4 Result = {{
        {1.0f,        0.0f,         0.0f, 0.0f},
        {0.0f, cosf(Angle), sinf(Angle), 0.0f},
        {0.0f, -sinf(Angle),  cosf(Angle), 0.0f},
        {0.0f,        0.0f,         0.0f, 1.0f}
    }};
    return Result;
}

mat4 RotationYMat(float Angle)
{

    mat4 Result = {{
        {cosf(Angle), 0.0f, -sinf(Angle), 0.0f},
        {       0.0f, 1.0f,         0.0f, 0.0f},
        {sinf(Angle), 0.0f,  cosf(Angle), 0.0f},
        {       0.0f, 0.0f,         0.0f, 1.0f}
    }};
    return Result;
}

mat4 RotationZMat(float Angle)
{
    mat4 Result = {{
        {cosf(Angle),   sinf(Angle), 0.0f, 0.0f},
        {-sinf(Angle),  cosf(Angle), 0.0f, 0.0f},
        {       0.0f,         0.0f, 1.0f, 0.0f},
        {       0.0f,         0.0f, 0.0f, 1.0f}
    }};
    return Result;
}

float LengthV2(v2 V)
{
    return sqrtf(V.X*V.X + V.Y*V.Y);
}

v2 NormalizeV2(v2 V)
{
    v2 Result = {};
    Result.X = V.X / LengthV2(V);
    Result.Y = V.Y / LengthV2(V);
    return Result;
}  

float LengthV3(v3 V)
{
    return sqrtf(V.X*V.X + V.Y*V.Y + V.Z*V.Z);
}

v3 NormalizeV3(v3 V)
{
    v3 Result = {};
    Result.X = V.X / LengthV3(V);
    Result.Y = V.Y / LengthV3(V);
    Result.Z = V.Z / LengthV3(V);
    return Result;
}  

float DotV3(v3 A, v3 B)
{
    return (A.X*B.X + A.Y*B.Y + A.Z*B.Z);
}

v3 CrossV3(v3 A, v3 B)
{
    v3 Result = {};
    Result.X = A.Y*B.Z - A.Z*B.Y; 
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;
    return Result;
}

mat4 ViewMat4(v3 eye, v3 target, v3 up)
{
    v3 z = NormalizeV3(target - eye);
    v3 x = NormalizeV3(CrossV3(up, z));
    v3 y = CrossV3(z, x);
    mat4 Result = {{
        { x.X,              y.X,              z.X,             0.0f},
        { x.Y,              y.Y,              z.Y,             0.0f},
        { x.Z,              y.Z,              z.Z,             0.0f},
        {-DotV3(x, eye),   -DotV3(y, eye),   -DotV3(z, eye),   1.0f}
    }}; 
    Result = TransposeMat4(Result);
    return Result; 
}

mat4
PerspectiveProjMat4(float YFov, float Aspect, float ZNear, float ZFar)
{
    float YScale = 1.0f/tanf(YFov*0.5);
    float XScale = YScale / Aspect;
    mat4 Result = {{
        {XScale, 0.0f, 0.0f                      , 0.0f},
        {0.0f, YScale, 0.0f                      , 0.0f},
        {0.0f, 0.0f  , ZFar/(ZFar - ZNear)       , 1.0f},
        {0.0f, 0.0f  , -ZNear*ZFar/(ZFar - ZNear), 0.0f}
    }};
    Result = TransposeMat4(Result);
    return Result;
}

mat4 OrthogonalProjMat4(int width, int height, float znear, float zfar)
{
    float w = (float)width;
    float h = (float)height;
    
    mat4 result = {{
        {2.0f / w, 0.0f,     0.0f,                   0.0f},
        {0.0f,     2.0f / h, 0.0f,                   0.0f},
        {0.0f,     0.0f,     1.0f / (zfar - znear),  0.0f},
        {0.0f,     0.0f,     znear / (znear - zfar), 1.0f}
    }};
    return result;
}

v2 LerpV2(v2 A, v2 B, float T)
{
    return A + ((B - A) * T);
}

static float
NormalizeAngle(float Angle)
{
    Angle = fmodf(Angle, 2.0f*PI);
    if(Angle < 0)
    {
        Angle = (2 * PI) + Angle;
    }
    return Angle;
}

#endif
