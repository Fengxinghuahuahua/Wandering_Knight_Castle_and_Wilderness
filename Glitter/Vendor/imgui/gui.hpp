#pragma once

#include "imgui.h"
#include <GLFW/glfw3.h>

/**
 * Initiate imgui
 * @param window The glfw's window
 * @return The io
*/
ImGuiIO& initIMGUI(GLFWwindow* window);

/**
 * Quit imgui
*/
void cleanIMGUI();