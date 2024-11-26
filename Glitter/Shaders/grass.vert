#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in float height;
layout (location = 3) in vec3 center;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    flat int tex;
    vec2 TexCoords;
    vec3 vertexColor;
    vec4 FragPosLightSpace;
} vs_out;

out MT_P {
    float ambient;
    float diffuse;
    float specular;
    float shadow;
} mt_p;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

uniform mat4 lightSpaceMatrix;
uniform int depthTest;

const int perlinNoiseGridSize = 10;
const int tileWidth = 10;
const int tileHeight = 10;

float rand(vec2 co){
    return fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 getRandomGradient(int id){
    float x = rand(vec2(1.f*id, 0.f));
    float y = rand(vec2(0.f, 1.f*id));
    return vec2(x, y);
}

vec2 getPerlinNoiseGradient(int u, int v){
    int id = u + v*perlinNoiseGridSize;
    return getRandomGradient(id);
}

vec2 getGridCell(vec2 pos, float cellWidth, float cellHeight){
    int cellX = int(floor(pos.x / cellWidth));
    int cellY = int(floor(pos.y / cellHeight));
    return vec2(cellX, cellY);
}

float blending(float t){
    return 6.f*t*t*t*t*t - 15.f*t*t*t*t + 10.f*t*t*t;
}

float perlin(vec2 pos){
    float cellWidth = (1.f*tileWidth) / perlinNoiseGridSize;
    float cellHeight = (1.f*tileHeight) / perlinNoiseGridSize;

    vec2 ij = getGridCell(pos, cellWidth, cellHeight);

    float x = pos.x;
    float y = pos.y;

    int i = int(ij.x);
    int j = int(ij.y);

    vec2 g00 = getPerlinNoiseGradient(i, j);
    vec2 g10 = getPerlinNoiseGradient(i+1, j);
    vec2 g01 = getPerlinNoiseGradient(i, j+1);
    vec2 g11 = getPerlinNoiseGradient(i+1, j+1);

    float u = x-i*cellWidth;
    float v = y-j*cellHeight;

    float n00 = dot(g00, vec2(u,v));
    float n10 = dot(g10, vec2(u-1.f,v));
    float n01 = dot(g01, vec2(u,v-1.f));
    float n11 = dot(g11, vec2(u-1.f,v-1.f));

    float nx0 = n00*(1-blending(u)) + n10*blending(u);
    float nx1 = n01*(1-blending(u)) + n11*blending(u);
    float nxy = nx0*(1-blending(v)) + nx1*blending(v);

    return nxy * 0.5f + 0.5f; // [0,1]
}

float infinitePerlin(vec2 pos){
    vec2 wrappedP = mod(pos, perlinNoiseGridSize);
    return perlin(wrappedP + vec2(cos(0.50f*time), sin(time)*0.35f));
}


vec3 getAnimatedPos(vec3 basePos, float height, float noise){
    float theta = noise;
    float phase = basePos.y / height;
    float amplitude = .5f;

    float yDelta = basePos.y == 0.f ? 0.f : amplitude*sin(theta + phase);
    float xDelta = basePos.y == 0.f ? 0.f : amplitude*sin(theta + phase);

    return vec3(basePos.x + xDelta, basePos.y + yDelta, basePos.z);
}

void main()
{
    vs_out.tex = 0;
    
    vs_out.vertexColor = color;

	vec3 windDir = vec3(1.f,-1.f,1.f);
	//float factor = 0.05f;
	//float factor = 0.06f;
	vec3 factor = vec3(0.04f,0.009f,0.04f);
	vec4 trans_center = model*vec4(center,1.f);
    vec3 windStrength = vec3(windDir.x*factor.x,windDir.y*factor.y,windDir.z*factor.z);  // 风的强度

    float noise = infinitePerlin(trans_center.xz*20.f);  
	
    vec3 windEffect = aPos.y/height*noise*((sin(pow(time*0.3,1.3f)-4*trans_center.x)))*windStrength; 
	windEffect.y = -abs(windEffect.y)*0.2;
    vec3 displacedPosition = aPos + windEffect;

    gl_Position = (depthTest == 1) ? lightSpaceMatrix * model * vec4(displacedPosition, 1.0f) : projection * view * model * vec4(displacedPosition, 1.0f);
    //gl_Position = projection * view * model * vec4(getAnimatedPos(aPos,height,noise), 1.0f);
    vs_out.FragPos = vec3(model * vec4(displacedPosition, 1.0f));
    vs_out.Normal = vec3(0.0f, 1.0f, 1.0f);
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    mt_p.ambient = 0.8;
    mt_p.diffuse = 1.2;
    mt_p.specular = 1.0;
    mt_p.shadow = 1.0;
}
