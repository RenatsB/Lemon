#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable    // Use for GLSL versions before 420.

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 P;
uniform mat3 N; // This is the inverse transpose of the mv matrix

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The texture coordinate attribute
layout (location=1) in vec2 TexCoord;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

// These attributes are passed onto the shader (should they all be smoothed?)
smooth out vec3 WSVertexPosition;
smooth out vec3 WSVertexNormal;
smooth out vec2 WSTexCoord;
//smooth out vec3 RawPosition;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
    mat4 rotmat = rotationMatrix(vec3(1,0,0), 3.1415f/2.0f);

    // Transform the vertex normal by the inverse transpose modelview matrix
    //WSVertexNormal = normalize(N * VertexNormal);
    WSVertexNormal = VertexNormal;

    // Compute the unprojected vertex position
    //WSVertexPosition = vec3(MV * vec4(VertexPosition, 1.0) );
    WSVertexPosition = VertexPosition;

    // Copy across the texture coordinates
    WSTexCoord = TexCoord;

    // Compute the position of the vertex
    gl_Position = MVP * rotmat * vec4(VertexPosition,1.0);
}
