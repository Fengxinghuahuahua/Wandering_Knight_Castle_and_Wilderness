#include "glm/detail/qualifier.hpp"
#include "glm/ext/vector_float3.hpp"
#include <iostream>
#include <vector>
#define  STB_IMAGE_STATIC
#include "camera.h"
#include "depthMap.h"
#include "model_animation.h"
#include "natureLight.h"
#include "skybox.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <camera.h>
#include <shader_m.h>
#include <gui.hpp>
#include <Knight.h>
#include <Grass.h>
#include <Terrain.h>
#include <PhysicsWorld.hpp>

class Application{
	private:
		float _Width = 1920;
		float _Height = 1080;
		int _mouse_mode = 0;
		int _selector;

		glm::vec3 _trans[6];
		Model* _house;
		Model* _tree1;
		Model* _tree2;
		Model* _tree3;
		Model* _tree4;
		Shader* _model_shader;

		Knight* _knight;
		Shader* _knight_shader;
		
		Grass* _grass;
		Shader* _grass_shader;

		Terrain* _terrain;
		Shader* _terrain_shader;

		NatureLight* _nature_light;
		DepthMap * _depth_map;

		//physics world
		btRigidBody* _houseBody, *_knightBody;
		
		Skybox* _skybox;

		GLFWwindow *_Window = nullptr;

		Camera* _camera_3P;
		Camera* _camera_1P;
		Camera* _current_camera;

		PhysicsWorld* _physics_world;

		glm::vec3 _camera_3P_Position;
		float _camera_1P_font = 0.15f; 
		float _camera_1P_height = 0.8f;
		float _CurrentFrameTime = 0.0f;
        float _LastFrameTime = 0.0f;
        float _DeltaTime = 0.0f;
		bool _flagCallMouse = true;
		bool _spacePressed = false;

		std::vector<bool> _Knight_KB;

        bool _FirstMouse = true;
        double _LastMouseX;
        double _LastMouseY;
		GLuint _MinFPS = 0;
        GLuint _MaxFPS = 0;
        GLuint _AvgFPS = 0;

		float _Duration = 0.f;
        float _MinDuration = INFINITY;

        float _SampleDuration = 1.f;
        float _MaxDuration = 0.f;
		bool _ImGuiShowAnalytics = true;
        bool _ImGuiShowHelp = true;
		bool _ImGuiShowWire = true;

        GLuint _NbFrames = 1;

        ImGuiIO* _ImGuiIo = nullptr;

		std::vector<glm::vec3> _randPositions;
        std::vector<GrassTile> _randGrassTiles;
		void initGLAD();
		void initGLFW();
		void initPhysics();
		void initAssets();
		void render();
		void handleInput();
		void handleWireFrameInput();
		void handleCameraInput();
		void update();
		void updateDt();
		void initShaders();
		void initGUI();
		void renderGUI();
		void processInput();
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
		{
		    // make sure the viewport matches the new window dimensions; note that width and
		    // height will be significantly larger than specified on retina displays.
		    glViewport(0, 0, width, height);
		}

		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){

		}
		void SyncPosition_Model_Rigibody(glm::vec3 position, btRigidBody* RigiBody) {
			btTransform transform;
			RigiBody->getMotionState()->getWorldTransform(transform);
			
			transform.setOrigin(btVector3(position.x, position.y, position.z));
			RigiBody->setWorldTransform(transform);
			RigiBody->getMotionState()->setWorldTransform(transform);
		}

		void updatePhysics(float deltaTime);
 	
	public:
		static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
            Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
			
            if (app->_FirstMouse) {
                app->_LastMouseX = xpos;
                app->_LastMouseY = ypos;
                app->_FirstMouse = false;
				return;
            }

            double xoffset = xpos - app->_LastMouseX;
            double yoffset = app->_LastMouseY - ypos; // reversed since y-coordinates range from bottom to top
													  //
			// std::cout<<xoffset<<" "<<yoffset<<std::endl;
    		float sensitivity = 0.1f;
    		xoffset *= sensitivity;
    		yoffset *= sensitivity;

    		app->_camera_1P->Yaw -= xoffset;
    		app->_camera_1P->Pitch += yoffset;
    		if (app->_camera_1P->Pitch > 89.0f)
    		    app->_camera_1P->Pitch = 89.0f;
    		if (app->_camera_1P->Pitch < -89.0f)
    		    app->_camera_1P->Pitch = -89.0f;

    		app->_camera_3P->Yaw -= xoffset * 0.5f;
    		app->_camera_3P->Pitch += yoffset * 0.5f;
    		if (app->_camera_3P->Pitch > 30.0f)
    		    app->_camera_3P->Pitch = 30.0f; 
    		if (app->_camera_3P->Pitch < -89.0f)
    		    app->_camera_3P->Pitch = -89.0f; 

            app->_LastMouseX = xpos;
            app->_LastMouseY = ypos;
        }
		void init();
		void run();
};

