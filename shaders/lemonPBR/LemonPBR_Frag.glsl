#version 430 core
const float texCoordScale = 10.0;
//these colours are slightly different from the ones used in Phong version
const vec3 yellow = vec3(1.0, 0.87, 0.09);
const vec3 grayish = vec3(0.1, 0.1, 0.05);
const vec3 white = vec3(0.9, 0.9, 0.82);
const vec3 green = vec3(0,1,0);

const ivec3 off = ivec3(-1,0,1);
const vec2 size = vec2(2.0,0.0);

// This code is based on code from here https://learnopengl.com/#!PBR/Lighting
layout (location =0) out vec4 fragColour;

smooth in vec2 FragmentTexCoord;
smooth in vec3 FragmentPosition;
smooth in vec3 FragmentNormal;
smooth in vec3 RawPosition;
in mat3 TBN;

// material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;
uniform float exposure;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
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

void main()
{
    //calculate the small dark spots
    float specnoise = sumOctave(FragmentTexCoord*12.f, 12, 0.105f, 1.0f, 0.4f, 0.8f);
    specnoise = smoothstep(specnoise, 0.0f, 0.77f);

    vec3 Nreg = normalize(FragmentNormal);
    vec3 V = normalize(camPos - FragmentPosition);
    float specPow = 1.0f;
    float freq = 0.025f;
    int nscale = 12;
    if(specnoise>0)
    {
    freq = 0.5f;
    specPow = 1.f - specnoise;
    }

    //get current fragment noise
    float f = sumOctave(FragmentTexCoord*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);

    float s11 = f;
    //get surrounding fragment noise values
    float s01 = sumOctave((FragmentTexCoord+off.xy)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);
    float s21 = sumOctave((FragmentTexCoord+off.zy)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);
    float s10 = sumOctave((FragmentTexCoord+off.yx)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);
    float s12 = sumOctave((FragmentTexCoord+off.yz)*texCoordScale, nscale, freq, 10.0f, 0.25f, 1.f);

    //create a bump vector based off the acquired noise data
    vec3 va = normalize(vec3(size.xy,s21-s01));
    vec3 vb = normalize(vec3(size.yx,s12-s10));
    vec4 bump = vec4( cross(va,vb), s11 );
    //set normal perturbation target
    vec3 tgt = normalize(bump.rgb);

    // The source is just up in the Z-direction
    vec3 src = vec3(0.0, 0.0, 1.0);

    // Perturb the normal according to the target
    vec3 N = rotateVector(src, tgt, Nreg);

    //N = normalize(bump.xyz);

    vec3 R = reflect(-V, N);

    vec3 texColor = yellow;


    float dst = distance(RawPosition.xz,vec2(0));

    //top green area
    if(dst < 0.7f && RawPosition.y > 0)
      texColor = mix(yellow, green, (0.7f - dst)*FragmentTexCoord.y/1.25f);
    //texColor = normalize(mix(texColor,
                //  normalize(mix(green, white, vec3((0.15f - distance(RawPosition.xz,vec2(0)))*(FragmentTexCoord.y-0.9)))), vec3((0.3f - distance(RawPosition.xz,vec2(0)))*FragmentTexCoord.y)));

    //top whipe circle with dark rim
    if(dst < 0.075f && RawPosition.y > 0)
      texColor = mix(grayish, white, (0.075f - dst)*FragmentTexCoord.y*15.f);

    //bottom green area
    if(dst < 0.35f && RawPosition.y < 0)
      texColor = mix(yellow, green, (0.35f - dst)*(1.f-FragmentTexCoord.y)*2.f);

    //bottom dark tip
    if(dst < 0.015f && RawPosition.y < 0)
      texColor = mix(texColor, grayish, (0.015f-dst)*100.f);

    //change colour at the dark spots
    texColor = mix(texColor, grayish, specnoise*1000.f);
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, texColor, metallic*specPow);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - FragmentPosition);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - FragmentPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness*specPow);
        float G   = GeometrySmith(N, V, L, roughness*specPow);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator*specPow / denominator;
        //specular = bump.rgb;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic*specPow;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * texColor / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * F0 * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    fragColour = vec4(color, 1.0);
}
