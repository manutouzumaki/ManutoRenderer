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
   float3 norm : NORMAL;
   float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float memoryData : TEXCOORD1;
};

PS_Input VS_Main( VS_Input vertex )
{   
    PS_Input vsOut = (PS_Input)0; 
    // calculate the position of the vertex against the world, and projection matrix
    float4 worldPos = mul(float4(vertex.pos, 1.0f), World);
    vsOut.pos = mul(worldPos, Proj);
    // store the texture coordinates for the pixel shader
    vsOut.tex0 = vertex.tex0;
    vsOut.memoryData = MemoryData;
    return vsOut;

}
