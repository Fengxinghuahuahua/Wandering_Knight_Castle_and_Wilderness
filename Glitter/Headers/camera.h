#ifndef CAMERA_H
#define CAMERA_H

#include "glm/ext/matrix_float4x4.hpp"
#include <cstdio>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;


struct Plane{
    glm::vec3 _Normal = {0.f, 1.f, 0.f};
    // distance from origin to the nearest point in the plane
    float _Distance = 0.f;

    Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: _Normal(glm::normalize(norm)),
		_Distance(glm::dot(_Normal, p1))
	{
    }

	float getSignedDistanceToPlane(const glm::vec3& point) const
	{
		return glm::dot(_Normal, point) - _Distance;
	}
};

struct Frustum{
    Plane _TopFace;
    Plane _BottomFace;
    Plane _LeftFace;
    Plane _RightFace;
    Plane _FarFace;
    Plane _NearFace;
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;

    float NearPlane = 0.01f;
    float FarPlane = 20.f;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
	float AspectRatio;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float aspect = 0.f) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
		AspectRatio = aspect;
        updateCameraVectors();
    }

    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float aspect) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
		AspectRatio = aspect;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

	glm::mat4 getProjection(){
		glm::mat4 projection = glm::perspective(glm::radians(Zoom), AspectRatio, 0.1f, 100.0f);
		return projection;
	}

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }
    Frustum createFrustum()const {
        glm::vec3 r = glm::normalize(glm::cross(Front, Up));
        Frustum frustum;
        float fovRadians = glm::radians(Zoom);
        float halfVSide = FarPlane * tanf(fovRadians * .5f);
        float halfHSide = halfVSide * AspectRatio;
        glm::vec3 frontMultFar = FarPlane * Front;
        glm::vec3 nearCenter = Position + Front * NearPlane;
        glm::vec3 farCenter = Position + Front * FarPlane;
        glm::vec3 nearNormal = Front;
        glm::vec3 farNormal = -Front;
        frustum._NearFace = { nearCenter, nearNormal };
        frustum._FarFace = { farCenter, farNormal };
        frustum._RightFace = { Position, glm::cross(Up,frontMultFar+r*halfHSide) };
        frustum._LeftFace = { Position, glm::cross(frontMultFar-r*halfHSide,Up) };
        frustum._TopFace = { Position, glm::cross(r,frontMultFar-Up*halfVSide) };
        frustum._BottomFace = { Position, glm::cross(frontMultFar+Up*halfVSide, r) };
        return frustum;
    }
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif
