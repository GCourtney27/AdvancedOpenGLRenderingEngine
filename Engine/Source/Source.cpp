
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Objects/Geometry/Model.h"
#include "Objects/Camera/Camera.h"
#include "Objects/Lights/Lights.h"

#define ThrowError(x) throw std::runtime_error(x)

#define IMGUI_NEW_FRAME \
	ImGui_ImplOpenGL3_NewFrame();\
	ImGui_ImplGlfw_NewFrame();\
	ImGui::NewFrame();\

#define IMGUI_RENDER \
	ImGui::Render();\
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());\

int g_windowWidth = 1600;
int g_windowHeight = 900;
int g_msaaSamples = 4;

float points[] = {
	-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
	 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
	 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
	-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
};

float skyboxVertices[] = {
	// positions          

	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
};

float planeVertices[] = {
	// positions            // normals         // texcoords
	 25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,  25.0f,  0.0f,
	-25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,   0.0f,  0.0f,
	-25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,   0.0f, 25.0f,

	 25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,  25.0f,  0.0f,
	-25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,   0.0f, 25.0f,
	 25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,  25.0f, 25.0f
};

// Lights
//-------
PointLight pointLight;
SpotLight spotLight;
DirectionalLight dirLight;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
unsigned int planeVAO, planeVBO;
unsigned int floorTextureGammaCorrected;
unsigned int floorSpecTextureGammaCorrected;
unsigned int cubemapTexture;
Model fileModel;

float lastX = g_windowWidth / 2.0f;
float lastY = g_windowHeight / 2.0f;
bool g_firstMouse = true;

bool gammaEnabled = false;
bool gammaKeyPressed = false;

float g_deltaTime = 0.0f; // Time between current frame and last frame
float g_lastFrame = 0.0f; // Time of last frame

// GLFW Callbacks
//---------------
void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int loadTexture(char const * path, bool gammaCorrection);
unsigned int loadCubeMap(std::vector<std::string> faces);
void ProcessInput(GLFWwindow* pWindow);
void RenderScene(const Shader& shader);
void renderCube();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, g_msaaSamples);

	GLFWwindow* pWindow = glfwCreateWindow(g_windowWidth, g_windowHeight, "OpenGL Engine!", NULL, NULL);
	if (pWindow == nullptr)
	{
		ThrowError("Failed to initialize glfw window!");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(pWindow);

	// Register callback functions
	glfwSetFramebufferSizeCallback(pWindow, frame_buffer_size_callback);
	glfwSetCursorPosCallback(pWindow, mouse_callback);
	glfwSetScrollCallback(pWindow, scroll_callback);

	//glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	gladLoadGL();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ThrowError("Failed to initialize GLAD!");
		return -1;
	}
	glEnable(GL_MULTISAMPLE);

	glViewport(0, 0, g_windowWidth, g_windowHeight);

	// Skybox VAO/VBO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// Geometry Shader VAO/VBO
	unsigned int geometryVAO, geometryVBO;
	glGenVertexArrays(1, &geometryVAO);
	glGenBuffers(1, &geometryVBO);
	glBindVertexArray(geometryVAO);
	glBindBuffer(GL_ARRAY_BUFFER, geometryVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

	// Floor plane VAO
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// Screen quad VAO/VBO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Create msaa buffer
	unsigned int frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	// Create the texture color buffer
	unsigned int textureColorBufferMultiSampled;
	glGenTextures(1, &textureColorBufferMultiSampled);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, g_msaaSamples, GL_RGB, g_windowWidth, g_windowHeight, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
	// Create render buffer
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, g_windowWidth, g_windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ThrowError("ERROR::FRAMEBUFFER::Framebuffer is not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create post process screen buffer; We need to create a regular texture to ba able to apply it to the screen quad in the shader
	unsigned int intermediateFBO;
	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
	unsigned int screenTexture;
	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_windowWidth, g_windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Shader screenShader("Shaders/ScreenQuadPostProcess.vert", "Shaders/ScreenQuadPostProcess.frag");
	screenShader.SetInt("screenTexture", 0);

	// Generate shadow map frame buffer
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Shader lightingDepthShader("Shaders/lightDepthPass.vert", "Shaders/lightDepthPass.frag");

	// Create cube map
	std::vector<std::string> faces{
		"../Assets//Skyboxes/Lake/right.jpg",
		"../Assets//Skyboxes/Lake/left.jpg",
		"../Assets//Skyboxes/Lake/top.jpg",
		"../Assets//Skyboxes/Lake/bottom.jpg",
		"../Assets//Skyboxes/Lake/front.jpg",
		"../Assets//Skyboxes/Lake/back.jpg",
	};
	cubemapTexture = loadCubeMap(faces);

	// Compile shaders
	Shader lightingShader("Shaders/VertexShader.vert", "Shaders/FragmentShader.frag");
	Shader lampShader("Shaders/lamp.vert", "Shaders/lamp.frag");
	Shader skyboxShader("Shaders/Skybox.vert", "Shaders/Skybox.frag");
	Shader geometryShader("Shaders/GPUGeometry.vert", "Shaders/GPUGeometry.frag", "Shaders/GPUGeometry.geom");

	unsigned int ubiLightingShader = glGetUniformBlockIndex(lightingShader.ProgramID, "Matrices");
	glUniformBlockBinding(lightingShader.ProgramID, ubiLightingShader, 0);
	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
	glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom), (float)g_windowWidth / (float)g_windowHeight, 0.1f, 100.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMat));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	floorTextureGammaCorrected = loadTexture("../Assets/Textures/Planks_Diff.png", true);
	floorSpecTextureGammaCorrected = loadTexture("../Assets/Textures/Planks_Spec.png", true);

	fileModel.Init("../Assets/Models/nanosuit/nanosuit.obj");
	//fileModel.Init("../Assets/Models/sponza/sponza.obj");
	Model light("../Assets/Models/Primatives/Cube.obj");

	pointLight.ambient = glm::vec3(0.05f);
	pointLight.diffuse = glm::vec3(0.8f);
	pointLight.specular = glm::vec3(1.0f);
	pointLight.linear = 0.09f;
	pointLight.quadratic = 0.032f;

	bool postProcessEnabled = true;
	bool filmGrainEnabled = true;
	float filmgrainStrength = 16.0f;
	bool vignetteEnabled = true;
	float vignetteInnerRadius = 0.1f;
	float vignetteOuterRadius = 1.0f;
	float vignetteOpacity = 1.0f;

	float near_plane = 1.0f, far_plane = 27.5f;
	camera.Position = glm::vec3(-2.0f, 4.0f, 1.0f);
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(pWindow))
	{
		float currentTime = static_cast<float>(glfwGetTime());
		g_deltaTime = currentTime - g_lastFrame;
		g_lastFrame = currentTime;

		ProcessInput(pWindow);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		IMGUI_NEW_FRAME;
		
		ImGui::Begin("Light Camera");
		{
			ImGui::DragFloat3("Position", &lightPos.x, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Near Plane", &near_plane, 0.1f, -50.0f, 100.0f);
			ImGui::DragFloat("Far Plane", &far_plane, 0.1f, -50.0f, 100.0f);
		}
		ImGui::End();
		// Shadow Depth Pre-Pass
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		lightingDepthShader.Use();
		lightingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_CULL_FACE);
			RenderScene(lightingDepthShader);
			glEnable(GL_CULL_FACE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ImGui::Begin("Shadow Depth Pre-Pass Result");
		{
			ImGui::GetWindowDrawList()->AddImage(
				(void *)depthMap,
				ImVec2(ImGui::GetCursorScreenPos()),
				ImVec2(ImGui::GetCursorScreenPos().x + g_windowWidth / 2,
					ImGui::GetCursorScreenPos().y + g_windowHeight / 2), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();

		// Color pass
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, g_windowWidth, g_windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// View transformations
		glm::mat4 viewMat = camera.GetViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMat));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
			   
		lightingShader.Use();
		lightingShader.SetMat4("projection", projectionMat);
		lightingShader.SetMat4("view", viewMat);
		lightingShader.SetVec3("viewPos", camera.Position);

		spotLight.position = glm::vec3(0.0f, -2.0f, 0.0f);
		spotLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);

		lightingShader.SetPointLight("pointLights[0]", pointLight);
		lightingShader.SetDirectionalLight("dirLight", dirLight);
		lightingShader.SetSpotLight("spotLight", spotLight);
		lightingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		lightingShader.SetInt("material.texture_diffuse1", 0);
		lightingShader.SetInt("material.texture_specular1", 1);
		lightingShader.SetInt("shadowMap", 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTextureGammaCorrected);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, floorSpecTextureGammaCorrected);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderScene(lightingShader);
		
		// Draw skybox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.Use();
		glm::mat4 skyBoxViewMat = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.SetMat4("projection", projectionMat);
		skyboxShader.SetMat4("view", skyBoxViewMat);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// PostProcess pass
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
		glBlitFramebuffer(0, 0, g_windowWidth, g_windowHeight, 0, 0, g_windowWidth, g_windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screenShader.Use();
		screenShader.SetFloat("time", (float)glfwGetTime());
		ImGui::Begin("Post Processing");
		{
			ImGui::Text("Film Grain");
			ImGui::Checkbox("FG Enabled", &filmGrainEnabled);
			ImGui::DragFloat("Strength", &filmgrainStrength, 0.1f, 0.0f, 80.0f);


			ImGui::Text("Vignette");
			ImGui::Checkbox("VN Enabled", &vignetteEnabled);
			ImGui::DragFloat("Inner Radius", &vignetteInnerRadius, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Outer Radius", &vignetteOuterRadius, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Opacity", &vignetteOpacity, 0.1f, 0.0f, 10.0f);
		}
		ImGui::End();
		screenShader.SetFloat("filmgrainEnabled", filmGrainEnabled);
		screenShader.SetFloat("grainStrength", filmgrainStrength);
		screenShader.SetFloat("vignetteEnabled", vignetteEnabled);
		screenShader.SetFloat("vignetteInnerRadius", vignetteInnerRadius);
		screenShader.SetFloat("vignetteOuterRadius", vignetteOuterRadius);
		screenShader.SetFloat("vignetteOpacity", vignetteOpacity);
		screenShader.SetFloat("near_plane", near_plane);
		screenShader.SetFloat("far_plane", far_plane);
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screenTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		IMGUI_RENDER;

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteVertexArrays(1, &geometryVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteFramebuffers(sizeof(intermediateFBO), &intermediateFBO);
	glDeleteFramebuffers(sizeof(frameBuffer), &frameBuffer);
	glDeleteRenderbuffers(sizeof(rbo), &rbo);
	glDeleteTextures(sizeof(floorSpecTextureGammaCorrected), &floorSpecTextureGammaCorrected);
	glDeleteTextures(sizeof(floorTextureGammaCorrected), &floorTextureGammaCorrected);

	CleanUp();

	return 0;
}

void CleanUp()
{
	fileModel.Destroy();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void RenderScene(const Shader& shader)
{
	// floor
	glm::mat4 floorMat = glm::mat4(1.0f);
	//floorMat = glm::scale(floorMat, glm::vec3(0.5f));
	floorMat = glm::rotate(floorMat, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	floorMat = glm::translate(floorMat, glm::vec3(0.0f, 1.5f, 0.0f));
	shader.SetMat4("model", floorMat);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// cubes
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.SetMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.SetMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.SetMat4("model", model);
	renderCube();

	// render the loaded model
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::scale(modelMat, glm::vec3(0.5f));
	modelMat = glm::translate(modelMat, glm::vec3(0.0f, -1.75f, -2.0f));
	shader.SetMat4("model", modelMat);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	fileModel.Draw(shader);

}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

unsigned int loadTexture(char const * path, bool gammaCorrection)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubeMap(std::vector<std::string> faces)
{
	unsigned int cubeMapTextureID;
	glGenTextures(1, &cubeMapTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);

	int width, height, nrChannels;
	for (int i = 0; i < faces.size(); i++)
	{
		unsigned char* data;
		data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load cube map texture at path: " << faces[i] << "\n";
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return cubeMapTextureID;
}

void ProcessInput(GLFWwindow* pWindow)
{
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);

	float camSpeedScale = 1.0f;
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camSpeedScale = 3.0f;

	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_C) == GLFW_PRESS)
		pointLight.position = camera.Position;

	if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS && !gammaKeyPressed)
	{
		gammaEnabled = !gammaEnabled;
		gammaKeyPressed = true;
	}

}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow * pWindow, double xpos, double ypos)
{
	if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
		return;
	
	if (g_firstMouse)
	{
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		g_firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}