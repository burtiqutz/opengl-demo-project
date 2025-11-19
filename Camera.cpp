#include "Camera.hpp"

namespace gps {

    //  Camera constructor
    //  https://learnopengl.com/Getting-started/Camera
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {

        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(this->cameraTarget- this->cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {

        return glm::lookAt(this->cameraPosition, this->cameraTarget, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {

        if(direction == MOVE_FORWARD) {
            //  Modific pozitia camerei dar si vectorul atasat ei
            this->cameraPosition +=  cameraFrontDirection * speed;
            this->cameraTarget += cameraFrontDirection * speed;
        } else if (direction == MOVE_BACKWARD) {
            this->cameraPosition -= cameraFrontDirection * speed;
            this->cameraTarget -= cameraFrontDirection * speed;
        } else if (direction == MOVE_LEFT) {
            this->cameraPosition -= cameraRightDirection * speed;
            this->cameraTarget -= cameraRightDirection * speed;
        } else if (direction == MOVE_RIGHT) {
            this->cameraPosition += cameraRightDirection * speed;
            this->cameraTarget += cameraRightDirection * speed;
        }
    }
    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(direction);

        cameraTarget = cameraPosition + cameraFrontDirection;
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
}