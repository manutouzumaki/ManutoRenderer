#ifndef MAIN_H
#define MAIN_H

#define Assert(condition) if(!(condition)) { *(unsigned int *)0 = 0; } 
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define WND_WIDTH 1280
#define WND_HEIGHT 720

#define LEFT_CLICK 0
#define MIDDLE_CLICK 1
#define RIGHT_CLICK 2
#define SHIFT_CLICK 3

#define SOLID_BACK_CULL 0
#define SOLID_FRONT_CULL 1
#define WIREFRAME 2

#define DEPTH_STENCIL_ON 0
#define DEPTH_STENCIL_OFF 1

static int
StringLength(char * String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return Count;
}

static bool
StringCompare(char *A, char *B, int Size)
{
    bool Result = true;
    for(int Index = 0;
        Index < Size;
        ++Index)
    {
        if(*A++ != *B++)
        {
            Result = false;
        } 
    }
    return Result;
}

struct terrain
{
    int X, Y, Z;
    int Cols;
    int Rows;
    int Size;
    mesh *Mesh;
};

struct meshes_list
{
    mesh *Mesh;
    int Counter;
};

struct game_state
{
    window *Window;
    renderer *Renderer;

    arena RenderArena;
    arena FileArena;
    arena MeshListArena;

    shader *Shader;
    shader *SkyboxShader;

    mat4 Proj; 
    arc_camera Camera;

    mesh *SkyBox;
    texture *SkyBoxTexture;

    mesh *TreeMesh;
    texture *TreeTexture;
    mesh *HouseMesh;
    texture *HouseTexture; 
    mesh *SphereMesh;
    texture *SphereTexture;

    terrain *Terrain;
    texture *TerrainTexture;


    bounding_sphere BoundingSpheres[2];
    
    bool MoveMesh; 
    bounding_sphere *SphereSelected;
    v3 SpherePositionWhenClick;
    v3 Offset;

    meshes_list MeshList;
};  

#endif
