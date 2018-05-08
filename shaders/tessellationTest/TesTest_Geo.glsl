#version 430

uniform mat3 N;
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
out vec3 gNormal;
in vec2 textcoord[];
out vec2 texcoord;

void main()
{
    vec4 A = gl_in[2].gl_Position - gl_in[0].gl_Position;
    vec4 B = gl_in[1].gl_Position - gl_in[0].gl_Position;
    gNormal = N * normalize(cross(A.xyz, B.xyz));
    texcoord = textcoord[0];
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    gNormal = N * normalize(cross(A.xyz, B.xyz));
    texcoord = textcoord[0];
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    gNormal = N * normalize(cross(A.xyz, B.xyz));
    texcoord = textcoord[0];
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}
