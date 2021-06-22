// Vertex attributes go here as input to the vertex shader.
struct vs_in
{
    float3 position_local : POS;  // Triangle made from 3 XYZ positions.
};

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 position_clip : SV_POSITION; // The position is identified by the SV_POSITION semantic.
};

vs_out vs_main(vs_in input)  // Vertex shader entry point called vs_main(). Entry points and structs can be named whatever we like. 
{
    vs_out output = (vs_out)0; // Zero the memory first.
    output.position_clip = float4(input.position_local, 1.0); // Vertex shader must output a float4 XYZW value to set the homogenous clip space (betwen -1 and 1 in XY axis and 0 and 1 in Z axis.
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET // Pixel shader entry point which must return a float4 RGBA color value. 
{
    return float4(1.0, 1.0, 1.0, 1.0); // Must return an RGBA colour.
}