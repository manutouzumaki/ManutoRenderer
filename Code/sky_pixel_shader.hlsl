TextureCube colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 tex0 : TEXCOORD0;
};

float4 PS_Main( PS_Input frag ) : SV_TARGET
{
    float4 objectColor = colorMap.Sample(colorSampler, frag.tex0);
    return objectColor;
}

