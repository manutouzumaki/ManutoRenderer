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
    float Yuv = input.tex0.y;
    input.tex0.y = 1 - input.tex0.y;
    input.tex0.x /= 5;
    input.Time *= 10.0f;
    unsigned int timeInInt = (unsigned int)input.Time;
    timeInInt = timeInInt % 5;
    input.tex0.x += 0.2f * timeInInt; 
    float4 surfaceColor = colorMap.Sample(colorSampler, input.tex0.xy);
    surfaceColor.w *= Yuv;
    return surfaceColor * (abs(input.norm.y) < 0.999f);
}
