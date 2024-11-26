#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 lightSpaceMatrix;
uniform int depthTest;

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

void main()
{
    vs_out.tex = 1;

    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);

    // 计算带有骨骼权重的顶点位置和法线
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (boneIds[i] == -1) 
            continue;
        if (boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(pos, 1.0f);
            break;
        }

        // 变换顶点位置
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0f);
        totalPosition += localPosition * weights[i];

        // 使用逆转置矩阵来变换法线
        mat3 normalMatrix = mat3(transpose(inverse(finalBonesMatrices[boneIds[i]])));
        totalNormal += normalize(normalMatrix * norm) * weights[i];
    }

    // 计算最终的顶点位置
    gl_Position = (depthTest == 1) ? lightSpaceMatrix * model * totalPosition : projection * view * model * totalPosition;

    // 输出到片段着色器
    vs_out.FragPos = vec3(model * totalPosition);
    vs_out.TexCoords = texCoords;
    vs_out.Normal = normalize(mat3(transpose(inverse(model))) * totalNormal);  // 确保法线归一化
    vs_out.vertexColor = vs_out.Normal;     // 使用法线作为颜色，便于调试
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    mt_p.ambient = 1.0;
    mt_p.diffuse = 1.5;
    mt_p.specular = 2.5;
    mt_p.shadow = 1.0;
}
