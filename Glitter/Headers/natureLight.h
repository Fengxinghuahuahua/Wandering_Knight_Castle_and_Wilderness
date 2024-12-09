#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glm/trigonometric.hpp"
#include "shader_m.h"
#include "camera.h"


class NatureLight {
private:
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    glm::mat4 lightSpaceMatrix;
	Camera** currentCamera;

public:
    NatureLight(glm::vec3 dir, glm::vec3 col, Camera** cur, float inten = 1.0f)
        : direction(dir), color(col), intensity(inten) {
		currentCamera = cur;
        calculateLightSpaceMatrix();
    }

    void calculateLightSpaceMatrix(float near_plane = 1.0f, float far_plane = 20.0f) {
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(direction, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;
    }

    void use(GLint depthMap, Shader& shader) {
        shader.use();
        glm::mat4 projection = (*currentCamera)->getProjection();
        glm::mat4 view = (*currentCamera)->GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // Set light uniforms
        shader.setVec3("lightPos", direction);
        shader.setVec3("viewPos", (*currentCamera)->Position);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setInt("shadowMap", 0);
        shader.setInt("depthTest", 0);
		shader.setInt("isTerrain", 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    glm::mat4 getLightSpaceMatrix() {
        return lightSpaceMatrix;
    }

};
