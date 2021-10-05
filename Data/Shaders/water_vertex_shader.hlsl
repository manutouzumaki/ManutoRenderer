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
    float2 tex0 : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float localYPos : TEXCOORD2;
};

PS_Input VS_Main( VS_Input vertex )
{   
    PS_Input vsOut = (PS_Input)0;
    // calculate the position of the vertex against the world, and projection matrix
    float TAU = 6.28318530718f;
    // create the waves
    vertex.tex0.y += Time * 0.1f;
    float XOffset = cos(vertex.tex0.x* TAU * 5) * 0.01f;
    float t0 = cos((vertex.tex0.y + XOffset) * TAU * 10) * 0.01f + 0.01f;
    float t1 = cos((vertex.tex0.x + XOffset) * TAU * 10) * 0.01f + 0.01f; 
    vertex.pos.y = t0 + t1;
    t0 = cos((vertex.tex0.y + XOffset) * TAU * 10) * 0.05f + 0.01f;
    t1 = cos((vertex.tex0.x + XOffset) * TAU * 10) * 0.05f + 0.01f;
    vsOut.localYPos = t0 + t1 + 0.5f;

    float4 worldPos = mul(float4(vertex.pos, 1.0f), World);
    vsOut.pos = mul(worldPos, View);
    vsOut.pos = mul(vsOut.pos, Proj);
    
    vsOut.tex0 = vertex.tex0;
    // calculate the normal vector against the world matrix only
    vsOut.norm = mul(vertex.norm, (float3x3)World);
    vsOut.norm = normalize(vertex.norm);

    vsOut.tex0 .y += Time * 0.1f;
    return vsOut;

}
