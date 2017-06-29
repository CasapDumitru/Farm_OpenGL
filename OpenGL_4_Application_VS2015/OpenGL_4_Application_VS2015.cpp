//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;



glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(glm::vec3(0.0f, 1.0f, 2.5f), glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat cameraSpeed = 0.2f;

bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle;

gps::Model3D myModel;
gps::Model3D ground;
gps::Model3D house;
gps::Model3D groundModel;
gps::Model3D streetLamp;
gps::Model3D car;
gps::Model3D man;
gps::Model3D soil;
gps::Model3D tractor;
gps::Model3D dog;
gps::Model3D water;
gps::Model3D boat;
gps::Model3D moskvitch;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;




gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;




struct Box {
	glm::vec4 min;
	glm::vec4 max;
};


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double mouseCoordinateX;
double mouseCoordinateY;
double mouseSensitivity = 10;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	mouseCoordinateX = xpos - (glWindowWidth / 2);
	mouseCoordinateY = ypos - (glWindowHeight / 2);
	float pitch = -mouseCoordinateY / mouseSensitivity;
	float yaw = mouseCoordinateX / mouseSensitivity;
	myCamera.rotate(pitch, yaw);
}

glm::vec3 colorFelinar = glm::vec3(0.0f, 0.0f, 0.0f);


float fogDensity = 0.0f;
float luminosity = 0.5f;
bool manInCar = false;
float manX = -2.0;
float manZ = 7.5;
float manAngle = 0.0;
float carX = 3.0;
float carZ = 8.0;
float carAngle = 0.0;
bool wireframe = false;



void printVec(glm::vec4 vec) {
	printf("\n%f  %f  %f", vec.x, vec.y, vec.z);
}


bool checkCollision(Box box1, Box box2)
{

	//Check if Box1's max is greater than Box2's min and Box1's min is less than Box2's max
	return( box1.max.x > box2.min.x &&
		    box1.min.x < box2.max.x &&
			box1.max.y > box2.min.y &&
			box1.min.y < box2.max.y &&
			box1.max.z > box2.min.z &&
			box1.min.z < box2.max.z);

	//If not, it will return false

}



//BoundingBox

Box boxWater;
Box boxMan;
Box boxCar;
Box boxHouse;
Box boxLamp;
Box boxDog;
Box boxTractor;



void moveForwardMan() {

	printVec(boxWater.min);
	printVec(boxWater.max);
	printVec(boxMan.min);
	printVec(boxMan.max);

	if (manInCar) {

		Box car1 = boxCar;

		float X = 0.05 * sin(carAngle);
		float Z = 0.05 * cos(carAngle);

		car1.min = car1.min + glm::vec4(X, 0.0f, Z, 0.0f);
		car1.max = car1.max + glm::vec4(X, 0.0f, Z, 0.0f);

		if (checkCollision(car1, boxWater) | checkCollision(car1, boxDog) | checkCollision(car1, boxLamp) |
			checkCollision(car1, boxHouse) | checkCollision(car1, boxTractor)) {
			printf("%s", "WARNING!!");
		}
		else {
			carX += 0.05 * sin(carAngle);
			carZ += 0.05 * cos(carAngle);
		}		
	}
	else {

		Box man1 = boxMan;

        float X =  0.05 * sin(manAngle);
		float Z =  0.05 * cos(manAngle);

		man1.min = man1.min + glm::vec4(X, 0.0f, Z, 0.0f);
		man1.max = man1.max + glm::vec4(X, 0.0f, Z, 0.0f);

		if (checkCollision(man1,boxWater) | checkCollision(man1, boxCar) | checkCollision(man1, boxDog) | checkCollision(man1, boxLamp) |
			 checkCollision(man1, boxHouse) | checkCollision(man1, boxTractor)) {
			printf("%s", "WARNING!!");		
		}
		else {
			manX += 0.05 * sin(manAngle);
			manZ += 0.05 * cos(manAngle);
		}		
	}
}



float tractorX = 0;
float tractorZ = 0;
bool forward = true;
void tractorMove() {

	if (forward)
	{
		tractorX += 0.05 * sin(0);
		tractorZ += 0.05 * cos(0);
		if (tractorZ > 10) {

			tractorX += 0.05 * sin(180);
			tractorZ += 0.05 * cos(180);
			forward = false;
		}

	}
	else {
		tractorX -= 0.05 * sin(0);
		tractorZ -= 0.05 * cos(0);
		if (tractorZ < 0) {

			tractorX += 0.05 * sin(180);
			tractorZ += 0.05 * cos(180);
			forward = true;
		}
	}




}




void processMovement()
{
	if (glfwGetKey(glWindow, GLFW_KEY_UP)) {
		moveForwardMan();
	}

	if (glfwGetKey(glWindow, GLFW_KEY_DOWN)) {
		if (manInCar) {

			Box car1 = boxCar;

			float X = 0.05 * sin(carAngle);
			float Z = 0.05 * cos(carAngle);
			
			car1.min = car1.min + glm::vec4(-X, 0.0f, -Z, 0.0f);
			car1.max = car1.max + glm::vec4(-X, 0.0f, -Z, 0.0f);

			if (checkCollision(car1, boxWater) | checkCollision(car1, boxDog) | checkCollision(car1, boxLamp) |
				checkCollision(car1, boxHouse) | checkCollision(car1, boxTractor)) {
				printf("%s", "WARNING!!");
			}
			else {
				carX -= 0.05 * sin(carAngle);
				carZ -= 0.05 * cos(carAngle);
			}	
		}
	}

	if (glfwGetKey(glWindow, GLFW_KEY_LEFT)) {
		if (manInCar) {
			carAngle += 0.03;
		}
		else {
			manAngle += 0.03;
		}

	}

	if (glfwGetKey(glWindow, GLFW_KEY_RIGHT)) {
		if (manInCar) {
			carAngle -= 0.03;
		}
		else {
			manAngle -= 0.03;
		}
	}

	if (pressedKeys[GLFW_KEY_U]) {
		float dis = sqrt((manX - carX)*(manX - carX) + (manZ - carZ)*(manZ - carZ));
		if (dis <= 2)
			manInCar = true;
	}
	if (pressedKeys[GLFW_KEY_J]) {
		if (manInCar) {
			manInCar = false;
			manX = carX + 2;
			manZ = carZ + 2;
		}

	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 1.0f;
		if (angle > 360.0f)
			angle = 0;
		lightAngle += 1.0f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 1.0f;
		if (angle < 0.0f)
			angle = 360.0f;
		lightAngle -= 1.0f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);

	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_T]) {
		if (luminosity < 1.0f)
			luminosity += 0.01;
	}
	if (pressedKeys[GLFW_KEY_G]) {
		if (luminosity > 0.0f)
			luminosity -= 0.01;
	}
	if (pressedKeys[GLFW_KEY_Y]) {
		wireframe = true;
	}
	if (pressedKeys[GLFW_KEY_H]) {
		wireframe = false;
	}
	if (pressedKeys[GLFW_KEY_V]) {
		colorFelinar = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_B]) {
		colorFelinar = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_N]) {
		colorFelinar = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_M]) {
		colorFelinar = glm::vec3(1.0f, 1.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_I]) {
		fogDensity += 0.0005;
	}
	if (pressedKeys[GLFW_KEY_K]) {
		fogDensity -= 0.0005;
		if (fogDensity < 0) {
			fogDensity = 0;
		}
	}
	/*if (pressedKeys[GLFW_KEY_Z]) {
		lampColor = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_X]) {
		lampColor = glm::vec3(1.0f, 1.0f, 0.0f);
	}*/




	if (pressedKeys[GLFW_KEY_O]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}


}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
    //glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}



void initModels()
{
	myModel = gps::Model3D("objects/nanosuit/nanosuit.obj", "objects/nanosuit/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	house = gps::Model3D("objects/oldhouse/oldhouse.obj", "objects/oldhouse/");
	streetLamp = gps::Model3D("objects/StreetLamp/StreetLamp.obj", "objects/StreetLamp/");
	car = gps::Model3D("objects/Wagon/StationWagon.obj", "objects/Wagon/");
	man = gps::Model3D("objects/male/male.obj", "objects/male/");
	soil = gps::Model3D("objects/soil/ground.obj", "objects/soil/");
	tractor = gps::Model3D("objects/tractor/tractor.obj", "objects/tractor/");
	dog = gps::Model3D("objects/The_dog/the_dog.obj", "objects/the_dog/");
	water = gps::Model3D("objects/water/ground.obj", "objects/water/");
	boat = gps::Model3D("objects/boat/boat_fishing_01.obj", "objects/boat/");
	moskvitch = gps::Model3D("objects/car_scrap/car_scrap.obj", "objects/car_scrap/");


}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

glm::vec4 felinarPosition = glm::vec4(5.2f, 1.5f, 5.7f,1.0f);


void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	myCustomShader.useShaderProgram();

	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.position"), felinarPosition.x, felinarPosition.y, felinarPosition.z);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.linear"), 0.22);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.quadratic"), 0.20);
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.lightColor"), colorFelinar.x,colorFelinar.y,colorFelinar.z);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.ambientStrength"), 0.7f);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.specularStrength"), 0.5f);

	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "viewPos"), myCamera.getCameraPosition().x, myCamera.getCameraPosition().y, myCamera.getCameraPosition().z);
	
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "luminosity"), luminosity);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
	
	skyboxShader.useShaderProgram();
	glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "luminosity"), luminosity);
	glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "fogDensity"), fogDensity);
	
}

glm::mat4 model1;
glm::mat4 model2;

void renderScene()
{	
	if(wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();	

	myCustomShader.useShaderProgram();
	
	
	glm::vec4 fPos = model * felinarPosition;
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.position"), fPos.x, fPos.y, fPos.z);

	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.lightColor"), colorFelinar.x, colorFelinar.y, colorFelinar.z);
	
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "luminosity"), luminosity);

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
	
	skyboxShader.useShaderProgram();
	glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "luminosity"), luminosity);
	glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "fogDensity"), fogDensity);
	
	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();
	

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
		
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	//**********SHADOWS!!!!
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));

	model1 = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	house.Draw(depthMapShader);
	
	model1 = glm::translate(model, glm::vec3(6, 0, 6));
	model1 = glm::scale(model1, glm::vec3(0.3, 0.3, 0.3));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	streetLamp.Draw(depthMapShader);
	
	model1 = glm::translate(model, glm::vec3(6, 0, 7));
	model1 = glm::scale(model1, glm::vec3(0.7, 0.7, 0.7));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	dog.Draw(depthMapShader);
	
	model1 = glm::translate(model, glm::vec3(carX, 0, carZ));
	model1 = glm::scale(model1, glm::vec3(0.8, 0.8, 0.8));
	model1 = glm::rotate(model1, carAngle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	car.Draw(depthMapShader);
	
	if (manInCar == false) {
		
		model1 = glm::translate(model, glm::vec3(manX, 0, manZ));
		model1 = glm::scale(model1, glm::vec3(0.45, 0.45, 0.45));
		model1 = glm::rotate(model1, manAngle, glm::vec3(0, 1, 0));
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	    man.Draw(depthMapShader);
	}
	
	model1 = glm::translate(model, glm::vec3(12+tractorX, 0.01, -8+tractorZ));
	model1 = glm::scale(model1, glm::vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	tractor.Draw(depthMapShader);
	
	model1 = glm::translate(model, glm::vec3(-5, 1.5, -3));
	model1 = glm::scale(model1, glm::vec3(0.005, 0.005, 0.005));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	boat.Draw(depthMapShader);
	
	model1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	ground.Draw(depthMapShader);
	
	model1 = glm::translate(model, glm::vec3(-10, 0.01, -6));
	model1 = glm::scale(model1, glm::vec3(0.7, 0.7, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	water.Draw(depthMapShader);
	
	model1 = glm::translate(model, glm::vec3(12, 0.01, -6));
	model1 = glm::scale(model1, glm::vec3(0.5, 0.5, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));
	soil.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);






	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));	

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	

	// ************** Draw Objects ********************

	

	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);

	
	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(-20, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);

	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(-20, 0, -20));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);


	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(0, 0, -20));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);

	
	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(20, 0, -20));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);

	
	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(20, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);
	
	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(-20, 0, 20));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);

	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(0, 0, 20));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);

	model1 = glm::scale(model, glm::vec3(2, 2, 2));
	model1 = glm::translate(model1, glm::vec3(20, 0, 20));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);


	boxHouse.min = house.min;
	boxHouse.max = house.max;

	model1 = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));

	boxHouse.min = model1 * boxHouse.min;
	boxHouse.max = model1 * boxHouse.max;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house.Draw(myCustomShader);



	boxLamp.min = streetLamp.min;
	boxLamp.max = streetLamp.max;

	model1 = glm::translate(model, glm::vec3(6, 0, 6));
	model1 = glm::scale(model1, glm::vec3(0.3, 0.3, 0.3));

	boxLamp.min = model1 * boxLamp.min;
	boxLamp.max = model1 * boxLamp.max;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	streetLamp.Draw(myCustomShader);




	boxDog.min = dog.min;
	boxDog.max = dog.max;

	model1 = glm::translate(model, glm::vec3(6, 0, 7));
	model1 = glm::scale(model1, glm::vec3(0.7, 0.7, 0.7));

	boxDog.min = model1 * boxDog.min;
	boxDog.max = model1 * boxDog.max;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	dog.Draw(myCustomShader);



	boxCar.min = car.min;
	boxCar.max = car.max;

	model1 = glm::translate(model, glm::vec3(carX, 0, carZ));
	model1 = glm::scale(model1, glm::vec3(0.8, 0.8, 0.8));

	boxCar.min = model1 * boxCar.min;
	boxCar.max = model1 * boxCar.max;


	model1 = glm::rotate(model1, carAngle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	car.Draw(myCustomShader);


	if (manInCar == false) {

		boxMan.min = man.min;
		boxMan.max = man.max;

		model1 = glm::translate(model, glm::vec3(manX, 0, manZ));
		model1 = glm::scale(model1, glm::vec3(0.45, 0.45, 0.45));

		boxMan.min = model1 * boxMan.min;
		boxMan.max = model1 * boxMan.max;
		//printVec(minMan);
		model1 = glm::rotate(model1, manAngle, glm::vec3(0, 1, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		man.Draw(myCustomShader);
	}


	model1 = glm::translate(model, glm::vec3(12, 0.01, -6));
	model1 = glm::scale(model1, glm::vec3(0.5, 0.5, 1));
	//model1 = glm::rotate(model1, carAngle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	soil.Draw(myCustomShader);

	tractorMove();


	boxTractor.min = tractor.min;
	boxTractor.max = tractor.max;

	model1 = glm::translate(model, glm::vec3(12 + tractorX, 0.01, -8 + tractorZ));
	model1 = glm::scale(model1, glm::vec3(0.5, 0.5, 0.5));

	boxTractor.min = model1 * boxTractor.min;
	boxTractor.max = model1 * boxTractor.max;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	tractor.Draw(myCustomShader);


	boxWater.min = water.min;
	boxWater.max = water.max;

	model1 = glm::translate(model, glm::vec3(-10, 0.01, -6));
	model1 = glm::scale(model1, glm::vec3(0.7, 0.7, 1));
	
	boxWater.min = model1 * boxWater.min;
	boxWater.max = model1 * boxWater.max;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	water.Draw(myCustomShader);


	model1 = glm::translate(model, glm::vec3(-5, 1.5, -3));
	model1 = glm::scale(model1, glm::vec3(0.005, 0.005, 0.005));
	//model1 = glm::rotate(model1, carAngle, glm::vec3(0, 1, 0));sky
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	boat.Draw(myCustomShader);


	model1 = glm::translate(model, glm::vec3(-0.5, 0, 3.5));
	model1 = glm::scale(model1, glm::vec3(0.6, 0.6, 0.45));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	moskvitch.Draw(myCustomShader);

	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));

	mySkyBox.Draw(skyboxShader, view, projection);
	




	//create model matrix for nanosuit
/*	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	model1 = glm::translate(model, glm::vec3(manX, 0, manZ));
	model1 = glm::rotate(model1, manAngle, glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));

	//compute normal matrix
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	man.Draw(myCustomShader);
		
	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	ground.Draw(myCustomShader);




	model1 = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model1));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house.Draw(myCustomShader);

	//draw a white cube around the light

	/*lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, lightDir);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(lightShader);*/
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	glCheckError();
	initUniforms();	
	//glCheckError();

	std::vector<const GLchar*> faces;
	/*faces.push_back("textures/skybox/right.tga");
	faces.push_back("textures/skybox/left.tga");
	faces.push_back("textures/skybox/top.tga");
	faces.push_back("textures/skybox/bottom.tga");
	faces.push_back("textures/skybox/back.tga");
	faces.push_back("textures/skybox/front.tga");*/

	faces.push_back("textures/ely_hills/hills_rt.tga");
	faces.push_back("textures/ely_hills/hills_lf.tga");
	faces.push_back("textures/ely_hills/hills_up.tga");
	faces.push_back("textures/ely_hills/hills_dn.tga");
	faces.push_back("textures/ely_hills/hills_bk.tga");
	faces.push_back("textures/ely_hills/hills_ft.tga");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");


	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
