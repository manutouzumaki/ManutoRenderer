cbuffer CBufferProjection : register(b0)
{
    matrix World;
    matrix Proj;
    matrix View;
    float3 ViewPosition;
    float MemoryData;
};

struct VS_Input
{
   float3 pos : POSITION;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 tex0 : TEXCOORD0;
};

PS_Input VS_Main( VS_Input vertex )
{  
    PS_Input vsOut = (PS_Input)0;
    float4x4 ViewProj = mul(View, Proj);
    vsOut.pos = mul(float4(vertex.pos, 1.0f), ViewProj);
    vsOut.tex0 = float3(vertex.pos.x, -vertex.pos.y, vertex.pos.z);
    return vsOut;

}
