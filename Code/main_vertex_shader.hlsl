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
    float3 viewDir : TEXCOORD2;
    float3 lightDir : TEXCOORD3;
};

PS_Input VS_Main( VS_Input vertex )
{   
    PS_Input vsOut = (PS_Input)0;
    // calculate the position of the vertex against the world, and projection matrix
    float4 worldPos = mul(float4(vertex.pos, 1.0f), World);
    vsOut.pos = mul(worldPos, View);
    vsOut.pos = mul(vsOut.pos, Proj);
    
    //float3 viewPosition = float3(10.0f, 0.0f, 0.0f);
    float3 lightPosition = float3(3.0f, 10.0f, 0.0f);
    float4 lightDirection = float4(lightPosition, 1.0f) - worldPos;
    float4 viewDirection = float4(ViewPosition, 1.0f) - worldPos;

    // store the texture coordinates for the pixel shader
    vsOut.tex0 = vertex.tex0;
    
    // calculate the normal vector against the world matrix only
    vsOut.norm = mul(vertex.norm, (float3x3)World);
    vsOut.norm = normalize(vsOut.norm);

    vsOut.viewDir = normalize(viewDirection.xyz);
    vsOut.lightDir = normalize(lightDirection.xyz);

    return vsOut;

}
