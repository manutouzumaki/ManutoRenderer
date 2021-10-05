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
    float TAU = 6.28318530718f;
     
    input.tex0.y += input.Time * 0.1f;
    float XOffset = cos(input.tex0.x* TAU * 5) * 0.01f;
    float t0 = cos((input.tex0.y + XOffset) * TAU * 10) * 0.5f + 0.5f;
    return t0 * (abs(input.norm.y) < 0.999f);

/*
    input.tex0.x += input.Time;
    float4 surfaceColor = colorMap.Sample(colorSampler, input.tex0);
    return surfaceColor;
*/
}
