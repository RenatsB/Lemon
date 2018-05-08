#version 430

layout(vertices = 3) out;
in vec3 vPosition[];
out vec3 tcPosition[];
const float TessLevelInner = 4.f;
const float TessLevelOuter = 4.f;
in float noise[];
out float tcNoise[];

#define ID gl_InvocationID

void main()
{
    tcPosition[ID] = vPosition[ID];
    tcNoise[ID] = noise[ID];
    if (ID == 0) {
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter;
        gl_TessLevelOuter[1] = TessLevelOuter;
        gl_TessLevelOuter[2] = TessLevelOuter;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
