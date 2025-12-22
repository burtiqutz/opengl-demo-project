#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Model3D.hpp"

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT};

    const float PLAYER_WIDTH = 0.1f;
    const float PLAYER_HEIGHT = 0.5f;
    const float EYE_HEIGHT = 0.3f;

    class Camera {

    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        BoundingBox GetPlayerBox();
        glm::vec3 getPosition() const { return cameraPosition; }
        void setPosition(const glm::vec3& position);
        
    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };
}

#endif /* Camera_hpp */
