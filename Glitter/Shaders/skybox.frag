#version 330 core
out vec4 FragColor;

// 纹理坐标是3维的
in vec3 TexCoords;// 纹理坐标

// 天空盒纹理采样
uniform samplerCube skybox;

void main(){ 
    vec3 uTexCoords = vec3(TexCoords.x, -TexCoords.y, TexCoords.z);
    FragColor = texture(skybox, uTexCoords);
}