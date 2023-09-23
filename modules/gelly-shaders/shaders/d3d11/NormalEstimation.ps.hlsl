#include "NDCQuadStages.hlsli"
#include "PerFrameCB.hlsli"

struct PS_OUTPUT {
    float4 Normal : SV_TARGET0;
};

Texture2D depth : register(t0);
SamplerState depthSampler {
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

float3 WorldPosFromDepth(float2 uv, float depth) {
    uv.y = 1.f - uv.y;
    float4 clipPos = float4(uv * 2.f - 1.f, depth, 1.0);
    float4 viewPos = mul(clipPos, matInvProj);
    viewPos /= viewPos.w;
    return viewPos.xyz;
}

float3 EstimateNormal(float2 texcoord) {
    float2 texelSize = 1.f / res;
    float c0 = depth.Sample(depthSampler, texcoord).r;
    float l2 = depth.Sample(depthSampler, texcoord - texelSize * 2).r;
    float l1 = depth.Sample(depthSampler, texcoord - texelSize).r;
    float r1 = depth.Sample(depthSampler, texcoord + texelSize).r;
    float r2 = depth.Sample(depthSampler, texcoord + texelSize * 2).r;
    float b2 = depth.Sample(depthSampler, texcoord - texelSize * float2(0, 2)).r;
    float b1 = depth.Sample(depthSampler, texcoord - texelSize * float2(0, 1)).r;
    float t1 = depth.Sample(depthSampler, texcoord + texelSize * float2(0, 1)).r;
    float t2 = depth.Sample(depthSampler, texcoord + texelSize * float2(0, 2)).r;

    float dl = abs(l1 * l2 / (2.0 * l2 - l1) - c0);
    float dr = abs(r1 * r2 / (2.0 * r2 - r1) - c0);
    float db = abs(b1 * b2 / (2.0 * b2 - b1) - c0);
    float dt = abs(t1 * t2 / (2.0 * t2 - t1) - c0);

    float3 ce = WorldPosFromDepth(texcoord, c0);

    float3 dpdx = (dl < dr) ? ce - WorldPosFromDepth(texcoord - texelSize, l1) :
        -ce + WorldPosFromDepth(texcoord + texelSize, r1);
    float3 dpdy = (db < dt) ? ce - WorldPosFromDepth(texcoord - texelSize * float2(0, 1), b1) :
        -ce + WorldPosFromDepth(texcoord + texelSize * float2(0, 1), t1);
    
    // Checks if the derivative is valid
    if (length(dpdx) < 0.0001 || length(dpdy) < 0.0001) {
        return float3(0, 0, 0);
    }

    float3 normal = normalize(cross(dpdx, dpdy));
    //return normal * 0.5 + 0.5;
    float3 lightStart = mul(float4(eye, 1), matView);
    float3 lightEnd = mul(float4(eye + float3(0.3, 0.4, -0.9), 1), matView);
    float3 lightDir = normalize(lightEnd - lightStart);
    float diffuse = saturate(dot(normal, lightDir.xyz));
    float specular = pow(saturate(dot(reflect(-lightDir, normal), normalize(eye - ce))), 16);
    specular *= 4; // Increase specular intensity, just for fun and visualization purposes (this can determine if normals are correct)
    specular = saturate(specular);

    float3 diffuseColor = float3(0.3, 0.3, 0.9);
    float3 specularColor = float3(1, 1, 1);

    float3 color = diffuseColor * diffuse + specularColor * specular;
    return color;
}

PS_OUTPUT main(VS_OUTPUT input) {
    float depthValue = depth.Sample(depthSampler, input.Texcoord).r;
    if (depthValue == 1.f) {
        // Again, r being 1.f is a signal that the pixel is empty.
        discard;
    }

    if (depthValue <= 0.f) {
        // We also don't want to estimate depth which is negative.
        discard;
    }

    PS_OUTPUT output;
    output.Normal = float4(EstimateNormal(input.Texcoord), 1.f);
    return output;
}