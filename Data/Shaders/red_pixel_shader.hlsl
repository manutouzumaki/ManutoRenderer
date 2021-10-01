Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
};

float4 PS_Main( PS_Input input ) : SV_TARGET
{
    float3 finalColor = float3(0.0f, 1.0f, 0.5f);
    return float4(finalColor, 1.0f);
}
