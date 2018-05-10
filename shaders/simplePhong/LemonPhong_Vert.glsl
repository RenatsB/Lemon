#version 430

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 N; // This is the inverse transpose of the mv matrix
//uniform float t;

const float stretch = 1.25;

out float noise;
//out mat4 mMVP;

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The texture coordinate attribute
layout (location=1) in vec2 TexCoord;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

// These attributes are passed onto the shader (should they all be smoothed?)
smooth out vec3 FragmentPosition;
smooth out vec3 FragmentNormal;
smooth out vec2 FragmentTexCoord;
smooth out vec3 RawPosition;
//out vec3 vPosition;

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 );
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
}

float turbulence( vec3 p ) {

  float w = 100.0;
  float t = -.5;

  for (float f = 1.0 ; f <= 5.0 ; f++ ){
    float power = pow( 1.9, f );
    t += abs( snoise( vec3( power * p)) / power );
  }
  return t;
}

void main() {

    vec3 strNormal = normalize(vec3(VertexNormal.x, VertexNormal.y*stretch, VertexNormal.z));
    vec3 strPos = vec3(VertexNormal.x, VertexNormal.y*stretch, VertexNormal.z);

    // get a turbulent 3d noise using the normal, normal to high freq
    noise = 1.0 *  -.10 * turbulence( .5 * strNormal );
    // get a 3d noise using the position, low frequency
    float b = 1.0 * snoise( 0.05 * strPos);
    // compose both noises
    float displacement = - 1. * noise + b;

    if(distance(strPos.xz, vec2(0)) <= 0.6f && strPos.y > 0)
      displacement += (0.6-distance(strPos.xz, vec2(0)))/2.5f;

    if(distance(strPos.xz, vec2(0)) <= 0.1f && strPos.y > 0)
      displacement -= (0.2-distance(strPos.xz, vec2(0)))/2.5f;

    if(distance(strPos.xz, vec2(0)) <= 0.6f && strPos.y < 0)
      displacement += (0.6-distance(strPos.xz, vec2(0)))/2.5f;

    if(distance(strPos.xz, vec2(0)) <= 0.1f && strPos.y < 0)
      displacement -= (0.1-distance(strPos.xz, vec2(0)))/2.5f;

    // Transform the vertex normal by the inverse transpose modelview matrix
    //FragmentNormal = normalize(N * strNormal);
    FragmentNormal = strNormal; //if you want to "freeze" the normals

    // Compute the unprojected vertex position
    FragmentPosition = vec3(MV * vec4(strPos, 1.0) );

    RawPosition = strPos;

    // Copy across the texture coordinates
    FragmentTexCoord = vec2(TexCoord.x, TexCoord.y*stretch);
    //mMVP = MVP;

    vec3 newPosition = strPos + strNormal * displacement;
    // Compute the position of the vertex
    gl_Position = MVP * vec4(newPosition,1.0);
}
