cbuffer CBufferProjection : register(b0)
{
   matrix World;
   matrix Proj;
   matrix View;
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
    float3 norm : NORMAL;
    float3 lightVec : TEXCOORD1;
    float3 viewVec : TEXCOORD2;
};

PS_Input VS_Main( VS_Input vertex )
{   
    float3 cameraPos = float3(0.0f, 2.0f, 5.0f);
    PS_Input vsOut = (PS_Input)0;
    float4 worldPos = mul(float4(vertex.pos, 1.0f), World);
    vsOut.pos = mul(worldPos, View);
    vsOut.pos = mul(vsOut.pos, Proj);

    vsOut.tex0 = vertex.tex0;
    vsOut.norm = mul(vertex.norm, (float3x3)World);
    vsOut.norm = normalize(vsOut.norm);

    float3 lightPos = float3(0.0f, 5.0f, 5.0f);
    vsOut.lightVec = normalize(lightPos - worldPos.xyz);
    vsOut.viewVec = normalize(cameraPos - worldPos.xyz);
    return vsOut;

}
