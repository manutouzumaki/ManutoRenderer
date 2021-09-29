Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};

float4 PS_Main( PS_Input input ) : SV_TARGET
{
    float4 surfaceColor = colorMap.Sample(colorSampler, input.tex0);
    return surfaceColor;

}

