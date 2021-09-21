
struct ModelViewProjection
{
    matrix mat;
};

ConstantBuffer<ModelViewProjection> mvp : register(b0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};



PSInput vsmain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = mul(mvp.mat, position);
    result.color = color;

    return result;
}


float4 psmain(PSInput input) : SV_TARGET
{
    return input.color;
}