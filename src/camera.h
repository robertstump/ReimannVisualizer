#ifndef CAMERA_H
#define CAMERA_H

#include "linmath.h"
#define DEG2RAD(degrees) ((degrees) * M_PI / 180.f)

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW         = -90.f;
const float PITCH       = 0.0f;
const float SPEED       = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM        = 45.f;

typedef struct Camera {
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float lastX;
    float lastY;

}Camera;

void UpdateCameraVectors(Camera* cam) {
    float cosRadYaw = cosf(DEG2RAD(cam->Yaw));
    float sinRadYaw = sinf(DEG2RAD(cam->Yaw));
    float cosRadPitch = cosf(DEG2RAD(cam->Pitch));
    float sinRadPitch = sinf(DEG2RAD(cam->Pitch));
    cam->Front[0] = cosRadYaw * cosRadPitch;
    cam->Front[1] = sinRadPitch;
    cam->Front[2] = sinRadYaw * cosRadPitch;
    vec3_norm(cam->Front, cam->Front);

    vec3_mul_cross(cam->Right, cam->Front, cam->WorldUp);
    vec3_norm(cam->Right, cam->Right);

    vec3_mul_cross(cam->Up, cam->Right, cam->Front);
    vec3_norm(cam->Up, cam->Up);
}

void CameraInit(Camera* cam, vec3 position, vec3 up, float yaw, float pitch) {
    vec3_dup(cam->Position, position);
    vec3_dup(cam->WorldUp, up);
    vec3_dup(cam->Front, (vec3){ 0.f, 0.f, -1.f });
    cam->Yaw = yaw;
    cam->Pitch = pitch;
    cam->MovementSpeed = SPEED;
    cam->MouseSensitivity = SENSITIVITY;
    cam->Zoom = ZOOM;
    cam->lastX = 0;
    cam->lastY = 0;

    UpdateCameraVectors(cam);
}

void CameraInitScalar(Camera* cam, float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) {
    vec3_dup(cam->Position, (vec3) { posX, posY, posZ } );
    vec3_dup(cam->WorldUp, (vec3) { upX, upY, upZ } );
    vec3_dup(cam->Front, (vec3) { 0.f, 0.f, -1.f } );
    cam->Yaw = yaw;
    cam->Pitch = pitch;
    cam->MovementSpeed = SPEED;
    cam->MouseSensitivity = SENSITIVITY;
    cam->Zoom = ZOOM;
}

void ProcessKeyboard(Camera *cam, int direction, float deltaTime) {
    float velocity = cam->MovementSpeed * deltaTime;
    if(direction == FORWARD) {
        vec3 frontSpeed;
        vec3_scale(frontSpeed, cam->Front, velocity);
        vec3_add(cam->Position, cam->Position, frontSpeed);
    }

    if(direction == BACKWARD) {
        vec3 frontSpeed;
        vec3_scale(frontSpeed, cam->Front, velocity);
        vec3_sub(cam->Position, cam->Position, frontSpeed);
    }

    if(direction == LEFT) {
        vec3 frontSpeed;
        vec3_scale(frontSpeed, cam->Right, velocity);
        vec3_sub(cam->Position, cam->Position, frontSpeed);
    }

    if(direction == RIGHT) {
        vec3 frontSpeed;
        vec3_scale(frontSpeed, cam->Right, velocity);
        vec3_add(cam->Position, cam->Position, frontSpeed);
    }
}

void ProcessMouseMovement(Camera *cam, float xOffset, float yOffset, int constrainPitch) {
    xOffset *= cam->MouseSensitivity;
    yOffset *= cam->MouseSensitivity;

    cam->Yaw += xOffset;
    cam->Pitch += yOffset;

    if(constrainPitch) {
        if(cam->Pitch > 89.f) {
            cam->Pitch = 89.f;
        }
        if(cam->Pitch < -89.f) {
            cam->Pitch = -89.f;
        }
    }
    UpdateCameraVectors(cam);
}

void ProcessMouseScroll(Camera *cam, float yOffset) {
    cam->Zoom -= yOffset;
    if(cam->Zoom < 1.f) { cam->Zoom = 1.f; }
    if(cam->Zoom > 45.f) { cam->Zoom = 45.f; }
}

#endif
