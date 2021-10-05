cbuffer CBufferProjection : register(b0)
{
    matrix World;
    matrix Proj;
    matrix View;
    float3 ViewPosition;
    float Time;
};

struct VS_Input
{
   float3 pos : POSITION;
   float2 tex0 : TEXCOORD0;
   float3 norm : NORMAL;
};

struct PS_Input
{
    float4 pos : SV_POSITION;  // clip space position
    float3 worldPos : TEXCOORD0;
    float2 tex0 : TEXCOORD1;
    float Time : TEXCOORD2;
    float3 norm : TEXCOORD3;
};

PS_Input VS_Main( VS_Input vertex )
{   
    PS_Input vsOut = (PS_Input)0;
    // calculate the position of the vertex against the world, and projection matrix
    float4 worldPos = mul(float4(vertex.pos, 1.0f), World);
    vsOut.pos = mul(worldPos, View);
    vsOut.pos = mul(vsOut.pos, Proj);
    vsOut.worldPos = worldPos.xyz;
    vsOut.tex0 = vertex.tex0;
    // calculate the normal vector against the world matrix only
    //vsOut.norm = mul(vertex.norm, (float3x3)World);
    vsOut.norm = normalize(vertex.norm);
    vsOut.Time = Time;

    return vsOut;

}
