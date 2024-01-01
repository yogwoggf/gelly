struct VS_INPUT {
    float4 Pos : SV_Position;
    float3 Absorption : ABSORPTION;
};

struct GS_OUTPUT {
    linear noperspective centroid float4 Pos : SV_Position;
    float2 Tex : TEXCOORD0;
    float3 Absorption : ABSORPTION;
};

struct PS_OUTPUT {
    float4 ShaderDepth : SV_Target0;
    float4 Albedo : SV_Target1;
    float Depth : SV_Depth;
};