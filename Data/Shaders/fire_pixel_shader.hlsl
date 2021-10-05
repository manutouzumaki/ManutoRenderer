Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float2 tex0 : TEXCOORD1;
    float Time : TEXCOORD2;
    float3 norm : TEXCOORD3;
};

float4 PS_Main( PS_Input input ) : SV_TARGET
{
    float4 surfaceColor = colorMap.Sample(colorSampler, input.tex0.yx);
    return surfaceColor * (abs(input.norm.y) < 0.999f);
}
