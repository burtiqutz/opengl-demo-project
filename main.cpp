//
//  main.cpp with General Collision System
//  OpenGL Advances Lighting
//

#if defined (__APPLE__)
	#define GLFW_INCLUDE_GLCOREARB
	#define GL_SILENCE_DEPRECATION
#else
	#define GLEW_STATIC
	#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <vector>
#include <string>

int glWindowWidth = 1024;
int glWindowHeight = 768;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

bool isSunOn;
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::mat4 lightRotation;

const unsigned int SHADOW_WIDTH = 4092;
const unsigned int SHADOW_HEIGHT = 4092;
GLuint shadowMapFBO;
GLuint depthMapTexture;

bool isPosOn;
glm::vec3 lightPos;
GLuint lightPosLoc;
glm::vec3 posColor;
GLuint posColorLoc;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

gps::Camera myCamera(
	glm::vec3(0.0f, 0.0f, 2.5f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 2.f;
float delta = 0.0f;

bool pressedKeys[1024];
float angleY = 0.0f;

float lastX = retina_width / 2, lastY = retina_height / 2;
constexpr float SENSITIVITY = 0.1f;
float pitch = 0.0f;
float yaw = -90.0f;
bool firstMouse = true;

// Models
gps::Model3D teapot;
gps::Model3D ground;
gps::Model3D test;

gps::Shader myCustomShader;
gps::Shader depthShader;

bool wireframe = false;
bool flatShading = false;

struct SceneObject {
    gps::Model3D* model;
    glm::mat4 modelMatrix;
    std::string name;
    bool hasCollision;

    SceneObject(gps::Model3D* m, const glm::mat4& mat, const std::string& n, bool collision = true)
        : model(m), modelMatrix(mat), name(n), hasCollision(collision) {}
};

std::vector<SceneObject> sceneObjects;

//	Collision detection functions
bool checkAABBCollision(const gps::BoundingBox& box1, const gps::BoundingBox& box2) {
    return (box1.min.x <= box2.max.x && box1.max.x >= box2.min.x) &&
           (box1.min.y <= box2.max.y && box1.max.y >= box2.min.y) &&
           (box1.min.z <= box2.max.z && box1.max.z >= box2.min.z);
}

gps::BoundingBox transformBoundingBox(const gps::BoundingBox& aabb, const glm::mat4& modelMatrix) {
    glm::vec3 corners[8] = {
        glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
        glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
        glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z)
    };

    gps::BoundingBox transformed;
    transformed.min = glm::vec3(std::numeric_limits<float>::max());
    transformed.max = glm::vec3(std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; i++) {
        glm::vec4 transformedCorner = modelMatrix * glm::vec4(corners[i], 1.0f);
        glm::vec3 corner3D = glm::vec3(transformedCorner);
        transformed.min = glm::min(transformed.min, corner3D);
        transformed.max = glm::max(transformed.max, corner3D);
    }

    return transformed;
}

bool checkPlayerCollision(const gps::BoundingBox& playerBox, std::string* collidedObjectName = nullptr) {
    for (const auto& obj : sceneObjects) {
        if (!obj.hasCollision) continue;

        gps::BoundingBox objectBox = transformBoundingBox(obj.model->GetBoundingBox(), obj.modelMatrix);

        if (checkAABBCollision(playerBox, objectBox)) {
            if (collidedObjectName) {
                *collidedObjectName = obj.name;
            }
            return true;
        }
    }
    return false;
}

//	Scene management
void initSceneObjects() {
    sceneObjects.clear();

    // Teapot
    glm::mat4 teapotModel = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    sceneObjects.emplace_back(&teapot, teapotModel, "Teapot", true);

    // Ground
    glm::mat4 groundModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    groundModel = glm::scale(groundModel, glm::vec3(0.5f));
    sceneObjects.emplace_back(&ground, groundModel, "Ground", true);

    // Add more objects here as you create them:
    // Example walls, buildings, trees, etc.
}

void updateSceneObjects() {
    // Update teapot rotation
    if (!sceneObjects.empty()) {
        sceneObjects[0].modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Update other dynamic objects here
}

// Helper function to add objects to scene
void addSceneObject(gps::Model3D* model, const glm::vec3& position,
                    const glm::vec3& scale = glm::vec3(1.0f),
                    const glm::vec3& rotation = glm::vec3(0.0f),
                    const std::string& name = "Object",
                    bool hasCollision = true) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, scale);

    sceneObjects.emplace_back(model, modelMatrix, name, hasCollision);
}

//	Main logic
GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
}

void processLights() {
	if (pressedKeys[GLFW_KEY_P]) {
		if (isPosOn) {
			isPosOn = false;
			posColor = glm::vec3(0.f, 0.f, 0.f);
		} else {
			isPosOn = true;
			posColor = glm::vec3(1.f, 1.f, 1.f);
		}
		glUniform3fv(posColorLoc, 1, glm::value_ptr(posColor));
	}
	if (pressedKeys[GLFW_KEY_G]) {
		if (isSunOn) {
			isSunOn = false;
			lightColor = glm::vec3(0.f, 0.f, 0.f);
		} else {
			isSunOn = true;
			lightColor = glm::vec3(1.f, 1.f, 1.f);
		}
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	if (key == GLFW_KEY_P && action == GLFW_PRESS || key == GLFW_KEY_G && action == GLFW_PRESS) {
		processLights();
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		wireframe = !wireframe;
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}
	float xoffset = (float)(xpos - lastX);
	float yoffset = (float)(lastY - ypos);
	lastX = (float)xpos;
	lastY = (float)ypos;

	yaw += xoffset * SENSITIVITY;
	pitch += yoffset * SENSITIVITY;
	pitch = glm::clamp(pitch, -89.f, 89.f);

	myCamera.rotate(pitch, yaw);
}

float rotationSpeed = 100.f;

void processMovement() {
	glm::vec3 previousPosition = myCamera.getPosition();

	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= rotationSpeed * delta;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += rotationSpeed * delta;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, delta);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, delta);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, delta);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, delta);
	}

	// Update dynamic objects
	updateSceneObjects();

	// Check collision
	gps::BoundingBox playerBox = myCamera.GetPlayerBox();
	std::string collidedObject;

	if (checkPlayerCollision(playerBox, &collidedObject)) {
		myCamera.setPosition(previousPosition);
	}
}

bool initOpenGLWindow() {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Project", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(glWindow);
	glfwSwapInterval(1);

#if not defined (__APPLE__)
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
	return true;
}

void initOpenGLState() {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0);
	glViewport(0, 0, retina_width, retina_height);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	teapot.LoadModel("models/teapot/teapot20segUT.obj", "models/teapot/");
	ground.LoadModel("models/ground/ground.obj", "models/ground/");
	// Initialize scene objects after loading models
	initSceneObjects();

	// Add more objects to scene
	// addSceneObject(&test, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), "Test Object");
}

void initShaders() {
	myCustomShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
	myCustomShader.useShaderProgram();
	depthShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	isSunOn = true;
	isPosOn = true;
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightPos = glm::vec3(0.0f, 0.0f, 5.0f);
	lightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	posColor = glm::vec3(1.0f, 1.0f, 1.0f);
	posColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "posColor");
	glUniform3fv(posColorLoc, 1, glm::value_ptr(posColor));
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkybox() {
	faces.push_back("skybox/arctic_rt.tga");
	faces.push_back("skybox/arctic_lf.tga");
	faces.push_back("skybox/arctic_up.tga");
	faces.push_back("skybox/arctic_dn.tga");
	faces.push_back("skybox/arctic_bk.tga");
	faces.push_back("skybox/arctic_ft.tga");
	mySkyBox.Load(faces);
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	constexpr GLfloat near_plane = 0.1f, far_plane = 6.0f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	return lightProjection * lightView;
}

void drawObjects(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();

	// Draw all scene objects
	for (const auto& obj : sceneObjects) {
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
						  1, GL_FALSE, glm::value_ptr(obj.modelMatrix));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * obj.modelMatrix));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		obj.model->Draw(shader);
	}
}

float lastTimeStamp = glfwGetTime();
int frameCount = 0;
int lastFPSTime = 0;

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double currentTimeStamp = glfwGetTime();
	double deltaTime = currentTimeStamp - lastTimeStamp;
	lastTimeStamp = currentTimeStamp;
	delta = static_cast<float>(deltaTime * cameraSpeed);

	frameCount++;
	if (currentTimeStamp - lastFPSTime >= 1.0) {
		double fps = (double)frameCount / (currentTimeStamp - lastFPSTime);
		std::cout << "FPS: " << fps << std::endl;
		lastFPSTime = currentTimeStamp;
		frameCount = 0;
	}

	// Shadow map pass
	depthShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceTrMatrix"),
		1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Main render pass
	glViewport(0, 0, retina_width, retina_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	myCustomShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	glm::vec4 lightPosEye = view * glm::vec4(lightPos, 1.0f);
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(lightPosEye)));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	glUniform3fv(posColorLoc, 1, glm::value_ptr(posColor));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	drawObjects(myCustomShader, false);
	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glfwDestroyWindow(glWindow);
	glfwTerminate();
}

int main(int argc, const char * argv[]) {
	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();
	return 0;
}