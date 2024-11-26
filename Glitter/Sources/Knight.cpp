#include "Knight.h"

    Knight::Knight(std::string idleModelPath, std::string walkModelPath, glm::vec3 position, float yaw){
        idleModel = new Model(idleModelPath);
		walkModel = new Model(walkModelPath);
		idleAnimation = new Animation(idleModelPath, idleModel);
		walkAnimation = new Animation(walkModelPath, walkModel);

        // 初始化人物为待机
        is_walking = false;
        currentAnimation = idleAnimation;
        animator = new Animator(idleAnimation);
    }
    Knight::~Knight(){
        delete idleModel;
        delete walkModel;
        delete idleAnimation;
        delete walkAnimation;
    }
    void Knight::Render(Shader& shader){
        // 设置骨骼动画矩阵
		auto transforms = animator->GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            
		if (is_walking) {
			walkModel->Draw(shader);
            // cout << "walking:" << (walkModel == nullptr? "walkModel is nullptr" : "walkModel is not nullptr") << endl;
		} else {
			idleModel->Draw(shader);
		}
    }

    void Knight::Update(float deltaTime, bool is_3P, Camera* currentCamera, int& WASD_state){
        if(is_3P)
        {
            if(WASD_state != WASD_State::IDLE)
            {
                // 根据摄像机Yaw计算前进方向
                if (WASD_state == WASD_State::W_Pressed) {
                    characterYaw = currentCamera->Yaw + 180.0f;  // 角色朝向与摄像机一致
                    is_walking = true;
                }
                if (WASD_state == WASD_State::S_Pressed) {
                    characterYaw = currentCamera->Yaw;
                    is_walking = true;
                }
                if (WASD_state == WASD_State::A_Pressed) {
                    characterYaw = currentCamera->Yaw - 90.0f;
                    is_walking = true;
                }
                if (WASD_state == WASD_State::D_Pressed) {
                    characterYaw = currentCamera->Yaw + 90.0f;
                    is_walking = true;
                }
                // 计算移动方向
                glm::vec3 forward = glm::normalize(glm::vec3(
                    sin(glm::radians(characterYaw)),
                    0.0f,
                    cos(glm::radians(characterYaw))
                ));
                
                characterPosition += forward * characterSpeed;
            }
        }
        else
        {
            float characterYaw_fake = characterYaw;
            //调试
            // cout<<"camera_1P.Yaw: "<<currentCamera->Yaw<<", camera_1P.Pitch: "<<currentCamera->Pitch 
            //     <<", camera_1P.Position: "<<currentCamera->Position.x<<", "
            //     <<currentCamera->Position.y<<", "<<currentCamera->Position.z<<endl;
            
            if(WASD_state != WASD_State::IDLE)
            {
                if (WASD_state == WASD_State::W_Pressed) {
                    characterYaw_fake = currentCamera->Yaw + 180.0f;  // 角色朝向与摄像机一致
                    is_walking = true;
                }
                if (WASD_state == WASD_State::S_Pressed) {
                    characterYaw_fake = currentCamera->Yaw;
                    is_walking = true;
                }
                if (WASD_state == WASD_State::A_Pressed) {
                    characterYaw_fake = currentCamera->Yaw - 90.0f;
                    is_walking = true;
                }
                if (WASD_state == WASD_State::D_Pressed) {
                    characterYaw_fake = currentCamera->Yaw + 90.0f;
                    is_walking = true;
                }

                // 计算移动方向
                glm::vec3 forward = glm::normalize(glm::vec3(
                    sin(glm::radians(characterYaw_fake)),
                    0.0f,
                    cos(glm::radians(characterYaw_fake))
                ));
                characterPosition += forward * characterSpeed;
            }
        }

        // if (is_walking) {
        // std::cout << "Position: " << characterPosition.x << ", " 
        //          << characterPosition.y << ", " 
        //          << characterPosition.z << std::endl;
        // }
        // 根据移动状态切换动画
		if (is_walking && currentAnimation != walkAnimation) {
			animator->PlayAnimation(walkAnimation);
			currentAnimation = walkAnimation;
            cout << "walking:" << (walkModel == nullptr? "walkModel is nullptr" : "walkModel is not nullptr") << endl;
		}
		else if (!is_walking && currentAnimation != idleAnimation) {
			animator->PlayAnimation(idleAnimation);
			currentAnimation = idleAnimation;
		}
		animator->UpdateAnimation(deltaTime);
    }

    void Knight::SetPosition(glm::vec3 position){
        characterPosition = position;
    }

    void Knight::SetYaw(float yaw){
        characterYaw = yaw;
    }

    void Knight::SetWalking(bool is_walking){
        this->is_walking = is_walking;
    }

    glm::vec3 Knight::GetPosition(){
        return characterPosition;
    }
    float Knight::GetYaw(){
        return characterYaw;
    }

    bool Knight::IsWalking(){
        return is_walking;
    }

    void Knight::CalSpeed(float speed){
        characterSpeed = speed;
    }