#version 330 core
layout (location = 0) in vec3 aPos;

// 纹理坐标是3维的
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
void main()
{
    // 纹理坐标等于位置坐标
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;// z为w=1，透视除法除后还是1，深度为最远
}
