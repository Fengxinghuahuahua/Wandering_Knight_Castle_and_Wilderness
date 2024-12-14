#include <glad/glad.h>
#include <glm/glm.hpp>

#include "animator.h"
#include "model_animation.h"
#include "shader_m.h"
#include "camera.h"

#include <iostream>
#include <vector>

//wasd控制
enum KB_KEYS {SPACE = 0, SHIFT, W, A, S, D, KK_END};
enum Character_State {IDLE_STATE = 0, WALK_STATE, RUN_STATE, JUMP_STATE, RUNNINGJUMP_STATE, TURN_BACK_STATE, CS_END};
enum LOCK_TYPE {NONE = 0, JUMP, TURNBACK, LT_END};

const float PERIODS[1][2] = {{800.0f, 1500.0f}};

class Knight {
private:
    const float _walk_speed = 0.03f;
    const float _run_speed = 0.08f;
    const float _jump_speed = 0.05f;

    bool lock_update = false;
    int Lock_Type = LOCK_TYPE::NONE;
    bool turn_back = false;
    //人物状态
    glm::vec3 characterPosition;
    float characterYaw;
    float characterSpeed;
    std::vector<bool> last_state, state;
    float state_update_interval= 0.0f;
    std::vector<bool> last_KB, KB;

    //模型加载
    Animation* currentAnimation = nullptr;
    Animation* lastAnimation = nullptr;
    std::vector<Model*> Models;
    std::vector<Animation*> Animations;
    Animator* animator;

public:
    Knight(std::vector<std::string>& ModelPath, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = 180.0f);
    ~Knight();
    void Render(Shader& shader);
    void Update(float deltatime, bool is_3P, Camera* currentCamera, std::vector<bool> & keys);
    void SetPosition(glm::vec3 position);
    void SetYaw(float yaw);
    glm::vec3 GetPosition();
    float GetYaw();

private:
    bool SetState(int s);
    int StateID();
    void SwitchAnim(float deltatime);
    void UpdateState(float deltatime, bool is_3P, Camera* currentCamera, std::vector<bool> & keys);
    void SetClock(int lock_type);
    void Wakeup(int lock_type);
    void ProcessMovements(float deltatime);
    void ProcessAnimation(float deltatime);
    void ProcessPosition();
};