#include "Knight.h"

    Knight::Knight(std::vector<std::string>& ModelPath, glm::vec3 position, float yaw){
        Models.resize(ModelPath.size());
        Animations.resize(ModelPath.size());
        for (int i = 0; i < ModelPath.size(); i++)
        {
            Models[i] = new Model(ModelPath[i]);
            Animations[i] = new Animation(ModelPath[i], Models[i]);
        }

        // 初始化人物为待机
        characterPosition  = position;
        characterYaw  = yaw;
        state.resize(Character_State::CS_END);
        last_state.resize(Character_State::CS_END);
        last_KB.resize(KB_KEYS::KK_END);
        KB.resize(KB_KEYS::KK_END);
        
        currentAnimation = Animations[this->StateID()];
        animator = new Animator(Animations[this->StateID()]);
    }
    Knight::~Knight(){
        for(Model* model : Models){
            delete model;
        }
        for(Animation* animation : Animations){
            delete animation;
        }
    }
    void Knight::Render(Shader& shader){
        // 设置骨骼动画矩阵
        // std::cout<<"Character Position : "<< characterPosition.x << " " << characterPosition.y << " " << characterPosition.z << std::endl;
		auto transforms = animator->GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            
		Models[this->StateID()]->Draw(shader);
    }

    void Knight::Update(float deltatime, bool is_3P, Camera* currentCamera, std::vector<bool> & keys){
        if(lock_update){
            ProcessMovements(deltatime);
        }
        else{
            UpdateState(deltatime, is_3P, currentCamera, keys);
        }
    }
    void Knight::UpdateState(float deltatime, bool is_3P, Camera* currentCamera, std::vector<bool> & keys){
        // process state
        state_update_interval += deltatime;
        // if(state_update_interval > 0.2f){
            last_state = state;
        // }
        last_KB = KB;
        KB = keys;
        std::fill(state.begin(), state.end(), false);
        if(keys[KB_KEYS::A] || keys[KB_KEYS::D] || keys[KB_KEYS::W] || keys[KB_KEYS::S]){
            state[Character_State::WALK_STATE] = true;
            if(keys[KB_KEYS::SHIFT]){
                state[Character_State::RUN_STATE] = true;
                cout<<"run"<<endl;
            }
        }
        if(keys[KB_KEYS::SPACE]){
            state[Character_State::JUMP_STATE] = true;
            std::cout<<"jump"<<endl;
        }

        //process YAW
        float last_YAW = characterYaw;
        if(keys[KB_KEYS::W]){
            characterYaw = currentCamera->Yaw + 180.0f;
        }
        if(keys[KB_KEYS::S]){
            characterYaw = currentCamera->Yaw;
        }
        if(keys[KB_KEYS::A]){
            characterYaw = currentCamera->Yaw - 90.0f;
        }
        if(keys[KB_KEYS::D]){
            characterYaw = currentCamera->Yaw + 90.0f;
        }

        //process position
        glm::vec3 forward = glm::normalize(glm::vec3(   // 计算移动方向
            sin(glm::radians(characterYaw)),
            0.0f,
            cos(glm::radians(characterYaw))
        ));
        if(state[Character_State::RUN_STATE]){
            characterPosition += forward * _run_speed;
        }
        else if(state[Character_State::WALK_STATE]){
            characterPosition += forward * _walk_speed;
        }
        else{}

        //postprocess YAW
        turn_back = false;
        if(abs(abs(last_YAW - characterYaw) - 180.0f) < 1e-6){
            turn_back = true;
        }
        if(!is_3P){ //always keep characterYaw unchanged in 1P mode
            characterYaw = last_YAW;
        }

        //TODO: 处理跳跃

        // 根据移动状态切换动画
        SwitchAnim(deltatime);
    }

    void Knight::SetPosition(glm::vec3 position){
        characterPosition = position;
    }

    void Knight::SetYaw(float yaw){
        characterYaw = yaw;
    }
    glm::vec3 Knight::GetPosition(){
        return characterPosition;
    }
    float Knight::GetYaw(){
        return characterYaw;
    }

    bool Knight::SetState(int s){
        for (int State = Character_State::IDLE_STATE; State < Character_State::JUMP_STATE; State++){
            if (s == State) {
                state[s] = true;
                return true;
            }
        }
        return false;
    }

    int Knight::StateID(){
        //优先级: 跑跳 >跳跃 > 跑步 > 走路 > 待机
        if(turn_back && state[Character_State::RUN_STATE] && last_state[Character_State::RUN_STATE]){
            return Character_State::TURN_BACK_STATE;
        }
        if (state[Character_State::JUMP_STATE] && state[Character_State::RUN_STATE]){
            return Character_State::RUNNINGJUMP_STATE;
        }
        if(state[Character_State::JUMP_STATE]){
            return Character_State::JUMP_STATE;
        }
        if(state[Character_State::RUN_STATE]){
            return Character_State::RUN_STATE;
        }
        if(state[Character_State::WALK_STATE]){
            return Character_State::WALK_STATE;
        }
        return Character_State::IDLE_STATE;
    }

    void Knight::SwitchAnim(float deltatime)
    {
        lastAnimation = currentAnimation;
        currentAnimation = Animations[this->StateID()];
        if(lastAnimation != currentAnimation){
            animator->PlayAnimation(currentAnimation);
            animator->UpdateAnimation(deltatime);
        }
        else{
		    animator->UpdateAnimation(deltatime);
        }
        if(StateID() == Character_State::RUNNINGJUMP_STATE){
            SetClock(LOCK_TYPE::JUMP);
        }
        if(StateID() == Character_State::JUMP_STATE){
            SetClock(LOCK_TYPE::JUMP);
        }
        if(StateID() == Character_State::TURN_BACK_STATE){
            SetClock(LOCK_TYPE::TURNBACK);
            characterYaw += 180.0f;
        }
    }

    void Knight::SetClock(int lock_type)
    {
        Lock_Type = lock_type;
        lock_update = true;
    }

    void Knight::Wakeup(int lock_type)
    {
        Lock_Type = LOCK_TYPE::NONE;
        lock_update = false;
        animator->ResetAnimation();
        if(turn_back){
            turn_back = false;
            characterYaw -= 180.0f;
        }
    }

    void Knight::ProcessMovements(float deltatime)
    {
        ProcessAnimation(deltatime);
        ProcessPosition();
    }

    void Knight::ProcessAnimation(float deltatime)
    {
        animator->UpdateAnimation(deltatime);
        if(animator->GetIter()){
            Wakeup(Lock_Type);
        }
        // std::cout << "DURATION:" << animator->GetDuration() << std::endl;
    }

    void Knight::ProcessPosition()
    {
        if(Lock_Type == LOCK_TYPE::JUMP){
            glm::vec3 forward = glm::normalize(glm::vec3(   // 计算移动方向
            sin(glm::radians(characterYaw)),
            0.0f,
            cos(glm::radians(characterYaw))
            ));
            if(state[Character_State::RUN_STATE]){
                characterPosition += forward * _run_speed;
            }
            else if(state[Character_State::WALK_STATE]){
                if(animator->GetAniCurrentTime() < PERIODS[0][0] || animator->GetAniCurrentTime() > PERIODS[0][1]){
                    return;
                }
                characterPosition += forward * _walk_speed;
            }
            else{}
        }
    }