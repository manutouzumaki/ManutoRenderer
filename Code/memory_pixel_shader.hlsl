Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float memoryData : TEXCOORD1;
};

float4 PS_Main( PS_Input input ) : SV_TARGET
{
    float healthBarMask = input.memoryData > input.tex0.x;
    
    float3 StartColor = float3(0.0f, 1.0f, 0.0f);
    float3 EndColor = float3(1.0f, 0.0f, 0.0f);
    float3 BKColor = float3(0.1f, 0.1f, 0.1f);
    
    // this is the actual lerp operation
    // StartColor + mul(EndColor - StartColor, health);
    float3 finalColor =  lerp(StartColor, EndColor, input.memoryData); 
    finalColor = lerp(BKColor, finalColor, healthBarMask); 

    return float4(finalColor, 1.0f);
}
