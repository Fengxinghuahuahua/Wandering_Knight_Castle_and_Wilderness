#include <glad/glad.h>
#include <glm/glm.hpp>

#include "animator.h"
#include "model_animation.h"
#include "shader_m.h"
#include "camera.h"

#include <iostream>

//wasd控制
enum WASD_State {IDLE = 0, W_Pressed, A_Pressed, S_Pressed, D_Pressed};

class Knight {
private:
    //人物状态
    glm::vec3 characterPosition;
    float characterYaw;
    bool is_walking;
    float characterSpeed;
    //模型加载
    Animation* currentAnimation = nullptr;
    Model* idleModel;
	Model* walkModel;
	Animation* idleAnimation;
	Animation* walkAnimation;
    Animator* animator;

public:
    Knight(std::string idleModelPath, std::string walkModelPath, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = 180.0f);
    ~Knight();
    void Render(Shader& shader);
    void Update(float deltaTime, bool is_3P, Camera* currentCamera, int& WASD_state);

    void SetPosition(glm::vec3 position);
    void SetYaw(float yaw);
    void SetWalking(bool is_walking);
    glm::vec3 GetPosition();
    float GetYaw();
    bool IsWalking();
    void CalSpeed(float speed);
};