#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
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

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;
uniform int depthTest;

void main()
{   
    vs_out.tex = 1;

    gl_Position = (depthTest == 1) ? lightSpaceMatrix * model * vec4(aPos, 1.0) : projection * view * model * vec4(aPos, 1.0);

	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;  
	vs_out.Normal =aNormal;
    vs_out.vertexColor = vs_out.Normal;     // 使用法线作为颜色，便于调试
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    mt_p.ambient = 0.3;
    mt_p.diffuse = 1.0;
    mt_p.specular = 0.0;
    mt_p.shadow = 0.7;
}
