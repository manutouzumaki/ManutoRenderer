Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float3 norm : NORMAL;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
};

float4 PS_Main( PS_Input input ) : SV_TARGET
{
    float4 surfaceColor = colorMap.Sample(colorSampler, input.tex0);
    float3 coolColor = float3(0.0f, 0.0f, 0.55f) + mul(surfaceColor.rgb, 0.25f);
    float3 warmColor = float3(0.3f, 0.3f, 0.0f) + mul(surfaceColor.rgb, 0.25f);
    float3 highlight = float3(0.8f, 0.8f, 0.5f);

    float t = (dot(input.norm, input.lightDir) + 1.0f) / 2.0f;

    float3 r = mul(2.0f*dot(input.norm, input.lightDir), input.norm) - input.lightDir;

    float s = clamp(100.0f*dot(r, input.viewDir)-97.0f, 0.0f, 1.0f);

    float3 finalColor = mul(highlight, s) +  mul(mul(warmColor, t) + mul(coolColor, (1.0f - t)), 1.0f - s);
    
    return float4(finalColor, 1.0f);

}

