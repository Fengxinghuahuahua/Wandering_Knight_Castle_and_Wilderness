#include "Application.h"
#include "BulletInverseDynamics/IDMath.hpp"
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
        _knight->Update(_DeltaTime, _current_camera == _camera_3P, _current_camera, _WASD_state);
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
		for(auto& each:_randPositions)
			_grass->render(_grass_shader, each, glfwGetTime());
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
		for(auto& each:_randPositions)
			_grass->render(_grass_shader, each, glfwGetTime());
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

	initGUI();
	initAssets();
	_terrain->set2Zero();
	_randPositions = _terrain->getRandomPositions();
	glm::vec3 position = _knight->GetPosition();
	glm::vec3 new_position = glm::vec3(position.x,_terrain->getHeight(position.x, position.z),position.z);
	_knight->SetPosition(new_position);
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
	_house = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/house/house2.fbx");
	_tree1 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Tree.pmx");
	_tree2 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/06/054.obj");
	_tree3 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/07/053.obj");
	_tree4 = new Model(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/08/051.obj");
	_knight = new Knight(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Idle.dae", std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Walking.dae");
	_grass = new Grass();
	_terrain = new Terrain();
	_nature_light = new NatureLight(glm::vec3(0.0f, 6.0f, 6.0f), glm::vec3(0.95f, 0.90f, 0.8f), &_current_camera,1.0f);

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
	if(glfwGetKey(_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
		_knight->CalSpeed(_speed*_DeltaTime*3);
	}else
		_knight->CalSpeed(_speed*_DeltaTime);
	_knight->SetWalking(false);
    if (glfwGetKey(_Window, GLFW_KEY_Q) == GLFW_PRESS) {
        _flagCallMouse = false;
    } else {
        _LastMouseX = _Width / 2;
        _LastMouseY = _Height / 2;
        //glfwSetCursorPos(_Window, _LastMouseX / 2, _LastMouseY / 2);
        _flagCallMouse = true;
    }
    if (glfwGetKey(_Window, GLFW_KEY_SPACE) == GLFW_PRESS) {
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
    _WASD_state = WASD_State::IDLE;
    if (glfwGetKey(_Window, GLFW_KEY_W) == GLFW_PRESS) {
        _WASD_state = WASD_State::W_Pressed;
    }
    if (glfwGetKey(_Window, GLFW_KEY_S) == GLFW_PRESS) {
        _WASD_state = WASD_State::S_Pressed;
    }
    if (glfwGetKey(_Window, GLFW_KEY_A) == GLFW_PRESS) {
        _WASD_state = WASD_State::A_Pressed;
    }
    if (glfwGetKey(_Window, GLFW_KEY_D) == GLFW_PRESS) {
        _WASD_state = WASD_State::D_Pressed;
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
        cout << "forward * glm::vec3(camera_1P_font, 0.0f, camera_1P_font)" << (forward * glm::vec3(_camera_1P_font, 0.0f, _camera_1P_font)).z << (forward * glm::vec3(_camera_1P_font, 0.0f, _camera_1P_font)).x << endl;
        cout << "forward:" << forward.x << "," << forward.y << "," << forward.z << endl;
        cout << "knight: " << _knight->GetPosition().x << "," << _knight->GetPosition().y << "," << _knight->GetPosition().z << endl;
 //       cout << "Camera_1P Position: " << _camera_1P->Position.x << "," << _camera_1P->Position.y << "," << _camera_1P_Position.z << endl;
        _camera_1P->Front.x = cos(glm::radians(-_camera_1P->Yaw - 90.0f));
        _camera_1P->Front.y = sin(glm::radians(_camera_1P->Pitch));
        _camera_1P->Front.z = sin(glm::radians(-_camera_1P->Yaw - 90.0f));
        _knight->SetYaw(_camera_1P->Yaw + 180.0f);
    } else if (_current_camera == _camera_3P) {
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

