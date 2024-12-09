#version 330 core

// 输入：来自顶点着色器的变量
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

// 输出：最终颜色
out vec4 FragColor;

// uniform变量，光源位置和相机位置
uniform vec3 lightPos;
//uniform vec3 viewPos;
uniform sampler2D texture1; // 高度图纹理
uniform sampler2D normalMap;
uniform sampler2D aoMap;

void main()
{
	vec3 baseColor = texture(texture1, TexCoord).rgb;
	float ao = texture(aoMap, TexCoord).r;
	vec3 ambient =   ao * baseColor;
    // 从法线贴图中读取切线空间法线
    vec3 tangentNormal = (texture(normalMap, TexCoord).rgb * 2.0 - 1.0);

    // 转换到世界空间法线
    vec3 worldNormal = normalize(TBN * tangentNormal);
    vec3 norm = worldNormal;
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = 1.f * diff * texture(texture1, TexCoord).rgb;  
	FragColor = vec4(0.1*ambient+0.9*diffuse,1);
}
