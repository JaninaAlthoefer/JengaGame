Texture2D ObjTexture;
SamplerState ObjSamplerState;
TextureCube SkyBox;

struct Light
{
	float3 dir;
	float3 pos;
	float range;
	float3 attenuation;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame
{
	Light light;
};

cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 normal: NORMAL;
};

struct SKYBOX_VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 normal : NORMAL)
{
    VS_OUTPUT output;

    output.Pos = mul(inPos, WVP);
	output.WorldPos = mul(inPos, World);
	output.normal = mul(normal, World);
    output.TexCoord = inTexCoord;

    return output;
}

SKYBOX_VS_OUTPUT SKYBOX_VS(float3 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 normal : NORMAL)
{
	SKYBOX_VS_OUTPUT output = (SKYBOX_VS_OUTPUT)0;

	output.Pos = mul(float4(inPos, 1.0f), WVP).xyww;
	output.TexCoord = inPos;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float4 trans = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

	float3 lightToPixelVector = light.pos - input.WorldPos;
	float dist = length(lightToPixelVector);
	float3 finalAmbientLight = trans * light.ambient;

	if(dist > light.range )
		return float4(finalAmbientLight, trans.a);

	lightToPixelVector = lightToPixelVector / dist;
	float howMuchLight = dot(lightToPixelVector, input.normal);

	if(howMuchLight > 0.0f)
	{
		finalColor += (howMuchLight*trans*light.diffuse);

		finalColor = finalColor / (light.attenuation[0]+(light.attenuation[1]*dist) + (light.attenuation[2]*dist*dist));
	}

	finalColor = saturate(finalColor + finalAmbientLight);

	return float4(finalColor, trans.a);
}

float4 SKYBOX_PS(SKYBOX_VS_OUTPUT input) : SV_TARGET
{
	return SkyBox.Sample(ObjSamplerState, input.TexCoord);
}

float4 TEX2D_PS(VS_OUTPUT input) : SV_TARGET
{
	
	float4 trans = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
    
	if (trans.a <= 0.0f)
		discard;
	
	return trans;
}
