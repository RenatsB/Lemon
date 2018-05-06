#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// Attributes passed on from the vertex shader
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

const vec2 resolution = vec2(512.f, 512.f);

// The output colour which will be output to the framebuffer
layout (location=0) out vec4 fragColour;

float rand(vec2 c){
  return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
  //float unit = screenWidth/freq;
  float unit = 512.f/freq;
  vec2 ij = floor(p/unit);
  vec2 xy = mod(p,unit)/unit;
  //xy = 3.*xy*xy-2.*xy*xy*xy;
  xy = .5*(1.-cos(3.1415f*xy));
  float a = rand((ij+vec2(0.,0.)));
  float b = rand((ij+vec2(1.,0.)));
  float c = rand((ij+vec2(0.,1.)));
  float d = rand((ij+vec2(1.,1.)));
  float x1 = mix(a, b, xy.x);
  float x2 = mix(c, d, xy.x);
  return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
  float persistance = .5;
  float n = 0.;
  float normK = 0.;
  float f = 4.;
  float amp = 1.;
  int iCount = 0;
  for (int i = 0; i<50; i++){
    n+=amp*noise(p, f);
    f*=2.;
    normK+=amp;
    amp*=persistance;
    if (iCount == res) break;
    iCount++;
  }
  float nf = n/normK;
  return nf*nf*nf*nf;
}

vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)),
           dot(p,vec2(269.5,183.3)),
           dot(p,vec2(419.2,371.9)) );
  return fract(sin(q)*43758.5453);
}

float iqnoise( in vec2 x, float u, float v )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

  float k = 1.0+63.0*pow(1.0-v,4.0);

  float va = 0.0;
  float wt = 0.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = vec2( float(i),float(j) );
    vec3 o = hash3( p + g )*vec3(u,u,1.0);
    vec2 r = g - f + o.xy;
    float d = dot(r,r);
    float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
    va += o.z*ww;
    wt += ww;
    }

    return va/wt;
}

/******************************************************
  * The following simplex noise functions have been taken from WebGL-noise
  * https://github.com/stegu/webgl-noise/blob/master/src/noise2D.glsl
  *>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                + i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
/*********** END REFERENCE ************************/

/***************************************************
  * This function is ported from
  * https://cmaher.github.io/posts/working-with-simplex-noise/
  ****************************************************/
float sumOctave(in vec2 pos,
                in int num_iterations,
                in float persistence,
                in float scale,
                in float low,
                in float high) {
    float maxAmp = 0.0f;
    float amp = 1.0f;
    float freq = scale;
    float noise = 0.0f;
    int i;

    for (i = 0; i < num_iterations; ++i) {
        noise += snoise(pos * freq) * amp;
        maxAmp += amp;
        amp *= persistence;
        freq *= 2.0f;
    }
    noise /= maxAmp;
    noise = noise*(high-low)*0.5f + (high+low)*0.5f;
    return noise;
}

void main() {  

    //vec2 p;// = 0.5 - 0.5*sin( iTime*vec2(1.01,1.71) );
    //p = WSTexCoord;// + vec2(1.0,-1.0)*vec2(0,0)/resolution.xy;

    //p = p*p*(3.0-2.0*p);
    //p = p*p*(3.0-2.0*p);
    //p = p*p*(3.0-2.0*p);

    //float f = iqnoise( 24.0*WSTexCoord, p.x, p.y );
    //f = smoothstep(0.1, f, 0.25f);
    //f = smoothstep(0.9, f, 0.95f);

    //vec3 texColor = vec3(noise(WSTexCoord,80000.f));
    //vec3 texColor = vec3(snoise(WSVertexPosition));
    //fragColour = vec4( texColor.rgb, 1.0 );
    //fragColour = vec4( f, f, f, 1.0 );
    //gl_FragColor = vec4( 1.0 );
    //fragColour = vec4( 1.0,1.0,1.0,1.0 );
    float noise = sumOctave(WSTexCoord, 12, 0.05f, 20.0f, 0.4f, 0.8f);
    noise = smoothstep(noise, 0.0f, 0.7f);
    noise = normalize(noise);

    fragColour = vec4(vec3(noise), 1.0);
}
