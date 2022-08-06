#version 460 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform int Button;
uniform sampler2D depthMap;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissionMap;
uniform sampler2D planeMap;
uniform sampler2D texture_diffuse1;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 cameraPos;

#define NUM_SAMPLES 100
#define NUM_RINGS 10
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define LIGHT_SIZE_UV 2

#define LSIZE 10.0
#define LWIDTH (LSIZE/240.0)
#define BLOKER_SIZE (LWIDTH/2.0)
#define MAX_PENUMBRA 0.5	

#define EPS 1e-3
#define PI 3.141592654
#define PI2 6.28318531

float rand_1to1(float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}
float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples(const in vec2 randomSeed) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = (rand_2to1( randomSeed ) * PI2);
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

float Bias(){
  vec3 lightDir = normalize(lightPositions[0] - fs_in.FragPos);
  vec3 normal = normalize(fs_in.Normal);
  float bias = max(0.1* pow((1.0 - dot(normal, lightDir)), 10.0), 0.01);
  return bias;
}

float searchWidth(float textureStrike)
{
  float textureSIze = 1000.0;

  return textureStrike / textureSIze;
}
float findBlocker(sampler2D shadowMap, vec2 uv, float zReceiver){
  
  float blockerSum = 0.0;
  float numBlockers = 0.0005;

  float bias = Bias();
  poissonDiskSamples(uv);
  float searchWidth = searchWidth(1);

  for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; i++){
    float shadowMapDepth = unpack(texture2D(shadowMap, uv + poissonDisk[i] * searchWidth));
    if(shadowMapDepth < zReceiver){
      blockerSum += shadowMapDepth;
      numBlockers++;
    }
  }
  return blockerSum / numBlockers;
}

float PCF(sampler2D shadowMap, vec4 coords) {
  float currentDepth = coords.z;
  float shadow = 0.0;
  float bias = Bias();
  float searchWidth = searchWidth(1);

  poissonDiskSamples(coords.xy);
  //shadow map 的大小，越大滤波的范围越小;
  for (int i = 0; i < NUM_SAMPLES; i++){
  vec4 closestDepthVec = texture2D(shadowMap, poissonDisk[i] * searchWidth + coords.xy);
  float lightDepth = unpack(closestDepthVec);
     if(lightDepth > currentDepth - bias){
       shadow += 1.0 / float(NUM_SAMPLES);
     }
   }
   return shadow;
}

float PCSS(sampler2D shadowMap, vec4 coords){

  // STEP 1: avgblocker depth
  float avgblocker = findBlocker(shadowMap, coords.xy, coords.z);
  // STEP 2: penumbra size
  float penumbraSize = (coords.z - avgblocker) / coords.z;
  
  // STEP 3: filtering
  float currentDepth = coords.z;
  float shadow = 0.0;
  float bias = Bias();
  float offset = searchWidth(1 + 10 * penumbraSize);

  poissonDiskSamples(coords.xy);
 
  for (int i = 0; i < NUM_SAMPLES; i++){
  vec4 closestDepthVec = texture2D(shadowMap, poissonDisk[i] * offset + coords.xy);
  float lightDepth = unpack(closestDepthVec);
     if(lightDepth > currentDepth - bias){
       shadow += 1.0 / float(NUM_SAMPLES);
     }
   }
   return shadow;
}

float ShadowCalculation(sampler2D shadowMap, vec4 coords)
{
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, coords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = coords.z;
    // Calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPositions[0] - fs_in.FragPos);
    float bias = Bias();
    //Check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 0.0	: 1.0;
   
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(coords.z > 1.0)
        shadow = 1.0;  
    return shadow;
}


mat3 getTBNmat()
{
	vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    return mat3(T, B, N);
}
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, fs_in.TexCoords).xyz * 2.0 - 1.0;

    mat3 TBN = getTBNmat();

    return normalize(TBN * tangentNormal);
}
// 菲涅尔项
vec3 fresnelSchlick(float cosTheta, vec3 F0){
	return F0 + (1.0 - F0) * pow(clamp(1 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 
// 微表面法线分布
float DistributionGGX(vec3 N, vec3 H, float roughness){
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0);
	float NdotH2 = NdotH * NdotH;

	float denom = (NdotH2 * (a2 - 1) + 1);
	denom = PI * denom * denom;

	return a2 / denom;
}
// 几何屏蔽项
float GeometrySchlickGGX(float NdotV, float roughness){
	float r = (roughness + 1);
	float k = (r * r)/8;

	float nom = NdotV;
	float denom = NdotV * (1 - k) + k;

	return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = max(dot(N, V), 0);
	float NdotL = max(dot(N, L), 0);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}


void main(){
	
	vec3 N = getNormalFromMap();
	vec2 texCoords = fs_in.TexCoords;
	vec3 V = normalize(cameraPos - fs_in.FragPos);
	vec3 R = reflect(-V, N);

	// texture
	vec3 albedo     = pow(texture(texture_diffuse1, texCoords).rgb, vec3(2.2));
    float metallic  = texture(metallicMap, texCoords).r;
    float roughness = texture(roughnessMap, texCoords).r;
    float ao        = texture(aoMap, texCoords).r;


	vec3 emission = pow(texture(emissionMap, texCoords).rgb, vec3(2.2));
	

	vec3 F0 = vec3(0.56,0.57,0.58); 
    F0 = mix(F0, albedo, metallic);
	F0 = albedo;
	// 渲染方程
	vec3 Lo = vec3(0.0);
	for(int i = 0; i < 4; i++){
		// calculate per-light radiance
		vec3 L = normalize(lightPositions[i] - fs_in.FragPos);
		vec3 H = normalize(L + V);
		float cosTheta = max(dot(H, V), 0);

		// 点光源
		float distance = length(lightPositions[i] - fs_in.FragPos);
		float attenuation = 1 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(cosTheta, F0);

		vec3 nominator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
		vec3 specular     = nominator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// ambient lighting (we now use IBL as the ambient term)
	vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
	//ambient = vec3(0.03) * albedo;

	vec3 L1 = vec3(0,0,0);
	L1 = emission * 50.0;

	// calculate shadow
	vec3 shadowCoord = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
	shadowCoord.xyz = shadowCoord * 0.5 + 0.5;
	
	float visibility = 1.0f;
//	switch(Button){
//		case 0:
//			visibility = ShadowCalculation(depthMap, vec4(shadowCoord, 1.0));
//			break;
//		case 1:
//			visibility = PCF(depthMap, vec4(shadowCoord, 1.0));
//			break;
//		case 2:
//			visibility = PCSS(depthMap, vec4(shadowCoord, 1.0));
//			break;
//	}


	// 归一化 + gamma校正
	vec3 color = ambient + Lo * visibility;
	color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

	FragColor = vec4(color, 1.0);
	//FragColor = vec4(diffuse, 1.0);
}