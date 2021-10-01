Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float3 viewDir : TEXCOORD2;
    float3 lightDir : TEXCOORD3;
};

float InverseLerp(float A, float B, float V)
{
    return (V -A)/(B - A);
}

float4 PS_Main( PS_Input input ) : SV_TARGET
{
    // Tests
    // float (32 bits float)
    // half (16 bit float)
    // float4 -> half4
    // float4x4 -> half4x4
    // bool 0 - 1 -> false - true    
    //float TAU = 6.28318530718f;
    //float XOffset = cos(input.tex0.x* TAU * 5) * 0.01f;
    //float t0 = cos((input.tex0.y + XOffset) * TAU * 10) * 0.5f + 0.5f;
    //t0 *= 1-input.tex0.y; 
    //return float4(t0, t0, t0, 1.0f);

    // real shader 
    float4 surfaceColor = colorMap.Sample(colorSampler, input.tex0);
    float3 coolColor = float3(0.0f, 0.0f, 0.55f) + mul(surfaceColor.rgb, 0.25f);
    float3 warmColor = float3(0.3f, 0.3f, 0.0f) + mul(surfaceColor.rgb, 0.25f);
    float3 highlight = float3(0.8f, 0.8f, 0.5f);
    float t = (dot(input.norm, input.lightDir) + 1.0f) / 2.0f;
    float3 r = mul(2.0f*dot(input.norm, input.lightDir), input.norm) - input.lightDir;
    float s = saturate(100.0f*dot(r, input.viewDir)-97.0f);
    float3 finalColor = mul(highlight, s) +  mul(mul(warmColor, t) + mul(coolColor, (1.0f - t)), 1.0f - s);
    return float4(finalColor, 1.0f);


}
