/******** OpenGL�ⲿ�� ********/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/******** ����ͷ�ļ� ********/
#include "shader_m.h"
#include "camera.h"
#include "animator.h"
#include "model_animation.h"

//Grass
#include "Grass.h"
#include "GrassGen.h"

//Knight
#include "Knight.h"

//NatureLight
#include "natureLight.h"
//Skybox
#include "skybox.h"
//DepthMap
#include "depthMap.h"

//sence
#include "model.h"

//water


/******** C++��׼�� ********/
#include <iostream>
using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Knight* knight);

/***************************/
/******** ȫ�ֱ��� *********/
/**************************/
// ���ڴ�С
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera��3P--�����˳ƣ�1P--��һ�˳�
glm::vec3 camera_3P_Position(0.0f, 1.0f, 0.8f);
float camera_1P_font = 0.08f, camera_1P_height = 0.8f;
glm::vec3 camera_1P_Position(camera_1P_font, camera_1P_height, 0.0f);
Camera camera_3P(
	camera_3P_Position,
	glm::vec3(0.0f, 1.0f, 0.0f),
	0.0f,                           // yaw
	0.0f                            // pitch
);
Camera camera_1P(
	camera_1P_Position,     		// position
	glm::vec3(0.0f, 1.0f, 0.0f),    // up
	0.0f,                           // yaw
	0.0f                            // pitch
);
Camera* currentCamera = &camera_3P;

// ������
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// �ո����?
bool spacePressed = false;

// wasd����
int WASD_state = WASD_State::IDLE;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//������Ʊ���?
const float speed = 1.0f;

bool flagCallMouse = true;

int main()
{
	// ------------------------ //
	/******** ��ʼ������ *********/
	// ------------------------ //

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);


	// ------------------------ //
	/******** 模型加载 *********/
	// ------------------------ //

	//house
	Model House(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/house/house2.fbx");
	//tree
	Model Tree1(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Tree.pmx");
	Model Tree2(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/06/054.obj" );
	Model Tree3(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/07/053.obj" );
	Model Tree4(std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/08/051.obj" );
	Shader Mshader("Model.vert", "test_light.frag");

	//Knight
	const std::string idleModelPath = std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Idle.dae";
	const std::string walkModelPath = std::string(PROJECT_SOURCE_DIR) + "/Glitter/Model/Walking.dae";
	Knight* knight = new Knight(idleModelPath, walkModelPath);
	Shader KnightShader("animation.vert", "test_light.frag");

	// Grass

	Grass grass;
	Shader grassShader("grass.vert", "test_light.frag");

	//natureLight
	NatureLight natureLight(glm::vec3(0.0f, 6.0f, 6.0f), glm::vec3(0.95f, 0.90f, 0.8f), 1.0f);
	DepthMap depthMap(8192, 8192);

	//skyBox
	vector<string> skybox_textures = {
		std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/right.jpg",
		std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/left.jpg", 
		std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/bottom.jpg", 
		std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/top.jpg", 
		std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/front.jpg", 
		std::string(PROJECT_SOURCE_DIR) + "/Glitter/Textures/skybox/back.jpg"
	};
	Skybox skybox(skybox_textures);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window, knight);

		knight->Update(deltaTime, currentCamera == &camera_3P, currentCamera, WASD_state);

		glm::mat4 model = glm::mat4(1.0f);
		
		// depthMap
		glm::mat4 lightSpaceMatrix = natureLight.getLightSpaceMatrix();
		
		depthMap.bind();
		
		//knight
		depthMap.use(lightSpaceMatrix, KnightShader);
		model = glm::translate(glm::mat4(1.0f), knight->GetPosition());
		model = glm::rotate(model, glm::radians(knight->GetYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		KnightShader.setMat4("model", model);
		knight->Render(KnightShader);

		//grass
		depthMap.use(lightSpaceMatrix, grassShader);
		grass.render(&grassShader, glm::vec3(0.0f, 0.0f, 0.0f), currentFrame);
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++)
				grass.render(&grassShader, glm::vec3(-5 + 0.1f * i, 0.0f, -5 + 0.1f * j), currentFrame);
		}

		//sence
		depthMap.use(lightSpaceMatrix, Mshader);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-23.0f, -0.5f, 5.0f));
		model = glm::scale(model, glm::vec3(0.0005,0.0005,0.0005));
		Mshader.setMat4("model", model);
		House.Draw(Mshader);
		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -3.0f, -16.0f));
		model = glm::scale(model, glm::vec3(0.15, 0.15, 0.15));
		Mshader.setMat4("model", model);
		Tree1.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, -3.0f, -40.0f));
		model = glm::scale(model, glm::vec3(0.10, 0.10, 0.10));
		Mshader.setMat4("model", model);
		Tree1.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -3.0f, -35.0f));
		model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
		Mshader.setMat4("model", model);
		Tree2.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -3.0f, -55.0f));
		model = glm::scale(model, glm::vec3(0.16, 0.16, 0.16));
		Mshader.setMat4("model", model);
		Tree3.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, -3.0f, -16.0f));
		model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
		Mshader.setMat4("model", model);
		Tree4.Draw(Mshader);

		depthMap.unbind();
		
		// render
		// ------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/******** Grass *********/
		natureLight.use(depthMap.getDepthMap(), grassShader);
		grass.render(&grassShader, glm::vec3(0.0f, 0.0f, 0.0f), currentFrame);
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++)
				grass.render(&grassShader, glm::vec3(-5 + 0.1f * i, 0.0f, -5 + 0.1f * j), currentFrame);
		}

		/******** Knight *********/
		natureLight.use(depthMap.getDepthMap(), KnightShader);

		// ����ģ�;���
		model = glm::translate(glm::mat4(1.0f), knight->GetPosition());
		model = glm::rotate(model, glm::radians(knight->GetYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		KnightShader.setMat4("model", model);
		knight->Render(KnightShader);

		//sence
		natureLight.use(depthMap.getDepthMap(), Mshader);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-23.0f, -0.5f, 5.0f));
		model = glm::scale(model, glm::vec3(0.0005,0.0005,0.0005));
		Mshader.setMat4("model", model);
		House.Draw(Mshader);
		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -3.0f, -16.0f));
		model = glm::scale(model, glm::vec3(0.15, 0.15, 0.15));
		Mshader.setMat4("model", model);
		Tree1.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, -3.0f, -40.0f));
		model = glm::scale(model, glm::vec3(0.10, 0.10, 0.10));
		Mshader.setMat4("model", model);
		Tree1.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -3.0f, -35.0f));
		model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
		Mshader.setMat4("model", model);
		Tree2.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -3.0f, -55.0f));
		model = glm::scale(model, glm::vec3(0.16, 0.16, 0.16));
		Mshader.setMat4("model", model);
		Tree3.Draw(Mshader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, -3.0f, -16.0f));
		model = glm::scale(model, glm::vec3(0.07, 0.07, 0.07));
		Mshader.setMat4("model", model);
		Tree4.Draw(Mshader);

		//skybox render
		skybox.render();
		
		//water
		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Knight* knight)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// ��ɫ�ƶ��ٶ�
	knight->CalSpeed(speed * deltaTime);
	knight->SetWalking(false);


	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
		flagCallMouse = false;
	}
	else{
		//重置鼠标偏移，鼠标回到窗口中心
		lastX = SCR_WIDTH / 2;
        lastY = SCR_HEIGHT / 2;
		glfwSetCursorPos(window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
		flagCallMouse = true;
	}

	// �л��ӽ� -- Space
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (!spacePressed) {
			if (currentCamera == &camera_3P) {
				currentCamera = &camera_1P;
				firstMouse = true;  // �������״�?
			}
			else {
				currentCamera = &camera_3P;
			}
			spacePressed = true;
		}
	}
	else {
		spacePressed = false;
	}

	//Get--WASD
	WASD_state = WASD_State::IDLE;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		WASD_state = WASD_State::W_Pressed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		WASD_state = WASD_State::S_Pressed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		WASD_state = WASD_State::A_Pressed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		WASD_state = WASD_State::D_Pressed;
	}

	/******** �����������? *********/
	if (currentCamera == &camera_1P) {
		glm::vec3 forward = glm::normalize(glm::vec3(
			sin(glm::radians(camera_1P.Yaw + 180.0f)),
			0.0f,
			cos(glm::radians(camera_1P.Yaw + 180.0f))
		));

		// ֱ��ʹ�����λ��?
		camera_1P.Position = knight->GetPosition() + glm::vec3(0.0f, camera_1P_height, 0.0f)
			+ forward * glm::vec3(camera_1P_font, 0.0f, camera_1P_font);
		cout<<"forward * glm::vec3(camera_1P_font, 0.0f, camera_1P_font)"<<(forward * glm::vec3(camera_1P_font, 0.0f, camera_1P_font)).z<<(forward * glm::vec3(camera_1P_font, 0.0f, camera_1P_font)).x<<endl;
		cout << "forward:"<<forward.x<<","<<forward.y<<","<<forward.z<<endl;
		cout << "knight: "<<knight->GetPosition().x<<","<<knight->GetPosition().y<<","<<knight->GetPosition().z<<endl;
		cout<<"Camera_1P Position: "<<camera_1P.Position.x<<","<<camera_1P.Position.y<<","<<camera_1P_Position.z<<endl;
		camera_1P.Front.x = cos(glm::radians(-camera_1P.Yaw - 90.0f));
		camera_1P.Front.y = sin(glm::radians(camera_1P.Pitch));
		camera_1P.Front.z = sin(glm::radians(-camera_1P.Yaw - 90.0f));
		knight->SetYaw(camera_1P.Yaw + 180.0f);
	}
	else if (currentCamera == &camera_3P) {
		float radius = glm::length(camera_3P_Position);
		
		glm::mat4 horizontalRotation = glm::rotate(glm::mat4(1.0f),
			glm::radians(camera_3P.Yaw),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 verticalRotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(camera_3P.Pitch),
        	glm::vec3(1.0f, 0.0f, 0.0f));
			
		glm::mat4 rotationMatrix = horizontalRotation * verticalRotation;
		
		glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, radius, 1.0f));
		camera_3P.Position = knight->GetPosition() + glm::vec3(0.0f, 0.7f, 0.0f) + rotatedOffset;
		
		glm::vec3 targetPoint = knight->GetPosition() + glm::vec3(0.0f, 0.7f, 0.0f);
		camera_3P.Front = glm::normalize(targetPoint - camera_3P.Position);
		
	}


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{	
	if(!flagCallMouse) return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		return;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;  // ���������?
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	// ��һ�˳��ӽǵ�������
	camera_1P.Yaw -= xoffset;
	camera_1P.Pitch += yoffset;
	if (camera_1P.Pitch > 89.0f) camera_1P.Pitch = 89.0f;
	if (camera_1P.Pitch < -89.0f) camera_1P.Pitch = -89.0f;

	// �����˳��ӽ�ֻ����ˮƽ��ת
	camera_3P.Yaw -= xoffset * 0.5f;
	camera_3P.Pitch += yoffset * 0.5f;
	if (camera_3P.Pitch > 30.0f) camera_3P.Pitch = 30.0f; // 限制最低角度
    if (camera_3P.Pitch < -89.0f) camera_3P.Pitch = -89.0f; // 限制最高角度
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// �����������¼�
}