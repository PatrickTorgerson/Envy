// HLSL shader version 5.0 (for Direct3D 11/ 12)


struct vsin
{
    float2 position : POSITION;
    float3 color : COLOR;
};


struct vsout
{
    float4 position : SV_Position;
    float3 color : COLOR;
};


// Vertex shader main function
vsout vsmain(vsin input)
{
    vsout output;
    output.position = float4(input.position, 0, 1);
    output.color = input.color;
    return output;
}


// Pixel shader main function
float4 psmain(vsout input) : SV_Target
{
    return float4(input.color, 1);
}