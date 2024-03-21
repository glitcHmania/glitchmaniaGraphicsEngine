struct Output
{
    float3 color : Color;
    float4 pos : SV_Position;
};

cbuffer ConstantBuffer
{
    row_major matrix transformation;
};

Output main(float2 pos : Position, float3 color : Color)
{
    Output op;
    op.color = color;
    op.pos = mul(float4(pos.x, pos.y, 0.0f, 1.0f), transformation);
    return op;
}