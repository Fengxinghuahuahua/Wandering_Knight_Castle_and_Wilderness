#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    flat int tex;
    vec2 TexCoords;
    vec3 vertexColor;
    vec4 FragPosLightSpace;
	mat3 TBN;
} fs_in;

in MT_P {
    float ambient;
    float diffuse;
    float specular;
    float shadow;
} mt_p;

uniform sampler2D texture_diffuse1;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int isTerrain;

uniform int depthTest;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 透视除法，将坐标从裁剪空间转换到NDC空间
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    // 从阴影贴图中采样最接近的深度
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    // 修改偏移量计算
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    //float dr = abs(projCoords.z - closestDepth);
    //int pcf_size = min(4, int(dr*10));
    //pcf_size = max(pcf_size, 2);
    int pcf_size = 2;

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -pcf_size; x <= pcf_size; ++x)
    {
        for(int y = -pcf_size; y <= pcf_size; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= (pcf_size*2+1) * (pcf_size*2+1); 

    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{   
    if (depthTest == 0)
    {         
        vec4 texcolor = (fs_in.tex == 1) ? texture(texture_diffuse1, fs_in.TexCoords) : vec4(fs_in.vertexColor, 1.0f);
        if (texcolor.a < 0.1) discard;
        vec3 color = texcolor.rgb;
		
        vec3 normal = normalize(fs_in.Normal);

		if(isTerrain==1){
		    vec3 tangentNormal = (texture(normalMap, fs_in.TexCoords).rgb * 2.0 - 1.0);
		    // 转换到世界空间法线
		    vec3 worldNormal = normalize(fs_in.TBN * tangentNormal);
			normal = worldNormal;
		}

        vec3 lightColor = vec3(1.0f, 1.0f, 0.95f);
        // Ambient
        vec3 ambient = color * mt_p.ambient;
        // Diffuse
        vec3 lightDir = normalize(lightPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * lightColor * mt_p.diffuse;
        // Specular
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        float spec = 0.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lightColor * mt_p.specular;    
        // Calculate shadow
        float shadow = ShadowCalculation(fs_in.FragPosLightSpace) * mt_p.shadow;                      
        shadow = min(shadow, 0.75); 
        vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

        vec3 fogColor = vec3(0.5f, 0.5f, 0.5f);
        float fogFactor = exp(-0.001 * pow(length((viewPos - fs_in.FragPos).xyz), 2));
        fogFactor = clamp(fogFactor, 0.0, 1.0);  // 限制在0到1之间

        // 最终颜色：物体颜色和雾的颜色按比例混合
        vec3 Color = mix(fogColor, lighting, fogFactor);
        
        FragColor = vec4(Color, 1.0f);
    }
}
