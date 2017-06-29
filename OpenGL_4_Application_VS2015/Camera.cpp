//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"

namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
    }

	glm::vec3 Camera::getCameraTarget()
	{
		return cameraTarget;
	}
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
                break;
                
            case MOVE_BACKWARD:
                cameraPosition -= cameraDirection * speed;
                break;
                
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                break;
        }

		/*if (this->cameraPosition.x > 30)
			this->cameraPosition.x = 30;
		if (this->cameraPosition.x < -20)
			this->cameraPosition.x = -20;
		if (this->cameraPosition.y > 5)
			this->cameraPosition.y = 5;
		if (this->cameraPosition.y < 0.5)
			this->cameraPosition.y = 0.5;
		if (this->cameraPosition.z > 30)
			this->cameraPosition.z = 30;
		if (this->cameraPosition.z < -20)
			this->cameraPosition.z = -20; */
    }
    
	const float DEG2RAD = 3.141593f / 180;

	void Camera::rotate(float pitch, float yaw)
	{
		this->cameraDirection.y = sin(pitch * DEG2RAD);
		this->cameraDirection.z = -cos(pitch * DEG2RAD) - cos(yaw * DEG2RAD);
		this->cameraDirection.x = sin(yaw * DEG2RAD);
	}

    
	glm::vec3 Camera::getCameraPosition()
	{
		return this->cameraPosition;
	}

	glm::vec3 Camera::getCameraDirection()
	{
		return this->cameraDirection;
	}
}
