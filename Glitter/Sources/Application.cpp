#include "Application.h"

#include "GLFW/glfw3.h"
#include "glm/ext/vector_float3.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void Application::initGLAD(){
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }
}

void Application::initGLFW(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // 隐藏窗口装饰（标题栏和工具栏）
    // glfw window creation
    _Window = glfwCreateWindow(_Width, _Height, "", NULL, NULL);
    if (_Window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
		return;
    }
    glfwMakeContextCurrent(_Window);
	glfwSetWindowUserPointer(_Window, this);
    glfwSetFramebufferSizeCallback(_Window, framebuffer_size_callback);
    glfwSetCursorPosCallback(_Window, mouseCallback);
    glfwSetScrollCallback(_Window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Application::initPhysics()
{
    _physics_world = new PhysicsWorld();

    //create the bounding box of the house
    glm::mat4 model;
    model = glm::translate(glm::mat4(1.0f), _trans[0]);
    model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
    _houseBody = _physics_world->addModelCollision(_house, RIGIBODY_TYPE::BOX, model);
    glm::vec3 knight_min = glm::vec3(0,0,0), knight_max = glm::vec3(0.6f, 0.8f, 0.4f);
    btVector3 knight_position = btVector3((knight_min.x + knight_max.x) / 2.0f, (knight_min.y + knight_max.y) / 2.0f, (knight_min.z + knight_max.z) / 2.0f);
    _knightBody = _physics_world->createRigidBody(_physics_world->createBoundingBoxCollisionShape(knight_min, knight_max), knight_position, 0.0f, false);
    updatePhysics(0.f);
    //register the collision listener and the handle logic
    _physics_world->registerCollision(_houseBody, _knightBody, [](btRigidBody* body1, btRigidBody* body2, const btVector3& normal, Knight* app){
        btTransform transform2;
        body2->getMotionState()->getWorldTransform(transform2);

        btVector3 position2 = transform2.getOrigin();

        // 获取 knight 的位置直接从 _knightBody (body2)
        btVector3 knightPosition = position2;  // 从body2获取knight的位置

        btVector3 correction = btVector3(0.08,0.08,0.08) * normal;
        knightPosition -= correction;  // 沿法线方向调整 knight 的位置

        // 更新 _knightBody 的位置
        transform2.setOrigin(knightPosition);
        body2->getMotionState()->setWorldTransform(transform2);
        app->SetPosition(glm::vec3(knightPosition.getX(), knightPosition.getY(), knightPosition.getZ()));

        std::cout << "Corrected Knight Position: (" << knightPosition.getX() << ", " << knightPosition.getY() << ", " << knightPosition.getZ() << ")" << std::endl;
    });

}

void Application::updatePhysics(float deltaTime) {
    glm::vec3 knight_currentPosition = _knight->GetPosition();
    glm::vec3 house_currentPosition = _trans[0] + glm::vec3(20.5324f, 0.0f, -20.9432);

    SyncPosition_Model_Rigibody(knight_currentPosition, _knightBody);
    SyncPosition_Model_Rigibody(house_currentPosition, _houseBody);

    _physics_world->stepSimulation(deltaTime, _knight);

    
    btTransform transform;
    _knightBody->getMotionState()->getWorldTransform(transform);
    btVector3 btPosition = transform.getOrigin();
    glm::vec3 newPosition(btPosition.x(), btPosition.y(), btPosition.z());
    _knight->SetPosition(newPosition);
}


void Application::handleInput(){
    if(glfwGetKey(_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(_Window, true);
    }
    handleWireFrameInput();
    handleCameraInput();
}
void Application::handleCameraInput(){
}
void Application::handleWireFrameInput(){
    if (glfwGetKey(_Window, GLFW_KEY_J) == GLFW_PRESS){
		_ImGuiShowWire = ~_ImGuiShowWire;
	}
}
void Application::initShaders(){
	_model_shader = new Shader("Model.vert", "test_light.frag");
	_knight_shader = new Shader("animation.vert", "test_light.frag");
	_grass_shader = new Shader("grass.vert", "test_light.frag");
	_terrain_shader = new Shader("terrain.vert","test_light.frag");

}
void Application::update(){

    updateDt();
    updatePhysics(_DeltaTime);
    
}
void Application::updateDt(){
	_CurrentFrameTime = static_cast<float>(glfwGetTime());
	_DeltaTime = _CurrentFrameTime - _LastFrameTime;
	_LastFrameTime = _CurrentFrameTime;
	// update fps for analytics
	_NbFrames++;
	_Duration += _DeltaTime;
	if(_DeltaTime > _MaxDuration){
	    _MaxDuration = _DeltaTime;
	}
	if(_DeltaTime < _MinDuration){
	    _MinDuration = _DeltaTime;
	}
	// reset
	if(_Duration >= _SampleDuration){
	    _MinFPS = 1.f / _MaxDuration;
	    _MaxFPS = 1.f / _MinDuration;
	    _AvgFPS = _NbFrames / _Duration;
	    _NbFrames = 1;
	    _Duration = 0.f;
	    _MinDuration = INFINITY;
	    _MaxDuration = 0.f;
	}
}
void Application::run(){
	while(!glfwWindowShouldClose(_Window)){
		update();	
		render();
	}

}
void Application::render(){
		processInput();
        Frustum frustum = _current_camera->createFrustum();
        _knight->Update(_DeltaTime, _current_camera == _camera_3P, _current_camera, _Knight_KB);
		glm::vec3 position = _knight->GetPosition();
		glm::vec3 new_position = glm::vec3(position.x,_terrain->getHeight(position.x, position.z),position.z);
		_knight->SetPosition(new_position);
        glm::mat4 lightSpaceMatrix = _nature_light->getLightSpaceMatrix();
		_depth_map->bind();

        _depth_map->use(lightSpaceMatrix, *_knight_shader);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(glm::mat4(1.0f), _knight->GetPosition());
        model = glm::rotate(model, glm::radians(_knight->GetYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        _knight_shader->setMat4("model", model);
        _knight->Render(*_knight_shader);

		_depth_map->use(lightSpaceMatrix, *_grass_shader);

        _grass->instanceRender(_grass_shader, _randGrassTiles, frustum, glfwGetTime());
        _depth_map->use(lightSpaceMatrix, *_model_shader);
        model = glm::translate(glm::mat4(1.0f), _trans[0]);
        model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
        _model_shader->setMat4("model", model);
        _house->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[1]);
        model = glm::scale(model, glm::vec3(0.15, 0.15, 0.15));
        _model_shader->setMat4("model", model);
        _tree1->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[2]);
        model = glm::scale(model, glm::vec3(0.10, 0.10, 0.10));
        _model_shader->setMat4("model", model);
        _tree1->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[3]);
        model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
        _model_shader->setMat4("model", model);
        _tree2->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[4]);
        model = glm::scale(model, glm::vec3(0.16, 0.16, 0.16));
        _model_shader->setMat4("model", model);
        _tree3->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[5]);
        model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
        _model_shader->setMat4("model", model);
        _tree4->Draw(*_model_shader);

        _depth_map->unbind();//

        glViewport(0, 0, _Width, _Height);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ///******** Knight *********/
        _nature_light->use(_depth_map->getDepthMap(), *_knight_shader);
        model = glm::translate(glm::mat4(1.0f), _knight->GetPosition());
        model = glm::rotate(model, glm::radians(_knight->GetYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        _knight_shader->setMat4("model", model);
        _knight->Render(*_knight_shader);

		_nature_light->use(_depth_map->getDepthMap(), *_grass_shader);

        _grass->instanceRender(_grass_shader, _randGrassTiles, frustum, glfwGetTime());
        //// sence
        _nature_light->use(_depth_map->getDepthMap(), *_model_shader);
        model = glm::translate(glm::mat4(1.0f), _trans[0]);
        model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
        _model_shader->setMat4("model", model);
        _house->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[1]);
        model = glm::scale(model, glm::vec3(0.15, 0.15, 0.15));
        _model_shader->setMat4("model", model);
        _tree1->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[2]);
        model = glm::scale(model, glm::vec3(0.10, 0.10, 0.10));
        _model_shader->setMat4("model", model);
        _tree1->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[3]);
        model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
        _model_shader->setMat4("model", model);
        _tree2->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[4]);
        model = glm::scale(model, glm::vec3(0.16, 0.16, 0.16));
        _model_shader->setMat4("model", model);
        _tree3->Draw(*_model_shader);

        model = glm::translate(glm::mat4(1.0f), _trans[5]);
        model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
        _model_shader->setMat4("model", model);
        _tree4->Draw(*_model_shader);
	
		//Terrain
        _nature_light->use(_depth_map->getDepthMap(), *_terrain_shader);
		_terrain->render(_terrain_shader);

        // skybox render
        _skybox->render();

        // cloud
        _cloud->bind();
        _skybox->render();
        _cloud->unbind();
        _cloud->draw(_light_dir, glfwGetTime());

        // water

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------

		renderGUI();
        glfwSwapBuffers(_Window);
        glfwPollEvents();
}
void Application::init(){
	initGLFW();
	initGLAD();
	initShaders();
	for(int i = 0;i<6;i+=1){
		_trans[i] = glm::vec3(0.f);
	}
	_camera_3P_Position = glm::vec3(0.0f, 1.0f, 0.8f);
    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "Failed to enable z-buffer: %d\n", error);
    }
	glm::vec3 camera_3P_Position(0.0f, 1.0f, 0.8f);
	float camera_1P_font = 0.08f, camera_1P_height = 0.8f;
	glm::vec3 camera_1P_Position(camera_1P_font, camera_1P_height, 0.0f);
	float aspect =  (float)(_Width)/_Height;
	_camera_3P = new Camera(
    	camera_3P_Position,
    	glm::vec3(0.0f, 1.0f, 0.0f),
    	0.0f, // yaw
    	0.0f, // pitch
		aspect
	);
	_camera_1P = new Camera(
    	camera_1P_Position, // position
    	glm::vec3(0.0f, 1.0f, 0.0f), // up
    	0.0f, // yaw
    	0.0f, // pitch
		aspect
	);
	_current_camera = _camera_3P;
    _Knight_KB.resize(KB_KEYS::KK_END);

    _light_dir = glm::vec3(0.0f, 6.0f, 6.0f);

	initGUI();
	initAssets();
    
	_terrain->set2Zero();
	//_randPositions = _terrain->getRandomPositions();
    _randGrassTiles = _terrain->GetGrassTiles(16.f,0.9f, 20);
	glm::vec3 position = _knight->GetPosition();
	glm::vec3 new_position = glm::vec3(position.x,_terrain->getHeight(position.x, position.z),position.z);
	_knight->SetPosition(new_position);
    //add to physics world
    initPhysics();
}

void Application::initGUI(){
	_ImGuiIo = &initIMGUI(_Window);
	_ImGuiIo->FontGlobalScale = 1.5f;
}

void Application::renderGUI(){
	ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // analytics
    if(true){
        ImVec2 size{170, 210};
        ImVec2 pos{_Width - size.x, 0};
        ImGui::SetNextWindowPos(pos);
        ImGui::Begin("Analytics", &_ImGuiShowAnalytics);
        ImGui::SetWindowSize(size);
        ImGui::Text("FPS:\n  Avg: %d\n  Min: %d\n  Max: %d", _AvgFPS, _MinFPS, _MaxFPS);
        ImGui::End();
    }

    // helper
    if(_ImGuiShowHelp){
        ImVec2 size{300, 400};
        ImVec2 pos{0, 0};
        ImGui::SetNextWindowPos(pos);
        ImGui::Begin("Help", &_ImGuiShowHelp);
        ImGui::SetWindowSize(size);
        ImGui::Text("Camera:\n" \
                    "  W: Go forward\n" \
                    "  A: Go left\n" \
                    "  S: Go back\n" \
                    "  D: Go right\n" \
                    "  J: Toggle mouse\n" \
                    "  Shift: Go faster\n" \
                    "  Mouse: Move camera\n" \
        );
		const char* items[] = {"House","Tree1","Tree2","Tree3","Tree4","Tree5"}; 
		static int cur_dix = 0;
		ImGui::Combo("select", &cur_dix, items,IM_ARRAYSIZE(items));
		static glm::vec3 tmp_trans = glm::vec3(0.f);
		tmp_trans = _trans[cur_dix];
		ImGui::SliderFloat("x", &tmp_trans.x, -23.0, 23.0);
		ImGui::SliderFloat("y", &tmp_trans.y, -23.0, 23.0);
		ImGui::SliderFloat("z", &tmp_trans.z, -23.0, 23.0);
		_trans[cur_dix] = tmp_trans;
        ImGui::End();
    }

     ImGui::Render();
     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::initAssets(){
    std::cout << "1" << endl;
	_house = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/house/house.fbx");
    std::cout << "2" << endl;
	_tree1 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Tree/Tree.pmx");
    std::cout << "3" << endl;
	_tree2 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/06/054.obj");
	_tree3 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/07/053.obj");
	_tree4 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/08/051.obj");
    std::vector<std::string> knight_model_paths = {std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/knight/Idle.dae", 
                                                   std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/knight/Walking.dae",
                                                   std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/knight/Running.dae",
                                                   std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/knight/Jump.dae",
                                                   std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/knight/RunningJump.dae",
                                                   std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/knight/Running_Turn_180.dae",};
	_knight = new Knight(knight_model_paths, glm::vec3(3.0f, 0.0f, 3.0f));
	_grass = new Grass();
	_terrain = new Terrain();
	_nature_light = new NatureLight(_light_dir, glm::vec3(0.95f, 0.90f, 0.8f), &_current_camera, 1.0f);

	_depth_map = new DepthMap(512,512);
    // skyBox
    vector<string> skybox_textures = {
        std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/right.jpg",
        std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/left.jpg",
        std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/bottom.jpg",
        std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/top.jpg",
        std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/front.jpg",
        std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/back.jpg"
    };
	_skybox = new Skybox(skybox_textures,&_current_camera);

    _cloud = new Cloud(128, 36, 4, &_current_camera);
}

void Application::processInput(){
	static bool _isPressedJ = false;
	if (glfwGetKey(_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(_Window, true);
	if(glfwGetKey(_Window, GLFW_KEY_M) == GLFW_PRESS){
        glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
    // Check if the 'M' key is pressed
    if (glfwGetKey(_Window, GLFW_KEY_J) == GLFW_PRESS) {
        if(!_isPressedJ){
            _isPressedJ = true;
            if(0 == _mouse_mode){
                glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                _mouse_mode = 1;
            } else {
                glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                _mouse_mode = 0;
            }
        }
    }
    if (glfwGetKey(_Window, GLFW_KEY_J) == GLFW_RELEASE) {
        if(_isPressedJ){
            _isPressedJ = false;
        }        
    }

    // free Mouse
    if (glfwGetKey(_Window, GLFW_KEY_Q) == GLFW_PRESS) {
        _flagCallMouse = false;
    } else {
        _LastMouseX = _Width / 2;
        _LastMouseY = _Height / 2;
        //glfwSetCursorPos(_Window, _LastMouseX / 2, _LastMouseY / 2);
        _flagCallMouse = true;
    }

    // camera control
    if (glfwGetKey(_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(_Window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        if (!_spacePressed) {
            if (_current_camera == _camera_3P) {
                _current_camera = _camera_1P;
                _FirstMouse = true; 
            } else {
                _current_camera = _camera_3P;
            }
            _spacePressed = true;
        }
    } else {
        _spacePressed = false;
    }

    //knight control
    std::fill(_Knight_KB.begin(), _Knight_KB.end(), false);
    if(glfwGetKey(_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        _Knight_KB[KB_KEYS::SHIFT] = true;
    }
    if (glfwGetKey(_Window, GLFW_KEY_W) == GLFW_PRESS) {
        _Knight_KB[KB_KEYS::W] = true;
    }
    if (glfwGetKey(_Window, GLFW_KEY_S) == GLFW_PRESS) {
        _Knight_KB[KB_KEYS::S] = true;
    }
    if (glfwGetKey(_Window, GLFW_KEY_A) == GLFW_PRESS) {
        _Knight_KB[KB_KEYS::A] = true;
    }
    if (glfwGetKey(_Window, GLFW_KEY_D) == GLFW_PRESS) {
        _Knight_KB[KB_KEYS::D] = true;
    }
    if (glfwGetKey(_Window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        _Knight_KB[KB_KEYS::SPACE] = true;
    }

	if(_mouse_mode==0)
		glfwSetCursorPos(_Window, float(_Width) / 2, float(_Height) / 2);
	if(_mouse_mode==1)
		return;

    if (_current_camera == _camera_1P) {
        glm::vec3 forward = glm::normalize(glm::vec3(
            sin(glm::radians(_camera_1P->Yaw + 180.0f)),
            0.0f,
            cos(glm::radians(_camera_1P->Yaw + 180.0f))));

        _camera_1P->Position = _knight->GetPosition() + glm::vec3(0.0f, _camera_1P_height, 0.0f)
            + forward * glm::vec3(_camera_1P_font, 0.0f, _camera_1P_font);
        _camera_1P->Front.x = cos(glm::radians(-_camera_1P->Yaw - 90.0f));
        _camera_1P->Front.y = sin(glm::radians(_camera_1P->Pitch));
        _camera_1P->Front.z = sin(glm::radians(-_camera_1P->Yaw - 90.0f));
        _knight->SetYaw(_camera_1P->Yaw + 180.0f);
    } 
    else if (_current_camera == _camera_3P) {
        float radius = glm::length(_camera_3P_Position);

        glm::mat4 horizontalRotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(_camera_3P->Yaw),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 verticalRotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(_camera_3P->Pitch),
            glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 rotationMatrix = horizontalRotation * verticalRotation;

        glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, radius, 1.0f));
        _camera_3P->Position = _knight->GetPosition() + glm::vec3(0.0f, 0.7f, 0.0f) + rotatedOffset;

        glm::vec3 targetPoint = _knight->GetPosition() + glm::vec3(0.0f, 0.7f, 0.0f);
        _camera_3P->Front = glm::normalize(targetPoint - _camera_3P->Position);
    }

	_LastMouseX = float(_Width) / 2;
	_LastMouseY = float(_Height) / 2;
	glm::vec3 camera_3P_Position(0.0f, 1.0f, 0.8f);

    float radius = glm::length(camera_3P_Position);

    glm::mat4 horizontalRotation = glm::rotate(glm::mat4(1.0f),
        glm::radians(_camera_3P->Yaw),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 verticalRotation = glm::rotate(glm::mat4(1.0f),
        glm::radians(_camera_3P->Pitch),
        glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 rotationMatrix = horizontalRotation * verticalRotation;

    glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, radius, 1.0f));
    _camera_3P->Position = _knight->GetPosition() + glm::vec3(0.0f, 0.7f, 0.0f) + rotatedOffset;
	auto p = _camera_3P->Position;
	p = _knight->GetPosition();
    glm::vec3 targetPoint = _knight->GetPosition() + glm::vec3(0.0f, 0.7f, 0.0f);
    _camera_3P->Front = glm::normalize(targetPoint - _camera_3P->Position);
    
}

