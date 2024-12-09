#version 330 core

layout(location = 0) in vec3 aPos;         // 顶点位置
layout(location = 1) in vec3 aNormal;      // 顶点法线
layout(location = 2) in vec2 aTexCoord;    // 顶点纹理坐标
layout(location = 3) in vec3 tangent;      // 顶点切线

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    flat int tex;
    vec2 TexCoords;
    vec3 vertexColor;
    vec4 FragPosLightSpace;
	mat3 TBN;
} vs_out;

out MT_P {
    float ambient;
    float diffuse;
    float specular;
    float shadow;
} mt_p;


uniform mat4 lightSpaceMatrix;
uniform int depthTest;
uniform int isTerrain;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{

    // 世界空间的法线和切线
    vec3 transformedNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 T = normalize(mat3(model) * tangent);
    vec3 N = normalize(transformedNormal);
    vec3 B = normalize(cross(N, T)); // 副切线
	mat3 TBN;      // TBN 矩阵
    TBN = mat3(T, B, N);

	gl_Position = (depthTest==1)?lightSpaceMatrix*model*vec4(aPos,1.f):projection*view*model*vec4(aPos,1.f);

	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.Normal = N;
	vs_out.FragPosLightSpace = lightSpaceMatrix*vec4(vs_out.FragPos,1.f);
	vs_out.TexCoords = aTexCoord;
	vs_out.tex = 1;
	vs_out.TBN = TBN;

    mt_p.ambient = 0.6;
    mt_p.diffuse = 0.8;
    mt_p.specular = 0.2;
    mt_p.shadow = 1.0;
}


