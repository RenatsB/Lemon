#version 430
// Set this property to change the scaling of the texture over the surface (higher means that the texture is repeated more frequently)
const float texCoordScale = 10.0;

const vec3 yellow = vec3(255.0/255.0, 242.0/255.0, 102.0/255.0);
const vec3 grayish = vec3(68.0/255.0, 68.0/255.0, 53.0/255.0);
const vec3 white = vec3(242/255.0, 242/255.0, 225/255.0);
const vec3 green = vec3(0,1,0);

in vec2 vUv;
//in float noise;
const ivec3 off = ivec3(-1,0,1);
const vec2 size = vec2(2.0,0.0);
//in mat4 mMVP;

// The output colour which will be output to the framebuffer
layout (location=0) out vec4 fragColour;

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(2.0, 2.0, 10.0, 1.0),   // position
            vec3(0.85, 0.85, 0.85),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(0.6, 0.6, 0.6)         // Ls
            );

// The material properties of our object
struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo(
            vec3(0.3, 0.3, 0.3),    // Ka
            vec3(0.75, 0.75, 0.75),    // Kd
            vec3(0.5, 0.5, 0.5),    // Ks
            9.0);                  // Shininess

// Attributes passed on from the vertex shader
smooth in vec3 FragmentPosition;
smooth in vec3 FragmentNormal;
smooth in vec2 FragmentTexCoord;
smooth in vec3 RawPosition;
//smooth in vec2 RawTexCoord;

/** From http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
  */
mat4 rotationMatrix(vec3 axis, float angle)
{
    //axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

/**
  * Rotate a vector vec by using the rotation that transforms from src to tgt.
  */
vec3 rotateVector(vec3 src, vec3 tgt, vec3 vec) {
    float angle = acos(dot(src,tgt));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0) {
        return vec;
    }
    vec3 axis = normalize(cross(src,tgt));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
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
    float specnoise = sumOctave(FragmentTexCoord*12.f, 12, 0.105f, 1.0f, 0.4f, 0.8f);
    specnoise = smoothstep(specnoise, 0.0f, 0.77f);
    //specnoise = normalize(specnoise);

    vec3 n = normalize( FragmentNormal );

    vec3 v = normalize(vec3(-FragmentPosition));

    float freq = 0.025f;
    int nscale = 12;
    float specPow = 1.f;
    if(specnoise>0)
      freq = 0.5f;
    //The following code was adapted from:
    //It creates a bump vector using grayscale heightmap (noise value in this case)
    //XYZ of result define the bump vector, while W component defines the height value (not used)
    float f = sumOctave(FragmentTexCoord*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);

    float s11 = f;
    float s01 = sumOctave((FragmentTexCoord+off.xy)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);
    float s21 = sumOctave((FragmentTexCoord+off.zy)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);
    float s10 = sumOctave((FragmentTexCoord+off.yx)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);
    float s12 = sumOctave((FragmentTexCoord+off.yz)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);

    vec3 va = normalize(vec3(size.xy,s21-s01));
    vec3 vb = normalize(vec3(size.yx,s12-s10));
    vec4 bump = vec4( cross(va,vb), s11 );
    //End of adapted fragment


    vec3 tgt = normalize(bump.rgb);

    // The source is just up in the Z-direction
    vec3 src = vec3(0.0, 0.0, 1.0);

    // Perturb the normal according to the target
    vec3 np = rotateVector(src, tgt, n);

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - FragmentPosition );

    // Reflect the light about the surface normal
    vec3 r = (reflect( -s, np )*f*1.45f);
    if(specnoise>0)
      r = mix(reflect( -s, np )/2.f, reflect( -s, np )*f*1.45f, specnoise*1000.f);
    if(distance(RawPosition.xz,vec2(0)) < 0.015f && RawPosition.y < 0)
      r = reflect( -s, np )/2.f;

    // Compute the light from the ambient, diffuse and specular components
    vec3 lightColor = (
            Light.La * Material.Ka +
            Light.Ld * Material.Kd * max( dot(s, np), f/2.f ) +
            Light.Ls * Material.Ks * pow( max( dot(r,v), f ), Material.Shininess*(2.f)-specnoise ));

    vec3 texColor = yellow;

    float dst = distance(RawPosition.xz,vec2(0));

    if(specnoise>0.f)
      texColor = mix(yellow, grayish, specnoise*1000.f);

    if(dst < 0.3f && RawPosition.y > 0)
      texColor = mix(yellow, green, (0.3f - dst));

    if(dst < 0.075f && RawPosition.y > 0)
      texColor = mix(grayish, white, (0.075f - dst)*FragmentTexCoord.y*15.f);

    if(dst < 0.15f && RawPosition.y < 0)
      texColor = mix(yellow, green, vec3((0.15f - dst)));

    if(dst < 0.015f && RawPosition.y < 0)
      texColor = mix(texColor, grayish, (0.015f-dst)*100.f);

    fragColour = vec4(texColor*lightColor,1.0);
}
