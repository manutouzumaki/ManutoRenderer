struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float3 norm : NORMAL;
    float3 lightVec : TEXCOORD1;
    float3 viewVec : TEXCOORD2;

};



float4 PS_Main( PS_Input frag ) : SV_TARGET
{ 
    float3 ambientColor = float3( 0.1f, 0.1f, 0.0f );
    float3 lightColor = float3( 0.7f, 0.7f, 0.7f );

    float3 lightVec = normalize( frag.lightVec );
    float3 normal = normalize( frag.norm );

    float diffuseTerm = clamp( dot( normal, lightVec ), 0.0f, 1.0f );
    float specularTerm = 0;

    if( diffuseTerm > 0.0f )
    {
        float3 viewVec = normalize( frag.viewVec );
        float3 halfVec = normalize( lightVec + viewVec );
        specularTerm = pow( saturate( dot( normal, halfVec ) ), 25 );
    }

    float3 finalColor = ambientColor + lightColor *
        diffuseTerm + lightColor * specularTerm;
    return float4( finalColor, 1.0f );
}

