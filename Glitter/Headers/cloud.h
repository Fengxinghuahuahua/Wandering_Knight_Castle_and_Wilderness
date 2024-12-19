#pragma once

#include "shader_m.h"

#define PI 3.1415926f

class Cloud {
public:
	float cloudSize;
	float cloudHeight;
	float cloudThickness;
    Camera** currentCamera;

    GLuint framebuffer, textureColorBuffer;

    GLfloat cubeVertices[24] = {
        // λ������         
        -1.0f, -1.0f, -1.0f, // ���½�
        1.0f, -1.0f, -1.0f,  // ���½�
        1.0f,  1.0f, -1.0f,  // ���Ͻ�
        -1.0f,  1.0f, -1.0f, // ���Ͻ�
        -1.0f, -1.0f,  1.0f, // ���½�
        1.0f, -1.0f,  1.0f,  // ���½�
        1.0f,  1.0f,  1.0f,  // ���Ͻ�
        -1.0f,  1.0f,  1.0f,  // ���Ͻ�
    };
    // ������Ⱦ��պе�������Ķ�������
    GLuint cubeIndices[36] = {
        0, 1, 2, 2, 3, 0, // ����
        4, 5, 6, 6, 7, 4, // ����
        3, 2, 6, 6, 7, 3, // ǰ��
        0, 1, 5, 5, 4, 0, // ����
        0, 3, 7, 7, 4, 0, // �����
        1, 2, 6, 6, 5, 1  // �Ҳ���
    };

	Shader rayMarchingShader = Shader("rayMarching.vert", "rayMarching.frag");

	Cloud(float size, float height, float thickness, Camera** cur)
		: cloudSize(size), cloudHeight(height), cloudThickness(thickness), currentCamera(cur) 
    {
        // 创建FBO
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // 创建纹理用于保存颜色信息
        glGenTextures(1, &textureColorBuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 将颜色纹理附加到FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void drawCube(Shader shader)
    {
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        shader.use();

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void draw(glm::vec3 lightPos, float time) 
    {
        glm::mat4 projection = (*currentCamera)->getProjection();
        glm::mat4 view = (*currentCamera)->GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        rayMarchingShader.use();
        rayMarchingShader.setMat4("projection", projection);
        rayMarchingShader.setMat4("view", view);
        model = glm::translate(model, glm::vec3(0.0f, cloudHeight, 0.0f)); 
        model = glm::scale(model, glm::vec3(cloudSize, cloudThickness, cloudSize));	
        rayMarchingShader.setMat4("model", model);
        rayMarchingShader.setVec3("viewPos", (*currentCamera)->Position);
        rayMarchingShader.setFloat("cloudSize", cloudSize);
        rayMarchingShader.setFloat("cloudHeight", cloudHeight);
        rayMarchingShader.setFloat("cloudThickness", cloudThickness);
        rayMarchingShader.setVec3("lightPos", lightPos);
        rayMarchingShader.setFloat("time", time);

        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        rayMarchingShader.setInt("TexBgColor", 10);

        drawCube(rayMarchingShader);
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};