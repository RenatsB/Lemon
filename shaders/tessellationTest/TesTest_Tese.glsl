#version 430

layout(triangles, equal_spacing, cw) in;
in float tcNoise[];
out float teNoise;

out vec2 textcoord;

uniform mat4 MVP;
//=============================
//   ^    0
//  ^^^   .
//   |    ...
//   |    .....
//   |    ....... 2
//       1       >
//   V  U ------->>>
//               >
//=============================
void main(){
    vec4 p0 = gl_TessCoord.x * gl_in[0].gl_Position;
    vec4 p1 = gl_TessCoord.y * gl_in[1].gl_Position;
    vec4 p2 = gl_TessCoord.z * gl_in[2].gl_Position;
    vec4 position = normalize(p0 + p1 + p2);
    textcoord = position.xy;
    teNoise = normalize(tcNoise[0]+tcNoise[1]+tcNoise[2]);
    gl_Position = MVP * position;
}
