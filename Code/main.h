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
#define SOLID_NONE_CULL 3

#define DEPTH_STENCIL_ON 0
#define DEPTH_STENCIL_OFF 1

enum states
{
    ENTITY_EDITOR,
    ENTITY_SELECTOR,
};

enum move_mesh_state
{
    ALL_AXIS,
    X_AXIS,
    Y_AXIS,
    Z_AXIS
};

enum ui_state
{
    MESH_SELECTED,
    TEXTURE_SELECTED,
    SHADER_SELECTED,
    ALPHA_SELECTED,
};

#define BIT(Value) (1 << Value)

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

struct texture_list
{
    texture *Texture;
    int Counter;
};

struct shader_list
{
    shader *Shader;
    int Counter;
};

struct entity
{
    int ID;
    v3 Position;
    v3 Scale;
    v3 Rotation;
    bounding_sphere BoundingSphere;
    int MeshIndex;
    int TextureIndex;
    int ShaderIndex;
    bool HasAlpha;
};

struct entity_list
{
    entity *Entities;
    int Counter;
};

struct ui_constant_buffer_data
{
    float MemoryData;
};

struct game_state
{
    window *Window;
    renderer *Renderer;
    unsigned int StateBitField;
    
    // arenas
    arena RenderArena;
    arena FileArena;
    arena MeshListArena;
    arena TextureListArena;
    arena ShaderListArena;
    arena EntityArena;
    // shaders
    shader *Shader;
    shader *SkyboxShader;
    shader *UIShader;
    shader *MemoryUIShader;
    // uiniform
    ui_constant_buffer_data UIData;
    constant_buffer *UIBuffer;  

    mat4 PerspectiveProj;
    mat4 OrthogonalProj; 
    arc_camera Camera;

    mesh *SkyBox;
    texture *SkyBoxTexture;
    mesh *UIQuad;
    texture *UITexture;
    mesh *SphereMesh;
    texture *SphereTexture;
    terrain *Terrain;
    texture *TerrainTexture;
    mesh *CubeMesh;
 
    bool MouseOnUI; 
    bool MoveMesh; 
    int MoveMeshState;
    int EntitySelectedID;
    v3 SpherePositionWhenClick;
    v3 Offset;

    entity_list EntityList;
    
    meshes_list MeshList;
    texture_list TextureList;
    shader_list ShaderList;
    
    int UIStateSelected;
    int MeshSelectedIndex;
    int TextureSelectedIndex;
    int ShaderSelectedIndex;
    int AlphaValueSelected;
};  

#endif
